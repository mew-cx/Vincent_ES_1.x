// ==========================================================================
//
// triangles.cpp	Rendering Context Class for 3D Rendering Library
//
//					Rendering Operations for Triangles
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


namespace {

	inline size_t ClipLow(Vertex * input[], size_t inputCount, Vertex * output[], Vertex *& nextTemporary, size_t coord, size_t numVarying) {

		if (inputCount < 3) {
			return 0;
		}

		Vertex * previous = input[inputCount - 1];
		Vertex * current;
		int resultCount = 0;

		for (size_t index = 0; index < inputCount; ++index) {

			current = input[index];

			if (current->m_ClipCoords[coord] >= -current->m_ClipCoords.w()) {

				if (previous->m_ClipCoords[coord] >= -previous->m_ClipCoords.w()) {
					// line segment between previous and current is fully contained in cube
					output[resultCount++] = current;
					//previous = current;
				} else {
					// line segment between previous and current is intersected;
					// create vertex at intersection, then add current
					Vertex & newVertex = *nextTemporary++;
					output[resultCount++] = &newVertex;

					EGL_Fixed c_x = current->m_ClipCoords[coord];
					EGL_Fixed c_w = current->m_ClipCoords.w();
					EGL_Fixed p_x = previous->m_ClipCoords[coord];
					EGL_Fixed p_w = previous->m_ClipCoords.w();
					EGL_Fixed num = p_w + p_x;
					EGL_Fixed denom = (p_w + p_x) - (c_w + c_x);

					Interpolate(newVertex, *current, *previous, num, denom, numVarying);
					newVertex.m_ClipCoords[coord] = -newVertex.m_ClipCoords.w();

					output[resultCount++] = current;
					//previous = current;
				}
			} else {
				if (previous->m_ClipCoords[coord] >= -previous->m_ClipCoords.w()) {
					// line segment between previous and current is intersected;
					// create vertex at intersection and add it
					Vertex & newVertex = *nextTemporary++;
					output[resultCount++] = &newVertex;

					EGL_Fixed c_x = current->m_ClipCoords[coord];
					EGL_Fixed c_w = current->m_ClipCoords.w();
					EGL_Fixed p_x = previous->m_ClipCoords[coord];
					EGL_Fixed p_w = previous->m_ClipCoords.w();
					EGL_Fixed num = p_w + p_x;
					EGL_Fixed denom = (p_w + p_x) - (c_w + c_x);

					Interpolate(newVertex, *current, *previous, num, denom, numVarying);
					newVertex.m_ClipCoords[coord] = -newVertex.m_ClipCoords.w();

					//previous = current;
				}
			}

			previous = current;
		}

		return resultCount;

	}

	inline size_t ClipHigh(Vertex * input[], size_t inputCount, Vertex * output[], Vertex *& nextTemporary, size_t coord, size_t numVarying) {

		if (inputCount < 3) {
			return 0;
		}

		Vertex * previous = input[inputCount - 1];
		Vertex * current;
		int resultCount = 0;

		for (size_t index = 0; index < inputCount; ++index) {

			current = input[index];

			if (current->m_ClipCoords[coord] < current->m_ClipCoords.w()) {

				if (previous->m_ClipCoords[coord] < previous->m_ClipCoords.w()) {
					// line segment between previous and current is fully contained in cube
					output[resultCount++] = current;
				} else {
					// line segment between previous and current is intersected;
					// create vertex at intersection, then add current
					Vertex & newVertex = *nextTemporary++;
					output[resultCount++] = &newVertex;

					EGL_Fixed c_x = current->m_ClipCoords[coord];
					EGL_Fixed c_w = current->m_ClipCoords.w();
					EGL_Fixed p_x = previous->m_ClipCoords[coord];
					EGL_Fixed p_w = previous->m_ClipCoords.w();
					EGL_Fixed num = p_w - p_x;
					EGL_Fixed denom = (p_w - p_x) - (c_w - c_x);

					Interpolate(newVertex, *current, *previous, num, denom, numVarying);
					newVertex.m_ClipCoords[coord] = newVertex.m_ClipCoords.w();

					output[resultCount++] = current;
				}
			} else {
				if (previous->m_ClipCoords[coord] < previous->m_ClipCoords.w()) {
					// line segment between previous and current is intersected;
					// create vertex at intersection and add it
					Vertex & newVertex = *nextTemporary++;
					output[resultCount++] = &newVertex;

					EGL_Fixed c_x = current->m_ClipCoords[coord];
					EGL_Fixed c_w = current->m_ClipCoords.w();
					EGL_Fixed p_x = previous->m_ClipCoords[coord];
					EGL_Fixed p_w = previous->m_ClipCoords.w();
					EGL_Fixed num = p_w - p_x;
					EGL_Fixed denom = (p_w - p_x) - (c_w - c_x);

					Interpolate(newVertex, *current, *previous, num, denom, numVarying);
					newVertex.m_ClipCoords[coord] = newVertex.m_ClipCoords.w();
				}
			}

			previous = current;
		}

		return resultCount;

	}

