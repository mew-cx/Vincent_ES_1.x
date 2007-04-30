#ifndef EGL_RASTER_PART_H
#define EGL_RASTER_PART_H 1

// ==========================================================================
//
// RasterPart.h			JIT Class for 3D Rendering Library
//
//						Base class to facilitate management of code
//						generation of various rasterizer parts
//
// --------------------------------------------------------------------------
//
// 04-24-2006	Hans-Martin Will	initial version
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


#include "PipelinePart.h"
#include "Rasterizer.h"
#include "RasterizerState.h"

namespace EGL {

	struct FragmentGenerationInfo;

	class RasterPart: public PipelinePart {
	public:
		// virtual method defined in PipelinePart
		void CopyState(void * target, const void * source) const;

	
	protected:
		// functions and members common to all rasterizer generation goes here
		void GenerateFragment(cg_proc_t * procedure, cg_block_t * currentBlock,
			cg_block_ref_t * continuation, FragmentGenerationInfo & fragmentInfo,
			int weight, bool forceScissor = false);

		cg_block_t * GenerateFragmentDepthStencil(cg_proc_t * procedure, cg_block_t * currentBlock,
			cg_block_ref_t * continuation, FragmentGenerationInfo & fragmentInfo,
			int weight, cg_virtual_reg_t * regDepthBuffer = 0,
			bool forceScissor = false, bool noScissor = false);

		cg_virtual_reg_t * UpdateStencilValue(cg_block_t * block, RasterizerState::StencilOp op,
			cg_virtual_reg_t * regStencilValue, cg_virtual_reg_t * regStencilRef);
								 
		void WriteDepthStencil(cg_block_t * block, cg_virtual_reg_t * regDepthStencilAddr,
							   cg_virtual_reg_t * regOldDepth, cg_virtual_reg_t * regNewDepth,
							   cg_virtual_reg_t * regOldStencil, cg_virtual_reg_t * regNewStencil);

		cg_virtual_reg_t * CalcBlockedOffset(cg_block_t * block, FragmentGenerationInfo & fragmentInfo,
											 cg_virtual_reg_t * regX, cg_virtual_reg_t * regY);

		void GenerateFragmentColorAlpha(cg_proc_t * procedure, cg_block_t * currentBlock,
			cg_block_ref_t * continuation, FragmentGenerationInfo & fragmentInfo,
			int weight, cg_virtual_reg_t * regColorBuffer = 0);

		void GenerateFetchTexColor(cg_proc_t * proc, cg_block_t * currentBlock,
								   size_t unit,
								   FragmentGenerationInfo & fragmentInfo,
								   cg_virtual_reg_t *& regTexColorR,
								   cg_virtual_reg_t *& regTexColorG,
								   cg_virtual_reg_t *& regTexColorB,
								   cg_virtual_reg_t *& regTexColorA,
								   cg_virtual_reg_t *& regTexColorWord);

		void FetchTexColor(cg_proc_t * proc, cg_block_t * currentBlock,
								   const RasterizerState::TextureState * textureState,
								   cg_virtual_reg_t * regTexData,
								   cg_virtual_reg_t * regTexOffset,
								   cg_virtual_reg_t *& regTexColorR,
								   cg_virtual_reg_t *& regTexColorG,
								   cg_virtual_reg_t *& regTexColorB,
								   cg_virtual_reg_t *& regTexColorA,
								   cg_virtual_reg_t *& regTexColorWord);

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
		void Color5551FromRGBA(cg_block_t * block, cg_virtual_reg_t * result,
							   cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b, cg_virtual_reg_t * a);
		void Color4444FromRGBA(cg_block_t * block, cg_virtual_reg_t * result,
							   cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b, cg_virtual_reg_t * a);
		void Color8888FromRGBA(cg_block_t * block, cg_virtual_reg_t * result,
							   cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b, cg_virtual_reg_t * a);
		void ColorWordFromRGBA(cg_block_t * block, cg_virtual_reg_t * result,
							   cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b, cg_virtual_reg_t * a);

		cg_virtual_reg_t * Color565FromRGB(cg_block_t * block,
							 cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b);
		cg_virtual_reg_t * Color5551FromRGBA(cg_block_t * block,
							 cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b, cg_virtual_reg_t * a);
		cg_virtual_reg_t * Color4444FromRGBA(cg_block_t * block,
							 cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b, cg_virtual_reg_t * a);
		cg_virtual_reg_t * Color8888FromRGBA(cg_block_t * block,
							 cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b, cg_virtual_reg_t * a);
		cg_virtual_reg_t * ColorWordFromRGBA(cg_block_t * block,
							 cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b, cg_virtual_reg_t * a);

		cg_virtual_reg_t * Blend255(cg_block_t * currentBlock, cg_virtual_reg_t * first, cg_virtual_reg_t * second,
								    cg_virtual_reg_t * alpha);
		cg_virtual_reg_t * Blend255(cg_block_t * currentBlock, U8 constant, cg_virtual_reg_t * second,
								    cg_virtual_reg_t * alpha);
										
	protected:
		const RasterizerState * m_State;
	};
	
}

#endif // ndef EGL_RASTER_PART_H */