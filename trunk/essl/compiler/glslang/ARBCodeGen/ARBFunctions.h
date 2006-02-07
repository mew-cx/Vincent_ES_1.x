#include "ARBInstruction.h"
#include "ARBBackEnd.h"
#include <vector>

using namespace std;

//operators
ARBInstructionList ARBAddition(ARBVar result, ARBVar vec1, ARBVar vec2);
ARBInstructionList ARBSubtraction(ARBVar result, ARBVar vec1, ARBVar vec2);
ARBInstructionList ARBMultiplication(ARBVar result, ARBVar vec1, ARBVar vec2);
ARBInstructionList ARBAssignment(ARBVar result, ARBVar vector);
//math operations
ARBInstructionList ARBMinimum(ARBVar result, ARBVar vec1, ARBVar vec2);
ARBInstructionList ARBMaximum(ARBVar result, ARBVar vec1, ARBVar vec2);
ARBInstructionList ARBClamp(ARBVar result, ARBVar vec1, ARBVar minVec, ARBVar maxVec);
ARBInstructionList ARBAbsolute(ARBVar result, ARBVar vector);
ARBInstructionList ARBSquareRoot(ARBVar result, ARBVar vector);
ARBInstructionList ARBPower(ARBVar result, ARBVar vec1, ARBVar vec2);
//geometric operations
ARBInstructionList ARBLength(ARBVar result, ARBVar vector, bool component3);
ARBInstructionList ARBDistance(ARBVar result, ARBVar vec1, ARBVar vec2, bool component3);
ARBInstructionList ARBDot(ARBVar result, ARBVar vec1, ARBVar vec2, bool component3);
ARBInstructionList ARB3ComponentNormalize(ARBVar result, ARBVar vector);
ARBInstructionList ARB4ComponentNormalize(ARBVar result, ARBVar vector);

ARBInstructionList ARBMatrixTimesVector(ARBVar result, ARBVar vec, ARBVar row0, ARBVar row1, ARBVar row2, ARBVar row3);
// Built-in functions
ARBInstructionList ARBTexture2D(ARBVar result, vector<ARBVar>& parameters);
