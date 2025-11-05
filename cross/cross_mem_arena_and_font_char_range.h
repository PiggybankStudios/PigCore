/*
File:   cross_mem_arena_and_font_char_range.h
Author: Taylor Robbins
Date:   11\05\2025
*/

#ifndef _CROSS_MEM_ARENA_AND_FONT_CHAR_RANGE_H
#define _CROSS_MEM_ARENA_AND_FONT_CHAR_RANGE_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	FontCharRange* SortAndMergeFontCharRanges(Arena* arena, uxx numRanges, const FontCharRange* charRanges, uxx* numRangesOut);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP FontCharRange* SortAndMergeFontCharRanges(Arena* arena, uxx numRanges, const FontCharRange* charRanges, uxx* numRangesOut)
{
	NotNull(arena);
	Assert(numRanges == 0 || charRanges != nullptr);
	if (numRanges == 0) { SetOptionalOutPntr(numRangesOut, 0); return nullptr; }
	
	//TODO: Somehow we need to determine how many char ranges we are going to end up with, so we can do a single allocation with the correct size
	
	//TODO: Implement me!
	SetOptionalOutPntr(numRangesOut, numRanges);
	return (FontCharRange*)charRanges;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_MEM_ARENA_AND_FONT_CHAR_RANGE_H
