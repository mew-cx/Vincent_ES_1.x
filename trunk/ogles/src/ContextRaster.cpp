// ==========================================================================
//
// raster.cpp	Triangle Rasterization Functions
//
//				Triangle rasterization code based on the Intel Graphics
//				Performance Primitive Library for PXA25x Processor
//
// --------------------------------------------------------------------------
//
// 08-02-2003	Hans-Martin Will	initial version
//
// ==========================================================================


#include "stdafx.h"
#include "context.h"
#include "surface.h"
#include <string.h>
#include "fixed.h"


#if 0

// --------------------------------------------------------------------------
// Interpolate along the ray specified through the two raster positions
// p0 and p1. Evaluate the interpolation for the given x coordinate
// and store the result into the specified location.
//
// Parameters:
//		p0			The first vertex on the ray
//		p1			The second vertex on the ray
//		x			The x-coordinate at which the interpolation should be
//					evaluated
//		result		The location into which the result should be stored
//
// Returns:
//		N/A
//
// --------------------------------------------------------------------------
void Context :: InterpolateRasterPos(RasterPos * p0, RasterPos * p1, 
									 GLfixed x, RasterPos * result) {

	I32 w1x1, w0x0, w0_w1x, denominator;

	gppMul_16_32s(p1->m_WindowsCoords.w, p1->m_WindowsCoords.x, &w1x1);
	gppMul_16_32s(p0->m_WindowsCoords.w, p0->m_WindowsCoords.x, &w0x0);

	gppMul_16_32s(p0->m_WindowsCoords.w - p1->m_WindowsCoords.w, x, &w0_w1x);

	denominator = w1x1 - w0x0 + w0_w1x;

	if (denominator == 0) {
		*result = *p0;
		return;
	}

	I32 invDenominator;
	gppInv_16_32s(denominator, &invDenominator);

	// store x
	result->m_WindowsCoords.x = x;

	I32 invDeltaX;
	gppInv_16_32s(p1->m_WindowsCoords.x - p0->m_WindowsCoords.x, &invDeltaX);

	// compute w
	I32 w0w1;
	gppMul_16_32s(p0->m_WindowsCoords.w, p1->m_WindowsCoords.w, &w0w1);

	I32 w0w1x1_x0;
	gppMul_16_32s(w0w1, p1->m_WindowsCoords.x - p0->m_WindowsCoords.x, &w0w1x1_x0);

	gppMul_16_32s(invDenominator, w0w1x1_x0, &result->m_WindowsCoords.w);

	// Linear Interpolation
	I32 a0x1, a1x0, a1_a0x;

#define LINEAR_INTERP(FIELD) \
	gppMul_16_32s(p0->FIELD, (p1->m_WindowsCoords.x) >> 16, &a0x1); \
	gppMul_16_32s(p1->FIELD, (p0->m_WindowsCoords.x) >> 16, &a1x0); \
	gppMul_16_32s(p1->FIELD - p0->FIELD, x >> 16, &a1_a0x); \
	result->FIELD = invDeltaX * (a0x1 - a1x0 + a1_a0x);

	// Perpsective Interpolation
	I32 a0w1x1, a1w0x0, a1w0, a0w1, a1w1_a0w1y;


#define PERPECTIVE_INTERP(FIELD) \
	gppMul_16_32s(p0->FIELD, w1x1, &a0w1x1); \
	gppMul_16_32s(p1->FIELD, w0x0, &a1w0x0); \
	gppMul_16_32s(p0->m_WindowsCoords.w, p1->FIELD, &a1w0); \
	gppMul_16_32s(p1->m_WindowsCoords.w, p0->FIELD, &a0w1); \
	gppMul_16_32s(a1w0 - a0w1, x, &a1w1_a0w1y); \
	gppMul_16_32s(invDenominator, a0w1x1 - a1w0x0 + a1w1_a0w1y, reinterpret_cast<I32*>(&result->FIELD));

	// Compute interpolated values
	LINEAR_INTERP(m_WindowsCoords.y);
	PERPECTIVE_INTERP(m_WindowsCoords.z);
	PERPECTIVE_INTERP(m_TextureCoords.tu);
	PERPECTIVE_INTERP(m_TextureCoords.tv);

	LINEAR_INTERP(m_Color.a);
	LINEAR_INTERP(m_Color.r);
	LINEAR_INTERP(m_Color.g);
	LINEAR_INTERP(m_Color.b);
}



