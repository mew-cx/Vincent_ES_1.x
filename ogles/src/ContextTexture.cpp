// ==========================================================================
//
// matrix.cpp	Rendering Context Class for 3D Rendering Library
//
//				Texturing Functions
//
// --------------------------------------------------------------------------
//
// 08-07-2003	Hans-Martin Will	initial version
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
#include "Context.h"
#include "Color.h"
#include "Surface.h"
#include "fixed.h"


using namespace EGL;


// --------------------------------------------------------------------------
// Allocation and selection of texture objects
// --------------------------------------------------------------------------

void Context :: BindTexture(GLenum target, GLuint texture) { 
	if (target != GL_TEXTURE_2D) {
		return;
	}

	if (texture >= m_Textures.size() || m_Textures[texture] == 0) {
		// allocate a new texture
		if (m_Textures.size() < texture) {
			U32 newSize = m_Textures.size() * 2;

			if (newSize <= texture) {
				newSize = texture + 1;
			}

			m_Textures.resize(newSize);
			m_Textures[texture] = new MultiTexture();
			m_Textures[texture]->SetWrappingModeS(GetRasterizerState()->GetWrappingModeS());
			m_Textures[texture]->SetWrappingModeT(GetRasterizerState()->GetWrappingModeT());
			m_Textures[texture]->SetMinFilterMode(GetRasterizerState()->GetMinFilterMode());
			m_Textures[texture]->SetMagFilterMode(GetRasterizerState()->GetMagFilterMode());
		}
	}

	GetRasterizer()->SetTexture(m_Textures[texture]);
}

void Context :: DeleteTextures(GLsizei n, const GLuint *textures) { 

	while (n-- != 0) {
		U32 texture = *textures++;

		if (texture != 0) {
			if (texture < m_Textures.size() && m_Textures[texture] != 0) {
				if (m_Textures[texture] == GetRasterizer()->GetTexture()) {
					GetRasterizer()->SetTexture(m_Textures[0]);
				}

				delete m_Textures[texture];
				m_Textures[texture] = 0;
			}
		}
	}
}

void Context :: GenTextures(GLsizei n, GLuint *textures) { 

	U32 nextIndex = 1;

	for (; n != 0 && nextIndex < m_Textures.size(); ++nextIndex) {
		if (m_Textures[nextIndex] == 0) {
			*textures++ = nextIndex;
			--n;
		}
	}
	
	while (n != 0) {
		*textures++ = nextIndex;
		--n;
	}
}

// --------------------------------------------------------------------------
// Texture specification methods
// --------------------------------------------------------------------------

namespace {
	RasterizerState::TextureFormat TextureFormatFromEnum(GLenum format) {
		switch (format) {
			case GL_ALPHA:
				return RasterizerState::TextureFormatAlpha;

			case 1:
			case GL_LUMINANCE:
				return RasterizerState::TextureFormatLuminance;

			case 2:
			case GL_LUMINANCE_ALPHA:
				return RasterizerState::TextureFormatLuminanceAlpha;

			case 3:
			case GL_RGB:
				return RasterizerState::TextureFormatRGB;

			case 4:
			case GL_RGBA:
				return RasterizerState::TextureFormatRGBA;

			default:
				return RasterizerState::TextureFormatInvalid;
		}
	}

	GLenum InternalTypeForInternalFormat(RasterizerState::TextureFormat format) {
		switch (format) {
			case RasterizerState::TextureFormatAlpha:
			case RasterizerState::TextureFormatLuminance:
			case RasterizerState::TextureFormatLuminanceAlpha:
			default:
				return GL_UNSIGNED_BYTE;

			case RasterizerState::TextureFormatRGB:
				return GL_UNSIGNED_SHORT_5_6_5;

			case RasterizerState::TextureFormatRGBA:
				return GL_UNSIGNED_SHORT_5_5_5_1;
		}
	}

	RasterizerState::WrappingMode WrappingModeFromEnum(GLenum mode) {
		switch (mode) {
			case GL_CLAMP_TO_EDGE:	return RasterizerState::WrappingModeClampToEdge;
			case GL_REPEAT:			return RasterizerState::WrappingModeRepeat;
			default:				return RasterizerState::WrappingModeInvalid;
		}
	}

