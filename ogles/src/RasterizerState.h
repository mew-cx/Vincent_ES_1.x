#ifndef EGL_RASTERIZER_STATE_H
#define EGL_RASTERIZER_STATE_H 1


#pragma once


// ==========================================================================
//
// RasterizerState	Rasterizer State Class for Embedded OpenGL Implementation
//
//					The rasterizer state maintains all the state information
//					that is necessary to determine the details of the
//					next primitive to be scan-converted.
//
// --------------------------------------------------------------------------
//
// 10-10-2003		Hans-Martin Will	initial version
//
// ==========================================================================


#include "OGLES.h"
#include "fixed.h"
#include "linalg.h"
#include "FractionalColor.h"


namespace EGL {
	class Rasterizer;
	class MultiTexture;

	class OGLES_API RasterizerState {

		friend class Rasterizer;

	public:
		enum LogicOp {
			LogicOpClear,
			LogicOpAnd,
			LogicOpAndReverse,
			LogicOpCopy,
			LogicOpAndInverted,
			LogicOpNoop,
			LogicOpXor,
			LogicOpOr,
			LogicOpNor,
			LogicOpEquiv,
			LogicOpInvert,
			LogicOpOrReverse,
			LogicOpCopyInverted,
			LogicOpOrInverted,
			LogicOpNand,
			LogicOpSet
		};

		enum ComparisonFunc {
			CompFuncNever,
			CompFuncLess,
			CompFuncEqual,
			CompFuncLEqual,
			CompFuncGreater,
			CompFuncNotEqual,
			CompFuncGEqual,
			CompFuncAlways
		};

		enum BlendFuncDst {
			BlendFuncDstZero,
			BlendFuncDstOne,
			BlendFuncDstSrcColor,
			BlendFuncDstOneMinusSrcColor,
			BlendFuncDstSrcAlpha,
			BlendFuncDstSrcOneMinusSrcAlpha,
			BlendFuncDstDstAlpha,
			BlendFuncDstOneMinusDstAlpha
		};

		enum BlendFuncSrc {
			BlendFuncSrcZero,
			BlendFuncSrcOne,
			BlendFuncSrcSrcColor,
			BlendFuncSrcOneMinusSrcColor,
			BlendFuncSrcOneMinusDstColor,
			BlendFuncSrcSrcAlphaSaturate,
			BlendFuncSrcOneMinusSrcAlpha,
			BlendFuncSrcSrcAlpha,
			BlendFuncSrcDstAlpha,
			BlendFuncSrcOneMinusDstAlpha
		};

		enum FogMode {
			FogLinear,
			FogModeExp,
			FogModeExp2
		};

		enum StencilOp {
			StencilOpZero,
			StencilOpKeep,
			StencilOpReplace,
			StencilOpIncr,
			StencilOpDecr,
			StencilOpInvert
		};

		enum ShadingModel {
			ShadeModelFlat,
			ShadeModelSmooth
		};

		enum TextureMode {
			TextureModeDecal,
			TextureModeReplace,
			TextureModeBlend,
			TextureModeAdd,
			TextureModeModulate
		};

	public:
		RasterizerState();
		~RasterizerState();

		U32 GetVersion() const;
		void VersionChanged();

		// ----------------------------------------------------------------------
		// Primitive rendering state
		// ----------------------------------------------------------------------

		void SetTexture(MultiTexture * texture);
		MultiTexture * GetTexture()					{ VersionChanged(); return m_Texture; }
		const MultiTexture * GetTexture() const		{ return m_Texture; }

		void SetTexEnvColor(const FractionalColor& color);
		void SetTextureMode(TextureMode mode);

		void SetDepthRange(EGL_Fixed zNear, EGL_Fixed zFar);
		void SetFogMode(FogMode mode);
		void SetFogStart(EGL_Fixed start);
		void SetFogEnd(EGL_Fixed end);
		void SetFogDensity(EGL_Fixed density);

		void SetFogColor(const FractionalColor& color);
		void EnableFog(bool enabled);

		void SetLineWidth(EGL_Fixed width);
		void SetLineSmoothEnabled(bool enabled);

		void SetLogicOp(LogicOp opcode);

		void SetShadeModel(ShadingModel mode);
		void EnableTexture(bool enabled);

		void SetPointSize(EGL_Fixed size);
		void SetPointSmoothEnabled(bool enabled);

		void SetPolygonOffset(EGL_Fixed factor, EGL_Fixed units);
		void SetSampleCoverage(EGL_Fixed value, bool invert);

		// ----------------------------------------------------------------------
		// Fragment rendering state
		// ----------------------------------------------------------------------

