/*
File:   struct_color.h
Author: Taylor Robbins
Date:   01\05\2025
Description:
	** Holds the Color32 plex which is a 32-bit value that has 4 8-bit channels
	** The order of the channels 
*/

#ifndef _STRUCT_COLOR_H
#define _STRUCT_COLOR_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "std/std_basic_math.h"

//NOTE: The name "Color" conflicts with a definition in <windows.h>
typedef car Color32 Color32;
car Color32
{
	u32 valueU32;
	plex { u8 channel[4]; };
	plex { u8 b, g, r, a; };
	plex { u8 blue, green, red, alpha; };
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Color32 NewColorU32(u32 valueU32);
	PIG_CORE_INLINE Color32 NewColor(u8 red, u8 green, u8 blue, u8 alpha);
	PIG_CORE_INLINE Color32 ColorLerpSimple(Color32 start, Color32 end, r32 amount);
	PIG_CORE_INLINE Color32 ColorWithAlphaU8(Color32 rgbColor, u8 alpha);
	PIG_CORE_INLINE Color32 ColorWithAlpha(Color32 rgbColor, r32 alpha);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

//NOTE: This is BGRA order in memory, or in hex that's 0xAARRGGBB
PEXPI Color32 NewColorU32(u32 valueU32)
{
	Color32 result;
	result.valueU32 = valueU32;
	return result;
}
PEXPI Color32 NewColor(u8 red, u8 green, u8 blue, u8 alpha)
{
	Color32 result;
	result.red = red;
	result.green = green;
	result.blue = blue;
	result.alpha = alpha;
	return result;
}

// +--------------------------------------------------------------+
// |                     Basic Math Functions                     |
// +--------------------------------------------------------------+
//"Simple" meaning we don't do lerp in linear color space, we lerp in Gamma sRGB space, which is innaccurate but simple to do
PEXPI Color32 ColorLerpSimple(Color32 start, Color32 end, r32 amount)
{
	return NewColor(
		ClampCastI32ToU8(RoundR32i((r32)start.r + (r32)(end.r - start.r) * amount)),
		ClampCastI32ToU8(RoundR32i((r32)start.g + (r32)(end.g - start.g) * amount)),
		ClampCastI32ToU8(RoundR32i((r32)start.b + (r32)(end.b - start.b) * amount)),
		ClampCastI32ToU8(RoundR32i((r32)start.a + (r32)(end.a - start.a) * amount))
	);
}

PEXPI Color32 ColorWithAlphaU8(Color32 rgbColor, u8 alpha)
{
	return NewColor(rgbColor.r, rgbColor.g, rgbColor.b, alpha);
}
PEXPI Color32 ColorWithAlpha(Color32 rgbColor, r32 alpha)
{
	return ColorWithAlphaU8(rgbColor, ClampCastI32ToU8(RoundR32i(alpha * 255)));
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_COLOR_H

#if defined(_STRUCT_COLOR_H) && defined(_STRUCT_VECTORS_H)
#include "cross/cross_color_and_vectors.h"
#endif

#if defined(_MISC_PARSING_H) && defined(_STRUCT_COLOR_H)
#include "cross/cross_parsing_and_color.h"
#endif

#if defined(_STRUCT_COLOR_H) && defined(RAYLIB_H)
#include "cross/cross_color_and_raylib.h"
#endif
