#ifndef EGL_TRIVM_H
#define EGL_TRIVM_H 1

#pragma once

// ==========================================================================
//
// triVM.h		triVM Intermediate Language for OpenGL (R) ES Implementation
//				
//				This class is part of the runtime compiler infrastructure
//				used by the OpenGL|ES implementation for compiling
//				shader code at runtime into machine language.
//
// --------------------------------------------------------------------------
//
// 11-21-2003	Hans-Martin Will	initial version
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


#include <list>
#include <map>
#include <set>
#include <vector>


namespace EGL {
namespace triVM {

	enum Opcode {

		nop,			// no operation

		// computational (integer)
		add,
		and,
		asr,
		cmp,
		div,
		lsl,
		lsr,
		mod,
		mul,
		neg,
		not,
		or,
		sub,
		udiv,
		umod,
		xor,

		// computational (float)
		fadd,
		fcmp,
		fdiv,
		fmul,
		fneg,
		fsub,

		finv,			// inverse
		fsqrt,			// sqrt

		// conversion (int <-> float)
		trunc,			// float -> int by truncation
		round,			// float -> int by rounding
		fcnv,			// int -> float

		// branch
		bae,
		bbl,
		beq,
		bge,
		ble,			// less equal than added
		bgt,			// greater than added
		blt,
		bne,
		bra,

		// load/store
		ldb,
		ldh,
		ldi,
		ldw,

		stb,
		sth,
		stw,

		// load/store volatile
		vldb,
		vldh,
		vldw,

		vstb,
		vsth,
		vstw,

		// special
		call,
		ret,
		phi
	};

	union Label;
	struct Instruction;
	struct Block;
	struct Procedure;
	struct Module;

	enum LabelKind {
		LabelUndefined,
		LabelProcedure,
		LabelBlock,
		LabelGlobalConst,
		LabelGlobalData,
		LabelStaticConst,
		LabelStaticData,
		LabelLocalConst,
		LabelLocalData
	};

	struct LabelBaseType {
		enum LabelKind		kind;
		const char *		name;
	};

	struct LabelProcedureType {
		LabelBaseType	base;
		Procedure *		procedure;
	};

	struct LabelBlockType {
		LabelBaseType	base;
		Block *			block;
	};

	struct LabelConstType {
		LabelBaseType	base;
	};

	struct LabelDataType {
		LabelBaseType	base;
	};

	union Label {
		LabelBaseType		base;
		LabelProcedureType	procedure;
		LabelBlockType		block;
		LabelConstType		constant;
		LabelDataType		data;

		static Label * create(const char * aName, LabelKind aKind) {
			Label * result = new Label();
			result->base.name = aName;
			result->base.kind = aKind;
			return result;
		}
	};

	typedef std::vector<Label *> LabelList;

	enum ConstantKind {
		ConstantInt,
		ConstantFixed,
		ConstantString,
		ConstantLabel
	};

	struct ConstantBaseType {
		ConstantKind kind;
	};

	struct ConstantIntType {
		ConstantBaseType base;
		int				 value;
	};

	struct ConstantFixedType {
		ConstantBaseType base;
		int				 value;
	};

	struct ConstantStringType {
		ConstantBaseType base;
		const char *	 value;
	};

	struct ConstantLabelType {
		ConstantBaseType base;
		Label *			 value;
	};

	union Constant {
		ConstantBaseType	base;
		ConstantIntType		intConstant;
		ConstantFixedType	fixedConstant;
		ConstantStringType	stringConstant;
		ConstantLabelType	labelConstant;

		static Constant * createInt(int value) {
			Constant * result = new Constant;
			result->base.kind = ConstantInt;
			result->intConstant.value = value;
			return result;
		}

		static Constant * createFixed(int value) {
			Constant * result = new Constant;
			result->base.kind = ConstantFixed;
			result->fixedConstant.value = value;
			return result;
		}

		static Constant * createString(const char * value) {
			Constant * result = new Constant;
			result->base.kind = ConstantString;
			result->stringConstant.value = value;
			return result;
		}

		static Constant * createLabel(Label * value) {
			Constant * result = new Constant;
			result->base.kind = ConstantLabel;
			result->labelConstant.value = value;
			return result;
		} 
	};

