// ==========================================================================
//
// RasterPart.cpp		JIT Class for 3D Rendering Library
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



#include "stdafx.h"
#include "RasterPart.h"
#include "inline.h"

using namespace EGL;


cg_virtual_reg_t * RasterPart :: Mul255(cg_block_t * block, cg_virtual_reg_t * first, cg_virtual_reg_t * second) {
	cg_proc_t * procedure = block->proc;

	DECL_REG		(regProduct);
	DECL_CONST_REG	(constant8, 8);
	DECL_REG		(regShifted);
	DECL_REG		(regAdjusted);
	DECL_REG		(regFinal);
	
	MUL			(regProduct,	first, second);
	ASR			(regShifted,	regProduct, constant8);
	ADD			(regAdjusted,	regProduct, regShifted);
	ASR			(regFinal,		regAdjusted, constant8);

	return regFinal;
}

cg_virtual_reg_t * RasterPart :: AddSaturate255(cg_block_t * block, cg_virtual_reg_t * first, cg_virtual_reg_t * second) {
	cg_proc_t * procedure = block->proc;

	DECL_REG		(regSum);
	DECL_CONST_REG	(constant255, 0xff);
	DECL_REG		(regResult);

	ADD				(regSum, first, second);
	MIN				(regResult, regSum, constant255);

	return regResult;
}

cg_virtual_reg_t * RasterPart :: ClampTo255(cg_block_t * block, cg_virtual_reg_t * value) {
	cg_proc_t * procedure = block->proc;

	if (value) {
		DECL_CONST_REG	(constant0, 0);
		DECL_CONST_REG	(constant17, 17);
		DECL_CONST_REG	(constant1, 0x10000);

		DECL_REG	(regClamped0);
		DECL_REG	(regClamped1);
		DECL_REG	(regAdjusted);
		DECL_REG	(regResult);

		MAX		(regClamped0, value, constant0);
		MIN		(regClamped1, regClamped0, constant1);

		DECL_CONST_REG	(constantFactor, 0x1ff);

		MUL		(regAdjusted, regClamped1, constantFactor);
		LSR		(regResult, regAdjusted, constant17);

		return regResult;
	} else {
		DECL_CONST_REG	(regResult, 0);

		return regResult;
	}
}

cg_virtual_reg_t * RasterPart :: AddSigned(cg_block_t * block, cg_virtual_reg_t * first, cg_virtual_reg_t * second) {
	cg_proc_t * procedure = block->proc;

	DECL_REG		(regResult);
	DECL_REG		(regSum);
	DECL_REG		(regAdjusted);
	DECL_CONST_REG	(constantHalf, 0x80);
	DECL_CONST_REG	(constant0, 0);

	ADD				(regSum, first, second);
	SUB				(regAdjusted, regSum, constantHalf);
	MAX				(regResult, regAdjusted, constant0);

	return regResult;
}

cg_virtual_reg_t * RasterPart :: Add(cg_block_t * block, cg_virtual_reg_t * first, cg_virtual_reg_t * second) {
	cg_proc_t * procedure = block->proc;

	DECL_REG		(regResult);

	ADD				(regResult, first, second);

	return regResult;
}

cg_virtual_reg_t * RasterPart :: Sub(cg_block_t * block, cg_virtual_reg_t * first, cg_virtual_reg_t * second) {
	cg_proc_t * procedure = block->proc;

	DECL_REG		(regResult);

	SUB				(regResult, first, second);

	return regResult;
}

cg_virtual_reg_t * RasterPart :: ExtractBitFieldTo255(cg_block_t * block, cg_virtual_reg_t * value, size_t low, size_t high) {
	cg_proc_t * procedure = block->proc;
#if 0
	if (high == low) {
		if (high < 8) {
			DECL_REG		(regShifted);
			DECL_CONST_REG	(constantShift, 8 - high);

			LSL				(regShifted, value, constantShift);

			value = regShifted;
		} else if (high > 8) {
			DECL_REG		(regShifted);
			DECL_CONST_REG	(constantShift, high - 8);

			LSR				(regShifted, value, constantShift);

			value = regShifted;
		}

		DECL_CONST_REG	(constantMask,	0x100);
		DECL_REG		(regMasked);

		AND				(regMasked,		value, constantMask);

		DECL_CONST_REG	(constant8,		8);
		DECL_REG		(regShifted);
		DECL_REG		(regAdjusted);

		LSR				(regShifted,	value, constant8);
		SUB				(regAdjusted,	value, regShifted);

		return regAdjusted;
	}
#endif

	if (high < 7) {
		DECL_REG		(regShifted);
		DECL_CONST_REG	(constantShift, 7 - high);

		LSL				(regShifted, value, constantShift);

		value = regShifted;
	} else if (high > 7) {
		DECL_REG		(regShifted);
		DECL_CONST_REG	(constantShift, high - 7);

		LSR				(regShifted, value, constantShift);

		value = regShifted;
	}

	size_t bits = high - low + 1;
	static const U8 mask[9] = { 0, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };

	DECL_CONST_REG		(constantMask,	mask[bits]);
	DECL_REG			(regMasked);

	AND					(regMasked,		value, constantMask);
	value = regMasked;

	while (bits < 8) {
		DECL_CONST_REG	(constantShift,	bits);
		DECL_REG		(regShifted);
		DECL_REG		(regOred);

		LSR				(regShifted,	value, constantShift);
		OR				(regOred,		value, regShifted);

		value = regOred;
		bits *= 2;
	}

	return value;
}

cg_virtual_reg_t * RasterPart :: BitFieldFrom255(cg_block_t * block, cg_virtual_reg_t * value, size_t low, size_t high) {
	cg_proc_t * procedure = block->proc;

	size_t bits = high - low + 1;
	assert(bits <= 8);
	size_t lowBit = 8 - bits;

	if (bits != 8) {
		static const U8 mask[9] = { 0, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };

		DECL_CONST_REG		(constantMask,	mask[bits]);
		DECL_REG			(regMasked);

		AND					(regMasked,		value, constantMask);
		value = regMasked;
	}
	
	if (low > lowBit) {
		DECL_CONST_REG		(constantShift,	low - lowBit);
		DECL_REG			(regShifted);

		LSL					(regShifted,	value, constantShift);
		value = regShifted;
	} else if (low < lowBit) {
		DECL_CONST_REG		(constantShift,	lowBit - low);
		DECL_REG			(regShifted);

		LSR					(regShifted,	value, constantShift);
		value = regShifted;
	}

	return value;
}

// ----------------------------------------------------------------------
// Emit code to convert a representation of a color as individual
// R, G and B components into a 16-bit 565 representation
//
// R, G B are within the range 0..0xff
// ----------------------------------------------------------------------
void RasterPart :: Color565FromRGB(cg_block_t * block, cg_virtual_reg_t * regRGB,
	cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b) {
	cg_proc_t * procedure = block->proc;

	DECL_REG	(regBG);

	cg_virtual_reg_t *	regFieldG = BitFieldFrom255(block, g, 5, 10);
	cg_virtual_reg_t *	regFieldB = BitFieldFrom255(block, b, 0, 4);

	OR			(regBG,		regFieldB, regFieldG);

	cg_virtual_reg_t *	regFieldR = BitFieldFrom255(block, r, 11, 15);

	OR			(regRGB,	regBG, regFieldR);
}

void RasterPart :: Color5551FromRGBA(cg_block_t * block, cg_virtual_reg_t * result,
									 cg_virtual_reg_t * r, cg_virtual_reg_t * g, 
									 cg_virtual_reg_t * b, cg_virtual_reg_t * a) {
	cg_proc_t * procedure = block->proc;

	DECL_REG	(regBG);
	DECL_REG	(regRGB);

	cg_virtual_reg_t *	regFieldG = BitFieldFrom255(block, g, 6, 10);
	cg_virtual_reg_t *	regFieldB = BitFieldFrom255(block, b, 1, 5);

	OR			(regBG,		regFieldB, regFieldG);

	cg_virtual_reg_t *	regFieldR = BitFieldFrom255(block, r, 11, 15);

	OR			(regRGB,	regBG, regFieldR);

	cg_virtual_reg_t *	regFieldA = BitFieldFrom255(block, a, 0, 0);

	OR			(result,	regRGB, regFieldA);
}

void RasterPart :: Color4444FromRGBA(cg_block_t * block, cg_virtual_reg_t * result,
									 cg_virtual_reg_t * r, cg_virtual_reg_t * g, 
									 cg_virtual_reg_t * b, cg_virtual_reg_t * a) {
	cg_proc_t * procedure = block->proc;

	DECL_REG	(regBG);
	DECL_REG	(regRGB);

	cg_virtual_reg_t *	regFieldG = BitFieldFrom255(block, g, 8, 11);
	cg_virtual_reg_t *	regFieldB = BitFieldFrom255(block, b, 4, 7);

	OR			(regBG,		regFieldB, regFieldG);

	cg_virtual_reg_t *	regFieldR = BitFieldFrom255(block, r, 12, 15);

	OR			(regRGB,	regBG, regFieldR);

	cg_virtual_reg_t *	regFieldA = BitFieldFrom255(block, a, 0, 3);

	OR			(result,	regRGB, regFieldA);
}

void RasterPart :: Color8888FromRGBA(cg_block_t * block, cg_virtual_reg_t * result,
									 cg_virtual_reg_t * r, cg_virtual_reg_t * g, 
									 cg_virtual_reg_t * b, cg_virtual_reg_t * a) {
	cg_proc_t * procedure = block->proc;

	DECL_REG	(regBG);
	DECL_REG	(regRGB);

	cg_virtual_reg_t *	regFieldG = BitFieldFrom255(block, g, 16, 23);
	cg_virtual_reg_t *	regFieldB = BitFieldFrom255(block, b, 8, 15);

	OR			(regBG,		regFieldB, regFieldG);

	cg_virtual_reg_t *	regFieldR = BitFieldFrom255(block, r, 24, 31);

	OR			(regRGB,	regBG, regFieldR);

	cg_virtual_reg_t *	regFieldA = BitFieldFrom255(block, a, 0, 7);

	OR			(result,	regRGB, regFieldA);
}

void RasterPart :: ColorWordFromRGBA(cg_block_t * block, cg_virtual_reg_t * result,
									 cg_virtual_reg_t * r, cg_virtual_reg_t * g, 
									 cg_virtual_reg_t * b, cg_virtual_reg_t * a) {
	switch (m_State->GetColorFormat()) {
	case ColorFormatRGB565:		Color565FromRGB(block, result, r, g, b);		break;
	case ColorFormatRGBA4444:	Color4444FromRGBA(block, result, r, g, b, a);	break;
	case ColorFormatRGBA5551:	Color5551FromRGBA(block, result, r, g, b, a);	break;
	case ColorFormatRGBA8:		Color8888FromRGBA(block, result, r, g, b, a);	break;
	default:
		assert(false);
	}
}

cg_virtual_reg_t * RasterPart :: Color565FromRGB(cg_block_t * block,
												 cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b) {
	cg_proc_t * procedure = block->proc;

	DECL_REG	(regResult);

	Color565FromRGB(block, regResult, r, g, b);

	return regResult;
}

cg_virtual_reg_t * RasterPart :: Color5551FromRGBA(cg_block_t * block,
					 cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b, cg_virtual_reg_t * a) {
	cg_proc_t * procedure = block->proc;

	DECL_REG	(regResult);

	Color5551FromRGBA(block, regResult, r, g, b, a);

	return regResult;
}

cg_virtual_reg_t * RasterPart :: Color4444FromRGBA(cg_block_t * block,
					 cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b, cg_virtual_reg_t * a) {
	cg_proc_t * procedure = block->proc;

	DECL_REG	(regResult);

	Color4444FromRGBA(block, regResult, r, g, b, a);

	return regResult;
}

cg_virtual_reg_t * RasterPart :: Color8888FromRGBA(cg_block_t * block,
					 cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b, cg_virtual_reg_t * a) {
	cg_proc_t * procedure = block->proc;

	DECL_REG	(regResult);

	Color8888FromRGBA(block, regResult, r, g, b, a);

	return regResult;
}

