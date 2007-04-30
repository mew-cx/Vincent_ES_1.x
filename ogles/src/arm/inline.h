#ifndef EGL_INLINE_H
#define EGL_INLINE_H 1

// ==========================================================================
//
// inline.h				JIT Class for 3D Rendering Library
//
//						Inline functions used in various code generation
//						functions
//
// --------------------------------------------------------------------------
//
// 04-26-2006	Hans-Martin Will	initial version
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

#include "Surface.h"
#include "Texture.h"
#include "codegen.h"
#include "instruction.h"
#include "emit.h"
#include "arm-dis.h"


namespace EGL {
	// -----------------------------------------------------------------------
	// virtual register bindings to be passed into fragment code generation
	// -----------------------------------------------------------------------
	struct FragmentGenerationInfo {
		cg_virtual_reg_t * regX;
		cg_virtual_reg_t * regY;
		cg_virtual_reg_t * regDepth;
		cg_virtual_reg_t * regU[EGL_NUM_TEXTURE_UNITS];
		cg_virtual_reg_t * regV[EGL_NUM_TEXTURE_UNITS];
		cg_virtual_reg_t * regFog;
		cg_virtual_reg_t * regR;
		cg_virtual_reg_t * regG;
		cg_virtual_reg_t * regB;
		cg_virtual_reg_t * regA;
		cg_virtual_reg_t * regCoverage;

		cg_virtual_reg_t * regInfo;
		cg_virtual_reg_t * regTexture[EGL_NUM_TEXTURE_UNITS];
	};
}

#define ALLOC_REG(reg) reg = cg_virtual_reg_create(procedure, cg_reg_type_general)
#define ALLOC_FLAGS(reg) reg = cg_virtual_reg_create(procedure, cg_reg_type_flags)
#define DECL_REG(reg) cg_virtual_reg_t * reg = cg_virtual_reg_create(procedure, cg_reg_type_general)
#define DECL_FLAGS(reg) cg_virtual_reg_t * reg = cg_virtual_reg_create(procedure, cg_reg_type_flags)
#define DECL_CONST_REG(reg, value) cg_virtual_reg_t * reg = cg_virtual_reg_create(procedure, cg_reg_type_general); LDI(reg, value)


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

	inline cg_virtual_reg_t * LOAD_DATA_HALF(cg_block_t * block, cg_virtual_reg_t * base, I32 constant) {
		cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
		cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);
		cg_virtual_reg_t * value = cg_virtual_reg_create(block->proc, cg_reg_type_general);

		LDI(offset, constant);
		ADD(addr, base, offset);
		LDH(value, addr);

		return value;
	}

	inline void STORE_DATA(cg_block_t * block, cg_virtual_reg_t * base, I32 constant, cg_virtual_reg_t * value) {
		cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
		cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);

		LDI(offset, constant);
		ADD(addr, base, offset);
		STW(value, addr);

	}

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

// -------------------------------------------------------------------------
// Various data structure offsets used by the JIT
// -------------------------------------------------------------------------

	// -------------------------------------------------------------------------
	// Offsets of structure members within Interpolant
	// -------------------------------------------------------------------------

#	define OFFSET_INTERPOLANT_VALUE			offsetof(Interpolant, Value)
#	define OFFSET_INTERPOLANT_DX			offsetof(Interpolant, dX)
#	define OFFSET_INTERPOLANT_DY			offsetof(Interpolant, dY)
#	define OFFSET_INTERPOLANT_D_BLOCK_LINE	offsetof(Interpolant, dBlockLine)

	// -------------------------------------------------------------------------
	// Offsets of structure members within Variables
	// -------------------------------------------------------------------------

#	define OFFSET_VARIABLES_X				offsetof(Variables, x)
#	define OFFSET_VARIABLES_Y				offsetof(Variables, y)
#	define OFFSET_VARIABLES_DEPTH			offsetof(Variables, Depth)
#	define OFFSET_VARIABLES_INV_W			offsetof(Variables, InvW)
#	define OFFSET_VARIABLES_VARYING_INV_W	offsetof(Variables, VaryingInvW)

	// -------------------------------------------------------------------------
	// Offsets of structure members within Edge
	// -------------------------------------------------------------------------

#	define OFFSET_EDGE_CY					offsetof(Edge, CY)
#	define OFFSET_EDGE_FDX					offsetof(Edge, FDX)
#	define OFFSET_EDGE_FDY					offsetof(Edge, FDY)

	// -------------------------------------------------------------------------
	// Offsets of structure members within Edges
	// -------------------------------------------------------------------------

