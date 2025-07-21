/*
File:   base_unicode.h
Author: Taylor Robbins
Date:   01\06\2025
Description:
	** Holds functions that help us convert codepoints to\from various encodings
	** like ASCII, UTF-8, UCS2, UTF-16, etc. The file is named Unicode simply because
	** the Unicode standard acts as a source of truth for most of these encodings.
	** If something is "not encoded" then it's a u32 that refers to a particular
	** codepoint in the Unicode standard.
	** Along with encoding conversions, this file also contains conversions and
	** comparisons between codepoints, and a few functions that operate on codepoints
	** and some defines for various codepoint pages, maximums, etc.
*/

#ifndef _BASE_UNICODE_H
#define _BASE_UNICODE_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "base/base_char.h"

#define UTF8_MAX_CODEPOINT 0x10FFFFUL
#define UCS2_MAX_CHAR_SIZE 2 //words
#define UTF8_MAX_CHAR_SIZE 4 //bytes

#define NUM_LETTERS_IN_ENGLISH_ALPHABET 26

#define UNICODE_LATIN_EXT_START 0x000000A0UL
#define UNICODE_LATIN_EXT_COUNT 96
#define UNICODE_LATIN_EXT_END   (UNICODE_LATIN_EXT_START + UNICODE_LATIN_EXT_COUNT)

#define UNICODE_CYRILLIC_START 0x00000400UL
#define UNICODE_CYRILLIC_COUNT 256
#define UNICODE_CYRILLIC_END   (UNICODE_CYRILLIC_START + UNICODE_CYRILLIC_COUNT)

#define UNICODE_HIRAGANA_START 0x000003041UL
#define UNICODE_HIRAGANA_COUNT 95
#define UNICODE_HIRAGANA_END   (UNICODE_HIRAGANA_START + UNICODE_HIRAGANA_COUNT)

#define UNICODE_KATAKANA_START 0x0000030A0UL
#define UNICODE_KATAKANA_COUNT 96
#define UNICODE_KATAKANA_END   (UNICODE_KATAKANA_START + UNICODE_KATAKANA_COUNT)

#define UNICODE_ELLIPSIS_CODEPOINT 0x2026 //Technically called "Horizontal Ellipsis"
#define UNICODE_ELLIPSIS_STR       "\xE2\x80\xA6" //UTF-8 encoding

#define UNICODE_RIGHT_ARROW_CODEPOINT 0x203A //Technically called "Single Right-Pointing Angle Quotation Mark"
#define UNICODE_RIGHT_ARROW_STR       "\xE2\x80\xBA" //UTF-8 encoding

// Basic Multilingual Plane Private Use Area: 0xE000-0xF8FF
#define CUSTOM_CODEPOINT_START        0xE000
#define CUSTOM_CODEPOINT_END          0xF900

#define CUSTOM_CODEPOINT_FIRST        0xF900
#define CUSTOM_CODEPOINT_LAST         0xF8FF

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE u32 GetLowercaseCodepoint(u32 codepoint);
	PIG_CORE_INLINE u32 GetUppercaseCodepoint(u32 codepoint);
	u8 GetUtf8BytesForCode(u32 codepoint, u8* byteBufferOut, bool doAssertions);
	u8 GetCodepointUtf8Size(u32 codepoint);
	u8 GetCodepointForUtf8(u64 maxNumBytes, const char* strPntr, u32* codepointOut);
	u8 GetPrevCodepointForUtf8(u64 numBytesBeforePntr, const char* strEndPntr, u32* codepointOut);
	u8 GetCodepointBeforeIndex(const char* strPntr, u64 startIndex, u32* codepointOut);
	i32 CompareCodepoints(u32 codepoint1, u32 codepoint2);
	bool DoesNtStrContainMultibyteUtf8Chars(const char* nullTermStr);
	u8 GetUcs2WordsForCode(u32 codepoint, u16* wordBufferOut, bool doAssertions);
	u8 GetCodepointForUcs2(u64 maxNumWords, const u16* strPntr, u32* codepointOut);
	PIG_CORE_INLINE u32 GetMonospaceCodepointFor(u32 codepoint);
	PIG_CORE_INLINE u32 GetRegularCodepointForMonospace(u32 monospaceCodepoint);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI u32 GetLowercaseCodepoint(u32 codepoint)
{
	if (codepoint >= 'A' && codepoint <= 'Z') { return 'a' + (codepoint - 'A'); }
	return codepoint;
}
PEXPI u32 GetUppercaseCodepoint(u32 codepoint)
{
	if (codepoint >= 'a' && codepoint <= 'z') { return 'A' + (codepoint - 'z'); }
	return codepoint;
}