	size_t ClipUser(const Vec4D& plane, Vertex * input[], size_t inputCount, Vertex * output[], Vertex *& nextTemporary, size_t numVarying) {
		if (inputCount < 3) {
			return 0;
		}

		Vertex * previous = input[inputCount - 1];
		Vertex * current;
		int resultCount = 0;

		for (size_t index = 0; index < inputCount; ++index) {

			current = input[index];

			EGL_Fixed c = current->m_EyeCoords * plane;
			EGL_Fixed p = previous->m_EyeCoords * plane;

			if (c > 0.0f) {
				if (p >= 0.0f) {
					// line segment between previous and current is fully contained in cube
					output[resultCount++] = current;
				} else {
					// line segment between previous and current is intersected;
					// create vertex at intersection, then add current
					Vertex & newVertex = *nextTemporary++;
					output[resultCount++] = &newVertex;

					InterpolateWithEye(newVertex, *current, *previous, p, p - c, numVarying);
					output[resultCount++] = current;
				}
			} else {
				if (p > 0.0f) {
					// line segment between previous and current is intersected;
					// create vertex at intersection and add it
					Vertex & newVertex = *nextTemporary++;
					output[resultCount++] = &newVertex;

					InterpolateWithEye(newVertex, *current, *previous, p, p - c, numVarying);
				}
			}

			previous = current;
		}

		return resultCount;
	}

}


void Context :: FrontFace(GLenum mode) {

	switch (mode) {
		case GL_CW:
			m_ReverseFaceOrientation = true;
			break;

		case GL_CCW:
			m_ReverseFaceOrientation = false;
			break;

		default:
			RecordError(GL_INVALID_ENUM);
			break;
	}
}

void Context :: CullFace(GLenum mode) {

	switch (mode) {
		case GL_FRONT:
			m_CullMode = CullModeFront;
			break;

		case GL_BACK:
			m_CullMode = CullModeBack;
			break;

		case GL_FRONT_AND_BACK:
			m_CullMode = CullModeBackAndFront;
			break;

		default:
			RecordError(GL_INVALID_ENUM);
			break;
	}
}

namespace {
	inline I64 MulLong(EGL_Fixed a, EGL_Fixed b) {
		return (((I64) a * (I64) b)  >> EGL_PRECISION);
	}

	inline EGL_Fixed Round(EGL_Fixed value) {
		return (value + 8) >> 4;
	}
}

