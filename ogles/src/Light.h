#ifndef EGL_LIGHT_H
#define EGL_LIGHT_H 1

#pragma once

// ==========================================================================
//
// Light.h			Light Class for Embedded OpenGL Implementation
//
// --------------------------------------------------------------------------
//
// 09-14-2003		Hans-Martin Will	initial version
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