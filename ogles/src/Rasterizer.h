#ifndef EGL_RASTERIZER_H
#define EGL_RASTERIZER_H 1

// ==========================================================================
//
// Rasterizer.h		Rasterizer Class for 3D Rendering Library
//
//					The rasterizer converts transformed and lit 
//					primitives and creates a raster image in the
//					current rendering surface.
//
// --------------------------------------------------------------------------
//
// 10-06-2003		Hans-Martin Will	initial version
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


#include "OGLES.h"
#include "fixed.h"
#include "linalg.h"
#include "RasterizerState.h"
#include "FractionalColor.h"
#include "Surface.h"


namespace EGL {

	class Texture;
	class FunctionCache;

	// ----------------------------------------------------------------------
	// u and v coordinates for texture mapping
	// ----------------------------------------------------------------------
	struct TexCoord {
		EGL_Fixed		tu, tv;			// texture coordinates between 0 and 1
		EGL_Fixed		dtudx, dtvdx;	// partial derivatives
		EGL_Fixed		dtudy, dtvdy;	// partial derivatives
	};

	struct ScreenCoord {
		EGL_Fixed		x, y;		// x, y window coords
		EGL_Fixed		invZ;		// 1/Z (w) from depth division

		// TO DO: once the rasterizer works properly, change this to binary 0..2^n-1
		EGL_Fixed		depth;		// depth coordinate after transformation
	};

	struct EdgeCoord {
		EGL_Fixed		x, invZ;	// x window coords

		// TO DO: once the rasterizer works properly, change this to binary 0..2^n-1
		EGL_Fixed		depth;		// depth coordinate
	};

	// ----------------------------------------------------------------------
	// Vertex information as input for rasterizer
	// ----------------------------------------------------------------------

	struct RasterPos {
		Vec4D				m_ClipCoords;
		ScreenCoord			m_WindowCoords;		
		FractionalColor		m_Color;
		FractionalColor		m_FrontColor;		// color in range 0..255
		FractionalColor		m_BackColor;		
		TexCoord			m_TextureCoords;	// texture coords 0..1
		EGL_Fixed			m_FogDensity;		// fog density at this vertex
	};

	// ----------------------------------------------------------------------
	// Coordinate increments used in various parts of the rasterizer
	// ----------------------------------------------------------------------

	struct EdgePos {
		EdgeCoord			m_WindowCoords;		// z over w
		FractionalColor		m_Color;			// color in range 0..255
		TexCoord			m_TextureCoords;	// texture coords 0..1 over w
		EGL_Fixed			m_FogDensity;		// fog density at this vertex
	};

	// ----------------------------------------------------------------------
	// Gradient data structure used for triangle rasterization
	// ----------------------------------------------------------------------

	struct Gradients {
		EdgePos				dx;
		EdgePos				dy;
	};

	struct EdgePos;

	struct RasterInfo {
		// surface info
		I32			SurfaceWidth;
		I32			SurfaceHeight;
		I32 *		DepthBuffer;
		U16 *		ColorBuffer;
		U32 *		StencilBuffer;
		U8 *		AlphaBuffer;
		const I32 *	InversionTablePtr;

		// TODO: will need to add a minimum texture level here
		// TODO: 

		// texture info
		Texture *	Textures;
		U32			MipmapLevel;
		U32			MaxMipmapLevel;


		void Init(Surface * surface, I32 y) {
			size_t offset = y * surface->GetWidth();
			
			SurfaceWidth = surface->GetWidth();
			SurfaceHeight = surface->GetHeight();
			DepthBuffer = surface->GetDepthBuffer() + offset;
			ColorBuffer = surface->GetColorBuffer() + offset;
			StencilBuffer = surface->GetStencilBuffer() + offset;
			AlphaBuffer = surface->GetAlphaBuffer() + offset;
			InversionTablePtr = InversionTable;
		}

		static const I32 InversionTable[];
	};

	// signature for generated scanline functions
	typedef void (ScanlineFunction)(const RasterInfo * info, const EdgePos * start, const EdgePos * end);

	class Rasterizer {

	public:
		enum {
			PolygonOffsetUnitSize = 1,		// how to determine this?
			DepthRangeMax = 0x7fffffff		// 31 bits 
		};

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

		void SetTexture(MultiTexture * texture);
		MultiTexture * GetTexture()					{ return m_Texture; }
		const MultiTexture * GetTexture() const		{ return m_Texture; }

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
		// State management
		// ----------------------------------------------------------------------

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
		// Rasterization of triangle scan line
		// ----------------------------------------------------------------------
		void RasterScanLine(RasterInfo & info, const EdgePos & start, const EdgePos & end);

		// ----------------------------------------------------------------------
		// Rasterization of fragment
		// ----------------------------------------------------------------------

		void Fragment(I32 x, I32 y, EGL_Fixed depth, EGL_Fixed tu, EGL_Fixed tv, 
			EGL_Fixed fogDensity, const Color& baseColor);
			// will have special cases based on settings
			// the coordinates are integer coordinates

		void Fragment(const RasterInfo * rasterInfo, I32 x, EGL_Fixed depth, 
					  EGL_Fixed tu, EGL_Fixed tv,
					  const Color& baseColor, EGL_Fixed fog);
			// fragment rendering with signature corresponding to function fragment
			// generated by code generator

		Color GetTexColor(const Texture * texture, EGL_Fixed tu, EGL_Fixed tv, 
						  RasterizerState::FilterMode filterMode);
			// retrieve the texture color from a texture plane 
						  
		Color GetRawTexColor(const Texture * texture, EGL_Fixed tu, EGL_Fixed tv);
			// retrieve the texture color from a texture plane
						  
	private:
		// ----------------------------------------------------------------------
		// other settings
		// ----------------------------------------------------------------------

		RasterInfo				m_RasterInfo;
		Surface *				m_Surface;			// rendering surface
		MultiTexture *			m_Texture;			// current texture 
		RasterizerState *		m_State;			// current rasterization settings
		FunctionCache *			m_FunctionCache;

		ScanlineFunction *		m_ScanlineFunction;

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

		bool					m_UseMipmap;

	};


	// --------------------------------------------------------------------------
	// Inline member definitions
	// --------------------------------------------------------------------------


	inline void Rasterizer :: SetSurface(Surface * surface) {
		m_Surface = surface;
	}


	inline Surface * Rasterizer :: GetSurface() const {
		return m_Surface;
	}

}



#endif //ndef EGL_RASTERIZER_H

