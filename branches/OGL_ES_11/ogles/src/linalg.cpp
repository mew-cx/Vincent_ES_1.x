// ==========================================================================
//
// linalg.cpp	Implementation of Linear Algebra using Fixed Point Arithmetic
//
// --------------------------------------------------------------------------
//
// 10-03-2003	Hans-Martin Will	initial version
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


#include "stdafx.h"
#include "linalg.h"
#include <math.h>


#ifndef M_PI
#	define M_PI       3.14159265358979323846
#endif

using namespace EGL;


// ==========================================================================
// class Matrix4x4
// ==========================================================================


namespace {
	inline EGL_Fixed Det2X2(EGL_Fixed a, EGL_Fixed b, EGL_Fixed c, EGL_Fixed d) {
		return EGL_Mul(a,d) - EGL_Mul(c,b);
	}
}

Matrix4x4 Matrix4x4 :: Inverse(bool rescale) const {

	Matrix4x4 result;

    int i = 0;
    EGL_Fixed d, r;
    /* compute 3x3 inverse using Cramer's rule: A^-1 = adj(A)/det(A) */
    /* cofactors */
    result.Element(0,0) =  Det2X2(Element(1,1), Element(1,2), Element(2,1), Element(2,2));
    result.Element(0,1) = -Det2X2(Element(1,0), Element(1,2), Element(2,0), Element(2,2));
    result.Element(0,2) =  Det2X2(Element(1,0), Element(1,1), Element(2,0), Element(2,1));

    result.Element(1,0) = -Det2X2(Element(0,1), Element(0,2), Element(2,1), Element(2,2));
    result.Element(1,1) =  Det2X2(Element(0,0), Element(0,2), Element(2,0), Element(2,2));
    result.Element(1,2) = -Det2X2(Element(0,0), Element(0,1), Element(2,0), Element(2,1));

    result.Element(2,0) =  Det2X2(Element(0,1), Element(0,2), Element(1,1), Element(1,2));
    result.Element(2,1) = -Det2X2(Element(0,0), Element(0,2), Element(1,0), Element(1,2));
    result.Element(2,2) =  Det2X2(Element(0,0), Element(0,1), Element(1,0), Element(1,1));

    /* determinant */
    d = 0;

    for(i = 0; i < 3; i++)
		d += EGL_Mul(Element(0,i),result.Element(0,i));

	if (d == 0) {
		// singluar matrix
		return result;
	}

    r = EGL_Inverse(d);

    i = 0;
    do {
		int j = 0;

		do {
			result.Element(i,j) = EGL_Mul(result.Element(i,j), r);
		} while(++j < 3);
    } while(++i < 3);

	result.m_identity = false;

	if (rescale) {
		EGL_Fixed sumOfSquares = 
			EGL_Mul(result.Element(2, 0), result.Element(2, 0)) +
			EGL_Mul(result.Element(2, 1), result.Element(2, 1)) +
			EGL_Mul(result.Element(2, 2), result.Element(2, 2));

		if (sumOfSquares != EGL_ONE) {
			EGL_Fixed factor = EGL_InvSqrt(sumOfSquares);

			for (size_t index = 0; index < 16; ++index) {
				result.Element(index) = EGL_Mul(result.Element(index), factor);
			}
		}
	}

	return result;
}


Matrix4x4 Matrix4x4 :: CreateScale(EGL_Fixed x, EGL_Fixed y, EGL_Fixed z) {
	Matrix4x4 result;

	result.Element(0, 0) = x;
	result.Element(1, 1) = y;
	result.Element(2, 2) = z;

	result.m_identity = false;
	return result;
}


