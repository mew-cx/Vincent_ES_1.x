#ifndef EGL_RASTERIZER_H
#define EGL_RASTERIZER_H 1


#pragma once


// ==========================================================================
//
// Rasterizer.h		Rasterizer Class for Embedded OpenGL Implementation
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


#include "OGLES.h"
#include "fixed.h"
#include "linalg.h"
#include "RasterizerState.h"
#include "FractionalColor.h"



namespace EGL {

	class Surface;
	class Texture;

	// ----------------------------------------------------------------------
	// u and v coordinates for texture mapping
	// ----------------------------------------------------------------------
	struct TexCoord {
		EGL_Fixed		tu, tv;		// texture coordinates between 0 and 1
	};

	struct ScreenCoord {
		EGL_Fixed		x, y;		// x, y window coords
		EGL_Fixed		w;			// z before division (0..1)
	};

	struct EdgeCoord {
		EGL_Fixed		x;			// x window coords
		EGL_Fixed		w;			// z before division
	};

	// ----------------------------------------------------------------------
	// Vertex information as input for rasterizer
	// ----------------------------------------------------------------------
	struct RasterPos {
		ScreenCoord			m_WindowCoords;		// x, y window coords
												// z
												// w, i.e. z before division
		FractionalColor		m_Color;			// color in range 0..255
		TexCoord			m_TextureCoords;	// texture coords 0..1

	};

	struct EdgePos {
		EdgeCoord			m_WindowCoords;		// x, window coords
												// z
												// w, i.e. z before division
		FractionalColor		m_Color;			// color in range 0..255
		TexCoord			m_TextureCoords;	// texture coords 0..1

	};



	class OGLES_API Rasterizer {

	public:
		enum PixelFormat {
			PixelFormatRGBA,
			PixelFormatRGB,
			PixelFormatLuminanceAlpha,
			PixelFormatLuminance,
			PixelFormatAlpha
		};

		enum PixelType {
			PixelTypeUnsignedByte,
			PixelTypeUnsignedShort_4_4_4_4,
			PixelTypeUnsignedShort_5_5_5_1,
			PixelTypeUnsignedShort_5_6_5
		};


	public:
		Rasterizer(RasterizerState * state);
		~Rasterizer();

		// ----------------------------------------------------------------------
		// Rendering surface
		// ----------------------------------------------------------------------
		void SetState(RasterizerState * state);
		RasterizerState * GetState() const;

		void SetSurface(Surface * surface);
		Surface * GetSurface() const;

		// ----------------------------------------------------------------------
		// Actual rasterization of primitives
		// These functions take care of anything downstream of the
		// scissor test, scan conversion, texturing, compositing, depth &
		// stencil test.
		// ----------------------------------------------------------------------

		typedef void (Rasterizer::*RasterPointFunction)(const RasterPos& point);
		typedef void (Rasterizer::*RasterLineFunction)(const RasterPos& from, const RasterPos& to);
		typedef void (Rasterizer::*RasterTriangleFunction)(const RasterPos& a, const RasterPos& b,
			const RasterPos& c);

		void RasterPoint(const RasterPos& point);
		void RasterLine(const RasterPos& from, const RasterPos& to);
		void RasterTriangle(const RasterPos& a, const RasterPos& b,
			const RasterPos& c);

		// ----------------------------------------------------------------------
		// Rasterization of triangle scan line
		// ----------------------------------------------------------------------
		void RasterScanLine(const EdgePos& start, const EdgePos& end, U32 y);

		// ----------------------------------------------------------------------
		// Rasterization of fragment
		// ----------------------------------------------------------------------

		void Fragment(I32 x, I32 y, EGL_Fixed depth, Color color);
			// will have special cases based on settings
			// the coordinates are integer coordinates

		// ----------------------------------------------------------------------
		// State management
		// ----------------------------------------------------------------------

		void InvalidateState();
		void PreparePoint();
		void PrepareLine();
		void PrepareTriangle();
		void Finish();
		//void UpdateWindowClipping(void);

		// ----------------------------------------------------------------------
		// Include actual rasterization functions here
		// ----------------------------------------------------------------------
		//#include "generated_rasterization_function_declarations.h"

	private:
		void RasterClippedXTriangle(RasterPos * pos1, RasterPos * pos2, RasterPos * pos3);

	private:
		// ----------------------------------------------------------------------
		// other settings
		// ----------------------------------------------------------------------

		Surface *				m_Surface;			// rendering surface
		RasterizerState *		m_State;			// current rasterization settings

		RasterPointFunction		m_RasterPointFunction;
		RasterLineFunction		m_RasterLineFunction;
		RasterTriangleFunction	m_RasterTriangleFunction;

		// ----------------------------------------------------------------------
		// internal state
		// ----------------------------------------------------------------------
		EGL_Fixed				m_MinX;
		EGL_Fixed				m_MaxX;
		EGL_Fixed				m_MinY;
		EGL_Fixed				m_MaxY;

		bool					m_IsPrepared;


	};


	// --------------------------------------------------------------------------
	// Inline member definitions
	// --------------------------------------------------------------------------


	inline void Rasterizer :: InvalidateState() {
		m_IsPrepared = false;
	}

	inline void Rasterizer :: SetSurface(Surface * surface) {
		m_Surface = surface;
	}


	inline Surface * Rasterizer :: GetSurface() const {
		return m_Surface;
	}

}



#endif //ndef EGL_RASTERIZER_H