// Have frame to provide x, y, z, w coordinates
//
// Interpolate r, g, b, a, tu, tv
// Interpolation can be linear or perspective
// r, g, b, a can be linear
// tu, tv and depth (w) should be with perspective correction
//
// For linear interpolation, we have a = a0 + y * a1, which can be evaluated
// within the edge buffer by incrementing with a1
//
// For perspective interpolation, we have a = (b0 + y * b1) / (c0 + y * c1),
// so we need additional variables to store these coefficients
//
// For linear interpolation, we have:
// - setup
// - inc
// - inc * factor for clipping jump
//
// For perpsective interpolation, we have:
// - setup
// - value at
// - <nothing> for clipping jump
//

struct PerspectiveInterpolator {

public:
	// y0 and y1 are integers, not fixed point numbers
	inline PerspectiveInterpolator(I32 a0, I32 a1, I32 w0, I32 w1,
									I32 y0, I32 y1) {
		I32 t0, t1;
		b0 = w1 * y1 - w0 *y0;
		b1 = w0 - w1;
		gppMul_16_32s(w0, a1, &t0);
		gppMul_16_32s(w1, a0, &t1);
		c0 = t1 * y1 - t0 * y0;
		c1 = t0 - t1;
	}

	// y is an integer, not a fixed point number
	inline I32 GetValueAt(I32 y) {
		I32 result;
		gppDiv_16_32s(b0 + y * b1, c0 + y * c1, &result);
		return result;
	}

private:
	I32 b0, b1, c0, c1;
};


struct LinearInterpolator {

public:
	inline LinearInterpolator(I32 a0, I32 a1, I32 y0, I32 y1) {
		b0 = a0;
		gppDiv_16_32s(a1 - a0, y1 - y0, &b1);
	}

	inline LinearInterpolator(I32 a0, I32 a1, I32 inv_delta_y) {
		b0 = a0;
		gppMul_16_32s(a1 - a0, inv_delta_y, &b1);
	}

	// y is an integer, not a fixed point number
	inline I32 GetValueAt(I32 y) {
		return b0 + y * b1;
	}

	inline I32 GetIncrement() {
		return b1;
	}

private:
	I32 b0, b1;
};


struct EdgeInterpolator {

public:
	inline EdgeInterpolator(RasterPos * p1, RasterPos * p2, I32 invDelta):
		x (p1->m_WindowsCoords.x, p2->m_WindowsCoords.x, invDelta),
		z (p1->m_WindowsCoords.z, p2->m_WindowsCoords.z, 
		   p1->m_WindowsCoords.w, p2->m_WindowsCoords.w,
		   gppRound_Fixed_16_To_Int(p1->m_WindowsCoords.y),
		   gppRound_Fixed_16_To_Int(p2->m_WindowsCoords.y)),
		r (p1->m_Color.r, p2->m_Color.r, invDelta),
		g (p1->m_Color.g, p2->m_Color.g, invDelta),
		b (p1->m_Color.b, p2->m_Color.b, invDelta),
		a (p1->m_Color.a, p2->m_Color.a, invDelta),
		tu (p1->m_TextureCoords.tu, p2->m_TextureCoords.tu, invDelta),
		tv (p1->m_TextureCoords.tv, p2->m_TextureCoords.tv, invDelta)
//		tu (p1->m_TextureCoords.tu, p2->m_TextureCoords.tu, 
//			p1->m_WindowsCoords.w, p2->m_WindowsCoords.w,
//		    gppRound_Fixed_16_To_Int(p1->m_WindowsCoords.y),
//		    gppRound_Fixed_16_To_Int(p2->m_WindowsCoords.y)),
//		tv (p1->m_TextureCoords.tv, p2->m_TextureCoords.tv, 
//		    p1->m_WindowsCoords.w, p2->m_WindowsCoords.w,
//		    gppRound_Fixed_16_To_Int(p1->m_WindowsCoords.y),
//		    gppRound_Fixed_16_To_Int(p2->m_WindowsCoords.y))
	{
	}

