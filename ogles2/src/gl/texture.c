/*
** ==========================================================================
**
** texture.c		Texture management functions
**
** --------------------------------------------------------------------------
**
** 06-13-2005		Hans-Martin Will	initial version
**
** --------------------------------------------------------------------------
**
** Copyright (c) 2003-2006, Hans-Martin Will. All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
**	 *  Redistributions of source code must retain the above copyright
** 		notice, this list of conditions and the following disclaimer.
**   *	Redistributions in binary form must reproduce the above copyright
** 		notice, this list of conditions and the following disclaimer in the
** 		documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
** OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
** THE POSSIBILITY OF SUCH DAMAGE.
**
** ==========================================================================
*/

#include <GLES/gl.h>
#include "config.h"
#include "platform/platform.h"
#include "gl/state.h"


/*
** --------------------------------------------------------------------------
** Local functions
** --------------------------------------------------------------------------
*/

static Texture2D * GetCurrentTexture2D(State * state) {
	if (state->texture2D) {
		assert(state->textures[state->texture2D].base.textureType == GL_TEXTURE_2D);
		return &state->textures[state->texture2D].texture2D;
	} else {
		return &state->textureState.texture2D;
	}
}

static Texture3D * GetCurrentTexture3D(State * state) {
	if (state->texture3D) {
		assert(state->textures[state->texture3D].base.textureType == GL_TEXTURE_3D);
		return &state->textures[state->texture3D].texture3D;
	} else {
		return &state->textureState.texture3D;
	}
}

static TextureCube * GetCurrentTextureCube(State * state) {
	if (state->textureCube) {
		assert(state->textures[state->textureCube].base.textureType == GL_TEXTURE_CUBE_MAP);
		return &state->textures[state->textureCube].textureCube;
	} else {
		return &state->textureState.textureCube;
	}
}

static void AllocateImage2D(State * state, Image2D * image, GLenum internalFormat, 
							GLsizei width, GLsizei height,
							GLuint pixelElementSize) {
	GLsizeiptr size = pixelElementSize * width * height;

	GlesDeleteImage2D(state, image);

	image->internalFormat	= internalFormat;
	image->data				= GlesMalloc(size);

	if (image->data) {
		image->width		= width;
		image->height		= height;
	}
}

static void AllocateImage3D(State * state, Image3D * image, GLenum internalFormat, 
							GLsizei width, GLsizei height, GLsizei depth,
							GLuint pixelElementSize) {
	GLsizeiptr size = pixelElementSize * width * height * depth;

	GlesDeleteImage3D(state, image);

	image->internalFormat	= internalFormat;
	image->data				= GlesMalloc(size);

	if (image->data) {
		image->width		= width;
		image->height		= height;
		image->depth		= depth;
	}
}

/*
** --------------------------------------------------------------------------
** Format information
** --------------------------------------------------------------------------
*/

static GLenum GetBaseInternalFormat(GLenum internalFormat) {
	switch (internalFormat) {
		case GL_LUMINANCE:
		case GL_LUMINANCE_ALPHA:
		case GL_ALPHA:
			return internalFormat;

		case GL_RGB8:
		case GL_UNSIGNED_SHORT_5_6_5:
			return GL_RGB;

		case GL_RGBA8:
		case GL_RGBA4:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
			return GL_RGBA;

		default:
			GLES_ASSERT(0);
			return GL_NONE;
	}
}

static GLenum GetInternalFormat(State * state, GLenum internalformat, GLenum type) {
	switch (type) {
		case GL_UNSIGNED_BYTE:

			switch (internalformat) {
				case GL_LUMINANCE:
					return GL_LUMINANCE;

				case GL_LUMINANCE_ALPHA:
					return GL_LUMINANCE_ALPHA;

				case GL_ALPHA:
					return GL_ALPHA;

				case GL_RGB:
					return GL_RGB8;

				case GL_RGBA:
					return GL_RGBA8;

				default:
					GlesRecordInvalidEnum(state);
					return GL_NONE;
			}

		case GL_UNSIGNED_SHORT_4_4_4_4:
			if (internalformat != GL_RGBA) {
				GlesRecordInvalidEnum(state);
				return GL_NONE;
			}

			return type;

		case GL_UNSIGNED_SHORT_5_5_5_1:
			if (internalformat != GL_RGBA) {
				GlesRecordInvalidEnum(state);
				return GL_NONE;
			}

			return type;

		case GL_UNSIGNED_SHORT_5_6_5:
			if (internalformat != GL_RGB) {
				GlesRecordInvalidEnum(state);
				return GL_NONE;
			}

			return type;

		default:
			GlesRecordInvalidEnum(state);
			return GL_NONE;
	}
}

static GLsizei GetPixelSize(GLenum internalFormat) {
	switch (internalFormat) {
		case GL_LUMINANCE:
		case GL_ALPHA:
			return sizeof(GLubyte);

		case GL_LUMINANCE_ALPHA:
			return sizeof(GLubyte) * 2;

		case GL_RGB8:
			return sizeof(GLubyte) * 3;

		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
			return sizeof(GLushort);

		case GL_RGBA8:
			return sizeof(GLubyte) * 4;

		default:
			GLES_ASSERT(0);
			return 0;
	}
}

