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

// There are 2,470 combining codepoints: https://codepoints.net/search?lb=CM

#define UTF8_MAX_CODEPOINT 0x10FFFFUL
#define UCS2_MAX_CHAR_SIZE 2 //words
#define UTF8_MAX_CHAR_SIZE 4 //bytes

#define NUM_LETTERS_IN_ENGLISH_ALPHABET 26

//https://codepoints.net/basic_latin
#define UNICODE_PRINTABLE_ASCII_START  0x0020
#define UNICODE_PRINTABLE_ASCII_COUNT  126 //through U+007E
#define UNICODE_PRINTABLE_ASCII_END    (UNICODE_PRINTABLE_ASCII_START + UNICODE_PRINTABLE_ASCII_COUNT)

//https://codepoints.net/latin-1_supplement
#define UNICODE_LATIN1_SUPPLEMENT_ACCENT_START 0x00C0
#define UNICODE_LATIN1_SUPPLEMENT_ACCENT_COUNT 64 //through U+00FF
#define UNICODE_LATIN1_SUPPLEMENT_ACCENT_END   (UNICODE_LATIN1_SUPPLEMENT_ACCENT_START + UNICODE_LATIN1_SUPPLEMENT_ACCENT_COUNT)

//https://codepoints.net/latin_extended-a
#define UNICODE_LATIN_EXT_A_START 0x0100
#define UNICODE_LATIN_EXT_A_COUNT 128 //through U+017F
#define UNICODE_LATIN_EXT_A_END   (UNICODE_LATIN_EXT_A_START + UNICODE_LATIN_EXT_A_COUNT)

//https://codepoints.net/cyrillic
#define UNICODE_CYRILLIC_START 0x0400
#define UNICODE_CYRILLIC_COUNT 256 //through U+04FF
#define UNICODE_CYRILLIC_END   (UNICODE_CYRILLIC_START + UNICODE_CYRILLIC_COUNT)

//https://codepoints.net/hiragana
#define UNICODE_HIRAGANA_START 0x3041
#define UNICODE_HIRAGANA_COUNT 86 //through U+3096
#define UNICODE_HIRAGANA_END   (UNICODE_HIRAGANA_START + UNICODE_HIRAGANA_COUNT)

//https://codepoints.net/katakana
#define UNICODE_KATAKANA_START 0x30A0
#define UNICODE_KATAKANA_COUNT 92 //through U+30FB
#define UNICODE_KATAKANA_END   (UNICODE_KATAKANA_START + UNICODE_KATAKANA_COUNT)

#define UNICODE_CJK_START 0x4E00
#define UNICODE_CJK_COUNT 20902 //aka through U+9FA5
#define UNICODE_CJK_END   (UNICODE_CJK_START + UNICODE_CJK_COUNT)

#define UNICODE_BASIC_MULTILINGUAL_PLANE_START 0x0000
#define UNICODE_BASIC_MULTILINGUAL_PLANE_COUNT 65535 //aka through U+FFFF
#define UNICODE_BASIC_MULTILINGUAL_PLANE_END   (UNICODE_BASIC_MULTILINGUAL_PLANE_START + UNICODE_BASIC_MULTILINGUAL_PLANE_COUNT)

// +==============================+
// |  Specific Codepoint Defines  |
// +==============================+
#define UNICODE_TAB_CODEPOINT             0x0009 //aka '\t' or "Horizontal Tab" or "Character Tabulation"
#define UNICODE_LINE_FEED_CODEPOINT       0x000A //aka '\n' or LF
#define UNICODE_CARRIAGE_RETURN_CODEPOINT 0x000D //aka '\r' or CR
#define UNICODE_SPACE_CODEPOINT           0x0020

// Uses Fitzpatrick scale with Type 1+2 being a single option - https://en.wikipedia.org/wiki/Fitzpatrick_scale
// NOTE: Some emoji support multiple skintone modifiers (like hands shaking emoji)
#define UNICODE_SKINTONE_START 0x1F3FB
#define UNICODE_SKINTONE_COUNT 5 //aka through U+1F3FF
#define UNICODE_SKINTONE_END   (UNICODE_SKINTONE_START + UNICODE_SKINTONE_COUNT)

