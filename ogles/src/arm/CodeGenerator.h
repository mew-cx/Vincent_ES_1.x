#ifndef EGL_CODE_GENERATOR_H
#define EGL_CODE_GENERATOR_H 1

#pragma once

// ==========================================================================
//
// CodeGenerator.h		JIT Class for 3D Rendering Library
//
//						This file contains the rasterizer functions that
//						implement the runtime code generation support
//						for optimized scan line rasterization routines.
//
// --------------------------------------------------------------------------
//
// 08-07-2003	Hans-Martin Will	initial version
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


#include "OGLES.h"
#include "fixed.h"
#include "linalg.h"
#include "Rasterizer.h"
#include "RasterizerState.h"
#include <map>


extern "C" {
	typedef struct cg_module_t cg_module_t;
	typedef struct cg_proc_t cg_proc_t;
	typedef struct cg_block_t cg_block_t;
	typedef struct cg_block_ref_t cg_block_ref_t;
	typedef struct cg_virtual_reg_t cg_virtual_reg_t;
}


namespace EGL {

	struct FragmentGenerationInfo {
		cg_virtual_reg_t * regX;
		cg_virtual_reg_t * regDepth;
		cg_virtual_reg_t * regU;
		cg_virtual_reg_t * regV; 
		cg_virtual_reg_t * regFog;
		cg_virtual_reg_t * regR;
		cg_virtual_reg_t * regG;
		cg_virtual_reg_t * regB; 
		cg_virtual_reg_t * regA;	

		cg_virtual_reg_t * regInfo;
	};

	class FunctionCache;

	class MultiTexture;

	class CodeGenerator {

	public:
		// ----------------------------------------------------------------------
		// Code generation of triangle scan line
		// ----------------------------------------------------------------------
		void CompileRasterScanLine(FunctionCache * target);

		void SetState(const RasterizerState * state)	{ m_State = state; }
		const RasterizerState * GetState()				{ return m_State; }

	private:
		void GenerateRasterScanLine();
		void GenerateFragment(cg_proc_t * procedure, cg_block_t * currentBlock,
			cg_block_ref_t * continuation, FragmentGenerationInfo & fragmentInfo,
			int weight);

		void GenerateFetchTexColor(cg_proc_t * proc, cg_block_t * currentBlock,
								   FragmentGenerationInfo & fragmentInfo,
								   cg_virtual_reg_t *& regTexColorR,
								   cg_virtual_reg_t *& regTexColorG,			
								   cg_virtual_reg_t *& regTexColorB,			
								   cg_virtual_reg_t *& regTexColorA,
								   cg_virtual_reg_t *& regTexColor565);

		void FetchTexColor(cg_proc_t * proc, cg_block_t * currentBlock,
								   cg_virtual_reg_t * regTexData, 
								   cg_virtual_reg_t * regTexOffset,
								   cg_virtual_reg_t *& regTexColorR,
								   cg_virtual_reg_t *& regTexColorG,			
								   cg_virtual_reg_t *& regTexColorB,			
								   cg_virtual_reg_t *& regTexColorA,
								   cg_virtual_reg_t *& regTexColor565);

	private:
		const RasterizerState *	m_State;
		struct cg_module_t *m_Module;
	};

}


	// -------------------------------------------------------------------------
	// Offsets of structure members within info structure
	// -------------------------------------------------------------------------

