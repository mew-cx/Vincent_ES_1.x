#ifndef EGL_ARRAYS_H
#define EGL_ARRAYS_H 1

// ==========================================================================
//
// Arrays.h			Array type declarations and helpers 
//
// --------------------------------------------------------------------------
//
// 05-26-2004		Hans-Martin Will	initial version
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
#include "Types.h"
#include "linalg.h"
#include "Texture.h"


namespace EGL {

	struct VertexArray {
		VertexArray() {
			pointer = 0;
			stride = 0;
			size = 4;
			size = 0;
		};

		GLfixed GetValue(int row, int column) {
			GLsizei rowOffset = row * stride;
			const unsigned char * base = reinterpret_cast<const unsigned char *>(pointer) + rowOffset;

			switch (type) {
			case GL_BYTE:
				return EGL_FixedFromInt(*(reinterpret_cast<const char *>(base) + column));

			case GL_SHORT:
				return EGL_FixedFromInt(*(reinterpret_cast<const short *>(base) + column));

			case GL_FIXED:
				return *(reinterpret_cast<const I32 *>(base) + column);

			case GL_FLOAT:
				return EGL_FixedFromFloat(*(reinterpret_cast<const float *>(base) + column));

			default:
				return 0;
			}
		}

		GLint			size;
		GLenum			type;
		GLsizei			stride;
		const GLvoid *	pointer;
	};

	struct TextureArray {

		struct TextureRecord {
			U32 value;

			void SetPointer(MultiTexture * texture) {
				value = reinterpret_cast<U32>(texture);
			}

			MultiTexture * GetPointer() {
				assert(IsPointer());
				return reinterpret_cast<MultiTexture *>(value);
			}

			bool IsPointer() {
				return (value & 1) == 0;
			}

			bool IsNil() {
				return value == 0xFFFFFFFFu;
			}

			void SetNil() {
				value = 0xFFFFFFFFu;
			}

			void SetIndex(size_t index) {
				value = (index << 1) | 1;
			}

			size_t GetIndex() {
				assert(!IsPointer() && !IsNil());
				return (value >> 1);
			}

		};

		enum {
			INITIAL_SIZE = 64,
			FACTOR = 2
		};

		TextureArray() {
			m_Textures = new TextureRecord[INITIAL_SIZE];

			for (size_t index = 0; index < INITIAL_SIZE; ++index) {
				m_Textures[index].SetIndex(index + 1);
			}

			m_Textures[INITIAL_SIZE - 1].SetNil();

			m_FreeTextures = m_AllocatedTextures = INITIAL_SIZE;
			m_FreeListHead = 0;
		}

		~TextureArray() {

			if (m_Textures != 0) {
				for (size_t index = 0; index < m_AllocatedTextures; ++index) {
					if (m_Textures[index].IsPointer() && m_Textures[index].GetPointer())
						delete m_Textures[index].GetPointer();
				}

				delete [] m_Textures;
			}
		}

		void Increase() {

			assert(m_FreeListHead == 0xffffffffu);

			size_t newAllocatedTextures = m_AllocatedTextures * FACTOR;

			TextureRecord * newTextures = new TextureRecord[newAllocatedTextures];

			for (size_t index = 0; index < m_AllocatedTextures; ++index) {
				newTextures[index] = m_Textures[index];
			}

			for (index = m_AllocatedTextures; index < newAllocatedTextures - 1; ++index) {
				newTextures[index].SetIndex(index + 1);
			}

			newTextures[newAllocatedTextures - 1].SetNil();

			delete [] m_Textures;
			m_Textures = newTextures;
			m_FreeTextures = newAllocatedTextures - m_AllocatedTextures;
			m_FreeListHead = m_AllocatedTextures;
			m_AllocatedTextures = newAllocatedTextures;
		}

		size_t Allocate() {

			if (m_FreeTextures == 0) {
				Increase();
				assert(m_FreeTextures != 0);
			}

			size_t result = m_FreeListHead;
			m_FreeListHead = m_Textures[result].IsNil() ? 0xffffffffu : m_Textures[result].GetIndex();
			m_FreeTextures--;
			m_Textures[result].SetPointer(0);

			return result;
		}

		void Deallocate(size_t index) {

			if (m_Textures[index].IsPointer()) {
				if (m_Textures[index].GetPointer())
					delete m_Textures[index].GetPointer();

				m_Textures[index].SetIndex(m_FreeListHead);
				m_FreeListHead = index;
				m_FreeTextures++;
			}
		}

		MultiTexture * GetTexture(size_t index) {

			if (index >= m_AllocatedTextures || !m_Textures[index].IsPointer()) 
				return 0;

			if (!m_Textures[index].GetPointer()) {
				m_Textures[index].SetPointer(new MultiTexture());
			}

			return m_Textures[index].GetPointer();
		}

		TextureRecord *		m_Textures;
		size_t				m_FreeTextures;
		size_t				m_AllocatedTextures;
		size_t				m_FreeListHead;
	};
}



#endif //ndef EGL_ARRAYS_H