	typedef std::vector<Constant *> ConstantList;
	typedef std::vector<int> RegisterList;

	enum InstructionKind {
		InstructionNone,
		InstructionUnary,
		InstructionBinary,
		InstructionCompare,
		InstructionLoad,
		InstructionStore,
		InstructionLoadImmediate,
		InstructionBranchReg,
		InstructionBranchLabel,
		InstructionBranchConditionally,
		InstructionPhi,
		InstructionCall,
		InstructionRet,

		// ARM specific formats -> should go to ARM folder...
		InstructionArmBinaryImmediate,
		InstructionArmBinaryShiftReg,
		InstructionArmBinaryShiftImmed,

		InstructionArmLoadImmedOffset,
		InstructionArmStoreImmedOffset,
		InstructionArmLoadRegOffset,
		InstructionArmStoreRegOffset,
		InstructionArmLoadRegImmedOffset,
		InstructionArmStoreRegImmedOffset
	};

	struct InstructionBaseType {
		InstructionKind	kind;
		Opcode opcode;
		Block * block;
		unsigned used: 1;

#ifndef NDEBUG
		const char * comment;
#endif
	};

	struct InstructionUnaryType {
		InstructionBaseType	base;
		int rS;
		int rD;
		int rC;
	};

	struct InstructionBinaryType {
		InstructionBaseType	base;
		int rS;
		int rD;
		int rC;
		int rM;
	};

	struct InstructionCompareType {
		InstructionBaseType	base;
		int rS;
		int rD;
		int rC;
	};

	struct InstructionLoadType {
		InstructionBaseType	base;
		int rS;
		int rD;
	};

	struct InstructionStoreType {
		InstructionBaseType	base;
		int rS;
		int rD;
	};

	struct InstructionLoadImmediateType {
		InstructionBaseType	base;
		int rD;
		Constant * constant;
	};

	struct InstructionBranchRegType {
		InstructionBaseType	base;
		int rS;
	};

	struct InstructionBranchLabelType {
		InstructionBaseType	base;
		Label * label;
	};

	struct InstructionBranchConditionallyType {
		InstructionBaseType	base;
		int rS;
		Label * label;
	};

	struct InstructionPhiType {
		InstructionBaseType	base;
		int rD;
		RegisterList * registers;
	};

	struct InstructionCallType {
		InstructionBaseType	base;
		int rS;
		RegisterList * args;
		RegisterList * results;
	};

	struct InstructionRetType {
		InstructionBaseType	base;
		RegisterList * registers;
	};

	struct Instruction {

		union {
			InstructionBaseType					base;
			InstructionUnaryType				unary;
			InstructionBinaryType				binary;
			InstructionCompareType				compare;
			InstructionLoadType					load;
			InstructionStoreType				store;
			InstructionLoadImmediateType		loadImmediate;
			InstructionBranchConditionallyType	branchConditionally;
			InstructionBranchRegType			branchReg;
			InstructionBranchLabelType			branchLabel;
			InstructionPhiType					phi;
			InstructionCallType					call;
			InstructionRetType					ret;
		};

		static Instruction * createUnary(Opcode anOpcode, int rD, int rS, const char * comment = "") {
			Instruction * result = new Instruction();
			result->base.kind = InstructionUnary;
			result->base.opcode = anOpcode;

#ifndef NDEBUG
			result->base.comment = comment;
#endif

			result->unary.rD = rD;
			result->unary.rS = rS;
			result->unary.rC = -1;
			return result;
		}

		static Instruction * createUnary(Opcode anOpcode, int rD, int rC, int rS, const char * comment = "") {
			Instruction * result = new Instruction();
			result->base.kind = InstructionUnary;
			result->base.opcode = anOpcode;

#ifndef NDEBUG
			result->base.comment = comment;
#endif

			result->unary.rD = rD;
			result->unary.rS = rS;
			result->unary.rC = rC;
			return result;
		}

