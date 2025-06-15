/*
File:   parse_c_tokenizer.h
Author: Taylor Robbins
Date:   04\26\2025
Description:
	** Holds an API that helps parse a simplified "C-like" syntax. This syntax has a the following features
	**  1. Whitespace is not significant, besides separating identifiers
	**  2. Identifiers are alphanumeric (with underscores, not starting with a number)
	**  3. Quotes can be added around a bit of text to treat it as a string. Minimal set of escape sequences are implemented inside the quotes, the escape sequences are handled by the tokenizer
	**  4. Double slash comments and slash-star comments are handled by the tokenizer, they are included in the token stream so that they can be re-inserted if the data is getting re-encoded
	**  5. Most non-identifier characters are treated as single-character operators, some well-known multi-character operators are combined into a single token (like ==, !=, ++, etc.) Other multi-character operators can be handled at a higher level
	**  6. Lines starting with # symbol are treated as "directives" with everything but the first identifier on the line being grouped into a single token. Directives are the only-thing that makes new-lines significant, and they can't exist in strings, nor can other syntax exist inside directive lines
	**  7. There are no "syntax errors" at the tokenizer level. The only failures that can occur are related to memory allocation and UTF-8 decoding
*/

#ifndef _PARSE_C_TOKENIZER_H
#define _PARSE_C_TOKENIZER_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "base/base_char.h"
#include "base/base_unicode.h"
#include "misc/misc_result.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
#include "struct/struct_string.h"
#include "struct/struct_var_array.h"

typedef enum cTokenType cTokenType;
enum cTokenType
{
	cTokenType_None = 0,
	cTokenType_Directive,
	cTokenType_Comment,
	cTokenType_Identifier,
	cTokenType_String,
	cTokenType_Number,
	cTokenType_Operator,
	cTokenType_Count,
};
const char* GetcTokenTypeStr(cTokenType enumValue)
{
	switch (enumValue)
	{
		case cTokenType_None:       return "None";
		case cTokenType_Directive:  return "Directive";
		case cTokenType_Comment:    return "Comment";
		case cTokenType_Identifier: return "Identifier";
		case cTokenType_String:     return "String";
		case cTokenType_Number:     return "Number";
		case cTokenType_Operator:   return "Operator";
		case cTokenType_Count:      return "Count";
		default: return "Unknown";
	}
}

typedef enum cTokenFlags cTokenFlags;
enum cTokenFlags
{
	cTokenFlag_None                    = 0x00,
	
	cTokenFlag_ContainedEscapeSequence = 0x01,
	cTokenFlag_IsDoubleSlashComment    = 0x02,
	cTokenFlag_IsOnNewLine             = 0x04,
	
	cTokenFlag_All                     = 0x07,
	cTokenFlag_Count                   = 3,
};

typedef struct cToken cToken;
struct cToken
{
	uxx index; //index in the tokens VarArray
	cTokenType type;
	u8 flags; //cTokenFlags
	Str8 rawStr; //quoted and escaped string, or leading/trailing syntax included for stuff like directives
	Str8 leadingWhitespace;
	Str8 str; //allocated on tokenizer arena
};