	RasterizerState::MinFilterMode MinFilterModeFromEnum(GLenum mode) {
		switch (mode) {
			case GL_NEAREST:				return RasterizerState::MinFilterModeNearest;
			case GL_LINEAR:					return RasterizerState::MinFilterModeLinear;
			case GL_NEAREST_MIPMAP_LINEAR:	return RasterizerState::MinFilterModeNearestMipmapLinear;
			case GL_NEAREST_MIPMAP_NEAREST:	return RasterizerState::MinFilterModeNearestMipmapNearest;
			case GL_LINEAR_MIPMAP_LINEAR:	return RasterizerState::MinFilterModeLinearMipmapLinear;
			case GL_LINEAR_MIPMAP_NEAREST:	return RasterizerState::MinFilterModeLinearMipmapNearest;
			default:						return RasterizerState::MinFilterModeInvalid;
		}
	}

	RasterizerState::MagFilterMode MagFilterModeFromEnum(GLenum mode) {
		switch (mode) {
			case GL_NEAREST:		return RasterizerState::MagFilterModeNearest;
			case GL_LINEAR:			return RasterizerState::MagFilterModeLinear;
			default:				return RasterizerState::MagFilterModeInvalid;
		}
	}

	// -------------------------------------------------------------------------
	// Given two bitmaps src and dst, where src has dimensions 
	// (srcWidth * srcHeight) and dst has dimensions (dstWidth * dstHeight),
	// copy the rectangle (srcX, srcY, copyWidth, copyHeight) into
	// dst at location (dstX, dstY).
	//
	// It is assumed that the copy rectangle is non-empty and has been clipped
	// against the src and target rectangles
	// -------------------------------------------------------------------------
	template<class PixelType> 
	void CopyPixels(const void * src, U32 srcWidth, U32 srcHeight, 
					U32 srcX, U32 srcY, U32 copyWidth, U32 copyHeight,
					void * dst, U32 dstWidth, U32 dstHeight, U32 dstX, U32 dstY) {

		U32 srcGap = srcWidth - copyWidth;	// how many pixels to skip for next line
		U32 dstGap = dstWidth - copyWidth;	// how many pixels to skip for next line

		const PixelType * srcPtr = reinterpret_cast<const PixelType *>(src) + srcX;
		PixelType * dstPtr = reinterpret_cast<PixelType *>(dst) + dstX;

		do {
			U32 span = copyWidth;

			do {
				*dstPtr++ = *srcPtr++;
			} while (--span);

			srcPtr += srcGap;
			dstPtr += dstGap;
		} while (--copyHeight);
	}

	struct RGB2Color {
		enum {
			baseIncr = 3
		};

		typedef U8 BaseType;

		Color operator()(const BaseType * &ptr) {
			U8 r = *ptr++;
			U8 g = *ptr++;
			U8 b = *ptr++;
			return Color(r, g, b, 0);
		}
	};

	struct Color2RGB {
		enum {
			baseIncr = 3
		};

		typedef U8 BaseType;

		void operator()(BaseType * &ptr, const Color& value) {
			*ptr++ = value.R();
			*ptr++ = value.G();
			*ptr++ = value.B();
		}
	};

	struct RGBA2Color {
		enum {
			baseIncr = 4
		};

		typedef U8 BaseType;

		Color operator()(const BaseType * &ptr) {
			U8 r = *ptr++;
			U8 g = *ptr++;
			U8 b = *ptr++;
			U8 a = *ptr++;
			return Color(r, g, b, a);
		}
	};

	struct Color2RGBA {
		enum {
			baseIncr = 4
		};

		typedef U8 BaseType;

		void operator()(BaseType * &ptr, const Color& value) {
			*ptr++ = value.R();
			*ptr++ = value.G();
			*ptr++ = value.B();
			*ptr++ = value.A();
		}
	};

	struct RGBA44442Color {
		enum {
			baseIncr = 1
		};

		typedef U16 BaseType;

		Color operator()(const BaseType * &ptr) {
			return Color::From4444(*ptr++);
		}
	};

	struct Color2RGBA4444 {
		enum {
			baseIncr = 1
		};

		typedef U16 BaseType;

		void operator()(BaseType * &ptr, const Color& value) {
			*ptr++ = value.ConvertTo4444();
		}
	};

