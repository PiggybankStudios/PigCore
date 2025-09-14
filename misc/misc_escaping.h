/*
File:   misc_escaping.h
Author: Taylor Robbins
Date:   09\14\2025
*/

#ifndef _MISC_ESCAPING_H
#define _MISC_ESCAPING_H

#include "base/base_defines_check.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "struct/struct_string.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "misc/misc_two_pass.h"

// These are escape sequences found in C-like strings
typedef enum EscapeSequence EscapeSequence;
enum EscapeSequence
{
	EscapeSequence_None = 0x00,
	
	EscapeSequence_Backslash      = 0x01,
	EscapeSequence_Quote          = 0x02,
	EscapeSequence_Apostrophe     = 0x04,
	EscapeSequence_NewLine        = 0x08,
	EscapeSequence_CarriageReturn = 0x10,
	EscapeSequence_Tab            = 0x20,
	EscapeSequence_Backspace      = 0x40,
	EscapeSequence_Bell           = 0x80,
	
	EscapeSequence_All            = 0xFF,
	EscapeSequence_Common         = (EscapeSequence_Backslash|EscapeSequence_Quote|EscapeSequence_Apostrophe|EscapeSequence_Tab|EscapeSequence_NewLine), //Not CarriageReturn, Backspace, or Bell
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	Str8 EscapeStringEx(Arena* arena, Str8 rawString, u8 escapeSequences, bool addNullTerm);
	PIG_CORE_INLINE Str8 EscapeString(Arena* arena, Str8 rawString);
	Str8 UnescapeStringEx(Arena* arena, Str8 escapedString, u8 escapeSequences, bool addNullTerm);
	PIG_CORE_INLINE Str8 UnescapeString(Arena* arena, Str8 escapedString);
	Str8 EscapeXmlString(Arena* arena, Str8 rawString, bool addNullTerm);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

// This is for escaping C-like strings
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

// This is for unescaping C-like strings
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

PEXP Str8 EscapeXmlString(Arena* arena, Str8 rawString, bool addNullTerm)
{
	TwoPassStr8Loop(result, arena, addNullTerm)
	{
		for (uxx cIndex = 0; cIndex < rawString.length; cIndex++)
		{
			char character = rawString.chars[cIndex];
			// There are only 5 characters that need to be escaped in XML.
			// For text inside an element we don't actually need to escape " ' or > but we do anyways
			// For attribute strings we don't need to escape > but we do anyways
			if (character == '&') { TwoPassStrNt(&result, "&amp;"); }
			else if (character == '<') { TwoPassStrNt(&result, "&lt;"); }
			else if (character == '>') { TwoPassStrNt(&result, "&gt;"); }
			else if (character == '"') { TwoPassStrNt(&result, "&quot;"); }
			else if (character == '\'') { TwoPassStrNt(&result, "&apos;"); }
			else { TwoPassChar(&result, character); }
		}
		TwoPassStr8LoopEnd(&result);
	}
	return result.str;
}

//TODO: Implement UnescapeXmlString

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _MISC_ESCAPING_H
