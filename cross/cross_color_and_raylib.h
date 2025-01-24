/*
File:   cross_color_and_raylib.h
Author: Taylor Robbins
Date:   01\24\2025
*/

#ifndef _CROSS_COLOR_AND_RAYLIB_H
#define _CROSS_COLOR_AND_RAYLIB_H

//NOTE: Intentionally no includes here

#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Color rlNewColor(u8 r, u8 g, u8 b, u8 a);
	PIG_CORE_INLINE Color ColorFromColor32(Color32 color);
	PIG_CORE_INLINE Color32 Color32FromColor(Color color);
#endif

#if PIG_CORE_IMPLEMENTATION

PEXPI Color rlNewColor(u8 r, u8 g, u8 b, u8 a) { return (Color){ r, g, b, a }; }
PEXPI Color ColorFromColor32(Color32 color) { return (Color){ color.r, color.g, color.b, color.a }; }
PEXPI Color32 Color32FromColor(Color color) { return NewColor(color.r, color.g, color.b, color.a); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_COLOR_AND_RAYLIB_H
