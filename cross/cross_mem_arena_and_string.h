/*
File:   cross_mem_arena_and_string.h
Author: Taylor Robbins
Date:   01\06\2025
Description:
	** Extensions to the Arena and String APIs that use both together
*/

#ifndef _CROSS_MEM_ARENA_AND_STRING_H
#define _CROSS_MEM_ARENA_AND_STRING_H

//NOTE: Intentionally no includes here!

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE char* AllocAndCopyChars(Arena* arena, uxx numChars, const char* charsToCopy, bool addNullTerm);
	PIG_CORE_INLINE char* AllocAndCopyCharsNt(Arena* arena, const char* nullTermStr, bool addNullTerm);
	PIG_CORE_INLINE Str8 AllocStrAndCopy(Arena* arena, uxx numChars, const char* charsToCopy, bool addNullTerm);
	PIG_CORE_INLINE Str8 AllocStrAndCopyNt(Arena* arena, const char* nullTermStr, bool addNullTerm);
	PIG_CORE_INLINE void FreeStr8(Arena* arena, Str8* stringPntr);
	PIG_CORE_INLINE void FreeStr8WithNt(Arena* arena, Str8* stringPntr);
	Str8 JoinStringsInArena(Arena* arena, Str8 left, Str8 right, bool addNullTerm);
	Str8 JoinStringsInArena3(Arena* arena, Str8 left, Str8 middle, Str8 right, bool addNullTerm);
	Str8 JoinStringsInArenaWithChar(Arena* arena, Str8 left, char sepChar, Str8 right, bool addNullTerm);
	Str8 StrReplace(Arena* arena, Str8 str, Str8 target, Str8 replacement, bool addNullTerm);
	Str8 EscapeStringEx(Arena* arena, Str8 rawString, u8 escapeSequences, bool addNullTerm);
	PIG_CORE_INLINE Str8 EscapeString(Arena* arena, Str8 rawString);
	Str8 UnescapeStringEx(Arena* arena, Str8 escapedString, u8 escapeSequences, bool addNullTerm);
	PIG_CORE_INLINE Str8 UnescapeString(Arena* arena, Str8 escapedString);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
//NOTE: These all implicitly assume addNullTerm=false
#define AllocStr8(arenaPntr, string) AllocStrAndCopy((arenaPntr), (string).length, (string).chars, false)
#define AllocStr8Nt(arenaPntr, nullTermStr) AllocStrAndCopyNt((arenaPntr), (nullTermStr), false)
#define AllocStr8Length(arenaPntr, length, charPntr) AllocStrAndCopy((arenaPntr), (length), (charPntr), false)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

//NOTE: When addNullTerm is true we will allocate one more byte than numChars to hold a null-terminating character
//      As such, when Freeing these strings, we will pass an innacurate size value to the Arena.
//      If the string is being allocated from an arena that doesn't track sizes, then you might
//      want to forego allocating the nullterm character so sizes are accurate during Free.
PEXPI char* AllocAndCopyChars(Arena* arena, uxx numChars, const char* charsToCopy, bool addNullTerm)
{
	DebugNotNull(arena);
	Assert(charsToCopy != nullptr || numChars == 0);
	if (numChars == 0 && !addNullTerm) { return nullptr; }
	char* result = (char*)AllocMem(arena, numChars + (addNullTerm ? 1 : 0));
	if (result == nullptr) { return nullptr; }
	if (numChars > 0)
	{
		MyMemCopy(result, charsToCopy, numChars);
	}
	if (addNullTerm)
	{
		result[numChars] = '\0';
	}
	return result;
}
PEXPI char* AllocAndCopyCharsNt(Arena* arena, const char* nullTermStr, bool addNullTerm)
{
	DebugNotNull(arena);
	uxx numChars = (nullTermStr != nullptr) ? MyStrLength64(nullTermStr) : 0;
	return AllocAndCopyChars(arena, numChars, nullTermStr, addNullTerm);
}
PEXPI Str8 AllocStrAndCopy(Arena* arena, uxx numChars, const char* charsToCopy, bool addNullTerm)
{
	DebugNotNull(arena);
	char* allocatedChars = AllocAndCopyChars(arena, numChars, charsToCopy, addNullTerm);
	return (allocatedChars != nullptr) ? NewStr8(numChars, allocatedChars) : Str8_Empty;
}
PEXPI Str8 AllocStrAndCopyNt(Arena* arena, const char* nullTermStr, bool addNullTerm)
{
	DebugNotNull(arena);
	uxx numChars = (nullTermStr != nullptr) ? MyStrLength64(nullTermStr) : 0;
	char* allocatedChars = AllocAndCopyChars(arena, numChars, nullTermStr, addNullTerm);
	return (allocatedChars != nullptr) ? NewStr8(numChars, allocatedChars) : Str8_Empty;
}

