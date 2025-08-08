/*
File:   struct_string.h
Author: Taylor Robbins
Date:   01\06\2025
Description:
	** Contains the super important data structure called Str8 (as well as Str16 and Str8Pair which we occassionally use)
	** Str8 simply is the pairing of a length and a pointer and is often used to store
	** string (or substring) data without the need for a null-terminating character
	** to determine it's length. Str8 also can be used for non-character data and
	** as such has an alias "Slice" which should be preferred in those cases. (NOTE: FilePath is also an alias of Str8)
	** The 8 in Str8 indicates it holds single-byte encodings such as UTF-8 or ASCII.
	** The exact encoding in a Str8 is not guaranteed by the data structure, it's up
	** to the usage code to remember what is stored in a Str8, and functions taking
	** Str8 with specific kinds of encodings should be clearly documented as to what
	** kind of data they expect to injest. Generally in this codebase we assume usage
	** of UTF-8 encoding wherever possible. Any robust function that takes a Str8 and
	** deals with unicode codepoints that are encoded as multibyte sequences should
	** use functions from base_unicode.h to convert the bytes to characters as it loops
	** over the data. However, most functions that deal with strings are only operating
	** on characters that are single-byte encoded and are the same as ASCII so they 
	** may choose to iterate over the bytes individually, treating them as ASCII
	** characters. This only works because all multi-byte characters in UTF-8 have
	** bytes that are ALL above value 128 meaning they will never be confused for
	** a valid single-byte encoded character.
	** NOTE: Str8 does not care about const. It converts const pointers to non-const.
	** It's up to the usage code to track whether a string holding a particular
	** address is allowed to be written to. Similarly, it's up to the usage code
	** to know if a string is null-terminated or not
	** NOTE: To make a Str8 from a string literal like "hello" wrap the literal with StrLit("hello")
	** This will call strlen on the pointer to fill the length of the Str8 struct.
	** Note this can also be used any string that has null-terminator not just string literals
	** NOTE: Any Str8 that has a non-zero length is required to have a non-null pointer value.
	** Many functions will check for this degenerate case with NotNullStr(...).
	** However a Str8 with 0 length is allowed to have a null pointer value.
	** A 0 length Str8 can have a valid pointer if it was allocated with a null-terminating
	** char but basically no function will touch the pointer if the length is 0
	** besides something like FreeStr8WithNt.
	** NOTE: This file does not directly depend on base_unicode.h or mem_arena.h
	** allowing it to be used it more limited scenarios (like embedded systems).
	** There are "cross" files that extend the API when those other files are
	** included alongside this one.
*/

#ifndef _STRUCT_STRING_H
#define _STRUCT_STRING_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "base/base_char.h"
#include "std/std_memset.h"

// +--------------------------------------------------------------+
// |                       String Structure                       |
// +--------------------------------------------------------------+
//NOTE: Str8 could hold UTF-8, ASCII, or any other encoding that uses 8-bit pieces
typedef plex Str8 Str8;
plex Str8
{
	uxx length;
	car { char* chars; u8* bytes; void* pntr; };
};
//NOTE: This structure is useful for all sorts of data encapsulation. "Slice" alias indicates that a piece of code isn't
//      treating the contents of the data as a "string" in the traditional sense, but it's still operating with the same
//      structure layout: a uxx length and void* pntr.
typedef Str8 Slice;

//NOTE: Str16 could hold UTF-16 or USC2 or any other encoding that uses 16-bit pieces
typedef plex Str16 Str16;
plex Str16
{
	uxx length;
	car { char16_t* chars; u16* words; void* pntr; };
};

