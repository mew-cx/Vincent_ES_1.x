// ==========================================================================
//
// CodeGenerator.cpp	JIT Class for 3D Rendering Library
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
#include "Surface.h"
#include "Texture.h"
#include "codegen.h"
#include "instruction.h"
#include "emit.h"
#include "arm-dis.h"

using namespace EGL;


namespace {

	cg_virtual_reg_t * LOAD_DATA(cg_block_t * block, cg_virtual_reg_t * base, I32 constant) {
		cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
		cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);
		cg_virtual_reg_t * value = cg_virtual_reg_create(block->proc, cg_reg_type_general);

		LDI(offset, constant);
		ADD(addr, base, offset);
		LDW(value, addr);

		return value;
	}

#define DECL_REG(reg) cg_virtual_reg_t * reg = cg_virtual_reg_create(procedure, cg_reg_type_general)
#define DECL_FLAGS(reg) cg_virtual_reg_t * reg = cg_virtual_reg_create(procedure, cg_reg_type_flags)

}

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

namespace {
	struct RasterInfo {
		// surface info
		I32		SurfaceWidth;
		I32		SurfaceHeight;
		I32 *	DepthBuffer;
		I32 *	ColorBuffer;
		U32 *	StencilBuffer;
		U8 *	AlphaBuffer;

		// texture info
		I32		TextureWidth;
		I32		TextureHeight;
		I32		TextureExponent;
		void *	TextureData;
	};

	// -------------------------------------------------------------------------
	// Offsets of structure members within info structure
	// -------------------------------------------------------------------------

#	define OFFSET_SURFACE_WIDTH				offsetof(RasterInfo, SurfaceWidth)
#	define OFFSET_SURFACE_HEIGHT			offsetof(RasterInfo, SurfaceHeight)
#	define OFFSET_SURFACE_DEPTH_BUFFER		offsetof(RasterInfo, DepthBuffer)
#	define OFFSET_SURFACE_COLOR_BUFFER		offsetof(RasterInfo, ColorBuffer)
#	define OFFSET_SURFACE_STENCIL_BUFFER	offsetof(RasterInfo, StencilBuffer)
#	define OFFSET_SURFACE_ALPHA_BUFFER		offsetof(RasterInfo, AlphaBuffer)
#	define OFFSET_TEXTURE_WIDTH				offsetof(RasterInfo, TextureWidth)
#	define OFFSET_TEXTURE_HEIGHT			offsetof(RasterInfo, TextureHeight)
#	define OFFSET_TEXTURE_EXPONENT			offsetof(RasterInfo, TextureExponent)
#	define OFFSET_TEXTURE_DATA				offsetof(RasterInfo, TextureData)

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
#	define OFFSET_EDGE_COORD_Z				offsetof(EdgeCoord, z)

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
#	define OFFSET_EDGE_BUFFER_WINDOW_Z		(OFFSET_EDGE_BUFFER_WINDOW + OFFSET_EDGE_COORD_Z)

#	define OFFSET_EDGE_BUFFER_COLOR_R		(OFFSET_EDGE_BUFFER_COLOR + OFFSET_COLOR_RED)
#	define OFFSET_EDGE_BUFFER_COLOR_G		(OFFSET_EDGE_BUFFER_COLOR + OFFSET_COLOR_GREEN)
#	define OFFSET_EDGE_BUFFER_COLOR_B		(OFFSET_EDGE_BUFFER_COLOR + OFFSET_COLOR_BLUE)
#	define OFFSET_EDGE_BUFFER_COLOR_A		(OFFSET_EDGE_BUFFER_COLOR + OFFSET_COLOR_ALPHA)

#	define OFFSET_EDGE_BUFFER_TEX_TU		(OFFSET_EDGE_BUFFER_TEXTURE + OFFSET_TEX_COORD_TU)
#	define OFFSET_EDGE_BUFFER_TEX_TV		(OFFSET_EDGE_BUFFER_TEXTURE + OFFSET_TEX_COORD_TV)
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

		cg_virtual_reg_t * regTextureData;
		cg_virtual_reg_t * regTextureWidth;
		cg_virtual_reg_t * regTextureHeight;
		cg_virtual_reg_t * regTextureExponent;

		// surface color buffer, depth buffer, alpha buffer, stencil buffer
		cg_virtual_reg_t * regColorBuffer;
		cg_virtual_reg_t * regDepthBuffer;
		cg_virtual_reg_t * regAlphaBuffer;
		cg_virtual_reg_t * regStencilBuffer;

		cg_virtual_reg_t * regSurfaceWidth;
	};
}


// Actually, we could extract the scaling of the texture coordinates into the outer driving loop, 
// and have the adjusted clipping range for tu and tv be stored in the rasterizer.

