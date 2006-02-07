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
#include "Util.h"
#include "../Public/ShaderLang.h"

#include <sstream>

using namespace std;

// Converts int to string
string inttostr(int i) {
	stringstream ss;
	ss << i;
	return ss.str();
}

// Converts float to string
string floattostr(float f) {
	stringstream ss;
	ss << f;
	return ss.str();
}

// Stores a node operators textual representation
struct opname {
	TOperator op;
	string name;
};

#define defop(a) { a, #a }

// List of operator names
opname opnames[] = {
    defop(EOpNull),            // if in a node, should only mean a node is still being built
    defop(EOpSequence),        // denotes a list of statements, or parameters, etc.
    defop(EOpFunctionCall),
    defop(EOpFunction),        // For function definition
    defop(EOpParameters),      // an aggregate listing the parameters to a function

    //
    // Unary operators
    //

    defop(EOpNegative),
    defop(EOpLogicalNot),
    defop(EOpVectorLogicalNot),
    defop(EOpBitwiseNot),

    defop(EOpPostIncrement),
    defop(EOpPostDecrement),
    defop(EOpPreIncrement),
    defop(EOpPreDecrement),

    defop(EOpConvIntToBool),
    defop(EOpConvFloatToBool),
    defop(EOpConvBoolToFloat),
    defop(EOpConvIntToFloat),
    defop(EOpConvFloatToInt),
    defop(EOpConvBoolToInt),

    //
    // binary operations
    //

    defop(EOpAdd),
    defop(EOpSub),
    defop(EOpMul),
    defop(EOpDiv),
    defop(EOpMod),
    defop(EOpRightShift),
    defop(EOpLeftShift),
    defop(EOpAnd),
    defop(EOpInclusiveOr),
    defop(EOpExclusiveOr),
    defop(EOpEqual),
    defop(EOpNotEqual),
    defop(EOpVectorEqual),
    defop(EOpVectorNotEqual),
    defop(EOpLessThan),
    defop(EOpGreaterThan),
    defop(EOpLessThanEqual),
    defop(EOpGreaterThanEqual),
    defop(EOpComma),

    defop(EOpVectorTimesScalar),
    defop(EOpVectorTimesMatrix),
    defop(EOpMatrixTimesVector),
    defop(EOpMatrixTimesScalar),

    defop(EOpLogicalOr),
    defop(EOpLogicalXor),
    defop(EOpLogicalAnd),

    defop(EOpIndexDirect),
    defop(EOpIndexIndirect),
    defop(EOpIndexDirectStruct),

    defop(EOpVectorSwizzle),

    //
    // Built-in functions potentially mapped to operators
    //

    defop(EOpRadians),
    defop(EOpDegrees),
    defop(EOpSin),
    defop(EOpCos),
    defop(EOpTan),
    defop(EOpAsin),
    defop(EOpAcos),
    defop(EOpAtan),

    defop(EOpPow),
    defop(EOpExp),
    defop(EOpLog),
    defop(EOpExp2),
    defop(EOpLog2),
    defop(EOpSqrt),
    defop(EOpInverseSqrt),

    defop(EOpAbs),
    defop(EOpSign),
    defop(EOpFloor),
    defop(EOpCeil),
    defop(EOpFract),
    defop(EOpMin),
    defop(EOpMax),
    defop(EOpClamp),
    defop(EOpMix),
    defop(EOpStep),
    defop(EOpSmoothStep),

    defop(EOpLength),
    defop(EOpDistance),
    defop(EOpDot),
    defop(EOpCross),
    defop(EOpNormalize),
    defop(EOpFaceForward),
    defop(EOpReflect),
    defop(EOpRefract),

    defop(EOpDPdx),            // Fragment only
    defop(EOpDPdy),            // Fragment only
    defop(EOpFwidth),          // Fragment only

    defop(EOpMatrixTimesMatrix),

    defop(EOpAny),
    defop(EOpAll),

    defop(EOpItof),         // pack/unpack only
    defop(EOpFtoi),         // pack/unpack only
    defop(EOpSkipPixels),   // pack/unpack only
    defop(EOpReadInput),    // unpack only
    defop(EOpWritePixel),   // unpack only
    defop(EOpBitmapLsb),    // unpack only
    defop(EOpBitmapMsb),    // unpack only
    defop(EOpWriteOutput),  // pack only
    defop(EOpReadPixel),    // pack only

    //
    // Branch
    //

    defop(EOpKill),            // Fragment only
    defop(EOpReturn),
    defop(EOpBreak),
    defop(EOpContinue),

    //
    // Constructors
    //

    defop(EOpConstructInt),
    defop(EOpConstructBool),
    defop(EOpConstructFloat),
    defop(EOpConstructVec2),
    defop(EOpConstructVec3),
    defop(EOpConstructVec4),
    defop(EOpConstructBVec2),
    defop(EOpConstructBVec3),
    defop(EOpConstructBVec4),
    defop(EOpConstructIVec2),
    defop(EOpConstructIVec3),
    defop(EOpConstructIVec4),
    defop(EOpConstructMat2),
    defop(EOpConstructMat3),
    defop(EOpConstructMat4),
    defop(EOpConstructStruct),

    //
    // moves
    //

    defop(EOpAssign),
    defop(EOpAddAssign),
    defop(EOpSubAssign),
    defop(EOpMulAssign),
    defop(EOpVectorTimesMatrixAssign),
    defop(EOpVectorTimesScalarAssign),
    defop(EOpMatrixTimesScalarAssign),
    defop(EOpMatrixTimesMatrixAssign),
    defop(EOpDivAssign),
    defop(EOpModAssign),
    defop(EOpAndAssign),
    defop(EOpInclusiveOrAssign),
    defop(EOpExclusiveOrAssign),
    defop(EOpLeftShiftAssign),
    defop(EOpRightShiftAssign),

    //
    // Array operators
    //

    defop(EOpArrayLength),
};

