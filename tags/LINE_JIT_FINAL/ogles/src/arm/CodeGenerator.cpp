// ==========================================================================
//
// CodeGenerator.cpp	JIT Class for 3D Rendering Library
//
//						This file contains the rasterizer functions that
//						implement the runtime code generation support
//						for optimized scan line rasterization routines.
//
// --------------------------------------------------------------------------
//
// 12-29-2003		Hans-Martin Will	initial version
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
#include "CodeGenerator.h"
#include "Rasterizer.h"
#include "FunctionCache.h"
#include "Surface.h"
#include "Texture.h"
#include "codegen.h"
#include "instruction.h"
#include "emit.h"
#include "arm-dis.h"


#ifdef EGL_ON_WINCE

// --------------------------------------------------------------------------
// These declarations for coredll are extracted from platform builder
// source code
// --------------------------------------------------------------------------

/* Flags for CacheSync/CacheRangeFlush */
#define CACHE_SYNC_DISCARD      0x001   /* write back & discard all cached data */
#define CACHE_SYNC_INSTRUCTIONS 0x002   /* discard all cached instructions */
#define CACHE_SYNC_WRITEBACK    0x004   /* write back but don't discard data cache*/
#define CACHE_SYNC_FLUSH_I_TLB  0x008   /* flush I-TLB */
#define CACHE_SYNC_FLUSH_D_TLB  0x010   /* flush D-TLB */
#define CACHE_SYNC_FLUSH_TLB    (CACHE_SYNC_FLUSH_I_TLB|CACHE_SYNC_FLUSH_D_TLB)    /* flush all TLB */
#define CACHE_SYNC_L2_WRITEBACK 0x020   /* write-back L2 Cache */
#define CACHE_SYNC_L2_DISCARD   0x040   /* discard L2 Cache */

#define CACHE_SYNC_ALL          0x07F   /* sync and discard everything in Cache/TLB */

extern "C" {
	void CacheSync(int flags);
	void CacheRangeFlush (LPVOID pAddr, DWORD dwLength, DWORD dwFlags);
}

#endif

using namespace EGL;



namespace {
	void Dump(const char * filename, cg_module_t * module)
	{
		FILE * fp = fopen(filename, "w");
		cg_module_dump(module, fp);
		fclose(fp);
	}

#ifndef EGL_USE_GPP

	void gppDivHP_16_32s(I32 num, I32 denom, I32* pDst) {
		*pDst = EGL_Div(num, denom);
	}

	void gppInvHP_16_32s(I32 src, I32* pDst) {
		*pDst = EGL_Inverse(src);
	}

	void gppDivLP_16_32s(I32 num, I32 denom, I32* pDst) {
		*pDst = EGL_Div(num, denom);
	}

	void gppInvLP_16_32s(I32 src, I32* pDst) {
		*pDst = EGL_Inverse(src);
	}

	void gppSqrtHP_16_32s(U32 src, U32* pDst) {
		*pDst = EGL_Sqrt(src);
	}

	void gppInvSqrtHP_16_32s(U32 src, U32* pDst) {
		*pDst = EGL_InvSqrt(src);
	}

	void gppSqrtLP_16_32s(U32 src, U32* pDst) {
		*pDst = EGL_Sqrt(src);
	}

	void gppInvSqrtLP_16_32s(U32 src, U32* pDst) {
		*pDst = EGL_InvSqrt(src);
	}

#endif

}

void CodeGenerator :: Compile(FunctionCache * target, FunctionCache::FunctionType type,
	void (CodeGenerator::*function)()) {

	cg_heap_t * heap = cg_heap_create(4096);
	cg_module_t * module = cg_module_create(heap);

	m_Module = module;

	(this->*function)();

#ifndef NDEBEUG
	Dump("dump1.txt", m_Module);
#endif

	cg_module_inst_def(m_Module);
	cg_module_amode(m_Module);

#ifndef NDEBEUG
	Dump("dump2.txt", m_Module);
#endif

	cg_module_eliminate_dead_code(m_Module);

#ifndef NDEBEUG
	Dump("dump3.txt", m_Module);
#endif

	cg_module_unify_registers(m_Module);
	cg_module_allocate_variables(m_Module);
	cg_module_inst_use_chains(m_Module);
	//cg_module_reorder_instructions(m_Module);

	Dump("dump35.txt", m_Module);

	cg_module_dataflow(m_Module);
	cg_module_interferences(m_Module);

#ifndef NDEBEUG
	Dump("dump4.txt", m_Module);
#endif

	cg_runtime_info_t runtime; 
	memset(&runtime, 0, sizeof runtime);

	runtime.div = div;

	runtime.div_HP_16_32s = gppDivHP_16_32s;
	runtime.div_LP_16_32s = gppDivLP_16_32s;
	runtime.inv_HP_16_32s = gppInvHP_16_32s;
	runtime.inv_LP_16_32s = gppInvLP_16_32s;
	runtime.inv_sqrt_HP_16_32s = gppInvSqrtHP_16_32s;
	runtime.inv_sqrt_LP_16_32s = gppInvSqrtLP_16_32s;
	runtime.sqrt_HP_16_32s = gppSqrtHP_16_32s;
	runtime.sqrt_LP_16_32s = gppSqrtLP_16_32s;

	cg_codegen_t * codegen = cg_codegen_create(heap, &runtime);
	cg_codegen_emit_module(codegen, m_Module);
	cg_codegen_fix_refs(codegen);

	cg_segment_t * cseg = cg_codegen_segment(codegen);

#ifndef NDEBEUG
	ARMDis dis;
	armdis_init(&dis);
	armdis_dump(&dis, "dump5.txt", cseg);
#endif

	void * targetBuffer = 
		target->AddFunction(type, 
							*m_State, cg_segment_size(cseg));

	cg_segment_get_block(cseg, 0, targetBuffer, cg_segment_size(cseg));

#if defined(EGL_ON_WINCE) && (defined(ARM) || defined(_ARM_))
	// flush data cache and clear instruction cache to make new code visible to execution unit
	CacheSync(CACHE_SYNC_INSTRUCTIONS | CACHE_SYNC_WRITEBACK);		
#endif

	cg_codegen_destroy(codegen);
	cg_heap_destroy(module->heap);
}

