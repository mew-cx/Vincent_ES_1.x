// ==========================================================================
//
// Transform	Sweeps for IL for OpenGL (R) ES Implementation
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
#include "Transform.h"

using namespace EGL;
using namespace triVM;


void Transform :: sweep(triVM::Module * module){ 

	begin(module);

	for (ProcedureList::iterator iter = module->procedures.begin();
		iter != module->procedures.end(); ++iter) {
		sweep(*iter);
	}

	end(module);
}

void Transform :: sweep(triVM::Procedure * procedure){ 

	begin(procedure);

	for (BlockList::iterator iter = procedure->blocks.begin();
		iter != procedure->blocks.end(); ++iter) {

		Block * block = *iter;
		sweep(block);
	}

	end(procedure);
}

void Transform :: sweep(triVM::Block * block){ 

	begin(block);

	for (InstructionList::iterator iter = block->instructions.begin();
		iter != block->instructions.end(); ++iter) {
		Instruction * oldInstruction = *iter;
		Instruction * newInstruction = dispatch(oldInstruction);

		if (oldInstruction != newInstruction) {
			*iter = newInstruction;
			newInstruction->block = block;
		}
	}

	end(block);
}

void Transform :: reverseSweep(triVM::Module * module){ 

	begin(module);

	for (ProcedureList::iterator iter = module->procedures.begin();
		iter != module->procedures.end(); ++iter) {
		reverseSweep(*iter);
	}

	end(module);
}

void Transform :: reverseSweep(triVM::Procedure * procedure){ 

	begin(procedure);

	for (BlockList::iterator iter = procedure->blocks.begin();
		iter != procedure->blocks.end(); ++iter) {

		Block * block = *iter;
		reverseSweep(block);
	}

	end(procedure);
}

void Transform :: reverseSweep(triVM::Block * block){ 

	begin(block);

	for (InstructionList::reverse_iterator iter = block->instructions.rbegin();
		iter != block->instructions.rend(); ++iter) {
		Instruction * oldInstruction = *iter;
		Instruction * newInstruction = dispatch(oldInstruction);

		if (oldInstruction != newInstruction) {
			*iter = newInstruction;
			newInstruction->block = block;
		}
	}

	end(block);
}

void Transform :: begin(triVM::Module * module){ }
void Transform :: begin(triVM::Procedure * procedure){ }
void Transform :: begin(triVM::Block * block){ }


triVM::Instruction * Transform :: dispatch(triVM::Instruction * instruction){ 
	switch (instruction->kind) {
	case InstructionUnary:
		return transform(static_cast<InstructionUnaryType *>(instruction));
		break;

	case InstructionBinary:
		return transform(static_cast<InstructionBinaryType *>(instruction));
		break;

	case InstructionCompare:
		return transform(static_cast<InstructionCompareType *>(instruction));
		break;

	case InstructionLoad:
		return transform(static_cast<InstructionLoadType *>(instruction));
		break;

	case InstructionStore:
		return transform(static_cast<InstructionStoreType *>(instruction));
		break;

	case InstructionLoadImmediate:
		return transform(static_cast<InstructionLoadImmediateType *>(instruction));
		break;

	case InstructionBranchReg:
		return transform(static_cast<InstructionBranchRegType *>(instruction));
		break;

	case InstructionBranchLabel:
		return transform(static_cast<InstructionBranchLabelType *>(instruction));
		break;

	case InstructionBranchConditionally:
		return transform(static_cast<InstructionBranchConditionallyType *>(instruction));
		break;

	case InstructionPhi:
		return transform(static_cast<InstructionPhiType *>(instruction));
		break;

	case InstructionCall:
		return transform(static_cast<InstructionCallType *>(instruction));
		break;

	case InstructionRet:
		return transform(static_cast<InstructionRetType *> (instruction));
		break;

	case InstructionArmUnaryImmediate:
        return transform(static_cast<triVM::InstructionArmUnaryImmediateType *> (instruction));
		break;

	case InstructionArmUnaryShiftReg:
        return transform(static_cast<triVM::InstructionArmUnaryShiftRegType *> (instruction));
		break;

	case InstructionArmUnaryShiftImmed:
        return transform(static_cast<triVM::InstructionArmUnaryShiftImmedType *> (instruction));
		break;

	case InstructionArmBinaryImmediate:
        return transform(static_cast<triVM::InstructionArmBinaryImmediateType *> (instruction));
		break;

	case InstructionArmBinaryShiftReg:
        return transform(static_cast<triVM::InstructionArmBinaryShiftRegType *> (instruction));
		break;

	case InstructionArmBinaryShiftImmed:
        return transform(static_cast<triVM::InstructionArmBinaryShiftImmedType *> (instruction));
		break;

	case InstructionArmLoadImmedOffset:
        return transform(static_cast<triVM::InstructionArmLoadImmedOffsetType *> (instruction));
		break;

	case InstructionArmLoadRegOffset:
        return transform(static_cast<triVM::InstructionArmLoadRegOffsetType *> (instruction));
		break;

	case InstructionArmStoreImmedOffset:
        return transform(static_cast<triVM::InstructionArmStoreImmedOffsetType *> (instruction));
		break;

	case InstructionArmStoreRegOffset:
        return transform(static_cast<triVM::InstructionArmStoreRegOffsetType *> (instruction));
		break;

	default:
		return instruction;
	}
}

