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