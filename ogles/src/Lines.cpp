// ==========================================================================
//
// lines.cpp	Rendering Context Class for Embedded OpenGL Implementation
//
//				Rendering Operations for Lines
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


void Context :: LineWidthx(GLfixed width) { 
	GetRasterizerState()->SetLineWidth(width);
}


// --------------------------------------------------------------------------
// Lines
// --------------------------------------------------------------------------


void Context :: RenderLines(GLint first, GLsizei count) {

	m_Rasterizer->PrepareLine();

	while (count >= 2) {
		count -= 2;

		RasterPos pos0, pos1;
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos0);
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos1);

		m_Rasterizer->RasterLine(pos0, pos1);
	}
}


void Context :: RenderLines(GLsizei count, const GLubyte * indices) {

	m_Rasterizer->PrepareLine();

	while (count >= 2) {
		count -= 2;

		RasterPos pos0, pos1;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);

		m_Rasterizer->RasterLine(pos0, pos1);
	}
}


void Context :: RenderLines(GLsizei count, const GLushort * indices) {

	m_Rasterizer->PrepareLine();

	while (count >= 2) {
		count -= 2;

		RasterPos pos0, pos1;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);

		m_Rasterizer->RasterLine(pos0, pos1);
	}
}


// --------------------------------------------------------------------------
// Line Strips
// --------------------------------------------------------------------------


void Context :: RenderLineStrip(GLint first, GLsizei count) {

	if (count >= 2) {
		m_Rasterizer->PrepareLine();

		RasterPos pos0, pos1;
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos0);
		--count;

		while (count >= 2) {
			count -= 2;

			SelectArrayElement(first++);
			CurrentValuesToRasterPos(&pos1);
			m_Rasterizer->RasterLine(pos0, pos1);

			SelectArrayElement(first++);
			CurrentValuesToRasterPos(&pos0);
			m_Rasterizer->RasterLine(pos1, pos0);
		}

		if (count >= 1) {
			SelectArrayElement(first++);
			CurrentValuesToRasterPos(&pos1);
			m_Rasterizer->RasterLine(pos0, pos1);
		}
	}
}


void Context :: RenderLineStrip(GLsizei count, const GLubyte * indices) {

	if (count >= 2) {
		m_Rasterizer->PrepareLine();

		RasterPos pos0, pos1;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		--count;

		while (count >= 2) {
			count -= 2;

			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos1);
			m_Rasterizer->RasterLine(pos0, pos1);

			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos0);
			m_Rasterizer->RasterLine(pos1, pos0);
		}

		if (count >= 1) {
			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos1);
			m_Rasterizer->RasterLine(pos0, pos1);
		}
	}
}


void Context :: RenderLineStrip(GLsizei count, const GLushort * indices) {

	if (count >= 2) {
		m_Rasterizer->PrepareLine();

		RasterPos pos0, pos1;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		--count;

		while (count >= 2) {
			count -= 2;

			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos1);
			m_Rasterizer->RasterLine(pos0, pos1);

			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos0);
			m_Rasterizer->RasterLine(pos1, pos0);
		}

		if (count >= 1) {
			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos1);
			m_Rasterizer->RasterLine(pos0, pos1);
		}
	}
}


// --------------------------------------------------------------------------
// Line Loops
// --------------------------------------------------------------------------


void Context :: RenderLineLoop(GLint first, GLsizei count) {

	if (count >= 2) {
		m_Rasterizer->PrepareLine();

		RasterPos pos0, pos1, start;
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&start);
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos0);
		m_Rasterizer->RasterLine(start, pos0);

		count -= 2;

		while (count >= 2) {
			count -= 2;

			SelectArrayElement(first++);
			CurrentValuesToRasterPos(&pos1);
			m_Rasterizer->RasterLine(pos0, pos1);

			SelectArrayElement(first++);
			CurrentValuesToRasterPos(&pos0);
			m_Rasterizer->RasterLine(pos1, pos0);
		}

		if (count >= 1) {
			SelectArrayElement(first++);
			CurrentValuesToRasterPos(&pos1);
			m_Rasterizer->RasterLine(pos0, pos1);
			m_Rasterizer->RasterLine(pos1, start);
		} else {
			m_Rasterizer->RasterLine(pos0, start);
		}
	}
}


void Context :: RenderLineLoop(GLsizei count, const GLubyte * indices) {

	if (count >= 2) {
		m_Rasterizer->PrepareLine();

		RasterPos pos0, pos1, start;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&start);
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		m_Rasterizer->RasterLine(start, pos0);

		count -= 2;

		while (count >= 2) {
			count -= 2;

			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos1);
			m_Rasterizer->RasterLine(pos0, pos1);

			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos0);
			m_Rasterizer->RasterLine(pos1, pos0);
		}

		if (count >= 1) {
			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos1);
			m_Rasterizer->RasterLine(pos0, pos1);
			m_Rasterizer->RasterLine(pos1, start);
		} else {
			m_Rasterizer->RasterLine(pos0, start);
		}
	}
}


void Context :: RenderLineLoop(GLsizei count, const GLushort * indices) {

	if (count >= 2) {
		m_Rasterizer->PrepareLine();

		RasterPos pos0, pos1, start;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&start);
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		m_Rasterizer->RasterLine(start, pos0);

		count -= 2;

		while (count >= 2) {
			count -= 2;

			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos1);
			m_Rasterizer->RasterLine(pos0, pos1);

			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos0);
			m_Rasterizer->RasterLine(pos1, pos0);
		}

		if (count >= 1) {
			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos1);
			m_Rasterizer->RasterLine(pos0, pos1);
			m_Rasterizer->RasterLine(pos1, start);
		} else {
			m_Rasterizer->RasterLine(pos0, start);
		}
	}
}

