#ifndef EGL_TRANSFORM_H
#define EGL_TRANSFORM_H 1

#pragma once

// ==========================================================================
//
// Transform	Sweeps for IL for OpenGL (R) ES Implementation
//				
//				This class is part of the runtime compiler infrastructure
//				used by the OpenGL|ES implementation for compiling
//				shader code at runtime into machine language.
//
// --------------------------------------------------------------------------
//
// 01-10-2004	Hans-Martin Will	initial version
//
// --------------------------------------------------------------------------
//
// Copyright (c) 2004, Hans-Martin Will. All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are 
// met:
// 
//	 *  Redistributions of source code must retain the above copyright
// 		notice, this list of conditions and the following disclaimer. 
//   *	Redistributions in binary form must reproduce the above copyright
// 		notice, this list of conditions and the following disclaimer in the 
// 		documentation and/or other materials provided with the distribution. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
// THE POSSIBILITY OF SUCH DAMAGE.
//
// ==========================================================================


#include "Inst.h"


namespace EGL {
namespace triVM {

	// -------------------------------------------------------------------------
	// Transform	Base class for code sweeps across the intermediate
	//				language representation
	// -------------------------------------------------------------------------
	class Transform {
	public:
		virtual void sweep(triVM::Module * module);
		virtual void sweep(triVM::Procedure * procedure);
		virtual void sweep(triVM::Block * block);

		virtual void reverseSweep(triVM::Module * module);
		virtual void reverseSweep(triVM::Procedure * procedure);
		virtual void reverseSweep(triVM::Block * block);

	protected:
		virtual void begin(triVM::Module * module);
		virtual void begin(triVM::Procedure * procedure);
		virtual void begin(triVM::Block * block);

		virtual triVM::Instruction * dispatch(triVM::Instruction * instruction);

		virtual void end(triVM::Module * module);
		virtual void end(triVM::Procedure * procedure);
		virtual void end(triVM::Block * block);

	protected:
		// base formats
		virtual triVM::Instruction * transform(triVM::InstructionUnaryType * instruction) = 0;					
		virtual triVM::Instruction * transform(triVM::InstructionBinaryType * instruction) = 0;					
		virtual triVM::Instruction * transform(triVM::InstructionCompareType * instruction) = 0;				
		virtual triVM::Instruction * transform(triVM::InstructionLoadType * instruction) = 0;					
		virtual triVM::Instruction * transform(triVM::InstructionStoreType * instruction) = 0;					
		virtual triVM::Instruction * transform(triVM::InstructionLoadImmediateType * instruction) = 0;			
		virtual triVM::Instruction * transform(triVM::InstructionBranchRegType * instruction) = 0;				
		virtual triVM::Instruction * transform(triVM::InstructionBranchLabelType * instruction) = 0;			
		virtual triVM::Instruction * transform(triVM::InstructionBranchConditionallyType * instruction) = 0;	
		virtual triVM::Instruction * transform(triVM::InstructionPhiType * instruction) = 0;					
		virtual triVM::Instruction * transform(triVM::InstructionCallType * instruction) = 0;		
		virtual triVM::Instruction * transform(triVM::InstructionRetType * instruction) = 0;

		// ARM-specific formats
		virtual triVM::Instruction * transform(triVM::InstructionArmUnaryImmediateType * instruction) = 0;
		virtual triVM::Instruction * transform(triVM::InstructionArmUnaryShiftRegType * instruction) = 0;	
		virtual triVM::Instruction * transform(triVM::InstructionArmUnaryShiftImmedType * instruction) = 0;	
		virtual triVM::Instruction * transform(triVM::InstructionArmBinaryImmediateType * instruction) = 0;	
		virtual triVM::Instruction * transform(triVM::InstructionArmBinaryShiftRegType * instruction) = 0;	
		virtual triVM::Instruction * transform(triVM::InstructionArmBinaryShiftImmedType * instruction) = 0;
		virtual triVM::Instruction * transform(triVM::InstructionArmLoadImmedOffsetType * instruction) = 0;	
		virtual triVM::Instruction * transform(triVM::InstructionArmLoadRegOffsetType * instruction) = 0;	
		virtual triVM::Instruction * transform(triVM::InstructionArmStoreImmedOffsetType * instruction) = 0;	
		virtual triVM::Instruction * transform(triVM::InstructionArmStoreRegOffsetType * instruction) = 0;		
	};

} // namespace triVM
} // namespace EGL


#endif //ndef EGL_TRANSFORM_H