Matrix4x4 Matrix4x4 :: CreateRotate(EGL_Fixed angle, EGL_Fixed x, 
									EGL_Fixed y, EGL_Fixed z) {

	Matrix4x4 matrix;
	Vec3D axis(x, y, z);
	axis.Normalize();
	EGL_Fixed r_x = axis.x();
	EGL_Fixed r_y = axis.y();
	EGL_Fixed r_z = axis.z();

	angle = EGL_Mul(angle, EGL_FixedFromFloat(static_cast<float>(M_PI) / 180.0f));

	EGL_Fixed sine = EGL_Sin(angle);
	EGL_Fixed cosine = EGL_Cos(angle);
	
	EGL_Fixed one_minus_cosine = EGL_ONE - cosine;

	matrix.Element(0, 0) = cosine + EGL_Mul(one_minus_cosine, EGL_Mul(r_x, r_x));
	matrix.Element(0, 1) = EGL_Mul(one_minus_cosine, EGL_Mul(r_x, r_y)) - EGL_Mul(r_z, sine);
	matrix.Element(0, 2) = EGL_Mul(EGL_Mul(one_minus_cosine, r_x), r_z) + EGL_Mul(r_y, sine);

	matrix.Element(1, 0) = EGL_Mul(EGL_Mul(one_minus_cosine, r_x),  r_y) + EGL_Mul(r_z, sine);
	matrix.Element(1, 1) = cosine + EGL_Mul(EGL_Mul(one_minus_cosine, r_y), r_y);
	matrix.Element(1, 2) = EGL_Mul(EGL_Mul(one_minus_cosine, r_y), r_z) - EGL_Mul(r_x, sine);

	matrix.Element(2, 0) = EGL_Mul(EGL_Mul(one_minus_cosine, r_x), r_z) - EGL_Mul(r_y, sine);
	matrix.Element(2, 1) = EGL_Mul(EGL_Mul(one_minus_cosine, r_y), r_z) + EGL_Mul(r_x, sine);
	matrix.Element(2, 2) = cosine + EGL_Mul(EGL_Mul(one_minus_cosine, r_z), r_z);

	matrix.m_identity = false;
	return matrix;
}


Matrix4x4 Matrix4x4 :: CreateTranslate(EGL_Fixed x, EGL_Fixed y, EGL_Fixed z) {
	Matrix4x4 result;

	result.Element(0, 3) = x;
	result.Element(1, 3) = y;
	result.Element(2, 3) = z;

	result.m_identity = false;
	return result;
}


Matrix4x4 Matrix4x4 :: CreateFrustrum(EGL_Fixed l, EGL_Fixed r, 
									  EGL_Fixed b, EGL_Fixed t, EGL_Fixed n, EGL_Fixed f) {
	Matrix4x4 matrix;

	EGL_Fixed inv_width = (r - l) ? EGL_Inverse(r - l) : 0;
	EGL_Fixed inv_height = (t - b) ? EGL_Inverse(t - b) : 0;
	EGL_Fixed inv_depth = (f - n) ? EGL_Inverse(f - n) : 0;

	EGL_Fixed two_n = n * 2;

	matrix.Element(0, 0) = EGL_Mul(two_n, inv_width);
	matrix.Element(0, 2) = EGL_Mul(r + l, inv_width);

	matrix.Element(1, 1) = EGL_Mul(two_n, inv_height);
	matrix.Element(1, 2) = EGL_Mul(t + b, inv_height);

	matrix.Element(2, 2) = EGL_Mul(-f - n, inv_depth);
	matrix.Element(2, 3) = EGL_Mul(EGL_Mul(-two_n, inv_depth), f);

	matrix.Element(3, 2) = -EGL_ONE;
	matrix.Element(3, 3) = 0;	

	matrix.m_identity = false;
	return matrix;
}

Matrix4x4 Matrix4x4 :: CreateOrtho(EGL_Fixed l, EGL_Fixed r, 
								   EGL_Fixed b, EGL_Fixed t, EGL_Fixed n, EGL_Fixed f) {
	Matrix4x4 matrix;

	EGL_Fixed inv_width = (r - l) ? EGL_Inverse(r - l) : 0;
	EGL_Fixed inv_height = (t - b) ? EGL_Inverse(t - b) : 0;
	EGL_Fixed inv_depth = (f - n) ? EGL_Inverse(f - n) : 0;

	EGL_Fixed two_n = n * 2;

	matrix.Element(0, 0) = 2 * inv_width;
	matrix.Element(0, 3) = -EGL_Mul(r + l, inv_width);

	matrix.Element(1, 1) = 2 * inv_height;
	matrix.Element(1, 3) = -EGL_Mul(t + b, inv_height);

	matrix.Element(2, 2) = -2 * inv_depth;
	matrix.Element(2, 3) = EGL_Mul(-f - n, inv_depth);

	matrix.m_identity = false;
	return matrix;
}