#define UNICODE_UNKNOWN_CHAR_CODEPOINT 0xFFFD //Technically this is called "Replacement Character"
#define UNICODE_UNKNOWN_CHAR_STR       "\xEF\xBF\xDD" //Technically this is called "Replacement Character"

#define UNICODE_ELLIPSIS_CODEPOINT 0x2026 //Technically called "Horizontal Ellipsis"
#define UNICODE_ELLIPSIS_STR       "\xE2\x80\xA6" //UTF-8 encoding

#define UNICODE_RIGHT_ARROW_CODEPOINT 0x203A //Technically called "Single Right-Pointing Angle Quotation Mark"
#define UNICODE_RIGHT_ARROW_STR       "\xE2\x80\xBA" //UTF-8 encoding

//NOTE: This character is not available in most Windows fonts as far as I can tell
#define UNICODE_CHECK_MARK_CODEPOINT 0x2713
#define UNICODE_CHECK_MARK_STR       "\xE2\x9C\x93" //UTF-8 encoding

//These are all non-breaking characters which we check in IsCodepointNonBreaking
#define UNICODE_NON_BREAKING_SPACE_CODEPOINT            0x00A0
#define UNICODE_NON_BREAKING_SPACE_STR                  "\xC2\xA0" //UTF-8 encoding
#define UNICODE_NO_BREAK_HERE_CODEPOINT                 0x0083
#define UNICODE_NO_BREAK_HERE_STR                       "\xC2\x83" //UTF-8 encoding
#define UNICODE_NON_BREAKING_HYPHEN_CODEPOINT           0x2011
#define UNICODE_NON_BREAKING_HYPHEN_STR                "\xE2\x80\x91" //UTF-8 encoding
#define UNICODE_NARROW_NON_BREAKING_SPACE_CODEPOINT     0x202F
#define UNICODE_NARROW_NON_BREAKING_SPACE_STR           "\xE2\x80\xAF" //UTF-8 encoding
#define UNICODE_WORD_JOINER_CODEPOINT                   0x2060
#define UNICODE_WORD_JOINER_STR                         "\xE2\x81\xA0" //UTF-8 encoding
#define UNICODE_INVISIBLE_SEPARATOR_CODEPOINT           0x2063
#define UNICODE_INVISIBLE_SEPARATOR_STR                 "\xE2\x81\xA3" //UTF-8 encoding
#define UNICODE_ZERO_WIDTH_NON_BREAKING_SPACE_CODEPOINT 0xFEFF
#define UNICODE_ZERO_WIDTH_NON_BREAKING_SPACE_STR       "\xEF\xBB\xBF" //UTF-8 encoding

#define UNICODE_ZERO_WIDTH_SPACE_CODEPOINT 0x200B
#define UNICODE_ZERO_WIDTH_SPACE_STR       "\xE2\x80\x8B" //UTF-8 encoding

// Can be used to prevent ligature formation between adjacent characters that would normally join
#define UNICODE_ZERO_WIDTH_NONJOINER_CODEPOINT 0x200C
#define UNICODE_ZERO_WIDTH_NONJOINER_STR       "\xE2\x80\x8C" //UTF-8 encoding

// Many emoji are combined using this character: https://emojipedia.org/emoji-zwj-sequence and https://www.unicode.org/emoji/charts/emoji-zwj-sequences.html
#define UNICODE_ZERO_WIDTH_JOINER_CODEPOINT 0x200D
#define UNICODE_ZERO_WIDTH_JOINER_STR       "\xE2\x80\x8D" //UTF-8 encoding

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
	PIG_CORE_INLINE bool IsCodepointNonBreaking(u32 codepoint);
	PIG_CORE_INLINE bool IsCodepointWhitespace(u32 codepoint, bool includeNewLines);
	PIG_CORE_INLINE bool IsCodepointZeroWidth(u32 codepoint);
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
	PIG_CORE_INLINE bool IsWordBoundary(u32 prevCodepoint, u32 nextCodepoint);
	uxx FindWordBoundary(uxx strLength, const char* strPntr, uxx startIndex, bool forward);
	#if DEBUG_BUILD
	const char* DebugGetCodepointName(u32 codepoint);
	#endif
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