// +--------------------------------------------------------------+
// |                       UTF-8 Functions                        |
// +--------------------------------------------------------------+
//returns number of bytes needed to store this codepoint in UTF-8 and stores the values in byteBufferOut if not nullptr
//NOTE: byteBufferOut is assumed to be 4 bytes or greater and no null-terminating character is written to the buffer
PEXP u8 GetUtf8BytesForCode(u32 codepoint, u8* byteBufferOut, bool doAssertions)
{
	UNUSED(doAssertions);
	if (codepoint <= 0x7F)
	{
		//0xxx xxxx
		if (byteBufferOut != nullptr) { byteBufferOut[0] = (u8)codepoint; }
		return 1;
	}
	else if (codepoint <= 0x7FF)
	{
		//110x xxxx 10xx xxxx
		if (byteBufferOut != nullptr)
		{
			byteBufferOut[0] = (0xC0 | ((codepoint >> 6) & 0x1F));
			byteBufferOut[1] = (0x80 | (codepoint & 0x3F));
		}
		return 2;
	}
	else if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
	{
		//invalid block
		Assert(!doAssertions || (false && "Invalid codepoint for UTF8"));
		if (byteBufferOut != nullptr) { byteBufferOut[0] = '\0'; }
		return 0;
	}
	else if (codepoint <= 0xFFFF)
	{
		//1110 xxxx 10xx xxxx 10xx xxxx
		if (byteBufferOut != nullptr)
		{
			byteBufferOut[0] = (0xE0 | ((codepoint >> 12) & 0x0F));
			byteBufferOut[1] = (0x80 | ((codepoint>>6) & 0x3F));
			byteBufferOut[2] = (0x80 | (codepoint & 0x3F));
		}
		return 3;
	}
	else if (codepoint <= UTF8_MAX_CODEPOINT)
	{
		//1111 0xxx 10xx xxxx 10xx xxxx 10xx xxxx
		if (byteBufferOut != nullptr)
		{
			byteBufferOut[0] = (0xF0 | ((codepoint >> 18) & 0x07));
			byteBufferOut[1] = (0x80 | ((codepoint>>12) & 0x3F));
			byteBufferOut[2] = (0x80 | ((codepoint>>6) & 0x3F));
			byteBufferOut[3] = (0x80 | (codepoint & 0x3F));
		}
		return 4;
	}
	else
	{
		//everything above this point is also invalid
		Assert(!doAssertions || (false && "Invalid codepoint value for UTF8"));
		if (byteBufferOut != nullptr) { byteBufferOut[0] = '\0'; }
		return 0;
	}
}
PEXPI u8 GetCodepointUtf8Size(u32 codepoint)
{
	if (codepoint <= 0x7F) { return 1; }
	else if (codepoint <= 0x7FF) { return 2; }
	else if (codepoint >= 0xD800 && codepoint <= 0xDFFF) { return 0; } //invalid block
	else if (codepoint <= 0xFFFF) { return 3; }
	else if (codepoint <= UTF8_MAX_CODEPOINT) { return 4; }
	else { return 0; } //everything above this point is also invalid
}

