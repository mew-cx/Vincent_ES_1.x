#ifndef EGL_TRIBM_INST_H
#define EGL_TRIBM_INST_H 1

#pragma once

// ==========================================================================
//
// inst.h		triVM Intermediate Language for OpenGL (R) ES Implementation
//				Processor independent instruction formats
//				
//				This class is part of the runtime compiler infrastructure
//				used by the OpenGL|ES implementation for compiling
//				shader code at runtime into machine language.
//
// --------------------------------------------------------------------------
//
// 01-10-2004	Hans-Martin Will	initial version
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


#include "trivm.h"



namespace EGL {
namespace triVM {

	// -------------------------------------------------------------------------
	// triVM base formats
	// -------------------------------------------------------------------------
	struct InstructionUnaryType: public InstructionBaseType {
		int rS;
		int rD;
		int rC;

		InstructionUnaryType(Opcode anOpcode, int _rD, int _rS, const char * comment = "");

		InstructionUnaryType(Opcode anOpcode, int _rD, int _rC, int _rS, const char * comment = "");

	};

	struct InstructionBinaryType: public InstructionBaseType {
		int rS;
		int rD;
		int rC;
		int rM;

		InstructionBinaryType(Opcode anOpcode, int _rD, int _rS, int _rM, const char * comment = "");

		InstructionBinaryType(Opcode anOpcode, int _rD, int _rC, int _rS, int _rM, const char * comment = "");

	};

	struct InstructionCompareType: public InstructionBaseType {
		int rS;
		int rD;
		int rC;

		InstructionCompareType(Opcode anOpcode, int _rD, int _rS, int _rC, const char * comment = "");

	};

	struct InstructionLoadType: public InstructionBaseType {
		int rS;
		int rD;

		InstructionLoadType(Opcode anOpcode, int _rD, int _rS, const char * comment = "");

	};

	struct InstructionStoreType: public InstructionBaseType {
		int rS;
		int rD;

		InstructionStoreType(Opcode anOpcode, int _rD, int _rS, const char * comment = "");

	};

	struct InstructionLoadImmediateType: public InstructionBaseType {
		int rD;
		Constant * constant;

		InstructionLoadImmediateType(Opcode anOpcode, int _rD, Constant * _value, const char * comment = "");

	};

	struct InstructionBranchRegType: public InstructionBaseType {
		int rS;

		InstructionBranchRegType(Opcode anOpcode, int _rS, const char * comment = "");

	};

	struct InstructionBranchLabelType: public InstructionBaseType {
		Label * label;

		InstructionBranchLabelType(Opcode anOpcode, Label * _label, const char * comment = "");

	};

	struct InstructionBranchConditionallyType: public InstructionBaseType {
		int rS;
		Label * label;

		InstructionBranchConditionallyType(Opcode anOpcode, int _rS, Label * _label, const char * comment = "");

	};

	struct InstructionPhiType: public InstructionBaseType {
		int rD;
		RegisterList * registers;

		InstructionPhiType(Opcode anOpcode, int _rD, RegisterList * _registers, const char * comment = "");

	};

	struct InstructionCallType: public InstructionBaseType {
		int rS;
		int rD;
		RegisterList * args;

		InstructionCallType(Opcode anOpcode, int _rS, RegisterList * _args,
			int rD = -1, const char * comment = "");

	};

	struct InstructionRetType: public InstructionBaseType {
		int rS;

		InstructionRetType(Opcode anOpcode, int rS = -1, const char * comment = "");
	};


	// -------------------------------------------------------------------------
	// ARM-specific formats
	// -------------------------------------------------------------------------


	struct InstructionArmUnaryImmediateType: public InstructionBaseType {
		int immed;
		int rD;
		int rC;

		InstructionArmUnaryImmediateType(Opcode anOpcode, int _rD, int _immed, const char * comment = "");
		InstructionArmUnaryImmediateType(Opcode anOpcode, int _rD, int _rC, int _immed, const char * comment = "");

	};

	struct InstructionArmUnaryShiftRegType: public InstructionBaseType {
		Opcode opShift;
		int rShift;
		int rS;
		int rD;
		int rC;

		InstructionArmUnaryShiftRegType(Opcode anOpcode, int _rD, int _rS, Opcode _opShift, int _rShift, const char * comment = "");
		InstructionArmUnaryShiftRegType(Opcode anOpcode, int _rD, int _rC, int _rS, Opcode _opShift, int _rShift, const char * comment = "");

	};

	struct InstructionArmUnaryShiftImmedType: public InstructionBaseType {
		Opcode opShift;
		int shiftBits;
		int rS;
		int rD;
		int rC;

