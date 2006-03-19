#ifndef EGL_UTILS_H
#define EGL_UTILS_H 1


// ==========================================================================
//
// Utils.h		Helper functions for 3D Rendering Library
//
// --------------------------------------------------------------------------
//
// 09-14-2004	Hans-Martin Will	initial version
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
#include "GLES/gl.h"
#include "FractionalColor.h"
#include "linalg.h"
#include "Context.h"


namespace EGL {
	void CopyVector(const Vec3D & vector, GLfixed *params);
	void CopyVector(const Vec4D & vector, GLfixed *params);
	void CopyColor(const FractionalColor & color, GLfixed *params);
	void CopyMatrix(const Matrix4x4& matrix, GLfixed *params);

#ifdef EGL_USE_CLZ
	inline U32 CountLeadingZeros(U32 x) {
		return _CountLeadingZeros(x);
	}
#else
	U32 CountLeadingZeros(U32 x);
#endif

	inline int Log2(int value) {
		return 31 - CountLeadingZeros(value);
	}

	inline EGL_Fixed Interpolate(EGL_Fixed x0f, EGL_Fixed x1f, EGL_Fixed coeff) {
		return x1f + EGL_Mul((x0f - x1f), coeff);
	}

	inline void Interpolate(Vertex& result, const Vertex& dst, const Vertex& src, EGL_Fixed coeff, size_t numVarying) {
		result.m_ClipCoords.setX(Interpolate(dst.m_ClipCoords.x(), src.m_ClipCoords.x(), coeff));
		result.m_ClipCoords.setY(Interpolate(dst.m_ClipCoords.y(), src.m_ClipCoords.y(), coeff));
		result.m_ClipCoords.setZ(Interpolate(dst.m_ClipCoords.z(), src.m_ClipCoords.z(), coeff));
		result.m_ClipCoords.setW(Interpolate(dst.m_ClipCoords.w(), src.m_ClipCoords.w(), coeff));

		for (size_t index = 0; index < numVarying; ++index) {
			result.m_Varying[index] = Interpolate(dst.m_Varying[index], src.m_Varying[index], coeff);
		}
	}

	inline void InterpolateWithEye(Vertex& result, const Vertex& dst, const Vertex& src, EGL_Fixed coeff, size_t numVarying) {
		result.m_EyeCoords.setX(Interpolate(dst.m_EyeCoords.x(), src.m_EyeCoords.x(), coeff));
		result.m_EyeCoords.setY(Interpolate(dst.m_EyeCoords.y(), src.m_EyeCoords.y(), coeff));
		result.m_EyeCoords.setZ(Interpolate(dst.m_EyeCoords.z(), src.m_EyeCoords.z(), coeff));
		result.m_EyeCoords.setW(Interpolate(dst.m_EyeCoords.w(), src.m_EyeCoords.w(), coeff));

		Interpolate(result, dst, src, coeff, numVarying);
	}
}



#endif //ndef EGL_UTILS_H
