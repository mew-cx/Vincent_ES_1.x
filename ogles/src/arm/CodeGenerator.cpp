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

#if defined(ARM) && defined(__gnu_linux__)
#define CLEAR_INSN_CACHE(BEG, END)									\
{																	\
  register unsigned long _beg __asm ("a1") = (unsigned long) (BEG);	\
  register unsigned long _end __asm ("a2") = (unsigned long) (END);	\
  register unsigned long _flg __asm ("a3") = 0;						\
  register unsigned long _scno __asm ("r7") = 0xf0002;				\
  __asm __volatile ("swi 0x9f0002		@ sys_cacheflush"			\
		    : "=r" (_beg)											\
		    : "0" (_beg), "r" (_end), "r" (_flg), "r" (_scno));		\
}
#endif

using namespace EGL;


#define ALLOC_REG(reg) reg = cg_virtual_reg_create(procedure, cg_reg_type_general)
#define ALLOC_FLAGS(reg) reg = cg_virtual_reg_create(procedure, cg_reg_type_flags)
#define DECL_REG(reg) cg_virtual_reg_t * reg = cg_virtual_reg_create(procedure, cg_reg_type_general)
#define DECL_FLAGS(reg) cg_virtual_reg_t * reg = cg_virtual_reg_create(procedure, cg_reg_type_flags)
#define DECL_CONST_REG(reg, value) cg_virtual_reg_t * reg = cg_virtual_reg_create(procedure, cg_reg_type_general); LDI(reg, value)


namespace {

	inline cg_virtual_reg_t * LOAD_DATA(cg_block_t * block, cg_virtual_reg_t * base, I32 constant) {
		cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
		cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);
		cg_virtual_reg_t * value = cg_virtual_reg_create(block->proc, cg_reg_type_general);

		LDI(offset, constant);
		ADD(addr, base, offset);
		LDW(value, addr);

		return value;
	}

	inline void STORE_DATA(cg_block_t * block, cg_virtual_reg_t * base, I32 constant, cg_virtual_reg_t * value) {
		cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
		cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);

		LDI(offset, constant);
		ADD(addr, base, offset);
		STW(value, addr);

	}

	void Dump(const char * filename, cg_module_t * module)
	{
		FILE * fp = fopen(filename, "w");
		cg_module_dump(module, fp);
		fclose(fp);
	}

}

void CodeGenerator :: Compile(FunctionCache * target, FunctionCache::FunctionType type, const VaryingInfo * varyingInfo,
	void (CodeGenerator::*function)(const VaryingInfo * varyingInfo)) {

	cg_heap_t * heap = cg_heap_create(4096);
	cg_module_t * module = cg_module_create(heap);

	m_Module = module;

	(this->*function)(varyingInfo);

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

	cg_codegen_t * codegen = cg_codegen_create(heap, &runtime, &processor);
	cg_codegen_emit_module(codegen, m_Module);
	cg_codegen_fix_refs(codegen);

	cg_segment_t * cseg = cg_codegen_segment(codegen);

#ifdef DEBUG
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
#elif defined(ARM) && defined(__gnu_linux__)
	CLEAR_INSN_CACHE(targetBuffer, (U8 *) targetBuffer + cg_segment_size(cseg))
#endif

	cg_codegen_destroy(codegen);
	cg_heap_destroy(module->heap);
}


cg_virtual_reg_t * CodeGenerator :: Mul255(cg_block_t * block, cg_virtual_reg_t * first, cg_virtual_reg_t * second) {
	cg_proc_t * procedure = block->proc;

	DECL_REG		(regProduct);
	DECL_CONST_REG	(constant8, 8);
	DECL_REG		(regShifted);
	DECL_REG		(regAdjusted);
	DECL_REG		(regFinal);
	
	MUL			(regProduct,	first, second);
	ASR			(regShifted,	regProduct, constant8);
	ADD			(regAdjusted,	regProduct, regShifted);
	ASR			(regFinal,		regAdjusted, constant8);

	return regFinal;
}

cg_virtual_reg_t * CodeGenerator :: AddSaturate255(cg_block_t * block, cg_virtual_reg_t * first, cg_virtual_reg_t * second) {
	cg_proc_t * procedure = block->proc;

	DECL_REG		(regSum);
	DECL_CONST_REG	(constant255, 0xff);
	DECL_REG		(regResult);

	ADD				(regSum, first, second);
	MIN				(regResult, regSum, constant255);

	return regResult;
}

