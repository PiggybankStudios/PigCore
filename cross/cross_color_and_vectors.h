/*
File:   cross_color_and_vectors.h
Author: Taylor Robbins
Date:   01\29\2025
*/

#ifndef _CROSS_COLOR_AND_VECTORS_H
#define _CROSS_COLOR_AND_VECTORS_H

//NOTE: Intentionally no includes here

typedef v4 Colorf;

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE v4 ToV4FromColor32(Color32 color);
	PIG_CORE_INLINE Color32 ToColor32FromV4(v4 vector);
	PIG_CORE_INLINE v4r ToV4rFromColor32(Color32 color);
	PIG_CORE_INLINE Color32 ToColor32FromV4r(v4r vector);
	PIG_CORE_INLINE r32 ToLinearFromGammaR32(r32 channelValue);
	PIG_CORE_INLINE r32 ToGammaFromLinearR32(r32 channelValue);
	Colorf ToLinearFromGamma(Colorf color);
	Colorf ToGammaFromLinear(Colorf color);
	PIG_CORE_INLINE Colorf ToLinearFromGamma32(Color32 gammaColor);
	PIG_CORE_INLINE Color32 ToGamma32FromLinear(Colorf linearColor);
	PIG_CORE_INLINE Color32 Color32Lerp(Color32 left, Color32 right, r32 amount);
	PIG_CORE_INLINE Color32 ColorAverage2(Color32 color1, Color32 color2);
	PIG_CORE_INLINE Color32 ColorAverage3(Color32 color1, Color32 color2, Color32 color3);
	PIG_CORE_INLINE Color32 ColorAverage4(Color32 color1, Color32 color2, Color32 color3, Color32 color4);
	PIG_CORE_INLINE Colorf LinearColorLerp(Colorf left, Colorf right, r32 amount);
	PIG_CORE_INLINE Colorf LinearColorAverage2(Colorf color1, Colorf color2);
	PIG_CORE_INLINE Colorf LinearColorAverage3(Colorf color1, Colorf color2, Colorf color3);
	PIG_CORE_INLINE Colorf LinearColorAverage4(Colorf color1, Colorf color2, Colorf color3, Colorf color4);
	#if TARGET_IS_ORCA
	PIG_CORE_INLINE oc_color ToOcColorFromColor32(Color32 color);
	PIG_CORE_INLINE Color32 ToColor32FromOcColor(oc_color orcaColor);
	#endif //TARGET_IS_ORCA
#endif

#define NewColorf(r, g, b, a)   NewV4((r), (g), (b), (a))
#define ToColorfFrom32(color32) ToV4FromColor32(color32)
#define ToColor32Fromf(color)   ToColor32FromV4(color)

#define Colorf_NoColor                 V4_Zero
#define Colorf_TransparentBlack        V4_Zero
#define Colorf_Transparent             NewColorf(1.0f, 1.0f, 1.0f, 0.0f)
#define Colorf_TransparentWhite        NewColorf(1.0f, 1.0f, 1.0f, 0.0f)

#define TO_LINEAR_FROM_GAMMA_R32(channelValue) (((channelValue) < 0.04045f)   ? (channelValue) * 0.0773993808f :           PowR32((channelValue) * 0.9478672986f + 0.0521327014f, 2.4f))
#define TO_GAMMA_FROM_LINEAR_R32(channelValue) (((channelValue) < 0.0031308f) ? (channelValue) * 12.92f        : (1.055f * PowR32((channelValue), 0.41666f)) - 0.055f)
#define TO_LINEAR_FROM_GAMMA_R64(channelValue) (((channelValue) < 0.04045)    ? (channelValue) * 0.0773993808  :           PowR64((channelValue) * 0.9478672986 + 0.0521327014, 2.4))
#define TO_GAMMA_FROM_LINEAR_R64(channelValue) (((channelValue) < 0.0031308)  ? (channelValue) * 12.92         : (1.055  * PowR64((channelValue), 0.41666)) - 0.055)

#define TO_LINEAR_FROM_GAMMA_R32_FAST(channelValue) (((channelValue) < 0.04045f)   ? (channelValue) * 0.0773993808f :           PowR32((channelValue) * 0.9478672986f + 0.0521327014f, 2.0f))
#define TO_GAMMA_FROM_LINEAR_R32_FAST(channelValue) (((channelValue) < 0.0031308f) ? (channelValue) * 12.92f        : (1.055f * PowR32((channelValue), 0.5f)) - 0.055f)
	
// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI v4 ToV4FromColor32(Color32 color)
{
	return NewV4(
		(r32)color.r / 255.0f,
		(r32)color.g / 255.0f,
		(r32)color.b / 255.0f,
		(r32)color.a / 255.0f
	);
}
PEXPI Color32 ToColor32FromV4(v4 vector)
{
	return NewColor(
		ClampCastI32ToU8(RoundR32i(vector.X * 255.0f)),
		ClampCastI32ToU8(RoundR32i(vector.Y * 255.0f)),
		ClampCastI32ToU8(RoundR32i(vector.Z * 255.0f)),
		ClampCastI32ToU8(RoundR32i(vector.W * 255.0f))
	);
}