PEXPI void FreeStr8(Arena* arena, Str8* stringPntr)
{
	NotNull(stringPntr);
	if (stringPntr->length > 0)
	{
		FreeMem(arena, stringPntr->chars, stringPntr->length);
	}
	ClearPointer(stringPntr);
}
PEXPI void FreeStr8WithNt(Arena* arena, Str8* stringPntr)
{
	NotNull(stringPntr);
	if (stringPntr->length > 0 || stringPntr->chars != nullptr)
	{
		FreeMem(arena, stringPntr->chars, stringPntr->length+1);
	}
	ClearPointer(stringPntr);
}

PEXP Str8 JoinStringsInArena(Arena* arena, Str8 left, Str8 right, bool addNullTerm)
{
	Str8 result;
	result.length = left.length + right.length;
	if (result.length == 0 && !addNullTerm) { return Str8_Empty; }
	result.chars = (char*)AllocMem(arena, result.length + (addNullTerm ? 1 : 0));
	if (result.chars == nullptr) { return Str8_Empty; }
	if (left.length  > 0) { MyMemCopy(result.chars + 0,           left.chars,  left.length);  }
	if (right.length > 0) { MyMemCopy(result.chars + left.length, right.chars, right.length); }
	if (addNullTerm) { result.chars[result.length] = '\0'; }
	return result;
}
PEXP Str8 JoinStringsInArena3(Arena* arena, Str8 left, Str8 middle, Str8 right, bool addNullTerm)
{
	Str8 result;
	result.length = left.length + middle.length + right.length;
	if (result.length == 0 && !addNullTerm) { return Str8_Empty; }
	result.chars = (char*)AllocMem(arena, result.length + (addNullTerm ? 1 : 0));
	if (result.chars == nullptr) { return Str8_Empty; }
	if (left.length   > 0) { MyMemCopy(result.chars + 0,                           left.chars,    left.length);   }
	if (middle.length > 0) { MyMemCopy(result.chars + left.length,                 middle.chars,  middle.length); }
	if (right.length  > 0) { MyMemCopy(result.chars + left.length + middle.length, right.chars,   right.length);  }
	if (addNullTerm) { result.chars[result.length] = '\0'; }
	return result;
}
PEXP Str8 JoinStringsInArenaWithChar(Arena* arena, Str8 left, char sepChar, Str8 right, bool addNullTerm)
{
	Str8 result;
	result.length = left.length + 1 + right.length;
	result.chars = (char*)AllocMem(arena, result.length + (addNullTerm ? 1 : 0));
	if (result.chars == nullptr) { return Str8_Empty; }
	if (left.length  > 0) { MyMemCopy(result.chars + 0,               left.chars,  left.length);  }
	if (right.length > 0) { MyMemCopy(result.chars + left.length + 1, right.chars, right.length); }
	result.chars[left.length] = sepChar;
	if (addNullTerm) { result.chars[result.length] = '\0'; }
	return result;
}
//TODO: Add JoinStringsInArena3 that takes left, middle, and right

PEXP Str8 StrReplace(Arena* arena, Str8 str, Str8 target, Str8 replacement, bool addNullTerm)
{
	NotNullStr(str);
	NotNullStr(target);
	NotNullStr(replacement);
	if (target.length == 0) { return AllocStr8(arena, str); } //nothing to target means nothing to replace
	Str8 result = Str8_Empty;
	
	for (u8 pass = 0; pass < 2; pass++)
	{
		uxx numBytesNeeded = 0;
		
		for (uxx bIndex = 0; bIndex < str.length; bIndex++)
		{
			if (bIndex + target.length <= str.length)
			{
				if (MyMemEquals(str.chars + bIndex, target.chars, target.length))
				{
					if (result.chars != nullptr)
					{
						Assert(numBytesNeeded + replacement.length <= result.length);
						MyMemCopy(&result.chars[numBytesNeeded], replacement.chars, replacement.length);
					}
					numBytesNeeded += replacement.length;
					bIndex += target.length-1;
				}
				else
				{
					if (result.chars != nullptr)
					{
						Assert(numBytesNeeded+1 <= result.length);
						result.chars[numBytesNeeded] = str.chars[bIndex];
					}
					numBytesNeeded += 1;
				}
			}
			else
			{
				if (result.chars != nullptr)
				{
					Assert(numBytesNeeded+1 <= result.length);
					result.chars[numBytesNeeded] = str.chars[bIndex];
				}
				numBytesNeeded += 1;
			}
		}
		
		if (pass == 0)
		{
			result.length = numBytesNeeded;
			if (arena == nullptr) { return result; }
			result.chars = (char*)AllocMem(arena, result.length + (addNullTerm ? 1 : 0));
			NotNull(result.chars);
		}
		else
		{
			Assert(numBytesNeeded == result.length);
			if (addNullTerm) { result.chars[result.length] = '\0'; }
		}
	}
	
	return result;
}

