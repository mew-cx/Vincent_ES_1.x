// ==========================================================================
//
// GenFragment.cpp		JIT Class for 3D Rendering Library
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

#define DECL_REG(reg) cg_virtual_reg_t * reg = cg_virtual_reg_create(procedure, cg_reg_type_general)
#define DECL_FLAGS(reg) cg_virtual_reg_t * reg = cg_virtual_reg_create(procedure, cg_reg_type_flags)

	// This function will create the color components after multiplication scaled by a factor
	// of 256
	void BlendColor(cg_block_t * block,
		cg_virtual_reg_t ** resultR, cg_virtual_reg_t ** resultG, cg_virtual_reg_t ** resultB, cg_virtual_reg_t ** resultA, 
		cg_virtual_reg_t * coeffR, cg_virtual_reg_t * coeffG, cg_virtual_reg_t * coeffB, cg_virtual_reg_t * coeffA,
		cg_virtual_reg_t * colorR, cg_virtual_reg_t * colorG, cg_virtual_reg_t * colorB, cg_virtual_reg_t * colorA) {

		// TO DO: implement color blending code here
		// we could do optimizations here in an attempt to share registers

		*resultR = cg_virtual_reg_create(block->proc, cg_reg_type_general);
		*resultG = cg_virtual_reg_create(block->proc, cg_reg_type_general);
		*resultB = cg_virtual_reg_create(block->proc, cg_reg_type_general);
		*resultA = cg_virtual_reg_create(block->proc, cg_reg_type_general);

		MUL(*resultR, colorR, coeffR);
		MUL(*resultG, colorG, coeffG);
		MUL(*resultB, colorB, coeffB);
		MUL(*resultA, colorA, coeffA);
	}

	// ----------------------------------------------------------------------
	// Emit code to convert a representation of a color as individual
	// R, G and B components into a 16-bit 565 representation
	//
	// R and B are within the range 0..0x1f, G is in 0..0x3f
	// ----------------------------------------------------------------------
	void Color565FromRGB(cg_block_t * block, cg_virtual_reg_t * result,
		cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b) {
		cg_proc_t * procedure = block->proc;

		DECL_REG	(regConstant5);
		DECL_REG	(regConstant11);
		DECL_REG	(regShiftedR);
		DECL_REG	(regShiftedG);
		DECL_REG	(regBG);

		LDI		(regConstant5, 5);
		LDI		(regConstant11, 11);
		LSL		(regShiftedR, r, regConstant11);
		LSL		(regShiftedG, g, regConstant5);
		OR		(regBG, b, regShiftedG);
		OR		(result, regBG, regShiftedR);
	}
}


// Actually, we could extract the scaling of the texture coordinates into the outer driving loop, 
// and have the adjusted clipping range for tu and tv be stored in the rasterizer.

