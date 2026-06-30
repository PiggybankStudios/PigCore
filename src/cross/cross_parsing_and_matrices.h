/*
File:   cross_parsing_and_matrices.h
Author: Taylor Robbins
Date:   02\01\2025
*/

#ifndef _CROSS_PARSING_AND_MATRICES_H
#define _CROSS_PARSING_AND_MATRICES_H

//TODO: Add support for parsing mat3 and mat2?

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	bool TryParseMat4Ex(Str8 str, mat4* valueOut, Result* errorOut, bool allow3x3Matrix);
	PIG_CORE_INLINE bool TryParseMat4(Str8 str, mat4* valueOut, Result* errorOut);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP bool TryParseMat4Ex(Str8 str, mat4* valueOut, Result* errorOut, bool allow3x3Matrix)
{
	NotNullStr(str);
	if (StrExactStartsWith(str, StrLit("(")) || StrExactStartsWith(str, StrLit("["))) { str = StrSliceFrom(str, 1); }
	if (StrExactEndsWith(str, StrLit(")")) || StrExactEndsWith(str, StrLit("]"))) { str = StrSlice(str, 0, str.length-1); }
	u8 numCommasFound = 0;
	u64 commaIndices[15];
	for (u64 cIndex = 0; cIndex < str.length; cIndex++)
	{
		if (str.chars[cIndex] == ',')
		{
			if (numCommasFound >= ArrayCount(commaIndices))
			{
				SetOptionalOutPntr(errorOut, Result_TooManyCommas);
				return false;
			}
			commaIndices[numCommasFound] = cIndex;
			numCommasFound++;
		}
	}
	
	bool is3x3Matrix = (numCommasFound == 8);
	bool is4x4Matrix = (numCommasFound == 15);
	if (!(allow3x3Matrix && is3x3Matrix) && !is4x4Matrix)
	{
		SetOptionalOutPntr(errorOut, Result_NotEnoughCommas);
		return false;
	}
	
	mat4 matrix = Mat4_Identity;
	for (u64 cIndex = 0; cIndex <= numCommasFound; cIndex++)
	{
		Str8 strPart = StrSlice(str,
			((cIndex > 0) ? commaIndices[cIndex-1]+1 : 0),
			((cIndex < numCommasFound) ? commaIndices[cIndex] : str.length));
		strPart = TrimWhitespaceAndNewLines(strPart);
		
		r32 valueR32 = 0.0f;
		if (!TryParseR32(strPart, &valueR32, errorOut)) { return false; }
		
		if (is3x3Matrix)
		{
			matrix.Elements[cIndex % 3][cIndex / 3] = valueR32;
		}
		else
		{
			matrix.Elements[cIndex % 4][cIndex / 4] = valueR32;
		}
	}
	
	SetOptionalOutPntr(valueOut, matrix);
	return true;
}
PEXPI bool TryParseMat4(Str8 str, mat4* valueOut, Result* errorOut)
{
	return TryParseMat4Ex(str, valueOut, errorOut, true);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_PARSING_AND_MATRICES_H
