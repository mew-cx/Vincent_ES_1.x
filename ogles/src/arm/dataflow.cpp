// ==========================================================================
//
// triVMDataFlow	triVM Intermediate Language for OpenGL (R) ES Implementation
//					Data flow analysis
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
#include "Transform.h"


using namespace EGL;
using namespace triVM;


namespace {

	Label * GetBranchTarget(Instruction * inst) {
		switch (inst->kind) {
			case InstructionBranchConditionally:
				return static_cast<InstructionBranchConditionallyType *>(inst)->label;

			case InstructionBranchLabel:
				return static_cast<InstructionBranchLabelType *>(inst)->label;

			default:
				return 0;
		}
	}

	bool IsBranchAlways(Instruction * inst) {
		return inst->kind == InstructionBranchLabel;
	}

	void AddControlFlowEdge(Block * source, Block * target) {

		if (std::find(source->successors.begin(), source->successors.end(), target) == source->successors.end()) {
			source->successors.push_back(target);
		}

		if (std::find(target->predecessors.begin(), target->predecessors.end(), source) == target->predecessors.end()) {
			target->predecessors.push_back(source);
		}
	}

	void DetermineBlockConnectivity(Module * module) {
		// for each branch, add target as successor block
		// whenever adding a successor block, also add backwards edge for predecessor
		// whenever last instruction of a block is not "branch always" add next block as successor

		for (ProcedureList::iterator procIter = module->procedures.begin();
			procIter != module->procedures.end(); ++procIter) {

			Procedure * procedure = *procIter;

			for (BlockList::iterator blockIter = procedure->blocks.begin();
				blockIter != procedure->blocks.end();) {

				Block * block = *blockIter;
				InstructionList & instructions = block->instructions;
				bool endsWithBranchAlways = false;

				for (InstructionList::iterator instIter = instructions.begin();
					instIter != instructions.end(); ++instIter) {
					Instruction * instruction = *instIter;

					Label * branchTarget = GetBranchTarget(instruction);

					if (branchTarget != 0 && branchTarget->base.kind == LabelBlock) {
						Block * targetBlock = branchTarget->block.block;
						AddControlFlowEdge(block, targetBlock);
					}

					endsWithBranchAlways = IsBranchAlways(instruction);
				}

				 ++blockIter;

				if (!endsWithBranchAlways && blockIter != procedure->blocks.end()) {
					Block * targetBlock = *blockIter;
					AddControlFlowEdge(block, targetBlock);
				}
			}
		}
	}

	class FindUseSet: public Sweep {
	public:

	protected:
		virtual void begin(triVM::Procedure * procedure) {			
			Sweep::begin(procedure);

		}

		virtual void begin(triVM::Block * block) {			
			currentBlock = block;
			block->use.clear();
			block->use.resize(block->procedure->module->registerCount, false);
		}

		virtual void end(triVM::Block * block) {
			currentBlock = 0;

			for (size_t reg = 0; reg < block->use.size(); ++reg) {
				block->use[reg] = block->use[reg] & !block->def[reg];
			}
		}

		virtual void  visit(triVM::InstructionUnaryType * instruction){ 
			addRegister(instruction->rS);
		}

		virtual void  visit(triVM::InstructionBinaryType * instruction){ 
			addRegister(instruction->rS);
			addRegister(instruction->rM);
		}
		
		virtual void  visit(triVM::InstructionCompareType * instruction){ 

			addRegister(instruction->rS);
			addRegister(instruction->rC);
		}

		virtual void  visit(triVM::InstructionLoadType * instruction){ 

			addRegister(instruction->rS);
		}

		virtual void  visit(triVM::InstructionStoreType * instruction){ 
			// always executed

			addRegister(instruction->rS);
			addRegister(instruction->rD);
		}

		virtual void  visit(triVM::InstructionLoadImmediateType * instruction){ 
		}

		virtual void  visit(triVM::InstructionBranchRegType * instruction){ 
			// always executed

			addRegister(instruction->rS);
		}

		virtual void  visit(triVM::InstructionBranchLabelType * instruction){ 
			// always executed
		}

		virtual void  visit(triVM::InstructionBranchConditionallyType * instruction){ 
			// always executed

			addRegister(instruction->rS);
		}

		virtual void  visit(triVM::InstructionPhiType * instruction) { 

			RegisterList * registers = instruction->registers;

			for (RegisterList::iterator iter = registers->begin(); iter != registers->end(); ++iter) {
				addRegister(*iter);
			}
		}

		virtual void  visit(triVM::InstructionCallType * instruction) { 
			// always executed

			RegisterList * registers = instruction->args;

			for (RegisterList::iterator iter = registers->begin(); iter != registers->end(); ++iter) {
				addRegister(*iter);
			}
		}