PEXPI v4r ToV4rFromColor32(Color32 color)
{
	return NewV4r(
		(r32)color.r / 255.0f,
		(r32)color.g / 255.0f,
		(r32)color.b / 255.0f,
		(r32)color.a / 255.0f
	);
}
PEXPI Color32 ToColor32FromV4r(v4r vector)
{
	return NewColor(
		ClampCastI32ToU8(RoundR32i(vector.X * 255.0f)),
		ClampCastI32ToU8(RoundR32i(vector.Y * 255.0f)),
		ClampCastI32ToU8(RoundR32i(vector.Z * 255.0f)),
		ClampCastI32ToU8(RoundR32i(vector.W * 255.0f))
	);
}

PEXPI r32 ToLinearFromGammaR32(r32 channelValue)
{
	return (channelValue < 0.04045f)
		? channelValue * 0.0773993808f
		: PowR32(channelValue * 0.9478672986f + 0.0521327014f, 2.4f);
}
PEXPI r32 ToGammaFromLinearR32(r32 channelValue)
{
	return (channelValue < 0.0031308f)
		? channelValue * 12.92f
		: (1.055f * PowR32(channelValue, 0.41666f)) - 0.055f;
}
PEXP Colorf ToLinearFromGamma(Colorf color)
{
	return NewColorf(
		ToLinearFromGammaR32(color.R),
		ToLinearFromGammaR32(color.G),
		ToLinearFromGammaR32(color.B),
		color.A
	);
}
PEXP Colorf ToGammaFromLinear(Colorf color)
{
	return NewColorf(
		ToGammaFromLinearR32(color.R),
		ToGammaFromLinearR32(color.G),
		ToGammaFromLinearR32(color.B),
		color.A
	);
}
PEXPI Colorf ToLinearFromGamma32(Color32 gammaColor)
{
	return ToLinearFromGamma(ToColorfFrom32(gammaColor));
}
PEXPI Color32 ToGamma32FromLinear(Colorf linearColor)
{
	return ToColor32Fromf(ToGammaFromLinear(linearColor));
}

PEXPI Color32 Color32Lerp(Color32 left, Color32 right, r32 amount)
{
	Colorf linearLeft = ToLinearFromGamma32(left);
	Colorf linearRight = ToLinearFromGamma32(right);
	Colorf resultLinear = LerpV4(linearLeft, linearRight, amount);
	return ToGamma32FromLinear(resultLinear);
}
PEXPI Color32 ColorAverage2(Color32 color1, Color32 color2)
{
	Colorf linearColor1 = ToLinearFromGamma32(color1);
	Colorf linearColor2 = ToLinearFromGamma32(color2);
	Colorf resultLinear = LerpV4(linearColor1, linearColor2, 0.5f);
	return ToGamma32FromLinear(resultLinear);
}
PEXPI Color32 ColorAverage3(Color32 color1, Color32 color2, Color32 color3)
{
	Colorf linearColor1 = ToLinearFromGamma32(color1);
	Colorf linearColor2 = ToLinearFromGamma32(color2);
	Colorf linearColor3 = ToLinearFromGamma32(color3);
	Colorf resultLinear = LerpV4(linearColor1, LerpV4(linearColor2, linearColor3, 0.5f), 0.5f);
	return ToGamma32FromLinear(resultLinear);
}
PEXPI Color32 ColorAverage4(Color32 color1, Color32 color2, Color32 color3, Color32 color4)
{
	Colorf linearColor1 = ToLinearFromGamma32(color1);
	Colorf linearColor2 = ToLinearFromGamma32(color2);
	Colorf linearColor3 = ToLinearFromGamma32(color3);
	Colorf linearColor4 = ToLinearFromGamma32(color4);
	Colorf resultLinear = LerpV4(LerpV4(linearColor1, linearColor2, 0.5f), LerpV4(linearColor3, linearColor4, 0.5f), 0.5f);
	return ToGamma32FromLinear(resultLinear);
}

PEXPI Colorf LinearColorLerp(Colorf left, Colorf right, r32 amount) { return LerpV4(left, right, amount); }
PEXPI Colorf LinearColorAverage2(Colorf color1, Colorf color2) { return LerpV4(color1, color2, 0.5f); }
PEXPI Colorf LinearColorAverage3(Colorf color1, Colorf color2, Colorf color3) { return LerpV4(color1, LerpV4(color2, color3, 0.5f), 0.5f); }
PEXPI Colorf LinearColorAverage4(Colorf color1, Colorf color2, Colorf color3, Colorf color4) { return LerpV4(LerpV4(color1, color2, 0.5f), LerpV4(color3, color4, 0.5f), 0.5f); }

#if TARGET_IS_ORCA
PEXPI oc_color ToOcColorFromColor32(Color32 color) { return ToOcColorFromV4r(ToV4rFromColor32(color)); }
PEXPI Color32 ToColor32FromOcColor(oc_color orcaColor) { return ToColor32FromV4r(ToV4rFromOcColor(orcaColor)); }
#endif //TARGET_IS_ORCA

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_COLOR_AND_VECTORS_H
