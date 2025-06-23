/*
File:   misc_regex.h
Author: Taylor Robbins
Date:   02\02\2025
Description:
	** This file holds functions and types that help us parse Regular Expressions
	** and search strings for matches to those regex patterns. Currently we rely on
	** the Super Light Regexp Engine (SLRE) for the implementation, which means we
	** only support a limited set of Perl Regex functionality. We may move to another
	** implementation later or make our own, but generally we don't rely on regex
	** terribly often, and when we do we usually only care about a subset of the
	** widely accepted functionality so a simple implementation like SLRE is better
	** than something like Google's RE2 (https://github.com/google/re2)
*/

#ifndef _MISC_REGEX_H
#define _MISC_REGEX_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "misc/misc_result.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "struct/struct_string.h"

#if !USING_CUSTOM_STDLIB //slre uses some some stdlib functions that our custom stdlib implementation doesn't support yet

#include "third_party/slre/slre.h"

//NOTE: When writing regex patterns with captures make sure you stay below this number (bump this number up as needed)
#define MAX_NUM_REGEX_CAPTURES    8

typedef struct slre_cap slre_cap;

typedef struct RegexResult RegexResult;
struct RegexResult
{
	Result result; //Typically Result_Success or Result_NoMatch, other error codes for invalid regex or input
	Str8 haystack;
	// uxx matchBeginIndex; //TODO: Can we somehow get the match beginning from SLRE?
	uxx matchEndIndex;
	uxx numCaptures;
	Str8 captures[MAX_NUM_REGEX_CAPTURES]; //these are all slices of haystack
	uxx captureIndices[MAX_NUM_REGEX_CAPTURES];
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	Result GetResultForSlreErrorCode(int slreErrorCode);
	RegexResult StrRegexFind(Str8 haystack, Str8 needleRegexPattern, bool caseSensitive);
	PIG_CORE_INLINE RegexResult StrExactRegexFind(Str8 haystack, Str8 needleRegexPattern);
	PIG_CORE_INLINE RegexResult StrAnyCaseRegexFind(Str8 haystack, Str8 needleRegexPattern);
	PIG_CORE_INLINE bool StrRegexContains(Str8 haystack, Str8 needleRegexPattern, bool caseSensitive);
	PIG_CORE_INLINE bool StrExactRegexContains(Str8 haystack, Str8 needleRegexPattern);
	PIG_CORE_INLINE bool StrAnyCaseRegexContains(Str8 haystack, Str8 needleRegexPattern);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

#if DEBUG_BUILD && !TARGET_IS_PLAYDATE
#define SLRE_DEBUG
#endif
#include "third_party/slre/slre.c"

PEXP Result GetResultForSlreErrorCode(int slreErrorCode)
{
	switch (slreErrorCode)
	{
		case SLRE_NO_MATCH:              return Result_NoMatch;
		case SLRE_UNEXPECTED_QUANTIFIER: return Result_UnexpectedQuantifier;
		case SLRE_UNBALANCED_BRACKETS:   return Result_UnbalancedBrackets;
		case SLRE_INTERNAL_ERROR:        return Result_Failure;
		case SLRE_INVALID_CHARACTER_SET: return Result_InvalidCharacter;
		case SLRE_INVALID_METACHARACTER: return Result_InvalidCharacter;
		case SLRE_CAPS_ARRAY_TOO_SMALL:  return Result_NotEnoughSpace;
		case SLRE_TOO_MANY_BRANCHES:     return Result_TooManyBranches;
		case SLRE_TOO_MANY_BRACKETS:     return Result_TooManyBrackets;
		default: return Result_Failure;
	}
}

PEXP RegexResult StrRegexFind(Str8 haystack, Str8 needleRegexPattern, bool caseSensitive)
{
	Assert(haystack.length <= INT_MAX);
	RegexResult result = ZEROED;
	result.haystack = haystack;
	// result.matchBeginIndex = haystack.length;
	result.matchEndIndex = haystack.length;
	
	ScratchBegin(scratch);
	Str8 haystackNt = AllocStrAndCopy(scratch, haystack.length, haystack.chars, true);
	NotNull(haystackNt.chars);
	Str8 regexNt = AllocStrAndCopy(scratch, needleRegexPattern.length, needleRegexPattern.chars, true);
	NotNull(regexNt.chars);
	
	slre_cap captures[MAX_NUM_REGEX_CAPTURES] = ZEROED;
	int matchResult = slre_match(regexNt.chars, haystackNt.chars, (int)haystackNt.length, &captures[0], MAX_NUM_REGEX_CAPTURES, caseSensitive ? 0 : SLRE_IGNORE_CASE);
	if (matchResult < 0) { result.result = GetResultForSlreErrorCode(matchResult); ScratchEnd(scratch); return result; }
	
	result.result = Result_Success;
	//TODO: We need to figure out why the matchResult is sometimes coming back as 1 past the end!
	Assert((uxx)matchResult <= haystack.length+1);
	result.matchEndIndex = (uxx)matchResult;
	
	for (uxx cIndex = 0; cIndex < MAX_NUM_REGEX_CAPTURES; cIndex++)
	{
		slre_cap* slreCapture = &captures[cIndex];
		if (slreCapture->ptr != nullptr)
		{
			//TODO: We need to figure out why the matchResult is sometimes coming back as 1 past the end!
			Assert(IsSizedPntrWithin(haystackNt.chars, haystackNt.length+1, slreCapture->ptr, slreCapture->len));
			if (slreCapture->ptr + slreCapture->len == haystackNt.chars + haystackNt.length+1) { Decrement(slreCapture->len); } //subtract 1 from the length if it dangles off the end
			result.captures[result.numCaptures] = NewStr8((uxx)slreCapture->len, &haystack.chars[(slreCapture->ptr - haystackNt.chars)]);
			result.captureIndices[result.numCaptures] = (uxx)(slreCapture->ptr - haystack.chars);
			// PrintLine_D("slreCapture[%llu]: \"%.*s\" at %llu", (u64)cIndex, StrPrint(result.captures[result.numCaptures]), (u64)result.captureIndices[result.numCaptures]);
			result.numCaptures++;
		}
	}
	
	ScratchEnd(scratch);
	return result;
}
PEXPI RegexResult StrExactRegexFind(Str8 haystack, Str8 needleRegexPattern)
{
	return StrRegexFind(haystack, needleRegexPattern, true);
}
PEXPI RegexResult StrAnyCaseRegexFind(Str8 haystack, Str8 needleRegexPattern)
{
	return StrRegexFind(haystack, needleRegexPattern, false);
}

PEXPI bool StrRegexContains(Str8 haystack, Str8 needleRegexPattern, bool caseSensitive)
{
	RegexResult result = StrRegexFind(haystack, needleRegexPattern, caseSensitive);
	return (result.result == Result_Success);
}
PEXPI bool StrExactRegexContains(Str8 haystack, Str8 needleRegexPattern)
{
	return StrRegexContains(haystack, needleRegexPattern, true);
}
PEXPI bool StrAnyCaseRegexContains(Str8 haystack, Str8 needleRegexPattern)
{
	return StrRegexContains(haystack, needleRegexPattern, false);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //!USING_CUSTOM_STDLIB

#endif //  _MISC_REGEX_H
