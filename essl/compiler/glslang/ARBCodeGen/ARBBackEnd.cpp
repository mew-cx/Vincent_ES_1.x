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
#include "../Include/intermediate.h"

#include "ARBCompiler.h"
#include "ARBInstruction.h"
#include "ARBInstructionType.h"
#include "ARBNodeHandling.h"
#include "ARBFunctions.h"
#include "ARBOptimization.h"
#include "Util.h"

#include "ARBBackEnd.h"

#include <string>
#include <map>
#include <queue>

using namespace std;

// funksjoner:
// ARBInstructionList foo(ARBVar result, vector<ARBVar> parameters);
map<string, ARBFunction> functions;

// Assigns a function to handle a specific unary opcode
ARBUnaryNodeHandler unaryNodeHandlers[] = {
	{ EOpAbs, ARBUnaryMathNode },
	{ EOpSqrt, ARBUnaryMathNode },
	{ EOpLength, ARBUnaryMathNode },
	{ EOpNormalize, ARBUnaryMathNode },
};

// Assigns a function to handle a specific binary opcode
ARBBinaryNodeHandler binaryNodeHandlers[] = {
	{ EOpAdd, ARBMathNode },
	{ EOpSub, ARBMathNode },
	{ EOpMul, ARBMathNode },
	{ EOpAssign, ARBAssignmentNode },
	{ EOpMatrixTimesVector, ARBMatrixMathNode },
	{ EOpVectorTimesScalar, ARBMathNode },
	{ EOpVectorSwizzle, ARBSwizzleNode },
};

// Assigns a function to handle a specific aggregate function opcode
ARBAggregateNodeHandler aggregateNodeHandlers[] = {
	{ EOpMin, ARBMathFunctionNode },
	{ EOpMax, ARBMathFunctionNode },
	{ EOpClamp, ARBMathFunctionNode },
	{ EOpPow, ARBMathFunctionNode },
	{ EOpDot, ARBMathFunctionNode },
	{ EOpDistance, ARBMathFunctionNode },
	{ EOpSequence, ARBSequenceNode },
	{ EOpConstructVec4, ARBConstructVectorNode },
};

// Assigns a function to handle a specific built-in functions
ARBBuiltInFunctionHandler builtInFunctionHandlers[] = {
//    mangled name, hasReturnValue, numParams, function
	{ "texture2D(s21;vf2;", true, 2, ARBTexture2D },
};

// Returns the parameter list from a given function definition node
TIntermAggregate* getParameterList(TIntermAggregate* node) {
	TIntermSequence seq = node->getSequence();
	for (unsigned int i = 0; i < seq.size(); ++i) {
		TIntermAggregate* aggregate = seq[i]->getAsAggregate();
		if (aggregate && aggregate->getOp() == EOpParameters) {
			return aggregate;
		}
	}
	return NULL;
}

// Stores necessary information about the function currently being parsed
map<string, ARBVar> functionParams;
nodeResult functionReturnValue(false);

