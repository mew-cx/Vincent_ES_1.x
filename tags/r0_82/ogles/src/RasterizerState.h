#ifndef EGL_RASTERIZER_STATE_H
#define EGL_RASTERIZER_STATE_H 1

// ==========================================================================
//
// RasterizerState	Rasterizer State Class for 3D Rendering Library
//
//					The rasterizer state maintains all the state information
//					that is necessary to determine the details of the
//					next primitive to be scan-converted.
//
// --------------------------------------------------------------------------
//
// 10-10-2003		Hans-Martin Will	initial version
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
#include "Color.h"


namespace EGL {
	class Rasterizer;
	class MultiTexture;

	class RasterizerState {

		friend class Rasterizer;
		friend class CodeGenerator;

	public:
		enum Limits {
			LogMaxTextureSize = 12,
			MaxTextureSize = 1 << LogMaxTextureSize
		};

		enum LogicOp {
			LogicOpInvalid = -1,
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
			CompFuncInvalid = -1,
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
			BlendFuncDstInvalid = -1,
			BlendFuncDstZero,
			BlendFuncDstOne,
			BlendFuncDstSrcColor,
			BlendFuncDstOneMinusSrcColor,
			BlendFuncDstSrcAlpha,
			BlendFuncDstOneMinusSrcAlpha,
			BlendFuncDstDstAlpha,
			BlendFuncDstOneMinusDstAlpha
		};

		enum BlendFuncSrc {
			BlendFuncSrcInvalid = -1,
			BlendFuncSrcZero,
			BlendFuncSrcOne,
			BlendFuncSrcDstColor,
			BlendFuncSrcOneMinusDstColor,
			BlendFuncSrcSrcAlpha,
			BlendFuncSrcOneMinusSrcAlpha,
			BlendFuncSrcDstAlpha,
			BlendFuncSrcOneMinusDstAlpha,
			BlendFuncSrcSrcAlphaSaturate
		};

		enum StencilOp {
			StencilOpInvalid = -1,
			StencilOpZero,
			StencilOpKeep,
			StencilOpReplace,
			StencilOpIncr,
			StencilOpDecr,
			StencilOpInvert
		};

		enum ShadingModel {
			ShadeModelInvalid = -1,
			ShadeModelFlat,
			ShadeModelSmooth
		};

		enum TextureMode {
			TextureModeInvalid = -1,
			TextureModeDecal,
			TextureModeReplace,
			TextureModeBlend,
			TextureModeAdd,
			TextureModeModulate
		};

		enum WrappingMode {
			WrappingModeInvalid = -1,
			WrappingModeClampToEdge,
			WrappingModeRepeat
		};

		enum FilterMode {
			FilterModeInvalid = -1,
			FilterModeNone,
			FilterModeNearest,
			FilterModeLinear
		};

		enum TextureFormat {
			TextureFormatInvalid = -1,
			TextureFormatAlpha = 0,				// 8
			TextureFormatLuminance = 1,			// 8
			TextureFormatLuminanceAlpha = 2,	// 8-8
			TextureFormatRGB8 = 3,				// 8-8-8
			TextureFormatRGBA8 = 4,				// 8-8-8-8
			TextureFormatRGB565 = 5,			// 5-6-5
			TextureFormatRGBA4444 = 6,			// 4-4-4-4
			TextureFormatRGBA5551 = 7			// 5-5-5-1
		};

	public:
		RasterizerState();

		bool CompareCommon(const RasterizerState& other) const;
		bool ComparePoint(const RasterizerState& other) const;
		bool CompareLine(const RasterizerState& other) const;
		bool ComparePolygon(const RasterizerState& other) const;

		typedef bool (RasterizerState::*CompareFunction)(const RasterizerState& other) const;

		// ----------------------------------------------------------------------
		// Primitive rendering state
		// ----------------------------------------------------------------------

		void SetTexEnvColor(const Color& color);
		void SetTextureMode(TextureMode mode);

		void SetMinFilterMode(FilterMode mode);
		void SetMagFilterMode(FilterMode mode);
		void SetMipmapFilterMode(FilterMode mode);
		void SetWrappingModeS(WrappingMode mode);
		void SetWrappingModeT(WrappingMode mode);

