// ==========================================================================
//
// RasterTriangleDepthStencilPart.h	
//						JIT Class for 3D Rendering Library
//
//						This class encapsulates the specifics for managing
//						code generation of depth/stencil processing of
//						triangle rasterizers
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


#include "stdafx.h"
#include "RasterTriangleDepthStencilPart.h"
#include "inline.h"


using namespace EGL;


void RasterTriangleDepthStencilPart :: GenerateRasterBlockDepthStencil(const VaryingInfo * varyingInfo) {
	cg_proc_t * procedure = cg_proc_create(m_Module);

	FragmentGenerationInfo info;
	memset(&info, 0, sizeof(info));

	//typedef PixelMask (BlockDepthStencilFunction)(const RasterInfo * info, const Variables * variables, PixelMask * pixelMask);

	DECL_REG	(regRasterInfo);// RasterInfo structure pointer
	DECL_REG	(regVars);		// variable structure pointer
	DECL_REG	(regPixelMask);	// pixel mask data pointer

	procedure->num_args = 3;	// the previous three declarations make up the arguments

	cg_block_t * block = cg_block_create(procedure, 1);

	//PixelMask * mask = pixelMask, totalMask = 0;
	DECL_REG	(regMask0);			// beginning of y loop
	DECL_REG	(regMask1);			// end of y loop

	DECL_REG	(regTotalMask0);	// beginning of y loop
	DECL_REG	(regTotalMask1);	// end of y loop

	DECL_CONST_REG	(zero, 0);		// 0
	DECL_CONST_REG	(one, 1);		// 1
	DECL_CONST_REG	(two, 2);		// 2
	DECL_CONST_REG	(four, 4);		// 4
	DECL_CONST_REG	(blockSize, EGL_RASTER_BLOCK_SIZE);
	DECL_CONST_REG	(logBlockSize, EGL_LOG_RASTER_BLOCK_SIZE);

	DECL_CONST_REG	(maskSize, sizeof(PixelMask));

	cg_virtual_reg_t *	regBaseX = LOAD_DATA(block, regVars, OFFSET_VARIABLES_X);
	cg_virtual_reg_t *	regBaseY = LOAD_DATA(block, regVars, OFFSET_VARIABLES_Y);

	//I32 depth0 = vars->Depth.Value;
	cg_virtual_reg_t *	regDepthInit = LOAD_DATA(block, regVars, OFFSET_VARIABLES_DEPTH + OFFSET_INTERPOLANT_VALUE);
	cg_virtual_reg_t *	regDepthDx = LOAD_DATA(block, regVars, OFFSET_VARIABLES_DEPTH + OFFSET_INTERPOLANT_DX);
	cg_virtual_reg_t *	regDepthDy = LOAD_DATA(block, regVars, OFFSET_VARIABLES_DEPTH + OFFSET_INTERPOLANT_DY);

	DECL_REG	(regDepth0);		// beginning of y loop
	DECL_REG	(regDepth1);		// end of y loop
	DECL_REG	(regDepth2);		// end of x loop
	DECL_REG	(regDepth3);		// beginning of x loop
	DECL_REG	(regDepthStepY);

	//vars->Depth.dY - (vars->Depth.dX << EGL_LOG_RASTER_BLOCK_SIZE);
	DECL_REG	(regShiftedDx);

	LSL			(regShiftedDx, regDepthDx, logBlockSize);
	SUB			(regDepthStepY, regDepthDy, regShiftedDx);

	// initialize surface pointers in local info block
	//SurfaceInfo surfaceInfo = m_RasterInfo.SurfaceInfo;

	DECL_REG	(regDepthBuffer0);		// begin y loop
	DECL_REG	(regDepthBuffer1);		// end y loop
	DECL_REG	(regStencilBuffer0);	// begin y loop
	DECL_REG	(regStencilBuffer1);	// end y loop

	cg_virtual_reg_t * regDepthBuffer = LOAD_DATA(block, regRasterInfo, OFFSET_SURFACE_DEPTH_BUFFER);
	cg_virtual_reg_t * regStencilBuffer = LOAD_DATA(block, regRasterInfo, OFFSET_SURFACE_STENCIL_BUFFER);
	cg_virtual_reg_t * regPitch = LOAD_DATA(block, regRasterInfo, OFFSET_SURFACE_PITCH);
	
    //for (I32 iy = 0; iy < EGL_RASTER_BLOCK_SIZE; iy++) {

	DECL_REG	(regIY0);				// begin y loop
	DECL_REG	(regIY1);				// end y loop

	cg_block_ref_t * yLoopTop = cg_block_ref_create(procedure);
	cg_block_ref_t * yLoopEnd = cg_block_ref_create(procedure);

	DECL_CONST_REG(initRegIY0, EGL_RASTER_BLOCK_SIZE);		
	DECL_CONST_REG(initMask, 0);

	// stencil test passed
	block = cg_block_create(procedure, 2);
	yLoopTop->block = block;

	PHI			(regIY0, cg_create_virtual_reg_list(procedure->module->heap, regIY1, initRegIY0, NULL));
	PHI			(regMask0, cg_create_virtual_reg_list(procedure->module->heap, regPixelMask, regMask1, NULL));
	PHI			(regTotalMask0, cg_create_virtual_reg_list(procedure->module->heap, initMask, regTotalMask1, NULL));
	PHI			(regDepth0, cg_create_virtual_reg_list(procedure->module->heap, regDepthInit, regDepth1, NULL));
	PHI			(regDepthBuffer0, cg_create_virtual_reg_list(procedure->module->heap, regDepthBuffer, regDepthBuffer1, NULL));
	PHI			(regStencilBuffer0, cg_create_virtual_reg_list(procedure->module->heap, regStencilBuffer, regStencilBuffer1, NULL));

	//	PixelMask rowMask = 0;

	DECL_CONST_REG	(initIX0, 0);
	DECL_CONST_REG	(initRowMask, 0);

	DECL_REG	(regRowMask0);		// beginning of x loop
	DECL_REG	(regRowMask1);		// end of x loop

    //    for (I32 ix = 0; ix < EGL_RASTER_BLOCK_SIZE; ix++) {

	cg_block_ref_t * xLoopTop = cg_block_ref_create(procedure);
	block = cg_block_create(procedure, 4);
	xLoopTop->block = block;

	DECL_REG	(regIX0);			// beginning of x loop
	DECL_REG	(regIX1);			// end of x loop

	PHI			(regIX0, cg_create_virtual_reg_list(procedure->module->heap, regIX1, initIX0, NULL));
	PHI			(regDepth3, cg_create_virtual_reg_list(procedure->module->heap, regDepth0, regDepth2, NULL));
	PHI			(regRowMask0, cg_create_virtual_reg_list(procedure->module->heap, regRowMask1, initRowMask, NULL));

	DECL_REG	(regRowMask2);		// shifted mask
	DECL_REG	(regRowMask3);		// modified mask if written

	// rowMask >>= 1
	LSR			(regRowMask2, regRowMask0, one);

            // test and write depth and stencil
	//		bool written = FragmentDepthStencil(&m_RasterInfo, &surfaceInfo, ix, depth0 >> 4);
	cg_block_ref_t * notWritten = cg_block_ref_create(procedure);

	if (m_State->IsEnabledScissorTest()) {
		// scissor test goes here
		DECL_REG	(regX);
		DECL_REG	(regY);
		DECL_REG	(regTempY);
		DECL_CONST_REG(blockSize, EGL_RASTER_BLOCK_SIZE);		

		ADD		(regX, regIX0, regBaseX);
		SUB		(regTempY, blockSize, regIY0);
		ADD		(regY, regTempY, regBaseY);

		DECL_CONST_REG	(xBottom, m_State->m_ScissorTest.X);
		DECL_CONST_REG	(xTop, m_State->m_ScissorTest.X + m_State->m_ScissorTest.Width);
		DECL_CONST_REG	(yBottom, m_State->m_ScissorTest.Y);
		DECL_CONST_REG	(yTop, m_State->m_ScissorTest.Y + m_State->m_ScissorTest.Height);

		DECL_FLAGS	(xLow);
		DECL_FLAGS	(xHigh);
		DECL_FLAGS	(yLow);
		DECL_FLAGS	(yHigh);

		CMP		(xLow, regX, xBottom);
		BLT		(xLow, notWritten);
		CMP		(xHigh, regX, xTop);
		BGE		(xHigh, notWritten);
		CMP		(yLow, regY, yBottom);
		BLT		(yLow, notWritten);
		CMP		(yHigh, regY, yTop);
		BGE		(yHigh, notWritten);
	}

	DECL_REG(regShiftedDepth);

	LSR		(regShiftedDepth, regDepth3, four);

	info.regX = regIX0;
	info.regDepth = regShiftedDepth;

	GenerateFragmentDepthStencil(procedure, block, notWritten, 
		info, 4, regDepthBuffer0, regStencilBuffer0, false, true);

	//		rowMask |= (written ? (1 << (EGL_RASTER_BLOCK_SIZE - 1)) : 0);
	block = cg_block_create(procedure, 4);

	DECL_CONST_REG	(shiftedOne, (1 << (EGL_RASTER_BLOCK_SIZE - 1)));

	OR			(regRowMask3, regRowMask2, shiftedOne);

	// stencil test passed
	block = cg_block_create(procedure, 4);
	notWritten->block = block;

	PHI			(regRowMask1, cg_create_virtual_reg_list(procedure->module->heap, regRowMask2, regRowMask3, NULL));

	//		depth0 += vars->Depth.dX;
	ADD			(regDepth2, regDepth3, regDepthDx);
    //    }

	DECL_FLAGS	(regReachedXLoopEnd);
	ADD			(regIX1, regIX0, one);
	CMP			(regReachedXLoopEnd, regIX1, blockSize);
	BNE			(regReachedXLoopEnd, xLoopTop);

	block = cg_block_create(procedure, 2);

	//	*mask++ = rowMask;
#if EGL_RASTER_BLOCK_SIZE <= 8
	STB			(regRowMask1, regMask0);
#elif EGL_RASTER_BLOCK_SIZE <= 16
	STH			(regRowMask1, regMask0);
#else
	ST			(regRowMask1, regMask0);
#endif

	ADD			(regMask1,		regMask0, maskSize);

	//	totalMask |= rowMask;
	OR			(regTotalMask1, regTotalMask0, regRowMask1);

	// <---- loop termination goes here
	DECL_FLAGS	(regReachedYLoopEnd);
	SUB_S		(regIY1, regReachedYLoopEnd, regIY0, one);
	BEQ			(regReachedYLoopEnd, yLoopEnd);

	block = cg_block_create(procedure, 2);

	//	depth0 += vars->Depth.dY - (vars->Depth.dX << EGL_LOG_RASTER_BLOCK_SIZE);
	ADD			(regDepth1, regDepth2, regDepthStepY);

	//	surfaceInfo.DepthBuffer += surfaceInfo.Pitch;
	DECL_REG	(regPitch2);
	LSL			(regPitch2, regPitch, one);
	ADD			(regDepthBuffer1, regDepthBuffer0, regPitch2);

	//	surfaceInfo.StencilBuffer += surfaceInfo.Pitch;
	DECL_REG	(regPitch4);
	LSL			(regPitch4, regPitch, two);
	ADD			(regStencilBuffer1, regStencilBuffer0, regPitch4);

    //}
	BRA			(yLoopTop);

	block = cg_block_create(procedure, 1);
	yLoopEnd->block = block;

	//return totalMask;
	RET_VALUE(regTotalMask1);
}

// --------------------------------------------------------------------------
// Implementation of virtual functions in PipelinePart
// --------------------------------------------------------------------------

bool RasterTriangleDepthStencilPart :: CompareState(const void * first, const void * second) const {
	const RasterizerState * firstState = static_cast<const RasterizerState *>(first);
	const RasterizerState * secondState = static_cast<const RasterizerState *>(second);
	
	return firstState->ComparePolygonDepthStencil(*secondState);
}

void RasterTriangleDepthStencilPart :: Compile(FunctionCache * target, const VaryingInfo * varyingInfo, const void * state) {
	m_State = static_cast<const RasterizerState *>(state);
	BeginGenerateCode();
	GenerateRasterBlockDepthStencil(varyingInfo);
	EndGenerateCode(target, state);
}

PipelinePart::Part RasterTriangleDepthStencilPart :: GetPart() const {
	return PipelinePart::PartRasterBlockDepthStencil;
}
