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
#include "../Include/intermediate.h"

#include "ARBInstructionType.h"
#include "Util.h"

#include "ARBInstruction.h"

using namespace std;

// Characters used for swizzling
string ARBVar::swizzleChars = "xyzw";

// Empty variable used for e.g. functions without return values
ARBVar ARBVar::None;

// ARBVar constructor, initializes all fields
ARBVar::ARBVar(): node(0), matrix(0), swizzleMask(""), tempvarNum(0), matrixCol(0), elements(0), hardcodedName("") {
}

// ARBMatrixVar constructor, does nothing
ARBMatrixVar::ARBMatrixVar() {
}

// Returns swizzle mask for the specified number of elements in vector
string ARBVar::GetSwizzleMask(int elements) {
	string swizzle = "";
	if (elements != 4) {
		for (int i = 0; i < elements; ++i) {
			swizzle += swizzleChars[i];
		}
	}
	return swizzle;
}

// Returns name to be used in generated assembly for a given node
string ARBVar::GetNodeVarName(TIntermTyped* node) {
	TIntermSymbol* symbol = node->getAsSymbolNode();
	if (symbol) {
		return string("symbol_") + symbol->getSymbol().c_str();
	}
	TIntermConstantUnion* cu = node->getAsConstantUnion();
	if (cu) {
		string ret = "";
		if (cu->getBasicType() == EbtFloat) {
			if (cu->getNominalSize() == 1) {
				ret = "const_" + floattostr(cu->getUnionArrayPointer()[0].getFConst());
			} else if (cu->isVector() && cu->getNominalSize() > 1 && cu->getNominalSize() <= 4) {
				ret = "const_vec" + inttostr(cu->getNominalSize());
				for (int i = 0; i < cu->getNominalSize(); ++i) {
					ret += "_" + floattostr(cu->getUnionArrayPointer()[i].getFConst());
				}
			}
		}
		if (ret != "") {
			for (unsigned int i = 0; i < ret.size(); ++i) {
				if (ret[i] == '.') {
					ret[i] = 'x';
				}
			}
			return ret;
		}
	}
	failmsg() << "Unknown node in GetNodeVarName [" << node->getCompleteString() << "]\n";
	return "{unknown node}";
}

// Returns ARB variable for the given column in a matrix, the first column = column 0
ARBVar ARBVar::getMatrixCol(unsigned int col) {
	assert(matrix);
	assert(matrix->cols.size() > col);
	return matrix->cols[col];
}

// Returns ARB variable representing the given node
ARBVar ARBVar::GetNodeVar(TIntermTyped* node) {
	if (node->isMatrix()) {
		ARBVar var;
		ARBMatrixVar* matrix = new ARBMatrixVar();
		recordMatrixVar(matrix);
		ARBVar col;
		col.node = node;
		for (col.matrixCol = 0; col.matrixCol < 4; ++col.matrixCol) {
			matrix->cols.push_back(col);
		}
		var.matrix = matrix;
		return var;
	} else {
		ARBVar var;
		var.node = node;
		return var;
	}
}

// Returns whether the variable is based on a sampler
bool ARBVar::isSampler() const {
	if (node && IsSampler(node->getBasicType())) {
		return true;
	} else {
		return false;
	}
}

// Returns a new temporary ARB variable
ARBVar ARBVar::NewTempVar(int elements) {
	static int tempvar = 0;
	ARBVar var;
	var.tempvarNum = ++tempvar;
	var.elements = elements;
	return var;
}

// Returns whether the variable is a temporary variable
bool ARBVar::isTemporary() const {
	if (tempvarNum != 0) {
		return true;
	} else if (node && node->getQualifier() == EvqTemporary) {
		return true;
	} else {
		return false;
	}
}

// Returns an ARB variable that evaluates to the hardcoded string
ARBVar ARBVar::NewTextVar(std::string hardcodedName) {
	ARBVar var;
	var.hardcodedName = hardcodedName;
	return var;
}

// Returns a swizzled version of a variable
ARBVar ARBVar::swizzle(string mask) {
	if (swizzleMask != "") { // TODO: implement
		failmsg() << "Swizzling already swizzled variables not implemented\n";
	}
	ARBVar var = *this;
	var.swizzleMask = mask;
	return var;
}

// Returns whether a variable is swizzled
bool ARBVar::isSwizzled() const {
	return swizzleMask != "";
}

// Returns textual representation of the variable used for debugging
string ARBVar::tostr() const {
	return "[" + getasm(true) + "]";
}

// Returns full swizzling string for a 4-element ARB vector based on partial swizzling information
string completeSwizzle(string swizzle) {
	if (swizzle == "xy" || swizzle == "xyz") {
		return "";
	} else if (swizzle.length() == 2) {
		return swizzle + "ww";
	} else if (swizzle.length() == 3) {
		return swizzle + "w";
	} else {
		return swizzle;
	}
}

