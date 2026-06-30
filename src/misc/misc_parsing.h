/*
File:   misc_parsing.h
Author: Taylor Robbins
Date:   02\01\2025
Description:
	** Contains a bunch of functions for parsing basic data types like integers, vectors, etc.
	** Parsing meaning taking in a string type and converting to some data type.
	** Unlike the C standard library functions for parsing integers and floats (atod, atof, etc.)
	** these parsers are designed to give you descriptive errors for any unsupported characters
	** or syntax for a particular data type.
*/

#ifndef _MISC_PARSING_H
#define _MISC_PARSING_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_char.h"
#include "std/std_printf.h"
#include "std/std_basic_math.h"
#include "misc/misc_result.h"
#include "base/base_unicode.h"
#include "struct/struct_string.h"

#define MAX_FLOAT_PARSE_LENGTH   64 //characters

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	bool TryParseU64Ex(Str8 str, u64* valueOut, Result* errorOut, bool allowHex, bool allowBinary, bool allowDecimal);
	PIG_CORE_INLINE bool TryParseU64(Str8 str, u64* valueOut, Result* errorOut);
	PIG_CORE_INLINE bool TryParseU32Ex(Str8 str, u32* valueOut, Result* errorOut, bool allowHex, bool allowBinary, bool allowDecimal);
	PIG_CORE_INLINE bool TryParseU32(Str8 str, u32* valueOut, Result* errorOut);
	PIG_CORE_INLINE bool TryParseU16Ex(Str8 str, u16* valueOut, Result* errorOut, bool allowHex, bool allowBinary, bool allowDecimal);
	PIG_CORE_INLINE bool TryParseU16(Str8 str, u16* valueOut, Result* errorOut);
	PIG_CORE_INLINE bool TryParseU8Ex(Str8 str, u8* valueOut, Result* errorOut, bool allowHex, bool allowBinary, bool allowDecimal);
	PIG_CORE_INLINE bool TryParseU8(Str8 str, u8* valueOut, Result* errorOut);
	bool TryParseI64Ex(Str8 str, i64* valueOut, Result* errorOut, bool allowHex, bool allowBinary, bool allowDecimal);
	PIG_CORE_INLINE bool TryParseI64(Str8 str, i64* valueOut, Result* errorOut);
	PIG_CORE_INLINE bool TryParseI32Ex(Str8 str, i32* valueOut, Result* errorOut, bool allowHex, bool allowBinary, bool allowDecimal);
	PIG_CORE_INLINE bool TryParseI32(Str8 str, i32* valueOut, Result* errorOut);
	PIG_CORE_INLINE bool TryParseI16Ex(Str8 str, i16* valueOut, Result* errorOut, bool allowHex, bool allowBinary, bool allowDecimal);
	PIG_CORE_INLINE bool TryParseI16(Str8 str, i16* valueOut, Result* errorOut);
	PIG_CORE_INLINE bool TryParseI8Ex(Str8 str, i8* valueOut, Result* errorOut, bool allowHex, bool allowBinary, bool allowDecimal);
	PIG_CORE_INLINE bool TryParseI8(Str8 str, i8* valueOut, Result* errorOut);
	bool TryParseR64Ex(Str8 str, r64* valueOut, Result* errorOut, bool allowInfinityOrNan);
	PIG_CORE_INLINE bool TryParseR64(Str8 str, r64* valueOut, Result* errorOut);
	bool TryParseR32Ex(Str8 str, r32* valueOut, Result* errorOut, bool allowSuffix, bool allowInfinityOrNan);
	PIG_CORE_INLINE bool TryParseR32(Str8 str, r32* valueOut, Result* errorOut);
	bool TryParseBoolEx(Str8 str, bool* valueOut, Result* errorOut, bool strict);
	PIG_CORE_INLINE bool TryParseBool(Str8 str, bool* valueOut, Result* errorOut);
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#if TARGET_IS_32BIT
#define TryParseUXX(str, valueOutPntr, errorOutPntr) TryParseU32((str), (u32*)(valueOutPntr), (errorOutPntr))
#else
#define TryParseUXX(str, valueOutPntr, errorOutPntr) TryParseU64((str), (u64*)(valueOutPntr), (errorOutPntr))
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

