// ==========================================================================
//
// GenLine.cpp			JIT Class for 3D Rendering Library
//
//						This file contains the rasterizer functions that
//						implement the runtime code generation support
//						for optimized scan line rasterization routines.
//
// --------------------------------------------------------------------------
//
// 12-29-2003		Hans-Martin Will	initial version
//
// --------------------------------------------------------------------------
//
// Copyright (c) 2004, Hans-Martin Will. All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are 
// met:
// 
//	 *  Redistributions of source code must retain the above copyright
// 		notice, this list of conditions and the following disclaimer. 
//   *	Redistributions in binary form must reproduce the above copyright
// 		notice, this list of conditions and the following disclaimer in the 
// 		documentation and/or other materials provided with the distribution. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
// THE POSSIBILITY OF SUCH DAMAGE.
//
// ==========================================================================


#include "stdafx.h"
#include "CodeGenerator.h"
#include "Rasterizer.h"
#include "FunctionCache.h"
#include "Surface.h"
#include "Texture.h"
#include "codegen.h"
#include "instruction.h"
#include "emit.h"
#include "arm-dis.h"


using namespace EGL;


namespace {

	inline cg_virtual_reg_t * LOAD_DATA(cg_block_t * block, cg_virtual_reg_t * base, I32 constant) {
		cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
		cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);
		cg_virtual_reg_t * value = cg_virtual_reg_create(block->proc, cg_reg_type_general);

		LDI(offset, constant);
		ADD(addr, base, offset);
		LDW(value, addr);

		return value;
	}

#define ALLOC_REG(reg) reg = cg_virtual_reg_create(procedure, cg_reg_type_general)
#define ALLOC_FLAGS(reg) reg = cg_virtual_reg_create(procedure, cg_reg_type_flags)
#define DECL_REG(reg) cg_virtual_reg_t * reg = cg_virtual_reg_create(procedure, cg_reg_type_general)
#define DECL_FLAGS(reg) cg_virtual_reg_t * reg = cg_virtual_reg_create(procedure, cg_reg_type_flags)
#define DECL_CONST_REG(reg, value) cg_virtual_reg_t * reg = cg_virtual_reg_create(procedure, cg_reg_type_general); LDI(reg, value)

	U32 Log(U32 value) {
		U32 result = 0;
		U32 mask = 1;

		while ((value & mask) != value) {
			++result;
			mask = (mask << 1) | 1;
		}

		return result;
	}
}




