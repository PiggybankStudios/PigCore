/*
File:   cross_string_and_ranges.h
Author: Taylor Robbins
Date:   08\08\2025
*/

#ifndef _CROSS_STRING_AND_RANGES_H
#define _CROSS_STRING_AND_RANGES_H

//NOTE: Intentionally no includes here

#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE RangeUXX SliceToRangeUXX(Str8 str, Str8 slice);
#endif

#if PIG_CORE_IMPLEMENTATION

PEXPI RangeUXX SliceToRangeUXX(Str8 str, Str8 slice)
{
	if (str.length == 0) { return RangeUXX_Zero; }
	NotNull(slice.chars);
	NotNull(str.chars);
	Assert(IsSizedPntrWithin(str.chars, str.length, slice.chars, slice.length));
	return NewRangeUXX((uxx)(slice.chars - str.chars), (uxx)(slice.chars - str.chars) + slice.length);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_STRING_AND_RANGES_H
