// ==========================================================================
//
// Sweep.cpp	Sweeps for IL for OpenGL (R) ES Implementation
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
#include "Sweep.h"

using namespace EGL;
using namespace triVM;


void Sweep :: sweep(triVM::Module * module){ 

	begin(module);

	for (ProcedureList::iterator iter = module->procedures.begin();
		iter != module->procedures.end(); ++iter) {
		sweep(*iter);
	}

	end(module);
}

void Sweep :: sweep(triVM::Procedure * procedure){ 

	begin(procedure);

	for (BlockList::iterator iter = procedure->blocks.begin();
		iter != procedure->blocks.end(); ++iter) {

		Block * block = *iter;
		sweep(block);
	}

	end(procedure);
}

void Sweep :: sweep(triVM::Block * block){ 

	begin(block);

	for (InstructionList::iterator iter = block->instructions.begin();
		iter != block->instructions.end(); ++iter) {
		dispatch(*iter);
	}

	end(block);
}

void Sweep :: reverseSweep(triVM::Module * module){ 

	begin(module);

	for (ProcedureList::iterator iter = module->procedures.begin();
		iter != module->procedures.end(); ++iter) {
		reverseSweep(*iter);
	}

	end(module);
}

void Sweep :: reverseSweep(triVM::Procedure * procedure){ 

	begin(procedure);

	for (BlockList::iterator iter = procedure->blocks.begin();
		iter != procedure->blocks.end(); ++iter) {

		Block * block = *iter;
		reverseSweep(block);
	}

	end(procedure);
}

void Sweep :: reverseSweep(triVM::Block * block){ 

	begin(block);

	for (InstructionList::reverse_iterator iter = block->instructions.rbegin();
		iter != block->instructions.rend(); ++iter) {
		dispatch(*iter);
	}

	end(block);
}

void Sweep :: begin(triVM::Module * module){ }
void Sweep :: begin(triVM::Procedure * procedure){ }
void Sweep :: begin(triVM::Block * block){ }


void Sweep :: dispatch(triVM::Instruction * instruction){ 
	switch (instruction->kind) {
	case InstructionUnary:
		visit(static_cast<InstructionUnaryType *>(instruction));
		break;

	case InstructionBinary:
		visit(static_cast<InstructionBinaryType *>(instruction));
		break;

	case InstructionCompare:
		visit(static_cast<InstructionCompareType *>(instruction));
		break;

	case InstructionLoad:
		visit(static_cast<InstructionLoadType *>(instruction));
		break;

	case InstructionStore:
		visit(static_cast<InstructionStoreType *>(instruction));
		break;

	case InstructionLoadImmediate:
		visit(static_cast<InstructionLoadImmediateType *>(instruction));
		break;

	case InstructionBranchReg:
		visit(static_cast<InstructionBranchRegType *>(instruction));
		break;

	case InstructionBranchLabel:
		visit(static_cast<InstructionBranchLabelType *>(instruction));
		break;

	case InstructionBranchConditionally:
		visit(static_cast<InstructionBranchConditionallyType *>(instruction));
		break;

	case InstructionPhi:
		visit(static_cast<InstructionPhiType *>(instruction));
		break;

	case InstructionCall:
		visit(static_cast<InstructionCallType *>(instruction));
		break;

	case InstructionRet:
		visit(static_cast<InstructionRetType *> (instruction));
		break;
	}
}

void Sweep :: end(triVM::Module * module){ }
void Sweep :: end(triVM::Procedure * procedure){ }
void Sweep :: end(triVM::Block * block){ }


#if 0
void Sweep :: visit(triVM::InstructionUnaryType * instruction){ }
void Sweep :: visit(triVM::InstructionBinaryType * instruction){ }
void Sweep :: visit(triVM::InstructionCompareType * instruction){ }
void Sweep :: visit(triVM::InstructionLoadType * instruction){ }
void Sweep :: visit(triVM::InstructionStoreType * instruction){ }
void Sweep :: visit(triVM::InstructionLoadImmediateType * instruction){ }
void Sweep :: visit(triVM::InstructionBranchRegType * instruction){ }
void Sweep :: visit(triVM::InstructionBranchLabelType * instruction){ }
void Sweep :: visit(triVM::InstructionBranchConditionallyType * instruction){ }
void Sweep :: visit(triVM::InstructionPhiType * instruction){ }
void Sweep :: visit(triVM::InstructionCallType * instruction){ }
void Sweep :: visit(triVM::InstructionRetType * instruction){ }

void Sweep :: visit(triVM::InstructionArmUnaryImmediateType * instruction) { }
void Sweep :: visit(triVM::InstructionArmUnaryShiftRegType * instruction) { }
void Sweep :: visit(triVM::InstructionArmUnaryShiftImmedType * instruction) { }
void Sweep :: visit(triVM::InstructionArmBinaryImmediateType * instruction) { }
void Sweep :: visit(triVM::InstructionArmBinaryShiftRegType * instruction) { }
void Sweep :: visit(triVM::InstructionArmBinaryShiftImmedType * instruction) { }
void Sweep :: visit(triVM::InstructionArmLoadImmedOffsetType * instruction) { }
void Sweep :: visit(triVM::InstructionArmLoadRegOffsetType * instruction) { }
void Sweep :: visit(triVM::InstructionArmLoadRegImmedOffsetType * instruction) { }
void Sweep :: visit(triVM::InstructionArmStoreImmedOffsetType * instruction) { }
void Sweep :: visit(triVM::InstructionArmStoreRegOffsetType * instruction) { }
void Sweep :: visit(triVM::InstructionArmStoreRegImmedOffsetType * instruction) { }	
#endif