PEXP Str8 EscapeStringEx(Arena* arena, Str8 rawString, u8 escapeSequences, bool addNullTerm)
{
	NotNullStr(rawString);
	Str8 result = Str8_Empty;
	for (u8 pass = 0; pass < 2; pass++)
	{
		uxx outIndex = 0;
		for (uxx bIndex = 0; bIndex < rawString.length; bIndex++)
		{
			char character = rawString.chars[bIndex];
			if (IsFlagSet(escapeSequences, EscapeSequence_Backslash) && character == '\\')
			{
				if (result.chars != nullptr)
				{
					Assert(outIndex+2 <= result.length);
					result.chars[outIndex+0] = '\\';
					result.chars[outIndex+1] = '\\';
				}
				outIndex += 2;
			}
			else if (IsFlagSet(escapeSequences, EscapeSequence_Quote) && character == '\"')
			{
				if (result.chars != nullptr)
				{
					Assert(outIndex+2 <= result.length);
					result.chars[outIndex+0] = '\\';
					result.chars[outIndex+1] = '\"';
				}
				outIndex += 2;
			}
			else if (IsFlagSet(escapeSequences, EscapeSequence_Apostrophe) && character == '\'')
			{
				if (result.chars != nullptr)
				{
					Assert(outIndex+2 <= result.length);
					result.chars[outIndex+0] = '\\';
					result.chars[outIndex+1] = '\'';
				}
				outIndex += 2;
			}
			else if (IsFlagSet(escapeSequences, EscapeSequence_NewLine) && character == '\n')
			{
				if (result.chars != nullptr)
				{
					Assert(outIndex+2 <= result.length);
					result.chars[outIndex+0] = '\\';
					result.chars[outIndex+1] = 'n';
				}
				outIndex += 2;
			}
			else if (IsFlagSet(escapeSequences, EscapeSequence_CarriageReturn) && character == '\r')
			{
				if (result.chars != nullptr)
				{
					Assert(outIndex+2 <= result.length);
					result.chars[outIndex+0] = '\\';
					result.chars[outIndex+1] = 'r';
				}
				outIndex += 2;
			}
			else if (IsFlagSet(escapeSequences, EscapeSequence_Tab) && character == '\t')
			{
				if (result.chars != nullptr)
				{
					Assert(outIndex+2 <= result.length);
					result.chars[outIndex+0] = '\\';
					result.chars[outIndex+1] = 't';
				}
				outIndex += 2;
			}
			else if (IsFlagSet(escapeSequences, EscapeSequence_Backspace) && character == '\b')
			{
				if (result.chars != nullptr)
				{
					Assert(outIndex+2 <= result.length);
					result.chars[outIndex+0] = '\\';
					result.chars[outIndex+1] = 'b';
				}
				outIndex += 2;
			}
			else if (IsFlagSet(escapeSequences, EscapeSequence_Bell) && character == '\a')
			{
				if (result.chars != nullptr)
				{
					Assert(outIndex+2 <= result.length);
					result.chars[outIndex+0] = '\\';
					result.chars[outIndex+1] = 'a';
				}
				outIndex += 2;
			}
			else
			{
				if (result.chars != nullptr) { Assert(outIndex < result.length); result.chars[outIndex] = character; }
				outIndex++;
			}
		}
		
		if (pass == 0)
		{
			result.length = outIndex;
			if (arena == nullptr || (result.length == 0 && !addNullTerm)) { return result; }
			result.chars = (char*)AllocMem(arena, result.length + (addNullTerm ? 1 : 0));
			NotNull(result.chars);
		}
		else
		{
			Assert(outIndex == result.length);
			if (addNullTerm) { result.chars[result.length] = '\0'; }
		}
	}
	return result;
}
PEXPI Str8 EscapeString(Arena* arena, Str8 rawString)
{
	return EscapeStringEx(arena, rawString, EscapeSequence_All, false);
}