	struct Color2RGB565 {
		enum {
			baseIncr = 1
		};

		typedef U16 BaseType;

		void operator()(BaseType * &ptr, const Color& value) {
			*ptr++ = value.ConvertTo565();
		}
	};

	struct RGB5652Color {
		enum {
			baseIncr = 1
		};

		typedef U16 BaseType;

		Color operator()(const BaseType * &ptr) {
			return Color::From565(*ptr++);
		}
	};

	struct RGBA55512Color {
		enum {
			baseIncr = 1
		};

		typedef U16 BaseType;

		Color operator()(const BaseType * &ptr) {
			return Color::From5551(*ptr++);
		}
	};

	struct Color2RGBA5551 {
		enum {
			baseIncr = 1
		};

		typedef U16 BaseType;

		void operator()(BaseType * &ptr, const Color& value) {
			*ptr++ = value.ConvertTo5551();
		}
	};

	// -------------------------------------------------------------------------
	// Given two bitmaps src and dst, where src has dimensions 
	// (srcWidth * srcHeight) and dst has dimensions (dstWidth * dstHeight),
	// copy the rectangle (srcX, srcY, copyWidth, copyHeight) into
	// dst at location (dstX, dstY).
	//
	// It is assumed that the copy rectangle is non-empty and has been clipped
	// against the src and target rectangles
	// -------------------------------------------------------------------------
	template<class SrcAccessor, class DstAccessor> 
	void CopyPixelsA(const void * src, U32 srcWidth, U32 srcHeight, 
					U32 srcX, U32 srcY, U32 copyWidth, U32 copyHeight,
					void * dst, U32 dstWidth, U32 dstHeight, U32 dstX, U32 dstY) {

		typedef typename SrcAccessor::BaseType SrcBaseType;
		typedef typename DstAccessor::BaseType DstBaseType;

		SrcAccessor srcAccessor;
		DstAccessor dstAccessor;

		U32 srcGap = srcWidth - copyWidth;	// how many pixels to skip for next line
		U32 dstGap = dstWidth - copyWidth;	// how many pixels to skip for next line

		const SrcBaseType * srcPtr = reinterpret_cast<const SrcBaseType *>(src) + srcX;
		DstBaseType * dstPtr = reinterpret_cast<DstBaseType *>(dst) + dstX;

		do {
			U32 span = copyWidth;

			do {
				dstAccessor(dstPtr, srcAccessor(srcPtr));
			} while (--span);

			srcPtr += srcGap * SrcAccessor::baseIncr;
			dstPtr += dstGap * DstAccessor::baseIncr;
		} while (--copyHeight);
	}