#	define OFFSET_EDGES_EDGE_12				offsetof(Edges, edge12)
#	define OFFSET_EDGES_EDGE_23				offsetof(Edges, edge23)
#	define OFFSET_EDGES_EDGE_31				offsetof(Edges, edge31)

	// -------------------------------------------------------------------------
	// Offsets of structure members within info structure
	// -------------------------------------------------------------------------

#	define OFFSET_SURFACE_WIDTH					offsetof(RasterInfo, RasterSurface.Width)
#	define OFFSET_SURFACE_HEIGHT				offsetof(RasterInfo, RasterSurface.Height)
#	define OFFSET_SURFACE_PITCH					offsetof(RasterInfo, RasterSurface.Pitch)
#	define OFFSET_SURFACE_DEPTH_STENCIL_BUFFER	offsetof(RasterInfo, RasterSurface.DepthStencilBuffer)
#	define OFFSET_SURFACE_COLOR_BUFFER			offsetof(RasterInfo, RasterSurface.ColorBuffer)

#	define OFFSET_TEXTURES						offsetof(RasterInfo, Textures)
#	define OFFSET_MIPMAP_LEVEL					offsetof(RasterInfo, MipmapLevel)
#	define OFFSET_MAX_MIPMAP_LEVEL				offsetof(RasterInfo, MaxMipmapLevel)

#	define OFFSET_TEXTURE_LOG_WIDTH				offsetof(Texture, m_LogWidth)
#	define OFFSET_TEXTURE_LOG_HEIGHT			offsetof(Texture, m_LogHeight)
#	define OFFSET_TEXTURE_DATA					offsetof(Texture, m_Data)
#	define SIZE_TEXTURE							sizeof(Texture)


#	define OFFSET_INVERSE_TABLE_PTR				offsetof(RasterInfo, InversionTablePtr)

	// -------------------------------------------------------------------------
	// For FractionalColor
	// -------------------------------------------------------------------------

#	define OFFSET_COLOR_RED					offsetof(FractionalColor, r)
#	define OFFSET_COLOR_GREEN				offsetof(FractionalColor, g)
#	define OFFSET_COLOR_BLUE				offsetof(FractionalColor, b)
#	define OFFSET_COLOR_ALPHA				offsetof(FractionalColor, a)

	// -------------------------------------------------------------------------
	// For ScreenCoord
	// -------------------------------------------------------------------------

#	define OFFSET_SCREEN_X					offsetof(ScreenCoord, x)
#	define OFFSET_SCREEN_Y					offsetof(ScreenCoord, y)
#	define OFFSET_SCREEN_INV_Z				offsetof(ScreenCoord, invW)
#	define OFFSET_SCREEN_DEPTH				offsetof(ScreenCoord, depth)

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
#	define OFFSET_TEX_COORD_DTUDX			offsetof(TexCoord, dtudx)
#	define OFFSET_TEX_COORD_DTUDY			offsetof(TexCoord, dtudy)
#	define OFFSET_TEX_COORD_DTVDX			offsetof(TexCoord, dtvdx)
#	define OFFSET_TEX_COORD_DTVDY			offsetof(TexCoord, dtvdy)

	// -------------------------------------------------------------------------
	// For Vertex
	// -------------------------------------------------------------------------

#	define OFFSET_RASTER_POS_CLIP			offsetof(Vertex, m_ClipCoords)
#	define OFFSET_RASTER_POS_EYE			offsetof(Vertex, m_EyeCoords)
#	define OFFSET_RASTER_POS_NORMAL			offsetof(Vertex, m_EyeNormal)
#	define OFFSET_RASTER_POS_WINDOW			offsetof(Vertex, m_WindowCoords)
#	define OFFSET_RASTER_POS_RAW_COLOR		offsetof(Vertex, m_Color)
#	define OFFSET_RASTER_POS_COLOR			(offsetof(Vertex, m_Varying) + varyingInfo->colorIndex * sizeof(U32))
#	define OFFSET_RASTER_POS_TEXTURE(unit)	(offsetof(Vertex, m_Varying) + varyingInfo->textureBase[unit] * sizeof(U32))
#	define OFFSET_RASTER_POS_FOG			(offsetof(Vertex, m_Varying) + varyingInfo->fogIndex * sizeof(U32))

#	define OFFSET_RASTER_POS_WINDOW_X		(OFFSET_RASTER_POS_WINDOW + OFFSET_SCREEN_X)
#	define OFFSET_RASTER_POS_WINDOW_Y		(OFFSET_RASTER_POS_WINDOW + OFFSET_SCREEN_Y)
#	define OFFSET_RASTER_POS_WINDOW_INV_Z	(OFFSET_RASTER_POS_WINDOW + OFFSET_SCREEN_INV_Z)
#	define OFFSET_RASTER_POS_WINDOW_DEPTH	(OFFSET_RASTER_POS_WINDOW + OFFSET_SCREEN_DEPTH)