cg_virtual_reg_t * CodeGenerator :: ClampTo255(cg_block_t * block, cg_virtual_reg_t * value) {
	cg_proc_t * procedure = block->proc;

	if (value) {
		DECL_CONST_REG	(constant0, 0);
		DECL_CONST_REG	(constant17, 17);
		DECL_CONST_REG	(constant1, 0x10000);
		DECL_CONST_REG	(constantFactor, 0x1ff);

		DECL_REG	(regClamped0);
		DECL_REG	(regClamped1);
		DECL_REG	(regAdjusted);
		DECL_REG	(regResult);

		MAX		(regClamped0, value, constant0);
		MIN		(regClamped1, regClamped0, constant1);
		MUL		(regAdjusted, regClamped1, constantFactor);
		LSR		(regResult, regAdjusted, constant17);

		return regResult;
	} else {
		DECL_CONST_REG	(regResult, 0);

		return regResult;
	}
}

cg_virtual_reg_t * CodeGenerator :: AddSigned(cg_block_t * block, cg_virtual_reg_t * first, cg_virtual_reg_t * second) {
	cg_proc_t * procedure = block->proc;

	DECL_REG		(regResult);
	DECL_REG		(regSum);
	DECL_REG		(regAdjusted);
	DECL_CONST_REG	(constantHalf, 0x80);
	DECL_CONST_REG	(constant0, 0);

	ADD				(regSum, first, second);
	SUB				(regAdjusted, regSum, constantHalf);
	MAX				(regResult, regAdjusted, constant0);

	return regResult;
}

cg_virtual_reg_t * CodeGenerator :: Add(cg_block_t * block, cg_virtual_reg_t * first, cg_virtual_reg_t * second) {
	cg_proc_t * procedure = block->proc;

	DECL_REG		(regResult);
	DECL_REG		(regSum);

	ADD				(regResult, first, second);

	return regResult;
}

cg_virtual_reg_t * CodeGenerator :: Sub(cg_block_t * block, cg_virtual_reg_t * first, cg_virtual_reg_t * second) {
	cg_proc_t * procedure = block->proc;

	DECL_REG		(regResult);

	SUB				(regResult, first, second);

	return regResult;
}

cg_virtual_reg_t * CodeGenerator :: ExtractBitFieldTo255(cg_block_t * block, cg_virtual_reg_t * value, size_t low, size_t high) {
	cg_proc_t * procedure = block->proc;

	if (high == low) {
		if (high < 8) {
			DECL_REG		(regShifted);
			DECL_CONST_REG	(constantShift, 8 - high);

			LSL				(regShifted, value, constantShift);

			value = regShifted;
		} else if (high > 8) {
			DECL_REG		(regShifted);
			DECL_CONST_REG	(constantShift, high - 8);

			LSR				(regShifted, value, constantShift);

			value = regShifted;
		}

		DECL_CONST_REG	(constantMask,	0x100);
		DECL_REG		(regMasked);

		AND				(regMasked,		value, constantMask);

		DECL_CONST_REG	(constant8,		8);
		DECL_REG		(regShifted);
		DECL_REG		(regAdjusted);

		LSR				(regShifted,	value, constant8);
		SUB				(regAdjusted,	value, regShifted);

		return regAdjusted;
	}

	if (high < 7) {
		DECL_REG		(regShifted);
		DECL_CONST_REG	(constantShift, 7 - high);

		LSL				(regShifted, value, constantShift);

		value = regShifted;
	} else if (high > 7) {
		DECL_REG		(regShifted);
		DECL_CONST_REG	(constantShift, high - 7);

		LSR				(regShifted, value, constantShift);

		value = regShifted;
	}

	size_t bits = high - low + 1;
	static const U8 mask[9] = { 0, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };

	DECL_CONST_REG		(constantMask,	mask[bits]);
	DECL_REG			(regMasked);

	AND					(regMasked,		value, constantMask);
	value = regMasked;

	while (bits < 8) {
		DECL_CONST_REG	(constantShift,	bits);
		DECL_REG		(regShifted);
		DECL_REG		(regOred);

		LSR				(regShifted,	value, constantShift);
		OR				(regOred,		value, regShifted);

		value = regOred;
		bits += 2;
	}

	return value;
}