	// -------------------------------------------------------------------------
	// Given two bitmaps src and dst, where src has dimensions 
	// (srcWidth * srcHeight) and dst has dimensions (dstWidth * dstHeight),
	// copy the rectangle (srcX, srcY, copyWidth, copyHeight) into
	// dst at location (dstX, dstY).
	//
	// The texture format and the type of the source format a given as
	// parameters.
	// -------------------------------------------------------------------------
	void CopyPixels(const void * src, U32 srcWidth, U32 srcHeight, 
					U32 srcX, U32 srcY, U32 copyWidth, U32 copyHeight,
					void * dst, U32 dstWidth, U32 dstHeight, U32 dstX, U32 dstY,
					RasterizerState::TextureFormat format, GLenum srcType, GLenum dstType) {

		// ---------------------------------------------------------------------
		// clip lower left corner
		// ---------------------------------------------------------------------

		if (srcX >= srcWidth || srcY >= srcHeight ||
			dstX >= dstWidth || dstY >= dstHeight ||
			copyWidth == 0 || copyHeight == 0) {
			return;
		}

		// ---------------------------------------------------------------------
		// clip the copy rectangle to the valid size
		// ---------------------------------------------------------------------

		if (srcX + copyWidth > dstX + dstWidth) {
			copyWidth = dstX + dstWidth - srcX;
		}

		if (srcX + copyWidth > srcX + srcWidth) {
			copyWidth = srcWidth - srcX;
		}

		if (srcY + copyHeight > dstY + dstHeight) {
			copyHeight = dstY + dstHeight - srcY;
		}

		if (srcY + copyHeight > srcY + srcHeight) {
			copyHeight = srcHeight - srcY;
		}

		// ---------------------------------------------------------------------
		// at this point we know that the copy rectangle is valid and non-empty
		// ---------------------------------------------------------------------

			
		switch (format) {
			case RasterizerState::TextureFormatAlpha:
			case RasterizerState::TextureFormatLuminance:
				CopyPixels<U8>(src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
					dst, dstWidth, dstHeight, dstX, dstY);
				break;

			case RasterizerState::TextureFormatLuminanceAlpha:
				CopyPixels<U16>(src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
					dst, dstWidth, dstHeight, dstX, dstY);
				break;

			case RasterizerState::TextureFormatRGB:
				switch (srcType) {
					case GL_UNSIGNED_BYTE:
						switch (dstType) {
							case GL_UNSIGNED_BYTE:
								CopyPixelsA<RGB2Color, Color2RGB>(src, srcWidth, srcHeight, srcX, srcY, 
									copyWidth, copyHeight, dst, dstWidth, dstHeight, dstX, dstY);
								break;

							case GL_UNSIGNED_SHORT_5_6_5:
								CopyPixelsA<RGB2Color, Color2RGB565>(src, srcWidth, srcHeight, srcX, srcY, 
									copyWidth, copyHeight, dst, dstWidth, dstHeight, dstX, dstY);
								break;
						}
						break;

					case GL_UNSIGNED_SHORT_5_6_5:
						switch (dstType) {
							case GL_UNSIGNED_BYTE:
								CopyPixelsA<RGB5652Color, Color2RGB>(src, srcWidth, srcHeight, srcX, srcY, 
									copyWidth, copyHeight, dst, dstWidth, dstHeight, dstX, dstY);
								break;

							case GL_UNSIGNED_SHORT_5_6_5:
								CopyPixels<U16>(src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
									dst, dstWidth, dstHeight, dstX, dstY);
								break;
						}
						break;
				}

				break;

			case RasterizerState::TextureFormatRGBA:
				switch (srcType) {
					case GL_UNSIGNED_BYTE:
						switch (dstType) {
							case GL_UNSIGNED_BYTE:
								CopyPixelsA<RGBA2Color, Color2RGBA>(src, srcWidth, srcHeight, srcX, srcY, 
									copyWidth, copyHeight, dst, dstWidth, dstHeight, dstX, dstY);
								break;
							case GL_UNSIGNED_SHORT_5_5_5_1:
								CopyPixelsA<RGBA2Color, Color2RGBA5551>(src, srcWidth, srcHeight, srcX, srcY, 
									copyWidth, copyHeight, dst, dstWidth, dstHeight, dstX, dstY);
								break;
							case GL_UNSIGNED_SHORT_4_4_4_4:
								CopyPixelsA<RGBA2Color, Color2RGBA4444>(src, srcWidth, srcHeight, srcX, srcY, 
									copyWidth, copyHeight, dst, dstWidth, dstHeight, dstX, dstY);
								break;
						}
						break;

					case GL_UNSIGNED_SHORT_5_5_5_1:
						switch (dstType) {
							case GL_UNSIGNED_BYTE:
								CopyPixelsA<RGBA55512Color, Color2RGBA>(src, srcWidth, srcHeight, srcX, srcY, 
									copyWidth, copyHeight, dst, dstWidth, dstHeight, dstX, dstY);
								break;
							case GL_UNSIGNED_SHORT_5_5_5_1:
								CopyPixels<U16>(src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
									dst, dstWidth, dstHeight, dstX, dstY);
								break;
							case GL_UNSIGNED_SHORT_4_4_4_4:
								CopyPixelsA<RGBA55512Color, Color2RGBA4444>(src, srcWidth, srcHeight, srcX, srcY, 
									copyWidth, copyHeight, dst, dstWidth, dstHeight, dstX, dstY);
								break;
						}
						break;

					case GL_UNSIGNED_SHORT_4_4_4_4:
						switch (dstType) {
							case GL_UNSIGNED_BYTE:
								CopyPixelsA<RGBA44442Color, Color2RGBA>(src, srcWidth, srcHeight, srcX, srcY, 
									copyWidth, copyHeight, dst, dstWidth, dstHeight, dstX, dstY);
								break;
							case GL_UNSIGNED_SHORT_5_5_5_1:
								CopyPixelsA<RGBA44442Color, Color2RGBA5551>(src, srcWidth, srcHeight, srcX, srcY, 
									copyWidth, copyHeight, dst, dstWidth, dstHeight, dstX, dstY);
								break;
							case GL_UNSIGNED_SHORT_4_4_4_4:
								CopyPixels<U16>(src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
									dst, dstWidth, dstHeight, dstX, dstY);
								break;
						}
						break;
				}

				break;

		}
	}

