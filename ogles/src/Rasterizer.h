#ifndef EGL_RASTERIZER_H
#define EGL_RASTERIZER_H 1


#pragma once


// ==========================================================================
//
// Rasterizer.h		Rasterizer Class for OpenGL (R) ES Implementation
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


namespace EGL {

	namespace triVM {
		union Label;
		struct Block;
		struct Procedure;
	}

	class Surface;
	class Texture;

	struct FragmentGenerationInfo;

	// ----------------------------------------------------------------------
	// u and v coordinates for texture mapping
	// ----------------------------------------------------------------------
	struct TexCoord {
		EGL_Fixed		tu, tv;		// texture coordinates between 0 and 1
	};

	struct ScreenCoord {
		EGL_Fixed		x, y, z;	// x, y, z window coords
	};

	struct EdgeCoord {
		EGL_Fixed		x, z;		// x window coords
	};

	// ----------------------------------------------------------------------
	// Vertex information as input for rasterizer
	// ----------------------------------------------------------------------

	struct RasterPos {
		Vec4D				m_ClipCoords;
		ScreenCoord			m_WindowCoords;		
		FractionalColor		m_Color;			// color in range 0..255
		TexCoord			m_TextureCoords;	// texture coords 0..1
		EGL_Fixed			m_FogDensity;		// fog density at this vertex
	};

	struct EdgePos {
		EdgeCoord			m_WindowCoords;		// z over w
		FractionalColor		m_Color;			// color in range 0..255
		TexCoord			m_TextureCoords;	// texture coords 0..1 over w
		EGL_Fixed			m_FogDensity;		// fog density at this vertex
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

		void SetTexture(MultiTexture * texture)		{ m_Texture = texture; }
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
		// Rasterization of triangle scan line
		// ----------------------------------------------------------------------
		void RasterScanLine(const EdgePos& start, const EdgePos& end, U32 y);

		// ----------------------------------------------------------------------
		// Code generation of triangle scan line
		// ----------------------------------------------------------------------
		void GenerateRasterScanLine();
		void GenerateFragment(triVM::Procedure * procedure, triVM::Block & currentBlock,
			triVM::Label * continuation, I32 & nextRegister,
			FragmentGenerationInfo & fragmentInfo);

		// ----------------------------------------------------------------------
		// Rasterization of fragment
		// ----------------------------------------------------------------------

		void Fragment(I32 x, I32 y, EGL_Fixed depth, EGL_Fixed tu, EGL_Fixed tv, 
			EGL_Fixed fogDensity, const Color& baseColor);
			// will have special cases based on settings
			// the coordinates are integer coordinates

	private:
		// ----------------------------------------------------------------------
		// other settings
		// ----------------------------------------------------------------------

		Surface *				m_Surface;			// rendering surface
		MultiTexture *			m_Texture;			// current texture 
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

		int						m_MipMapLevel;

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