typedef plex Str8Pair Str8Pair;
plex Str8Pair
{
	car
	{
		Str8 strs[2];
		plex
		{
			car { Str8 key; Str8 left; Str8 first; };
			car { Str8 value; Str8 right; Str8 second; };
		};
	};
};

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
	PIG_CORE_INLINE Str8 StrLit(const char* nullTermStr);
	PIG_CORE_INLINE Str8 NewStr8(uxx length, const void* pntr);
	PIG_CORE_INLINE Str16 Str16Lit(const char16_t* nullTermStr);
	PIG_CORE_INLINE Str16 NewStr16(uxx length, const void* pntr);
	PIG_CORE_INLINE Str8Pair NewStr8Pair(Str8 left, Str8 right);
	#if TARGET_IS_ORCA
	PIG_CORE_INLINE oc_str8 ToOcStr8(Str8 str);
	PIG_CORE_INLINE Str8 ToStr8FromOc(oc_str8 orcaStr);
	#endif //TARGET_IS_ORCA
	PIG_CORE_INLINE bool IsEmptyStr(Str8 string) { return (string.length == 0); };
	PIG_CORE_INLINE bool IsNullStr(Str8 string) { return (string.length > 0 && string.pntr == nullptr); };
	PIG_CORE_INLINE bool IsNullTerminated(Str8 string) { return (string.pntr != nullptr && string.chars[string.length] == '\0'); };
	PIG_CORE_INLINE bool IsBufferNullTerminated(uxx bufferSize, const void* bufferPntr);
	Str8 TrimLeadingWhitespaceAndNewLines(Str8 target);
	Str8 TrimTrailingWhitespaceAndNewLines(Str8 target);
	Str8 TrimLeadingWhitespace(Str8 target);
	Str8 TrimTrailingWhitespace(Str8 target);
	PIG_CORE_INLINE Str8 TrimWhitespaceAndNewLines(Str8 target);
	PIG_CORE_INLINE Str8 TrimWhitespace(Str8 target);
	PIG_CORE_INLINE Str8 StrSlice(Str8 target, uxx startIndex, uxx endIndex);
	PIG_CORE_INLINE Str8 StrSlicePntrs(Str8 target, const void* startPntr, const void* endPntr);
	PIG_CORE_INLINE Str8 StrSliceFrom(Str8 target, uxx startIndex);
	PIG_CORE_INLINE Str8 StrSliceFromPntr(Str8 target, const void* startPntr);
	PIG_CORE_INLINE Str8 StrSliceLength(Str8 target, uxx startIndex, uxx length);
	PIG_CORE_INLINE Str8 StrSliceMaxLength(Str8 target, uxx startIndex, uxx maxLength);
	PIG_CORE_INLINE bool StrExactEquals(Str8 left, Str8 right);
	PIG_CORE_INLINE bool StrExactEqualsAt(Str8 left, Str8 right, uxx leftIndex);
	PIG_CORE_INLINE bool StrExactStartsWith(Str8 target, Str8 prefix);
	PIG_CORE_INLINE bool StrExactEndsWith(Str8 target, Str8 suffix);
	bool StrExactContains(Str8 haystack, Str8 needle);
	uxx StrExactFind(Str8 haystack, Str8 needle);
	PIG_CORE_INLINE bool StrTryExactFind(Str8 haystack, Str8 needle, uxx* indexOut);
	bool StrAnyCaseEquals(Str8 left, Str8 right);
	PIG_CORE_INLINE bool StrAnyCaseEqualsAt(Str8 left, Str8 right, uxx leftIndex);
	PIG_CORE_INLINE bool StrAnyCaseStartsWith(Str8 target, Str8 prefix);
	PIG_CORE_INLINE bool StrAnyCaseEndsWith(Str8 target, Str8 suffix);
	PIG_CORE_INLINE bool StrAnyCaseContains(Str8 haystack, Str8 needle);
	PIG_CORE_INLINE uxx StrAnyCaseFind(Str8 haystack, Str8 needle);
	PIG_CORE_INLINE bool StrTryAnyCaseFind(Str8 haystack, Str8 needle, uxx* indexOut);
	PIG_CORE_INLINE bool StrEquals(Str8 left, Str8 right, bool caseSensitive);
	PIG_CORE_INLINE bool StrEqualsAt(Str8 left, Str8 right, uxx leftIndex, bool caseSensitive);
	PIG_CORE_INLINE bool StrStartsWith(Str8 target, Str8 prefix, bool caseSensitive);
	PIG_CORE_INLINE bool StrEndsWith(Str8 target, Str8 suffix, bool caseSensitive);
	PIG_CORE_INLINE bool StrContains(Str8 haystack, Str8 needle, bool caseSensitive);
	PIG_CORE_INLINE uxx StrFind(Str8 haystack, Str8 needle, bool caseSensitive);
	PIG_CORE_INLINE bool StrTryFind(Str8 haystack, Str8 needle, bool caseSensitive, uxx* indexOut);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define Str8_Empty        NewStr8(0, nullptr)