cg_virtual_reg_t * RasterPart :: ColorWordFromRGBA(cg_block_t * block,
					 cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b, cg_virtual_reg_t * a) {
	cg_proc_t * procedure = block->proc;

	DECL_REG	(regResult);

	ColorWordFromRGBA(block, regResult, r, g, b, a);

	return regResult;
}

cg_virtual_reg_t * RasterPart :: Blend255(cg_block_t * block, cg_virtual_reg_t * first, cg_virtual_reg_t * second,
											 cg_virtual_reg_t * alpha) {

	cg_proc_t * procedure = block->proc;

	DECL_REG		(regDiff);

	SUB				(regDiff,		second, first);		// diff = (second - first)

	cg_virtual_reg_t *	regProd = Mul255(block, regDiff, alpha);	//	alpha * (second - first)

	return Add(block, first, regProd);					// first + alpha * (second - first)
}


cg_virtual_reg_t * RasterPart :: Blend255(cg_block_t * block, U8 constant, cg_virtual_reg_t * second,
											 cg_virtual_reg_t * alpha) {
	cg_proc_t * procedure = block->proc;

	DECL_CONST_REG	(regConst,	constant);

	return Blend255(block, regConst, second, alpha);
}

cg_virtual_reg_t * RasterPart :: SignedVal(cg_block_t * block, cg_virtual_reg_t * value) {
	cg_proc_t * procedure = block->proc;

	DECL_REG		(regShifted);
	DECL_CONST_REG	(constantShift, 7);
	DECL_REG		(regExpanded);
	DECL_CONST_REG	(c128, 128);
	DECL_REG		(regResult);

	// expand 0..255 -> 0..256
	LSR				(regShifted, value, constantShift);
	ADD				(regExpanded, value, regShifted);
	SUB				(regResult, regExpanded, c128);

	return regResult;
}

cg_virtual_reg_t * RasterPart :: Dot3(cg_block_t * block, 
										 cg_virtual_reg_t * r[], cg_virtual_reg_t * g[], cg_virtual_reg_t * b[]) {
	cg_proc_t * procedure = block->proc;

	DECL_REG		(regProdR);
	DECL_REG		(regProdG);
	DECL_REG		(regProdB);
	DECL_REG		(regSumRG);
	DECL_REG		(regSumRGB);

	MUL				(regProdR, SignedVal(block, r[0]), SignedVal(block, r[1]));
	MUL				(regProdG, SignedVal(block, g[0]), SignedVal(block, g[1]));
	ADD				(regSumRG, regProdR, regProdG);
	MUL				(regProdB, SignedVal(block, b[0]), SignedVal(block, b[1]));
	ADD				(regSumRGB, regSumRG, regProdB);

	DECL_CONST_REG	(constant6, 6);
	DECL_CONST_REG	(constant7, 7);
	DECL_REG		(regShifted6);
	DECL_REG		(regShifted13);
	DECL_REG		(regAdjusted);

	ASR				(regShifted6, regSumRGB, constant6);
	ASR				(regShifted13, regShifted6, constant7);
	SUB				(regAdjusted, regShifted6, regShifted13);

	DECL_REG		(regClamped0);
	DECL_REG		(regClamped255);

	DECL_CONST_REG	(constant0, 0);
	DECL_CONST_REG	(constant255, 255);

	MAX				(regClamped0, regAdjusted, constant0);
	MIN				(regClamped255, regClamped0, constant255);

	return regClamped255;
}


void RasterPart :: FetchTexColor(cg_proc_t * procedure, cg_block_t * currentBlock,
 								    const RasterizerState::TextureState * textureState,
								    cg_virtual_reg_t * regTextureData, 
								    cg_virtual_reg_t * regTexOffset,
								    cg_virtual_reg_t *& regTexColorR,
								    cg_virtual_reg_t *& regTexColorG,			
								    cg_virtual_reg_t *& regTexColorB,			
								    cg_virtual_reg_t *& regTexColorA,
								    cg_virtual_reg_t *& regTexColorWord) {

	cg_block_t * block = currentBlock;

	regTexColorWord = 0;

	switch (textureState->InternalFormat) {
		case ColorFormatAlpha:				// 8
			{
				//texColor = Color(0, 0, 0, reinterpret_cast<const U8 *>(data)[texOffset]);
				regTexColorR = regTexColorG = regTexColorB = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);

				DECL_REG	(regTexAddr);
				

				ADD		(regTexAddr, regTexOffset, regTextureData);
				LDB		(regTexColorA, regTexAddr);
				LDI		(regTexColorR, 0);

			}
			break;

		case ColorFormatLuminance:			// 8
			{
				//U8 luminance = reinterpret_cast<const U8 *>(data)[texOffset];
				//texColor = Color (luminance, luminance, luminance, 0xff);
				regTexColorR = regTexColorB = regTexColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);

				DECL_REG	(regTexAddr);

				ADD		(regTexAddr, regTexOffset, regTextureData);
				LDB		(regTexColorR, regTexAddr);
				LDI		(regTexColorA, 0xff);
			}
			break;

		case ColorFormatLuminanceAlpha:		// 8-8
			{
				//U8 luminance = reinterpret_cast<const U8 *>(data)[texOffset * 2];
				//U8 alpha = reinterpret_cast<const U8 *>(data)[texOffset * 2 + 1];
				//texColor = Color (luminance, luminance, luminance, alpha);
				regTexColorR = regTexColorB = regTexColorG = cg_virtual_reg_create(procedure, cg_reg_type_general);
				regTexColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);

				DECL_REG		(regScaledOffset);
				DECL_REG		(regTexAddr);
				DECL_REG		(regTexAddr1);
				DECL_CONST_REG	(regConstantOne, 1);

				LSL		(regScaledOffset, regTexOffset, regConstantOne);
				ADD		(regTexAddr, regScaledOffset, regTextureData);
				LDB		(regTexColorR, regTexAddr);
				ADD		(regTexAddr1, regTexAddr, regConstantOne);
				LDB		(regTexColorA, regTexAddr1);
			}
			break;

		case ColorFormatRGB565:					// 5-6-5
			//texColor = Color::From565(reinterpret_cast<const U16 *>(data)[texOffset]);
			{
				cg_virtual_reg_t * regTexColor565 = cg_virtual_reg_create(procedure, cg_reg_type_general);

				regTexColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);

				DECL_REG	(regScaledOffset);
				DECL_REG	(regConstantOne);
				DECL_REG	(regTexAddr);

				LDI		(regConstantOne, 1);
				LSL		(regScaledOffset, regTexOffset, regConstantOne);
				ADD		(regTexAddr, regScaledOffset, regTextureData);
				LDH		(regTexColor565, regTexAddr);
				LDI		(regTexColorA, 0xff);

				regTexColorR = ExtractBitFieldTo255(block, regTexColor565, 11, 15);
				regTexColorG = ExtractBitFieldTo255(block, regTexColor565,  5, 10);
				regTexColorB = ExtractBitFieldTo255(block, regTexColor565,  0,  4);

				if (m_State->GetColorFormat() == ColorFormatRGB565) {
					regTexColorWord = regTexColor565;
				}
			}
			break;

		case ColorFormatRGB8:						// 8-8-8
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

				LDI			(regTexColorA, 0xff);
				LDI			(regConstant1, 1);
				LDI			(regConstant2, 2);
				LDI			(regConstant3, 3);

				LSL		(regShiftedOffset,	regTexOffset, regConstant1);
				ADD		(regScaledOffset,	regTexOffset, regShiftedOffset);
				ADD		(regTexAddr0,		regTextureData, regScaledOffset);
				LDB		(regTexColorR,		regTexAddr0);
				ADD		(regTexAddr1,		regTexAddr0, regConstant1);
				LDB		(regTexColorG,		regTexAddr1);
				ADD		(regTexAddr2,		regTexAddr0, regConstant2);
				LDB		(regTexColorB,		regTexAddr2);
			}

			break;

		case ColorFormatRGBA4444:					// 4-4-4-4
			{
				DECL_REG	(regScaledOffset);
				DECL_REG	(regConstantOne);
				DECL_REG	(regTexAddr);
				DECL_REG	(regTexColor4444);

				LDI		(regConstantOne, 1);
				LSL		(regScaledOffset, regTexOffset, regConstantOne);
				ADD		(regTexAddr, regScaledOffset, regTextureData);
				LDH		(regTexColor4444, regTexAddr);

				regTexColorR = ExtractBitFieldTo255(block, regTexColor4444, 12, 15);
				regTexColorG = ExtractBitFieldTo255(block, regTexColor4444,  8, 11);
				regTexColorB = ExtractBitFieldTo255(block, regTexColor4444,  4,  7);
				regTexColorA = ExtractBitFieldTo255(block, regTexColor4444,  0,  3);

				if (m_State->GetColorFormat() == ColorFormatRGBA4444) {
					regTexColorWord = regTexColor4444;
				}
			}

			break;

		case ColorFormatRGBA8:						// 8-8-8-8
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

				LDI		(regConstant1, 1);
				LDI		(regConstant2, 2);
				LDI		(regConstant3, 3);
				LDI		(regConstant7, 7);

				LSL		(regScaledOffset, regTexOffset, regConstant2);
				ADD		(regTexAddr0, regTextureData, regScaledOffset);
				LDB		(regTexColorR, regTexAddr0);
				ADD		(regTexAddr1, regTexAddr0, regConstant1);
				LDB		(regTexColorG, regTexAddr1);
				ADD		(regTexAddr2, regTexAddr0, regConstant2);
				LDB		(regTexColorB, regTexAddr2);
				ADD		(regTexAddr3, regTexAddr0, regConstant3);
				LDB		(regTexColorA, regTexAddr3);

				// TODO: Once the color byte order is fixed, we can also pre-assign the right word here
			}

			break;

		case ColorFormatRGBA5551:					// 5-5-5-1
			//texColor = Color::From5551(reinterpret_cast<const U16 *>(data)[texOffset]);
			{
				DECL_REG	(regScaledOffset);
				DECL_REG	(regConstantOne);
				DECL_REG	(regTexAddr);
				DECL_REG	(regTexColor5551);

				LDI		(regConstantOne, 1);
				LSL		(regScaledOffset, regTexOffset, regConstantOne);
				ADD		(regTexAddr, regScaledOffset, regTextureData);
				LDH		(regTexColor5551, regTexAddr);

				regTexColorR = ExtractBitFieldTo255(block, regTexColor5551, 11, 15);
				regTexColorG = ExtractBitFieldTo255(block, regTexColor5551,  7, 10);
				regTexColorB = ExtractBitFieldTo255(block, regTexColor5551,  1,  5);
				regTexColorA = ExtractBitFieldTo255(block, regTexColor5551,  0,  0);

				if (m_State->GetColorFormat() == ColorFormatRGBA5551) {
					regTexColorWord = regTexColor5551;
				}
			}
			break;

		default:
			//texColor = Color(0xff, 0xff, 0xff, 0xff);
			{
			regTexColorR = regTexColorB = regTexColorG = regTexColorA = cg_virtual_reg_create(procedure, cg_reg_type_general);

			LDI		(regTexColorR, 0xff);
			}
			break;
	}
}

namespace {

	void WrapOrClamp(cg_proc_t * procedure, cg_block_t * block, 
					 cg_virtual_reg_t * regIn, 
					 cg_virtual_reg_t * regOut, 
					 cg_virtual_reg_t * regMask,  
					 RasterizerState::WrappingMode mode) {

		switch (mode) {
			case RasterizerState::WrappingModeClampToEdge:
				//tu0 = EGL_CLAMP(tu, 0, EGL_ONE);
				{
					DECL_REG	(regConstantZero);
					DECL_REG	(regTemp);

					LDI		(regConstantZero, EGL_FixedFromInt(0));
					MIN		(regTemp, regIn, regMask);
					MAX		(regOut, regTemp, regConstantZero);
				}
				break;

			default:
			case RasterizerState::WrappingModeRepeat:
				//tu0 = tu & 0xffff;
				{
					AND		(regOut, regIn, regMask);
				}
				break;
		}
	}