void Context :: RenderTriangle(Vertex& a, Vertex& b, Vertex& c) {

	EGL_Fixed x0 = a.m_ClipCoords.w();
	EGL_Fixed x1 = a.m_ClipCoords.x();
	EGL_Fixed x2 = a.m_ClipCoords.y();
								
	EGL_Fixed y0 = b.m_ClipCoords.w();
	EGL_Fixed y1 = b.m_ClipCoords.x();
	EGL_Fixed y2 = b.m_ClipCoords.y();
								
	EGL_Fixed z0 = c.m_ClipCoords.w();
	EGL_Fixed z1 = c.m_ClipCoords.x();
	EGL_Fixed z2 = c.m_ClipCoords.y();

	I64 sign;
	
	if (((x0 & 0xff000000) == 0 || (x0 & 0xff000000) == 0xff000000) &&
		((y0 & 0xff000000) == 0 || (y0 & 0xff000000) == 0xff000000) &&
		((z0 & 0xff000000) == 0 || (z0 & 0xff000000) == 0xff000000)) {
		sign = 
			+ Round(x0) * (MulLong(Round(y1), Round(z2)) - MulLong(Round(z1), Round(y2)))
			- Round(y0) * (MulLong(Round(x1), Round(z2)) - MulLong(Round(z1), Round(x2)))
			+ Round(z0) * (MulLong(Round(x1), Round(y2)) - MulLong(Round(y1), Round(x2)));
	} else {
		sign = 
			+ Round(x0 >> 6) * (MulLong(Round(y1), Round(z2)) - MulLong(Round(z1), Round(y2)))
			- Round(y0 >> 6) * (MulLong(Round(x1), Round(z2)) - MulLong(Round(z1), Round(x2)))
			+ Round(z0 >> 6) * (MulLong(Round(x1), Round(y2)) - MulLong(Round(y1), Round(x2)));
	}

	bool cw = (sign < 0);
	bool backFace = cw ^ m_ReverseFaceOrientation;

	if (m_CullFaceEnabled) {
		if ((m_CullMode == CullModeFront) ^ backFace)
			return;
	}

	if (m_VaryingInfo->colorIndex >= 0) {
		if (m_RasterizerState.GetShadeModel() == RasterizerState::ShadeModelSmooth) {
			if (m_TwoSidedLightning && backFace) {
				a.m_BackColor.toArray(a.m_Varying + m_VaryingInfo->colorIndex);
				b.m_BackColor.toArray(b.m_Varying + m_VaryingInfo->colorIndex);
				c.m_BackColor.toArray(c.m_Varying + m_VaryingInfo->colorIndex);
			} else {
				a.m_FrontColor.toArray(a.m_Varying + m_VaryingInfo->colorIndex);
				b.m_FrontColor.toArray(b.m_Varying + m_VaryingInfo->colorIndex);
				c.m_FrontColor.toArray(c.m_Varying + m_VaryingInfo->colorIndex);
			}
		} else {
			if (m_TwoSidedLightning && backFace) {
				c.m_BackColor.toArray(a.m_Varying + m_VaryingInfo->colorIndex);
				c.m_BackColor.toArray(b.m_Varying + m_VaryingInfo->colorIndex);
				c.m_BackColor.toArray(c.m_Varying + m_VaryingInfo->colorIndex);
			} else {
				c.m_FrontColor.toArray(a.m_Varying + m_VaryingInfo->colorIndex);
				c.m_FrontColor.toArray(b.m_Varying + m_VaryingInfo->colorIndex);
				c.m_FrontColor.toArray(c.m_Varying + m_VaryingInfo->colorIndex);
			}
		}
	}

	Vertex * array1[16];
	array1[0] = &a;

	if (!cw) {
		array1[1] = &b;
		array1[2] = &c;
	} else {
		array1[2] = &b;
		array1[1] = &c;
	}

	Vertex * array2[16];
	Vertex * tempVertices = m_Temporary;

	size_t numVertices = 3;
	size_t numVarying = m_VaryingInfo->numVarying;

	if (m_ClipPlaneEnabled) {
		for (size_t index = 0, mask = 1; index < NUM_CLIP_PLANES; ++index, mask <<= 1) {
			if (m_ClipPlaneEnabled & mask) {
				numVertices = ClipUser(m_ClipPlanes[index], array1, numVertices, array2, tempVertices, numVarying);

				if (!numVertices) {
					return;
				}

				for (size_t idx = 0; idx < numVertices; ++idx) {
					array1[idx] = array2[idx];
				}
			}
		}
	}

	for (size_t coord = 0; coord < 3; ++coord) {
		numVertices = ClipLow(array1, numVertices, array2, tempVertices, coord, numVarying);
		numVertices = ClipHigh(array2, numVertices, array1, tempVertices, coord, numVarying);
	}

	if (numVertices >= 3) {
		ClipCoordsToWindowCoords(*array1[0]);
		ClipCoordsToWindowCoords(*array1[1]);

		for (size_t index = 2; index < numVertices; ++index) {
			ClipCoordsToWindowCoords(*array1[index]);
			m_Rasterizer->RasterTriangle(*array1[0], *array1[index - 1], *array1[index]);
		}
	}
}


void Context :: DrawTriangle(int index) {
	SelectArrayElement(index);
	CurrentValuesToRasterPos(&m_Input[m_NextIndex++]);

	if (m_NextIndex == 3) {
		RenderTriangle(m_Input[0], m_Input[1], m_Input[2]);
		m_NextIndex = 0;
	}
}

void Context :: DrawTriangleStrip(int index) {
	SelectArrayElement(index);
	CurrentValuesToRasterPos(&m_Input[m_NextIndex++]);

	if (m_PrimitiveState == 3) {
		// even triangle
		RenderTriangle(m_Input[0], m_Input[2], m_Input[1]);
		m_PrimitiveState = 2;
	} else if (m_PrimitiveState == 2) {
		// odd triangle
		RenderTriangle(m_Input[0], m_Input[1], m_Input[2]);
		m_PrimitiveState = 3;
	} else {
		// remember seen a vertex
		++m_PrimitiveState;
	}

	if (m_NextIndex == 3)
		m_NextIndex = 0;
}

void Context :: DrawTriangleFan(int index) {
	SelectArrayElement(index);
	CurrentValuesToRasterPos(&m_Input[m_NextIndex++]);

	if (m_PrimitiveState == 3) {
		// even triangle
		RenderTriangle(m_Input[0], m_Input[2], m_Input[1]);
		m_PrimitiveState = 2;
	} else if (m_PrimitiveState == 2) {
		// odd triangle
		RenderTriangle(m_Input[0], m_Input[1], m_Input[2]);
		m_PrimitiveState = 3;
	} else if (m_PrimitiveState == 1) {
		// remember seen second vertex
		m_PrimitiveState = 2;
	} else if (m_PrimitiveState == 0) {
		// remember seen first vertex
		m_PrimitiveState = 1;
	}

	if (m_NextIndex == 3)
		m_NextIndex = 1;
}