		FilterMode GetMinFilterMode() const			{ return m_Texture.MinFilterMode; }
		FilterMode GetMagFilterMode() const			{ return m_Texture.MagFilterMode; }
		FilterMode GetMipmapFilterMode() const		{ return m_Texture.MipmapFilterMode; }
		WrappingMode GetWrappingModeS() const		{ return m_Texture.WrappingModeS; }
		WrappingMode GetWrappingModeT() const		{ return m_Texture.WrappingModeT; }

		void SetInternalFormat(TextureFormat format);

		void SetDepthRange(EGL_Fixed zNear, EGL_Fixed zFar);

		void SetFogColor(const Color& color);
		void EnableFog(bool enabled);
		bool IsEnabledFog() const;

		void SetLineWidth(EGL_Fixed width);
		void SetLineSmoothEnabled(bool enabled);

		void SetLogicOp(LogicOp opcode);

		void SetShadeModel(ShadingModel mode);
		ShadingModel GetShadeModel() const;

		void EnableTexture(bool enabled);

		void SetPointSmoothEnabled(bool enabled);
		void SetPointSpriteEnabled(bool enabled);
		void SetPointCoordReplaceEnabled(bool enabled);

		void EnablePolygonOffsetFill(bool enabled);
		void SetPolygonOffset(EGL_Fixed factor, EGL_Fixed units);
		void SetSampleCoverage(EGL_Fixed value, bool invert);

		// ----------------------------------------------------------------------
		// Fragment rendering state
		// ----------------------------------------------------------------------

		void SetAlphaFunc(ComparisonFunc func, EGL_Fixed ref);
		void SetBlendFunc(BlendFuncSrc sfactor, BlendFuncDst dfactor);
		void SetColorMask(bool red, bool green, bool blue, bool alpha);
		Color GetColorMask() const;

		void SetDepthFunc(ComparisonFunc func);
		void SetDepthMask(bool flag);
		bool GetDepthMask() const;

		void SetScissor(I32 x, I32 y, U32 width, U32 height);

		void EnableAlphaTest(bool enabled);
		void EnableDepthTest(bool enabled);
		void EnableLogicOp(bool enabled);
		void EnableScissorTest(bool enabled);
		void EnableBlending(bool enabled);

		void SetStencilFunc(ComparisonFunc func, I32 ref, U32 mask);
		void SetStencilMask(U32 mask);
		U32 GetStencilMask() const;

		void SetStencilOp(StencilOp fail, StencilOp zfail, StencilOp zpass);
		void EnableStencilTest(bool enabled);

	private:
		// ----------------------------------------------------------------------
		// Primitve specific rendering state
		// ----------------------------------------------------------------------

		struct PointState {
			PointState() {
				SmoothEnabled = false;
				SpriteEnabled = false;
				CoordReplaceEnabled = false;
			}

			PointState(const PointState& other) {
				SmoothEnabled = other.SmoothEnabled;
				SpriteEnabled = other.SpriteEnabled;
				CoordReplaceEnabled = other.CoordReplaceEnabled;
			}

			PointState& operator=(const PointState& other) {
				SmoothEnabled = other.SmoothEnabled;
				SpriteEnabled = other.SpriteEnabled;
				CoordReplaceEnabled = other.CoordReplaceEnabled;
				return *this;
			}

			bool operator==(const PointState& other) const {
				return	SmoothEnabled == other.SmoothEnabled &&
					SpriteEnabled == other.SpriteEnabled &&
					(!SpriteEnabled ||
					 CoordReplaceEnabled == other.CoordReplaceEnabled);
			}

			bool				SmoothEnabled;
			bool				SpriteEnabled;
			bool				CoordReplaceEnabled;
		}
								m_Point;

		struct LineState {
			LineState() {
				SmoothEnabled = false;
				Width = EGL_ONE;
			}

			LineState(const LineState& other) {
				SmoothEnabled = other.SmoothEnabled;
				Width = other.Width;
			}

			LineState& operator=(const LineState& other) {
				SmoothEnabled = other.SmoothEnabled;
				Width = other.Width;
				return *this;
			}

			bool operator==(const LineState& other) const {
				return SmoothEnabled == other.SmoothEnabled &&
					Width == other.Width;
			}

			bool				SmoothEnabled;
			EGL_Fixed			Width;	// current line width
		}
								m_Line;