cg_virtual_reg_t * CodeGenerator :: BitFieldFrom255(cg_block_t * block, cg_virtual_reg_t * value, size_t low, size_t high) {
	cg_proc_t * procedure = block->proc;

	size_t bits = high - low + 1;
	assert(bits <= 8);
	size_t lowBit = 8 - bits;

	if (bits != 8) {
		static const U8 mask[9] = { 0, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };

		DECL_CONST_REG		(constantMask,	mask[bits]);
		DECL_REG			(regMasked);

		AND					(regMasked,		value, constantMask);
		value = regMasked;
	}
	
	if (low > lowBit) {
		DECL_CONST_REG		(constantShift,	low - lowBit);
		DECL_REG			(regShifted);

		LSL					(regShifted,	value, constantShift);
		value = regShifted;
	} else if (low < lowBit) {
		DECL_CONST_REG		(constantShift,	lowBit - low);
		DECL_REG			(regShifted);

		LSR					(regShifted,	value, constantShift);
		value = regShifted;
	}

	return value;
}

// ----------------------------------------------------------------------
// Emit code to convert a representation of a color as individual
// R, G and B components into a 16-bit 565 representation
//
// R, G B are within the range 0..0xff
// ----------------------------------------------------------------------
void CodeGenerator :: Color565FromRGB(cg_block_t * block, cg_virtual_reg_t * regRGB,
	cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b) {
	cg_proc_t * procedure = block->proc;

	cg_virtual_reg_t *	regFieldR = BitFieldFrom255(block, r, 11, 15);
	cg_virtual_reg_t *	regFieldG = BitFieldFrom255(block, g, 5, 10);
	cg_virtual_reg_t *	regFieldB = BitFieldFrom255(block, b, 0, 4);

	DECL_REG	(regBG);

	OR			(regBG,		regFieldB, regFieldG);
	OR			(regRGB,	regBG, regFieldR);
}


cg_virtual_reg_t * CodeGenerator :: Color565FromRGB(cg_block_t * block,
													cg_virtual_reg_t * r, cg_virtual_reg_t * g, cg_virtual_reg_t * b) {
	cg_proc_t * procedure = block->proc;

	DECL_REG	(regResult);

	Color565FromRGB(block, regResult, r, g, b);

	return regResult;
}


cg_virtual_reg_t * CodeGenerator :: Blend255(cg_block_t * block, cg_virtual_reg_t * first, cg_virtual_reg_t * second,
											 cg_virtual_reg_t * alpha) {

	cg_proc_t * procedure = block->proc;

	DECL_REG		(regDiff);

	SUB				(regDiff,		second, first);		// diff = (second - first)

	cg_virtual_reg_t *	regProd = Mul255(block, regDiff, alpha);	//	alpha * (second - first)

	return Add(block, first, regProd);					// first + alpha * (second - first)
}


cg_virtual_reg_t * CodeGenerator :: Blend255(cg_block_t * block, U8 constant, cg_virtual_reg_t * second,
											 cg_virtual_reg_t * alpha) {
	cg_proc_t * procedure = block->proc;

	DECL_CONST_REG	(regConst,	constant);

	return Blend255(block, regConst, second, alpha);
}

cg_virtual_reg_t * CodeGenerator :: SignedVal(cg_block_t * block, cg_virtual_reg_t * value) {
	cg_proc_t * procedure = block->proc;

	DECL_REG		(regShifted);
	DECL_CONST_REG	(constantShift, 7);
	DECL_REG		(regExpanded);
	DECL_CONST_REG	(c128, 128);
	DECL_REG		(regResult);

	// expand 0..255 -> 0..256
	LSR				(regShifted, value, constantShift);
	ADD				(regExpanded, value, regShifted);
	SUB				(regResult, regExpanded, c128);

	return regResult;
}