void CodeGenerator :: GenerateFragment(cg_proc_t * procedure,  cg_block_t * currentBlock,
			cg_block_ref_t * continuation, FragmentGenerationInfo & fragmentInfo) {

	cg_block_t * block = currentBlock;

	// Signature of generated function is:
	// (I32 x, I32 y, EGL_Fixed depth, EGL_Fixed tu, EGL_Fixed tv, EGL_Fixed fogDensity, const Color& baseColor);
	
	// fragment level clipping (for now)

	//if (m_Surface->GetWidth() <= x || x < 0 ||
	//	m_Surface->GetHeight() <= y || y < 0) {
	//	return;
	//}

	//bool depthTest;
	//U32 offset = x + y * m_Surface->GetWidth();
	//I32 zBufferValue = m_Surface->GetDepthBuffer()[offset];
	cg_virtual_reg_t * regOffset = fragmentInfo.regX;

	DECL_FLAGS	(regDepthTest);
	DECL_REG	(regScaledY);
	DECL_REG	(regConstant1);
	DECL_REG	(regConstant2);
	DECL_REG	(regOffset4);
	DECL_REG	(regOffset2);
	DECL_REG	(regZBufferAddr);
	DECL_REG	(regZBufferValue);

	LDI		(regConstant1, 1);
	LDI		(regConstant2, 2);
	LSL		(regOffset2, regOffset, regConstant1);
	LSL		(regOffset4, regOffset, regConstant2);
	ADD		(regZBufferAddr, fragmentInfo.regDepthBuffer, regOffset4);
	LDW		(regZBufferValue, regZBufferAddr);
	FCMP	(regDepthTest, regZBufferValue, fragmentInfo.regDepth);

	cg_opcode_t branchOnDepthTestPassed, branchOnDepthTestFailed;

	switch (m_State->m_DepthFunc) {
		default:
		case RasterizerState::CompFuncNever:	
			//depthTest = false;						
			branchOnDepthTestPassed = cg_op_nop;
			branchOnDepthTestFailed = cg_op_bra;
			break;

		case RasterizerState::CompFuncLess:		
			//depthTest = depth < zBufferValue;		
			branchOnDepthTestPassed = cg_op_blt;
			branchOnDepthTestFailed = cg_op_bge;
			break;

		case RasterizerState::CompFuncEqual:	
			//depthTest = depth == zBufferValue;		
			branchOnDepthTestPassed = cg_op_beq;
			branchOnDepthTestFailed = cg_op_bne;
			break;

		case RasterizerState::CompFuncLEqual:	
			//depthTest = depth <= zBufferValue;		
			branchOnDepthTestPassed = cg_op_ble;
			branchOnDepthTestFailed = cg_op_bgt;
			break;

		case RasterizerState::CompFuncGreater:	
			//depthTest = depth > zBufferValue;		
			branchOnDepthTestPassed = cg_op_bgt;
			branchOnDepthTestFailed = cg_op_ble;
			break;

		case RasterizerState::CompFuncNotEqual:	
			//depthTest = depth != zBufferValue;		
			branchOnDepthTestPassed = cg_op_bne;
			branchOnDepthTestFailed = cg_op_beq;
			break;

		case RasterizerState::CompFuncGEqual:	
			//depthTest = depth >= zBufferValue;		
			branchOnDepthTestPassed = cg_op_bge;
			branchOnDepthTestFailed = cg_op_blt;
			break;

		case RasterizerState::CompFuncAlways:	
			//depthTest = true;						
			branchOnDepthTestPassed = cg_op_bra;
			branchOnDepthTestFailed = cg_op_nop;
			break;
	}

	if (!m_State->m_StencilTestEnabled && m_State->m_DepthTestEnabled) {
		//if (!depthTest)
		//	return;

		if (branchOnDepthTestFailed == cg_op_nop) {
			// nothing
		} else if (branchOnDepthTestFailed == cg_op_bra) {
			BRA		(continuation);
		} else {
			cg_create_inst_branch_cond(block, branchOnDepthTestFailed, regDepthTest, continuation CG_INST_DEBUG_ARGS);
		}
	}

	//Color color = baseColor;
	cg_virtual_reg_t * regColorR;
	cg_virtual_reg_t * regColorG;
	cg_virtual_reg_t * regColorB;
	cg_virtual_reg_t * regColorA;
	cg_virtual_reg_t * regColor565;

	if (m_State->m_TextureEnabled) {

		//EGL_Fixed tu0;
		//EGL_Fixed tv0;
		DECL_REG	(regU0);
		DECL_REG	(regV0);

		switch (m_Texture->GetWrappingModeS()) {
			case MultiTexture::WrappingModeClampToEdge:
				//tu0 = EGL_CLAMP(tu, 0, EGL_ONE);
				{
					DECL_REG	(regConstantOne);
					DECL_REG	(regConstantZero);
					DECL_FLAGS	(regCompareOne);
					DECL_FLAGS	(regCompareZero);
					DECL_REG	(regNewU1);
					DECL_REG	(regNewU2);

					cg_block_ref_t * label1 = cg_block_ref_create(procedure);
					cg_block_ref_t * label2 = cg_block_ref_create(procedure);

					LDI		(regConstantOne, EGL_FixedFromInt(1));
					FCMP	(regCompareOne, fragmentInfo.regU, regConstantOne);
					BLE		(regCompareOne, label1);
					LDI		(regNewU1, EGL_FixedFromInt(1));
					BRA		(label2);

					block = cg_block_create(procedure);
					label1->block = block;

					LDI		(regConstantZero, EGL_FixedFromInt(0));
					FCMP	(regCompareZero, fragmentInfo.regU, regConstantZero);
					BGE		(regCompareZero, label2);
					LDI		(regNewU2, EGL_FixedFromInt(0));

					block = cg_block_create(procedure);
					label2->block = block;

					cg_virtual_reg_list_t * regList = 
						cg_create_virtual_reg_list(procedure->module->heap,
												   fragmentInfo.regU, regNewU1, regNewU2, NULL);

					PHI		(regU0, regList);
				}
				break;

			default:
			case MultiTexture::WrappingModeRepeat:
				//tu0 = tu & 0xffff;
				{
					DECL_REG(regMask);

					LDI		(regMask, 0xffff);
					AND		(regU0, fragmentInfo.regU, regMask);
				}
				break;
		}

		switch (m_Texture->GetWrappingModeT()) {
			case MultiTexture::WrappingModeClampToEdge:
				//tv0 = EGL_CLAMP(tv, 0, EGL_ONE);
				{
					DECL_REG	(regConstantOne);
					DECL_REG	(regConstantZero);
					DECL_FLAGS	(regCompareOne);
					DECL_FLAGS	(regCompareZero);
					DECL_REG	(regNewV1);
					DECL_REG	(regNewV2);

					cg_block_ref_t * label1 = cg_block_ref_create(procedure);
					cg_block_ref_t * label2 = cg_block_ref_create(procedure);

					LDI		(regConstantOne, EGL_FixedFromInt(1));
					FCMP	(regCompareOne, fragmentInfo.regV, regConstantOne);
					BLE		(regCompareOne, label1);
					LDI		(regNewV1, EGL_FixedFromInt(1));
					BRA		(label2);

					block = cg_block_create(procedure);
					label1->block = block;

					LDI		(regConstantZero, EGL_FixedFromInt(0));
					FCMP	(regCompareZero, fragmentInfo.regV, regConstantZero);
					BGE		(regCompareZero, label2);
					LDI		(regNewV2, EGL_FixedFromInt(0));

					block = cg_block_create(procedure);
					label2->block = block;

					PHI		(regV0, cg_create_virtual_reg_list(procedure->module->heap, fragmentInfo.regV, regNewV1, regNewV2, NULL));
				}
				break;

			default:
			case MultiTexture::WrappingModeRepeat:
				//tv0 = tv & 0xffff;
				{
					DECL_REG	(regMask);

					LDI		(regMask, 0xffff);
					AND		(regV0, fragmentInfo.regV, regMask);
				}
				break;
		}

		// get the pixel color
		//Texture * texture = m_Texture->GetTexture(m_MipMapLevel);
		//Color texColor; 
		cg_virtual_reg_t * regTexColorR;			
		cg_virtual_reg_t * regTexColorG;			
		cg_virtual_reg_t * regTexColorB;			
		cg_virtual_reg_t * regTexColorA;
		cg_virtual_reg_t * regTexColor565;

		//cg_virtual_reg_t * texX = EGL_IntFromFixed(texture->GetWidth() * tu0);
		//cg_virtual_reg_t * texY = EGL_IntFromFixed(texture->GetHeight() * tv0);
		//cg_virtual_reg_t * texOffset = texX + (texY << texture->GetExponent());
		//void * data = texture->GetData();
		DECL_REG	(regScaledU);
		DECL_REG	(regTexX);
		DECL_REG	(regScaledV);
		DECL_REG	(regTexY);
		DECL_REG	(regScaledTexY);
		DECL_REG	(regTexOffset);

		MUL		(regScaledU, regU0, fragmentInfo.regTextureWidth);
		TRUNC	(regTexX, regScaledU);
		MUL		(regScaledV, regV0, fragmentInfo.regTextureHeight);
		TRUNC	(regTexY, regScaledV);
		LSL		(regScaledTexY, regTexY, fragmentInfo.regTextureExponent);
		ADD		(regTexOffset, regTexX, regScaledTexY);

		switch (m_Texture->GetInternalFormat()) {
			case Texture::TextureFormatAlpha:				// 8
				{
				//texColor = Color(0xff, 0xff, 0xff, reinterpret_cast<const U8 *>(data)[texOffset]);
				regTexColorR = regTexColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

				DECL_REG	(regTexAddr);
				DECL_REG	(regConstant7);
				DECL_REG	(regShifted7);
				DECL_REG	(regTexData);
				

				ADD		(regTexAddr, regTexOffset, fragmentInfo.regTextureData);
				LDB		(regTexData, regTexAddr);
				LDI		(regConstant7, 7);
				LSR		(regShifted7, regTexData, regConstant7);
				ADD		(regTexColorA, regTexData, regShifted7);
				LDI		(regTexColorR, 0x1f);
				LDI		(regTexColorG, 0x3f);
				LDI		(regTexColor565, 0xffff);

				}
				break;

			case Texture::TextureFormatLuminance:			// 8
				{
				//U8 luminance = reinterpret_cast<const U8 *>(data)[texOffset];
				//texColor = Color (luminance, luminance, luminance, 0xff);
				regTexColorR = regTexColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

				DECL_REG	(regTexAddr);
				DECL_REG	(regTexData);
				DECL_REG	(regMask5);
				DECL_REG	(regMask6);
				DECL_REG	(regConstant2);
				DECL_REG	(regConstant3);
				DECL_REG	(regConstant5);
				DECL_REG	(regConstant11);
				DECL_REG	(regColor5);
				DECL_REG	(regColor6);
				DECL_REG	(regShiftedB);
				DECL_REG	(regShiftedG);
				DECL_REG	(regRG);

				ADD		(regTexAddr, regTexOffset, fragmentInfo.regTextureData);
				LDB		(regTexData, regTexAddr);
				LDI		(regTexColorA, 0x100);
				LDI		(regMask5, 0x1f);
				LDI		(regMask6, 0x3f);
				LDI		(regConstant2, 2);
				LDI		(regConstant3, 3);
				LSL		(regColor5, regTexData, regConstant3);
				AND		(regTexColorR, regColor5, regMask5);
				LSL		(regColor6, regTexData, regConstant2);
				AND		(regTexColorG, regColor6, regMask6);
				LDI		(regConstant5, 5);
				LDI		(regConstant11, 11);
				LSL		(regShiftedB, regTexColorB, regConstant11);
				LSL		(regShiftedG, regTexColorG, regConstant5);
				OR		(regRG, regTexColorR, regShiftedG);
				OR		(regTexColor565, regRG, regShiftedB);
				}
				break;

			case Texture::TextureFormatLuminanceAlpha:		// 8-8
				{
				//U8 luminance = reinterpret_cast<const U8 *>(data)[texOffset * 2];
				//U8 alpha = reinterpret_cast<const U8 *>(data)[texOffset * 2 + 1];
				//texColor = Color (luminance, luminance, luminance, alpha);
				regTexColorR = regTexColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

				DECL_REG	(regTexAddr);
				DECL_REG	(regTexData);
				DECL_REG	(regMask5);
				DECL_REG	(regMask6);
				DECL_REG	(regConstant2);
				DECL_REG	(regConstant3);
				DECL_REG	(regConstant5);
				DECL_REG	(regConstant11);
				DECL_REG	(regColor5);
				DECL_REG	(regColor6);
				DECL_REG	(regShiftedB);
				DECL_REG	(regShiftedG);
				DECL_REG	(regRG);
				DECL_REG	(regScaledOffset);
				DECL_REG	(regConstantOne);
				DECL_REG	(regMask);
				DECL_REG	(regConstant8);
				DECL_REG	(regAlpha);
				DECL_REG	(regMaskedAlphaByte);
				DECL_REG	(regConstant7);
				DECL_REG	(regShifted7);

				LDI		(regConstantOne, 1);
				LSL		(regScaledOffset, regTexOffset, regConstantOne);
				ADD		(regTexAddr, regScaledOffset, fragmentInfo.regTextureData);
				LDH		(regTexData, regTexAddr);
				LDI		(regMask, 0xff);
				LDI		(regConstant8, 8);
				LSR		(regAlpha, regTexData, regConstant8);
				AND		(regMaskedAlphaByte, regAlpha, regMask);
				LDI		(regConstant7, 7);
				LSR		(regShifted7, regMaskedAlphaByte, regConstant7);
				ADD		(regTexColorA, regMaskedAlphaByte, regShifted7);
				LDI		(regMask5, 0x1f);
				LDI		(regMask6, 0x3f);
				LDI		(regConstant2, 2);
				LDI		(regConstant3, 3);
				LSL		(regColor5, regTexData, regConstant3);
				AND		(regTexColorR, regColor5, regMask5);
				LSL		(regColor6, regTexData, regConstant2);
				AND		(regTexColorG, regColor6, regMask6);
				LDI		(regConstant5, 5);
				LDI		(regConstant11, 11);
				LSL		(regShiftedB, regTexColorB, regConstant11);
				LSL		(regShiftedG, regTexColorG, regConstant5);
				OR		(regRG, regTexColorR, regShiftedG);
				OR		(regTexColor565, regRG, regShiftedB);

				}
				break;

			case Texture::TextureFormatRGB:					// 5-6-5
				//texColor = Color::From565(reinterpret_cast<const U16 *>(data)[texOffset]);
				{
				regTexColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

				DECL_REG	(regScaledOffset);
				DECL_REG	(regConstantOne);
				DECL_REG	(regTexAddr);
				DECL_REG	(regMask5);
				DECL_REG	(regMask6);
				DECL_REG	(regConstant5);
				DECL_REG	(regShifted5);
				DECL_REG	(regConstant11);
				DECL_REG	(regShifted11);

				LDI		(regConstantOne, 1);
				LSL		(regScaledOffset, regTexOffset, regConstantOne);
				ADD		(regTexAddr, regScaledOffset, fragmentInfo.regTextureData);
				LDH		(regTexColor565, regTexAddr);
				LDI		(regTexColorA, 0x100);
				LDI		(regConstant5, 5);
				LDI		(regConstant11, 11);
				LSR		(regShifted5, regTexColor565, regConstant5);
				LSR		(regShifted11, regTexColor565, regConstant11);
				LDI		(regMask5, 0x1f);
				LDI		(regMask6, 0x3f);
				AND		(regTexColorR, regTexColor565, regMask5);
				AND		(regTexColorG, regShifted5, regMask6);
				AND		(regTexColorB, regShifted11, regMask5);

				}
				break;

			case Texture::TextureFormatRGBA:					// 5-5-5-1
				//texColor = Color::From5551(reinterpret_cast<const U16 *>(data)[texOffset]);
				{
				regTexColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);

				DECL_REG	(regTexData);
				DECL_REG	(regScaledOffset);
				DECL_REG	(regConstantOne);
				DECL_REG	(regTexAddr);

				LDI		(regConstantOne, 1);
				LSL		(regScaledOffset, regTexOffset, regConstantOne);
				ADD		(regTexAddr, regScaledOffset, fragmentInfo.regTextureData);
				LDH		(regTexData, regTexAddr);

				DECL_REG	(regConstant7);
				DECL_REG	(regShifted7);
				DECL_REG	(regMask100);

				LDI		(regConstant7, 7);
				LDI		(regMask100, 0x100);
				LSR		(regShifted7, regTexData, regConstant7);
				AND		(regTexColorA, regShifted7, regMask100);

				DECL_REG	(regMask5);
				DECL_REG	(regMask51);
				DECL_REG	(regConstant4);
				DECL_REG	(regConstant10);
				DECL_REG	(regShifted4);
				DECL_REG	(regShifted10);

				LDI		(regMask5, 0x1f);
				AND		(regTexColorR, regTexData, regMask5);
				LDI		(regMask51, 0x3e);
				LDI		(regConstant4, 4);
				LDI		(regConstant10, 10);
				LSR		(regShifted4, regTexData, regConstant4);
				AND		(regTexColorG, regShifted4, regMask51);
				LSR		(regShifted10, regTexData, regConstant10);
				AND		(regTexColorB, regShifted10, regMask5);

				regTexColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

				DECL_REG	(regConstant5);
				DECL_REG	(regConstant11);
				DECL_REG	(regShiftedB);
				DECL_REG	(regShiftedG);
				DECL_REG	(regRG);

				LDI		(regConstant5, 5);
				LDI		(regConstant11, 11);
				LSL		(regShiftedB, regTexColorB, regConstant11);
				LSL		(regShiftedG, regTexColorG, regConstant5);
				OR		(regRG, regTexColorR, regShiftedG);
				OR		(regTexColor565, regRG, regShiftedB);
				}
				break;

			default:
				//texColor = Color(0xff, 0xff, 0xff, 0x100);
				{
				regTexColorR = regTexColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

				LDI		(regTexColorR, 0x1f);
				LDI		(regTexColorG, 0x3f);
				LDI		(regTexColorA, 0x100);
				LDI		(regTexColor565, 0xffff);
				}
				break;
		}

		switch (m_Texture->GetInternalFormat()) {
			default:
			case Texture::TextureFormatAlpha:
				switch (m_State->m_TextureMode) {
					case RasterizerState::TextureModeReplace:
						{
						//color = Color(color.r, color.g, color.b, texColor.a);
						regColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

						DECL_REG	(regConstant31);
						DECL_REG	(regConstant63);
						DECL_REG	(regConstant16);
						DECL_REG	(regScaledR);
						DECL_REG	(regScaledG);
						DECL_REG	(regScaledB);

						LDI		(regConstant31, 0x1f);
						LDI		(regConstant63, 0x3f);
						LDI		(regConstant16, 0x10);

						MUL		(regScaledR, fragmentInfo.regR, regConstant31);
						LSR		(regColorR, regScaledR, regConstant16);
						MUL		(regScaledG, fragmentInfo.regG, regConstant63);
						LSR		(regColorG, regScaledG, regConstant16);
						MUL		(regScaledB, fragmentInfo.regB, regConstant31);
						LSR		(regColorB, regScaledB, regConstant16);

						DECL_REG	(regConstant5);
						DECL_REG	(regConstant11);
						DECL_REG	(regShiftedB);
						DECL_REG	(regShiftedG);
						DECL_REG	(regRG);

						LDI		(regConstant5, 5);
						LDI		(regConstant11, 11);
						LSL		(regShiftedB, regTexColorB, regConstant11);
						LSL		(regShiftedG, regTexColorG, regConstant5);
						OR		(regRG, regTexColorR, regShiftedG);
						OR		(regColor565, regRG, regShiftedB);

						regColorA = regTexColorA;
						}

						break;

					case RasterizerState::TextureModeModulate:
					case RasterizerState::TextureModeBlend:
					case RasterizerState::TextureModeAdd:
						{
						//color = Color(color.r, color.g, color.b, MulU8(color.a, texColor.a));
						regColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);

						DECL_REG	(regConstant31);
						DECL_REG	(regConstant63);
						DECL_REG	(regConstant16);
						DECL_REG	(regScaledR);
						DECL_REG	(regScaledG);
						DECL_REG	(regScaledB);

						LDI		(regConstant31, 0x1f);
						LDI		(regConstant63, 0x3f);
						LDI		(regConstant16, 0x10);

						MUL		(regScaledR, fragmentInfo.regR, regConstant31);
						LSR		(regColorR, regScaledR, regConstant16);
						MUL		(regScaledG, fragmentInfo.regG, regConstant63);
						LSR		(regColorG, regScaledG, regConstant16);
						MUL		(regScaledB, fragmentInfo.regB, regConstant31);
						LSR		(regColorB, regScaledB, regConstant16);

						DECL_REG	(regConstant5);
						DECL_REG	(regConstant11);
						DECL_REG	(regShiftedB);
						DECL_REG	(regShiftedG);
						DECL_REG	(regRG);

						LDI		(regConstant5, 5);
						LDI		(regConstant11, 11);
						LSL		(regShiftedB, regTexColorB, regConstant11);
						LSL		(regShiftedG, regTexColorG, regConstant5);
						OR		(regRG, regTexColorR, regShiftedG);
						OR		(regColor565, regRG, regShiftedB);

						DECL_REG	(regAlphaProduct);

						MUL		(regAlphaProduct, fragmentInfo.regA, regTexColorA);
						LSR		(regColorA, regAlphaProduct, regConstant16);
						}

						break;
				}
				break;

			case Texture::TextureFormatLuminance:
			case Texture::TextureFormatRGB:
				switch (m_State->m_TextureMode) {
					case RasterizerState::TextureModeDecal:
					case RasterizerState::TextureModeReplace:
						{
						//color = Color(texColor.r, texColor.g, texColor.b, color.a);
						regColorR = regTexColorR;
						regColorG = regTexColorG;
						regColorB = regTexColorB;
						regColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColor565 = regTexColor565;

						DECL_REG	(regConstant8);

						LDI		(regConstant8, 8);
						LSR		(regColorA, fragmentInfo.regA, regConstant8);
						}

						break;

					case RasterizerState::TextureModeModulate:
						{
						//color = Color(MulU8(color.r, texColor.r), 
						//	MulU8(color.g, texColor.g), MulU8(color.b, texColor.b), color.a);
						regColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

						DECL_REG	(regConstant8);
						DECL_REG	(regConstant16);
						DECL_REG	(regScaledR);
						DECL_REG	(regScaledG);
						DECL_REG	(regScaledB);

						LDI		(regConstant8, 8);
						LSR		(regColorA, fragmentInfo.regA, regConstant8);

						LDI		(regConstant16, 16);
						MUL		(regScaledR, regTexColorR, fragmentInfo.regR);
						LSR		(regColorR, regScaledR, regConstant16);
						MUL		(regScaledG, regTexColorG, fragmentInfo.regG);
						LSR		(regColorG, regScaledG, regConstant16);
						MUL		(regScaledB, regTexColorB, fragmentInfo.regB);
						LSR		(regColorB, regScaledB, regConstant16);

						DECL_REG	(regConstant5);
						DECL_REG	(regConstant11);
						DECL_REG	(regShiftedB);
						DECL_REG	(regShiftedG);
						DECL_REG	(regRG);

						LDI		(regConstant5, 5);
						LDI		(regConstant11, 11);
						LSL		(regShiftedB, regColorB, regConstant11);
						LSL		(regShiftedG, regColorG, regConstant5);
						OR		(regRG, regColorR, regShiftedG);
						OR		(regColor565, regRG, regShiftedB);
						}

						break;

					case RasterizerState::TextureModeBlend:
						{
						//color = 
						//	Color(
						//		MulU8(color.r, 0xff - texColor.r) + MulU8(m_State->m_TexEnvColor.r, texColor.r),
						//		MulU8(color.g, 0xff - texColor.g) + MulU8(m_State->m_TexEnvColor.g, texColor.g),
						//		MulU8(color.b, 0xff - texColor.b) + MulU8(m_State->m_TexEnvColor.b, texColor.b),
						//		color.a);
						regColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);

						DECL_REG	(regConstant2);
						DECL_REG	(regConstant3);
						DECL_REG	(regConstant8);
						DECL_REG	(regConstant15);

						LSR		(regColorA, fragmentInfo.regA, regConstant8);
						LDI		(regConstant2, 2);
						LDI		(regConstant3, 3);
						LDI		(regConstant8, 8);
						LDI		(regConstant15, 15);

						// red component
						{
							regColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regShifted);
							DECL_REG	(regAdjusted);
							DECL_REG	(regColorAdjusted);

							LSR		(regShifted, fragmentInfo.regR, regConstant15);
							SUB		(regAdjusted, fragmentInfo.regR, regShifted);
							LSR		(regColorAdjusted, regAdjusted, regConstant3);

							DECL_REG	(regShiftedAdjusted);
							DECL_REG	(regDiff);
							DECL_REG	(regTexEnv);
							DECL_REG	(regProduct);
							DECL_REG	(regDifference);

							LSR		(regShiftedAdjusted, regAdjusted, regConstant8);
							LDI		(regTexEnv, m_State->m_TexEnvColor.r);
							SUB		(regDiff, regShiftedAdjusted, regTexEnv);
							MUL		(regProduct, regDiff, regTexColorR);
							SUB		(regDifference, regColorAdjusted, regProduct);
							LSR		(regColorR, regDifference, regConstant8);
						}

						// green component
						{
							regColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regShifted);
							DECL_REG	(regAdjusted);
							DECL_REG	(regColorAdjusted);

							LSR		(regShifted, fragmentInfo.regG, regConstant15);
							SUB		(regAdjusted, fragmentInfo.regG, regShifted);
							LSR		(regColorAdjusted, regAdjusted, regConstant2);

							DECL_REG	(regShiftedAdjusted);
							DECL_REG	(regDiff);
							DECL_REG	(regTexEnv);
							DECL_REG	(regProduct);
							DECL_REG	(regDifference);

							LSR		(regShiftedAdjusted, regAdjusted, regConstant8);
							LDI		(regTexEnv, m_State->m_TexEnvColor.g);
							SUB		(regDiff, regShiftedAdjusted, regTexEnv);
							MUL		(regProduct, regDiff, regTexColorG);
							SUB		(regDifference, regColorAdjusted, regProduct);
							LSR		(regColorG, regDifference, regConstant8);
						}

						// blue component
						{
							regColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regShifted);
							DECL_REG	(regAdjusted);
							DECL_REG	(regColorAdjusted);

							LSR		(regShifted, fragmentInfo.regB, regConstant15);
							SUB		(regAdjusted, fragmentInfo.regB, regShifted);
							LSR		(regColorAdjusted, regAdjusted, regConstant3);

							DECL_REG	(regShiftedAdjusted);
							DECL_REG	(regDiff);
							DECL_REG	(regTexEnv);
							DECL_REG	(regProduct);
							DECL_REG	(regDifference);

							LSR		(regShiftedAdjusted, regAdjusted, regConstant8);
							LDI		(regTexEnv, m_State->m_TexEnvColor.b);
							SUB		(regDiff, regShiftedAdjusted, regTexEnv);
							MUL		(regProduct, regDiff, regTexColorB);
							SUB		(regDifference, regColorAdjusted, regProduct);
							LSR		(regColorB, regDifference, regConstant8);
						}

						// create RGB 565 representation
						{
							regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regConstant5);
							DECL_REG	(regConstant11);
							DECL_REG	(regShiftedB);
							DECL_REG	(regShiftedG);
							DECL_REG	(regRG);

							LDI		(regConstant5, 5);
							LDI		(regConstant11, 11);
							LSL		(regShiftedB, regColorB, regConstant11);
							LSL		(regShiftedG, regColorG, regConstant5);
							OR		(regRG, regColorR, regShiftedG);
							OR		(regColor565, regRG, regShiftedB);
						}
						}

						break;

					case RasterizerState::TextureModeAdd:
						{
						//color =
						//	Color(
						//		ClampU8(color.r + texColor.r),
						//		ClampU8(color.g + texColor.g),
						//		ClampU8(color.b + texColor.b),
						//		color.a);
						regColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);

						DECL_REG	(regConstant10);
						DECL_REG	(regConstant11);
						DECL_REG	(regConstant8);

						LDI		(regConstant10, 10);
						LDI		(regConstant11, 11);
						LDI		(regConstant8, 8);
						LSR		(regColorA, fragmentInfo.regA, regConstant8);

						// R channel
						{
							regColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regScaledR);
							DECL_REG	(regShiftedR);
							DECL_REG	(regSumR);
							DECL_FLAGS	(regNeedsClampingR);
							DECL_REG	(regConstant255);

							LDI		(regConstant255, 0x1f);
							LSR		(regShiftedR, fragmentInfo.regR, regConstant11);
							ADD		(regSumR, regShiftedR, regTexColorR);
							CMP		(regNeedsClampingR, regSumR, regConstant255);

							cg_block_ref_t * noClampingR = cg_block_ref_create(procedure);
							DECL_REG	(regClampedR);

							BLE		(regNeedsClampingR, noClampingR);
							LDI		(regClampedR, 0x1f);

							block = cg_block_create(procedure);
							noClampingR->block = block;

							PHI		(regColorR, cg_create_virtual_reg_list(procedure->module->heap, regClampedR, regSumR, NULL));
						}
						// G channel
						{
							regColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regScaledG);
							DECL_REG	(regShiftedG);
							DECL_REG	(regSumG);
							DECL_FLAGS	(regNeedsClampingG);
							DECL_REG	(regConstant255);

							LDI		(regConstant255, 0x3f);
							LSR		(regShiftedG, fragmentInfo.regG, regConstant10);
							ADD		(regSumG, regShiftedG, regTexColorR);
							CMP		(regNeedsClampingG, regSumG, regConstant255);

							cg_block_ref_t * noClampingG = cg_block_ref_create(procedure);
							DECL_REG	(regClampedG);

							BLE		(regNeedsClampingG, noClampingG);
							LDI		(regClampedG, 0x3f);

							block = cg_block_create(procedure);
							noClampingG->block = block;

							PHI		(regColorG, cg_create_virtual_reg_list(procedure->module->heap, regClampedG, regSumG, NULL));
						}
						// B channel
						{
							regColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regScaledB);
							DECL_REG	(regShiftedB);
							DECL_REG	(regSumB);
							DECL_FLAGS	(regNeedsClampingB);
							DECL_REG	(regConstant255);

							LDI		(regConstant255, 0x1f);
							LSR		(regShiftedB, fragmentInfo.regB, regConstant10);
							ADD		(regSumB, regShiftedB, regTexColorR);
							CMP		(regNeedsClampingB, regSumB, regConstant255);

							cg_block_ref_t * noClampingB = cg_block_ref_create(procedure);
							DECL_REG	(regClampedB);

							BLE		(regNeedsClampingB, noClampingB);
							LDI		(regClampedB, 0x1f);

							block = cg_block_create(procedure);
							noClampingB->block = block;

							PHI		(regColorB, cg_create_virtual_reg_list(procedure->module->heap, regClampedB, regSumB, NULL));
						}
						// create RGB 565 representation
						{
							regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regConstant5);
							DECL_REG	(regConstant11);
							DECL_REG	(regShiftedB);
							DECL_REG	(regShiftedG);
							DECL_REG	(regRG);

							LDI		(regConstant5, 5);
							LDI		(regConstant11, 11);
							LSL		(regShiftedB, regColorB, regConstant11);
							LSL		(regShiftedG, regColorG, regConstant5);
							OR		(regRG, regColorR, regShiftedG);
							OR		(regColor565, regRG, regShiftedB);
						}
						}

						break;
				}
				break;

			case Texture::TextureFormatLuminanceAlpha:
			case Texture::TextureFormatRGBA:
				switch (m_State->m_TextureMode) {
					case RasterizerState::TextureModeReplace:
						{
						//color = texColor;
						regColorR = regTexColorR;
						regColorG = regTexColorG;
						regColorB = regTexColorB;
						regColorA = regTexColorA;
						regColor565 = regTexColor565;
						}

						break;

					case RasterizerState::TextureModeModulate:
						{
						//color = color * texColor;
						regColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

						DECL_REG	(regConstant8);
						DECL_REG	(regConstant16);
						DECL_REG	(regScaledR);
						DECL_REG	(regScaledG);
						DECL_REG	(regScaledB);
						DECL_REG	(regAlphaProduct);

						LDI		(regConstant16, 16);
						MUL		(regAlphaProduct, fragmentInfo.regA, regTexColorA);
						LSR		(regColorA, regAlphaProduct, regConstant16);

						MUL		(regScaledR, regTexColorR, fragmentInfo.regR);
						LSR		(regColorR, regScaledR, regConstant16);
						MUL		(regScaledG, regTexColorG, fragmentInfo.regG);
						LSR		(regColorG, regScaledG, regConstant16);
						MUL		(regScaledB, regTexColorB, fragmentInfo.regB);
						LSR		(regColorB, regScaledB, regConstant16);

						DECL_REG	(regConstant5);
						DECL_REG	(regConstant11);
						DECL_REG	(regShiftedB);
						DECL_REG	(regShiftedG);
						DECL_REG	(regRG);

						LDI		(regConstant5, 5);
						LDI		(regConstant11, 11);
						LSL		(regShiftedB, regColorB, regConstant11);
						LSL		(regShiftedG, regColorG, regConstant5);
						OR		(regRG, regColorR, regShiftedG);
						OR		(regColor565, regRG, regShiftedB);

						}

						break;

					case RasterizerState::TextureModeDecal:
						{
						//color = 
						//	Color(
						//		MulU8(color.r, 0xff - texColor.a) + MulU8(texColor.r, texColor.a),
						//		MulU8(color.g, 0xff - texColor.a) + MulU8(texColor.g, texColor.a),
						//		MulU8(color.b, 0xff - texColor.a) + MulU8(texColor.b, texColor.a),
						//		color.a);

						regColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);

						DECL_REG	(regConstant2);
						DECL_REG	(regConstant3);
						DECL_REG	(regConstant8);
						DECL_REG	(regConstant10);
						DECL_REG	(regConstant11);
						DECL_REG	(regConstant15);
						DECL_REG	(regShiftedA);

						LDI		(regConstant2, 2);
						LDI		(regConstant3, 3);
						LDI		(regConstant8, 8);
						LDI		(regConstant10, 10);
						LDI		(regConstant11, 11);
						LDI		(regConstant15, 15);
						LSR		(regShiftedA, fragmentInfo.regA, regConstant15);
						SUB		(regColorA, fragmentInfo.regA, regShiftedA);

						// red component
						{
							regColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regShifted);
							DECL_REG	(regAdjusted);
							DECL_REG	(regToColor);
							DECL_REG	(regDeltaColor);
							DECL_REG	(regProduct);
							DECL_REG	(regColorShift);
							DECL_REG	(regDelta);

							LSR		(regShifted, fragmentInfo.regR, regConstant15);
							SUB		(regAdjusted, fragmentInfo.regR, regShifted);
							LSR		(regToColor, regAdjusted, regConstant11);
							SUB		(regDeltaColor, regTexColorR, regToColor);
							MUL		(regProduct, regDeltaColor, regTexColorA);	
							LSR		(regColorShift, regAdjusted, regConstant3);
							ADD		(regDelta, regColorShift, regProduct);
							ASR		(regColorR, regDelta, regConstant8);
						}

						// green component
						{
							regColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regShifted);
							DECL_REG	(regAdjusted);
							DECL_REG	(regToColor);
							DECL_REG	(regDeltaColor);
							DECL_REG	(regProduct);
							DECL_REG	(regColorShift);
							DECL_REG	(regDelta);

							LSR		(regShifted, fragmentInfo.regG, regConstant15);
							SUB		(regAdjusted, fragmentInfo.regG, regShifted);
							LSR		(regToColor, regAdjusted, regConstant10);
							SUB		(regDeltaColor, regTexColorG, regToColor);
							MUL		(regProduct, regDeltaColor, regTexColorA);	
							LSR		(regColorShift, regAdjusted, regConstant2);
							ADD		(regDelta, regColorShift, regProduct);
							ASR		(regColorG, regDelta, regConstant8);
						}

						// blue component
						{
							regColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regShifted);
							DECL_REG	(regAdjusted);
							DECL_REG	(regToColor);
							DECL_REG	(regDeltaColor);
							DECL_REG	(regProduct);
							DECL_REG	(regColorShift);
							DECL_REG	(regDelta);

							LSR		(regShifted, fragmentInfo.regB, regConstant15);
							SUB		(regAdjusted, fragmentInfo.regB, regShifted);
							LSR		(regToColor, regAdjusted, regConstant11);
							SUB		(regDeltaColor, regTexColorB, regToColor);
							MUL		(regProduct, regDeltaColor, regTexColorA);	
							LSR		(regColorShift, regAdjusted, regConstant3);
							ADD		(regDelta, regColorShift, regProduct);
							ASR		(regColorB, regDelta, regConstant8);
						}

						// create RGB 565 representation
						{
							regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regConstant5);
							DECL_REG	(regConstant11);
							DECL_REG	(regShiftedB);
							DECL_REG	(regShiftedG);
							DECL_REG	(regRG);

							LDI		(regConstant5, 5);
							LDI		(regConstant11, 11);
							LSL		(regShiftedB, regColorB, regConstant11);
							LSL		(regShiftedG, regColorG, regConstant5);
							OR		(regRG, regColorR, regShiftedG);
							OR		(regColor565, regRG, regShiftedB);
						}
						}

						break;

					case RasterizerState::TextureModeBlend:
						{
						//color = 
						//	Color(
						//		MulU8(color.r, 0xff - texColor.r) + MulU8(m_State->m_TexEnvColor.r, texColor.r),
						//		MulU8(color.g, 0xff - texColor.g) + MulU8(m_State->m_TexEnvColor.g, texColor.g),
						//		MulU8(color.b, 0xff - texColor.b) + MulU8(m_State->m_TexEnvColor.b, texColor.b),
						//		MulU8(color.a, texColor.a));
						regColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);

						DECL_REG	(regConstant2);
						DECL_REG	(regConstant3);
						DECL_REG	(regConstant8);
						DECL_REG	(regConstant15);
						DECL_REG	(regConstant16);
						DECL_REG	(regAlphaProduct);

						LDI		(regConstant2, 2);
						LDI		(regConstant3, 3);
						LDI		(regConstant8, 8);
						LDI		(regConstant16, 16);
						MUL		(regAlphaProduct, fragmentInfo.regA, regTexColorA);
						LSR		(regColorA, regAlphaProduct, regConstant16);
						LDI		(regConstant15, 15);

						// red component
						{
							regColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regShifted);
							DECL_REG	(regAdjusted);
							DECL_REG	(regColorAdjusted);

							LSR		(regShifted, fragmentInfo.regR, regConstant15);
							SUB		(regAdjusted, fragmentInfo.regR, regShifted);
							LSR		(regColorAdjusted, regAdjusted, regConstant3);

							DECL_REG	(regShiftedAdjusted);
							DECL_REG	(regDiff);
							DECL_REG	(regTexEnv);
							DECL_REG	(regProduct);
							DECL_REG	(regDifference);

							LSR		(regShiftedAdjusted, regAdjusted, regConstant8);
							LDI		(regTexEnv, m_State->m_TexEnvColor.r);
							SUB		(regDiff, regShiftedAdjusted, regTexEnv);
							MUL		(regProduct, regDiff, regTexColorR);
							SUB		(regDifference, regColorAdjusted, regProduct);
							LSR		(regColorR, regDifference, regConstant8);
						}

						// green component
						{
							regColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regShifted);
							DECL_REG	(regAdjusted);
							DECL_REG	(regColorAdjusted);

							LSR		(regShifted, fragmentInfo.regG, regConstant15);
							SUB		(regAdjusted, fragmentInfo.regG, regShifted);
							LSR		(regColorAdjusted, regAdjusted, regConstant2);

							DECL_REG	(regShiftedAdjusted);
							DECL_REG	(regDiff);
							DECL_REG	(regTexEnv);
							DECL_REG	(regProduct);
							DECL_REG	(regDifference);

							LSR		(regShiftedAdjusted, regAdjusted, regConstant8);
							LDI		(regTexEnv, m_State->m_TexEnvColor.g);
							SUB		(regDiff, regShiftedAdjusted, regTexEnv);
							MUL		(regProduct, regDiff, regTexColorG);
							SUB		(regDifference, regColorAdjusted, regProduct);
							LSR		(regColorG, regDifference, regConstant8);
						}

						// blue component
						{
							regColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regShifted);
							DECL_REG	(regAdjusted);
							DECL_REG	(regColorAdjusted);

							LSR		(regShifted, fragmentInfo.regB, regConstant15);
							SUB		(regAdjusted, fragmentInfo.regB, regShifted);
							LSR		(regColorAdjusted, regAdjusted, regConstant3);

							DECL_REG	(regShiftedAdjusted);
							DECL_REG	(regDiff);
							DECL_REG	(regTexEnv);
							DECL_REG	(regProduct);
							DECL_REG	(regDifference);

							LSR		(regShiftedAdjusted, regAdjusted, regConstant8);
							LDI		(regTexEnv, m_State->m_TexEnvColor.b);
							SUB		(regDiff, regShiftedAdjusted, regTexEnv);
							MUL		(regProduct, regDiff, regTexColorB);
							SUB		(regDifference, regColorAdjusted, regProduct);
							LSR		(regColorB, regDifference, regConstant8);
						}

						// create RGB 565 representation
						{
							regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regConstant5);
							DECL_REG	(regConstant11);
							DECL_REG	(regShiftedB);
							DECL_REG	(regShiftedG);
							DECL_REG	(regRG);

							LDI		(regConstant5, 5);
							LDI		(regConstant11, 11);
							LSL		(regShiftedB, regColorB, regConstant11);
							LSL		(regShiftedG, regColorG, regConstant5);
							OR		(regRG, regColorR, regShiftedG);
							OR		(regColor565, regRG, regShiftedB);
						}
						}

						break;

					case RasterizerState::TextureModeAdd:
						{
						//color =
						//	Color(
						//		ClampU8(color.r + texColor.r),
						//		ClampU8(color.g + texColor.g),
						//		ClampU8(color.b + texColor.b),
						//		MulU8(color.a, texColor.a));
						regColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);

						DECL_REG	(regConstant10);
						DECL_REG	(regConstant11);
						DECL_REG	(regConstant16);
						DECL_REG	(regConstant8);
						DECL_REG	(regAlphaProduct);

						LDI		(regConstant10, 10);
						LDI		(regConstant11, 11);
						LDI		(regConstant8, 8);

						LDI		(regConstant16, 16);
						MUL		(regAlphaProduct, fragmentInfo.regA, regTexColorA);
						LSR		(regColorA, regAlphaProduct, regConstant16);

						// R channel
						{
							regColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regConstant255);
							DECL_REG	(regScaledR);
							DECL_REG	(regShiftedR);
							DECL_REG	(regSumR);
							DECL_FLAGS	(regNeedsClampingR);

							LDI		(regConstant255, 0x1f);
							LSR		(regShiftedR, fragmentInfo.regR, regConstant11);
							ADD		(regSumR, regShiftedR, regTexColorR);
							CMP		(regNeedsClampingR, regSumR, regConstant255);

							cg_block_ref_t * noClampingR = cg_block_ref_create(procedure);
							DECL_REG	(regClampedR);

							BLE		(regNeedsClampingR, noClampingR);
							LDI		(regClampedR, 0x1f);

							block = cg_block_create(procedure);
							noClampingR->block = block;

							PHI		(regColorR, cg_create_virtual_reg_list(procedure->module->heap, regClampedR, regSumR, NULL));
						}
						// G channel
						{
							regColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regConstant255);
							DECL_REG	(regScaledG);
							DECL_REG	(regShiftedG);
							DECL_REG	(regSumG);
							DECL_FLAGS	(regNeedsClampingG);

							LDI		(regConstant255, 0x3f);
							LSR		(regShiftedG, fragmentInfo.regG, regConstant10);
							ADD		(regSumG, regShiftedG, regTexColorR);
							CMP		(regNeedsClampingG, regSumG, regConstant255);

							cg_block_ref_t * noClampingG = cg_block_ref_create(procedure);
							DECL_REG	(regClampedG);

							BLE		(regNeedsClampingG, noClampingG);
							LDI		(regClampedG, 0x3f);

							block = cg_block_create(procedure);
							noClampingG->block = block;

							PHI		(regColorG, cg_create_virtual_reg_list(procedure->module->heap, regClampedG, regSumG, NULL));
						}
						// B channel
						{
							regColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regConstant255);
							DECL_REG	(regScaledB);
							DECL_REG	(regShiftedB);
							DECL_REG	(regSumB);
							DECL_FLAGS	(regNeedsClampingB);

							LDI		(regConstant255, 0x1f);
							LSR		(regShiftedB, fragmentInfo.regB, regConstant10);
							ADD		(regSumB, regShiftedB, regTexColorR);
							CMP		(regNeedsClampingB, regSumB, regConstant255);

							cg_block_ref_t * noClampingB = cg_block_ref_create(procedure);
							DECL_REG	(regClampedB);

							BLE		(regNeedsClampingB, noClampingB);
							LDI		(regClampedB, 0x1f);

							block = cg_block_create(procedure);
							noClampingB->block = block;

							PHI		(regColorB, cg_create_virtual_reg_list(procedure->module->heap, regClampedB, regSumB, NULL));
						}
						// create RGB 565 representation
						{
							regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regConstant5);
							DECL_REG	(regConstant11);
							DECL_REG	(regShiftedB);
							DECL_REG	(regShiftedG);
							DECL_REG	(regRG);

							LDI		(regConstant5, 5);
							LDI		(regConstant11, 11);
							LSL		(regShiftedB, regColorB, regConstant11);
							LSL		(regShiftedG, regColorG, regConstant5);
							OR		(regRG, regColorR, regShiftedG);
							OR		(regColor565, regRG, regShiftedB);
						}
						}

						break;
				}
				break;
		}
	} else {
		// color = baseColor
		DECL_REG	(regConstant16);
		DECL_REG	(regConstant11);
		DECL_REG	(regConstant10);
		DECL_REG	(regConstant8);

		LDI		(regConstant16, 16);
		LDI		(regConstant11, 11);
		LDI		(regConstant10, 10);
		LDI		(regConstant8, 8);

		//  needs to be converted to 565 format, alpha to 8 bits
		{
			regColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);

			DECL_REG	(regShifted);
			DECL_REG	(regAdjusted);

			LSR		(regShifted, fragmentInfo.regR, regConstant16);
			SUB		(regAdjusted, fragmentInfo.regR, regShifted);
			LSR		(regColorR, regAdjusted, regConstant11);
		}

		{
			regColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);

			DECL_REG	(regShifted);
			DECL_REG	(regAdjusted);

			LSR		(regShifted, fragmentInfo.regG, regConstant16);
			SUB		(regAdjusted, fragmentInfo.regG, regShifted);
			LSR		(regColorG, regAdjusted, regConstant10);
		}

		{
			regColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);

			DECL_REG	(regShifted);
			DECL_REG	(regAdjusted);

			LSR		(regShifted, fragmentInfo.regB, regConstant16);
			SUB		(regAdjusted, fragmentInfo.regB, regShifted);
			LSR		(regColorB, regAdjusted, regConstant11);
		}
		{
			regColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);

			LSR		(regColorA, fragmentInfo.regA, regConstant8);
		}
		{
			// create RGB 565 representation
			regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

			DECL_REG	(regConstant5);
			DECL_REG	(regShiftedB);
			DECL_REG	(regShiftedG);
			DECL_REG	(regRG);

			LDI		(regConstant5, 5);
			LDI		(regConstant11, 11);
			LSL		(regShiftedB, regColorB, regConstant11);
			LSL		(regShiftedG, regColorG, regConstant5);
			OR		(regRG, regColorR, regShiftedG);
			OR		(regColor565, regRG, regShiftedB);
		}
	}

	// fog
	if (m_State->m_FogEnabled) {
		//color = Color::Blend(color, m_State->m_FogColor, fogDensity);
		DECL_REG	(regFogColorR);
		DECL_REG	(regFogColorG);
		DECL_REG	(regFogColorB);

		LDI		(regFogColorR, m_State->m_FogColor.r >> 3);
		LDI		(regFogColorG, m_State->m_FogColor.g >> 2);
		LDI		(regFogColorB, m_State->m_FogColor.b >> 3);

		DECL_REG	(regDeltaR);
		DECL_REG	(regDeltaG);
		DECL_REG	(regDeltaB);

		SUB		(regDeltaR, regColorR, regFogColorR);
		SUB		(regDeltaG, regColorG, regFogColorG);
		SUB		(regDeltaB, regColorB, regFogColorB);

		DECL_REG	(regProductR);
		DECL_REG	(regProductG);
		DECL_REG	(regProductB);

		MUL		(regProductR, regDeltaR, fragmentInfo.regFog);
		MUL		(regProductG, regDeltaG, fragmentInfo.regFog);
		MUL		(regProductB, regDeltaB, fragmentInfo.regFog);

		DECL_REG	(regConstant16);
		DECL_REG	(regShiftedProductR);
		DECL_REG	(regShiftedProductG);
		DECL_REG	(regShiftedProductB);

		LDI		(regConstant16, 16);
		ASR		(regShiftedProductR, regProductR, regConstant16);
		ASR		(regShiftedProductG, regProductG, regConstant16);
		ASR		(regShiftedProductB, regProductB, regConstant16);

		DECL_REG	(regNewColorR);
		DECL_REG	(regNewColorG);
		DECL_REG	(regNewColorB);

		ADD		(regNewColorR, regShiftedProductR, regFogColorR);
		ADD		(regNewColorG, regShiftedProductG, regFogColorG);
		ADD		(regNewColorB, regShiftedProductB, regFogColorB);

		regColorR = regNewColorR;
		regColorG = regNewColorG;
		regColorB = regNewColorB;

		// create RGB 565 representation
		DECL_REG	(regConstant5);
		DECL_REG	(regConstant11);
		DECL_REG	(regShiftedB);
		DECL_REG	(regShiftedG);
		DECL_REG	(regRG);
		DECL_REG	(regNewColor565);

		LDI		(regConstant5, 5);
		LDI		(regConstant11, 11);
		LSL		(regShiftedB, regColorB, regConstant11);
		LSL		(regShiftedG, regColorG, regConstant5);
		OR		(regRG, regColorR, regShiftedG);
		OR		(regColor565, regRG, regShiftedB);

		regColor565 = regNewColor565;
	}

	if (m_State->m_AlphaTestEnabled) {
		//bool alphaTest;
		//U8 alpha = color.A();
		//U8 alphaRef = EGL_IntFromFixed(m_State->m_AlphaReference * 255);
		DECL_REG	(regAlphaRef);
		DECL_FLAGS	(regAlphaTest);

		LDI		(regAlphaRef, m_State->m_AlphaReference >> 8);
		CMP		(regAlphaTest, regColorA, regAlphaRef);

		cg_opcode_t failedTest;
		
		switch (m_State->m_AlphaFunc) {
			default:
			case RasterizerState::CompFuncNever:	
				//alphaTest = false;					
				failedTest = cg_op_bra;
				break;

			case RasterizerState::CompFuncLess:		
				//alphaTest = alpha < alphaRef;		
				failedTest = cg_op_bge;
				break;

			case RasterizerState::CompFuncEqual:	
				//alphaTest = alpha == alphaRef;		
				failedTest = cg_op_bne;
				break;

			case RasterizerState::CompFuncLEqual:	
				//alphaTest = alpha <= alphaRef;		
				failedTest = cg_op_bgt;
				break;

			case RasterizerState::CompFuncGreater:	
				//alphaTest = alpha > alphaRef;		
				failedTest = cg_op_ble;
				break;

			case RasterizerState::CompFuncNotEqual:	
				//alphaTest = alpha != alphaRef;		
				failedTest = cg_op_beq;
				break;

			case RasterizerState::CompFuncGEqual:	
				//alphaTest = alpha >= alphaRef;		
				failedTest = cg_op_blt;
				break;

			case RasterizerState::CompFuncAlways:	
				//alphaTest = true;					
				failedTest = cg_op_nop;
				break;
		}

		//if (!alphaTest) {
		//	return;
		//}
		if (failedTest != cg_op_nop) {
			if (failedTest == cg_op_bra) {
				BRA		(continuation);
			} else {
				cg_create_inst_branch_cond(block, failedTest, regAlphaTest, continuation CG_INST_DEBUG_ARGS);
			}
		}
	}

	if (m_State->m_StencilTestEnabled) {

		//bool stencilTest;
		//U32 stencilRef = m_State->m_StencilReference & m_State->m_StencilMask;
		//U32 stencilValue = m_Surface->GetStencilBuffer()[offset];
		//U32 stencil = stencilValue & m_State->m_StencilMask;
		DECL_REG	(regStencilRef);
		DECL_REG	(regStencilMask);
		DECL_REG	(regStencilAddr);
		DECL_REG	(regStencilValue);
		DECL_REG	(regStencil);
		DECL_FLAGS	(regStencilTest);

		LDI		(regStencilRef, m_State->m_StencilReference & m_State->m_StencilMask);
		LDI		(regStencilMask, m_State->m_StencilMask);
		ADD		(regStencilAddr, fragmentInfo.regStencilBuffer, regOffset4);
		LDW		(regStencilValue, regStencilAddr);
		AND		(regStencil, regStencilValue, regStencilMask);
		CMP		(regStencilTest, regStencil, regStencilRef);

		cg_opcode_t passedTest;

		switch (m_State->m_StencilFunc) {
			default:
			case RasterizerState::CompFuncNever:	
				//stencilTest = false;				
				passedTest = cg_op_nop;
				break;

			case RasterizerState::CompFuncLess:		
				//stencilTest = stencil < stencilRef;	
				passedTest = cg_op_blt;
				break;

			case RasterizerState::CompFuncEqual:	
				//stencilTest = stencil == stencilRef;
				passedTest = cg_op_beq;
				break;

			case RasterizerState::CompFuncLEqual:	
				//stencilTest = stencil <= stencilRef;
				passedTest = cg_op_ble;
				break;

			case RasterizerState::CompFuncGreater:	
				//stencilTest = stencil > stencilRef;	
				passedTest = cg_op_bgt;
				break;

			case RasterizerState::CompFuncNotEqual:	
				//stencilTest = stencil != stencilRef;
				passedTest = cg_op_bne;
				break;

			case RasterizerState::CompFuncGEqual:	
				//stencilTest = stencil >= stencilRef;
				passedTest = cg_op_bge;
				break;

			case RasterizerState::CompFuncAlways:	
				//stencilTest = true;					
				passedTest = cg_op_bra;
				break;
		}

		// branch on stencil test
		cg_block_ref_t * labelStencilPassed = cg_block_ref_create(procedure);
		cg_block_ref_t * labelStencilBypassed = cg_block_ref_create(procedure);

		cg_create_inst_branch_cond(block, passedTest,	regStencilTest, labelStencilPassed CG_INST_DEBUG_ARGS);

		//if (!stencilTest) {
		{
			cg_virtual_reg_t * regNewStencilValue;

			switch (m_State->m_StencilFail) {
				default:
				case RasterizerState::StencilOpKeep: 
					regNewStencilValue = regStencilValue;
					break;

				case RasterizerState::StencilOpZero: 
					//stencilValue = 0; 
					regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);

					LDI		(regNewStencilValue, 0);
					break;

				case RasterizerState::StencilOpReplace: 
					//stencilValue = m_State->m_StencilReference; 
					regNewStencilValue = regStencilRef;
					break;

				case RasterizerState::StencilOpIncr: 
					//if (stencilValue != 0xffffffff) {
					//	stencilValue++; 
					//}
					{
						regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);

						DECL_REG	(regConstant1);
						DECL_FLAGS	(regFlag);

						LDI		(regConstant1, 1);
						ADD_S	(regNewStencilValue, regFlag, regStencilValue, regConstant1);
						BEQ		(regFlag, continuation);
					}
					
					break;

				case RasterizerState::StencilOpDecr: 
					//if (stencilValue != 0) {
					//	stencilValue--; 
					//}
					{
						regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);

						DECL_REG	(regConstant0);
						DECL_REG	(regConstant1);
						DECL_FLAGS	(regFlag);

						LDI		(regConstant0, 0);
						CMP		(regFlag, regStencilValue, regConstant0);
						BEQ		(regFlag, continuation);
						LDI		(regConstant1, 1);
						SUB		(regNewStencilValue, regStencilValue, regConstant1);
					}
					
					break;

				case RasterizerState::StencilOpInvert: 
					//stencilValue = ~stencilValue; 
					regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);

					NOT		(regNewStencilValue, regStencilValue);
					break;
			}

			STW		(regNewStencilValue, regStencilAddr);
			BRA		(continuation);
		//}
		}

		cg_block_ref_t * labelStencilZTestPassed = cg_block_ref_create(procedure);

		// stencil test passed
		block = cg_block_create(procedure);
		labelStencilPassed->block = block;

		//if (!depthTest) {
			if (branchOnDepthTestPassed == cg_op_nop) {
				// nothing
			} else if (branchOnDepthTestPassed == cg_op_bra) {
				BRA		(labelStencilZTestPassed);
			} else {
				cg_create_inst_branch_cond(currentBlock, branchOnDepthTestPassed, regDepthTest, labelStencilZTestPassed CG_INST_DEBUG_ARGS);
			}

			{
				cg_virtual_reg_t * regNewStencilValue;

				switch (m_State->m_StencilZFail) {
					default:
					case RasterizerState::StencilOpKeep: 
						regNewStencilValue = regStencilValue;
						break;

					case RasterizerState::StencilOpZero: 
						//stencilValue = 0; 
						regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);

						LDI		(regNewStencilValue, 0);
						break;

					case RasterizerState::StencilOpReplace: 
						//stencilValue = m_State->m_StencilReference; 
						regNewStencilValue = regStencilRef;
						break;

					case RasterizerState::StencilOpIncr: 
						//if (stencilValue != 0xffffffff) {
						//	stencilValue++; 
						//}
						{
							regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regConstant1);
							DECL_FLAGS	(regFlag);

							LDI		(regConstant1, 1);
							ADD_S	(regNewStencilValue, regFlag, regStencilValue, regConstant1);

							if (m_State->m_DepthTestEnabled) {
								BEQ		(regFlag, continuation);
							} else {
								BEQ		(regFlag, labelStencilBypassed);
							}
						}
						
						break;

					case RasterizerState::StencilOpDecr: 
						//if (stencilValue != 0) {
						//	stencilValue--; 
						//}
						{
							regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regConstant0);
							DECL_REG	(regConstant1);
							DECL_FLAGS	(regFlag);

							LDI		(regConstant0, 0);
							CMP		(regFlag, regStencilValue, regConstant0);

							if (m_State->m_DepthTestEnabled) {
								BEQ		(regFlag, continuation);
							} else {
								BEQ		(regFlag, labelStencilBypassed);
							}

							LDI		(regConstant1, 1);
							SUB		(regNewStencilValue, regStencilValue, regConstant1);
						}
						
						break;

					case RasterizerState::StencilOpInvert: 
						//stencilValue = ~stencilValue; 
						regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);

						NOT		(regNewStencilValue, regStencilValue);
						break;
				}

				//m_Surface->GetStencilBuffer()[offset] = stencilValue;
				STW		(regNewStencilValue, regStencilAddr);
			//}
			}

			if (m_State->m_DepthTestEnabled) {
				// return;
				BRA		(continuation);
			}
		//} else {
		// stencil nad z-test passed
		block = cg_block_create(procedure);
		labelStencilZTestPassed->block = block;

			{
				cg_virtual_reg_t * regNewStencilValue;

				switch (m_State->m_StencilZPass) {
					default:
					case RasterizerState::StencilOpKeep: 
						regNewStencilValue = regStencilValue;
						break;

					case RasterizerState::StencilOpZero: 
						//stencilValue = 0; 
						regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);

						LDI		(regNewStencilValue, 0);
						break;

					case RasterizerState::StencilOpReplace: 
						//stencilValue = m_State->m_StencilReference; 
						regNewStencilValue = regStencilRef;
						break;

					case RasterizerState::StencilOpIncr: 
						//if (stencilValue != 0xffffffff) {
						//	stencilValue++; 
						//}
						{
							regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regConstant1);
							DECL_FLAGS	(regFlag);

							LDI		(regConstant1, 1);
							ADD_S	(regNewStencilValue, regFlag, regStencilValue, regConstant1);
							BEQ		(regFlag, labelStencilBypassed);
						}
						
						break;

					case RasterizerState::StencilOpDecr: 
						//if (stencilValue != 0) {
						//	stencilValue--; 
						//}
						{
							regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);

							DECL_REG	(regConstant0);
							DECL_REG	(regConstant1);
							DECL_FLAGS	(regFlag);

							LDI		(regConstant0, 0);
							CMP		(regFlag, regStencilValue, regConstant0);
							BEQ		(regFlag, labelStencilBypassed);
							LDI		(regConstant1, 1);
							SUB		(regNewStencilValue, regStencilValue, regConstant1);
						}
						
						break;

					case RasterizerState::StencilOpInvert: 
						//stencilValue = ~stencilValue; 
						regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);

						NOT		(regNewStencilValue, regStencilValue);
						break;
				}

				//m_Surface->GetStencilBuffer()[offset] = stencilValue;
				STW		(regNewStencilValue, regStencilAddr);
			//}
			}

		// stencil test bypassed
		block = cg_block_create(procedure);
		labelStencilBypassed->block = block;
	}

	//U16 dstValue = m_Surface->GetColorBuffer()[offset];
	//U8 dstAlpha = m_Surface->GetAlphaBuffer()[offset];
	DECL_REG	(regDstValue);
	DECL_REG	(regDstAlpha);
	DECL_REG	(regColorAddr);
	DECL_REG	(regAlphaAddr);
	DECL_REG	(regDstR);
	DECL_REG	(regDstG);
	DECL_REG	(regDstB);

	{
		DECL_REG	(regConstant5);
		DECL_REG	(regConstant11);
		DECL_REG	(regShifted5);
		DECL_REG	(regShifted11);
		DECL_REG	(regMask5);
		DECL_REG	(regMask6);

		ADD		(regColorAddr, fragmentInfo.regColorBuffer, regOffset2);
		ADD		(regAlphaAddr, fragmentInfo.regAlphaBuffer, regOffset);
		LDH		(regDstValue, regColorAddr);
		LDB		(regDstAlpha, regAlphaAddr);
		LDI		(regConstant5, 5);
		LDI		(regConstant11, 11);
		LSR		(regShifted5, regDstValue, regConstant5);
		LSR		(regShifted11, regDstValue, regConstant11);
		LDI		(regMask5, 0x1f);
		LDI		(regMask6, 0x3f);
		AND		(regDstR, regDstValue, regMask5);
		AND		(regDstG, regShifted5, regMask6);
		AND		(regDstB, regShifted11, regMask5);
	}

	// Blending
	if (m_State->m_BlendingEnabled) {

		//Color dstColor = Color::From565A(dstValue, dstAlpha);

		//Color srcCoeff, dstCoeff;

		switch (m_State->m_BlendFuncSrc) {
			default:
			case RasterizerState::BlendFuncSrcZero:
				//srcCoeff = Color(0, 0, 0, 0);
				break;

			case RasterizerState::BlendFuncSrcOne:
				//srcCoeff = Color(Color::MAX, Color::MAX, Color::MAX, Color::MAX);
				break;

			case RasterizerState::BlendFuncSrcSrcColor:
				//srcCoeff = color;
				break;

			case RasterizerState::BlendFuncSrcOneMinusSrcColor:
				//srcCoeff = Color(Color::MAX - color.R(), Color::MAX - color.G(), Color::MAX - color.B(), Color::MAX - color.A());
				break;

			case RasterizerState::BlendFuncSrcSrcAlpha:
				//srcCoeff = Color(color.A(), color.A(), color.A(), color.A());
				break;

			case RasterizerState::BlendFuncSrcOneMinusSrcAlpha:
				//srcCoeff = Color(Color::MAX - color.A(), Color::MAX - color.A(), Color::MAX - color.A(), Color::MAX - color.A());
				break;

			case RasterizerState::BlendFuncSrcDstAlpha:
				//srcCoeff = Color(dstAlpha, dstAlpha, dstAlpha, dstAlpha);
				break;

			case RasterizerState::BlendFuncSrcOneMinusDstAlpha:
				//srcCoeff = Color(Color::MAX - dstAlpha, Color::MAX - dstAlpha, Color::MAX - dstAlpha, Color::MAX - dstAlpha);
				break;
		}

		switch (m_State->m_BlendFuncDst) {
			default:
			case RasterizerState::BlendFuncDstZero:
				//dstCoeff = Color(0, 0, 0, 0);
				break;

			case RasterizerState::BlendFuncDstOne:
				//dstCoeff = Color(Color::MAX, Color::MAX, Color::MAX, Color::MAX);
				break;

			case RasterizerState::BlendFuncDstSrcColor:
				//dstCoeff = color;
				break;

			case RasterizerState::BlendFuncDstOneMinusSrcColor:
				//dstCoeff = Color(Color::MAX - color.R(), Color::MAX - color.G(), Color::MAX - color.B(), Color::MAX - color.A());
				break;

			case RasterizerState::BlendFuncDstSrcAlpha:
				//dstCoeff = Color(color.A(), color.A(), color.A(), color.A());
				break;

			case RasterizerState::BlendFuncDstSrcOneMinusSrcAlpha:
				//dstCoeff = Color(Color::MAX - color.A(), Color::MAX - color.A(), Color::MAX - color.A(), Color::MAX - color.A());
				break;

			case RasterizerState::BlendFuncDstDstAlpha:
				//dstCoeff = Color(dstAlpha, dstAlpha, dstAlpha, dstAlpha);
				break;

			case RasterizerState::BlendFuncDstOneMinusDstAlpha:
				//dstCoeff = Color(Color::MAX - dstAlpha, Color::MAX - dstAlpha, Color::MAX - dstAlpha, Color::MAX - dstAlpha);
				break;

			case RasterizerState::BlendFuncDstSrcAlphaSaturate:
				//{
				//	U8 rev = Color::MAX - dstAlpha;
				//	U8 f = (rev < color.A() ? rev : color.A());
				//	dstCoeff = Color(f, f, f, Color::MAX);
				//}
				break;
		}

		//color = srcCoeff * color + dstCoeff * dstColor;
	}

	// Masking and write to framebuffer
	if (m_State->m_MaskDepth) {
		//m_Surface->GetDepthBuffer()[offset] = depth;
		STW		(fragmentInfo.regDepth, regZBufferAddr);
	}

	if (m_State->m_LogicOpEnabled) {

		//Color maskedColor = 
		//	color.Mask(m_State->m_MaskRed, m_State->m_MaskGreen, m_State->m_MaskBlue, m_State->m_MaskAlpha);

		//U32 newValue = maskedColor.ConvertToRGBA();
		//U32 oldValue = Color::From565A(dstValue, dstAlpha).ConvertToRGBA();
		//U32 value;

		switch (m_State->m_LogicOpcode) {
			default:
			case RasterizerState:: LogicOpClear:		
				//value = 0;						
				break;

			case RasterizerState:: LogicOpAnd:			
				//value = newValue & dstValue;	
				break;

			case RasterizerState:: LogicOpAndReverse:	
				//value = newValue & ~dstValue;	
				break;

			case RasterizerState:: LogicOpCopy:			
				//value = newValue;				
				break;

			case RasterizerState:: LogicOpAndInverted:	
				//value = ~newValue & dstValue;	
				break;

			case RasterizerState:: LogicOpNoop:			
				//value = dstValue;				
				break;

			case RasterizerState:: LogicOpXor:			
				//value = newValue ^ dstValue;	
				break;

			case RasterizerState:: LogicOpOr:			
				//value = newValue | dstValue;	
				break;

			case RasterizerState:: LogicOpNor:			
				//value = ~(newValue | dstValue); 
				break;

			case RasterizerState:: LogicOpEquiv:		
				//value = ~(newValue ^ dstValue); 
				break;

			case RasterizerState:: LogicOpInvert:		
				//value = ~dstValue;				
				break;

			case RasterizerState:: LogicOpOrReverse:	
				//value = newValue | ~dstValue;	
				break;

			case RasterizerState:: LogicOpCopyInverted:	
				//value = ~newValue;				
				break;

			case RasterizerState:: LogicOpOrInverted:	
				//value = ~newValue | dstValue;	
				break;

			case RasterizerState:: LogicOpNand:			
				//value = ~(newValue & dstValue); 
				break;

			case RasterizerState:: LogicOpSet:			
				//value = 0xFFFF;					
				break;
		}

		//maskedColor = Color::FromRGBA(value);
		//m_Surface->GetColorBuffer()[offset] = maskedColor.ConvertTo565();
		//m_Surface->GetAlphaBuffer()[offset] = maskedColor.A();

	} else {
		//Color maskedColor = 
		//	color.Mask(m_State->m_MaskRed, m_State->m_MaskGreen, m_State->m_MaskBlue, m_State->m_MaskAlpha);
		if (m_State->m_MaskRed & m_State->m_MaskGreen & m_State->m_MaskBlue) {
			//m_Surface->GetColorBuffer()[offset] = maskedColor.ConvertTo565();
			STH		(regColor565, regColorAddr);
		} else {
			//m_Surface->GetColorBuffer()[offset] = maskedColor.ConvertTo565();
			DECL_REG	(regSrcMask);
			DECL_REG	(regDstMask);
			DECL_REG	(regMaskedSrc);
			DECL_REG	(regMaskedDst);
			DECL_REG	(regCombined);

			U32 mask = (m_State->m_MaskRed ? 0x001f : 0) |
				(m_State->m_MaskGreen ? 0x07e0 : 0) |
				(m_State->m_MaskBlue ? 0xF800 : 0);

			LDI		(regSrcMask, mask);
			LDI		(regDstMask, ~mask);
			AND		(regMaskedSrc, regColor565, regSrcMask);
			AND		(regMaskedDst, regDstValue, regDstMask);
			OR		(regCombined, regMaskedSrc, regMaskedDst);
			STH		(regCombined, regColorAddr);
		}


		if (m_State->m_MaskAlpha) {
			//m_Surface->GetAlphaBuffer()[offset] = maskedColor.A();
			DECL_REG	(regConstant8);
			DECL_REG	(regShiftedA);
			DECL_REG	(regAdjustedA);

			LDI		(regConstant8, 8);
			LSR		(regShiftedA, regColorA, regConstant8);
			SUB		(regAdjustedA, regColorA, regShiftedA);
			STB		(regAdjustedA, regAlphaAddr);
		}
	}
}


