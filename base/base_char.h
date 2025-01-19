/*
File:   base_char.h
Author: Taylor Robbins
Date:   01\06\2025
Description:
	** Holds a small collection of functions that let us ask questions about a
	** particular character (aka "char"). These are often simple questions meant
	** for use with strictly ASCII only strings, more complex\accurate functions
	** can be found in base_unicode.h
*/

#ifndef _BASE_CHAR_H
#define _BASE_CHAR_H

#include "base/base_typedefs.h"
#include "base/base_assert.h"

#define ALPHA_LOWERCASE_CHARS "abcdefghijklmnopqrstuvwxyz"
#define ALPHA_UPPERCASE_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define ALPHA_ALL_CHARS       "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define HEX_LOWERCASE_CHARS   "0123456789abcdef"
#define HEX_UPPERCASE_CHARS   "0123456789ABCDEF"
#define HEX_ALL_CHARS         "0123456789abcdefABCDEF"
#define WHITESPACE_CHARS_EX   " \t\r\n"
#define WHITESPACE_CHARS      " \t"

char ToLowerChar(char c)
{
	if (c >= 'A' && c <= 'Z') { return 'a' + (c - 'A'); }
	return c;
}
char ToUpperChar(char c)
{
	if (c >= 'a' && c <= 'z') { return 'A' + (c - 'a'); }
	return c;
}

u8 CharToU8(char c)
{
	return *((u8*)&c);
}
u32 CharToU32(char c)
{
	return (u32)(*((u8*)&c));
}

char GetHexChar(u8 hexValue, bool upperCase)
{
	if (hexValue <= 9) { return '0' + hexValue; }
	else if (hexValue < 16) { return (upperCase ? 'A' : 'a') + (hexValue - 10); }
	else { return '?'; }
}
u8 GetUpperNibble(u8 value)
{
	return ((value & 0xF0) >> 4);
}
u8 GetLowerNibble(u8 value)
{
	return ((value & 0x0F) >> 0);
}

u8 GetHexCharValue(char hexChar)
{
	if (hexChar >= '0' && hexChar <= '9')
	{
		return (u8)(hexChar - '0');
	}
	else if (hexChar >= 'A' && hexChar <= 'F')
	{
		return (u8)(hexChar - 'A') + 10;
	}
	else if (hexChar >= 'a' && hexChar <= 'f')
	{
		return (u8)(hexChar - 'a') + 10;
	}
	else { return 0; }
}


bool IsCharNumeric(u32 codepoint)
{
	return (codepoint >= '0' && codepoint <= '9');
}
bool IsCharHexadecimal(u32 codepoint)
{
	return ((codepoint >= '0' && codepoint <= '9') ||
		(codepoint >= 'A' && codepoint <= 'F') ||
		(codepoint >= 'a' && codepoint <= 'f'));
}
bool AreCharsHexidecimal(u64 numChars, char* charsPntr)
{
	if (numChars > 0) { Assert(charsPntr != nullptr); }
	for (u64 cIndex = 0; cIndex < numChars; cIndex++)
	{
		if (!IsCharHexadecimal(CharToU32(charsPntr[cIndex]))) { return false; }
	}
	return true;
}
bool IsCharAlphabetic(u32 codepoint)
{
	if (codepoint >= 'a' && codepoint <= 'z') { return true; }
	else if (codepoint >= 'A' && codepoint <= 'Z') { return true; }
	else { return false; }
}
bool IsCharLowercaseAlphabet(u32 codepoint)
{
	return (codepoint >= 'a' && codepoint <= 'z');
}
bool IsCharUppercaseAlphabet(u32 codepoint)
{
	return (codepoint >= 'A' && codepoint <= 'Z');
}
bool IsCharWhitespace(u32 codepoint, bool includeNewLines)
{
	if (codepoint == ' ') { return true; }
	else if (codepoint == '\t') { return true; }
	else if (codepoint == '\n' && includeNewLines) { return true; }
	else if (codepoint == '\r' && includeNewLines) { return true; }
	else { return false; }
}
bool IsCharAnsii(u32 codepoint)
{
	return (codepoint < 128);
}
bool IsCharRenderableAnsii(u32 codepoint)
{
	return (codepoint >= ' ' && codepoint <= '~');
}
bool IsCharExtendedAnsii(u32 codepoint)
{
	return (codepoint <= 255);
}
#ifdef IsCharAlphaNumeric
//NOTE: winuser.h defines this macro as an alias for IsCharAlphaNumericA or IsCharAlphaNumericW
#undef IsCharAlphaNumeric
#endif
bool IsCharAlphaNumeric(u32 codepoint)
{
	return (IsCharAlphabetic(codepoint) || IsCharNumeric(codepoint));
}
bool IsCharSyntax(u32 codepoint)
{
	return (IsCharRenderableAnsii(codepoint) && !IsCharAlphaNumeric(codepoint));
}
bool IsCharPunctuationStart(u32 codepoint)
{
	return (codepoint == '(' || codepoint == '[' || codepoint == '<' || codepoint == '{' || codepoint == '"');
}
bool IsCharPunctuationEnd(u32 codepoint)
{
	return (codepoint == ')' || codepoint == ']' || codepoint == '>' || codepoint == '}' || codepoint == '"');
}

bool IsValidIdentifier(uxx strLength, const char* strPntr, bool allowEmpty, bool allowDashes, bool allowSpaces)
{
	Assert(strPntr != nullptr || strLength == 0);
	if (strLength == 0) { return allowEmpty; }
	
	for (uxx cIndex = 0; cIndex < strLength; cIndex++)
	{
		char nextChar = strPntr[cIndex];
		if ((nextChar >= 'a' && nextChar <= 'z') ||
			(nextChar >= 'A' && nextChar <= 'Z'))
		{
			continue;
		}
		else if (nextChar >= '0' && nextChar <= '9')
		{
			if (cIndex == 0) { return false; }
			continue;
		}
		else if (nextChar == '_')
		{
			continue;
		}
		else if (nextChar == ' ' && allowSpaces)
		{
			continue;
		}
		else if (nextChar == '-' && allowDashes)
		{
			continue;
		}
		else
		{
			return false;
		}
	}
	
	return true;
}

#endif //  _BASE_CHAR_H