		struct PolygonState {
			PolygonState() {
				OffsetFillEnabled = false;
				OffsetFactor = 0;
				OffsetUnits = 0;
			}

			PolygonState(const PolygonState& other) {
				OffsetFillEnabled = other.OffsetFillEnabled;
				OffsetFactor = other.OffsetFactor;
				OffsetUnits = other.OffsetUnits;
			}

			PolygonState& operator=(const PolygonState& other) {
				OffsetFillEnabled = other.OffsetFillEnabled;
				OffsetFactor = other.OffsetFactor;
				OffsetUnits = other.OffsetUnits;
				return *this;
			}

			bool operator==(const PolygonState& other) const {
				return OffsetFillEnabled == other.OffsetFillEnabled &&
					(!OffsetFillEnabled ||
					 OffsetFactor == other.OffsetFactor &&
					 OffsetUnits == other.OffsetUnits);
			}

			bool				OffsetFillEnabled;
			EGL_Fixed			OffsetFactor;
			EGL_Fixed			OffsetUnits;
		}
								m_Polygon;

		// ----------------------------------------------------------------------
		// General rendering state
		// ----------------------------------------------------------------------

		struct FogState {
			FogState() {
				Enabled = false;
				Color = EGL::Color(0, 0, 0, 0);

			}

			FogState(const FogState& other) {
				Enabled = other.Enabled;
				Color = other.Color;
			}

			FogState& operator=(const FogState& other) {
				Enabled = other.Enabled;
				Color = other.Color;
				return *this;
			}

			bool operator==(const FogState& other) const {
				return Enabled == other.Enabled &&
					(!Enabled || Color == other.Color);
			}

			bool				Enabled;
			Color				Color;
		}
								m_Fog;

		struct TextureState {
			TextureState() {
				Enabled = false;
				EnvColor = Color(0, 0, 0, 0);
				Mode = TextureModeModulate;
				MinFilterMode = FilterModeNearest;
				MagFilterMode = FilterModeNearest;
				MipmapFilterMode = FilterModeNone;
				WrappingModeS = WrappingModeRepeat;
				WrappingModeT = WrappingModeRepeat;
				InternalFormat = TextureFormatLuminance;
			}

			TextureState(const TextureState& other) {
				Enabled = other.Enabled;
				EnvColor = other.EnvColor;
				Mode = other.Mode;
				MinFilterMode = other.MinFilterMode;
				MagFilterMode = other.MagFilterMode;
				MipmapFilterMode = other.MipmapFilterMode;
				WrappingModeS = other.WrappingModeS;
				WrappingModeT = other.WrappingModeT;
				InternalFormat = other.InternalFormat;
			}

			TextureState& operator=(const TextureState& other) {
				Enabled = other.Enabled;
				EnvColor = other.EnvColor;
				Mode = other.Mode;
				MinFilterMode = other.MinFilterMode;
				MagFilterMode = other.MagFilterMode;
				MipmapFilterMode = other.MipmapFilterMode;
				WrappingModeS = other.WrappingModeS;
				WrappingModeT = other.WrappingModeT;
				InternalFormat = other.InternalFormat;
				return *this;
			}

			bool operator==(const TextureState& other) const {
				return Enabled == other.Enabled &&
					(!Enabled ||
					 EnvColor == other.EnvColor &&
					 Mode == other.Mode &&
					 MinFilterMode == other.MinFilterMode &&
					 MagFilterMode == other.MagFilterMode &&
					 MipmapFilterMode == other.MipmapFilterMode &&
					 WrappingModeS == other.WrappingModeS &&
					 WrappingModeT == other.WrappingModeT &&
					 InternalFormat == other.InternalFormat);
			}

			bool				Enabled;
			Color				EnvColor;
			TextureMode			Mode;
			FilterMode			MinFilterMode;
			FilterMode			MagFilterMode;
			FilterMode			MipmapFilterMode;
			WrappingMode		WrappingModeS;
			WrappingMode		WrappingModeT;
			TextureFormat		InternalFormat;
		}
								m_Texture;

		struct ScissorTestState {
			ScissorTestState() {
				Enabled = false;
				X = 0;
				Y = 0;
				Width = 1024;
				Height = 1024;
			}

			ScissorTestState(const ScissorTestState& other) {
				Enabled = other.Enabled;
				X = other.X;
				Y = other.Y;
				Width = other.Width;
				Height = other.Height;
			}

