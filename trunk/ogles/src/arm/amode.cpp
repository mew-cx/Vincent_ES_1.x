// ==========================================================================
//
// triVM.cpp	triVM Intermediate Language for OpenGL (R) ES Implementation
//				Selection of ARM-specific addressing modes
//				
//				This class is part of the runtime compiler infrastructure
//				used by the OpenGL|ES implementation for compiling
//				shader code at runtime into machine language.
//
// --------------------------------------------------------------------------
//
// 01-06-2004	Hans-Martin Will	initial version
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
#include "fixed.h"
#include "trivm.h"
#include "Inst.h"
#include "Transform.h"


using namespace EGL;
using namespace triVM;


namespace {

	bool isArmBinaryDataOpcode(Opcode opcode) {
		switch (opcode) {
			case add:
			case and:
			case or:
			case sub:
			case xor:
			case fadd:
			case fsub:
				return true;

			default:
				return false;
		}
	}

	bool isShiftOrRotate(Opcode opcode) {
		switch (opcode) {
			case asr:
			case lsr:
			case lsl:
				return true;

			default:
				return false;
		}
	}

	bool isArmDataProcImmediate(U32 value) {
		// preliminary rule

		if ((value & 0xff) == value) {
			return true;
		}

		for (int shift = 2; shift <= 12; shift += 2) {
			if (((0xff << shift) & value) == value)
				return true;
		}

		return false;
	}

	class AmodeTransform: public Transform {

		RegisterDefinitionMap * defs;

	public:
		AmodeTransform(RegisterDefinitionMap * _defs): defs(_defs) {
		}

	protected:
		triVM::Instruction * transform(triVM::InstructionUnaryType * instruction) { 
			return instruction;
		}

		triVM::Instruction * transform(triVM::InstructionBinaryType * instruction){ 

			if (isArmBinaryDataOpcode(instruction->opcode)) {

				Instruction * operand = (*defs)[instruction->rM];

				if (operand == 0) {
					// this is iffy
					return instruction;
				}

				if (operand->kind == InstructionLoadImmediate) {
					InstructionLoadImmediateType * immediateOperand = static_cast<InstructionLoadImmediateType *>(operand);

					Constant * constant = immediateOperand->constant;

					if (constant->base.kind == ConstantInt && isArmDataProcImmediate(constant->intConstant.value) ||	
						constant->base.kind == ConstantFixed && isArmDataProcImmediate(constant->fixedConstant.value)) {

						int value;

						if (constant->base.kind == ConstantInt) {
							value = constant->intConstant.value;
						} else {
							value = constant->fixedConstant.value;
						}

						return new InstructionArmBinaryImmediateType(instruction->opcode,
							instruction->rD, instruction->rC, instruction->rS, value, instruction->comment);
					} else {
						return instruction;
					}
				} else if (operand->kind == InstructionBinary && isShiftOrRotate(operand->opcode)) {

					InstructionBinaryType * shiftOperand = static_cast<InstructionBinaryType *>(operand);
					Instruction * shifter = (*defs)[shiftOperand->rM];

					if (shifter->kind == InstructionLoadImmediate) {
						// create binary shift immediate
						InstructionLoadImmediateType * immediateOperand = static_cast<InstructionLoadImmediateType *>(shifter);
						Constant * constant = immediateOperand->constant;

						return new InstructionArmBinaryShiftImmedType(instruction->opcode,
							instruction->rD, instruction->rC, instruction->rS, shiftOperand->rS,
							operand->opcode, constant->intConstant.value,
							instruction->comment);
					} else {
						// create binary shift register
						return new InstructionArmBinaryShiftRegType(instruction->opcode,
							instruction->rD, instruction->rC, instruction->rS, shiftOperand->rS,
							operand->opcode, shiftOperand->rM,
							instruction->comment);
					}
				}
			} else if (isShiftOrRotate(instruction->opcode)) {
				Instruction * operand = (*defs)[instruction->rM];

				if (operand == 0) {
					// this is iffy
					return instruction;
				}

				if (operand->kind == InstructionLoadImmediate) {
					InstructionLoadImmediateType * immediateOperand = static_cast<InstructionLoadImmediateType *>(operand);

					Constant * constant = immediateOperand->constant;

					return new InstructionArmBinaryImmediateType(instruction->opcode,
						instruction->rD, instruction->rC, instruction->rS, constant->intConstant.value,
						instruction->comment);
				}
			}

			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionCompareType * instruction){ 
			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionLoadType * instruction){ 
			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionStoreType * instruction){ 
			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionLoadImmediateType * instruction){ 
			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionBranchRegType * instruction){ 
			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionBranchLabelType * instruction){ 
			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionBranchConditionallyType * instruction){ 
			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionPhiType * instruction){ 
			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionCallType * instruction){ 
			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionRetType * instruction){ 
			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionArmUnaryImmediateType * instruction) { 
			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionArmUnaryShiftRegType * instruction) { 
			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionArmUnaryShiftImmedType * instruction) { 
			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionArmBinaryImmediateType * instruction) { 
			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionArmBinaryShiftRegType * instruction) { 
			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionArmBinaryShiftImmedType * instruction) { 
			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionArmLoadImmedOffsetType * instruction) { 
			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionArmLoadRegOffsetType * instruction) { 
			return instruction;
		}



		triVM::Instruction * transform(triVM::InstructionArmStoreImmedOffsetType * instruction) { 
			return instruction;
		}


		triVM::Instruction * transform(triVM::InstructionArmStoreRegOffsetType * instruction) { 
			return instruction;
		}

	};
}


void CodeGenerator :: SelectAddressingModes(Module * module, RegisterDefinitionMap * defs) {
	AmodeTransform selectAmode(defs);
	selectAmode.sweep(module);
}
