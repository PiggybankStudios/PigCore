/*
File:   cross_color_and_raylib.h
Author: Taylor Robbins
Date:   01\24\2025
*/

#ifndef _CROSS_COLOR_AND_RAYLIB_H
#define _CROSS_COLOR_AND_RAYLIB_H

//NOTE: Intentionally no includes here

#if BUILD_WITH_RAYLIB

#define rlMakeColor(rValue, gValue, bValue, aValue) NEW_STRUCT(Color){ .r=(rValue), .g=(gValue), .b=(bValue), .a=(aValue) }

#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Color ColorFromColor32(Color32 color);
	PIG_CORE_INLINE Color32 Color32FromColor(Color color);
#endif

#if PIG_CORE_IMPLEMENTATION

PEXPI Color ColorFromColor32(Color32 color) { return rlMakeColor(color.r, color.g, color.b, color.a); }
PEXPI Color32 Color32FromColor(Color color) { return MakeColor(color.r, color.g, color.b, color.a); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_RAYLIB

#endif //  _CROSS_COLOR_AND_RAYLIB_H
