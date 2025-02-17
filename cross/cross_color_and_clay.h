/*
File:   cross_color_and_clay.h
Author: Taylor Robbins
Date:   02\16\2025
*/

#ifndef _CROSS_COLOR_AND_CLAY_H
#define _CROSS_COLOR_AND_CLAY_H

//NOTE: Intentionally no includes here

#if BUILD_WITH_CLAY

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Color32 ToColorFromClay(Clay_Color clayColor);
	PIG_CORE_INLINE Clay_Color ToClayColor(Color32 color);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI Color32 ToColorFromClay(Clay_Color clayColor) { return ToColor32FromV4(NewV4(clayColor.r/255.0f, clayColor.g/255.0f, clayColor.b/255.0f, clayColor.a/255.0f)); }
PEXPI Clay_Color ToClayColor(Color32 color) { v4 colorVec = ToV4FromColor32(color); return (Clay_Color){ .r = colorVec.R*255.0f, .g = colorVec.G*255.0f, .b = colorVec.B*255.0f, .a = colorVec.A*255.0f }; }

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_CLAY

#endif //  _CROSS_COLOR_AND_CLAY_H
