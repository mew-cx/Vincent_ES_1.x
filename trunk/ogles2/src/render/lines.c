/*
** ==========================================================================
**
** lines.c			Line rendering functions
**
** --------------------------------------------------------------------------
**
** 06-14-2005		Hans-Martin Will	initial version
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
#include "render/render.h"

/*
** --------------------------------------------------------------------------
** Line Rendering
** --------------------------------------------------------------------------
*/
void GlesPrepareLines(State * state) {
}

void GlesRenderLine(State * state, const Vertex * p1, const Vertex * p2) {
}

/*
** --------------------------------------------------------------------------
** Lines
** --------------------------------------------------------------------------
*/
void GlesRenderLines(State * state, GLint first, GLsizei count, GLenum type, const void * indices) {

	if (count >= 2) {
		Vertex pos[2];

		GlesPrepareLines(state);

		while (count >= 2) {
			GlesSelectArrayElement(state, NEXT_INDEX);
			GlesProcessVertex(state, pos + 0);
			GlesSelectArrayElement(state, NEXT_INDEX);
			GlesProcessVertex(state, pos + 1);
			GlesRenderLine(state, pos, pos + 1);

			count -= 2;
		}
	}
}

/*
** --------------------------------------------------------------------------
** Line Strips
** --------------------------------------------------------------------------
*/

void GlesRenderLineStrip(State * state, GLint first, GLsizei count, GLenum type, const void * indices) {

	if (count >= 2) {
		Vertex pos[2];
		GLsizei idx;

		GlesPrepareLines(state);
		GlesSelectArrayElement(state, NEXT_INDEX);
		GlesProcessVertex(state, pos);
		--count;

		idx = 1;

		while (count >= 1) {
			GlesSelectArrayElement(state, NEXT_INDEX);
			GlesProcessVertex(state, pos + idx);
			GlesRenderLine(state, pos + (idx ^ 1), pos + idx);

			idx ^= 1;
			--count;
		}
	}
}


/*
** --------------------------------------------------------------------------
** Line Loops
** --------------------------------------------------------------------------
*/

void GlesRenderLineLoop(State * state, GLint first, GLsizei count, GLenum type, const void * indices) {

	if (count >= 2) {
		Vertex pos[3];
		GLsizei idx;

		GlesPrepareLines(state);
		GlesSelectArrayElement(state, NEXT_INDEX);
		GlesProcessVertex(state, pos + 2);
		GlesSelectArrayElement(state, NEXT_INDEX);
		GlesProcessVertex(state, pos);
		GlesRenderLine(state, pos + 2, pos);

		count -= 2;
		idx = 1;

		while (count >= 1) {
			GlesSelectArrayElement(state, NEXT_INDEX);
			GlesProcessVertex(state, pos + idx);
			GlesRenderLine(state, pos + (idx ^ 1), pos + idx);

			idx ^= 1;
			--count;
		}

		GlesRenderLine(state, pos + idx, pos + 2);
	}
}