			ScissorTestState& operator=(const ScissorTestState& other) {
				Enabled = other.Enabled;
				X = other.X;
				Y = other.Y;
				Width = other.Width;
				Height = other.Height;
				return *this;
			}

			bool operator==(const ScissorTestState& other) const {
				return Enabled == other.Enabled &&
					(!Enabled ||
					 X == other.X && Y == other.Y &&
					 Width == other.Width && Height == other.Height);
			}

			bool				Enabled;
			I32					X, Y;
			U32					Width, Height;
		}
								m_ScissorTest;

		struct MaskState {
			MaskState() {
				Red = true;
				Blue = true;
				Green = true;
				Alpha = true;
				Depth = true;
			}

			MaskState(const MaskState& other) {
				Red = other.Red;
				Blue = other.Blue;
				Green = other.Green;
				Alpha = other.Alpha;
				Depth = other.Depth;
			}

			MaskState& operator=(const MaskState& other) {
				Red = other.Red;
				Blue = other.Blue;
				Green = other.Green;
				Alpha = other.Alpha;
				Depth = other.Depth;
				return *this;
			}

			bool operator==(const MaskState& other) const {
				return Red == other.Red &&
					Green == other.Green &&
					Blue == other.Blue &&
					Alpha == other.Alpha &&
					Depth == other.Depth;
			}

			bool				Red;	// if true, write red channel
			bool				Green;	// if true, write green channel
			bool				Blue;	// if true, write blue channel
			bool				Alpha;	// if true, write alpha channel
			bool				Depth;	// if true, write depth buffer
		}
								m_Mask;

		struct AlphaState {
			AlphaState() {
				Enabled = false;
				Func = CompFuncAlways;
				Reference = 0; 
			}

			AlphaState(const AlphaState& other) {
				Enabled = other.Enabled;
				Func = other.Func;
				Reference = other.Reference; 
			}

			AlphaState& operator=(const AlphaState& other) {
				Enabled = other.Enabled;
				Func = other.Func;
				Reference = other.Reference; 
				return *this;
			}

			bool operator==(const AlphaState& other) const {
				return Enabled == other.Enabled &&
					(!Enabled ||
					 Func == other.Func &&
					 Reference == other.Reference);
			}

			bool				Enabled;
			ComparisonFunc		Func;
			EGL_Fixed			Reference;
		}
								m_Alpha;

		struct BlendState {
			BlendState() {
				Enabled = false;
				FuncSrc = BlendFuncSrcOne;
				FuncDst = BlendFuncDstZero;
			}

			BlendState(const BlendState& other) {
				Enabled = other.Enabled;
				FuncSrc = other.FuncSrc;
				FuncDst = other.FuncDst;
			}

			BlendState& operator=(const BlendState& other) {
				Enabled = other.Enabled;
				FuncSrc = other.FuncSrc;
				FuncDst = other.FuncDst;
				return *this;
			}

			bool operator==(const BlendState& other) const {
				return Enabled == other.Enabled &&
					(!Enabled ||
					 FuncSrc == other.FuncSrc && FuncDst == other.FuncDst);
			}

			bool				Enabled;
			BlendFuncSrc		FuncSrc;
			BlendFuncDst		FuncDst;
		}
								m_Blend;

		struct DepthTestState {
			DepthTestState() {
				Enabled = false;
				Func = CompFuncLess;
			}

			DepthTestState(const DepthTestState& other) {
				Enabled = other.Enabled;
				Func = other.Func;
			}

			DepthTestState& operator=(const DepthTestState& other) {
				Enabled = other.Enabled;
				Func = other.Func;
				return *this;
			}

			bool operator==(const DepthTestState& other) const {
				return Enabled == other.Enabled &&
					(!Enabled || Func == other.Func);
			}

			bool				Enabled;
			ComparisonFunc		Func;
		}
								m_DepthTest;

		struct LogicOpState {
			LogicOpState() {
				Enabled = false;
				Opcode = LogicOpCopy;
			}

			LogicOpState(const LogicOpState& other) {
				Enabled = other.Enabled;
				Opcode = other.Opcode;
			}

			LogicOpState& operator=(const LogicOpState& other) {
				Enabled = other.Enabled;
				Opcode = other.Opcode;
				return *this;
			}