void CodeGenerator :: GenerateFragment(cg_proc_t * procedure,  cg_block_t * currentBlock,
			cg_block_ref_t * continuation, FragmentGenerationInfo & fragmentInfo,
			int weight) {

	cg_block_t * block = currentBlock;

	cg_virtual_reg_t * regDepthBuffer =			LOAD_DATA(block, fragmentInfo.regInfo, OFFSET_SURFACE_DEPTH_BUFFER);

	// Signature of generated function is:
	// (I32 x, I32 y, EGL_Fixed depth, EGL_Fixed tu, EGL_Fixed tv, EGL_Fixed fogDensity, const Color& baseColor);
	
	// fragment level clipping (for now)

	//if (m_Surface->GetWidth() <= x || x < 0 ||
	//	m_Surface->GetHeight() <= y || y < 0) {
	//	return;
	//}

	if (m_State->m_ScissorTestEnabled) {
		DECL_REG	(regConstXStart);
		DECL_REG	(regConstXEnd);
		DECL_FLAGS	(regXStartTest);
		DECL_FLAGS	(regXEndTest);

		LDI			(regConstXStart, m_State->m_ScissorX);
		LDI			(regConstXEnd, m_State->m_ScissorX + m_State->m_ScissorWidth);

		CMP			(regXStartTest, fragmentInfo.regX, regConstXStart);
		BLT			(regXStartTest, continuation);
		CMP			(regXEndTest, fragmentInfo.regX, regConstXEnd);
		BGE			(regXEndTest, continuation);
	}

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
	ADD		(regZBufferAddr, regDepthBuffer, regOffset4);
	LDW		(regZBufferValue, regZBufferAddr);
	CMP		(regDepthTest, fragmentInfo.regDepth, regZBufferValue);

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

		cg_virtual_reg_t * regU = fragmentInfo.regU;
		cg_virtual_reg_t * regV = fragmentInfo.regV;

		//EGL_Fixed tu0;
		//EGL_Fixed tv0;

		DECL_REG	(regU0);
		DECL_REG	(regV0);

		DECL_REG	(regMask);

		LDI		(regMask, 0xffff);

		switch (m_State->m_WrappingModeS) {
			case RasterizerState::WrappingModeClampToEdge:
				//tu0 = EGL_CLAMP(tu, 0, EGL_ONE);
				{
					DECL_REG	(regConstantZero);
					DECL_REG	(regTemp);

					LDI		(regConstantZero, EGL_FixedFromInt(0));
					MIN		(regTemp, regU, regMask);
					MAX		(regU0, regTemp, regConstantZero);
				}
				break;

			default:
			case RasterizerState::WrappingModeRepeat:
				//tu0 = tu & 0xffff;
				{
					AND		(regU0, regU, regMask);
				}
				break;
		}

		switch (m_State->m_WrappingModeT) {
			case RasterizerState::WrappingModeClampToEdge:
				//tv0 = EGL_CLAMP(tv, 0, EGL_ONE);
				{
					DECL_REG	(regConstantZero);
					DECL_REG	(regTemp);

					LDI		(regConstantZero, EGL_FixedFromInt(0));
					MIN		(regTemp, regV, regMask);
					MAX		(regV0, regTemp, regConstantZero);
				}
				break;

			default:
			case RasterizerState::WrappingModeRepeat:
				//tv0 = tv & 0xffff;
				{
					AND		(regV0, regV, regMask);
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
		DECL_REG	(regConstant16);


		cg_virtual_reg_t * regTextureLogWidth =		LOAD_DATA(block, fragmentInfo.regInfo, OFFSET_TEXTURE_LOG_WIDTH);
		cg_virtual_reg_t * regTextureLogHeight =	LOAD_DATA(block, fragmentInfo.regInfo, OFFSET_TEXTURE_LOG_HEIGHT);
		cg_virtual_reg_t * regTextureData =			LOAD_DATA(block, fragmentInfo.regInfo, OFFSET_TEXTURE_DATA);
		cg_virtual_reg_t * regTextureExponent =		LOAD_DATA(block, fragmentInfo.regInfo, OFFSET_TEXTURE_EXPONENT);

		LDI		(regConstant16, 16);
		LSL		(regScaledU, regU0, regTextureLogWidth);
		ASR		(regTexX, regScaledU, regConstant16);
		LSL		(regScaledV, regV0, regTextureLogHeight);
		ASR		(regTexY, regScaledV, regConstant16);
		LSL		(regScaledTexY, regTexY, regTextureLogWidth);
		ADD		(regTexOffset, regScaledTexY, regTexX);

		switch (m_State->m_InternalFormat) {
			case RasterizerState::TextureFormatAlpha:				// 8
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
				

				ADD		(regTexAddr, regTexOffset, regTextureData);
				LDB		(regTexData, regTexAddr);
				LDI		(regConstant7, 7);
				LSR		(regShifted7, regTexData, regConstant7);
				ADD		(regTexColorA, regTexData, regShifted7);
				LDI		(regTexColorR, 0x1f);
				LDI		(regTexColorG, 0x3f);
				LDI		(regTexColor565, 0xffff);

				}
				break;

			case RasterizerState::TextureFormatLuminance:			// 8
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

				ADD		(regTexAddr, regTexOffset, regTextureData);
				LDB		(regTexData, regTexAddr);
				LDI		(regTexColorA, 0x100);
				LDI		(regMask5, 0x1f);
				LDI		(regMask6, 0x3f);
				LDI		(regConstant2, 2);
				LDI		(regConstant3, 3);
				LSR		(regColor5, regTexData, regConstant3);
				AND		(regTexColorR, regColor5, regMask5);
				LSR		(regColor6, regTexData, regConstant2);
				AND		(regTexColorG, regColor6, regMask6);

				Color565FromRGB(block, regTexColor565, regTexColorR,
					regTexColorG, regTexColorB);
				}
				break;

			case RasterizerState::TextureFormatLuminanceAlpha:		// 8-8
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
				ADD		(regTexAddr, regScaledOffset, regTextureData);
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
				LSR		(regColor5, regTexData, regConstant3);
				AND		(regTexColorR, regColor5, regMask5);
				LSR		(regColor6, regTexData, regConstant2);
				AND		(regTexColorG, regColor6, regMask6);

				Color565FromRGB(block, regTexColor565, regTexColorR,
					regTexColorG, regTexColorB);

				}
				break;

			case RasterizerState::TextureFormatRGB565:					// 5-6-5
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
				ADD		(regTexAddr, regScaledOffset, regTextureData);
				LDH		(regTexColor565, regTexAddr);
				LDI		(regTexColorA, 0x100);
				LDI		(regConstant5, 5);
				LDI		(regConstant11, 11);
				LSR		(regShifted5, regTexColor565, regConstant5);
				LSR		(regShifted11, regTexColor565, regConstant11);
				LDI		(regMask5, 0x1f);
				LDI		(regMask6, 0x3f);
				AND		(regTexColorB, regTexColor565, regMask5);
				AND		(regTexColorG, regShifted5, regMask6);
				AND		(regTexColorR, regShifted11, regMask5);

				}
				break;

			case RasterizerState::TextureFormatRGB8:						// 8-8-8
				{
				regTexColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);

				DECL_REG	(regConstant1);
				DECL_REG	(regConstant2);
				DECL_REG	(regConstant3);
				DECL_REG	(regShiftedOffset);
				DECL_REG	(regScaledOffset);
				DECL_REG	(regTexAddr0);
				DECL_REG	(regTexAddr1);
				DECL_REG	(regTexAddr2);
				DECL_REG	(regByteR);
				DECL_REG	(regByteG);
				DECL_REG	(regByteB);

				LDI		(regTexColorA, 0x100);
				LDI		(regConstant1, 1);
				LDI		(regConstant2, 2);
				LDI		(regConstant3, 3);

				LSL		(regShiftedOffset, regTexOffset, regConstant1);
				ADD		(regScaledOffset, regTexOffset, regShiftedOffset);
				ADD		(regTexAddr0, regTextureData, regScaledOffset);
				LDB		(regByteR, regTexAddr0);
				ADD		(regTexAddr1, regTexAddr0, regConstant1);
				LSR		(regTexColorR, regByteR, regConstant3);
				LDB		(regByteG, regTexAddr1);
				ADD		(regTexAddr2, regTexAddr0, regConstant2);
				LDB		(regByteB, regTexAddr2);
				LSR		(regTexColorG, regByteG, regConstant2);
				LSR		(regTexColorB, regByteB, regConstant3);

				regTexColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

				Color565FromRGB(block, regTexColor565, regTexColorR,
					regTexColorG, regTexColorB);

				}

				break;

			case RasterizerState::TextureFormatRGBA4444:					// 4-4-4-4
				{
				DECL_REG	(regScaledOffset);
				DECL_REG	(regConstantOne);
				DECL_REG	(regTexAddr);
				DECL_REG	(regTexColor4444);

				LDI		(regConstantOne, 1);
				LSL		(regScaledOffset, regTexOffset, regConstantOne);
				ADD		(regTexAddr, regScaledOffset, regTextureData);
				LDH		(regTexColor4444, regTexAddr);

				regTexColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);

				//U8 r = (u4444 & 0xF000u) >> 8;
				//r |= r >> 4;
				DECL_REG	(regConstant11);
				DECL_REG	(regMask41);
				DECL_REG	(regBaseR);
				DECL_REG	(regMaskedR);
				DECL_REG	(regConstant4);
				DECL_REG	(regShiftedR);

				LDI			(regConstant11, 11);
				LSR			(regBaseR, regTexColor4444, regConstant11);
				LDI			(regMask41, 0x1E);
				AND			(regMaskedR, regBaseR, regMask41);
				LDI			(regConstant4, 4);
				LSR			(regShiftedR, regMaskedR, regConstant4);
				OR			(regTexColorR, regMaskedR, regShiftedR);
				
				//U8 g = (u4444 & 0x0F00u) >> 4;
				//g |= g >> 4;
				DECL_REG	(regConstant6);
				DECL_REG	(regMask42);
				DECL_REG	(regBaseG);
				DECL_REG	(regMaskedG);
				DECL_REG	(regShiftedG);

				LDI			(regConstant6, 6);
				LSR			(regBaseG, regTexColor4444, regConstant6);
				LDI			(regMask42, 0x3C);
				AND			(regMaskedG, regBaseG, regMask42);
				LSR			(regShiftedG, regMaskedG, regConstant4);
				OR			(regTexColorG, regMaskedG, regShiftedG);				

				//U8 b = (u4444 & 0x00F0u);
				//b |= b >> 4;
				DECL_REG	(regConstant3);
				DECL_REG	(regBaseB);
				DECL_REG	(regMaskedB);
				DECL_REG	(regShiftedB);

				LDI			(regConstant3, 3);
				LSR			(regBaseB, regTexColor4444, regConstant3);
				AND			(regMaskedB, regBaseB, regMask41);
				LSR			(regShiftedB, regMaskedB, regConstant4);
				OR			(regTexColorB, regMaskedB, regShiftedB);
				

				//U8 a = (u4444 & 0x000Fu) << 4;
				//a |= a >> 4;
				DECL_REG	(regMask4);
				DECL_REG	(regMaskedA);
				DECL_REG	(regShiftedA);
				DECL_REG	(regAdjustedA);

				LDI			(regMask4, 0xf);
				AND			(regMaskedA, regTexColor4444, regMask4);
				LSR			(regShiftedA, regMaskedA, regConstant3);
				ADD			(regAdjustedA, regMaskedA, regShiftedA);
				LSL			(regTexColorA, regAdjustedA, regConstant4);

				regTexColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

				Color565FromRGB(block, regTexColor565, regTexColorR,
					regTexColorG, regTexColorB);

				}

				break;

			case RasterizerState::TextureFormatRGBA8:						// 8-8-8-8
				{
				regTexColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);

				DECL_REG	(regConstant1);
				DECL_REG	(regConstant2);
				DECL_REG	(regConstant3);
				DECL_REG	(regConstant7);
				DECL_REG	(regScaledOffset);
				DECL_REG	(regTexAddr0);
				DECL_REG	(regTexAddr1);
				DECL_REG	(regTexAddr2);
				DECL_REG	(regTexAddr3);
				DECL_REG	(regByteR);
				DECL_REG	(regByteG);
				DECL_REG	(regByteB);
				DECL_REG	(regByteA);
				DECL_REG	(regShifted7);

				LDI		(regConstant1, 1);
				LDI		(regConstant2, 2);
				LDI		(regConstant3, 3);
				LDI		(regConstant7, 7);

				LSL		(regScaledOffset, regTexOffset, regConstant2);
				ADD		(regTexAddr0, regTextureData, regScaledOffset);
				LDB		(regByteR, regTexAddr0);
				ADD		(regTexAddr1, regTexAddr0, regConstant1);
				LSR		(regTexColorR, regByteR, regConstant3);
				LDB		(regByteG, regTexAddr1);
				ADD		(regTexAddr2, regTexAddr0, regConstant2);
				LDB		(regByteB, regTexAddr2);
				LSR		(regTexColorG, regByteG, regConstant2);
				ADD		(regTexAddr3, regTexAddr0, regConstant3);
				LDB		(regByteA, regTexAddr3);
				LSR		(regTexColorB, regByteB, regConstant3);
				LSR		(regShifted7, regByteA, regConstant7);
				ADD		(regTexColorA, regByteA, regShifted7);

				regTexColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

				Color565FromRGB(block, regTexColor565, regTexColorR,
					regTexColorG, regTexColorB);

				}

				break;

			case RasterizerState::TextureFormatRGBA5551:					// 5-5-5-1
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
				ADD		(regTexAddr, regScaledOffset, regTextureData);
				LDH		(regTexData, regTexAddr);

				DECL_REG	(regConstant8);
				DECL_REG	(regShifted8);
				DECL_REG	(regMask100);

				LDI		(regConstant8, 8);
				LDI		(regMask100, 0x100);
				LSL		(regShifted8, regTexData, regConstant8);
				AND		(regTexColorA, regShifted8, regMask100);

				DECL_REG	(regMask5);
				DECL_REG	(regMask51);
				DECL_REG	(regConstant5);
				DECL_REG	(regConstant11);
				DECL_REG	(regShifted5);
				DECL_REG	(regShifted11);
				DECL_REG	(regShifted1);
				DECL_REG	(regBaseG);
				DECL_REG	(regShiftedG);

				LDI		(regMask5, 0x1f);
				LSR		(regShifted1, regTexData, regConstant1);
				AND		(regTexColorB, regShifted1, regMask5);
				LDI		(regMask51, 0x3e);
				LDI		(regConstant5, 5);
				LDI		(regConstant11, 11);
				LSR		(regShifted5, regTexData, regConstant5);
				AND		(regBaseG, regShifted5, regMask51);
				LSR		(regShiftedG, regBaseG, regConstant5);
				OR		(regTexColorG, regBaseG, regShiftedG);
				LSR		(regShifted11, regTexData, regConstant11);
				AND		(regTexColorR, regShifted11, regMask5);

				regTexColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

				Color565FromRGB(block, regTexColor565, regTexColorR,
					regTexColorG, regTexColorB);
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

		switch (m_State->m_InternalFormat) {
			default:
			case RasterizerState::TextureFormatAlpha:
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

						Color565FromRGB(block, regColor565, regColorR,
							regColorG, regColorB);

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

						Color565FromRGB(block, regColor565, regColorR,
							regColorG, regColorB);

						DECL_REG	(regAlphaProduct);

						MUL		(regAlphaProduct, fragmentInfo.regA, regTexColorA);
						LSR		(regColorA, regAlphaProduct, regConstant16);
						}

						break;
				}
				break;

			case RasterizerState::TextureFormatLuminance:
			case RasterizerState::TextureFormatRGB565:
			case RasterizerState::TextureFormatRGB8:
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

						Color565FromRGB(block, regColor565, regColorR,
							regColorG, regColorB);
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

							Color565FromRGB(block, regColor565, regColorR,
								regColorG, regColorB);
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

							block = cg_block_create(procedure, weight);
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

							block = cg_block_create(procedure, weight);
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

							block = cg_block_create(procedure, weight);
							noClampingB->block = block;

							PHI		(regColorB, cg_create_virtual_reg_list(procedure->module->heap, regClampedB, regSumB, NULL));
						}
						// create RGB 565 representation
						{
							regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

							Color565FromRGB(block, regColor565, regColorR,
								regColorG, regColorB);
						}
						}

						break;
				}
				break;

			case RasterizerState::TextureFormatLuminanceAlpha:
			case RasterizerState::TextureFormatRGBA5551:
			case RasterizerState::TextureFormatRGBA4444:
			case RasterizerState::TextureFormatRGBA8:
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

						Color565FromRGB(block, regColor565, regColorR,
							regColorG, regColorB);

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

							Color565FromRGB(block, regColor565, regColorR,
								regColorG, regColorB);
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

							Color565FromRGB(block, regColor565, regColorR,
								regColorG, regColorB);
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

							block = cg_block_create(procedure, weight);
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

							block = cg_block_create(procedure, weight);
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

							block = cg_block_create(procedure, weight);
							noClampingB->block = block;

							PHI		(regColorB, cg_create_virtual_reg_list(procedure->module->heap, regClampedB, regSumB, NULL));
						}
						// create RGB 565 representation
						{
							regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

							Color565FromRGB(block, regColor565, regColorR,
								regColorG, regColorB);
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

			Color565FromRGB(block, regColor565, regColorR,
				regColorG, regColorB);
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
		regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

		Color565FromRGB(block, regColor565, regColorR,
			regColorG, regColorB);
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

		cg_virtual_reg_t * regStencilBuffer =		LOAD_DATA(block, fragmentInfo.regInfo, OFFSET_SURFACE_STENCIL_BUFFER);

		LDI		(regStencilRef, m_State->m_StencilReference & m_State->m_StencilMask);
		LDI		(regStencilMask, m_State->m_StencilMask);
		ADD		(regStencilAddr, regStencilBuffer, regOffset4);
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
				//stencilTest = stencilRef < stencil;	
				passedTest = cg_op_bge;
				break;

			case RasterizerState::CompFuncEqual:	
				//stencilTest = stencilRef == stencil;
				passedTest = cg_op_beq;
				break;

			case RasterizerState::CompFuncLEqual:	
				//stencilTest = stencilRef <= stencil;
				passedTest = cg_op_bgt;
				break;

			case RasterizerState::CompFuncGreater:	
				//stencilTest = stencilRef > stencil;	
				passedTest = cg_op_ble;
				break;

			case RasterizerState::CompFuncNotEqual:	
				//stencilTest = stencilRef != stencil;
				passedTest = cg_op_bne;
				break;

			case RasterizerState::CompFuncGEqual:	
				//stencilTest = stencilRef >= stencil;
				passedTest = cg_op_blt;
				break;

			case RasterizerState::CompFuncAlways:	
				//stencilTest = true;					
				passedTest = cg_op_bra;
				break;
		}

		// branch on stencil test
		cg_block_ref_t * labelStencilPassed = cg_block_ref_create(procedure);
		cg_block_ref_t * labelStencilBypassed = cg_block_ref_create(procedure);

		if (passedTest != cg_op_nop) {
			cg_create_inst_branch_cond(block, passedTest,	regStencilTest, labelStencilPassed CG_INST_DEBUG_ARGS);
		}

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
		block = cg_block_create(procedure, weight);
		labelStencilPassed->block = block;

		//if (!depthTest) {
			if (branchOnDepthTestPassed == cg_op_nop) {
				// nothing
			} else if (branchOnDepthTestPassed == cg_op_bra) {
				BRA		(labelStencilZTestPassed);
			} else {
				DECL_FLAGS(regDepthTest1);

				CMP	(regDepthTest1, fragmentInfo.regDepth, regZBufferValue);
				cg_create_inst_branch_cond(block, branchOnDepthTestPassed, regDepthTest1, labelStencilZTestPassed CG_INST_DEBUG_ARGS);
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
				//STW		(regNewStencilValue, regStencilAddr);
			//}
			}

			if (m_State->m_DepthTestEnabled) {
				// return;
				BRA		(continuation);
			}
		//} else {
		// stencil nad z-test passed
		block = cg_block_create(procedure, weight);
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
				//STW		(regNewStencilValue, regStencilAddr);
			//}
			}

		// stencil test bypassed
		block = cg_block_create(procedure, weight);
		labelStencilBypassed->block = block;
	}

	// surface color buffer, depth buffer, alpha buffer, stencil buffer
	cg_virtual_reg_t * regColorBuffer =			LOAD_DATA(block, fragmentInfo.regInfo, OFFSET_SURFACE_COLOR_BUFFER);
	cg_virtual_reg_t * regAlphaBuffer =			LOAD_DATA(block, fragmentInfo.regInfo, OFFSET_SURFACE_ALPHA_BUFFER);

	//U16 dstValue = m_Surface->GetColorBuffer()[offset];
	//U8 dstAlpha = m_Surface->GetAlphaBuffer()[offset];
	DECL_REG	(regDstValue);
	DECL_REG	(regDstAlphaValue);
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

		ADD		(regColorAddr, regColorBuffer, regOffset2);
		ADD		(regAlphaAddr, regAlphaBuffer, regOffset);
		LDH		(regDstValue, regColorAddr);
		LDB		(regDstAlphaValue, regAlphaAddr);
		LDI		(regConstant5, 5);
		LDI		(regConstant11, 11);
		LSR		(regShifted5, regDstValue, regConstant5);
		LSR		(regShifted11, regDstValue, regConstant11);
		LDI		(regMask5, 0x1f);
		LDI		(regMask6, 0x3f);
		AND		(regDstB, regDstValue, regMask5);
		AND		(regDstG, regShifted5, regMask6);
		AND		(regDstR, regShifted11, regMask5);
	}

	// Blending
	if (m_State->m_BlendingEnabled) {

		cg_virtual_reg_t * regSrcBlendR = 0;
		cg_virtual_reg_t * regSrcBlendG = 0;
		cg_virtual_reg_t * regSrcBlendB = 0;
		cg_virtual_reg_t * regSrcBlendA = 0;

		cg_virtual_reg_t * regDstBlendR = 0;
		cg_virtual_reg_t * regDstBlendG = 0;
		cg_virtual_reg_t * regDstBlendB = 0;
		cg_virtual_reg_t * regDstBlendA = 0;

		bool noSource = false;
		bool noTarget = false;

		//Color dstColor = Color::From565A(dstValue, dstAlpha);
		DECL_REG	(regDstAlpha);

		{
			// convert alpha from 0..0xff to 0..0x100
			DECL_REG	(regConstant8);
			DECL_REG	(regShiftedAlpha);

			LDI			(regConstant8, 8);
			LSR			(regShiftedAlpha, regDstAlphaValue, regConstant8);
			ADD			(regDstAlpha, regDstAlphaValue, regShiftedAlpha);
		}

		//Color srcCoeff, dstCoeff;

		// ------------------------------------------------------------------
		// In all the blending code, the assumption is that the blending
		// coefficients are in the range 0x00 - 0x100, while the color to
		// be modulated is R:5, G:6, B:5, A:8 bits format
		// ------------------------------------------------------------------

		switch (m_State->m_BlendFuncSrc) {
			default:
			case RasterizerState::BlendFuncSrcZero:
				//srcCoeff = Color(0, 0, 0, 0);
				regSrcBlendR = cg_virtual_reg_create(block->proc, cg_reg_type_general);

				LDI			(regSrcBlendR, 0);

				regSrcBlendG = regSrcBlendB = regSrcBlendA = regSrcBlendR;
				noSource = true;

				break;

			case RasterizerState::BlendFuncSrcOne:
				{
					DECL_REG(regConstant8);
					LDI		(regConstant8, 8);

					//srcCoeff = Color(Color::MAX, Color::MAX, Color::MAX, Color::MAX);
					// regular blending will create a color multiplied by 256, so lets
					// scale the values up

					regSrcBlendR = cg_virtual_reg_create(block->proc, cg_reg_type_general);
					regSrcBlendG = cg_virtual_reg_create(block->proc, cg_reg_type_general);
					regSrcBlendB = cg_virtual_reg_create(block->proc, cg_reg_type_general);
					regSrcBlendA = cg_virtual_reg_create(block->proc, cg_reg_type_general);

					LSL		(regSrcBlendR, regColorR, regConstant8);
					LSL		(regSrcBlendG, regColorG, regConstant8);
					LSL		(regSrcBlendB, regColorB, regConstant8);
					LSL		(regSrcBlendA, regColorA, regConstant8);
				}
				break;

			case RasterizerState::BlendFuncSrcDstColor:
				{
					//srcCoeff = color;	// adjust scaling of R, G, B
					DECL_REG(regShiftedR);
					DECL_REG(regShiftedG);
					DECL_REG(regShiftedB);

					DECL_REG(regConstant3);
					LDI		(regConstant3, 3);
					DECL_REG(regConstant2);
					LDI		(regConstant2, 2);

					LSL		(regShiftedR, regDstR, regConstant3);
					LSL		(regShiftedG, regDstG, regConstant2);
					LSL		(regShiftedB, regDstB, regConstant3);

					//srcCoeff * color

					BlendColor(block,
						&regSrcBlendR, &regSrcBlendG, &regSrcBlendB, &regSrcBlendA, 
						regShiftedR, regShiftedG, regShiftedB, regDstAlpha,
						regColorR, regColorG, regColorB, regColorA);
				}
				break;

			case RasterizerState::BlendFuncSrcOneMinusDstColor:
				{
					//srcCoeff = Color(Color::MAX - color.R(), Color::MAX - color.G(), Color::MAX - color.B(), Color::MAX - color.A());

					DECL_REG(regConstant256);
					DECL_REG(regShiftedR);
					DECL_REG(regCoeffR);
					DECL_REG(regShiftedG);
					DECL_REG(regCoeffG);
					DECL_REG(regShiftedB);
					DECL_REG(regCoeffB);
					DECL_REG(regCoeffA);

					DECL_REG(regConstant3);
					LDI		(regConstant3, 3);
					DECL_REG(regConstant2);
					LDI		(regConstant2, 2);
					LDI		(regConstant256, 256);

					LSL		(regShiftedR, regDstR, regConstant3);
					LSL		(regShiftedG, regDstG, regConstant2);
					LSL		(regShiftedB, regDstB, regConstant3);

					SUB		(regCoeffR, regConstant256, regShiftedR);
					SUB		(regCoeffG, regConstant256, regShiftedG);
					SUB		(regCoeffB, regConstant256, regShiftedB);
					SUB		(regCoeffA, regConstant256, regDstAlpha);

					//srcCoeff * color

					BlendColor(block,
						&regSrcBlendR, &regSrcBlendG, &regSrcBlendB, &regSrcBlendA, 
						regCoeffR, regCoeffG, regCoeffB, regCoeffA,
						regColorR, regColorG, regColorB, regColorA);
				}
				break;

			case RasterizerState::BlendFuncSrcSrcAlpha:
				//srcCoeff = Color(color.A(), color.A(), color.A(), color.A());
				//srcCoeff * color
					BlendColor(block,
						&regSrcBlendR, &regSrcBlendG, &regSrcBlendB, &regSrcBlendA, 
						regColorA, regColorA, regColorA, regColorA,
						regColorR, regColorG, regColorB, regColorA);

				break;

			case RasterizerState::BlendFuncSrcOneMinusSrcAlpha:
				{
					//srcCoeff = Color(Color::MAX - color.A(), Color::MAX - color.A(), Color::MAX - color.A(), Color::MAX - color.A());
					DECL_REG(regConstant256);
					DECL_REG(regDiffA);

					LDI		(regConstant256, 256);
					SUB		(regDiffA, regConstant256, regColorA);

					//srcCoeff * color
						BlendColor(block,
							&regSrcBlendR, &regSrcBlendG, &regSrcBlendB, &regSrcBlendA, 
							regDiffA, regDiffA, regDiffA, regDiffA,
							regColorR, regColorG, regColorB, regColorA);
				}
				break;

			case RasterizerState::BlendFuncSrcDstAlpha:
				//srcCoeff = Color(dstAlpha, dstAlpha, dstAlpha, dstAlpha);
				//srcCoeff * color
					BlendColor(block,
						&regSrcBlendR, &regSrcBlendG, &regSrcBlendB, &regSrcBlendA, 
						regDstAlpha, regDstAlpha, regDstAlpha, regDstAlpha,
						regColorR, regColorG, regColorB, regColorA);

				break;

			case RasterizerState::BlendFuncSrcOneMinusDstAlpha:
				{
					//srcCoeff = Color(Color::MAX - dstAlpha, Color::MAX - dstAlpha, Color::MAX - dstAlpha, Color::MAX - dstAlpha);
					DECL_REG(regConstant256);
					DECL_REG(regDiffA);

					LDI		(regConstant256, 256);
					SUB		(regDiffA, regConstant256, regDstAlpha);

					//srcCoeff * color
						BlendColor(block,
							&regSrcBlendR, &regSrcBlendG, &regSrcBlendB, &regSrcBlendA, 
							regDiffA, regDiffA, regDiffA, regDiffA,
							regColorR, regColorG, regColorB, regColorA);
				}
				break;

			case RasterizerState::BlendFuncSrcSrcAlphaSaturate:
				{
					//	U8 rev = Color::MAX - dstAlpha;
					DECL_REG(regConstant256);
					DECL_REG(regRev);

					LDI		(regConstant256, 256);
					SUB		(regRev, regConstant256, regDstAlpha);

					//	U8 f = (rev < color.A() ? rev : color.A());
					DECL_REG(regF);
					MIN		(regF, regRev, regColorA);

					//	dstCoeff = Color(f, f, f, Color::MAX);
					//dstCoeff * dstColor
					BlendColor(block,
						&regSrcBlendR, &regSrcBlendG, &regSrcBlendB, &regSrcBlendA, 
						regF, regF, regF, regConstant256,
						regColorR, regColorG, regColorB, regColorA);
				}
				break;
		}

		switch (m_State->m_BlendFuncDst) {
			default:
			case RasterizerState::BlendFuncDstZero:
				//dstCoeff = Color(0, 0, 0, 0);
				regDstBlendR = cg_virtual_reg_create(block->proc, cg_reg_type_general);
				LDI			(regDstBlendR, 0);

				regDstBlendG = regDstBlendB = regDstBlendA = regDstBlendR;
				noTarget = true;

				break;

			case RasterizerState::BlendFuncDstOne:
				{
					//dstCoeff = Color(Color::MAX, Color::MAX, Color::MAX, Color::MAX);

					DECL_REG(regConstant8);
					LDI		(regConstant8, 8);

					regDstBlendR = cg_virtual_reg_create(block->proc, cg_reg_type_general);
					regDstBlendG = cg_virtual_reg_create(block->proc, cg_reg_type_general);
					regDstBlendB = cg_virtual_reg_create(block->proc, cg_reg_type_general);
					regDstBlendA = cg_virtual_reg_create(block->proc, cg_reg_type_general);

					LSL		(regDstBlendR, regDstR, regConstant8);
					LSL		(regDstBlendG, regDstG, regConstant8);
					LSL		(regDstBlendB, regDstB, regConstant8);
					LSL		(regDstBlendA, regDstAlpha, regConstant8);
				}
				break;

			case RasterizerState::BlendFuncDstSrcColor:
				{
					//dstCoeff = color;
					DECL_REG(regShiftedR);
					DECL_REG(regShiftedG);
					DECL_REG(regShiftedB);

					DECL_REG(regConstant3);
					LDI		(regConstant3, 3);
					DECL_REG(regConstant2);
					LDI		(regConstant2, 2);

					LSL		(regShiftedR, regColorR, regConstant3);
					LSL		(regShiftedG, regColorG, regConstant2);
					LSL		(regShiftedB, regColorB, regConstant3);

					//dstCoeff * dstColor
					BlendColor(block,
						&regDstBlendR, &regDstBlendG, &regDstBlendB, &regDstBlendA, 
						regShiftedR, regShiftedG, regShiftedB, regColorA,
						regDstR, regDstG, regDstB, regDstAlpha);
				}
				break;

			case RasterizerState::BlendFuncDstOneMinusSrcColor:
				{
					//dstCoeff = Color(Color::MAX - color.R(), Color::MAX - color.G(), Color::MAX - color.B(), Color::MAX - color.A());
					DECL_REG(regConstant256);
					DECL_REG(regShiftedR);
					DECL_REG(regCoeffR);
					DECL_REG(regShiftedG);
					DECL_REG(regCoeffG);
					DECL_REG(regShiftedB);
					DECL_REG(regCoeffB);
					DECL_REG(regCoeffA);

					DECL_REG(regConstant3);
					LDI		(regConstant3, 3);
					DECL_REG(regConstant2);
					LDI		(regConstant2, 2);
					LDI		(regConstant256, 256);

					LSL		(regShiftedR, regColorR, regConstant3);
					LSL		(regShiftedG, regColorG, regConstant2);
					LSL		(regShiftedB, regColorB, regConstant3);

					SUB		(regCoeffR, regConstant256, regShiftedR);
					SUB		(regCoeffG, regConstant256, regShiftedG);
					SUB		(regCoeffB, regConstant256, regShiftedB);
					SUB		(regCoeffA, regConstant256, regColorA);

					//dstCoeff * dstColor
					BlendColor(block,
						&regDstBlendR, &regDstBlendG, &regDstBlendB, &regDstBlendA, 
						regCoeffR, regCoeffG, regCoeffB, regCoeffA,
						regDstR, regDstG, regDstB, regDstAlpha);
				}
				break;

			case RasterizerState::BlendFuncDstSrcAlpha:
				//dstCoeff = Color(color.A(), color.A(), color.A(), color.A());
				//dstCoeff * dstColor
					BlendColor(block,
						&regDstBlendR, &regDstBlendG, &regDstBlendB, &regDstBlendA, 
						regColorA, regColorA, regColorA, regColorA,
						regDstR, regDstG, regDstB, regDstAlpha);
				break;

			case RasterizerState::BlendFuncDstOneMinusSrcAlpha:
				{
					//dstCoeff = Color(Color::MAX - color.A(), Color::MAX - color.A(), Color::MAX - color.A(), Color::MAX - color.A());
					DECL_REG(regConstant256);
					DECL_REG(regDiffA);

					LDI		(regConstant256, 256);
					SUB		(regDiffA, regConstant256, regColorA);

					//dstCoeff * dstColor
					BlendColor(block,
						&regDstBlendR, &regDstBlendG, &regDstBlendB, &regDstBlendA, 
						regDiffA, regDiffA, regDiffA, regDiffA,
						regDstR, regDstG, regDstB, regDstAlpha);
				}
				break;

			case RasterizerState::BlendFuncDstDstAlpha:
				{
					//dstCoeff = Color(dstAlpha, dstAlpha, dstAlpha, dstAlpha);
					//dstCoeff * dstColor
					BlendColor(block,
						&regDstBlendR, &regDstBlendG, &regDstBlendB, &regDstBlendA, 
						regDstAlpha, regDstAlpha, regDstAlpha, regDstAlpha,
						regDstR, regDstG, regDstB, regDstAlpha);
				}
				break;

			case RasterizerState::BlendFuncDstOneMinusDstAlpha:
				{
					//dstCoeff = Color(Color::MAX - dstAlpha, Color::MAX - dstAlpha, Color::MAX - dstAlpha, Color::MAX - dstAlpha);
					DECL_REG(regConstant256);
					DECL_REG(regDiffA);

					LDI		(regConstant256, 256);
					SUB		(regDiffA, regConstant256, regDstAlpha);

					//dstCoeff * dstColor
					BlendColor(block,
						&regDstBlendR, &regDstBlendG, &regDstBlendB, &regDstBlendA, 
						regDiffA, regDiffA, regDiffA, regDiffA,
						regDstR, regDstG, regDstB, regDstAlpha);
				}
				break;
		}

		// Note: At this point, we expect the products of the colors srcCoeff * color and
		// dstCoeff * dstColor to be multiplied by a factor of 256!

		//color = srcCoeff * color + dstCoeff * dstColor;

		// just add modulated source to modulated destination

		if (noSource) {
			regColorR = regDstBlendR;
			regColorG = regDstBlendG;
			regColorB = regDstBlendB;
			regColorA = regDstBlendA;
		} else if (noTarget) {
			regColorR = regSrcBlendR;
			regColorG = regSrcBlendG;
			regColorB = regSrcBlendB;
			regColorA = regSrcBlendA;
		} else {
			DECL_REG(regTempR);
			DECL_REG(regTempG);
			DECL_REG(regTempB);
			DECL_REG(regTempA);

			ADD		(regTempR, regDstBlendR, regSrcBlendR);
			ADD		(regTempG, regDstBlendG, regSrcBlendG);
			ADD		(regTempB, regDstBlendB, regSrcBlendB);
			ADD		(regTempA, regDstBlendA, regSrcBlendA);

			DECL_REG	(regMaxRB);
			DECL_REG	(regMaxG);
			DECL_REG	(regMaxA);

			DECL_REG(regClampedR);
			DECL_REG(regClampedG);
			DECL_REG(regClampedB);
			DECL_REG(regClampedA);

			LDI		(regMaxRB, 0x1f00);
			MIN		(regClampedR, regTempR, regMaxRB);
			MIN		(regClampedB, regTempB, regMaxRB);
			LDI		(regMaxG, 0x3f00);
			MIN		(regClampedG, regTempG, regMaxG);
			LDI		(regMaxA, 0x10000);
			MIN		(regClampedA, regTempA, regMaxA);

			regColorR = regClampedR;
			regColorG = regClampedG;
			regColorB = regClampedB;
			regColorA = regClampedA;
		}

		// create RGB 565 representation
		DECL_REG	(regConstant8);
		DECL_REG	(regConstant3);
		DECL_REG	(regShiftedR);
		DECL_REG	(regShiftedB);
		DECL_REG	(regShiftedG);
		DECL_REG	(regBG);
		DECL_REG	(regNewColor565);
		DECL_REG	(regNewColorA);
		DECL_REG	(regMaskR);
		DECL_REG	(regMaskG);
		DECL_REG	(regMaskedR);
		DECL_REG	(regMaskedG);

		LDI		(regConstant8, 8);
		LSR		(regShiftedB, regColorB, regConstant8);
		LDI		(regMaskR, 0x1f00);
		AND		(regMaskedR, regColorR, regMaskR);
		LDI		(regConstant3, 3);
		LSL		(regShiftedR, regMaskedR, regConstant3);
		LDI		(regMaskG, 0x3f00);
		AND		(regMaskedG, regColorG, regMaskG);
		LSR		(regShiftedG, regMaskedG, regConstant3);
		OR		(regBG, regShiftedB, regShiftedG);
		OR		(regNewColor565, regBG, regShiftedR);
		LSR		(regNewColorA, regColorA, regConstant8);

		regColor565 = regNewColor565;
		regColorA = regNewColorA;
	}

	// Masking and write to framebuffer
	if (m_State->m_MaskDepth) {
		//m_Surface->GetDepthBuffer()[offset] = depth;
		STW		(fragmentInfo.regDepth, regZBufferAddr);
	}

	{
		// convert Alpha from 0..0x100 range to 0..0xff
		DECL_REG	(regConstant8);
		DECL_REG	(regConstant255);
		DECL_REG	(regAdjustedA);
		DECL_REG	(regMultipliedA);

		LDI		(regConstant255, 255);
		LDI		(regConstant8, 8);
		MUL		(regMultipliedA, regColorA, regConstant255);
		LSR		(regAdjustedA, regMultipliedA, regConstant8);

		regColorA = regAdjustedA;
	}

	if (m_State->m_LogicOpEnabled) {

		//U32 newValue = maskedColor.ConvertToRGBA();
		//U32 oldValue = Color::From565A(dstValue, dstAlpha).ConvertToRGBA();
		//U32 value;
		cg_virtual_reg_t * regNewValue = regColor565;
		cg_virtual_reg_t * regNewValueA = regColorA;
		cg_virtual_reg_t * regOldValue = regDstValue;
		cg_virtual_reg_t * regOldValueA = regDstAlphaValue;

		regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);
		regColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
		
		switch (m_State->m_LogicOpcode) {
			default:
			case RasterizerState:: LogicOpClear:		
				//value = 0;						
				XOR		(regColor565, regOldValue, regOldValue);
				XOR		(regColorA, regOldValueA, regOldValueA);
				break;

			case RasterizerState:: LogicOpAnd:			
				//value = newValue & dstValue;	
				AND		(regColor565, regOldValue, regNewValue);
				AND		(regColorA, regOldValueA, regNewValueA);
				break;

			case RasterizerState:: LogicOpAndReverse:	
				{
				//value = newValue & ~dstValue;	
				DECL_REG (regNotOldValue);
				DECL_REG (regNotOldValueA);

				NOT		(regNotOldValue, regOldValue);
				NOT		(regNotOldValueA, regOldValueA);
				AND		(regColor565, regNotOldValue, regNewValue);
				AND		(regColorA, regNotOldValueA, regNewValueA);
				break;
				}

			case RasterizerState:: LogicOpCopy:			
				//value = newValue;		
				regColor565 = regNewValue;
				regColorA = regNewValueA;
				break;

			case RasterizerState:: LogicOpAndInverted:	
				{
				//value = ~newValue & dstValue;	
				DECL_REG (regNotNewValue);
				DECL_REG (regNotNewValueA);

				NOT		(regNotNewValue, regNewValue);
				NOT		(regNotNewValueA, regNewValueA);
				AND		(regColor565, regNotNewValue, regOldValue);
				AND		(regColorA, regNotNewValueA, regOldValueA);
				break;
				}

			case RasterizerState:: LogicOpNoop:			
				//value = dstValue;				
				regColor565 = regOldValue;
				regColorA = regOldValueA;
				break;

			case RasterizerState:: LogicOpXor:			
				//value = newValue ^ dstValue;	
				XOR		(regColor565, regOldValue, regNewValue);
				XOR		(regColorA, regOldValueA, regNewValueA);
				break;

			case RasterizerState:: LogicOpOr:			
				//value = newValue | dstValue;	
				OR		(regColor565, regOldValue, regNewValue);
				OR		(regColorA, regOldValueA, regNewValueA);
				break;

			case RasterizerState:: LogicOpNor:			
				{
				//value = ~(newValue | dstValue); 
				DECL_REG(regCombinedValue);
				DECL_REG(regCombinedValueA);

				OR		(regCombinedValue, regOldValue, regNewValue);
				OR		(regCombinedValueA, regOldValueA, regNewValueA);
				NOT		(regColor565, regCombinedValue);
				NOT		(regColorA,	regCombinedValueA);
				break;
				}

			case RasterizerState:: LogicOpEquiv:		
				{
				//value = ~(newValue ^ dstValue); 
				DECL_REG(regCombinedValue);
				DECL_REG(regCombinedValueA);

				XOR		(regCombinedValue, regOldValue, regNewValue);
				XOR		(regCombinedValueA, regOldValueA, regNewValueA);
				NOT		(regColor565, regCombinedValue);
				NOT		(regColorA,	regCombinedValueA);
				break;
				}

			case RasterizerState:: LogicOpInvert:		
				//value = ~dstValue;				
				NOT		(regColor565, regOldValue);
				NOT		(regColorA, regOldValueA);
				break;

			case RasterizerState:: LogicOpOrReverse:	
				{
				//value = newValue | ~dstValue;	
				DECL_REG (regNotOldValue);
				DECL_REG (regNotOldValueA);

				NOT		(regNotOldValue, regOldValue);
				NOT		(regNotOldValueA, regOldValueA);
				OR		(regColor565, regNotOldValue, regNewValue);
				OR		(regColorA, regNotOldValueA, regNewValueA);
				break;
				}

			case RasterizerState:: LogicOpCopyInverted:	
				//value = ~newValue;			
				NOT		(regColor565, regNewValue);
				NOT		(regColorA, regNewValueA);
				break;

			case RasterizerState:: LogicOpOrInverted:	
				{
				//value = ~newValue | dstValue;	
				DECL_REG (regNotNewValue);
				DECL_REG (regNotNewValueA);

				NOT		(regNotNewValue, regNewValue);
				NOT		(regNotNewValueA, regNewValueA);
				OR		(regColor565, regNotNewValue, regOldValue);
				OR		(regColorA, regNotNewValueA, regOldValueA);
				break;
				}

			case RasterizerState:: LogicOpNand:			
				{
				//value = ~(newValue & dstValue); 
				DECL_REG(regCombinedValue);
				DECL_REG(regCombinedValueA);

				AND		(regCombinedValue, regOldValue, regNewValue);
				AND		(regCombinedValueA, regOldValueA, regNewValueA);
				NOT		(regColor565, regCombinedValue);
				NOT		(regColorA,	regCombinedValueA);
				break;
				}

			case RasterizerState:: LogicOpSet:			
				//value = 0xFFFF;					
				{
				LDI		(regColor565,	0xffff);
				LDI		(regColorA,		0xff);
				}
				break;
		}
	} 
	
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

		U32 mask = (m_State->m_MaskBlue ? 0x001f : 0) |
			(m_State->m_MaskGreen ? 0x07e0 : 0) |
			(m_State->m_MaskRed ? 0xF800 : 0);

		LDI		(regSrcMask, mask);
		LDI		(regDstMask, ~mask);
		AND		(regMaskedSrc, regColor565, regSrcMask);
		AND		(regMaskedDst, regDstValue, regDstMask);
		OR		(regCombined, regMaskedSrc, regMaskedDst);
		STH		(regCombined, regColorAddr);
	}

	if (m_State->m_MaskAlpha) {
		//m_Surface->GetAlphaBuffer()[offset] = maskedColor.A();
		STB		(regColorA, regAlphaAddr);
	}
}


