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
#include "base/base_macros.h"
#include "std/std_basic_math.h"

//NOTE: The name "Color" conflicts with a definition in <windows.h>
//NOTE: This is BGRA order in memory, or in hex that's 0xAARRGGBB
typedef car Color32 Color32;
car Color32
{
	u32 valueU32;
	plex { u8 channel[4]; };
	plex { u8 b, g, r, a; };
	plex { u8 blue, green, red, alpha; };
};
#define MakeColorU32_Const(value)   { .valueU32=(value) }
#define MakeColor_Const(r, g, b, a) { .blue=(b), .green=(g), .red=(r), .alpha=(a) }
#define MakeColorU32(value)         NEW_STRUCT(Color32)MakeColorU32_Const((value))
#define MakeColor(r, g, b, a)       NEW_STRUCT(Color32)MakeColor_Const((r), (g), (b), (a))

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Color32 ColorLerpSimple(Color32 start, Color32 end, r32 amount);
	PIG_CORE_INLINE Color32 ColorWithAlphaU8(Color32 rgbColor, u8 alpha);
	PIG_CORE_INLINE Color32 ColorWithAlpha(Color32 rgbColor, r32 alpha);
#endif //!PIG_CORE_IMPLEMENTATION

#define NoColor_Value          0x00000000UL
#define TransparentBlack_Value 0x00000000UL
#define TransparentWhite_Value 0x00FFFFFFUL
#define Transparent_Value      TransparentWhite_Value
#define Black_Value            0xFF000000UL
#define White_Value            0xFFFFFFFFUL

#define NoColor_Const          MakeColorU32_Const(NoColor_Value)
#define TransparentBlack_Const MakeColorU32_Const(TransparentBlack_Value)
#define TransparentWhite_Const MakeColorU32_Const(TransparentWhite_Value)
#define Transparent_Const      TransparentWhite_Const
#define Black_Const            MakeColorU32_Const(Black_Value)
#define White_Const            MakeColorU32_Const(White_Value)

#define NoColor          MakeColorU32(NoColor_Value)
#define TransparentBlack MakeColorU32(TransparentBlack_Value)
#define TransparentWhite MakeColorU32(TransparentWhite_Value)
#define Transparent      TransparentWhite
#define Black            MakeColorU32(Black_Value)
#define White            MakeColorU32(White_Value)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                     Basic Math Functions                     |
// +--------------------------------------------------------------+
//"Simple" meaning we don't do lerp in linear color space, we lerp in Gamma sRGB space, which is innaccurate but simple to do
PEXPI Color32 ColorLerpSimple(Color32 start, Color32 end, r32 amount)
{
	return MakeColor(
		ClampCastI32ToU8(RoundR32i((r32)start.r + (r32)(end.r - start.r) * amount)),
		ClampCastI32ToU8(RoundR32i((r32)start.g + (r32)(end.g - start.g) * amount)),
		ClampCastI32ToU8(RoundR32i((r32)start.b + (r32)(end.b - start.b) * amount)),
		ClampCastI32ToU8(RoundR32i((r32)start.a + (r32)(end.a - start.a) * amount))
	);
}

PEXPI Color32 ColorWithAlphaU8(Color32 rgbColor, u8 alpha)
{
	return MakeColor(rgbColor.r, rgbColor.g, rgbColor.b, alpha);
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
