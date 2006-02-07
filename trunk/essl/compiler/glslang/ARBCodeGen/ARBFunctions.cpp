#include "ARBFunctions.h"

#include "Util.h"
#include <vector>
#include "ARBInstruction.h"
using namespace std;

//
// Operators
//

// Generates vector + vector assembly code
ARBInstructionList ARBAddition(ARBVar result, ARBVar vec1, ARBVar vec2) {
	ARBInstructionList list;
	//                    Instruction name  result  var1  var2
	list.push_back(ARBInstruction(EInstADD, result, vec1, vec2));
	return list;
}

// Generates vector - vector assembly code
ARBInstructionList ARBSubtraction(ARBVar result, ARBVar vec1, ARBVar vec2) {
	ARBInstructionList list;
	list.push_back(ARBInstruction(EInstSUB, result, vec1, vec2));
	return list;
}

// Generates vector * vector assembly code
ARBInstructionList ARBMultiplication(ARBVar result, ARBVar vec1, ARBVar vec2) {
	ARBInstructionList list;
	list.push_back(ARBInstruction(EInstMUL, result, vec1, vec2));
	return list;
}

// Generates vector = vector assembly code
ARBInstructionList ARBAssignment(ARBVar result, ARBVar vector) {
	ARBInstructionList list;
	list.push_back(ARBInstruction(EInstMOV, result, vector));
	return list;
}

//
// Math Functions
//

// Generates minimum min(a, b) assembly code
ARBInstructionList ARBMinimum(ARBVar result, ARBVar vec1, ARBVar vec2) {
	ARBInstructionList list;
	list.push_back(ARBInstruction(EInstMIN, result, vec1, vec2));
	return list;
}

// Generates maximun max(a, b) assembly code
ARBInstructionList ARBMaximum(ARBVar result, ARBVar vec1, ARBVar vec2) {
	ARBInstructionList list;
	list.push_back(ARBInstruction(EInstMAX, result, vec1, vec2));
	return list;
}

// Generates clamp(a, min, max) assembly code
ARBInstructionList ARBClamp(ARBVar result, ARBVar vec1, ARBVar minVec, ARBVar maxVec) {
	ARBInstructionList list;
	list.push_back(ARBInstruction(EInstMAX, result, vec1, minVec));
	list.push_back(ARBInstruction(EInstMIN, result, result, maxVec));
	return list;
}

// Generates absolute abs(a) assembly code
ARBInstructionList ARBAbsolute(ARBVar result, ARBVar vector) {
	ARBInstructionList list;
	list.push_back(ARBInstruction(EInstABS, result, vector));
	return list;
}

// Generates square root sqrt(a) assemply code
ARBInstructionList ARBSquareRoot(ARBVar result, ARBVar vector) {
	ARBInstructionList list;
	list.push_back(ARBInstruction(EInstRSQ, result, vector));
	list.push_back(ARBInstruction(EInstRCP, result, result));
	return list;
}

// Generates a raised to the y power pow(a, b) assemply code
ARBInstructionList ARBPower(ARBVar result, ARBVar vec1, ARBVar vec2) {
	ARBInstructionList list;
	list.push_back(ARBInstruction(EInstPOW, result, vec1, vec2));
	return list;
}

//
// Geometric Functions
//

// Generates the vector length length(a) assembly code
ARBInstructionList ARBLength(ARBVar result, ARBVar vector, bool component3) {
	ARBInstructionList list;
	if(component3) {
	list.push_back(ARBInstruction(EInstDP3, result, vector, vector));
	} else {
	list.push_back(ARBInstruction(EInstDP4, result, vector, vector));
	}
	list.push_back(ARBInstruction(EInstRSQ, result, result.swizzle("w")));
	list.push_back(ARBInstruction(EInstRCP, result, result));
	return list;
}

// Generates the distance between two vectors, distance(a, b) assembly code
ARBInstructionList ARBDistance(ARBVar result, ARBVar vec1, ARBVar vec2, bool component3) {
	ARBInstructionList list;
	list.push_back(ARBInstruction(EInstSUB, result, vec1, vec2));
	if(component3) {
	list.push_back(ARBInstruction(EInstDP3, result, result, result));
	} else {
	list.push_back(ARBInstruction(EInstDP4, result, result, result));
	}
	list.push_back(ARBInstruction(EInstRSQ, result, result.swizzle("w")));
	list.push_back(ARBInstruction(EInstRCP, result, result));
	return list;
}

// Generates dot product of 3- and 4-component vectors dot(a, b) assembly code 
ARBInstructionList ARBDot(ARBVar result, ARBVar vec1, ARBVar vec2, bool component3) {
	ARBInstructionList list;
	if (component3) {
		list.push_back(ARBInstruction(EInstDP3, result, vec1, vec2));
	} else {
		list.push_back(ARBInstruction(EInstDP4, result, vec1, vec2));
	}
	return list;
}

// Generates 3-component normalized vector
ARBInstructionList ARB3ComponentNormalize(ARBVar result, ARBVar vector) {
	ARBInstructionList list;
	list.push_back(ARBInstruction(EInstDP3, result.swizzle("w"), vector, vector));
	list.push_back(ARBInstruction(EInstRSQ, result.swizzle("w"), result.swizzle("w")));
	list.push_back(ARBInstruction(EInstMUL, result.swizzle("xyz"), result.swizzle("w"), vector));
	return list;
}

// Generates 4-component normalized vector
ARBInstructionList ARB4ComponentNormalize(ARBVar result, ARBVar vector) {
	ARBInstructionList list;
	list.push_back(ARBInstruction(EInstDP4, result.swizzle("w"), vector, vector));
	list.push_back(ARBInstruction(EInstRSQ, result.swizzle("w"), result.swizzle("w")));
	list.push_back(ARBInstruction(EInstMUL, result, result.swizzle("w"), vector));
	return list;
}

// Generates matrix * vector assembly code
ARBInstructionList ARBMatrixTimesVector(ARBVar result, ARBVar vec, ARBVar row0, ARBVar row1, ARBVar row2, ARBVar row3) {
	ARBInstructionList list;
	list.push_back(ARBInstruction(EInstDP4, result.swizzle("x"), vec, row0));
	list.push_back(ARBInstruction(EInstDP4, result.swizzle("y"), vec, row1));
	list.push_back(ARBInstruction(EInstDP4, result.swizzle("z"), vec, row2));
	list.push_back(ARBInstruction(EInstDP4, result.swizzle("w"), vec, row3));
	return list;
}

//
// Built-in functions
//

// This refers to the build-in functions texture
ARBInstructionList ARBTexture2D(ARBVar result, vector<ARBVar>& parameters) {
	// The first parameter must be a sampler2D.
	
	if (parameters.size() == 2) {					// texture2D(Sampler2D sampler, vec2 coords );
		ARBInstructionList list;
		list.push_back(ARBInstruction(EInstTEX, result, parameters[1], parameters[0], ARBVar::NewTextVar("2D")));
		return list;
	} else if (parameters.size() == 3) {			// texture2D(Sampler2D sampler, vec2 coords, float bias );
		ARBInstructionList list = ARBAssignment( parameters[1].swizzle("w"), parameters[2].swizzle("x") );
		list.push_back(ARBInstruction(EInstTXB, result, parameters[1], ARBVar::NewTextVar("texture[0]"), ARBVar::NewTextVar("2D")));
		return list;
	} else {
		assert(false && "Illegal argument list to ARBTexture2D");
		return ARBInstructionList();
	}
}
