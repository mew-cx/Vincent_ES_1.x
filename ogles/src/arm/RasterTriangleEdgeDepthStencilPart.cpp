// ==========================================================================
//
// RasterTriangleEdgeDepthStencilPart.h	JIT Class for 3D Rendering Library
//
//						This class encapsulates the specifics for managing
//						code generation of edge/depth/stencil processing
//						of triangle rasterization
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
#include "RasterTriangleEdgeDepthStencilPart.h"
#include "inline.h"


using namespace EGL;


void RasterTriangleEdgeDepthStencilPart :: GenerateRasterBlockEdgeDepthStencil(const VaryingInfo * varyingInfo) {
	cg_proc_t * procedure = cg_proc_create(m_Module);

	FragmentGenerationInfo info;
	memset(&info, 0, sizeof(info));

	//typedef PixelMask (BlockEdgeDepthStencilFunction)(const RasterInfo * info, const Variables * variables, const Edges * edges, PixelMask * pixelMask);

	DECL_REG	(regRasterInfo);// RasterInfo structure pointer
	DECL_REG	(regVars);		// Variables structure pointer
	DECL_REG	(regEdges);		// Edges structure pointer
	DECL_REG	(regPixelMask);	// pixel mask data pointer

	procedure->num_args = 4;	// the previous three declarations make up the arguments

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

	DECL_REG	(regDepth0);		// beginning of y loop
	DECL_REG	(regDepth1);		// end of y loop
	DECL_REG	(regDepth2);		// end of x loop
	DECL_REG	(regDepth3);		// beginning of x loop

	// initialize surface pointers in local info block
	//SurfaceInfo surfaceInfo = m_RasterInfo.SurfaceInfo;

	DECL_REG	(regDepthBuffer0);		// begin y loop
	DECL_REG	(regDepthBuffer1);		// end y loop

	cg_virtual_reg_t * regDepthBuffer = LOAD_DATA(block, regRasterInfo, OFFSET_SURFACE_DEPTH_STENCIL_BUFFER);
	cg_virtual_reg_t * regPitch = LOAD_DATA(block, regRasterInfo, OFFSET_SURFACE_PITCH);
	
	//I32 CY1 = edges->edge12.CY;
	//I32 CY2 = edges->edge23.CY;
	//I32 CY3 = edges->edge31.CY;
	cg_virtual_reg_t * regCY1Init = LOAD_DATA(block, regEdges, OFFSET_EDGES_EDGE_12 + OFFSET_EDGE_CY);
	cg_virtual_reg_t * regCY2Init = LOAD_DATA(block, regEdges, OFFSET_EDGES_EDGE_23 + OFFSET_EDGE_CY);
	cg_virtual_reg_t * regCY3Init = LOAD_DATA(block, regEdges, OFFSET_EDGES_EDGE_31 + OFFSET_EDGE_CY);

    //for (I32 iy = 0; iy < EGL_RASTER_BLOCK_SIZE; iy++) {

	DECL_REG	(regIY0);				// begin y loop
	DECL_REG	(regIY1);				// end y loop
	DECL_REG	(regCY10);				// begin y loop
	DECL_REG	(regCY11);				// end y loop
	DECL_REG	(regCY20);				// begin y loop
	DECL_REG	(regCY21);				// end y loop
	DECL_REG	(regCY30);				// begin y loop
	DECL_REG	(regCY31);				// end y loop

	DECL_CONST_REG(initRegIY0, EGL_RASTER_BLOCK_SIZE);		
	DECL_CONST_REG(initMask, 0);

	cg_block_ref_t * yLoopTop = cg_block_ref_create(procedure);
	cg_block_ref_t * yLoopEnd = cg_block_ref_create(procedure);

	// stencil test passed
	block = cg_block_create(procedure, 2);
	yLoopTop->block = block;

	PHI			(regCY10, cg_create_virtual_reg_list(procedure->module->heap, regCY11, regCY1Init, NULL)); 
	PHI			(regCY20, cg_create_virtual_reg_list(procedure->module->heap, regCY21, regCY2Init, NULL)); 
	PHI			(regCY30, cg_create_virtual_reg_list(procedure->module->heap, regCY31, regCY3Init, NULL)); 
	PHI			(regIY0, cg_create_virtual_reg_list(procedure->module->heap, regIY1, initRegIY0, NULL));
	PHI			(regMask0, cg_create_virtual_reg_list(procedure->module->heap, regPixelMask, regMask1, NULL));
	PHI			(regTotalMask0, cg_create_virtual_reg_list(procedure->module->heap, initMask, regTotalMask1, NULL));
	PHI			(regDepth0, cg_create_virtual_reg_list(procedure->module->heap, regDepthInit, regDepth1, NULL));
	PHI			(regDepthBuffer0, cg_create_virtual_reg_list(procedure->module->heap, regDepthBuffer, regDepthBuffer1, NULL));

    //  GLint CX1 = CY1, CX2 = CY2, CX3 = CY3;
	DECL_REG	(regCX1Init);
	DECL_REG	(regCX2Init);
	DECL_REG	(regCX3Init);

	ADD			(regCX1Init, regCY10, zero);
	ADD			(regCX2Init, regCY20, zero);
	ADD			(regCX3Init, regCY30, zero);

	DECL_CONST_REG	(initIX0, 0);
	DECL_CONST_REG	(initRowMask, 0);
	DECL_CONST_REG	(initDone, 0);

	//	PixelMask rowMask = 0;
	DECL_REG	(regRowMask0);		// beginning of x loop
	DECL_REG	(regRowMask1);		// end of x loop

	DECL_REG	(regInitInnerDepth);

	ADD			(regInitInnerDepth, regDepth0, zero);

    //    for (I32 ix = 0; ix < EGL_RASTER_BLOCK_SIZE; ix++) {

	cg_block_ref_t * xLoopTop = cg_block_ref_create(procedure);
	cg_block_ref_t * xLoopEnd = cg_block_ref_create(procedure);
	block = cg_block_create(procedure, 4);
	xLoopTop->block = block;

	DECL_REG	(regCX10);			// begin x loop
	DECL_REG	(regCX11);			// end x loop
	DECL_REG	(regCX20);			// begin x loop
	DECL_REG	(regCX21);			// end x loop
	DECL_REG	(regCX30);			// begin x loop
	DECL_REG	(regCX31);			// end x loop
	
	DECL_REG	(regIX0);			// beginning of x loop
	DECL_REG	(regIX1);			// end of x loop

	DECL_REG	(regDone0);			// beginning of x loop
	DECL_REG	(regDone1);			// end of x loop

	PHI			(regCX10, cg_create_virtual_reg_list(procedure->module->heap, regCX11, regCX1Init, NULL));
	PHI			(regCX20, cg_create_virtual_reg_list(procedure->module->heap, regCX21, regCX2Init, NULL));
	PHI			(regCX30, cg_create_virtual_reg_list(procedure->module->heap, regCX31, regCX3Init, NULL));
	PHI			(regIX0, cg_create_virtual_reg_list(procedure->module->heap, regIX1, initIX0, NULL));
	PHI			(regRowMask0, cg_create_virtual_reg_list(procedure->module->heap, regRowMask1, initRowMask, NULL));
	PHI			(regDone0, cg_create_virtual_reg_list(procedure->module->heap, regDone1, initDone, NULL));

	// I32 depth1 = depth0;
	PHI			(regDepth3, cg_create_virtual_reg_list(procedure->module->heap, regDepth2, regInitInnerDepth, NULL));

	DECL_REG	(regRowMask2);		// shifted mask
	DECL_REG	(regRowMask3);		// modified mask if written
	DECL_REG	(regRowMask4);		// if shifted during abortion of loop

	// rowMask >>= 1
	LSR			(regRowMask2, regRowMask0, one);

    //        if (CX1 > 0 && CX2 > 0 && CX3 > 0) {
	cg_block_ref_t * notWritten = cg_block_ref_create(procedure);
	cg_block_ref_t * notInside = cg_block_ref_create(procedure);

	DECL_FLAGS	(regCmp1);
	DECL_FLAGS	(regCmp2);
	DECL_FLAGS	(regCmp3);

	CMP			(regCmp1, regCX10, zero);
	BLE			(regCmp1, notInside);
	CMP			(regCmp2, regCX20, zero);
	BLE			(regCmp2, notInside);
	CMP			(regCmp3, regCX30, zero);
	BLE			(regCmp3, notInside);

	//		done = true;
	LDI			(regDone1, 1);

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

	//		bool written = FragmentDepthStencil(&m_RasterInfo, &surfaceInfo, ix, depth1 >> 4);
	DECL_REG(regShiftedDepth);

	LSR		(regShiftedDepth, regDepth3, four);

	info.regX = regIX0;
	info.regDepth = regShiftedDepth;

	GenerateFragmentDepthStencil(procedure, block, notWritten, 
		info, 4, regDepthBuffer0, false, true);

	//		rowMask |= (written ? (1 << (EGL_RASTER_BLOCK_SIZE - 1)) : 0);
	block = cg_block_create(procedure, 4);

	DECL_CONST_REG	(shiftedOne, (1 << (EGL_RASTER_BLOCK_SIZE - 1)));

	OR			(regRowMask3, regRowMask2, shiftedOne);
	BRA			(notWritten);

	// interior test not passed
	block = cg_block_create(procedure, 4);
	notInside->block = block;

#if 1
    //        } else if (done) {
	//			rowMask >>= EGL_RASTER_BLOCK_SIZE - ix - 1;
	//			break;
	DECL_FLAGS	(flagsDone);
	DECL_REG	(regDummy1);
	OR_S		(regDummy1, flagsDone, regDone0, regDone0);
	BEQ			(flagsDone, notWritten);

	block = cg_block_create(procedure, 4);
	DECL_CONST_REG	(blockSizeMinus1, EGL_RASTER_BLOCK_SIZE - 1);
	DECL_REG	(regToShift);
	DECL_FLAGS	(zeroShift);

	SUB_S		(regToShift, zeroShift, blockSizeMinus1, regIX0);
	BEQ			(zeroShift, xLoopEnd);
	LSR			(regRowMask4, regRowMask2, regToShift);
	BRA			(xLoopEnd);

	// stencil test passed
	block = cg_block_create(procedure, 4);

#endif
	notWritten->block = block;

	PHI			(regRowMask1, cg_create_virtual_reg_list(procedure->module->heap, regRowMask2, regRowMask3, regRowMask4, NULL));

    //      CX1 += edges->edge12.FDY; 
	//		CX2 += edges->edge23.FDY; 
	//		CX3 += edges->edge31.FDY;

	cg_virtual_reg_t * regCX1FDY = LOAD_DATA(block, regEdges, OFFSET_EDGES_EDGE_12 + OFFSET_EDGE_FDY);

	//		depth1 += vars->Depth.dX;
	ADD			(regDepth2, regDepth3, regDepthDx);

	cg_virtual_reg_t * regCX2FDY = LOAD_DATA(block, regEdges, OFFSET_EDGES_EDGE_23 + OFFSET_EDGE_FDY);

	ADD			(regCX11, regCX10, regCX1FDY);

	cg_virtual_reg_t * regCX3FDY = LOAD_DATA(block, regEdges, OFFSET_EDGES_EDGE_31 + OFFSET_EDGE_FDY);

	ADD			(regCX21, regCX20, regCX2FDY);
	ADD			(regCX31, regCX30, regCX3FDY);

    //    }

	DECL_FLAGS	(regReachedXLoopEnd);
	ADD			(regIX1, regIX0, one);
	CMP			(regReachedXLoopEnd, regIX1, blockSize);
	BNE			(regReachedXLoopEnd, xLoopTop);

	block = cg_block_create(procedure, 2);
	xLoopEnd->block = block;

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

    //  CY1 -= edges->edge12.FDX; 
	//	CY2 -= edges->edge23.FDX; 
	//	CY3 -= edges->edge31.FDX;

	cg_virtual_reg_t *regDepthDy = LOAD_DATA(block, regVars, OFFSET_VARIABLES_DEPTH + OFFSET_INTERPOLANT_DY);
	cg_virtual_reg_t * regCY1FDX = LOAD_DATA(block, regEdges, OFFSET_EDGES_EDGE_12 + OFFSET_EDGE_FDX);

	//	depth0 += vars->Depth.dY;
	ADD			(regDepth1, regDepth0, regDepthDy);

	cg_virtual_reg_t * regCY2FDX = LOAD_DATA(block, regEdges, OFFSET_EDGES_EDGE_23 + OFFSET_EDGE_FDX);

	SUB			(regCY11, regCY10, regCY1FDX);

	cg_virtual_reg_t * regCY3FDX = LOAD_DATA(block, regEdges, OFFSET_EDGES_EDGE_31 + OFFSET_EDGE_FDX);

	SUB			(regCY21, regCY20, regCY2FDX);
	SUB			(regCY31, regCY30, regCY3FDX);

	//	surfaceInfo.DepthBuffer += surfaceInfo.Pitch;
	U32				shift = 0;

	switch (m_State->GetDepthStencilFormat()) {
	case DepthStencilFormatDepth16:				shift = 1;		break;
	case DepthStencilFormatDepth16Stencil16:	shift = 2;		break;

	default:					
		assert(false);	break;
	}

	DECL_CONST_REG	(regDepthStencilIncrement, EGL_RASTER_BLOCK_SIZE << shift);
	ADD			(regDepthBuffer1, regDepthBuffer0, regDepthStencilIncrement);

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

bool RasterTriangleEdgeDepthStencilPart :: CompareState(const void * first, const void * second) const {
	const RasterizerState * firstState = static_cast<const RasterizerState *>(first);
	const RasterizerState * secondState = static_cast<const RasterizerState *>(second);
	
	return firstState->ComparePolygonDepthStencil(*secondState);
}

void RasterTriangleEdgeDepthStencilPart :: Compile(FunctionCache * target, const VaryingInfo * varyingInfo, const void * state) {
	m_State = static_cast<const RasterizerState *>(state);
	BeginGenerateCode();
	GenerateRasterBlockEdgeDepthStencil(varyingInfo);
	EndGenerateCode(target, state);
}

PipelinePart::Part RasterTriangleEdgeDepthStencilPart :: GetPart() const {
	return PipelinePart::PartRasterBlockEdgeDepthStencil;
}