cg_virtual_reg_t * CodeGenerator :: Dot3(cg_block_t * block, 
										 cg_virtual_reg_t * r[], cg_virtual_reg_t * g[], cg_virtual_reg_t * b[]) {
	cg_proc_t * procedure = block->proc;

	DECL_REG		(regProdR);
	DECL_REG		(regProdG);
	DECL_REG		(regProdB);
	DECL_REG		(regSumRG);
	DECL_REG		(regSumRGB);

	MUL				(regProdR, SignedVal(block, r[0]), SignedVal(block, r[1]));
	MUL				(regProdG, SignedVal(block, g[0]), SignedVal(block, g[1]));
	ADD				(regSumRG, regProdR, regProdG);
	MUL				(regProdB, SignedVal(block, b[0]), SignedVal(block, b[1]));
	ADD				(regSumRGB, regSumRG, regProdB);

	DECL_CONST_REG	(constant6, 6);
	DECL_CONST_REG	(constant7, 7);
	DECL_REG		(regShifted6);
	DECL_REG		(regShifted13);
	DECL_REG		(regAdjusted);

	ASR				(regShifted6, regSumRGB, constant6);
	ASR				(regShifted13, regShifted6, constant7);
	SUB				(regAdjusted, regShifted6, regShifted13);

	DECL_REG		(regClamped0);
	DECL_REG		(regClamped255);

	DECL_CONST_REG	(constant0, 0);
	DECL_CONST_REG	(constant255, 255);

	MAX				(regClamped0, regAdjusted, constant0);
	MIN				(regClamped255, regClamped0, constant255);

	return regClamped255;
}


// ----------------------------------------------------------------------
// Code generation of vertex processing
// ----------------------------------------------------------------------

// fetch vertex coordinates and transform by given matrix
void CodeGenerator :: GenerateCoordFetchMultiply(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	cg_virtual_reg_t * matrixBase,
	GLenum type, size_t size) {
	switch (size) {
	case 2:
		GenerateCoordFetchMultiply2(block, vectorBase, resultBase, resultOffset, matrixBase, type);
		break;

	case 3:
		GenerateCoordFetchMultiply3(block, vectorBase, resultBase, resultOffset, matrixBase, type);
		break;

	case 4:
		GenerateCoordFetchMultiply4(block, vectorBase, resultBase, resultOffset, matrixBase, type);
		break;
	}
}

void CodeGenerator :: GenerateCoordFetchMultiply2(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	cg_virtual_reg_t * matrixBase, GLenum type) {

	cg_proc_t * procedure = block->proc;

	cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
	cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);

	for (int row = 0; row < 4; ++row) {
		DECL_REG	(p0);
		DECL_REG	(p1);
		DECL_REG	(p2);
		DECL_REG	(p3);

		DECL_REG	(s01);
		DECL_REG	(s013);

		cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, row, 0);
		cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, row, 1);

		FMUL		(p0, m0, v0);

		cg_virtual_reg_t * m3 = LoadMatrixElement(block, matrixBase, row, 3);

		FMUL		(p1, m1, v1);
		FADD		(s01, p0, p1);
		FADD		(s013, s01, m3);

		STORE_DATA(block, resultBase, resultOffset + row * sizeof(EGL_Fixed), s013);
	}
}

void CodeGenerator :: GenerateCoordFetchMultiply3(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	cg_virtual_reg_t * matrixBase, GLenum type) {


	cg_proc_t * procedure = block->proc;

	cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
	cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);
	cg_virtual_reg_t * v2  = LoadVectorElement(block, vectorBase, 2, type);

	for (int row = 0; row < 4; ++row) {
		DECL_REG	(p0);
		DECL_REG	(p1);
		DECL_REG	(p2);
		DECL_REG	(p3);

		DECL_REG	(s01);
		DECL_REG	(s012);
		DECL_REG	(s0123);

		cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, row, 0);
		cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, row, 1);

		FMUL		(p0, m0, v0);

		cg_virtual_reg_t * m2 = LoadMatrixElement(block, matrixBase, row, 2);

		FMUL		(p1, m1, v1);
		FADD		(s01, p0, p1);

		cg_virtual_reg_t * m3 = LoadMatrixElement(block, matrixBase, row, 3);

		FMUL		(p2, m2, v2);
		FADD		(s012, s01, p2);
		FADD		(s0123, s012, m3);

		STORE_DATA(block, resultBase, resultOffset + row * sizeof(EGL_Fixed), s0123);
	}
}

