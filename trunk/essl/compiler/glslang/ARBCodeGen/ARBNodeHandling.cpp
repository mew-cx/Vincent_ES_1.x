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
#include "ARBFunctions.h"
#include "Util.h"

#include "ARBNodeHandling.h"

using namespace std;

// Handles unary math operations like abs, sqrt
nodeResult ARBUnaryMathNode(TIntermUnary* node) {
	nodeResult operand = parseNode(node->getOperand());

	if(operand.fullyParsed) {
		ARBVar result = ARBVar::NewTempVar();
		ARBInstructionList list;
		list.append(operand.list);

		switch(node->getOp()) {
			case EOpAbs: {
				list.append(ARBAbsolute(result, operand.var));
				break;
			}
			case EOpSqrt: {
				list.append(ARBSquareRoot(result, operand.var));
				break;
			}
			case EOpLength: {
				if(node->getSize() == 3) {
					list.append(ARBLength(result, operand.var, true));
				} else {
					list.append(ARBLength(result, operand.var, false));
				}
				break;
			}
			case EOpNormalize: {
				if(node->getSize() == 3) {
					list.append(ARB3ComponentNormalize(result, operand.var));
				} else {
					list.append(ARB4ComponentNormalize(result, operand.var));
				}
				break;
			}
			default: {
		        failmsg() << "Error in ARBUnaryMathNode, uknown operator " << getopname(node->getOp()) << "\n";
				return nodeResult(false);
			}
		}
		msg() << " " << getopname(node->getOp()) << operand.var.tostr() << "\n";
		return nodeResult(result, list);
	}
	else {
		failmsg() << " unhandled function call to " << getopname(node->getOp()) << " on line " << node->getLine() << "\n";
		failmsg() << "   operand: " << operand.var.tostr() << " { " << (operand.fullyParsed?"parsed":"not parsed") << " }\n";
		failmsg() << "\n";
		return nodeResult(false);
	}
}

// Handles binary math operators +-* e.g. a + b
nodeResult ARBMathNode(TIntermBinary* node) {
	// Generate result variable and instruction list from children
	nodeResult left = parseNode(node->getLeft());
	nodeResult right = parseNode(node->getRight());

	if (left.fullyParsed && right.fullyParsed) {
		ARBVar result = ARBVar::NewTempVar();

		ARBInstructionList list;
		// Include instructions for generating children
		list.append(left.list);
		list.append(right.list);
		// Add instruction for addition

		switch(node->getOp()) {
			case EOpAdd:
				list.append(ARBAddition(result, left.var, right.var));
				break;
			case EOpSub:
				list.append(ARBSubtraction(result, left.var, right.var));
				break;
			case EOpMul:
				list.append(ARBMultiplication(result, left.var, right.var));
				break;
			case EOpVectorTimesScalar:
				list.append(ARBMultiplication(result, left.var, right.var));
				break;
			default:
				failmsg() << "Error in ARBMathNode, uknown operator " << getopname(node->getOp()) << "\n";
				return nodeResult(false);
				break;
		}

		msg() << " " << getopname(node->getOp()) << ", " << result.tostr() << " = " << left.var.tostr() << " op " << right.var.tostr() << "\n";
		return nodeResult(result, list);
	} else {
		failmsg() << " unhandled " << getopname(node->getOp()) << " , [" << ARBVar::GetNodeVar(node->getLeft()).tostr() << "] + [" << ARBVar::GetNodeVar(node->getRight()).tostr() << "] on line " << node->getLine() << "\n";
		failmsg() << "   left: " << left.var.tostr() << " { " << (left.fullyParsed?"parsed":"not parsed") << " }\n";
		failmsg() << "   right: " << right.var.tostr() << " { " << (right.fullyParsed?"parsed":"not parsed") << " }\n";
		failmsg() << "\n";
		return nodeResult(false);
	}
}