static Image2D * GetImage2DForTargetAndLevel(State * state, GLenum target, GLint level) {
	if (level < 0 || level >= GLES_MAX_MIPMAP_LEVELS) {
		GlesRecordInvalidValue(state);
		return NULL;
	}

	switch (target) {
		case GL_TEXTURE_2D:
			return GetCurrentTexture2D(state)->image + level;			

		case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
			return GetCurrentTextureCube(state)->positiveX + level;			

		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
			return GetCurrentTextureCube(state)->negativeX + level;			

		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
			return GetCurrentTextureCube(state)->positiveY + level;			

		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
			return GetCurrentTextureCube(state)->negativeY + level;			

		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
			return GetCurrentTextureCube(state)->positiveZ + level;			

		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
			return GetCurrentTextureCube(state)->negativeZ + level;			

		default:
			GlesRecordInvalidEnum(state);
			return NULL;
	}
}

/*
** --------------------------------------------------------------------------
** Bitmap copy and conversion functions
** --------------------------------------------------------------------------
*/
typedef void (*CopyConversion)(GLubyte * dst, const GLubyte * src, GLsizei elements);

static void CopyRGB8fromRGB8(GLubyte * dst, const GLubyte * src, GLsizei elements) {
	GLsizeiptr size = elements * 3;

	do {
		*dst++ = *src++;
	} while (--size);
}

static void CopyRGB8fromRGB565(GLubyte * dst, const GLubyte * src, GLsizei elements) {
	const GLushort * srcPtr = (const GLushort *) src;

	do {
		GLushort u565 = *srcPtr++;
		GLubyte b = (u565 & 0x001Fu) << 3;
		GLubyte g = (u565 & 0x07E0u) >> 3;
		GLubyte r = (u565 & 0xF800u) >> 8;

		r |= r >> 5;
		g |= g >> 6;
		b |= b >> 5;

		*dst++ = r;
		*dst++ = g;
		*dst++ = b;
	} while (--elements);
}

static void CopyRGB565fromRGB8(GLubyte * dst, const GLubyte * src, GLsizei elements) {
	GLushort * dstPtr = (GLushort *) dst;

	do {
		GLushort r = *src++;
		GLushort g = *src++;
		GLushort b = *src++;

		*dstPtr++ = (b & 0xF8) >> 3 | (g & 0xFC) << 3 | (r & 0xF8) << 8;
	} while (--elements);
}

static void CopyRGBA8fromRGBA8(GLubyte * dst, const GLubyte * src, GLsizei elements) {
	GLsizeiptr size = elements * 4;

	do {
		*dst++ = *src++;
	} while (--size);
}

static void CopyRGBA8fromRGBA51(GLubyte * dst, const GLubyte * src, GLsizei elements) {
	const GLushort * srcPtr = (const GLushort *) src;

	do {
		GLushort u5551 = *srcPtr++;
		GLubyte b = (u5551 & 0x003Eu) << 2;
		GLubyte g = (u5551 & 0x07C0u) >> 3;
		GLubyte r = (u5551 & 0xF800u) >> 8;
		GLubyte a = (u5551 & 0x0001u) << 7;

		r |= r >> 5;
		g |= g >> 5;
		b |= b >> 5;
		if (a) a |= 0x7f;

		*dst++ = r;
		*dst++ = g;
		*dst++ = b;
		*dst++ = a;
	} while (--elements);
}

static void CopyRGBA51fromRGBA8(GLubyte * dst, const GLubyte * src, GLsizei elements) {
	GLushort * dstPtr = (GLushort *) dst;

	do {
		GLushort r = *src++;
		GLushort g = *src++;
		GLushort b = *src++;
		GLushort a = *src++;

		*dstPtr++ = (b & 0xF8) >> 2 | (g & 0xF8) << 3 | (r & 0xF8) << 8 | (a & 0x80) >> 7;
	} while (--elements);
}

static void CopyRGBA8fromRGBA4(GLubyte * dst, const GLubyte * src, GLsizei elements) {
	const GLushort * srcPtr = (const GLushort *) src;

	do {
		GLushort u4444 = *srcPtr++;
		GLubyte r = (u4444 & 0xF000u) >> 8;
		GLubyte g = (u4444 & 0x0F00u) >> 4;
		GLubyte b = (u4444 & 0x00F0u);
		GLubyte a = (u4444 & 0x000Fu) << 4;

		r |= r >> 4;
		g |= g >> 4;
		b |= b >> 4;
		a |= a >> 4;

		*dst++ = r;
		*dst++ = g;
		*dst++ = b;
		*dst++ = a;
	} while (--elements);
}

