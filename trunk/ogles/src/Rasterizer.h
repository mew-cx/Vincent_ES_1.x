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


class Surface;
class Texture;


namespace EGL {

	typedef struct {
		U32		tu, tv;				// u and v coordinates for texture mapping
	} TEX_COORD;


	typedef struct RasterPos {
		Vec4D						m_WindowsCoords;	
		FractionalColor				m_Color;
		TEX_COORD					m_TextureCoords;

	} EGL_RASTER_POS;



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
		Rasterizer();
		~Rasterizer();

		// ----------------------------------------------------------------------
		// Rendering surface
		// ----------------------------------------------------------------------
		void SetState(RasterizerState * state);
		RasterizerState * GetState() const;

		// ----------------------------------------------------------------------
		// Whole framebuffer operations
		// ----------------------------------------------------------------------
		void Clear(U32 mask);

		void ReadPixels(I32 x, I32 y, U32 width, U32 height,
			PixelFormat format, PixelType type, void *pixels);

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
		// Rasterization of fragment
		// ----------------------------------------------------------------------

		void Fragment(const Vec3D & coords, const FractionalColor & color);
			// will have special cases based on settings

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
		// ----------------------------------------------------------------------
		// other settings
		// ----------------------------------------------------------------------

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
}

#endif //ndef EGL_RASTERIZER_H

