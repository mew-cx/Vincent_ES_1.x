/*
** ==========================================================================
**
** triangles.c		Triangle rendering functions
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


void GlesPrepareTriangles(State * state) {
}

void GlesRenderTriangle(State * state, const Vertex * p1, const Vertex * p2, const Vertex * p3) {
}

void GlesRenderTriangles(State * state, GLint first, GLsizei count, GLenum type, const void * indices) {

	if (count >= 3) {
		Vertex pos[3];

		GlesPrepareTriangles(state);

		while (count >= 3) {
			GLsizei idx;

			for (idx = 0; idx < 3; ++idx) {
				GlesSelectArrayElement(state, NEXT_INDEX);
				GlesProcessVertex(state, pos + idx);
			}

			GlesRenderTriangle(state, pos, pos + 1, pos + 2);

			count -= 3;
		}
	}
}

void GlesRenderTriangleStrip(State * state, GLint first, GLsizei count, GLenum type, const void * indices) {

	if (count >= 3) {
		Vertex pos[3];
		GLsizei idx;
		GLsizei even;

		GlesPrepareTriangles(state);

		for (idx = 0; idx < 2; ++idx) {
			GlesSelectArrayElement(state, NEXT_INDEX);
			GlesProcessVertex(state, pos + idx);
		}

		count -= 2;
		even = 0;

		while (count >= 1) {
			GlesSelectArrayElement(state, NEXT_INDEX);
			GlesProcessVertex(state, pos + idx);
			GlesRenderTriangle(state, pos + 2, pos + even, pos + (1 ^ even));

			count -= 1;
			if (--idx < 0)
				idx = 2;

			even ^= 1;
		}
	}
}


void GlesRenderTriangleFan(State * state, GLint first, GLsizei count, GLenum type, const void * indices) {

	if (count >= 3) {
		Vertex pos[3];
		GLsizei idx;

		GlesPrepareTriangles(state);

		GlesSelectArrayElement(state, NEXT_INDEX);
		GlesProcessVertex(state, pos + 2);
		GlesSelectArrayElement(state, NEXT_INDEX);
		GlesProcessVertex(state, pos + 0);

		count -= 2;
		idx = 1;

		while (count >= 1) {
			GlesSelectArrayElement(state, NEXT_INDEX);
			GlesProcessVertex(state, pos + idx);
			GlesRenderTriangle(state, pos + 2, pos + (idx ^ 1), pos + idx);

			count -= 1;
			idx ^= 1;
		}
	}
}