// Returns the textual representation of an operator
string getopname(TOperator op) {
	for (int i = 0; i < sizeof(opnames)/sizeof(opnames[0]); ++i) {
		if (op == opnames[i].op) {
			return opnames[i].name;
		}
	}
	return "{unknown operator}";
}

// Contain information about which infosinks are being used
TInfoSink nullsink;
TInfoSink* utilInfoSink = &nullsink;
int debugOptions = EDebugOpNone;

// Sets an infosink and the debug options to be used
void setInfoSink(TInfoSink& infosink, int dOptions) {
	utilInfoSink = &infosink;
	debugOptions = dOptions;
}

// Prints out an error message and returns infosink stream
TInfoSinkBase& fail_func(string file, int line) {
	assert(utilInfoSink);
	return utilInfoSink->info << "# ERROR [" << file << ":" << line << "] ";
}

// Prints out message and returns infosink stream if debugging is turned on, otherwise it returns a stream that is never used
TInfoSinkBase& msg() {
	assert(utilInfoSink);
	if (debugOptions & EDebugOpAssembly) {
		return utilInfoSink->debug << "# ";
	} else {
		return nullsink.debug;
	}
}

// Resturns infosink to be used for writing textual assembly code
TInfoSinkBase& asmout() {
	assert(utilInfoSink);
	return utilInfoSink->info;
}

// Contains information about which variable numbers have been used
int uniformVarNum = 0;
int attribVarNum = 0;
int texcoordVarNum = 0;
int samplerVarNum = 0;
map<string, int> samplerValues;

// Resets information about used variables, called when switching to compiling a new shader program
void resetVarNumbering() {
	uniformVarNum = 0;
	attribVarNum = 0;
	texcoordVarNum = 0;
	samplerVarNum = 0;
	samplerValues.clear();
}

// Gets next unused number for a uniform variable
int getUniformNum(std::string varname) {
	return uniformVarNum++;
}

// Gets next unused number for an attribute variable
int getAttribNum(std::string varname) {
	return attribVarNum++;
}

// Gets next unused number for a texture coordinate variable
int getTexcoordNum(std::string varname) {
	return texcoordVarNum++;
}

// Gets number for a sampler, and uses the next unused one if it's not assigned yet
int getSamplerNum(std::string varname) {
	if (samplerValues.find(varname) == samplerValues.end()) {
		samplerValues[varname] = samplerVarNum++;
	}
	return samplerValues[varname];
}

// Contains information about dynamically allocated matrices
set<ARBMatrixVar*> matrices;

// Stores a pointer to an allocated matrix
void recordMatrixVar(ARBMatrixVar* matrix) {
	matrices.insert(matrix);
}

// Frees all dynamically allocated memory, called when switching to compiling a new shader program
void cleanUpMemory() {
	for (set<ARBMatrixVar*>::const_iterator i = matrices.begin(); i != matrices.end(); ++i) {
		delete *i;
	}
	matrices.clear();
}
