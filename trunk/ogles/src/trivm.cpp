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
#include "fixed.h"
#include "trivm.h"


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
		visit(*iter);
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
		visit(*iter);
	}

	end(block);
}

void Sweep :: begin(triVM::Module * module){ }
void Sweep :: begin(triVM::Procedure * procedure){ }
void Sweep :: begin(triVM::Block * block){ }

void Sweep :: visit(triVM::Instruction * instruction){ 
	switch (instruction->base.kind) {
	case InstructionUnary:
		visitUnary(instruction);
		break;

	case InstructionBinary:
		visitBinary(instruction);
		break;

	case InstructionCompare:
		visitCompare(instruction);
		break;

	case InstructionLoad:
		visitLoad(instruction);
		break;

	case InstructionStore:
		visitStore(instruction);
		break;

	case InstructionLoadImmediate:
		visitLoadImmediate(instruction);
		break;

	case InstructionBranchReg:
		visitBranchReg(instruction);
		break;

	case InstructionBranchLabel:
		visitBranchLabel(instruction);
		break;

	case InstructionBranchConditionally:
		visitBranchConditionally(instruction);
		break;

	case InstructionPhi:
		visitPhi(instruction);
		break;

	case InstructionCall:
		visitCall(instruction);
		break;

	case InstructionRet:
		visitRet(instruction);
		break;
	}
}

void Sweep :: end(triVM::Module * module){ }
void Sweep :: end(triVM::Procedure * procedure){ }
void Sweep :: end(triVM::Block * block){ }

void Sweep :: visitUnary(triVM::Instruction * instruction){ }
void Sweep :: visitBinary(triVM::Instruction * instruction){ }
void Sweep :: visitCompare(triVM::Instruction * instruction){ }
void Sweep :: visitLoad(triVM::Instruction * instruction){ }
void Sweep :: visitStore(triVM::Instruction * instruction){ }
void Sweep :: visitLoadImmediate(triVM::Instruction * instruction){ }
void Sweep :: visitBranchReg(triVM::Instruction * instruction){ }
void Sweep :: visitBranchLabel(triVM::Instruction * instruction){ }
void Sweep :: visitBranchConditionally(triVM::Instruction * instruction){ }
void Sweep :: visitPhi(triVM::Instruction * instruction){ }
void Sweep :: visitCall(triVM::Instruction * instruction){ }
void Sweep :: visitRet(triVM::Instruction * instruction){ }


namespace {

	const char* opcodeName[] = {
		"nop",			// no operation

		// computational (integer)
		"add",
		"and",
		"asr",
		"cmp",
		"div",
		"lsl",
		"lsr",
		"mod",
		"mul",
		"neg",
		"not",
		"or",
		"sub",
		"udiv",
		"umod",
		"xor",

		// computational (float)
		"fadd",
		"fcmp",
		"fdiv",
		"fmul",
		"fneg",
		"fsub",

		"finv",			// inverse
		"fsqrt",			// sqrt

		// conversion (int <-> float)
		"trunc",			// float -> int by truncation
		"round",			// float -> int by rounding
		"fcnv",			// int -> float

		// branch
		"bae",
		"bbl",
		"beq",
		"bge",
		"ble",			// less equal than added
		"bgt",			// greater than added
		"blt",
		"bne",
		"bra",

		// load/store
		"ldb",
		"ldh",
		"ldi",
		"ldw",

		"stb",
		"sth",
		"stw",

		// load/store volatile
		"vldb",
		"vldh",
		"vldw",

		"vstb",
		"vsth",
		"vstw",

		// special
		"call",
		"ret",
		"phi"
	};

	void DumpRegisterList(FILE * out, RegisterList * list) {
		const char * separator = "";

		for (RegisterList::iterator iter = list->begin(); iter != list->end(); ++iter) {
			fprintf(out, "%sr%d", separator, *iter);
			separator = ", ";
		}
	}

	void DumpConstant(FILE * out, Constant * constant) {
		switch (constant->base.kind) {
		case ConstantInt:
			fprintf(out, "%d", constant->intConstant.value);
			break;

		case ConstantFixed:
			fprintf(out, "%f", (float) EGL_FloatFromFixed(constant->fixedConstant.value));
			break;

		case ConstantString:
			fprintf(out,  "\"%s\"", constant->stringConstant.value);
			break;

		case ConstantLabel:
			fprintf(out, "%s", constant->labelConstant.value->base.name);
			break;
		}

	}
}


class Dump: public Sweep {
public:
	Dump(FILE * stream): out(stream) {
	}

protected:
	virtual void begin(triVM::Module * module){ 
		fprintf(out, "module %s\n", module->name);
	}

	virtual void begin(triVM::Procedure * procedure){ 
		fprintf(out, "proc %s\n", procedure->def->base.name);
	}

	virtual void begin(triVM::Block * block){ 
		fprintf(out, "\n");

		for (LabelList::iterator iter = block->labels.begin(); iter != block->labels.end(); ++iter) {
			fprintf(out, "%s:\n", (*iter)->base.name);
		}
	}

