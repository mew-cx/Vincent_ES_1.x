// ==========================================================================
//
// Surface.cpp		Drawing Surface Class for 3D Rendering Library
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


#include "stdafx.h"
#include "Surface.h"
#include "Color.h"
#include <string.h>


using namespace EGL;


Surface :: Surface(const Config & config, HDC hdc) 
:	m_Config(config),
	m_Width (config.GetConfigAttrib(EGL_WIDTH)), 
	m_Height (config.GetConfigAttrib(EGL_HEIGHT)),
	m_Bitmap(reinterpret_cast<HBITMAP>(INVALID_HANDLE_VALUE)),
	m_HDC(reinterpret_cast<HDC>(INVALID_HANDLE_VALUE))
{
	//m_ColorBuffer = new U16[m_Width * m_Height];
	m_AlphaBuffer = new U8[m_Width * m_Height];
	m_DepthBuffer = new I32[m_Width * m_Height];
	m_StencilBuffer = new U32[m_Width * m_Height];

	if (hdc != INVALID_HANDLE_VALUE) {
		m_HDC = CreateCompatibleDC(hdc);
	}

	struct {
		BITMAPINFOHEADER bmiHeader;
		DWORD            bmiColors[3];
	} info;

	info.bmiHeader.biSize = sizeof(info.bmiHeader); 
	info.bmiHeader.biWidth = m_Width; 
	info.bmiHeader.biHeight = m_Height; 
	info.bmiHeader.biPlanes = 1; 
	info.bmiHeader.biBitCount = 16; 
	info.bmiHeader.biCompression = BI_BITFIELDS; 
	info.bmiHeader.biSizeImage = m_Width * m_Height * sizeof(U16); 
	info.bmiHeader.biXPelsPerMeter = 72 * 25; 
	info.bmiHeader.biYPelsPerMeter = 72 * 25; 
	info.bmiHeader.biClrUsed = 0; 
	info.bmiHeader.biClrImportant = 0; 

	info.bmiColors[0] = 0xF800;
	info.bmiColors[1] = 0x07E0;
	info.bmiColors[2] = 0x001F;

	m_Bitmap = CreateDIBSection(m_HDC, reinterpret_cast<BITMAPINFO *>(&info), DIB_RGB_COLORS, 
		reinterpret_cast<void **>(&m_ColorBuffer), NULL, 0);
}


Surface :: ~Surface() {

	
	if (m_Bitmap != INVALID_HANDLE_VALUE) {
		DeleteObject(m_Bitmap);
		m_Bitmap = reinterpret_cast<HBITMAP>(INVALID_HANDLE_VALUE);
	}

	if (m_HDC != INVALID_HANDLE_VALUE) {
		DeleteDC(m_HDC);
		m_HDC = reinterpret_cast<HDC>(INVALID_HANDLE_VALUE);
	}

	/*if (m_ColorBuffer != 0) {
		delete [] m_ColorBuffer;
		m_ColorBuffer = 0;
	}*/

	if (m_AlphaBuffer != 0) {
		delete [] m_AlphaBuffer;
		m_AlphaBuffer = 0;
	}

	if (m_DepthBuffer != 0) {
		delete[] m_DepthBuffer;
		m_DepthBuffer = 0;
	}

	if (m_StencilBuffer != 0) {
		delete[] m_StencilBuffer;
		m_StencilBuffer = 0;
	}
}


void Surface :: Dispose() {
	if (GetCurrentContext() != 0) {
		m_Disposed = true;
	} else {
		delete this;
	}
}


void Surface :: SetCurrentContext(Context * context) {
	m_CurrentContext = context;

	if (context == 0 && m_Disposed) {
		delete this;
	}
}


void Surface :: ClearDepthBuffer(GLclampx depth) {
	I32 depthValue = depth;//DepthBitsFromDepth(depth);

	int count = GetPixels();
	I32 * ptr = m_DepthBuffer;

	while (--count > 0) {
		*ptr++ = depthValue;
	}
}


void Surface :: ClearStencilBuffer(U32 value) {
	int count = GetPixels();
	U32 * ptr = m_StencilBuffer;

	while (--count > 0) {
		*ptr++ = value;
	}
}

/*
I32 Surface :: DepthBitsFromDepth(GLclampx depth) {
	I32 result;
	gppMul_16_32s(EGL_CLAMP(depth, EGL_FIXED_0, EGL_FIXED_1), 0xffffff, &result);
	return result;
}
*/

void Surface :: ClearColorBuffer(const Color & rgba) {
	U16 color = rgba.ConvertTo565();

	int count = GetPixels();
	U16 * ptr = m_ColorBuffer;

	while (--count > 0) {
		*ptr++ = color;
	}
}

