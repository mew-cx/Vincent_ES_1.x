// ==========================================================================
//
// triangles.cpp	Rendering Context Class for Embedded OpenGL Implementation
//
//					Rendering Operations for Triangles
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


// --------------------------------------------------------------------------
// Triangles
// --------------------------------------------------------------------------


void Context :: RenderTriangles(GLint first, GLsizei count) {

	m_Rasterizer->PrepareTriangle();

	while (count >= 3) {
		count -= 3;

		RasterPos pos0, pos1, pos2;
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos0);
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos1);
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos2);

		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);
	}
}


void Context :: RenderTriangles(GLsizei count, const GLubyte * indices) {

	m_Rasterizer->PrepareTriangle();

	while (count >= 3) {
		count -= 3;

		RasterPos pos0, pos1, pos2;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);

		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);
	}
}


void Context :: RenderTriangles(GLsizei count, const GLushort * indices) {

	m_Rasterizer->PrepareTriangle();

	while (count >= 3) {
		count -= 3;

		RasterPos pos0, pos1, pos2;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);

		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);
	}
}


// --------------------------------------------------------------------------
// Triangle Strips
// --------------------------------------------------------------------------


void Context :: RenderTriangleStrip(GLint first, GLsizei count) {

	if (count < 3) {
		return;
	}

	m_Rasterizer->PrepareTriangle();

	RasterPos pos0, pos1, pos2;

	SelectArrayElement(first++);
	CurrentValuesToRasterPos(&pos0);
	SelectArrayElement(first++);
	CurrentValuesToRasterPos(&pos1);

	count -= 2;

	while (count >= 6) {
		count -= 6;

		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos2);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);

		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos0);
		m_Rasterizer->RasterTriangle(pos0, pos2, pos1);

		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos1);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);

		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos2);
		m_Rasterizer->RasterTriangle(pos0, pos2, pos1);

		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos0);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);

		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos1);
		m_Rasterizer->RasterTriangle(pos0, pos2, pos1);
	}

	switch (count) {
	case 5:
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos2);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);

	case 4:
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos0);
		m_Rasterizer->RasterTriangle(pos0, pos2, pos1);

	case 3:
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos1);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);

	case 2:
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos2);
		m_Rasterizer->RasterTriangle(pos0, pos2, pos1);

	case 1:
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos0);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);
	}

}


void Context :: RenderTriangleStrip(GLsizei count, const GLubyte * indices) {

	if (count < 3) {
		return;
	}

	m_Rasterizer->PrepareTriangle();

	RasterPos pos0, pos1, pos2;

	SelectArrayElement(*indices++);
	CurrentValuesToRasterPos(&pos0);
	SelectArrayElement(*indices++);
	CurrentValuesToRasterPos(&pos1);

	count -= 2;

	while (count >= 6) {
		count -= 6;

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		m_Rasterizer->RasterTriangle(pos0, pos2, pos1);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		m_Rasterizer->RasterTriangle(pos0, pos2, pos1);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		m_Rasterizer->RasterTriangle(pos0, pos2, pos1);
	}

	switch (count) {
	case 5:
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);

	case 4:
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		m_Rasterizer->RasterTriangle(pos0, pos2, pos1);

	case 3:
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);

	case 2:
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		m_Rasterizer->RasterTriangle(pos0, pos2, pos1);

	case 1:
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);
	}

}


void Context :: RenderTriangleStrip(GLsizei count, const GLushort * indices) {

	if (count < 3) {
		return;
	}

	m_Rasterizer->PrepareTriangle();

	RasterPos pos0, pos1, pos2;

	SelectArrayElement(*indices++);
	CurrentValuesToRasterPos(&pos0);
	SelectArrayElement(*indices++);
	CurrentValuesToRasterPos(&pos1);

	count -= 2;

	while (count >= 6) {
		count -= 6;

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		m_Rasterizer->RasterTriangle(pos0, pos2, pos1);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		m_Rasterizer->RasterTriangle(pos0, pos2, pos1);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		m_Rasterizer->RasterTriangle(pos0, pos2, pos1);
	}

	switch (count) {
	case 5:
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);

	case 4:
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		m_Rasterizer->RasterTriangle(pos0, pos2, pos1);

	case 3:
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);

	case 2:
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		m_Rasterizer->RasterTriangle(pos0, pos2, pos1);

	case 1:
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);
	}
}


// --------------------------------------------------------------------------
// Triangle Fans
// --------------------------------------------------------------------------


void Context :: RenderTriangleFan(GLint first, GLsizei count) {

	if (count < 3) {
		return;
	}

	m_Rasterizer->PrepareTriangle();

	RasterPos pos0, pos1, pos2;

	SelectArrayElement(first++);
	CurrentValuesToRasterPos(&pos0);
	SelectArrayElement(first++);
	CurrentValuesToRasterPos(&pos1);

	count -= 2;

	while (count >= 2) {
		count -= 2;

		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos2);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);

		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos1);
		m_Rasterizer->RasterTriangle(pos0, pos2, pos1);
	}

	if (count >= 1) {
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos2);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);
	}
}


void Context :: RenderTriangleFan(GLsizei count, const GLubyte * indices) {

	if (count < 3) {
		return;
	}

	m_Rasterizer->PrepareTriangle();

	RasterPos pos0, pos1, pos2;

	SelectArrayElement(*indices++);
	CurrentValuesToRasterPos(&pos0);
	SelectArrayElement(*indices++);
	CurrentValuesToRasterPos(&pos1);

	count -= 2;

	while (count >= 2) {
		count -= 2;

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		m_Rasterizer->RasterTriangle(pos0, pos2, pos1);
	}

	if (count >= 1) {
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);
	}
}


void Context :: RenderTriangleFan(GLsizei count, const GLushort * indices) {

	if (count < 3) {
		return;
	}

	m_Rasterizer->PrepareTriangle();

	RasterPos pos0, pos1, pos2;

	SelectArrayElement(*indices++);
	CurrentValuesToRasterPos(&pos0);
	SelectArrayElement(*indices++);
	CurrentValuesToRasterPos(&pos1);

	count -= 2;

	while (count >= 2) {
		count -= 2;

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		m_Rasterizer->RasterTriangle(pos0, pos2, pos1);
	}

	if (count >= 1) {
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		m_Rasterizer->RasterTriangle(pos0, pos1, pos2);
	}
}

