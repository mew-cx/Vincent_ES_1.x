/*
//
//Copyright (C) 2002-2005  Falanx Microsystems AS
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of Falanx Microsystems AS nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//POSSIBILITY OF SUCH DAMAGE.
//
*/
#ifndef ARBINSTRUCTION_H
#define ARBINSTRUCTION_H

#include "ARBInstructionType.h"

#include "../Include/intermediate.h"
#include "../Include/Common.h"
#include "../Include/ShHandle.h"

#include <string>
#include <vector>

class ARBMatrixVar;

// Contains information about an ARB variable
class ARBVar {
private:
	ARBVar();
	TIntermTyped* node;
	ARBMatrixVar* matrix;
	std::string swizzleMask;
	int tempvarNum;
	int matrixCol;
	int elements;
	std::string hardcodedName;
	static std::string GetNodeVarName(TIntermTyped* node);
public:
	static ARBVar NewTextVar(std::string hardcodedName);
	static ARBVar GetNodeVar(TIntermTyped* node);
	static ARBVar NewTempVar(int elements = 4);
	ARBVar swizzle(std::string mask);
	ARBVar getMatrixCol(unsigned int col);
	std::string tostr() const;
	std::string getasm(bool swizzled = false, bool asInput = false) const;
	std::string getARBVarDeclaration() const;
	std::string ARBVar::getARBOutputAssignment() const;
	int getasmvartype() const;
	bool isSwizzled() const;
	bool isTemporary() const;
	bool isSampler() const;
	static ARBVar None;
	static std::string swizzleChars;
	static std::string GetSwizzleMask(int elements);
};

// Contains information about a matrix (4 ARBVars)
class ARBMatrixVar {
private:
	ARBMatrixVar();
	std::vector<ARBVar> cols;
	friend class ARBVar;
};

// Contains information about an ARB instruction
class ARBInstruction {
private:
	ARBInstructionType inst;
	std::vector<ARBVar> input;
	ARBVar output;
public:
	ARBInstruction(ARBInstructionType inst, ARBVar output, std::vector<ARBVar> input);
	ARBInstruction(ARBInstructionType inst, ARBVar output, ARBVar input1);
	ARBInstruction(ARBInstructionType inst, ARBVar output, ARBVar input1, ARBVar input2);
	ARBInstruction(ARBInstructionType inst, ARBVar output, ARBVar input1, ARBVar input2, ARBVar input3);
	std::string tostr() const;
	std::string genasm() const;
	const std::vector<ARBVar>& getInputVarList() const;
	const ARBVar& getOutputVar() const;
	const ARBInstructionType& getInstructionType() const;
};

#endif
