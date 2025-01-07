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

#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"

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
	union { wchar_t* chars; u16* words; void* pntr; };
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
// |                       Basic String API                       |
// +--------------------------------------------------------------+
Str8 StrNt(const char* nullTermStr)
{
	#if DEBUG_BUILD
	Str8 result = ZEROED;
	#else
	Str8 result;
	#endif
	result.length = (nullTermStr != nullptr) ? (uxx)MyStrLength64(nullTermStr) : 0;
	result.chars = (char*)nullTermStr; //throw away const qualifier
	return result;
}
Str8 NewStr8(uxx length, const void* pntr)
{
	#if DEBUG_BUILD
	Str8 result = ZEROED;
	#else
	Str8 result;
	#endif
	result.length = length;
	result.pntr = (void*)pntr; //throw away const qualifier
	return result;
}

Str16 NewStr16(uxx length, const void* pntr)
{
	#if DEBUG_BUILD
	Str16 result = ZEROED;
	#else
	Str16 result;
	#endif
	result.length = length;
	result.pntr = (void*)pntr; //throw away const qualifier
	return result;
}

Str8Pair NewStr8Pair(Str8 left, Str8 right)
{
	#if DEBUG_BUILD
	Str8Pair result = ZEROED;
	#else
	Str8Pair result;
	#endif
	result.left = left;
	result.right = right;
	return result;
}

#define Str8_Empty       NewStr8(0, nullptr)
#define Str8_Empty_Const ((Str8)ZEROED)

#define Str16_Empty       NewStr16(0, nullptr)
#define Str16_Empty_Const ((Str16)ZEROED)

#define Str8Pair_Empty       NewStr8Pair(MyStr_Empty_Const, MyStr_Empty_Const)
#define Str8Pair_Empty_Const ((Str8Pair)ZEROED)

bool IsEmptyStr(Str8 string) { return (string.length == 0); }
bool IsNullStr(Str8 string) { return (string.length > 0 && string.pntr == nullptr); }
bool IsNullTerminated(Str8 string) { return (string.pntr != nullptr && string.chars[string.length] == '\0'); }
bool IsBufferNullTerminated(uxx bufferSize, const void* bufferPntr)
{
	if (bufferPntr == nullptr) { return false; }
	for (uxx bIndex = 0; bIndex < bufferSize; bIndex++) { if (((char*)bufferPntr)[bIndex] == '\0') { return true; } }
	return false;
}

//NOTE: Before we changed the u64 length in Str8 to uxx type, printing a string
//      with %.*s was one of the scenarios where 32-bit platforms would cause us
//      trouble. So this macro used to sneak in a u32 cast for those platforms.
//      Now it simply serves as a shorthand for passing the length and then chars
//      in the right order, and also will help us fix up this use case in the future
//      if we find any more idiosyncrasies with this format specifier
#define StrPrint(myStrStruct)   (myStrStruct).length, (myStrStruct).chars
#define StrPntrPrint(myStrPntr) (myStrPntr)->length, (myStrPntr)->chars

// +--------------------------------------------------------------+
// |                      Assertion Helpers                       |
// +--------------------------------------------------------------+
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

#endif //  _STRUCT_STRING_H

#if defined(_MEM_ARENA_H) && defined(_STRUCT_STRING_H)
#include "cross/cross_mem_arena_and_string.h"
#endif

#if defined(_STRUCT_STRING_H) && defined(_BASE_UNICODE_H)
#include "cross/cross_string_and_unicode.h"
#endif
