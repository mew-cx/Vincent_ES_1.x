// ==========================================================================
//
// triVM.cpp	triVM Intermediate Language for OpenGL (R) ES Implementation
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
#include "Sweep.h"


using namespace EGL;
using namespace triVM;


namespace {
	class MarkUsed: public Sweep {
	public:
		MarkUsed(): changed(false) {
		}

		void resetChanged() {
			changed = false;
		}

		bool hasChanged() {
			return changed;
		}

		void markAllUsedInstructions(Module * module) {
			do {
				resetChanged();
				reverseSweep(module);
			} while (hasChanged());
		}

	protected:
		virtual void  visit(triVM::InstructionUnaryType * instruction){ 
			if (instruction->rC >= 0) {
				if (isUsed(instruction->rD) || isUsed(instruction->rC)) {
					instruction->used = true;
					addRegister(instruction->rS);
				} else {
					instruction->used = false;
				}
			} else {
				if (isUsed(instruction->rD)) {
					instruction->used = true;
					addRegister(instruction->rS);
				} else {
					instruction->used = false;
				}
			}
		}

		virtual void  visit(triVM::InstructionBinaryType * instruction){ 
			if (instruction->rC >= 0) {
				if (isUsed(instruction->rD) || isUsed(instruction->rC)) {
					instruction->used = true;
					addRegister(instruction->rS);
					addRegister(instruction->rM);
				} else {
					instruction->used = false;
				}
			} else {
				if (isUsed(instruction->rD)) {
					instruction->used = true;
					addRegister(instruction->rS);
					addRegister(instruction->rM);
				} else {
					instruction->used = false;
				}
			}
		}
		
		virtual void  visit(triVM::InstructionCompareType * instruction){ 

			if (isUsed(instruction->rD)) {
				instruction->used = true;
				addRegister(instruction->rS);
				addRegister(instruction->rC);
			} else {
				instruction->used = false;
			}
		}

		virtual void  visit(triVM::InstructionLoadType * instruction){ 

			if (isUsed(instruction->rD)) {
				instruction->used = true;
				addRegister(instruction->rS);
			} else {
				instruction->used = false;
			}
		}

		virtual void  visit(triVM::InstructionStoreType * instruction){ 
			// always executed

			instruction->used = true;
			addRegister(instruction->rS);
			addRegister(instruction->rD);
		}

		virtual void  visit(triVM::InstructionLoadImmediateType * instruction){ 
			if (isUsed(instruction->rD)) {
				instruction->used = true;
			}
		}

		virtual void  visit(triVM::InstructionBranchRegType * instruction){ 
			// always executed

			instruction->used = true;
			addRegister(instruction->rS);
		}

		virtual void  visit(triVM::InstructionBranchLabelType * instruction){ 
			// always executed
			instruction->used = true;
		}

		virtual void  visit(triVM::InstructionBranchConditionallyType * instruction){ 
			// always executed

			instruction->used = true;
			addRegister(instruction->rS);
		}

		virtual void  visit(triVM::InstructionPhiType * instruction) { 

			if (isUsed(instruction->rD)) {
				instruction->used = true;
				RegisterList * registers = instruction->registers;

				for (RegisterList::iterator iter = registers->begin(); iter != registers->end(); ++iter) {
					addRegister(*iter);
				}
			} else {
				instruction->used = false;
			}
		}

		virtual void  visit(triVM::InstructionCallType * instruction) { 
			// always executed

			instruction->used = true;
			RegisterList * registers = instruction->args;

			for (RegisterList::iterator iter = registers->begin(); iter != registers->end(); ++iter) {
				addRegister(*iter);
			}
		}

		virtual void  visit(triVM::InstructionRetType * instruction) { 
			// always executed

			instruction->used = true;

			if (instruction->rS != -1) {
				addRegister(instruction->rS);
			}
		}

		virtual void visit(triVM::InstructionArmUnaryImmediateType * instruction) { 
			if (instruction->rC >= 0) {
				if (isUsed(instruction->rD) || isUsed(instruction->rC)) {
					instruction->used = true;
				} else {
					instruction->used = false;
				}
			} else {
				if (isUsed(instruction->rD)) {
					instruction->used = true;
				} else {
					instruction->used = false;
				}
			}
		}

		virtual void visit(triVM::InstructionArmUnaryShiftRegType * instruction) { 
			if (instruction->rC >= 0) {
				if (isUsed(instruction->rD) || isUsed(instruction->rC)) {
					instruction->used = true;
					addRegister(instruction->rS);
					addRegister(instruction->rShift);
				} else {
					instruction->used = false;
				}
			} else {
				if (isUsed(instruction->rD)) {
					instruction->used = true;
					addRegister(instruction->rS);
					addRegister(instruction->rShift);
				} else {
					instruction->used = false;
				}
			}
		}

