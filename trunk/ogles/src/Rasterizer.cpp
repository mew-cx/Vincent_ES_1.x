// ==========================================================================
//
// Rasterizer.cpp	Rasterizer Class for Embedded OpenGL Implementation
//
//					The rasterizer converts transformed and lit 
//					primitives and creates a raster image in the
//					current rendering surface.
//
// --------------------------------------------------------------------------
//
// 10-06-2003		Hans-Martin Will	initial version
//
// ==========================================================================


#include "stdafx.h"
#include "Rasterizer.h"
#include "Surface.h"
#include "Texture.h"


using namespace EGL;

// --------------------------------------------------------------------------
// Local helper functions
// --------------------------------------------------------------------------

namespace {
	I8 Permutation[8][3] = {
		{ 0, 1, 2 },
		{ 0, 2, 1 },
		{ 0, 0, 0 },	// impossible
		{ 2, 0, 1 },
		{ 1, 0, 2 },
		{ 0, 0, 0 },	// impossible
		{ 1, 2, 0 },
		{ 2, 1, 0 },
	};

	inline I8 * SortPermutation(I32 x0, I32 x1, I32 x2) {
		U32 y0 = static_cast<U32>(x0);
		U32 y1 = static_cast<U32>(x1);
		U32 y2 = static_cast<U32>(x2);

		return Permutation[
			(((y1 - y0) >> 29) & 4) |
			(((y2 - y0) >> 30) & 2) |
			(((y2 - y1) >> 31) & 1)];
	}

	inline int Greater(I32 x0, I32 x1) {
		U32 y0 = static_cast<U32>(x0);
		U32 y1 = static_cast<U32>(x1);

		return (y1 - y0) >> 31;
	}
}


// --------------------------------------------------------------------------
// Class Rasterizer
// --------------------------------------------------------------------------


Rasterizer :: Rasterizer(RasterizerState * state):
	m_IsPrepared(false),
	m_State(state)
{
}


Rasterizer :: ~Rasterizer() {
}


void Rasterizer :: SetState(RasterizerState * state) {
	m_State = state;
}


RasterizerState * Rasterizer :: GetState() const {
	return m_State;
}


inline void Rasterizer :: Fragment(I32 x, I32 y, EGL_Fixed depth, const Color & color) {
	// will have special cases based on settings
	// for now, no special support for blending etc.

	bool depthTest;
	
	// fragment level clipping (for now)
	if (m_Surface->GetWidth() <= x || x < 0 ||
		m_Surface->GetHeight() <= y || y < 0) {
		return;
	}

	U32 offset = x + y * m_Surface->GetWidth();
	I32 zBufferValue = m_Surface->GetDepthBuffer()[offset];

	switch (m_State->m_DepthFunc) {
		default:
		case RasterizerState::CompFuncNever:	depthTest = false;						break;
		case RasterizerState::CompFuncLess:		depthTest = depth < zBufferValue;		break;
		case RasterizerState::CompFuncEqual:	depthTest = depth == zBufferValue;		break;
		case RasterizerState::CompFuncLEqual:	depthTest = depth <= zBufferValue;		break;
		case RasterizerState::CompFuncGreater:	depthTest = depth > zBufferValue;		break;
		case RasterizerState::CompFuncNotEqual:	depthTest = depth != zBufferValue;		break;
		case RasterizerState::CompFuncGEqual:	depthTest = depth >= zBufferValue;		break;
		case RasterizerState::CompFuncAlways:	depthTest = true;						break;
	}

	// TODO: Update of stencil buffer

	if (!depthTest && m_State->m_DepthTestEnabled) {
		return;
	}

	// TODO: Alpha Buffer
	// TODO: Blending
	// TODO: Masking of color and depth

	m_Surface->GetDepthBuffer()[offset] = depth;
	m_Surface->GetColorBuffer()[offset] = color.ConvertTo565();
}


