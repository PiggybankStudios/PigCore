/*
File:   cross_scratch_and_clay.h
Author: Taylor Robbins
Date:   02\16\2025
*/

#ifndef _CROSS_SCRATCH_AND_CLAY_H
#define _CROSS_SCRATCH_AND_CLAY_H

//NOTE: Intentionally no includes here

#if BUILD_WITH_CLAY

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE ClayId ToClayIdPrintEx(Arena* arena, uxx index, const char* formatString, ...);
	PIG_CORE_INLINE ClayId ToClayIdPrint(Arena* arena, const char* formatString, ...);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI ClayId ToClayIdPrintEx(Arena* arena, uxx index, const char* formatString, ...)
{
	PrintInArenaVa(arena, formattedString, formattedStringLength, formatString);
	Assert(formattedStringLength >= 0);
	ClayId result = ToClayIdEx(NewStr8((uxx)formattedStringLength, formattedString), index);
	return result;
}
PEXPI ClayId ToClayIdPrint(Arena* arena, const char* formatString, ...)
{
	PrintInArenaVa(arena, formattedString, formattedStringLength, formatString);
	Assert(formattedStringLength >= 0);
	ClayId result = ToClayIdEx(NewStr8((uxx)formattedStringLength, formattedString), 0);
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_CLAY

#endif //  _CROSS_SCRATCH_AND_CLAY_H
