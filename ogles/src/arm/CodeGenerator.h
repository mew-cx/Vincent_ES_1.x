#ifndef EGL_CODE_GENERATOR_H
#define EGL_CODE_GENERATOR_H 1

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
#include "FunctionCache.h"


extern "C" {
#ifdef __GCC32__
    struct cg_module_t;
    struct cg_proc_t;
    struct cg_block_t;
    struct cg_block_ref_t;
    struct cg_virtual_reg_t;
#else
	typedef struct cg_module_t cg_module_t;
	typedef struct cg_proc_t cg_proc_t;
	typedef struct cg_block_t cg_block_t;
	typedef struct cg_block_ref_t cg_block_ref_t;
	typedef struct cg_virtual_reg_t cg_virtual_reg_t;
#endif
}


namespace EGL {

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

		cg_virtual_reg_t * regInfo;
		cg_virtual_reg_t * regTexture[EGL_NUM_TEXTURE_UNITS];
	};

	class FunctionCache;

	class MultiTexture;

	// ----------------------------------------------------------------------
	// Compile-time information for vertex fetch and transformation
	// ----------------------------------------------------------------------

	struct ArrayState {
		GLenum		Type;
		size_t		Size;
		boolean		Enabled;
	};

	struct RenderState {
		VaryingInfo	Varying;
		ArrayState	Coord;
		ArrayState	Normal;
		ArrayState	Color;
		ArrayState	TexCoord[EGL_NUM_TEXTURE_UNITS];
		boolean		TextureMatrixIdentity[EGL_NUM_TEXTURE_UNITS];
		boolean		NeedsNormal;
		boolean		NeedsColor;
		boolean		NeedsEyeCoords;
	};

	// ----------------------------------------------------------------------
	// Runtime information for vertex fetch and transformation
	// ----------------------------------------------------------------------

	struct ArrayInfo {
		void * 	Base;
		GLsizei	Stride;
	};

	struct RenderInfo {
		ArrayInfo	Coord;
		ArrayInfo	Normal;
		ArrayInfo	Color;
		ArrayInfo	TexCoord[EGL_NUM_TEXTURE_UNITS];
		const GLfixed *	ModelviewProjectionMatrix;
		const GLfixed * ModelviewMatrix;
		const GLfixed * InvModelviewMatrix;
		const GLfixed * TextureMatrix[EGL_NUM_TEXTURE_UNITS];
	};

	class CodeGenerator {

	public:
		// ----------------------------------------------------------------------
		// Code generation of rasterizer
		// ----------------------------------------------------------------------
		void Compile(FunctionCache * target, FunctionCache::FunctionType type, const VaryingInfo * varyingInfo,
			void (CodeGenerator::*function)(const VaryingInfo * varyingInfo));

		void SetState(const RasterizerState * state)	{ m_State = state; }
		const RasterizerState * GetState()				{ return m_State; }

		void GenerateRasterLine(const VaryingInfo * varyingInfo);
		void GenerateRasterPoint(const VaryingInfo * varyingInfo);
		void GenerateRasterBlockDepthStencil(const VaryingInfo * varyingInfo);
		void GenerateRasterBlockEdgeDepthStencil(const VaryingInfo * varyingInfo);
		void GenerateRasterBlockColorAlpha(const VaryingInfo * varyingInfo);
		void GenerateRasterScanLine();

		// ----------------------------------------------------------------------
		// Code generation of vertex processing
		// ----------------------------------------------------------------------

		void GenerateFetch(const RenderState * state);

		// fetch vertex coordinates and transform by given matrix
		void GenerateCoordFetchMultiply(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			cg_virtual_reg_t * matrixBase,
			GLenum type, size_t size);

		void GenerateCoordFetchMultiply2(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			cg_virtual_reg_t * matrixBase, GLenum type);

		void GenerateCoordFetchMultiply3(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			cg_virtual_reg_t * matrixBase, GLenum type);

		void GenerateCoordFetchMultiply4(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			cg_virtual_reg_t * matrixBase, GLenum type);

		// fetch normal and transform by given matrix
		void GenerateNormalFetchMultiply(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			cg_virtual_reg_t * matrixBase,
			GLenum type);

		// fetch color
		void GenerateColorFetch(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			GLenum type);

		// fetch tex coords and transform by given matrix if not null
		void GenerateTexCoordFetchMultiply(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			cg_virtual_reg_t * matrixBase, GLenum type, size_t size);

		void GenerateTexCoordFetchMultiply2(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			cg_virtual_reg_t * matrixBase, GLenum type);

		void GenerateTexCoordFetchMultiply3(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			cg_virtual_reg_t * matrixBase, GLenum type);

		void GenerateTexCoordFetchMultiply4(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			cg_virtual_reg_t * matrixBase, GLenum type);

		// load a specific element of a matrix
		cg_virtual_reg_t * LoadMatrixElement(cg_block_t * block,
				cg_virtual_reg_t * base, int row, int column);

		cg_virtual_reg_t * LoadVectorElement(cg_block_t * block, cg_virtual_reg_t * base, int element, GLenum type);

	private:
		void GenerateFragment(cg_proc_t * procedure, cg_block_t * currentBlock,
			cg_block_ref_t * continuation, FragmentGenerationInfo & fragmentInfo,
			int weight, bool forceScissor = false);

		cg_block_t * GenerateFragmentDepthStencil(cg_proc_t * procedure, cg_block_t * currentBlock,
			cg_block_ref_t * continuation, FragmentGenerationInfo & fragmentInfo,
			int weight, cg_virtual_reg_t * regDepthBuffer = 0,
			cg_virtual_reg_t * regStencilBuffer = 0, bool forceScissor = false, bool noScissor = false);

		void GenerateFragmentColorAlpha(cg_proc_t * procedure, cg_block_t * currentBlock,
			cg_block_ref_t * continuation, FragmentGenerationInfo & fragmentInfo,
			int weight, cg_virtual_reg_t * regColorBuffer = 0,
			cg_virtual_reg_t * regAlphaBuffer = 0);

		void GenerateFetchTexColor(cg_proc_t * proc, cg_block_t * currentBlock,
								   size_t unit,
								   FragmentGenerationInfo & fragmentInfo,
								   cg_virtual_reg_t *& regTexColorR,
								   cg_virtual_reg_t *& regTexColorG,
								   cg_virtual_reg_t *& regTexColorB,
								   cg_virtual_reg_t *& regTexColorA,
								   cg_virtual_reg_t *& regTexColor565);

		void FetchTexColor(cg_proc_t * proc, cg_block_t * currentBlock,
								   const RasterizerState::TextureState * textureState,
								   cg_virtual_reg_t * regTexData,
								   cg_virtual_reg_t * regTexOffset,
								   cg_virtual_reg_t *& regTexColorR,
								   cg_virtual_reg_t *& regTexColorG,
								   cg_virtual_reg_t *& regTexColorB,
								   cg_virtual_reg_t *& regTexColorA,
								   cg_virtual_reg_t *& regTexColor565);

		cg_virtual_reg_t * Mul255(cg_block_t * currentBlock, cg_virtual_reg_t * first, cg_virtual_reg_t * second);
		cg_virtual_reg_t * Add(cg_block_t * currentBlock, cg_virtual_reg_t * first, cg_virtual_reg_t * second);
		cg_virtual_reg_t * AddSigned(cg_block_t * currentBlock, cg_virtual_reg_t * first, cg_virtual_reg_t * second);
		cg_virtual_reg_t * Sub(cg_block_t * currentBlock, cg_virtual_reg_t * first, cg_virtual_reg_t * second);
		cg_virtual_reg_t * AddSaturate255(cg_block_t * currentBlock, cg_virtual_reg_t * first, cg_virtual_reg_t * second);
		cg_virtual_reg_t * ClampTo255(cg_block_t * currentBlock, cg_virtual_reg_t * value);
		cg_virtual_reg_t * ExtractBitFieldTo255(cg_block_t * currentBlock, cg_virtual_reg_t * value, size_t low, size_t high);
		cg_virtual_reg_t * BitFieldFrom255(cg_block_t * currentBlock, cg_virtual_reg_t * value, size_t low, size_t high);
		cg_virtual_reg_t * Dot3(cg_block_t * currentBlock, cg_virtual_reg_t * r[], cg_virtual_reg_t * g[], cg_virtual_reg_t * b[]);
		cg_virtual_reg_t * SignedVal(cg_block_t * block, cg_virtual_reg_t * value);
		void Color565FromRGB(cg_block_t * block, cg_virtual_reg_t * result,
							 cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b);
		cg_virtual_reg_t * Color565FromRGB(cg_block_t * block,
							 cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b);

		cg_virtual_reg_t * Blend255(cg_block_t * currentBlock, cg_virtual_reg_t * first, cg_virtual_reg_t * second,
								    cg_virtual_reg_t * alpha);
		cg_virtual_reg_t * Blend255(cg_block_t * currentBlock, U8 constant, cg_virtual_reg_t * second,
								    cg_virtual_reg_t * alpha);

	private:
		const RasterizerState *	m_State;
		struct cg_module_t *m_Module;
	};

}


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
#	define OFFSET_SURFACE_DEPTH_BUFFER			offsetof(RasterInfo, RasterSurface.DepthBuffer)
#	define OFFSET_SURFACE_COLOR_BUFFER			offsetof(RasterInfo, RasterSurface.ColorBuffer)
#	define OFFSET_SURFACE_STENCIL_BUFFER		offsetof(RasterInfo, RasterSurface.StencilBuffer)
#	define OFFSET_SURFACE_ALPHA_BUFFER			offsetof(RasterInfo, RasterSurface.AlphaBuffer)

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

#endif //ndef EGL_CODE_GENERATOR_H
