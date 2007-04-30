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


namespace {
	struct InfoHeader {
		BITMAPINFOHEADER bmiHeader;
		DWORD            bmiColors[3];

		InfoHeader(ColorFormat colorFormat, U32 width, U32 height) {
			bmiHeader.biSize = sizeof(bmiHeader);
			bmiHeader.biWidth = width;
			bmiHeader.biHeight = height;
			bmiHeader.biPlanes = 1;
			bmiHeader.biCompression = BI_BITFIELDS;
			bmiHeader.biXPelsPerMeter = 72 * 25;
			bmiHeader.biYPelsPerMeter = 72 * 25;
			bmiHeader.biClrUsed = 0;
			bmiHeader.biClrImportant = 0;

			switch (colorFormat) {
			case ColorFormatRGB565:
				bmiHeader.biBitCount = 16;
				bmiHeader.biSizeImage = width * height * sizeof(U16);

				bmiColors[0] = Color(0xff, 0, 0, 0).ConvertTo565();
				bmiColors[1] = Color(0, 0xff, 0, 0).ConvertTo565();
				bmiColors[2] = Color(0, 0, 0xff, 0).ConvertTo565();
				break;

			case ColorFormatRGBA5551:
				bmiHeader.biBitCount = 16;
				bmiHeader.biSizeImage = width * height * sizeof(U16);

				bmiColors[0] = Color(0xff, 0, 0, 0).ConvertTo5551();
				bmiColors[1] = Color(0, 0xff, 0, 0).ConvertTo5551();
				bmiColors[2] = Color(0, 0, 0xff, 0).ConvertTo5551();
				break;

			case ColorFormatRGBA4444:
				bmiHeader.biBitCount = 16;
				bmiHeader.biSizeImage = width * height * sizeof(U16);

				bmiColors[0] = Color(0xff, 0, 0, 0).ConvertTo4444();
				bmiColors[1] = Color(0, 0xff, 0, 0).ConvertTo4444();
				bmiColors[2] = Color(0, 0, 0xff, 0).ConvertTo4444();
				break;

			case ColorFormatRGBA8:
				// this is realized as ARGB format
				bmiHeader.biBitCount = 32;
				bmiHeader.biSizeImage = width * height * sizeof(U32);

				bmiColors[0] = Color(0xff, 0, 0, 0).ConvertToRGBA();
				bmiColors[1] = Color(0, 0xff, 0, 0).ConvertToRGBA();
				bmiColors[2] = Color(0, 0, 0xff, 0).ConvertToRGBA();
				break;

			default:
				assert(false);
			}
		}
	};
}