//Returns the number of bytes consumed to complete the UTF-8 encoded character pointed to by strPntr
//If more bytes are needed for a full UTF-8 character than is specified by maxNumBytes then we return 0
//If an invalid encoding is detected then we return 0. If maxNumBytes == 0 then we return 0.
//TODO: Should we accept characters that are technically valid but encoded in a larger number of bytes than needed?
//      Like a 4-byte encoding of \0 would be dumb because it could cause non-null-terminated UTF-8 strings to become null terminated collapsed ASCII strings
PEXP u8 GetCodepointForUtf8(u64 maxNumBytes, const char* strPntr, u32* codepointOut) //somewhat tested
{
	Assert(strPntr != nullptr || maxNumBytes == 0);
	SetOptionalOutPntr(codepointOut, 0);
	if (maxNumBytes == 0) { return 0; }
	const u8* bytePntr = (const u8*)strPntr;
	if (bytePntr[0] <= 127)
	{
		SetOptionalOutPntr(codepointOut, (u32)bytePntr[0]);
		return 1;
	}
	else if (bytePntr[0] < 0xC0)
	{
		//An 10xx xxxx format for the first byte of a character is invalid
		return 0;
	}
	else if (bytePntr[0] < 0xE0)
	{
		if (maxNumBytes < 2) { return 0; }
		if (bytePntr[1] < 0x80 || bytePntr[1] >= 0xC0) { return 0; }
		SetOptionalOutPntr(codepointOut, ((u32)(bytePntr[0] & 0x1F) << 6) | ((u32)(bytePntr[1] & 0x3F) << 0));
		return 2;
	}
	else if (bytePntr[0] < 0xF0)
	{
		if (maxNumBytes < 3) { return 0; }
		if (bytePntr[1] < 0x80 || bytePntr[1] >= 0xC0) { return 0; }
		if (bytePntr[2] < 0x80 || bytePntr[2] >= 0xC0) { return 0; }
		SetOptionalOutPntr(codepointOut, ((u32)(bytePntr[0] & 0x0F) << 12) | ((u32)(bytePntr[1] & 0x3F) << 6) | ((u32)(bytePntr[2] & 0x3F) << 0));
		return 3;
	}
	else if (bytePntr[0] < 0xF8)
	{
		if (maxNumBytes < 4) { return 0; }
		if (bytePntr[1] < 0x80 || bytePntr[1] >= 0xC0) { return 0; }
		if (bytePntr[2] < 0x80 || bytePntr[2] >= 0xC0) { return 0; }
		if (bytePntr[3] < 0x80 || bytePntr[3] >= 0xC0) { return 0; }
		SetOptionalOutPntr(codepointOut, ((u32)(bytePntr[0] & 0x07) << 18) | ((u32)(bytePntr[1] & 0x3F) << 12) | ((u32)(bytePntr[2] & 0x3F) << 6) | ((u32)(bytePntr[3] & 0x3F) << 0));
		return 4;
	}
	else
	{
		//Everything above this point is considered an invalid character to exist in UTF-8 encoded strings
		return 0;
	}
}

// This is sort of like GetCodepointForUtf8 but it walks backwards from the pointer until it finds a valid UTF-8 byte sequence
PEXP u8 GetPrevCodepointForUtf8(u64 numBytesBeforePntr, const char* strEndPntr, u32* codepointOut)
{
	// The first byte is rather special in a UTF-8 character, we will call it a "prefix byte" but it probably technically should be called something like "non-continuation" byte
	u8 codepointSize = 0;
	for (u8 bIndex = 1; bIndex <= UTF8_MAX_CHAR_SIZE && bIndex < numBytesBeforePntr; bIndex++)
	{
		u8 prevByte = *(((const u8*)strEndPntr) - bIndex);
		if (prevByte <= 127) //single byte
		{
			if (bIndex != 1) { return 0; } //invalid UTF-8, some number of continuation bytes preceeded by a single-byte character
			codepointSize = 1;
			break;
		}
		else if (prevByte < 0xC0) //continuation byte
		{
			continue;
		}
		else if (prevByte < 0xE0) //prefix byte that declares a 2-byte character
		{
			if (bIndex != 2) { return 0; } //invalid UTF-8, we may be in the middle of the character!
			codepointSize = 2;
			break;
		}
		else if (prevByte < 0xF0) //prefix byte that declares a 3-byte character
		{
			if (bIndex != 3) { return 0; } //invalid UTF-8, we may be in the middle of the character!
			codepointSize = 3;
			break;
		}
		else if (prevByte < 0xF8) //prefix byte that declares a 4-byte character
		{
			if (bIndex != 4) { return 0; } //invalid UTF-8, we may be in the middle of the character!
			codepointSize = 4;
			break;
		}
		else
		{
			//Everything above this point is considered an invalid character to exist in UTF-8 encoded strings
			return 0;
		}
	}
	if (codepointSize == 0) { return 0; } //invalid UTF-8, we didn't find a prefix byte within 4 bytes
	if (codepointOut != nullptr)
	{
		u8 calcSize = GetCodepointForUtf8(codepointSize, strEndPntr - codepointSize, codepointOut);
		DebugAssert(calcSize = codepointSize); UNUSED(calcSize);
	}
	return codepointSize;
}