		virtual void visit(triVM::InstructionArmUnaryShiftImmedType * instruction) { 
			if (instruction->rC >= 0) {
				if (isUsed(instruction->rD) || isUsed(instruction->rC)) {
					instruction->used = true;
					addRegister(instruction->rS);
				} else {
					instruction->used = false;
				}
			} else {
				if (isUsed(instruction->rD)) {
					instruction->used = true;
					addRegister(instruction->rS);
				} else {
					instruction->used = false;
				}
			}
		}

		virtual void visit(triVM::InstructionArmBinaryImmediateType * instruction) { 
			if (instruction->rC >= 0) {
				if (isUsed(instruction->rD) || isUsed(instruction->rC)) {
					instruction->used = true;
					addRegister(instruction->rS);
				} else {
					instruction->used = false;
				}
			} else {
				if (isUsed(instruction->rD)) {
					instruction->used = true;
					addRegister(instruction->rS);
				} else {
					instruction->used = false;
				}
			}
		}

		virtual void visit(triVM::InstructionArmBinaryShiftRegType * instruction) { 
			if (instruction->rC >= 0) {
				if (isUsed(instruction->rD) || isUsed(instruction->rC)) {
					instruction->used = true;
					addRegister(instruction->rS);
					addRegister(instruction->rM);
					addRegister(instruction->rShift);
				} else {
					instruction->used = false;
				}
			} else {
				if (isUsed(instruction->rD)) {
					instruction->used = true;
					addRegister(instruction->rS);
					addRegister(instruction->rM);
					addRegister(instruction->rShift);
				} else {
					instruction->used = false;
				}
			}
		}

		virtual void visit(triVM::InstructionArmBinaryShiftImmedType * instruction) { 
			if (instruction->rC >= 0) {
				if (isUsed(instruction->rD) || isUsed(instruction->rC)) {
					instruction->used = true;
					addRegister(instruction->rS);
					addRegister(instruction->rM);
				} else {
					instruction->used = false;
				}
			} else {
				if (isUsed(instruction->rD)) {
					instruction->used = true;
					addRegister(instruction->rS);
					addRegister(instruction->rM);
				} else {
					instruction->used = false;
				}
			}
		}

		virtual void visit(triVM::InstructionArmLoadImmedOffsetType * instruction) { 
			if (isUsed(instruction->rD)) {
				instruction->used = true;
				addRegister(instruction->rS);
			} else {
				instruction->used = false;
			}
		}

		virtual void visit(triVM::InstructionArmLoadRegOffsetType * instruction) { 
			if (isUsed(instruction->rD)) {
				instruction->used = true;
				addRegister(instruction->rS);
				addRegister(instruction->rOffset);
			} else {
				instruction->used = false;
			}
		}

		virtual void visit(triVM::InstructionArmStoreImmedOffsetType * instruction) { 
			// always executed

			instruction->used = true;
			addRegister(instruction->rS);
			addRegister(instruction->rD);
		}

		virtual void visit(triVM::InstructionArmStoreRegOffsetType * instruction) { 
			// always executed

			instruction->used = true;
			addRegister(instruction->rS);
			addRegister(instruction->rD);
			addRegister(instruction->rOffset);
		}

	private:
		typedef std::set<int> RegisterSet;

		RegisterSet usedRegisters;
		bool changed;

	private:
		bool isUsed(int reg) {
			return usedRegisters.find(reg) != usedRegisters.end();
		}

		void addRegister(int reg) {
			if (usedRegisters.find(reg) == usedRegisters.end()) {
				usedRegisters.insert(reg);
				changed = true;
			}
		}
	};

}


// --------------------------------------------------------------------------
// Remove any unused instructions from the tree
// --------------------------------------------------------------------------
void CodeGenerator :: RemoveUnusedCode(Module * module) {

	for (ProcedureList::iterator iter = module->procedures.begin();
		iter != module->procedures.end(); ++iter) {

		Procedure * procedure = *iter;

		for (BlockList::iterator blockIter = procedure->blocks.begin();
			blockIter != procedure->blocks.end(); ++blockIter) {

			Block * block = *blockIter;
			InstructionList & instructions = block->instructions;

			for (InstructionList::iterator instIter = instructions.begin();
				instIter != instructions.end(); ++instIter) {
				Instruction * instruction = *instIter;

				instruction->used = false;
			}
		}
	}

	MarkUsed used;
	used.markAllUsedInstructions(module);

	for (ProcedureList::iterator iter = module->procedures.begin();
		iter != module->procedures.end(); ++iter) {

		Procedure * procedure = *iter;

		for (BlockList::iterator blockIter = procedure->blocks.begin();
			blockIter != procedure->blocks.end(); ++blockIter) {

			Block * block = *blockIter;
			InstructionList & instructions = block->instructions;

			for (InstructionList::iterator instIter = instructions.begin();
				instIter != instructions.end(); ) {
				Instruction * instruction = *instIter;

				if (instruction->used) {
					++instIter;
				} else {
					instructions.erase(instIter);
				}
			}
		}
	}
}


