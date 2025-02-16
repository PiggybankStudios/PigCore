/*
File:   struct_color.h
Author: Taylor Robbins
Date:   01\05\2025
Description:
	** Holds the Color struct which is a 32-bit value that has 4 8-bit channels
	** The order of the channels 
*/

#ifndef _STRUCT_COLOR_H
#define _STRUCT_COLOR_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"

//NOTE: The name "Color" conflicts with a definition in <windows.h>
typedef union Color32 Color32;
union Color32
{
	u32 valueU32;
	struct { u8 channel[4]; };
	struct { u8 b, g, r, a; };
	struct { u8 blue, green, red, alpha; };
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Color32 NewColorU32(u32 valueU32);
	PIG_CORE_INLINE Color32 NewColor(u8 red, u8 green, u8 blue, u8 alpha);
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

#if defined(_STRUCT_COLOR_H) && defined(_UI_CLAY_H)
#include "cross/cross_color_and_clay.h"
#endif
