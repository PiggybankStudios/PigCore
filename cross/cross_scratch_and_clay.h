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
	PIG_CORE_INLINE ClayId ToClayIdPrintEx(uxx index, const char* formatString, ...);
	PIG_CORE_INLINE ClayId ToClayIdPrint(const char* formatString, ...);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI ClayId ToClayIdPrintEx(uxx index, const char* formatString, ...)
{
	ScratchBegin(scratch);
	PrintInArenaVa(scratch, formattedString, formattedStringLength, formatString);
	Assert(formattedStringLength >= 0);
	ClayId result = ToClayIdEx(NewStr8((uxx)formattedStringLength, formattedString), index);
	ScratchEnd(scratch);
	return result;
}
PEXPI ClayId ToClayIdPrint(const char* formatString, ...)
{
	ScratchBegin(scratch);
	PrintInArenaVa(scratch, formattedString, formattedStringLength, formatString);
	Assert(formattedStringLength >= 0);
	ClayId result = ToClayIdEx(NewStr8((uxx)formattedStringLength, formattedString), 0);
	ScratchEnd(scratch);
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_CLAY

#endif //  _CROSS_SCRATCH_AND_CLAY_H