	cg_virtual_reg_t * BlendComponent(cg_proc_t * procedure, cg_block_t * block, 
									  cg_virtual_reg_t *regA, cg_virtual_reg_t *regB, 
									  cg_virtual_reg_t * regAlpha) {

		DECL_REG	(regDiff);
		DECL_REG	(regProduct);
		DECL_REG	(regConstant8);
		DECL_REG	(regShifted);
		DECL_REG	(regResult);

		SUB			(regDiff, regB, regA);
		MUL			(regProduct, regDiff, regAlpha);
		LDI			(regConstant8, 8);
		ASR			(regShifted, regProduct, regConstant8);
		ADD			(regResult, regA, regShifted);

		return regResult;
	}

	U32 EncodeColor(ColorFormat format, Color color) {
		switch (format) {
		case ColorFormatRGB565:		return color.ConvertTo565();
		case ColorFormatRGBA5551:	return color.ConvertTo5551();
		case ColorFormatRGBA4444:	return color.ConvertTo4444();
		case ColorFormatRGBA8:		return color.ConvertToRGBA();
		default:					return 0;
		}
	}
}


void RasterPart :: GenerateFetchTexColor(cg_proc_t * procedure, cg_block_t * currentBlock, 
											size_t unit,
											FragmentGenerationInfo & fragmentInfo,
										    cg_virtual_reg_t *& regTexColorR,
										    cg_virtual_reg_t *& regTexColorG,			
										    cg_virtual_reg_t *& regTexColorB,			
										    cg_virtual_reg_t *& regTexColorA,
											cg_virtual_reg_t *& regTexColorWord) {

	cg_block_t * block = currentBlock;

	cg_virtual_reg_t * regU = fragmentInfo.regU[unit];
	cg_virtual_reg_t * regV = fragmentInfo.regV[unit];

	//EGL_Fixed tu0;
	//EGL_Fixed tv0;

	if (m_State->GetMinFilterMode(unit) == RasterizerState::FilterModeNearest) {
		DECL_REG	(regU0);
		DECL_REG	(regV0);

		DECL_REG	(regMask);

		LDI		(regMask, 0xffff);

		WrapOrClamp(procedure, block, regU, regU0, regMask, m_State->m_Texture[unit].WrappingModeS);
		WrapOrClamp(procedure, block, regV, regV0, regMask, m_State->m_Texture[unit].WrappingModeT);

		// get the pixel color
		//Texture * texture = m_Texture->GetTexture(m_MipMapLevel);
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

		cg_virtual_reg_t * regTextureLogWidth =		LOAD_DATA(block, fragmentInfo.regTexture[unit], OFFSET_TEXTURE_LOG_WIDTH);
		cg_virtual_reg_t * regTextureLogHeight =	LOAD_DATA(block, fragmentInfo.regTexture[unit], OFFSET_TEXTURE_LOG_HEIGHT);

		LSL		(regScaledU, regU0, regTextureLogWidth);
		LSL		(regScaledV, regV0, regTextureLogHeight);
		LDI		(regConstant16, 16);
		ASR		(regTexX, regScaledU, regConstant16);
		ASR		(regTexY, regScaledV, regConstant16);
		LSL		(regScaledTexY, regTexY, regTextureLogWidth);

		cg_virtual_reg_t * regTextureData =			LOAD_DATA(block, fragmentInfo.regTexture[unit], OFFSET_TEXTURE_DATA);

		ADD		(regTexOffset, regScaledTexY, regTexX);

		FetchTexColor(procedure, block, m_State->m_Texture + unit, regTextureData, regTexOffset,
					  regTexColorR, regTexColorG, regTexColorB, regTexColorA, regTexColorWord);
	} else {
		assert(m_State->GetMinFilterMode(unit) == RasterizerState::FilterModeLinear);

		cg_virtual_reg_t * regTextureLogWidth =		LOAD_DATA(block, fragmentInfo.regTexture[unit], OFFSET_TEXTURE_LOG_WIDTH);

		DECL_REG	(regHalf);
		DECL_REG	(regHalfU);
		DECL_REG	(regHalfV);

		LDI			(regHalf, 0x8000);

		cg_virtual_reg_t * regTextureLogHeight =	LOAD_DATA(block, fragmentInfo.regTexture[unit], OFFSET_TEXTURE_LOG_HEIGHT);

		ASR			(regHalfU, regHalf, regTextureLogWidth);
		ASR			(regHalfV, regHalf, regTextureLogHeight);

		DECL_REG	(regRoundedU);
		DECL_REG	(regRoundedV);

		SUB			(regRoundedU, regU, regHalfU);
		SUB			(regRoundedV, regV, regHalfV);

		DECL_REG	(regScaledU);
		DECL_REG	(regScaledV);
		DECL_REG	(regFracU);
		DECL_REG	(regFracV);
		DECL_REG	(regMask);

		LDI			(regMask, 0xffff);
		LSL			(regScaledU, regRoundedU, regTextureLogWidth);
		LSL			(regScaledV, regRoundedV, regTextureLogHeight);
		AND			(regFracU, regScaledU, regMask);
		AND			(regFracV, regScaledV, regMask);

		DECL_REG	(regTexX);
		DECL_REG	(regTexY);
		DECL_REG	(regConstant16);

		LDI			(regConstant16, 16);
		ASR			(regTexX, regScaledU, regConstant16);
		ASR			(regTexY, regScaledV, regConstant16);

		DECL_REG	(regConstant1);
		DECL_REG	(regIntMaskU0);
		DECL_REG	(regIntMaskU);
		DECL_REG	(regIntMaskV0);
		DECL_REG	(regIntMaskV);

		LDI			(regConstant1, 1);
		LSL			(regIntMaskU0, regConstant1, regTextureLogWidth);
		LSL			(regIntMaskV0, regConstant1, regTextureLogHeight);
		SUB			(regIntMaskU, regIntMaskU0, regConstant1);
		SUB			(regIntMaskV, regIntMaskV0, regConstant1);

		DECL_REG	(regI0);
		DECL_REG	(regI1);
		DECL_REG	(regJ0);
		DECL_REG	(regJ1);

		DECL_REG	(regTexX1);
		DECL_REG	(regTexY1);

		ADD			(regTexX1, regTexX, regConstant1);
		ADD			(regTexY1, regTexY, regConstant1);

		WrapOrClamp(procedure, block, regTexX, regI0, regIntMaskU, m_State->m_Texture[unit].WrappingModeS);
		WrapOrClamp(procedure, block, regTexX1, regI1, regIntMaskU, m_State->m_Texture[unit].WrappingModeS);
		WrapOrClamp(procedure, block, regTexY, regJ0, regIntMaskV, m_State->m_Texture[unit].WrappingModeT);
		WrapOrClamp(procedure, block, regTexY1, regJ1, regIntMaskV, m_State->m_Texture[unit].WrappingModeT);

		DECL_REG	(regScaledJ0);
		DECL_REG	(regScaledJ1);

		LSL		(regScaledJ0, regJ0, regTextureLogWidth);
		LSL		(regScaledJ1, regJ1, regTextureLogWidth);

		DECL_REG	(regTexOffset00);
		DECL_REG	(regTexOffset01);
		DECL_REG	(regTexOffset10);
		DECL_REG	(regTexOffset11);

		ADD		(regTexOffset00, regI0, regScaledJ0);
		ADD		(regTexOffset01, regI1, regScaledJ0);
		ADD		(regTexOffset10, regI0, regScaledJ1);
		ADD		(regTexOffset11, regI1, regScaledJ1);

		cg_virtual_reg_t * regColorR00,	* regColorR01, *regColorR10, * regColorR11;
		cg_virtual_reg_t * regColorG00, * regColorG01, *regColorG10, * regColorG11;
		cg_virtual_reg_t * regColorB00, * regColorB01, *regColorB10, * regColorB11;
		cg_virtual_reg_t * regColorA00, * regColorA01, *regColorA10, * regColorA11;
		cg_virtual_reg_t * regColorWord00, * regColorWord01, *regColorWord10, * regColorWord11;

		cg_virtual_reg_t * regTextureData =			LOAD_DATA(block, fragmentInfo.regTexture[unit], OFFSET_TEXTURE_DATA);

		FetchTexColor(procedure, block, m_State->m_Texture + unit, regTextureData, regTexOffset00,
					  regColorR00, regColorG00, regColorB00, regColorA00, regColorWord00);

		FetchTexColor(procedure, block, m_State->m_Texture + unit, regTextureData, regTexOffset01,
					  regColorR01, regColorG01, regColorB01, regColorA01, regColorWord01);

		FetchTexColor(procedure, block, m_State->m_Texture + unit, regTextureData, regTexOffset10,
					  regColorR10, regColorG10, regColorB10, regColorA10, regColorWord10);

		FetchTexColor(procedure, block, m_State->m_Texture + unit, regTextureData, regTexOffset11,
					  regColorR11, regColorG11, regColorB11, regColorA11, regColorWord11);

		cg_virtual_reg_t * regColorR0, * regColorR1;
		cg_virtual_reg_t * regColorG0, * regColorG1;
		cg_virtual_reg_t * regColorB0, * regColorB1;
		cg_virtual_reg_t * regColorA0, * regColorA1;

		// blend into 0 and 1 colors
		DECL_REG	(regConstant8);
		DECL_REG	(regAdjustedFracU);
		DECL_REG	(regAdjustedFracV);

		LDI			(regConstant8, 8);
		ASR			(regAdjustedFracU, regFracU, regConstant8);
		ASR			(regAdjustedFracV, regFracV, regConstant8);

		regColorR0 = BlendComponent(procedure, block, regColorR00, regColorR01, regAdjustedFracU);
		regColorR1 = BlendComponent(procedure, block, regColorR10, regColorR11, regAdjustedFracU);
		regTexColorR = BlendComponent(procedure, block, regColorR0, regColorR1, regAdjustedFracV);

		regColorG0 = BlendComponent(procedure, block, regColorG00, regColorG01, regAdjustedFracU);
		regColorG1 = BlendComponent(procedure, block, regColorG10, regColorG11, regAdjustedFracU);
		regTexColorG = BlendComponent(procedure, block, regColorG0, regColorG1, regAdjustedFracV);

		regColorB0 = BlendComponent(procedure, block, regColorB00, regColorB01, regAdjustedFracU);
		regColorB1 = BlendComponent(procedure, block, regColorB10, regColorB11, regAdjustedFracU);
		regTexColorB = BlendComponent(procedure, block, regColorB0, regColorB1, regAdjustedFracV);

		regColorA0 = BlendComponent(procedure, block, regColorA00, regColorA01, regAdjustedFracU);
		regColorA1 = BlendComponent(procedure, block, regColorA10, regColorA11, regAdjustedFracU);
		regTexColorA = BlendComponent(procedure, block, regColorA0, regColorA1, regAdjustedFracV);

		regTexColorWord = 0;			// no composite word
	}
}

void RasterPart :: GenerateFragment(cg_proc_t * procedure, cg_block_t * currentBlock,
	cg_block_ref_t * continuation, FragmentGenerationInfo & fragmentInfo,
	int weight, bool forceScissor) {
	currentBlock = 
		GenerateFragmentDepthStencil(procedure, currentBlock, continuation,
			fragmentInfo, weight, 0, 0, forceScissor);

	GenerateFragmentColorAlpha(procedure, currentBlock, continuation,
		fragmentInfo, weight, 0);
}