Surface :: Surface(const Config & config, HDC hdc)
:	m_Config(config),
	m_Rect (0, 0, config.GetConfigAttrib(EGL_WIDTH), config.GetConfigAttrib(EGL_HEIGHT)),
	m_Bitmap(reinterpret_cast<HBITMAP>(INVALID_HANDLE_VALUE)),
	m_HDC(reinterpret_cast<HDC>(INVALID_HANDLE_VALUE))
{
	//m_ColorBuffer = new U16[m_Width * m_Height];
	U32 width = GetWidth();
	U32 height = GetHeight();

	m_Pitch = width;

	switch (m_Config.GetDepthStencilFormat()) {
	case DepthStencilFormatDepth16:
		m_DepthStencilBuffer = new U8[width * height * sizeof(U16)];
		break;

	case DepthStencilFormatDepth16Stencil16:
		m_DepthStencilBuffer = new U8[width * height * sizeof(U32)];
		break;

	default:
		m_DepthStencilBuffer = 0;
		assert(false);
		break;
	}

	if (hdc != INVALID_HANDLE_VALUE) {
		m_HDC = CreateCompatibleDC(hdc);
	}

	InfoHeader info(m_Config.GetColorFormat(), width, height);

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

	if (m_DepthStencilBuffer != 0) {
		delete[] m_DepthStencilBuffer;
		m_DepthStencilBuffer = 0;
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


namespace {

	template <class T> void FillRect(T * base, const Rect & bufferRect, const Rect & fillRect,
		const T& value, const T& mask) {
		Rect rect = Rect::Intersect(fillRect, bufferRect);

		base += fillRect.x + fillRect.y * bufferRect.width;
		size_t gap = bufferRect.width - fillRect.width;

		size_t rows = fillRect.height;
		T inverseMask = ~mask;
		T maskedValue = value & mask;

		while (rows--) {
			for (size_t columns = fillRect.width; columns > 0; columns--) {
				*base = (*base & inverseMask) | maskedValue;
				++base;
			}

			base += gap;
		}
	}

	template <class T> void FillRect(T * base, const Rect & bufferRect, const Rect & fillRect,
		const T& value) {
		Rect rect = Rect::Intersect(fillRect, bufferRect);

		base += fillRect.x + fillRect.y * bufferRect.width;
		size_t gap = bufferRect.width - fillRect.width;

		size_t rows = fillRect.height;

		while (rows--) {
			for (size_t columns = fillRect.width; columns > 0; columns--) {
				*base = value;
				++base;
			}

			base += gap;
		}
	}
}


void Surface :: ClearBuffer16(U8 * buffer, U16 value, U16 mask, const Rect& scissor) {
	if (mask == 0xffff) {
		FillRect((U16 *) buffer, GetRect(), scissor, value);
	} else {
		FillRect((U16 *) buffer, GetRect(), scissor, value, mask);
	}
}

void Surface :: ClearBuffer32(U8 * buffer, U32 value, U32 mask, const Rect& scissor) {
	if (mask == 0xffffffff) {
		FillRect((U32 *) buffer, GetRect(), scissor, value);
	} else {
		FillRect((U32 *) buffer, GetRect(), scissor, value, mask);
	}
}

void Surface :: ClearDepthStencilBuffer(U32 depth, bool depthMask, U32 stencil, U32 stencilMask, const Rect& scissor) {

	switch (GetDepthStencilFormat()) {
	case DepthStencilFormatDepth16:
		ClearBuffer16(m_DepthStencilBuffer, depth, depthMask ? 0xffff : 0, scissor);
		break;

	case DepthStencilFormatDepth16Stencil16:
		stencilMask &= 0xffff;
		depth &= 0xffff;
		stencil &= 0xffff;
		ClearBuffer32(m_DepthStencilBuffer, depth | (stencil << 16), (depthMask ? 0xffff : 0) | (stencilMask << 16), scissor);
		break;

	default:
		assert(false);
	}
}

void Surface :: ClearColorBuffer(const Color & rgba, const Color & mask, const Rect& scissor) {
	switch (GetColorFormat()) {
	case ColorFormatRGB565:
		ClearBuffer16(m_ColorBuffer, rgba.ConvertTo565(), mask.ConvertTo565(), scissor);
		break;

	case ColorFormatRGBA4444:
		ClearBuffer16(m_ColorBuffer, rgba.ConvertTo4444(), mask.ConvertTo4444(), scissor);
		break;

	case ColorFormatRGBA5551:
		ClearBuffer16(m_ColorBuffer, rgba.ConvertTo5551(), mask.ConvertTo5551(), scissor);
		break;

	case ColorFormatRGBA8:
		ClearBuffer32(m_ColorBuffer, rgba.ConvertToRGBA(), mask.ConvertToRGBA(), scissor);
		break;

	default:
		assert(false);
	}


}


bool Surface :: Save(const TCHAR * filename) {

	InfoHeader info(GetColorFormat(), GetWidth(), GetHeight());

    BITMAPFILEHEADER header;
    header.bfType      = 0x4d42;
    header.bfSize      = sizeof(BITMAPFILEHEADER) + sizeof(info) + info.bmiHeader.biSizeImage;
    header.bfReserved1 = 0;
    header.bfReserved2 = 0;
    header.bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(info);

    // Create the file
    HANDLE hFile = ::CreateFile(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD temp;

    // Write the header + bitmap info
    ::WriteFile(hFile, &header, sizeof(header), &temp, 0);
    ::WriteFile(hFile, &info, sizeof(info), &temp, 0);

    // Write the image
	switch (GetColorFormat()) {
	case ColorFormatRGB565:
	case ColorFormatRGBA5551:
	case ColorFormatRGBA4444:
		{
			U16 * pixels = (U16 *) m_ColorBuffer;

			for (int h = GetHeight(); h; --h) {
				::WriteFile(hFile, pixels, GetWidth() * sizeof(U16), &temp, 0 );
				pixels += GetWidth();
			}
		}

		break;

	case ColorFormatRGBA8:
		{
			U32 * pixels = (U32 *) m_ColorBuffer;

			for (int h = GetHeight(); h; --h) {
				::WriteFile(hFile, pixels, GetWidth() * sizeof(U32), &temp, 0 );
				pixels += GetWidth();
			}
		}

		break;

	default:
		assert(false);
	}

    ::CloseHandle(hFile);

    return true;
}