	inline I32 GetXIncrement() {
		return x.GetIncrement();
	}

public:
	LinearInterpolator			x;
	PerspectiveInterpolator		z;
	LinearInterpolator			r; 
	LinearInterpolator			g; 
	LinearInterpolator			b; 
	LinearInterpolator			a; 
	//PerspectiveInterpolator		tu;
	//PerspectiveInterpolator		tv;
	LinearInterpolator		tu;
	LinearInterpolator		tv;
};

struct EdgeBuffer {
public:

	inline void SetToVertex(RasterPos * p) {
		m_Buffer.x = p->m_WindowsCoords.x;
		m_Buffer.w = p->m_WindowsCoords.z;
		m_Buffer.r = p->m_Color.r;
		m_Buffer.g = p->m_Color.g;
		m_Buffer.b = p->m_Color.b;
		m_Buffer.a = p->m_Color.a;
		m_Buffer.tu = p->m_TextureCoords.tu;
		m_Buffer.tv = p->m_TextureCoords.tv;
	}

	inline void SetToInterpolationAt(EdgeInterpolator & interpolator, I32 at, I32 maxX) {
		m_Buffer.x = EGL_CLAMP(interpolator.x.GetValueAt(at), 0, maxX);

		m_Buffer.w = interpolator.z.GetValueAt(at);
		m_Buffer.r = EGL_CLAMP(interpolator.r.GetValueAt(at), 0, 0xffffff);
		m_Buffer.g = EGL_CLAMP(interpolator.g.GetValueAt(at), 0, 0xffffff);
		m_Buffer.b = EGL_CLAMP(interpolator.b.GetValueAt(at), 0, 0xffffff);
		m_Buffer.a = EGL_CLAMP(interpolator.a.GetValueAt(at), 0, 0xffffff);
		m_Buffer.tu = interpolator.tu.GetValueAt(at);
		m_Buffer.tv = interpolator.tv.GetValueAt(at);
	}

	inline void IncrementWithInterpolatorAt(EdgeInterpolator & interpolator, I32 at, I32 maxX) {
		m_Buffer.x = EGL_CLAMP(m_Buffer.x + interpolator.x.GetIncrement(), 0, maxX);
		m_Buffer.w = interpolator.z.GetValueAt(at);
		m_Buffer.r = EGL_CLAMP((I32) m_Buffer.r + interpolator.r.GetIncrement(), 0, 0xffffff);
		m_Buffer.g = EGL_CLAMP((I32) m_Buffer.g + interpolator.g.GetIncrement(), 0, 0xffffff);
		m_Buffer.b = EGL_CLAMP((I32) m_Buffer.b + interpolator.b.GetIncrement(), 0, 0xffffff);
		m_Buffer.a = EGL_CLAMP((I32) m_Buffer.a + interpolator.a.GetIncrement(), 0, 0xffffff);
		//m_Buffer.tu = interpolator.tu.GetValueAt(at);
		//m_Buffer.tv = interpolator.tv.GetValueAt(at);
		m_Buffer.tu += interpolator.tu.GetIncrement();
		m_Buffer.tv += interpolator.tv.GetIncrement();
	}

	inline GPP_TLVERTEX_V2F_C4F_T2F * GetBuffer() {
		return &m_Buffer;
	}

private:
	GPP_TLVERTEX_V2F_C4F_T2F	m_Buffer;
};