void CodeGenerator :: GenerateCoordFetchMultiply4(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	cg_virtual_reg_t * matrixBase, GLenum type) {

	cg_proc_t * procedure = block->proc;

	cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
	cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);
	cg_virtual_reg_t * v2  = LoadVectorElement(block, vectorBase, 2, type);
	cg_virtual_reg_t * v3  = LoadVectorElement(block, vectorBase, 3, type);

	for (int row = 0; row < 4; ++row) {
		DECL_REG	(p0);
		DECL_REG	(p1);
		DECL_REG	(p2);
		DECL_REG	(p3);

		DECL_REG	(s01);
		DECL_REG	(s012);
		DECL_REG	(s0123);

		cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, row, 0);
		cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, row, 1);

		FMUL		(p0, m0, v0);

		cg_virtual_reg_t * m2 = LoadMatrixElement(block, matrixBase, row, 2);

		FMUL		(p1, m1, v1);
		FADD		(s01, p0, p1);

		cg_virtual_reg_t * m3 = LoadMatrixElement(block, matrixBase, row, 3);

		FMUL		(p2, m2, v2);
		FADD		(s012, s01, p2);
		FMUL		(p3, m3, v3);
		FADD		(s0123, s012, p3);

		STORE_DATA(block, resultBase, resultOffset + row * sizeof(EGL_Fixed), s0123);
	}
}

// fetch normal and transform by given matrix
void CodeGenerator :: GenerateNormalFetchMultiply(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	cg_virtual_reg_t * matrixBase,
	GLenum type) {

	cg_proc_t * procedure = block->proc;

	cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
	cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);
	cg_virtual_reg_t * v2  = LoadVectorElement(block, vectorBase, 2, type);

	for (int row = 0; row < 3; ++row) {
		DECL_REG	(p0);
		DECL_REG	(p1);
		DECL_REG	(p2);

		DECL_REG	(s01);
		DECL_REG	(s012);

		cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, row, 0);
		cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, row, 1);

		FMUL		(p0, m0, v0);

		cg_virtual_reg_t * m2 = LoadMatrixElement(block, matrixBase, row, 2);

		FMUL		(p1, m1, v1);
		FADD		(s01, p0, p1);
		FMUL		(p2, m2, v2);
		FADD		(s012, s01, p2);

		STORE_DATA(block, resultBase, resultOffset + row * sizeof(EGL_Fixed), s012);
	}
}

// fetch color
void CodeGenerator :: GenerateColorFetch(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	GLenum type) {

	cg_proc_t * procedure = block->proc;

	cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
	cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);
	STORE_DATA(block, resultBase, resultOffset + 0 * sizeof(EGL_Fixed), v0);
	cg_virtual_reg_t * v2  = LoadVectorElement(block, vectorBase, 2, type);
	STORE_DATA(block, resultBase, resultOffset + 1 * sizeof(EGL_Fixed), v1);
	cg_virtual_reg_t * v3  = LoadVectorElement(block, vectorBase, 3, type);
	STORE_DATA(block, resultBase, resultOffset + 2 * sizeof(EGL_Fixed), v2);
	STORE_DATA(block, resultBase, resultOffset + 3 * sizeof(EGL_Fixed), v3);
}

// fetch tex coords and transform by given matrix if not null
void CodeGenerator :: GenerateTexCoordFetchMultiply(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	cg_virtual_reg_t * matrixBase,
	GLenum type, size_t size) {
	switch (size) {
	case 2:
		GenerateTexCoordFetchMultiply2(block, vectorBase, resultBase, resultOffset, matrixBase, type);
		break;

	case 3:
		GenerateTexCoordFetchMultiply3(block, vectorBase, resultBase, resultOffset, matrixBase, type);
		break;

	case 4:
		GenerateTexCoordFetchMultiply4(block, vectorBase, resultBase, resultOffset, matrixBase, type);
		break;
	}
}

void CodeGenerator :: GenerateTexCoordFetchMultiply2(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	cg_virtual_reg_t * matrixBase, GLenum type) {

	cg_proc_t * procedure = block->proc;

	if (!matrixBase) {
		cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
		cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);

		STORE_DATA(block, resultBase, resultOffset + 0 * sizeof(EGL_Fixed), v0);
		STORE_DATA(block, resultBase, resultOffset + 1 * sizeof(EGL_Fixed), v1);
	} else {
		cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
		cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);

		DECL_REG		(inverse);

		{
			DECL_REG	(p0);
			DECL_REG	(p1);

			DECL_REG	(s01);
			DECL_REG	(s013);

			cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, 3, 0);
			cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, 3, 1);

			FMUL		(p0, m0, v0);

			cg_virtual_reg_t * m3 = LoadMatrixElement(block, matrixBase, 3, 3);

			FMUL		(p1, m1, v1);
			FADD		(s01, p0, p1);
			FADD		(s013, s01, m3);

			FINV		(inverse, s013);
		}

		for (int row = 0; row < 2; ++row) {
			DECL_REG	(p0);
			DECL_REG	(p1);

			DECL_REG	(s01);
			DECL_REG	(s013);

			DECL_REG	(prod);

			cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, row, 0);
			cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, row, 1);

			FMUL		(p0, m0, v0);

			cg_virtual_reg_t * m3 = LoadMatrixElement(block, matrixBase, row, 3);

			FMUL		(p1, m1, v1);
			FADD		(s01, p0, p1);
			FADD		(s013, s01, m3);
			FMUL		(prod, s013, inverse);

			STORE_DATA(block, resultBase, resultOffset + row * sizeof(EGL_Fixed), prod);
		}
	}
}

