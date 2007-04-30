#ifndef EGL_PIPELINE_PART_H
#define EGL_PIPELINE_PART_H 1

// ==========================================================================
//
// PipelinePart.h		JIT Class for 3D Rendering Library
//
//						An instance of PipelinePart represents a part of
//						the rendering pipeline that can be generated at
//						runtime using the JIT.
//
//						The individual sub-classes for the different types of
//						pipeline parts determine:
//
//						- actual code generation of the part
//						- comparison of relevant state information
//						- copying of relevant state information into
//						  the function cache
//
// --------------------------------------------------------------------------
//
// 04-24-2006	Hans-Martin Will	initial version
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


extern "C" {
	// ----------------------------------------------------------------------
	// Data types that are defined in the codegen library
	// ----------------------------------------------------------------------
#ifdef __GCC32__
    struct cg_module_t;
    struct cg_proc_t;
    struct cg_block_t;
    struct cg_block_ref_t;
    struct cg_virtual_reg_t;
#else
	typedef struct cg_module_t cg_module_t;
	typedef struct cg_proc_t cg_proc_t;
	typedef struct cg_block_t cg_block_t;
	typedef struct cg_block_ref_t cg_block_ref_t;
	typedef struct cg_virtual_reg_t cg_virtual_reg_t;
#endif
}


namespace EGL {

	class FunctionCache;
	struct VaryingInfo;
	
	class PipelinePart {
	public:
		enum Part {
			PartInvalid,
			PartRasterPoint,
			PartRasterLine,
			PartRasterBlockDepthStencil,
			PartRasterBlockEdgeDepthStencil,
			PartRasterBlockColorAlpha,
			PartFetchVertex
		};
			
		virtual void CopyState(void * target, const void * source) const = 0;
		virtual bool CompareState(const void * first, const void * second) const = 0;
		virtual void Compile(FunctionCache * target, const VaryingInfo * varyingInfo, const void * state) = 0;
		virtual Part GetPart() const = 0;
		
		static PipelinePart & Get(Part part);
		
	protected:
		// Common code generator stuff goes here
		void BeginGenerateCode();
		void EndGenerateCode(FunctionCache * target, const void * state);
		
		struct cg_module_t *m_Module;
	};
}

#endif // ndef EGL_PIPELINE_PART_H */