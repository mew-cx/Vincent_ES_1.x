// ==========================================================================
//
// RasterTriangleColorAlphaPart.h	
//						JIT Class for 3D Rendering Library
//
//						This class encapsulates the specifics for managing
//						code generation of color/alpha processing for
//						triangle rasterization
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
#include "RasterTriangleColorAlphaPart.h"
#include "inline.h"


using namespace EGL;


void RasterTriangleColorAlphaPart :: GenerateRasterBlockColorAlpha(const VaryingInfo * varyingInfo) {
	cg_proc_t * procedure = cg_proc_create(m_Module);

	FragmentGenerationInfo info;
	memset(&info, 0, sizeof(info));

	//typedef void (BlockColorAlphaFunction)(const RasterInfo * info, I32 varying[][2][2], const PixelMask * pixelMask);

	DECL_REG	(regRasterInfo);// RasterInfo structure pointer
	DECL_REG	(regVarying);	// Varying array pointer
	DECL_REG	(regPixelMask);	// pixel mask data pointer

	procedure->num_args = 3;	// the previous three declarations make up the arguments

	cg_block_t * block = cg_block_create(procedure, 1);

	//const PixelMask * mask = pixelMask;

	DECL_REG	(regMask0);			// beginning of y loop
	DECL_REG	(regMask1);			// end of y loop

	DECL_CONST_REG	(zero, 0);		// 0
	DECL_CONST_REG	(one, 1);		// 1
	DECL_CONST_REG	(two, 2);		// 2
	DECL_CONST_REG	(four, 4);		// 4
	DECL_CONST_REG	(eight, 8);		// 8
	DECL_CONST_REG	(twelve, 12);	// 12
	DECL_CONST_REG	(sixteen, 16);	// 16
	DECL_CONST_REG	(blockSize, EGL_RASTER_BLOCK_SIZE);
	DECL_CONST_REG	(logBlockSize, EGL_LOG_RASTER_BLOCK_SIZE);
	DECL_CONST_REG	(numVarying, varyingInfo->numVarying);

	DECL_CONST_REG	(maskSize, sizeof(PixelMask));

	// initialize surface pointers in local info block
	//SurfaceInfo surfaceInfo = m_RasterInfo.SurfaceInfo;

	DECL_REG	(regColorBuffer0);	// begin y loop
	DECL_REG	(regColorBuffer1);	// end y loop

	cg_virtual_reg_t * regColorBuffer = LOAD_DATA(block, regRasterInfo, OFFSET_SURFACE_COLOR_BUFFER);
	cg_virtual_reg_t * regPitch = LOAD_DATA(block, regRasterInfo, OFFSET_SURFACE_PITCH);
	
	cg_virtual_reg_t * regTexture = LOAD_DATA(block, regRasterInfo, OFFSET_TEXTURES);

	size_t unit;

	info.regInfo = regRasterInfo;
	info.regTexture[0] = regTexture;

	for (unit = 1; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
		info.regTexture[unit] =  LOAD_DATA(block, regRasterInfo, OFFSET_TEXTURES + unit * sizeof(void *));
	}

    //for (I32 iy = 0; iy < EGL_RASTER_BLOCK_SIZE; iy++) {

	DECL_REG	(regIY0);				// begin y loop
	DECL_REG	(regIY1);				// end y loop

	DECL_CONST_REG(initIY1, EGL_RASTER_BLOCK_SIZE);

	cg_block_ref_t * yLoopTop = cg_block_ref_create(procedure);
	cg_block_ref_t * yLoopEnd = cg_block_ref_create(procedure);
	cg_block_ref_t * yLoopIncr = cg_block_ref_create(procedure);

	// stencil test passed
	block = cg_block_create(procedure, 2);
	yLoopTop->block = block;

	PHI			(regIY0, cg_create_virtual_reg_list(procedure->module->heap, regIY1, initIY1, NULL));
	PHI			(regMask0, cg_create_virtual_reg_list(procedure->module->heap, regPixelMask, regMask1, NULL));
	PHI			(regColorBuffer0, cg_create_virtual_reg_list(procedure->module->heap, regColorBuffer, regColorBuffer1, NULL));

	size_t index;

	//	I32 varying0[EGL_MAX_NUM_VARYING][2];
	cg_virtual_reg_t * regVaryingInc[EGL_MAX_NUM_VARYING];
	cg_virtual_reg_t * regVaryingInit[EGL_MAX_NUM_VARYING];

	//	PixelMask rowMask = *mask++;
	DECL_REG	(regRowMask);

#if EGL_RASTER_BLOCK_SIZE <= 8
	LDB			(regRowMask, regMask0);
#elif EGL_RASTER_BLOCK_SIZE <= 16
	LDH			(regRowMask, regMask0);
#else
	LDW			(regRowMask, regMask0);
#endif

	ADD			(regMask1, regMask0, maskSize);

	DECL_FLAGS	(skipEndLoop);

	CMP		(skipEndLoop, regRowMask, zero);
	BEQ		(skipEndLoop, yLoopIncr);

	for (index = 0; index < varyingInfo->numVarying; ++index) {
		//	varying0[index][0] = varying[index][0][0];
		regVaryingInit[index] = LOAD_DATA(block, regVarying, index * 16);

		//	varying0[index][1] = (varying[index][1][0] - varying[index][0][0]) >> EGL_LOG_RASTER_BLOCK_SIZE;
		cg_virtual_reg_t * regLimit = LOAD_DATA(block, regVarying, index * 16 + 8);

		DECL_REG	(regDiff);
		DECL_REG	(regShifted);

		SUB			(regDiff, regLimit, regVaryingInit[index]);
		ASR			(regShifted, regDiff, logBlockSize);

		regVaryingInc[index] = regShifted;
	}

	cg_virtual_reg_t * regVarying0[EGL_MAX_NUM_VARYING];		// begin of x loop
	cg_virtual_reg_t * regVarying1[EGL_MAX_NUM_VARYING];		// end of x loop

	DECL_REG		(regIX0);									// begin of x loop
	DECL_REG		(regIX1);									// begin of x loop
	DECL_REG		(regRowMask0);								// begin of x loop
	DECL_REG		(regRowMask1);								// begin of x loop

	DECL_CONST_REG	(initIX0, 0);

    //    for (I32 ix = 0; ix < EGL_RASTER_BLOCK_SIZE; ix++) {
	cg_block_ref_t * xLoopTop = cg_block_ref_create(procedure);
	cg_block_ref_t * xLoopEnd = cg_block_ref_create(procedure);
	block = cg_block_create(procedure, 4);
	xLoopTop->block = block;

	for (index = 0; index < varyingInfo->numVarying; ++index) {
		regVarying0[index] = cg_virtual_reg_create(block->proc, cg_reg_type_general);
		regVarying1[index] = cg_virtual_reg_create(block->proc, cg_reg_type_general);

		PHI		(regVarying0[index], cg_create_virtual_reg_list(procedure->module->heap, regVaryingInit[index], regVarying1[index], NULL));
	}

	PHI		(regIX0, cg_create_virtual_reg_list(procedure->module->heap, initIX0, regIX1, NULL));
	PHI		(regRowMask0, cg_create_virtual_reg_list(procedure->module->heap, regRowMask, regRowMask1, NULL));

	//		if (rowMask & 1) {	
	cg_block_ref_t * noPixel = cg_block_ref_create(procedure);

	DECL_REG	(regDummy);
	DECL_FLAGS	(regHasPixel);

	AND_S		(regDummy, regHasPixel, regRowMask0, one);
	BEQ			(regHasPixel, noPixel);

	//			I32 tu[EGL_NUM_TEXTURE_UNITS], tv[EGL_NUM_TEXTURE_UNITS];
	//			Color baseColor;
	//			I32 fog;

				info.regX = regIX0;

				for (unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
					size_t textureBase = varyingInfo->textureBase[unit];

					if (textureBase >= 0) {
	//					tu[unit] = varying0[textureBase][0];
						info.regU[unit] = regVarying0[textureBase];
	//					tv[unit] = varying0[textureBase + 1][0];
						info.regV[unit] = regVarying0[textureBase + 1];
					}

				}

				if (varyingInfo->colorIndex >= 0) {
	//				baseColor = FractionalColor(
	//					varying0[m_VaryingInfo.colorIndex][0], 
	//					varying0[m_VaryingInfo.colorIndex + 1][0], 
	//					varying0[m_VaryingInfo.colorIndex + 2][0],
	//					varying0[m_VaryingInfo.colorIndex + 3][0]);
					info.regR = regVarying0[varyingInfo->colorIndex];
					info.regG = regVarying0[varyingInfo->colorIndex + 1];
					info.regB = regVarying0[varyingInfo->colorIndex + 2];
					info.regA = regVarying0[varyingInfo->colorIndex + 3];
				}

				if (varyingInfo->fogIndex >= 0) {
	//				fog = varying0[m_VaryingInfo.fogIndex][0];
					info.regFog = regVarying0[varyingInfo->fogIndex];
				}

	//			FragmentColorAlpha(&m_RasterInfo, &surfaceInfo, ix, tu, tv, baseColor, fog);
				GenerateFragmentColorAlpha(procedure, block, noPixel, 
					info, 4, regColorBuffer0);
	//		}

	block = cg_block_create(procedure, 4);
	noPixel->block = block;

	//		if (!(rowMask >>= 1))
	//			break;
	DECL_FLAGS	(regEndXLoop);

	LSR_S	(regRowMask1, regEndXLoop, regRowMask0, one);
	BEQ		(regEndXLoop, xLoopEnd);

	block = cg_block_create(procedure, 4);

	for (index = 0; index < varyingInfo->numVarying; ++index) {
		//		varying0[index][0] += varying0[index][1];
		ADD		(regVarying1[index], regVarying0[index], regVaryingInc[index]);
	}

	ADD			(regIX1, regIX0, one);

    //    }
	BRA			(xLoopTop);

	block = cg_block_create(procedure, 2);
	xLoopEnd->block = block;
	yLoopIncr->block = block;

	// <---- loop termination goes here
	DECL_FLAGS	(regReachedYLoopEnd);
	SUB_S		(regIY1, regReachedYLoopEnd, regIY0, one);
	BEQ			(regReachedYLoopEnd, yLoopEnd);

	block = cg_block_create(procedure, 2);

	//	for (index = 0; index < m_VaryingInfo.numVarying; ++index) {
	DECL_REG	(regCounter0);
	DECL_REG	(regCounter1);
	DECL_REG	(regPointer0);
	DECL_REG	(regPointer1);

	DECL_CONST_REG	(initCounter0, varyingInfo->numVarying);
	DECL_REG		(initPointer0);

	OR			(initPointer0, regVarying, zero);

	cg_block_ref_t * incrLoopTop = cg_block_ref_create(procedure);
	block = cg_block_create(procedure, 4);
	incrLoopTop->block = block;

	PHI			(regCounter0, cg_create_virtual_reg_list(procedure->module->heap, regCounter1, initCounter0, NULL));
	PHI			(regPointer0, cg_create_virtual_reg_list(procedure->module->heap, regPointer1, initPointer0, NULL));

	//		varying[index][0][0] += varying[index][0][1];
	//		varying[index][1][0] += varying[index][1][1];
	cg_virtual_reg_t * regPtrVal0 = LOAD_DATA(block, regPointer0, 0);
	cg_virtual_reg_t * regPtrInc0 = LOAD_DATA(block, regPointer0, 4);
	cg_virtual_reg_t * regPtrVal1 = LOAD_DATA(block, regPointer0, 8);
	cg_virtual_reg_t * regPtrInc1 = LOAD_DATA(block, regPointer0, 12);
	
	DECL_REG	(regPtrValInc0);
	DECL_REG	(regPtrValInc1);

	ADD			(regPtrValInc0, regPtrVal0, regPtrInc0);	
	STORE_DATA	(block, regPointer0, 0, regPtrValInc0);
	ADD			(regPtrValInc1, regPtrVal1, regPtrInc1);	
	STORE_DATA	(block, regPointer0, 8, regPtrValInc1);

	//	}
	DECL_FLAGS	(regEndIncrLoop);

	ADD			(regPointer1, regPointer0, sixteen);
	SUB_S		(regCounter1, regEndIncrLoop, regCounter0, one);
	BNE			(regEndIncrLoop, incrLoopTop);

	block = cg_block_create(procedure, 1);

	//	surfaceInfo.ColorBuffer += surfaceInfo.Pitch;
	DECL_REG		(regPitch2);
	U32				shift = 0;

	switch (m_State->GetColorFormat()) {
	case ColorFormatRGB565:
	case ColorFormatRGBA5551:
	case ColorFormatRGBA4444:	shift = 1;		break;
	case ColorFormatRGBA8:		shift = 2;		break;
	default:					assert(false);	break;
	}

	DECL_CONST_REG	(constShift, shift);

	LSL			(regPitch2, regPitch, constShift);
	ADD			(regColorBuffer1, regColorBuffer0, regPitch2);

    //}
	BRA			(yLoopTop);

	block = cg_block_create(procedure, 1);
	yLoopEnd->block = block;

	RET();
}

// --------------------------------------------------------------------------
// Implementation of virtual functions in PipelinePart
// --------------------------------------------------------------------------

bool RasterTriangleColorAlphaPart :: CompareState(const void * first, const void * second) const {
	const RasterizerState * firstState = static_cast<const RasterizerState *>(first);
	const RasterizerState * secondState = static_cast<const RasterizerState *>(second);
	
	return firstState->ComparePolygonColorAlpha(*secondState);
}

void RasterTriangleColorAlphaPart :: Compile(FunctionCache * target, const VaryingInfo * varyingInfo, const void * state) {
	m_State = static_cast<const RasterizerState *>(state);
	BeginGenerateCode();
	GenerateRasterBlockColorAlpha(varyingInfo);
	EndGenerateCode(target, state);
}

PipelinePart::Part RasterTriangleColorAlphaPart :: GetPart() const {
	return PipelinePart::PartRasterBlockColorAlpha;
}