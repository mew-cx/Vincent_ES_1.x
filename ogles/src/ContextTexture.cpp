// ==========================================================================
//
// matrix.cpp	Rendering Context Class for Embedded OpenGL Implementation
//
//				Texturing Functions
//
// --------------------------------------------------------------------------
//
// 08-07-2003	Hans-Martin Will	initial version
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
	}

	m_CurrentTexture = m_Textures[texture];
	GetRasterizerState()->SetTexture(m_CurrentTexture);
}

void Context :: DeleteTextures(GLsizei n, const GLuint *textures) { 
}

void Context :: GenTextures(GLsizei n, GLuint *textures) { 
}

// --------------------------------------------------------------------------
// Texture specification methods
// --------------------------------------------------------------------------

namespace {
	Texture::TextureFormat TextureFormatFromEnum(GLenum format) {
		switch (format) {
			case GL_ALPHA:
				return Texture::TextureFormatAlpha;

			case 1:
			case GL_LUMINANCE:
				return Texture::TextureFormatLuminance;

			case 2:
			case GL_LUMINANCE_ALPHA:
				return Texture::TextureFormatLuminanceAlpha;

			case 3:
			case GL_RGB:
				return Texture::TextureFormatRGB;

			case 4:
			case GL_RGBA:
				return Texture::TextureFormatRGBA;

			default:
				return Texture::TextureFormatInvalid;
		}
	}

	MultiTexture::WrappingMode WrappingModeFromEnum(GLenum mode) {
		switch (mode) {
			case GL_CLAMP_TO_EDGE:	return MultiTexture::WrappingModeClampToEdge;
			case GL_REPEAT:			return MultiTexture::WrappingModeRepeat;
			default:				return MultiTexture::WrappingModeInvalid;
		}
	}

	MultiTexture::MinFilterMode MinFilterModeFromEnum(GLenum mode) {
		switch (mode) {
			case GL_NEAREST:				return MultiTexture::MinFilterModeNearest;
			case GL_LINEAR:					return MultiTexture::MinFilterModeLinear;
			case GL_NEAREST_MIPMAP_LINEAR:	return MultiTexture::MinFilterModeNearestMipmapLinear;
			case GL_NEAREST_MIPMAP_NEAREST:	return MultiTexture::MinFilterModeNearestMipmapNearest;
			case GL_LINEAR_MIPMAP_LINEAR:	return MultiTexture::MinFilterModeLinearMipmapLinear;
			case GL_LINEAR_MIPMAP_NEAREST:	return MultiTexture::MinFilterModeLinearMipmapNearest;
			default:						return MultiTexture::MinFilterModeInvalid;
		}
	}

	MultiTexture::MagFilterMode MagFilterModeFromEnum(GLenum mode) {
		switch (mode) {
			case GL_NEAREST:		return MultiTexture::MagFilterModeNearest;
			case GL_LINEAR:			return MultiTexture::MagFilterModeLinear;
			default:				return MultiTexture::MagFilterModeInvalid;
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
			return Color(*ptr++, *ptr++, *ptr++, 0);
		}
	};

	struct RGBA2Color {
		enum {
			baseIncr = 4
		};

		typedef U8 BaseType;

		Color operator()(const BaseType * &ptr) {
			return Color(*ptr++, *ptr++, *ptr++, *ptr++);
		}
	};

	struct RGBA44442Color {
		enum {
			baseIncr = 2
		};

		typedef U8 BaseType;

		Color operator()(const BaseType * &ptr) {
			U8 r = *ptr & 0xF0;
			U8 g = (*ptr++ & 0xF) << 4;
			U8 b = *ptr & 0xF0;
			U8 a = (*ptr++ & 0xF) << 4;
			return Color(r, g, b, a);
		}
	};

	struct Color2RGB565 {
		enum {
			baseIncr = 1
		};

		typedef U16 BaseType;

		void operator()(BaseType * &ptr, const Color value) {
			*ptr++ = value.ConvertTo565();
		}
	};

	struct Color2RGBA5551 {
		enum {
			baseIncr = 1
		};

		typedef U16 BaseType;

