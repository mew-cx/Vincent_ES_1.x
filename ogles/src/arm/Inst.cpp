// ==========================================================================
//
// inst.cpp		triVM Intermediate Language for OpenGL (R) ES Implementation
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


#include "stdafx.h"
#include "fixed.h"
#include "trivm.h"
#include "Inst.h"
#include "Init.h"


using namespace EGL;
using namespace triVM;


InstructionUnaryType::InstructionUnaryType(Opcode anOpcode, int _rD, int _rS, const char * comment):
	InstructionBaseType(InstructionUnary, anOpcode, comment) {
	rD = _rD;
	rS = _rS;
	rC = -1;
}

InstructionUnaryType::InstructionUnaryType(Opcode anOpcode, int _rD, int _rC, int _rS, const char * comment):
	InstructionBaseType(InstructionUnary, anOpcode, comment) {
	rD = _rD;
	rS = _rS;
	rC = _rC;
}


InstructionBinaryType::InstructionBinaryType(Opcode anOpcode, int _rD, int _rS, int _rM, const char * comment):
	InstructionBaseType(InstructionBinary, anOpcode, comment) {
	rD = _rD;
	rS = _rS;
	rC = -1;
	rM = _rM;
}

InstructionBinaryType::InstructionBinaryType(Opcode anOpcode, int _rD, int _rC, int _rS, int _rM, const char * comment):
	InstructionBaseType(InstructionBinary, anOpcode, comment) {
	rD = _rD;
	rS = _rS;
	rC = _rC;
	rM = _rM;
}


InstructionCompareType::InstructionCompareType(Opcode anOpcode, int _rD, int _rS, int _rC, const char * comment):
	InstructionBaseType(InstructionCompare, anOpcode, comment) {
	rD = _rD;
	rC = _rC;
	rS = _rS;
}

InstructionLoadType::InstructionLoadType(Opcode anOpcode, int _rD, int _rS, const char * comment):
	InstructionBaseType(InstructionLoad, anOpcode, comment) {
	rD = _rD;
	rS = _rS;
}

InstructionStoreType::InstructionStoreType(Opcode anOpcode, int _rD, int _rS, const char * comment):
	InstructionBaseType(InstructionStore, anOpcode, comment) {
	rD = _rD;
	rS = _rS;
}

InstructionLoadImmediateType::InstructionLoadImmediateType(Opcode anOpcode, int _rD, Constant * _value, const char * comment) :
	InstructionBaseType(InstructionLoadImmediate, anOpcode, comment) {
	rD = _rD;
	constant = _value;
}

InstructionBranchRegType::InstructionBranchRegType(Opcode anOpcode, int _rS, const char * comment) :
	InstructionBaseType(InstructionBranchReg, anOpcode, comment) {
	rS = _rS;
}

InstructionBranchLabelType::InstructionBranchLabelType(Opcode anOpcode, Label * _label, const char * comment) :
	InstructionBaseType(InstructionBranchLabel, anOpcode, comment) {
	label = _label;
}

InstructionBranchConditionallyType::InstructionBranchConditionallyType(Opcode anOpcode, int _rS, Label * _label, const char * comment) :
	InstructionBaseType(InstructionBranchConditionally, anOpcode, comment) {
	rS = _rS;
	label = _label;
}

InstructionPhiType::InstructionPhiType(Opcode anOpcode, int _rD, RegisterList * _registers, const char * comment) :
	InstructionBaseType(InstructionPhi, anOpcode, comment) {
	rD = _rD;
	registers = _registers;
}

InstructionCallType::InstructionCallType(Opcode anOpcode, int _rS, RegisterList * _args,
	int _rD, const char * comment)  :
	InstructionBaseType(InstructionCall, anOpcode, comment) {
	rS = _rS;
	args = _args;
	rD = _rD;
}

InstructionRetType::InstructionRetType(Opcode anOpcode, int _rS, const char * comment) :
	InstructionBaseType(InstructionRet, anOpcode, comment) {
	rS = _rS;
}


	InstructionArmUnaryImmediateType::InstructionArmUnaryImmediateType(Opcode anOpcode, int _rD, int _immed, const char * comment) :
	InstructionBaseType(InstructionArmUnaryImmediate, anOpcode, comment) {
	rD = _rD;
	rC = -1;
	immed = _immed;
}

InstructionArmUnaryImmediateType::InstructionArmUnaryImmediateType(Opcode anOpcode, int _rD, int _rC, int _immed, const char * comment) :
	InstructionBaseType(InstructionArmUnaryImmediate, anOpcode, comment) {
	rD = _rD;
	rC = _rC;
	immed = _immed;
}


InstructionArmUnaryShiftRegType::InstructionArmUnaryShiftRegType(Opcode anOpcode, int _rD, int _rS, Opcode _opShift, int _rShift, const char * comment) :
	InstructionBaseType(InstructionArmUnaryShiftReg, anOpcode, comment) {
	rD = _rD;
	rC = -1;
	rS = _rS;
	opShift = _opShift;
	rShift = _rShift;
}