//Using the startIndex as a known max length to walk backwards this function will look backwards through a string until it finds a full encoded character
//Returns the number of bytes that encoded character took up and stores the codepoint for it in codepointOut
PEXP u8 GetCodepointBeforeIndex(const char* strPntr, u64 startIndex, u32* codepointOut)
{
	if (startIndex == 0) { return 0; }
	NotNull(strPntr);
	for (u8 encodedSize = 1; encodedSize <= UTF8_MAX_CHAR_SIZE && encodedSize <= startIndex; encodedSize++)
	{
		char thisChar = strPntr[startIndex - encodedSize];
		u8 thisByte = CharToU8(thisChar);
		if (thisByte <= 127)
		{
			if (codepointOut != nullptr) { *codepointOut = (u32)thisByte; }
			return 1;
		}
		else if (thisByte < 0xC0)
		{
			//The 10xx xxxx format means a second/third/fourth byte in a sequence, keep walking
		}
		else if (thisByte < 0xE0)
		{
			if (encodedSize != 2) { return 0; } //We walked either too many characters before finding this 2-byte encoding start, or not enough
			u8 nextByte = CharToU8(strPntr[startIndex - 1]);
			if (nextByte < 0x80 || nextByte >= 0xC0) { return 0; } //not sure how this would happen?
			if (codepointOut != nullptr) { *codepointOut = ((u32)(thisByte & 0x1F) << 6) | ((u32)(nextByte & 0x3F) << 0); }
			return 2;
		}
		else if (thisByte < 0xF0)
		{
			if (encodedSize != 3) { return 0; } //We walked either too many characters before finding this 2-byte encoding start, or not enough
			u8 nextByte = CharToU8(strPntr[startIndex - 2]);
			u8 nextNextByte = CharToU8(strPntr[startIndex - 1]);
			if (nextByte < 0x80 || nextByte >= 0xC0) { return 0; } //not sure how this would happen?
			if (nextNextByte < 0x80 || nextNextByte >= 0xC0) { return 0; } //not sure how this would happen?
			if (codepointOut != nullptr) { *codepointOut = ((u32)(thisByte & 0x0F) << 12) | ((u32)(nextByte & 0x3F) << 6) | ((u32)(nextNextByte & 0x3F) << 0); }
			return 3;
		}
		else if (thisByte < 0xF8)
		{
			if (encodedSize != 4) { return 0; } //We walked either too many characters before finding this 2-byte encoding start, or not enough
			u8 nextByte = CharToU8(strPntr[startIndex - 3]);
			u8 nextNextByte = CharToU8(strPntr[startIndex - 2]);
			u8 nextNextNextByte = CharToU8(strPntr[startIndex - 1]);
			if (nextByte < 0x80 || nextByte >= 0xC0) { return 0; } //not sure how this would happen?
			if (nextNextByte < 0x80 || nextNextByte >= 0xC0) { return 0; } //not sure how this would happen?
			if (nextNextNextByte < 0x80 || nextNextNextByte >= 0xC0) { return 0; } //not sure how this would happen?
			if (codepointOut != nullptr) { *codepointOut = ((u32)(thisByte & 0x07) << 18) | ((u32)(nextByte & 0x3F) << 12) | ((u32)(nextNextByte & 0x3F) << 6) | ((u32)(nextNextNextByte & 0x3F) << 0); }
			return 4;
		}
		else
		{
			//Everything above this point is considered an invalid character to exist in UTF-8 encoded strings
			return 0;
		}
	}
	//We had to walk farther than 4 bytes (or the length of the string). Something is wrong with this encoding
	return 0;
}

//Compares two characters for alphabetic order
PEXP i32 CompareCodepoints(u32 codepoint1, u32 codepoint2)
{
	if (codepoint1 == codepoint2) { return 0; }
	u32 majorClass1 = 0;
	if (IsCharNumeric(codepoint1)) { majorClass1 = 1; }
	if (IsCharAlphabetic(codepoint1)) { majorClass1 = 2; }
	u32 majorClass2 = 0;
	if (IsCharNumeric(codepoint2)) { majorClass2 = 1; }
	if (IsCharAlphabetic(codepoint2)) { majorClass2 = 2; }
	if (majorClass1 < majorClass2) { return -1; }
	else if (majorClass1 > majorClass2) { return 1; }
	if (majorClass1 == 2) //alphabet
	{
		u32 lower1 = GetLowercaseCodepoint(codepoint1);
		u32 lower2 = GetLowercaseCodepoint(codepoint2);
		if (lower1 < lower2) { return -1; }
		else if (lower1 > lower2) { return 1; }
		else { return 0; }
	}
	if (codepoint1 < codepoint2) { return -1; }
	else if (codepoint1 > codepoint2) { return 1; }
	else { return 0; }
}