static void CopyRGBA4fromRGBA8(GLubyte * dst, const GLubyte * src, GLsizei elements) {
	GLushort * dstPtr = (GLushort *) dst;

	do {
		GLushort r = *src++;
		GLushort g = *src++;
		GLushort b = *src++;
		GLushort a = *src++;

		*dstPtr++ = (r & 0xf0) << 8 | (g & 0xf0) << 4 | (b & 0xf0) | a >> 4;
	} while (--elements);
}

static GLES_INLINE GLsizeiptr Align(GLsizeiptr offset, GLuint alignment) {
	return (offset + alignment - 1) & ~(alignment - 1);
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
static void SimpleCopyPixels(GLsizei pixelSize, const GLubyte * src, GLsizei srcWidth, GLsizei srcHeight, 
				GLint srcX, GLint srcY, GLsizei copyWidth, GLsizei copyHeight,
				GLubyte * dst, GLsizei dstWidth, GLsizei dstHeight, GLint dstX, GLint dstY,
				GLuint srcAlignment, GLuint dstAlignment) {

	GLsizeiptr srcBytesWidth = Align(srcWidth * pixelSize, srcAlignment);
	GLsizeiptr dstBytesWidth = Align(dstWidth * pixelSize, dstAlignment);

	GLsizeiptr srcGap = srcBytesWidth - copyWidth * pixelSize;	// how many bytes to skip for next line
	GLsizeiptr dstGap = dstBytesWidth - copyWidth * pixelSize;	// how many bytes to skip for next line

	const GLubyte * srcPtr = src + srcX * pixelSize + srcY * srcBytesWidth;
	GLubyte * dstPtr = dst + dstX * pixelSize + dstY * dstBytesWidth;

	do {
		GLsizeiptr span = copyWidth * pixelSize;

		do {
			*dstPtr++ = *srcPtr++;
		} while (--span);

		srcPtr += srcGap;
		dstPtr += dstGap;
	} while (--copyHeight);
}

/*
** Given two bitmaps src and dst, where src has dimensions 
** (srcWidth * srcHeight) and dst has dimensions (dstWidth * dstHeight),
** copy the rectangle (srcX, srcY, copyWidth, copyHeight) into
** dst at location (dstX, dstY).
**
** It is assumed that the copy rectangle is non-empty and has been clipped
** against the src and target rectangles
*/
static void ConvertCopyPixels(GLsizei srcPixelSize, GLsizei dstPixelSize,
							  const GLubyte * src, GLsizei srcWidth, GLsizei srcHeight, 
							  GLint srcX, GLint srcY, GLsizei copyWidth, GLsizei copyHeight,
							  GLubyte * dst, GLsizei dstWidth, GLsizei dstHeight, GLint dstX, GLint dstY,
							  CopyConversion srcConversion, CopyConversion dstConversion,
							  GLuint srcAlignment, GLuint dstAlignment) {

	GLsizeiptr srcBytesWidth = Align(srcWidth * srcPixelSize, srcAlignment);
	GLsizeiptr dstBytesWidth = Align(dstWidth * dstPixelSize, dstAlignment);

	GLsizeiptr srcGap = srcBytesWidth - copyWidth * srcPixelSize;	// how many bytes to skip for next line
	GLsizeiptr dstGap = dstBytesWidth - copyWidth * dstPixelSize;	// how many bytes to skip for next line

	const GLubyte * srcPtr = src + srcX * srcPixelSize + srcY * srcBytesWidth;
	GLubyte * dstPtr = dst + dstX * dstPixelSize + dstY * dstBytesWidth;

	do {
		GLsizeiptr span = copyWidth;
		GLubyte buffer[128 * 4];

		while (span > 128) {
			srcConversion(buffer, srcPtr, 128);
			dstConversion(dstPtr, buffer, 128);
			srcPtr += srcPixelSize * 128;
			dstPtr += dstPixelSize * 128;

			span -= 128;
		}

		if (span > 0) {
			srcConversion(buffer, srcPtr, span);
			dstConversion(dstPtr, buffer, span);
			srcPtr += srcPixelSize * span;
			dstPtr += dstPixelSize * span;
		}

		srcPtr += srcGap;
		dstPtr += dstGap;
	} while (--copyHeight);
}

/*
** Given two bitmaps src and dst, where src has dimensions 
** (srcWidth * srcHeight) and dst has dimensions (dstWidth * dstHeight),
** copy the rectangle (srcX, srcY, copyWidth, copyHeight) into
** dst at location (dstX, dstY).
**
** The texture format and the type of the source format a given as
** parameters.
*/
static void CopyPixels(const void * src, GLsizei srcWidth, GLsizei srcHeight, 
					   GLint srcX, GLint srcY, GLsizei copyWidth, GLsizei copyHeight,
					   void * dst, GLsizei dstWidth, GLsizei dstHeight, GLint dstX, GLint dstY,
					   GLenum baseInternalFormat, GLenum srcInternalFormat, GLenum dstInternalFormat,
					   GLuint srcAlignment, GLuint dstAlignment) {

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

	if (copyWidth > dstWidth) {
		copyWidth = dstWidth;
	}

	if (copyWidth > srcWidth) {
		copyWidth = srcWidth;
	}

	if (copyHeight > dstHeight) {
		copyHeight = dstHeight;
	}

	if (copyHeight > srcHeight) {
		copyHeight = srcHeight;
	}

	// ---------------------------------------------------------------------
	// at this point we know that the copy rectangle is valid and non-empty
	// ---------------------------------------------------------------------

		
	switch (baseInternalFormat) {
		case GL_ALPHA:
		case GL_LUMINANCE:
			SimpleCopyPixels(sizeof(GLubyte), (const GLubyte *) src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
							 (GLubyte *) dst, dstWidth, dstHeight, dstX, dstY, srcAlignment, dstAlignment);
			break;

		case GL_LUMINANCE_ALPHA:
			SimpleCopyPixels(sizeof(GLubyte) * 2, (const GLubyte *) src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
							 (GLubyte *) dst, dstWidth, dstHeight, dstX, dstY, srcAlignment, dstAlignment);
			break;

		case GL_RGB:
			switch (srcInternalFormat) {
				case GL_RGB:
					switch (dstInternalFormat) {
						case GL_RGB:
							SimpleCopyPixels(sizeof(GLubyte) * 3, (const GLubyte *) src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
								 (GLubyte *) dst, dstWidth, dstHeight, dstX, dstY, srcAlignment, dstAlignment);
							break;

						case GL_UNSIGNED_SHORT_5_6_5:
							ConvertCopyPixels(sizeof(GLubyte) * 3, sizeof(GLushort), (const GLubyte *) src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
								(GLubyte *) dst, dstWidth, dstHeight, dstX, dstY, CopyRGB8fromRGB8, CopyRGB565fromRGB8, srcAlignment, dstAlignment);
							break;
					}
					break;

				case GL_UNSIGNED_SHORT_5_6_5:
					switch (dstInternalFormat) {
						case GL_RGB8:
							ConvertCopyPixels(sizeof(GLushort), sizeof(GLubyte) * 3, (const GLubyte *) src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
								(GLubyte *) dst, dstWidth, dstHeight, dstX, dstY, CopyRGB8fromRGB565, CopyRGB8fromRGB8, srcAlignment, dstAlignment);
							break;

						case GL_UNSIGNED_SHORT_5_6_5:
							SimpleCopyPixels(sizeof(GLushort), (const GLubyte *) src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
								 (GLubyte *) dst, dstWidth, dstHeight, dstX, dstY, srcAlignment, dstAlignment);
							break;
					}
					break;
			}

			break;

		case GL_RGBA:
			switch (srcInternalFormat) {
				case GL_RGBA8:
					switch (dstInternalFormat) {
						case GL_RGBA8:
							SimpleCopyPixels(sizeof(GLubyte) * 4, (const GLubyte *) src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
								 (GLubyte *) dst, dstWidth, dstHeight, dstX, dstY, srcAlignment, dstAlignment);
							break;
						case GL_UNSIGNED_SHORT_5_5_5_1:
							ConvertCopyPixels(sizeof(GLubyte) * 4, sizeof(GLushort), (const GLubyte *) src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
								(GLubyte *) dst, dstWidth, dstHeight, dstX, dstY, CopyRGBA8fromRGBA8, CopyRGBA51fromRGBA8, srcAlignment, dstAlignment);
							break;
						case GL_UNSIGNED_SHORT_4_4_4_4:
							ConvertCopyPixels(sizeof(GLubyte) * 4, sizeof(GLushort), (const GLubyte *) src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
								(GLubyte *) dst, dstWidth, dstHeight, dstX, dstY, CopyRGBA8fromRGBA8, CopyRGBA4fromRGBA8, srcAlignment, dstAlignment);
							break;
					}
					break;

				case GL_UNSIGNED_SHORT_5_5_5_1:
					switch (dstInternalFormat) {
						case GL_RGBA8:
							ConvertCopyPixels(sizeof(GLushort), sizeof(GLubyte) * 4, (const GLubyte *) src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
								(GLubyte *) dst, dstWidth, dstHeight, dstX, dstY, CopyRGBA8fromRGBA51, CopyRGBA8fromRGBA8, srcAlignment, dstAlignment);
							break;
						case GL_UNSIGNED_SHORT_5_5_5_1:
							SimpleCopyPixels(sizeof(GLushort), (const GLubyte *) src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
								 (GLubyte *) dst, dstWidth, dstHeight, dstX, dstY, srcAlignment, dstAlignment);
							break;
						case GL_UNSIGNED_SHORT_4_4_4_4:
							ConvertCopyPixels(sizeof(GLushort), sizeof(GLushort), (const GLubyte *) src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
								(GLubyte *) dst, dstWidth, dstHeight, dstX, dstY, CopyRGBA8fromRGBA51, CopyRGBA4fromRGBA8, srcAlignment, dstAlignment);
							break;
					}
					break;

				case GL_UNSIGNED_SHORT_4_4_4_4:
					switch (dstInternalFormat) {
						case GL_RGBA8:
							ConvertCopyPixels(sizeof(GLushort), sizeof(GLubyte) * 4, (const GLubyte *) src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
								(GLubyte *) dst, dstWidth, dstHeight, dstX, dstY, CopyRGBA8fromRGBA4, CopyRGBA8fromRGBA8, srcAlignment, dstAlignment);
							break;
						case GL_UNSIGNED_SHORT_5_5_5_1:
							ConvertCopyPixels(sizeof(GLushort), sizeof(GLushort), (const GLubyte *) src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
								(GLubyte *) dst, dstWidth, dstHeight, dstX, dstY, CopyRGBA8fromRGBA4, CopyRGBA51fromRGBA8, srcAlignment, dstAlignment);
							break;
						case GL_UNSIGNED_SHORT_4_4_4_4:
							SimpleCopyPixels(sizeof(GLushort), (const GLubyte *) src, srcWidth, srcHeight, srcX, srcY, copyWidth, copyHeight,
								 (GLubyte *) dst, dstWidth, dstHeight, dstX, dstY, srcAlignment, dstAlignment);
							break;
					}
					break;
			}

			break;

	}
}

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

void GlesInitImage2D(Image2D * image) {

	image->data				= NULL;
	image->internalFormat	= GL_LUMINANCE;
	image->width			= 0;
	image->height			= 0;
}

void GlesDeleteImage2D(State * state, Image2D * image) {

	if (image->data != NULL) {
		GlesFree(image->data);
		image->data = NULL;
	}

	image->width			= 0;
	image->height			= 0;
}

void GlesInitImage3D(Image3D * image) {

	image->data				= NULL;
	image->internalFormat	= GL_LUMINANCE;
	image->width			= 0;
	image->height			= 0;
	image->depth			= 0;
}

void GlesDeleteImage3D(State * state, Image3D * image) {

	if (image->data != NULL) {
		GlesFree(image->data);
		image->data = NULL;
	}

	image->width			= 0;
	image->height			= 0;
	image->depth			= 0;
}

void GlesInitTextureBase(TextureBase * texture, GLenum textureType) {
	texture->textureType	= textureType;
	texture->isComplete		= GL_FALSE;
	texture->minFilter		= GL_NEAREST_MIPMAP_LINEAR;
	texture->magFilter		= GL_LINEAR;
	texture->wrapR			= GL_REPEAT;
	texture->wrapS			= GL_REPEAT;
	texture->wrapT			= GL_REPEAT;
}

void GlesInitTexture2D(Texture2D * texture) {

	GLuint index;

	GlesInitTextureBase(&texture->base, GL_TEXTURE_2D);

	for (index = 0; index < GLES_MAX_MIPMAP_LEVELS; ++index) {
		GlesInitImage2D(texture->image + index);
	}
}

void GlesDeleteTexture2D(State * state, Texture2D * texture) {

	GLuint index;

	for (index = 0; index < GLES_MAX_MIPMAP_LEVELS; ++index) {
		GlesDeleteImage2D(state, texture->image + index);
	}
}

void GlesInitTexture3D(Texture3D * texture) {

	GLuint index;

	GlesInitTextureBase(&texture->base, GL_TEXTURE_3D);

	for (index = 0; index < GLES_MAX_MIPMAP_LEVELS; ++index) {
		GlesInitImage3D(texture->image + index);
	}
}

void GlesDeleteTexture3D(State * state, Texture3D * texture) {

	GLuint index;

	for (index = 0; index < GLES_MAX_MIPMAP_LEVELS; ++index) {
		GlesDeleteImage3D(state, texture->image + index);
	}
}

void GlesInitTextureCube(TextureCube * texture) {

	GLuint index;

	GlesInitTextureBase(&texture->base, GL_TEXTURE_CUBE_MAP);

	for (index = 0; index < GLES_MAX_MIPMAP_LEVELS; ++index) {
		GlesInitImage2D(texture->negativeX + index);
		GlesInitImage2D(texture->positiveX + index);
		GlesInitImage2D(texture->negativeY + index);
		GlesInitImage2D(texture->positiveY + index);
		GlesInitImage2D(texture->negativeZ + index);
		GlesInitImage2D(texture->positiveZ + index);
	}
}

void GlesDeleteTextureCube(State * state, TextureCube * texture) {

	GLuint index;

	for (index = 0; index < GLES_MAX_MIPMAP_LEVELS; ++index) {

		GlesDeleteImage2D(state, texture->negativeX + index);
		GlesDeleteImage2D(state, texture->positiveX + index);
		GlesDeleteImage2D(state, texture->negativeY + index);
		GlesDeleteImage2D(state, texture->positiveY + index);
		GlesDeleteImage2D(state, texture->negativeZ + index);
		GlesDeleteImage2D(state, texture->positiveZ + index);
	}
}


/*
** --------------------------------------------------------------------------
** Public API entry points - Texture allocation and binding
** --------------------------------------------------------------------------
*/
GL_API void GL_APIENTRY glBindTexture (GLenum target, GLuint texture) {

	State * state = GLES_GET_STATE();
	GLuint * textureRef;

	/************************************************************************/
	/* Validate parameters													*/
	/************************************************************************/

	if (target != GL_TEXTURE_2D && target != GL_TEXTURE_3D && target != GL_TEXTURE_CUBE_MAP) {
		GlesRecordInvalidEnum(state);
		return;
	}

	if (texture >= GLES_MAX_TEXTURES) {
		GlesRecordInvalidValue(state);
		return;
	}

	switch (target) {
		case GL_TEXTURE_2D:			
			textureRef = &state->texture2D;	
			break;

		case GL_TEXTURE_3D:			
			textureRef = &state->texture3D;	
			break;

		case GL_TEXTURE_CUBE_MAP:	
			textureRef = &state->textureCube;	
			break;
	}

	if (texture == 0) {
		/********************************************************************/
		/* 1. case: Reset to default texture state							*/
		/********************************************************************/

		*textureRef = 0;

	} else if (state->textures[texture].base.textureType != GL_INVALID_ENUM) {
		/********************************************************************/
		/* 2. case: Re-use a prviously allocated texture					*/
		/********************************************************************/

		if (state->textures[texture].base.textureType != target) {

			GlesRecordError(state, GL_INVALID_OPERATION);
			return;
		}

		*textureRef = texture;

	} else {
		/********************************************************************/
		/* 3. case: create a new texture object								*/
		/********************************************************************/

		switch (target) {

			case GL_TEXTURE_2D:			
				GlesInitTexture2D(&state->textures[texture].texture2D);	
				break;

			case GL_TEXTURE_3D:			
				GlesInitTexture3D(&state->textures[texture].texture3D);	
				break;

			case GL_TEXTURE_CUBE_MAP:	
				GlesInitTextureCube(&state->textures[texture].textureCube);	
				break;
		}

		*textureRef = texture;
	}
}

GL_API void GL_APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures) {

	State * state = GLES_GET_STATE();

	/************************************************************************/
	/* Validate parameters													*/
	/************************************************************************/

	if (n < 0 || textures == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Delete the individual textures										*/
	/************************************************************************/

	while (n--) {
		if (GlesIsBoundObject(state->textureFreeListHead, state->textureFreeList, GLES_MAX_TEXTURES, *textures)) {

			GLuint * textureRef = NULL;

			switch (state->textures[*textures].base.textureType) {

				case GL_TEXTURE_2D:			
					textureRef = &state->texture2D;	
					GlesDeleteTexture2D(state, &state->textures[*textures].texture2D);
					break;

				case GL_TEXTURE_3D:			
					textureRef = &state->texture3D;	
					GlesDeleteTexture3D(state, &state->textures[*textures].texture3D);
					break;

				case GL_TEXTURE_CUBE_MAP:	
					textureRef = &state->textureCube;	
					GlesDeleteTextureCube(state, &state->textures[*textures].textureCube);
					break;
			}

			if (textureRef != NULL && *textureRef == *textures) {
				/************************************************************************/
				/* Unbind the texture if it is currently in use and bound				*/
				/************************************************************************/

				*textureRef = 0;
			}
		}

		++textures;
	}
}

GL_API void GL_APIENTRY glGenTextures (GLsizei n, GLuint *textures) {

	State * state = GLES_GET_STATE();
	GlesGenObjects(state, state->textureFreeListHead, state->textureFreeList, GLES_MAX_TEXTURES, n, textures);
}

GL_API GLboolean GL_APIENTRY glIsTexture (GLuint texture) {

	State * state = GLES_GET_STATE();
	return GlesIsBoundObject(state->textureFreeListHead, state->textureFreeList, GLES_MAX_TEXTURES, texture) &&
		state->textures[texture].base.textureType != GL_INVALID_ENUM;
}

/*
** --------------------------------------------------------------------------
** Public API entry points - Texture parameters
** --------------------------------------------------------------------------
*/
GL_API void GL_APIENTRY glPixelStorei (GLenum pname, GLint param) {

	State * state = GLES_GET_STATE();

	switch (pname) {
		case GL_UNPACK_ALIGNMENT:
			switch (param) {
			case 1:
			case 2:
			case 4:
			case 8:
				state->packAlignment = param;
				break;

			default:
				GlesRecordInvalidValue(state);
				break;
			}

			break;

		case GL_PACK_ALIGNMENT:
			switch (param) {
			case 1:
			case 2:
			case 4:
			case 8:
				state->unpackAlignment = param;
				break;

			default:
				GlesRecordInvalidValue(state);
				break;
			}

			break;

		default:
			GlesRecordInvalidEnum(state);
			break;
	}
}

GL_API void GL_APIENTRY glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params) {

	State * state = GLES_GET_STATE();
	GLint result;

	if (params == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	glGetTexParameteriv(target, pname, &result);
	*params = (GLfloat) result;
}

GL_API void GL_APIENTRY glGetTexParameteriv (GLenum target, GLenum pname, GLint *params) {

	State * state = GLES_GET_STATE();
	Texture * texture;


	if (params == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	switch (target) {
		case GL_TEXTURE_2D:
			if (state->texture2D) {
				texture = &state->textures[state->texture2D];
			} else {
				texture = (Texture *) &state->textureState.texture2D;
			}

			assert(texture->base.textureType == GL_TEXTURE_2D);
			break;

		case GL_TEXTURE_3D:
			if (state->texture3D) {
				texture = &state->textures[state->texture3D];
			} else {
				texture = (Texture *) &state->textureState.texture3D;
			}

			assert(texture->base.textureType == GL_TEXTURE_3D);
			break;

		case GL_TEXTURE_CUBE_MAP:
			if (state->textureCube) {
				texture = &state->textures[state->textureCube];
			} else {
				texture = (Texture *) &state->textureState.textureCube;
			}

			assert(texture->base.textureType == GL_TEXTURE_CUBE_MAP);
			break;

		default:
			GlesRecordInvalidEnum(state);
			return;
	}

	switch (pname) {
		case GL_TEXTURE_MIN_FILTER:
			*params = texture->base.minFilter;
			break;

		case GL_TEXTURE_MAG_FILTER:
			*params = texture->base.magFilter;
			break;

		case GL_TEXTURE_WRAP_S:
			*params = texture->base.wrapS;
			break;

		case GL_TEXTURE_WRAP_T:
			*params = texture->base.wrapT;
			break;

		case GL_TEXTURE_WRAP_R:

			if (target != GL_TEXTURE_3D) {
				GlesRecordInvalidEnum(state);
				return;
			}

			*params = texture->base.wrapT;
			break;

		default:
			GlesRecordInvalidEnum(state);
			return;
	}
}

GL_API void GL_APIENTRY glTexParameterf (GLenum target, GLenum pname, GLfloat param) {
	glTexParameteri(target, pname, (GLint) param);
}

GL_API void GL_APIENTRY glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params) {

	State * state = GLES_GET_STATE();

	if (params == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	glTexParameteri(target, pname, (GLint) *params);
}

GL_API void GL_APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param) {

	State * state = GLES_GET_STATE();
	Texture * texture;

	switch (target) {
		case GL_TEXTURE_2D:
			if (state->texture2D) {
				texture = &state->textures[state->texture2D];
			} else {
				texture = (Texture *) &state->textureState.texture2D;
			}

			assert(texture->base.textureType == GL_TEXTURE_2D);
			break;

		case GL_TEXTURE_3D:
			if (state->texture3D) {
				texture = &state->textures[state->texture3D];
			} else {
				texture = (Texture *) &state->textureState.texture3D;
			}

			assert(texture->base.textureType == GL_TEXTURE_3D);
			break;

		case GL_TEXTURE_CUBE_MAP:
			if (state->textureCube) {
				texture = &state->textures[state->textureCube];
			} else {
				texture = (Texture *) &state->textureState.textureCube;
			}

			assert(texture->base.textureType == GL_TEXTURE_CUBE_MAP);
			break;

		default:
			GlesRecordInvalidEnum(state);
			return;
	}

	switch (pname) {
		case GL_TEXTURE_MIN_FILTER:
			switch (param) {
				case GL_LINEAR:
				case GL_NEAREST:
				case GL_LINEAR_MIPMAP_LINEAR:
				case GL_LINEAR_MIPMAP_NEAREST: 
				case GL_NEAREST_MIPMAP_LINEAR:
				case GL_NEAREST_MIPMAP_NEAREST:
					texture->base.minFilter = param;
					break;

				default:
					GlesRecordInvalidValue(state);
					return;
			}

			break;

		case GL_TEXTURE_MAG_FILTER:
			switch (param) {
				case GL_LINEAR:
				case GL_NEAREST:
					texture->base.magFilter = param;
					break;

				default:
					GlesRecordInvalidValue(state);
					return;
			}

			break;

		case GL_TEXTURE_WRAP_S:
			switch (param) {
				case GL_REPEAT:
				case GL_CLAMP_TO_EDGE:
				case GL_MIRRORED_REPEAT:
					texture->base.wrapS = param;
					break;

				default:
					GlesRecordInvalidValue(state);
					return;
			}

			break;

		case GL_TEXTURE_WRAP_T:
			switch (param) {
				case GL_REPEAT:
				case GL_CLAMP_TO_EDGE:
				case GL_MIRRORED_REPEAT:
					texture->base.wrapT = param;
					break;

				default:
					GlesRecordInvalidValue(state);
					return;
			}

			break;

		case GL_TEXTURE_WRAP_R:

			if (target != GL_TEXTURE_3D) {
				GlesRecordInvalidEnum(state);
				return;
			}

			switch (param) {
				case GL_REPEAT:
				case GL_CLAMP_TO_EDGE:
				case GL_MIRRORED_REPEAT:
					texture->base.wrapT = param;
					break;

				default:
					GlesRecordInvalidValue(state);
					return;
			}

			break;

		default:
			GlesRecordInvalidEnum(state);
			return;
	}
}

GL_API void GL_APIENTRY glTexParameteriv (GLenum target, GLenum pname, const GLint *params) {

	State * state = GLES_GET_STATE();

	if (params == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	glTexParameteri(target, pname, *params);
}

/*
** --------------------------------------------------------------------------
** Public API entry points - Texture image functions
** --------------------------------------------------------------------------
*/
GL_API void GL_APIENTRY 
glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, 
						GLsizei width, GLsizei height, GLint border, 
						GLsizei imageSize, const void *data) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY 
glCompressedTexImage3D (GLenum target, GLint level, GLenum internalformat, 
						GLsizei width, GLsizei height, GLsizei depth, GLint border, 
						GLsizei imageSize, const void *data) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY 
glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, 
						   GLsizei width, GLsizei height, GLenum format, 
						   GLsizei imageSize, const void *data) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY 
