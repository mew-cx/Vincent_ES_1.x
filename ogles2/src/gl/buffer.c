/*
** ==========================================================================
**
** buffer.c			Buffer management functions
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

static GLenum BufferUsage[] = {
	GL_STATIC_DRAW,
	GL_DYNAMIC_DRAW
};

/*
** --------------------------------------------------------------------------
** Module-local functions
** --------------------------------------------------------------------------
*/

static Buffer * GetBufferForTarget(State * state, GLenum target) {
	switch (target) {
		case GL_ARRAY_BUFFER:
			return state->buffers + state->arrayBuffer;

		case GL_ELEMENT_ARRAY_BUFFER:
			return state->buffers + state->elementArrayBuffer;

		default:
			GlesRecordInvalidEnum(state);
			return NULL;
	}
}

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

void GlesInitBuffer(Buffer * buffer) {
	buffer->data		= NULL;
	buffer->size		= 0;
	buffer->bufferType	= GL_INVALID_ENUM;
	buffer->usage		= GL_INVALID_ENUM;
	buffer->access		= GL_WRITE_ONLY;
	buffer->mapped		= GL_FALSE;
	buffer->mapPointer	= NULL;
}

void GlesDeallocateBuffer(Buffer * buffer) {
	if (buffer->data != NULL) {
		GlesFree(buffer->data);
	}

	GlesInitBuffer(buffer);
}

/*
** --------------------------------------------------------------------------
** Public API entry points
** --------------------------------------------------------------------------
*/

GL_API void GL_APIENTRY glBindBuffer (GLenum target, GLuint buffer) {
	State * state = GLES_GET_STATE();
	GLuint * bufferRef = NULL;

	switch (target) {
		case GL_ARRAY_BUFFER:
			bufferRef = &state->arrayBuffer;
			break;

		case GL_ELEMENT_ARRAY_BUFFER:
			bufferRef = &state->elementArrayBuffer;
			break;

		default:
			GlesRecordInvalidEnum(state);
			return;
	}

	if (buffer > GLES_MAX_BUFFERS) {
		GlesRecordOutOfMemory(state);
		return;
	}

	*bufferRef = buffer;
}

GL_API void GL_APIENTRY glBufferData (GLenum target, GLsizeiptr size, const void *data, GLenum usage) {
	State * state = GLES_GET_STATE();
	Buffer * buffer = GetBufferForTarget(state, target);

	if (buffer == NULL) {
		return;
	}

	if (!GlesValidateEnum(state, usage, BufferUsage, GLES_ELEMENTSOF(BufferUsage))) {
		return;
	}

	if (size < 0) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (buffer->data) {
		GlesDeleteBuffer(buffer);
	}

	buffer->data = GlesMalloc(size);

	if (buffer->data == NULL) {
		GlesRecordOutOfMemory(state);
		return;
	}

	buffer->usage = usage;
	buffer->size = size;

	GLES_MEMCPY(buffer->data, data, size);
}

GL_API void GL_APIENTRY glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const void *data) {
	State * state = GLES_GET_STATE();
	Buffer * buffer = GetBufferForTarget(state, target);

	if (buffer == NULL) {
		return;
	}

	if (size < 0 || offset < 0 || size + offset > buffer->size) {
		GlesRecordInvalidValue(state);
		return;
	}

	GLES_MEMCPY((GLubyte * )buffer->data + offset, data, size);
}

GL_API void GL_APIENTRY glDeleteBuffers (GLsizei n, const GLuint *buffers) {
	State * state = GLES_GET_STATE();

	/************************************************************************/
	/* Validate parameters													*/
	/************************************************************************/

	if (n < 0 || buffers == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Delete the individual textures										*/
	/************************************************************************/

	while (n--) {
		if (GlesIsBoundObject(state->bufferFreeListHead, state->bufferFreeList, GLES_MAX_BUFFERS, *buffers)) {
			if (*buffers == state->arrayBuffer) {
				state->arrayBuffer = 0;
			}

			if (*buffers == state->elementArrayBuffer) {
				state->elementArrayBuffer = 0;
			}

			GlesDeallocateBuffer(state->buffers + *buffers);
		}

		++buffers;
	}
}

GL_API void GL_APIENTRY glGenBuffers (GLsizei n, GLuint *buffers) {

	State * state = GLES_GET_STATE();
	GlesGenObjects(state, state->bufferFreeListHead, state->bufferFreeList, GLES_MAX_BUFFERS, n, buffers);
}

GL_API void GL_APIENTRY glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params) {
	State * state = GLES_GET_STATE();
	Buffer * buffer = GetBufferForTarget(state, target);

	if (buffer == NULL) {
		return;
	}

	switch (pname) {
	case GL_BUFFER_SIZE:
		params[0] = buffer->size;
		break;

	case GL_BUFFER_USAGE:
		params[0] = buffer->usage;
		break;

	case GL_BUFFER_ACCESS:
		params[0] = buffer->access;
		break;

	default:
		GlesRecordInvalidEnum(state);
		return;
	}
}

GL_API void GL_APIENTRY glGetBufferPointerv (GLenum target, GLenum pname, void* *params) {
	State * state = GLES_GET_STATE();
	Buffer * buffer = GetBufferForTarget(state, target);

	if (buffer == NULL) {
		return;
	}

	switch (pname) {
		case GL_BUFFER_MAP_POINTER:
			params[0] = buffer->mapPointer;
			break;

		default:
			GlesRecordInvalidEnum(state);
			return;
	}
}

GL_API GLboolean GL_APIENTRY glIsBuffer (GLuint buffer) {

	State * state = GLES_GET_STATE();
	return GlesIsBoundObject(state->bufferFreeListHead, state->bufferFreeList, GLES_MAX_BUFFERS, buffer) &&
		state->buffers[buffer].bufferType != GL_INVALID_ENUM;
}

GL_API void* GL_APIENTRY glMapBuffer (GLenum target, GLenum access) {
	State * state = GLES_GET_STATE();
	Buffer * buffer = GetBufferForTarget(state, target);

	if (buffer == NULL) {
		return NULL;
	}

	if (buffer->mapped) {
		GlesRecordError(state, GL_INVALID_OPERATION);
		return NULL;
	}

	buffer->mapped = GL_TRUE;
	buffer->mapPointer = buffer->data;

	return buffer->mapPointer;
}

GL_API GLboolean GL_APIENTRY glUnmapBuffer (GLenum target) {
	State * state = GLES_GET_STATE();
	Buffer * buffer = GetBufferForTarget(state, target);

	if (buffer == NULL) {
		return GL_FALSE;
	}

	if (!buffer->mapped) {
		GlesRecordError(state, GL_INVALID_OPERATION);
		return GL_FALSE;
	}

	buffer->mapped = GL_FALSE;
	buffer->mapPointer = NULL;

	return GL_TRUE;
}

