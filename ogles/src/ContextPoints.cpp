// ==========================================================================
//
// points.cpp	Rendering Context Class for Embedded OpenGL Implementation
//
//				Rendering Operations for Points
//
// --------------------------------------------------------------------------
//
// 08-12-2003	Hans-Martin Will	initial version
//
// ==========================================================================


#include "stdafx.h"
#include "Context.h"
#include "fixed.h"
#include "Rasterizer.h"

using namespace EGL;


void Context :: PointSizex(GLfixed size) { 
	GetRasterizerState()->SetPointSize(size);
}


void Context :: RenderPoints(GLint first, GLsizei count) {

	m_Rasterizer->PreparePoint();

	while (count >= 1) {
		count -= 1;

		EGL_RASTER_POS pos0;
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos0);

		m_Rasterizer->RasterPoint(pos0);
	}
}


void Context :: RenderPoints(GLsizei count, const GLubyte * indices) {

	m_Rasterizer->PreparePoint();

	while (count >= 1) {
		count -= 1;

		EGL_RASTER_POS pos0;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);

		m_Rasterizer->RasterPoint(pos0);
	}
}


void Context :: RenderPoints(GLsizei count, const GLushort * indices) {

	m_Rasterizer->PreparePoint();

	while (count >= 1) {
		count -= 1;

		EGL_RASTER_POS pos0;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);

		m_Rasterizer->RasterPoint(pos0);
	}
}