			bool operator==(const LogicOpState& other) const {
				return Enabled == other.Enabled &&
					(!Enabled || Opcode == other.Opcode);
			}

			bool				Enabled;
			LogicOp				Opcode;
		}
								m_LogicOp;

		struct StencilState {
			StencilState() {
				Enabled = false;
				Func = CompFuncAlways;
				Reference = 0;
				Mask = 0xFFFFFFFFu;
				ComparisonMask = 0xFFFFFFFFu;
				Fail = StencilOpKeep;
				ZFail = StencilOpKeep;
				ZPass = StencilOpKeep;
			}

			StencilState(const StencilState& other) {
				Enabled = other.Enabled;
				Func = other.Func;
				Reference = other.Reference;
				ComparisonMask = other.ComparisonMask;
				Mask = other.Mask;
				Fail = other.Fail;
				ZFail = other.ZFail;
				ZPass = other.ZPass;
			}

			StencilState& operator=(const StencilState& other) {
				Enabled = other.Enabled;
				Func = other.Func;
				Reference = other.Reference;
				Mask = other.Mask;
				ComparisonMask = other.ComparisonMask;
				Fail = other.Fail;
				ZFail = other.ZFail;
				ZPass = other.ZPass;
				return *this;
			}

			bool operator==(const StencilState& other) const {
				return Enabled == other.Enabled &&
					(!Enabled ||
					 Func == other.Func &&
					 Reference == other.Reference &&
					 Mask == other.Mask &&
					 ComparisonMask == other.ComparisonMask &&
					 Fail == other.Fail &&
					 ZFail == other.ZFail &&
					 ZPass == other.ZPass);
			}

			bool				Enabled;
			ComparisonFunc		Func;
			I32					Reference;
			U32					ComparisonMask;
			U32					Mask;

			StencilOp			Fail, ZFail, ZPass;
		}
								m_Stencil;

		ShadingModel			m_ShadingModel;

		EGL_Fixed				m_SampleCoverage;
		bool					m_InvertSampleCoverage;

	};


	// ----------------------------------------------------------------------
	// Inline member definitions
	// ----------------------------------------------------------------------


	inline void RasterizerState :: SetTexEnvColor(const Color& color) {
		m_Texture.EnvColor = color;
	}

	inline void RasterizerState :: SetTextureMode(TextureMode mode) {
		m_Texture.Mode = mode;
	}

	inline void RasterizerState :: SetMinFilterMode(FilterMode mode) {
		m_Texture.MinFilterMode = mode;
	}

	inline void RasterizerState :: SetMagFilterMode(FilterMode mode) {
		m_Texture.MagFilterMode = mode;
	}

	inline void RasterizerState :: SetMipmapFilterMode(FilterMode mode) {
		m_Texture.MipmapFilterMode = mode;
	}

	inline void RasterizerState :: SetWrappingModeS(WrappingMode mode) {
		m_Texture.WrappingModeS = mode;
	}

	inline void RasterizerState :: SetWrappingModeT(WrappingMode mode) {
		m_Texture.WrappingModeT = mode;
	}

	inline void RasterizerState :: SetInternalFormat(TextureFormat format) {
		m_Texture.InternalFormat = format;
	}

	inline void RasterizerState :: SetFogColor(const Color& color) {
		m_Fog.Color = color;
	}

	inline void RasterizerState :: EnableFog(bool enabled) {
		m_Fog.Enabled = enabled;
	}

	inline void RasterizerState :: SetLineWidth(EGL_Fixed width) {
		m_Line.Width = width;
	}

	inline void RasterizerState :: SetLogicOp(LogicOp opcode) {
		m_LogicOp.Opcode = opcode;
	}

	inline void RasterizerState :: SetShadeModel(ShadingModel mode) {
		m_ShadingModel = mode;
	}

	inline RasterizerState::ShadingModel RasterizerState :: GetShadeModel() const {
		return m_ShadingModel;
	}

	inline void RasterizerState :: EnableTexture(bool enabled) {
		m_Texture.Enabled = enabled;
	}

	inline void RasterizerState :: EnablePolygonOffsetFill(bool enabled) {
		m_Polygon.OffsetFillEnabled = enabled;
	}