void CodeGenerator :: GenerateTexCoordFetchMultiply3(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	cg_virtual_reg_t * matrixBase, GLenum type) {

	cg_proc_t * procedure = block->proc;

	if (!matrixBase) {
		cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
		cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);

		STORE_DATA(block, resultBase, resultOffset + 0 * sizeof(EGL_Fixed), v0);
		STORE_DATA(block, resultBase, resultOffset + 1 * sizeof(EGL_Fixed), v1);
	} else {
		cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
		cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);
		cg_virtual_reg_t * v2  = LoadVectorElement(block, vectorBase, 2, type);

		DECL_REG		(inverse);

		{
			DECL_REG	(p0);
			DECL_REG	(p1);
			DECL_REG	(p2);

			DECL_REG	(s01);
			DECL_REG	(s012);
			DECL_REG	(s0123);

			cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, 3, 0);
			cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, 3, 1);

			FMUL		(p0, m0, v0);

			cg_virtual_reg_t * m2 = LoadMatrixElement(block, matrixBase, 3, 2);

			FMUL		(p1, m1, v1);
			FADD		(s01, p0, p1);

			cg_virtual_reg_t * m3 = LoadMatrixElement(block, matrixBase, 3, 3);

			FMUL		(p2, m2, v2);
			FADD		(s012, s01, p2);
			FADD		(s0123, s012, m3);

			FINV		(inverse, s0123);
		}

		for (int row = 0; row < 2; ++row) {
			DECL_REG	(p0);
			DECL_REG	(p1);
			DECL_REG	(p2);
			DECL_REG	(p3);

			DECL_REG	(s01);
			DECL_REG	(s012);
			DECL_REG	(s0123);

			DECL_REG	(prod);

			cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, row, 0);
			cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, row, 1);

			FMUL		(p0, m0, v0);

			cg_virtual_reg_t * m2 = LoadMatrixElement(block, matrixBase, row, 2);

			FMUL		(p1, m1, v1);
			FADD		(s01, p0, p1);

			cg_virtual_reg_t * m3 = LoadMatrixElement(block, matrixBase, row, 3);

			FMUL		(p2, m2, v2);
			FADD		(s012, s01, p2);
			FADD		(s0123, s012, m3);

			FMUL		(prod, s0123, inverse);

			STORE_DATA(block, resultBase, resultOffset + row * sizeof(EGL_Fixed), prod);
		}
	}
}

