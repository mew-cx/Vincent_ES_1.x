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
#include "fixed.h"


using namespace EGL;


// --------------------------------------------------------------------------
// Allocation and selection of texture objects
// --------------------------------------------------------------------------

void Context :: BindTexture(GLenum target, GLuint texture) { 
	if (target != GL_TEXTURE_2D) {
		return;
	}
}

void Context :: DeleteTextures(GLsizei n, const GLuint *textures) { 
}

void Context :: GenTextures(GLsizei n, GLuint *textures) { 
}

// --------------------------------------------------------------------------
// Texture specification methods
// --------------------------------------------------------------------------

void Context :: CompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data) { 
	if (target != GL_TEXTURE_2D) {
		return;
	}
}

void Context :: CompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data) { 
	if (target != GL_TEXTURE_2D) {
		return;
	}
}

void Context :: CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) { 
	if (target != GL_TEXTURE_2D) {
		return;
	}
}

void Context :: CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) { 
	if (target != GL_TEXTURE_2D) {
		return;
	}
}

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
}


void Context :: TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) { 
	if (target != GL_TEXTURE_2D) {
		return;
	}

	if (level < 0 || level >= MultiTexture::MAX_LEVELS) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	Texture::TextureFormat internalFormat = TextureFormatFromEnum(internalformat);
	Texture::TextureFormat externalFormat = TextureFormatFromEnum(format);

	if (internalFormat == Texture::TextureFormatInvalid || 
		externalFormat == Texture::TextureFormatInvalid || 
		internalFormat != externalFormat) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	switch (internalFormat) {
		case Texture::TextureFormatAlpha:
		case Texture::TextureFormatLuminance:
		case Texture::TextureFormatLuminanceAlpha:
			if (type != GL_UNSIGNED_BYTE) {
				RecordError(GL_INVALID_VALUE);
				return;
			}

			break;

		case Texture::TextureFormatRGB:
			if (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT_5_6_5) {
				RecordError(GL_INVALID_VALUE);
				return;
			}

			break;

		case Texture::TextureFormatRGBA:
			if (type != GL_UNSIGNED_BYTE &&
				type != GL_UNSIGNED_SHORT_4_4_4_4 &&
				type != GL_UNSIGNED_SHORT_5_5_5_1) {
				RecordError(GL_INVALID_VALUE);
				return;
			}

			break;

	}
	// check that width and height and border are of valid size
	// check for validity of format

	MultiTexture * multiTexture = GetCurrentTexture();
	Texture * texture = multiTexture->GetTexture(level);
	texture->Initialize(width, height, internalFormat);

	switch (internalFormat) {
		case Texture::TextureFormatAlpha:
		case Texture::TextureFormatLuminance:
		case Texture::TextureFormatLuminanceAlpha:
			memcpy(texture->GetData(), pixels, width * height * texture->GetBytesPerPixel());
			break;

		case Texture::TextureFormatRGB:
			switch (type) {
				case GL_UNSIGNED_BYTE:
					{
						U32 count = width * height;
						const U8 * src = reinterpret_cast<const U8 *>(pixels);
						U16 * dst = reinterpret_cast<U16 *>(texture->GetData());

						while (count != 0) {
							--count;
							Color color = Color(*src++, *src++, *src++, 0);
							*dst++ = color.ConvertTo565();
						}
					}
					break;
				case GL_UNSIGNED_SHORT_5_6_5:
					memcpy(texture->GetData(), pixels, width * height * texture->GetBytesPerPixel());
					break;
			}

			break;

		case Texture::TextureFormatRGBA:
			switch (type) {
				case GL_UNSIGNED_BYTE:
					{
						U32 count = width * height;
						const U8 * src = reinterpret_cast<const U8 *>(pixels);
						U16 * dst = reinterpret_cast<U16 *>(texture->GetData());

						while (count != 0) {
							--count;
							Color color = Color(*src++, *src++, *src++, *src++);
							*dst++ = color.ConvertTo5551();
						}
					}
					break;
				case GL_UNSIGNED_SHORT_5_5_5_1:
					memcpy(texture->GetData(), pixels, width * height * texture->GetBytesPerPixel());
					break;

				case GL_UNSIGNED_SHORT_4_4_4_4:
					{
						U32 count = width * height;
						const U8 * src = reinterpret_cast<const U8 *>(pixels);
						U16 * dst = reinterpret_cast<U16 *>(texture->GetData());

						while (count != 0) {
							--count;
							U8 r = *src & 0xF0;
							U8 g = (*src++ & 0xF) << 4;
							U8 b = *src & 0xF0;
							U8 a = (*src++ & 0xF) << 4;
							Color color = Color(r, g, b, a);
							*dst++ = color.ConvertTo5551();
						}
					}
					break;
			}

			break;

	}
}

void Context :: TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) { 
	if (target != GL_TEXTURE_2D) {
		return;
	}
}

// --------------------------------------------------------------------------
// Texture parameters
// --------------------------------------------------------------------------

void Context :: TexParameterx(GLenum target, GLenum pname, GLfixed param) { 
	if (target != GL_TEXTURE_2D) {
		return;
	}

	switch (pname) {
		case GL_TEXTURE_MIN_FILTER:
			break;

		case GL_TEXTURE_MAG_FILTER:
			break;

		case GL_TEXTURE_WRAP_S:
			break;

		case GL_TEXTURE_WRAP_T:
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

