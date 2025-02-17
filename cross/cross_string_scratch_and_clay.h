/*
File:   cross_string_scratch_and_clay.h
Author: Taylor Robbins
Date:   02\16\2025
*/

#ifndef _CROSS_STRING_SCRATCH_AND_CLAY_H
#define _CROSS_STRING_SCRATCH_AND_CLAY_H

//NOTE: Intentionally no includes here

#if BUILD_WITH_CLAY

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Clay_ElementId ToClayIdPrintEx(uxx index, const char* formatString, ...);
	PIG_CORE_INLINE Clay_ElementId ToClayIdPrint(const char* formatString, ...);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI Clay_ElementId ToClayIdPrintEx(uxx index, const char* formatString, ...)
{
	ScratchBegin(scratch);
	PrintInArenaVa(scratch, formattedString, formattedStringLength, formatString);
	Assert(formattedStringLength >= 0);
	Clay_ElementId result = ToClayIdEx(NewStr8((uxx)formattedStringLength, formattedString), index);
	ScratchEnd(scratch);
	return result;
}
PEXPI Clay_ElementId ToClayIdPrint(const char* formatString, ...)
{
	ScratchBegin(scratch);
	PrintInArenaVa(scratch, formattedString, formattedStringLength, formatString);
	Assert(formattedStringLength >= 0);
	Clay_ElementId result = ToClayIdEx(NewStr8((uxx)formattedStringLength, formattedString), 0);
	ScratchEnd(scratch);
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_CLAY

#endif //  _CROSS_STRING_SCRATCH_AND_CLAY_H
