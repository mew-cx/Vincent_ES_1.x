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
#include "context.h"
#include <string.h>
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

void Context :: TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) { 
	if (target != GL_TEXTURE_2D) {
		return;
	}

	if (level < 0 || level >= MultiTexture::MAX_LEVELS) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	Texture::TextureFormatInternal internalFormat;

	switch (internalformat) {
		case 1:
		case GL_LUMINANCE:
			internalFormat = Texture::TextureFormatLuminance;
			break;

		case 2:
		case GL_LUMINANCE_ALPHA:
			internalFormat = Texture::TextureFormatLuminanceAlpha;
			break;

		case 3:
		case GL_RGB:
			internalFormat = Texture::TextureFormatRGB;
			break;

		case 4:
		case GL_RGBA:
			internalFormat = Texture::TextureFormatRGBA;
			break;

		default:
			RecordError(GL_INVALID_ENUM);
			return;
	}

	// check that width and height and border are of valid size
	// check for validity of format

	MultiTexture * multiTexture = GetCurrentTexture();
	Texture * texture = multiTexture->GetTexture(level);
	texture->Initialize(width, height, internalFormat);

	// based on external and internal format, allocate copy operator
	// execute copy
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