void CodeGenerator :: GenerateTexCoordFetchMultiply4(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	cg_virtual_reg_t * matrixBase, GLenum type) {

	cg_proc_t * procedure = block->proc;

	if (!matrixBase) {
		cg_virtual_reg_t * v3  = LoadVectorElement(block, vectorBase, 3, type);

		DECL_REG		(invV3);
		DECL_REG		(v0InvV3);
		DECL_REG		(v1InvV3);

		FINV			(invV3, v3);

		cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);

		FMUL			(v0InvV3, v0, invV3);

		cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);
		STORE_DATA(block, resultBase, resultOffset + 0 * sizeof(EGL_Fixed), v0InvV3);

		FMUL			(v1InvV3, v1, invV3);

		STORE_DATA(block, resultBase, resultOffset + 1 * sizeof(EGL_Fixed), v1InvV3);
	} else {
		cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
		cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);
		cg_virtual_reg_t * v2  = LoadVectorElement(block, vectorBase, 2, type);
		cg_virtual_reg_t * v3  = LoadVectorElement(block, vectorBase, 3, type);

		DECL_REG		(inverse);

		{
			DECL_REG	(p0);
			DECL_REG	(p1);
			DECL_REG	(p2);
			DECL_REG	(p3);

			DECL_REG	(s01);
			DECL_REG	(s012);
			DECL_REG	(s0123);

			cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, 3, 0);
			cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, 3, 1);

			FMUL		(p0, m0, v0);

			cg_virtual_reg_t * m2 = LoadMatrixElement(block, matrixBase, 3, 2);

			FMUL		(p1, m1, v1);
			FADD		(s01, p0, p1);

			cg_virtual_reg_t * m3 = LoadMatrixElement(block, matrixBase, 3, 3);

			FMUL		(p2, m2, v2);
			FADD		(s012, s01, p2);
			FMUL		(p3, m3, v3);
			FADD		(s0123, s012, p3);

			FINV		(inverse, s0123);
		}

		for (int row = 0; row < 2; ++row) {
			DECL_REG	(p0);
			DECL_REG	(p1);
			DECL_REG	(p2);
			DECL_REG	(p3);

			DECL_REG	(s01);
			DECL_REG	(s012);
			DECL_REG	(s0123);

			DECL_REG	(prod);

			cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, row, 0);
			cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, row, 1);

			FMUL		(p0, m0, v0);

			cg_virtual_reg_t * m2 = LoadMatrixElement(block, matrixBase, row, 2);

			FMUL		(p1, m1, v1);
			FADD		(s01, p0, p1);

			cg_virtual_reg_t * m3 = LoadMatrixElement(block, matrixBase, row, 3);

			FMUL		(p2, m2, v2);
			FADD		(s012, s01, p2);
			FMUL		(p3, m3, v3);
			FADD		(s0123, s012, p3);

			FMUL		(prod, s0123, inverse);

			STORE_DATA(block, resultBase, resultOffset + row * sizeof(EGL_Fixed), prod);
		}
	}
}

// load a specific element of a matrix
cg_virtual_reg_t * CodeGenerator :: LoadMatrixElement(cg_block_t * block,
													  cg_virtual_reg_t * base, int row, int column) {
	cg_proc_t * procedure = block->proc;

	int index = sizeof(EGL_Fixed) * (row + column * 4);	// offset of element in 4 by 4 matrix

	cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
	cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);
	cg_virtual_reg_t * value = cg_virtual_reg_create(block->proc, cg_reg_type_general);

	LDI(offset, index);
	ADD(addr, base, offset);
	LDW(value, addr);

	return value;
}

cg_virtual_reg_t * CodeGenerator :: LoadVectorElement(cg_block_t * block, 
													  cg_virtual_reg_t * base, int element, GLenum type) {
	cg_proc_t * procedure = block->proc;

	switch (type) {
	case GL_BYTE:
		{
			cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * value = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * shiftedLeft = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * shiftedRight = cg_virtual_reg_create(block->proc, cg_reg_type_general);

			DECL_CONST_REG	(num24, 24);
			DECL_CONST_REG	(num8, 8);

			LDI		(offset, element);
			ADD		(addr, base, offset);
			LDB		(value, addr);
			LSL		(shiftedLeft, value, num24);			/* sign extend and convert to 16.16 */
			ASR		(shiftedRight, shiftedLeft, num8);

			return shiftedRight;
		}
		break;

	case GL_SHORT:
		{
			cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * value = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * shiftedLeft = cg_virtual_reg_create(block->proc, cg_reg_type_general);

			DECL_CONST_REG	(num16, 16);

			LDI		(offset, element * sizeof(GLshort));
			ADD		(addr, base, offset);
			LDH		(value, addr);
			LSL		(shiftedLeft, value, num16);			/* sign extend and convert to 16.16 */

			return shiftedLeft;
		}
		break;

	case GL_FIXED:
		{
			cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * value = cg_virtual_reg_create(block->proc, cg_reg_type_general);

			LDI		(offset, element * sizeof(EGL_Fixed));
			ADD		(addr, base, offset);
			LDW		(value, addr);

			return value;
		}
		break;

	case GL_FLOAT:
		{
			cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * value = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * result = cg_virtual_reg_create(block->proc, cg_reg_type_general);

			LDI		(offset, element * sizeof(EGL_Fixed));
			ADD		(addr, base, offset);
			LDW		(value, addr);
			FCONVFLT(result, value);

			return result;
		}
		break;

	case GL_UNSIGNED_BYTE:
		/* for color coordinates! */
		{
			cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * value = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * shiftedLeft = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * shiftedRight = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * ored = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * sum = cg_virtual_reg_create(block->proc, cg_reg_type_general);

			DECL_CONST_REG	(num8, 8);
			DECL_CONST_REG	(num7, 7);

			LDI		(offset, element);
			ADD		(addr, base, offset);
			LDB		(value, addr);
			LSL		(shiftedLeft, value, num8);			/* sign extend and convert to 16.16 */
			LSR		(shiftedRight, value, num7);
			OR		(ored, value, shiftedLeft);
			ADD		(sum, ored, shiftedRight);

			return sum;
		}
		break;

	}

	return 0;
}

