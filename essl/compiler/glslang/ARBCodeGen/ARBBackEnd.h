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