		void operator()(BaseType * &ptr, const Color value) {
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
					Texture::TextureFormat format, GLenum type) {

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
			case Texture::TextureFormatAlpha:
			case Texture::TextureFormatLuminance:
				CopyPixels<U8>(src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
					dst, dstWidth, dstHeight, dstX, dstY);
				break;

			case Texture::TextureFormatLuminanceAlpha:
				CopyPixels<U16>(src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
					dst, dstWidth, dstHeight, dstX, dstY);
				break;

			case Texture::TextureFormatRGB:
				switch (type) {
					case GL_UNSIGNED_BYTE:
						CopyPixelsA<RGB2Color, Color2RGB565>(src, srcWidth, srcHeight, srcX, srcY, 
							copyWidth, copyHeight, dst, dstWidth, dstHeight, dstX, dstY);
						break;

					case GL_UNSIGNED_SHORT_5_6_5:
						CopyPixels<U16>(src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
							dst, dstWidth, dstHeight, dstX, dstY);
						break;
				}

				break;

			case Texture::TextureFormatRGBA:
				switch (type) {
					case GL_UNSIGNED_BYTE:
						CopyPixelsA<RGBA2Color, Color2RGBA5551>(src, srcWidth, srcHeight, srcX, srcY, 
							copyWidth, copyHeight, dst, dstWidth, dstHeight, dstX, dstY);
						break;

					case GL_UNSIGNED_SHORT_5_5_5_1:
						CopyPixels<U16>(src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
							dst, dstWidth, dstHeight, dstX, dstY);
						break;

					case GL_UNSIGNED_SHORT_4_4_4_4:
						CopyPixelsA<RGBA44442Color, Color2RGBA5551>(src, srcWidth, srcHeight, srcX, srcY, 
							copyWidth, copyHeight, dst, dstWidth, dstHeight, dstX, dstY);
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
	bool ValidateFormats(Texture::TextureFormat internalFormat,
		Texture::TextureFormat externalFormat, GLenum type) {
		if (internalFormat == Texture::TextureFormatInvalid || 
			externalFormat == Texture::TextureFormatInvalid || 
			internalFormat != externalFormat) {
			return false;
		}

		switch (internalFormat) {
			case Texture::TextureFormatAlpha:
			case Texture::TextureFormatLuminance:
			case Texture::TextureFormatLuminanceAlpha:
				if (type != GL_UNSIGNED_BYTE) {
					return false;
				}

				break;

			case Texture::TextureFormatRGB:
				if (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT_5_6_5) {
					return false;
				}

				break;

			case Texture::TextureFormatRGBA:
				if (type != GL_UNSIGNED_BYTE &&
					type != GL_UNSIGNED_SHORT_4_4_4_4 &&
					type != GL_UNSIGNED_SHORT_5_5_5_1) {
					return false;
				}

				break;

		}

		return true;
	}

	GLenum TypeForInternalFormat(Texture::TextureFormat format) {
		switch (format) {
			default:
			case Texture::TextureFormatAlpha:
			case Texture::TextureFormatLuminance:
			case Texture::TextureFormatLuminanceAlpha:
				return GL_UNSIGNED_BYTE;

			case Texture::TextureFormatRGB:
				return GL_UNSIGNED_SHORT_5_6_5;

			case Texture::TextureFormatRGBA:
				return GL_UNSIGNED_SHORT_5_5_5_1;
		}
	}
}

void Context :: CompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data) { 

	Texture::TextureFormat internalFormat = TextureFormatFromEnum(internalformat);
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

	Texture::TextureFormat internalFormat = texture->GetInternalFormat();
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

	Texture::TextureFormat internalFormat = TextureFormatFromEnum(internalformat);
	Texture::TextureFormat externalFormat = TextureFormatFromEnum(format);

	if (!ValidateFormats(internalFormat, externalFormat, type)) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	MultiTexture * multiTexture = GetCurrentTexture();
	Texture * texture = multiTexture->GetTexture(level);
	texture->Initialize(width, height, internalFormat);

	CopyPixels(const_cast<const void *>(pixels), width, height, 0, 0, width, height,
		texture->GetData(), width, height, 0, 0, internalFormat, type);
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

	Texture::TextureFormat internalFormat = texture->GetInternalFormat();
	Texture::TextureFormat externalFormat = TextureFormatFromEnum(format);

	if (!ValidateFormats(internalFormat, externalFormat, type)) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	CopyPixels(const_cast<const void *>(pixels), width, height, 0, 0, width, height,
		texture->GetData(), texture->GetWidth(), texture->GetHeight(),
		xoffset, yoffset, internalFormat, type);
}

void Context :: CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, 
							   GLint x, GLint y, GLsizei width, GLsizei height, GLint border) { 
	if (target != GL_TEXTURE_2D) {
		return;
	}

	Surface * drawSurface = GetDrawSurface();

	if (level < 0 || level >= MultiTexture::MAX_LEVELS) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	Texture::TextureFormat internalFormat = TextureFormatFromEnum(internalformat);

	// These parameters really depend on the actual drawing surface, and should
	// be determined from there
	Texture::TextureFormat externalFormat = Texture::TextureFormatRGB;
	GLenum type = GL_UNSIGNED_SHORT_5_6_5;

	if (!ValidateFormats(internalFormat, externalFormat, type)) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	MultiTexture * multiTexture = GetCurrentTexture();
	Texture * texture = multiTexture->GetTexture(level);
	texture->Initialize(width, height, internalFormat);

	CopyPixels(drawSurface->GetColorBuffer(), drawSurface->GetWidth(), drawSurface->GetHeight(), 
				0, 0, width, height,
				texture->GetData(), width, height, 0, 0, internalFormat, type);
}

void Context :: CopyTexSubImage2D(GLenum target, GLint level, 
								  GLint xoffset, GLint yoffset, 
								  GLint x, GLint y, GLsizei width, GLsizei height) { 
	if (target != GL_TEXTURE_2D) {
		return;
	}

	Surface * drawSurface = GetDrawSurface();

	if (level < 0 || level >= MultiTexture::MAX_LEVELS) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	MultiTexture * multiTexture = GetCurrentTexture();
	Texture * texture = multiTexture->GetTexture(level);
	Texture::TextureFormat internalFormat = texture->GetInternalFormat();

	// These parameters really depend on the actual drawing surface, and should
	// be determined from there
	Texture::TextureFormat externalFormat = Texture::TextureFormatRGB;
	GLenum type = GL_UNSIGNED_SHORT_5_6_5;

	if (!ValidateFormats(internalFormat, externalFormat, type)) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	CopyPixels(drawSurface->GetColorBuffer(), drawSurface->GetWidth(), drawSurface->GetHeight(), 
				0, 0, width, height,
				texture->GetData(), texture->GetWidth(), texture->GetHeight(), 0, 0, 
				internalFormat, type);
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
				MultiTexture::MinFilterMode mode = MinFilterModeFromEnum(intParam);

				if (mode != MultiTexture::MinFilterModeInvalid) {
					multiTexture->SetMinFilterMode(mode);
				} else {
					RecordError(GL_INVALID_VALUE);
				}
			}
			break;

		case GL_TEXTURE_MAG_FILTER:
			{
				MultiTexture::MagFilterMode mode = MagFilterModeFromEnum(intParam);

				if (mode != MultiTexture::MagFilterModeInvalid) {
					multiTexture->SetMagFilterMode(mode);
				} else {
					RecordError(GL_INVALID_VALUE);
				}
			}
			break;

		case GL_TEXTURE_WRAP_S:
			{
				MultiTexture::WrappingMode mode = WrappingModeFromEnum(intParam);

				if (mode != MultiTexture::WrappingModeInvalid) {
					multiTexture->SetWrappingModeS(mode);
				} else {
					RecordError(GL_INVALID_VALUE);
				}
			}
			break;

		case GL_TEXTURE_WRAP_T:
			{
				MultiTexture::WrappingMode mode = WrappingModeFromEnum(intParam);

				if (mode != MultiTexture::WrappingModeInvalid) {
					multiTexture->SetWrappingModeT(mode);
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