// +==============================+
// |    Parse Unsigned Integer    |
// +==============================+
//TODO: For some reason 18446744073709551616-18446744073709551619 incorrectly succeed parsing and return 0-3
PEXP bool TryParseU64Ex(Str8 str, u64* valueOut, Result* errorOut, bool allowHex, bool allowBinary, bool allowDecimal)
{
	NotNullStr(str);
	Assert(allowDecimal || allowHex || allowBinary);
	str = TrimWhitespace(str);
	u64 result = 0;
	bool foundHexDesignation = (allowHex && !allowDecimal);
	bool foundBinaryDesignation = (allowBinary && !allowDecimal && !allowHex);
	if (allowHex && StrExactStartsWith(str, StrLit("0x"))) { foundHexDesignation = true; str = StrSliceFrom(str, 2); }
	else if (allowBinary && StrExactStartsWith(str, StrLit("0b"))) { foundBinaryDesignation = true; str = StrSliceFrom(str, 2); }
	
	bool foundNumbers = false;
	while (str.length > 0)
	{
		//TODO: Do we need to handle parsing utf-8 multi-byte characters? I think we only care about finding 0-9, period, negative, and A-F/a-f for hex which are all single-byte characters
		u32 codepoint = 0;
		u8 codepointSize = GetCodepointForUtf8Str(str, 0, &codepoint);
		if (codepointSize == 0)
		{
			SetOptionalOutPntr(errorOut, Result_InvalidUtf8);
			return false;
		}
		
		if (foundHexDesignation && codepoint >= '0' && codepoint <= '9')
		{
			if (result > UINT64_MAX / 16ULL)
			{
				SetOptionalOutPntr(errorOut, Result_Overflow);
				return false;
			}
			result = result * 16ULL + (u64)(codepoint - '0');
			foundNumbers = true;
		}
		else if (foundHexDesignation && codepoint >= 'A' && codepoint <= 'F')
		{
			if (result > UINT64_MAX / 16ULL)
			{
				SetOptionalOutPntr(errorOut, Result_Overflow);
				return false;
			}
			result = result * 16ULL + (u64)((codepoint - 'A') + 10);
			foundNumbers = true;
		}
		else if (foundHexDesignation && codepoint >= 'a' && codepoint <= 'f')
		{
			if (result > UINT64_MAX / 16ULL)
			{
				SetOptionalOutPntr(errorOut, Result_Overflow);
				return false;
			}
			result = result * 16ULL + (u64)((codepoint - 'a') + 10);
			foundNumbers = true;
		}
		else if (foundBinaryDesignation && codepoint >= '0' && codepoint <= '1')
		{
			if (result > UINT64_MAX / 2ULL)
			{
				SetOptionalOutPntr(errorOut, Result_Overflow);
				return false;
			}
			result = result * 2ULL + (u64)((codepoint == '1') ? 1 : 0);
			foundNumbers = true;
		}
		else if (!foundHexDesignation && !foundBinaryDesignation && codepoint >= '0' && codepoint <= '9')
		{
			if (result > UINT64_MAX / 10ULL)
			{
				SetOptionalOutPntr(errorOut, Result_Overflow);
				return false;
			}
			result = result * 10ULL + (u64)(codepoint - '0');
			foundNumbers = true;
		}
		else 
		{
			SetOptionalOutPntr(errorOut, Result_InvalidCharacter);
			return false;
		}
		
		str.chars += codepointSize;
		str.length -= codepointSize;
	}
	
	if (!foundNumbers)
	{
		SetOptionalOutPntr(errorOut, Result_NoNumbers);
		return false;
	}
	
	SetOptionalOutPntr(valueOut, result);
	return true;
}
PEXPI bool TryParseU64(Str8 str, u64* valueOut, Result* errorOut)
{
	return TryParseU64Ex(str, valueOut, errorOut, true, true, true);
}

