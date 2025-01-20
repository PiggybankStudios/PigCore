/*
File:   cross_mem_arena_string_and_unicode.h
Author: Taylor Robbins
Date:   01\17\2025
*/

#ifndef _CROSS_MEM_ARENA_STRING_AND_UNICODE_H
#define _CROSS_MEM_ARENA_STRING_AND_UNICODE_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	Str8 ConvertUcs2StrToUtf8(Arena* arena, Str16 usc2Str, bool addNullTerm);
	Str16 ConvertUtf8StrToUcs2(Arena* arena, Str8 utf8Str, bool addNullTerm);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define ToStr16From8(arena, usc2Str, addNullTerm) ConvertUcs2StrToUtf8((arena), (usc2Str), (addNullTerm))
#define ToStr8From16(arena, utf8Str, addNullTerm) ConvertUtf8StrToUcs2((arena), (utf8Str), (addNullTerm))

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

//TODO: This function isn't fully implemented, it just casts each 16-bit character to u32
PEXP Str8 ConvertUcs2StrToUtf8(Arena* arena, Str16 usc2Str, bool addNullTerm)
{
	Assert(usc2Str.pntr != nullptr || usc2Str.length == 0);
	u8 encodeBuffer[UTF8_MAX_CHAR_SIZE];
	Str8 result = Str8_Empty;
	for (u8 pass = 0; pass < 2; pass++)
	{
		uxx byteIndex = 0;
		
		for (uxx cIndex = 0; cIndex < usc2Str.length; cIndex++)
		{
			char16_t wideChar = usc2Str.chars[cIndex];
			//TODO: Do better than casting wideChar to u32! We need to implement GetCodepointForUcs2
			u8 encodeSize = GetUtf8BytesForCode((u32)wideChar, &encodeBuffer[0], false);
			if (encodeSize == 0) { return result; }
			if (result.chars != nullptr)
			{
				Assert(result.length >= byteIndex + encodeSize);
				MyMemCopy(&result.chars[byteIndex], &encodeBuffer[0], encodeSize);
			}
			byteIndex += encodeSize;
		}
		
		if (pass == 0)
		{
			result.length = byteIndex;
			if (arena == nullptr) { return result; }
			result.chars = AllocArray(char, arena, result.length + (addNullTerm ? 1 : 0));
			NotNull(result.chars);
		}
		else
		{
			Assert(byteIndex == result.length);
			if (addNullTerm) { result.chars[result.length] = '\0'; }
		}
	}
	return result;
}

PEXP Str16 ConvertUtf8StrToUcs2(Arena* arena, Str8 utf8Str, bool addNullTerm)
{
	NotNullStr(utf8Str);
	u16 encodeBuffer[UCS2_MAX_CHAR_SIZE];
	Str16 result = Str16_Empty;
	for (u8 pass = 0; pass < 2; pass++)
	{
		uxx wordIndex = 0;
		
		for (uxx cIndex = 0; cIndex < utf8Str.length; cIndex++)
		{
			u32 codepoint;
			u8 codepointSize = GetCodepointForUtf8Str(utf8Str, cIndex, &codepoint);
			if (codepointSize == 0) { return result; }
			u8 numWords = GetUcs2WordsForCode(codepoint, &encodeBuffer[0], false);
			if (numWords == 0) { return result; }
			if (result.chars != nullptr)
			{
				Assert(result.length >= wordIndex + numWords);
				MyMemCopy(&result.chars[wordIndex], &encodeBuffer[0], numWords * sizeof(u16));
			}
			wordIndex += numWords;
		}
		
		if (pass == 0)
		{
			result.length = wordIndex;
			if (arena == nullptr) { return result; }
			result.chars = AllocArray(char16_t, arena, result.length + (addNullTerm ? 1 : 0));
			NotNull(result.chars);
		}
		else
		{
			Assert(wordIndex == result.length);
			if (addNullTerm) { result.chars[result.length] = 0; }
		}
	}
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_MEM_ARENA_STRING_AND_UNICODE_H
