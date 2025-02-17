/*
File:   cross_string_and_clay.h
Author: Taylor Robbins
Date:   02\16\2025
*/

#ifndef _CROSS_STRING_AND_CLAY_H
#define _CROSS_STRING_AND_CLAY_H

//NOTE: Intentionally no includes here

#if BUILD_WITH_CLAY

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Str8 ToStrFromClay(Clay_String clayString);
	PIG_CORE_INLINE Clay_String ToClayString(Str8 str);
	PIG_CORE_INLINE Clay_ElementId ToClayIdEx(Str8 idStr, uxx index);
	PIG_CORE_INLINE Clay_ElementId ToClayId(Str8 idStr);
	PIG_CORE_INLINE Clay_ElementId ToClayIdNt(const char* idNullTermString);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI Str8 ToStrFromClay(Clay_String clayString) { return NewStr8((uxx)clayString.length, clayString.chars); }
PEXPI Clay_String ToClayString(Str8 str) { return (Clay_String){ .length = (int32_t)str.length, .chars = str.chars }; }
PEXPI Clay_ElementId ToClayIdEx(Str8 idStr, uxx index) { Assert(index <= UINT32_MAX); return Clay__HashString(ToClayString(idStr), (uint32_t)index, 0); }
PEXPI Clay_ElementId ToClayId(Str8 idStr) { return ToClayIdEx(idStr, 0); }
PEXPI Clay_ElementId ToClayIdNt(const char* idNullTermString) { return ToClayId(StrLit(idNullTermString)); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_CLAY

#endif //  _CROSS_STRING_AND_CLAY_H