PEXPI bool TryParseU32Ex(Str8 str, u32* valueOut, Result* errorOut, bool allowHex, bool allowBinary, bool allowDecimal)
{
	u64 resultU64 = 0;
	if (!TryParseU64Ex(str, &resultU64, errorOut, allowHex, allowBinary, allowDecimal)) { return false; }
	if (resultU64 > UINT32_MAX)
	{
		SetOptionalOutPntr(errorOut, Result_Overflow);
		return false;
	}
	SetOptionalOutPntr(valueOut, (u32)resultU64);
	return true;
}
PEXPI bool TryParseU32(Str8 str, u32* valueOut, Result* errorOut)
{
	return TryParseU32Ex(str, valueOut, errorOut, true, true, true);
}

PEXPI bool TryParseU16Ex(Str8 str, u16* valueOut, Result* errorOut, bool allowHex, bool allowBinary, bool allowDecimal)
{
	u64 resultU64 = 0;
	if (!TryParseU64Ex(str, &resultU64, errorOut, allowHex, allowBinary, allowDecimal)) { return false; }
	if (resultU64 > UINT16_MAX)
	{
		SetOptionalOutPntr(errorOut, Result_Overflow);
		return false;
	}
	SetOptionalOutPntr(valueOut, (i16)resultU64);
	return true;
}
PEXPI bool TryParseU16(Str8 str, u16* valueOut, Result* errorOut)
{
	return TryParseU16Ex(str, valueOut, errorOut, true, true, true);
}

PEXPI bool TryParseU8Ex(Str8 str, u8* valueOut, Result* errorOut, bool allowHex, bool allowBinary, bool allowDecimal)
{
	u64 resultU64 = 0;
	if (!TryParseU64Ex(str, &resultU64, errorOut, allowHex, allowBinary, allowDecimal)) { return false; }
	if (resultU64 > UINT8_MAX)
	{
		SetOptionalOutPntr(errorOut, Result_Overflow);
		return false;
	}
	SetOptionalOutPntr(valueOut, (i8)resultU64);
	return true;
}
PEXPI bool TryParseU8(Str8 str, u8* valueOut, Result* errorOut)
{
	return TryParseU8Ex(str, valueOut, errorOut, true, true, true);
}

// +==============================+
// |     Parse Signed Integer     |
// +==============================+
PEXP bool TryParseI64Ex(Str8 str, i64* valueOut, Result* errorOut, bool allowHex, bool allowBinary, bool allowDecimal)
{
	NotNullStr(str);
	str = TrimWhitespace(str);
	bool isNegative = false;
	if (StrExactStartsWith(str, StrLit("-"))) { isNegative = true; str = StrSliceFrom(str, 1); }
	else if (StrExactStartsWith(str, StrLit("+"))) { isNegative = false; str = StrSliceFrom(str, 1); }
	u64 resultU64 = 0;
	if (!TryParseU64Ex(str, &resultU64, errorOut, allowHex, allowBinary, allowDecimal)) { return false; }
	if (!isNegative && resultU64 > INT64_MAX)
	{
		SetOptionalOutPntr(errorOut, Result_Overflow);
		return false;
	}
	if (isNegative && resultU64 > (u64)INT64_MAX + 1)
	{
		SetOptionalOutPntr(errorOut, Result_Underflow);
		return false;
	}
	if (valueOut != nullptr)
	{
		if (isNegative)
		{
			resultU64 = (~resultU64) + 1;
			*valueOut = *((i64*)&resultU64);
		}
		else
		{
			*valueOut = (i64)resultU64;
		}
	}
	return true;
}
PEXPI bool TryParseI64(Str8 str, i64* valueOut, Result* errorOut)
{
	return TryParseI64Ex(str, valueOut, errorOut, true, true, true);
}

