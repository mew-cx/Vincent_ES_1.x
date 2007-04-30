#ifndef EGL_SURFACE_H
#define EGL_SURFACE_H 1

// ==========================================================================
//
// Surface.h		Drawing Surface Class for 3D Rendering Library
//
// --------------------------------------------------------------------------
//
// 08-14-2003		Hans-Martin Will	initial version
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
#include "GLES/egl.h"
#include "GLES/gl.h"
#include "Types.h"
#include "Config.h"
#include "fixed.h"
#include "Color.h"


namespace EGL {

	class Surface {
		friend Context;

	public:
		// Create a PBuffer surface
		Surface(const Config & config, HDC hdc = 0);
		~Surface();

		// Is the depth value re-scaled based on near/far settings?.
		void ClearDepthStencilBuffer(U32 depth, bool depthMask, U32 stencil, U32 stencilMask, const Rect& scissor);
		void ClearColorBuffer(const Color & rgba, const Color & mask, const Rect& scissor);

		void ClearDepthStencilBuffer(U32 depth, bool depthMask, U32 stencil, U32 stencilMask);
		void ClearColorBuffer(const Color & rgba, const Color & mask);

		U16 GetWidth() const;
		U16 GetHeight() const;
		U32 GetPixels() const;
		U32 GetPitch() const;
		const Rect& GetRect() const;

		void SetCurrentContext(Context * context);
		Context * GetCurrentContext();

		U8 * GetColorBuffer();
		U8 * GetDepthStencilBuffer();

		Config * GetConfig();

		bool Save(const TCHAR * filename);
		void Dispose();

		// Windows integration
		HDC GetMemoryDC();
		HBITMAP GetBitmap();

		ColorFormat GetColorFormat() const;
		DepthStencilFormat GetDepthStencilFormat() const;

	private:
		void ClearBuffer16(U8 * buffer, U16 value, U16 mask, const Rect& scissor);
		void ClearBuffer32(U8 * buffer, U32 value, U32 mask, const Rect& scissor);

	private:
		HDC		m_HDC;				// windows device context handle
		HBITMAP	m_Bitmap;			// windows bitmap handle
		Config	m_Config;			// configuration arguments
		U8 *	m_ColorBuffer;		// pointer to frame buffer base address
		U8 *	m_DepthStencilBuffer;	// pointer to Z/stencil-buffer base address

		Rect	m_Rect;
		U32		m_Pitch;			// increment top move from y to y + 1

		Context *	m_CurrentContext;

		bool	m_Disposed;			// the surface 
	};


	// --------------------------------------------------------------------------
	// Inline accessors
	// --------------------------------------------------------------------------


	inline Config * Surface :: GetConfig() {
		return &m_Config;
	}

	inline U32 Surface :: GetPixels() const {
		return GetWidth() * GetHeight();
	}

	inline Context * Surface :: GetCurrentContext() {
		return m_CurrentContext;
	}

	inline HDC Surface :: GetMemoryDC() {
		return m_HDC;
	}

	inline HBITMAP Surface :: GetBitmap() {
		return m_Bitmap;
	}

	inline U8 * Surface :: GetColorBuffer() {
		return m_ColorBuffer;
	}

	inline U8 * Surface :: GetDepthStencilBuffer() {
		return m_DepthStencilBuffer;
	}

	inline U16 Surface :: GetWidth() const {
		return m_Rect.width;
	}

	inline U16 Surface :: GetHeight() const {
		return m_Rect.height;
	}

	inline U32 Surface :: GetPitch() const {
		return m_Pitch;
	}

	inline const Rect& Surface :: GetRect() const {
		return m_Rect;
	}

	inline void Surface :: ClearDepthStencilBuffer(U32 depth, bool depthMask, U32 stencil, U32 stencilMask) {
		ClearDepthStencilBuffer(depth, depthMask, stencil, stencilMask, GetRect());
	}

	inline void Surface :: ClearColorBuffer(const Color & rgba, const Color & mask) {
		ClearColorBuffer(rgba, mask, GetRect());
	}

	inline ColorFormat Surface :: GetColorFormat() const {
		return m_Config.GetColorFormat();
	}

	inline DepthStencilFormat Surface :: GetDepthStencilFormat() const {
		return m_Config.GetDepthStencilFormat();
	}
}

#endif // ndef EGL_SURFACE_H