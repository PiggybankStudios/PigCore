/*
File:   cross_string_and_unicode.h
Author: Taylor Robbins
Date:   01\06\2025
Description:
	** Extensions to the Unicode and Str8 APIs that use both together
*/

#ifndef _CROSS_STRING_AND_UNICODE_H
#define _CROSS_STRING_AND_UNICODE_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE u8 GetCodepointForUtf8Str(Str8 str, uxx index, u32* codepointOut);
	PIG_CORE_INLINE u8 GetPrevCodepointForUtf8Str(Str8 str, uxx index, u32* codepointOut);
	bool DoesStrContainMultibyteUtf8Chars(Str8 str);
	uxx FindNextCharInStrEx(Str8 target, uxx startIndex, Str8 searchCharsStr, bool ignoreCharsInQuotes);
	PIG_CORE_INLINE uxx FindNextCharInStr(Str8 target, uxx startIndex, Str8 searchCharsStr);
	uxx FindNextUnknownCharInStrEx(Str8 target, uxx startIndex, Str8 knownCharsStr, bool ignoreCharsInQuotes);
	PIG_CORE_INLINE uxx FindNextUnknownCharInStr(Str8 target, uxx startIndex, Str8 knownCharsStr);
	PIG_CORE_INLINE uxx FindNextWhitespaceInStrEx(Str8 target, uxx startIndex, bool ignoreCharsInQuotes);
	PIG_CORE_INLINE bool FindNextWhitespaceInStr(Str8 target, uxx startIndex);
	PIG_CORE_INLINE uxx FindWordBoundaryStr(Str8 str, uxx startIndex, bool forward);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI u8 GetCodepointForUtf8Str(Str8 str, uxx index, u32* codepointOut)
{
	Assert(index <= str.length);
	return GetCodepointForUtf8(str.length - index, str.chars + index, codepointOut);
}
PEXPI u8 GetPrevCodepointForUtf8Str(Str8 str, uxx index, u32* codepointOut)
{
	Assert(index <= str.length);
	return GetPrevCodepointForUtf8(index, str.chars + index, codepointOut);
}

PEXP bool DoesStrContainMultibyteUtf8Chars(Str8 str)
{
	for (uxx bIndex = 0; bIndex < str.length; bIndex++)
	{
		if (GetCodepointForUtf8Str(str, bIndex, nullptr) > 0) { return true; }
	}
	return false;
}

//Returns target.length if no matching char is found
PEXP uxx FindNextCharInStrEx(Str8 target, uxx startIndex, Str8 searchCharsStr, bool ignoreCharsInQuotes)
{
	NotNullStr(target);
	Assert(startIndex <= target.length);
	bool inQuotes = false;
	u32 previousCodepoint = 0;
	for (uxx cIndex = startIndex; cIndex < target.length; )
	{
		u32 codepoint = 0;
		u8 codepointSize = GetCodepointForUtf8Str(target, cIndex, &codepoint);
		if (codepointSize == 0) { cIndex++; continue; } //invalid utf-8 encoding in target
		for (uxx sIndex = 0; sIndex < searchCharsStr.length; )
		{
			u32 searchCodepoint = 0;
			u8 searchCodepointSize = GetCodepointForUtf8Str(searchCharsStr, sIndex, &searchCodepoint);
			DebugAssert(searchCodepointSize > 0);
			if (searchCodepointSize == 0) { return target.length; } //invalid utf-8 encoding in searchCharsStr
			if (codepoint == searchCodepoint && !inQuotes)
			{
				return cIndex;
			}
			sIndex += searchCodepointSize;
		}
		if (ignoreCharsInQuotes && codepoint == '"' && !(inQuotes && previousCodepoint == '\\'))
		{
			inQuotes = !inQuotes;
		}
		previousCodepoint = codepoint;
		cIndex += codepointSize;
	}
	return target.length;
}
PEXPI uxx FindNextCharInStr(Str8 target, uxx startIndex, Str8 searchCharsStr) { return FindNextCharInStrEx(target, startIndex, searchCharsStr, false); }

PEXP uxx FindNextUnknownCharInStrEx(Str8 target, uxx startIndex, Str8 knownCharsStr, bool ignoreCharsInQuotes)
{
	NotNullStr(target);
	Assert(startIndex <= target.length);
	bool inQuotes = false;
	u32 previousCodepoint = 0;
	for (uxx cIndex = startIndex; cIndex < target.length; )
	{
		u32 codepoint = 0;
		u8 codepointSize = GetCodepointForUtf8Str(target, cIndex, &codepoint);
		if (codepointSize == 0) { cIndex++; continue; } //invalid utf-8 encoding in target
		bool isKnownChar = false;
		for (uxx sIndex = 0; sIndex < knownCharsStr.length; )
		{
			u32 searchCodepoint = 0;
			u8 searchCodepointSize = GetCodepointForUtf8Str(knownCharsStr, sIndex, &searchCodepoint);
			DebugAssert(searchCodepointSize > 0);
			if (searchCodepointSize == 0) { return target.length; } //invalid utf-8 encoding in knownCharsStr
			if (codepoint == searchCodepoint && !inQuotes)
			{
				isKnownChar = true;
				break;
			}
			sIndex += searchCodepointSize;
		}
		if (!isKnownChar)
		{
			return cIndex;
		}
		if (ignoreCharsInQuotes && codepoint == '"' && !(inQuotes && previousCodepoint == '\\'))
		{
			inQuotes = !inQuotes;
		}
		previousCodepoint = codepoint;
		cIndex += codepointSize;
	}
	return target.length;
}
PEXPI uxx FindNextUnknownCharInStr(Str8 target, uxx startIndex, Str8 knownCharsStr) { return FindNextUnknownCharInStrEx(target, startIndex, knownCharsStr, false); }

PEXPI uxx FindNextWhitespaceInStrEx(Str8 target, uxx startIndex, bool ignoreCharsInQuotes)
{
	return FindNextCharInStrEx(target, startIndex, StrLit(WHITESPACE_CHARS), ignoreCharsInQuotes);
}
PEXPI bool FindNextWhitespaceInStr(Str8 target, uxx startIndex) { return FindNextWhitespaceInStrEx(target, startIndex, false); }

PEXPI uxx FindWordBoundaryStr(Str8 str, uxx startIndex, bool forward)
{
	return FindWordBoundary(str.length, str.chars, startIndex, forward);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_STRING_AND_UNICODE_H