void Transform :: end(triVM::Module * module){ }
void Transform :: end(triVM::Procedure * procedure){ }
void Transform :: end(triVM::Block * block){ }


#if 0
triVM::Instruction * Transform :: transform(triVM::InstructionUnaryType * instruction){ }
triVM::Instruction * Transform :: transform(triVM::InstructionBinaryType * instruction){ }
triVM::Instruction * Transform :: transform(triVM::InstructionCompareType * instruction){ }
triVM::Instruction * Transform :: transform(triVM::InstructionLoadType * instruction){ }
triVM::Instruction * Transform :: transform(triVM::InstructionStoreType * instruction){ }
triVM::Instruction * Transform :: transform(triVM::InstructionLoadImmediateType * instruction){ }
triVM::Instruction * Transform :: transform(triVM::InstructionBranchRegType * instruction){ }
triVM::Instruction * Transform :: transform(triVM::InstructionBranchLabelType * instruction){ }
triVM::Instruction * Transform :: transform(triVM::InstructionBranchConditionallyType * instruction){ }
triVM::Instruction * Transform :: transform(triVM::InstructionPhiType * instruction){ }
triVM::Instruction * Transform :: transform(triVM::InstructionCallType * instruction){ }
triVM::Instruction * Transform :: transform(triVM::InstructionRetType * instruction){ }

triVM::Instruction * Transform :: transform(triVM::InstructionArmUnaryImmediateType * instruction) { }
triVM::Instruction * Transform :: transform(triVM::InstructionArmUnaryShiftRegType * instruction) { }
triVM::Instruction * Transform :: transform(triVM::InstructionArmUnaryShiftImmedType * instruction) { }
triVM::Instruction * Transform :: transform(triVM::InstructionArmBinaryImmediateType * instruction) { }
triVM::Instruction * Transform :: transform(triVM::InstructionArmBinaryShiftRegType * instruction) { }
triVM::Instruction * Transform :: transform(triVM::InstructionArmBinaryShiftImmedType * instruction) { }
triVM::Instruction * Transform :: transform(triVM::InstructionArmLoadImmedOffsetType * instruction) { }
triVM::Instruction * Transform :: transform(triVM::InstructionArmLoadRegOffsetType * instruction) { }
triVM::Instruction * Transform :: transform(triVM::InstructionArmLoadRegImmedOffsetType * instruction) { }
triVM::Instruction * Transform :: transform(triVM::InstructionArmStoreImmedOffsetType * instruction) { }
triVM::Instruction * Transform :: transform(triVM::InstructionArmStoreRegOffsetType * instruction) { }
triVM::Instruction * Transform :: transform(triVM::InstructionArmStoreRegImmedOffsetType * instruction) { }	
#endif