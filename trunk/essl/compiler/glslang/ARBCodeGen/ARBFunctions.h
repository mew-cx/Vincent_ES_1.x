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