inline void Rasterizer :: RasterScanLine(const EdgePos& start, const EdgePos& end, U32 y) {

	// TODO: The depth coordinate should really be interpolated perspectively

	FractionalColor color = start.m_Color;
	EGL_Fixed invSpan = EGL_Inverse(end.m_WindowCoords.x - start.m_WindowCoords.x);

	FractionalColor colorIncrement = (end.m_Color - start.m_Color) * invSpan;
	EGL_Fixed w = start.m_WindowCoords.w;
	EGL_Fixed deltaW = EGL_Mul(end.m_WindowCoords.w - start.m_WindowCoords.w, invSpan);
	EGL_Fixed deltaU = EGL_Mul(end.m_TextureCoords.tu - start.m_TextureCoords.tu, invSpan);
	EGL_Fixed deltaV = EGL_Mul(end.m_TextureCoords.tv - start.m_TextureCoords.tv, invSpan);
	EGL_Fixed tu = start.m_TextureCoords.tu;
	EGL_Fixed tv = start.m_TextureCoords.tv;
	I32 x = EGL_IntFromFixed(start.m_WindowCoords.x);
	I32 xEnd = EGL_IntFromFixed(end.m_WindowCoords.x);

	for (; x < xEnd; ++x) {

		if (m_State->m_TextureEnabled) {
			FractionalColor texColor = m_State->m_Texture->GetTexture(0)->GetPixel(tu, tv);

#if 0
			switch (m_State->m_TextureMode) {
				case TextureModeDecal:
				case TextureModeReplace:
				case TextureModeBlend:
				case TextureModeAdd:
				case TextureModeModulate:
			}
#endif
			Fragment(x, y, w, color * texColor);
		} else {
			Fragment(x, y, w, color);
		}

		color += colorIncrement;
		w += deltaW;
		tu += deltaU;
		tv += deltaV;
	}
}


// --------------------------------------------------------------------------
// Prepare rasterizer with according to current state settings
// --------------------------------------------------------------------------
void Rasterizer :: PreparePoint() {
	if (!m_IsPrepared) {
	}

	m_IsPrepared = true;
}

void Rasterizer :: PrepareLine() {
	if (!m_IsPrepared) {
	}

	m_IsPrepared = true;
}

void Rasterizer :: PrepareTriangle() {
	if (!m_IsPrepared) {
	}

	m_IsPrepared = true;
}




void Rasterizer :: Finish() {
}


void Rasterizer :: RasterPoint(const RasterPos& point) {
}


void Rasterizer :: RasterLine(const RasterPos& from, const RasterPos& to) {
}