#define Str8_Empty_Const  ((Str8)ZEROED)
#define Str8_Space        NewStr8(1, " ")
#define Str8_Space_Const  NEW_STRUCT(Str8){ .length=1, .chars=" " }

#define Slice_Empty       NewStr8(0, nullptr)
#define Slice_Empty_Const ((Slice)ZEROED)

#define Str16_Empty       NewStr16(0, nullptr)
#define Str16_Empty_Const ((Str16)ZEROED)

#define Str8Pair_Empty       NewStr8Pair(MyStr_Empty_Const, MyStr_Empty_Const)
#define Str8Pair_Empty_Const ((Str8Pair)ZEROED)

//NOTE: These are meant to be used when formatting Str8 using any printf like functions
//      Use the format specifier %.*s and then this macro in the var-args
#define StrPrint(string)   (int)(string).length, (string).chars
#define StrPntrPrint(strPntr) (int)(strPntr)->length, (strPntr)->chars

#define IsSliceFromStr(str, slice) ((slice).length == 0 || IsSizedPntrWithin((str).chars, (str).length, (slice).chars, (slice).length))

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

#if TARGET_IS_ORCA
PEXPI oc_str8 ToOcStr8(Str8 str) { return NEW_STRUCT(oc_str8){ .ptr = str.chars, .len = (size_t)str.length }; }
PEXPI Str8 ToStr8FromOc(oc_str8 orcaStr) { DebugAssert(orcaStr.len <= UINTXX_MAX); return NewStr8((uxx)orcaStr.len, orcaStr.ptr); }
#endif //TARGET_IS_ORCA

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
PEXP Str8 TrimLeadingWhitespaceAndNewLines(Str8 target)
{
	NotNullStr(target);
	Str8 result = target;
	while (result.length > 0)
	{
		if (IsCharWhitespace(result.chars[0], true))
		{
			result.length--;
			result.chars++;
		}
		else { break; }
	}
	return result;
}
PEXP Str8 TrimTrailingWhitespaceAndNewLines(Str8 target)
{
	NotNullStr(target);
	Str8 result = target;
	while (result.length > 0)
	{
		if (IsCharWhitespace(result.chars[result.length-1], true))
		{
			result.length--;
		}
		else { break; }
	}
	return result;
}
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