//NOTE: Invalid escape sequences are just left as 2 characters (i.e. "\n" will stay as '\' and 'n' if the EscapeSequence_NewLine flag is not set)
//      This means if you have an invalid escaped string, then the round-trip Unescape->Escape will produce extra backslash characters, which may not be desireable. 
//      In general it's safe to assume round-trips Escape->Unescape are safe but not the other way around
PEXP Str8 UnescapeStringEx(Arena* arena, Str8 escapedString, u8 escapeSequences, bool addNullTerm)
{
	NotNullStr(escapedString);
	Str8 result = Str8_Empty;
	for (u8 pass = 0; pass < 2; pass++)
	{
		uxx outIndex = 0;
		for (uxx bIndex = 0; bIndex < escapedString.length; bIndex++)
		{
			char character = escapedString.chars[bIndex];
			char nextCharacter = (bIndex+1 < escapedString.length) ? escapedString.chars[bIndex+1] : '\0';
			if (character == '\\')
			{
				if (IsFlagSet(escapeSequences, EscapeSequence_Backslash) && nextCharacter == '\\')
				{
					if (result.chars != nullptr)
					{
						Assert(outIndex < result.length);
						result.chars[outIndex] = '\\';
					}
					outIndex++;
					bIndex++;
				}
				else if (IsFlagSet(escapeSequences, EscapeSequence_Quote) && nextCharacter == '\"')
				{
					if (result.chars != nullptr)
					{
						Assert(outIndex < result.length);
						result.chars[outIndex] = '\"';
					}
					outIndex++;
					bIndex++;
				}
				else if (IsFlagSet(escapeSequences, EscapeSequence_Apostrophe) && nextCharacter == '\'')
				{
					if (result.chars != nullptr)
					{
						Assert(outIndex < result.length);
						result.chars[outIndex] = '\'';
					}
					outIndex++;
					bIndex++;
				}
				else if (IsFlagSet(escapeSequences, EscapeSequence_NewLine) && nextCharacter == 'n')
				{
					if (result.chars != nullptr)
					{
						Assert(outIndex < result.length);
						result.chars[outIndex] = '\n';
					}
					outIndex++;
					bIndex++;
				}
				else if (IsFlagSet(escapeSequences, EscapeSequence_CarriageReturn) && nextCharacter == 'r')
				{
					if (result.chars != nullptr)
					{
						Assert(outIndex < result.length);
						result.chars[outIndex] = '\r';
					}
					outIndex++;
					bIndex++;
				}
				else if (IsFlagSet(escapeSequences, EscapeSequence_Tab) && nextCharacter == 't')
				{
					if (result.chars != nullptr)
					{
						Assert(outIndex < result.length);
						result.chars[outIndex] = '\t';
					}
					outIndex++;
					bIndex++;
				}
				else if (IsFlagSet(escapeSequences, EscapeSequence_Backspace) && nextCharacter == 'b')
				{
					if (result.chars != nullptr)
					{
						Assert(outIndex < result.length);
						result.chars[outIndex] = '\b';
					}
					outIndex++;
					bIndex++;
				}
				else if (IsFlagSet(escapeSequences, EscapeSequence_Bell) && nextCharacter == 'a')
				{
					if (result.chars != nullptr)
					{
						Assert(outIndex < result.length);
						result.chars[outIndex] = '\a';
					}
					outIndex++;
					bIndex++;
				}
				else
				{
					if (result.chars != nullptr) { Assert(outIndex < result.length); result.chars[outIndex] = character; }
					outIndex++;
				}
			}
			else
			{
				if (result.chars != nullptr) { Assert(outIndex < result.length); result.chars[outIndex] = character; }
				outIndex++;
			}
		}
		
		if (pass == 0)
		{
			result.length = outIndex;
			if (arena == nullptr || (result.length == 0 && !addNullTerm)) { return result; }
			result.chars = (char*)AllocMem(arena, result.length + (addNullTerm ? 1 : 0));
			NotNull(result.chars);
		}
		else
		{
			Assert(outIndex == result.length);
			if (addNullTerm) { result.chars[result.length] = '\0'; }
		}
	}
	return result;
}
PEXPI Str8 UnescapeString(Arena* arena, Str8 escapedString)
{
	return UnescapeStringEx(arena, escapedString, EscapeSequence_All, false);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_MEM_ARENA_AND_STRING_H