	virtual void visit(triVM::Instruction * instruction) {
		fprintf(out, "\t");
		Sweep::visit(instruction);

#ifndef NDEBUG
		if (instruction->base.comment != 0 && *instruction->base.comment != '\0') {
			fprintf(out, "\t\t\t\t; %s", instruction->base.comment);
		}
#endif

		fprintf(out, "\n");
	}

protected:
	virtual void  visitUnary(triVM::Instruction * instruction){ 
		if (instruction->unary.rC >= 0) {
			fprintf(out, "%s (r%d, r%d), r%d",
				opcodeName[instruction->base.opcode],
				instruction->unary.rD,
				instruction->unary.rC,
				instruction->unary.rS);
		} else {
			fprintf(out, "%s r%d, r%d",
				opcodeName[instruction->base.opcode],
				instruction->unary.rD,
				instruction->unary.rS);
		}
	}

	virtual void  visitBinary(triVM::Instruction * instruction) { 
		if (instruction->binary.rC >= 0) {
			fprintf(out, "%s (r%d, r%d), r%d, r%d",
				opcodeName[instruction->base.opcode],
				instruction->binary.rD,
				instruction->binary.rC,
				instruction->binary.rS,
				instruction->binary.rM);
		} else {
			fprintf(out, "%s r%d, r%d, r%d",
				opcodeName[instruction->base.opcode],
				instruction->binary.rD,
				instruction->binary.rS,
				instruction->binary.rM);
		}
	}
	
	virtual void  visitCompare(triVM::Instruction * instruction) {
		fprintf(out, "%s  r%d, r%d, r%d",
			opcodeName[instruction->base.opcode],
			instruction->compare.rD,
			instruction->compare.rS,
			instruction->compare.rC);
	}

	virtual void  visitLoad(triVM::Instruction * instruction) { 
		fprintf(out, "%s r%d, [r%d]",
			opcodeName[instruction->base.opcode],
			instruction->load.rD,
			instruction->load.rS);
	}

	virtual void  visitStore(triVM::Instruction * instruction) { 
		fprintf(out, "%s [r%d], %d",
			opcodeName[instruction->base.opcode],
			instruction->store.rD,
			instruction->store.rS);
	}

	virtual void  visitLoadImmediate(triVM::Instruction * instruction) { 
		fprintf(out, "%s r%d, ",
			opcodeName[instruction->base.opcode],
			instruction->loadImmediate.rD);
		DumpConstant(out, instruction->loadImmediate.constant);
	}

	virtual void  visitBranchReg(triVM::Instruction * instruction) { 
		fprintf(out, "%s [r%d]", 
			opcodeName[instruction->base.opcode],
			instruction->branchReg.rS);
	}

	virtual void  visitBranchLabel(triVM::Instruction * instruction) { 
		fprintf(out, "%s %s",
			opcodeName[instruction->base.opcode],
			instruction->branchLabel.label->base.name);
	}

	virtual void  visitBranchConditionally(triVM::Instruction * instruction) { 
		fprintf(out, "%s r%d, %s",
			opcodeName[instruction->base.opcode],
			instruction->branchConditionally.rS,
			instruction->branchConditionally.label->base.name);
	}

	virtual void  visitPhi(triVM::Instruction * instruction) { 
		fprintf(out, "phi r%d, ", instruction->phi.rD);
		DumpRegisterList(out, instruction->phi.registers);
	}

	virtual void  visitCall(triVM::Instruction * instruction) { 

		fprintf(out, "call [r%d], (", instruction->call.rS);
		DumpRegisterList(out, instruction->call.args);
		fprintf(out, ")");
		
		if (instruction->call.results != 0) {
			fprintf(out, ", ");
			DumpRegisterList(out, instruction->call.results);
		}
	}

	virtual void  visitRet(triVM::Instruction * instruction) { 

		if (instruction->ret.registers != 0) {
			fprintf(out, "ret ");
			DumpRegisterList(out, instruction->ret.registers);
		} else {
			fprintf(out, "ret");
		}
	}

private:
	FILE * out;
};