glCompressedTexSubImage3D (GLenum target, GLint level, 
						   GLint xoffset, GLint yoffset, GLint zoffset, 
						   GLsizei width, GLsizei height, GLsizei depth, 
						   GLenum format, GLsizei imageSize, const void *data) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY 
glCopyTexImage2D (GLenum target, GLint level, GLenum internalformat, 
				  GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY 
glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, 
					 GLint x, GLint y, GLsizei width, GLsizei height) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY 
glCopyTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, 
					 GLint x, GLint y, GLsizei width, GLsizei height) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY 
glTexImage2D (GLenum target, GLint level, GLenum internalformat, 
			  GLsizei width, GLsizei height, GLint border, 
			  GLenum format, GLenum type, const void *pixels) {

	State * state = GLES_GET_STATE();
	GLsizei pixelSize;
	GLenum textureFormat;

	/************************************************************************/
	/* Determine texture image to load										*/
	/************************************************************************/

	Image2D * image = GetImage2DForTargetAndLevel(state, target, level);

	if (image == NULL) {
		return;
	}

	/************************************************************************/
	/* Determine texture format												*/
	/************************************************************************/

	textureFormat = GetInternalFormat(state, internalformat, type);

	if (textureFormat == GL_NONE) {
		return;
	}

	pixelSize = GetPixelSize(textureFormat);

	/************************************************************************/
	/* Verify image dimensions												*/
	/************************************************************************/

	if (width > GLES_MAX_TEXTURE_WIDTH || height > GLES_MAX_TEXTURE_HEIGHT) {
		GlesRecordInvalidValue(state);
		return;
	}

	if ((width == 0 || height == 0) && pixels != NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (width != 0 && height != 0 && pixels == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Copy the actual image data											*/
	/************************************************************************/

	AllocateImage2D(state, image, textureFormat, width, height, pixelSize);

	if (!image->data) {
		GlesRecordOutOfMemory(state);
		return;
	}

	CopyPixels(pixels, 
			   width, height, 
			   0, 0, width, height, 
			   image->data, width, height, 0, 0, internalformat, textureFormat, textureFormat, state->packAlignment, 1);
}

GL_API void GL_APIENTRY 
glTexImage3D (GLenum target, GLint level, GLenum internalformat, 
			  GLsizei width, GLsizei height, GLsizei depth, GLint border, 
			  GLenum format, GLenum type, const void *pixels) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY 
glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, 
				 GLsizei width, GLsizei height, 
				 GLenum format, GLenum type, const void *pixels) {
	State * state = GLES_GET_STATE();
	GLsizei pixelSize;
	GLenum textureFormat;

	/************************************************************************/
	/* Determine texture image to load										*/
	/************************************************************************/

	Image2D * image = GetImage2DForTargetAndLevel(state, target, level);

	if (image == NULL) {
		return;
	}

	if (GetBaseInternalFormat(image->internalFormat) != format) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Determine texture format												*/
	/************************************************************************/

	textureFormat = GetInternalFormat(state, format, type);

	if (textureFormat == GL_NONE) {
		return;
	}

	pixelSize = GetPixelSize(textureFormat);

	/************************************************************************/
	/* Verify image dimensions												*/
	/************************************************************************/

	if (width > GLES_MAX_TEXTURE_WIDTH || height > GLES_MAX_TEXTURE_HEIGHT) {
		GlesRecordInvalidValue(state);
		return;
	}

	if ((width == 0 || height == 0) && pixels != NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (width != 0 && height != 0 && pixels == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (xoffset < 0 || yoffset < 0 || 
		xoffset + width > image->width || 
		yoffset + height > image->height) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Copy the actual image data											*/
	/************************************************************************/

	if (!image->data) {
		GlesRecordOutOfMemory(state);
		return;
	}

	CopyPixels(pixels, 
			   width, height, 
			   0, 0, width, height, 
			   image->data, image->width, image->height, xoffset, yoffset, format, image->internalFormat, textureFormat, state->packAlignment, 1);
}

GL_API void GL_APIENTRY 
glTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, 
				 GLsizei width, GLsizei height, GLsizei depth, 
				 GLenum format, GLenum type, const void *pixels) {
	State * state = GLES_GET_STATE();
}