void CodeGenerator :: GenerateFetch(const RenderState * state) {

	cg_proc_t * procedure = cg_proc_create(m_Module);

	// signature is (const RenderInfo * info, int index, Vertx * result)

	DECL_REG	(regRenderInfo);// RenderInfo structure pointer
	DECL_REG	(regIndex);		// Index of element to fetch
	DECL_REG	(regResult);	// Pointer to resulting vertex

	procedure->num_args = 3;	// the previous three declarations make up the arguments

	cg_block_t * block = cg_block_create(procedure, 1);

	cg_virtual_reg_t *	coordStride	= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_COORD_STRIDE);
	cg_virtual_reg_t *	coordBase	= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_COORD_BASE);

	DECL_REG	(coordOffset);
	DECL_REG	(coordVector);

	MUL			(coordOffset, coordStride, regIndex);

	cg_virtual_reg_t *	mvp			= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_MVP);
	ADD			(coordVector, coordBase, coordOffset);

	GenerateCoordFetchMultiply(block, coordVector, regResult, OFFSET_RASTER_POS_CLIP, mvp, state->Coord.Type, state->Coord.Size);

	if (state->NeedsEyeCoords) {
		// we might be able to optimize this to avoid reloading coords
		cg_virtual_reg_t *	mv		= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_MV);
		GenerateCoordFetchMultiply(block, coordVector, regResult, OFFSET_RASTER_POS_EYE, mv, state->Coord.Type, state->Coord.Size);
	}

	if (state->NeedsNormal && state->Normal.Enabled) {
		cg_virtual_reg_t *	normalStride= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_NORMAL_STRIDE);
		cg_virtual_reg_t *	normalBase	= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_NORMAL_BASE);

		DECL_REG	(normalOffset);
		DECL_REG	(normalVector);

		MUL			(normalOffset, normalStride, regIndex);

		cg_virtual_reg_t *	invMv			= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_INV_MV);
		ADD			(normalVector, normalBase, normalOffset);

		GenerateNormalFetchMultiply(block, normalVector, regResult, OFFSET_RASTER_POS_NORMAL, invMv, state->Normal.Type);
	}

	if (state->NeedsColor && state->Color.Enabled) {
		cg_virtual_reg_t *	colorStride= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_COLOR_STRIDE);
		cg_virtual_reg_t *	colorBase	= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_COLOR_BASE);

		DECL_REG	(colorOffset);
		DECL_REG	(colorVector);

		MUL			(colorOffset, colorStride, regIndex);
		ADD			(colorVector, colorBase, colorOffset);

		GenerateColorFetch(block, colorVector, regResult, OFFSET_RASTER_POS_RAW_COLOR, state->Color.Type);
	}

	for (size_t unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
		if (state->Varying.textureBase[unit] >= 0 && state->TexCoord[unit].Enabled) {
			cg_virtual_reg_t *	texCoordStride	= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_TEX_COORD_STRIDE(unit));
			cg_virtual_reg_t *	texCoordBase	= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_TEX_COORD_BASE(unit));

			DECL_REG	(texCoordOffset);
			DECL_REG	(texCoordVector);

			MUL			(texCoordOffset, texCoordStride, regIndex);

			cg_virtual_reg_t *	tcm = 0;

			if (!state->TextureMatrixIdentity[unit]) {
				tcm = LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_TEX(unit));
			}

			ADD			(texCoordVector, texCoordBase, texCoordOffset);

			GenerateTexCoordFetchMultiply(block, texCoordVector, regResult, state->Varying.textureBase[unit], tcm, 
				state->TexCoord[unit].Type, state->TexCoord[unit].Size);
		}
	}

	RET();
}