		static Instruction * createBinary(Opcode anOpcode, int rD, int rS, int rM, const char * comment = "") {
			Instruction * result = new Instruction();
			result->base.kind = InstructionBinary;
			result->base.opcode = anOpcode;

#ifndef NDEBUG
			result->base.comment = comment;
#endif

			result->binary.rD = rD;
			result->binary.rS = rS;
			result->binary.rC = -1;
			result->binary.rM = rM;
			return result;
		}

		static Instruction * createBinary(Opcode anOpcode, int rD, int rC, int rS, int rM, const char * comment = "") {
			Instruction * result = new Instruction();
			result->base.kind = InstructionBinary;
			result->base.opcode = anOpcode;

#ifndef NDEBUG
			result->base.comment = comment;
#endif

			result->binary.rD = rD;
			result->binary.rS = rS;
			result->binary.rC = rC;
			result->binary.rM = rM;
			return result;
		}

		static Instruction * createCompare(Opcode anOpcode, int rD, int rS, int rC, const char * comment = "") {
			Instruction * result = new Instruction();
			result->base.kind = InstructionCompare;
			result->base.opcode = anOpcode;

#ifndef NDEBUG
			result->base.comment = comment;
#endif

			result->compare.rD = rD;
			result->compare.rC = rC;
			result->compare.rS = rS;
			return result;
		}

		static Instruction * createLoad(Opcode anOpcode, int rD, int rS, const char * comment = "") {
			Instruction * result = new Instruction();
			result->base.kind = InstructionLoad;
			result->base.opcode = anOpcode;

#ifndef NDEBUG
			result->base.comment = comment;
#endif

			result->load.rD = rD;
			result->load.rS = rS;
			return result;
		}

		static Instruction * createStore(Opcode anOpcode, int rD, int rS, const char * comment = "") {
			Instruction * result = new Instruction();
			result->base.kind = InstructionStore;
			result->base.opcode = anOpcode;

#ifndef NDEBUG
			result->base.comment = comment;
#endif

			result->store.rD = rD;
			result->store.rS = rS;
			return result;
		}

		static Instruction * createLoadImmediate(Opcode anOpcode, int rD, Constant * value, const char * comment = "") {
			Instruction * result = new Instruction();
			result->base.kind = InstructionLoadImmediate;
			result->base.opcode = anOpcode;

#ifndef NDEBUG
			result->base.comment = comment;
#endif

			result->loadImmediate.rD = rD;
			result->loadImmediate.constant = value;
			return result;
		}

		static Instruction * createBranchReg(Opcode anOpcode, int rS, const char * comment = "") {
			Instruction * result = new Instruction();
			result->base.kind = InstructionBranchReg;
			result->base.opcode = anOpcode;

#ifndef NDEBUG
			result->base.comment = comment;
#endif

			result->branchReg.rS = rS;
			return result;
		}

		static Instruction * createBranchLabel(Opcode anOpcode, Label * label, const char * comment = "") {
			Instruction * result = new Instruction();
			result->base.kind = InstructionBranchLabel;
			result->base.opcode = anOpcode;

#ifndef NDEBUG
			result->base.comment = comment;
#endif

			result->branchLabel.label = label;
			return result;
		}

		static Instruction * createBranchConditionally(Opcode anOpcode, int rS, Label * label, const char * comment = "") {
			Instruction * result = new Instruction();
			result->base.kind = InstructionBranchConditionally;
			result->base.opcode = anOpcode;

#ifndef NDEBUG
			result->base.comment = comment;
#endif

			result->branchConditionally.rS = rS;
			result->branchConditionally.label = label;
			return result;
		}

		static Instruction * createPhi(Opcode anOpcode, int rD, RegisterList * registers, const char * comment = "") {
			Instruction * result = new Instruction();
			result->base.kind = InstructionPhi;
			result->base.opcode = anOpcode;

#ifndef NDEBUG
			result->base.comment = comment;
#endif

			result->phi.rD = rD;
			result->phi.registers = registers;
			return result;
		}

		static Instruction * createCall(Opcode anOpcode, int rS, RegisterList * args,
			RegisterList * results, const char * comment = "") {
			Instruction * result = new Instruction();
			result->base.kind = InstructionCall;
			result->base.opcode = anOpcode;

#ifndef NDEBUG
			result->base.comment = comment;
#endif

			result->call.rS = rS;
			result->call.args = args;
			result->call.results = results;
			return result;
		}

