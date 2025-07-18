/*
File:   cross_parsing_and_rectangles.h
Author: Taylor Robbins
Date:   02\01\2025
*/

#ifndef _CROSS_PARSING_AND_RECTANGLES_H
#define _CROSS_PARSING_AND_RECTANGLES_H

//TODO: Add support for parsing rec, box, boxi, obb2, and obb3?

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	bool TryParseRec(Str8 str, rec* valueOut, Result* errorOut);
	bool TryParseReci(Str8 str, reci* valueOut, Result* errorOut);
	bool TryParseRecd(Str8 str, recd* valueOut, Result* errorOut);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP bool TryParseRec(Str8 str, rec* valueOut, Result* errorOut)
{
	NotNullStr(str);
	if (StrExactStartsWith(str, StrLit("("))) { str = StrSliceFrom(str, 1); }
	if (StrExactEndsWith(str, StrLit(")"))) { str = StrSlice(str, 0, str.length-1); }
	u8 numCommasFound = 0;
	u64 commaIndices[3];
	for (u64 cIndex = 0; cIndex < str.length; cIndex++)
	{
		if (str.chars[cIndex] == ',')
		{
			if (numCommasFound >= 3)
			{
				SetOptionalOutPntr(errorOut, Result_TooManyCommas);
				return false;
			}
			commaIndices[numCommasFound] = cIndex;
			numCommasFound++;
		}
	}
	if (numCommasFound < 3)
	{
		SetOptionalOutPntr(errorOut, Result_NotEnoughCommas);
		return false;
	}
	Str8 xStr = StrSlice(str, 0, commaIndices[0]);
	Str8 yStr = StrSlice(str, commaIndices[0]+1, commaIndices[1]);
	Str8 widthStr = StrSlice(str, commaIndices[1]+1, commaIndices[2]);
	Str8 heightStr = StrSliceFrom(str, commaIndices[2]+1);
	rec rectangle = Rec_Zero;
	if (!TryParseR32(xStr, &rectangle.X, errorOut)) { return false; }
	if (!TryParseR32(yStr, &rectangle.Y, errorOut)) { return false; }
	if (!TryParseR32(widthStr, &rectangle.Width, errorOut)) { return false; }
	if (!TryParseR32(heightStr, &rectangle.Height, errorOut)) { return false; }
	SetOptionalOutPntr(valueOut, rectangle);
	return true;
}

PEXP bool TryParseReci(Str8 str, reci* valueOut, Result* errorOut)
{
	NotNullStr(str);
	if (StrExactStartsWith(str, StrLit("("))) { str = StrSliceFrom(str, 1); }
	if (StrExactEndsWith(str, StrLit(")"))) { str = StrSlice(str, 0, str.length-1); }
	u8 numCommasFound = 0;
	u64 commaIndices[3];
	for (u64 cIndex = 0; cIndex < str.length; cIndex++)
	{
		if (str.chars[cIndex] == ',')
		{
			if (numCommasFound >= 3)
			{
				SetOptionalOutPntr(errorOut, Result_TooManyCommas);
				return false;
			}
			commaIndices[numCommasFound] = cIndex;
			numCommasFound++;
		}
	}
	if (numCommasFound < 3)
	{
		SetOptionalOutPntr(errorOut, Result_NotEnoughCommas);
		return false;
	}
	Str8 xStr = StrSlice(str, 0, commaIndices[0]);
	Str8 yStr = StrSlice(str, commaIndices[0]+1, commaIndices[1]);
	Str8 widthStr = StrSlice(str, commaIndices[1]+1, commaIndices[2]);
	Str8 heightStr = StrSliceFrom(str, commaIndices[2]+1);
	reci rectangle = Reci_Zero;
	if (!TryParseI32(xStr, &rectangle.X, errorOut)) { return false; }
	if (!TryParseI32(yStr, &rectangle.Y, errorOut)) { return false; }
	if (!TryParseI32(widthStr, &rectangle.Width, errorOut)) { return false; }
	if (!TryParseI32(heightStr, &rectangle.Height, errorOut)) { return false; }
	SetOptionalOutPntr(valueOut, rectangle);
	return true;
}

PEXP bool TryParseRecd(Str8 str, recd* valueOut, Result* errorOut)
{
	NotNullStr(str);
	if (StrExactStartsWith(str, StrLit("("))) { str = StrSliceFrom(str, 1); }
	if (StrExactEndsWith(str, StrLit(")"))) { str = StrSlice(str, 0, str.length-1); }
	u8 numCommasFound = 0;
	u64 commaIndices[3];
	for (u64 cIndex = 0; cIndex < str.length; cIndex++)
	{
		if (str.chars[cIndex] == ',')
		{
			if (numCommasFound >= 3)
			{
				SetOptionalOutPntr(errorOut, Result_TooManyCommas);
				return false;
			}
			commaIndices[numCommasFound] = cIndex;
			numCommasFound++;
		}
	}
	if (numCommasFound < 3)
	{
		SetOptionalOutPntr(errorOut, Result_NotEnoughCommas);
		return false;
	}
	Str8 xStr = StrSlice(str, 0, commaIndices[0]);
	Str8 yStr = StrSlice(str, commaIndices[0]+1, commaIndices[1]);
	Str8 widthStr = StrSlice(str, commaIndices[1]+1, commaIndices[2]);
	Str8 heightStr = StrSliceFrom(str, commaIndices[2]+1);
	recd rectangle = Recd_Zero;
	if (!TryParseR64(xStr, &rectangle.X, errorOut)) { return false; }
	if (!TryParseR64(yStr, &rectangle.Y, errorOut)) { return false; }
	if (!TryParseR64(widthStr, &rectangle.Width, errorOut)) { return false; }
	if (!TryParseR64(heightStr, &rectangle.Height, errorOut)) { return false; }
	SetOptionalOutPntr(valueOut, rectangle);
	return true;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_PARSING_AND_RECTANGLES_H