#	define OFFSET_SURFACE_WIDTH					offsetof(RasterInfo, SurfaceWidth)
#	define OFFSET_SURFACE_HEIGHT				offsetof(RasterInfo, SurfaceHeight)
#	define OFFSET_SURFACE_DEPTH_BUFFER			offsetof(RasterInfo, DepthBuffer)
#	define OFFSET_SURFACE_COLOR_BUFFER			offsetof(RasterInfo, ColorBuffer)
#	define OFFSET_SURFACE_STENCIL_BUFFER		offsetof(RasterInfo, StencilBuffer)
#	define OFFSET_SURFACE_ALPHA_BUFFER			offsetof(RasterInfo, AlphaBuffer)
#	define OFFSET_TEXTURE_LOG_WIDTH				offsetof(RasterInfo, TextureLogWidth)
#	define OFFSET_TEXTURE_LOG_HEIGHT			offsetof(RasterInfo, TextureLogHeight)
#	define OFFSET_TEXTURE_LOG_BYTES_PER_PIXEL	offsetof(RasterInfo, TextureLogBytesPerPixel)
#	define OFFSET_TEXTURE_EXPONENT				offsetof(RasterInfo, TextureExponent)
#	define OFFSET_TEXTURE_DATA					offsetof(RasterInfo, TextureData)
#	define OFFSET_INVERSE_TABLE_PTR				offsetof(RasterInfo, InversionTablePtr)

	// -------------------------------------------------------------------------
	// For FractionalColor
	// -------------------------------------------------------------------------

#	define OFFSET_COLOR_RED					offsetof(FractionalColor, r)
#	define OFFSET_COLOR_GREEN				offsetof(FractionalColor, g)
#	define OFFSET_COLOR_BLUE				offsetof(FractionalColor, b)
#	define OFFSET_COLOR_ALPHA				offsetof(FractionalColor, a)

	// -------------------------------------------------------------------------
	// For EdgeCoord
	// -------------------------------------------------------------------------

#	define OFFSET_EDGE_COORD_X				offsetof(EdgeCoord, x)
#	define OFFSET_EDGE_COORD_INV_Z			offsetof(EdgeCoord, invZ)
#	define OFFSET_EDGE_COORD_DEPTH			offsetof(EdgeCoord, depth)

	// -------------------------------------------------------------------------
	// For TexCoord
	// -------------------------------------------------------------------------

#	define OFFSET_TEX_COORD_TU				offsetof(TexCoord, tu)
#	define OFFSET_TEX_COORD_TV				offsetof(TexCoord, tv)

	// -------------------------------------------------------------------------
	// For EdgeBuffer
	// -------------------------------------------------------------------------

#	define OFFSET_EDGE_BUFFER_WINDOW		offsetof(EdgePos, m_WindowCoords)
#	define OFFSET_EDGE_BUFFER_COLOR			offsetof(EdgePos, m_Color)
#	define OFFSET_EDGE_BUFFER_TEXTURE		offsetof(EdgePos, m_TextureCoords)
#	define OFFSET_EDGE_BUFFER_FOG			offsetof(EdgePos, m_FogDensity)

#	define OFFSET_EDGE_BUFFER_WINDOW_X		(OFFSET_EDGE_BUFFER_WINDOW + OFFSET_EDGE_COORD_X)
#	define OFFSET_EDGE_BUFFER_WINDOW_INV_Z	(OFFSET_EDGE_BUFFER_WINDOW + OFFSET_EDGE_COORD_INV_Z)
#	define OFFSET_EDGE_BUFFER_WINDOW_DEPTH	(OFFSET_EDGE_BUFFER_WINDOW + OFFSET_EDGE_COORD_DEPTH)

#	define OFFSET_EDGE_BUFFER_COLOR_R		(OFFSET_EDGE_BUFFER_COLOR + OFFSET_COLOR_RED)
#	define OFFSET_EDGE_BUFFER_COLOR_G		(OFFSET_EDGE_BUFFER_COLOR + OFFSET_COLOR_GREEN)
#	define OFFSET_EDGE_BUFFER_COLOR_B		(OFFSET_EDGE_BUFFER_COLOR + OFFSET_COLOR_BLUE)
#	define OFFSET_EDGE_BUFFER_COLOR_A		(OFFSET_EDGE_BUFFER_COLOR + OFFSET_COLOR_ALPHA)

#	define OFFSET_EDGE_BUFFER_TEX_TU		(OFFSET_EDGE_BUFFER_TEXTURE + OFFSET_TEX_COORD_TU)
#	define OFFSET_EDGE_BUFFER_TEX_TV		(OFFSET_EDGE_BUFFER_TEXTURE + OFFSET_TEX_COORD_TV)


#endif //ndef EGL_CODE_GENERATOR_H