// Handles a generic node by dispatching it to helper functions
nodeResult parseNode(TIntermNode* node) {
	string nodetype = "unknown node";

	TIntermTyped* typed = node->getAsTyped();
	if (typed) {
		// TODO: handle
		nodetype = string("typed ") + typed->getCompleteString().c_str();
	}

	// Symbol node
	TIntermSymbol* symbol = node->getAsSymbolNode();
	if (symbol) {
		ARBVar var = ARBVar::GetNodeVar(symbol);
		if (symbol->getQualifier() == EvqIn) {
			msg() << "input param[" << var.getasm() << "] needs mapping\n";
			if (functionParams.find(var.getasm()) != functionParams.end()) {
				var = functionParams.find(var.getasm())->second;
				msg() << "  mapped to " + var.getasm() + "\n";
			}
		}
		return nodeResult(var);
	}

	// Unary node
	TIntermUnary* unary = node->getAsUnaryNode();
	if (unary) {
		for (unsigned int i = 0; i < sizeof(unaryNodeHandlers)/sizeof(unaryNodeHandlers[0]); ++i) {
			if (unary->getOp() == unaryNodeHandlers[i].op) {
				return unaryNodeHandlers[i].function(unary);
			}
		}
		// TODO: handle
		nodetype = "unary node " + getopname(unary->getOp());
	}

	// Binary node
	TIntermBinary* binary = node->getAsBinaryNode();
	if (binary) {
		for (unsigned int i = 0; i < sizeof(binaryNodeHandlers)/sizeof(binaryNodeHandlers[0]); ++i) {
			if (binary->getOp() == binaryNodeHandlers[i].op) {
				return binaryNodeHandlers[i].function(binary);
			}
		}
		// TODO: handle
		nodetype = "binary node " + getopname(binary->getOp());
	}

	// Aggregate node
	TIntermAggregate* aggregate = node->getAsAggregate();
	if (aggregate) {
		for (unsigned int i = 0; i < sizeof(aggregateNodeHandlers)/sizeof(aggregateNodeHandlers[0]); ++i) {
			if (aggregate->getOp() == aggregateNodeHandlers[i].op) {
				return aggregateNodeHandlers[i].function(aggregate);
			}
		}
		if (functions.count(aggregate->getName().c_str())) {
			msg() << "Calling function " << aggregate->getName().c_str() << "\n";
			TIntermSequence seq = aggregate->getSequence();
			ARBFunction& function = functions[aggregate->getName().c_str()];
			if (seq.size() == function.parameters.size()) {
				ARBInstructionList list;
				for (unsigned int i = 0; i < seq.size(); ++i) {
					nodeResult res = parseNode(seq[i]);
					if (!res.fullyParsed) {
						failmsg() << "  parameter " << i << " not fully parsed\n";
						return nodeResult(false);
					}
					list.append(ARBAssignment(function.parameters[i], res.var));
				}
				list.append(function.instructions);
				if (function.hasReturnValue) {
					ARBVar result = ARBVar::NewTempVar();
					list.append(ARBAssignment(result, function.result));
					return nodeResult(result, list);
				} else {
					return nodeResult(list);
				}
			} else {
				failmsg() << "  function takes " << function.parameters.size() << " parameters, called with " << seq.size() << " parameters\n";
				nodetype = string("aggregate ") + aggregate->getName().c_str() + " " + getopname(aggregate->getOp());
			}
		} else {
			// TODO: handle
			nodetype = string("aggregate ") + aggregate->getName().c_str() + " " + getopname(aggregate->getOp());
		}
	}

	// Constant union node
	TIntermConstantUnion* cu = node->getAsConstantUnion();
	if (cu) {
		return nodeResult(ARBVar::GetNodeVar(cu));
	}

	// Selection node
	TIntermSelection* selection = node->getAsSelectionNode();
	if (selection) {
		// TODO: handle
		nodetype = "selection";
	}

	// Branching node
	TIntermBranch* branch = node->getAsBranchNode();
	if (branch) {
		if (branch->getFlowOp() == EOpReturn) {
			if (branch->getExpression()) {
				msg() << "Returning expression\n";
				functionReturnValue = parseNode(branch->getExpression());
				return nodeResult(ARBInstructionList());
			} else {
				msg() << "Returning without expression\n";
				functionReturnValue = nodeResult(false);
				return nodeResult(ARBInstructionList());
			}
		}
		// TODO: handle
		nodetype = "branch";
	}

	// Failed to handle node, print error message
	failmsg() << " Failed to parse " << nodetype << " on line " << node->getLine() << "\n";

	return nodeResult(false);
}

// Returns the function sequence from a given function definition node
TIntermAggregate* getFunctionSequence(TIntermAggregate* node) {
	TIntermSequence seq = node->getSequence();
	for (unsigned int i = 0; i < seq.size(); ++i) {
		TIntermAggregate* aggregate = seq[i]->getAsAggregate();
		if (aggregate && aggregate->getOp() == EOpSequence) {
			return aggregate;
		}
	}
	return NULL;
}

// Registers built-in functions for the function dispatcher
void addBuiltInFunctions(map<string, ARBFunction>& functions) {
	for (unsigned int i = 0; i < sizeof(builtInFunctionHandlers)/sizeof(builtInFunctionHandlers[0]); ++i) {
		ARBBuiltInFunctionHandler& handler = builtInFunctionHandlers[i];
		ARBFunction function;
		function.name = handler.name;
		if (handler.hasResult) {
			function.result = ARBVar::NewTempVar();
			function.hasReturnValue = true;
		} else {
			function.hasReturnValue = false;
		}
		for (int j = 0; j < handler.inputParameters; ++j) {
			function.parameters.push_back(ARBVar::NewTempVar());
		}
		msg() << "Calling " << function.result.tostr() << " = " + function.name + " (" << function.parameters.size() << " parameters)\n";
		function.instructions = handler.function(function.result, function.parameters);
		for (unsigned int j = 0; j < function.instructions.size(); ++j) {
			msg() << "  " << function.instructions[j].genasm() << "\n";
		}
		functions[function.name] = function;
	}
}