#define LOG_LINEAR_SPAN 3					// logarithm of value base 2
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

	cg_block_t * block = cg_block_create(procedure);

	// Create instructions to calculate addresses of individual fields of
	// edge buffer input arguments

	// texture data, width, height, exponent
	cg_virtual_reg_t * regTextureData =			LOAD_DATA(block, regInfo, OFFSET_TEXTURE_DATA);
	cg_virtual_reg_t * regTextureWidth =		LOAD_DATA(block, regInfo, OFFSET_TEXTURE_WIDTH);
	cg_virtual_reg_t * regTextureHeight =		LOAD_DATA(block, regInfo, OFFSET_TEXTURE_HEIGHT);
	cg_virtual_reg_t * regTextureExponent =		LOAD_DATA(block, regInfo, OFFSET_TEXTURE_EXPONENT);


	// surface color buffer, depth buffer, alpha buffer, stencil buffer
	cg_virtual_reg_t * regColorBuffer =			LOAD_DATA(block, regInfo, OFFSET_SURFACE_COLOR_BUFFER);
	cg_virtual_reg_t * regDepthBuffer =			LOAD_DATA(block, regInfo, OFFSET_SURFACE_DEPTH_BUFFER);
	cg_virtual_reg_t * regAlphaBuffer =			LOAD_DATA(block, regInfo, OFFSET_SURFACE_ALPHA_BUFFER);
	cg_virtual_reg_t * regStencilBuffer =		LOAD_DATA(block, regInfo, OFFSET_SURFACE_STENCIL_BUFFER);
	cg_virtual_reg_t * regSurfaceWidth =		LOAD_DATA(block, regInfo, OFFSET_SURFACE_WIDTH);

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

	LDI		(regOffsetWindowZ, OFFSET_EDGE_BUFFER_WINDOW_Z);
	ADD		(regAddrStartWindowZ, regStart, regOffsetWindowZ);
	ADD		(regAddrEndWindowZ, regEnd, regOffsetWindowZ);

	// u texture coordinate
	DECL_REG	(regOffsetTextureU);
	DECL_REG	(regAddrStartTextureU);
	DECL_REG	(regAddrEndTextureU);

	LDI		(regOffsetTextureU, OFFSET_EDGE_BUFFER_TEX_TU);
	ADD		(regAddrStartTextureU, regStart, regOffsetTextureU);
	ADD		(regAddrEndTextureU, regEnd, regOffsetTextureU);

	// v texture coordinate
	DECL_REG	(regOffsetTextureV);
	DECL_REG	(regAddrStartTextureV);
	DECL_REG	(regAddrEndTextureV);

	LDI		(regOffsetTextureV, OFFSET_EDGE_BUFFER_TEX_TV);
	ADD		(regAddrStartTextureV, regStart, regOffsetTextureV);
	ADD		(regAddrEndTextureV, regEnd, regOffsetTextureV);

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

	//EGL_Fixed invSpan = EGL_Inverse(end.m_WindowCoords.x - start.m_WindowCoords.x);
	DECL_REG	(regEndWindowX);
	DECL_REG	(regStartWindowX);
	DECL_REG	(regDiffX);
	DECL_REG	(regInvSpan);

	LDW		(regEndWindowX, regAddrEndWindowX);
	LDW		(regStartWindowX, regAddrStartWindowX);
	SUB		(regDiffX, regEndWindowX, regStartWindowX);
	FINV	(regInvSpan, regDiffX);

	//FractionalColor baseColor = start.m_Color;
	DECL_REG	(regStartColorR);
	DECL_REG	(regStartColorG);
	DECL_REG	(regStartColorB);
	DECL_REG	(regStartColorA);

	LDW		(regStartColorR, regAddrStartColorR);
	LDW		(regStartColorG, regAddrStartColorG);
	LDW		(regStartColorB, regAddrStartColorB);
	LDW		(regStartColorA, regAddrStartColorA);

	//FractionalColor colorIncrement = (end.m_Color - start.m_Color) * invSpan;
	DECL_REG	(regEndColorR);
	DECL_REG	(regEndColorG);
	DECL_REG	(regEndColorB);
	DECL_REG	(regEndColorA);

	LDW		(regEndColorR, regAddrEndColorR);
	LDW		(regEndColorG, regAddrEndColorG);
	LDW		(regEndColorB, regAddrEndColorB);
	LDW		(regEndColorA, regAddrEndColorA);

	DECL_REG	(regDiffColorR);
	DECL_REG	(regDiffColorG);
	DECL_REG	(regDiffColorB);
	DECL_REG	(regDiffColorA);

	SUB		(regDiffColorR, regEndColorR, regStartColorR);
	SUB		(regDiffColorG, regEndColorG, regStartColorG);
	SUB		(regDiffColorB, regEndColorB, regStartColorB);
	SUB		(regDiffColorA, regEndColorA, regStartColorA);

	DECL_REG	(regColorIncrementR);
	DECL_REG	(regColorIncrementG);
	DECL_REG	(regColorIncrementB);
	DECL_REG	(regColorIncrementA);

	FMUL	(regColorIncrementR, regDiffColorR, regInvSpan);
	FMUL	(regColorIncrementG, regDiffColorG, regInvSpan);
	FMUL	(regColorIncrementB, regDiffColorB, regInvSpan);
	FMUL	(regColorIncrementA, regDiffColorA, regInvSpan);

	//EGL_Fixed deltaInvZ = EGL_Mul(end.m_WindowCoords.z - start.m_WindowCoords.z, invSpan);
	DECL_REG	(regEndWindowZ);
	DECL_REG	(regStartWindowZ);
	DECL_REG	(regDiffInvZ);
	DECL_REG	(regDeltaInvZ);

	LDW		(regEndWindowZ, regAddrEndWindowZ);
	LDW		(regStartWindowZ, regAddrStartWindowZ);
	SUB		(regDiffInvZ, regEndWindowZ, regStartWindowZ);
	FMUL	(regDeltaInvZ, regDiffInvZ, regInvSpan);

	//EGL_Fixed deltaInvU = EGL_Mul(end.m_TextureCoords.tu - start.m_TextureCoords.tu, invSpan);
	DECL_REG	(regEndTextureU);
	DECL_REG	(regStartTextureU);
	DECL_REG	(regDiffInvU);
	DECL_REG	(regDeltaInvU);

	LDW		(regEndTextureU, regAddrEndTextureU);
	LDW		(regStartTextureU, regAddrStartTextureU);
	SUB		(regDiffInvU, regEndTextureU, regStartTextureU);
	FMUL	(regDeltaInvU, regDiffInvU, regInvSpan);

	//EGL_Fixed deltaInvV = EGL_Mul(end.m_TextureCoords.tv - start.m_TextureCoords.tv, invSpan);
	DECL_REG	(regEndTextureV);
	DECL_REG	(regStartTextureV);
	DECL_REG	(regDiffInvV);
	DECL_REG	(regDeltaInvV);

	LDW		(regEndTextureV, regAddrEndTextureV);
	LDW		(regStartTextureV, regAddrStartTextureV);
	SUB		(regDiffInvV, regEndTextureV, regStartTextureV);
	FMUL	(regDeltaInvV, regDiffInvV, regInvSpan);

	//EGL_Fixed deltaFog = EGL_Mul(end.m_FogDensity - start.m_FogDensity, invSpan);
	DECL_REG	(regEndFog);
	DECL_REG	(regStartFog);
	DECL_REG	(regDiffFog);
	DECL_REG	(regDeltaFog);

	LDW		(regEndFog, regAddrEndFog);
	LDW		(regStartFog, regAddrStartFog);
	SUB		(regDiffFog, regEndFog, regStartFog);
	FMUL	(regDeltaFog, regDiffFog, regInvSpan);

	//EGL_Fixed invTu = start.m_TextureCoords.tu;
	//EGL_Fixed invTv = start.m_TextureCoords.tv;
	//EGL_Fixed invZ = start.m_WindowCoords.z;
	//EGL_Fixed fogDensity = start.m_FogDensity;

	//EGL_Fixed z = EGL_Inverse(invZ);
	//EGL_Fixed tu = EGL_Mul(invTu, z);
	//EGL_Fixed tv = EGL_Mul(invTv, z);
	DECL_REG	(regZ);
	DECL_REG	(regU);
	DECL_REG	(regV);

	FINV	(regZ, regStartWindowZ);
	FMUL	(regU, regStartTextureU, regZ);
	FMUL	(regV, regStartTextureV, regZ);

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
	block = cg_block_create(procedure);
	beginLoop0->block = block;

	// Here we define all the loop registers and phi mappings
	DECL_REG	(regLoop0ZEntry);
	DECL_REG	(regLoop0Z);
	DECL_REG	(regLoop1Z);

	PHI		(regLoop0ZEntry, cg_create_virtual_reg_list(procedure->module->heap, regZ, regLoop0Z, regLoop1Z, NULL));

	DECL_REG	(regLoop0UEntry);
	DECL_REG	(regLoop0U);
	DECL_REG	(regLoop1U);

	PHI		(regLoop0UEntry, cg_create_virtual_reg_list(procedure->module->heap, regU, regLoop0U, regLoop1U, NULL));

	DECL_REG	(regLoop0VEntry);
	DECL_REG	(regLoop0V);
	DECL_REG	(regLoop1V);

	PHI		(regLoop0VEntry, cg_create_virtual_reg_list(procedure->module->heap, regV, regLoop0V, regLoop1V, NULL));

	DECL_REG	(regLoop0InvZEntry);
	DECL_REG	(regLoop0InvZ);

	PHI		(regLoop0InvZEntry, cg_create_virtual_reg_list(procedure->module->heap, regStartWindowZ, regLoop0InvZ, NULL));

	DECL_REG	(regLoop0InvUEntry);
	DECL_REG	(regLoop0InvU);

	PHI		(regLoop0InvUEntry, cg_create_virtual_reg_list(procedure->module->heap, regStartTextureU, regLoop0InvU, NULL));

	DECL_REG	(regLoop0InvVEntry);
	DECL_REG	(regLoop0InvV);

	PHI		(regLoop0InvVEntry, cg_create_virtual_reg_list(procedure->module->heap, regStartTextureV, regLoop0InvV, NULL));

		//invZ += deltaInvZ << LOG_LINEAR_SPAN;
		//invTu += deltaInvU << LOG_LINEAR_SPAN;
		//invTv += deltaInvV << LOG_LINEAR_SPAN;
	DECL_REG	(regLinearSpan);
	DECL_REG	(regConstant1);
	DECL_REG	(regDeltaInvZTimesLinearSpan);
	DECL_REG	(regDeltaInvUTimesLinearSpan);
	DECL_REG	(regDeltaInvVTimesLinearSpan);

	LDI		(regConstant1, 1);
	LDI		(regLinearSpan, LINEAR_SPAN);
	FMUL	(regDeltaInvZTimesLinearSpan, regDeltaInvZ, regLinearSpan);	
	FADD	(regLoop0InvZ, regLoop0InvZEntry, regDeltaInvZTimesLinearSpan);
	FMUL	(regDeltaInvUTimesLinearSpan, regDeltaInvU, regLinearSpan);	
	FADD	(regLoop0InvU, regLoop0InvUEntry, regDeltaInvUTimesLinearSpan);
	FMUL	(regDeltaInvVTimesLinearSpan, regDeltaInvV, regLinearSpan);	
	FADD	(regLoop0InvV, regLoop0InvVEntry, regDeltaInvVTimesLinearSpan);

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

	FSUB	(regLoop0DiffZ, regLoop0EndZ, regLoop0Z); // Entry?
	FDIV	(regLoop0ScaledDiffZ, regLoop0DiffZ, regLinearSpan);
	FSUB	(regLoop0DiffU, regLoop0EndU, regLoop0U); // Entry?
	FDIV	(regLoop0ScaledDiffU, regLoop0DiffU, regLinearSpan);
	FSUB	(regLoop0DiffV, regLoop0EndV, regLoop0V); // Entry?
	FDIV	(regLoop0ScaledDiffV, regLoop0DiffV, regLinearSpan);

	// also not to include phi projection for z coming from inner loop

		//int count = LINEAR_SPAN; 

	cg_block_ref_t * beginLoop1 = cg_block_ref_create(procedure);
	cg_block_ref_t * endLoop1 = cg_block_ref_create(procedure);
	cg_block_ref_t * postFragmentLoop1 = cg_block_ref_create(procedure);

	block = cg_block_create(procedure);
	beginLoop1->block = block;

		//do {

	// phi for count, x, z, tu, tv, fog, r, g, b, a

	DECL_REG	(regLoop1CountEntry);
	DECL_REG	(regLoop1Count);
	DECL_REG	(regLoop1XEntry);
	DECL_REG	(regLoop1X);
	DECL_REG	(regLoop1ZEntry);
	DECL_REG	(regLoop1UEntry);
	DECL_REG	(regLoop1VEntry);
	DECL_REG	(regLoop1FogEntry);
	DECL_REG	(regLoop1Fog);
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
	PHI		(regLoop1UEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1U, regU, NULL));
	PHI		(regLoop1VEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1V, regV, NULL));
	PHI		(regLoop1FogEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1Fog, regStartFog, NULL));
	PHI		(regLoop1REntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1R, regStartColorR, NULL));
	PHI		(regLoop1GEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1G, regStartColorG, NULL));
	PHI		(regLoop1BEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1B, regStartColorB, NULL));
	PHI		(regLoop1AEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1A, regStartColorA, NULL));
		
	FragmentGenerationInfo info;
	info.regX = regLoop1XEntry;
	info.regDepth = regLoop1ZEntry;
	info.regU = regLoop1UEntry;
	info.regV = regLoop1VEntry; 
	info.regFog = regLoop1FogEntry;
	info.regR = regLoop1REntry;
	info.regG = regLoop1GEntry;
	info.regB = regLoop1BEntry; 
	info.regA = regLoop1AEntry;	

	info.regTextureData = regTextureData;
	info.regTextureWidth = regTextureWidth;
	info.regTextureHeight = regTextureHeight;
	info.regTextureExponent = regTextureExponent;

	// surface color buffer, depth buffer, alpha buffer, stencil buffer
	info.regColorBuffer = regColorBuffer;
	info.regDepthBuffer = regDepthBuffer;
	info.regAlphaBuffer = regAlphaBuffer;
	info.regStencilBuffer = regStencilBuffer;
	info.regSurfaceWidth = regSurfaceWidth;

	GenerateFragment(procedure, block, postFragmentLoop1, info); 

	block = cg_block_create(procedure);
	postFragmentLoop1->block = block;

			//baseColor += colorIncrement;
	FADD	(regLoop1R, regLoop1REntry, regColorIncrementR);
	FADD	(regLoop1G, regLoop1REntry, regColorIncrementG);
	FADD	(regLoop1B, regLoop1REntry, regColorIncrementB);
	FADD	(regLoop1A, regLoop1REntry, regColorIncrementA);

			//fogDensity += deltaFog;
			//z += deltaZ;
			//tu += deltaTu;
			//tv += deltaTv;
	FADD	(regLoop1Fog, regLoop1FogEntry, regDeltaFog);
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

	DECL_FLAGS	(regLoop0Condition);

	CMP		(regLoop0Condition, regLoop1X, regXLinEnd);
	BNE		(regLoop0Condition, beginLoop0);

	//if (x != xEnd) {
	block = cg_block_create(procedure);
	endLoop0->block = block;

	cg_block_ref_t * beginLoop2 = cg_block_ref_create(procedure);
	cg_block_ref_t * endLoop2 = cg_block_ref_create(procedure);
	cg_block_ref_t * postFragmentLoop2 = cg_block_ref_create(procedure);

	DECL_REG	(regBlock4X);
	DECL_REG	(regBlock4Z);
	DECL_REG	(regBlock4U);
	DECL_REG	(regBlock4V);
	DECL_REG	(regBlock4DiffX);
	DECL_FLAGS	(regBlock4Condition);

	PHI		(regBlock4X, cg_create_virtual_reg_list(procedure->module->heap, regX, regLoop1X, NULL));
	PHI		(regBlock4Z, cg_create_virtual_reg_list(procedure->module->heap, regLoop1Z, regZ, NULL));
	PHI		(regBlock4U, cg_create_virtual_reg_list(procedure->module->heap, regLoop1U, regU, NULL));
	PHI		(regBlock4V, cg_create_virtual_reg_list(procedure->module->heap, regLoop1V, regV, NULL));
	SUB_S	(regBlock4DiffX, regBlock4Condition, regXEnd, regBlock4X);
	BEQ		(regBlock4Condition, endLoop2);

		//EGL_Fixed endZ = EGL_Inverse(end.m_WindowCoords.z);
		//EGL_Fixed endTu = EGL_Mul(end.m_TextureCoords.tu, endZ);
		//EGL_Fixed endTv = EGL_Mul(end.m_TextureCoords.tv, endZ);
	DECL_REG	(regEndZ);
	DECL_REG	(regEndU);
	DECL_REG	(regEndV);

	FINV	(regEndZ, regEndWindowZ);
	FMUL	(regEndU, regEndZ, regEndTextureU);
	FMUL	(regEndV, regEndZ, regEndTextureV);

		//invSpan = EGL_Inverse(EGL_FixedFromInt(xEnd - x));
	DECL_REG	(regFixedBlock4DiffX);
	DECL_REG	(regBlock4InvSpan);

	FCNV	(regFixedBlock4DiffX, regBlock4DiffX);
	FINV	(regBlock4InvSpan, regFixedBlock4DiffX);

		//EGL_Fixed deltaZ = EGL_Mul(endZ - z, invSpan);
		//EGL_Fixed deltaTu = EGL_Mul(endTu - tu, invSpan);
		//EGL_Fixed deltaTv = EGL_Mul(endTv - tv, invSpan);
	DECL_REG	(regBlock4DiffZ);
	DECL_REG	(regBlock4DiffU);
	DECL_REG	(regBlock4DiffV);
	DECL_REG	(regLoop2ScaledDiffZ);
	DECL_REG	(regLoop2ScaledDiffU);
	DECL_REG	(regLoop2ScaledDiffV);

	FSUB	(regBlock4DiffZ, regEndZ, regBlock4Z);
	FMUL	(regLoop2ScaledDiffZ, regBlock4DiffZ, regBlock4InvSpan);
	FSUB	(regBlock4DiffU, regEndU, regBlock4U);
	FMUL	(regLoop2ScaledDiffU, regBlock4DiffU, regBlock4InvSpan);
	FSUB	(regBlock4DiffV, regEndV, regBlock4V);
	FMUL	(regLoop2ScaledDiffV, regBlock4DiffV, regBlock4InvSpan);

		//for (; x < xEnd; ++x) {
	block = cg_block_create(procedure);
	beginLoop2->block = block;

	// phi for x, z, tu, tv, fog, r, g, b, a

	DECL_REG	(regLoop2XEntry);
	DECL_REG	(regLoop2X);
	DECL_REG	(regLoop2ZEntry);
	DECL_REG	(regLoop2Z);
	DECL_REG	(regLoop2UEntry);
	DECL_REG	(regLoop2U);
	DECL_REG	(regLoop2VEntry);
	DECL_REG	(regLoop2V);
	DECL_REG	(regLoop2FogEntry);
	DECL_REG	(regLoop2Fog);
	DECL_REG	(regLoop2REntry);
	DECL_REG	(regLoop2R);
	DECL_REG	(regLoop2GEntry);
	DECL_REG	(regLoop2G);
	DECL_REG	(regLoop2BEntry);
	DECL_REG	(regLoop2B);
	DECL_REG	(regLoop2AEntry);
	DECL_REG	(regLoop2A);


	PHI		(regLoop2XEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2X, regBlock4X, NULL));
	PHI		(regLoop2ZEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2Z, regBlock4Z, NULL));
	PHI		(regLoop2UEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2U, regBlock4U, NULL));
	PHI		(regLoop2VEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2V, regBlock4V, NULL));
	PHI		(regLoop2FogEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2Fog, regLoop1Fog, regStartFog, NULL));
	PHI		(regLoop2REntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2R, regLoop1R, regStartColorR, NULL));
	PHI		(regLoop2GEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2G, regLoop1G, regStartColorG, NULL));
	PHI		(regLoop2BEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2B, regLoop1B, regStartColorB, NULL));
	PHI		(regLoop2AEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2A, regLoop1A, regStartColorA, NULL));

	FragmentGenerationInfo info2;
	info2.regX = regLoop2XEntry;
	info2.regDepth = regLoop2ZEntry;
	info2.regU = regLoop2UEntry;
	info2.regV = regLoop2VEntry; 
	info2.regFog = regLoop2FogEntry;
	info2.regR = regLoop2REntry;
	info2.regG = regLoop2GEntry;
	info2.regB = regLoop2BEntry; 
	info2.regA = regLoop2AEntry;	

	info2.regTextureData = regTextureData;
	info2.regTextureWidth = regTextureWidth;
	info2.regTextureHeight = regTextureHeight;
	info2.regTextureExponent = regTextureExponent;

	// surface color buffer, depth buffer, alpha buffer, stencil buffer
	info2.regColorBuffer = regColorBuffer;
	info2.regDepthBuffer = regDepthBuffer;
	info2.regAlphaBuffer = regAlphaBuffer;
	info2.regStencilBuffer = regStencilBuffer;
	info2.regSurfaceWidth = regSurfaceWidth;

	GenerateFragment(procedure, block, postFragmentLoop2, info2); 

	block = cg_block_create(procedure);
	postFragmentLoop2->block = block;

			//baseColor += colorIncrement;
	FADD	(regLoop2R, regLoop2REntry, regColorIncrementR);
	FADD	(regLoop2G, regLoop2REntry, regColorIncrementG);
	FADD	(regLoop2B, regLoop2REntry, regColorIncrementB);
	FADD	(regLoop2A, regLoop2REntry, regColorIncrementA);

			//fogDensity += deltaFog;
			//z += deltaZ;
			//tu += deltaTu;
			//tv += deltaTv;
	FADD	(regLoop2Fog, regLoop2FogEntry, regDeltaFog);
	FADD	(regLoop2Z, regLoop2ZEntry, regLoop2ScaledDiffZ);
	FADD	(regLoop2U, regLoop2UEntry, regLoop2ScaledDiffU);
	FADD	(regLoop2V, regLoop2VEntry, regLoop2ScaledDiffV);

			//++x;
	ADD		(regLoop2X, regLoop2XEntry, regConstant1);

	DECL_FLAGS	(regCondLoopEnd);

	CMP		(regCondLoopEnd, regLoop2X, regXEnd);
	BLT		(regCondLoopEnd, beginLoop2);

		//}

	//}
	block = cg_block_create(procedure);
	endLoop2->block = block;

	RET();

}

