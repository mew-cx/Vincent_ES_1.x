#ifndef EGL_RASTER_TRIANGLE_EDGE_DEPTH_STENCIL_PART_H
#define EGL_RASTER_TRIANGLE_EDGE_DEPTH_STENCIL_PART_H 1

// ==========================================================================
//
// RasterTriangleEdgeDepthStencilPart.h	JIT Class for 3D Rendering Library
//
//						This class encapsulates the specifics for managing
//						code generation of edge/depth/stencil processing
//						of triangle rasterization
//
// --------------------------------------------------------------------------
//
// 04-24-2006	Hans-Martin Will	initial version
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
#include "RasterPart.h"


namespace EGL {
	class RasterTriangleEdgeDepthStencilPart: public RasterPart {
	public:
		bool CompareState(const void * first, const void * second) const;
		void Compile(FunctionCache * target, const VaryingInfo * varyingInfo, const void * state);
		Part GetPart() const;
		
	private:
		void GenerateRasterBlockEdgeDepthStencil(const VaryingInfo * varyingInfo);
	};
	
}

#endif // ndef EGL_RASTER_TRIANGLE_EDGE_DEPTH_STENCIL_PART_H