cg_virtual_reg_t * 
RasterPart :: UpdateStencilValue(cg_block_t * block, RasterizerState::StencilOp op,
								 cg_virtual_reg_t * regStencilValue, cg_virtual_reg_t * regStencilRef) {

	cg_proc_t * procedure = block->proc;
	cg_virtual_reg_t * regNewStencilValue = 0;
	int stencilBits;

	switch (m_State->GetDepthStencilFormat()) {
	default:
		assert(false);
		// fall through

	case DepthStencilFormatDepth16:
		return regStencilValue;

	case DepthStencilFormatDepth16Stencil16:
		stencilBits = 16;
		break;
	}

	switch (op) {
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

				DECL_CONST_REG	(regShift, stencilBits);
				DECL_CONST_REG	(regConstant1, 1);

				DECL_REG	(regShifted);
				DECL_REG	(regIncremented);

				ADD		(regIncremented, regStencilValue, regConstant1);
				LSR		(regShifted, regIncremented, regShift);
				SUB		(regNewStencilValue, regIncremented, regShifted);
			}
			
			break;

		case RasterizerState::StencilOpDecr: 
			//if (stencilValue != 0) {
			//	stencilValue--; 
			//}
			{
				regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);

				DECL_CONST_REG	(regConstant1, 1);
				DECL_CONST_REG	(regConstant31, 31);

				DECL_REG	(regDecremented);
				DECL_REG	(regShifted);

				SUB		(regDecremented, regStencilValue, regConstant1);
				LSR		(regShifted, regDecremented, regConstant31);
				ADD		(regNewStencilValue, regDecremented, regShifted);
			}
			
			break;

		case RasterizerState::StencilOpInvert: 
			//stencilValue = ~stencilValue; 
			regNewStencilValue = cg_virtual_reg_create(procedure, cg_reg_type_general);

			NOT		(regNewStencilValue, regStencilValue);
			break;
	}

	return regNewStencilValue;
}

void RasterPart :: WriteDepthStencil(cg_block_t * block, cg_virtual_reg_t * regDepthStencilAddr,
									 cg_virtual_reg_t * regOldDepth, cg_virtual_reg_t * regNewDepth,
									 cg_virtual_reg_t * regOldStencil, cg_virtual_reg_t * regNewStencil) {

	cg_proc_t * procedure = block->proc;
	int stencilBits;
	U32 stencilBitMask, stencilWriteMask = m_State->GetStencilMask();
	U32 depthWriteMask = m_State->GetDepthMask() ? 0xffffffff : 0;
	int stencilShift, depthShift;
	cg_virtual_reg_t * regNewDepthStencilValue;

	switch (m_State->GetDepthStencilFormat()) {
	default:
		assert(false);
		return;

	case DepthStencilFormatDepth16:
		stencilBits = 0;
		stencilBitMask = 0;
		stencilWriteMask = 0;
		depthWriteMask &= 0xffff;
		stencilShift = 0;
		depthShift = 0;
		break;

	case DepthStencilFormatDepth16Stencil16:
		stencilBits = 16;
		stencilBitMask = 0xffff;
		stencilWriteMask &= stencilBitMask;
		depthWriteMask &= 0xffff;
		stencilShift = 16;
		depthShift = 0;
		break;
	}

	if (!regNewDepth) {
		regNewDepth = regOldDepth;
	}

	if (!regNewStencil) {
		regNewStencil = regOldStencil;
	}

	if ((!m_State->GetDepthMask() || regOldDepth == regNewDepth) &&
		(!stencilWriteMask || regOldStencil == regNewStencil)) {
			// no depth value to write
			// no stencil value to write
			// => Don't write anything
			return;
	}

	// Prepare the stencil write value
	cg_virtual_reg_t * regStencilWriteValue;

	if (stencilWriteMask != stencilBitMask) {
		DECL_CONST_REG		(regMaskOld, stencilBitMask & ~stencilWriteMask);
		DECL_REG			(regMaskedOld);

		AND					(regMaskedOld, regOldStencil, regMaskOld);

		DECL_CONST_REG		(regMaskNew, stencilWriteMask);
		DECL_REG			(regMaskedNew);

		AND					(regMaskedNew, regNewStencil, regMaskNew);

		DECL_REG			(regCombined);

		OR					(regCombined, regMaskedNew, regMaskedOld);

		regStencilWriteValue = regCombined;
	} else {
		regStencilWriteValue = regNewStencil;
	}

	if (stencilShift) {
		DECL_CONST_REG		(regShift, stencilShift);
		DECL_REG			(regShifted);

		LSL					(regShifted, regStencilWriteValue, regShift);

		regStencilWriteValue = regShifted;
	}

	// Prepare the depth write value
	
	cg_virtual_reg_t * regDepthWriteValue = 0;

	if (!m_State->GetDepthMask() || regOldDepth == regNewDepth) {
		regDepthWriteValue = regOldDepth;
	} else {
		regDepthWriteValue = regNewDepth;
	}

	if (depthShift) {
		DECL_REG		(regShifted);
		DECL_CONST_REG	(regDepthShift, depthShift);

		LSL				(regShifted, regDepthWriteValue, regDepthShift);

		regDepthWriteValue = regShifted;
	}

	if (stencilBitMask) {
		DECL_REG		(regCombined);

		OR				(regCombined, regDepthWriteValue, regStencilWriteValue);

		regNewDepthStencilValue = regCombined;
	} else {
		regNewDepthStencilValue = regDepthWriteValue;
	}

	switch (m_State->GetDepthStencilFormat()) {
	default:
		assert(false);
		break;

	case DepthStencilFormatDepth16:
		STH			(regNewDepthStencilValue, regDepthStencilAddr);
		break;

	case DepthStencilFormatDepth16Stencil16:
		STW			(regNewDepthStencilValue, regDepthStencilAddr);
		break;
	}
}

cg_virtual_reg_t * 
RasterPart :: CalcBlockedOffset(cg_block_t * block, FragmentGenerationInfo & fragmentInfo,
								cg_virtual_reg_t * regX, cg_virtual_reg_t * regY) {

	cg_proc_t * procedure = block->proc;

	DECL_CONST_REG		(regBlockIndexMask, EGL_RASTER_BLOCK_SIZE - 1);

	DECL_REG			(regBlockX);
	DECL_REG			(regBlockY);
	DECL_REG			(regBlockIndexX);
	DECL_REG			(regBlockIndexY);
	DECL_REG			(regSkewOffset);

	AND					(regBlockX, regX, regBlockIndexMask);
	SUB					(regBlockIndexX, regX, regBlockX);
	AND					(regBlockY, regY, regBlockIndexMask);
	SUB					(regBlockIndexY, regY, regBlockY);
	ADD					(regSkewOffset, regBlockY, regBlockIndexX);

	DECL_CONST_REG		(regShift, EGL_LOG_RASTER_BLOCK_SIZE);
	DECL_REG			(regScaledBlockIndexY);
	DECL_REG			(regSkewOffsetShifted);
	DECL_REG			(regOffset0);
	DECL_REG			(regOffset);

	cg_virtual_reg_t * regWidth = LOAD_DATA_HALF(block, fragmentInfo.regInfo, OFFSET_SURFACE_WIDTH);

	MUL					(regScaledBlockIndexY, regBlockIndexY, regWidth);
	ADD					(regOffset0, regScaledBlockIndexY, regBlockX);
	LSL					(regSkewOffsetShifted, regSkewOffset, regShift);
	ADD					(regOffset, regOffset0, regSkewOffsetShifted);

	return regOffset;
}

