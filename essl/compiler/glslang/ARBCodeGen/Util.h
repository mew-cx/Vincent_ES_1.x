/*
//
//Copyright (C) 2005-2006  Falanx Microsystems AS
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