#	define OFFSET_RASTER_POS_COLOR_R		(OFFSET_RASTER_POS_COLOR + OFFSET_COLOR_RED)
#	define OFFSET_RASTER_POS_COLOR_G		(OFFSET_RASTER_POS_COLOR + OFFSET_COLOR_GREEN)
#	define OFFSET_RASTER_POS_COLOR_B		(OFFSET_RASTER_POS_COLOR + OFFSET_COLOR_BLUE)
#	define OFFSET_RASTER_POS_COLOR_A		(OFFSET_RASTER_POS_COLOR + OFFSET_COLOR_ALPHA)

#	define OFFSET_RASTER_POS_TEX_TU(unit)	(OFFSET_RASTER_POS_TEXTURE(unit) + OFFSET_TEX_COORD_TU)
#	define OFFSET_RASTER_POS_TEX_TV(unit)	(OFFSET_RASTER_POS_TEXTURE(unit) + OFFSET_TEX_COORD_TV)

#	define OFFSET_RASTER_POS_TEX_DTUDX		(OFFSET_RASTER_POS_TEXTURE + OFFSET_TEX_COORD_DTUDX)
#	define OFFSET_RASTER_POS_TEX_DTUDY		(OFFSET_RASTER_POS_TEXTURE + OFFSET_TEX_COORD_DTUDY)
#	define OFFSET_RASTER_POS_TEX_DTVDX		(OFFSET_RASTER_POS_TEXTURE + OFFSET_TEX_COORD_DTVDX)
#	define OFFSET_RASTER_POS_TEX_DTVDY		(OFFSET_RASTER_POS_TEXTURE + OFFSET_TEX_COORD_DTVDY)

	// -------------------------------------------------------------------------
	// For ArrayInfo
	// -------------------------------------------------------------------------

#	define OFFSET_ARRAY_INFO_BASE			offsetof(ArrayInfo, Base)
#	define OFFSET_ARRAY_INFO_STRIDE			offsetof(ArrayInfo, Stride)

	// -------------------------------------------------------------------------
	// For RenderInfo
	// -------------------------------------------------------------------------

#	define OFFSET_RENDER_INFO_COORD				offsetof(RenderInfo, Coord)
#	define OFFSET_RENDER_INFO_NORMAL			offsetof(RenderInfo, Normal)
#	define OFFSET_RENDER_INFO_COLOR				offsetof(RenderInfo, Color)
#	define OFFSET_RENDER_INFO_TEX_COORD(unit)	(offsetof(RenderInfo, TexCoord) + (unit) * sizeof(ArrayInfo))
#	define OFFSET_RENDER_INFO_MV				offsetof(RenderInfo, ModelviewMatrix)
#	define OFFSET_RENDER_INFO_MVP				offsetof(RenderInfo, ModelviewProjectionMatrix)
#	define OFFSET_RENDER_INFO_INV_MV			offsetof(RenderInfo, InvModelviewMatrix)
#	define OFFSET_RENDER_INFO_TEX(unit)			(offsetof(RenderInfo, TextureMatrix) + (unit) * sizeof(const GLfixed *))

#	define OFFSET_RENDER_INFO_COORD_BASE		(OFFSET_RENDER_INFO_COORD + OFFSET_ARRAY_INFO_BASE)
#	define OFFSET_RENDER_INFO_COORD_STRIDE		(OFFSET_RENDER_INFO_COORD + OFFSET_ARRAY_INFO_STRIDE)
#	define OFFSET_RENDER_INFO_NORMAL_BASE		(OFFSET_RENDER_INFO_NORMAL + OFFSET_ARRAY_INFO_BASE)
#	define OFFSET_RENDER_INFO_NORMAL_STRIDE		(OFFSET_RENDER_INFO_NORMAL + OFFSET_ARRAY_INFO_STRIDE)
#	define OFFSET_RENDER_INFO_COLOR_BASE		(OFFSET_RENDER_INFO_COLOR + OFFSET_ARRAY_INFO_BASE)
#	define OFFSET_RENDER_INFO_COLOR_STRIDE		(OFFSET_RENDER_INFO_COLOR + OFFSET_ARRAY_INFO_STRIDE)

#	define OFFSET_RENDER_INFO_TEX_COORD_BASE(u)		(OFFSET_RENDER_INFO_TEX_COORD(u) + OFFSET_ARRAY_INFO_BASE)
#	define OFFSET_RENDER_INFO_TEX_COORD_STRIDE(u)	(OFFSET_RENDER_INFO_TEX_COORD(u) + OFFSET_ARRAY_INFO_STRIDE)

#endif // ndef EGL_INLINE_H