		static Instruction * createRet(Opcode anOpcode, RegisterList * registers, const char * comment = "") {
			Instruction * result = new Instruction();
			result->base.kind = InstructionRet;
			result->base.opcode = anOpcode;

#ifndef NDEBUG
			result->base.comment = comment;
#endif

			result->ret.registers = registers;
			return result;
		}
	};

	typedef std::vector<Instruction *> InstructionList;
	typedef std::map<int, Instruction *> Definition;

	struct Block;

	typedef std::vector<Block *> BlockList;

	struct Block {
		int					number;			// assigned during ROP ordering of blocks
		Procedure	*		procedure;
		LabelList			labels;
		InstructionList		instructions;
		BlockList			predecessors;
		BlockList			successors;

		Block(Procedure * owner) {
			procedure = owner;
		}

		Block& operator+=(Instruction * inst) {
			instructions.push_back(inst);
			return *this;
		}

		void AttachLabel(Label * label) {
			labels.push_back(label);
			label->block.block = this;
		}
	};

	struct Procedure {
		Label *			def;
		Module *		module;
		Definition		definitions;
		BlockList		blocks;
		LabelList		labels;
		int				parameters;
		int				returnValues;

		Procedure(Label * label, Module * owner, int args, int retVals) {
			def = label;
			module = owner;
			parameters = args;
			returnValues = retVals;
		}

		Block * CreateBlock() {
			Block * result = new Block(this);
			blocks.push_back(result);
			return result;
		}
	};

	typedef std::vector<Procedure *> ProcedureList;

	struct Module {
		const char *	name;
		ProcedureList	procedures;
		LabelList		labels;
		ConstantList	constants;

		Module(const char * aName) {
			name = aName;
		}
	};

	struct StringCompare {
		typedef const char * Key ;

		bool operator()(const Key& _Left, const Key& _Right) const {
			return strcmp(_Left, _Right) < 0;
		}
	};

	typedef std::map<const char *, Label *, StringCompare> IdentifierMap;


	// -------------------------------------------------------------------------
	// Sweep	-	Base class for code sweeps across the intermediate
	//				language representation
	// -------------------------------------------------------------------------
	class Sweep {
	public:
		virtual void sweep(triVM::Module * module);
		virtual void sweep(triVM::Procedure * procedure);
		virtual void sweep(triVM::Block * block);

		virtual void reverseSweep(triVM::Module * module);
		virtual void reverseSweep(triVM::Procedure * procedure);
		virtual void reverseSweep(triVM::Block * block);

	protected:
		virtual void begin(triVM::Module * module);
		virtual void begin(triVM::Procedure * procedure);
		virtual void begin(triVM::Block * block);

		virtual void visit(triVM::Instruction * instruction);

		virtual void end(triVM::Module * module);
		virtual void end(triVM::Procedure * procedure);
		virtual void end(triVM::Block * block);

	protected:
		virtual void visitUnary(triVM::Instruction * instruction);
		virtual void visitBinary(triVM::Instruction * instruction);
		virtual void visitCompare(triVM::Instruction * instruction);
		virtual void visitLoad(triVM::Instruction * instruction);
		virtual void visitStore(triVM::Instruction * instruction);
		virtual void visitLoadImmediate(triVM::Instruction * instruction);
		virtual void visitBranchReg(triVM::Instruction * instruction);
		virtual void visitBranchLabel(triVM::Instruction * instruction);
		virtual void visitBranchConditionally(triVM::Instruction * instruction);
		virtual void visitPhi(triVM::Instruction * instruction);
		virtual void visitCall(triVM::Instruction * instruction);
		virtual void visitRet(triVM::Instruction * instruction);

	};


	extern void RemoveUnusedCode(Module * module);

	typedef std::map<int, Instruction *> RegisterDefinitionMap;

	extern RegisterDefinitionMap * FindDefinitions(Module * module);

	extern void DumpModule(FILE * out, EGL::triVM::Module * module);

} // namespace triVM
} // namespace EGL


#endif //ndef EGL_TRIVM_H

