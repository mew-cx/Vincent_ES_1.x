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
#ifndef ARBBACKEND_H
#define ARBBACKEND_H

#include "ARBInstruction.h"

#include "../Include/intermediate.h"

#include <string>
#include <vector>

// List of assembly instructions
class ARBInstructionList: public std::vector<ARBInstruction> {
public:
	void append(ARBInstructionList list) {
		for (unsigned int i = 0; i < list.size(); ++i) {
			push_back(list[i]);
		}
	}
};

// Result of parsing a node, contains the resulting variable and the instruction list for computing the results
struct nodeResult {
	bool fullyParsed;
	ARBVar var;
	ARBInstructionList list;
	bool hasResultVar;
	nodeResult(ARBVar var, ARBInstructionList list): fullyParsed(true), var(var), list(list), hasResultVar(true) {}
	nodeResult(ARBVar var): fullyParsed(true), var(var), hasResultVar(true) {}
	nodeResult(ARBInstructionList list): fullyParsed(true), var(ARBVar::None), list(list), hasResultVar(false) {}
	nodeResult(bool fullyParsed): fullyParsed(fullyParsed), var(ARBVar::None), hasResultVar(false) {
		assert(fullyParsed == false);
	}
};

// Handles a generic node by dispatching it to helper functions
nodeResult parseNode(TIntermNode* node);

// Contains information about built-in and user defined functions
struct ARBFunction {
	std::string name;
	ARBInstructionList instructions;
	std::vector<ARBVar> parameters;
	ARBVar result;
	bool hasReturnValue;
	ARBFunction(): result(ARBVar::None), hasReturnValue(false) {
	}
};

#endif
