// ==========================================================================
//
// FunctionCache.cpp	Cache of compiled functions for 3D Rendering Library
//
// --------------------------------------------------------------------------
//
// 03-08-2003		Hans-Martin Will	initial version
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
#include "FunctionCache.h"
#include "FetchVertexPart.h"

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

// ----------------------------------------------------------------------
// Info-Block to manage a single compiled function
// ----------------------------------------------------------------------

namespace EGL {
	enum Flags {
		FlagNone = 0,					// no flags
		FlagUsed = 1,					// function is still in use
		FlagExternal = 2,				// function is external
	};

	struct FunctionInfo {
		FunctionInfo	* m_Prev;		// previous item in LRU chain
		FunctionInfo	* m_Next;		// next item in LRU chain

										// the state that was compiled into this function
		U8				m_State[sizeof(RasterizerState) > sizeof(RenderState) ?
							    sizeof(RasterizerState) : sizeof(RenderState)];

		union {
			size_t		m_Offset;		// offset of function in code segment
			const void *m_Pointer;		// external function pointer
		};

		size_t			m_Size;			// size of function in code segment
		U32				m_Flags;		// flags for garbage collection

		PipelinePart::Part m_Part;		// what part of the pipeline is this?
	};
}

FunctionCache :: FunctionCache(size_t totalSize, float percentageKeep, size_t maxExternalFunctions) {
	m_Total = totalSize;
	m_Used = 0;
	m_PercentageKeep = percentageKeep;

	m_UsedExternalFunctions = 0;
	m_MaxExternalFunctions = maxExternalFunctions;
	m_UsedFunctions = 0;
	m_MaxFunctions = totalSize / 256 + m_MaxExternalFunctions;

	m_MostRecentlyUsed = 0;
	m_LeastRecentlyUsed = 0;

	m_Functions = (FunctionInfo *) malloc(sizeof(FunctionInfo) * m_MaxFunctions);
	memset(m_Functions, 0, sizeof(FunctionInfo)  * m_MaxFunctions);

#if defined(EGL_ON_WINCE)
	m_Code = reinterpret_cast<U8 *>(VirtualAlloc(0, totalSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE));
#elif defined(EGL_ON_SYMBIAN)
    m_Code = reinterpret_cast<U8*>(User::Alloc(totalSize));
#else
  m_Code = (U8 *)malloc(totalSize);
#endif
}


FunctionCache :: ~FunctionCache() {
	free(m_Functions);

#if defined(EGL_ON_WINCE)
	VirtualFree(m_Code, m_Total, MEM_DECOMMIT);
#elif defined(EGL_ON_SYMBIAN)
    User::Free(m_Code);
#else
  free(m_Code);
#endif
}


void * FunctionCache :: GetFunction(PipelinePart::Part part, const void * state) {

	PipelinePart & ppart = PipelinePart::Get(part);

	for (FunctionInfo * function = m_MostRecentlyUsed; function; function = function->m_Next) {
		if (function->m_Part == part && ppart.CompareState(function->m_State, state)) {
			if (function->m_Flags & FlagExternal) {
				return const_cast<void *>(function->m_Pointer);
			} else {
				return reinterpret_cast<void *>(m_Code + function->m_Offset);
			}
		}
	}

	assert(0);
	return 0;
}

bool FunctionCache :: SetFunction(PipelinePart::Part part, const void * state, const void * ptr) {

	// Determine existing cache entry for this configuration
	PipelinePart & ppart = PipelinePart::Get(part);
	FunctionInfo * function;

	for (function = m_MostRecentlyUsed; function; function = function->m_Next) {
		if (function->m_Part == part && ppart.CompareState(function->m_State, state)) {
			break;
		}
	}

	if (!ptr) {
		// clear existing entry if exists

		if (function) {
			// clear the entry
			--m_UsedExternalFunctions;
			function->m_Part = PipelinePart::PartInvalid;
			function->m_Pointer = 0;
			function->m_Flags = 0;
		}
	} else {
		if (!function) {
			// Create a new entry

			if (m_UsedExternalFunctions >= m_MaxExternalFunctions) {
				return false;
			}

			++m_UsedExternalFunctions;

			function = AllocateFunction(part, state);
			assert(function);
		} else if (!(function->m_Flags & FlagExternal)) {
			if (m_UsedExternalFunctions >= m_MaxExternalFunctions) {
				return false;
			} else {
				++m_UsedExternalFunctions;
			}
		}

		// record the function pointer
		function->m_Flags = FlagExternal;
		function->m_Pointer = ptr;
	}

	return true;
}

void FunctionCache :: PrepareFunction(PipelinePart::Part part, const void * state, const VaryingInfo * varyingInfo) {

	PipelinePart & ppart = PipelinePart::Get(part);

	for (FunctionInfo * function = m_MostRecentlyUsed; function; function = function->m_Next) {
		if (function->m_Part == part && ppart.CompareState(function->m_State, state)) {
			// move to front
			if (function->m_Prev) {
				function->m_Prev->m_Next = function->m_Next;

				if (function->m_Next) {
					function->m_Next->m_Prev = function->m_Prev;
				} else {
					m_LeastRecentlyUsed = function->m_Prev;
				}

				function->m_Next = m_MostRecentlyUsed;
				function->m_Prev = 0;
				m_MostRecentlyUsed->m_Prev = function;
				m_MostRecentlyUsed = function;
			}

			return;
		}
	}

	// not found in cache, need to compile

	ppart.Compile(this, varyingInfo, state);
}