PEXPI bool TryParseI32Ex(Str8 str, i32* valueOut, Result* errorOut, bool allowHex, bool allowBinary, bool allowDecimal)
{
	NotNullStr(str);
	i64 resultI64 = 0;
	if (!TryParseI64Ex(str, &resultI64, errorOut, allowHex, allowBinary, allowDecimal)) { return false; }
	if (resultI64 > INT32_MAX)
	{
		SetOptionalOutPntr(errorOut, Result_Overflow);
		return false;
	}
	if (resultI64 < INT32_MIN)
	{
		SetOptionalOutPntr(errorOut, Result_Underflow);
		return false;
	}
	SetOptionalOutPntr(valueOut, (i32)resultI64);
	return true;
}
PEXPI bool TryParseI32(Str8 str, i32* valueOut, Result* errorOut)
{
	return TryParseI32Ex(str, valueOut, errorOut, true, true, true);
}

PEXPI bool TryParseI16Ex(Str8 str, i16* valueOut, Result* errorOut, bool allowHex, bool allowBinary, bool allowDecimal)
{
	NotNullStr(str);
	i64 resultI64 = 0;
	if (!TryParseI64Ex(str, &resultI64, errorOut, allowHex, allowBinary, allowDecimal)) { return false; }
	if (resultI64 > INT16_MAX)
	{
		SetOptionalOutPntr(errorOut, Result_Overflow);
		return false;
	}
	if (resultI64 < INT16_MIN)
	{
		SetOptionalOutPntr(errorOut, Result_Underflow);
		return false;
	}
	SetOptionalOutPntr(valueOut, (i16)resultI64);
	return true;
}
PEXPI bool TryParseI16(Str8 str, i16* valueOut, Result* errorOut)
{
	return TryParseI16Ex(str, valueOut, errorOut, true, true, true);
}

PEXPI bool TryParseI8Ex(Str8 str, i8* valueOut, Result* errorOut, bool allowHex, bool allowBinary, bool allowDecimal)
{
	NotNullStr(str);
	i64 resultI64 = 0;
	if (!TryParseI64Ex(str, &resultI64, errorOut, allowHex, allowBinary, allowDecimal)) { return false; }
	if (resultI64 > INT8_MAX)
	{
		SetOptionalOutPntr(errorOut, Result_Overflow);
		return false;
	}
	if (resultI64 < INT8_MIN)
	{
		SetOptionalOutPntr(errorOut, Result_Underflow);
		return false;
	}
	SetOptionalOutPntr(valueOut, (i8)resultI64);
	return true;
}
PEXPI bool TryParseI8(Str8 str, i8* valueOut, Result* errorOut)
{
	return TryParseI8Ex(str, valueOut, errorOut, true, true, true);
}

