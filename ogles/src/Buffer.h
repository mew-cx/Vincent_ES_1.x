#ifndef EGL_BUFFER_H
#define EGL_BUFFER_H 1

// ==========================================================================
//
// Buffer.h			Buffer Object Class for 3D Rendering Library
//
// --------------------------------------------------------------------------
//
// 08-30-2004		Hans-Martin Will	initial version
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
#include "GLES/gl.h"


namespace EGL {

	class Buffer { 

	public:
		Buffer(): m_Data(0), m_Size(0) { }

		~Buffer() { 
			Deallocate();
		}

		bool Allocate(size_t size) {
			Deallocate();

			if (size) {
				m_Data = malloc(size);

				if (m_Data == 0) {
					return false;
				} else {
					m_Size = size;
					return true;
				}
			} else {
				return true;
			}
		}

		void Deallocate() {
			if (m_Data) {
				free(m_Data);
				m_Data = 0;
				m_Size = 0;
			}
		}

		void *	GetData() {
			return m_Data;
		}

		size_t GetSize() {
			return m_Size;
		}

	private:
		void *	m_Data;
		size_t	m_Size;
	};

}

#endif //ndef EGL_BUFFER_H
