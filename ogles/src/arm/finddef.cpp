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
	class FindDefinitionSweep: public Sweep {
	public:
		FindDefinitionSweep(RegisterDefinitionMap * result): definitions(result) {
		}

	protected:
		virtual void  visit(triVM::InstructionUnaryType * instruction){ 
			if (instruction->rC >= 0) {
				addRegister(instruction->rC, instruction);
			}

			addRegister(instruction->rD, instruction);
		}

		virtual void  visit(triVM::InstructionBinaryType * instruction){ 
			if (instruction->rC >= 0) {
				addRegister(instruction->rC, instruction);
			}

			addRegister(instruction->rD, instruction);
		}
		
		virtual void  visit(triVM::InstructionCompareType * instruction){ 

			addRegister(instruction->rD, instruction);
		}

		virtual void  visit(triVM::InstructionLoadType * instruction){ 

			addRegister(instruction->rD, instruction);
		}

		virtual void  visit(triVM::InstructionStoreType * instruction){ 
		}

		virtual void  visit(triVM::InstructionLoadImmediateType * instruction){ 
			addRegister(instruction->rD, instruction);
		}

		virtual void  visit(triVM::InstructionBranchRegType * instruction){ 
		}

		virtual void  visit(triVM::InstructionBranchLabelType * instruction){ 
		}

		virtual void  visit(triVM::InstructionBranchConditionallyType * instruction){ 
		}

		virtual void  visit(triVM::InstructionPhiType * instruction) { 

			addRegister(instruction->rD, instruction);
		}

		virtual void  visit(triVM::InstructionCallType * instruction) { 
			// always executed

			if (instruction->rD != -1) {
				addRegister(instruction->rD, instruction);
			}
		}

		virtual void  visit(triVM::InstructionRetType * instruction) { 
		}

		virtual void visit(triVM::InstructionArmUnaryImmediateType * instruction) { 
			if (instruction->rC >= 0) {
				addRegister(instruction->rC, instruction);
			}

			addRegister(instruction->rD, instruction);
		}

		virtual void visit(triVM::InstructionArmUnaryShiftRegType * instruction) { 
			if (instruction->rC >= 0) {
				addRegister(instruction->rC, instruction);
			}

			addRegister(instruction->rD, instruction);
		}

		virtual void visit(triVM::InstructionArmUnaryShiftImmedType * instruction) { 
			if (instruction->rC >= 0) {
				addRegister(instruction->rC, instruction);
			}

			addRegister(instruction->rD, instruction);
		}

		virtual void visit(triVM::InstructionArmBinaryImmediateType * instruction) { 
			if (instruction->rC >= 0) {
				addRegister(instruction->rC, instruction);
			}

			addRegister(instruction->rD, instruction);
		}

		virtual void visit(triVM::InstructionArmBinaryShiftRegType * instruction) { 
			if (instruction->rC >= 0) {
				addRegister(instruction->rC, instruction);
			}

			addRegister(instruction->rD, instruction);
		}

		virtual void visit(triVM::InstructionArmBinaryShiftImmedType * instruction) { 
			if (instruction->rC >= 0) {
				addRegister(instruction->rC, instruction);
			}

			addRegister(instruction->rD, instruction);
		}

		virtual void visit(triVM::InstructionArmLoadImmedOffsetType * instruction) { 
			addRegister(instruction->rD, instruction);
		}

		virtual void visit(triVM::InstructionArmLoadRegOffsetType * instruction) { 
			addRegister(instruction->rD, instruction);
		}

		virtual void visit(triVM::InstructionArmStoreImmedOffsetType * instruction) { 
		}

		virtual void visit(triVM::InstructionArmStoreRegOffsetType * instruction) { 
		}


	private:
		RegisterDefinitionMap * definitions;

	private:
		void addRegister(int reg, Instruction * instruction) {
			(*definitions)[reg] = instruction;
		}
	};

}


RegisterDefinitionMap * CodeGenerator :: FindDefinitions(Module * module) {
	RegisterDefinitionMap * result = new RegisterDefinitionMap;
	FindDefinitionSweep finder(result);
	finder.sweep(module);
	return result;
}
