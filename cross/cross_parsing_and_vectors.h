/*
File:   cross_parsing_and_vectors.h
Author: Taylor Robbins
Date:   02\01\2025
*/

#ifndef _CROSS_PARSING_AND_VECTORS_H
#define _CROSS_PARSING_AND_VECTORS_H

//TODO: Add support for parsing v4!

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	bool TryParseV2i(Str8 str, v2i* valueOut, Result* errorOut);
	bool TryParseV2(Str8 str, v2* valueOut, Result* errorOut);
	bool TryParseV3i(Str8 str, v3i* valueOut, Result* errorOut);
	bool TryParseV3(Str8 str, v3* valueOut, Result* errorOut);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP bool TryParseV2i(Str8 str, v2i* valueOut, Result* errorOut)
{
	NotNullStr(str);
	if (StrExactStartsWith(str, StrLit("("))) { str = StrSliceFrom(str, 1); }
	if (StrExactEndsWith(str, StrLit(")"))) { str = StrSlice(str, 0, str.length-1); }
	uxx commaIndex = 0;
	bool strContainsComma = StrTryExactFind(str, StrLit(","), &commaIndex);
	if (!strContainsComma)
	{
		SetOptionalOutPntr(errorOut, Result_NotEnoughCommas);
		return false;
	}
	Str8 xStr = StrSlice(str, 0, commaIndex);
	Str8 yStr = StrSliceFrom(str, commaIndex+1);
	v2i vector = V2i_Zero;
	if (!TryParseI32(xStr, &vector.X, errorOut)) { return false; }
	if (!TryParseI32(yStr, &vector.Y, errorOut)) { return false; }
	SetOptionalOutPntr(valueOut, vector);
	return true;
}
PEXP bool TryParseV2(Str8 str, v2* valueOut, Result* errorOut)
{
	NotNullStr(str);
	if (StrExactStartsWith(str, StrLit("("))) { str = StrSliceFrom(str, 1); }
	if (StrExactEndsWith(str, StrLit(")"))) { str = StrSlice(str, 0, str.length-1); }
	uxx commaIndex = 0;
	bool strContainsComma = StrTryExactFind(str, StrLit(","), &commaIndex);
	if (!strContainsComma)
	{
		SetOptionalOutPntr(errorOut, Result_NotEnoughCommas);
		return false;
	}
	Str8 xStr = StrSlice(str, 0, commaIndex);
	Str8 yStr = StrSliceFrom(str, commaIndex+1);
	v2 vector = V2_Zero;
	if (!TryParseR32(xStr, &vector.X, errorOut)) { return false; }
	if (!TryParseR32(yStr, &vector.Y, errorOut)) { return false; }
	SetOptionalOutPntr(valueOut, vector);
	return true;
}

PEXP bool TryParseV3i(Str8 str, v3i* valueOut, Result* errorOut)
{
	NotNullStr(str);
	if (StrExactStartsWith(str, StrLit("("))) { str = StrSliceFrom(str, 1); }
	if (StrExactEndsWith(str, StrLit(")"))) { str = StrSlice(str, 0, str.length-1); }
	uxx commaIndex1 = 0;
	bool strContainsComma1 = StrTryExactFind(str, StrLit(","), &commaIndex1);
	if (!strContainsComma1)
	{
		SetOptionalOutPntr(errorOut, Result_NotEnoughCommas);
		return false;
	}
	uxx commaIndex2 = 0;
	bool strContainsComma2 = StrTryExactFind(StrSliceFrom(str, commaIndex1+1), StrLit(","), &commaIndex2);
	commaIndex2 += commaIndex1+1;
	if (!strContainsComma2)
	{
		SetOptionalOutPntr(errorOut, Result_NotEnoughCommas);
		return false;
	}
	Str8 xStr = StrSlice(str, 0, commaIndex1);
	Str8 yStr = StrSlice(str, commaIndex1+1, commaIndex2);
	Str8 zStr = StrSliceFrom(str, commaIndex2+1);
	v3i vector = V3i_Zero;
	if (!TryParseI32(xStr, &vector.X, errorOut)) { return false; }
	if (!TryParseI32(yStr, &vector.Y, errorOut)) { return false; }
	if (!TryParseI32(zStr, &vector.Z, errorOut)) { return false; }
	SetOptionalOutPntr(valueOut, vector);
	return true;
}
PEXP bool TryParseV3(Str8 str, v3* valueOut, Result* errorOut)
{
	NotNullStr(str);
	if (StrExactStartsWith(str, StrLit("("))) { str = StrSliceFrom(str, 1); }
	if (StrExactEndsWith(str, StrLit(")"))) { str = StrSlice(str, 0, str.length-1); }
	uxx commaIndex1 = 0;
	bool strContainsComma1 = StrTryExactFind(str, StrLit(","), &commaIndex1);
	if (!strContainsComma1)
	{
		SetOptionalOutPntr(errorOut, Result_NotEnoughCommas);
		return false;
	}
	uxx commaIndex2 = 0;
	bool strContainsComma2 = StrTryExactFind(StrSliceFrom(str, commaIndex1+1), StrLit(","), &commaIndex2);
	commaIndex2 += commaIndex1+1;
	if (!strContainsComma2)
	{
		SetOptionalOutPntr(errorOut, Result_NotEnoughCommas);
		return false;
	}
	Str8 xStr = StrSlice(str, 0, commaIndex1);
	Str8 yStr = StrSlice(str, commaIndex1+1, commaIndex2);
	Str8 zStr = StrSliceFrom(str, commaIndex2+1);
	v3 vector = V3_Zero;
	if (!TryParseR32(xStr, &vector.X, errorOut)) { return false; }
	if (!TryParseR32(yStr, &vector.Y, errorOut)) { return false; }
	if (!TryParseR32(zStr, &vector.Z, errorOut)) { return false; }
	if (valueOut != nullptr) { *valueOut = vector; }
	return true;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_PARSING_AND_VECTORS_H