		void SetAlphaFunc(ComparisonFunc func, EGL_Fixed ref);
		void SetBlendFunc(BlendFuncSrc sfactor, BlendFuncDst dfactor);
		void SetColorMask(bool red, bool green, bool blue, bool alpha);
		void SetDepthFunc(ComparisonFunc func);
		void SetDepthMask(bool flag);
		void SetScissor(I32 x, I32 y, U32 width, U32 height);

		void EnableAlphaTest(bool enabled);
		void EnableDepthTest(bool enabled);
		void EnableLogicOp(bool enabled);
		void EnableScissorTest(bool enabled);
		void EnableBlending(bool enabled);

		void SetStencilFunc(ComparisonFunc func, I32 ref, U32 mask);
		void SetStencilMask(U32 mask);
		void SetStencilOp(StencilOp fail, StencilOp zfail, StencilOp zpass);
		void EnableStencilTest(bool enabled);

	private:
		// ----------------------------------------------------------------------
		// Primitve rendering state
		// ----------------------------------------------------------------------

		EGL_Fixed				m_PointSize;	// current point size
		EGL_Fixed				m_LineWidth;	// current line width
		bool					m_PointSmoothEnabled;
		bool					m_LineSmoothEnabled;

		EGL_Fixed				m_DepthRangeNear, m_DepthRangeFar;

		FogMode					m_FogMode;
		EGL_Fixed				m_FogStart, m_FogDensity, m_FogEnd;
		FractionalColor			m_FogColor;

		bool					m_FogEnabled;

		EGL_Fixed				m_PolygonOffsetFactor;
		EGL_Fixed				m_PolygonOffsetUnits;
		EGL_Fixed				m_SampleCoverage;
		bool					m_InvertSampleCoverage;

		ShadingModel			m_ShadingModel;

		// ----------------------------------------------------------------------
		// Texture environment rendering state
		// ----------------------------------------------------------------------

		FractionalColor			m_TexEnvColor;
		bool					m_TextureEnabled;
		TextureMode				m_TextureMode;

		// ----------------------------------------------------------------------
		// Fragment rendering state
		// ----------------------------------------------------------------------

		I32						m_ScissorX, m_ScissorY;
		U32						m_ScissorWidth, m_ScissorHeight;

		bool					m_MaskRed;		// if true, write red channel
		bool					m_MaskGreen;	// if true, write green channel
		bool					m_MaskBlue;		// if true, write blue channel
		bool					m_MaskAlpha;	// if true, write alpha channel
		bool					m_MaskDepth;	// if true, write depth buffer

		bool					m_ScissorTestEnabled;
		bool					m_AlphaTestEnabled;
		bool					m_DepthTestEnabled;
		bool					m_LogicOpEnabled;
		bool					m_BlendingEnabled;

		ComparisonFunc			m_AlphaFunc;
		EGL_Fixed				m_AlphaReference;

		BlendFuncSrc			m_BlendFuncSrc;
		BlendFuncDst			m_BlendFuncDst;

		ComparisonFunc			m_DepthFunc;

		LogicOp					m_LogicOpcode;

		// -------------------------------------------------------------------------
		// Stencil buffer not required by OpenGL|ES
		// -------------------------------------------------------------------------
		bool					m_StencilTestEnabled;

		ComparisonFunc			m_StencilFunc;
		I32						m_StencilReference;
		U32						m_StencilComparisonMask;
		U32						m_StencilMask;

		StencilOp				m_StencilFail, m_StencilZFail, m_StencilZPass;

		MultiTexture *			m_Texture;			// current texture 

		U32						m_Version;			// version numbers gets
													// incremented with each 
													// state change
	};


	// ----------------------------------------------------------------------
	// Inline member definitions
	// ----------------------------------------------------------------------

	inline U32 RasterizerState :: GetVersion() const { 
		return m_Version;
	}


	inline void RasterizerState :: VersionChanged() { 
		++m_Version; 
	};


	inline void RasterizerState :: SetTexture(MultiTexture * texture) {
		m_Texture = texture;
		VersionChanged();
	}

	inline void RasterizerState :: SetTexEnvColor(const FractionalColor& color) {
		m_TexEnvColor = color;
		VersionChanged();
	}

	inline void RasterizerState :: SetDepthRange(EGL_Fixed zNear, EGL_Fixed zFar) {
		m_DepthRangeNear = zNear;
		m_DepthRangeFar = zFar;
		VersionChanged();
	}

	inline void RasterizerState :: SetFogMode(FogMode mode) {
		m_FogMode = mode;
		VersionChanged();
	}

	inline void RasterizerState :: SetFogStart(EGL_Fixed start) {
		m_FogStart = start;
		VersionChanged();
	}

	inline void RasterizerState :: SetFogEnd(EGL_Fixed end) {
		m_FogEnd = end;
		VersionChanged();
	}

	inline void RasterizerState :: SetFogDensity(EGL_Fixed density) {
		m_FogDensity = density;
		VersionChanged();
	}