// +==============================+
// |         Parse Float          |
// +==============================+
PEXP bool TryParseR64Ex(Str8 str, r64* valueOut, Result* errorOut, bool allowInfinityOrNan)
{
	NotNullStr(str);
	str = TrimWhitespace(str);
	if (str.length > MAX_FLOAT_PARSE_LENGTH)
	{
		SetOptionalOutPntr(errorOut, Result_StringOverflow);
		return false;
	}
	if (str.length == 0)
	{
		SetOptionalOutPntr(errorOut, Result_EmptyString);
		return false;
	}
	
	r64 result = 0.0;
	
	if (StrAnyCaseEquals(str, StrLit("nan"))) { if (allowInfinityOrNan) { result = NAN; } else { SetOptionalOutPntr(errorOut, Result_InfinityOrNan); return false; } }
	else if (StrAnyCaseEquals(str, StrLit("inf"))) {  if (allowInfinityOrNan) { result = INFINITY; } else { SetOptionalOutPntr(errorOut, Result_InfinityOrNan); return false; } }
	else if (StrAnyCaseEquals(str, StrLit("+inf"))) {  if (allowInfinityOrNan) { result = INFINITY; } else { SetOptionalOutPntr(errorOut, Result_InfinityOrNan); return false; } }
	else if (StrAnyCaseEquals(str, StrLit("-inf"))) {  if (allowInfinityOrNan) { result = -INFINITY; } else { SetOptionalOutPntr(errorOut, Result_InfinityOrNan); return false; } }
	else if (StrAnyCaseEquals(str, StrLit("infinity"))) {  if (allowInfinityOrNan) { result = INFINITY; } else { SetOptionalOutPntr(errorOut, Result_InfinityOrNan); return false; } }
	else if (StrAnyCaseEquals(str, StrLit("+infinity"))) {  if (allowInfinityOrNan) { result = INFINITY; } else { SetOptionalOutPntr(errorOut, Result_InfinityOrNan); return false; } }
	else if (StrAnyCaseEquals(str, StrLit("-infinity"))) {  if (allowInfinityOrNan) { result = -INFINITY; } else { SetOptionalOutPntr(errorOut, Result_InfinityOrNan); return false; } }
	else
	{
		u64 bIndex = 0;
		
		bool isNegative = false;
		if (bIndex < str.length && str.chars[bIndex] == '+') { bIndex++; isNegative = false; }
		else if (bIndex < str.length && str.chars[bIndex] == '-') { bIndex++; isNegative = true; }
		
		bool foundNumbersBeforePeriod = false;
		for (; bIndex < str.length; bIndex++)
		{
			char nextChar = str.chars[bIndex];
			if (!IsCharNumeric(nextChar)) { break; }
			foundNumbersBeforePeriod = true;
			result = (result * 10.0) + (r64)GetNumericCharValue(nextChar);
		}
		
		// bool foundPeriod = false;
		bool foundNumbersAfterPeriod = false;
		if (bIndex < str.length && str.chars[bIndex] == '.')
		{
			bIndex++;
			// foundPeriod = true;
			
			r64 digitInvPower = 10.0;
			for (; bIndex < str.length; bIndex++)
			{
				char nextChar = str.chars[bIndex];
				if (!IsCharNumeric(nextChar)) { break; }
				foundNumbersAfterPeriod = true;
				result = result + ((r64)GetNumericCharValue(nextChar) / digitInvPower);
				digitInvPower *= 10.0;
			}
		}
		
		// bool foundScientificNotation = false;
		bool negativeExponent = false;
		r64 exponentMultiplier = 1.0;
		if (bIndex < str.length && (str.chars[bIndex] == 'e' || str.chars[bIndex] == 'E'))
		{
			bIndex++;
			// foundScientificNotation = true;
			
			if (bIndex < str.length && str.chars[bIndex] == '+') { bIndex++; negativeExponent = false; }
			else if (bIndex < str.length && str.chars[bIndex] == '-') { bIndex++; negativeExponent = true; }
			
			bool foundExponentNumbers = false;
			r64 exponent = 0.0;
			for (; bIndex < str.length; bIndex++)
			{
				char nextChar = str.chars[bIndex];
				if (!IsCharNumeric(nextChar)) { break; }
				foundExponentNumbers = true;
				exponent = (exponent * 10.0) + (r64)GetNumericCharValue(nextChar);
			}
			if (exponent > 308.0) { exponent = 308.0; }
			
			if (!foundExponentNumbers)
			{
				if (bIndex >= str.length)
				{
					SetOptionalOutPntr(errorOut, Result_MissingExponent);
					return false;
				}
				else
				{
					//NOTE: If we didn't make it to the end, there is some non-numeric
					//      character that prevented the loop from continuing. We can
					//      consider it an invalid character no matter what it is at this point.
					SetOptionalOutPntr(errorOut, Result_InvalidCharacter);
					return false;
				}
			}
			
			while (exponent >= 50.0) { exponentMultiplier *= 1e50; exponent -= 50.0; }
			while (exponent >= 8.0)  { exponentMultiplier *= 1e8;  exponent -= 8.0;  }
			while (exponent >  0.0)  { exponentMultiplier *= 10.0; exponent -= 1.0;  }
		}
		
		// If we haven't made it to the end of the string then there must be some
		// invalid character that blocked the loops above from continuing
		if (bIndex < str.length)
		{
			SetOptionalOutPntr(errorOut, Result_InvalidCharacter);
			return false;
		}
		
		if (!foundNumbersBeforePeriod && !foundNumbersAfterPeriod)
		{
			SetOptionalOutPntr(errorOut, Result_NoNumbers);
			return false;
		}
		
		result = (isNegative ? -1.0 : 1.0) * (negativeExponent ? (result / exponentMultiplier) : (result * exponentMultiplier));
	}
	
	SetOptionalOutPntr(valueOut, result);
	return true;
}
PEXPI bool TryParseR64(Str8 str, r64* valueOut, Result* errorOut)
{
	return TryParseR64Ex(str, valueOut, errorOut, false);
}