// ---------------------------------------------------------------------------
// Render the triangle specified by the three transformed and lit vertices
// passed as arguments. Before calling into the actual rasterization, the
// triangle will be subject to the scissor test, which may subdivide it
// into up to 3 sub-triangles.
//
// Parameters:
//		a, b, c		The three vertices of the triangle
//
// Returns:
//		N/A
// --------------------------------------------------------------------------
void Rasterizer :: RasterTriangle(const RasterPos& a, const RasterPos& b,
								  const RasterPos& c) {

	const RasterPos * pos[3];
	pos[0] = &a;
	pos[1] = &b;
	pos[2] = &c;

	// sort by y
	I8 * permutation = SortPermutation(a.m_WindowCoords.y, b.m_WindowCoords.y, c.m_WindowCoords.y);
	const RasterPos &pos1 = *pos[permutation[0]];
	const RasterPos &pos2 = *pos[permutation[1]];
	const RasterPos &pos3 = *pos[permutation[2]];

	EdgePos start, end;
	start.m_WindowCoords.x = end.m_WindowCoords.x = pos1.m_WindowCoords.x;
	start.m_WindowCoords.w = end.m_WindowCoords.w = pos1.m_WindowCoords.w;
	start.m_Color = end.m_Color = pos1.m_Color;
	start.m_TextureCoords = end.m_TextureCoords = pos1.m_TextureCoords;

	// set up the triangle
	// init start, end, deltas
	EGL_Fixed invDeltaY2 = EGL_Inverse(pos2.m_WindowCoords.y - pos1.m_WindowCoords.y);

	EGL_Fixed incX2 = EGL_Mul(pos2.m_WindowCoords.x - pos1.m_WindowCoords.x, invDeltaY2);
	EGL_Fixed incR2 = EGL_Mul(pos2.m_Color.r - pos1.m_Color.r, invDeltaY2);
	EGL_Fixed incG2 = EGL_Mul(pos2.m_Color.g - pos1.m_Color.g, invDeltaY2);
	EGL_Fixed incB2 = EGL_Mul(pos2.m_Color.b - pos1.m_Color.b, invDeltaY2);
	EGL_Fixed incA2 = EGL_Mul(pos2.m_Color.a - pos1.m_Color.a, invDeltaY2);

	// initially, use linear interpolation, change to perspective later:
	EGL_Fixed incW2 = EGL_Mul(pos2.m_WindowCoords.w - pos1.m_WindowCoords.w, invDeltaY2);
	EGL_Fixed incTu2 = EGL_Mul(pos2.m_TextureCoords.tu - pos1.m_TextureCoords.tu, invDeltaY2);
	EGL_Fixed incTv2 = EGL_Mul(pos2.m_TextureCoords.tv - pos1.m_TextureCoords.tv, invDeltaY2);

	EGL_Fixed invDeltaY3 = EGL_Inverse(pos3.m_WindowCoords.y - pos1.m_WindowCoords.y);

	EGL_Fixed incX3 = EGL_Mul(pos3.m_WindowCoords.x - pos1.m_WindowCoords.x, invDeltaY3);
	EGL_Fixed incR3 = EGL_Mul(pos3.m_Color.r - pos1.m_Color.r, invDeltaY3);
	EGL_Fixed incG3 = EGL_Mul(pos3.m_Color.g - pos1.m_Color.g, invDeltaY3);
	EGL_Fixed incB3 = EGL_Mul(pos3.m_Color.b - pos1.m_Color.b, invDeltaY3);
	EGL_Fixed incA3 = EGL_Mul(pos3.m_Color.a - pos1.m_Color.a, invDeltaY3);

	// initially, use linear interpolation, change to perspective later:
	EGL_Fixed incW3 = EGL_Mul(pos3.m_WindowCoords.w - pos1.m_WindowCoords.w, invDeltaY3);
	EGL_Fixed incTu3 = EGL_Mul(pos3.m_TextureCoords.tu - pos1.m_TextureCoords.tu, invDeltaY3);
	EGL_Fixed incTv3 = EGL_Mul(pos3.m_TextureCoords.tv - pos1.m_TextureCoords.tv, invDeltaY3);

	EGL_Fixed invDeltaY23 = EGL_Inverse(pos3.m_WindowCoords.y - pos2.m_WindowCoords.y);

	EGL_Fixed incX23 = EGL_Mul(pos3.m_WindowCoords.x - pos2.m_WindowCoords.x, invDeltaY23);
	EGL_Fixed incR23 = EGL_Mul(pos3.m_Color.r - pos2.m_Color.r, invDeltaY23);
	EGL_Fixed incG23 = EGL_Mul(pos3.m_Color.g - pos2.m_Color.g, invDeltaY23);
	EGL_Fixed incB23 = EGL_Mul(pos3.m_Color.b - pos2.m_Color.b, invDeltaY23);
	EGL_Fixed incA23 = EGL_Mul(pos3.m_Color.a - pos2.m_Color.a, invDeltaY23);

	// initially, use linear interpolation, change to perspective later:
	EGL_Fixed incW23 = EGL_Mul(pos3.m_WindowCoords.w - pos2.m_WindowCoords.w, invDeltaY23);
	EGL_Fixed incTu23 = EGL_Mul(pos3.m_TextureCoords.tu - pos2.m_TextureCoords.tu, invDeltaY23);
	EGL_Fixed incTv23 = EGL_Mul(pos3.m_TextureCoords.tv - pos2.m_TextureCoords.tv, invDeltaY23);

	I32 yStart = EGL_IntFromFixed(pos1.m_WindowCoords.y);
	I32 yEnd = EGL_IntFromFixed(pos2.m_WindowCoords.y);
	I32 y;

	if (incX2 < incX3) {
		for (y = yStart; y < yEnd; ++y) {
			RasterScanLine(start, end, y);

			// update start
			start.m_WindowCoords.x += incX2;
			start.m_Color.r += incR2;
			start.m_Color.g += incG2;
			start.m_Color.b += incB2;
			start.m_Color.a += incA2;

			start.m_WindowCoords.w += incW2;
			start.m_TextureCoords.tu += incTu2;
			start.m_TextureCoords.tv += incTv2;

			// update end
			end.m_WindowCoords.x += incX3;
			end.m_Color.r += incR3;
			end.m_Color.g += incG3;
			end.m_Color.b += incB3;
			end.m_Color.a += incA3;

			end.m_WindowCoords.w += incW3;
			end.m_TextureCoords.tu += incTu3;
			end.m_TextureCoords.tv += incTv3;

		}

		yEnd = EGL_IntFromFixed(pos3.m_WindowCoords.y);

		start.m_WindowCoords.x = pos2.m_WindowCoords.x;
		start.m_WindowCoords.w = pos2.m_WindowCoords.w;
		start.m_Color = pos2.m_Color;
		start.m_TextureCoords = pos2.m_TextureCoords;

		for (; y < yEnd; ++y) {
			RasterScanLine(start, end, y);
			// update start
			start.m_WindowCoords.x += incX23;
			start.m_Color.r += incR23;
			start.m_Color.g += incG23;
			start.m_Color.b += incB23;
			start.m_Color.a += incA23;

			start.m_WindowCoords.w += incW23;
			start.m_TextureCoords.tu += incTu23;
			start.m_TextureCoords.tv += incTv23;

			// update end
			end.m_WindowCoords.x += incX3;
			end.m_Color.r += incR3;
			end.m_Color.g += incG3;
			end.m_Color.b += incB3;
			end.m_Color.a += incA3;

			end.m_WindowCoords.w += incW3;
			end.m_TextureCoords.tu += incTu3;
			end.m_TextureCoords.tv += incTv3;

		}
	} else {
		for (y = yStart; y < yEnd; ++y) {
			RasterScanLine(start, end, y);

			// update start
			start.m_WindowCoords.x += incX3;
			start.m_Color.r += incR3;
			start.m_Color.g += incG3;
			start.m_Color.b += incB3;
			start.m_Color.a += incA3;

			start.m_WindowCoords.w += incW3;
			start.m_TextureCoords.tu += incTu3;
			start.m_TextureCoords.tv += incTv3;

			// update end
			end.m_WindowCoords.x += incX2;
			end.m_Color.r += incR2;
			end.m_Color.g += incG2;
			end.m_Color.b += incB2;
			end.m_Color.a += incA2;

			end.m_WindowCoords.w += incW2;
			end.m_TextureCoords.tu += incTu2;
			end.m_TextureCoords.tv += incTv2;

		}

		yEnd = EGL_IntFromFixed(pos3.m_WindowCoords.y);

		end.m_WindowCoords.x = pos2.m_WindowCoords.x;
		end.m_WindowCoords.w = pos2.m_WindowCoords.w;
		end.m_Color = pos2.m_Color;
		end.m_TextureCoords = pos2.m_TextureCoords;

		for (; y < yEnd; ++y) {
			RasterScanLine(start, end, y);
			// update start
			start.m_WindowCoords.x += incX3;
			start.m_Color.r += incR3;
			start.m_Color.g += incG3;
			start.m_Color.b += incB3;
			start.m_Color.a += incA3;

			start.m_WindowCoords.w += incW3;
			start.m_TextureCoords.tu += incTu3;
			start.m_TextureCoords.tv += incTv3;

			// update end
			end.m_WindowCoords.x += incX23;
			end.m_Color.r += incR23;
			end.m_Color.g += incG23;
			end.m_Color.b += incB23;
			end.m_Color.a += incA23;

			end.m_WindowCoords.w += incW23;
			end.m_TextureCoords.tu += incTu23;
			end.m_TextureCoords.tv += incTv23;

		}
	}
}