		virtual void  visit(triVM::InstructionRetType * instruction) { 
			// always executed

			if (instruction->rS != -1) {
				addRegister(instruction->rS);
			}
		}

		virtual void visit(triVM::InstructionArmUnaryImmediateType * instruction) { 
		}

		virtual void visit(triVM::InstructionArmUnaryShiftRegType * instruction) { 
			addRegister(instruction->rS);
			addRegister(instruction->rShift);
		}

		virtual void visit(triVM::InstructionArmUnaryShiftImmedType * instruction) { 
			addRegister(instruction->rS);
		}

		virtual void visit(triVM::InstructionArmBinaryImmediateType * instruction) { 
			addRegister(instruction->rS);
		}

		virtual void visit(triVM::InstructionArmBinaryShiftRegType * instruction) { 
			addRegister(instruction->rS);
			addRegister(instruction->rM);
			addRegister(instruction->rShift);
		}

		virtual void visit(triVM::InstructionArmBinaryShiftImmedType * instruction) { 
			addRegister(instruction->rS);
			addRegister(instruction->rM);
		}

		virtual void visit(triVM::InstructionArmLoadImmedOffsetType * instruction) { 
			addRegister(instruction->rS);
		}

		virtual void visit(triVM::InstructionArmLoadRegOffsetType * instruction) { 
			addRegister(instruction->rS);
			addRegister(instruction->rOffset);
		}

		virtual void visit(triVM::InstructionArmStoreImmedOffsetType * instruction) { 
			// always executed

			addRegister(instruction->rS);
			addRegister(instruction->rD);
		}

		virtual void visit(triVM::InstructionArmStoreRegOffsetType * instruction) { 
			// always executed

			addRegister(instruction->rS);
			addRegister(instruction->rD);
			addRegister(instruction->rOffset);
		}

	private:
		void addRegister(int reg) {
			currentBlock->use[reg] = true;
		}

		Block * currentBlock;
	};

	class FindDefSet: public Sweep {
	public:

	protected:
		virtual void begin(triVM::Block * block) {			
			block->def.clear();
			block->def.resize(block->procedure->module->registerCount, false);
		}
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
		void addRegister(int reg, Instruction * instruction) {
			instruction->block->def[reg] = true;
		}
	};

	void DetermineDefUseSets(Module * module) {
		// for each block:
		// initialize def & use sets to number of variables
		// iterate over instructions adding def & use entries as in dead-code elimination
		// set use := use \ def

		FindDefSet findDef;
		findDef.sweep(module);

		FindUseSet findUse;
		findUse.sweep(module);

	}

	void InitLifeSets(Procedure * procedure) {
		for (BlockList::iterator blockIter = procedure->blocks.begin();
			blockIter != procedure->blocks.end(); ++blockIter) {

			Block * block = *blockIter;

			block->liveIn = block->use;
			block->liveOut.clear();
			block->liveOut.resize(block->liveIn.size());
		}
	}

	bool Union(RegisterSet& result, const RegisterSet& arg) {
		bool changed = false;

		for (size_t reg = 0; reg < result.size(); ++reg) {
			if (arg[reg] & !result[reg]) {
				changed = true;
				result[reg] = true;
			}
		}

		return changed;
	}

	bool UnionMinus(RegisterSet& result, const RegisterSet& arg1, const RegisterSet& arg2) {

		bool changed = false;

		for (size_t reg = 0; reg < result.size(); ++reg) {
			bool newValue = arg1[reg] & !arg2[reg];

			if (!result[reg] & newValue) {
				result[reg] = newValue;
				changed = true;
			}
		}

		return changed;
	}

	void DetermineLifeSets(Procedure * procedure) {
		// initialize lifeIn with use
		InitLifeSets(procedure);

		bool changed = false;

		do {
			changed = false;

			for (BlockList::iterator blockIter = procedure->blocks.begin();
				blockIter != procedure->blocks.end(); ++blockIter) {

				Block * block = *blockIter;

				//	liveOut(b) := UNION liveIn(c), c being successor block
				for (BlockList::iterator succIter = block->successors.begin();
					succIter != block->successors.end(); ++succIter) {
				
					Block * successor = *succIter;
					changed |= Union(block->liveOut, successor->liveIn);
				}

				//	lifeIn(b) := lifeOut(b) \ def(b)
				changed |= UnionMinus(block->liveIn, block->liveOut, block->def);

			}
		} while (changed);
	}

	void DetermineLifeSets(Module * module) {
		for (ProcedureList::iterator procIter = module->procedures.begin();
			procIter != module->procedures.end(); ++procIter) {

			Procedure * procedure = *procIter;
			DetermineLifeSets(procedure);
		}
	}
}


void CodeGenerator :: PerformDataFlowAnalysis(Module * module) {
	DetermineBlockConnectivity(module);
	DetermineDefUseSets(module);
	DetermineLifeSets(module);
}