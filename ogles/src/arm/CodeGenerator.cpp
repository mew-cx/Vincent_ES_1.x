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

using namespace EGL;


namespace {

	cg_virtual_reg_t * LOAD_DATA(cg_block_t * block, cg_virtual_reg_t * base, I32 constant) {
		cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
		cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);
		cg_virtual_reg_t * value = cg_virtual_reg_create(block->proc, cg_reg_type_general);

		cg_create_inst_load_immed(block, cg_op_ldi,	offset, constant);
		cg_create_inst_binary(block, cg_op_add,	addr, base, offset);
		cg_create_inst_load(block, cg_op_ldw,	value, addr);

		return value;
	}

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
	cg_virtual_reg_t * regDepthTest = cg_virtual_reg_create(procedure, cg_reg_type_flags);
	cg_virtual_reg_t * regScaledY = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regConstant1 = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regConstant2 = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regOffset4 = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regOffset2 = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regZBufferAddr = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regZBufferValue = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load_immed(currentBlock,		cg_op_ldi,		regConstant1, 1);
	cg_create_inst_load_immed(currentBlock,		cg_op_ldi,		regConstant2, 2);
	cg_create_inst_binary(currentBlock,			cg_op_lsl,		regOffset2, regOffset, regConstant1);
	cg_create_inst_binary(currentBlock,			cg_op_lsl,		regOffset4,	regOffset, regConstant2);
	cg_create_inst_binary(currentBlock,			cg_op_add,		regZBufferAddr, fragmentInfo.regDepthBuffer, regOffset4);
	cg_create_inst_load(currentBlock,			cg_op_ldw,		regZBufferValue, regZBufferAddr);
	cg_create_inst_compare(currentBlock,		cg_op_fcmp,		regDepthTest, regZBufferValue, fragmentInfo.regDepth);

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
			cg_create_inst_branch_label(currentBlock,	cg_op_bra,		continuation);
		} else {
			cg_create_inst_branch_cond(currentBlock,	branchOnDepthTestFailed, regDepthTest, continuation);
		}
	}

	//Color color = baseColor;
	cg_virtual_reg_t * regColorR;
	cg_virtual_reg_t * regColorG;
	cg_virtual_reg_t * regColorB;
	cg_virtual_reg_t * regColorA;
	cg_virtual_reg_t * regColor565;

	cg_block_t * block = currentBlock;

	if (m_State->m_TextureEnabled) {

		//EGL_Fixed tu0;
		//EGL_Fixed tv0;
		cg_virtual_reg_t * regU0 = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regV0 = cg_virtual_reg_create(procedure, cg_reg_type_general);

		switch (m_Texture->GetWrappingModeS()) {
			case MultiTexture::WrappingModeClampToEdge:
				//tu0 = EGL_CLAMP(tu, 0, EGL_ONE);
				{
					cg_virtual_reg_t * regConstantOne = cg_virtual_reg_create(procedure, cg_reg_type_general);
					cg_virtual_reg_t * regConstantZero = cg_virtual_reg_create(procedure, cg_reg_type_general);
					cg_virtual_reg_t * regCompareOne = cg_virtual_reg_create(procedure, cg_reg_type_flags);
					cg_virtual_reg_t * regCompareZero = cg_virtual_reg_create(procedure, cg_reg_type_flags);
					cg_virtual_reg_t * regNewU1 = cg_virtual_reg_create(procedure, cg_reg_type_general);
					cg_virtual_reg_t * regNewU2 = cg_virtual_reg_create(procedure, cg_reg_type_general);

					cg_block_ref_t * label1 = cg_block_ref_create(procedure);
					cg_block_ref_t * label2 = cg_block_ref_create(procedure);

					cg_create_inst_load_immed(block,	cg_op_ldi,		regConstantOne, EGL_FixedFromInt(1));
					cg_create_inst_compare(block,		cg_op_fcmp,		regCompareOne, fragmentInfo.regU, regConstantOne);
					cg_create_inst_branch_cond(block,	cg_op_ble,		regCompareOne, label1);
					cg_create_inst_load_immed(block,	cg_op_ldi,		regNewU1, EGL_FixedFromInt(1));
					cg_create_inst_branch_label(block,	cg_op_bra,		label2);

					block = cg_block_create(procedure);
					label1->block = block;

					cg_create_inst_load_immed(block,	cg_op_ldi,		regConstantZero, EGL_FixedFromInt(0));
					cg_create_inst_compare(block,		cg_op_fcmp,		regCompareZero, fragmentInfo.regU, regConstantZero);
					cg_create_inst_branch_cond(block,	cg_op_bge,		regCompareZero, label2);
					cg_create_inst_load_immed(block,	cg_op_ldi,		regNewU2, EGL_FixedFromInt(0));

					block = cg_block_create(procedure);
					label2->block = block;

					cg_virtual_reg_list_t * regList = 
						cg_create_virtual_reg_list(procedure->module->heap,
												   fragmentInfo.regU, regNewU1, regNewU2, NULL);

					cg_create_inst_phi(block,			cg_op_phi,		regU0, regList);
				}
				break;

			default:
			case MultiTexture::WrappingModeRepeat:
				//tu0 = tu & 0xffff;
				{
					cg_virtual_reg_t * regMask = cg_virtual_reg_create(procedure, cg_reg_type_general);
					cg_create_inst_load_immed(block,	cg_op_ldi,		regMask, 0xffff);
					cg_create_inst_binary(block,		cg_op_and,		regU0, fragmentInfo.regU, regMask);
				}
				break;
		}

		switch (m_Texture->GetWrappingModeT()) {
			case MultiTexture::WrappingModeClampToEdge:
				//tv0 = EGL_CLAMP(tv, 0, EGL_ONE);
				{
					cg_virtual_reg_t * regConstantOne = cg_virtual_reg_create(procedure, cg_reg_type_general);
					cg_virtual_reg_t * regConstantZero = cg_virtual_reg_create(procedure, cg_reg_type_general);
					cg_virtual_reg_t * regCompareOne = cg_virtual_reg_create(procedure, cg_reg_type_flags);
					cg_virtual_reg_t * regCompareZero = cg_virtual_reg_create(procedure, cg_reg_type_flags);
					cg_virtual_reg_t * regNewV1 = cg_virtual_reg_create(procedure, cg_reg_type_general);
					cg_virtual_reg_t * regNewV2 = cg_virtual_reg_create(procedure, cg_reg_type_general);

					cg_block_ref_t * label1 = cg_block_ref_create(procedure);
					cg_block_ref_t * label2 = cg_block_ref_create(procedure);

					cg_create_inst_load_immed(block,	cg_op_ldi,	regConstantOne, EGL_FixedFromInt(1));
					cg_create_inst_compare(block,		cg_op_fcmp,	regCompareOne, fragmentInfo.regV, regConstantOne);
					cg_create_inst_branch_cond(block,	cg_op_ble,	regCompareOne, label1);
					cg_create_inst_load_immed(block,	cg_op_ldi,	regNewV1, EGL_FixedFromInt(1));
					cg_create_inst_branch_label(block,	cg_op_bra,	label2);

					block = cg_block_create(procedure);
					label1->block = block;

					cg_create_inst_load_immed(block, cg_op_ldi,	regConstantZero, EGL_FixedFromInt(0));
					cg_create_inst_compare(block, cg_op_fcmp,	regCompareZero, fragmentInfo.regV, regConstantZero);
					cg_create_inst_branch_cond(block, cg_op_bge,	regCompareZero, label2);
					cg_create_inst_load_immed(block, cg_op_ldi,	regNewV2, EGL_FixedFromInt(0));

					block = cg_block_create(procedure);
					label2->block = block;

					cg_create_inst_phi(block, cg_op_phi,	regV0, cg_create_virtual_reg_list(procedure->module->heap, fragmentInfo.regV, regNewV1, regNewV2, NULL));
				}
				break;

			default:
			case MultiTexture::WrappingModeRepeat:
				//tv0 = tv & 0xffff;
				{
					cg_virtual_reg_t * regMask = cg_virtual_reg_create(procedure, cg_reg_type_general);
					cg_create_inst_load_immed(block, cg_op_ldi,	regMask, 0xffff);
					cg_create_inst_binary(block, cg_op_and,	regV0, fragmentInfo.regV, regMask);
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
		cg_virtual_reg_t * regScaledU = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regTexX = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regScaledV = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regTexY = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regScaledTexY = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regTexOffset = cg_virtual_reg_create(procedure, cg_reg_type_general);

		cg_create_inst_binary(block, cg_op_mul,		regScaledU, regU0, fragmentInfo.regTextureWidth);
		cg_create_inst_unary(block, cg_op_trunc,		regTexX, regScaledU);
		cg_create_inst_binary(block, cg_op_mul,		regScaledV, regV0, fragmentInfo.regTextureHeight);
		cg_create_inst_unary(block, cg_op_trunc,		regTexY, regScaledV);
		cg_create_inst_binary(block, cg_op_lsl,		regScaledTexY, regTexY, fragmentInfo.regTextureExponent);
		cg_create_inst_binary(block, cg_op_add,		regTexOffset, regTexX, regScaledTexY);

		switch (m_Texture->GetInternalFormat()) {
			case Texture::TextureFormatAlpha:				// 8
				{
				//texColor = Color(0xff, 0xff, 0xff, reinterpret_cast<const U8 *>(data)[texOffset]);
				regTexColorR = regTexColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regTexAddr = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstant7 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regShifted7 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regTexData = cg_virtual_reg_create(procedure, cg_reg_type_general);
				

				cg_create_inst_binary(block, cg_op_add,	regTexAddr, regTexOffset, fragmentInfo.regTextureData);
				cg_create_inst_load(block, cg_op_ldb,	regTexData, regTexAddr);
				cg_create_inst_load_immed(block, cg_op_ldi,	regConstant7, 7);
				cg_create_inst_binary(block, cg_op_lsr,	regShifted7, regTexData, regConstant7);
				cg_create_inst_binary(block, cg_op_add,	regTexColorA, regTexData, regShifted7);
				cg_create_inst_load_immed(block, cg_op_ldi,	regTexColorR, 0x1f);
				cg_create_inst_load_immed(block, cg_op_ldi,	regTexColorG, 0x3f);
				cg_create_inst_load_immed(block, cg_op_ldi,	regTexColor565, 0xffff);

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
				cg_virtual_reg_t * regTexAddr = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regTexData = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regMask5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regMask6 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstant2 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstant3 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstant5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regColor5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regColor6 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regShiftedB = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regShiftedG = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regRG = cg_virtual_reg_create(procedure, cg_reg_type_general);

				cg_create_inst_binary(block, cg_op_add,	regTexAddr, regTexOffset, fragmentInfo.regTextureData);
				cg_create_inst_load(block, cg_op_ldb,	regTexData, regTexAddr);
				cg_create_inst_load_immed(block, cg_op_ldi,	regTexColorA, 0x100);
				cg_create_inst_load_immed(block, cg_op_ldi,	regMask5, 0x1f);
				cg_create_inst_load_immed(block, cg_op_ldi,	regMask6, 0x3f);
				cg_create_inst_load_immed(block, cg_op_ldi,	regConstant2, 2);
				cg_create_inst_load_immed(block, cg_op_ldi,	regConstant3, 3);
				cg_create_inst_binary(block, cg_op_lsl,	regColor5, regTexData, regConstant3);
				cg_create_inst_binary(block, cg_op_and,	regTexColorR, regColor5, regMask5);
				cg_create_inst_binary(block, cg_op_lsl,	regColor6, regTexData, regConstant2);
				cg_create_inst_binary(block, cg_op_and,	regTexColorG, regColor6, regMask6);
				cg_create_inst_load_immed(block, cg_op_ldi,	regConstant5, 5);
				cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
				cg_create_inst_binary(block, cg_op_lsl,	regShiftedB, regTexColorB, regConstant11);
				cg_create_inst_binary(block, cg_op_lsl,	regShiftedG, regTexColorG, regConstant5);
				cg_create_inst_binary(block, cg_op_or,	regRG, regTexColorR, regShiftedG);
				cg_create_inst_binary(block, cg_op_or,	regTexColor565, regRG, regShiftedB);
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
				cg_virtual_reg_t * regTexAddr = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regTexData = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regMask5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regMask6 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstant2 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstant3 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstant5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regColor5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regColor6 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regShiftedB = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regShiftedG = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regRG = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regScaledOffset = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstantOne = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regMask = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstant8 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regAlpha = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regMaskedAlphaByte = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstant7 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regShifted7 = cg_virtual_reg_create(procedure, cg_reg_type_general);

				cg_create_inst_load_immed(block, cg_op_ldi,	regConstantOne, 1);
				cg_create_inst_binary(block, cg_op_lsl,	regScaledOffset, regTexOffset, regConstantOne);
				cg_create_inst_binary(block, cg_op_add,	regTexAddr, regScaledOffset, fragmentInfo.regTextureData);
				cg_create_inst_load(block, cg_op_ldh,	regTexData, regTexAddr);
				cg_create_inst_load_immed(block, cg_op_ldi,	regMask, 0xff);
				cg_create_inst_load_immed(block, cg_op_ldi,	regConstant8, 8);
				cg_create_inst_binary(block, cg_op_lsr,	regAlpha, regTexData, regConstant8);
				cg_create_inst_binary(block, cg_op_and,	regMaskedAlphaByte, regAlpha, regMask);
				cg_create_inst_load_immed(block, cg_op_ldi,	regConstant7, 7);
				cg_create_inst_binary(block, cg_op_lsr,	regShifted7, regMaskedAlphaByte, regConstant7);
				cg_create_inst_binary(block, cg_op_add,	regTexColorA, regMaskedAlphaByte, regShifted7);
				cg_create_inst_load_immed(block, cg_op_ldi,	regMask5, 0x1f);
				cg_create_inst_load_immed(block, cg_op_ldi,	regMask6, 0x3f);
				cg_create_inst_load_immed(block, cg_op_ldi,	regConstant2, 2);
				cg_create_inst_load_immed(block, cg_op_ldi,	regConstant3, 3);
				cg_create_inst_binary(block, cg_op_lsl,	regColor5, regTexData, regConstant3);
				cg_create_inst_binary(block, cg_op_and,	regTexColorR, regColor5, regMask5);
				cg_create_inst_binary(block, cg_op_lsl,	regColor6, regTexData, regConstant2);
				cg_create_inst_binary(block, cg_op_and,	regTexColorG, regColor6, regMask6);
				cg_create_inst_load_immed(block, cg_op_ldi,	regConstant5, 5);
				cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
				cg_create_inst_binary(block, cg_op_lsl,	regShiftedB, regTexColorB, regConstant11);
				cg_create_inst_binary(block, cg_op_lsl,	regShiftedG, regTexColorG, regConstant5);
				cg_create_inst_binary(block, cg_op_or,	regRG, regTexColorR, regShiftedG);
				cg_create_inst_binary(block, cg_op_or,	regTexColor565, regRG, regShiftedB);

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
				cg_virtual_reg_t * regScaledOffset = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstantOne = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regTexAddr = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regMask5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regMask6 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstant5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regShifted5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regShifted11 = cg_virtual_reg_create(procedure, cg_reg_type_general);

				cg_create_inst_load_immed(block, cg_op_ldi,	regConstantOne, 1);
				cg_create_inst_binary(block, cg_op_lsl,	regScaledOffset, regTexOffset, regConstantOne);
				cg_create_inst_binary(block, cg_op_add,	regTexAddr, regScaledOffset, fragmentInfo.regTextureData);
				cg_create_inst_load(block, cg_op_ldh,	regTexColor565, regTexAddr);
				cg_create_inst_load_immed(block, cg_op_ldi,	regTexColorA, 0x100);
				cg_create_inst_load_immed(block, cg_op_ldi,	regConstant5, 5);
				cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
				cg_create_inst_binary(block, cg_op_lsr,	regShifted5, regTexColor565, regConstant5);
				cg_create_inst_binary(block, cg_op_lsr,	regShifted11, regTexColor565, regConstant11);
				cg_create_inst_load_immed(block, cg_op_ldi,	regMask5, 0x1f);
				cg_create_inst_load_immed(block, cg_op_ldi,	regMask6, 0x3f);
				cg_create_inst_binary(block, cg_op_and,	regTexColorR, regTexColor565, regMask5);
				cg_create_inst_binary(block, cg_op_and,	regTexColorG, regShifted5, regMask6);
				cg_create_inst_binary(block, cg_op_and,	regTexColorB, regShifted11, regMask5);

				}
				break;

			case Texture::TextureFormatRGBA:					// 5-5-5-1
				//texColor = Color::From5551(reinterpret_cast<const U16 *>(data)[texOffset]);
				{
				cg_virtual_reg_t * regTexData = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regScaledOffset = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstantOne = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regTexAddr = cg_virtual_reg_create(procedure, cg_reg_type_general);

				cg_create_inst_load_immed(block, cg_op_ldi,	regConstantOne, 1);
				cg_create_inst_binary(block, cg_op_lsl,	regScaledOffset, regTexOffset, regConstantOne);
				cg_create_inst_binary(block, cg_op_add,	regTexAddr, regScaledOffset, fragmentInfo.regTextureData);
				cg_create_inst_load(block, cg_op_ldh,	regTexData, regTexAddr);

				regTexColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstant7 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regShifted7 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regMask100 = cg_virtual_reg_create(procedure, cg_reg_type_general);

				cg_create_inst_load_immed(block, cg_op_ldi,	regConstant7, 7);
				cg_create_inst_load_immed(block, cg_op_ldi,	regMask100, 0x100);
				cg_create_inst_binary(block, cg_op_lsr,	regShifted7, regTexData, regConstant7);
				cg_create_inst_binary(block, cg_op_and,	regTexColorA, regShifted7, regMask100);

				cg_virtual_reg_t * regMask5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regMask51 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstant4 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstant10 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regShifted4 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regShifted10 = cg_virtual_reg_create(procedure, cg_reg_type_general);

				cg_create_inst_load_immed(block, cg_op_ldi,	regMask5, 0x1f);
				cg_create_inst_binary(block, cg_op_and,	regTexColorR, regTexData, regMask5);
				cg_create_inst_load_immed(block, cg_op_ldi,	regMask51, 0x3e);
				cg_create_inst_load_immed(block, cg_op_ldi,	regConstant4, 4);
				cg_create_inst_load_immed(block, cg_op_ldi,	regConstant10, 10);
				cg_create_inst_binary(block, cg_op_lsr,	regShifted4, regTexData, regConstant4);
				cg_create_inst_binary(block, cg_op_and,	regTexColorG, regShifted4, regMask51);
				cg_create_inst_binary(block, cg_op_lsr,	regShifted10, regTexData, regConstant10);
				cg_create_inst_binary(block, cg_op_and,	regTexColorB, regShifted10, regMask5);

				regTexColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstant5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regShiftedB = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regShiftedG = cg_virtual_reg_create(procedure, cg_reg_type_general);
				cg_virtual_reg_t * regRG = cg_virtual_reg_create(procedure, cg_reg_type_general);

				cg_create_inst_load_immed(block, cg_op_ldi,	regConstant5, 5);
				cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
				cg_create_inst_binary(block, cg_op_lsl,	regShiftedB, regTexColorB, regConstant11);
				cg_create_inst_binary(block, cg_op_lsl,	regShiftedG, regTexColorG, regConstant5);
				cg_create_inst_binary(block, cg_op_or,	regRG, regTexColorR, regShiftedG);
				cg_create_inst_binary(block, cg_op_or,	regTexColor565, regRG, regShiftedB);
				}
				break;

			default:
				//texColor = Color(0xff, 0xff, 0xff, 0x100);
				{
				regTexColorR = regTexColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

				cg_create_inst_load_immed(block, cg_op_ldi,	regTexColorR, 0x1f);
				cg_create_inst_load_immed(block, cg_op_ldi,	regTexColorG, 0x3f);
				cg_create_inst_load_immed(block, cg_op_ldi,	regTexColorA, 0x100);
				cg_create_inst_load_immed(block, cg_op_ldi,	regTexColor565, 0xffff);
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
						cg_virtual_reg_t * regConstant31 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant63 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant16 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regScaledR = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regScaledG = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regScaledB = cg_virtual_reg_create(procedure, cg_reg_type_general);

						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant31, 0x1f);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant63, 0x3f);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant16, 0x10);

						cg_create_inst_binary(block, cg_op_mul,	regScaledR, fragmentInfo.regR, regConstant31);
						cg_create_inst_binary(block, cg_op_lsr,	regColorR, regScaledR, regConstant16);
						cg_create_inst_binary(block, cg_op_mul,	regScaledG, fragmentInfo.regG, regConstant63);
						cg_create_inst_binary(block, cg_op_lsr,	regColorG, regScaledG, regConstant16);
						cg_create_inst_binary(block, cg_op_mul,	regScaledB, fragmentInfo.regB, regConstant31);
						cg_create_inst_binary(block, cg_op_lsr,	regColorB, regScaledB, regConstant16);

						cg_virtual_reg_t * regConstant5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regShiftedB = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regShiftedG = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regRG = cg_virtual_reg_create(procedure, cg_reg_type_general);

						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant5, 5);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
						cg_create_inst_binary(block, cg_op_lsl,	regShiftedB, regTexColorB, regConstant11);
						cg_create_inst_binary(block, cg_op_lsl,	regShiftedG, regTexColorG, regConstant5);
						cg_create_inst_binary(block, cg_op_or,	regRG, regTexColorR, regShiftedG);
						cg_create_inst_binary(block, cg_op_or,	regColor565, regRG, regShiftedB);

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
						cg_virtual_reg_t * regConstant31 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant63 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant16 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regScaledR = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regScaledG = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regScaledB = cg_virtual_reg_create(procedure, cg_reg_type_general);

						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant31, 0x1f);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant63, 0x3f);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant16, 0x10);

						cg_create_inst_binary(block, cg_op_mul,	regScaledR, fragmentInfo.regR, regConstant31);
						cg_create_inst_binary(block, cg_op_lsr,	regColorR, regScaledR, regConstant16);
						cg_create_inst_binary(block, cg_op_mul,	regScaledG, fragmentInfo.regG, regConstant63);
						cg_create_inst_binary(block, cg_op_lsr,	regColorG, regScaledG, regConstant16);
						cg_create_inst_binary(block, cg_op_mul,	regScaledB, fragmentInfo.regB, regConstant31);
						cg_create_inst_binary(block, cg_op_lsr,	regColorB, regScaledB, regConstant16);

						cg_virtual_reg_t * regConstant5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regShiftedB = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regShiftedG = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regRG = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant5, 5);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
						cg_create_inst_binary(block, cg_op_lsl,	regShiftedB, regTexColorB, regConstant11);
						cg_create_inst_binary(block, cg_op_lsl,	regShiftedG, regTexColorG, regConstant5);
						cg_create_inst_binary(block, cg_op_or,	regRG, regTexColorR, regShiftedG);
						cg_create_inst_binary(block, cg_op_or,	regColor565, regRG, regShiftedB);

						cg_virtual_reg_t * regAlphaProduct = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);

						cg_create_inst_binary(block, cg_op_mul,	regAlphaProduct, fragmentInfo.regA, regTexColorA);
						cg_create_inst_binary(block, cg_op_lsr,	regColorA, regAlphaProduct, regConstant16);
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
						cg_virtual_reg_t * regConstant8 = cg_virtual_reg_create(procedure, cg_reg_type_general);

						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant8, 8);
						cg_create_inst_binary(block, cg_op_lsr,	regColorA, fragmentInfo.regA, regConstant8);
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
						cg_virtual_reg_t * regConstant8 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant16 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regScaledR = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regScaledG = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regScaledB = cg_virtual_reg_create(procedure, cg_reg_type_general);

						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant8, 8);
						cg_create_inst_binary(block, cg_op_lsr,	regColorA, fragmentInfo.regA, regConstant8);

						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant16, 16);
						cg_create_inst_binary(block, cg_op_mul,	regScaledR, regTexColorR, fragmentInfo.regR);
						cg_create_inst_binary(block, cg_op_lsr,	regColorR, regScaledR, regConstant16);
						cg_create_inst_binary(block, cg_op_mul,	regScaledG, regTexColorG, fragmentInfo.regG);
						cg_create_inst_binary(block, cg_op_lsr,	regColorG, regScaledG, regConstant16);
						cg_create_inst_binary(block, cg_op_mul,	regScaledB, regTexColorB, fragmentInfo.regB);
						cg_create_inst_binary(block, cg_op_lsr,	regColorB, regScaledB, regConstant16);

						cg_virtual_reg_t * regConstant5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regShiftedB = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regShiftedG = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regRG = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant5, 5);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
						cg_create_inst_binary(block, cg_op_lsl,	regShiftedB, regColorB, regConstant11);
						cg_create_inst_binary(block, cg_op_lsl,	regShiftedG, regColorG, regConstant5);
						cg_create_inst_binary(block, cg_op_or,	regRG, regColorR, regShiftedG);
						cg_create_inst_binary(block, cg_op_or,	regColor565, regRG, regShiftedB);
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
						cg_virtual_reg_t * regConstant2 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant3 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant8 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant15 = cg_virtual_reg_create(procedure, cg_reg_type_general);

						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant2, 2);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant3, 3);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant8, 8);
						cg_create_inst_binary(block, cg_op_lsr,	regColorA, fragmentInfo.regA, regConstant8);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant15, 15);

						// red component
						{
							regColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShifted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regColorAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_binary(block, cg_op_lsr,	regShifted, fragmentInfo.regR, regConstant15);
							cg_create_inst_binary(block, cg_op_sub,	regAdjusted, fragmentInfo.regR, regShifted);
							cg_create_inst_binary(block, cg_op_lsr,	regColorAdjusted, regAdjusted, regConstant3);

							cg_virtual_reg_t * regShiftedAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t *	regDiff = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regTexEnv = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regProduct = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regDifference = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_binary(block, cg_op_lsr,	regShiftedAdjusted, regAdjusted, regConstant8);
							cg_create_inst_load_immed(block, cg_op_ldi,	regTexEnv, m_State->m_TexEnvColor.r);
							cg_create_inst_binary(block, cg_op_sub,	regDiff, regShiftedAdjusted, regTexEnv);
							cg_create_inst_binary(block, cg_op_mul,	regProduct, regDiff, regTexColorR);
							cg_create_inst_binary(block, cg_op_sub,	regDifference, regColorAdjusted, regProduct);
							cg_create_inst_binary(block, cg_op_lsr,	regColorR, regDifference, regConstant8);
						}

						// green component
						{
							regColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShifted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regColorAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_binary(block, cg_op_lsr,	regShifted, fragmentInfo.regG, regConstant15);
							cg_create_inst_binary(block, cg_op_sub,	regAdjusted, fragmentInfo.regG, regShifted);
							cg_create_inst_binary(block, cg_op_lsr,	regColorAdjusted, regAdjusted, regConstant2);

							cg_virtual_reg_t * regShiftedAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t *	regDiff = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regTexEnv = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regProduct = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regDifference = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_binary(block, cg_op_lsr,	regShiftedAdjusted, regAdjusted, regConstant8);
							cg_create_inst_load_immed(block, cg_op_ldi,	regTexEnv, m_State->m_TexEnvColor.g);
							cg_create_inst_binary(block, cg_op_sub,	regDiff, regShiftedAdjusted, regTexEnv);
							cg_create_inst_binary(block, cg_op_mul,	regProduct, regDiff, regTexColorG);
							cg_create_inst_binary(block, cg_op_sub,	regDifference, regColorAdjusted, regProduct);
							cg_create_inst_binary(block, cg_op_lsr,	regColorG, regDifference, regConstant8);
						}

						// blue component
						{
							regColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShifted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regColorAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_binary(block, cg_op_lsr,	regShifted, fragmentInfo.regB, regConstant15);
							cg_create_inst_binary(block, cg_op_sub,	regAdjusted, fragmentInfo.regB, regShifted);
							cg_create_inst_binary(block, cg_op_lsr,	regColorAdjusted, regAdjusted, regConstant3);

							cg_virtual_reg_t * regShiftedAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t *	regDiff = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regTexEnv = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regProduct = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regDifference = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_binary(block, cg_op_lsr,	regShiftedAdjusted, regAdjusted, regConstant8);
							cg_create_inst_load_immed(block, cg_op_ldi,	regTexEnv, m_State->m_TexEnvColor.b);
							cg_create_inst_binary(block, cg_op_sub,	regDiff, regShiftedAdjusted, regTexEnv);
							cg_create_inst_binary(block, cg_op_mul,	regProduct, regDiff, regTexColorB);
							cg_create_inst_binary(block, cg_op_sub,	regDifference, regColorAdjusted, regProduct);
							cg_create_inst_binary(block, cg_op_lsr,	regColorB, regDifference, regConstant8);
						}

						// create RGB 565 representation
						{
							cg_virtual_reg_t * regConstant5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShiftedB = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShiftedG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regRG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant5, 5);
							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
							cg_create_inst_binary(block, cg_op_lsl,	regShiftedB, regColorB, regConstant11);
							cg_create_inst_binary(block, cg_op_lsl,	regShiftedG, regColorG, regConstant5);
							cg_create_inst_binary(block, cg_op_or,	regRG, regColorR, regShiftedG);
							cg_create_inst_binary(block, cg_op_or,	regColor565, regRG, regShiftedB);
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
						cg_virtual_reg_t * regConstant10 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant8 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant255 = cg_virtual_reg_create(procedure, cg_reg_type_general);

						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant10, 10);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant8, 8);
						cg_create_inst_binary(block, cg_op_lsr,	regColorA, fragmentInfo.regA, regConstant8);

						// R channel
						{
							regColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regScaledR = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShiftedR = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regSumR = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regNeedsClampingR = cg_virtual_reg_create(procedure, cg_reg_type_flags);

							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant255, 0x1f);
							cg_create_inst_binary(block, cg_op_lsr,	regShiftedR, fragmentInfo.regR, regConstant11);
							cg_create_inst_binary(block, cg_op_add,	regSumR, regShiftedR, regTexColorR);
							cg_create_inst_compare(block, cg_op_cmp,	regNeedsClampingR, regSumR, regConstant255);

							cg_block_ref_t * noClampingR = cg_block_ref_create(procedure);
							cg_virtual_reg_t * regClampedR = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_branch_cond(block, cg_op_ble,	regNeedsClampingR, noClampingR);
							cg_create_inst_load_immed(block, cg_op_ldi,	regClampedR, 0x1f);

							block = cg_block_create(procedure);
							noClampingR->block = block;

							cg_create_inst_phi(block, cg_op_phi,	regColorR, cg_create_virtual_reg_list(procedure->module->heap, regClampedR, regSumR, NULL));
						}
						// G channel
						{
							regColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regScaledG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShiftedG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regSumG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regNeedsClampingG = cg_virtual_reg_create(procedure, cg_reg_type_flags);

							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant255, 0x3f);
							cg_create_inst_binary(block, cg_op_lsr,	regShiftedG, fragmentInfo.regG, regConstant10);
							cg_create_inst_binary(block, cg_op_add,	regSumG, regShiftedG, regTexColorR);
							cg_create_inst_compare(block, cg_op_cmp,	regNeedsClampingG, regSumG, regConstant255);

							cg_block_ref_t * noClampingG = cg_block_ref_create(procedure);
							cg_virtual_reg_t * regClampedG = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_branch_cond(block, cg_op_ble,	regNeedsClampingG, noClampingG);
							cg_create_inst_load_immed(block, cg_op_ldi,	regClampedG, 0x3f);

							block = cg_block_create(procedure);
							noClampingG->block = block;

							cg_create_inst_phi(block, cg_op_phi,	regColorG, cg_create_virtual_reg_list(procedure->module->heap, regClampedG, regSumG, NULL));
						}
						// B channel
						{
							regColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regScaledB = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShiftedB = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regSumB = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regNeedsClampingB = cg_virtual_reg_create(procedure, cg_reg_type_flags);

							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant255, 0x1f);
							cg_create_inst_binary(block, cg_op_lsr,	regShiftedB, fragmentInfo.regB, regConstant10);
							cg_create_inst_binary(block, cg_op_add,	regSumB, regShiftedB, regTexColorR);
							cg_create_inst_compare(block, cg_op_cmp,	regNeedsClampingB, regSumB, regConstant255);

							cg_block_ref_t * noClampingB = cg_block_ref_create(procedure);
							cg_virtual_reg_t * regClampedB = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_branch_cond(block, cg_op_ble,	regNeedsClampingB, noClampingB);
							cg_create_inst_load_immed(block, cg_op_ldi,	regClampedB, 0x1f);

							block = cg_block_create(procedure);
							noClampingB->block = block;

							cg_create_inst_phi(block, cg_op_phi,	regColorB, cg_create_virtual_reg_list(procedure->module->heap, regClampedB, regSumB, NULL));
						}
						// create RGB 565 representation
						{
							cg_virtual_reg_t * regConstant5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShiftedB = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShiftedG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regRG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant5, 5);
							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
							cg_create_inst_binary(block, cg_op_lsl,	regShiftedB, regColorB, regConstant11);
							cg_create_inst_binary(block, cg_op_lsl,	regShiftedG, regColorG, regConstant5);
							cg_create_inst_binary(block, cg_op_or,	regRG, regColorR, regShiftedG);
							cg_create_inst_binary(block, cg_op_or,	regColor565, regRG, regShiftedB);
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
						cg_virtual_reg_t * regConstant8 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant16 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regScaledR = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regScaledG = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regScaledB = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regAlphaProduct = cg_virtual_reg_create(procedure, cg_reg_type_general);

						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant16, 16);
						cg_create_inst_binary(block, cg_op_mul,	regAlphaProduct, fragmentInfo.regA, regTexColorA);
						cg_create_inst_binary(block, cg_op_lsr,	regColorA, regAlphaProduct, regConstant16);

						cg_create_inst_binary(block, cg_op_mul,	regScaledR, regTexColorR, fragmentInfo.regR);
						cg_create_inst_binary(block, cg_op_lsr,	regColorR, regScaledR, regConstant16);
						cg_create_inst_binary(block, cg_op_mul,	regScaledG, regTexColorG, fragmentInfo.regG);
						cg_create_inst_binary(block, cg_op_lsr,	regColorG, regScaledG, regConstant16);
						cg_create_inst_binary(block, cg_op_mul,	regScaledB, regTexColorB, fragmentInfo.regB);
						cg_create_inst_binary(block, cg_op_lsr,	regColorB, regScaledB, regConstant16);

						cg_virtual_reg_t * regConstant5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regShiftedB = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regShiftedG = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regRG = cg_virtual_reg_create(procedure, cg_reg_type_general);
						regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant5, 5);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
						cg_create_inst_binary(block, cg_op_lsl,	regShiftedB, regColorB, regConstant11);
						cg_create_inst_binary(block, cg_op_lsl,	regShiftedG, regColorG, regConstant5);
						cg_create_inst_binary(block, cg_op_or,	regRG, regColorR, regShiftedG);
						cg_create_inst_binary(block, cg_op_or,	regColor565, regRG, regShiftedB);

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
						cg_virtual_reg_t * regConstant2 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant3 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant8 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant10 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant15 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regShiftedA = cg_virtual_reg_create(procedure, cg_reg_type_general);

						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant2, 2);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant3, 3);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant8, 8);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant10, 10);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant15, 15);
						cg_create_inst_binary(block, cg_op_lsr,	regShiftedA, fragmentInfo.regA, regConstant15);
						cg_create_inst_binary(block, cg_op_sub,	regColorA, fragmentInfo.regA, regShiftedA);

						// red component
						{
							cg_virtual_reg_t * regShifted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regToColor = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regDeltaColor = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regProduct = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regColorShift = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regDelta = cg_virtual_reg_create(procedure, cg_reg_type_general);
							regColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_binary(block, cg_op_lsr,	regShifted, fragmentInfo.regR, regConstant15);
							cg_create_inst_binary(block, cg_op_sub,	regAdjusted, fragmentInfo.regR, regShifted);
							cg_create_inst_binary(block, cg_op_lsr,	regToColor, regAdjusted, regConstant11);
							cg_create_inst_binary(block, cg_op_sub,	regDeltaColor, regTexColorR, regToColor);
							cg_create_inst_binary(block, cg_op_mul,	regProduct, regDeltaColor, regTexColorA);	
							cg_create_inst_binary(block, cg_op_lsr,	regColorShift, regAdjusted, regConstant3);
							cg_create_inst_binary(block, cg_op_add,	regDelta, regColorShift, regProduct);
							cg_create_inst_binary(block, cg_op_asr,	regColorR, regDelta, regConstant8);
						}

						// green component
						{
							cg_virtual_reg_t * regShifted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regToColor = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regDeltaColor = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regProduct = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regColorShift = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regDelta = cg_virtual_reg_create(procedure, cg_reg_type_general);
							regColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_binary(block, cg_op_lsr,	regShifted, fragmentInfo.regG, regConstant15);
							cg_create_inst_binary(block, cg_op_sub,	regAdjusted, fragmentInfo.regG, regShifted);
							cg_create_inst_binary(block, cg_op_lsr,	regToColor, regAdjusted, regConstant10);
							cg_create_inst_binary(block, cg_op_sub,	regDeltaColor, regTexColorG, regToColor);
							cg_create_inst_binary(block, cg_op_mul,	regProduct, regDeltaColor, regTexColorA);	
							cg_create_inst_binary(block, cg_op_lsr,	regColorShift, regAdjusted, regConstant2);
							cg_create_inst_binary(block, cg_op_add,	regDelta, regColorShift, regProduct);
							cg_create_inst_binary(block, cg_op_asr,	regColorG, regDelta, regConstant8);
						}

						// blue component
						{
							cg_virtual_reg_t * regShifted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regToColor = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regDeltaColor = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regProduct = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regColorShift = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regDelta = cg_virtual_reg_create(procedure, cg_reg_type_general);
							regColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_binary(block, cg_op_lsr,	regShifted, fragmentInfo.regB, regConstant15);
							cg_create_inst_binary(block, cg_op_sub,	regAdjusted, fragmentInfo.regB, regShifted);
							cg_create_inst_binary(block, cg_op_lsr,	regToColor, regAdjusted, regConstant11);
							cg_create_inst_binary(block, cg_op_sub,	regDeltaColor, regTexColorB, regToColor);
							cg_create_inst_binary(block, cg_op_mul,	regProduct, regDeltaColor, regTexColorA);	
							cg_create_inst_binary(block, cg_op_lsr,	regColorShift, regAdjusted, regConstant3);
							cg_create_inst_binary(block, cg_op_add,	regDelta, regColorShift, regProduct);
							cg_create_inst_binary(block, cg_op_asr,	regColorB, regDelta, regConstant8);
						}

						// create RGB 565 representation
						{
							cg_virtual_reg_t * regConstant5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShiftedB = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShiftedG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regRG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant5, 5);
							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
							cg_create_inst_binary(block, cg_op_lsl,	regShiftedB, regColorB, regConstant11);
							cg_create_inst_binary(block, cg_op_lsl,	regShiftedG, regColorG, regConstant5);
							cg_create_inst_binary(block, cg_op_or,	regRG, regColorR, regShiftedG);
							cg_create_inst_binary(block, cg_op_or,	regColor565, regRG, regShiftedB);
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
						cg_virtual_reg_t * regConstant2 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant3 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant8 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant15 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant16 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regAlphaProduct = cg_virtual_reg_create(procedure, cg_reg_type_general);

						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant2, 2);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant3, 3);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant8, 8);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant16, 16);
						cg_create_inst_binary(block, cg_op_mul,	regAlphaProduct, fragmentInfo.regA, regTexColorA);
						cg_create_inst_binary(block, cg_op_lsr,	regColorA, regAlphaProduct, regConstant16);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant15, 15);

						// red component
						{
							regColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShifted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regColorAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_binary(block, cg_op_lsr,	regShifted, fragmentInfo.regR, regConstant15);
							cg_create_inst_binary(block, cg_op_sub,	regAdjusted, fragmentInfo.regR, regShifted);
							cg_create_inst_binary(block, cg_op_lsr,	regColorAdjusted, regAdjusted, regConstant3);

							cg_virtual_reg_t * regShiftedAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t *	regDiff = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regTexEnv = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regProduct = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regDifference = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_binary(block, cg_op_lsr,	regShiftedAdjusted, regAdjusted, regConstant8);
							cg_create_inst_load_immed(block, cg_op_ldi,	regTexEnv, m_State->m_TexEnvColor.r);
							cg_create_inst_binary(block, cg_op_sub,	regDiff, regShiftedAdjusted, regTexEnv);
							cg_create_inst_binary(block, cg_op_mul,	regProduct, regDiff, regTexColorR);
							cg_create_inst_binary(block, cg_op_sub,	regDifference, regColorAdjusted, regProduct);
							cg_create_inst_binary(block, cg_op_lsr,	regColorR, regDifference, regConstant8);
						}

						// green component
						{
							regColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShifted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regColorAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_binary(block, cg_op_lsr,	regShifted, fragmentInfo.regG, regConstant15);
							cg_create_inst_binary(block, cg_op_sub,	regAdjusted, fragmentInfo.regG, regShifted);
							cg_create_inst_binary(block, cg_op_lsr,	regColorAdjusted, regAdjusted, regConstant2);

							cg_virtual_reg_t * regShiftedAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t *	regDiff = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regTexEnv = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regProduct = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regDifference = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_binary(block, cg_op_lsr,	regShiftedAdjusted, regAdjusted, regConstant8);
							cg_create_inst_load_immed(block, cg_op_ldi,	regTexEnv, m_State->m_TexEnvColor.g);
							cg_create_inst_binary(block, cg_op_sub,	regDiff, regShiftedAdjusted, regTexEnv);
							cg_create_inst_binary(block, cg_op_mul,	regProduct, regDiff, regTexColorG);
							cg_create_inst_binary(block, cg_op_sub,	regDifference, regColorAdjusted, regProduct);
							cg_create_inst_binary(block, cg_op_lsr,	regColorG, regDifference, regConstant8);
						}

						// blue component
						{
							regColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShifted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regColorAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_binary(block, cg_op_lsr,	regShifted, fragmentInfo.regB, regConstant15);
							cg_create_inst_binary(block, cg_op_sub,	regAdjusted, fragmentInfo.regB, regShifted);
							cg_create_inst_binary(block, cg_op_lsr,	regColorAdjusted, regAdjusted, regConstant3);

							cg_virtual_reg_t * regShiftedAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t *	regDiff = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regTexEnv = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regProduct = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regDifference = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_binary(block, cg_op_lsr,	regShiftedAdjusted, regAdjusted, regConstant8);
							cg_create_inst_load_immed(block, cg_op_ldi,	regTexEnv, m_State->m_TexEnvColor.b);
							cg_create_inst_binary(block, cg_op_sub,	regDiff, regShiftedAdjusted, regTexEnv);
							cg_create_inst_binary(block, cg_op_mul,	regProduct, regDiff, regTexColorB);
							cg_create_inst_binary(block, cg_op_sub,	regDifference, regColorAdjusted, regProduct);
							cg_create_inst_binary(block, cg_op_lsr,	regColorB, regDifference, regConstant8);
						}

						// create RGB 565 representation
						{
							cg_virtual_reg_t * regConstant5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShiftedB = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShiftedG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regRG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant5, 5);
							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
							cg_create_inst_binary(block, cg_op_lsl,	regShiftedB, regColorB, regConstant11);
							cg_create_inst_binary(block, cg_op_lsl,	regShiftedG, regColorG, regConstant5);
							cg_create_inst_binary(block, cg_op_or,	regRG, regColorR, regShiftedG);
							cg_create_inst_binary(block, cg_op_or,	regColor565, regRG, regShiftedB);
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
						cg_virtual_reg_t * regConstant10 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant16 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant8 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant255 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regAlphaProduct = cg_virtual_reg_create(procedure, cg_reg_type_general);

						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant10, 10);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant8, 8);

						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant16, 16);
						cg_create_inst_binary(block, cg_op_mul,	regAlphaProduct, fragmentInfo.regA, regTexColorA);
						cg_create_inst_binary(block, cg_op_lsr,	regColorA, regAlphaProduct, regConstant16);

						// R channel
						{
							regColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regScaledR = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShiftedR = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regSumR = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regNeedsClampingR = cg_virtual_reg_create(procedure, cg_reg_type_flags);

							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant255, 0x1f);
							cg_create_inst_binary(block, cg_op_lsr,	regShiftedR, fragmentInfo.regR, regConstant11);
							cg_create_inst_binary(block, cg_op_add,	regSumR, regShiftedR, regTexColorR);
							cg_create_inst_compare(block, cg_op_cmp,	regNeedsClampingR, regSumR, regConstant255);

							cg_block_ref_t * noClampingR = cg_block_ref_create(procedure);
							cg_virtual_reg_t * regClampedR = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_branch_cond(block, cg_op_ble,	regNeedsClampingR, noClampingR);
							cg_create_inst_load_immed(block, cg_op_ldi,	regClampedR, 0x1f);

							block = cg_block_create(procedure);
							noClampingR->block = block;

							cg_create_inst_phi(block, cg_op_phi,	regColorR, cg_create_virtual_reg_list(procedure->module->heap, regClampedR, regSumR, NULL));
						}
						// G channel
						{
							regColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regScaledG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShiftedG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regSumG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regNeedsClampingG = cg_virtual_reg_create(procedure, cg_reg_type_flags);

							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant255, 0x3f);
							cg_create_inst_binary(block, cg_op_lsr,	regShiftedG, fragmentInfo.regG, regConstant10);
							cg_create_inst_binary(block, cg_op_add,	regSumG, regShiftedG, regTexColorR);
							cg_create_inst_compare(block, cg_op_cmp,	regNeedsClampingG, regSumG, regConstant255);

							cg_block_ref_t * noClampingG = cg_block_ref_create(procedure);
							cg_virtual_reg_t * regClampedG = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_branch_cond(block, cg_op_ble,	regNeedsClampingG, noClampingG);
							cg_create_inst_load_immed(block, cg_op_ldi,	regClampedG, 0x3f);

							block = cg_block_create(procedure);
							noClampingG->block = block;

							cg_create_inst_phi(block, cg_op_phi,	regColorG, cg_create_virtual_reg_list(procedure->module->heap, regClampedG, regSumG, NULL));
						}
						// B channel
						{
							regColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regScaledB = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShiftedB = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regSumB = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regNeedsClampingB = cg_virtual_reg_create(procedure, cg_reg_type_flags);

							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant255, 0x1f);
							cg_create_inst_binary(block, cg_op_lsr,	regShiftedB, fragmentInfo.regB, regConstant10);
							cg_create_inst_binary(block, cg_op_add,	regSumB, regShiftedB, regTexColorR);
							cg_create_inst_compare(block, cg_op_cmp,	regNeedsClampingB, regSumB, regConstant255);

							cg_block_ref_t * noClampingB = cg_block_ref_create(procedure);
							cg_virtual_reg_t * regClampedB = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_branch_cond(block, cg_op_ble,	regNeedsClampingB, noClampingB);
							cg_create_inst_load_immed(block, cg_op_ldi,	regClampedB, 0x1f);

							block = cg_block_create(procedure);
							noClampingB->block = block;

							cg_create_inst_phi(block, cg_op_phi,	regColorB, cg_create_virtual_reg_list(procedure->module->heap, regClampedB, regSumB, NULL));
						}
						// create RGB 565 representation
						{
							cg_virtual_reg_t * regConstant5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShiftedB = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regShiftedG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regRG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant5, 5);
							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
							cg_create_inst_binary(block, cg_op_lsl,	regShiftedB, regColorB, regConstant11);
							cg_create_inst_binary(block, cg_op_lsl,	regShiftedG, regColorG, regConstant5);
							cg_create_inst_binary(block, cg_op_or,	regRG, regColorR, regShiftedG);
							cg_create_inst_binary(block, cg_op_or,	regColor565, regRG, regShiftedB);
						}
						}

						break;
				}
				break;
		}
	} else {
		// color = baseColor
		cg_virtual_reg_t * regConstant16 = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regConstant10 = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regConstant8 = cg_virtual_reg_create(procedure, cg_reg_type_general);

		cg_create_inst_load_immed(block, cg_op_ldi,	regConstant16, 16);
		cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
		cg_create_inst_load_immed(block, cg_op_ldi,	regConstant10, 10);
		cg_create_inst_load_immed(block, cg_op_ldi,	regConstant8, 8);

		//  needs to be converted to 565 format, alpha to 8 bits
		{
			regColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
			cg_virtual_reg_t * regShifted = cg_virtual_reg_create(procedure, cg_reg_type_general);
			cg_virtual_reg_t * regAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);

			cg_create_inst_binary(block, cg_op_lsr,	regShifted, fragmentInfo.regR, regConstant16);
			cg_create_inst_binary(block, cg_op_sub,	regAdjusted, fragmentInfo.regR, regShifted);
			cg_create_inst_binary(block, cg_op_lsr,	regColorR, regAdjusted, regConstant11);
		}

		{
			regColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
			cg_virtual_reg_t * regShifted = cg_virtual_reg_create(procedure, cg_reg_type_general);
			cg_virtual_reg_t * regAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);

			cg_create_inst_binary(block, cg_op_lsr,	regShifted, fragmentInfo.regG, regConstant16);
			cg_create_inst_binary(block, cg_op_sub,	regAdjusted, fragmentInfo.regG, regShifted);
			cg_create_inst_binary(block, cg_op_lsr,	regColorG, regAdjusted, regConstant10);
		}

		{
			regColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
			cg_virtual_reg_t * regShifted = cg_virtual_reg_create(procedure, cg_reg_type_general);
			cg_virtual_reg_t * regAdjusted = cg_virtual_reg_create(procedure, cg_reg_type_general);

			cg_create_inst_binary(block, cg_op_lsr,	regShifted, fragmentInfo.regB, regConstant16);
			cg_create_inst_binary(block, cg_op_sub,	regAdjusted, fragmentInfo.regB, regShifted);
			cg_create_inst_binary(block, cg_op_lsr,	regColorB, regAdjusted, regConstant11);
		}
		{
			regColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
			cg_create_inst_binary(block, cg_op_lsr,	regColorA, fragmentInfo.regA, regConstant8);
		}
		// create RGB 565 representation
		cg_virtual_reg_t * regConstant5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regShiftedB = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regShiftedG = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regRG = cg_virtual_reg_create(procedure, cg_reg_type_general);
		regColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

		cg_create_inst_load_immed(block, cg_op_ldi,	regConstant5, 5);
		cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
		cg_create_inst_binary(block, cg_op_lsl,	regShiftedB, regColorB, regConstant11);
		cg_create_inst_binary(block, cg_op_lsl,	regShiftedG, regColorG, regConstant5);
		cg_create_inst_binary(block, cg_op_or,	regRG, regColorR, regShiftedG);
		cg_create_inst_binary(block, cg_op_or,	regColor565, regRG, regShiftedB);
	}

	// fog
	if (m_State->m_FogEnabled) {
		//color = Color::Blend(color, m_State->m_FogColor, fogDensity);
		cg_virtual_reg_t * regFogColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regFogColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regFogColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);

		cg_create_inst_load_immed(block, cg_op_ldi,	regFogColorR, m_State->m_FogColor.r >> 3);
		cg_create_inst_load_immed(block, cg_op_ldi,	regFogColorG, m_State->m_FogColor.g >> 2);
		cg_create_inst_load_immed(block, cg_op_ldi,	regFogColorB, m_State->m_FogColor.b >> 3);

		cg_virtual_reg_t * regDeltaR = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regDeltaG = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regDeltaB = cg_virtual_reg_create(procedure, cg_reg_type_general);

		cg_create_inst_binary(block, cg_op_sub,	regDeltaR, regColorR, regFogColorR);
		cg_create_inst_binary(block, cg_op_sub,	regDeltaG, regColorG, regFogColorG);
		cg_create_inst_binary(block, cg_op_sub,	regDeltaB, regColorB, regFogColorB);

		cg_virtual_reg_t * regProductR = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regProductG = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regProductB = cg_virtual_reg_create(procedure, cg_reg_type_general);

		cg_create_inst_binary(block, cg_op_mul,	regProductR, regDeltaR, fragmentInfo.regFog);
		cg_create_inst_binary(block, cg_op_mul,	regProductG, regDeltaG, fragmentInfo.regFog);
		cg_create_inst_binary(block, cg_op_mul,	regProductB, regDeltaB, fragmentInfo.regFog);

		cg_virtual_reg_t * regConstant16 = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regShiftedProductR = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regShiftedProductG = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regShiftedProductB = cg_virtual_reg_create(procedure, cg_reg_type_general);

		cg_create_inst_load_immed(block, cg_op_ldi,	regConstant16, 16);
		cg_create_inst_binary(block, cg_op_asr,	regShiftedProductR, regProductR, regConstant16);
		cg_create_inst_binary(block, cg_op_asr,	regShiftedProductG, regProductG, regConstant16);
		cg_create_inst_binary(block, cg_op_asr,	regShiftedProductB, regProductB, regConstant16);

		cg_virtual_reg_t * regNewColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regNewColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regNewColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);

		cg_create_inst_binary(block, cg_op_add,	regNewColorR, regShiftedProductR, regFogColorR);
		cg_create_inst_binary(block, cg_op_add,	regNewColorG, regShiftedProductG, regFogColorG);
		cg_create_inst_binary(block, cg_op_add,	regNewColorB, regShiftedProductB, regFogColorB);

		regColorR = regNewColorR;
		regColorG = regNewColorG;
		regColorB = regNewColorB;

		// create RGB 565 representation
		cg_virtual_reg_t * regConstant5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regShiftedB = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regShiftedG = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regRG = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regNewColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

		cg_create_inst_load_immed(block, cg_op_ldi,	regConstant5, 5);
		cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
		cg_create_inst_binary(block, cg_op_lsl,	regShiftedB, regColorB, regConstant11);
		cg_create_inst_binary(block, cg_op_lsl,	regShiftedG, regColorG, regConstant5);
		cg_create_inst_binary(block, cg_op_or,	regRG, regColorR, regShiftedG);
		cg_create_inst_binary(block, cg_op_or,	regColor565, regRG, regShiftedB);

		regColor565 = regNewColor565;
	}

	if (m_State->m_AlphaTestEnabled) {
		//bool alphaTest;
		//U8 alpha = color.A();
		//U8 alphaRef = EGL_IntFromFixed(m_State->m_AlphaReference * 255);
		cg_virtual_reg_t * regAlphaRef = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regAlphaTest = cg_virtual_reg_create(procedure, cg_reg_type_flags);

		cg_create_inst_load_immed(block, cg_op_ldi,	regAlphaRef, m_State->m_AlphaReference >> 8);
		cg_create_inst_compare(block, cg_op_cmp,	regAlphaTest, regColorA, regAlphaRef);

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
				cg_create_inst_branch_label(block, cg_op_bra,	continuation);
			} else {
				cg_create_inst_branch_cond(block, failedTest, regAlphaTest, continuation);
			}
		}
	}

	if (m_State->m_StencilTestEnabled) {

		//bool stencilTest;
		//U32 stencilRef = m_State->m_StencilReference & m_State->m_StencilMask;
		//U32 stencilValue = m_Surface->GetStencilBuffer()[offset];
		//U32 stencil = stencilValue & m_State->m_StencilMask;
		cg_virtual_reg_t * regStencilRef = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regStencilMask = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regStencilAddr = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regStencil = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regStencilTest = cg_virtual_reg_create(procedure, cg_reg_type_flags);

		cg_create_inst_load_immed(block, cg_op_ldi,	regStencilRef, m_State->m_StencilReference & m_State->m_StencilMask);
		cg_create_inst_load_immed(block, cg_op_ldi,	regStencilMask, m_State->m_StencilMask);
		cg_create_inst_binary(block, cg_op_add,	regStencilAddr, fragmentInfo.regStencilBuffer, regOffset4);
		cg_create_inst_load(block, cg_op_ldw,	regStencilValue, regStencilAddr);
		cg_create_inst_binary(block, cg_op_and,	regStencil, regStencilValue, regStencilMask);
		cg_create_inst_compare(block, cg_op_cmp,	regStencilTest, regStencil, regStencilRef);

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

		cg_create_inst_branch_cond(block, passedTest,	regStencilTest, labelStencilPassed);

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

					cg_create_inst_load_immed(block, cg_op_ldi, regNewStencilValue, 0);
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
						cg_virtual_reg_t * regConstant1 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regFlag = cg_virtual_reg_create(procedure, cg_reg_type_flags);

						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant1, 1);
						cg_create_inst_binary_s(block, cg_op_add,	regNewStencilValue, regFlag, regStencilValue, regConstant1);
						cg_create_inst_branch_cond(block, cg_op_beq,	regFlag, continuation);
					}
					
					break;

				case RasterizerState::StencilOpDecr: 
					//if (stencilValue != 0) {
					//	stencilValue--; 
					//}
					{
						regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant0 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regConstant1 = cg_virtual_reg_create(procedure, cg_reg_type_general);
						cg_virtual_reg_t * regFlag = cg_virtual_reg_create(procedure, cg_reg_type_flags);

						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant0, 0);
						cg_create_inst_compare(block, cg_op_cmp,	regFlag, regStencilValue, regConstant0);
						cg_create_inst_branch_cond(block, cg_op_beq,	regFlag, continuation);
						cg_create_inst_load_immed(block, cg_op_ldi,	regConstant1, 1);
						cg_create_inst_binary(block, cg_op_sub,	regNewStencilValue, regStencilValue, regConstant1);
					}
					
					break;

				case RasterizerState::StencilOpInvert: 
					//stencilValue = ~stencilValue; 
					regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);

					cg_create_inst_unary(block, cg_op_not,	regNewStencilValue, regStencilValue);
					break;
			}

			cg_create_inst_store(block, cg_op_stw,	regStencilAddr, regNewStencilValue);
			cg_create_inst_branch_label(block, cg_op_bra,	continuation);
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
				cg_create_inst_branch_label(currentBlock, cg_op_bra,	labelStencilZTestPassed);
			} else {
				cg_create_inst_branch_cond(currentBlock, branchOnDepthTestPassed, regDepthTest, labelStencilZTestPassed);
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

						cg_create_inst_load_immed(block, cg_op_ldi, regNewStencilValue, 0);
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
							cg_virtual_reg_t * regConstant1 = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regFlag = cg_virtual_reg_create(procedure, cg_reg_type_flags);

							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant1, 1);
							cg_create_inst_binary_s(block, cg_op_add,	regNewStencilValue, regFlag, regStencilValue, regConstant1);

							if (m_State->m_DepthTestEnabled) {
								cg_create_inst_branch_cond(block, cg_op_beq,	regFlag, continuation);
							} else {
								cg_create_inst_branch_cond(block, cg_op_beq,	regFlag, labelStencilBypassed);
							}
						}
						
						break;

					case RasterizerState::StencilOpDecr: 
						//if (stencilValue != 0) {
						//	stencilValue--; 
						//}
						{
							regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regConstant0 = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regConstant1 = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regFlag = cg_virtual_reg_create(procedure, cg_reg_type_flags);

							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant0, 0);
							cg_create_inst_compare(block, cg_op_cmp,	regFlag, regStencilValue, regConstant0);

							if (m_State->m_DepthTestEnabled) {
								cg_create_inst_branch_cond(block, cg_op_beq,	regFlag, continuation);
							} else {
								cg_create_inst_branch_cond(block, cg_op_beq,	regFlag, labelStencilBypassed);
							}

							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant1, 1);
							cg_create_inst_binary(block, cg_op_sub,	regNewStencilValue, regStencilValue, regConstant1);
						}
						
						break;

					case RasterizerState::StencilOpInvert: 
						//stencilValue = ~stencilValue; 
						regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);

						cg_create_inst_unary(block, cg_op_not,	regNewStencilValue, regStencilValue);
						break;
				}

				//m_Surface->GetStencilBuffer()[offset] = stencilValue;
				cg_create_inst_store(block, cg_op_stw,	regStencilAddr, regNewStencilValue);
			//}
			}

			if (m_State->m_DepthTestEnabled) {
				// return;
				cg_create_inst_branch_label(block, cg_op_bra,	continuation);
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

						cg_create_inst_load_immed(block, cg_op_ldi, regNewStencilValue, 0);
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
							cg_virtual_reg_t * regConstant1 = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regFlag = cg_virtual_reg_create(procedure, cg_reg_type_flags);

							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant1, 1);
							cg_create_inst_binary_s(block, cg_op_add,	regNewStencilValue, regFlag, regStencilValue, regConstant1);
							cg_create_inst_branch_cond(block, cg_op_beq,	regFlag, labelStencilBypassed);
						}
						
						break;

					case RasterizerState::StencilOpDecr: 
						//if (stencilValue != 0) {
						//	stencilValue--; 
						//}
						{
							regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regConstant0 = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regConstant1 = cg_virtual_reg_create(procedure, cg_reg_type_general);
							cg_virtual_reg_t * regFlag = cg_virtual_reg_create(procedure, cg_reg_type_flags);

							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant0, 0);
							cg_create_inst_compare(block, cg_op_cmp,	regFlag, regStencilValue, regConstant0);
							cg_create_inst_branch_cond(block, cg_op_beq,	regFlag, labelStencilBypassed);
							cg_create_inst_load_immed(block, cg_op_ldi,	regConstant1, 1);
							cg_create_inst_binary(block, cg_op_sub,	regNewStencilValue, regStencilValue, regConstant1);
						}
						
						break;

					case RasterizerState::StencilOpInvert: 
						//stencilValue = ~stencilValue; 
						regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);

						cg_create_inst_unary(block, cg_op_not,	regNewStencilValue, regStencilValue);
						break;
				}

				//m_Surface->GetStencilBuffer()[offset] = stencilValue;
				cg_create_inst_store(block, cg_op_stw,	regStencilAddr, regNewStencilValue);
			//}
			}

		// stencil test bypassed
		block = cg_block_create(procedure);
		labelStencilBypassed->block = block;
	}

	//U16 dstValue = m_Surface->GetColorBuffer()[offset];
	//U8 dstAlpha = m_Surface->GetAlphaBuffer()[offset];
	cg_virtual_reg_t * regDstValue = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDstAlpha = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regColorAddr = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAlphaAddr = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDstR = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDstG = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDstB = cg_virtual_reg_create(procedure, cg_reg_type_general);

	{
		cg_virtual_reg_t * regConstant5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regConstant11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regShifted5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regShifted11 = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regMask5 = cg_virtual_reg_create(procedure, cg_reg_type_general);
		cg_virtual_reg_t * regMask6 = cg_virtual_reg_create(procedure, cg_reg_type_general);

		cg_create_inst_binary(block, cg_op_add,	regColorAddr, fragmentInfo.regColorBuffer, regOffset2);
		cg_create_inst_binary(block, cg_op_add,	regAlphaAddr, fragmentInfo.regAlphaBuffer, regOffset);
		cg_create_inst_load(block, cg_op_ldh,	regDstValue, regColorAddr);
		cg_create_inst_load(block, cg_op_ldb,	regDstAlpha, regAlphaAddr);
		cg_create_inst_load_immed(block, cg_op_ldi,	regConstant5, 5);
		cg_create_inst_load_immed(block, cg_op_ldi,	regConstant11, 11);
		cg_create_inst_binary(block, cg_op_lsr,	regShifted5, regDstValue, regConstant5);
		cg_create_inst_binary(block, cg_op_lsr,	regShifted11, regDstValue, regConstant11);
		cg_create_inst_load_immed(block, cg_op_ldi,	regMask5, 0x1f);
		cg_create_inst_load_immed(block, cg_op_ldi,	regMask6, 0x3f);
		cg_create_inst_binary(block, cg_op_and,	regDstR, regDstValue, regMask5);
		cg_create_inst_binary(block, cg_op_and,	regDstG, regShifted5, regMask6);
		cg_create_inst_binary(block, cg_op_and,	regDstB, regShifted11, regMask5);
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
		cg_create_inst_store(block, cg_op_stw,	regZBufferAddr, fragmentInfo.regDepth);
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
			cg_create_inst_store(block, cg_op_sth, regColorAddr, regColor565);
		} else {
			//m_Surface->GetColorBuffer()[offset] = maskedColor.ConvertTo565();
			cg_virtual_reg_t * regSrcMask = cg_virtual_reg_create(procedure, cg_reg_type_general);
			cg_virtual_reg_t * regDstMask = cg_virtual_reg_create(procedure, cg_reg_type_general);
			cg_virtual_reg_t * regMaskedSrc = cg_virtual_reg_create(procedure, cg_reg_type_general);
			cg_virtual_reg_t * regMaskedDst = cg_virtual_reg_create(procedure, cg_reg_type_general);
			cg_virtual_reg_t * regCombined = cg_virtual_reg_create(procedure, cg_reg_type_general);

			U32 mask = (m_State->m_MaskRed ? 0x001f : 0) |
				(m_State->m_MaskGreen ? 0x07e0 : 0) |
				(m_State->m_MaskBlue ? 0xF800 : 0);
			cg_create_inst_load_immed(block, cg_op_ldi,	regSrcMask, mask);
			cg_create_inst_load_immed(block, cg_op_ldi,	regDstMask, ~mask);
			cg_create_inst_binary(block, cg_op_and,	regMaskedSrc, regColor565, regSrcMask);
			cg_create_inst_binary(block, cg_op_and,	regMaskedDst, regDstValue, regDstMask);
			cg_create_inst_binary(block, cg_op_or,	regCombined, regMaskedSrc, regMaskedDst);
			cg_create_inst_store(block, cg_op_sth,	regColorAddr, regCombined);
		}


		if (m_State->m_MaskAlpha) {
			//m_Surface->GetAlphaBuffer()[offset] = maskedColor.A();
			cg_virtual_reg_t * regConstant8 = cg_virtual_reg_create(procedure, cg_reg_type_general);
			cg_virtual_reg_t * regShiftedA = cg_virtual_reg_create(procedure, cg_reg_type_general);
			cg_virtual_reg_t * regAdjustedA = cg_virtual_reg_create(procedure, cg_reg_type_general);

			cg_create_inst_load_immed(block, cg_op_ldi,	regConstant8, 8);
			cg_create_inst_binary(block, cg_op_lsr,	regShiftedA, regColorA, regConstant8);
			cg_create_inst_binary(block, cg_op_sub,	regAdjustedA, regColorA, regShiftedA);
			cg_create_inst_store(block, cg_op_stb,	regAlphaAddr, regAdjustedA);
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

	cg_heap_t * heap = cg_heap_create(4096);
	cg_module_t * module = cg_module_create(heap);

	m_Module = module;
	cg_proc_t * procedure = cg_proc_create(module);

	// The signature of the generated function is:
	//	(const RasterInfo * info, const EdgePos& start, const EdgePos& end);
	// Do not pass in y coordinate but rather assume that raster info pointers have been
	// adjusted to point to current scanline in memory
	// In the edge buffers, z, tu and tv are actually divided by w

	cg_virtual_reg_t * regInfo = cg_virtual_reg_create(procedure, cg_reg_type_general);		// virtual register containing info structure pointer
	cg_virtual_reg_t * regStart = cg_virtual_reg_create(procedure, cg_reg_type_general);		// virtual register containing start edge buffer pointer
	cg_virtual_reg_t * regEnd = cg_virtual_reg_create(procedure, cg_reg_type_general);		// virtual register containing end edge buffer pointer

	cg_block_t * block0 = cg_block_create(procedure);

	// Create instructions to calculate addresses of individual fields of
	// edge buffer input arguments

	// texture data, width, height, exponent
	cg_virtual_reg_t * regTextureData =		LOAD_DATA(block0, regInfo, OFFSET_TEXTURE_DATA);
	cg_virtual_reg_t * regTextureWidth =		LOAD_DATA(block0, regInfo, OFFSET_TEXTURE_WIDTH);
	cg_virtual_reg_t * regTextureHeight =		LOAD_DATA(block0, regInfo, OFFSET_TEXTURE_HEIGHT);
	cg_virtual_reg_t * regTextureExponent =	LOAD_DATA(block0, regInfo, OFFSET_TEXTURE_EXPONENT);


	// surface color buffer, depth buffer, alpha buffer, stencil buffer
	cg_virtual_reg_t * regColorBuffer =		LOAD_DATA(block0, regInfo, OFFSET_SURFACE_COLOR_BUFFER);
	cg_virtual_reg_t * regDepthBuffer =		LOAD_DATA(block0, regInfo, OFFSET_SURFACE_DEPTH_BUFFER);
	cg_virtual_reg_t * regAlphaBuffer =		LOAD_DATA(block0, regInfo, OFFSET_SURFACE_ALPHA_BUFFER);
	cg_virtual_reg_t * regStencilBuffer =		LOAD_DATA(block0, regInfo, OFFSET_SURFACE_STENCIL_BUFFER);
	cg_virtual_reg_t * regSurfaceWidth =		LOAD_DATA(block0, regInfo, OFFSET_SURFACE_WIDTH);

	// x coordinate
	cg_virtual_reg_t * regOffsetWindowX = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAddrStartWindowX = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAddrEndWindowX = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load_immed(block0, cg_op_ldi,	regOffsetWindowX, OFFSET_EDGE_BUFFER_WINDOW_X);
	cg_create_inst_binary(block0, cg_op_add,	regAddrStartWindowX, regStart, regOffsetWindowX);
	cg_create_inst_binary(block0, cg_op_add,	regAddrEndWindowX, regEnd, regOffsetWindowX);

	// z coordinate
	cg_virtual_reg_t * regOffsetWindowZ = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAddrStartWindowZ = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAddrEndWindowZ = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load_immed(block0, cg_op_ldi,	regOffsetWindowZ, OFFSET_EDGE_BUFFER_WINDOW_Z);
	cg_create_inst_binary(block0, cg_op_add,	regAddrStartWindowZ, regStart, regOffsetWindowZ);
	cg_create_inst_binary(block0, cg_op_add,	regAddrEndWindowZ, regEnd, regOffsetWindowZ);

	// u texture coordinate
	cg_virtual_reg_t * regOffsetTextureU = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAddrStartTextureU = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAddrEndTextureU = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load_immed(block0, cg_op_ldi,	regOffsetTextureU, OFFSET_EDGE_BUFFER_TEX_TU);
	cg_create_inst_binary(block0, cg_op_add,	regAddrStartTextureU, regStart, regOffsetTextureU);
	cg_create_inst_binary(block0, cg_op_add,	regAddrEndTextureU, regEnd, regOffsetTextureU);

	// v texture coordinate
	cg_virtual_reg_t * regOffsetTextureV = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAddrStartTextureV = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAddrEndTextureV = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load_immed(block0, cg_op_ldi,	regOffsetTextureV, OFFSET_EDGE_BUFFER_TEX_TV);
	cg_create_inst_binary(block0, cg_op_add,	regAddrStartTextureV, regStart, regOffsetTextureV);
	cg_create_inst_binary(block0, cg_op_add,	regAddrEndTextureV, regEnd, regOffsetTextureV);

	// r color component
	cg_virtual_reg_t * regOffsetColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAddrStartColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAddrEndColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load_immed(block0, cg_op_ldi,	regOffsetColorR, OFFSET_EDGE_BUFFER_COLOR_R);
	cg_create_inst_binary(block0, cg_op_add,	regAddrStartColorR, regStart, regOffsetColorR);
	cg_create_inst_binary(block0, cg_op_add,	regAddrEndColorR, regEnd, regOffsetColorR);

	// g color component
	cg_virtual_reg_t * regOffsetColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAddrStartColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAddrEndColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load_immed(block0, cg_op_ldi,	regOffsetColorG, OFFSET_EDGE_BUFFER_COLOR_G);
	cg_create_inst_binary(block0, cg_op_add,	regAddrStartColorG, regStart, regOffsetColorG);
	cg_create_inst_binary(block0, cg_op_add,	regAddrEndColorG, regEnd, regOffsetColorG);

	// b color component
	cg_virtual_reg_t * regOffsetColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAddrStartColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAddrEndColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load_immed(block0, cg_op_ldi,	regOffsetColorB, OFFSET_EDGE_BUFFER_COLOR_B);
	cg_create_inst_binary(block0, cg_op_add,	regAddrStartColorB, regStart, regOffsetColorB);
	cg_create_inst_binary(block0, cg_op_add,	regAddrEndColorB, regEnd, regOffsetColorB);

	// a color component
	cg_virtual_reg_t * regOffsetColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAddrStartColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAddrEndColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load_immed(block0, cg_op_ldi,	regOffsetColorA, OFFSET_EDGE_BUFFER_COLOR_A);
	cg_create_inst_binary(block0, cg_op_add,	regAddrStartColorA, regStart, regOffsetColorA);
	cg_create_inst_binary(block0, cg_op_add,	regAddrEndColorA, regEnd, regOffsetColorA);

	// fog density
	cg_virtual_reg_t * regOffsetFog = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAddrStartFog = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regAddrEndFog = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load_immed(block0, cg_op_ldi,	regOffsetFog, OFFSET_EDGE_BUFFER_FOG);
	cg_create_inst_binary(block0, cg_op_add,	regAddrStartFog, regStart, regOffsetFog);
	cg_create_inst_binary(block0, cg_op_add,	regAddrEndFog, regEnd, regOffsetFog);

	//EGL_Fixed invSpan = EGL_Inverse(end.m_WindowCoords.x - start.m_WindowCoords.x);
	cg_virtual_reg_t * regEndWindowX = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regStartWindowX = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDiffX = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regInvSpan = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load(block0, cg_op_ldw,	regEndWindowX, regAddrEndWindowX);
	cg_create_inst_load(block0, cg_op_ldw,	regStartWindowX, regAddrStartWindowX);
	cg_create_inst_binary(block0, cg_op_sub,	regDiffX, regEndWindowX, regStartWindowX);
	cg_create_inst_unary(block0, cg_op_finv,	regInvSpan, regDiffX);

	//FractionalColor baseColor = start.m_Color;
	cg_virtual_reg_t * regStartColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regStartColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regStartColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regStartColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load(block0, cg_op_ldw,	regStartColorR, regAddrStartColorR);
	cg_create_inst_load(block0, cg_op_ldw,	regStartColorG, regAddrStartColorG);
	cg_create_inst_load(block0, cg_op_ldw,	regStartColorB, regAddrStartColorB);
	cg_create_inst_load(block0, cg_op_ldw,	regStartColorA, regAddrStartColorA);

	//FractionalColor colorIncrement = (end.m_Color - start.m_Color) * invSpan;
	cg_virtual_reg_t * regEndColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regEndColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regEndColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regEndColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load(block0, cg_op_ldw,	regEndColorR, regAddrEndColorR);
	cg_create_inst_load(block0, cg_op_ldw,	regEndColorG, regAddrEndColorG);
	cg_create_inst_load(block0, cg_op_ldw,	regEndColorB, regAddrEndColorB);
	cg_create_inst_load(block0, cg_op_ldw,	regEndColorA, regAddrEndColorA);

	cg_virtual_reg_t * regDiffColorR = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDiffColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDiffColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDiffColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_binary(block0, cg_op_sub,	regDiffColorR, regEndColorR, regStartColorR);
	cg_create_inst_binary(block0, cg_op_sub,	regDiffColorG, regEndColorG, regStartColorG);
	cg_create_inst_binary(block0, cg_op_sub,	regDiffColorB, regEndColorB, regStartColorB);
	cg_create_inst_binary(block0, cg_op_sub,	regDiffColorA, regEndColorA, regStartColorA);

	cg_virtual_reg_t * regColorIncrementR = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regColorIncrementG = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regColorIncrementB = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regColorIncrementA = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_binary(block0, cg_op_fmul,	regColorIncrementR, regDiffColorR, regInvSpan);
	cg_create_inst_binary(block0, cg_op_fmul,	regColorIncrementG, regDiffColorG, regInvSpan);
	cg_create_inst_binary(block0, cg_op_fmul,	regColorIncrementB, regDiffColorB, regInvSpan);
	cg_create_inst_binary(block0, cg_op_fmul,	regColorIncrementA, regDiffColorA, regInvSpan);

	//EGL_Fixed deltaInvZ = EGL_Mul(end.m_WindowCoords.z - start.m_WindowCoords.z, invSpan);
	cg_virtual_reg_t * regEndWindowZ = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regStartWindowZ = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDiffInvZ = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDeltaInvZ = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load(block0, cg_op_ldw,	regEndWindowZ, regAddrEndWindowZ);
	cg_create_inst_load(block0, cg_op_ldw,	regStartWindowZ, regAddrStartWindowZ);
	cg_create_inst_binary(block0, cg_op_sub,	regDiffInvZ, regEndWindowZ, regStartWindowZ);
	cg_create_inst_binary(block0, cg_op_fmul,	regDeltaInvZ, regDiffInvZ, regInvSpan);

	//EGL_Fixed deltaInvU = EGL_Mul(end.m_TextureCoords.tu - start.m_TextureCoords.tu, invSpan);
	cg_virtual_reg_t * regEndTextureU = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regStartTextureU = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDiffInvU = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDeltaInvU = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load(block0, cg_op_ldw,	regEndTextureU, regAddrEndTextureU);
	cg_create_inst_load(block0, cg_op_ldw,	regStartTextureU, regAddrStartTextureU);
	cg_create_inst_binary(block0, cg_op_sub,	regDiffInvU, regEndTextureU, regStartTextureU);
	cg_create_inst_binary(block0, cg_op_fmul,	regDeltaInvU, regDiffInvU, regInvSpan);

	//EGL_Fixed deltaInvV = EGL_Mul(end.m_TextureCoords.tv - start.m_TextureCoords.tv, invSpan);
	cg_virtual_reg_t * regEndTextureV = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regStartTextureV = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDiffInvV = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDeltaInvV = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load(block0, cg_op_ldw,	regEndTextureV, regAddrEndTextureV);
	cg_create_inst_load(block0, cg_op_ldw,	regStartTextureV, regAddrStartTextureV);
	cg_create_inst_binary(block0, cg_op_sub,	regDiffInvV, regEndTextureV, regStartTextureV);
	cg_create_inst_binary(block0, cg_op_fmul,	regDeltaInvV, regDiffInvV, regInvSpan);

	//EGL_Fixed deltaFog = EGL_Mul(end.m_FogDensity - start.m_FogDensity, invSpan);
	cg_virtual_reg_t * regEndFog = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regStartFog = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDiffFog = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDeltaFog = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load(block0, cg_op_ldw,	regEndFog, regAddrEndFog);
	cg_create_inst_load(block0, cg_op_ldw,	regStartFog, regAddrStartFog);
	cg_create_inst_binary(block0, cg_op_sub,	regDiffFog, regEndFog, regStartFog);
	cg_create_inst_binary(block0, cg_op_fmul,	regDeltaFog, regDiffFog, regInvSpan);

	//EGL_Fixed invTu = start.m_TextureCoords.tu;
	//EGL_Fixed invTv = start.m_TextureCoords.tv;
	//EGL_Fixed invZ = start.m_WindowCoords.z;
	//EGL_Fixed fogDensity = start.m_FogDensity;

	//EGL_Fixed z = EGL_Inverse(invZ);
	//EGL_Fixed tu = EGL_Mul(invTu, z);
	//EGL_Fixed tv = EGL_Mul(invTv, z);
	cg_virtual_reg_t * regZ = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regU = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regV = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_unary(block0, cg_op_finv,	regZ, regStartWindowZ);
	cg_create_inst_binary(block0, cg_op_fmul,	regU, regStartTextureU, regZ);
	cg_create_inst_binary(block0, cg_op_fmul,	regV, regStartTextureV, regZ);

	//cg_virtual_reg_t * x = EGL_IntFromFixed(start.m_WindowCoords.x);
	//cg_virtual_reg_t * xEnd = EGL_IntFromFixed(end.m_WindowCoords.x);
	cg_virtual_reg_t * regX = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regXEnd = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_unary(block0, cg_op_trunc,	regX, regStartWindowX);
	cg_create_inst_unary(block0, cg_op_trunc,	regXEnd, regEndWindowX);

	//cg_virtual_reg_t * xLinEnd = x + ((xEnd - x) & ~(LINEAR_SPAN - 1));
	cg_virtual_reg_t * regSpanMask = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regMaskedSpan = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regXLinEnd = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regCompare0 = cg_virtual_reg_create(procedure, cg_reg_type_flags);
	cg_virtual_reg_t * regIntDiffX = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load_immed(block0, cg_op_ldi,	regSpanMask, ~(LINEAR_SPAN - 1));
	cg_create_inst_unary(block0, cg_op_trunc,	regIntDiffX, regDiffX);
	cg_create_inst_binary_s(block0, cg_op_and,	regMaskedSpan, regCompare0, regIntDiffX, regSpanMask);
	cg_create_inst_binary(block0, cg_op_add,	regXLinEnd, regX, regMaskedSpan);

	//for (; x < xLinEnd;) {

	cg_block_ref_t * endLoop0 = cg_block_ref_create(procedure);

	cg_create_inst_branch_cond(block0, cg_op_beq,	regCompare0, endLoop0);

	cg_block_ref_t * beginLoop0 = cg_block_ref_create(procedure);
	cg_block_t * block1 = cg_block_create(procedure);
	beginLoop0->block = block1;

	// Here we define all the loop registers and phi mappings
	cg_virtual_reg_t * regLoop0ZEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop0Z = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1Z = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_phi(block1, cg_op_phi, regLoop0ZEntry, cg_create_virtual_reg_list(procedure->module->heap, regZ, regLoop0Z, regLoop1Z, NULL));

	cg_virtual_reg_t * regLoop0UEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop0U = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1U = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_phi(block1, cg_op_phi, regLoop0UEntry, cg_create_virtual_reg_list(procedure->module->heap, regU, regLoop0U, regLoop1U, NULL));

	cg_virtual_reg_t * regLoop0VEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop0V = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1V = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_phi(block1, cg_op_phi, regLoop0VEntry, cg_create_virtual_reg_list(procedure->module->heap, regV, regLoop0V, regLoop1V, NULL));

	cg_virtual_reg_t * regLoop0InvZEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop0InvZ = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_phi(block1, cg_op_phi, regLoop0InvZEntry, cg_create_virtual_reg_list(procedure->module->heap, regStartWindowZ, regLoop0InvZ, NULL));

	cg_virtual_reg_t * regLoop0InvUEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop0InvU = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_phi(block1, cg_op_phi, regLoop0InvUEntry, cg_create_virtual_reg_list(procedure->module->heap, regStartTextureU, regLoop0InvU, NULL));

	cg_virtual_reg_t * regLoop0InvVEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop0InvV = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_phi(block1, cg_op_phi, regLoop0InvVEntry, cg_create_virtual_reg_list(procedure->module->heap, regStartTextureV, regLoop0InvV, NULL));

		//invZ += deltaInvZ << LOG_LINEAR_SPAN;
		//invTu += deltaInvU << LOG_LINEAR_SPAN;
		//invTv += deltaInvV << LOG_LINEAR_SPAN;
	cg_virtual_reg_t * regLinearSpan = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regConstant1 = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDeltaInvZTimesLinearSpan = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDeltaInvUTimesLinearSpan = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regDeltaInvVTimesLinearSpan = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_load_immed(block1, cg_op_ldi,	regConstant1, 1);
	cg_create_inst_load_immed(block1, cg_op_ldi,	regLinearSpan, LINEAR_SPAN);
	cg_create_inst_binary(block1, cg_op_fmul,	regDeltaInvZTimesLinearSpan, regDeltaInvZ, regLinearSpan);	
	cg_create_inst_binary(block1, cg_op_fadd,	regLoop0InvZ, regLoop0InvZEntry, regDeltaInvZTimesLinearSpan);
	cg_create_inst_binary(block1, cg_op_fmul,	regDeltaInvUTimesLinearSpan, regDeltaInvU, regLinearSpan);	
	cg_create_inst_binary(block1, cg_op_fadd,	regLoop0InvU, regLoop0InvUEntry, regDeltaInvUTimesLinearSpan);
	cg_create_inst_binary(block1, cg_op_fmul,	regDeltaInvVTimesLinearSpan, regDeltaInvV, regLinearSpan);	
	cg_create_inst_binary(block1, cg_op_fadd,	regLoop0InvV, regLoop0InvVEntry, regDeltaInvVTimesLinearSpan);

		//EGL_Fixed endZ = EGL_Inverse(invZ);
		//EGL_Fixed endTu = EGL_Mul(invTu, endZ);
		//EGL_Fixed endTv = EGL_Mul(invTv, endZ);

	cg_virtual_reg_t * regLoop0EndZ = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop0EndU = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop0EndV = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_unary(block1, cg_op_finv,	regLoop0EndZ, regLoop0InvZ);
	cg_create_inst_binary(block1, cg_op_fmul,	regLoop0EndU, regLoop0InvU, regLoop0EndZ);
	cg_create_inst_binary(block1, cg_op_fmul,	regLoop0EndV, regLoop0InvV, regLoop0EndZ);

		//EGL_Fixed deltaZ = (endZ - z) >> LOG_LINEAR_SPAN;
		//EGL_Fixed deltaTu = (endTu - tu) >> LOG_LINEAR_SPAN; 
		//EGL_Fixed deltaTv = (endTv - tv) >> LOG_LINEAR_SPAN;

	cg_virtual_reg_t * regLoop0DiffZ = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop0ScaledDiffZ = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop0DiffU = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop0ScaledDiffU = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop0DiffV = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop0ScaledDiffV = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_binary(block1, cg_op_fsub,	regLoop0DiffZ, regLoop0EndZ, regLoop0Z); // Entry?
	cg_create_inst_binary(block1, cg_op_fdiv,	regLoop0ScaledDiffZ, regLoop0DiffZ, regLinearSpan);
	cg_create_inst_binary(block1, cg_op_fsub,	regLoop0DiffU, regLoop0EndU, regLoop0U); // Entry?
	cg_create_inst_binary(block1, cg_op_fdiv,	regLoop0ScaledDiffU, regLoop0DiffU, regLinearSpan);
	cg_create_inst_binary(block1, cg_op_fsub,	regLoop0DiffV, regLoop0EndV, regLoop0V); // Entry?
	cg_create_inst_binary(block1, cg_op_fdiv,	regLoop0ScaledDiffV, regLoop0DiffV, regLinearSpan);

	// also not to include phi projection for z coming from inner loop

		//int count = LINEAR_SPAN; 

	cg_block_ref_t * beginLoop1 = cg_block_ref_create(procedure);
	cg_block_ref_t * endLoop1 = cg_block_ref_create(procedure);
	cg_block_ref_t * postFragmentLoop1 = cg_block_ref_create(procedure);

	cg_block_t * block2 = cg_block_create(procedure);
	beginLoop1->block = block2;

		//do {

	// phi for count, x, z, tu, tv, fog, r, g, b, a

	cg_virtual_reg_t * regLoop1CountEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1Count = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1XEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1X = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1ZEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1UEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1VEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1FogEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1Fog = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1REntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1R = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1GEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1G = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1BEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1B = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1AEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop1A = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_phi(block2, cg_op_phi,	regLoop1CountEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1Count, regLinearSpan, NULL));
	cg_create_inst_phi(block2, cg_op_phi,	regLoop1XEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1X, regX, NULL));
	cg_create_inst_phi(block2, cg_op_phi,	regLoop1ZEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1Z, regZ, NULL));
	cg_create_inst_phi(block2, cg_op_phi,	regLoop1UEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1U, regU, NULL));
	cg_create_inst_phi(block2, cg_op_phi,	regLoop1VEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1V, regV, NULL));
	cg_create_inst_phi(block2, cg_op_phi,	regLoop1FogEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1Fog, regStartFog, NULL));
	cg_create_inst_phi(block2, cg_op_phi,	regLoop1REntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1R, regStartColorR, NULL));
	cg_create_inst_phi(block2, cg_op_phi,	regLoop1GEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1G, regStartColorG, NULL));
	cg_create_inst_phi(block2, cg_op_phi,	regLoop1BEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1B, regStartColorB, NULL));
	cg_create_inst_phi(block2, cg_op_phi,	regLoop1AEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop1A, regStartColorA, NULL));
		
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

	GenerateFragment(procedure, block2, postFragmentLoop1, info); 

	cg_block_t * block3 = cg_block_create(procedure);
	postFragmentLoop1->block = block3;

			//baseColor += colorIncrement;
	cg_create_inst_binary(block3, cg_op_fadd, regLoop1R, regLoop1REntry, regColorIncrementR);
	cg_create_inst_binary(block3, cg_op_fadd, regLoop1G, regLoop1REntry, regColorIncrementG);
	cg_create_inst_binary(block3, cg_op_fadd, regLoop1B, regLoop1REntry, regColorIncrementB);
	cg_create_inst_binary(block3, cg_op_fadd, regLoop1A, regLoop1REntry, regColorIncrementA);

			//fogDensity += deltaFog;
			//z += deltaZ;
			//tu += deltaTu;
			//tv += deltaTv;
	cg_create_inst_binary(block3, cg_op_fadd, regLoop1Fog, regLoop1FogEntry, regDeltaFog);
	cg_create_inst_binary(block3, cg_op_fadd, regLoop1Z, regLoop1ZEntry, regLoop0ScaledDiffZ);
	cg_create_inst_binary(block3, cg_op_fadd, regLoop1U, regLoop1UEntry, regLoop0ScaledDiffU);
	cg_create_inst_binary(block3, cg_op_fadd, regLoop1V, regLoop1VEntry, regLoop0ScaledDiffV);

			//++x;
	cg_create_inst_binary(block3, cg_op_add, regLoop1X, regLoop1XEntry, regConstant1);

		//} while (--count);
	cg_virtual_reg_t * regLoop1Condition = cg_virtual_reg_create(procedure, cg_reg_type_flags);

	cg_create_inst_binary_s(block3, cg_op_sub,	regLoop1Count, regLoop1Condition, regLoop1CountEntry, regConstant1);
	cg_create_inst_branch_cond(block3, cg_op_bne,	regLoop1Condition, beginLoop1);
	//}

	cg_virtual_reg_t * regLoop0Condition = cg_virtual_reg_create(procedure, cg_reg_type_flags);
	cg_create_inst_compare(block3, cg_op_cmp,	regLoop0Condition, regLoop1X, regXLinEnd);
	cg_create_inst_branch_cond(block3, cg_op_bne,	regLoop0Condition, beginLoop0);

	//if (x != xEnd) {
	cg_block_t * block4 = cg_block_create(procedure);
	endLoop0->block = block4;

	cg_block_ref_t * beginLoop2 = cg_block_ref_create(procedure);
	cg_block_ref_t * endLoop2 = cg_block_ref_create(procedure);
	cg_block_ref_t * postFragmentLoop2 = cg_block_ref_create(procedure);

	cg_virtual_reg_t * regBlock4X = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regBlock4Z = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regBlock4U = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regBlock4V = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regBlock4DiffX = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regBlock4Condition = cg_virtual_reg_create(procedure, cg_reg_type_flags);

	cg_create_inst_phi(block4, cg_op_phi,	regBlock4X, cg_create_virtual_reg_list(procedure->module->heap, regX, regLoop1X, NULL));
	cg_create_inst_phi(block4, cg_op_phi,	regBlock4Z, cg_create_virtual_reg_list(procedure->module->heap, regLoop1Z, regZ, NULL));
	cg_create_inst_phi(block4, cg_op_phi,	regBlock4U, cg_create_virtual_reg_list(procedure->module->heap, regLoop1U, regU, NULL));
	cg_create_inst_phi(block4, cg_op_phi,	regBlock4V, cg_create_virtual_reg_list(procedure->module->heap, regLoop1V, regV, NULL));
	cg_create_inst_binary_s(block4, cg_op_sub,	regBlock4DiffX, regBlock4Condition, regXEnd, regBlock4X);
	cg_create_inst_branch_cond(block4, cg_op_beq,	regBlock4Condition, endLoop2);

		//EGL_Fixed endZ = EGL_Inverse(end.m_WindowCoords.z);
		//EGL_Fixed endTu = EGL_Mul(end.m_TextureCoords.tu, endZ);
		//EGL_Fixed endTv = EGL_Mul(end.m_TextureCoords.tv, endZ);
	cg_virtual_reg_t * regEndZ = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regEndU = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regEndV = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_unary(block4, cg_op_finv,	regEndZ, regEndWindowZ);
	cg_create_inst_binary(block4, cg_op_fmul,	regEndU, regEndZ, regEndTextureU);
	cg_create_inst_binary(block4, cg_op_fmul,	regEndV, regEndZ, regEndTextureV);

		//invSpan = EGL_Inverse(EGL_FixedFromInt(xEnd - x));
	cg_virtual_reg_t * regFixedBlock4DiffX = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regBlock4InvSpan = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_unary(block4, cg_op_fcnv,	regFixedBlock4DiffX, regBlock4DiffX);
	cg_create_inst_unary(block4, cg_op_finv,	regBlock4InvSpan, regFixedBlock4DiffX);

		//EGL_Fixed deltaZ = EGL_Mul(endZ - z, invSpan);
		//EGL_Fixed deltaTu = EGL_Mul(endTu - tu, invSpan);
		//EGL_Fixed deltaTv = EGL_Mul(endTv - tv, invSpan);
	cg_virtual_reg_t * regBlock4DiffZ = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regBlock4DiffU = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regBlock4DiffV = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2ScaledDiffZ = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2ScaledDiffU = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2ScaledDiffV = cg_virtual_reg_create(procedure, cg_reg_type_general);

	cg_create_inst_binary(block4, cg_op_fsub,	regBlock4DiffZ, regEndZ, regBlock4Z);
	cg_create_inst_binary(block4, cg_op_fmul,	regLoop2ScaledDiffZ, regBlock4DiffZ, regBlock4InvSpan);
	cg_create_inst_binary(block4, cg_op_fsub,	regBlock4DiffU, regEndU, regBlock4U);
	cg_create_inst_binary(block4, cg_op_fmul,	regLoop2ScaledDiffU, regBlock4DiffU, regBlock4InvSpan);
	cg_create_inst_binary(block4, cg_op_fsub,	regBlock4DiffV, regEndV, regBlock4V);
	cg_create_inst_binary(block4, cg_op_fmul,	regLoop2ScaledDiffV, regBlock4DiffV, regBlock4InvSpan);

		//for (; x < xEnd; ++x) {
	cg_block_t * block5 = cg_block_create(procedure);
	beginLoop2->block = block5;

	// phi for x, z, tu, tv, fog, r, g, b, a

	cg_virtual_reg_t * regLoop2XEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2X = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2ZEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2Z = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2UEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2U = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2VEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2V = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2FogEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2Fog = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2REntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2R = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2GEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2G = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2BEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2B = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2AEntry = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_virtual_reg_t * regLoop2A = cg_virtual_reg_create(procedure, cg_reg_type_general);


	cg_create_inst_phi(block5, cg_op_phi,	regLoop2XEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2X, regBlock4X, NULL));
	cg_create_inst_phi(block5, cg_op_phi,	regLoop2ZEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2Z, regBlock4Z, NULL));
	cg_create_inst_phi(block5, cg_op_phi,	regLoop2UEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2U, regBlock4U, NULL));
	cg_create_inst_phi(block5, cg_op_phi,	regLoop2VEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2V, regBlock4V, NULL));
	cg_create_inst_phi(block5, cg_op_phi,	regLoop2FogEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2Fog, regLoop1Fog, regStartFog, NULL));
	cg_create_inst_phi(block5, cg_op_phi,	regLoop2REntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2R, regLoop1R, regStartColorR, NULL));
	cg_create_inst_phi(block5, cg_op_phi,	regLoop2GEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2G, regLoop1G, regStartColorG, NULL));
	cg_create_inst_phi(block5, cg_op_phi,	regLoop2BEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2B, regLoop1B, regStartColorB, NULL));
	cg_create_inst_phi(block5, cg_op_phi,	regLoop2AEntry, cg_create_virtual_reg_list(procedure->module->heap, regLoop2A, regLoop1A, regStartColorA, NULL));

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

	GenerateFragment(procedure, block5, postFragmentLoop2, info2); 

	cg_block_t * block6 = cg_block_create(procedure);
	postFragmentLoop2->block = block6;

			//baseColor += colorIncrement;
	cg_create_inst_binary(block6, cg_op_fadd,	regLoop2R, regLoop2REntry, regColorIncrementR);
	cg_create_inst_binary(block6, cg_op_fadd,	regLoop2G, regLoop2REntry, regColorIncrementG);
	cg_create_inst_binary(block6, cg_op_fadd,	regLoop2B, regLoop2REntry, regColorIncrementB);
	cg_create_inst_binary(block6, cg_op_fadd,	regLoop2A, regLoop2REntry, regColorIncrementA);

			//fogDensity += deltaFog;
			//z += deltaZ;
			//tu += deltaTu;
			//tv += deltaTv;
	cg_create_inst_binary(block6, cg_op_fadd,	regLoop2Fog, regLoop2FogEntry, regDeltaFog);
	cg_create_inst_binary(block6, cg_op_fadd,	regLoop2Z, regLoop2ZEntry, regLoop2ScaledDiffZ);
	cg_create_inst_binary(block6, cg_op_fadd,	regLoop2U, regLoop2UEntry, regLoop2ScaledDiffU);
	cg_create_inst_binary(block6, cg_op_fadd,	regLoop2V, regLoop2VEntry, regLoop2ScaledDiffV);

			//++x;
	cg_create_inst_binary(block6, cg_op_add,	regLoop2X, regLoop2XEntry, regConstant1);

	cg_virtual_reg_t * regCondLoopEnd = cg_virtual_reg_create(procedure, cg_reg_type_general);
	cg_create_inst_binary(block6, cg_op_cmp,	regCondLoopEnd, regLoop2X, regXEnd);
	cg_create_inst_branch_cond(block6, cg_op_blt,	regCondLoopEnd, beginLoop2);

		//}

	//}
	cg_block_t * block7 = cg_block_create(procedure);
	endLoop2->block = block7;

	cg_create_inst_ret(block7, cg_op_ret);

}

void CodeGenerator :: CompileRasterScanLine() {
	GenerateRasterScanLine();




#ifdef WINCE
	// flush data cache and clear instruction cache to make new code visible to execution unit
	CacheSync(CACHE_SYNC_INSTRUCTIONS | CACHE_SYNC_WRITEBACK);		

#endif
}