namespace {
	inline I32 Inverse(I32 value) {
		I32 result;

		gppInv_16_32s(value, &result);

		return result;
	}
}


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
// Prepare the internal state variables of the rasterizer for rendering
// of triangles.
// --------------------------------------------------------------------------
void Context :: PrepareRenderTriangle() {

	// ----------------------------------------------------------------------
	// Select the appropriate scan line rasterization function
	// ----------------------------------------------------------------------

	// Lookup table for appropriate scan line rasterization function
	// Bit 0: SMOOTH (true), FLAT (false)
	// Bit 1: Depth Test Enabled
	// Bit 2: Texturing Enabled
	static ScanLineFunction scanLineFunctions[] = {
		gppScanLine_F_Zal_16_32s,		// FLAT, no depth, no texture
		gppScanLine_G_Zal_16_32s,		// SMOOTH, no depth, no texture
		gppScanLine_F_Zlt_16_32s,		// FLAT, depth, no texture
		gppScanLine_G_Zlt_16_32s,		// SMOOTH, depth, no texture
		gppScanLine_Zal_Tdw_16_32s,		// FLAT, no depth, texture
		gppScanLine_G_Zal_Tmw_16_32s,	// SMOOTH, no depth, texture
		gppScanLine_Zlt_Tdw_16_32s,		// FLAT, depth, texture
		gppScanLine_G_Zlt_Tmw_16_32s	// SMOOTH, depth, texture
	};

	int index = (m_ShadingModel == GL_SMOOTH) |
		m_DepthTestEnabled << 1 |
		m_Texture2DEnabled << 2;


	m_ScanLineFunction = scanLineFunctions[index];

	// ----------------------------------------------------------------------
	// Precompute lightning colors
	// ----------------------------------------------------------------------

	for (int light = 0, mask = 1; light < EGL_NUMBER_LIGHTS; ++light, mask <<= 1) {
		if (mask & m_LightEnabled) {
			m_Lights[light].InitWithMaterial(m_FrontMaterial);
		}
	}


	// ----------------------------------------------------------------------
	// TO DO: Load texture data into m_RasterParameters.
	// ----------------------------------------------------------------------
}


