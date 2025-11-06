/*
File:   cross_sorting_and_font_char_range.h
Author: Taylor Robbins
Date:   11\05\2025
*/

#ifndef _CROSS_SORTING_AND_FONT_CHAR_RANGE_H
#define _CROSS_SORTING_AND_FONT_CHAR_RANGE_H

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
	ScratchBegin1(scratch, arena);
	
	bool* mergedBools = AllocArray(bool, scratch, numRanges);
	NotNull(mergedBools);
	MyMemSet(mergedBools, 0x00, sizeof(bool) * numRanges);
	uxx numMergedRanges = 0;
	for (uxx rIndex = 0; rIndex < numRanges; rIndex++)
	{
		if (!mergedBools[rIndex])
		{
			FontCharRange mergedRange = charRanges[rIndex];
			for (uxx rIndex2 = rIndex+1; rIndex2 < numRanges; rIndex2++)
			{
				if (!mergedBools[rIndex2])
				{
					FontCharRange otherRange = charRanges[rIndex2];
					if (otherRange.startCodepoint <= mergedRange.endCodepoint+1 &&
						otherRange.endCodepoint+1 >= mergedRange.startCodepoint)
					{
						mergedRange.startCodepoint = MinU32(mergedRange.startCodepoint, otherRange.startCodepoint);
						mergedRange.endCodepoint = MaxU32(mergedRange.endCodepoint, otherRange.endCodepoint);
						mergedBools[rIndex2] = true;
					}
				}
			}
			numMergedRanges++;
		}
	}
	
	FontCharRange* mergedRanges = AllocArray(FontCharRange, arena, numRanges);
	NotNull(mergedRanges);
	MyMemSet(mergedRanges, 0x00, sizeof(FontCharRange) * numMergedRanges);
	
	uxx mergeIndex = 0;
	MyMemSet(mergedBools, 0x00, sizeof(bool) * numRanges);
	for (uxx rIndex = 0; rIndex < numRanges; rIndex++)
	{
		if (!mergedBools[rIndex])
		{
			FontCharRange mergedRange = charRanges[rIndex];
			for (uxx rIndex2 = rIndex+1; rIndex2 < numRanges; rIndex2++)
			{
				if (!mergedBools[rIndex2])
				{
					FontCharRange otherRange = charRanges[rIndex2];
					if (otherRange.startCodepoint <= mergedRange.endCodepoint+1 &&
						otherRange.endCodepoint+1 >= mergedRange.startCodepoint)
					{
						// PrintLine_D("Merging %u-%u into %u-%u", otherRange.startCodepoint, otherRange.endCodepoint, mergedRange.startCodepoint, mergedRange.endCodepoint);
						mergedRange.startCodepoint = MinU32(mergedRange.startCodepoint, otherRange.startCodepoint);
						mergedRange.endCodepoint = MaxU32(mergedRange.endCodepoint, otherRange.endCodepoint);
						mergedBools[rIndex2] = true;
					}
				}
			}
			Assert(mergeIndex < numMergedRanges);
			mergedRanges[mergeIndex] = mergedRange;
			mergeIndex++;
		}
	}
	Assert(mergeIndex == numMergedRanges);
	
	QuickSortFlatOnUintMember(FontCharRange, startCodepoint, mergedRanges, numMergedRanges, sizeof(FontCharRange));
	
	SetOptionalOutPntr(numRangesOut, numMergedRanges);
	ScratchEnd(scratch);
	return (FontCharRange*)mergedRanges;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_SORTING_AND_FONT_CHAR_RANGE_H
