// ==========================================================================
//
// CodeGenerator.cpp	Rasterizer Class for OpenGL (R) ES Implementation
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
#include "Rasterizer.h"
#include "Surface.h"
#include "Texture.h"
#include "trivm.h"


using namespace EGL;
using namespace triVM;


#define IMMEDIATE	Instruction::createLoadImmediate
#define UNARY		Instruction::createUnary
#define BINARY		Instruction::createBinary
#define COMPARE		Instruction::createCompare
#define LOAD		Instruction::createLoad
#define STORE		Instruction::createStore
#define	BCOND		Instruction::createBranchConditionally
#define	BLABEL		Instruction::createBranchLabel
#define	BREG		Instruction::createBranchReg
#define CALL		Instruction::createCall
#define RET			Instruction::createRet
#define PHI			Instruction::createPhi

namespace {
	RegisterList * REG_LIST(I32 r1) {
		RegisterList * result = new RegisterList();
		result->push_back(r1);
		return result;
	}

	RegisterList * REG_LIST(I32 r1, I32 r2) {
		RegisterList * result = new RegisterList();
		result->push_back(r1);
		result->push_back(r2);
		return result;
	}

	RegisterList * REG_LIST(I32 r1, I32 r2, I32 r3) {
		RegisterList * result = new RegisterList();
		result->push_back(r1);
		result->push_back(r2);
		result->push_back(r3);
		return result;
	}

	I32 LOAD_DATA(Block & block, I32& nextRegister, I32 base, I32 constant) {
		I32 offset = nextRegister++;
		I32 addr = nextRegister++;
		I32 value = nextRegister++;

		block +=	IMMEDIATE	(ldi,	offset, Constant::createInt(constant));
		block +=	BINARY		(add,	addr, base, offset);
		block +=	LOAD		(ldw,	value, addr);

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
		I32 regX;
		I32 regY; 
		I32 regDepth;
		I32 regU;
		I32 regV; 
		I32 regFog;
		I32 regR;
		I32 regG;
		I32 regB; 
		I32 regA;	

		I32 regTextureData;
		I32 regTextureWidth;
		I32 regTextureHeight;
		I32 regTextureExponent;

		// surface color buffer, depth buffer, alpha buffer, stencil buffer
		I32 regColorBuffer;
		I32 regDepthBuffer;
		I32 regAlphaBuffer;
		I32 regStencilBuffer;

		I32 regSurfaceWidth;
	};
}


// Actually, we could extract the scaling of the texture coordinates into the outer driving loop, 
// and have the adjusted clipping range for tu and tv be stored in the rasterizer.