// ---------------------------------------------------------------------------
// Render the triangle specified by the three transformed and lit vertices
// passed as arguments. Before calling into the actual rasterization, the
// triangle will be subject to the scissor test, which may subdivide it
// into up to 3 sub-triangles.
//
// Parameters:
//		pos0, pos1, pos2	The three vertices of the triangle
//
// Returns:
//		N/A
// --------------------------------------------------------------------------
void Context :: RenderTriangle(RasterPos * pos0, RasterPos * pos1, 
							   RasterPos * pos2) {

	// ----------------------------------------------------------------------
	// First, quick check if the triangle is fully cut away in y-direction
	// If so, return immediately
	// ----------------------------------------------------------------------

	int alpha = 
		Greater(m_MinY, pos0->m_WindowsCoords.y) +
		Greater(m_MinY, pos1->m_WindowsCoords.y) +
		Greater(m_MinY, pos2->m_WindowsCoords.y);

	int beta = 
		Greater(pos0->m_WindowsCoords.y, m_MaxY) +
		Greater(pos1->m_WindowsCoords.y, m_MaxY) +
		Greater(pos2->m_WindowsCoords.y, m_MaxY);

	if (alpha == 3 || beta == 3) {
		return;
	}

	// ----------------------------------------------------------------------
	// In case of flat shading, grab the color from the right vertex before
	// we perform any further sub-division or permutation of the vertices
	// ----------------------------------------------------------------------

	m_RasterParameters.m_rgba =
		EGL_MAP_BITFIELD(m_CurrentRGBA.r, 8) << 24 |
		EGL_MAP_BITFIELD(m_CurrentRGBA.g, 8) << 16 |
		EGL_MAP_BITFIELD(m_CurrentRGBA.b, 8) <<  8 |
		EGL_MAP_BITFIELD(m_CurrentRGBA.a, 8);


	// ----------------------------------------------------------------------
	// Sort the vertices by x-coordinate in preparation of clipping in 
	// x-direction
	// ----------------------------------------------------------------------

	RasterPos * pos[3];
	pos[0] = pos0;
	pos[1] = pos1;
	pos[2] = pos2;

	I8 * permutation = SortPermutation(pos0->m_WindowsCoords.x, 
		pos1->m_WindowsCoords.x, pos2->m_WindowsCoords.x);

	pos0 = pos[permutation[0]];
	pos1 = pos[permutation[1]];
	pos2 = pos[permutation[2]];

	// ----------------------------------------------------------------------
	// apply the scissor test to the triangle
	// ----------------------------------------------------------------------

	alpha = 
		Greater(m_MinX, pos0->m_WindowsCoords.x) +
		Greater(m_MinX, pos1->m_WindowsCoords.x) +
		Greater(m_MinX, pos2->m_WindowsCoords.x);

	beta = 
		Greater(m_MaxX, pos0->m_WindowsCoords.x) +
		Greater(m_MaxX, pos1->m_WindowsCoords.x) +
		Greater(m_MaxX, pos2->m_WindowsCoords.x);

	switch (alpha * 4 + beta) {
	case 0: // configuration A
		return;

	case 1: // configuration E
		{
			RasterPos pos3, pos4;
			InterpolateRasterPos(pos0, pos1, m_MaxX, &pos3);
			InterpolateRasterPos(pos0, pos2, m_MaxX, &pos4);
			RenderClippedXTriangle(pos0, &pos3, &pos4);
		}

		break;

	case 2: // configuration D
		{
			RasterPos pos3, pos4;
			InterpolateRasterPos(pos0, pos2, m_MaxX, &pos3);
			InterpolateRasterPos(pos1, pos2, m_MaxX, &pos4);
			RenderClippedXTriangle(pos0, &pos3, pos1);
			RenderClippedXTriangle(pos1, &pos3, &pos4);
		}

		break;

	case 3: // configuration C
		// leave triangle as is
		RenderClippedXTriangle(pos0, pos1, pos2);

		break;

	case 5: // configuration J
		{
			RasterPos pos3, pos4, pos5, pos6;
			InterpolateRasterPos(pos0, pos2, m_MinX, &pos3);
			InterpolateRasterPos(pos0, pos2, m_MaxX, &pos4);
			InterpolateRasterPos(pos0, pos1, m_MinX, &pos5);
			InterpolateRasterPos(pos0, pos1, m_MaxX, &pos6);
			RenderClippedXTriangle(&pos3, &pos4, &pos6);
			RenderClippedXTriangle(&pos3, &pos6, &pos5);
		}

		break;

	case 6: // configuration I
		{
			RasterPos pos3, pos4, pos5, pos6;
			InterpolateRasterPos(pos0, pos2, m_MinX, &pos3);
			InterpolateRasterPos(pos0, pos2, m_MaxX, &pos4);
			InterpolateRasterPos(pos0, pos1, m_MinX, &pos5);
			InterpolateRasterPos(pos1, pos2, m_MaxX, &pos6);
			RenderClippedXTriangle(&pos3, &pos4, &pos5);
			RenderClippedXTriangle(&pos5, &pos4, &pos6);
			RenderClippedXTriangle(&pos5, &pos6, pos1);
		}

		break;

	case 7: // configuration H
		{
			RasterPos pos3, pos4;
			InterpolateRasterPos(pos0, pos1, m_MinX, &pos3);
			InterpolateRasterPos(pos0, pos2, m_MinX, &pos4);
			RenderClippedXTriangle(&pos3, &pos4, pos1);
			RenderClippedXTriangle(&pos4, pos2, pos1);
		}

		break;

	case 10: // configuration F
		{
			RasterPos pos3, pos4, pos5, pos6;
			InterpolateRasterPos(pos0, pos2, m_MinX, &pos3);
			InterpolateRasterPos(pos0, pos2, m_MaxX, &pos4);
			InterpolateRasterPos(pos1, pos2, m_MinX, &pos5);
			InterpolateRasterPos(pos1, pos2, m_MaxX, &pos6);
			RenderClippedXTriangle(&pos3, &pos4, &pos6);
			RenderClippedXTriangle(&pos3, &pos6, &pos5);
		}

		break;

	case 11: // configuration G
		{
			RasterPos pos3, pos4;
			InterpolateRasterPos(pos1, pos2, m_MinX, &pos3);
			InterpolateRasterPos(pos0, pos2, m_MinX, &pos4);
			RenderClippedXTriangle(pos2, &pos3, &pos4);
		}

		break;


	case 15: // configuration B
		return;

	default:
		return; 
	}
}


