// ==========================================================================
//
// triVMDump	triVM Intermediate Language for OpenGL (R) ES Implementation
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
#include "Inst.h"
#include "Sweep.h"


using namespace EGL;
using namespace triVM;


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

		virtual void dispatch(triVM::Instruction * instruction) {
			fprintf(out, "\t");
			Sweep::dispatch(instruction);

	#ifndef NDEBUG
			if (instruction->comment != 0 && *instruction->comment != '\0') {
				fprintf(out, "\t\t\t\t; %s", instruction->comment);
			}
	#endif

			fprintf(out, "\n");
		}

	protected:
		virtual void  visit(triVM::InstructionUnaryType * instruction){ 
			if (instruction->rC >= 0) {
				fprintf(out, "%s (r%d, r%d), r%d",
					opcodeName[instruction->opcode],
					instruction->rD,
					instruction->rC,
					instruction->rS);
			} else {
				fprintf(out, "%s r%d, r%d",
					opcodeName[instruction->opcode],
					instruction->rD,
					instruction->rS);
			}
		}

		virtual void  visit(triVM::InstructionBinaryType * instruction) { 
			if (instruction->rC >= 0) {
				fprintf(out, "%s (r%d, r%d), r%d, r%d",
					opcodeName[instruction->opcode],
					instruction->rD,
					instruction->rC,
					instruction->rS,
					instruction->rM);
			} else {
				fprintf(out, "%s r%d, r%d, r%d",
					opcodeName[instruction->opcode],
					instruction->rD,
					instruction->rS,
					instruction->rM);
			}
		}
		
		virtual void  visit(triVM::InstructionCompareType * instruction) {
			fprintf(out, "%s  r%d, r%d, r%d",
				opcodeName[instruction->opcode],
				instruction->rD,
				instruction->rS,
				instruction->rC);
		}

		virtual void  visit(triVM::InstructionLoadType * instruction) { 
			fprintf(out, "%s r%d, [r%d]",
				opcodeName[instruction->opcode],
				instruction->rD,
				instruction->rS);
		}

		virtual void  visit(triVM::InstructionStoreType * instruction) { 
			fprintf(out, "%s [r%d], %d",
				opcodeName[instruction->opcode],
				instruction->rD,
				instruction->rS);
		}

		virtual void  visit(triVM::InstructionLoadImmediateType * instruction) { 
			fprintf(out, "%s r%d, ",
				opcodeName[instruction->opcode],
				instruction->rD);
			DumpConstant(out, instruction->constant);
		}

		virtual void  visit(triVM::InstructionBranchRegType * instruction) { 
			fprintf(out, "%s [r%d]", 
				opcodeName[instruction->opcode],
				instruction->rS);
		}

		virtual void  visit(triVM::InstructionBranchLabelType * instruction) { 
			fprintf(out, "%s %s",
				opcodeName[instruction->opcode],
				instruction->label->base.name);
		}

		virtual void  visit(triVM::InstructionBranchConditionallyType * instruction) { 
			fprintf(out, "%s r%d, %s",
				opcodeName[instruction->opcode],
				instruction->rS,
				instruction->label->base.name);
		}

		virtual void  visit(triVM::InstructionPhiType * instruction) { 
			fprintf(out, "phi r%d, ", instruction->rD);
			DumpRegisterList(out, instruction->registers);
		}

		virtual void  visit(triVM::InstructionCallType * instruction) { 

			fprintf(out, "call [r%d], (", instruction->rS);
			DumpRegisterList(out, instruction->args);
			fprintf(out, ")");
			
			if (instruction->results != 0) {
				fprintf(out, ", ");
				DumpRegisterList(out, instruction->results);
			}
		}

		virtual void  visit(triVM::InstructionRetType * instruction) { 

			if (instruction->registers != 0) {
				fprintf(out, "ret ");
				DumpRegisterList(out, instruction->registers);
			} else {
				fprintf(out, "ret");
			}
		}

		virtual void visit(triVM::InstructionArmUnaryImmediateType * instruction) { 
			if (instruction->rC >= 0) {
				fprintf(out, "%s (r%d, r%d), %d",
					opcodeName[instruction->opcode],
					instruction->rD,
					instruction->rC,
					instruction->immed);
			} else {
				fprintf(out, "%s r%d, %d",
					opcodeName[instruction->opcode],
					instruction->rD,
					instruction->immed);
			}
		}

		virtual void visit(triVM::InstructionArmUnaryShiftRegType * instruction) { 
			if (instruction->rC >= 0) {
				fprintf(out, "%s (r%d, r%d), r%d %s r%d",
					opcodeName[instruction->opcode],
					instruction->rD,
					instruction->rC,
					instruction->rS,
					opcodeName[instruction->opShift],
					instruction->rShift);
			} else {
				fprintf(out, "%s r%d, r%d %s r%d",
					opcodeName[instruction->opcode],
					instruction->rD,
					instruction->rS,
					opcodeName[instruction->opShift],
					instruction->rShift);
			}
		}

		virtual void visit(triVM::InstructionArmUnaryShiftImmedType * instruction) { 
			if (instruction->rC >= 0) {
				fprintf(out, "%s (r%d, r%d), r%d %s %d",
					opcodeName[instruction->opcode],
					instruction->rD,
					instruction->rC,
					instruction->rS,
					opcodeName[instruction->opShift],
					instruction->shiftBits);
			} else {
				fprintf(out, "%s r%d, r%d %s %d",
					opcodeName[instruction->opcode],
					instruction->rD,
					instruction->rS,
					opcodeName[instruction->opShift],
					instruction->shiftBits);
			}
		}

		virtual void visit(triVM::InstructionArmBinaryImmediateType * instruction) { 
			if (instruction->rC >= 0) {
				fprintf(out, "%s (r%d, r%d), r%d, %d",
					opcodeName[instruction->opcode],
					instruction->rD,
					instruction->rC,
					instruction->rS,
					instruction->immed);
			} else {
				fprintf(out, "%s r%d, r%d, r%d",
					opcodeName[instruction->opcode],
					instruction->rD,
					instruction->rS,
					instruction->immed);
			}
		}

		virtual void visit(triVM::InstructionArmBinaryShiftRegType * instruction) { 
			if (instruction->rC >= 0) {
				fprintf(out, "%s (r%d, r%d), r%d, r%d %s r%d",
					opcodeName[instruction->opcode],
					instruction->rD,
					instruction->rC,
					instruction->rS,
					instruction->rM,
					opcodeName[instruction->opShift],
					instruction->rShift);
			} else {
				fprintf(out, "%s r%d, r%d, r%d %s r%d",
					opcodeName[instruction->opcode],
					instruction->rD,
					instruction->rS,
					instruction->rM,
					opcodeName[instruction->opShift],
					instruction->rShift);
			}
		}

		virtual void visit(triVM::InstructionArmBinaryShiftImmedType * instruction) { 
			if (instruction->rC >= 0) {
				fprintf(out, "%s (r%d, r%d), r%d, r%d %s %d",
					opcodeName[instruction->opcode],
					instruction->rD,
					instruction->rC,
					instruction->rS,
					instruction->rM,
					opcodeName[instruction->opShift],
					instruction->shiftBits);
			} else {
				fprintf(out, "%s r%d, r%d, r%d %s %d",
					opcodeName[instruction->opcode],
					instruction->rD,
					instruction->rS,
					instruction->rM,
					opcodeName[instruction->opShift],
					instruction->shiftBits);
			}
		}

		virtual void visit(triVM::InstructionArmLoadImmedOffsetType * instruction) { 
			fprintf(out, "%s r%d, %d[r%d]",
				opcodeName[instruction->opcode],
				instruction->rD,
				instruction->immed12,
				instruction->rS);
		}

		virtual void visit(triVM::InstructionArmLoadRegOffsetType * instruction) { 
			fprintf(out, "%s r%d, [r%d, r%d]",
				opcodeName[instruction->opcode],
				instruction->rD,
				instruction->rS,
				instruction->rOffset);
		}

		virtual void visit(triVM::InstructionArmLoadRegImmedOffsetType * instruction) { 
			fprintf(out, "%s r%d, %d[r%d, r%d]",
				opcodeName[instruction->opcode],
				instruction->rD,
				instruction->immed8,
				instruction->rS,
				instruction->rOffset);
		}

		virtual void visit(triVM::InstructionArmStoreImmedOffsetType * instruction) { 
			fprintf(out, "%s %d[r%d], %d",
				opcodeName[instruction->opcode],
				instruction->immed12,
				instruction->rD,
				instruction->rS);
		}

		virtual void visit(triVM::InstructionArmStoreRegOffsetType * instruction) { 
			fprintf(out, "%s [r%d, r%d], %d",
				opcodeName[instruction->opcode],
				instruction->rD,
				instruction->rOffset,
				instruction->rS);
		}

		virtual void visit(triVM::InstructionArmStoreRegImmedOffsetType * instruction) { 
			fprintf(out, "%s %d[r%d, r%d], %d",
				opcodeName[instruction->opcode],
				instruction->immed8,
				instruction->rD,
				instruction->rOffset,
				instruction->rS);
		}	

	private:
		FILE * out;
	};
}



void EGL::triVM::DumpModule(FILE * out, EGL::triVM::Module * module) {
	Dump dump(out);
	dump.sweep(module);
}


