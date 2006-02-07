#ifndef UTIL_H
#define UTIL_H

#include "../Include/intermediate.h"
#include "../Include/InfoSink.h"

#include "ARBInstruction.h"

#include <string>

// Converts numbers to strings
std::string inttostr(int i);
std::string floattostr(float f);

// Returns the textual representation of an operator
std::string getopname(TOperator op);

#define failmsg() fail_func(__FILE__, __LINE__)

// Handles numbering for ARB variable declarations
void resetVarNumbering();
int getUniformNum(std::string varname);
int getAttribNum(std::string varname);
int getTexcoordNum(std::string varname);
int getSamplerNum(std::string varname);

// Handles returning the appropriate stream for writing messages
void setInfoSink(TInfoSink& infosink, int debugOptions);
TInfoSinkBase& fail_func(std::string file, int line);
TInfoSinkBase& msg();
TInfoSinkBase& asmout();

// Handles dynamic memory management
void recordMatrixVar(ARBMatrixVar* matrix);
void cleanUpMemory();

#endif
