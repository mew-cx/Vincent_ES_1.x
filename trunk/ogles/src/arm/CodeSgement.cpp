// ==========================================================================
//
// CodeSegment.cpp	A piece of runtime generated code that can be executed
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



#include "stdafx.h"
#include "CodeSegment.h"


using namespace EGL;


CodeSegment :: CodeSegment(U32 initialSize) {
	m_Size = initialSize;
	m_Base = reinterpret_cast<U32 *>(VirtualAlloc(0, m_Size, MEM_COMMIT, PAGE_EXECUTE_READWRITE));
}


CodeSegment :: ~CodeSegment() {

	if (m_Base != 0) {
		VirtualFree(m_Base, m_Size, MEM_RELEASE);
		m_Base = 0;
	}
}


void CodeSegment :: EnsureSize(U32 newSize) {

	if (newSize > m_Size) {
		if (m_Base != 0) {
			VirtualFree(m_Base, m_Size, MEM_RELEASE);
		}

		m_Size = newSize;
		m_Base = reinterpret_cast<U32 *>(VirtualAlloc(0, m_Size, MEM_COMMIT, PAGE_EXECUTE_READWRITE));
	}
}