namespace {
	void Dump(const char * filename, cg_module_t * module)
	{
		FILE * fp = fopen(filename, "w");
		cg_module_dump(module, fp);
		fclose(fp);
	}
}

void CodeGenerator :: CompileRasterScanLine() {

	cg_heap_t * heap = cg_heap_create(4096);
	cg_module_t * module = cg_module_create(heap);

	m_Module = module;

	GenerateRasterScanLine();

	Dump("dump1.txt", m_Module);

	cg_module_inst_def(m_Module);
	cg_module_amode(m_Module);

	Dump("dump2.txt", m_Module);

	cg_module_eliminate_dead_code(m_Module);

	Dump("dump3.txt", m_Module);

	cg_module_unify_registers(m_Module);
	cg_module_allocate_variables(m_Module);
	cg_module_inst_use_chains(m_Module);
	cg_module_dataflow(m_Module);

	Dump("dump4.txt", m_Module);

	cg_runtime_info_t runtime; 
	memset(&runtime, 0, sizeof runtime);

	cg_codegen_t * codegen = cg_codegen_create(heap, &runtime);
	cg_codegen_emit_module(codegen, m_Module);

	ARMDis dis;
	armdis_init(&dis);
	armdis_dump(&dis, "dump5.s", cg_codegen_segment(codegen));

#ifdef WINCE
	// flush data cache and clear instruction cache to make new code visible to execution unit
	CacheSync(CACHE_SYNC_INSTRUCTIONS | CACHE_SYNC_WRITEBACK);		

#endif

	cg_codegen_destroy(codegen);
	cg_heap_destroy(module->heap);
}