cg_block_t * RasterPart :: GenerateFragmentDepthStencil(cg_proc_t * procedure, cg_block_t * currentBlock,
	cg_block_ref_t * continuation, FragmentGenerationInfo & fragmentInfo,
	int weight, cg_virtual_reg_t * regDepthBuffer, bool forceScissor, bool noScissor) {

	cg_block_t * block = currentBlock;


	// Signature of generated function is:
	// (I32 x, I32 y, EGL_Fixed depth, EGL_Fixed tu, EGL_Fixed tv, EGL_Fixed fogDensity, const Color& baseColor);
	
	// fragment level clipping (for now)

	//if (m_Surface->GetWidth() <= x || x < 0 ||
	//	m_Surface->GetHeight() <= y || y < 0) {
	//	return;
	//}

	if (forceScissor || (!noScissor && m_State->m_ScissorTest.Enabled)) {
		DECL_REG	(regConstXStart);
		DECL_REG	(regConstXEnd);
		DECL_FLAGS	(regXStartTest);
		DECL_FLAGS	(regXEndTest);

		LDI			(regConstXStart, m_State->m_ScissorTest.X);
		LDI			(regConstXEnd, m_State->m_ScissorTest.X + m_State->m_ScissorTest.Width);

		CMP			(regXStartTest, fragmentInfo.regX, regConstXStart);
		BLT			(regXStartTest, continuation);
		CMP			(regXEndTest, fragmentInfo.regX, regConstXEnd);
		BGE			(regXEndTest, continuation);

		if (fragmentInfo.regY) {
			DECL_REG	(regConstYStart);
			DECL_REG	(regConstYEnd);
			DECL_FLAGS	(regYStartTest);
			DECL_FLAGS	(regYEndTest);

			LDI			(regConstYStart, m_State->m_ScissorTest.Y);
			LDI			(regConstYEnd, m_State->m_ScissorTest.Y + m_State->m_ScissorTest.Height);

			CMP			(regYStartTest, fragmentInfo.regY, regConstYStart);
			BLT			(regYStartTest, continuation);
			CMP			(regYEndTest, fragmentInfo.regY, regConstYEnd);
			BGE			(regYEndTest, continuation);
		}
	}

	//bool depthTest;
	//U32 offset = x + y * m_Surface->GetWidth();
	//I32 zBufferValue = m_Surface->GetDepthBuffer()[offset];
	cg_virtual_reg_t * regOffset;
	
	if (fragmentInfo.regY) {
		regOffset = CalcBlockedOffset(block, fragmentInfo, fragmentInfo.regX, fragmentInfo.regY);
	} else {
		regOffset = fragmentInfo.regX;
	}

	if (!regDepthBuffer) 
		regDepthBuffer = LOAD_DATA(block, fragmentInfo.regInfo, OFFSET_SURFACE_DEPTH_STENCIL_BUFFER);

	DECL_FLAGS	(regDepthTest);
	DECL_REG	(regConstant1);
	DECL_REG	(regConstant2);
	DECL_REG	(regOffset4);
	DECL_REG	(regOffset2);
	DECL_REG	(regDepthStencilBufferAddr);
	DECL_REG	(regDepthStencilValue);
	DECL_REG	(regZBufferValue);
	DECL_REG	(regStencilValue);

	LDI		(regConstant1, 1);
	LDI		(regConstant2, 2);
	LSL		(regOffset2, regOffset, regConstant1);
	LSL		(regOffset4, regOffset, regConstant2);

	switch (m_State->GetDepthStencilFormat()) {
	case DepthStencilFormatDepth16:
		ADD		(regDepthStencilBufferAddr, regDepthBuffer, regOffset2);
		LDH		(regDepthStencilValue, regDepthStencilBufferAddr);
		XOR		(regStencilValue, regDepthStencilValue, regDepthStencilValue);	// empty stencil value

		regZBufferValue = regDepthStencilValue;

		break;

	case DepthStencilFormatDepth16Stencil16:
		{
			DECL_CONST_REG	(regMaskDepth, 0xffff);
			DECL_CONST_REG	(regStencilShift, 16);

			ADD		(regDepthStencilBufferAddr, regDepthBuffer, regOffset4);
			LDW		(regDepthStencilValue, regDepthStencilBufferAddr);
			AND		(regZBufferValue, regDepthStencilValue, regMaskDepth);
			LSR		(regStencilValue, regDepthStencilValue, regStencilShift);
		}

		break;

	default:
		assert(false);
	}

	/*
	 * Enable this piece if we want to clamp the depth value to 0 .. 0xffff
	{
		DECL_CONST_REG	(regConstant0, 0);
		DECL_CONST_REG	(regConstant1, 0xffff);
		DECL_REG		(regNewDepth);
		DECL_REG		(regTempDepth);

		MIN				(regTempDepth, fragmentInfo.regDepth, regConstant1);
		MAX				(regNewDepth, regTempDepth, regConstant0);

		fragmentInfo.regDepth = regNewDepth;
	}*/


	cg_opcode_t branchOnDepthTestPassed = cg_op_bra;
	cg_opcode_t branchOnDepthTestFailed = cg_op_nop;

	if (m_State->m_DepthTest.Enabled) {
		switch (m_State->m_DepthTest.Func) {
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
	}

	if (!m_State->m_Stencil.Enabled && m_State->m_DepthTest.Enabled) {
		CMP		(regDepthTest, fragmentInfo.regDepth, regZBufferValue);

		//if (!depthTest)
		//	return;

		if (branchOnDepthTestFailed == cg_op_nop) {
			// nothing
		} else if (branchOnDepthTestFailed == cg_op_bra) {
			BRA		(continuation);
		} else {
			cg_create_inst_branch_cond(block, branchOnDepthTestFailed, regDepthTest, continuation CG_INST_DEBUG_ARGS);
		}

		// Masking and write to framebuffer
		if (m_State->m_Mask.Depth) {
			//m_Surface->GetDepthBuffer()[offset] = depth;
			switch (m_State->GetDepthStencilFormat()) {
			case DepthStencilFormatDepth16:
				STH		(fragmentInfo.regDepth, regDepthStencilBufferAddr);
				break;

			case DepthStencilFormatDepth16Stencil16:
				{
					DECL_REG		(regStoreValue);
					DECL_REG		(regMaskedDepthStencil);
					DECL_CONST_REG	(regMask, 0xffff0000);

					AND				(regMaskedDepthStencil, regDepthStencilValue, regMask);
					OR				(regStoreValue, regMaskedDepthStencil, fragmentInfo.regDepth);
					STW				(regStoreValue, regDepthStencilBufferAddr);
				}

				break;

			default:
				assert(false);
			}
		}

		return block;
	} else if (m_State->m_Stencil.Enabled) {

		//bool stencilTest;
		//U32 stencilRef = m_State->m_Stencil.Reference & m_State->ComparisonMask;
		//U32 stencilValue = m_Surface->GetStencilBuffer()[offset];
		//U32 stencil = stencilValue & m_State->m_Stencil.ComparisonMask;
		DECL_REG	(regStencilRef);
		DECL_REG	(regStencilMask);
		DECL_REG	(regStencil);
		DECL_FLAGS	(regStencilTest);

		LDI		(regStencilRef, m_State->m_Stencil.Reference & m_State->m_Stencil.ComparisonMask);
		LDI		(regStencilMask, m_State->m_Stencil.ComparisonMask);
		AND		(regStencil, regStencilValue, regStencilMask);
		CMP		(regStencilTest, regStencil, regStencilRef);

		cg_opcode_t passedTest;

		switch (m_State->GetDepthStencilFormat()) {
		case DepthStencilFormatDepth16:
			// no stencil buffer: behave as if always passed
			passedTest = cg_op_bra;
			break;

		case DepthStencilFormatDepth16Stencil16:
			switch (m_State->m_Stencil.Func) {
				default:
				case RasterizerState::CompFuncNever:	
					//stencilTest = false;				
					passedTest = cg_op_nop;
					break;

				case RasterizerState::CompFuncLess:		
					//stencilTest = stencilRef < stencil;	
					passedTest = cg_op_bgt;
					break;

				case RasterizerState::CompFuncEqual:	
					//stencilTest = stencilRef == stencil;
					passedTest = cg_op_beq;
					break;

				case RasterizerState::CompFuncLEqual:	
					//stencilTest = stencilRef <= stencil;
					passedTest = cg_op_bge;
					break;

				case RasterizerState::CompFuncGreater:	
					//stencilTest = stencilRef > stencil;	
					passedTest = cg_op_blt;
					break;

				case RasterizerState::CompFuncNotEqual:	
					//stencilTest = stencilRef != stencil;
					passedTest = cg_op_bne;
					break;

				case RasterizerState::CompFuncGEqual:	
					//stencilTest = stencilRef >= stencil;
					passedTest = cg_op_ble;
					break;

				case RasterizerState::CompFuncAlways:	
					//stencilTest = true;					
					passedTest = cg_op_bra;
					break;
			}
		}

		// branch on stencil test
		cg_block_ref_t * labelStencilPassed = cg_block_ref_create(procedure);
		cg_block_ref_t * labelStencilBypassed = cg_block_ref_create(procedure);

		if (passedTest != cg_op_nop) {
			cg_create_inst_branch_cond(block, passedTest,	regStencilTest, labelStencilPassed CG_INST_DEBUG_ARGS);
		}

		//if (!stencilTest) {
		{
			cg_virtual_reg_t * regNewStencilValue =
				UpdateStencilValue(block, m_State->m_Stencil.Fail, regStencilValue, regStencilRef);

			WriteDepthStencil(block, regDepthStencilBufferAddr,
							  regZBufferValue, 0,
							  regStencilValue, regNewStencilValue);

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
			cg_virtual_reg_t * regNewStencilValue =
				UpdateStencilValue(block, m_State->m_Stencil.ZFail, regStencilValue, regStencilRef);


			WriteDepthStencil(block, regDepthStencilBufferAddr,
							  regZBufferValue, 0,
							  regStencilValue, regNewStencilValue);
		}

		if (m_State->m_DepthTest.Enabled) {
			// return;
			BRA		(continuation);
		} else {
			BRA		(labelStencilBypassed);
		}
		//} else {
		// stencil nad z-test passed
		block = cg_block_create(procedure, weight);
		labelStencilZTestPassed->block = block;

		{
			cg_virtual_reg_t * regNewStencilValue =
				UpdateStencilValue(block, m_State->m_Stencil.ZPass, regStencilValue, regStencilRef);


			WriteDepthStencil(block, regDepthStencilBufferAddr,
							  regZBufferValue, fragmentInfo.regDepth,
							  regStencilValue, regNewStencilValue);
		}

		// stencil test bypassed
		block = cg_block_create(procedure, weight);
		labelStencilBypassed->block = block;
	}

	return block;
}

void RasterPart :: GenerateFragmentColorAlpha(cg_proc_t * procedure, cg_block_t * currentBlock,
	cg_block_ref_t * continuation, FragmentGenerationInfo & fragmentInfo,
	int weight, cg_virtual_reg_t * regColorBuffer) {

	cg_block_t * block = currentBlock;

	//bool depthTest;
	//U32 offset = x + y * m_Surface->GetWidth();
	//I32 zBufferValue = m_Surface->GetDepthBuffer()[offset];
	cg_virtual_reg_t * regOffset;
	
	if (fragmentInfo.regY) {
		regOffset = cg_virtual_reg_create(procedure, cg_reg_type_general);

		cg_virtual_reg_t * regPitch = LOAD_DATA(block, fragmentInfo.regInfo, OFFSET_SURFACE_PITCH);

		DECL_REG	(regScaledY);

		MUL		(regScaledY, fragmentInfo.regY, regPitch);
		ADD		(regOffset, regScaledY, fragmentInfo.regX);
	} else {
		regOffset = fragmentInfo.regX;
	}

	//Color color = baseColor;
	cg_virtual_reg_t * regColorR = ClampTo255(block, fragmentInfo.regR);
	cg_virtual_reg_t * regColorG = ClampTo255(block, fragmentInfo.regG);
	cg_virtual_reg_t * regColorB = ClampTo255(block, fragmentInfo.regB);
	cg_virtual_reg_t * regColorA = ClampTo255(block, fragmentInfo.regA);
	cg_virtual_reg_t * regColorWord = 0;		// if we ever decide to handle constant colors, this variable will be used

	cg_virtual_reg_t * regBaseColorR = regColorR;
	cg_virtual_reg_t * regBaseColorG = regColorG;
	cg_virtual_reg_t * regBaseColorB = regColorB;
	cg_virtual_reg_t * regBaseColorA = regColorA;
	cg_virtual_reg_t * regBaseColorWord = regColorWord;

	for (size_t unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
		if (m_State->m_Texture[unit].Enabled) {

			//Color texColor; 
			cg_virtual_reg_t * regTexColorR = 0;			
			cg_virtual_reg_t * regTexColorG = 0;			
			cg_virtual_reg_t * regTexColorB = 0;			
			cg_virtual_reg_t * regTexColorA = 0;
			cg_virtual_reg_t * regTexColorWord = 0;

			GenerateFetchTexColor(procedure, block, unit, fragmentInfo, 
								  regTexColorR, regTexColorG, regTexColorB, regTexColorA, regTexColorWord);



			regColorWord = 0;

			if (m_State->m_Texture[unit].Mode == RasterizerState::TextureModeCombine) {

				//Color arg[3];
				cg_virtual_reg_t * regArgR[3];			
				cg_virtual_reg_t * regArgG[3];			
				cg_virtual_reg_t * regArgB[3];			
				cg_virtual_reg_t * regArgA[3];

				for (size_t idx = 0; idx < 3; ++idx) {
					//Color rgbIn;
					cg_virtual_reg_t * regRgbInR = 0;			
					cg_virtual_reg_t * regRgbInG = 0;			
					cg_virtual_reg_t * regRgbInB = 0;			
					cg_virtual_reg_t * regRgbInA = 0;			

					//U8 alphaIn;
					cg_virtual_reg_t * regAlphaIn;

					switch (m_State->m_Texture[unit].CombineSrcRGB[idx]) {
					case RasterizerState::TextureCombineSrcTexture:
						//rgbIn = texColor;
						{
							regRgbInR = regTexColorR;
							regRgbInG = regTexColorG;
							regRgbInB = regTexColorB;
							regRgbInA = regTexColorA;
						}
						break;

					case RasterizerState::TextureCombineSrcConstant:
						//rgbIn = m_State->m_Texture[unit].EnvColor;
						{
							regRgbInR = cg_virtual_reg_create(procedure, cg_reg_type_general);
							regRgbInG = cg_virtual_reg_create(procedure, cg_reg_type_general);
							regRgbInB = cg_virtual_reg_create(procedure, cg_reg_type_general);
							regRgbInA = cg_virtual_reg_create(procedure, cg_reg_type_general);

							LDI		(regRgbInR, m_State->m_Texture[unit].EnvColor.r);
							LDI		(regRgbInG, m_State->m_Texture[unit].EnvColor.g);
							LDI		(regRgbInB, m_State->m_Texture[unit].EnvColor.b);
							LDI		(regRgbInA, m_State->m_Texture[unit].EnvColor.a);
						}
						break;

					case RasterizerState::TextureCombineSrcPrimaryColor:
						//rgbIn = baseColor;
						{
							regRgbInR = regBaseColorR;
							regRgbInG = regBaseColorG;
							regRgbInB = regBaseColorB;
							regRgbInA = regBaseColorA;
						}

						break;

					case RasterizerState::TextureCombineSrcPrevious:
						//rgbIn = color;
						{
							regRgbInR = regColorR;
							regRgbInG = regColorG;
							regRgbInB = regColorB;
							regRgbInA = regColorA;
						}
						break;
					}

					switch (m_State->m_Texture[unit].CombineSrcAlpha[idx]) {
					case RasterizerState::TextureCombineSrcTexture:
						//alphaIn = texColor.a;
						regAlphaIn = regTexColorA;
						break;

					case RasterizerState::TextureCombineSrcConstant:
						//alphaIn = m_State->m_Texture[unit].EnvColor.a;
						{
							regAlphaIn = cg_virtual_reg_create(procedure, cg_reg_type_general);
							LDI		(regAlphaIn, m_State->m_Texture[unit].EnvColor.a);
						}

						break;

					case RasterizerState::TextureCombineSrcPrimaryColor:
						//alphaIn = baseColor.a;
						regAlphaIn = regBaseColorA;
						break;

					case RasterizerState::TextureCombineSrcPrevious:
						//alphaIn = color.a;
						regAlphaIn = regColorA;
						break;
					}

					//U8 alphaOut;
					cg_virtual_reg_t * regAlphaOut;

					if (m_State->m_Texture[unit].CombineOpAlpha[idx] == RasterizerState::TextureCombineOpSrcAlpha) {
						//alphaOut = alphaIn;
						regAlphaOut = regAlphaIn;
					} else {
						//alphaOut = 0xFF - alphaIn;
						DECL_CONST_REG	(constantMaxColor, 0xff);

						regAlphaOut = Sub(block, constantMaxColor, regAlphaIn);
					}

					switch (m_State->m_Texture[unit].CombineOpRGB[idx]) {
					case RasterizerState::TextureCombineOpSrcColor:
						//arg[idx] = Color(rgbIn.r, rgbIn.g, rgbIn.b, alphaOut);
						{
							regArgR[idx] = regRgbInR;
							regArgG[idx] = regRgbInG;
							regArgB[idx] = regRgbInB;
							regArgA[idx] = regAlphaOut;
						}
						break;

					case RasterizerState::TextureCombineOpOneMinusSrcColor:
						//arg[idx] = Color(0xFF - rgbIn.r, 0xFF - rgbIn.g, 0xFF - rgbIn.b, alphaOut);
						{
							DECL_CONST_REG	(constantMaxColor, 0xff);

							regArgR[idx] = Sub(block, constantMaxColor, regRgbInR);
							regArgG[idx] = Sub(block, constantMaxColor, regRgbInG);
							regArgB[idx] = Sub(block, constantMaxColor, regRgbInB);
							regArgA[idx] = regAlphaOut;
						}
						break;

					case RasterizerState::TextureCombineOpSrcAlpha:
						//arg[idx] = Color(rgbIn.a, rgbIn.a, rgbIn.a, alphaOut);
						{
							regArgR[idx] = regArgG[idx] = regArgB[idx] = regRgbInA;
							regArgA[idx] = regAlphaOut;
						}

						break;

					case RasterizerState::TextureCombineOpOneMinusSrcAlpha:
						//arg[idx] = Color(0xFF - rgbIn.a, 0xFF - rgbIn.a, 0xFF - rgbIn.a, alphaOut);
						{
							DECL_CONST_REG	(constantMaxColor, 0xff);

							regArgR[idx] = regArgG[idx] = regArgB[idx] = Sub(block, constantMaxColor, regRgbInA);
							regArgA[idx] = regAlphaOut;
						}

						break;
					}
				}

				//U8 combineAlpha;
				cg_virtual_reg_t * regCombineAlpha;

				switch (m_State->m_Texture[unit].CombineFuncAlpha) {
				case RasterizerState::TextureModeCombineReplace:
					{
						//combineAlpha = MulU8(arg[0].a, 0xFF, scaleAlpha);
						regCombineAlpha = regArgA[0];
					}

					break;

				case RasterizerState::TextureModeCombineModulate:
					//combineAlpha = MulU8(arg[0].a, arg[1].a, scaleAlpha);
					regCombineAlpha = Mul255(block, regArgA[0], regArgA[1]);
					break;

				case RasterizerState::TextureModeCombineAdd:
					//combineAlpha = AddU8(arg[0].a, arg[1].a, scaleAlpha);
					regCombineAlpha = Add(block, regArgA[0], regArgA[1]);
					break;

				case RasterizerState::TextureModeCombineAddSigned:
					//combineAlpha = AddSignedU8(arg[0].a, arg[1].a, scaleAlpha);
					regCombineAlpha = AddSigned(block, regArgA[0], regArgA[1]);
					break;

				case RasterizerState::TextureModeCombineInterpolate:
					//combineAlpha = InterpolateU8(arg[0].a, arg[1].a, arg[2].a, scaleAlpha);
					//regCombineAlpha = Blend255(block, regArgA[0], regArgA[1], regArgA[2]);
					regCombineAlpha = Blend255(block, regArgA[1], regArgA[0], regArgA[2]);
					break;

				case RasterizerState::TextureModeCombineSubtract:
					//combineAlpha = SubU8(arg[0].a, arg[1].a, scaleAlpha);
					regCombineAlpha = Sub(block, regArgA[0], regArgA[1]);
					break;
				}

				switch (m_State->m_Texture[unit].CombineFuncRGB) {
				case RasterizerState::TextureModeCombineReplace:
					//color = Color(
					//		MulU8(arg[0].r, 0xFF, scaleRGB), 
					//		MulU8(arg[0].g, 0xFF, scaleRGB), 
					//		MulU8(arg[0].b, 0xFF, scaleRGB), 
					//		combineAlpha); 
					regColorR = regArgR[0];
					regColorG = regArgG[0];
					regColorB = regArgB[0];
					regColorA = regCombineAlpha;
					break;

				case RasterizerState::TextureModeCombineModulate:
					//color = 
					//	Color(
					//		MulU8(arg[0].r, arg[1].r, scaleRGB), 
					//		MulU8(arg[0].g, arg[1].g, scaleRGB), 
					//		MulU8(arg[0].b, arg[1].b, scaleRGB), 
					//		combineAlpha); 
					regColorR = Mul255(block, regArgR[0], regArgR[1]);
					regColorG = Mul255(block, regArgG[0], regArgG[1]);
					regColorB = Mul255(block, regArgB[0], regArgB[1]);
					regColorA = regCombineAlpha;
					break;

				case RasterizerState::TextureModeCombineAdd:
					//color = 
					//	Color(
					//		AddU8(arg[0].r, arg[1].r, scaleRGB), 
					//		AddU8(arg[0].g, arg[1].g, scaleRGB), 
					//		AddU8(arg[0].b, arg[1].b, scaleRGB), 
					//		combineAlpha); 
					regColorR = Add(block, regArgR[0], regArgR[1]);
					regColorG = Add(block, regArgG[0], regArgG[1]);
					regColorB = Add(block, regArgB[0], regArgB[1]);
					regColorA = regCombineAlpha;
					break;

				case RasterizerState::TextureModeCombineAddSigned:
					//color = 
					//	Color(
					//		AddSignedU8(arg[0].r, arg[1].r, scaleRGB), 
					//		AddSignedU8(arg[0].g, arg[1].g, scaleRGB), 
					//		AddSignedU8(arg[0].b, arg[1].b, scaleRGB), 
					//		combineAlpha); 
					regColorR = AddSigned(block, regArgR[0], regArgR[1]);
					regColorG = AddSigned(block, regArgG[0], regArgG[1]);
					regColorB = AddSigned(block, regArgB[0], regArgB[1]);
					regColorA = regCombineAlpha;
					break;

				case RasterizerState::TextureModeCombineInterpolate:
					//color =
					//	Color(
					//		InterpolateU8(arg[0].r, arg[1].r, arg[2].r, scaleRGB),
					//		InterpolateU8(arg[0].g, arg[1].g, arg[2].g, scaleRGB),
					//		InterpolateU8(arg[0].b, arg[1].b, arg[2].b, scaleRGB),
					//		combineAlpha); 
					//regColorR = Blend255(block, regArgR[0], regArgR[1], regArgR[2]);
					//regColorG = Blend255(block, regArgG[0], regArgG[1], regArgG[2]);
					//regColorB = Blend255(block, regArgB[0], regArgB[1], regArgB[2]);
					regColorR = Blend255(block, regArgR[1], regArgR[0], regArgR[2]);
					regColorG = Blend255(block, regArgG[1], regArgG[0], regArgG[2]);
					regColorB = Blend255(block, regArgB[1], regArgB[0], regArgB[2]);
					regColorA = regCombineAlpha;
					break;

				case RasterizerState::TextureModeCombineSubtract:
					//color = 
					//	Color(
					//		SubU8(arg[0].r, arg[1].r, scaleRGB), 
					//		SubU8(arg[0].g, arg[1].g, scaleRGB), 
					//		SubU8(arg[0].b, arg[1].b, scaleRGB), 
					//		combineAlpha); 
					regColorR = Sub(block, regArgR[0], regArgR[1]);
					regColorG = Sub(block, regArgG[0], regArgG[1]);
					regColorB = Sub(block, regArgB[0], regArgB[1]);
					regColorA = regCombineAlpha;
					break;

				case RasterizerState::TextureModeCombineDot3RGB:
				case RasterizerState::TextureModeCombineDot3RGBA:

					//{
					//	U8 dotRGB = Dot3U8(arg[0], arg[1], scaleRGB);
					//	color = Color(dotRGB, dotRGB, dotRGB, combineAlpha);
					//}
					//{
					//	U8 dotRGB = Dot3U8(arg[0], arg[1], scaleRGB);
					//	U8 dotAlpha = Dot3U8(arg[0], arg[1], scaleAlpha);
					//	color = Color(dotRGB, dotRGB, dotRGB, dotAlpha);
					//}

					regColorR = Dot3(block, regArgR, regArgG, regArgB); 
					regColorG = regColorB = regColorR;

					if (m_State->m_Texture[unit].CombineFuncRGB == RasterizerState::TextureModeCombineDot3RGBA)
						regColorA = regColorR;
					else 
						regColorA = regCombineAlpha;

					break;
				}

				EGL_Fixed scaleAlpha = m_State->m_Texture[unit].ScaleAlpha;

				if (scaleAlpha != EGL_ONE) {
					DECL_REG		(regResultA);
					DECL_CONST_REG	(regScaleAlpha, scaleAlpha);

					FMUL			(regResultA, regColorA, regScaleAlpha);

					regColorA = regResultA;
				}

				// Clamp to 0 .. 0xff
				{
					DECL_REG		(regClampLow);
					DECL_REG		(regClampHigh);
					DECL_CONST_REG	(constantMaxColor, 0xff);
					DECL_CONST_REG	(constant0, 0);

					MAX		(regClampLow, regColorA, constant0);
					MIN		(regClampHigh, regClampLow, constantMaxColor);

					regColorA = regClampHigh;
				}

				EGL_Fixed scaleRGB = m_State->m_Texture[unit].ScaleRGB;
	
				if (scaleRGB != EGL_ONE) {
					DECL_REG		(regResultR);
					DECL_REG		(regResultG);
					DECL_REG		(regResultB);
					DECL_CONST_REG	(regScaleRGB, scaleRGB);

					FMUL			(regResultR, regColorR, regScaleRGB);
					FMUL			(regResultG, regColorG, regScaleRGB);
					FMUL			(regResultB, regColorB, regScaleRGB);

					regColorR = regResultR;
					regColorG = regResultG;
					regColorB = regResultB;
				}

				// Clamp to 0 .. 0xff

				{
					DECL_REG		(regClampLowR);
					DECL_REG		(regClampHighR);
					DECL_REG		(regClampLowG);
					DECL_REG		(regClampHighG);
					DECL_REG		(regClampLowB);
					DECL_REG		(regClampHighB);
					DECL_CONST_REG	(constantMaxColor, 0xff);
					DECL_CONST_REG	(constant0, 0);

					MAX		(regClampLowR, regColorR, constant0);
					MIN		(regClampHighR, regClampLowR, constantMaxColor);
					MAX		(regClampLowG, regColorG, constant0);
					MIN		(regClampHighG, regClampLowG, constantMaxColor);
					MAX		(regClampLowB, regColorB, constant0);
					MIN		(regClampHighB, regClampLowB, constantMaxColor);

					regColorR = regClampHighR;
					regColorG = regClampHighG;
					regColorB = regClampHighB;
				}
			} else {
				switch (m_State->m_Texture[unit].InternalFormat) {
					default:
					case ColorFormatAlpha:
						switch (m_State->m_Texture[unit].Mode) {
							case RasterizerState::TextureModeReplace:
								{
								//color = Color(color.r, color.g, color.b, texColor.a);
								regColorA = regTexColorA;
								}

								break;

							case RasterizerState::TextureModeModulate:
							case RasterizerState::TextureModeBlend:
							case RasterizerState::TextureModeAdd:
								{
								//color = Color(color.r, color.g, color.b, MulU8(color.a, texColor.a));
								regColorA = Mul255(block, regColorA, regTexColorA);
								}

								break;
						}
						break;

					case ColorFormatLuminance:
					case ColorFormatRGB565:
					case ColorFormatRGB8:
						switch (m_State->m_Texture[unit].Mode) {
							case RasterizerState::TextureModeDecal:
							case RasterizerState::TextureModeReplace:
								{
								//color = Color(texColor.r, texColor.g, texColor.b, color.a);
								regColorR = regTexColorR;
								regColorG = regTexColorG;
								regColorB = regTexColorB;

								if (m_State->GetColorFormat() == ColorFormatRGB565) {
									regColorWord = regTexColorWord;
								}
								}

								break;

							case RasterizerState::TextureModeModulate:
								{
								//color = Color(MulU8(color.r, texColor.r), 
								//	MulU8(color.g, texColor.g), MulU8(color.b, texColor.b), color.a);
								regColorR = Mul255(block, regColorR, regTexColorR);
								regColorG = Mul255(block, regColorG, regTexColorG);
								regColorB = Mul255(block, regColorB, regTexColorB);
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

								regColorR   = Blend255(block, m_State->m_Texture[unit].EnvColor.r, regColorR, regTexColorR);
								regColorG   = Blend255(block, m_State->m_Texture[unit].EnvColor.g, regColorG, regTexColorG);
								regColorB   = Blend255(block, m_State->m_Texture[unit].EnvColor.b, regColorB, regTexColorB);
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

								regColorR	= AddSaturate255(block, regColorR, regTexColorR);
								regColorG	= AddSaturate255(block, regColorG, regTexColorG);
								regColorB	= AddSaturate255(block, regColorB, regTexColorB);
								}

								break;
						}
						break;

					case ColorFormatLuminanceAlpha:
					case ColorFormatRGBA5551:
					case ColorFormatRGBA4444:
					case ColorFormatRGBA8:
						switch (m_State->m_Texture[unit].Mode) {
							case RasterizerState::TextureModeReplace:
								{
								//color = texColor;
								regColorR = regTexColorR;
								regColorG = regTexColorG;
								regColorB = regTexColorB;
								regColorA = regTexColorA;
								regColorWord = regTexColorWord;
								}

								break;

							case RasterizerState::TextureModeModulate:
								{
								//color = color * texColor;
								regColorR = Mul255(block, regColorR, regTexColorR);
								regColorG = Mul255(block, regColorG, regTexColorG);
								regColorB = Mul255(block, regColorB, regTexColorB);
								regColorA = Mul255(block, regColorA, regTexColorA);
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

								regColorR   = Blend255(block, regColorR, regTexColorR, regTexColorA);
								regColorG   = Blend255(block, regColorG, regTexColorG, regTexColorA);
								regColorB   = Blend255(block, regColorB, regTexColorB, regTexColorA);
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
								regColorR   = Blend255(block, m_State->m_Texture[unit].EnvColor.r, regColorR, regTexColorR);
								regColorG   = Blend255(block, m_State->m_Texture[unit].EnvColor.g, regColorG, regTexColorG);
								regColorB   = Blend255(block, m_State->m_Texture[unit].EnvColor.b, regColorB, regTexColorB);
								regColorA	= Mul255(block, regColorA, regTexColorA);
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
								regColorR	= AddSaturate255(block, regColorR, regTexColorR);
								regColorG	= AddSaturate255(block, regColorG, regTexColorG);
								regColorB	= AddSaturate255(block, regColorB, regTexColorB);
								regColorA	= Mul255(block, regColorA, regTexColorA);
								}

								break;
						}
						break;
				}
			}
		}
	}

	// fog
	if (m_State->m_Fog.Enabled) {
		//color = Color::Blend(color, m_State->m_FogColor, fogDensity);
		DECL_REG	(regFogColorR);
		DECL_REG	(regFogColorG);
		DECL_REG	(regFogColorB);

		LDI		(regFogColorR, m_State->m_Fog.Color.r);
		LDI		(regFogColorG, m_State->m_Fog.Color.g);
		LDI		(regFogColorB, m_State->m_Fog.Color.b);

		cg_virtual_reg_t * regFog = ClampTo255(block, fragmentInfo.regFog);

		regColorR = Blend255(block, regFogColorR, regColorR, regFog);
		regColorG = Blend255(block, regFogColorG, regColorG, regFog);
		regColorB = Blend255(block, regFogColorB, regColorB, regFog);

		regColorWord = 0;
	}

	if (fragmentInfo.regCoverage) {
		// apply coverage value if anti-aliasing is enabled
		// color.ScaleA(coverage);
		
		DECL_REG	(regScaledA);
		DECL_REG	(regShiftedA);
		
		DECL_CONST_REG	(sixteen, 16);
		
		MUL		(regScaledA, regColorA, fragmentInfo.regCoverage);
		ASR		(regShiftedA, regScaledA, sixteen);
		
		regColorA = regShiftedA;
		regColorWord = 0;
	}
	
	if (m_State->m_Alpha.Enabled) {
		//bool alphaTest;
		//U8 alpha = color.A();
		//U8 alphaRef = EGL_IntFromFixed(m_State->m_AlphaReference * 255);
		DECL_REG	(regAlphaRef);
		DECL_FLAGS	(regAlphaTest);

		LDI		(regAlphaRef, EGL_IntFromFixed(m_State->m_Alpha.Reference * 255));
		CMP		(regAlphaTest, regColorA, regAlphaRef);

		cg_opcode_t failedTest;
		
		switch (m_State->m_Alpha.Func) {
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

	// surface color buffer
	if (!regColorBuffer) {
		regColorBuffer = LOAD_DATA(block, fragmentInfo.regInfo, OFFSET_SURFACE_COLOR_BUFFER);
	}

	//U16 dstValue = m_Surface->GetColorBuffer()[offset];
	//U8 dstAlpha = m_Surface->GetAlphaBuffer()[offset];
	DECL_REG	(regDstValue);

	cg_virtual_reg_t	* regDstR, *regDstG, *regDstB, *regDstAlpha;

	DECL_REG	(regColorAddr);

	switch (m_State->GetColorFormat()) {
	case ColorFormatRGB565:
		{
			DECL_REG	(regConstant1);
			DECL_REG	(regOffset2);

			LDI		(regConstant1, 1);
			LSL		(regOffset2, regOffset, regConstant1);
			ADD		(regColorAddr, regColorBuffer, regOffset2);
			LDH		(regDstValue, regColorAddr);

			regDstR = ExtractBitFieldTo255(block, regDstValue, 11, 15);
			regDstG = ExtractBitFieldTo255(block, regDstValue,  5, 10);
			regDstB = ExtractBitFieldTo255(block, regDstValue,  0,  4);
			regDstAlpha = cg_virtual_reg_create(procedure, cg_reg_type_general); 
			
			LDI		(regDstAlpha, 0xff);		
		}

		break;

	case ColorFormatRGBA5551:
		{
			DECL_REG	(regConstant1);
			DECL_REG	(regOffset2);

			LDI		(regConstant1, 1);
			LSL		(regOffset2, regOffset, regConstant1);
			ADD		(regColorAddr, regColorBuffer, regOffset2);
			LDH		(regDstValue, regColorAddr);

			regDstR = ExtractBitFieldTo255(block, regDstValue, 11, 15);
			regDstG = ExtractBitFieldTo255(block, regDstValue,  6, 10);
			regDstB = ExtractBitFieldTo255(block, regDstValue,  1,  5);
			regDstAlpha = ExtractBitFieldTo255(block, regDstValue,  0,  0);
		}

		break;

	case ColorFormatRGBA4444:
		{
			DECL_REG	(regConstant1);
			DECL_REG	(regOffset2);

			LDI		(regConstant1, 1);
			LSL		(regOffset2, regOffset, regConstant1);
			ADD		(regColorAddr, regColorBuffer, regOffset2);
			LDH		(regDstValue, regColorAddr);

			regDstR = ExtractBitFieldTo255(block, regDstValue, 12, 15);
			regDstG = ExtractBitFieldTo255(block, regDstValue,  8, 11);
			regDstB = ExtractBitFieldTo255(block, regDstValue,  4,  7);
			regDstAlpha = ExtractBitFieldTo255(block, regDstValue,  0,  3);
		}

		break;

	case ColorFormatRGBA8:
		{
			DECL_REG	(regConstant2);
			DECL_REG	(regOffset4);

			LDI		(regConstant2, 2);
			LSL		(regOffset4, regOffset, regConstant2);
			ADD		(regColorAddr, regColorBuffer, regOffset4);
			LDW		(regDstValue, regColorAddr);

			regDstR = ExtractBitFieldTo255(block, regDstValue, 24, 31);
			regDstG = ExtractBitFieldTo255(block, regDstValue, 16, 23);
			regDstB = ExtractBitFieldTo255(block, regDstValue,  8, 15);
			regDstAlpha = ExtractBitFieldTo255(block, regDstValue,  0,  7);
		}

		break;

	default:
		assert(0);
	}

	// Blending
	if (m_State->m_Blend.Enabled) {

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
		//Color srcCoeff, dstCoeff;

		// ------------------------------------------------------------------
		// In all the blending code, the assumption is that the blending
		// coefficients are in the range 0x00 - 0x100, while the color to
		// be modulated is R:5, G:6, B:5, A:8 bits format
		// ------------------------------------------------------------------

		switch (m_State->m_Blend.FuncSrc) {
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
					//srcCoeff = Color(Color::MAX, Color::MAX, Color::MAX, Color::MAX);

					regSrcBlendR = regColorR;
					regSrcBlendG = regColorG;
					regSrcBlendB = regColorB;
					regSrcBlendA = regColorA;
				}
				break;

			case RasterizerState::BlendFuncSrcDstColor:
				{
					//srcCoeff = color;	// adjust scaling of R, G, B
					regSrcBlendR = Mul255(block, regColorR, regDstR);
					regSrcBlendG = Mul255(block, regColorG, regDstG);
					regSrcBlendB = Mul255(block, regColorB, regDstB);
					regSrcBlendA = Mul255(block, regColorA, regDstAlpha);
				}
				break;

			case RasterizerState::BlendFuncSrcOneMinusDstColor:
				{
					//srcCoeff = Color(Color::MAX - color.R(), Color::MAX - color.G(), Color::MAX - color.B(), Color::MAX - color.A());

					DECL_CONST_REG	(constantMaxColor, 0xff);

					regSrcBlendR = Mul255(block, regColorR, Sub(block, constantMaxColor, regDstR));
					regSrcBlendG = Mul255(block, regColorG, Sub(block, constantMaxColor, regDstG));
					regSrcBlendB = Mul255(block, regColorB, Sub(block, constantMaxColor, regDstB));
					regSrcBlendA = Mul255(block, regColorA, Sub(block, constantMaxColor, regDstAlpha));
				}
				break;

			case RasterizerState::BlendFuncSrcSrcAlpha:
				//srcCoeff = Color(color.A(), color.A(), color.A(), color.A());
				//srcCoeff * color
				{
					regSrcBlendR = Mul255(block, regColorR, regColorA);
					regSrcBlendG = Mul255(block, regColorG, regColorA);
					regSrcBlendB = Mul255(block, regColorB, regColorA);
					regSrcBlendA = Mul255(block, regColorA, regColorA);
				}
				break;

			case RasterizerState::BlendFuncSrcOneMinusSrcAlpha:
				{
					//srcCoeff = Color(Color::MAX - color.A(), Color::MAX - color.A(), Color::MAX - color.A(), Color::MAX - color.A());
					DECL_CONST_REG	(constantMaxColor, 0xff);

					cg_virtual_reg_t * regOneMinusSrcAlpha = Sub(block, constantMaxColor, regColorA);

					regSrcBlendR = Mul255(block, regColorR, regOneMinusSrcAlpha);
					regSrcBlendG = Mul255(block, regColorG, regOneMinusSrcAlpha);
					regSrcBlendB = Mul255(block, regColorB, regOneMinusSrcAlpha);
					regSrcBlendA = Mul255(block, regColorA, regOneMinusSrcAlpha);
				}
				break;

			case RasterizerState::BlendFuncSrcDstAlpha:
				//srcCoeff = Color(dstAlpha, dstAlpha, dstAlpha, dstAlpha);
				//srcCoeff * color
				{
					regSrcBlendR = Mul255(block, regColorR, regDstAlpha);
					regSrcBlendG = Mul255(block, regColorG, regDstAlpha);
					regSrcBlendB = Mul255(block, regColorB, regDstAlpha);
					regSrcBlendA = Mul255(block, regColorA, regDstAlpha);
				}

				break;

			case RasterizerState::BlendFuncSrcOneMinusDstAlpha:
					//srcCoeff = Color(Color::MAX - dstAlpha, Color::MAX - dstAlpha, Color::MAX - dstAlpha, Color::MAX - dstAlpha);
				{
					DECL_CONST_REG	(constantMaxColor, 0xff);

					cg_virtual_reg_t * regOneMinusDstAlpha = Sub(block, constantMaxColor, regDstAlpha);

					regSrcBlendR = Mul255(block, regColorR, regOneMinusDstAlpha);
					regSrcBlendG = Mul255(block, regColorG, regOneMinusDstAlpha);
					regSrcBlendB = Mul255(block, regColorB, regOneMinusDstAlpha);
					regSrcBlendA = Mul255(block, regColorA, regOneMinusDstAlpha);
				}
				break;

			case RasterizerState::BlendFuncSrcSrcAlphaSaturate:
				{
					DECL_CONST_REG	(constantMaxColor, 0xff);

					//	U8 rev = Color::MAX - dstAlpha;
					cg_virtual_reg_t * regRev = Sub(block, constantMaxColor, regDstAlpha);

					//	U8 f = (rev < color.A() ? rev : color.A());
					DECL_REG(regF);
					MIN		(regF, regRev, regColorA);

					//	dstCoeff = Color(f, f, f, Color::MAX);
					//dstCoeff * dstColor
					regSrcBlendR = Mul255(block, regColorR, regF);
					regSrcBlendG = Mul255(block, regColorG, regF);
					regSrcBlendB = Mul255(block, regColorB, regF);
					regSrcBlendA = regColorA;
				}
				break;
		}

		switch (m_State->m_Blend.FuncDst) {
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

					regDstBlendR = regDstR;
					regDstBlendG = regDstG;
					regDstBlendB = regDstB;
					regDstBlendA = regDstAlpha;
				}
				break;

			case RasterizerState::BlendFuncDstSrcColor:
				{
					//dstCoeff = color;
					//dstCoeff * dstColor
					regDstBlendR = Mul255(block, regDstR, regColorR);
					regDstBlendG = Mul255(block, regDstG, regColorG);
					regDstBlendB = Mul255(block, regDstB, regColorB);
					regDstBlendA = Mul255(block, regDstAlpha, regColorA);
				}
				break;

			case RasterizerState::BlendFuncDstOneMinusSrcColor:
				{
					//dstCoeff = Color(Color::MAX - color.R(), Color::MAX - color.G(), Color::MAX - color.B(), Color::MAX - color.A());
					//dstCoeff * dstColor
					DECL_CONST_REG	(constantMaxColor, 0xff);

					regDstBlendR = Mul255(block, regDstR, Sub(block, constantMaxColor, regColorR));
					regDstBlendG = Mul255(block, regDstG, Sub(block, constantMaxColor, regColorG));
					regDstBlendB = Mul255(block, regDstB, Sub(block, constantMaxColor, regColorB));
					regDstBlendA = Mul255(block, regDstAlpha, Sub(block, constantMaxColor, regColorA));
				}
				break;

			case RasterizerState::BlendFuncDstSrcAlpha:
				{
					//dstCoeff = Color(color.A(), color.A(), color.A(), color.A());
					//dstCoeff * dstColor
					regDstBlendR = Mul255(block, regDstR, regColorA);
					regDstBlendG = Mul255(block, regDstG, regColorA);
					regDstBlendB = Mul255(block, regDstB, regColorA);
					regDstBlendA = Mul255(block, regDstAlpha, regColorA);
				}
				break;

			case RasterizerState::BlendFuncDstOneMinusSrcAlpha:
				{
					//dstCoeff = Color(Color::MAX - color.A(), Color::MAX - color.A(), Color::MAX - color.A(), Color::MAX - color.A());
					//dstCoeff * dstColor

					DECL_CONST_REG	(constantMaxColor, 0xff);

					cg_virtual_reg_t * regOneMinusSrcAlpha = Sub(block, constantMaxColor, regColorA);

					regDstBlendR = Mul255(block, regDstR, regOneMinusSrcAlpha);
					regDstBlendG = Mul255(block, regDstG, regOneMinusSrcAlpha);
					regDstBlendB = Mul255(block, regDstB, regOneMinusSrcAlpha);
					regDstBlendA = Mul255(block, regDstAlpha, regOneMinusSrcAlpha);
				}
				break;

			case RasterizerState::BlendFuncDstDstAlpha:
				{
					//dstCoeff = Color(dstAlpha, dstAlpha, dstAlpha, dstAlpha);
					//dstCoeff * dstColor
					regDstBlendR = Mul255(block, regDstR, regDstAlpha);
					regDstBlendG = Mul255(block, regDstG, regDstAlpha);
					regDstBlendB = Mul255(block, regDstB, regDstAlpha);
					regDstBlendA = Mul255(block, regDstAlpha, regDstAlpha);
				}
				break;

			case RasterizerState::BlendFuncDstOneMinusDstAlpha:
				{
					//dstCoeff = Color(Color::MAX - dstAlpha, Color::MAX - dstAlpha, Color::MAX - dstAlpha, Color::MAX - dstAlpha);
					//dstCoeff * dstColor

					DECL_CONST_REG	(constantMaxColor, 0xff);

					cg_virtual_reg_t * regOneMinusDstAlpha = Sub(block, constantMaxColor, regDstAlpha);

					regDstBlendR = Mul255(block, regDstR, regOneMinusDstAlpha);
					regDstBlendG = Mul255(block, regDstG, regOneMinusDstAlpha);
					regDstBlendB = Mul255(block, regDstB, regOneMinusDstAlpha);
					regDstBlendA = Mul255(block, regDstAlpha, regOneMinusDstAlpha);
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
			regColorR = AddSaturate255(block, regSrcBlendR, regDstBlendR);
			regColorG = AddSaturate255(block, regSrcBlendG, regDstBlendG);
			regColorB = AddSaturate255(block, regSrcBlendB, regDstBlendB);
			regColorA = AddSaturate255(block, regSrcBlendA, regDstBlendA);
		}

		regColorWord = 0;
	}

	if (!regColorWord) {
		regColorWord = ColorWordFromRGBA(block, regColorR, regColorG, regColorB, regColorA);
	}

	if (m_State->m_LogicOp.Enabled) {

		//U32 newValue = maskedColor.ConvertToRGBA();
		//U32 oldValue = Color::From565A(dstValue, dstAlpha).ConvertToRGBA();
		//U32 value;
		cg_virtual_reg_t * regNewValue = regColorWord;
		cg_virtual_reg_t * regOldValue = regDstValue;

		regColorWord = cg_virtual_reg_create(procedure, cg_reg_type_general);
		
		switch (m_State->m_LogicOp.Opcode) {
			default:
			case RasterizerState:: LogicOpClear:		
				//value = 0;						
				XOR		(regColorWord, regOldValue, regOldValue);
				break;

			case RasterizerState:: LogicOpAnd:			
				//value = newValue & dstValue;	
				AND		(regColorWord, regOldValue, regNewValue);
				break;

			case RasterizerState:: LogicOpAndReverse:	
				{
				//value = newValue & ~dstValue;	
				DECL_REG (regNotOldValue);

				NOT		(regNotOldValue, regOldValue);
				AND		(regColorWord, regNotOldValue, regNewValue);
				break;
				}

			case RasterizerState:: LogicOpCopy:			
				//value = newValue;		
				regColorWord = regNewValue;
				break;

			case RasterizerState:: LogicOpAndInverted:	
				{
				//value = ~newValue & dstValue;	
				DECL_REG (regNotNewValue);

				NOT		(regNotNewValue, regNewValue);
				AND		(regColorWord, regNotNewValue, regOldValue);
				break;
				}

			case RasterizerState:: LogicOpNoop:			
				//value = dstValue;				
				regColorWord = regOldValue;
				break;

			case RasterizerState:: LogicOpXor:			
				//value = newValue ^ dstValue;	
				XOR		(regColorWord, regOldValue, regNewValue);
				break;

			case RasterizerState:: LogicOpOr:			
				//value = newValue | dstValue;	
				OR		(regColorWord, regOldValue, regNewValue);
				break;

			case RasterizerState:: LogicOpNor:			
				{
				//value = ~(newValue | dstValue); 
				DECL_REG(regCombinedValue);

				OR		(regCombinedValue, regOldValue, regNewValue);
				NOT		(regColorWord, regCombinedValue);
				break;
				}

			case RasterizerState:: LogicOpEquiv:		
				{
				//value = ~(newValue ^ dstValue); 
				DECL_REG(regCombinedValue);

				XOR		(regCombinedValue, regOldValue, regNewValue);
				NOT		(regColorWord, regCombinedValue);
				break;
				}

			case RasterizerState:: LogicOpInvert:		
				//value = ~dstValue;				
				NOT		(regColorWord, regOldValue);
				break;

			case RasterizerState:: LogicOpOrReverse:	
				{
				//value = newValue | ~dstValue;	
				DECL_REG (regNotOldValue);

				NOT		(regNotOldValue, regOldValue);
				OR		(regColorWord, regNotOldValue, regNewValue);
				break;
				}

			case RasterizerState:: LogicOpCopyInverted:	
				//value = ~newValue;			
				NOT		(regColorWord, regNewValue);
				break;

			case RasterizerState:: LogicOpOrInverted:	
				{
				//value = ~newValue | dstValue;	
				DECL_REG (regNotNewValue);

				NOT		(regNotNewValue, regNewValue);
				OR		(regColorWord, regNotNewValue, regOldValue);
				break;
				}

			case RasterizerState:: LogicOpNand:			
				{
				//value = ~(newValue & dstValue); 
				DECL_REG(regCombinedValue);

				AND		(regCombinedValue, regOldValue, regNewValue);
				NOT		(regColorWord, regCombinedValue);
				break;
				}

			case RasterizerState:: LogicOpSet:			
				//value = 0xFFFF;					
				{
				LDI		(regColorWord,	EncodeColor(m_State->GetColorFormat(), Color(0xff, 0xff, 0xff, 0xff)));
				}
				break;
		}
	} 
	
	U32 mask = 
		EncodeColor(m_State->GetColorFormat(), 
					Color(0xff, 0xff, 0xff, 0xff).Mask(m_State->m_Mask.Red, 
													   m_State->m_Mask.Green, 
													   m_State->m_Mask.Blue, 
													   m_State->m_Mask.Alpha));
	U32 fullMask = 
		EncodeColor(m_State->GetColorFormat(), Color(0xff, 0xff, 0xff, 0xff));

	//Color maskedColor = 
	//	color.Mask(m_State->m_MaskRed, m_State->m_MaskGreen, m_State->m_MaskBlue, m_State->m_MaskAlpha);
	if (mask != fullMask) {
		//m_Surface->GetColorBuffer()[offset] = maskedColor.ConvertTo565();
		DECL_REG	(regSrcMask);
		DECL_REG	(regDstMask);
		DECL_REG	(regMaskedSrc);
		DECL_REG	(regMaskedDst);
		DECL_REG	(regCombined);


		LDI		(regSrcMask, mask);
		LDI		(regDstMask, !mask);
		AND		(regMaskedSrc, regColorWord, regSrcMask);
		AND		(regMaskedDst, regDstValue, regDstMask);
		OR		(regCombined, regMaskedSrc, regMaskedDst);

		regColorWord = regCombined;
	}

	switch (m_State->GetColorFormat()) {
	case ColorFormatRGB565:
	case ColorFormatRGBA5551:
	case ColorFormatRGBA4444:
		STH		(regColorWord, regColorAddr);
		break;

	case ColorFormatRGBA8:
		STW		(regColorWord, regColorAddr);
		break;

	default:
		assert(false);
	}
}

// --------------------------------------------------------------------------
// Implementation of virtual functions in PipelinePart
// --------------------------------------------------------------------------

void RasterPart :: CopyState(void * target, const void * source) const {
	new (target) RasterizerState(*static_cast<const RasterizerState *>(source));
}



