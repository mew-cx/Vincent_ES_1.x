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

	std::ostream& operator<<(std::ostream& out, RegisterList * list) {
		const char * separator = "";

		for (RegisterList::iterator iter = list->begin(); iter != list->end(); ++iter) {
			out << separator << "r" << *iter;
			separator = ", ";
		}

		return out;
	}

	std::ostream& operator<<(std::ostream& out, Constant * constant) {
		switch (constant->base.kind) {
		case ConstantInt:
			out << constant->intConstant.value;
			break;

		case ConstantFloat:
			out << constant->floatConstant.value;
			break;

		case ConstantString:
			out << "\"" << constant->stringConstant.value << "\"";
			break;

		case ConstantLabel:
			out << constant->labelConstant.value->base.name;
			break;
		}

		return out;
	}
}


class Dump: public Sweep {
public:
	Dump(std::ostream& stream): out(stream) {
	}

protected:
	virtual void begin(triVM::Module * module){ 
		out << "module " << module->name << std::endl;
	}

	virtual void begin(triVM::Procedure * procedure){ 
		out << "proc " << procedure->def->base.name << std::endl;
	}

	virtual void begin(triVM::Block * block){ 
		out << std::endl;

		for (LabelList::iterator iter = block->labels.begin(); iter != block->labels.end(); ++iter) {
			out << (*iter)->base.name << ":" << std::endl;
		}
	}

	virtual void visit(triVM::Instruction * instruction) {
		out << "\t";
		Sweep::visit(instruction);
		out << std::endl;
	}

protected:
	virtual void  visitUnary(triVM::Instruction * instruction){ 
		if (instruction->unary.rC >= 0) {
			out << opcodeName[instruction->base.opcode] << " (r" <<
				instruction->unary.rD << ", r" << instruction->unary.rC << "), r" <<
				instruction->unary.rS;
		} else {
			out << opcodeName[instruction->base.opcode] << " r" <<
				instruction->unary.rD << ", r" << instruction->unary.rS;
		}
	}

	virtual void  visitBinary(triVM::Instruction * instruction){ 
		if (instruction->binary.rC >= 0) {
			out << opcodeName[instruction->base.opcode] << " (r" <<
				instruction->binary.rD << ", r" << instruction->binary.rC << "), r" <<
				instruction->binary.rS << ", r" << instruction->binary.rM;
		} else {
			out << opcodeName[instruction->base.opcode] << " r" <<
				instruction->binary.rD << ", r" << instruction->binary.rS << 
				", r" << instruction->binary.rM;
		}
	}
	
	virtual void  visitCompare(triVM::Instruction * instruction){ 
		out << opcodeName[instruction->base.opcode] << " r" <<
			instruction->compare.rD << ", r" << instruction->compare.rS << ", r" <<
			instruction->compare.rC;
	}

	virtual void  visitLoad(triVM::Instruction * instruction){ 
		out << opcodeName[instruction->base.opcode] << " r" <<
			instruction->load.rD << ", [r" << instruction->load.rS << "]";
	}

	virtual void  visitStore(triVM::Instruction * instruction){ 
		out << opcodeName[instruction->base.opcode] << " [r" <<
			instruction->store.rD << "], r" << instruction->store.rS;
	}

	virtual void  visitLoadImmediate(triVM::Instruction * instruction){ 
		out << opcodeName[instruction->base.opcode] << " r" <<
			instruction->loadImmediate.rD << ", " << instruction->loadImmediate.constant;
	}

	virtual void  visitBranchReg(triVM::Instruction * instruction){ 
		out << opcodeName[instruction->base.opcode] << " [r" <<
			instruction->branchReg.rS << "]";
	}

	virtual void  visitBranchLabel(triVM::Instruction * instruction){ 
		out << opcodeName[instruction->base.opcode] << " " <<
			instruction->branchLabel.label->base.name;
	}

	virtual void  visitBranchConditionally(triVM::Instruction * instruction){ 
		out << opcodeName[instruction->base.opcode] << " r" <<
			instruction->branchConditionally.rS << ", " << 
			instruction->branchConditionally.label->base.name;
	}

	virtual void  visitPhi(triVM::Instruction * instruction){ 
		out << "phi r" << instruction->phi.rD << ", " <<
			instruction->phi.registers;
	}

	virtual void  visitCall(triVM::Instruction * instruction){ 

		out << "call [r" << instruction->call.rS << "], (" <<
			instruction->call.args << ")";
		
		if (instruction->call.results != 0) {
			out << ", " << instruction->call.results;
		}
	}

	virtual void  visitRet(triVM::Instruction * instruction){ 

		if (instruction->ret.registers != 0) {
			out << "ret " << instruction->ret.registers;
		} else {
			out << "ret";
		}
	}

private:
	std::ostream& out;
};


std::ostream& operator<<(std::ostream& out, EGL::triVM::Module& module) {
	Dump dump(out);
	dump.sweep(&module);
	return out;
}
