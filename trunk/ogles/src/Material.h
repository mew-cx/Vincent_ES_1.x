#ifndef EGL_MATERIAL_H
#define EGL_MATERIAL_H 1

#pragma once

// ==========================================================================
//
// Material.h		Material Class for Embedded OpenGL Implementation
//
// --------------------------------------------------------------------------
//
// 09-14-2003		Hans-Martin Will	initial version
//
// ==========================================================================



#include "OGLES.h"
#include "FractionalColor.h"


namespace EGL {

	class OGLES_API Material {

	public:

		void SetAmbientColor(const FractionalColor & color);
		inline const FractionalColor& GetAmbientColor() const;

		void SetDiffuseColor(const FractionalColor & color);
		inline const FractionalColor& GetDiffuseColor() const;

		void SetSpecularColor(const FractionalColor & color);
		inline const FractionalColor& GetSpecularColor() const;

		void SetEmisiveColor(const FractionalColor & color);
		inline const FractionalColor& GetEmisiveColor() const;

		void SetSpecularExponent(EGL_Fixed exponent);
		inline EGL_Fixed GetSpecularExponent() const;

	private:
		FractionalColor			m_AmbientColor;
		FractionalColor			m_DiffuseColor;
		FractionalColor			m_SpecularColor;
		FractionalColor			m_EmisiveColor;
		EGL_Fixed					m_SpecularExponent;
	};


	// --------------------------------------------------------------------------
	// Inline Functions
	// --------------------------------------------------------------------------


	inline const FractionalColor& Material :: GetAmbientColor() const {
		return m_AmbientColor;
	}


	inline const FractionalColor& Material :: GetDiffuseColor() const {
		return m_DiffuseColor;
	}


	inline const FractionalColor& Material :: GetSpecularColor() const {
		return m_SpecularColor;
	}


	inline const FractionalColor& Material :: GetEmisiveColor() const {
		return m_EmisiveColor;
	}


	inline EGL_Fixed Material :: GetSpecularExponent() const {
		return m_SpecularExponent;
	}
}

#endif //ndef EGL_MATERIAL_H