void EGL::triVM::DumpModule(FILE * out, EGL::triVM::Module * module) {
	Dump dump(out);
	dump.sweep(module);
}


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
		virtual void  visitUnary(triVM::Instruction * instruction){ 
			if (instruction->unary.rC >= 0) {
				if (isUsed(instruction->unary.rD) || isUsed(instruction->unary.rC)) {
					instruction->base.used = true;
					addRegister(instruction->unary.rS);
				} else {
					instruction->base.used = false;
				}
			} else {
				if (isUsed(instruction->unary.rD)) {
					instruction->base.used = true;
					addRegister(instruction->unary.rS);
				} else {
					instruction->base.used = false;
				}
			}
		}

		virtual void  visitBinary(triVM::Instruction * instruction){ 
			if (instruction->binary.rC >= 0) {
				if (isUsed(instruction->binary.rD) || isUsed(instruction->binary.rC)) {
					instruction->base.used = true;
					addRegister(instruction->binary.rS);
					addRegister(instruction->binary.rM);
				} else {
					instruction->base.used = false;
				}
			} else {
				if (isUsed(instruction->binary.rD)) {
					instruction->base.used = true;
					addRegister(instruction->binary.rS);
					addRegister(instruction->binary.rM);
				} else {
					instruction->base.used = false;
				}
			}
		}
		
		virtual void  visitCompare(triVM::Instruction * instruction){ 

			if (isUsed(instruction->compare.rD)) {
				instruction->base.used = true;
				addRegister(instruction->compare.rS);
				addRegister(instruction->compare.rC);
			} else {
				instruction->base.used = false;
			}
		}

		virtual void  visitLoad(triVM::Instruction * instruction){ 

			if (isUsed(instruction->load.rD)) {
				instruction->base.used = true;
				addRegister(instruction->load.rS);
			} else {
				instruction->base.used = false;
			}
		}

		virtual void  visitStore(triVM::Instruction * instruction){ 
			// always executed

			instruction->base.used = true;
			addRegister(instruction->store.rS);
			addRegister(instruction->store.rD);
		}

		virtual void  visitLoadImmediate(triVM::Instruction * instruction){ 
			if (isUsed(instruction->loadImmediate.rD)) {
				instruction->base.used = true;
			}
		}

		virtual void  visitBranchReg(triVM::Instruction * instruction){ 
			// always executed

			instruction->base.used = true;
			addRegister(instruction->branchReg.rS);
		}

		virtual void  visitBranchLabel(triVM::Instruction * instruction){ 
			// always executed
			instruction->base.used = true;
		}

		virtual void  visitBranchConditionally(triVM::Instruction * instruction){ 
			// always executed

			instruction->base.used = true;
			addRegister(instruction->branchConditionally.rS);
		}

		virtual void  visitPhi(triVM::Instruction * instruction) { 

			if (isUsed(instruction->phi.rD)) {
				instruction->base.used = true;
				RegisterList * registers = instruction->phi.registers;

				for (RegisterList::iterator iter = registers->begin(); iter != registers->end(); ++iter) {
					addRegister(*iter);
				}
			} else {
				instruction->base.used = false;
			}
		}

		virtual void  visitCall(triVM::Instruction * instruction) { 
			// always executed

			instruction->base.used = true;
			RegisterList * registers = instruction->call.args;

			for (RegisterList::iterator iter = registers->begin(); iter != registers->end(); ++iter) {
				addRegister(*iter);
			}
		}

		virtual void  visitRet(triVM::Instruction * instruction) { 
			// always executed

			instruction->base.used = true;
			RegisterList * registers = instruction->ret.registers;

			for (RegisterList::iterator iter = registers->begin(); iter != registers->end(); ++iter) {
				addRegister(*iter);
			}
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
void EGL::triVM::RemoveUnusedCode(Module * module) {
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

				if (instruction->base.used) {
					++instIter;
				} else {
					instructions.erase(instIter);
				}
			}
		}
	}
}


namespace {
	class FindDefinitionSweep: public Sweep {
	public:
		FindDefinitionSweep(RegisterDefinitionMap * result): definitions(result) {
		}

	protected:
		virtual void  visitUnary(triVM::Instruction * instruction){ 
			if (instruction->unary.rC >= 0) {
				addRegister(instruction->unary.rC, instruction);
			}

			addRegister(instruction->unary.rD, instruction);
		}

		virtual void  visitBinary(triVM::Instruction * instruction){ 
			if (instruction->binary.rC >= 0) {
				addRegister(instruction->binary.rC, instruction);
			}

			addRegister(instruction->binary.rD, instruction);
		}
		
		virtual void  visitCompare(triVM::Instruction * instruction){ 

			addRegister(instruction->compare.rD, instruction);
		}

		virtual void  visitLoad(triVM::Instruction * instruction){ 

			addRegister(instruction->load.rD, instruction);
		}

		virtual void  visitStore(triVM::Instruction * instruction){ 
		}

		virtual void  visitLoadImmediate(triVM::Instruction * instruction){ 
			addRegister(instruction->loadImmediate.rD, instruction);
		}

		virtual void  visitBranchReg(triVM::Instruction * instruction){ 
		}

		virtual void  visitBranchLabel(triVM::Instruction * instruction){ 
		}

		virtual void  visitBranchConditionally(triVM::Instruction * instruction){ 
		}

		virtual void  visitPhi(triVM::Instruction * instruction) { 

			addRegister(instruction->phi.rD, instruction);
		}

		virtual void  visitCall(triVM::Instruction * instruction) { 
			// always executed

			RegisterList * registers = instruction->call.results;

			for (RegisterList::iterator iter = registers->begin(); iter != registers->end(); ++iter) {
				addRegister(*iter, instruction);
			}
		}

		virtual void  visitRet(triVM::Instruction * instruction) { 
		}

	private:
		RegisterDefinitionMap * definitions;

	private:
		void addRegister(int reg, Instruction * instruction) {
			(*definitions)[reg] = instruction;
		}
	};

}


RegisterDefinitionMap * EGL::triVM::FindDefinitions(Module * module) {
	RegisterDefinitionMap * result = new RegisterDefinitionMap;
	FindDefinitionSweep finder(result);
	finder.sweep(module);
	return result;
}