	inline void RasterizerState :: SetFogColor(const FractionalColor& color) {
		m_FogColor = color;
		VersionChanged();
	}

	inline void RasterizerState :: EnableFog(bool enabled) {
		m_FogEnabled = enabled;
		VersionChanged();
	}

	inline void RasterizerState :: SetLineWidth(EGL_Fixed width) {
		m_LineWidth = width;
		VersionChanged();
	}

	inline void RasterizerState :: SetLogicOp(LogicOp opcode) {
		m_LogicOpcode = opcode;
		VersionChanged();
	}

	inline void RasterizerState :: SetShadeModel(ShadingModel mode) {
		m_ShadingModel = mode;
		VersionChanged();
	}

	inline void RasterizerState :: EnableTexture(bool enabled) {
		m_TextureEnabled = enabled;
		VersionChanged();
	}

	inline void RasterizerState :: SetTextureMode(const TextureMode mode) {
		m_TextureMode = mode;
		VersionChanged();
	}

	inline void RasterizerState :: SetPointSize(EGL_Fixed size) {
		m_PointSize = size;
		VersionChanged();
	}

	inline void RasterizerState :: SetPolygonOffset(EGL_Fixed factor, EGL_Fixed units) {
		m_PolygonOffsetFactor = factor;
		m_PolygonOffsetUnits = units;
		VersionChanged();
	}

	inline void RasterizerState :: SetSampleCoverage(EGL_Fixed value, bool invert) {
		m_SampleCoverage = value;
		m_InvertSampleCoverage = invert;
		VersionChanged();
	}

	// ----------------------------------------------------------------------
	// Fragment rendering state
	// ----------------------------------------------------------------------

	inline void RasterizerState :: SetAlphaFunc(ComparisonFunc func, EGL_Fixed ref) {
		m_AlphaFunc = func;
		m_AlphaReference = ref;
		VersionChanged();
	}

	inline void RasterizerState :: SetBlendFunc(BlendFuncSrc sfactor, BlendFuncDst dfactor) {
		m_BlendFuncSrc = sfactor;
		m_BlendFuncDst = dfactor;
		VersionChanged();
	}

	inline void RasterizerState :: SetColorMask(bool red, bool green, bool blue, bool alpha) {
		m_MaskAlpha = alpha;
		m_MaskRed = red;
		m_MaskGreen = green;
		m_MaskBlue = blue;
		VersionChanged();
	}

	inline void RasterizerState :: SetDepthFunc(ComparisonFunc func) {
		m_DepthFunc = func;
		VersionChanged();
	}

	inline void RasterizerState :: SetDepthMask(bool flag) {
		m_MaskDepth = flag;
		VersionChanged();
	}

	inline void RasterizerState :: SetScissor(I32 x, I32 y, U32 width, U32 height) {
		m_ScissorX = x;
		m_ScissorY = y;
		m_ScissorWidth = width;
		m_ScissorHeight = height;
		VersionChanged();
	}

	inline void RasterizerState :: EnableAlphaTest(bool enabled) {
		m_AlphaTestEnabled = enabled;
		VersionChanged();
	}

	inline void RasterizerState :: EnableDepthTest(bool enabled) {
		m_DepthTestEnabled = enabled;
		VersionChanged();
	}

	inline void RasterizerState :: EnableLogicOp(bool enabled) {
		m_LogicOpEnabled = enabled;
		VersionChanged();
	}

	inline void RasterizerState :: EnableScissorTest(bool enabled) {
		m_ScissorTestEnabled = enabled;
		VersionChanged();
	}

	inline void RasterizerState :: EnableBlending(bool enabled) {
		m_BlendingEnabled = enabled;
		VersionChanged();
	}

	inline void RasterizerState :: SetStencilFunc(ComparisonFunc func, I32 ref, U32 mask) {
		m_StencilFunc = func;
		m_StencilReference = ref;
		m_StencilComparisonMask = mask;
		VersionChanged();
	}

	inline void RasterizerState :: SetStencilMask(U32 mask) {
		m_StencilMask = mask;
		VersionChanged();
	}

	inline void RasterizerState :: SetStencilOp(StencilOp fail, StencilOp zfail, StencilOp zpass) {
		m_StencilFail = fail;
		m_StencilZFail = zfail;
		m_StencilZPass = zpass;
		VersionChanged();
	}

	inline void RasterizerState :: EnableStencilTest(bool enabled) {
		m_StencilTestEnabled = enabled;
		VersionChanged();
	}

	inline void RasterizerState :: SetLineSmoothEnabled(bool enabled) {
		m_LineSmoothEnabled = enabled;
		VersionChanged();
	}

	inline void RasterizerState :: SetPointSmoothEnabled(bool enabled) {
		m_PointSmoothEnabled = enabled;
		VersionChanged();
	}

}

#endif //ndef EGL_RASTERIZER_STATE_H