void CodeGenerator :: GenerateRasterLine() {

	cg_proc_t * procedure = cg_proc_create(m_Module);

	// The signature of the generated function is:
	//	(const RasterInfo * info, const EdgePos& start, const EdgePos& end);
	// Do not pass in y coordinate but rather assume that raster info pointers have been
	// adjusted to point to current scanline in memory
	// In the edge buffers, z, tu and tv are actually divided by w

	DECL_REG	(regInfo);		// virtual register containing info structure pointer
	DECL_REG	(regFrom);		// virtual register containing start RasterPos pointer
	DECL_REG	(regTo);		// virtual register containing end RasterPos pointer

	procedure->num_args = 3;	// the previous three declarations make up the arguments

	cg_block_t * block = cg_block_create(procedure, 1);
	cg_block_ref_t * blockRefEndProc = cg_block_ref_create(procedure);


	// EGL_Fixed deltaX = p_to.m_WindowCoords.x - p_from.m_WindowCoords.x;
	// EGL_Fixed deltaY = p_to.m_WindowCoords.y - p_from.m_WindowCoords.y;
	DECL_REG	(regDeltaX);
	DECL_REG	(regDeltaY);

	cg_virtual_reg_t *	regToX		= LOAD_DATA(block, regTo, OFFSET_RASTER_POS_WINDOW_X);
	cg_virtual_reg_t *	regFromX	= LOAD_DATA(block, regTo, OFFSET_RASTER_POS_WINDOW_X);
	cg_virtual_reg_t *	regToY		= LOAD_DATA(block, regTo, OFFSET_RASTER_POS_WINDOW_Y);
	cg_virtual_reg_t *	regFromY	= LOAD_DATA(block, regTo, OFFSET_RASTER_POS_WINDOW_Y);

	FSUB		(regDeltaX, regToX, regFromX);
	FSUB		(regDeltaY, regToY, regFromY);

	// if (EGL_Abs(deltaX) > EGL_Abs(deltaY)) {

	DECL_FLAGS	(regCompareXY);
	FCMP		(regCompareXY, regDeltaY, regDeltaX);

	cg_block_ref_t * blockRefRasterY = cg_block_ref_create(procedure);

	BGT			(regCompareXY, blockRefRasterY);

		// Bresenham along x-axis

	// 	const RasterPos *start, *end;

	// 	I32 x;
	// 	I32 endX;
	// 	EGL_Fixed roundedX;

	// 	if (deltaX < 0) {
	DECL_REG		(regSignX);
	DECL_CONST_REG	(regZero, 0);
	
	FCMP		(regSignX, regDeltaX, regZero);

	cg_block_ref_t * blockRefPositiveDeltaX = cg_block_ref_create(procedure);

	BLT			(regSignX, blockRefPositiveDeltaX);

	block = cg_block_create(procedure, 1);

	// 		deltaY = -deltaY;
	// 		deltaX = -deltaX;
	DECL_REG	(regMinusDeltaX);
	DECL_REG	(regMinusDeltaY);

	FNEG		(regMinusDeltaX, regDeltaX);
	FNEG		(regMinusDeltaY, regDeltaY);

	// 		start = &p_to;
	// 		end = &p_from;
	DECL_REG	(regStart0);
	DECL_REG	(regEnd0);

	OR			(regStart0, regTo, regTo);
	OR			(regEnd0, regFrom, regFrom);

	// 		roundedX = EGL_NearestInt(p_to.m_WindowCoords.x + 1);
	// 		x = EGL_IntFromFixed(roundedX);
	// 		endX = EGL_IntFromFixed(p_from.m_WindowCoords.x + ((EGL_ONE)/2));
	DECL_REG	(regX0);
	DECL_REG	(regEndX0);
	DECL_CONST_REG	(regHalf, 0x8000);
	DECL_REG	(regXToPlusHalf0);
	DECL_REG	(regXFromPlusHalf0);

	FADD		(regXToPlusHalf0, regToX, regHalf);
	TRUNC		(regX0, regXToPlusHalf0);
	FADD		(regXFromPlusHalf0, regFromX, regHalf);
	TRUNC		(regEndX0, regXFromPlusHalf0);

	cg_block_ref_t * blockRefPostDeltaX = cg_block_ref_create(procedure);

	BRA			(blockRefPostDeltaX);

	// 	} else {
	block = cg_block_create(procedure, 1);
	blockRefPositiveDeltaX->block = block;

	// 		start = &p_from;
	// 		end = &p_to;
	DECL_REG	(regStart1);
	DECL_REG	(regEnd1);

	OR			(regStart1, regFrom, regFrom);
	OR			(regEnd1, regTo, regTo);

	// 		roundedX = EGL_NearestInt(p_from.m_WindowCoords.x);
	// 		x = EGL_IntFromFixed(roundedX);
	// 		endX = EGL_IntFromFixed(p_to.m_WindowCoords.x + ((EGL_ONE)/2-1));
	DECL_REG	(regX1);
	DECL_REG	(regEndX1);
	DECL_CONST_REG	(regHalf1, 0x7fff);
	DECL_REG	(regXToPlusHalf1);
	DECL_REG	(regXFromPlusHalf1);

	FADD		(regXToPlusHalf1, regToX, regHalf1);
	TRUNC		(regEndX1, regXToPlusHalf1);
	FADD		(regXFromPlusHalf1, regFromX, regHalf1);
	TRUNC		(regX1, regXFromPlusHalf1);

	// 	}

	block = cg_block_create(procedure, 1);
	blockRefPostDeltaX->block = block;

	DECL_REG	(regCommonDeltaX);
	DECL_REG	(regCommonDeltaY);

	PHI			(regCommonDeltaX, cg_create_virtual_reg_list(procedure->module->heap, regDeltaX, regMinusDeltaX, NULL));
	PHI			(regCommonDeltaY, cg_create_virtual_reg_list(procedure->module->heap, regDeltaY, regMinusDeltaY, NULL));

	DECL_REG	(regCommonX);
	DECL_REG	(regCommonEndX);

	PHI			(regCommonDeltaX, cg_create_virtual_reg_list(procedure->module->heap, regX0, regX1, NULL));
	PHI			(regCommonDeltaY, cg_create_virtual_reg_list(procedure->module->heap, regEndX0, regEndX1, NULL));

	// 	const RasterPos& from = *start;
	// 	const RasterPos& to = *end;

	DECL_REG	(regCommonFrom);
	DECL_REG	(regCommonTo);

	PHI			(regCommonFrom, cg_create_virtual_reg_list(procedure->module->heap, regStart0, regStart1, NULL));
	PHI			(regCommonTo, cg_create_virtual_reg_list(procedure->module->heap, regEnd0, regEnd1, NULL));

	// 	I32 yIncrement = (deltaY > 0) ? 1 : -1;
	DECL_REG		(regSignY);
	DECL_CONST_REG	(regZero, 0);
	
	FCMP		(regSignY, regCommonDeltaX, regZero);

	cg_block_ref_t * blockRefPositiveDeltaY = cg_block_ref_create(procedure);

	BLT			(regSignY, blockRefPositiveDeltaY);

	block = cg_block_create(procedure, 1);

	DECL_CONST_REG	(regYIncrementNeg, -1);

	cg_block_ref_t * blockRefPostDeltaX = cg_block_ref_create(procedure);

	BRA			(blockRefPostDeltaX);

	block = cg_block_create(procedure, 1);
	blockRefPositiveDeltaX->block = block;

	DECL_CONST_REG	(regYIncrementPos, 1);

	block = cg_block_create(procedure, 1);
	blockRefPostDeltaY->block = block;

	DECL_REG		(regYIncrement);
	PHI				(regYIncrement, cg_create_virtual_reg_list(procedure->module->heap, regYIncrementPos, regYIncrementNeg, NULL));

		// -- initialize with starting vertex attributes
	// 	FractionalColor baseColor = from.m_Color;
	// 	EGL_Fixed OneOverZ = from.m_WindowCoords.invZ;
	// 	EGL_Fixed tuOverZ = EGL_Mul(from.m_TextureCoords.tu, OneOverZ);
	// 	EGL_Fixed tvOverZ = EGL_Mul(from.m_TextureCoords.tv, OneOverZ);
	// 	EGL_Fixed fogDensity = from.m_FogDensity;
	// 	EGL_Fixed depth = from.m_WindowCoords.depth;

	cg_virtual_reg_t * regColorR0 = LOAD_DATA(block, regCommonFrom, OFFSET_RASTER_POS_COLOR_R);
	cg_virtual_reg_t * regColorG0 = LOAD_DATA(block, regCommonFrom, OFFSET_RASTER_POS_COLOR_G);
	cg_virtual_reg_t * regColorB0 = LOAD_DATA(block, regCommonFrom, OFFSET_RASTER_POS_COLOR_B);
	cg_virtual_reg_t * regColorA0 = LOAD_DATA(block, regCommonFrom, OFFSET_RASTER_POS_COLOR_A);
	cg_virtual_reg_t * regInvZ0 = LOAD_DATA(block, regCommonFrom, OFFSET_RASTER_POS_WINDOW_INV_Z);
	cg_virtual_reg_t * regTu0 = LOAD_DATA(block, regCommonFrom, OFFSET_RASTER_POS_TEX_TU);
	cg_virtual_reg_t * regTv0 = LOAD_DATA(block, regCommonFrom, OFFSET_RASTER_POS_TEX_TV);
	cg_virtual_reg_t * regFog0 = LOAD_DATA(block, regCommonFrom, OFFSET_RASTER_POS_FOG);
	cg_virtual_reg_t * regDepth0 = LOAD_DATA(block, regCommonFrom, OFFSET_RASTER_POS_WINDOW_DEPTH);

	DECL_REG	(regTuOverZ0);
	DECL_REG	(regTvOverZ0);

	FMUL		(regTuOverZ0, regTu0, regInvZ0);
	FMUL		(regTvOverZ0, regTv0, regInvZ0);

		// -- end initialize

	// 	EGL_Fixed invSpan = EGL_Inverse(deltaX);
	// 	EGL_Fixed slope = EGL_Mul(EGL_Abs(deltaY), invSpan);
	// 	EGL_Fixed OneOverZTo = to.m_WindowCoords.invZ;
	DECL_REG	(regInvSpan);
	DECL_REG	(regSlope);
	DECL_REG	(regAbsDeltaY);

	FINV		(regInvSpan, regCommonDeltaX);
	ABS			(regAbsDeltaY, regCommonDeltaY);
	FMUL		(regSlope, regAbsDeltaY, regInvSpan);

	cg_virtual_reg_t * regEndColorR0 = LOAD_DATA(block, regCommonTo, OFFSET_RASTER_POS_COLOR_R);
	cg_virtual_reg_t * regEndColorG0 = LOAD_DATA(block, regCommonTo, OFFSET_RASTER_POS_COLOR_G);
	cg_virtual_reg_t * regEndColorB0 = LOAD_DATA(block, regCommonTo, OFFSET_RASTER_POS_COLOR_B);
	cg_virtual_reg_t * regEndColorA0 = LOAD_DATA(block, regCommonTo, OFFSET_RASTER_POS_COLOR_A);
	cg_virtual_reg_t * regEndInvZ0 = LOAD_DATA(block, regCommonTo, OFFSET_RASTER_POS_WINDOW_INV_Z);
	cg_virtual_reg_t * regEndTu0 = LOAD_DATA(block, regCommonTo, OFFSET_RASTER_POS_TEX_TU);
	cg_virtual_reg_t * regEndTv0 = LOAD_DATA(block, regCommonTo, OFFSET_RASTER_POS_TEX_TV);
	cg_virtual_reg_t * regEndFog0 = LOAD_DATA(block, regCommonTo, OFFSET_RASTER_POS_FOG);
	cg_virtual_reg_t * regEndDepth0 = LOAD_DATA(block, regCommonTo, OFFSET_RASTER_POS_WINDOW_DEPTH);

	DECL_REG	(regEndTuOverZ0);
	DECL_REG	(regEndTvOverZ0);

	FMUL		(regEndTuOverZ0, regEndTu0, regEndInvZ0);
	FMUL		(regEndTvOverZ0, regEndTv0, regEndInvZ0);


	// -- increments(to, from, invSpan)

	// 	FractionalColor colorIncrement = (to.m_Color - from.m_Color) * invSpan;
	// 	EGL_Fixed deltaFog = EGL_Mul(to.m_FogDensity - from.m_FogDensity, invSpan);

	// 	EGL_Fixed deltaZ = EGL_Mul(OneOverZTo - OneOverZ, invSpan);

	// 	EGL_Fixed deltaU = EGL_Mul(EGL_Mul(to.m_TextureCoords.tu, OneOverZTo) -
	// 							   EGL_Mul(from.m_TextureCoords.tu, OneOverZ), invSpan);

	// 	EGL_Fixed deltaV = EGL_Mul(EGL_Mul(to.m_TextureCoords.tv, OneOverZTo) -
	// 							   EGL_Mul(from.m_TextureCoords.tv, OneOverZ), invSpan);

	// 	EGL_Fixed deltaDepth = EGL_Mul(to.m_WindowCoords.depth - from.m_WindowCoords.depth, invSpan);

	DECL_REG		(regDiffColorR);
	DECL_REG		(regDeltaColorR);
	DECL_REG		(regDiffColorG);
	DECL_REG		(regDeltaColorG);
	DECL_REG		(regDiffColorB);
	DECL_REG		(regDeltaColorB);
	DECL_REG		(regDiffColorA);
	DECL_REG		(regDeltaColorA);
	DECL_REG		(regDiffFog);
	DECL_REG		(regDeltaFog);
	DECL_REG		(regDiffDepth);
	DECL_REG		(regDeltaDepth);

	FSUB			(regDiffColorR, regEndColorR0, regColorR0);
	FMUL			(regDeltaColorR, regDiffColorR, regInvSpan);
	FSUB			(regDiffColorG, regEndColorG0, regColorG0);
	FMUL			(regDeltaColorG, regDiffColorG, regInvSpan);
	FSUB			(regDiffColorB, regEndColorB0, regColorB0);
	FMUL			(regDeltaColorG, regDiffColorB, regInvSpan);
	FSUB			(regDiffColorA, regEndColorA0, regColorA0);
	FMUL			(regDeltaColorA, regDiffColorA, regInvSpan);

	FSUB			(regDiffFog, regEndFog0, regFog0);
	FMUL			(regDeltaFog, regDiffFog, regInvSpan);
	FSUB			(regDiffDepth, regEndDepth0, regDepth0);
	FMUL			(regDeltaDepth, regDiffDepth, regInvSpan);

	DECL_REG		(regDiffInvZ);
	DECL_REG		(regDeltaInvZ);
	DECL_REG		(regDiffTuOverZ);
	DECL_REG		(regDeltaTuOverZ);
	DECL_REG		(regDiffTvOverZ);
	DECL_REG		(regDeltaTvOverZ);

	FSUB			(regDiffInvZ, regEndInvZ0, regInvZ0);
	FMUL			(regDeltaInvZ, regDiffInvZ, regInvSpan);
	FSUB			(regDiffTuOverZ, regEndTuOverZ0, regTuOverZ0);
	FMUL			(regDeltaTuOverZ, regDiffTuOverZ, regInvSpan);
	FSUB			(regDiffTvOverZ, regEndTvOverZ0, regTvOverZ0);
	FMUL			(regDeltaTvOverZ, regDiffTvOverZ, regInvSpan);
	// -- end increments

	// 	I32 y = EGL_IntFromFixed(from.m_WindowCoords.y + ((EGL_ONE)/2-1));
	cg_virtual_reg_t * regFromY0 = LOAD_DATA(block, regCommonFrom, OFFSET_RASTER_POS_WINDOW_Y);

	DECL_CONST_REG	(regHalf2, 0x7fff);
	DECL_REG		(regFromY0PlusHalf);
	DECL_REG		(regY0);

	FADD			(regFromY0PlusHalf, regFromY0, regHalf2);
	TRUNC			(regY0, regFromY0PlusHalf);

	// 	EGL_Fixed error = 0;
	LOAD_CONST_REG	(regError0, 0);

	// 	for (; x < endX; ++x) {

	// 		EGL_Fixed z = EGL_Inverse(OneOverZ);
	// 		EGL_Fixed tu = EGL_Mul(tuOverZ, z);
	// 		EGL_Fixed tv = EGL_Mul(tvOverZ, z);

	// 		Fragment(x, y, depth, tu, tv, fogDensity, baseColor);

	// 		error += slope;
	// 		if (error > EGL_ONE) {
	// 			y += yIncrement;
	// 			error -= EGL_ONE;
	// 		}

	// 		baseColor += colorIncrement;
	// 		depth += deltaDepth;
	// 		OneOverZ += deltaZ;
	// 		tuOverZ += deltaU;
	// 		tvOverZ += deltaV;
	// 		fogDensity += deltaFog;
	// 	}

	BRA		(blockRefEndProc);

	// } else {
		// Bresenham along y-axis

	block = cg_block_create(procedure, 1);
	blockRefRasterY->block = block;

	// 	const RasterPos *start, *end;

	// 	I32 y;
	// 	I32 endY;
	// 	EGL_Fixed roundedY; //, preStepY;

	// 	if (deltaY < 0) {
	// 		deltaY = -deltaY;
	// 		deltaX = -deltaX;
	// 		start = &p_to;
	// 		end = &p_from;
	// 		roundedY = EGL_NearestInt(p_to.m_WindowCoords.y + 1);
	// 		y = EGL_IntFromFixed(roundedY);
	// 		endY = EGL_IntFromFixed(p_from.m_WindowCoords.y + ((EGL_ONE)/2));
	// 	} else {
	// 		start = &p_from;
	// 		end = &p_to;
	// 		roundedY = EGL_NearestInt(p_from.m_WindowCoords.y);
	// 		y = EGL_IntFromFixed(roundedY);
	// 		endY = EGL_IntFromFixed(p_to.m_WindowCoords.y + ((EGL_ONE)/2-1));
	// 	}

	// 	const RasterPos& from = *start;
	// 	const RasterPos& to = *end;

		//-- initialize with from vertex attributes
	// 	FractionalColor baseColor = from.m_Color;
	// 	EGL_Fixed OneOverZ = from.m_WindowCoords.invZ;
	// 	EGL_Fixed tuOverZ = EGL_Mul(from.m_TextureCoords.tu, OneOverZ);
	// 	EGL_Fixed tvOverZ = EGL_Mul(from.m_TextureCoords.tv, OneOverZ);
	// 	EGL_Fixed fogDensity = from.m_FogDensity;
	// 	EGL_Fixed depth = from.m_WindowCoords.depth;
		//-- end initialize

	// 	EGL_Fixed invSpan = EGL_Inverse(deltaY);
	// 	EGL_Fixed slope = EGL_Mul(EGL_Abs(deltaX), invSpan);
	// 	EGL_Fixed OneOverZTo = to.m_WindowCoords.invZ;

		// -- increments(to, from, invSpan)
	// 	FractionalColor colorIncrement = (to.m_Color - from.m_Color) * invSpan;
	// 	EGL_Fixed deltaFog = EGL_Mul(to.m_FogDensity - from.m_FogDensity, invSpan);

	// 	EGL_Fixed deltaZ = EGL_Mul(OneOverZTo - OneOverZ, invSpan);

	// 	EGL_Fixed deltaU = EGL_Mul(EGL_Mul(to.m_TextureCoords.tu, OneOverZTo) -
	// 							   EGL_Mul(from.m_TextureCoords.tu, OneOverZ), invSpan);

	// 	EGL_Fixed deltaV = EGL_Mul(EGL_Mul(to.m_TextureCoords.tv, OneOverZTo) -
	// 							   EGL_Mul(from.m_TextureCoords.tv, OneOverZ), invSpan);

	// 	EGL_Fixed deltaDepth = EGL_Mul(to.m_WindowCoords.depth - from.m_WindowCoords.depth, invSpan);
		// -- end increments

	// 	I32 x = EGL_IntFromFixed(from.m_WindowCoords.x + ((EGL_ONE)/2-1));

	// 	I32 xIncrement = (deltaX > 0) ? 1 : -1;
	// 	EGL_Fixed error = 0;

		// can have xIncrement; yIncrement; xBaseIncrement, yBaseIncrement
		// then both x/y loops become the same
		// question: how to add correct mipmap selection?

	// 	for (; y < endY; ++y) {

	// 		EGL_Fixed z = EGL_Inverse(OneOverZ);
	// 		EGL_Fixed tu = EGL_Mul(tuOverZ, z);
	// 		EGL_Fixed tv = EGL_Mul(tvOverZ, z);

	// 		Fragment(x, y, depth, tu, tv, fogDensity, baseColor);

	// 		error += slope;
	// 		if (error > EGL_ONE) {
	// 			x += xIncrement;
	// 			error -= EGL_ONE;
	// 		}

	// 		baseColor += colorIncrement;
	// 		depth += deltaDepth;
	// 		OneOverZ += deltaZ;
	// 		tuOverZ += deltaU;
	// 		tvOverZ += deltaV;
	// 		fogDensity += deltaFog;
	// 	}
	// }

	block = cg_block_create(procedure, 1);
	blockRefEndProc->block = block;

	RET();

}

