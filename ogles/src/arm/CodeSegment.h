#ifndef EGL_CODE_SEGMENT_H
#define EGL_CODE_SEGMENT_H 1

#pragma once

// ==========================================================================
//
// CodeSegment.h	A piece of runtime generated code that can be executed
//				
//					This class is part of the runtime compiler infrastructure
//					used by the OpenGL|ES implementation for compiling
//					shader code at runtime into machine language.
//
// --------------------------------------------------------------------------
//
// 11-13-2003		Hans-Martin Will	initial version
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


namespace EGL {

	// -----------------------------------------------------------------------
	// A container for runtime generated code
	// -----------------------------------------------------------------------
	class OGLES_API CodeSegment {

		typedef void (*Function)();

	public:
		CodeSegment(U32 initialSize);
		~CodeSegment();

		void EnsureSize(U32 newSize);

		U32 * GetBase()					{ return m_Base; }
		const U32 * GetBase() const		{ return m_Base; }

		U32 GetSize() const				{ return m_Size; }

		void SetOffset(U32 offset)		{ m_Offset = offset; }
		U32 GetOffset() const			{ return m_Offset; }

		Function GetEntryPoint()			{ 
			return reinterpret_cast<Function>(
				m_Base + (m_Offset / sizeof(U32))); 
		}

		void Execute()					{ (GetEntryPoint())(); }

	private:
		U32 * m_Base;
		U32 m_Size;
		U32 m_Offset;
	};

}

#endif //ndef EGL_CODE_SEGMENT_H