// Returns full text to be used in the generated assembly for the variable
// swizzled: whether the resulting string should contain swizzling information
// asInput: whether the variable is being used as input, in which case the swizzling is changed to be usable as source vector in ARB programs
string ARBVar::getasm(bool swizzled, bool asInput) const {
	string varname = "";
	string swizzle = swizzleMask;
	if (node) {
		varname = GetNodeVarName(node);
		if (node->isMatrix()) {
			varname += string("_col") + inttostr(matrixCol);
		}
	} else {
		if (hardcodedName != "") {
			varname = hardcodedName;
		} else {
			if (tempvarNum) {
				varname = string("tempvar_") + inttostr(tempvarNum);
			} else {
				varname = string("tempvar_ERROR_ILLEGAL_VAR");
			}
		}
	}
	if (asInput) {
		swizzle = completeSwizzle(swizzle);
	}
	if (swizzled && swizzle != "") {
		varname += "." + swizzle;
	}
	return varname;
}

// Returns textual ARB program declaration for a given variable, including initialization
// Returns "" to indicate that the variable shouldn't be declared, "ATTRIB" and "PARAM" for input, "TEMP" for temporary and "OUTPUT" for output variables
string ARBVar::getARBVarDeclaration() const {
	if (isSampler()) {
		return "";
	} else if (node) {
		TQualifier qual = node->getQualifier();
		switch(node->getQualifier()) {
			case EvqPosition:
				return "OUTPUT out" + getasm() + " = result.position;\nTEMP " + getasm() + ";";
			case EvqFragColor:
				return "OUTPUT out" + getasm() + " = result.color;\nTEMP " + getasm() + ";";
			case EvqAttribute:
				return "ATTRIB " + getasm() + " = vertex.attrib[" + inttostr(getAttribNum(getasm())) + "]; # FIXME: Change to correct index";
			case EvqConst: {
				string value = "{ ??? }; # FIXME: Enter correct values";
				TIntermConstantUnion* cu = node->getAsConstantUnion();
				if (cu && cu->getBasicType() == EbtFloat) {
					if (cu->getNominalSize() == 1) {
						value = floattostr(cu->getUnionArrayPointer()[0].getFConst());
						value = "{ " + value + ", " + value + ", " + value + ", " + value + " };";
					} else if (cu->isVector() && cu->getNominalSize() >= 1 && cu->getNominalSize() <= 4) {
						value = "{ ";
						for (int i = 0; i < 4; ++i) {
							if (i) {
								value += ", ";
							}
							if (i < cu->getNominalSize()) {
								value += floattostr(cu->getUnionArrayPointer()[i].getFConst());
							} else {
								value += "0.0";
							}
						}
						value += " };";
					}
				}
				return "PARAM " + getasm() + " = " + value;
			}
			case EvqVaryingOut:
				return "OUTPUT out" + getasm() + " = result.texcoord[" + inttostr(getTexcoordNum(getasm())) + "]; # FIXME: Change to correct index;\nTEMP " + getasm() + ";";
			case EvqVaryingIn:
				return "ATTRIB " + getasm() + " = fragment.texcoord[" + inttostr(getTexcoordNum(getasm())) + "]; # FIXME: Change to correct index";
			case EvqTemporary:
				return "TEMP " + getasm() + ";";
			case EvqUniform:
				return "PARAM " + getasm() + " = program.env[" + inttostr(getUniformNum(getasm())) + "]; # FIXME: Change to correct index";
			default:
				return string("UNKNOWN NODE ") + getasm() + " - " + node->getQualifierString();
		}
	} else {
		if (hardcodedName != "") {
			return "";
		} else {
			return "TEMP " + getasm() + ";";
		}
	}
}

// Returns "" to indicate that extra instructions aren't needed, otherwise the additional instruction that should be appended to the instruction list
string ARBVar::getARBOutputAssignment() const {
	if (node) {
		TQualifier qual = node->getQualifier();
		switch(node->getQualifier()) {
			case EvqPosition:
			case EvqFragColor:
			case EvqVaryingOut:
				return "MOV out" + getasm() + ", " + getasm() + ";\n";
		}
	}
	return "";
}

// Returns type of variable, used for ordering the declaration list
int ARBVar::getasmvartype() const {
	if (node) {
		TQualifier qual = node->getQualifier();
		switch(node->getQualifier()) {
			case EvqUniform:
				return 0;
			case EvqAttribute:
				return 1;
			case EvqVaryingIn:
				return 2;
			case EvqVaryingOut:
				return 3;
			case EvqPosition:
				return 4;
			case EvqFragColor:
				return 5;
			case EvqConst:
				return 6;
			case EvqTemporary:
				return 7;
			default:
				return 9;
		}
	} else {
		if (hardcodedName != "") {
			return -1;
		} else {
			return 8;
		}
	}
}

// Constructs an ARBInstruction based on type, output variable and vector specifying input
ARBInstruction::ARBInstruction(ARBInstructionType inst, ARBVar output, vector<ARBVar> input): inst(inst), output(output), input(input) {
}