	// -------------------------------------------------------------------------
	// Ensure that the two formats (internalFormat) and
	// (externalFormat, type) are assignment compatible
	//
	// Parameters:
	//	internalFormat	-	the format of the target texture
	//	externalFormat	-	the external bitmap format
	//	type			-	the external bitmap type
	// -------------------------------------------------------------------------
	bool ValidateFormats(RasterizerState::TextureFormat internalFormat,
		RasterizerState::TextureFormat externalFormat, GLenum type) {
		if (internalFormat == RasterizerState::TextureFormatInvalid || 
			externalFormat == RasterizerState::TextureFormatInvalid || 
			internalFormat != externalFormat) {
			return false;
		}

		switch (internalFormat) {
			case RasterizerState::TextureFormatAlpha:
			case RasterizerState::TextureFormatLuminance:
			case RasterizerState::TextureFormatLuminanceAlpha:
				if (type != GL_UNSIGNED_BYTE) {
					return false;
				}

				break;

			case RasterizerState::TextureFormatRGB:
				if (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT_5_6_5) {
					return false;
				}

				break;

			case RasterizerState::TextureFormatRGBA:
				if (type != GL_UNSIGNED_BYTE &&
					type != GL_UNSIGNED_SHORT_4_4_4_4 &&
					type != GL_UNSIGNED_SHORT_5_5_5_1) {
					return false;
				}

				break;

		}

		return true;
	}

	GLenum TypeForInternalFormat(RasterizerState::TextureFormat format) {
		switch (format) {
			default:
			case RasterizerState::TextureFormatAlpha:
			case RasterizerState::TextureFormatLuminance:
			case RasterizerState::TextureFormatLuminanceAlpha:
				return GL_UNSIGNED_BYTE;

			case RasterizerState::TextureFormatRGB:
				return GL_UNSIGNED_SHORT_5_6_5;

			case RasterizerState::TextureFormatRGBA:
				return GL_UNSIGNED_SHORT_5_5_5_1;
		}
	}
}

void Context :: CompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data) { 

	RasterizerState::TextureFormat internalFormat = TextureFormatFromEnum(internalformat);
	TexImage2D(target, level, internalformat, width, height, border, 
		internalformat, TypeForInternalFormat(internalFormat), data);
}

void Context :: CompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data) { 

	if (target != GL_TEXTURE_2D) {
		return;
	}

	if (level < 0 || level >= MultiTexture::MAX_LEVELS) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	MultiTexture * multiTexture = GetCurrentTexture();
	Texture * texture = multiTexture->GetTexture(level);

	RasterizerState::TextureFormat internalFormat = texture->GetInternalFormat();
	TexSubImage2D(target, level, xoffset, yoffset, width, height, 
		internalFormat, TypeForInternalFormat(internalFormat), data);
}


void Context :: TexImage2D(GLenum target, GLint level, GLint internalformat, 
						   GLsizei width, GLsizei height, GLint border, 
						   GLenum format, GLenum type, const GLvoid *pixels) { 

	if (target != GL_TEXTURE_2D) {
		return;
	}

	if (level < 0 || level >= MultiTexture::MAX_LEVELS) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	RasterizerState::TextureFormat internalFormat = TextureFormatFromEnum(internalformat);
	RasterizerState::TextureFormat externalFormat = TextureFormatFromEnum(format);

	if (!ValidateFormats(internalFormat, externalFormat, type)) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	MultiTexture * multiTexture = GetCurrentTexture();
	Texture * texture = multiTexture->GetTexture(level);
	texture->Initialize(width, height, internalFormat);

	if (!level) {
		GetRasterizerState()->SetInternalFormat(internalFormat);
	}

	CopyPixels(const_cast<const void *>(pixels), width, height, 0, 0, width, height,
		texture->GetData(), width, height, 0, 0, internalFormat, type,
		InternalTypeForInternalFormat(internalFormat));
}

