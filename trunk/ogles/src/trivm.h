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

	enum Opcode;
	union Label;
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

	enum InstructionKind;
	struct InstructionBaseType {
		InstructionKind	kind;
		Opcode opcode;
		Block * block;
		unsigned used: 1;

#ifndef NDEBUG
		const char * comment;
#endif

		InstructionBaseType(InstructionKind aKind, Opcode anOpcode, const char * aComment) {
			kind = aKind;
			opcode = anOpcode;

	#ifndef NDEBUG
			comment = aComment;
	#endif
		
			used = 0;
		}
	};

	typedef InstructionBaseType Instruction;

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

	extern void RemoveUnusedCode(Module * module);

	typedef std::map<int, Instruction *> RegisterDefinitionMap;

	extern RegisterDefinitionMap * FindDefinitions(Module * module);

	extern void DumpModule(FILE * out, EGL::triVM::Module * module);

} // namespace triVM
} // namespace EGL


#endif //ndef EGL_TRIVM_H

