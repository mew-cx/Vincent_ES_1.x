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