	inline void RasterizerState :: SetPolygonOffset(EGL_Fixed factor, EGL_Fixed units) {
		m_Polygon.OffsetFactor = factor;
		m_Polygon.OffsetUnits = units;
	}

	inline void RasterizerState :: SetSampleCoverage(EGL_Fixed value, bool invert) {
		m_SampleCoverage = value;
		m_InvertSampleCoverage = invert;
	}

	// ----------------------------------------------------------------------
	// Fragment rendering state
	// ----------------------------------------------------------------------

	inline void RasterizerState :: SetAlphaFunc(ComparisonFunc func, EGL_Fixed ref) {
		m_Alpha.Func = func;
		m_Alpha.Reference = ref;
	}

	inline void RasterizerState :: SetBlendFunc(BlendFuncSrc sfactor, BlendFuncDst dfactor) {
		m_Blend.FuncSrc = sfactor;
		m_Blend.FuncDst = dfactor;
	}

	inline void RasterizerState :: SetColorMask(bool red, bool green, bool blue, bool alpha) {
		m_Mask.Alpha = alpha;
		m_Mask.Red = red;
		m_Mask.Green = green;
		m_Mask.Blue = blue;
	}

	inline Color RasterizerState :: GetColorMask() const {
		return Color(m_Mask.Red		? 0xff : 0,
					 m_Mask.Green	? 0xff : 0,
					 m_Mask.Blue	? 0xff : 0,
					 m_Mask.Alpha	? 0xff : 0);
	}

	inline bool RasterizerState :: GetDepthMask() const {
		return m_Mask.Depth;
	}

	inline U32 RasterizerState :: GetStencilMask() const {
		return m_Stencil.Mask;
	}

	inline void RasterizerState :: SetDepthFunc(ComparisonFunc func) {
		m_DepthTest.Func = func;
	}

	inline void RasterizerState :: SetDepthMask(bool flag) {
		m_Mask.Depth = flag;
	}

	inline void RasterizerState :: SetScissor(I32 x, I32 y, U32 width, U32 height) {
		m_ScissorTest.X = x;
		m_ScissorTest.Y = y;
		m_ScissorTest.Width = width;
		m_ScissorTest.Height = height;
	}

	inline void RasterizerState :: EnableAlphaTest(bool enabled) {
		m_Alpha.Enabled = enabled;
	}

	inline void RasterizerState :: EnableDepthTest(bool enabled) {
		m_DepthTest.Enabled = enabled;
	}

	inline void RasterizerState :: EnableLogicOp(bool enabled) {
		m_LogicOp.Enabled = enabled;
	}

	inline void RasterizerState :: EnableScissorTest(bool enabled) {
		m_ScissorTest.Enabled = enabled;
	}

	inline void RasterizerState :: EnableBlending(bool enabled) {
		m_Blend.Enabled = enabled;
	}

	inline void RasterizerState :: SetStencilFunc(ComparisonFunc func, I32 ref, U32 mask) {
		m_Stencil.Func = func;
		m_Stencil.Reference = ref;
		m_Stencil.ComparisonMask = mask;
	}

	inline void RasterizerState :: SetStencilMask(U32 mask) {
		m_Stencil.Mask = mask;
	}

	inline void RasterizerState :: SetStencilOp(StencilOp fail, StencilOp zfail, StencilOp zpass) {
		m_Stencil.Fail = fail;
		m_Stencil.ZFail = zfail;
		m_Stencil.ZPass = zpass;
	}

	inline void RasterizerState :: EnableStencilTest(bool enabled) {
		m_Stencil.Enabled = enabled;
	}

	inline void RasterizerState :: SetLineSmoothEnabled(bool enabled) {
		m_Line.SmoothEnabled = enabled;
	}

	inline void RasterizerState :: SetPointSmoothEnabled(bool enabled) {
		m_Point.SmoothEnabled = enabled;
	}

	inline void RasterizerState :: SetPointSpriteEnabled(bool enabled) {
		m_Point.SpriteEnabled = enabled;
	}

	inline void RasterizerState :: SetPointCoordReplaceEnabled(bool enabled) {
		m_Point.CoordReplaceEnabled = enabled;
	}

	inline bool RasterizerState :: IsEnabledFog() const {
		return m_Fog.Enabled;
	}

}

#endif //ndef EGL_RASTERIZER_STATE_H

