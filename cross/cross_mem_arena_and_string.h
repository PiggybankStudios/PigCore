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
	Str8 JoinStringsInArenaWithChar(Arena* arena, Str8 left, char sepChar, Str8 right, bool addNullTerm);
	Str8 StrReplace(Arena* arena, Str8 str, Str8 target, Str8 replacement, bool addNullTerm);
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

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_MEM_ARENA_AND_STRING_H
