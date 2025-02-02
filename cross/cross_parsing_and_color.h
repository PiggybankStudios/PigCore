/*
File:   cross_parsing_and_color.h
Author: Taylor Robbins
Date:   02\01\2025
*/

#ifndef _CROSS_PARSING_AND_COLOR_H
#define _CROSS_PARSING_AND_COLOR_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	bool TryParseColorEx(Str8 str, Color32* valueOut, Result* errorOut, bool alphaAtBeginning);
	PIG_CORE_INLINE bool TryParseColor(Str8 str, Color32* valueOut, Result* errorOut);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP bool TryParseColorEx(Str8 str, Color32* valueOut, Result* errorOut, bool alphaAtBeginning)
{
	NotNullStr(str);
	Color32 result = TransparentBlack;
	if (StrExactStartsWith(str, StrLit("#"))) { str = StrSliceFrom(str, 1); }
	if (str.length == 8)
	{
		if (!AreCharsHexidecimal(str.length, str.chars))
		{
			SetOptionalOutPntr(errorOut, Result_InvalidCharacter);
			return false;
		}
		if (alphaAtBeginning)
		{
			result.a = (GetHexCharValue(str.chars[0]) << 4) | (GetHexCharValue(str.chars[1]) << 0);
			result.r = (GetHexCharValue(str.chars[2]) << 4) | (GetHexCharValue(str.chars[3]) << 0);
			result.g = (GetHexCharValue(str.chars[4]) << 4) | (GetHexCharValue(str.chars[5]) << 0);
			result.b = (GetHexCharValue(str.chars[6]) << 4) | (GetHexCharValue(str.chars[7]) << 0);
		}
		else
		{
			result.r = (GetHexCharValue(str.chars[0]) << 4) | (GetHexCharValue(str.chars[1]) << 0);
			result.g = (GetHexCharValue(str.chars[2]) << 4) | (GetHexCharValue(str.chars[3]) << 0);
			result.b = (GetHexCharValue(str.chars[4]) << 4) | (GetHexCharValue(str.chars[5]) << 0);
			result.a = (GetHexCharValue(str.chars[6]) << 4) | (GetHexCharValue(str.chars[7]) << 0);
		}
		SetOptionalOutPntr(valueOut, result);
		return true;
	}
	else if (str.length == 6)
	{
		result.a = 255;
		if (!AreCharsHexidecimal(str.length, str.chars))
		{
			SetOptionalOutPntr(errorOut, Result_InvalidCharacter);
			return false;
		}
		result.r = (GetHexCharValue(str.chars[0]) << 4) | (GetHexCharValue(str.chars[1]) << 0);
		result.g = (GetHexCharValue(str.chars[2]) << 4) | (GetHexCharValue(str.chars[3]) << 0);
		result.b = (GetHexCharValue(str.chars[4]) << 4) | (GetHexCharValue(str.chars[5]) << 0);
		SetOptionalOutPntr(valueOut, result);
		return true;
	}
	else
	{
		SetOptionalOutPntr(errorOut, Result_WrongNumCharacters);
		return false;
	}
}
PEXPI bool TryParseColor(Str8 str, Color32* valueOut, Result* errorOut)
{
	return TryParseColorEx(str, valueOut, errorOut, true);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_PARSING_AND_COLOR_H
