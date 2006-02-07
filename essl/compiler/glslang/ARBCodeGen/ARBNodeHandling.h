#include "../Include/intermediate.h"

#include "ARBBackEnd.h"

#include <string>
#include <vector>

// Node handlers, compile a node into the resulting variable and the instructions used to compute it
nodeResult ARBMathNode(TIntermBinary* node);
nodeResult ARBUnaryMathNode(TIntermUnary* node);
nodeResult ARBMathFunctionNode(TIntermAggregate* node);
nodeResult ARBAssignmentNode(TIntermBinary* node);
nodeResult ARBMatrixMathNode(TIntermBinary* node);
nodeResult ARBSwizzleNode(TIntermBinary* node);
nodeResult ARBSequenceNode(TIntermAggregate* node);
nodeResult ARBConstructVectorNode(TIntermAggregate* node);

// Stores mapping between operators and unary node handler functions
typedef struct {
	TOperator op;
	nodeResult (*function)(TIntermUnary*);
} ARBUnaryNodeHandler;

// Stores mapping between operators and binary node handler functions
typedef struct {
	TOperator op;
	nodeResult (*function)(TIntermBinary*);
} ARBBinaryNodeHandler;

// Stores mapping between operators and aggregate node handler functions
typedef struct {
	TOperator op;
	nodeResult (*function)(TIntermAggregate*);
} ARBAggregateNodeHandler;

// Stores information about available built-in functions
typedef struct {
	std::string name;
	bool hasResult;
	int inputParameters;
	ARBInstructionList (*function)(ARBVar result, std::vector<ARBVar>& parameters);
} ARBBuiltInFunctionHandler;