void Context :: TexSubImage2D(GLenum target, GLint level, 
							  GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, 
							  GLenum format, GLenum type, const GLvoid *pixels) { 

	if (target != GL_TEXTURE_2D) {
		return;
	}

	if (level < 0 || level >= MultiTexture::MAX_LEVELS) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	MultiTexture * multiTexture = GetCurrentTexture();
	Texture * texture = multiTexture->GetTexture(level);

	RasterizerState::TextureFormat internalFormat = texture->GetInternalFormat();
	RasterizerState::TextureFormat externalFormat = TextureFormatFromEnum(format);

	if (!ValidateFormats(internalFormat, externalFormat, type)) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	CopyPixels(const_cast<const void *>(pixels), width, height, 0, 0, width, height,
		texture->GetData(), texture->GetWidth(), texture->GetHeight(),
		xoffset, yoffset, internalFormat, type, InternalTypeForInternalFormat(internalFormat));
}

void Context :: CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, 
							   GLint x, GLint y, GLsizei width, GLsizei height, GLint border) { 
	if (target != GL_TEXTURE_2D) {
		return;
	}

	Surface * readSurface = GetReadSurface();

	if (level < 0 || level >= MultiTexture::MAX_LEVELS) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	RasterizerState::TextureFormat internalFormat = TextureFormatFromEnum(internalformat);

	// These parameters really depend on the actual reading surface, and should
	// be determined from there
	RasterizerState::TextureFormat externalFormat = RasterizerState::TextureFormatRGB;
	GLenum type = GL_UNSIGNED_SHORT_5_6_5;

	if (!ValidateFormats(internalFormat, externalFormat, type)) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	MultiTexture * multiTexture = GetCurrentTexture();
	Texture * texture = multiTexture->GetTexture(level);
	texture->Initialize(width, height, internalFormat);

	CopyPixels(readSurface->GetColorBuffer(), readSurface->GetWidth(), readSurface->GetHeight(), 
				0, 0, width, height,
				texture->GetData(), width, height, 0, 0, internalFormat, type,
				InternalTypeForInternalFormat(internalFormat));
}

void Context :: CopyTexSubImage2D(GLenum target, GLint level, 
								  GLint xoffset, GLint yoffset, 
								  GLint x, GLint y, GLsizei width, GLsizei height) { 
	if (target != GL_TEXTURE_2D) {
		return;
	}

	Surface * readSurface = GetReadSurface();

	if (level < 0 || level >= MultiTexture::MAX_LEVELS) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	MultiTexture * multiTexture = GetCurrentTexture();
	Texture * texture = multiTexture->GetTexture(level);
	RasterizerState::TextureFormat internalFormat = texture->GetInternalFormat();

	// These parameters really depend on the actual reading surface, and should
	// be determined from there
	RasterizerState::TextureFormat externalFormat = RasterizerState::TextureFormatRGB;
	GLenum type = GL_UNSIGNED_SHORT_5_6_5;

	if (!ValidateFormats(internalFormat, externalFormat, type)) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	CopyPixels(readSurface->GetColorBuffer(), readSurface->GetWidth(), readSurface->GetHeight(), 
				0, 0, width, height,
				texture->GetData(), texture->GetWidth(), texture->GetHeight(), 0, 0, 
				internalFormat, type, InternalTypeForInternalFormat(internalFormat));
}

// --------------------------------------------------------------------------
// Texture parameters
// --------------------------------------------------------------------------

