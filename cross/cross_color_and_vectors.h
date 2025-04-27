/*
File:   cross_color_and_vectors.h
Author: Taylor Robbins
Date:   01\29\2025
*/

#ifndef _CROSS_COLOR_AND_VECTORS_H
#define _CROSS_COLOR_AND_VECTORS_H

#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE v4 ToV4FromColor32(Color32 color);
	PIG_CORE_INLINE Color32 ToColor32FromV4(v4 vector);
	PIG_CORE_INLINE v4r ToV4rFromColor32(Color32 color);
	PIG_CORE_INLINE Color32 ToColor32FromV4r(v4r vector);
	#if TARGET_IS_ORCA
	PIG_CORE_INLINE oc_color ToOcColorFromColor32(Color32 color);
	PIG_CORE_INLINE Color32 ToColor32FromOcColor(oc_color orcaColor);
	#endif //TARGET_IS_ORCA
#endif

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

#if TARGET_IS_ORCA
PEXPI oc_color ToOcColorFromColor32(Color32 color) { return ToOcColorFromV4r(ToV4rFromColor32(color)); }
PEXPI Color32 ToColor32FromOcColor(oc_color orcaColor) { return ToColor32FromV4r(ToV4rFromOcColor(orcaColor)); }
#endif //TARGET_IS_ORCA

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_COLOR_AND_VECTORS_H