typedef struct cTokenizer cTokenizer;
struct cTokenizer
{
	Arena* arena;
	Str8 inputStr;
	bool finished;
	Result error;
	uxx inputByteIndex;
	uxx outputTokenIndex;
	VarArray tokens; //cToken
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeCTokenizer(cTokenizer* tokenizer);
	PIG_CORE_INLINE cTokenizer NewCTokenizer(Arena* arena, Str8 inputStr);
	cToken* NextCToken(cTokenizer* tokenizer);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

static const char* WellKnownMulticharOperators[] = {
	"==", "!=",
	"++", "--", "+=", "-=", "/=", "*=",
	"&&", "||", "&=", "|=",
	"//", "/*", "*/",
	"->",
};

PEXPI void FreeCTokenizer(cTokenizer* tokenizer)
{
	NotNull(tokenizer);
	if (tokenizer->arena != nullptr)
	{
		VarArrayLoop(&tokenizer->tokens, tIndex)
		{
			VarArrayLoopGet(cToken, token, &tokenizer->tokens, tIndex);
			FreeStr8(tokenizer->arena, &token->str);
		}
		FreeVarArray(&tokenizer->tokens);
	}
	ClearPointer(tokenizer);
}

PEXPI cTokenizer NewCTokenizer(Arena* arena, Str8 inputStr)
{
	NotNull(arena);
	NotNullStr(inputStr);
	cTokenizer result = ZEROED;
	result.arena = arena;
	result.inputStr = inputStr;
	result.finished = false;
	result.error = Result_None;
	result.inputByteIndex = 0;
	result.outputTokenIndex = 0;
	InitVarArray(cToken, &result.tokens, arena);
	return result;
}

PEXP cToken* NextCToken(cTokenizer* tokenizer)
{
	NotNull(tokenizer);
	NotNull(tokenizer->arena);
	
	if (tokenizer->finished) { return nullptr; }
	
	if (tokenizer->outputTokenIndex < tokenizer->tokens.length)
	{
		cToken* nextToken = VarArrayGetHard(cToken, &tokenizer->tokens, tokenizer->outputTokenIndex);
		tokenizer->outputTokenIndex++;
		if (tokenizer->outputTokenIndex >= tokenizer->tokens.length && tokenizer->inputByteIndex >= tokenizer->inputStr.length)
		{
			tokenizer->finished = true;
			tokenizer->error = Result_Success;
		}
		return nextToken;
	}
	
	bool isOnNewLine = (tokenizer->inputByteIndex == 0);
	cToken newToken = ZEROED;
	newToken.index = tokenizer->tokens.length;
	if (isOnNewLine) { FlagSet(newToken.flags, cTokenFlag_IsOnNewLine); }
	
	while (tokenizer->inputByteIndex < tokenizer->inputStr.length)
	{
		u32 codepoint = 0;
		u8 codepointSize = GetCodepointForUtf8Str(tokenizer->inputStr, tokenizer->inputByteIndex, &codepoint);
		if (codepointSize == 0)
		{
			tokenizer->finished = true;
			tokenizer->error = Result_InvalidUtf8;
			return nullptr;
		}
		u32 nextCodepoint = 0;
		u8 nextCodepointSize = 0;
		if (tokenizer->inputByteIndex + codepointSize < tokenizer->inputStr.length)
		{
			nextCodepointSize = GetCodepointForUtf8Str(tokenizer->inputStr, tokenizer->inputByteIndex + codepointSize, &nextCodepoint);
		}
		
		// +==============================+
		// |      Consume New-Lines       |
		// +==============================+
		if (codepoint == '\n' || codepoint == '\r')
		{
			bool twoCharNewline = (nextCodepoint != codepoint && (nextCodepoint == '\n' || nextCodepoint == '\r'));
			isOnNewLine = true;
			FlagSet(newToken.flags, cTokenFlag_IsOnNewLine);
			tokenizer->inputByteIndex += codepointSize + (twoCharNewline ? nextCodepointSize : 0);
		}
		// +==============================+
		// |      Consume Whitespace      |
		// +==============================+
		else if (IsCharWhitespace(codepoint, false))
		{
			if (newToken.leadingWhitespace.chars == nullptr) { newToken.leadingWhitespace.chars = &tokenizer->inputStr.chars[tokenizer->inputByteIndex]; }
			newToken.leadingWhitespace.length += codepointSize;
			tokenizer->inputByteIndex += codepointSize;
		}
		// +==============================+
		// |   Consume Directive Token    |
		// +==============================+
		else if (isOnNewLine && codepoint == '#')
		{
			uxx lineEndIndex = tokenizer->inputStr.length;
			uxx identifierEndIndex = tokenizer->inputStr.length;
			for (uxx cIndex = tokenizer->inputByteIndex + codepointSize; cIndex < tokenizer->inputStr.length; cIndex++)
			{
				if (tokenizer->inputStr.chars[cIndex] == '\r' || tokenizer->inputStr.chars[cIndex] == '\n')
				{
					lineEndIndex = cIndex;
					if (identifierEndIndex > lineEndIndex) { identifierEndIndex = lineEndIndex; }
					break;
				}
				else if (identifierEndIndex > cIndex &&
					!IsCharAlphaNumeric(tokenizer->inputStr.chars[cIndex]) && tokenizer->inputStr.chars[cIndex] != '_')
				{
					identifierEndIndex = cIndex;
				}
			}
			
			newToken.type = cTokenType_Directive;
			newToken.rawStr = StrSlice(tokenizer->inputStr, tokenizer->inputByteIndex, lineEndIndex);
			Str8 identiferStr = StrSlice(tokenizer->inputStr, tokenizer->inputByteIndex + codepointSize, identifierEndIndex);
			newToken.str = AllocStr8(tokenizer->arena, identiferStr);
			if (newToken.str.chars == nullptr && identiferStr.length > 0)
			{
				tokenizer->finished = true;
				tokenizer->error = Result_FailedToAllocateMemory;
				return nullptr;
			}
			
			tokenizer->inputByteIndex = lineEndIndex;
			
			cToken* allocToken = VarArrayAdd(cToken, &tokenizer->tokens);
			if (allocToken == nullptr)
			{
				tokenizer->finished = true;
				tokenizer->error = Result_FailedToAllocateMemory;
				return nullptr;
			}
			MyMemCopy(allocToken, &newToken, sizeof(cToken));
			break;
		}
		// +==============================+
		// |     Consume String Token     |
		// +==============================+
		else if (codepoint == '"')
		{
			uxx stringEndIndex = tokenizer->inputStr.length;
			uxx innerStringEndIndex = tokenizer->inputStr.length;
			for (uxx cIndex = tokenizer->inputByteIndex + codepointSize; cIndex < tokenizer->inputStr.length; cIndex++)
			{
				if (tokenizer->inputStr.chars[cIndex] == '\\')
				{
					//skip the next character, we don't need to validate the escape sequence because
					//all we care about is differentiating between a close quote and an escaped quote
					cIndex++;
				}
				else if (tokenizer->inputStr.chars[cIndex] == '"')
				{
					stringEndIndex = cIndex+1;
					innerStringEndIndex = cIndex;
					break;
				}
				else if (tokenizer->inputStr.chars[cIndex] == '\n' || tokenizer->inputStr.chars[cIndex] == '\r')
				{
					stringEndIndex = cIndex;
					innerStringEndIndex = cIndex;
					break;
				}
			}
			
			newToken.type = cTokenType_String;
			newToken.rawStr = StrSlice(tokenizer->inputStr, tokenizer->inputByteIndex, stringEndIndex);
			Str8 innerStr = StrSlice(tokenizer->inputStr, tokenizer->inputByteIndex + codepointSize, innerStringEndIndex);
			newToken.str = UnescapeStringEx(tokenizer->arena, innerStr, EscapeSequence_All, false);
			if (newToken.str.chars == nullptr && innerStr.length > 0)
			{
				tokenizer->finished = true;
				tokenizer->error = Result_FailedToAllocateMemory;
				return nullptr;
			}
			if (innerStr.length != newToken.str.length) { FlagSet(newToken.flags, cTokenFlag_ContainedEscapeSequence); }
			
			tokenizer->inputByteIndex = stringEndIndex;
			
			cToken* allocToken = VarArrayAdd(cToken, &tokenizer->tokens);
			if (allocToken == nullptr)
			{
				tokenizer->finished = true;
				tokenizer->error = Result_FailedToAllocateMemory;
				return nullptr;
			}
			MyMemCopy(allocToken, &newToken, sizeof(cToken));
			break;
		}
		// +==============================+
		// |   Consume Identifier Token   |
		// +==============================+
		else if (IsCharAlphabetic(codepoint) || codepoint == '_')
		{
			uxx identifierEndIndex = tokenizer->inputStr.length;
			for (uxx cIndex = tokenizer->inputByteIndex + codepointSize; cIndex < tokenizer->inputStr.length; cIndex++)
			{
				if (tokenizer->inputStr.chars[cIndex] != '_' && !IsCharAlphaNumeric(tokenizer->inputStr.chars[cIndex]))
				{
					identifierEndIndex = cIndex;
					break;
				}
			}
			
			newToken.type = cTokenType_Identifier;
			newToken.rawStr = StrSlice(tokenizer->inputStr, tokenizer->inputByteIndex, identifierEndIndex);
			newToken.str = AllocStr8(tokenizer->arena, newToken.rawStr);
			if (newToken.str.chars == nullptr && newToken.rawStr.length > 0)
			{
				tokenizer->finished = true;
				tokenizer->error = Result_FailedToAllocateMemory;
				return nullptr;
			}
			
			tokenizer->inputByteIndex = identifierEndIndex;
			
			cToken* allocToken = VarArrayAdd(cToken, &tokenizer->tokens);
			if (allocToken == nullptr)
			{
				tokenizer->finished = true;
				tokenizer->error = Result_FailedToAllocateMemory;
				return nullptr;
			}
			MyMemCopy(allocToken, &newToken, sizeof(cToken));
			break;
		}
		// +==============================+
		// |     Consume Number Token     |
		// +==============================+
		else if (IsCharNumeric(codepoint) || (codepoint == '.' && IsCharNumeric(nextCodepoint)))
		{
			uxx numberEndIndex = tokenizer->inputStr.length;
			bool foundPeriod = (codepoint == '.');
			bool isHexNumber = false;
			bool isBinNumber = false;
			for (uxx cIndex = tokenizer->inputByteIndex + codepointSize; cIndex < tokenizer->inputStr.length; cIndex++)
			{
				if ((isHexNumber && IsCharHexadecimal(tokenizer->inputStr.chars[cIndex])) ||
					(isBinNumber && (tokenizer->inputStr.chars[cIndex] == '0' || tokenizer->inputStr.chars[cIndex] == '1')) ||
					(!isHexNumber && !isBinNumber && IsCharNumeric(tokenizer->inputStr.chars[cIndex])))
				{
					continue;
				}
				else if (cIndex == tokenizer->inputByteIndex + codepointSize && codepoint == '0' && (tokenizer->inputStr.chars[cIndex] == 'x' || tokenizer->inputStr.chars[cIndex] == 'b'))
				{
					isHexNumber = (tokenizer->inputStr.chars[cIndex] == 'x');
					isBinNumber = (tokenizer->inputStr.chars[cIndex] == 'b');
					continue;
				}
				else if (!foundPeriod && tokenizer->inputStr.chars[cIndex] == '.') { foundPeriod = true; continue; }
				//TODO: Should we handle 'f'/'l'/'u'/etc. suffixes?
				numberEndIndex = cIndex;
				break;
			}
			
			newToken.type = cTokenType_Number;
			newToken.rawStr = StrSlice(tokenizer->inputStr, tokenizer->inputByteIndex, numberEndIndex);
			newToken.str = AllocStr8(tokenizer->arena, newToken.rawStr);
			if (newToken.str.chars == nullptr && newToken.rawStr.length > 0)
			{
				tokenizer->finished = true;
				tokenizer->error = Result_FailedToAllocateMemory;
				return nullptr;
			}
			
			tokenizer->inputByteIndex = numberEndIndex;
			
			cToken* allocToken = VarArrayAdd(cToken, &tokenizer->tokens);
			if (allocToken == nullptr)
			{
				tokenizer->finished = true;
				tokenizer->error = Result_FailedToAllocateMemory;
				return nullptr;
			}
			MyMemCopy(allocToken, &newToken, sizeof(cToken));
			break;
		}
		// +==============================+
		// |    Consume Comment Token     |
		// +==============================+
		else if ((codepoint == '/' && nextCodepoint == '/') || (codepoint == '/' && nextCodepoint == '*'))
		{
			bool isSingleLine = (nextCodepoint == '/');
			uxx commentEndIndex = tokenizer->inputStr.length;
			uxx innerEndIndex = tokenizer->inputStr.length;
			for (uxx cIndex = tokenizer->inputByteIndex + codepointSize + nextCodepointSize; cIndex < tokenizer->inputStr.length; cIndex++)
			{
				if (isSingleLine && (tokenizer->inputStr.chars[cIndex] == '\n' || tokenizer->inputStr.chars[cIndex] == '\r'))
				{
					commentEndIndex = cIndex;
					innerEndIndex = cIndex;
					break;
				}
				else if (!isSingleLine && cIndex+1 < tokenizer->inputStr.length && tokenizer->inputStr.chars[cIndex] == '*' && tokenizer->inputStr.chars[cIndex+1] == '/')
				{
					commentEndIndex = cIndex+2;
					innerEndIndex = cIndex;
					break;
				}
			}
			
			if (isSingleLine) { FlagSet(newToken.flags, cTokenFlag_IsDoubleSlashComment); }
			
			newToken.type = cTokenType_Comment;
			newToken.rawStr = StrSlice(tokenizer->inputStr, tokenizer->inputByteIndex, commentEndIndex);
			Str8 innerStr = StrSlice(tokenizer->inputStr, tokenizer->inputByteIndex + codepointSize + nextCodepointSize, innerEndIndex);
			newToken.str = AllocStr8(tokenizer->arena, innerStr);
			if (newToken.str.chars == nullptr && innerStr.length > 0)
			{
				tokenizer->finished = true;
				tokenizer->error = Result_FailedToAllocateMemory;
				return nullptr;
			}
			
			tokenizer->inputByteIndex = commentEndIndex;
			
			cToken* allocToken = VarArrayAdd(cToken, &tokenizer->tokens);
			if (allocToken == nullptr)
			{
				tokenizer->finished = true;
				tokenizer->error = Result_FailedToAllocateMemory;
				return nullptr;
			}
			MyMemCopy(allocToken, &newToken, sizeof(cToken));
			break;
		}
		// +==============================+
		// |    Consume Operator Token    |
		// +==============================+
		else
		{
			uxx operatorEndIndex = tokenizer->inputByteIndex + codepointSize;
			for (uxx oIndex = 0; oIndex < ArrayCount(WellKnownMulticharOperators); oIndex++)
			{
				DebugAssertMsg(WellKnownMulticharOperators[oIndex][2] == '\0', "This code only handles 2-character operators right now!");
				if (codepoint == CharToU32(WellKnownMulticharOperators[oIndex][0]) && nextCodepoint == CharToU32(WellKnownMulticharOperators[oIndex][1]))
				{
					operatorEndIndex = tokenizer->inputByteIndex + codepointSize + nextCodepointSize;
					break;
				}
			}
			
			newToken.type = cTokenType_Operator;
			newToken.rawStr = StrSlice(tokenizer->inputStr, tokenizer->inputByteIndex, operatorEndIndex);
			newToken.str = AllocStr8(tokenizer->arena, newToken.rawStr);
			if (newToken.str.chars == nullptr && newToken.rawStr.length > 0)
			{
				tokenizer->finished = true;
				tokenizer->error = Result_FailedToAllocateMemory;
				return nullptr;
			}
			
			tokenizer->inputByteIndex = operatorEndIndex;
			
			cToken* allocToken = VarArrayAdd(cToken, &tokenizer->tokens);
			if (allocToken == nullptr)
			{
				tokenizer->finished = true;
				tokenizer->error = Result_FailedToAllocateMemory;
				return nullptr;
			}
			MyMemCopy(allocToken, &newToken, sizeof(cToken));
			break;
		}
	}
	
	if (tokenizer->outputTokenIndex < tokenizer->tokens.length)
	{
		cToken* nextToken = VarArrayGetHard(cToken, &tokenizer->tokens, tokenizer->outputTokenIndex);
		tokenizer->outputTokenIndex++;
		if (tokenizer->outputTokenIndex >= tokenizer->tokens.length && tokenizer->inputByteIndex >= tokenizer->inputStr.length)
		{
			tokenizer->finished = true;
			tokenizer->error = Result_Success;
		}
		return nextToken;
	}
	
	if (tokenizer->outputTokenIndex >= tokenizer->tokens.length && tokenizer->inputByteIndex >= tokenizer->inputStr.length)
	{
		tokenizer->finished = true;
		tokenizer->error = Result_Success;
	}
	return nullptr;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _PARSE_C_TOKENIZER_H
