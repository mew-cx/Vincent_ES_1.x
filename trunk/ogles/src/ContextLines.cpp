// ==========================================================================
//
// ContextLines.cpp	Rendering Context Class for 3D Rendering Library
//
//					Rendering Operations for Lines
//
// --------------------------------------------------------------------------
//
// 08-12-2003	Hans-Martin Will	initial version
//
// --------------------------------------------------------------------------
//
// Copyright (c) 2004, Hans-Martin Will. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//	 *  Redistributions of source code must retain the above copyright
// 		notice, this list of conditions and the following disclaimer.
//   *	Redistributions in binary form must reproduce the above copyright
// 		notice, this list of conditions and the following disclaimer in the
// 		documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
// ==========================================================================


#include "stdafx.h"
#include "Context.h"
#include "fixed.h"
#include "Rasterizer.h"
#include "Utils.h"

using namespace EGL;


void Context :: LineWidthx(GLfixed width) {

	if (width <= 0) {
		RecordError(GL_INVALID_VALUE);
	} else {
		GetRasterizerState()->SetLineWidth(width);
	}
}


namespace {

	inline bool Clip(Vertex*& from, Vertex*& to, Vertex *&tempVertices, size_t coord, size_t numVarying) {
		if (from->m_ClipCoords[coord] < -from->m_ClipCoords.w()) {
			if (to->m_ClipCoords[coord] < -to->m_ClipCoords.w()) {
				return false;
			}

			EGL_Fixed c_x = from->m_ClipCoords[coord];
			EGL_Fixed c_w = -from->m_ClipCoords.w();
			EGL_Fixed p_x = to->m_ClipCoords[coord];
			EGL_Fixed p_w = -to->m_ClipCoords.w();

			EGL_Fixed num = p_w - p_x;
			EGL_Fixed denom = (p_w - p_x) - (c_w - c_x);

			Interpolate(*tempVertices, *from, *to, num, denom, numVarying);
			from = tempVertices++;

			return true;

		} else if (from->m_ClipCoords[coord] > from->m_ClipCoords.w()) {
			if (to->m_ClipCoords[coord] > to->m_ClipCoords.w()) {
				return false;
			}

			EGL_Fixed c_x = from->m_ClipCoords[coord];
			EGL_Fixed c_w = from->m_ClipCoords.w();
			EGL_Fixed p_x = to->m_ClipCoords[coord];
			EGL_Fixed p_w = to->m_ClipCoords.w();

			EGL_Fixed num = p_w - p_x;
			EGL_Fixed denom = (p_w - p_x) - (c_w - c_x);

			Interpolate(*tempVertices, *from, *to, num, denom, numVarying);
			from = tempVertices++;

			return true;

		} else if (to->m_ClipCoords[coord] < -to->m_ClipCoords.w()) {

			EGL_Fixed c_x = to->m_ClipCoords[coord];
			EGL_Fixed c_w = -to->m_ClipCoords.w();
			EGL_Fixed p_x = from->m_ClipCoords[coord];
			EGL_Fixed p_w = -from->m_ClipCoords.w();

			EGL_Fixed num = p_w - p_x;
			EGL_Fixed denom = (p_w - p_x) - (c_w - c_x);

			Interpolate(*tempVertices, *to, *from, num, denom, numVarying);
			to = tempVertices++;

			return true;

		} else if (to->m_ClipCoords[coord] > to->m_ClipCoords.w()) {

			EGL_Fixed c_x = to->m_ClipCoords[coord];
			EGL_Fixed c_w = to->m_ClipCoords.w();
			EGL_Fixed p_x = from->m_ClipCoords[coord];
			EGL_Fixed p_w = from->m_ClipCoords.w();

			EGL_Fixed num = p_w - p_x;
			EGL_Fixed denom = (p_w - p_x) - (c_w - c_x);

			Interpolate(*tempVertices, *to, *from, num, denom, numVarying);
			to = tempVertices++;

			return true;

		} else {
			// no clipping
			return true;
		}
	}

