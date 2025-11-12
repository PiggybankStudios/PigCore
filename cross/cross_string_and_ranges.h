/*
File:   cross_string_and_ranges.h
Author: Taylor Robbins
Date:   08\08\2025
*/

#ifndef _CROSS_STRING_AND_RANGES_H
#define _CROSS_STRING_AND_RANGES_H

//NOTE: Intentionally no includes here

typedef plex StrRange StrRange;
plex StrRange
{
	Str8 str;
	RangeUXX range;
};
#define MakeStrRange(strValue, rangeValue) NEW_STRUCT(StrRange){ .str=(strValue), .range=(rangeValue) }

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE RangeUXX SliceToRangeUXX(Str8 str, Str8 slice);
	PIG_CORE_INLINE Str8 StrSliceRange(Str8 str, RangeUXX range);
	PIG_CORE_INLINE Str8 ToStr8FromRange(StrRange strRange);
	PIG_CORE_INLINE StrRange ToStrRange(Str8 str, Str8 slice);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI RangeUXX SliceToRangeUXX(Str8 str, Str8 slice)
{
	if (str.length == 0) { return RangeUXX_Zero; }
	NotNull(slice.chars);
	NotNull(str.chars);
	Assert(IsSizedPntrWithin(str.chars, str.length, slice.chars, slice.length));
	return MakeRangeUXX((uxx)(slice.chars - str.chars), (uxx)(slice.chars - str.chars) + slice.length);
}

PEXPI Str8 StrSliceRange(Str8 str, RangeUXX range) { return StrSlice(str, range.min, range.max); }

PEXPI Str8 ToStr8FromRange(StrRange strRange)
{
	return StrSliceRange(strRange.str, strRange.range);
}

PEXPI StrRange ToStrRange(Str8 str, Str8 slice)
{
	return MakeStrRange(str, SliceToRangeUXX(str, slice));
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_STRING_AND_RANGES_H