FunctionInfo * FunctionCache :: AllocateFunction(PipelinePart::Part part, const void * state, size_t size) {

	PipelinePart & ppart = PipelinePart::Get(part);

	if (size + m_Used >= m_Total || m_UsedFunctions >= m_MaxFunctions) {
		CompactCode();
	}

	assert(m_UsedFunctions < m_MaxFunctions);
	assert(size + m_Used < m_Total);

	FunctionInfo * function = m_Functions + m_UsedFunctions++;

	function->m_Next = m_MostRecentlyUsed;
	function->m_Prev = 0;

	if (m_MostRecentlyUsed) {
		m_MostRecentlyUsed->m_Prev = function;
	} else {
		m_LeastRecentlyUsed = function;
	}

	m_MostRecentlyUsed = function;

	function->m_Flags = FlagNone;
	ppart.CopyState(function->m_State, state);
	function->m_Part = part;

	return function;
}

void * FunctionCache :: BeginAddFunction(PipelinePart::Part part, const void * state, size_t size) {

	FunctionInfo * function = AllocateFunction(part, state, size);

	function->m_Offset = m_Used;
	function->m_Size = size;
	m_Used += size;

	return reinterpret_cast<void *>(m_Code + function->m_Offset);
}

void FunctionCache :: EndAddFunction(void * addr, size_t size) {
	SyncCache(addr, size);
}

void FunctionCache :: CompactCode() {

	size_t limit = (size_t) (m_Total * m_PercentageKeep);
	size_t limitFunctions = (size_t) ((m_MaxFunctions - m_MaxExternalFunctions) * m_PercentageKeep);

	size_t countExternalFunctions = 0;
	size_t countFunctions = 0;
	size_t countMemory = 0;

	// mark those functions that need to be retained
	FunctionInfo * function = m_MostRecentlyUsed;

	for (; function; function = function->m_Next) {
		if (function->m_Flags & FlagExternal) {
			++countExternalFunctions;
			continue;
		}

		if (function->m_Size + countMemory > limit ||
			countFunctions >= limitFunctions)
			break;

		function->m_Flags = 1;
		++countFunctions;
		countMemory += function->m_Size;
	}

	for (; function; function = function->m_Next) {
		if (function->m_Flags & FlagExternal) {
			++countExternalFunctions;
		} else {
			function->m_Flags = 0;
		}
	}

	assert(countExternalFunctions == m_UsedExternalFunctions);

	// now compact the list of functions

	m_Used = 0;

	FunctionInfo * target = m_Functions;

	for (function = m_Functions; function < m_Functions + m_UsedFunctions; ++function) {
		if (function->m_Flags) {
			if (function->m_Flags & FlagExternal) {
				target->m_Pointer = function->m_Pointer;
			} else {
				memmove(m_Code + m_Used, m_Code + function->m_Offset, function->m_Size);
				target->m_Offset = m_Used;
				m_Used += function->m_Size;
			}

			if (function != target) {
				memmove(target->m_State, function->m_State, sizeof(target->m_State));
			}

			target->m_Size = function->m_Size;
			target->m_Part = function->m_Part;

			++target;
		}
	}

	m_UsedFunctions = countFunctions + countExternalFunctions;

	// re-link LRU chain

	if (m_UsedFunctions == 0) {
		m_MostRecentlyUsed = m_LeastRecentlyUsed = 0;
	} else if (m_UsedFunctions == 1) {
		m_MostRecentlyUsed = m_LeastRecentlyUsed = m_Functions;
		m_Functions->m_Prev = m_Functions->m_Next = 0;
	} else if (m_UsedFunctions > 1) {
		m_MostRecentlyUsed = m_Functions;
		m_LeastRecentlyUsed = m_Functions + m_UsedFunctions - 1;
		m_Functions[0].m_Prev = m_Functions[m_UsedFunctions - 1].m_Next = 0;
		m_Functions[0].m_Next = m_Functions + 1;
		m_Functions[m_UsedFunctions - 1].m_Prev = m_Functions + m_UsedFunctions - 2;

		for (size_t index = 1; index < m_UsedFunctions - 1; ++index) {
			m_Functions[index].m_Prev = m_Functions + index - 1;
			m_Functions[index].m_Next = m_Functions + index + 1;
		}
	}

	SyncCache(m_Code, m_Used);
}

void FunctionCache :: SyncCache(void * base, size_t size) {
#if defined(EGL_ON_WINCE) && (defined(ARM) || defined(_ARM_))
	// flush data cache and clear instruction cache to make new code visible to execution unit
	CacheSync(CACHE_SYNC_INSTRUCTIONS | CACHE_SYNC_WRITEBACK);
#elif defined(ARM) && defined(__gnu_linux__)
	CLEAR_INSN_CACHE(targetBuffer, (U8 *) targetBuffer + cg_segment_size(cseg))
#endif

}