	inline bool ClipUser(const Vec4D& plane, Vertex*& from, Vertex*& to, Vertex *&tempVertices, size_t numVarying) {

		EGL_Fixed f = from->m_EyeCoords * plane;
		EGL_Fixed t = to->m_EyeCoords * plane;

		if (f < 0) {
			if (t <= 0) {
				return false;
			}

			Interpolate(*tempVertices, *from, *to, t, t - f, numVarying);
			from = tempVertices++;

			return true;

		} else if (t < 0) {

			Interpolate(*tempVertices, *to, *from, f, f - t, numVarying);
			to = tempVertices++;

			return true;

		} else {
			// no clipping
			return true;
		}
	}
}


void Context :: RenderLine(Vertex& from, Vertex& to) {

	Vertex * tempVertices = m_Temporary;
	Vertex * pFrom = &from;
	Vertex * pTo = &to;

	if (m_ClipPlaneEnabled) {
		for (size_t index = 0, mask = 1; index < NUM_CLIP_PLANES; ++index, mask <<= 1) {
			if (m_ClipPlaneEnabled & mask) {
				if (!ClipUser(m_ClipPlanes[index], pFrom, pTo, tempVertices, m_VaryingInfo->numVarying)) {
					return;
				}
			}
		}
	}

	if (Clip(pFrom, pTo, tempVertices, 0, m_VaryingInfo->numVarying) &&
		Clip(pFrom, pTo, tempVertices, 1, m_VaryingInfo->numVarying) &&
		Clip(pFrom, pTo, tempVertices, 2, m_VaryingInfo->numVarying)) {

		ClipCoordsToWindowCoords(*pFrom);
		ClipCoordsToWindowCoords(*pTo);

		if (m_VaryingInfo->colorIndex >= 0) {
			if (m_RasterizerState.GetShadeModel() == RasterizerState::ShadeModelSmooth) {
				pFrom->m_FrontColor.toArray(pFrom->m_Varying + m_VaryingInfo->colorIndex);
			} else {
				pTo->m_FrontColor.toArray(pFrom->m_Varying + m_VaryingInfo->colorIndex);
			}

			pTo->m_FrontColor.toArray(pTo->m_Varying + m_VaryingInfo->colorIndex);
		}

		m_Rasterizer->RasterLine(*pFrom, *pTo);
	}
}


void Context :: DrawLine(int index) {
	SelectArrayElement(index);
	CurrentValuesToRasterPos(&m_Input[m_NextIndex++]);

	if (m_NextIndex == 2) {
		RenderLine(m_Input[0], m_Input[1]);
		m_NextIndex = 0;
	}
}

void Context :: DrawLineStrip(int index) {
	SelectArrayElement(index);
	CurrentValuesToRasterPos(&m_Input[m_NextIndex++]);

	if (m_PrimitiveState != 0) {
		// determine line orienation based on parity
		if (m_NextIndex & 1) {
			RenderLine(m_Input[1], m_Input[0]);
		} else {
			RenderLine(m_Input[0], m_Input[1]);
		}
	} else {
		// remember that we have seen the first vertex
		m_PrimitiveState = 1;
	}

	if (m_NextIndex == 2) {
		m_NextIndex = 0;
	}
}

void Context :: DrawLineLoop(int index) {
	SelectArrayElement(index);
	CurrentValuesToRasterPos(&m_Input[m_NextIndex++]);

	if (m_PrimitiveState == 2) {
		// we have seen at least 3 vertices
		if (m_NextIndex & 1) {
			RenderLine(m_Input[1], m_Input[2]);
		} else {
			RenderLine(m_Input[2], m_Input[1]);
		}
	} else if (m_PrimitiveState == 1) {
		// determine line orienation based on parity
		RenderLine(m_Input[0], m_Input[1]);

		// we have seen at least 2 vertices
		m_PrimitiveState = 2;
	} else {
		// remember that we have seen the first vertex
		m_PrimitiveState = 1;
	}

	if (m_NextIndex == 3) {
		m_NextIndex = 1;
	}
}

void Context :: EndLineLoop() {
	if (m_PrimitiveState == 2) {
		// index of last vertex written
		I32 prevIndex = 3 - m_NextIndex;

		RenderLine(m_Input[prevIndex], m_Input[0]);
	}
}