void Context :: TexParameterx(GLenum target, GLenum pname, GLfixed param) { 
	if (target != GL_TEXTURE_2D) {
		return;
	}

	MultiTexture * multiTexture = GetCurrentTexture();
	int intParam = EGL_IntFromFixed(param);

	switch (pname) {
		case GL_TEXTURE_MIN_FILTER:
			{
				RasterizerState::MinFilterMode mode = MinFilterModeFromEnum(intParam);

				if (mode != RasterizerState::MinFilterModeInvalid) {
					multiTexture->SetMinFilterMode(mode);
					GetRasterizerState()->SetMinFilterMode(mode);
				} else {
					RecordError(GL_INVALID_VALUE);
				}
			}
			break;

		case GL_TEXTURE_MAG_FILTER:
			{
				RasterizerState::MagFilterMode mode = MagFilterModeFromEnum(intParam);

				if (mode != RasterizerState::MagFilterModeInvalid) {
					multiTexture->SetMagFilterMode(mode);
					GetRasterizerState()->SetMagFilterMode(mode);
				} else {
					RecordError(GL_INVALID_VALUE);
				}
			}
			break;

		case GL_TEXTURE_WRAP_S:
			{
				RasterizerState::WrappingMode mode = WrappingModeFromEnum(intParam);

				if (mode != RasterizerState::WrappingModeInvalid) {
					multiTexture->SetWrappingModeS(mode);
					GetRasterizerState()->SetWrappingModeS(mode);
				} else {
					RecordError(GL_INVALID_VALUE);
				}
			}
			break;

		case GL_TEXTURE_WRAP_T:
			{
				RasterizerState::WrappingMode mode = WrappingModeFromEnum(intParam);

				if (mode != RasterizerState::WrappingModeInvalid) {
					multiTexture->SetWrappingModeT(mode);
					GetRasterizerState()->SetWrappingModeT(mode);
				} else {
					RecordError(GL_INVALID_VALUE);
				}
			}
			break;

		default:
			RecordError(GL_INVALID_ENUM);
			break;
	}
}

void Context :: TexEnvx(GLenum target, GLenum pname, GLfixed param) { 
	if (target != GL_TEXTURE_2D) {
		return;
	}

	switch (pname) {
		case GL_TEXTURE_ENV_MODE:
			switch (param) {
				case GL_MODULATE:
					GetRasterizerState()->SetTextureMode(RasterizerState::TextureModeModulate);
					break;

				case GL_REPLACE:
					GetRasterizerState()->SetTextureMode(RasterizerState::TextureModeReplace);
					break;

				case GL_DECAL:
					GetRasterizerState()->SetTextureMode(RasterizerState::TextureModeDecal);
					break;

				case GL_BLEND:
					GetRasterizerState()->SetTextureMode(RasterizerState::TextureModeBlend);
					break;

				case GL_ADD:
					GetRasterizerState()->SetTextureMode(RasterizerState::TextureModeAdd);
					break;

				default:
					RecordError(GL_INVALID_ENUM);
					break;
			}

			break;

		default:
			RecordError(GL_INVALID_ENUM);
			break;
	}
}

void Context :: TexEnvxv(GLenum target, GLenum pname, const GLfixed *params) { 
	if (target != GL_TEXTURE_2D) {
		return;
	}

	switch (pname) {
		case GL_TEXTURE_ENV_COLOR:
			GetRasterizerState()->SetTexEnvColor(FractionalColor(params));
			break;

		default:
			TexEnvx(target, pname, *params);
			break;
	}
}

// --------------------------------------------------------------------------
// The following methods are not really implemented
// --------------------------------------------------------------------------

void Context :: ActiveTexture(GLenum texture) { 
}


void Context :: ClientActiveTexture(GLenum texture) { 
}

// --------------------------------------------------------------------------
// Pixel transfer to bitmap - basically the inverse of the texture copy
// functions
// --------------------------------------------------------------------------

void Context :: ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, 
						   GLenum format, GLenum type, GLvoid *pixels) { 

   // right now, use a hardcoded image format
	RasterizerState::TextureFormat internalFormat = RasterizerState::TextureFormatRGB;
	RasterizerState::TextureFormat externalFormat = TextureFormatFromEnum(format);
	GLenum internalType = GL_UNSIGNED_SHORT_5_6_5;

	if (!ValidateFormats(internalFormat, externalFormat, type)) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	Surface * readSurface = GetReadSurface();

	CopyPixels(readSurface->GetColorBuffer(), readSurface->GetWidth(), readSurface->GetHeight(), 
				x, y, width, height,
				pixels, width, height, 0, 0, 
				internalFormat, internalType, type);
}


void Context :: PixelStorei(GLenum pname, GLint param) { 
	switch (pname) {
		case GL_UNPACK_ALIGNMENT:
			m_PixelStoreUnpackAlignment = param;
			break;

		case GL_PACK_ALIGNMENT:
			m_PixelStorePackAlignment = param;
			break;

		default:
			RecordError(GL_INVALID_ENUM);
			break;
	}
}


