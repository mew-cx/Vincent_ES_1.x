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
** Copyright (c) 2005, Hans-Martin Will. All rights reserved.
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

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

void InitImage2D(Image2D * image) {

	image->data		= NULL;
	image->format	= GL_LUMINANCE;
	image->width	= 0;
	image->height	= 0;
}

void DeleteImage2D(State * state, Image2D * image) {

	if (image->data != NULL) {
		Deallocate(state, image->data);
		image->data = NULL;
	}
}

void InitImage3D(Image3D * image) {

	image->data		= NULL;
	image->format	= GL_LUMINANCE;
	image->width	= 0;
	image->height	= 0;
	image->depth	= 0;
}

void DeleteImage3D(State * state, Image3D * image) {

	if (image->data != NULL) {
		Deallocate(state, image->data);
		image->data = NULL;
	}
}

void InitTextureBase(TextureBase * texture, GLenum textureType) {
	texture->textureType	= textureType;
	texture->isComplete		= GL_FALSE;
	texture->minFilter		= GL_NEAREST_MIPMAP_LINEAR;
	texture->magFilter		= GL_LINEAR;
	texture->wrapR			= GL_REPEAT;
	texture->wrapS			= GL_REPEAT;
	texture->wrapT			= GL_REPEAT;
}

void InitTexture2D(Texture2D * texture) {

	GLuint index;

	InitTextureBase(&texture->base, GL_TEXTURE_2D);

	for (index = 0; index < GLES_MAX_MIPMAP_LEVELS; ++index) {
		InitImage2D(texture->image + index);
	}
}

void DeleteTexture2D(State * state, Texture2D * texture) {

	GLuint index;

	for (index = 0; index < GLES_MAX_MIPMAP_LEVELS; ++index) {
		DeleteImage2D(state, texture->image + index);
	}
}

void InitTexture3D(Texture3D * texture) {

	GLuint index;

	InitTextureBase(&texture->base, GL_TEXTURE_3D);

	for (index = 0; index < GLES_MAX_MIPMAP_LEVELS; ++index) {
		InitImage3D(texture->image + index);
	}
}

void DeleteTexture3D(State * state, Texture3D * texture) {

	GLuint index;

	for (index = 0; index < GLES_MAX_MIPMAP_LEVELS; ++index) {
		DeleteImage3D(state, texture->image + index);
	}
}

void InitTextureCube(TextureCube * texture) {

	GLuint index;

	InitTextureBase(&texture->base, GL_TEXTURE_CUBE_MAP);

	for (index = 0; index < GLES_MAX_MIPMAP_LEVELS; ++index) {
		InitImage2D(texture->negativeX + index);
		InitImage2D(texture->positiveX + index);
		InitImage2D(texture->negativeY + index);
		InitImage2D(texture->positiveY + index);
		InitImage2D(texture->negativeZ + index);
		InitImage2D(texture->positiveZ + index);
	}
}

void DeleteTextureCube(State * state, TextureCube * texture) {

	GLuint index;

	for (index = 0; index < GLES_MAX_MIPMAP_LEVELS; ++index) {

		DeleteImage2D(state, texture->negativeX + index);
		DeleteImage2D(state, texture->positiveX + index);
		DeleteImage2D(state, texture->negativeY + index);
		DeleteImage2D(state, texture->positiveY + index);
		DeleteImage2D(state, texture->negativeZ + index);
		DeleteImage2D(state, texture->positiveZ + index);
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
		RecordInvalidEnum(state);
		return;
	}

	if (texture >= GLES_MAX_TEXTURES) {
		RecordInvalidValue(state);
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

			RecordError(state, GL_INVALID_OPERATION);
			return;
		}

		*textureRef = texture;

	} else {
		/********************************************************************/
		/* 3. case: create a new texture object								*/
		/********************************************************************/

		switch (target) {

			case GL_TEXTURE_2D:			
				InitTexture2D(&state->textures[texture].texture2D);	
				break;

			case GL_TEXTURE_3D:			
				InitTexture3D(&state->textures[texture].texture3D);	
				break;

			case GL_TEXTURE_CUBE_MAP:	
				InitTextureCube(&state->textures[texture].textureCube);	
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
		RecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Delete the individual textures										*/
	/************************************************************************/

	while (n--) {
		if (IsBoundObject(state->textureFreeListHead, state->textureFreeList, GLES_MAX_TEXTURES, *textures)) {

			GLuint * textureRef = NULL;

			switch (state->textures[*textures].base.textureType) {

				case GL_TEXTURE_2D:			
					textureRef = &state->texture2D;	
					DeleteTexture2D(state, &state->textures[*textures].texture2D);
					break;

				case GL_TEXTURE_3D:			
					textureRef = &state->texture3D;	
					DeleteTexture3D(state, &state->textures[*textures].texture3D);
					break;

				case GL_TEXTURE_CUBE_MAP:	
					textureRef = &state->textureCube;	
					DeleteTextureCube(state, &state->textures[*textures].textureCube);
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
	GenObjects(state, state->textureFreeListHead, state->textureFreeList, GLES_MAX_TEXTURES, n, textures);
}

GL_API GLboolean GL_APIENTRY glIsTexture (GLuint texture) {

	State * state = GLES_GET_STATE();
	return IsBoundObject(state->textureFreeListHead, state->textureFreeList, GLES_MAX_TEXTURES, texture) &&
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
				RecordInvalidValue(state);
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
				RecordInvalidValue(state);
				break;
			}

			break;

		default:
			RecordInvalidEnum(state);
			break;
	}
}

GL_API void GL_APIENTRY glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params) {

	State * state = GLES_GET_STATE();
	GLint result;

	if (params == NULL) {
		RecordInvalidValue(state);
		return;
	}

	glGetTexParameteriv(target, pname, &result);
	*params = (GLfloat) result;
}

GL_API void GL_APIENTRY glGetTexParameteriv (GLenum target, GLenum pname, GLint *params) {

	State * state = GLES_GET_STATE();
	Texture * texture;


	if (params == NULL) {
		RecordInvalidValue(state);
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
			RecordInvalidEnum(state);
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
				RecordInvalidEnum(state);
				return;
			}

			*params = texture->base.wrapT;
			break;

		default:
			RecordInvalidEnum(state);
			return;
	}
}

