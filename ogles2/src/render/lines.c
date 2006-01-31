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
#include "render/render.h"
#include "raster/raster.h"

/*
** --------------------------------------------------------------------------
** Line Rendering
** --------------------------------------------------------------------------
*/
void GlesPrepareLines(State * state) {
}

static GLboolean Clip(const Vertex ** from, const Vertex ** to, Vertex ** nextTemporary, GLuint coord) {
	if ((*from)->data[coord] < -(*from)->v.position.w) {
		if ((*to)->data[coord] < -(*to)->v.position.w) {
			return GL_FALSE;
		} else {
			Vertex * newVertex = (*nextTemporary)++;
			GLfloat c_x = (*from)->data[coord];
			GLfloat c_w = -(*from)->v.position.w;
			GLfloat p_x = (*to)->data[coord];
			GLfloat p_w = -(*to)->v.position.w;
			
			GLfloat num = p_w - p_x; 
			GLfloat denom = (p_w - p_x) - (c_w - c_x);

			GlesInterpolateVertex(newVertex, *from, *to, num / denom);
			*from = newVertex;

			return GL_TRUE;
		}
	} else if ((*from)->data[coord] > (*from)->v.position.w) {
		if ((*to)->data[coord] > (*to)->v.position.w) {
			return GL_FALSE;
		} else {		
			Vertex * newVertex = (*nextTemporary)++;
			GLfloat c_x = (*from)->data[coord];
			GLfloat c_w = (*from)->v.position.w;
			GLfloat p_x = (*to)->data[coord];
			GLfloat p_w = (*to)->v.position.w;
			
			GLfloat num = p_w - p_x; 
			GLfloat denom = (p_w - p_x) - (c_w - c_x);

			GlesInterpolateVertex(newVertex, *from, *to, num / denom);
			*from = newVertex;

			return GL_TRUE;
		}
	} else if ((*to)->data[coord] < -(*to)->v.position.w) {

		Vertex * newVertex = (*nextTemporary)++;
		GLfloat c_x = (*to)->data[coord];
		GLfloat c_w = -(*to)->v.position.w;
		GLfloat p_x = (*from)->data[coord];
		GLfloat p_w = -(*from)->v.position.w;
		
		GLfloat num = p_w - p_x; 
		GLfloat denom = (p_w - p_x) - (c_w - c_x);

		GlesInterpolateVertex(newVertex, *to, *from, num / denom);
		*to = newVertex;

		return GL_TRUE;

	} else if ((*to)->data[coord] > (*to)->v.position.w) {

		Vertex * newVertex = (*nextTemporary)++;
		GLfloat c_x = (*to)->data[coord];
		GLfloat c_w = (*to)->v.position.w;
		GLfloat p_x = (*from)->data[coord];
		GLfloat p_w = (*from)->v.position.w;
		
		GLfloat num = p_w - p_x; 
		GLfloat denom = (p_w - p_x) - (c_w - c_x);

		GlesInterpolateVertex(newVertex, *to, *from, num / denom);
		*to = newVertex;

		return GL_TRUE;

	} else {
		// no clipping
		return GL_TRUE;
	}
}


void GlesRenderLine(State * state, const Vertex * p1, const Vertex * p2) {

	Vertex temporary[16];					/* space for vertices introduced through clipping	*/
	Vertex * nextTemporary = temporary;		/* pointer for next temporary vertex to use			*/
	RasterVertex a, b;

	if (Clip(&p1, &p2, &nextTemporary, 0) &&
		Clip(&p1, &p2, &nextTemporary, 1) &&
		Clip(&p1, &p2, &nextTemporary, 2)) {

		// TODO
		GlesViewportTransform(state, &a, p1);
		GlesViewportTransform(state, &b, p2);

#if 0
		if (m_RasterizerState.GetShadeModel() == RasterizerState::ShadeModelSmooth) {
			pFrom->m_Color = pFrom->m_FrontColor;
		} else {
			pFrom->m_Color = pTo->m_FrontColor;
		}

		pTo->m_Color = pTo->m_FrontColor;
# endif
		GlesRasterLine(state, &a, &b);
	}
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

