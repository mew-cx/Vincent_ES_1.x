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


using namespace EGL;


Rasterizer :: Rasterizer():
		m_IsPrepared(false)
{
}


Rasterizer :: ~Rasterizer() {
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


void Rasterizer :: RasterTriangle(const RasterPos& a, const RasterPos& b,
									  const RasterPos& c) {
}


#if 0 
namespace {
	inline GLint Min(GLint a, GLint b) {
		return a < b ? a : b;
	}

	inline GLint Max(GLint a, GLint b) {
		return a > b ? a : b;
	}
}


void Context :: UpdateWindowClipping(void) {

	if (m_ScissorTestEnabled) {

		GLint minX = Max(m_ViewportX, m_ScissorX);
		GLint minY = Max(m_ViewportY, m_ScissorY);
		GLint maxX = Min(m_ViewportX + m_ViewportWidth, m_ScissorX + m_ScissorWidth);
		GLint maxY = Min(m_ViewportY + m_ViewportHeight, m_ScissorY + m_ScissorHeight);

		m_MinX = EGL_FixedFromInt(minX);
		m_MinY = EGL_FixedFromInt(minY);
		m_MaxX = EGL_FixedFromInt(maxX) - 1;
		m_MaxY = EGL_FixedFromInt(maxY) - 1;
	} else {
		// just use viewport dimensions
		m_MinX = EGL_FixedFromInt(m_ViewportX);
		m_MinY = EGL_FixedFromInt(m_ViewportY);
		m_MaxX = EGL_FixedFromInt((m_ViewportX + m_ViewportWidth)) - 1;
		m_MaxY = EGL_FixedFromInt((m_ViewportY + m_ViewportHeight)) - 1;
	}
}


#endif



#if 0
	//
	// THIS SECTION IS ONLY TEMPORARILY HERE UNTIL WE REFACTORED THE RASTERIZER
	//


	#ifdef EGL_USE_GPP
		typedef GPP_TLVERTEX_V2F_C4F_T2F EGL_TLVERTEX_V2F_C4F_T2F;
		typedef GPP_TEXTURE_PARAMS EGL_TEXTURE_PARAMS;
		typedef GPP_RASTER_PARAMS EGL_RASTER_PARAMS;
		typedef GppStatus EglStatus;
	#else
	#define EGL_MAX_TEXTURES 1

	typedef struct{
		I32		x, w;
		U32		r, g, b, a;
		U32		tu, tv;
	} EGL_TLVERTEX_V2F_C4F_T2F;

	typedef struct{
		U16*	m_pTexBuffer;					//RGB565 Format
		U16		m_Height;
		U16		m_Width;
	} EGL_TEXTURE_PARAMS;

	typedef struct{
		U16*	m_pFrameBuf;					//RGB565 Format
		I32*	m_pZBuffer;
		U32		m_rgba;							//RGBA8888 Format
		EGL_TEXTURE_PARAMS m_texture[EGL_MAX_TEXTURES];
	} EGL_RASTER_PARAMS;

	typedef enum {

		gppStsNoErr			=  0,				// OK MODE

		gppStsNullPtrErr    = -5,				// ERROR MODES
		gppStsBadArgErr     = -4,
		gppStsDivByZeroErr	= -3,
		gppStsOverFlowErr   = -2,
		gppStsUnderFlowErr  = -1,
		
	} EglStatus;								// NOTE: Error codes are returned 

	#endif


	typedef EglStatus (*ScanLineFunction)(EGL_TLVERTEX_V2F_C4F_T2F* startPt, EGL_TLVERTEX_V2F_C4F_T2F* endPt, EGL_RASTER_PARAMS* pRaster_Params);


#endif