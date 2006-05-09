// ==========================================================================
//
// PipelinePart.cpp		JIT Class for 3D Rendering Library
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



#include "stdafx.h"
#include "PipelinePart.h"
#include "inline.h"
#include "FunctionCache.h"
#include "FetchVertexPart.h"
#include "RasterLinePart.h"
#include "RasterPointPart.h"
#include "RasterTriangleColorAlphaPart.h"
#include "RasterTriangleDepthStencilPart.h"
#include "RasterTriangleEdgeDepthStencilPart.h"


using namespace EGL;


namespace {
	// ----------------------------------------------------------------------
	// Dump the current intermediate code to the given temporary file
	// ----------------------------------------------------------------------
	
	void Dump(const char * filename, cg_module_t * module)
	{
		FILE * fp = fopen(filename, "w");
		cg_module_dump(module, fp);
		fclose(fp);
	}
}

void PipelinePart::BeginGenerateCode() {
	cg_heap_t * heap = cg_heap_create(4096);
	cg_module_t * module = cg_module_create(heap);

	m_Module = module;
}

void PipelinePart::EndGenerateCode(FunctionCache * target, const void * state) {
#ifdef DEBUG
	Dump("dump1.txt", m_Module);
#endif

	cg_module_inst_def(m_Module);
	cg_module_amode(m_Module);

#ifdef DEBUG
	Dump("dump2.txt", m_Module);
#endif

	cg_module_eliminate_dead_code(m_Module);

#ifdef DEBUG
	Dump("dump3.txt", m_Module);
#endif

	cg_module_unify_registers(m_Module);
	cg_module_allocate_variables(m_Module);
	cg_module_inst_use_chains(m_Module);
	//cg_module_reorder_instructions(m_Module);

#ifdef DEBUG
	Dump("dump35.txt", m_Module);
#endif

	cg_module_dataflow(m_Module);
	cg_module_interferences(m_Module);

#ifdef DEBUG
	Dump("dump4.txt", m_Module);
#endif

	cg_runtime_info_t runtime; 
	memset(&runtime, 0, sizeof runtime);

	runtime.div = div;

	runtime.div_HP_16_32s = EGL_Div;
	runtime.div_LP_16_32s = EGL_Div;
	runtime.inv_HP_16_32s = EGL_Inverse;
	runtime.inv_LP_16_32s = EGL_Inverse;
	runtime.inv_sqrt_HP_16_32s = EGL_InvSqrt;
	runtime.inv_sqrt_LP_16_32s = EGL_InvSqrt;
	runtime.sqrt_HP_16_32s = EGL_Sqrt;
	runtime.sqrt_LP_16_32s = EGL_Sqrt;
	runtime.convert_float = (I32 (*)(I32)) EGL_FixedFromFloat;

	cg_processor_info_t processor;

#ifdef EGL_XSCALE
	processor.useV5 = 1;
#else
	processor.useV5 = 0;
#endif

	cg_codegen_t * codegen = cg_codegen_create(m_Module->heap, &runtime, &processor);
	cg_codegen_emit_module(codegen, m_Module);
	cg_codegen_fix_refs(codegen);

	cg_segment_t * cseg = cg_codegen_segment(codegen);

#ifdef DEBUG
	ARMDis dis;
	armdis_init(&dis);
	armdis_dump(&dis, "dump5.txt", cseg);
#endif

	void * targetBuffer = 
		target->BeginAddFunction(GetPart(), state, cg_segment_size(cseg));

	cg_segment_get_block(cseg, 0, targetBuffer, cg_segment_size(cseg));

	target->EndAddFunction(targetBuffer, cg_segment_size(cseg));
	
	cg_codegen_destroy(codegen);
	cg_heap_destroy(m_Module->heap);
}


namespace {
	FetchVertexPart						fetchVertexPart;
	RasterLinePart						rasterLinePart;
	RasterPointPart						rasterPointPart;
	RasterTriangleColorAlphaPart		rasterTriangleColorAlphaPart;
	RasterTriangleDepthStencilPart		rasterTriangleDepthStencilPart;
	RasterTriangleEdgeDepthStencilPart	rasterTriangleEdgeDepthStencilPart;
}


PipelinePart & PipelinePart :: Get(Part part) {
	switch (part) {
	default:								assert(0);
	case PartRasterPoint:					return rasterPointPart;
	case PartRasterLine:					return rasterLinePart;
	case PartRasterBlockDepthStencil:		return rasterTriangleDepthStencilPart;
	case PartRasterBlockEdgeDepthStencil:	return rasterTriangleEdgeDepthStencilPart;
	case PartRasterBlockColorAlpha:			return rasterTriangleColorAlphaPart;
	case PartFetchVertex:					return fetchVertexPart;
	}
}