PEXP bool TryParseR32Ex(Str8 str, r32* valueOut, Result* errorOut, bool allowSuffix, bool allowInfinityOrNan)
{
	NotNullStr(str);
	str = TrimWhitespace(str);
	if (allowSuffix && str.length > 0 && str.chars[str.length-1] == 'f')
	{
		str.length--;
	}
	if (str.length > MAX_FLOAT_PARSE_LENGTH)
	{
		SetOptionalOutPntr(errorOut, Result_StringOverflow);
		return false;
	}
	if (str.length == 0)
	{
		SetOptionalOutPntr(errorOut, Result_NoNumbers);
		return false;
	}
	
	//We need a local buffer to ensure the string is null-terminated (a requirement of atof)
	char localBuffer[MAX_FLOAT_PARSE_LENGTH+1];
	MyMemCopy(&localBuffer[0], str.chars, str.length);
	localBuffer[str.length] = '\0';
	Str8 tempStr = MakeStr8(str.length, &localBuffer[0]);
	
	r64 resultR64 = MyStrToFloat(tempStr.chars);
	if (!allowInfinityOrNan && IsInfiniteOrNanR64(resultR64))
	{
		SetOptionalOutPntr(errorOut, Result_InfinityOrNan);
		return false;
	}
	//atof doesn't really give us errors. When it fails it gives us 0.0.
	//We are going to do some sanity checks to try and tell between a failed parse and an actual parse of 0.0
	if (resultR64 == 0.0)
	{
		bool containsZero = false;
		bool containsOtherNumbers = false;
		for (u64 cIndex = 0; cIndex < tempStr.length; cIndex++)
		{
			if (tempStr.chars[cIndex] == '0') { containsZero = true; break; }
			if (tempStr.chars[cIndex] >= '1' && tempStr.chars[cIndex] <= '9') { containsOtherNumbers = true; break; }
		}
		if (!containsZero || containsOtherNumbers)
		{
			SetOptionalOutPntr(errorOut, Result_FloatParseFailure);
			return false;
		}
	}
	
	SetOptionalOutPntr(valueOut, (r32)resultR64);
	return true;
}
PEXPI bool TryParseR32(Str8 str, r32* valueOut, Result* errorOut)
{
	return TryParseR32Ex(str, valueOut, errorOut, true, false);
}

// +==============================+
// |          Parse Bool          |
// +==============================+
PEXP bool TryParseBoolEx(Str8 str, bool* valueOut, Result* errorOut, bool strict)
{
	NotNullStr(str);
	str = TrimWhitespace(str);
	bool result = false;
	if      (StrAnyCaseEquals(str, StrLit("true")))         { result = true;  }
	else if (StrAnyCaseEquals(str, StrLit("false")))        { result = false; }
	else if (StrAnyCaseEquals(str, StrLit("t")) && !strict) { result = true;  }
	else if (StrAnyCaseEquals(str, StrLit("f")) && !strict) { result = false; }
	else if (StrExactEquals(str, StrLit("1")) && !strict)   { result = true;  }
	else if (StrExactEquals(str, StrLit("0")) && !strict)   { result = false; }
	else if (StrAnyCaseEquals(str, StrLit("yes")))          { result = true;  }
	else if (StrAnyCaseEquals(str, StrLit("no")))           { result = false; }
	else if (StrAnyCaseEquals(str, StrLit("y")) && !strict) { result = true;  }
	else if (StrAnyCaseEquals(str, StrLit("n")) && !strict) { result = false; }
	else if (StrAnyCaseEquals(str, StrLit("on")))           { result = true;  }
	else if (StrAnyCaseEquals(str, StrLit("off")))          { result = false; }
	else if (StrAnyCaseEquals(str, StrLit("enable")))       { result = true;  }
	else if (StrAnyCaseEquals(str, StrLit("disable")))      { result = false; }
	else if (StrAnyCaseEquals(str, StrLit("enabled")))      { result = true;  }
	else if (StrAnyCaseEquals(str, StrLit("disabled")))     { result = false; }
	else if (StrAnyCaseEquals(str, StrLit("high")))         { result = true;  }
	else if (StrAnyCaseEquals(str, StrLit("low")))          { result = false; }
	else
	{
		SetOptionalOutPntr(errorOut, Result_UnknownString);
		return false;
	}
	SetOptionalOutPntr(valueOut, result);
	return true;
}
PEXPI bool TryParseBool(Str8 str, bool* valueOut, Result* errorOut)
{
	return TryParseBoolEx(str, valueOut, errorOut, false);
}

