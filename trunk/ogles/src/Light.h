#ifndef EGL_LIGHT_H
#define EGL_LIGHT_H 1

#pragma once

// ==========================================================================
//
// Light.h			Light Class for 3D Rendering Library
//
// --------------------------------------------------------------------------
//
// 09-14-2003		Hans-Martin Will	initial version
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
#include "linalg.h"
#include "FractionalColor.h"


namespace EGL {

class Material;


class OGLES_API Light {

public:
	Light();

	void SetAmbientColor(const FractionalColor & color);

	void SetDiffuseColor(const FractionalColor & color);

	void SetSpecularColor(const FractionalColor & color);

	void SetPosition(const Vec4D & position);

	void SetDirection(const Vec3D & direction);

	void SetConstantAttenuation(EGL_Fixed attenuation);

	void SetLinearAttenuation(EGL_Fixed attenuation);

	void SetQuadraticAttenuation(EGL_Fixed attenuation);

	void SetSpecularExponent(EGL_Fixed exponent);

	void SetSpotExponent(EGL_Fixed exponent);

	void SetSpotCutoff(EGL_Fixed cutoff);

	void InitWithMaterial(const Material& material);

	void AccumulateLight(const Vec4D& vertexCoords, const Vec3D& vertexNormal, 
		FractionalColor& result);

	void AccumulateLight(const Vec4D& vertexCoords, const Vec3D& vertexNormal, 
		const FractionalColor& currentColor, FractionalColor& result);

private:
	FractionalColor			m_AmbientColor;
	FractionalColor			m_DiffuseColor;
	FractionalColor			m_SpecularColor;
	Vec4D					m_Position;
	Vec3D					m_SpotDirection;
	EGL_Fixed					m_ConstantAttenuation;
	EGL_Fixed					m_LinearAttenuation;
	EGL_Fixed					m_QuadraticAttenuation;
	EGL_Fixed					m_SpecularExponent;
	EGL_Fixed					m_SpotExponent;
	EGL_Fixed					m_SpotCutoff;
	EGL_Fixed					m_CosineSpotCutoff;

	// effective color contributions for current material settings
	FractionalColor			m_EffectiveAmbientColor;
	FractionalColor			m_EffectiveDiffuseColor;
	FractionalColor			m_EffectiveSpecularColor;
};

}

#endif //ndef EGL_LIGHT_H