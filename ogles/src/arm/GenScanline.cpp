// ==========================================================================
//
// GenScanline.cpp		JIT Class for 3D Rendering Library
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




#define LOG_LINEAR_SPAN  3 					// logarithm of value base 2
#define LINEAR_SPAN (1 << LOG_LINEAR_SPAN)	// must be power of 2


// This method needs access to the following:
// Surface:
//	m_Surface->GetWidth()
//	m_Surface->GetHeight()
//	m_Surface->GetDepthBuffer()
//	m_Surface->GetStencilBuffer()
//  m_Surface->GetColorBuffer()
//	m_Surface->GetAlphaBuffer()
//
// Texture:
//	m_Texture->GetTexture(m_MipMapLevel)
//  texture->GetWidth()
//  texture->GetHeight()
//  texture->GetExponent()
//	texture->GetData()
void CodeGenerator :: GenerateRasterScanLine() {

	cg_proc_t * procedure = cg_proc_create(m_Module);

	// The signature of the generated function is:
	//	(const RasterInfo * info, const EdgePos& start, const EdgePos& end);
	// Do not pass in y coordinate but rather assume that raster info pointers have been
	// adjusted to point to current scanline in memory
	// In the edge buffers, z, tu and tv are actually divided by w

	DECL_REG	(regInfo);		// virtual register containing info structure pointer
	DECL_REG	(regStart);		// virtual register containing start edge buffer pointer
	DECL_REG	(regEnd);		// virtual register containing end edge buffer pointer

	procedure->num_args = 3;	// the previous three declarations make up the arguments

	cg_block_t * block = cg_block_create(procedure, 1);

	// Create instructions to calculate addresses of individual fields of
	// edge buffer input arguments

	DECL_REG	(regConstant1);
	LDI		(regConstant1, 1);

	// x coordinate
	DECL_REG	(regOffsetWindowX);
	DECL_REG	(regAddrStartWindowX);
	DECL_REG	(regAddrEndWindowX);

	LDI		(regOffsetWindowX, OFFSET_EDGE_BUFFER_WINDOW_X);
	ADD		(regAddrStartWindowX, regStart, regOffsetWindowX);
	ADD		(regAddrEndWindowX, regEnd, regOffsetWindowX);

	// z coordinate
	DECL_REG	(regOffsetWindowZ);
	DECL_REG	(regAddrStartWindowZ);
	DECL_REG	(regAddrEndWindowZ);

	LDI		(regOffsetWindowZ, OFFSET_EDGE_BUFFER_WINDOW_INV_Z);
	ADD		(regAddrStartWindowZ, regStart, regOffsetWindowZ);
	ADD		(regAddrEndWindowZ, regEnd, regOffsetWindowZ);

	// depth coordinate
	DECL_REG	(regOffsetWindowDepth);
	DECL_REG	(regAddrStartWindowDepth);
	DECL_REG	(regAddrEndWindowDepth);

	LDI		(regOffsetWindowDepth, OFFSET_EDGE_BUFFER_WINDOW_DEPTH);
	ADD		(regAddrStartWindowDepth, regStart, regOffsetWindowDepth);
	ADD		(regAddrEndWindowDepth, regEnd, regOffsetWindowDepth);

	// u texture coordinate
	DECL_REG	(regOffsetTextureU);
	DECL_REG	(regAddrStartTextureU);
	DECL_REG	(regAddrEndTextureU);

	LDI		(regOffsetTextureU, OFFSET_EDGE_BUFFER_TEX_TU);
	ADD		(regAddrStartTextureU, regStart, regOffsetTextureU);
	ADD		(regAddrEndTextureU, regEnd, regOffsetTextureU);

	// du/dx texture coordinate
	DECL_REG	(regOffsetTextureDuDx);
	DECL_REG	(regAddrStartTextureDuDx);
	DECL_REG	(regAddrEndTextureDuDx);

	LDI		(regOffsetTextureDuDx, OFFSET_EDGE_BUFFER_TEX_DTUDX);
	ADD		(regAddrStartTextureDuDx, regStart, regOffsetTextureDuDx);
	ADD		(regAddrEndTextureDuDx, regEnd, regOffsetTextureDuDx);

	// du/dy texture coordinate
	DECL_REG	(regOffsetTextureDuDy);
	DECL_REG	(regAddrStartTextureDuDy);
	DECL_REG	(regAddrEndTextureDuDy);

	LDI		(regOffsetTextureDuDy, OFFSET_EDGE_BUFFER_TEX_DTUDY);
	ADD		(regAddrStartTextureDuDy, regStart, regOffsetTextureDuDy);
	ADD		(regAddrEndTextureDuDy, regEnd, regOffsetTextureDuDy);

	// v texture coordinate
	DECL_REG	(regOffsetTextureV);
	DECL_REG	(regAddrStartTextureV);
	DECL_REG	(regAddrEndTextureV);

	LDI		(regOffsetTextureV, OFFSET_EDGE_BUFFER_TEX_TV);
	ADD		(regAddrStartTextureV, regStart, regOffsetTextureV);
	ADD		(regAddrEndTextureV, regEnd, regOffsetTextureV);

	// dv/dx texture coordinate
	DECL_REG	(regOffsetTextureDvDx);
	DECL_REG	(regAddrStartTextureDvDx);
	DECL_REG	(regAddrEndTextureDvDx);

	LDI		(regOffsetTextureDvDx, OFFSET_EDGE_BUFFER_TEX_DTVDX);
	ADD		(regAddrStartTextureDvDx, regStart, regOffsetTextureDvDx);
	ADD		(regAddrEndTextureDvDx, regEnd, regOffsetTextureDvDx);

	// dv/dy texture coordinate
	DECL_REG	(regOffsetTextureDvDy);
	DECL_REG	(regAddrStartTextureDvDy);
	DECL_REG	(regAddrEndTextureDvDy);

	LDI		(regOffsetTextureDvDy, OFFSET_EDGE_BUFFER_TEX_DTVDY);
	ADD		(regAddrStartTextureDvDy, regStart, regOffsetTextureDvDy);
	ADD		(regAddrEndTextureDvDy, regEnd, regOffsetTextureDvDy);

	// r color component
	DECL_REG	(regOffsetColorR);
	DECL_REG	(regAddrStartColorR);
	DECL_REG	(regAddrEndColorR);

	LDI		(regOffsetColorR, OFFSET_EDGE_BUFFER_COLOR_R);
	ADD		(regAddrStartColorR, regStart, regOffsetColorR);
	ADD		(regAddrEndColorR, regEnd, regOffsetColorR);

	// g color component
	DECL_REG	(regOffsetColorG);
	DECL_REG	(regAddrStartColorG);
	DECL_REG	(regAddrEndColorG);

	LDI		(regOffsetColorG, OFFSET_EDGE_BUFFER_COLOR_G);
	ADD		(regAddrStartColorG, regStart, regOffsetColorG);
	ADD		(regAddrEndColorG, regEnd, regOffsetColorG);

	// b color component
	DECL_REG	(regOffsetColorB);
	DECL_REG	(regAddrStartColorB);
	DECL_REG	(regAddrEndColorB);

	LDI		(regOffsetColorB, OFFSET_EDGE_BUFFER_COLOR_B);
	ADD		(regAddrStartColorB, regStart, regOffsetColorB);
	ADD		(regAddrEndColorB, regEnd, regOffsetColorB);

	// a color component
	DECL_REG	(regOffsetColorA);
	DECL_REG	(regAddrStartColorA);
	DECL_REG	(regAddrEndColorA);

	LDI		(regOffsetColorA, OFFSET_EDGE_BUFFER_COLOR_A);
	ADD		(regAddrStartColorA, regStart, regOffsetColorA);
	ADD		(regAddrEndColorA, regEnd, regOffsetColorA);

	// fog density
	DECL_REG	(regOffsetFog);
	DECL_REG	(regAddrStartFog);
	DECL_REG	(regAddrEndFog);

	LDI		(regOffsetFog, OFFSET_EDGE_BUFFER_FOG);
	ADD		(regAddrStartFog, regStart, regOffsetFog);
	ADD		(regAddrEndFog, regEnd, regOffsetFog);

	// Texture base address
	cg_virtual_reg_t * regAddrTextures = LOAD_DATA(block, regInfo, OFFSET_TEXTURES);
	cg_virtual_reg_t * regTexture = regAddrTextures;

	//EGL_Fixed invSpan = EGL_Inverse(end.m_WindowCoords.x - start.m_WindowCoords.x);
	DECL_REG	(regEndWindowX);
	DECL_REG	(regStartWindowX);
	DECL_REG	(regDiffX);
	DECL_FLAGS	(flagXCompare);

	cg_block_ref_t * endProc = cg_block_ref_create(procedure);

	LDW		(regEndWindowX, regAddrEndWindowX);
	LDW		(regStartWindowX, regAddrStartWindowX);
	SUB_S	(regDiffX, flagXCompare, regEndWindowX, regStartWindowX);
	BLE		(flagXCompare, endProc);

	DECL_REG	(regStartWindowZ);
	DECL_REG	(regStartTextureU);
	DECL_REG	(regStartTextureDuDy);
	DECL_REG	(regStartTextureV);
	DECL_REG	(regStartTextureDvDy);
	DECL_REG	(regStartFog);
	DECL_REG	(regStartDepth);

	LDW		(regStartWindowZ, regAddrStartWindowZ);


	//EGL_Fixed invTu = start.m_TextureCoords.tu;
	//EGL_Fixed dTuDxOverInvZ2 = start.m_TextureCoords.dtudx;
	//EGL_Fixed dTuDyOverInvZ2 = start.m_TextureCoords.dtudy;
	//EGL_Fixed invTv = start.m_TextureCoords.tv;
	//EGL_Fixed dTvDxOverInvZ2 = start.m_TextureCoords.dtvdx;
	//EGL_Fixed dTvDyOverInvZ2 = start.m_TextureCoords.dtvdy;
	//EGL_Fixed invZ = start.m_WindowCoords.z;
	//EGL_Fixed fogDensity = start.m_FogDensity;

	//EGL_Fixed z = EGL_Inverse(invZ);
	//EGL_Fixed tu = EGL_Mul(invTu, z);
	//EGL_Fixed tv = EGL_Mul(invTv, z);
	DECL_REG	(regZ);
	DECL_REG	(regU);
	DECL_REG	(regV);

	LDW		(regStartTextureU, regAddrStartTextureU);
	FINV	(regZ, regStartWindowZ);
	LDW		(regStartTextureV, regAddrStartTextureV);
	LDW		(regStartFog, regAddrStartFog);
	FMUL	(regU, regStartTextureU, regZ);
	LDW		(regStartDepth, regAddrStartWindowDepth);
	FMUL	(regV, regStartTextureV, regZ);

	// texture gradients for mipmap selection
	DECL_REG	(regDuDx);
	DECL_REG	(regDvDx);
	DECL_REG	(regAbsDuDx);
	DECL_REG	(regAbsDvDx);

	LDW		(regDuDx, regAddrStartTextureDuDx);
	ABS		(regAbsDuDx, regDuDx);	
	LDW		(regDvDx, regAddrStartTextureDvDx);
	ABS		(regAbsDvDx, regDvDx);	

	LDW		(regStartTextureDuDy, regAddrStartTextureDuDy);
	LDW		(regStartTextureDvDy, regAddrStartTextureDvDy);



	//FractionalColor baseColor = start.m_Color;
	DECL_REG	(regStartColorR);
	DECL_REG	(regStartColorG);
	DECL_REG	(regStartColorB);
	DECL_REG	(regStartColorA);

	LDW		(regStartColorR, regAddrStartColorR);
	LDW		(regStartColorG, regAddrStartColorG);
	LDW		(regStartColorB, regAddrStartColorB);
	LDW		(regStartColorA, regAddrStartColorA);
	
	//cg_virtual_reg_t * x = EGL_IntFromFixed(start.m_WindowCoords.x);
	//cg_virtual_reg_t * xEnd = EGL_IntFromFixed(end.m_WindowCoords.x);
	DECL_REG	(regX);
	DECL_REG	(regXEnd);

	TRUNC	(regX, regStartWindowX);
	TRUNC	(regXEnd, regEndWindowX);

	//cg_virtual_reg_t * xLinEnd = x + ((xEnd - x) & ~(LINEAR_SPAN - 1));
	DECL_REG	(regSpanMask);
	DECL_REG	(regMaskedSpan);
	DECL_REG	(regXLinEnd);
	DECL_FLAGS	(regCompare0);
	DECL_REG	(regIntDiffX);

	LDI		(regSpanMask, ~(LINEAR_SPAN - 1));
	TRUNC	(regIntDiffX, regDiffX);
	AND_S	(regMaskedSpan, regCompare0, regIntDiffX, regSpanMask);
	ADD		(regXLinEnd, regX, regMaskedSpan);

	//for (; x < xLinEnd;) {

	cg_block_ref_t * endLoop0 = cg_block_ref_create(procedure);

	BEQ		(regCompare0, endLoop0);

	cg_block_ref_t * beginLoop0 = cg_block_ref_create(procedure);
	block = cg_block_create(procedure, 2);
	beginLoop0->block = block;

	// Here we define all the loop registers and phi mappings
	DECL_REG	(regLoop0ZEntry);
	DECL_REG	(regLoop0Z);
	DECL_REG	(regLoop1Z);

	PHI		(regLoop0ZEntry, cg_create_virtual_reg_list(procedure->module->heap, regZ, regLoop0Z, regLoop1Z, NULL));

	DECL_REG	(regLoop0UEntry);
	DECL_REG	(regLoop0U);

	PHI		(regLoop0UEntry, cg_create_virtual_reg_list(procedure->module->heap, regU, regLoop0U, NULL));

	DECL_REG	(regLoop0VEntry);
	DECL_REG	(regLoop0V);

	PHI		(regLoop0VEntry, cg_create_virtual_reg_list(procedure->module->heap, regV, regLoop0V, NULL));

	DECL_REG	(regLoop0InvZEntry);
	DECL_REG	(regLoop0InvZ);

	PHI		(regLoop0InvZEntry, cg_create_virtual_reg_list(procedure->module->heap, regStartWindowZ, regLoop0InvZ, NULL));

	DECL_REG	(regLoop0InvUEntry);
	DECL_REG	(regLoop0InvU);

	PHI		(regLoop0InvUEntry, cg_create_virtual_reg_list(procedure->module->heap, regStartTextureU, regLoop0InvU, NULL));

	DECL_REG	(regLoop0InvVEntry);
	DECL_REG	(regLoop0InvV);

	PHI		(regLoop0InvVEntry, cg_create_virtual_reg_list(procedure->module->heap, regStartTextureV, regLoop0InvV, NULL));

	DECL_REG	(regLoop0DuDyEntry);
	DECL_REG	(regLoop0DuDy);
	PHI		(regLoop0DuDyEntry, cg_create_virtual_reg_list(procedure->module->heap, regStartTextureDuDy, regLoop0DuDy, NULL));

	DECL_REG	(regLoop0DvDyEntry);
	DECL_REG	(regLoop0DvDy);
	PHI		(regLoop0DvDyEntry, cg_create_virtual_reg_list(procedure->module->heap, regStartTextureDvDy, regLoop0DvDy, NULL));


		//invZ += deltaInvZ << LOG_LINEAR_SPAN;
		//invTu += deltaInvU << LOG_LINEAR_SPAN;
		//invTv += deltaInvV << LOG_LINEAR_SPAN;
	DECL_REG	(regLinearSpan);
	DECL_REG	(regLogLinearSpan);
	DECL_REG	(regDeltaInvZTimesLinearSpan);
	DECL_REG	(regDeltaInvUTimesLinearSpan);
	DECL_REG	(regDeltaInvVTimesLinearSpan);

	LDI		(regLinearSpan, LINEAR_SPAN);
	LDI		(regLogLinearSpan, LOG_LINEAR_SPAN);

	{
		DECL_REG	(regDeltaInvZ);
		DECL_REG	(regDeltaInvU);
		DECL_REG	(regDeltaInvV);

		LDW		(regDeltaInvZ, regAddrEndWindowZ);
		LDW		(regDeltaInvU, regAddrEndTextureU);
		LDW		(regDeltaInvV, regAddrEndTextureV);

		LSL		(regDeltaInvZTimesLinearSpan, regDeltaInvZ, regLogLinearSpan);	
		FADD	(regLoop0InvZ, regLoop0InvZEntry, regDeltaInvZTimesLinearSpan);
		LSL		(regDeltaInvUTimesLinearSpan, regDeltaInvU, regLogLinearSpan);	
		FADD	(regLoop0InvU, regLoop0InvUEntry, regDeltaInvUTimesLinearSpan);
		LSL		(regDeltaInvVTimesLinearSpan, regDeltaInvV, regLogLinearSpan);	
		FADD	(regLoop0InvV, regLoop0InvVEntry, regDeltaInvVTimesLinearSpan);
	}
	// ----------------------------------------------------------------------
	// if multi-texture, determine texture mode (magnification vs. minifacation)
	// and texture level here
	// ----------------------------------------------------------------------

	if (m_State->m_Texture[TODO].MipmapFilterMode != RasterizerState::FilterModeNone) {

		if (m_State->m_Texture[TODO].MipmapFilterMode == RasterizerState::FilterModeNearest ||
			/* remove this */ m_State->m_Texture[TODO].MipmapFilterMode == RasterizerState::FilterModeLinear) {
			//	EGL_Fixed maxDu = EGL_Mul(EGL_Max(EGL_Abs(dTuDxOverInvZ2), EGL_Abs(dTuDyOverInvZ2)), m_Texture->GetTexture(0)->GetWidth());
			//	EGL_Fixed maxDv = EGL_Mul(EGL_Max(EGL_Abs(dTvDxOverInvZ2), EGL_Abs(dTvDyOverInvZ2)), m_Texture->GetTexture(0)->GetHeight());

			DECL_REG	(regAbsDuDy);
			DECL_REG	(regAbsDvDy);

			ABS			(regAbsDuDy, regLoop0DuDyEntry);
			ABS			(regAbsDvDy, regLoop0DvDyEntry);

			DECL_REG	(regMaxDu0);
			DECL_REG	(regMaxDv0);
			DECL_REG	(regMaxDu);
			DECL_REG	(regMaxDv);

			cg_virtual_reg_t * regLogWidth = LOAD_DATA(block, regAddrTextures, OFFSET_TEXTURE_LOG_WIDTH);
			DECL_CONST_REG	(regConstant16, 16);

			DECL_REG	(regShiftDu);
			DECL_REG	(regShiftDv);

			MAX			(regMaxDu0, regAbsDuDx, regAbsDuDy);
			SUB			(regShiftDu, regConstant16, regLogWidth);
			ASR			(regMaxDu, regMaxDu0, regShiftDu);

			cg_virtual_reg_t * regLogHeight = LOAD_DATA(block, regAddrTextures, OFFSET_TEXTURE_LOG_HEIGHT);
			MAX			(regMaxDv0, regAbsDvDx, regAbsDvDy);
			SUB			(regShiftDv, regConstant16, regLogHeight);
			ASR			(regMaxDv, regMaxDv0, regShiftDv);
			
			//	EGL_Fixed maxD = maxDu + maxDv;
			//	EGL_Fixed rho = EGL_Mul(z2, maxD);

			DECL_REG	(regMaxD);
			DECL_REG	(regRho);

			FADD		(regMaxD, regMaxDu, regMaxDv);

			//	EGL_Fixed z2 = EGL_Mul(z << 4, z << 4);
			DECL_REG	(regShiftedZ);
			DECL_REG	(regConstant4);
			DECL_REG	(regZ2);

			LDI			(regConstant4, 4);
			LSL			(regShiftedZ, regLoop0ZEntry, regConstant4);
			FMUL		(regZ2, regShiftedZ, regShiftedZ);

			FMUL		(regRho, regMaxD, regZ2);

			// we start with nearest/minification only selection; will add LINEAR later
			//	m_MipMapLevel = EGL_Min(EGL_Max(0, Log2(rho)), m_MaxMipmapLevel);

			DECL_REG	(regLog2);
			DECL_REG	(regMipmapLevel);

			LOG2		(regLog2, regRho);
			
			cg_virtual_reg_t * regMaxMipmapLevel = LOAD_DATA(block, regInfo, OFFSET_MAX_MIPMAP_LEVEL);
			
			MIN			(regMipmapLevel, regLog2, regMaxMipmapLevel);

			// now multiply the texture block size by the mipmap level and add this to the texture base

			assert		((1 << Log(sizeof(Texture))) == sizeof(Texture));

			DECL_CONST_REG	(regLogTextureSize, Log(sizeof(Texture)));
			DECL_REG	(regScaledLevel);

			LSL			(regScaledLevel, regMipmapLevel, regLogTextureSize);
			ALLOC_REG	(regTexture);
			ADD			(regTexture, regAddrTextures, regScaledLevel);

		// TBD: } else if (m_State->m_MipmapFilterMode == RasterizerState::FilterModeLinear) {
		}


		//EGL_Fixed deltaInvDu = delta.m_TextureCoords.dtudy;
		//EGL_Fixed deltaInvDv = delta.m_TextureCoords.dtvdy;

		DECL_REG(regDuDyDx);
		DECL_REG(regDvDyDx);
		DECL_REG(regShiftedDuDyDx);
		DECL_REG(regShiftedDvDyDx);
		LDW		(regDuDyDx, regAddrEndTextureDuDy);
		LDW		(regDvDyDx, regAddrEndTextureDvDy);

		//dTuDyOverInvZ2 += deltaInvDu << LOG_LINEAR_SPAN;
		//dTvDyOverInvZ2 += deltaInvDv << LOG_LINEAR_SPAN;

		LSL		(regShiftedDuDyDx, regDuDyDx, regLogLinearSpan);
		LSL		(regShiftedDvDyDx, regDvDyDx, regLogLinearSpan);

		FADD	(regLoop0DuDy, regLoop0DuDyEntry, regShiftedDuDyDx);
		FADD	(regLoop0DvDy, regLoop0DvDyEntry, regShiftedDvDyDx);
	}

		//EGL_Fixed endZ = EGL_Inverse(invZ);
		//EGL_Fixed endTu = EGL_Mul(invTu, endZ);
		//EGL_Fixed endTv = EGL_Mul(invTv, endZ);

	DECL_REG	(regLoop0EndZ);
	DECL_REG	(regLoop0EndU);
	DECL_REG	(regLoop0EndV);

	FINV	(regLoop0EndZ, regLoop0InvZ);
	FMUL	(regLoop0EndU, regLoop0InvU, regLoop0EndZ);
	FMUL	(regLoop0EndV, regLoop0InvV, regLoop0EndZ);

		//EGL_Fixed deltaZ = (endZ - z) >> LOG_LINEAR_SPAN;
		//EGL_Fixed deltaTu = (endTu - tu) >> LOG_LINEAR_SPAN; 
		//EGL_Fixed deltaTv = (endTv - tv) >> LOG_LINEAR_SPAN;

	DECL_REG	(regLoop0DiffZ);
	DECL_REG	(regLoop0ScaledDiffZ);
	DECL_REG	(regLoop0DiffU);
	DECL_REG	(regLoop0ScaledDiffU);
	DECL_REG	(regLoop0DiffV);
	DECL_REG	(regLoop0ScaledDiffV);
	//DECL_REG	(regLoop0ScaledDiffUOver2);
	//DECL_REG	(regLoop0ScaledDiffVOver2);
	//DECL_REG	(regAdjustedU0);
	//DECL_REG	(regAdjustedV0);


	FSUB	(regLoop0DiffZ, regLoop0EndZ, regLoop0ZEntry); 
	ASR		(regLoop0ScaledDiffZ, regLoop0DiffZ, regLogLinearSpan);

	FSUB	(regLoop0DiffU, regLoop0EndU, regLoop0UEntry); 
	ASR		(regLoop0ScaledDiffU, regLoop0DiffU, regLogLinearSpan);

	FSUB	(regLoop0DiffV, regLoop0EndV, regLoop0VEntry); 
	ASR		(regLoop0ScaledDiffV, regLoop0DiffV, regLogLinearSpan);


	// also not to include phi projection for z coming from inner loop

		//int count = LINEAR_SPAN; 

	cg_block_ref_t * beginLoop1 = cg_block_ref_create(procedure);
	cg_block_ref_t * endLoop1 = cg_block_ref_create(procedure);
	cg_block_ref_t * postFragmentLoop1 = cg_block_ref_create(procedure);

	block = cg_block_create(procedure, 8);
	beginLoop1->block = block;

		//do {

	// phi for count, x, z, tu, tv, fog, depth, r, g, b, a

	DECL_REG	(regLoop1CountEntry);
	DECL_REG	(regLoop1Count);
	DECL_REG	(regLoop1XEntry);
	DECL_REG	(regLoop1X);
	DECL_REG	(regLoop1ZEntry);
	DECL_REG	(regLoop1U);
	DECL_REG	(regLoop1UEntry);
	DECL_REG	(regLoop1V);
	DECL_REG	(regLoop1VEntry);
	DECL_REG	(regLoop1FogEntry);
	DECL_REG	(regLoop1Fog);
	DECL_REG	(regLoop1DepthEntry);
	DECL_REG	(regLoop1Depth);
	DECL_REG	(regLoop1REntry);
	DECL_REG	(regLoop1R);
	DECL_REG	(regLoop1GEntry);
	DECL_REG	(regLoop1G);
	DECL_REG	(regLoop1BEntry);
	DECL_REG	(regLoop1B);
	DECL_REG	(regLoop1AEntry);
	DECL_REG	(regLoop1A);

	PHI		(regLoop1CountEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1Count, regLinearSpan, NULL));
	PHI		(regLoop1XEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1X, regX, NULL));
	PHI		(regLoop1ZEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1Z, regZ, NULL));
	PHI		(regLoop1UEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1U, regLoop0UEntry, NULL));
	PHI		(regLoop1VEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1V, regLoop0VEntry, NULL));
	PHI		(regLoop1FogEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1Fog, regStartFog, NULL));
	PHI		(regLoop1DepthEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1Depth, regStartDepth, NULL));
	PHI		(regLoop1REntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1R, regStartColorR, NULL));
	PHI		(regLoop1GEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1G, regStartColorG, NULL));
	PHI		(regLoop1BEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1B, regStartColorB, NULL));
	PHI		(regLoop1AEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1A, regStartColorA, NULL));

	FragmentGenerationInfo info;
	info.regX = regLoop1XEntry;
	info.regY = 0;
	info.regU = regLoop1UEntry;
	info.regV = regLoop1VEntry; 
	info.regFog = regLoop1FogEntry;
	info.regDepth = regLoop1DepthEntry;
	info.regR = regLoop1REntry;
	info.regG = regLoop1GEntry;
	info.regB = regLoop1BEntry; 
	info.regA = regLoop1AEntry;
	info.regInfo = regInfo;
	info.regTexture = regTexture;

	GenerateFragment(procedure, block, postFragmentLoop1, info, 8); 

	block = cg_block_create(procedure, 8);
	postFragmentLoop1->block = block;

	{
		//baseColor += colorIncrement;
		//depth += deltaDepth;
		//fogDensity += deltaFog;

		DECL_REG	(regColorIncrementR);
		DECL_REG	(regColorIncrementG);
		DECL_REG	(regColorIncrementB);
		DECL_REG	(regColorIncrementA);
		DECL_REG	(regDeltaFog);
		DECL_REG	(regDeltaDepth);

		LDW		(regColorIncrementR, regAddrEndColorR);
		LDW		(regColorIncrementG, regAddrEndColorG);
		LDW		(regColorIncrementB, regAddrEndColorB);
		FADD	(regLoop1R, regLoop1REntry, regColorIncrementR);
		LDW		(regColorIncrementA, regAddrEndColorA);
		FADD	(regLoop1G, regLoop1GEntry, regColorIncrementG);
		LDW		(regDeltaFog, regAddrEndFog);
		FADD	(regLoop1B, regLoop1BEntry, regColorIncrementB);
		LDW		(regDeltaDepth, regAddrEndWindowDepth);
		FADD	(regLoop1A, regLoop1AEntry, regColorIncrementA);
		FADD	(regLoop1Fog, regLoop1FogEntry, regDeltaFog);
		FADD	(regLoop1Depth, regLoop1DepthEntry, regDeltaDepth);
	}
			//z += deltaZ;
			//tu += deltaTu;
			//tv += deltaTv;
	FADD	(regLoop1Z, regLoop1ZEntry, regLoop0ScaledDiffZ);
	FADD	(regLoop1U, regLoop1UEntry, regLoop0ScaledDiffU);
	FADD	(regLoop1V, regLoop1VEntry, regLoop0ScaledDiffV);

			//++x;
	ADD		(regLoop1X, regLoop1XEntry, regConstant1);

		//} while (--count);
	DECL_FLAGS	(regLoop1Condition);

	SUB_S	(regLoop1Count, regLoop1Condition, regLoop1CountEntry, regConstant1);
	BNE		(regLoop1Condition, beginLoop1);
	//}

	block = cg_block_create(procedure, 2);

	//DECL_REG	(regLoop1UExit);
	//DECL_REG	(regLoop1VExit);

	PHI		(regLoop0U, cg_create_virtual_reg_list(procedure->module->heap, regLoop1U, regLoop0UEntry, NULL));
	PHI		(regLoop0V, cg_create_virtual_reg_list(procedure->module->heap, regLoop1V, regLoop0VEntry, NULL));
	
	//FSUB		(regLoop0U, regLoop1UExit, regLoop0ScaledDiffUOver2);
	//FSUB		(regLoop0V, regLoop1VExit, regLoop0ScaledDiffVOver2);

	DECL_FLAGS	(regLoop0Condition);

	CMP		(regLoop0Condition, regLoop1X, regXLinEnd);
	BNE		(regLoop0Condition, beginLoop0);

	//if (x != xEnd) {
	block = cg_block_create(procedure, 1);
	endLoop0->block = block;

	cg_block_ref_t * beginLoop2 = cg_block_ref_create(procedure);
	cg_block_ref_t * endLoop2 = cg_block_ref_create(procedure);
	cg_block_ref_t * postFragmentLoop2 = cg_block_ref_create(procedure);

	DECL_REG	(regBlock4X);
	DECL_REG	(regBlock4Z);
	DECL_REG	(regBlock4U);
	DECL_REG	(regBlock4V);
	DECL_REG	(regBlock4InvZ);
	DECL_REG	(regBlock4InvU);
	DECL_REG	(regBlock4InvV);
	DECL_REG	(regBlock4DiffX);
	DECL_FLAGS	(regBlock4Condition);
	DECL_REG	(regLoop4DuDy);
	DECL_REG	(regLoop4DvDy);
		

	PHI		(regLoop4DuDy, cg_create_virtual_reg_list(procedure->module->heap, regStartTextureDuDy, regLoop0DuDy, NULL));
	PHI		(regLoop4DvDy, cg_create_virtual_reg_list(procedure->module->heap, regStartTextureDvDy, regLoop0DvDy, NULL));
	PHI		(regBlock4InvZ, cg_create_virtual_reg_list(procedure->module->heap, regLoop0InvZ, regStartWindowZ, NULL));
	PHI		(regBlock4InvU, cg_create_virtual_reg_list(procedure->module->heap, regLoop0InvU, regStartTextureU, NULL));
	PHI		(regBlock4InvV, cg_create_virtual_reg_list(procedure->module->heap, regLoop0InvV, regStartTextureV, NULL));
	PHI		(regBlock4X, cg_create_virtual_reg_list(procedure->module->heap, regX, regLoop1X, NULL));
	PHI		(regBlock4Z, cg_create_virtual_reg_list(procedure->module->heap, regLoop1Z, regZ, NULL));
	PHI		(regBlock4U, cg_create_virtual_reg_list(procedure->module->heap, regLoop0U, regU, NULL));
	PHI		(regBlock4V, cg_create_virtual_reg_list(procedure->module->heap, regLoop0V, regV, NULL));
	SUB_S	(regBlock4DiffX, regBlock4Condition, regXEnd, regBlock4X);
	BEQ		(regBlock4Condition, endLoop2);

	// ----------------------------------------------------------------------
	// if multi-texture, determine texture mode (magnification vs. minifacation)
	// and texture level here
	// ----------------------------------------------------------------------

	if (m_State->m_Texture[TODO].MipmapFilterMode != RasterizerState::FilterModeNone) {

		if (m_State->m_Texture[TODO].MipmapFilterMode == RasterizerState::FilterModeNearest ||
			/* remove this */ m_State->m_Texture[TODO].MipmapFilterMode == RasterizerState::FilterModeLinear) {
			//	EGL_Fixed maxDu = EGL_Mul(EGL_Max(EGL_Abs(dTuDxOverInvZ2), EGL_Abs(dTuDyOverInvZ2)), m_Texture->GetTexture(0)->GetWidth());
			//	EGL_Fixed maxDv = EGL_Mul(EGL_Max(EGL_Abs(dTvDxOverInvZ2), EGL_Abs(dTvDyOverInvZ2)), m_Texture->GetTexture(0)->GetHeight());

			DECL_REG	(regAbsDuDy);
			DECL_REG	(regAbsDvDy);

			ABS			(regAbsDuDy, regLoop4DuDy);
			ABS			(regAbsDvDy, regLoop4DvDy);

			DECL_REG	(regMaxDu0);
			DECL_REG	(regMaxDv0);
			DECL_REG	(regMaxDu);
			DECL_REG	(regMaxDv);

			cg_virtual_reg_t * regLogWidth = LOAD_DATA(block, regAddrTextures, OFFSET_TEXTURE_LOG_WIDTH);
			DECL_CONST_REG	(regConstant16, 16);

			DECL_REG	(regShiftDu);
			DECL_REG	(regShiftDv);

			MAX			(regMaxDu0, regAbsDuDx, regAbsDuDy);
			SUB			(regShiftDu, regConstant16, regLogWidth);
			ASR			(regMaxDu, regMaxDu0, regShiftDu);

			cg_virtual_reg_t * regLogHeight = LOAD_DATA(block, regAddrTextures, OFFSET_TEXTURE_LOG_HEIGHT);
			MAX			(regMaxDv0, regAbsDvDx, regAbsDvDy);
			SUB			(regShiftDv, regConstant16, regLogHeight);
			ASR			(regMaxDv, regMaxDv0, regShiftDv);

			//	EGL_Fixed maxD = maxDu + maxDv;
			//	EGL_Fixed rho = EGL_Mul(z2, maxD);

			DECL_REG	(regMaxD);
			DECL_REG	(regRho);

			FADD		(regMaxD, regMaxDu, regMaxDv);

			//	EGL_Fixed z2 = EGL_Mul(z << 4, z << 4);
			DECL_REG	(regShiftedZ);
			DECL_REG	(regConstant4);
			DECL_REG	(regZ2);

			LDI			(regConstant4, 4);
			LSL			(regShiftedZ, regBlock4Z, regConstant4);
			FMUL		(regZ2, regShiftedZ, regShiftedZ);

			FMUL		(regRho, regMaxD, regZ2);

			// we start with nearest/minification only selection; will add LINEAR later
			//	m_MipMapLevel = EGL_Min(EGL_Max(0, Log2(rho)), m_MaxMipmapLevel);

			DECL_REG	(regLog2);
			DECL_REG	(regMipmapLevel);

			LOG2		(regLog2, regRho);
			
			cg_virtual_reg_t * regMaxMipmapLevel = LOAD_DATA(block, regInfo, OFFSET_MAX_MIPMAP_LEVEL);
			
			MIN			(regMipmapLevel, regLog2, regMaxMipmapLevel);

			// now multiply the texture block size by the mipmap level and add this to the texture base

			assert		((1 << Log(sizeof(Texture))) == sizeof(Texture));

			DECL_CONST_REG	(regLogTextureSize, Log(sizeof(Texture)));
			DECL_REG	(regScaledLevel);

			LSL			(regScaledLevel, regMipmapLevel, regLogTextureSize);
			ALLOC_REG	(regTexture);
			ADD			(regTexture, regAddrTextures, regScaledLevel);

		// TBD: } else if (m_State->m_MipmapFilterMode == RasterizerState::FilterModeLinear) {
		}
	}
		//I32 deltaX = xEnd - x;

		//EGL_Fixed endZ = EGL_Inverse(invZ + deltaX * deltaInvZ);
		//EGL_Fixed endTu = EGL_Mul(invTu + deltaX * deltaInvU, endZ);
		//EGL_Fixed endTv = EGL_Mul(invTv + deltaX * deltaInvV, endZ);
	DECL_REG(regDeltaXDeltaInvZ);
	DECL_REG(regDeltaXDeltaInvU);
	DECL_REG(regDeltaXDeltaInvV);

	DECL_REG(regEndWindowZ);
	DECL_REG(regEndTextureU);
	DECL_REG(regEndTextureV);

	DECL_REG	(regEndZ);
	DECL_REG	(regEndU);
	DECL_REG	(regEndV);

	{
		DECL_REG	(regDeltaInvZ);
		DECL_REG	(regDeltaInvU);
		DECL_REG	(regDeltaInvV);

		LDW		(regDeltaInvZ, regAddrEndWindowZ);

		MUL		(regDeltaXDeltaInvZ, regBlock4DiffX, regDeltaInvZ);
		LDW		(regDeltaInvU, regAddrEndTextureU);
		FADD	(regEndWindowZ, regBlock4InvZ, regDeltaXDeltaInvZ);
		FINV	(regEndZ, regEndWindowZ);

		MUL		(regDeltaXDeltaInvU, regBlock4DiffX, regDeltaInvU);
		LDW		(regDeltaInvV, regAddrEndTextureV);
		FADD	(regEndTextureU, regBlock4InvU, regDeltaXDeltaInvU);
		MUL		(regDeltaXDeltaInvV, regBlock4DiffX, regDeltaInvV);

		FADD	(regEndTextureV, regBlock4InvV, regDeltaXDeltaInvV);
		FMUL	(regEndU, regEndZ, regEndTextureU);
		FMUL	(regEndV, regEndZ, regEndTextureV);
	}

		//invSpan = EGL_Inverse(EGL_FixedFromInt(deltaX));
	DECL_REG	(regShiftedBlock4DiffX);
	DECL_REG	(regConstant2);
	DECL_REG	(regInverseAddr);

	cg_virtual_reg_t * regInverseTablePtr =		LOAD_DATA(block, regInfo, OFFSET_INVERSE_TABLE_PTR);

	LDI			(regConstant2, 2);
	LSL			(regShiftedBlock4DiffX, regBlock4DiffX, regConstant2);
	ADD			(regInverseAddr, regInverseTablePtr, regShiftedBlock4DiffX);

	DECL_REG	(regBlock4InvSpan);

	LDW			(regBlock4InvSpan, regInverseAddr);

		//EGL_Fixed deltaZ = EGL_Mul(endZ - z, invSpan);
		//EGL_Fixed deltaTu = EGL_Mul(endTu - tu, invSpan);
		//EGL_Fixed deltaTv = EGL_Mul(endTv - tv, invSpan);
	DECL_REG	(regBlock4DiffZ);
	DECL_REG	(regBlock4DiffU);
	DECL_REG	(regBlock4DiffV);
	DECL_REG	(regLoop2ScaledDiffZ);
	DECL_REG	(regLoop2ScaledDiffU);
	DECL_REG	(regLoop2ScaledDiffV);
	//DECL_REG	(regLoop2ScaledDiffUOver2);
	//DECL_REG	(regLoop2ScaledDiffVOver2);
	//DECL_REG	(regAdjustedU);
	//DECL_REG	(regAdjustedV);

	FSUB	(regBlock4DiffZ, regEndZ, regBlock4Z);
	FMUL	(regLoop2ScaledDiffZ, regBlock4DiffZ, regBlock4InvSpan);

	FSUB	(regBlock4DiffU, regEndU, regBlock4U);
	FSUB	(regBlock4DiffV, regEndV, regBlock4V);

	FMUL	(regLoop2ScaledDiffU, regBlock4DiffU, regBlock4InvSpan);
	//ASR		(regLoop2ScaledDiffUOver2, regLoop2ScaledDiffU, regConstant1);
	//FADD		(regAdjustedU, regBlock4U, regLoop2ScaledDiffUOver2);

	FMUL	(regLoop2ScaledDiffV, regBlock4DiffV, regBlock4InvSpan);
	//ASR		(regLoop2ScaledDiffVOver2, regLoop2ScaledDiffV, regConstant1);
	//FADD		(regAdjustedV, regBlock4V, regLoop2ScaledDiffVOver2);

	//for (; x < xEnd; ++x) {
	block = cg_block_create(procedure, 4);
	beginLoop2->block = block;

	// phi for x, z, tu, tv, fog, depth, r, g, b, a

	DECL_REG	(regLoop2XEntry);
	DECL_REG	(regLoop2X);
	DECL_REG	(regLoop2UEntry);
	DECL_REG	(regLoop2U);
	DECL_REG	(regLoop2VEntry);
	DECL_REG	(regLoop2V);
	DECL_REG	(regLoop2FogEntry);
	DECL_REG	(regLoop2Fog);
	DECL_REG	(regLoop2DepthEntry);
	DECL_REG	(regLoop2Depth);
	DECL_REG	(regLoop2REntry);
	DECL_REG	(regLoop2R);
	DECL_REG	(regLoop2GEntry);
	DECL_REG	(regLoop2G);
	DECL_REG	(regLoop2BEntry);
	DECL_REG	(regLoop2B);
	DECL_REG	(regLoop2AEntry);
	DECL_REG	(regLoop2A);


	PHI		(regLoop2XEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2X, regBlock4X, NULL));
	PHI		(regLoop2UEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2U, regBlock4U, NULL));
	PHI		(regLoop2VEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2V, regBlock4V, NULL));
	PHI		(regLoop2FogEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2Fog, regLoop1Fog, regStartFog, NULL));
	PHI		(regLoop2DepthEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2Depth, regLoop1Depth, regStartDepth, NULL));
	PHI		(regLoop2REntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2R, regLoop1R, regStartColorR, NULL));
	PHI		(regLoop2GEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2G, regLoop1G, regStartColorG, NULL));
	PHI		(regLoop2BEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2B, regLoop1B, regStartColorB, NULL));
	PHI		(regLoop2AEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2A, regLoop1A, regStartColorA, NULL));

	FragmentGenerationInfo info2;
	info2.regX = regLoop2XEntry;
	info2.regY = 0;
	info2.regU = regLoop2UEntry;
	info2.regV = regLoop2VEntry; 
	info2.regFog = regLoop2FogEntry;
	info2.regDepth = regLoop2DepthEntry;
	info2.regR = regLoop2REntry;
	info2.regG = regLoop2GEntry;
	info2.regB = regLoop2BEntry; 
	info2.regA = regLoop2AEntry;	
	info2.regInfo = regInfo;
	info2.regTexture = regTexture;

	GenerateFragment(procedure, block, postFragmentLoop2, info2, 4); 

	block = cg_block_create(procedure, 4);
	postFragmentLoop2->block = block;

	{
		DECL_REG	(regColorIncrementR);
		DECL_REG	(regColorIncrementG);
		DECL_REG	(regColorIncrementB);
		DECL_REG	(regColorIncrementA);
		DECL_REG	(regDeltaFog);
		DECL_REG	(regDeltaDepth);

				//baseColor += colorIncrement;
				//depth += deltaDepth;
				//fogDensity += deltaFog;
				//z += deltaZ;
				//tu += deltaTu;
				//tv += deltaTv;
		LDW		(regDeltaDepth, regAddrEndWindowDepth);
		FADD	(regLoop2V, regLoop2VEntry, regLoop2ScaledDiffV);
		LDW		(regColorIncrementR, regAddrEndColorR);
		FADD	(regLoop2Depth, regLoop2DepthEntry, regDeltaDepth);
		FADD	(regLoop2U, regLoop2UEntry, regLoop2ScaledDiffU);
		LDW		(regColorIncrementG, regAddrEndColorG);
		FADD	(regLoop2R, regLoop2REntry, regColorIncrementR);
		LDW		(regColorIncrementB, regAddrEndColorB);
		FADD	(regLoop2G, regLoop2GEntry, regColorIncrementG);
		LDW		(regColorIncrementA, regAddrEndColorA);
		FADD	(regLoop2B, regLoop2BEntry, regColorIncrementB);
		LDW		(regDeltaFog, regAddrEndFog);
		FADD	(regLoop2A, regLoop2AEntry, regColorIncrementA);
		FADD	(regLoop2Fog, regLoop2FogEntry, regDeltaFog);
	}
			//++x;
	ADD		(regLoop2X, regLoop2XEntry, regConstant1);

	DECL_FLAGS	(regCondLoopEnd);

	CMP		(regCondLoopEnd, regLoop2X, regXEnd);
	BLT		(regCondLoopEnd, beginLoop2);

		//}
	//}
	block = cg_block_create(procedure, 1);
	endLoop2->block = block;
	endProc->block = block;

	RET();

}

