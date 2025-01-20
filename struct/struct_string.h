/*
File:   struct_string.h
Author: Taylor Robbins
Date:   01\06\2025
Description:
	** TODO:
	** NOTE: Str8 does not care about const. It converts const pointers to non-const.
	** It's up to the usage code to track whether a string holding a particular
	** address is allowed to be written to. Similarly, it's up to the usage code
	** to know if a string is null-terminated or not
*/

#ifndef _STRUCT_STRING_H
#define _STRUCT_STRING_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "base/base_char.h"

// +--------------------------------------------------------------+
// |                       String Structure                       |
// +--------------------------------------------------------------+
//NOTE: Str8 could hold UTF-8, ASCII, or any other encoding that uses 8-bit pieces
typedef struct Str8 Str8;
struct Str8
{
	uxx length;
	union { char* chars; u8* bytes; void* pntr; };
};
//NOTE: This structure is useful for all sorts of data encapsulation. "Slice" alias indicates that a piece of code isn't
//      treating the contents of the data as a "string" in the traditional sense, but it's still operating with the same
//      structure layout: a uxx length and void* pntr.
typedef Str8 Slice;

//NOTE: Str16 could hold UTF-16 or USC2 or any other encoding that uses 16-bit pieces
typedef struct Str16 Str16;
struct Str16
{
	uxx length;
	union { char16_t* chars; u16* words; void* pntr; };
};