		InstructionArmUnaryShiftImmedType(Opcode anOpcode, int _rD, int _rS, Opcode _opShift, int _shiftBits, const char * comment = "");
		InstructionArmUnaryShiftImmedType(Opcode anOpcode, int _rD, int _rC, int _rS, Opcode _opShift, int _shiftBits, const char * comment = "");

	};

	struct InstructionArmBinaryImmediateType: public InstructionBaseType {
		int immed;
		int rS;
		int rD;
		int rC;

		InstructionArmBinaryImmediateType(Opcode anOpcode, int _rD, int _rS, int _immed, const char * comment = "");

		InstructionArmBinaryImmediateType(Opcode anOpcode, int _rD, int _rC, int _rS, int _immed, const char * comment = "");

	};

	struct InstructionArmBinaryShiftImmedType: public InstructionBaseType {
		Opcode opShift;
		int shiftBits;
		int rS;
		int rD;
		int rC;
		int rM;

		InstructionArmBinaryShiftImmedType(Opcode anOpcode, int _rD, int _rS, int _rM, Opcode _opShift, int _shiftBits, const char * comment = "");

		InstructionArmBinaryShiftImmedType(Opcode anOpcode, int _rD, int _rC, int _rS, int _rM, Opcode _opShift, int _shiftBits, const char * comment = "");

	};

	struct InstructionArmBinaryShiftRegType: public InstructionBaseType {
		Opcode opShift;
		int rShift;
		int rS;
		int rD;
		int rC;
		int rM;

		InstructionArmBinaryShiftRegType(Opcode anOpcode, int _rD, int _rS, int _rM, Opcode _opShift, int _rShift, const char * comment = "");

		InstructionArmBinaryShiftRegType(Opcode anOpcode, int _rD, int _rC, int _rS, int _rM, Opcode _opShift, int _rShift, const char * comment = "");

	};

	struct InstructionArmLoadImmedOffsetType: public InstructionBaseType {
		int rS;
		int rD;
		int immed12;

		InstructionArmLoadImmedOffsetType(Opcode anOpcode, int _rD, int _rS, int _immed12, const char * comment = "");

	};

	struct InstructionArmLoadRegOffsetType: public InstructionBaseType {
		int rS;
		int rD;
		int rOffset;

		InstructionArmLoadRegOffsetType(Opcode anOpcode, int _rD, int _rS, int _rOffset, const char * comment = "");

	};

	struct InstructionArmStoreImmedOffsetType: public InstructionBaseType {
		int rS;
		int rD;
		int immed12;

		InstructionArmStoreImmedOffsetType(Opcode anOpcode, int _rD, int _rS, int _immed12, const char * comment = "");

	};

	struct InstructionArmStoreRegOffsetType: public InstructionBaseType {
		int rS;
		int rD;
		int rOffset;

		InstructionArmStoreRegOffsetType(Opcode anOpcode, int _rD, int _rS, int _rOffset, const char * comment = "");

	};

	enum InstructionKind {
		InstructionNone,
		InstructionUnary,		
		InstructionBinary,		
		InstructionCompare,	
		InstructionLoad,	
		InstructionStore,	
		InstructionLoadImmediate,
		InstructionBranchReg,
		InstructionBranchLabel,	
		InstructionBranchConditionally,	
		InstructionPhi,	
		InstructionCall,
		InstructionRet,						

		// ARM specific formats
		InstructionArmUnaryImmediate,	
		InstructionArmUnaryShiftReg,	
		InstructionArmUnaryShiftImmed,	
		InstructionArmBinaryImmediate,	
		InstructionArmBinaryShiftReg,	
		InstructionArmBinaryShiftImmed,	
		InstructionArmLoadImmedOffset,	
		InstructionArmLoadRegOffset,	
		InstructionArmStoreImmedOffset,	
		InstructionArmStoreRegOffset,	

		InstructionKindCount
	};

	enum Opcode {
		nop,					
		add,	and,	asr,	cmp,	div,	lsl,	lsr,	mod,	mul,	neg,	
		not,	or,		sub,	udiv,	umod,	xor,									
		fadd,	fcmp,	fdiv,	fmul,	fneg,	fsub,	finv,	fsqrt,					
		trunc,	round,	fcnv,															
		bae,	bbl,	beq,	bge,	ble,	bgt,	blt,	bne,	bra,			
		ldb,	ldh,	ldi,	ldw,	stb,	sth,	stw,							
		call,	ret,	phi
	};



} // namespace triVM
} // namespace EGL


#endif //ndef EGL_TRIBM_INST_H