void Context :: RenderClippedXTriangle(RasterPos * pos1, RasterPos * pos2, RasterPos * pos3) {

	RasterPos * pos[3];
	pos[0] = pos1;
	pos[1] = pos2;
	pos[2] = pos3;

	// sort by y
	I8 * permutation = SortPermutation(pos1->m_WindowsCoords.y, pos2->m_WindowsCoords.y, pos3->m_WindowsCoords.y);
	pos1 = pos[permutation[0]];
	pos2 = pos[permutation[1]];
	pos3 = pos[permutation[2]];

	// optimize memory layout!!!
	//raster_params.m_texture[0] = textureParams;

	// here we go

	EdgeInterpolator p1p3(pos1, pos3, Inverse(pos3->m_WindowsCoords.y - pos1->m_WindowsCoords.y));
	EdgeInterpolator p1p2(pos1, pos2, Inverse(pos2->m_WindowsCoords.y - pos1->m_WindowsCoords.y));
	EdgeInterpolator p2p3(pos2, pos3, Inverse(pos3->m_WindowsCoords.y - pos2->m_WindowsCoords.y));

	I32 y3 = gppFixed_16_To_Int(pos3->m_WindowsCoords.y);
	I32 y2 = gppFixed_16_To_Int(pos2->m_WindowsCoords.y);
	I32 y1 = gppFixed_16_To_Int(pos1->m_WindowsCoords.y);
	I32 yMin = gppFixed_16_To_Int(m_MinY);
	I32 yMax = gppFixed_16_To_Int((m_MaxY + 1));

	// clip on high end of scanning
	if (yMax <= y2) {
		y2 = y3 = yMax - 1;
	} else if (yMax <= y3) {
		y3 = yMax - 1;
	}

	I32 y;

	EdgeBuffer edge_buffer_min;
	EdgeBuffer edge_buffer_max;

	// adjust raster parameter pointers to scan line address
	U32 width = m_DrawSurface->Width();
	
	if (p1p2.GetXIncrement() < p1p3.GetXIncrement()) {
		// CCW orientation of triangle

		if (yMin > y2) {

			y = yMin;
			edge_buffer_min.SetToInterpolationAt(p2p3, y, gppInt_To_Fixed_16(width) - 1);
			edge_buffer_max.SetToInterpolationAt(p1p3, y, gppInt_To_Fixed_16(width) - 1);
			I32 scan_line_offset = y * width;

			m_RasterParameters.m_pFrameBuf = m_DrawSurface->GetColorBuffer() + scan_line_offset;
			m_RasterParameters.m_pZBuffer = m_DrawSurface->GetDepthBuffer() + scan_line_offset;
			goto renderp2p3l;

		} else if (yMin > y1) {

			y = yMin;
			edge_buffer_min.SetToInterpolationAt(p1p2, y, gppInt_To_Fixed_16(width) - 1);
			edge_buffer_max.SetToInterpolationAt(p1p3, y, gppInt_To_Fixed_16(width) - 1);
			I32 scan_line_offset = y * width;

			m_RasterParameters.m_pFrameBuf = m_DrawSurface->GetColorBuffer() + scan_line_offset;
			m_RasterParameters.m_pZBuffer = m_DrawSurface->GetDepthBuffer() + scan_line_offset;

		} else {

			y = y1;
			edge_buffer_min.SetToVertex(pos1);
			edge_buffer_max.SetToVertex(pos1);
			I32 scan_line_offset = y * width;

			m_RasterParameters.m_pFrameBuf = m_DrawSurface->GetColorBuffer() + scan_line_offset;
			m_RasterParameters.m_pZBuffer = m_DrawSurface->GetDepthBuffer() + scan_line_offset;
		}

		for (; y <= y2; ) {
			++y;

			edge_buffer_min.IncrementWithInterpolatorAt(p1p2, y, gppInt_To_Fixed_16(width) - 1);			
			m_ScanLineFunction(edge_buffer_min.GetBuffer(), edge_buffer_max.GetBuffer(), &m_RasterParameters);
			edge_buffer_max.IncrementWithInterpolatorAt(p1p3, y, gppInt_To_Fixed_16(width) - 1);
			
			m_RasterParameters.m_pFrameBuf += width;
			m_RasterParameters.m_pZBuffer += width;
		}

		edge_buffer_min.SetToVertex(pos2);

renderp2p3l:
		while (y < y3) {
			++y;

			m_ScanLineFunction(edge_buffer_min.GetBuffer(), edge_buffer_max.GetBuffer(), &m_RasterParameters);
			edge_buffer_min.IncrementWithInterpolatorAt(p2p3, y, gppInt_To_Fixed_16(width) - 1);			
			edge_buffer_max.IncrementWithInterpolatorAt(p1p3, y, gppInt_To_Fixed_16(width) - 1);
			
			m_RasterParameters.m_pFrameBuf += width;
			m_RasterParameters.m_pZBuffer += width;
		}
	} else {
		// CW orientation of triangle

		if (yMin > y2) {

			y = yMin;
			edge_buffer_min.SetToInterpolationAt(p1p3, y, gppInt_To_Fixed_16(width) - 1);
			edge_buffer_max.SetToInterpolationAt(p2p3, y, gppInt_To_Fixed_16(width) - 1);
			I32 scan_line_offset = y * width;

			m_RasterParameters.m_pFrameBuf = m_DrawSurface->GetColorBuffer() + scan_line_offset;
			m_RasterParameters.m_pZBuffer = m_DrawSurface->GetDepthBuffer() + scan_line_offset;
			goto renderp2p3r;

		} else if (yMin > y1) {

			y = yMin;
			edge_buffer_min.SetToInterpolationAt(p1p3, y, gppInt_To_Fixed_16(width) - 1);
			edge_buffer_max.SetToInterpolationAt(p1p2, y, gppInt_To_Fixed_16(width) - 1);
			I32 scan_line_offset = y * width;

			m_RasterParameters.m_pFrameBuf = m_DrawSurface->GetColorBuffer() + scan_line_offset;
			m_RasterParameters.m_pZBuffer = m_DrawSurface->GetDepthBuffer() + scan_line_offset;

		} else {

			y = y1;
			edge_buffer_min.SetToVertex(pos1);
			edge_buffer_max.SetToVertex(pos1);
			I32 scan_line_offset = y * width;

			m_RasterParameters.m_pFrameBuf = m_DrawSurface->GetColorBuffer() + scan_line_offset;
			m_RasterParameters.m_pZBuffer = m_DrawSurface->GetDepthBuffer() + scan_line_offset;
		}

		for (; y <= y2; ) {
			++y;

			edge_buffer_min.IncrementWithInterpolatorAt(p1p3, y, gppInt_To_Fixed_16(width) - 1);
			m_ScanLineFunction(edge_buffer_min.GetBuffer(), edge_buffer_max.GetBuffer(), &m_RasterParameters);
			edge_buffer_max.IncrementWithInterpolatorAt(p1p2, y, gppInt_To_Fixed_16(width) - 1);
			
			m_RasterParameters.m_pFrameBuf += width;
			m_RasterParameters.m_pZBuffer += width;
		}

		edge_buffer_max.SetToVertex(pos2);

renderp2p3r:
		while (y < y3) {
			++y;

			m_ScanLineFunction(edge_buffer_min.GetBuffer(), edge_buffer_max.GetBuffer(), &m_RasterParameters);
			edge_buffer_min.IncrementWithInterpolatorAt(p1p3, y, gppInt_To_Fixed_16(width) - 1);			
			edge_buffer_max.IncrementWithInterpolatorAt(p2p3, y, gppInt_To_Fixed_16(width) - 1);
			
			m_RasterParameters.m_pFrameBuf += width;
			m_RasterParameters.m_pZBuffer += width;
		}
	}

}

#endif