// Compiles functions while traversing the tree
bool ARBHandleAggregate(bool previsit, TIntermAggregate* node, TIntermTraverser* it) {
	// Only interested in function definitions
	if (node->getOp() == EOpFunction) {
		ARBFunction function;
		msg() << "  function def (" << getopname(node->getOp()) << ") (" << node->getTypePointer()->getCompleteString() << ") [" << node->getName() << "])\n";
		function.name = node->getName().c_str();

		msg() << "    parameters:\n";
		functionParams.clear();
		functionReturnValue = nodeResult(false);
		TIntermAggregate* params = getParameterList(node);
		if (params) {
			TIntermSequence seq = params->getSequence();
			msg() << " " << seq.size() << " parameters\n";
			if (seq.size()) {
				for (unsigned int i = 0; i < seq.size(); ++i) {
					ARBVar var = ARBVar::NewTempVar();
					TIntermTyped* typed = seq[i]->getAsTyped();
					if (typed) {
						string asmname = ARBVar::GetNodeVar(typed).getasm();
						msg() << "param " << typed->getQualifierString() << " " << asmname << "\n";
						if (typed->getQualifier() == EvqIn) {
							functionParams.insert(pair<string, ARBVar>(asmname, var));
						}
					} else {
						msg() << "nontypedparam, wtf :o\n";
					}
					function.parameters.push_back(var);
				}
			}
		} else {
			msg() << " no parameter list\n";
		}

		msg() << "    sequence:\n";
		TIntermAggregate* seqagg = getFunctionSequence(node);
		if (seqagg) {
			TIntermSequence seq = seqagg->getSequence();

			ARBInstructionList list;
			for (unsigned int i = 0; i < seq.size(); ++i) {
				nodeResult res = parseNode(seq[i]);
				if (res.fullyParsed) {
					list.append(res.list);
				} else {
					failmsg() << "Failed to parse line " << seq[i]->getLine() << "\n";
					return false;
				}
			}
			if (functionReturnValue.hasResultVar) {
				function.hasReturnValue = true;
				list.append(functionReturnValue.list);
				function.result = functionReturnValue.var;
			} else {
				function.hasReturnValue = false;
			}
			function.instructions = list;
			functions[function.name] = function;
			msg() << " instruction list:\n";
			for (unsigned int i = 0; i < list.size(); ++i) {
				msg() << list[i].genasm() << "\n";
			}
			msg() << " ---\n";
		} else {
			msg() << " no function sequence\n";
		}
		msg() << "\n";
		return false;
	} else {
		return true;
	}
}

//  Generate code from the given parse tree
bool ARBCompiler::compile(TIntermNode* root) {
	setInfoSink(infoSink, debugOptions);

	msg() << "Code parsed correctly, now compiling...\n";

	if (root == 0)
		return false;

	functions.clear();
	addBuiltInFunctions(functions);

	// Use traversal functions to find function declarations
	TIntermTraverser it;
	it.visitAggregate = ARBHandleAggregate;
	root->traverse(&it);

	for (map<string, ARBFunction>::const_iterator i = functions.begin(); i != functions.end(); ++i) {
		msg() << "Function \"" << i->first << "\" (" << i->second.instructions.size() << " instructions)\n";
	}

	if (functions.count("main(")) {
		ARBInstructionList instructions = functions["main("].instructions;
		ARBCodeOptimization(instructions);
		map<pair<int, string>, ARBVar> variables;
		for (unsigned int i = 0; i < instructions.size(); ++i) {
			ARBVar output = instructions[i].getOutputVar();
			variables.insert(pair<pair<int, string>, ARBVar>(pair<int, string>(output.getasmvartype(), output.getasm()), output));
			const vector<ARBVar>& input = instructions[i].getInputVarList();
			for (unsigned int j = 0; j < input.size(); ++j) {
				variables.insert(pair<pair<int, string>, ARBVar>(pair<int, string>(input[j].getasmvartype(), input[j].getasm()), input[j]));
			}
		}

		if (language == EShLangVertex) {
			asmout() << "!!ARBvp1.0\n\n";
		} else if (language == EShLangFragment) {
			asmout() << "!!ARBfp1.0\n\n";
		}
		resetVarNumbering();
		string padding = "";
		string outputAssignments = "";
		for (map<pair<int, string>, ARBVar>::const_iterator i = variables.begin(); i != variables.end(); ++i) {
			outputAssignments += i->second.getARBOutputAssignment();
			string decl = i->second.getARBVarDeclaration();
			if (decl != "") {
				asmout() << decl << "\n";
				padding = "\n";
			}
		}

		asmout() << padding;

		for (unsigned int i = 0; i < instructions.size(); ++i) {
			asmout() << instructions[i].genasm() << "\n";
		}
		asmout() << outputAssignments;
		asmout() << "END\n";
	} else {
		failmsg() << "main() function not found\n";
	}

	cleanUpMemory();

	haveValidObjectCode = false;
	return haveValidObjectCode;
}
