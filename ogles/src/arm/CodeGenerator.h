#ifndef EGL_CONTEXT_H
#define EGL_CONTEXT_H 1

#pragma once

// ==========================================================================
//
// CodeGenerator.h		JIT Class for OpenGL (R) ES Implementation
//
//						This file contains the rasterizer functions that
//						implement the runtime code generation support
//						for optimized scan line rasterization routines.
//
// --------------------------------------------------------------------------
//
// 08-07-2003	Hans-Martin Will	initial version
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


#include "OGLES.h"
#include "fixed.h"
#include "linalg.h"
#include "RasterizerState.h"
#include "FractionalColor.h"
#include <map>

namespace EGL {

	namespace triVM {
		union Label;
		struct InstructionBaseType;
		struct Block;
		struct Procedure;
		struct Module;
	}

	struct FragmentGenerationInfo;

	class MultiTexture;

	typedef std::map<int, triVM::InstructionBaseType *> RegisterDefinitionMap;

	class CodeGenerator {

	public:
		// ----------------------------------------------------------------------
		// Code generation of triangle scan line
		// ----------------------------------------------------------------------
		void CompileRasterScanLine();

		void SetState(RasterizerState * state)	{ m_State = state; }
		RasterizerState * GetState()			{ return m_State; }

		void SetTexture(MultiTexture * texture)	{ m_Texture = texture; }
		MultiTexture * GetTexture()				{ return m_Texture; }

	private:
		void GenerateRasterScanLine();
		void GenerateFragment(triVM::Procedure * procedure, triVM::Block & currentBlock,
			triVM::Label * continuation, I32 & nextRegister,
			FragmentGenerationInfo & fragmentInfo);

		// to do: make all code generation functions members of this class
		// code generation should be done into a CodeSegment
		void RemoveUnusedCode(triVM::Module * module);

		RegisterDefinitionMap * FindDefinitions(triVM::Module * module);
		void SelectAddressingModes(triVM::Module * module, RegisterDefinitionMap * defs);
		void DumpModule(FILE * out, EGL::triVM::Module * module);
		void DumpModule(const char * filename, EGL::triVM::Module * module);
		void PerformDataFlowAnalysis(triVM::Module * module);
		void AllocateGlobalRegisters(triVM::Module * module);
		void EmitCode(triVM::Module * module);


	private:
		RasterizerState *	m_State;
		MultiTexture *		m_Texture;
		triVM::Module *		m_Module;
	};

}

#endif //ndef EGL_CONTEXT_H
