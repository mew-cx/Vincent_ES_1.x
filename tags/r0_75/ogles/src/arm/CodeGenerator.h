#ifndef EGL_CODE_GENERATOR_H
#define EGL_CODE_GENERATOR_H 1

#pragma once

// ==========================================================================
//
// CodeGenerator.h		JIT Class for 3D Rendering Library
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
#include "Rasterizer.h"
#include "RasterizerState.h"
#include <map>


extern "C" {
	typedef struct cg_module_t cg_module_t;
	typedef struct cg_proc_t cg_proc_t;
	typedef struct cg_block_t cg_block_t;
	typedef struct cg_block_ref_t cg_block_ref_t;
}


namespace EGL {

	struct FragmentGenerationInfo;
	class FunctionCache;

	class MultiTexture;

	class CodeGenerator {

	public:
		// ----------------------------------------------------------------------
		// Code generation of triangle scan line
		// ----------------------------------------------------------------------
		void CompileRasterScanLine(FunctionCache * target);

		void SetState(const RasterizerState * state)	{ m_State = state; }
		const RasterizerState * GetState()				{ return m_State; }

	private:
		void GenerateRasterScanLine();
		void GenerateFragment(cg_proc_t * procedure, cg_block_t * currentBlock,
			cg_block_ref_t * continuation, FragmentGenerationInfo & fragmentInfo,
			int weight);

	private:
		const RasterizerState *	m_State;
		struct cg_module_t *m_Module;
	};

}

#endif //ndef EGL_CODE_GENERATOR_H