// Handles different math operations like min, max
nodeResult ARBMathFunctionNode(TIntermAggregate* node) {
	// Generate result variable and instruction list from children

	ARBVar result = ARBVar::NewTempVar();

	//get the parameters
	TIntermSequence& seq = node->getSequence();

	ARBInstructionList list;

	// grouping into number of parametres

	switch(seq.size()) {
		case 2: {
			nodeResult param1 = parseNode(seq[0]);
			nodeResult param2 = parseNode(seq[1]);

			if(param1.fullyParsed && param2.fullyParsed) {

				if(node->getOp() == EOpMin || node->getOp() == EOpMax) {

					TIntermTyped* param2typed = seq[1]->getAsTyped();
					if(param2typed) {
						ARBVar var2 = param2.var;

						if (param2typed->getSize() == 1) { // param2 is a float, must be copied to all vector components
							var2 = param2.var.swizzle("x");
						}

						list.append(param1.list);
						list.append(param2.list);

						if(node->getOp() == EOpMin) {
							list.append(ARBMinimum(result, param1.var, var2));
						}
						else if(node->getOp() == EOpMax) {
							list.append(ARBMaximum(result, param1.var, var2));
						}
						msg() << " " << getopname(node->getOp()) << ", " << param1.var.tostr() << " " << var2.tostr() << "\n";
						return nodeResult(result, list);
					}
				}
				else if (node->getOp() == EOpPow) {
					list.append(param1.list);
					list.append(param2.list);
					list.append(ARBPower(result, param1.var, param2.var));

					msg() << " " << getopname(node->getOp()) << ", " << param1.var.tostr() << " " << param2.var.tostr() << "\n";
					return nodeResult(result, list);
				}
				else if (node->getOp() == EOpDot) {
					list.append(param1.list);
					list.append(param2.list);

					TIntermTyped* typed = seq[0]->getAsTyped();
					if (!typed) {
						failmsg() << "First parameter node to dot() not a typed value!\n";
					}
					if (typed->getSize() == 3) {
						list.append(ARBDot(result, param1.var, param2.var, true));
					}
					else {
						list.append(ARBDot(result, param1.var, param2.var, false));
					}

					msg() << " " << getopname(node->getOp()) << ", " << param1.var.tostr() << " " << param2.var.tostr() << "\n";
					return nodeResult(result, list);
				}
				else if (node->getOp() == EOpDistance) {
					list.append(param1.list);
					list.append(param2.list);

					TIntermTyped* typed = seq[0]->getAsTyped();
					if (!typed) {
						failmsg() << "First parameter node to distance() not a typed value!\n";
						return nodeResult(false);
					}
					if (typed->getSize() == 3) {
						list.append(ARBDistance(result, param1.var, param2.var, true));
					}
					else {
						list.append(ARBDistance(result, param1.var, param2.var, false));
					}

					msg() << " " << getopname(node->getOp()) << ", " << param1.var.tostr() << " " << param2.var.tostr() << "\n";
					return nodeResult(result, list);
				}
				failmsg() << " unhandled function call " << getopname(node->getOp()) << " on line " << node->getLine() << "\n";
				failmsg() << "   first param: " << param1.var.tostr() << " { " << (param1.fullyParsed?"parsed":"not parsed") << " }\n";
				failmsg() << "   second param: " << param2.var.tostr() << " { " << (param2.fullyParsed?"parsed":"not parsed") << " }\n";
				failmsg() << "\n";
				return nodeResult(false);
				}
				break;
			}
			case 3: {
				nodeResult param1 = parseNode(seq[0]);
				nodeResult param2 = parseNode(seq[1]);
				nodeResult param3 = parseNode(seq[2]);

				if(param1.fullyParsed && param2.fullyParsed && param3.fullyParsed) {

					if(node->getOp() == EOpClamp) {

						TIntermTyped* param2typed = seq[1]->getAsTyped();
						TIntermTyped* param3typed = seq[2]->getAsTyped();

						if(param2typed && param3typed) {
							ARBVar var2 = param2.var;
							ARBVar var3 = param3.var;

							if (param2typed->getSize() == 1 && param3typed->getSize() == 1) { // param2  and param3 is a float, must be copied to all vector components
								var2 = param2.var.swizzle("x");
								var3 = param3.var.swizzle("x");
							}

							list.append(param1.list);
							list.append(param2.list);
							list.append(param3.list);

							list.append(ARBClamp(result, param1.var, var2, var3));

							msg() << " " << getopname(node->getOp()) << ", " << param1.var.tostr() << " " << var2.tostr() << " " << var3.tostr() << "\n";
							return nodeResult(result, list);
						}
					}
				} else {
					failmsg() << " unhandled function call " << getopname(node->getOp()) << " on line " << node->getLine() << "\n";
					failmsg() << "   first param: " << param1.var.tostr() << " { " << (param1.fullyParsed?"parsed":"not parsed") << " }\n";
					failmsg() << "   second param: " << param2.var.tostr() << " { " << (param2.fullyParsed?"parsed":"not parsed") << " }\n";
					failmsg() << "   third param: " << param3.var.tostr() << " { " << (param3.fullyParsed?"parsed":"not parsed") << " }\n";
					failmsg() << "\n";
					return nodeResult(false);
				}
				break;
			}
			default:
                failmsg() << "Error in ARBMathNode, uknown operator " << getopname(node->getOp()) << "\n";
				return nodeResult(false);
		}
		return nodeResult(false);
}