GL_API void GL_APIENTRY glTexParameterf (GLenum target, GLenum pname, GLfloat param) {
	glTexParameteri(target, pname, (GLint) param);
}

GL_API void GL_APIENTRY glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params) {

	State * state = GLES_GET_STATE();

	if (params == NULL) {
		RecordInvalidValue(state);
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
			RecordInvalidEnum(state);
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
					RecordInvalidValue(state);
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
					RecordInvalidValue(state);
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
					RecordInvalidValue(state);
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
					RecordInvalidValue(state);
					return;
			}

			break;

		case GL_TEXTURE_WRAP_R:

			if (target != GL_TEXTURE_3D) {
				RecordInvalidEnum(state);
				return;
			}

			switch (param) {
				case GL_REPEAT:
				case GL_CLAMP_TO_EDGE:
				case GL_MIRRORED_REPEAT:
					texture->base.wrapT = param;
					break;

				default:
					RecordInvalidValue(state);
					return;
			}

			break;

		default:
			RecordInvalidEnum(state);
			return;
	}
}

GL_API void GL_APIENTRY glTexParameteriv (GLenum target, GLenum pname, const GLint *params) {

	State * state = GLES_GET_STATE();

	if (params == NULL) {
		RecordInvalidValue(state);
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
glTexImage2D (GLenum target, GLint level, GLint internalformat, 
			  GLsizei width, GLsizei height, GLint border, 
			  GLenum format, GLenum type, const void *pixels) {

	State * state = GLES_GET_STATE();
	GLsizei pixelSize, pixelElements;
	GLenum textureFormat;

	/************************************************************************/
	/* Determine texture image to load										*/
	/************************************************************************/

	Image2D * image = NULL;

	if (level < 0 || level >= GLES_MAX_MIPMAP_LEVELS) {
		RecordInvalidValue(state);
		return;
	}

	switch (target) {
		case GL_TEXTURE_2D:
			image = GetCurrentTexture2D(state)->image + level;			
			break;

		case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
			image = GetCurrentTextureCube(state)->positiveX + level;			
			break;

		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
			image = GetCurrentTextureCube(state)->negativeX + level;			
			break;

		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
			image = GetCurrentTextureCube(state)->positiveY + level;			
			break;

		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
			image = GetCurrentTextureCube(state)->negativeY + level;			
			break;

		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
			image = GetCurrentTextureCube(state)->positiveZ + level;			
			break;

		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
			image = GetCurrentTextureCube(state)->negativeZ + level;			
			break;

		default:
			RecordInvalidEnum(state);
			return;
	}

	/************************************************************************/
	/* Determine texture format												*/
	/************************************************************************/

	switch (type) {
		case GL_UNSIGNED_BYTE:

			pixelSize = 1;

			switch (internalformat) {
				case GL_LUMINANCE:
					pixelElements = 1;
					break;

				case GL_LUMINANCE_ALPHA:
					pixelElements = 2;
					break;

				case GL_ALPHA:
					pixelElements = 1;
					break;

				case GL_RGB:
					pixelElements = 3;
					break;

				case GL_RGBA:
					pixelElements = 4;
					break;

				default:
					RecordInvalidEnum(state);
					return;
			}

			textureFormat = internalformat;
			break;

		case GL_UNSIGNED_SHORT_4_4_4_4:
			if (internalformat != GL_RGBA) {
				RecordInvalidEnum(state);
				return;
			}

			textureFormat = type;
			pixelSize = 2;
			pixelElements = 1;
			break;

		case GL_UNSIGNED_SHORT_5_5_5_1:
			if (internalformat != GL_RGBA) {
				RecordInvalidEnum(state);
				return;
			}

			textureFormat = type;
			pixelSize = 2;
			pixelElements = 1;

			break;

		case GL_UNSIGNED_SHORT_5_6_5:
			if (internalformat != GL_RGBA) {
				RecordInvalidEnum(state);
				return;
			}

			textureFormat = type;
			pixelSize = 2;
			pixelElements = 1;

			break;

		default:
			RecordInvalidEnum(state);
			return;
	}

	/************************************************************************/
	/* Verify image dimensions												*/
	/************************************************************************/

	if (width > GLES_MAX_TEXTURE_WIDTH || height > GLES_MAX_TEXTURE_HEIGHT) {
		RecordInvalidValue(state);
		return;
	}

	if ((width == 0 || height == 0) && pixels != NULL) {
		RecordInvalidValue(state);
		return;
	}

	if (width != 0 && height != 0 && pixels == NULL) {
		RecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Copy the actual image data											*/
	/************************************************************************/

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
}

GL_API void GL_APIENTRY 
glTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, 
				 GLsizei width, GLsizei height, GLsizei depth, 
				 GLenum format, GLenum type, const void *pixels) {
	State * state = GLES_GET_STATE();
}

