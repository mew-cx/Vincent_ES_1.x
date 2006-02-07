/*
** ==========================================================================
**
** platform.c		Platform bindings
**
** --------------------------------------------------------------------------
**
** 10-21-2005		Hans-Martin Will	initial version
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
#include <stdlib.h>
#include <string.h>


void GLES_MEMSET(void * ptr, GLubyte value, GLsizeiptr size)
{
	memset(ptr, value, size);
}

void GLES_MEMCPY(void * dst, const void * src, GLsizeiptr size) {
	memcpy(dst, src, size);
}

void GLES_MEMMOVE(void * dst, const void * src, GLsizeiptr size) {
	memmove(dst, src, size);
}


/*
** --------------------------------------------------------------------------
** Memory Management
** --------------------------------------------------------------------------
*/

void * GlesMalloc(GLsizeiptr size) {
	void * result = malloc(size);

	if (result) {
		GLES_MEMSET(result, 0, size);
	}

	return result;
}

void GlesFree(void * ptr) {
	free(ptr);
}