//TODO: Move this to a cross_parsing_and_uuid.h file if we ever add struct_uuid.h
#if 0
bool TryParseUuid(Str8 str, Uuid_t* valueOut, Result* errorOut)
{
	NotNullStr(str);
	if (str.length < UUID_STR_LENGTH_NO_HYPHENS) { SetOptionalOutPntr(errorOut, Result_TooShort); return false; }
	if (str.length > UUID_STR_LENGTH) { SetOptionalOutPntr(errorOut, Result_TooLong); return false; }
	u8 byteIndex = 0;
	Uuid_t result = ZEROED;
	for (u64 cIndex = 0; cIndex < str.length; )
	{
		if (str.chars[cIndex] == '-') { cIndex++; continue; }
		if (!IsCharHexadecimal(str.chars[cIndex+0])) { SetOptionalOutPntr(errorOut, Result_InvalidCharacter); return false; }
		if (!IsCharHexadecimal(str.chars[cIndex+1])) { SetOptionalOutPntr(errorOut, Result_InvalidCharacter); return false; }
		if (cIndex+2 > str.length) { SetOptionalOutPntr(errorOut, Result_WrongNumCharacters); return false; }
		if (byteIndex >= UUID_BYTE_LENGTH) { SetOptionalOutPntr(errorOut, Result_TooLong); return false; }
		u8 insertByteIndex = 0;
		if (byteIndex < 4) { insertByteIndex = (3 - byteIndex); }
		else if (byteIndex < 4+2) { insertByteIndex = 4 + (1 - (byteIndex - 4)); }
		else if (byteIndex < 4+2+2) { insertByteIndex = 6 + (1 - (byteIndex - 6)); }
		else if (byteIndex < 4+2+2+2) { insertByteIndex = 8 + (1 - (byteIndex - 8)); }
		else if (byteIndex < 4+2+2+2+2) { insertByteIndex = 10 + (1 - (byteIndex - 10)); }
		else { insertByteIndex = 12 + (3 - (byteIndex - 12)); }
		result.bytes[insertByteIndex] = ((GetHexCharValue(str.chars[cIndex + 0]) << 4) | (GetHexCharValue(str.chars[cIndex + 1]) << 0)); 
		byteIndex++;
		cIndex += 2;
	}
	SetOptionalOutPntr(valueOut, result);
	return true;
}
Uuid_t ParseUuid(Str8 str)
{
	Uuid_t result;
	Result failureReason;
	bool parseSuccess = TryParseUuid(str, &result, &failureReason);
	Assert(parseSuccess);
	return result;
}
#endif

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _MISC_PARSING_H

#if defined(_MISC_PARSING_H) && defined(_STRUCT_VECTORS_H)
#include "cross/cross_parsing_and_vectors.h"
#endif

#if defined(_MISC_PARSING_H) && defined(_STRUCT_MATRICES_H)
#include "cross/cross_parsing_and_matrices.h"
#endif

#if defined(_MISC_PARSING_H) && defined(_STRUCT_DIRECTIONS_H)
#include "cross/cross_parsing_and_directions.h"
#endif

#if defined(_MISC_PARSING_H) && defined(_STRUCT_RECTANGLES_H)
#include "cross/cross_parsing_and_rectangles.h"
#endif

#if defined(_MISC_PARSING_H) && defined(_STRUCT_COLOR_H)
#include "cross/cross_parsing_and_color.h"
#endif