// Constructs an ARBInstruction based on type, output variable and one source variable
ARBInstruction::ARBInstruction(ARBInstructionType inst, ARBVar output, ARBVar input1): inst(inst), output(output) {
	input.push_back(input1);
}

// Constructs an ARBInstruction based on type, output variable and two source variables
ARBInstruction::ARBInstruction(ARBInstructionType inst, ARBVar output, ARBVar input1, ARBVar input2): inst(inst), output(output) {
	input.push_back(input1);
	input.push_back(input2);
}

// Constructs an ARBInstruction based on type, output variable and three source variables
ARBInstruction::ARBInstruction(ARBInstructionType inst, ARBVar output, ARBVar input1, ARBVar input2, ARBVar input3): inst(inst), output(output) {
	input.push_back(input1);
	input.push_back(input2);
	input.push_back(input3);
}

// Contains information about an ARB program instruction
struct ARBInstructionName {
	ARBInstructionType type;
	std::string name;
	std::string description;
};

// ARB instruction set names and descriptions
ARBInstructionName instructionNames[] = {
	{ EInstABS, "ABS", "Absolute value" },
	{ EInstADD, "ADD", "Add" },
	{ EInstARL, "ARL", "Address register load" },
	{ EInstCMP, "CMP", "Compare" },
	{ EInstCOS, "COS", "Cosine with reduction to [-PI,PI]" },
	{ EInstDP3, "DP3", "3-component dot product" },
	{ EInstDP4, "DP4", "4-component dot product" },
	{ EInstDPH, "DPH", "Homogeneous dot product" },
	{ EInstDST, "DST", "Distance vector" },
	{ EInstEX2, "EX2", "Exponential base 2" },
	{ EInstEXP, "EXP", "Exponential base 2 (approximate)" },
	{ EInstFLR, "FLR", "Floor" },
	{ EInstFRC, "FRC", "Fraction" },
	{ EInstLG2, "LG2", "Logarithm base 2" },
	{ EInstKIL, "KIL", "Kill fragment" },
	{ EInstLIT, "LIT", "Compute light coefficients" },
	{ EInstLOG, "LOG", "Logarithm base 2 (approximate)" },
	{ EInstLRP, "LRP", "Linear interpolation" },
	{ EInstMAD, "MAD", "Multiply and add" },
	{ EInstMAX, "MAX", "Maximum" },
	{ EInstMIN, "MIN", "Minimum" },
	{ EInstMOV, "MOV", "Move" },
	{ EInstMUL, "MUL", "Multiply" },
	{ EInstPOW, "POW", "Exponentiate" },
	{ EInstRCP, "RCP", "Reciprocal" },
	{ EInstRSQ, "RSQ", "Reciprocal square root" },
	{ EInstSCS, "SCS", "Sine/cosine without reduction" },
	{ EInstSGE, "SGE", "Set on greater than or equal" },
	{ EInstSIN, "SIN", "Sine with reduction to [-PI,PI]" },
	{ EInstSLT, "SLT", "Set on less than" },
	{ EInstSUB, "SUB", "Subtract" },
	{ EInstSWZ, "SWZ", "Extended swizzle" },
	{ EInstTEX, "TEX", "Texture sample" },
	{ EInstTXB, "TXB", "Texture sample with bias" },
	{ EInstTXP, "TXP", "Texture sample with projection" },
	{ EInstXPD, "XPD", "Cross product" },
};

// Returns textual ARB program representation of the given ARB instruction type
string insttypetostr(ARBInstructionType type) {
	for (unsigned int i = 0; i < sizeof(instructionNames)/sizeof(instructionNames[0]); ++i) {
		if (instructionNames[i].type == type) {
			return instructionNames[i].name;
		}
	}
	return "{unknown instruction type}";
}

// Return list of variables used as input for instruction
const vector<ARBVar>& ARBInstruction::getInputVarList() const {
	return input;
}

// Return variable used as output for instruction
const ARBVar& ARBInstruction::getOutputVar() const {
	return output;
}

// Returns debug string for instruction
string ARBInstruction::tostr() const {
	string ret = insttypetostr(inst) + " " + output.tostr() + " <- ";
	for (unsigned int i = 0; i < input.size(); ++i) {
		ret += ", ";
		ret += input[i].tostr();
	}
	return ret;
}

// Returns assembly code for instruction
string ARBInstruction::genasm() const {
	string ret = insttypetostr(inst) + " " + output.getasm(true);
	string extra = "";
	for (unsigned int i = 0; i < input.size(); ++i) {
		ret += ", ";
		if (input[i].isSampler()) {
			ret += "texture[" + inttostr(getSamplerNum(input[i].getasm())) + "]";
			extra = " # FIXME: Change to correct texture (" + input[i].getasm() + ")";
		} else {
			ret += input[i].getasm(true, true);
		}
	}
	return ret + ";" + extra;
}

// Returns instruction type used for instruction
const ARBInstructionType& ARBInstruction::getInstructionType() const {
	return inst;
}