PEXPI bool IsCodepointNonBreaking(u32 codepoint)
{
	switch (codepoint)
	{
		//TODO: This may not be a complete list, it's just some of the more common codepoints I know about
		case UNICODE_NON_BREAKING_SPACE_CODEPOINT: return true;
		case UNICODE_NO_BREAK_HERE_CODEPOINT: return true;
		case UNICODE_NON_BREAKING_HYPHEN_CODEPOINT: return true;
		case UNICODE_NARROW_NON_BREAKING_SPACE_CODEPOINT: return true;
		case UNICODE_WORD_JOINER_CODEPOINT: return true;
		case UNICODE_INVISIBLE_SEPARATOR_CODEPOINT: return true;
		case UNICODE_ZERO_WIDTH_NON_BREAKING_SPACE_CODEPOINT: return true;
		default: return false;
	}
}
PEXPI bool IsCodepointWhitespace(u32 codepoint, bool includeNewLines)
{
	switch (codepoint)
	{
		//TODO: This may not be a complete list, it's just some of the more common codepoints I know about
		case UNICODE_TAB_CODEPOINT: return true;
		case UNICODE_LINE_FEED_CODEPOINT: return includeNewLines;
		case UNICODE_CARRIAGE_RETURN_CODEPOINT: return includeNewLines;
		case UNICODE_SPACE_CODEPOINT: return true;
		case UNICODE_NON_BREAKING_SPACE_CODEPOINT: return true;
		case UNICODE_NARROW_NON_BREAKING_SPACE_CODEPOINT: return true;
		case UNICODE_ZERO_WIDTH_SPACE_CODEPOINT: return true;
		case UNICODE_ZERO_WIDTH_NON_BREAKING_SPACE_CODEPOINT: return true;
		default: return false;
	}
}
PEXPI bool IsCodepointZeroWidth(u32 codepoint)
{
	switch (codepoint)
	{
		//TODO: This may not be a complete list, it's just some of the more common codepoints I know about
		case UNICODE_ZERO_WIDTH_SPACE_CODEPOINT: return true;
		case UNICODE_ZERO_WIDTH_NON_BREAKING_SPACE_CODEPOINT: return true;
		case UNICODE_ZERO_WIDTH_JOINER_CODEPOINT: return true;
		case UNICODE_ZERO_WIDTH_NONJOINER_CODEPOINT: return true;
		default: return false;
	}
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
	for (u8 bIndex = 1; bIndex <= UTF8_MAX_CHAR_SIZE && bIndex <= numBytesBeforePntr; bIndex++)
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
		DebugAssert(calcSize == codepointSize);
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

// +--------------------------------------------------------------+
// |                    Word and Subword Logic                    |
// +--------------------------------------------------------------+
PEXPI bool IsWordBoundary(u32 prevCodepoint, u32 nextCodepoint)
{
	bool isNextCharWord = IsCharAlphaNumeric(nextCodepoint);
	bool isPrevCharWord = IsCharAlphaNumeric(prevCodepoint);
	bool isNextCodepointWhitespace = IsCodepointWhitespace(nextCodepoint, true);
	bool isPrevCodepointWhitespace = IsCodepointWhitespace(prevCodepoint, true);
	bool isNextCodepointNonBreaking = IsCodepointNonBreaking(nextCodepoint);
	bool isPrevCodepointNonBreaking = IsCodepointNonBreaking(prevCodepoint);
	return ((isNextCharWord != isPrevCharWord || isPrevCodepointWhitespace != isNextCodepointWhitespace) && !isPrevCodepointNonBreaking && !isNextCodepointNonBreaking);
}

PEXP uxx FindWordBoundary(uxx strLength, const char* strPntr, uxx startIndex, bool forward)
{
	Assert(strPntr != nullptr || strLength == 0);
	if (startIndex == 0 && !forward) { return startIndex; }
	if (startIndex >= strLength && forward) { return strLength; }
	
	for (uxx bIndex = startIndex;
		(forward && bIndex < strLength) || (!forward && bIndex > 0);
		bIndex += (forward ? 1 : -1))
	{
		u32 nextCodepoint = 0;
		u8 nextCodepointSize = GetCodepointForUtf8(strLength - bIndex, &strPntr[bIndex], &nextCodepoint);
		//TODO: What do we do with invalid UTF-8?
		u32 prevCodepoint = 0;
		u8 prevCodepointSize = GetPrevCodepointForUtf8(bIndex, &strPntr[bIndex], &prevCodepoint);
		//TODO: What do we do with invalid UTF-8?
		if (!forward) { SwapVariables(u32, nextCodepoint, prevCodepoint); SwapVariables(u8, nextCodepointSize, prevCodepointSize); }
		
		if (bIndex != startIndex && IsWordBoundary(prevCodepoint, nextCodepoint))
		{
			return bIndex;
		}
		
		if (nextCodepointSize > 1) { bIndex += (nextCodepointSize-1) * (forward ? 1 : -1); }
	}
	
	return (forward ? strLength : 0);
}

#if DEBUG_BUILD
PEXP const char* DebugGetCodepointName(u32 codepoint)
{
	switch (codepoint)
	{
		case '\t': return "tab";
		case '\n': return "LF";
		case '\r': return "CR";
		case ' ': return "space";
		
		case '!': return "!";
		case '@': return "@";
		case '#': return "#";
		case '$': return "$";
		case '%': return "%";
		case '^': return "^";
		case '&': return "&";
		case '*': return "*";
		case '(': return "(";
		case ')': return ")";
		case '-': return "-";
		case '+': return "+";
		case '_': return "_";
		case '=': return "=";
		case '[': return "[";
		case ']': return "]";
		case '{': return "{";
		case '}': return "}";
		case '|': return "|";
		case ';': return ";";
		case ':': return ":";
		case '\'': return "\'";
		case '\"': return "\"";
		case '<': return "<";
		case '>': return ">";
		case ',': return ",";
		case '.': return ".";
		case '/': return "/";
		case '?': return "?";
		
		case '0': return "0";
		case '1': return "1";
		case '2': return "2";
		case '3': return "3";
		case '4': return "4";
		case '5': return "5";
		case '6': return "6";
		case '7': return "7";
		case '8': return "8";
		case '9': return "9";
		
		case 'A': return "A";
		case 'B': return "B";
		case 'C': return "C";
		case 'D': return "D";
		case 'E': return "E";
		case 'F': return "F";
		case 'G': return "G";
		case 'H': return "H";
		case 'I': return "I";
		case 'J': return "J";
		case 'K': return "K";
		case 'L': return "L";
		case 'M': return "M";
		case 'N': return "N";
		case 'O': return "O";
		case 'P': return "P";
		case 'Q': return "Q";
		case 'R': return "R";
		case 'S': return "S";
		case 'T': return "T";
		case 'U': return "U";
		case 'V': return "V";
		case 'W': return "W";
		case 'X': return "X";
		case 'Y': return "Y";
		case 'Z': return "Z";
		
		case 'a': return "a";
		case 'b': return "b";
		case 'c': return "c";
		case 'd': return "d";
		case 'e': return "e";
		case 'f': return "f";
		case 'g': return "g";
		case 'h': return "h";
		case 'i': return "i";
		case 'j': return "j";
		case 'k': return "k";
		case 'l': return "l";
		case 'm': return "m";
		case 'n': return "n";
		case 'o': return "o";
		case 'p': return "p";
		case 'q': return "q";
		case 'r': return "r";
		case 's': return "s";
		case 't': return "t";
		case 'u': return "u";
		case 'v': return "v";
		case 'w': return "w";
		case 'x': return "x";
		case 'y': return "y";
		case 'z': return "z";
		
		case UNICODE_ZERO_WIDTH_SPACE_CODEPOINT: return "zero-width space";
		case UNICODE_NON_BREAKING_SPACE_CODEPOINT: return "non-breaking space";
		case UNICODE_NON_BREAKING_HYPHEN_CODEPOINT: return "non-breaking hyphen";
		case UNICODE_UNKNOWN_CHAR_CODEPOINT: return "replacement";
		case UNICODE_ELLIPSIS_CODEPOINT: return "ellipsis";
		case UNICODE_RIGHT_ARROW_CODEPOINT: return "right arrow";
		case UNICODE_CHECK_MARK_CODEPOINT: return "check mark";
		
		case 0x3042: return "hira-a";
		case 0x3044: return "hira-i";
		case 0x3046: return "hira-u";
		case 0x3048: return "hira-e";
		case 0x304A: return "hira-o";
		
		case 0x304B: return "hira-ka";
		case 0x304D: return "hira-ki";
		case 0x304F: return "hira-ku";
		case 0x3051: return "hira-ke";
		case 0x3053: return "hira-ko";
		
		case 0x304C: return "hira-ga";
		case 0x304E: return "hira-gi";
		case 0x3050: return "hira-gu";
		case 0x3052: return "hira-ge";
		case 0x3054: return "hira-go";
		
		case 0x3055: return "hira-sa";
		case 0x3057: return "hira-shi";
		case 0x3059: return "hira-su";
		case 0x305B: return "hira-se";
		case 0x305D: return "hira-so";
		
		case 0x3056: return "hira-za";
		case 0x3058: return "hira-ji";
		case 0x305A: return "hira-zu";
		case 0x305C: return "hira-ze";
		case 0x305E: return "hira-zo";
		
		case 0x305F: return "hira-ta";
		case 0x3061: return "hira-chi";
		case 0x3064: return "hira-tsu";
		case 0x3066: return "hira-te";
		case 0x3068: return "hira-to";
		
		case 0x3060: return "hira-da";
		case 0x3062: return "hira-dji";
		case 0x3065: return "hira-dzu";
		case 0x3067: return "hira-de";
		case 0x3069: return "hira-do";
		
		case 0x306A: return "hira-na";
		case 0x306B: return "hira-ni";
		case 0x306C: return "hira-nu";
		case 0x306D: return "hira-ne";
		case 0x306E: return "hira-no";
		
		case 0x306F: return "hira-ha";
		case 0x3072: return "hira-hi";
		case 0x3075: return "hira-hu";
		case 0x3078: return "hira-he";
		case 0x307B: return "hira-ho";
		
		case 0x3070: return "hira-ba";
		case 0x3073: return "hira-bi";
		case 0x3076: return "hira-bu";
		case 0x3079: return "hira-be";
		case 0x307C: return "hira-bo";
		
		case 0x3071: return "hira-pa";
		case 0x3074: return "hira-pi";
		case 0x3077: return "hira-pu";
		case 0x307A: return "hira-pe";
		case 0x307D: return "hira-po";
		
		case 0x307E: return "hira-ma";
		case 0x307F: return "hira-mi";
		case 0x3080: return "hira-mu";
		case 0x3081: return "hira-me";
		case 0x3082: return "hira-mo";
		
		case 0x3084: return "hira-ya";
		case 0x3086: return "hira-yu";
		case 0x3088: return "hira-yo";
		
		case 0x3089: return "hira-ra";
		case 0x308A: return "hira-ri";
		case 0x308B: return "hira-ru";
		case 0x308C: return "hira-re";
		case 0x308D: return "hira-ro";
		
		case 0x308F: return "hira-wa";
		case 0x3090: return "hira-wi";
		case 0x3091: return "hira-we";
		case 0x3092: return "hira-wo";
		
		case 0x3093: return "hira-n";
		
		default: return "(?)";
	}
}

#endif

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _BASE_UNICODE_H

#if defined(_STRUCT_STRING_H) && defined(_BASE_UNICODE_H)
#include "cross/cross_string_and_unicode.h"
#endif

#if defined(_MEM_ARENA_H) && defined(_STRUCT_STRING_H) && defined(_BASE_UNICODE_H)
#include "cross/cross_mem_arena_string_and_unicode.h"
#endif