// Handles assignment, i.e. a = b
nodeResult ARBAssignmentNode(TIntermBinary* node) {
	nodeResult left = parseNode(node->getLeft());
	nodeResult right = parseNode(node->getRight());

	if (left.fullyParsed && right.fullyParsed) {
		ARBInstructionList list;

		list.append(left.list);
		list.append(right.list);
		list.append(ARBAssignment(left.var, right.var));

		msg() << " Assignment, " << left.var.tostr() << " = " << right.var.tostr() << "\n";
		return nodeResult(list);
	} else {
		failmsg() << " unhandled assignment, [" << ARBVar::GetNodeVar(node->getLeft()).tostr() << "] = [" << ARBVar::GetNodeVar(node->getRight()).tostr() << "] on line " << node->getLine() << "\n";
		failmsg() << "   left: " << left.var.tostr() << " { " << (left.fullyParsed?"parsed":"not parsed") << " }\n";
		failmsg() << "   right: " << right.var.tostr() << " { " << (right.fullyParsed?"parsed":"not parsed") << " }\n";
		failmsg() << "\n";
		return nodeResult(false);
	}
}

// Handles matrix math operators, e.g. *
nodeResult ARBMatrixMathNode(TIntermBinary* node) {
	switch (node->getOp()) {
		case EOpMatrixTimesVector: {
			ARBVar result = ARBVar::NewTempVar();
			ARBVar matrix = ARBVar::GetNodeVar(node->getLeft());
			nodeResult right = parseNode(node->getRight());

			ARBVar row0 = matrix.getMatrixCol(0);
			ARBVar row1 = matrix.getMatrixCol(1);
			ARBVar row2 = matrix.getMatrixCol(2);
			ARBVar row3 = matrix.getMatrixCol(3);

			ARBInstructionList list;
			list.append(right.list);
			list.append(ARBMatrixTimesVector(result, right.var, row0, row1, row2, row3));

			msg() << "Matrix times vector -> " << result.tostr() << "\n";
			return nodeResult(result, list);
		}
		default:
			failmsg() << "Matrix math operator " << getopname(node->getOp()) << " not handled yet\n\n";
			return nodeResult(false);
	}
}

// Handles sequence of nodes by compiling each of them
nodeResult ARBSequenceNode(TIntermAggregate* node) {
	TIntermSequence& seq = node->getSequence();
	if (seq.size()) {
		ARBVar result = ARBVar::None;
		ARBInstructionList list;
		for (unsigned int i = 0; i < seq.size(); ++i) {
			nodeResult res = parseNode(seq[i]);
			list.append(res.list);
			result = res.var;
		}
		return nodeResult(result, list);
	} else {
		return nodeResult(ARBInstructionList());
	}
}

// Handles vector swizzling, e.g. var.yyx
nodeResult ARBSwizzleNode(TIntermBinary* node) {
	nodeResult res = parseNode(node->getLeft());
	TIntermAggregate* right = node->getRight()->getAsAggregate();
	if (res.fullyParsed && right) {
		TIntermSequence& seq = right->getSequence();
		string swizzle = "";
		for (unsigned int i = 0; i < seq.size(); ++i) {
			TIntermConstantUnion* cu = seq[i]->getAsConstantUnion();
			if (cu && cu->getBasicType() == EbtInt && cu->getNominalSize() == 1) {
				int i = cu->getUnionArrayPointer()[0].getIConst();
				if (i >= 0 && i <= 4) {
					swizzle += ARBVar::swizzleChars[i];
				} else {
					failmsg() << "Swizzling element " << i << " not possible\n";
				}
			} else {
				failmsg() << "Swizzling " << res.var.tostr() << " failed\n";
				return nodeResult(false);
			}
		}
		ARBVar var = res.var.swizzle(swizzle);
		return nodeResult(var, res.list);
	}
	return nodeResult(false);
}

// Handles vector constructors, e.g. vec4(var, 1.0)
nodeResult ARBConstructVectorNode(TIntermAggregate* node) {
	if (node->getBasicType() != EbtFloat) { //TODO: implementer
		failmsg() << "Non-float vector constructors not implemented\n";
		return nodeResult(false);
	}
	TIntermSequence& seq = node->getSequence();
	ARBVar var = ARBVar::NewTempVar();
	ARBInstructionList list;
	int elementsFilled = 0;
	for (unsigned int i = 0; i < seq.size(); ++i) {
		TIntermTyped* typed = seq[i]->getAsTyped();
		if (typed) {
			string swizzle = "";
			for (int j = 0; j < typed->getNominalSize(); ++j) {
				swizzle += ARBVar::swizzleChars[elementsFilled++];
			}
			msg() << "Param " << i << " size: " << typed->getNominalSize() << " (" << swizzle << ")\n";
			nodeResult res = parseNode(typed);
			list.append(res.list);
			list.append(ARBAssignment(var.swizzle(swizzle), res.var));
		} else {
			failmsg() << "Non-typed node used as input to constructor\n";
			return nodeResult(false);
		}
	}
	return nodeResult(var, list);
}