void Rasterizer :: GenerateFragment(Procedure * procedure, Block & currentBlock,
			Label * continuation, I32 & nextRegister, FragmentGenerationInfo & fragmentInfo) {
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
	I32 regDepthTest = nextRegister++;
	I32 regScaledY = nextRegister++;
	I32 regOffset = nextRegister++;
	I32 regConstant1 = nextRegister++;
	I32 regConstant2 = nextRegister++;
	I32 regOffset4 = nextRegister++;
	I32 regOffset2 = nextRegister++;
	I32 regZBufferAddr = nextRegister++;
	I32 regZBufferValue = nextRegister++;

	currentBlock +=		BINARY		(mul,	regScaledY, fragmentInfo.regY, fragmentInfo.regSurfaceWidth);
	currentBlock +=		BINARY		(add,	regOffset, regScaledY, fragmentInfo.regX);
	currentBlock +=		IMMEDIATE	(ldi,	regConstant1, Constant::createInt(1));
	currentBlock +=		IMMEDIATE	(ldi,	regConstant2, Constant::createInt(2));
	currentBlock +=		BINARY		(lsl,	regOffset2, regOffset, regConstant1); 
	currentBlock +=		BINARY		(lsl,	regOffset4, regOffset, regConstant2);
	currentBlock +=		BINARY		(add,	regZBufferAddr, fragmentInfo.regDepthBuffer, regOffset4);
	currentBlock +=		LOAD		(ldw,	regZBufferValue, regZBufferAddr);
	currentBlock +=		COMPARE		(fcmp,	regDepthTest, regZBufferValue, fragmentInfo.regDepth);

	Opcode branchOnDepthTestPassed, branchOnDepthTestFailed;

	switch (m_State->m_DepthFunc) {
		default:
		case RasterizerState::CompFuncNever:	
			//depthTest = false;						
			branchOnDepthTestPassed = nop;
			branchOnDepthTestFailed = bra;
			break;

		case RasterizerState::CompFuncLess:		
			//depthTest = depth < zBufferValue;		
			branchOnDepthTestPassed = blt;
			branchOnDepthTestFailed = bge;
			break;

		case RasterizerState::CompFuncEqual:	
			//depthTest = depth == zBufferValue;		
			branchOnDepthTestPassed = beq;
			branchOnDepthTestFailed = bne;
			break;

		case RasterizerState::CompFuncLEqual:	
			//depthTest = depth <= zBufferValue;		
			branchOnDepthTestPassed = ble;
			branchOnDepthTestFailed = bgt;
			break;

		case RasterizerState::CompFuncGreater:	
			//depthTest = depth > zBufferValue;		
			branchOnDepthTestPassed = bgt;
			branchOnDepthTestFailed = ble;
			break;

		case RasterizerState::CompFuncNotEqual:	
			//depthTest = depth != zBufferValue;		
			branchOnDepthTestPassed = bne;
			branchOnDepthTestFailed = beq;
			break;

		case RasterizerState::CompFuncGEqual:	
			//depthTest = depth >= zBufferValue;		
			branchOnDepthTestPassed = bge;
			branchOnDepthTestFailed = blt;
			break;

		case RasterizerState::CompFuncAlways:	
			//depthTest = true;						
			branchOnDepthTestPassed = bra;
			branchOnDepthTestFailed = nop;
			break;
	}

	if (!m_State->m_StencilTestEnabled && m_State->m_DepthTestEnabled) {
		//if (!depthTest)
		//	return;

		if (branchOnDepthTestFailed == nop) {
			// nothing
		} else if (branchOnDepthTestFailed == bra) {
			currentBlock +=		BLABEL	(bra,	continuation);
		} else {
			currentBlock +=		BCOND	(branchOnDepthTestFailed, regDepthTest, continuation);
		}
	}

	//Color color = baseColor;
	I32 colorR;
	I32 colorG;
	I32 colorB;
	I32 colorA;

	Block * block = &currentBlock;

	if (m_State->m_TextureEnabled) {

		//EGL_Fixed tu0;
		//EGL_Fixed tv0;
		I32 regU0 = nextRegister++;
		I32 regV0 = nextRegister++;

		switch (m_Texture->GetWrappingModeS()) {
			case MultiTexture::WrappingModeClampToEdge:
				//tu0 = EGL_CLAMP(tu, 0, EGL_ONE);
				{
					I32 regConstantOne = nextRegister++;
					I32 regConstantZero = nextRegister++;
					I32 regCompareOne = nextRegister++;
					I32 regCompareZero = nextRegister++;
					I32 regNewU1 = nextRegister++;
					I32 regNewU2 = nextRegister++;

					Label * label1 = Label::create("wrapU1", LabelBlock);
					Label * label2 = Label::create("wrapU2", LabelBlock);

					*block +=	IMMEDIATE		(ldi,	regConstantOne, Constant::createFloat(1.0f));
					*block +=	COMPARE			(fcmp,	regCompareOne, fragmentInfo.regU, regConstantOne);
					*block +=	BCOND			(ble,	regCompareOne, label1);
					*block +=	IMMEDIATE		(ldi,	regNewU1, Constant::createFloat(1.0f));
					*block +=	BLABEL			(bra,	label2);

					Block& block1 = *new Block(procedure);
					procedure->blocks.push_back(&block1);
					label1->block.block = &block1;
					block1.labels.push_back(label1);
					block = &block1;

					*block +=	IMMEDIATE		(ldi,	regConstantZero, Constant::createFloat(0.0f));
					*block +=	COMPARE			(fcmp,	regCompareZero, fragmentInfo.regU, regConstantZero);
					*block +=	BCOND			(bge,	regCompareZero, label2);
					*block +=	IMMEDIATE		(ldi,	regNewU2, Constant::createFloat(0.0f));

					Block& block2 = *new Block(procedure);
					procedure->blocks.push_back(&block2);
					label2->block.block = &block2;
					block2.labels.push_back(label2);
					block = &block2;

					*block +=	PHI				(phi,	regU0, REG_LIST(fragmentInfo.regU, regNewU1, regNewU2));
				}
				break;

			default:
			case MultiTexture::WrappingModeRepeat:
				//tu0 = tu & 0xffff;
				{
					I32 regMask = nextRegister++;
					*block +=	IMMEDIATE		(ldi,	regMask, Constant::createInt(0xffff));
					*block +=	BINARY			(and,	regU0, fragmentInfo.regU, regMask);
				}
				break;
		}

		switch (m_Texture->GetWrappingModeT()) {
			case MultiTexture::WrappingModeClampToEdge:
				//tv0 = EGL_CLAMP(tv, 0, EGL_ONE);
				{
					I32 regConstantOne = nextRegister++;
					I32 regConstantZero = nextRegister++;
					I32 regCompareOne = nextRegister++;
					I32 regCompareZero = nextRegister++;
					I32 regNewV1 = nextRegister++;
					I32 regNewV2 = nextRegister++;

					Label * label1 = Label::create("wrapV1", LabelBlock);
					Label * label2 = Label::create("wrapV2", LabelBlock);

					*block +=	IMMEDIATE		(ldi,	regConstantOne, Constant::createFloat(1.0f));
					*block +=	COMPARE			(fcmp,	regCompareOne, fragmentInfo.regV, regConstantOne);
					*block +=	BCOND			(ble,	regCompareOne, label1);
					*block +=	IMMEDIATE		(ldi,	regNewV1, Constant::createFloat(1.0f));
					*block +=	BLABEL			(bra,	label2);

					Block& block1 = *new Block(procedure);
					procedure->blocks.push_back(&block1);
					label1->block.block = &block1;
					block1.labels.push_back(label1);
					block = &block1;

					*block +=	IMMEDIATE		(ldi,	regConstantZero, Constant::createFloat(0.0f));
					*block +=	COMPARE			(fcmp,	regCompareZero, fragmentInfo.regV, regConstantZero);
					*block +=	BCOND			(bge,	regCompareZero, label2);
					*block +=	IMMEDIATE		(ldi,	regNewV2, Constant::createFloat(0.0f));

					Block& block2 = *new Block(procedure);
					procedure->blocks.push_back(&block2);
					label2->block.block = &block2;
					block2.labels.push_back(label2);
					block = &block2;

					*block +=	PHI				(phi,	regV0, REG_LIST(fragmentInfo.regV, regNewV1, regNewV2));
				}
				break;

			default:
			case MultiTexture::WrappingModeRepeat:
				//tv0 = tv & 0xffff;
				{
					I32 regMask = nextRegister++;
					*block +=	IMMEDIATE		(ldi,	regMask, Constant::createInt(0xffff));
					*block +=	BINARY			(and,	regV0, fragmentInfo.regV, regMask);
				}
				break;
		}

		// get the pixel color
		//Texture * texture = m_Texture->GetTexture(m_MipMapLevel);
		//Color texColor; 
		I32 regTexColorR;			
		I32 regTexColorG;			
		I32 regTexColorB;			
		I32 regTexColorA;			

		//I32 texX = EGL_IntFromFixed(texture->GetWidth() * tu0);
		//I32 texY = EGL_IntFromFixed(texture->GetHeight() * tv0);
		//I32 texOffset = texX + (texY << texture->GetExponent());
		//void * data = texture->GetData();
		I32 regScaledU = nextRegister++;
		I32 regTexX = nextRegister++;
		I32 regScaledV = nextRegister++;
		I32 regTexY = nextRegister++;
		I32 regScaledTexY = nextRegister++;
		I32 regTexOffset = nextRegister++;

		*block +=	BINARY		(mul,		regScaledU, regU0, fragmentInfo.regTextureWidth);
		*block +=	UNARY		(trunc,		regTexX, regScaledU);
		*block +=	BINARY		(mul,		regScaledV, regV0, fragmentInfo.regTextureHeight);
		*block +=	UNARY		(trunc,		regTexY, regScaledV);
		*block +=	BINARY		(lsl,		regScaledTexY, regTexY, fragmentInfo.regTextureExponent);
		*block +=	BINARY		(add,		regTexOffset, regTexX, regScaledTexY);

		switch (m_Texture->GetInternalFormat()) {
			case Texture::TextureFormatAlpha:				// 8
				{
				//texColor = Color(0xff, 0xff, 0xff, reinterpret_cast<const U8 *>(data)[texOffset]);
				regTexColorR = regTexColorB = regTexColorG = nextRegister++;
				regTexColorA = nextRegister++;
				I32 regTexAddr = nextRegister++;

				*block +=	BINARY		(add,	regTexAddr, regTexOffset, fragmentInfo.regTextureData);
				*block +=	LOAD		(ldb,	regTexColorA, regTexAddr);
				*block +=	IMMEDIATE	(ldi,	regTexColorR, Constant::createInt(0xff));
				}
				break;

			case Texture::TextureFormatLuminance:			// 8
				{
				//U8 luminance = reinterpret_cast<const U8 *>(data)[texOffset];
				//texColor = Color (luminance, luminance, luminance, 0xff);
				regTexColorR = regTexColorB = regTexColorG = nextRegister++;
				regTexColorA = nextRegister++;
				I32 regTexAddr = nextRegister++;

				*block +=	BINARY		(add,	regTexAddr, regTexOffset, fragmentInfo.regTextureData);
				*block +=	LOAD		(ldb,	regTexColorR, regTexAddr);
				*block +=	IMMEDIATE	(ldi,	regTexColorA, Constant::createInt(0xff));
				}
				break;

			case Texture::TextureFormatLuminanceAlpha:		// 8-8
				{
				//U8 luminance = reinterpret_cast<const U8 *>(data)[texOffset * 2];
				//U8 alpha = reinterpret_cast<const U8 *>(data)[texOffset * 2 + 1];
				//texColor = Color (luminance, luminance, luminance, alpha);
				regTexColorR = regTexColorB = regTexColorG = nextRegister++;
				regTexColorA = nextRegister++;
				I32 regTexData = nextRegister++;
				I32 regScaledOffset = nextRegister++;
				I32 regConstantOne = nextRegister++;
				I32 regTexAddr = nextRegister++;
				I32 regMask = nextRegister++;
				I32 regConstant8 = nextRegister++;
				I32 regAlpha = nextRegister++;

				*block +=	IMMEDIATE	(ldi,	regConstantOne, Constant::createInt(1));
				*block +=	BINARY		(lsl,	regScaledOffset, regTexOffset, regConstantOne);
				*block +=	BINARY		(add,	regTexAddr, regScaledOffset, fragmentInfo.regTextureData);
				*block +=	LOAD		(ldh,	regTexData, regTexAddr);
				*block +=	IMMEDIATE	(ldi,	regMask, Constant::createInt(0xff));
				*block +=   BINARY		(and,	regTexColorR, regTexData, regMask);
				*block +=	IMMEDIATE	(ldi,	regConstant8, Constant::createInt(8));
				*block +=	BINARY		(lsr,	regAlpha, regTexData, regConstant8);
				*block +=	BINARY		(and,	regTexColorA, regAlpha, regMask);

				}
				break;

			case Texture::TextureFormatRGB:					// 5-6-5
				//texColor = Color::From565(reinterpret_cast<const U16 *>(data)[texOffset]);
				{
				regTexColorR = nextRegister++;
				regTexColorB = nextRegister++;
				regTexColorG = nextRegister++;
				regTexColorA = nextRegister++;
				I32 regTexData = nextRegister++;
				I32 regScaledOffset = nextRegister++;
				I32 regConstantOne = nextRegister++;
				I32 regTexAddr = nextRegister++;
				I32 regMask = nextRegister++;

				*block +=	IMMEDIATE	(ldi,	regConstantOne, Constant::createInt(1));
				*block +=	BINARY		(lsl,	regScaledOffset, regTexOffset, regConstantOne);
				*block +=	BINARY		(add,	regTexAddr, regScaledOffset, fragmentInfo.regTextureData);
				*block +=	LOAD		(ldh,	regTexData, regTexAddr);
				*block +=	IMMEDIATE	(ldi,	regMask, Constant::createInt(0xff));

				// TO DO: continue here
				}
				break;

			case Texture::TextureFormatRGBA:					// 5-5-5-1
				//texColor = Color::From5551(reinterpret_cast<const U16 *>(data)[texOffset]);
				{
				}
				break;

			default:
				//texColor = Color(0xff, 0xff, 0xff, 0xff);
				{
				regTexColorR = regTexColorB = regTexColorG = regTexColorA = nextRegister++;
				*block +=	IMMEDIATE	(ldi,	regTexColorR, Constant::createInt(0xff));
				}
				break;
		}

		switch (m_Texture->GetInternalFormat()) {
			default:
			case Texture::TextureFormatAlpha:
				switch (m_State->m_TextureMode) {
					case RasterizerState::TextureModeReplace:
						//color = Color(color.r, color.g, color.b, texColor.a);
						break;

					case RasterizerState::TextureModeModulate:
					case RasterizerState::TextureModeBlend:
					case RasterizerState::TextureModeAdd:
						//color = Color(color.r, color.g, color.b, MulU8(color.a, texColor.a));
						break;
				}
				break;

			case Texture::TextureFormatLuminance:
			case Texture::TextureFormatRGB:
				switch (m_State->m_TextureMode) {
					case RasterizerState::TextureModeDecal:
					case RasterizerState::TextureModeReplace:
						//color = Color(texColor.r, texColor.g, texColor.b, color.a);
						break;

					case RasterizerState::TextureModeModulate:
						//color = Color(MulU8(color.r, texColor.r), 
						//	MulU8(color.g, texColor.g), MulU8(color.b, texColor.b), color.a);
						break;

					case RasterizerState::TextureModeBlend:
						//color = 
						//	Color(
						//		MulU8(color.r, 0xff - texColor.r) + MulU8(m_State->m_TexEnvColor.r, texColor.r),
						//		MulU8(color.g, 0xff - texColor.g) + MulU8(m_State->m_TexEnvColor.g, texColor.g),
						//		MulU8(color.b, 0xff - texColor.b) + MulU8(m_State->m_TexEnvColor.b, texColor.b),
						//		color.a);
						break;

					case RasterizerState::TextureModeAdd:
						//color =
						//	Color(
						//		ClampU8(color.r + texColor.r),
						//		ClampU8(color.g + texColor.g),
						//		ClampU8(color.b + texColor.b),
						//		color.a);
						break;
				}
				break;

			case Texture::TextureFormatLuminanceAlpha:
			case Texture::TextureFormatRGBA:
				switch (m_State->m_TextureMode) {
					case RasterizerState::TextureModeReplace:
						//color = texColor;
						break;

					case RasterizerState::TextureModeModulate:
						//color = color * texColor;
						break;

					case RasterizerState::TextureModeDecal:
						//color = 
						//	Color(
						//		MulU8(color.r, 0xff - texColor.a) + MulU8(texColor.r, texColor.a),
						//		MulU8(color.g, 0xff - texColor.a) + MulU8(texColor.g, texColor.a),
						//		MulU8(color.b, 0xff - texColor.a) + MulU8(texColor.b, texColor.a),
						//		color.a);
						break;

					case RasterizerState::TextureModeBlend:
						//color = 
						//	Color(
						//		MulU8(color.r, 0xff - texColor.r) + MulU8(m_State->m_TexEnvColor.r, texColor.r),
						//		MulU8(color.g, 0xff - texColor.g) + MulU8(m_State->m_TexEnvColor.g, texColor.g),
						//		MulU8(color.b, 0xff - texColor.b) + MulU8(m_State->m_TexEnvColor.b, texColor.b),
						//		MulU8(color.a, texColor.a));
						break;

					case RasterizerState::TextureModeAdd:
						//color =
						//	Color(
						//		ClampU8(color.r + texColor.r),
						//		ClampU8(color.g + texColor.g),
						//		ClampU8(color.b + texColor.b),
						//		MulU8(color.a, texColor.a));
						break;
				}
				break;
		}
	} else {
		// color = baseColor
		colorR = fragmentInfo.regR;
		colorG = fragmentInfo.regG;
		colorB = fragmentInfo.regB;
		colorA = fragmentInfo.regA;
	}

	// fog
	if (m_State->m_FogEnabled) {
		//color = Color::Blend(color, m_State->m_FogColor, fogDensity);
	}

	if (m_State->m_AlphaTestEnabled) {
		//bool alphaTest;
		//U8 alpha = color.A();
		U8 alphaRef = EGL_IntFromFixed(m_State->m_AlphaReference * 255);

		switch (m_State->m_AlphaFunc) {
			default:
			case RasterizerState::CompFuncNever:	
				//alphaTest = false;					
				break;

			case RasterizerState::CompFuncLess:		
				//alphaTest = alpha < alphaRef;		
				break;

			case RasterizerState::CompFuncEqual:	
				//alphaTest = alpha == alphaRef;		
				break;

			case RasterizerState::CompFuncLEqual:	
				//alphaTest = alpha <= alphaRef;		
				break;

			case RasterizerState::CompFuncGreater:	
				//alphaTest = alpha > alphaRef;		
				break;

			case RasterizerState::CompFuncNotEqual:	
				//alphaTest = alpha != alphaRef;		
				break;

			case RasterizerState::CompFuncGEqual:	
				//alphaTest = alpha >= alphaRef;		
				break;

			case RasterizerState::CompFuncAlways:	
				//alphaTest = true;					
				break;
		}

		//if (!alphaTest) {
		//	return;
		//}
	}

	if (m_State->m_StencilTestEnabled) {

		//bool stencilTest;
		U32 stencilRef = m_State->m_StencilReference & m_State->m_StencilMask;
		//U32 stencilValue = m_Surface->GetStencilBuffer()[offset];
		//U32 stencil = stencilValue & m_State->m_StencilMask;

		switch (m_State->m_StencilFunc) {
			default:
			case RasterizerState::CompFuncNever:	
				//stencilTest = false;				
				break;

			case RasterizerState::CompFuncLess:		
				//stencilTest = stencil < stencilRef;	
				break;

			case RasterizerState::CompFuncEqual:	
				//stencilTest = stencil == stencilRef;
				break;

			case RasterizerState::CompFuncLEqual:	
				//stencilTest = stencil <= stencilRef;
				break;

			case RasterizerState::CompFuncGreater:	
				//stencilTest = stencil > stencilRef;	
				break;

			case RasterizerState::CompFuncNotEqual:	
				//stencilTest = stencil != stencilRef;
				break;

			case RasterizerState::CompFuncGEqual:	
				//stencilTest = stencil >= stencilRef;
				break;

			case RasterizerState::CompFuncAlways:	
				//stencilTest = true;					
				break;
		}

		//if (!stencilTest) {

			switch (m_State->m_StencilFail) {
				default:
				case RasterizerState::StencilOpKeep: 
					break;

				case RasterizerState::StencilOpZero: 
					//stencilValue = 0; 
					break;

				case RasterizerState::StencilOpReplace: 
					//stencilValue = m_State->m_StencilReference; 
					break;

				case RasterizerState::StencilOpIncr: 
					//if (stencilValue != 0xffffffff) {
					//	stencilValue++; 
					//}
					
					break;

				case RasterizerState::StencilOpDecr: 
					//if (stencilValue != 0) {
					//	stencilValue--; 
					//}
					
					break;

				case RasterizerState::StencilOpInvert: 
					//stencilValue = ~stencilValue; 
					break;
			}

			//m_Surface->GetStencilBuffer()[offset] = stencilValue;
			//return;
		//}

		//if (depthTest) {
			switch (m_State->m_StencilZPass) {
				default:
				case RasterizerState::StencilOpKeep: 
					break;

				case RasterizerState::StencilOpZero: 
					//stencilValue = 0; 
					break;

				case RasterizerState::StencilOpReplace: 
					//stencilValue = m_State->m_StencilReference; 
					break;

				case RasterizerState::StencilOpIncr: 
					//if (stencilValue != 0xffffffff) {
					//	stencilValue++; 
					//}
					
					break;

				case RasterizerState::StencilOpDecr: 
					//if (stencilValue != 0) {
					//	stencilValue--; 
					//}
					
					break;

				case RasterizerState::StencilOpInvert: 
					//stencilValue = ~stencilValue; 
					break;
			}

			//m_Surface->GetStencilBuffer()[offset] = stencilValue;
		//} else {
			switch (m_State->m_StencilZFail) {
				default:
				case RasterizerState::StencilOpKeep: 
					break;

				case RasterizerState::StencilOpZero: 
					//stencilValue = 0; 
					break;

				case RasterizerState::StencilOpReplace: 
					//stencilValue = m_State->m_StencilReference; 
					break;

				case RasterizerState::StencilOpIncr: 
					//if (stencilValue != 0xffffffff) {
					//	stencilValue++; 
					//}
					
					break;

				case RasterizerState::StencilOpDecr: 
					//if (stencilValue != 0) {
					//	stencilValue--; 
					//}
					
					break;

				case RasterizerState::StencilOpInvert: 
					//stencilValue = ~stencilValue; 
					break;
			}

			//m_Surface->GetStencilBuffer()[offset] = stencilValue;
		//}
	}

	if (m_State->m_StencilTestEnabled && m_State->m_DepthTestEnabled) {
		// otherwise we returned at the top
		//if (!depthTest)
		//	return;
	}

	//U16 dstValue = m_Surface->GetColorBuffer()[offset];
	//U8 dstAlpha = m_Surface->GetAlphaBuffer()[offset];

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
	}

	//Color maskedColor = 
	//	color.Mask(m_State->m_MaskRed, m_State->m_MaskGreen, m_State->m_MaskBlue, m_State->m_MaskAlpha);

	if (m_State->m_LogicOpEnabled) {

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
		//m_Surface->GetColorBuffer()[offset] = maskedColor.ConvertTo565();

		if (m_State->m_MaskAlpha) {
			//m_Surface->GetAlphaBuffer()[offset] = maskedColor.A();
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
void Rasterizer :: GenerateRasterScanLine() {

	Module * module = new Module("Scanline");
	I32 nextRegister = 0;

	// The signature of the generated function is:
	//	(const RasterInfo * info, const EdgePos& start, const EdgePos& end, U32 y);
	// In the edge buffers, z, tu and tv are actually divided by w

	I32 regInfo = nextRegister++;		// virtual register containing info structure pointer
	I32 regStart = nextRegister++;		// virtual register containing start edge buffer pointer
	I32 regEnd = nextRegister++;		// virtual register containing end edge buffer pointer
	I32 regY = nextRegister++;			// virtual register containing y coordinate

	Label * label0 = Label::create("RasterScanline", LabelProcedure);
	Procedure * procedure = new Procedure(label0, module, nextRegister, 0);
	label0->procedure.procedure = procedure;
	module->procedures.push_back(procedure);

	Block & block0 = *new Block(procedure);
	procedure->blocks.push_back(&block0);

	// Create instructions to calculate addresses of individual fields of
	// edge buffer input arguments

	// texture data, width, height, exponent
	I32 regTextureData =		LOAD_DATA(block0, nextRegister, regInfo, OFFSET_TEXTURE_DATA);
	I32 regTextureWidth =		LOAD_DATA(block0, nextRegister, regInfo, OFFSET_TEXTURE_WIDTH);
	I32 regTextureHeight =		LOAD_DATA(block0, nextRegister, regInfo, OFFSET_TEXTURE_HEIGHT);
	I32 regTextureExponent =	LOAD_DATA(block0, nextRegister, regInfo, OFFSET_TEXTURE_EXPONENT);


	// surface color buffer, depth buffer, alpha buffer, stencil buffer
	I32 regColorBuffer =		LOAD_DATA(block0, nextRegister, regInfo, OFFSET_SURFACE_COLOR_BUFFER);
	I32 regDepthBuffer =		LOAD_DATA(block0, nextRegister, regInfo, OFFSET_SURFACE_DEPTH_BUFFER);
	I32 regAlphaBuffer =		LOAD_DATA(block0, nextRegister, regInfo, OFFSET_SURFACE_ALPHA_BUFFER);
	I32 regStencilBuffer =		LOAD_DATA(block0, nextRegister, regInfo, OFFSET_SURFACE_STENCIL_BUFFER);
	I32 regSurfaceWidth =		LOAD_DATA(block0, nextRegister, regInfo, OFFSET_SURFACE_WIDTH);

	// x coordinate
	I32 regOffsetWindowX = nextRegister++;
	I32 regAddrStartWindowX = nextRegister++;
	I32 regAddrEndWindowX = nextRegister++;

	block0 += 		IMMEDIATE	(ldi,	regOffsetWindowX, Constant::createInt(OFFSET_EDGE_BUFFER_WINDOW_X));
	block0 +=		BINARY		(add,	regAddrStartWindowX, regStart, regOffsetWindowX);
	block0 +=		BINARY		(add,	regAddrEndWindowX, regEnd, regOffsetWindowX);

	// z coordinate
	I32 regOffsetWindowZ = nextRegister++;
	I32 regAddrStartWindowZ = nextRegister++;
	I32 regAddrEndWindowZ = nextRegister++;

	block0 +=		IMMEDIATE	(ldi,	regOffsetWindowZ, Constant::createInt(OFFSET_EDGE_BUFFER_WINDOW_Z));
	block0 +=		BINARY		(add,	regAddrStartWindowZ, regStart, regOffsetWindowZ);
	block0 +=		BINARY		(add,	regAddrEndWindowZ, regEnd, regOffsetWindowZ);

	// u texture coordinate
	I32 regOffsetTextureU = nextRegister++;
	I32 regAddrStartTextureU = nextRegister++;
	I32 regAddrEndTextureU = nextRegister++;

	block0 +=		IMMEDIATE	(ldi,	regOffsetTextureU, Constant::createInt(OFFSET_EDGE_BUFFER_TEX_TU));
	block0 +=		BINARY		(add,	regAddrStartTextureU, regStart, regOffsetTextureU);
	block0 +=		BINARY		(add,	regAddrEndTextureU, regEnd, regOffsetTextureU);

	// v texture coordinate
	I32 regOffsetTextureV = nextRegister++;
	I32 regAddrStartTextureV = nextRegister++;
	I32 regAddrEndTextureV = nextRegister++;

	block0 +=		IMMEDIATE	(ldi,	regOffsetTextureV, Constant::createInt(OFFSET_EDGE_BUFFER_TEX_TV));
	block0 +=		BINARY		(add,	regAddrStartTextureV, regStart, regOffsetTextureV);
	block0 +=		BINARY		(add,	regAddrEndTextureV, regEnd, regOffsetTextureV);

	// r color component
	I32 regOffsetColorR = nextRegister++;
	I32 regAddrStartColorR = nextRegister++;
	I32 regAddrEndColorR = nextRegister++;

	block0 +=		IMMEDIATE	(ldi,	regOffsetColorR, Constant::createInt(OFFSET_EDGE_BUFFER_COLOR_R));
	block0 +=		BINARY		(add,	regAddrStartColorR, regStart, regOffsetColorR);
	block0 +=		BINARY		(add,	regAddrEndColorR, regEnd, regOffsetColorR);

	// g color component
	I32 regOffsetColorG = nextRegister++;
	I32 regAddrStartColorG = nextRegister++;
	I32 regAddrEndColorG = nextRegister++;

	block0 +=		IMMEDIATE	(ldi,	regOffsetColorG, Constant::createInt(OFFSET_EDGE_BUFFER_COLOR_G));
	block0 +=		BINARY		(add,	regAddrStartColorG, regStart, regOffsetColorG);
	block0 +=		BINARY		(add,	regAddrEndColorG, regEnd, regOffsetColorG);

	// b color component
	I32 regOffsetColorB = nextRegister++;
	I32 regAddrStartColorB = nextRegister++;
	I32 regAddrEndColorB = nextRegister++;

	block0 +=	IMMEDIATE		(ldi,	regOffsetColorB, Constant::createInt(OFFSET_EDGE_BUFFER_COLOR_B));
	block0 +=	BINARY			(add,	regAddrStartColorB, regStart, regOffsetColorB);
	block0 +=	BINARY			(add,	regAddrEndColorB, regEnd, regOffsetColorB);

	// a color component
	I32 regOffsetColorA = nextRegister++;
	I32 regAddrStartColorA = nextRegister++;
	I32 regAddrEndColorA = nextRegister++;

	block0 +=	IMMEDIATE		(ldi,	regOffsetColorA, Constant::createInt(OFFSET_EDGE_BUFFER_COLOR_A));
	block0 +=	BINARY			(add,	regAddrStartColorA, regStart, regOffsetColorA);
	block0 +=	BINARY			(add,	regAddrEndColorA, regEnd, regOffsetColorA);

	// fog density
	I32 regOffsetFog = nextRegister++;
	I32 regAddrStartFog = nextRegister++;
	I32 regAddrEndFog = nextRegister++;

	block0 +=	IMMEDIATE		(ldi,	regOffsetFog, Constant::createInt(OFFSET_EDGE_BUFFER_FOG));
	block0 +=	BINARY			(add,	regAddrStartFog, regStart, regOffsetFog);
	block0 +=	BINARY			(add,	regAddrEndFog, regEnd, regOffsetFog);

	//EGL_Fixed invSpan = EGL_Inverse(end.m_WindowCoords.x - start.m_WindowCoords.x);
	I32 regEndWindowX = nextRegister++;
	I32 regStartWindowX = nextRegister++;
	I32 regDiffX = nextRegister++;
	I32 regInvSpan = nextRegister++;

	block0 +=	LOAD			(ldw,	regEndWindowX, regAddrEndWindowX);
	block0 +=	LOAD			(ldw,	regStartWindowX, regAddrStartWindowX);
	block0 +=	BINARY			(sub,	regDiffX, regEndWindowX, regStartWindowX);
	block0 +=	UNARY			(finv,	regInvSpan, regDiffX);

	//FractionalColor baseColor = start.m_Color;
	I32 regStartColorR = nextRegister++;
	I32 regStartColorG = nextRegister++;
	I32 regStartColorB = nextRegister++;
	I32 regStartColorA = nextRegister++;

	block0 +=	LOAD			(ldw,	regStartColorR, regAddrStartColorR);
	block0 +=	LOAD			(ldw,	regStartColorG, regAddrStartColorG);
	block0 +=	LOAD			(ldw,	regStartColorB, regAddrStartColorB);
	block0 +=	LOAD			(ldw,	regStartColorA, regAddrStartColorA);

	//FractionalColor colorIncrement = (end.m_Color - start.m_Color) * invSpan;
	I32 regEndColorR = nextRegister++;
	I32 regEndColorG = nextRegister++;
	I32 regEndColorB = nextRegister++;
	I32 regEndColorA = nextRegister++;

	block0 +=	LOAD			(ldw,	regEndColorR, regAddrEndColorR);
	block0 +=	LOAD			(ldw,	regEndColorG, regAddrEndColorG);
	block0 +=	LOAD			(ldw,	regEndColorB, regAddrEndColorB);
	block0 +=	LOAD			(ldw,	regEndColorA, regAddrEndColorA);

	I32 regDiffColorR = nextRegister++;
	I32 regDiffColorG = nextRegister++;
	I32 regDiffColorB = nextRegister++;
	I32 regDiffColorA = nextRegister++;

	block0 +=	BINARY			(sub,	regDiffColorR, regEndColorR, regStartColorR);
	block0 +=	BINARY			(sub,	regDiffColorG, regEndColorG, regStartColorG);
	block0 +=	BINARY			(sub,	regDiffColorB, regEndColorB, regStartColorB);
	block0 +=	BINARY			(sub,	regDiffColorA, regEndColorA, regStartColorA);

	I32 regColorIncrementR = nextRegister++;
	I32 regColorIncrementG = nextRegister++;
	I32 regColorIncrementB = nextRegister++;
	I32 regColorIncrementA = nextRegister++;

	block0 +=	BINARY			(fmul,	regColorIncrementR, regDiffColorR, regInvSpan);
	block0 +=	BINARY			(fmul,	regColorIncrementG, regDiffColorG, regInvSpan);
	block0 +=	BINARY			(fmul,	regColorIncrementB, regDiffColorB, regInvSpan);
	block0 +=	BINARY			(fmul,	regColorIncrementA, regDiffColorA, regInvSpan);

	//EGL_Fixed deltaInvZ = EGL_Mul(end.m_WindowCoords.z - start.m_WindowCoords.z, invSpan);
	I32 regEndWindowZ = nextRegister++;
	I32 regStartWindowZ = nextRegister++;
	I32 regDiffInvZ = nextRegister++;
	I32 regDeltaInvZ = nextRegister++;

	block0 +=	LOAD			(ldw,	regEndWindowZ, regAddrEndWindowZ);
	block0 +=	LOAD			(ldw,	regStartWindowZ, regAddrStartWindowZ);
	block0 +=	BINARY			(sub,	regDiffInvZ, regEndWindowZ, regStartWindowZ);
	block0 +=	BINARY			(fmul,	regDeltaInvZ, regDiffInvZ, regInvSpan);

	//EGL_Fixed deltaInvU = EGL_Mul(end.m_TextureCoords.tu - start.m_TextureCoords.tu, invSpan);
	I32 regEndTextureU = nextRegister++;
	I32 regStartTextureU = nextRegister++;
	I32 regDiffInvU = nextRegister++;
	I32 regDeltaInvU = nextRegister++;

	block0 +=	LOAD			(ldw,	regEndTextureU, regAddrEndTextureU);
	block0 +=	LOAD			(ldw,	regStartTextureU, regAddrStartTextureU);
	block0 +=	BINARY			(sub,	regDiffInvU, regEndTextureU, regStartTextureU);
	block0 +=	BINARY			(fmul,	regDeltaInvU, regDiffInvU, regInvSpan);

	//EGL_Fixed deltaInvV = EGL_Mul(end.m_TextureCoords.tv - start.m_TextureCoords.tv, invSpan);
	I32 regEndTextureV = nextRegister++;
	I32 regStartTextureV = nextRegister++;
	I32 regDiffInvV = nextRegister++;
	I32 regDeltaInvV = nextRegister++;

	block0 +=	LOAD			(ldw,	regEndTextureV, regAddrEndTextureV);
	block0 +=	LOAD			(ldw,	regStartTextureV, regAddrStartTextureV);
	block0 +=	BINARY			(sub,	regDiffInvV, regEndTextureV, regStartTextureV);
	block0 +=	BINARY			(fmul,	regDeltaInvV, regDiffInvV, regInvSpan);

	//EGL_Fixed deltaFog = EGL_Mul(end.m_FogDensity - start.m_FogDensity, invSpan);
	I32 regEndFog = nextRegister++;
	I32 regStartFog = nextRegister++;
	I32 regDiffFog = nextRegister++;
	I32 regDeltaFog = nextRegister++;

	block0 +=	LOAD			(ldw,	regEndFog, regAddrEndFog);
	block0 +=	LOAD			(ldw,	regStartFog, regAddrStartFog);
	block0 +=	BINARY			(sub,	regDiffFog, regEndFog, regStartFog);
	block0 +=	BINARY			(fmul,	regDeltaFog, regDiffFog, regInvSpan);

	//EGL_Fixed invTu = start.m_TextureCoords.tu;
	//EGL_Fixed invTv = start.m_TextureCoords.tv;
	//EGL_Fixed invZ = start.m_WindowCoords.z;
	//EGL_Fixed fogDensity = start.m_FogDensity;

	//EGL_Fixed z = EGL_Inverse(invZ);
	//EGL_Fixed tu = EGL_Mul(invTu, z);
	//EGL_Fixed tv = EGL_Mul(invTv, z);
	I32 regZ = nextRegister++;
	I32 regU = nextRegister++;
	I32 regV = nextRegister++;

	block0 +=	UNARY			(finv,	regZ, regStartWindowZ);
	block0 +=	BINARY			(fmul,	regU, regStartTextureU, regZ);
	block0 +=	BINARY			(fmul,	regV, regStartTextureV, regZ);

	//I32 x = EGL_IntFromFixed(start.m_WindowCoords.x);
	//I32 xEnd = EGL_IntFromFixed(end.m_WindowCoords.x);
	I32 regX = nextRegister++;
	I32 regXEnd = nextRegister++;

	block0 +=	UNARY			(trunc,	regX, regStartWindowX);
	block0 +=	UNARY			(trunc,	regXEnd, regEndWindowX);

	//I32 xLinEnd = x + ((xEnd - x) & ~(LINEAR_SPAN - 1));
	I32 regSpanMask = nextRegister++;
	I32 regMaskedSpan = nextRegister++;
	I32 regXLinEnd = nextRegister++;
	I32 regCompare0 = nextRegister++;
	I32 regIntDiffX = nextRegister++;

	block0 +=	IMMEDIATE		(ldi,	regSpanMask, Constant::createInt(~(LINEAR_SPAN - 1)));
	block0 +=	UNARY			(trunc,	regIntDiffX, regDiffX);
	block0 +=	BINARY			(and,	regMaskedSpan, regCompare0, regIntDiffX, regSpanMask);
	block0 +=	BINARY			(add,	regXLinEnd, regX, regMaskedSpan);

	//for (; x < xLinEnd;) {

	Label * endLoop0 = Label::create("endLoop0", LabelBlock);

	block0 +=	BCOND			(beq,	regCompare0, endLoop0);

	Label * beginLoop0 = Label::create("beginLoop0", LabelBlock);
	Block& block1 = *new Block(procedure);
	procedure->blocks.push_back(&block1);
	beginLoop0->block.block = &block1;
	block1.labels.push_back(beginLoop0);

	// Here we define all the loop registers and phi mappings
	I32 regLoop0ZEntry = nextRegister++;
	I32 regLoop0Z = nextRegister++;
	I32 regLoop1Z = nextRegister++;

	block1 +=	PHI				(phi, regLoop0ZEntry, REG_LIST(regZ, regLoop0Z, regLoop1Z));

	I32 regLoop0UEntry = nextRegister++;
	I32 regLoop0U = nextRegister++;
	I32 regLoop1U = nextRegister++;

	block1 +=	PHI				(phi, regLoop0UEntry, REG_LIST(regU, regLoop0U, regLoop1U));

	I32 regLoop0VEntry = nextRegister++;
	I32 regLoop0V = nextRegister++;
	I32 regLoop1V = nextRegister++;

	block1 +=	PHI				(phi, regLoop0VEntry, REG_LIST(regV, regLoop0V, regLoop1V));

	I32 regLoop0InvZEntry = nextRegister++;
	I32 regLoop0InvZ = nextRegister++;

	block1 +=	PHI				(phi, regLoop0InvZEntry, REG_LIST(regStartWindowZ, regLoop0InvZ));

	I32 regLoop0InvUEntry = nextRegister++;
	I32 regLoop0InvU = nextRegister++;

	block1 +=	PHI				(phi, regLoop0InvUEntry, REG_LIST(regStartTextureU, regLoop0InvU));

	I32 regLoop0InvVEntry = nextRegister++;
	I32 regLoop0InvV = nextRegister++;

	block1 +=	PHI				(phi, regLoop0InvVEntry, REG_LIST(regStartTextureV, regLoop0InvV));

		//invZ += deltaInvZ << LOG_LINEAR_SPAN;
		//invTu += deltaInvU << LOG_LINEAR_SPAN;
		//invTv += deltaInvV << LOG_LINEAR_SPAN;
	I32 regLinearSpan = nextRegister++;
	I32 regConstant1 = nextRegister++;
	I32 regDeltaInvZTimesLinearSpan = nextRegister++;
	I32 regDeltaInvUTimesLinearSpan = nextRegister++;
	I32 regDeltaInvVTimesLinearSpan = nextRegister++;

	block1 +=		IMMEDIATE	(ldi,	regConstant1, Constant::createInt(1));
	block1 +=		IMMEDIATE	(ldi,	regLinearSpan, Constant::createInt(LINEAR_SPAN));
	block1 +=		BINARY		(fmul,	regDeltaInvZTimesLinearSpan, regDeltaInvZ, regLinearSpan);	
	block1 +=		BINARY		(fadd,	regLoop0InvZ, regLoop0InvZEntry, regDeltaInvZTimesLinearSpan);
	block1 +=		BINARY		(fmul,	regDeltaInvUTimesLinearSpan, regDeltaInvU, regLinearSpan);	
	block1 +=		BINARY		(fadd,	regLoop0InvU, regLoop0InvUEntry, regDeltaInvUTimesLinearSpan);
	block1 +=		BINARY		(fmul,	regDeltaInvVTimesLinearSpan, regDeltaInvV, regLinearSpan);	
	block1 +=		BINARY		(fadd,	regLoop0InvV, regLoop0InvVEntry, regDeltaInvVTimesLinearSpan);

		//EGL_Fixed endZ = EGL_Inverse(invZ);
		//EGL_Fixed endTu = EGL_Mul(invTu, endZ);
		//EGL_Fixed endTv = EGL_Mul(invTv, endZ);

	I32 regLoop0EndZ = nextRegister++;
	I32 regLoop0EndU = nextRegister++;
	I32 regLoop0EndV = nextRegister++;

	block1 +=		UNARY		(finv,	regLoop0EndZ, regLoop0InvZ);
	block1 +=		BINARY		(fmul,	regLoop0EndU, regLoop0InvU, regLoop0EndZ);
	block1 +=		BINARY		(fmul,	regLoop0EndV, regLoop0InvV, regLoop0EndZ);

		//EGL_Fixed deltaZ = (endZ - z) >> LOG_LINEAR_SPAN;
		//EGL_Fixed deltaTu = (endTu - tu) >> LOG_LINEAR_SPAN; 
		//EGL_Fixed deltaTv = (endTv - tv) >> LOG_LINEAR_SPAN;

	I32 regLoop0DiffZ = nextRegister++;
	I32 regLoop0ScaledDiffZ = nextRegister++;
	I32 regLoop0DiffU = nextRegister++;
	I32 regLoop0ScaledDiffU = nextRegister++;
	I32 regLoop0DiffV = nextRegister++;
	I32 regLoop0ScaledDiffV = nextRegister++;

	block1 +=		BINARY		(fsub,	regLoop0DiffZ, regLoop0EndZ, regLoop0Z);
	block1 +=		BINARY		(fdiv,	regLoop0ScaledDiffZ, regLoop0DiffZ, regLinearSpan);
	block1 +=		BINARY		(fsub,	regLoop0DiffU, regLoop0EndU, regLoop0U);
	block1 +=		BINARY		(fdiv,	regLoop0ScaledDiffU, regLoop0DiffU, regLinearSpan);
	block1 +=		BINARY		(fsub,	regLoop0DiffV, regLoop0EndV, regLoop0V);
	block1 +=		BINARY		(fdiv,	regLoop0ScaledDiffV, regLoop0DiffV, regLinearSpan);

	// also not to include phi projection for z coming from inner loop

		//int count = LINEAR_SPAN; 

	Label * beginLoop1 = Label::create("beginLoop1", LabelBlock);
	Label * endLoop1 = Label::create("endLoop1", LabelBlock);
	Label * postFragmentLoop1 = Label::create("postFragmentLoop1", LabelBlock);

	Block& block2 = *new Block(procedure);
	procedure->blocks.push_back(&block2);
	block2.labels.push_back(beginLoop1);
	beginLoop1->block.block = &block2;

		//do {

	// phi for count, x, z, tu, tv, fog, r, g, b, a

	I32 regLoop1CountEntry = nextRegister++;
	I32 regLoop1Count = nextRegister++;
	I32 regLoop1XEntry = nextRegister++;
	I32 regLoop1X = nextRegister++;
	I32 regLoop1ZEntry = nextRegister++;
	I32 regLoop1UEntry = nextRegister++;
	I32 regLoop1VEntry = nextRegister++;
	I32 regLoop1FogEntry = nextRegister++;
	I32 regLoop1Fog = nextRegister++;
	I32 regLoop1REntry = nextRegister++;
	I32 regLoop1R = nextRegister++;
	I32 regLoop1GEntry = nextRegister++;
	I32 regLoop1G = nextRegister++;
	I32 regLoop1BEntry = nextRegister++;
	I32 regLoop1B = nextRegister++;
	I32 regLoop1AEntry = nextRegister++;
	I32 regLoop1A = nextRegister++;


	block2 +=		PHI			(phi,	regLoop1CountEntry, REG_LIST(regLoop1Count, regLinearSpan));
	block2 +=		PHI			(phi,	regLoop1XEntry, REG_LIST(regLoop1X, regX));
	block2 +=		PHI			(phi,	regLoop1ZEntry, REG_LIST(regLoop1Z, regZ));
	block2 +=		PHI			(phi,	regLoop1UEntry, REG_LIST(regLoop1U, regU));
	block2 +=		PHI			(phi,	regLoop1VEntry, REG_LIST(regLoop1V, regV));
	block2 +=		PHI			(phi,	regLoop1FogEntry, REG_LIST(regLoop1Fog, regStartFog));
	block2 +=		PHI			(phi,	regLoop1REntry, REG_LIST(regLoop1R, regStartColorR));
	block2 +=		PHI			(phi,	regLoop1GEntry, REG_LIST(regLoop1G, regStartColorG));
	block2 +=		PHI			(phi,	regLoop1BEntry, REG_LIST(regLoop1B, regStartColorB));
	block2 +=		PHI			(phi,	regLoop1AEntry, REG_LIST(regLoop1A, regStartColorA));
		
	FragmentGenerationInfo info;
	info.regX = regLoop1XEntry;
	info.regY = regY; 
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

	GenerateFragment(procedure, block2, postFragmentLoop1, nextRegister, info); 

	Block& block3 = *new Block(procedure);
	procedure->blocks.push_back(&block3);
	block3.labels.push_back(postFragmentLoop1);
	postFragmentLoop1->block.block = &block3;

			//baseColor += colorIncrement;
	block3 +=		BINARY		(fadd, regLoop1R, regLoop1REntry, regColorIncrementR);
	block3 +=		BINARY		(fadd, regLoop1G, regLoop1REntry, regColorIncrementG);
	block3 +=		BINARY		(fadd, regLoop1B, regLoop1REntry, regColorIncrementB);
	block3 +=		BINARY		(fadd, regLoop1A, regLoop1REntry, regColorIncrementA);

			//fogDensity += deltaFog;
			//z += deltaZ;
			//tu += deltaTu;
			//tv += deltaTv;
	block3 +=		BINARY		(fadd, regLoop1Fog, regLoop1FogEntry, regDeltaFog);
	block3 +=		BINARY		(fadd, regLoop1Z, regLoop1ZEntry, regLoop0ScaledDiffZ);
	block3 +=		BINARY		(fadd, regLoop1U, regLoop1UEntry, regLoop0ScaledDiffU);
	block3 +=		BINARY		(fadd, regLoop1V, regLoop1VEntry, regLoop0ScaledDiffV);

			//++x;
	block3 +=		BINARY	(add, regLoop1X, regLoop1XEntry, regConstant1);

		//} while (--count);
	I32 regLoop1Condition = nextRegister++;

	block3 +=		BINARY		(sub,	regLoop1Count, regLoop1Condition, regLoop1CountEntry, regConstant1);
	block3 +=		BCOND		(bne,	regLoop1Condition, beginLoop1);
	//}

	I32 regLoop0Condition = nextRegister++;
	block3 +=		COMPARE		(cmp,	regLoop0Condition, regLoop1X, regXLinEnd);
	block3 +=		BCOND		(bne,	regLoop0Condition, beginLoop0);

	//if (x != xEnd) {
	Block& block4 = *new Block(procedure);
	procedure->blocks.push_back(&block4);
	endLoop0->block.block = &block4;
	block4.labels.push_back(endLoop0);

	Label * beginLoop2 = Label::create("beginLoop2", LabelBlock);
	Label * endLoop2 = Label::create("endLoop2", LabelBlock);
	Label * postFragmentLoop2 = Label::create("postFragmentLoop2", LabelBlock);

	I32 regBlock4X = nextRegister++;
	I32 regBlock4Z = nextRegister++;
	I32 regBlock4U = nextRegister++;
	I32 regBlock4V = nextRegister++;
	I32 regBlock4DiffX = nextRegister++;
	I32 regBlock4Condition = nextRegister++;

	block4 +=		PHI			(phi,	regBlock4X, REG_LIST(regX, regLoop1X));
	block4 +=		PHI			(phi,	regBlock4Z, REG_LIST(regLoop1Z, regZ));
	block4 +=		PHI			(phi,	regBlock4U, REG_LIST(regLoop1U, regU));
	block4 +=		PHI			(phi,	regBlock4V, REG_LIST(regLoop1V, regV));
	block4 +=		BINARY		(cmp,	regBlock4DiffX, regBlock4Condition, regXEnd, regBlock4X);
	block4 +=		BCOND		(beq,	regBlock4Condition, endLoop2);

		//EGL_Fixed endZ = EGL_Inverse(end.m_WindowCoords.z);
		//EGL_Fixed endTu = EGL_Mul(end.m_TextureCoords.tu, endZ);
		//EGL_Fixed endTv = EGL_Mul(end.m_TextureCoords.tv, endZ);
	I32 regEndZ = nextRegister++;
	I32 regEndU = nextRegister++;
	I32 regEndV = nextRegister++;

	block4 +=		UNARY		(finv,	regEndZ, regEndWindowZ);
	block4 +=		BINARY		(fmul,	regEndU, regEndZ, regEndTextureU);
	block4 +=		BINARY		(fmul,	regEndV, regEndZ, regEndTextureV);

		//invSpan = EGL_Inverse(EGL_FixedFromInt(xEnd - x));
	I32 regFixedBlock4DiffX = nextRegister++;
	I32 regBlock4InvSpan = nextRegister++;

	block4 +=		UNARY		(fcnv,	regFixedBlock4DiffX, regBlock4DiffX);
	block4 +=		UNARY		(finv,	regBlock4InvSpan, regFixedBlock4DiffX);

		//EGL_Fixed deltaZ = EGL_Mul(endZ - z, invSpan);
		//EGL_Fixed deltaTu = EGL_Mul(endTu - tu, invSpan);
		//EGL_Fixed deltaTv = EGL_Mul(endTv - tv, invSpan);
	I32 regBlock4DiffZ = nextRegister++;
	I32 regBlock4DiffU = nextRegister++;
	I32 regBlock4DiffV = nextRegister++;
	I32 regLoop2ScaledDiffZ = nextRegister++;
	I32 regLoop2ScaledDiffU = nextRegister++;
	I32 regLoop2ScaledDiffV = nextRegister++;

	block4 +=		BINARY		(fsub,	regBlock4DiffZ, regEndZ, regBlock4Z);
	block4 +=		BINARY		(fmul,	regLoop2ScaledDiffZ, regBlock4DiffZ, regBlock4InvSpan);
	block4 +=		BINARY		(fsub,	regBlock4DiffU, regEndU, regBlock4U);
	block4 +=		BINARY		(fmul,	regLoop2ScaledDiffU, regBlock4DiffU, regBlock4InvSpan);
	block4 +=		BINARY		(fsub,	regBlock4DiffV, regEndV, regBlock4V);
	block4 +=		BINARY		(fmul,	regLoop2ScaledDiffV, regBlock4DiffV, regBlock4InvSpan);

		//for (; x < xEnd; ++x) {
	Block& block5 = *new Block(procedure);
	procedure->blocks.push_back(&block5);
	beginLoop2->block.block = &block5;
	block5.labels.push_back(beginLoop2);

	// phi for x, z, tu, tv, fog, r, g, b, a

	I32 regLoop2XEntry = nextRegister++;
	I32 regLoop2X = nextRegister++;
	I32 regLoop2ZEntry = nextRegister++;
	I32 regLoop2Z = nextRegister++;
	I32 regLoop2UEntry = nextRegister++;
	I32 regLoop2U = nextRegister++;
	I32 regLoop2VEntry = nextRegister++;
	I32 regLoop2V = nextRegister++;
	I32 regLoop2FogEntry = nextRegister++;
	I32 regLoop2Fog = nextRegister++;
	I32 regLoop2REntry = nextRegister++;
	I32 regLoop2R = nextRegister++;
	I32 regLoop2GEntry = nextRegister++;
	I32 regLoop2G = nextRegister++;
	I32 regLoop2BEntry = nextRegister++;
	I32 regLoop2B = nextRegister++;
	I32 regLoop2AEntry = nextRegister++;
	I32 regLoop2A = nextRegister++;


	block5 +=		PHI			(phi,	regLoop2XEntry, REG_LIST(regLoop2X, regBlock4X));
	block5 +=		PHI			(phi,	regLoop2ZEntry, REG_LIST(regLoop2Z, regBlock4Z));
	block5 +=		PHI			(phi,	regLoop2UEntry, REG_LIST(regLoop2U, regBlock4U));
	block5 +=		PHI			(phi,	regLoop2VEntry, REG_LIST(regLoop2V, regBlock4V));
	block5 +=		PHI			(phi,	regLoop2FogEntry, REG_LIST(regLoop2Fog, regLoop1Fog, regStartFog));
	block5 +=		PHI			(phi,	regLoop2REntry, REG_LIST(regLoop2R, regLoop1R, regStartColorR));
	block5 +=		PHI			(phi,	regLoop2GEntry, REG_LIST(regLoop2G, regLoop1G, regStartColorG));
	block5 +=		PHI			(phi,	regLoop2BEntry, REG_LIST(regLoop2B, regLoop1B, regStartColorB));
	block5 +=		PHI			(phi,	regLoop2AEntry, REG_LIST(regLoop2A, regLoop1A, regStartColorA));

	FragmentGenerationInfo info2;
	info2.regX = regLoop2XEntry;
	info2.regY = regY; 
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

	GenerateFragment(procedure, block5, postFragmentLoop2, nextRegister, info2); 

	Block& block6 = *new Block(procedure);
	procedure->blocks.push_back(&block6);
	postFragmentLoop2->block.block = &block6;
	block6.labels.push_back(postFragmentLoop2);

			//baseColor += colorIncrement;
	block6 +=		BINARY		(fadd,	regLoop2R, regLoop2REntry, regColorIncrementR);
	block6 +=		BINARY		(fadd,	regLoop2G, regLoop2REntry, regColorIncrementG);
	block6 +=		BINARY		(fadd,	regLoop2B, regLoop2REntry, regColorIncrementB);
	block6 +=		BINARY		(fadd,	regLoop2A, regLoop2REntry, regColorIncrementA);

			//fogDensity += deltaFog;
			//z += deltaZ;
			//tu += deltaTu;
			//tv += deltaTv;
	block6 +=		BINARY		(fadd,	regLoop2Fog, regLoop2FogEntry, regDeltaFog);
	block6 +=		BINARY		(fadd,	regLoop2Z, regLoop2ZEntry, regLoop2ScaledDiffZ);
	block6 +=		BINARY		(fadd,	regLoop2U, regLoop2UEntry, regLoop2ScaledDiffU);
	block6 +=		BINARY		(fadd,	regLoop2V, regLoop2VEntry, regLoop2ScaledDiffV);

			//++x;
	block6 +=		BINARY		(add,	regLoop2X, regLoop2XEntry, regConstant1);

		//}

	//}
	Block& block7 = *new Block(procedure);
	procedure->blocks.push_back(&block7);
	endLoop2->block.block = &block7;
	block7.labels.push_back(endLoop2);

	block7 +=		RET			(ret,	new RegisterList());
}