InstructionArmUnaryShiftRegType::InstructionArmUnaryShiftRegType(Opcode anOpcode, int _rD, int _rC, int _rS, Opcode _opShift, int _rShift, const char * comment) :
	InstructionBaseType(InstructionArmUnaryShiftReg, anOpcode, comment) {
	rD = _rD;
	rC = _rC;
	rS = _rS;
	opShift = _opShift;
	rShift = _rShift;
}



InstructionArmUnaryShiftImmedType::InstructionArmUnaryShiftImmedType(Opcode anOpcode, int _rD, int _rS, Opcode _opShift, int _shiftBits, const char * comment) :
	InstructionBaseType(InstructionArmUnaryShiftImmed, anOpcode, comment) {
	rD = _rD;
	rC = -1;
	rS = _rS;
	opShift = _opShift;
	shiftBits = _shiftBits;
}


InstructionArmUnaryShiftImmedType::InstructionArmUnaryShiftImmedType(Opcode anOpcode, int _rD, int _rC, int _rS, Opcode _opShift, int _shiftBits, const char * comment) :
	InstructionBaseType(InstructionArmUnaryShiftImmed, anOpcode, comment) {
	rD = _rD;
	rC = _rC;
	rS = _rS;
	opShift = _opShift;
	shiftBits = _shiftBits;
}



InstructionArmBinaryImmediateType::InstructionArmBinaryImmediateType(Opcode anOpcode, int _rD, int _rS, int _immed, const char * comment) :
	InstructionBaseType(InstructionArmBinaryImmediate, anOpcode, comment) {
	rD = _rD;
	rC = -1;
	rS = _rS;
	immed = _immed;
}



InstructionArmBinaryImmediateType::InstructionArmBinaryImmediateType(Opcode anOpcode, int _rD, int _rC, int _rS, int _immed, const char * comment) :
	InstructionBaseType(InstructionArmBinaryImmediate, anOpcode, comment) {
	rD = _rD;
	rC = _rC;
	rS = _rS;
	immed = _immed;
}



InstructionArmBinaryShiftImmedType::InstructionArmBinaryShiftImmedType(Opcode anOpcode, int _rD, int _rS, int _rM, Opcode _opShift, int _shiftBits, const char * comment) :
	InstructionBaseType(InstructionArmBinaryShiftImmed, anOpcode, comment) {
	rD = _rD;
	rC = -1;
	rS = _rS;
	rM = _rM;
	opShift = _opShift;
	shiftBits = _shiftBits;
}



InstructionArmBinaryShiftImmedType::InstructionArmBinaryShiftImmedType(Opcode anOpcode, int _rD, int _rC, int _rS, int _rM, Opcode _opShift, int _shiftBits, const char * comment) :
	InstructionBaseType(InstructionArmBinaryShiftImmed, anOpcode, comment) {
	rD = _rD;
	rC = _rC;
	rS = _rS;
	rM = _rM;
	opShift = _opShift;
	shiftBits = _shiftBits;
}



InstructionArmBinaryShiftRegType::InstructionArmBinaryShiftRegType(Opcode anOpcode, int _rD, int _rS, int _rM, Opcode _opShift, int _rShift, const char * comment) :
	InstructionBaseType(InstructionArmBinaryShiftReg, anOpcode, comment) {
	rD = _rD;
	rC = -1;
	rS = _rS;
	rM = _rM;
	opShift = _opShift;
	rShift = _rShift;
}



InstructionArmBinaryShiftRegType::InstructionArmBinaryShiftRegType(Opcode anOpcode, int _rD, int _rC, int _rS, int _rM, Opcode _opShift, int _rShift, const char * comment) :
	InstructionBaseType(InstructionArmBinaryShiftReg, anOpcode, comment) {
	rD = _rD;
	rC = _rC;
	rS = _rS;
	rM = _rM;
	opShift = _opShift;
	rShift = _rShift;
}



InstructionArmLoadImmedOffsetType::InstructionArmLoadImmedOffsetType(Opcode anOpcode, int _rD, int _rS, int _immed12, const char * comment) :
	InstructionBaseType(InstructionArmLoadImmedOffset, anOpcode, comment) {
	rD = _rD;
	rS = _rS;
	immed12 = _immed12;
}



InstructionArmLoadRegOffsetType::InstructionArmLoadRegOffsetType(Opcode anOpcode, int _rD, int _rS, int _rOffset, const char * comment) :
	InstructionBaseType(InstructionArmLoadRegOffset, anOpcode, comment) {
	rD = _rD;
	rS = _rS;
	rOffset = _rOffset;
}



InstructionArmStoreImmedOffsetType::InstructionArmStoreImmedOffsetType(Opcode anOpcode, int _rD, int _rS, int _immed12, const char * comment) :
	InstructionBaseType(InstructionArmStoreImmedOffset, anOpcode, comment) {
	rD = _rD;
	rS = _rS;
	immed12 = _immed12;
}



InstructionArmStoreRegOffsetType::InstructionArmStoreRegOffsetType(Opcode anOpcode, int _rD, int _rS, int _rOffset, const char * comment) :
	InstructionBaseType(InstructionArmStoreRegOffset, anOpcode, comment) {
	rD = _rD;
	rS = _rS;
	rOffset = _rOffset;
}



