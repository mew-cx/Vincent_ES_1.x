// ==========================================================================
//
// GenPoint.cpp			JIT Class for 3D Rendering Library
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


void CodeGenerator :: GenerateRasterPoint() {

	cg_proc_t * procedure = cg_proc_create(m_Module);

	// The signature of the generated function is:
	//	(const RasterInfo * info, const RasterPos& pos);

	DECL_REG	(regInfo);		// virtual register containing info structure pointer
	DECL_REG	(regPos);		// virtual register containing vertex coordinate pointer
	DECL_REG	(regSize);		// virtual register containing point size

	procedure->num_args = 3;	// the previous three declarations make up the arguments

	cg_block_t * block = cg_block_create(procedure, 1);

	// load argument values
	cg_virtual_reg_t * regTexture = LOAD_DATA(block, regInfo, OFFSET_TEXTURES);

	FragmentGenerationInfo info;

	info.regInfo = regInfo;
	info.regTexture = regTexture;

	//I32 x = EGL_IntFromFixed(point.m_WindowCoords.x);
	//I32 y = EGL_IntFromFixed(point.m_WindowCoords.y);
	DECL_REG	(regX);
	DECL_REG	(regY);

	TRUNC		(regX, LOAD_DATA(block, regPos, OFFSET_RASTER_POS_WINDOW_X));
	TRUNC		(regY, LOAD_DATA(block, regPos, OFFSET_RASTER_POS_WINDOW_Y));

	info.regX = regX;
	info.regY = regY;

	//EGL_Fixed depth = point.m_WindowCoords.depth;
	//EGL_Fixed tu = point.m_TextureCoords.tu;
	//EGL_Fixed tv = point.m_TextureCoords.tv;
	//FractionalColor baseColor = point.m_Color;
	//EGL_Fixed fogDensity = point.m_FogDensity;
	info.regU = LOAD_DATA(block, regPos, OFFSET_RASTER_POS_TEX_TU);
	info.regV = LOAD_DATA(block, regPos, OFFSET_RASTER_POS_TEX_TV); 
	info.regFog = LOAD_DATA(block, regPos, OFFSET_RASTER_POS_FOG);
	info.regDepth = LOAD_DATA(block, regPos, OFFSET_RASTER_POS_WINDOW_DEPTH);
	info.regR = LOAD_DATA(block, regPos, OFFSET_RASTER_POS_COLOR_R);
	info.regG = LOAD_DATA(block, regPos, OFFSET_RASTER_POS_COLOR_G);
	info.regB = LOAD_DATA(block, regPos, OFFSET_RASTER_POS_COLOR_B);
	info.regA = LOAD_DATA(block, regPos, OFFSET_RASTER_POS_COLOR_A);

	cg_block_ref_t * postFragment = cg_block_ref_create(procedure);

	GenerateFragment(procedure, block, postFragment, info, 1);

	block = cg_block_create(procedure, 1);
	postFragment->block = block;

	RET();

}