PEXP bool DoesNtStrContainMultibyteUtf8Chars(const char* nullTermStr)
{
	for (uxx bIndex = 0; nullTermStr[bIndex] != '\0'; bIndex++)
	{
		u8 numCharsLeft = nullTermStr[bIndex+1] == '\0' ? 1
			: (nullTermStr[bIndex+2] == '\0' ? 2
			: (nullTermStr[bIndex+3] == '\0' ? 3
			: 4));
		if (GetCodepointForUtf8(numCharsLeft, nullTermStr + bIndex, nullptr) > 0) { return true; }
	}
	return false;
}

// +--------------------------------------------------------------+
// |                       UCS-2 Functions                        |
// +--------------------------------------------------------------+
//returns number of 16-bit words needed to store this codepoint in UCS-2 and stores the values in wordBufferOut if not nullptr
//NOTE: wordBufferOut is assumed to be 2 words or greater and no null-terminating character is written to the buffer
PEXP u8 GetUcs2WordsForCode(u32 codepoint, u16* wordBufferOut, bool doAssertions) //untested
{
	if (codepoint >= 0x0000 && codepoint <= 0xD7FF)
	{
		wordBufferOut[0] = (u16)codepoint;
		return 1;
	}
	else if (codepoint >= 0xE000 && codepoint <= 0xFFFF)
	{
		wordBufferOut[0] = (u16)codepoint;
		return 1;
	}
	else if (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
	{
		//0xD800-0xDFFF is the "surrogate" area where no unicode points exist so we use it to form "surrogate pairs" in UCS-2
		wordBufferOut[0] = 0xD800 + (u16)(((codepoint - 0x10000) >> 10) & 0x03FF);
		wordBufferOut[1] = 0xDC00 + (u16)(((codepoint - 0x10000) >>  0) & 0x03FF);
		return 2;
	}
	else
	{
		if (doAssertions) { AssertMsg(false, "Codepoint can't be converted to UCS-2. Outside supported range of characters"); }
		return 0;
	}
}

PEXP u8 GetCodepointForUcs2(u64 maxNumWords, const u16* strPntr, u32* codepointOut)
{
	UNUSED(maxNumWords);
	UNUSED(strPntr);
	UNUSED(codepointOut);
	Assert(false); //TODO: Implement me!
	return 0;
}

// +--------------------------------------------------------------+
// |                     Bespoke Conversions                      |
// +--------------------------------------------------------------+
// Works on all alphanumeric characters
PEXPI u32 GetMonospaceCodepointFor(u32 codepoint)
{
	if (codepoint >= 'A' && codepoint <= 'Z') { return 0x1D670 + (codepoint - 'A'); }
	if (codepoint >= 'a' && codepoint <= 'z') { return 0x1D68A + (codepoint - 'a'); }
	if (codepoint >= '0' && codepoint <= '9') { return 0x1D7F6 + (codepoint - '0'); }
	return 0;
}
PEXPI u32 GetRegularCodepointForMonospace(u32 monospaceCodepoint)
{
	if (monospaceCodepoint >= 0x1D670 && monospaceCodepoint <= 0x1D689) { return CharToU32('A') + (monospaceCodepoint - 0x1D670); }
	if (monospaceCodepoint >= 0x1D68A && monospaceCodepoint <= 0x1D6A3) { return CharToU32('a') + (monospaceCodepoint - 0x1D68A); }
	if (monospaceCodepoint >= 0x1D7F6 && monospaceCodepoint <= 0x1D7FF) { return CharToU32('0') + (monospaceCodepoint - 0x1D7F6); }
	return 0;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _BASE_UNICODE_H

#if defined(_STRUCT_STRING_H) && defined(_BASE_UNICODE_H)
#include "cross/cross_string_and_unicode.h"
#endif

#if defined(_MEM_ARENA_H) && defined(_STRUCT_STRING_H) && defined(_BASE_UNICODE_H)
#include "cross/cross_mem_arena_string_and_unicode.h"
#endif
