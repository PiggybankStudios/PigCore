/*
File:   cross_font_char_range_and_var_array.h
Author: Taylor Robbins
Date:   11\05\2025
*/

#ifndef _CROSS_FONT_CHAR_RANGE_AND_VAR_ARRAY_H
#define _CROSS_FONT_CHAR_RANGE_AND_VAR_ARRAY_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void RemoveCodepointsFromCharRanges(VarArray* charRanges, uxx numCodepoints, u32* codepoints);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void RemoveCodepointsFromCharRanges(VarArray* charRanges, uxx numCodepoints, u32* codepoints)
{
	for (uxx cIndex = 0; cIndex < numCodepoints; cIndex++)
	{
		u32 codepointToRemove = codepoints[cIndex];
		bool foundCharRange = false;
		VarArrayLoop(charRanges, rIndex)
		{
			VarArrayLoopGet(FontCharRange, charRange, charRanges, rIndex);
			if (charRange->startCodepoint <= codepointToRemove && charRange->endCodepoint >= codepointToRemove)
			{
				foundCharRange = true;
				if (charRange->startCodepoint == charRange->endCodepoint) //single codepoint range needs to be removed entirely
				{
					VarArrayRemoveAt(FontCharRange, charRanges, rIndex);
				}
				else if (charRange->startCodepoint == codepointToRemove) //bump up startCodepoint by one
				{
					charRange->startCodepoint++;
				}
				else if (charRange->endCodepoint == codepointToRemove) //bump down endCodepoint by one
				{
					charRange->endCodepoint--;
				}
				else //otherwise we need to split the charRange into two
				{
					FontCharRange lowerRange = NewFontCharRange(charRange->startCodepoint, codepointToRemove-1);
					FontCharRange upperRange = NewFontCharRange(codepointToRemove+1, charRange->endCodepoint);
					MyMemCopy(charRange, &lowerRange, sizeof(FontCharRange));
					VarArrayInsertValue(FontCharRange, charRanges, rIndex+1, upperRange);
				}
				break;
			}
		}
		Assert(foundCharRange);
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_FONT_CHAR_RANGE_AND_VAR_ARRAY_H