PEXPI Str8 TrimWhitespaceAndNewLines(Str8 target)
{
	NotNullStr(target);
	Str8 result = target;
	result = TrimLeadingWhitespaceAndNewLines(result);
	result = TrimTrailingWhitespaceAndNewLines(result);
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

PEXPI Str8 StrSlice(Str8 target, uxx startIndex, uxx endIndex)
{
	DebugAssert(startIndex <= target.length);
	DebugAssert(endIndex <= target.length);
	DebugAssert(startIndex <= endIndex);
	return NewStr8(endIndex - startIndex, target.chars + startIndex);
}
PEXPI Str8 StrSlicePntrs(Str8 target, const void* startPntr, const void* endPntr)
{
	Assert(IsPntrWithin(target.chars, target.length, startPntr));
	Assert(IsPntrWithin(target.chars, target.length, endPntr));
	return StrSlice(target, (uxx)(startPntr - target.chars), (uxx)(endPntr - target.chars));
}
PEXPI Str8 StrSliceFrom(Str8 target, uxx startIndex)
{
	return StrSlice(target, startIndex, target.length);
}
PEXPI Str8 StrSliceFromPntr(Str8 target, const void* startPntr)
{
	Assert(IsPntrWithin(target.chars, target.length, startPntr));
	return StrSliceFrom(target, (uxx)(startPntr - target.chars));
}
PEXPI Str8 StrSliceLength(Str8 target, uxx startIndex, uxx length)
{
	return StrSlice(target, startIndex, startIndex + length);
}
PEXPI Str8 StrSliceMaxLength(Str8 target, uxx startIndex, uxx maxLength)
{
	return StrSlice(target, startIndex, (startIndex + maxLength) < target.length ? (startIndex + maxLength) : target.length);
}

PEXPI bool StrExactEquals(Str8 left, Str8 right)
{
	if (left.length != right.length) { return false; }
	if (left.length == 0) { return true; }
	if (MyMemCompare(left.chars, right.chars, left.length) != 0) { return false; }
	return true;
}
PEXPI bool StrExactEqualsAt(Str8 left, Str8 right, uxx leftIndex)
{
	return StrExactEquals(StrSliceLength(left, leftIndex, right.length), right);
}
PEXPI bool StrExactStartsWith(Str8 target, Str8 prefix)
{
	if (target.length < prefix.length) { return false; }
	return StrExactEquals(StrSlice(target, 0, prefix.length), prefix);
}
PEXPI bool StrExactEndsWith(Str8 target, Str8 suffix)
{
	if (target.length < suffix.length) { return false; }
	return StrExactEquals(StrSlice(target, target.length - suffix.length, target.length), suffix);
}
PEXP bool StrExactContains(Str8 haystack, Str8 needle)
{
	Assert(needle.length > 0);
	if (haystack.length < needle.length) { return false; }
	for (uxx bIndex = 0; bIndex <= haystack.length - needle.length; bIndex++)
	{
		if (StrExactEqualsAt(haystack, needle, bIndex)) { return true; }
	}
	return false;
}
PEXP uxx StrExactFind(Str8 haystack, Str8 needle)
{
	Assert(needle.length > 0);
	if (haystack.length < needle.length) { return haystack.length; }
	for (uxx bIndex = 0; bIndex <= haystack.length - needle.length; bIndex++)
	{
		if (StrExactEqualsAt(haystack, needle, bIndex)) { return bIndex; }
	}
	return haystack.length;
}
PEXPI bool StrTryExactFind(Str8 haystack, Str8 needle, uxx* indexOut)
{
	uxx index = StrExactFind(haystack, needle);
	SetOptionalOutPntr(indexOut, index);
	return (index < haystack.length);
}

PEXP bool StrAnyCaseEquals(Str8 left, Str8 right)
{
	if (StrExactEquals(left, right)) { return true; }
	if (left.length != right.length) { return false; }
	for (uxx bIndex = 0; bIndex < left.length; bIndex++)
	{
		if (ToLowerChar(left.chars[bIndex]) != ToLowerChar(right.chars[bIndex])) { return false; }
	}
	return true;
}
PEXPI bool StrAnyCaseEqualsAt(Str8 left, Str8 right, uxx leftIndex)
{
	return StrAnyCaseEquals(StrSliceLength(left, leftIndex, right.length), right);
}
PEXPI bool StrAnyCaseStartsWith(Str8 target, Str8 prefix)
{
	if (StrExactStartsWith(target, prefix)) { return true; }
	if (target.length < prefix.length) { return false; }
	return StrAnyCaseEquals(StrSlice(target, 0, prefix.length), prefix);
}
PEXPI bool StrAnyCaseEndsWith(Str8 target, Str8 suffix)
{
	if (StrExactEndsWith(target, suffix)) { return true; }
	if (target.length < suffix.length) { return false; }
	return StrAnyCaseEquals(StrSliceFrom(target, target.length - suffix.length), suffix);
}
PEXPI bool StrAnyCaseContains(Str8 haystack, Str8 needle)
{
	Assert(needle.length > 0);
	if (StrExactContains(haystack, needle)) { return true; }
	if (needle.length > haystack.length) { return false; }
	for (uxx bIndex = 0; bIndex <= haystack.length - needle.length; bIndex++)
	{
		if (StrAnyCaseEqualsAt(haystack, needle, bIndex)) { return true; }
	}
	return false;
}
PEXPI uxx StrAnyCaseFind(Str8 haystack, Str8 needle)
{
	Assert(needle.length > 0);
	if (needle.length > haystack.length) { return haystack.length; }
	for (uxx bIndex = 0; bIndex <= haystack.length - needle.length; bIndex++)
	{
		if (StrAnyCaseEqualsAt(haystack, needle, bIndex)) { return bIndex; }
	}
	return haystack.length;
}
PEXPI bool StrTryAnyCaseFind(Str8 haystack, Str8 needle, uxx* indexOut)
{
	uxx index = StrAnyCaseFind(haystack, needle);
	SetOptionalOutPntr(indexOut, index);
	return (index < haystack.length);
}

PEXPI bool StrEquals(Str8 left, Str8 right, bool caseSensitive)
{
	return (caseSensitive ? StrExactEquals(left, right) : StrAnyCaseEquals(left, right));
}
PEXPI bool StrEqualsAt(Str8 left, Str8 right, uxx leftIndex, bool caseSensitive)
{
	return (caseSensitive ? StrExactEqualsAt(left, right, leftIndex) : StrAnyCaseEqualsAt(left, right, leftIndex));
}
PEXPI bool StrStartsWith(Str8 target, Str8 prefix, bool caseSensitive)
{
	return (caseSensitive ? StrExactStartsWith(target, prefix) : StrAnyCaseStartsWith(target, prefix));
}
PEXPI bool StrEndsWith(Str8 target, Str8 suffix, bool caseSensitive)
{
	return (caseSensitive ? StrExactEndsWith(target, suffix) : StrAnyCaseEndsWith(target, suffix));
}
PEXPI bool StrContains(Str8 haystack, Str8 needle, bool caseSensitive)
{
	return (caseSensitive ? StrExactContains(haystack, needle) : StrAnyCaseContains(haystack, needle));
}
PEXPI uxx StrFind(Str8 haystack, Str8 needle, bool caseSensitive)
{
	return (caseSensitive ? StrExactFind(haystack, needle) : StrAnyCaseFind(haystack, needle));
}
PEXPI bool StrTryFind(Str8 haystack, Str8 needle, bool caseSensitive, uxx* indexOut)
{
	uxx index = (caseSensitive ? StrExactFind(haystack, needle) : StrAnyCaseFind(haystack, needle));
	SetOptionalOutPntr(indexOut, index);
	return (index < haystack.length);
}



//TODO: Str8 CombineStrs(MemArena_t* memArena, Str8 str1, Str8 str2)
//TODO: Str8 CombineStrs(MemArena_t* memArena, Str8 str1, Str8 str2, Str8 str3)
//TODO: Str8 CombineStrs(MemArena_t* memArena, Str8 str1, Str8 str2, Str8 str3, Str8 str4)
//TODO: i32 StrCompareIgnoreCase(Str8 str1, Str8 str2, uxx compareLength)
//TODO: i32 StrCompareIgnoreCase(Str8 str1, Str8 str2)
//TODO: i32 StrCompareIgnoreCase(Str8 str1, const char* nullTermStr, uxx compareLength)
//TODO: i32 StrCompareIgnoreCase(Str8 str1, const char* nullTermStr)
//TODO: i32 StrCompareIgnoreCase(const char* str1, const char* str2, uxx compareLength)
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

#if defined(_STRUCT_STRING_H) && defined(_STRUCT_TYPED_ARRAY_H)
#include "cross/cross_string_and_typed_array.h"
#endif

#if defined(_MEM_ARENA_H) && defined(_STRUCT_STRING_H) && defined(_BASE_UNICODE_H)
#include "cross/cross_mem_arena_string_and_unicode.h"
#endif

#if defined(_MISC_HASH_H) && defined(_STRUCT_STRING_H)
#include "cross/cross_string_and_hash.h"
#endif