typedef struct Str8Pair Str8Pair;
struct Str8Pair
{
	union
	{
		Str8 strs[2];
		struct
		{
			union { Str8 key; Str8 left; Str8 first; };
			union { Str8 value; Str8 right; Str8 second; };
		};
	};
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Str8 StrLit(const char* nullTermStr);
	PIG_CORE_INLINE Str8 NewStr8(uxx length, const void* pntr);
	PIG_CORE_INLINE Str16 Str16Lit(const char16_t* nullTermStr);
	PIG_CORE_INLINE Str16 NewStr16(uxx length, const void* pntr);
	PIG_CORE_INLINE Str8Pair NewStr8Pair(Str8 left, Str8 right);
	PIG_CORE_INLINE bool IsEmptyStr(Str8 string) { return (string.length == 0); };
	PIG_CORE_INLINE bool IsNullStr(Str8 string) { return (string.length > 0 && string.pntr == nullptr); };
	PIG_CORE_INLINE bool IsNullTerminated(Str8 string) { return (string.pntr != nullptr && string.chars[string.length] == '\0'); };
	PIG_CORE_INLINE bool IsBufferNullTerminated(uxx bufferSize, const void* bufferPntr);
	Str8 TrimLeadingWhitespace(Str8 target);
	Str8 TrimTrailingWhitespace(Str8 target);
	PIG_CORE_INLINE Str8 TrimWhitespace(Str8 target);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define Str8_Empty       NewStr8(0, nullptr)
#define Str8_Empty_Const ((Str8)ZEROED)

#define Str16_Empty       NewStr16(0, nullptr)
#define Str16_Empty_Const ((Str16)ZEROED)

#define Str8Pair_Empty       NewStr8Pair(MyStr_Empty_Const, MyStr_Empty_Const)
#define Str8Pair_Empty_Const ((Str8Pair)ZEROED)

//NOTE: These are meant to be used when formatting Str8 using any printf like functions
//      Use the format specifier %.*s and then this macro in the var-args
#define StrPrint(string)   (int)(string).length, (string).chars
#define StrPntrPrint(strPntr) (int)(strPntr)->length, (strPntr)->chars

#define AssertNullTerm(string)      Assert(IsNullTerminated(string))
#define NotNullStr(string)          Assert(!IsNullStr(string))
#define NotNullStrPntr(stringPntr)  Assert((stringPntr) != nullptr && !IsNullStr(*(stringPntr)))
#define NotEmptyStr(string)         Assert(!IsEmptyStr(string))
#define NotEmptyStrPntr(stringPntr) Assert(stringPntr != nullptr && !IsEmptyStr(*(stringPntr)))
#if DEBUG_BUILD
#define DebugAssertNullTerm(string)      AssertNullTerm(string)
#define DebugNotNullStr(string)          NotNullStr(string)
#define DebugNotNullStrPntr(stringPntr)  NotNullStrPntr(stringPntr)
#define DebugNotEmptyStr(string)         NotEmptyStr(string)
#define DebugNotEmptyStrPntr(stringPntr) NotEmptyStrPntr(stringPntr)
#else
#define DebugAssertNullTerm(string)      //nothing
#define DebugNotNullStr(string)          //nothing
#define DebugNotNullStrPntr(stringPntr)  //nothing
#define DebugNotEmptyStr(string)         //nothing
#define DebugNotEmptyStrPntr(stringPntr) //nothing
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                       Basic String API                       |
// +--------------------------------------------------------------+
PEXPI Str8 StrLit(const char* nullTermStr)
{
	Str8 result;
	result.length = (nullTermStr != nullptr) ? (uxx)MyStrLength64(nullTermStr) : 0;
	result.chars = (char*)nullTermStr; //throw away const qualifier
	return result;
}
PEXPI Str8 NewStr8(uxx length, const void* pntr)
{
	Str8 result;
	result.length = length;
	result.pntr = (void*)pntr; //throw away const qualifier
	return result;
}

PEXPI Str16 Str16Lit(const char16_t* nullTermStr)
{
	Str16 result;
	result.length = ((nullTermStr != nullptr) ? (uxx)MyWideStrLength(nullTermStr) : 0);
	result.chars = (char16_t*)nullTermStr;
	return result;
}
PEXPI Str16 NewStr16(uxx length, const void* pntr)
{
	Str16 result;
	result.length = length;
	result.pntr = (void*)pntr; //throw away const qualifier
	return result;
}

PEXPI Str8Pair NewStr8Pair(Str8 left, Str8 right)
{
	Str8Pair result;
	result.left = left;
	result.right = right;
	return result;
}

PEXPI bool IsEmptyStr(Str8 string) { return (string.length == 0); }
PEXPI bool IsNullStr(Str8 string) { return (string.length > 0 && string.pntr == nullptr); }
PEXPI bool IsNullTerminated(Str8 string) { return (string.pntr != nullptr && string.chars[string.length] == '\0'); }
PEXPI bool IsBufferNullTerminated(uxx bufferSize, const void* bufferPntr)
{
	if (bufferPntr == nullptr) { return false; }
	for (uxx bIndex = 0; bIndex < bufferSize; bIndex++) { if (((char*)bufferPntr)[bIndex] == '\0') { return true; } }
	return false;
}

// +--------------------------------------------------------------+
// |                 String Manipulation Helpers                  |
// +--------------------------------------------------------------+
PEXP Str8 TrimLeadingWhitespace(Str8 target)
{
	NotNullStr(target);
	Str8 result = target;
	while (result.length > 0)
	{
		if (IsCharWhitespace(result.chars[0], false))
		{
			result.length--;
			result.chars++;
		}
		else { break; }
	}
	return result;
}
PEXP Str8 TrimTrailingWhitespace(Str8 target)
{
	NotNullStr(target);
	Str8 result = target;
	while (result.length > 0)
	{
		if (IsCharWhitespace(result.chars[result.length-1], false))
		{
			result.length--;
		}
		else { break; }
	}
	return result;
}

PEXPI Str8 TrimWhitespace(Str8 target)
{
	NotNullStr(target);
	Str8 result = target;
	result = TrimLeadingWhitespace(result);
	result = TrimTrailingWhitespace(result);
	return result;
}

//TODO: Str8 StrSubstring(Str8* target, uxx startIndex)
//TODO: Str8 StrSubstring(Str8* target, uxx startIndex, uxx endIndex)
//TODO: Str8 StrSubstringLength(Str8* target, uxx startIndex, uxx length)
//TODO: Str8 CombineStrs(MemArena_t* memArena, Str8 str1, Str8 str2)
//TODO: Str8 CombineStrs(MemArena_t* memArena, Str8 str1, Str8 str2, Str8 str3)
//TODO: Str8 CombineStrs(MemArena_t* memArena, Str8 str1, Str8 str2, Str8 str3, Str8 str4)
//TODO: bool StrEquals(Str8 target, Str8 comparison)
//TODO: bool StrEquals(Str8 target, const char* comparisonNt)
//TODO: bool StrEquals(Str8 target, uxx comparisonLength, const char* comparisonPntr)
//TODO: bool StrEquals(const char* comparisonNt, Str8 target)
//TODO: bool StrEquals(uxx comparisonLength, const char* comparisonPntr, Str8 target)
//TODO: i32 StrCompareIgnoreCase(Str8 str1, Str8 str2, uxx compareLength)
//TODO: i32 StrCompareIgnoreCase(Str8 str1, Str8 str2)
//TODO: i32 StrCompareIgnoreCase(Str8 str1, const char* nullTermStr, uxx compareLength)
//TODO: i32 StrCompareIgnoreCase(Str8 str1, const char* nullTermStr)
//TODO: i32 StrCompareIgnoreCase(const char* str1, const char* str2, uxx compareLength)
//TODO: bool StrEqualsIgnoreCase(Str8 target, Str8 comparison)
//TODO: bool StrEqualsIgnoreCase(Str8 target, const char* comparisonNt)
//TODO: bool StrEqualsIgnoreCase(Str8 target, uxx comparisonLength, const char* comparisonPntr)
//TODO: bool StrEqualsIgnoreCase(const char* comparisonNt, Str8 target)
//TODO: bool StrEqualsIgnoreCase(uxx comparisonLength, const char* comparisonPntr, Str8 target)
//TODO: bool StrStartsWith(Str8 str, Str8 prefix, bool ignoreCase = false)
//TODO: bool StrStartsWith(Str8 str, const char* nullTermPrefixStr, bool ignoreCase = false)
//TODO: bool StrStartsWith(const char* nullTermStr, Str8 prefix, bool ignoreCase = false)
//TODO: bool StrStartsWith(const char* nullTermStr, const char* nullTermPrefixStr, bool ignoreCase = false)
//TODO: bool StrEndsWith(Str8 str, Str8 suffix, bool ignoreCase = false)
//TODO: bool StrEndsWith(Str8 str, const char* nullTermSuffix, bool ignoreCase = false)
//TODO: bool StrEndsWith(const char* nullTermStr, Str8 suffix, bool ignoreCase = false)
//TODO: bool StrEndsWith(const char* nullTermStr, const char* nullTermSuffix, bool ignoreCase = false)
//TODO: bool StrStartsWithSlash(Str8 str)
//TODO: bool StrStartsWithSlash(const char* nullTermStr)
//TODO: bool StrEndsWithSlash(Str8 str)
//TODO: bool StrEndsWithSlash(const char* nullTermStr)
//TODO: bool SplitStringFixed(Str8 target, Str8 delineator, uxx numPieces, Str8* piecesBuffer, bool ignoreCase = false)
//TODO: bool SplitStringFixed(Str8 target, const char* delineatorNullTerm, uxx numPieces, Str8* piecesBuffer, bool ignoreCase = false)
//TODO: bool SplitStringFixed(const char* targetNullTerm, Str8 delineator, uxx numPieces, Str8* piecesBuffer, bool ignoreCase = false)
//TODO: bool SplitStringFixed(const char* targetNullTerm, const char* delineatorNullTerm, uxx numPieces, Str8* piecesBuffer, bool ignoreCase = false)
//TODO: Str8* SplitString(MemArena_t* memArena, Str8 target, Str8 delineator, uxx* numPiecesOut = nullptr, bool ignoreCase = false)
//TODO: Str8* SplitString(MemArena_t* memArena, Str8 target, const char* delineatorNt, uxx* numPiecesOut = nullptr, bool ignoreCase = false)
//TODO: Str8* SplitString(MemArena_t* memArena, const char* targetNt, Str8 delineator, uxx* numPiecesOut = nullptr, bool ignoreCase = false)
//TODO: Str8* SplitString(MemArena_t* memArena, const char* targetNt, const char* delineatorNt, uxx* numPiecesOut = nullptr, bool ignoreCase = false)
//TODO: bool SplitStringFast(SplitStringContext_t* context, Str8 target, char separatorChar, bool includeEmptyPieces = false)
//TODO: Str8* SplitStringBySpacesFastTemp(MemArena_t* tempArena, Str8 target, uxx* numPiecesOut)
//TODO: Str8* SplitStringBySlashesFastTemp(MemArena_t* tempArena, Str8 target, uxx* numPiecesOut)
//TODO: uxx UnescapeQuotedStringInPlace(Str8* target, bool removeQuotes = true, bool allowNewLineEscapes = true, bool allowOtherEscapeCodes = false)
//TODO: Str8 UnescapeQuotedStringInArena(MemArena_t* memArena, Str8 target, bool removeQuotes = true, bool allowNewLineEscapes = true, bool allowOtherEscapeCodes = false)
//TODO: Str8* SplitStringBySpacesWithQuotesAndUnescape(MemArena_t* memArena, Str8 target, uxx* numPiecesOut)
//TODO: void StrSpliceInPlace(Str8 target, uxx startIndex, Str8 replacement)
//TODO: void StrSpliceInPlace(Str8 target, uxx startIndex, const char* replacementNullTerm)
//TODO: void StrSpliceInPlace(char* targetNullTermStr, uxx startIndex, Str8 replacement)
//TODO: void StrSpliceInPlace(char* targetNullTermStr, uxx startIndex, const char* replacementNullTerm)
//TODO: Str8 StrSplice(Str8 target, uxx startIndex, uxx endIndex, Str8 replacement, MemArena_t* memArena)
//TODO: Str8 StrSplice(Str8 target, uxx startIndex, uxx endIndex, const char* replacementNullTerm, MemArena_t* memArena)
//TODO: Str8 StrSplice(char* targetNullTermStr, uxx startIndex, uxx endIndex, Str8 replacement, MemArena_t* memArena)
//TODO: Str8 StrSplice(char* targetNullTermStr, uxx startIndex, uxx endIndex, const char* replacement, MemArena_t* memArena)
//TODO: uxx StrReplaceInPlace(Str8 str, Str8 target, Str8 replacement, bool ignoreCase = false, bool allowShrinking = false)
//TODO: uxx StrReplaceInPlace(Str8 str, const char* target, const char* replacement, bool ignoreCase = false, bool allowShrinking = false)
//TODO: Str8 StrReplace(Str8 str, Str8 target, Str8 replacement, MemArena_t* memArena)
//TODO: Str8 StrReplace(Str8 str, const char* target, const char* replacement, MemArena_t* memArena)
//TODO: Str8 StrReplaceMultiple(Str8 str, uxx numReplacements, const MyStrPair_t* replacements, MemArena_t* memArena)
//TODO: bool FindSubstring(Str8 target, Str8 substring, uxx* indexOut = nullptr, bool ignoreCase = false, uxx startIndex = 0)
//TODO: bool FindSubstring(Str8 target, const char* nullTermSubstring, uxx* indexOut= nullptr, bool ignoreCase = false, uxx startIndex = 0)
//TODO: bool FindSubstring(const char* nullTermTarget, Str8 substring, uxx* indexOut= nullptr, bool ignoreCase = false, uxx startIndex = 0)
//TODO: bool FindSubstring(const char* nullTermTarget, const char* nullTermSubstring, uxx* indexOut= nullptr, bool ignoreCase = false, uxx startIndex = 0)
//TODO: Str8 FindStrParensPart(Str8 target, char openParensChar = '(', char closeParensChar = ')')
//TODO: Str8 FindStrParensPart(const char* nullTermTarget, char openParensChar = '(', char closeParensChar = ')')
//TODO: Str8 StringRepeat(MemArena_t* memArena, Str8 str, uxx numRepetitions)
//TODO: Str8 StringRepeat(MemArena_t* memArena, const char* nullTermStr, uxx numRepetitions)
//TODO: Str8 FormatBytes(uxx numBytes, MemArena_t* memArena)
//TODO: const char* FormatBytesNt(uxx numBytes, MemArena_t* memArena)
//TODO: Str8 FormatNumberWithCommas(uxx number, MemArena_t* memArena = nullptr)
//TODO: const char* FormatNumberWithCommasNt(uxx number, MemArena_t* memArena = nullptr)
//TODO: bool IsValidIdentifierStr(Str8 str, bool allowUnderscores = true, bool allowNumbers = true, bool allowLeadingNumbers = false, bool allowEmpty = false, bool allowSpaces = false)
//TODO: bool IsStringMadeOfChars(Str8 str, Str8 allowedChars, uxx* firstInvalidCharOut = nullptr)
//TODO: void StrReallocAppend(Str8* baseStr, Str8 appendStr, MemArena_t* memArena)
//TODO: void StrReallocAppend(Str8* baseStr, const char* appendNullTermStr, MemArena_t* memArena)
//TODO: WordBreakCharClass_t GetWordBreakCharClass(u32 codepoint)
//TODO: bool IsCharPairWordBreak(u32 prevCodepoint, u32 nextCodepoint, bool forward, bool subwords)
//TODO: uxx FindNextWordBreakInString(Str8 str, uxx startIndex, bool forward, bool subwords, bool includeBreakAtStartIndex = false)
//TODO: Str8 FormatRealTime(const RealTime_t* realTime, MemArena_t* memArena, bool includeDayOfWeek = true, bool includeHourMinuteSecond = true, bool includeMonthDayYear = true)
//TODO: const char* FormatRealTimeNt(const RealTime_t* realTime, MemArena_t* memArena, bool includeDayOfWeek = true, bool includeHourMinuteSecond = true, bool includeMonthDayYear = true)
//TODO: Str8 FormatMilliseconds(uxx milliseconds, MemArena_t* memArena)
//TODO: const char* FormatMillisecondsNt(uxx milliseconds, MemArena_t* memArena)

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_STRING_H

#if defined(_MEM_ARENA_H) && defined(_STRUCT_STRING_H)
#include "cross/cross_mem_arena_and_string.h"
#endif

#if defined(_STRUCT_STRING_H) && defined(_BASE_UNICODE_H)
#include "cross/cross_string_and_unicode.h"
#endif

#if defined(_MEM_ARENA_H) && defined(_STRUCT_STRING_H) && defined(_BASE_UNICODE_H)
#include "cross/cross_mem_arena_string_and_unicode.h"
#endif

#if defined(_MISC_HASH_H) && defined(_STRUCT_STRING_H)
#include "cross/cross_string_and_hash.h"
#endif
