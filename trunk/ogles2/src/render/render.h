#ifndef GLES_RENDER_RENDER_H
#define GLES_RENDER_RENDER_H 1

/*
** ==========================================================================
**
** render.h			Geometry Processing Function Declarations
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


#include "gl/state.h"


/*
** --------------------------------------------------------------------------
** General Rendering Functions
** --------------------------------------------------------------------------
*/

void GlesPrepareRendering(State * state);
void GlesSelectArrayElement(State * state, GLint index);
void GlesProcessVertex(State * state, Vertex * result);
void GlesInterpolateVertex(Vertex * newVertex, const Vertex * v0, const Vertex * v1, GLfloat coeff);
void GlesViewportTransform(State * state, RasterVertex * raster, const Vertex * vertex);

/*
** --------------------------------------------------------------------------
** Point Rendering Functions
** --------------------------------------------------------------------------
*/
void GlesPreparePoints(State * state);
void GlesRenderPoint(State * state, const Vertex * p1);
void GlesRenderPoints(State * state, GLint first, GLsizei count, GLenum type, const void * indices);

/*
** --------------------------------------------------------------------------
** Line Rendering Functions
** --------------------------------------------------------------------------
*/
void GlesPrepareLines(State * state);
void GlesRenderLine(State * state, const Vertex * p1, const Vertex * p2);

void GlesRenderLines(State * state, GLint first, GLsizei count, GLenum type, const void * indices);
void GlesRenderLineStrip(State * state, GLint first, GLsizei count, GLenum type, const void * indices);
void GlesRenderLineLoop(State * state, GLint first, GLsizei count, GLenum type, const void * indices);

/*
** --------------------------------------------------------------------------
** Triangle Rendering Functions
** --------------------------------------------------------------------------
*/
void GlesPrepareTriangles(State * state);
void GlesRenderTriangle(State * state, const Vertex * p1, const Vertex * p2, const Vertex * p3);

void GlesRenderTriangles(State * state, GLint first, GLsizei count, GLenum type, const void * indices);
void GlesRenderTriangleStrip(State * state, GLint first, GLsizei count, GLenum type, const void * indices);
void GlesRenderTriangleFan(State * state, GLint first, GLsizei count, GLenum type, const void * indices);

# define NEXT_INDEX (indices ? (type == GL_UNSIGNED_BYTE ? (*((const GLubyte *) indices)++) : (*((const GLushort *) indices)++)) : first++)

#endif /* ndef GLES_RENDER_RENDER_H */