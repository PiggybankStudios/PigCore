/*
File:   misc_two_pass.h
Author: Taylor Robbins
Date:   08\07\2025
Description:
	** A common pattern we use is to do two passes over some logic to first measure
	** the amount of space something is going to take, allocate the appropriate space,
	** and then perform another pass where we fill out the allocated space.
	** Often this takes the form: for (u8 pass = 0; pass < 2; pass++)
	** with logic inside the loop that wants to always do the measurement (bumping of an index)
	** and only on the second pass does the filling. To make that code a little more
	** readable we have a set of macros here that help do that conditional filling
	** (also some Asserts) for various kinds of common operations we do.
	** This approach also allows us to do measurement by only doing the first pass and returning early.
	** Usually this is indicated by passing nullptr for the Arena parameter which
	** causes the code to return an improper string with a non-zero length with nullptr chars
	** Example:
	**  TwoPassStr8Loop(result, memArena, true)
	**  {
	**  	VarArrayLoop(&stringArray, sIndex)
	**  	{
	**  		VarArrayLoopGet(Str8, str, &stringArray, sIndex);
	**  		if (sIndex > 0) { TwoPassChar(&result, '-'); }
	**  		TwoPassStr(&result, str);
	**  	}
	**  	TwoPassStr8LoopEnd(&result);
	**  }
	**  return result.str;
*/

#ifndef _MISC_TWO_PASS_H
#define _MISC_TWO_PASS_H

#include "base/base_defines_check.h"
#include "base/base_macros.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "mem/mem_arena.h"
#include "struct/struct_string.h"
#include "misc/misc_printing.h"

typedef plex TwoPassStr8 TwoPassStr8;
plex TwoPassStr8
{
	Arena* arena;
	bool addNullTerm;
	Str8 str;
	u8 pass;
	uxx index; //reset on each pass, tracks current calculated size or current write index into str.chars
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE TwoPassStr8 NewTwoPassStr8(Arena* arena, bool addNullTerm);
	void TwoPassPrint(TwoPassStr8* twoPassStrPntr, const char* formatString, ...);
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define TwoPassStr8_Empty       NewTwoPassStr8(nullptr, false)
#define TwoPassStr8_Empty_Const { .arena=nullptr, .addNullTerm=false, .str=Str8_Empty_Const, .pass=0, .index=0 }

#define TwoPassStr8Loop(structName, arenaPntr, addNullTerm) TwoPassStr8 structName = NewTwoPassStr8((arenaPntr), (addNullTerm)); \
	for (structName.pass = 0, structName.index = 0; structName.pass < 2; structName.pass++, structName.index = 0)

#define TwoPassStr8LoopEnd(twoPassStrPntr) do {                                                                                                            \
	if ((twoPassStrPntr)->pass == 0)                                                                                                                       \
	{                                                                                                                                                      \
		(twoPassStrPntr)->str.length = (twoPassStrPntr)->index;                                                                                            \
		if ((twoPassStrPntr)->arena == nullptr || ((twoPassStrPntr)->index == 0 && !(twoPassStrPntr)->addNullTerm)) { (twoPassStrPntr)->pass = 1; break; } \
		(twoPassStrPntr)->str.chars = (char*)AllocMem((twoPassStrPntr)->arena, (twoPassStrPntr)->index + ((twoPassStrPntr)->addNullTerm ? 1 : 0));         \
		AssertMsg((twoPassStrPntr)->str.chars != nullptr, "Failed to allocate chars in TwoPassStr8LoopEnd");                                               \
	}                                                                                                                                                      \
	else                                                                                                                                                   \
	{                                                                                                                                                      \
		AssertMsg((twoPassStrPntr)->index == (twoPassStrPntr)->str.length, "Second pass came up short in TwoPassStr8LoopEnd");                             \
		if ((twoPassStrPntr)->addNullTerm) { (twoPassStrPntr)->str.chars[(twoPassStrPntr)->str.length] = '\0'; }                                           \
	}                                                                                                                                                      \
} while(0)


#define TwoPassChar(twoPassStrPntr, character) do {                                                                       \
	if ((twoPassStrPntr)->pass == 1)                                                                                      \
	{                                                                                                                     \
		AssertMsg((twoPassStrPntr)->index + 1 <= (twoPassStrPntr)->str.length, "Second pass discrepency in TwoPassChar"); \
		(twoPassStrPntr)->str.chars[(twoPassStrPntr)->index] = (character);                                               \
	}                                                                                                                     \
	(twoPassStrPntr)->index += 1;                                                                                         \
} while(0)

#define TwoPassStrNt(twoPassStrPntr, nullTermStr) do {                                                                             \
	uxx strLength = (uxx)MyStrLength(nullTermStr);                                                                                 \
	if ((twoPassStrPntr)->pass == 1 && strLength > 0)                                                                              \
	{                                                                                                                              \
		AssertMsg((twoPassStrPntr)->index + strLength <= (twoPassStrPntr)->str.length, "Second pass discrepency in TwoPassStrNt"); \
		MyMemCopy(&(twoPassStrPntr)->str.chars[(twoPassStrPntr)->index], (nullTermStr), strLength);                                \
	}                                                                                                                              \
	(twoPassStrPntr)->index += strLength;                                                                                          \
} while(0)

#define TwoPassStr(twoPassStrPntr, str8) do {                                                                                        \
	if ((twoPassStrPntr)->pass == 1 && (str8).length > 0)                                                                            \
	{                                                                                                                                \
		AssertMsg((twoPassStrPntr)->index + (str8).length <= (twoPassStrPntr)->str.length, "Second pass discrepency in TwoPassStr"); \
		MyMemCopy(&(twoPassStrPntr)->str.chars[(twoPassStrPntr)->index], (str8).chars, (str8).length);                               \
	}                                                                                                                                \
	(twoPassStrPntr)->index += (str8).length;                                                                                        \
} while(0)

#define TwoPassBytes(twoPassStrPntr, numBytes, bytesPntr) do {                                                                      \
	if ((twoPassStrPntr)->pass == 1 && (numBytes) > 0)                                                                              \
	{                                                                                                                               \
		AssertMsg((twoPassStrPntr)->index + (numBytes) <= (twoPassStrPntr)->str.length, "Second pass discrepency in TwoPassBytes"); \
		MyMemCopy(&(twoPassStrPntr)->str.chars[(twoPassStrPntr)->index], (bytesPntr), (numBytes));                                  \
	}                                                                                                                               \
	(twoPassStrPntr)->index += (numBytes);                                                                                          \
} while(0)

//NOTE: When calling functions that do TwoPass internally from within another function that does TwoPass
#define CreateTwoPassInnerArena(twoPassStrPntr, arenaPntrName) Arena arenaPntrName##_Local; Arena* arenaPntrName; do \
{                                                                                                                    \
	arenaPntrName = ((twoPassStrPntr)->pass == 1) ? &(arenaPntrName##_Local) : nullptr;                              \
	if (arenaPntrName != nullptr)                                                                                    \
	{                                                                                                                \
		InitArenaBuffer(arenaPntrName,                                                                               \
			&(twoPassStrPntr)->str.chars[(twoPassStrPntr)->index],                                                   \
			(twoPassStrPntr)->str.length - (twoPassStrPntr)->index                                                   \
		);                                                                                                           \
		FlagSet(arenaPntrName->flags, ArenaFlag_SingleAlloc);                                                        \
	}                                                                                                                \
} while(0)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI TwoPassStr8 NewTwoPassStr8(Arena* arena, bool addNullTerm)
{
	TwoPassStr8 result = TwoPassStr8_Empty_Const;
	result.arena = arena;
	result.addNullTerm = addNullTerm;
	return result;
}

// Sometimes we want to do a single memory allocation for a collection of strings
// TwoPassPrint is meant to be put inside a loop that runs twice, first pass it only
// does measurements of how many chars are needed for each print, and second pass it
// expects an allocation to have been made and it will start putting the actual
// formatted strings into the allocated buffer
PEXP void TwoPassPrint(TwoPassStr8* twoPassStrPntr, const char* formatString, ...)
{
	NotNull(twoPassStrPntr);
	Assert(twoPassStrPntr->str.chars == nullptr || twoPassStrPntr->str.length > 0);
	NotNull(formatString);
	
	u64 printSize = 0;
	va_list args;
	
	va_start(args, formatString);
	int printResult = PrintVa_Measure(formatString, args);
	va_end(args);
	
	if (printResult >= 0)
	{
		printSize = (u64)printResult;
		if (twoPassStrPntr->pass == 1)
		{
			Assert(twoPassStrPntr->index <= twoPassStrPntr->str.length);
			u64 spaceLeft = twoPassStrPntr->str.length - twoPassStrPntr->index;
			Assert(printSize <= spaceLeft);
			va_start(args, formatString);
			PrintVa_Print(formatString, args, &twoPassStrPntr->str.chars[twoPassStrPntr->index], printResult);
			va_end(args);
		}
	}
	else
	{
		//Print error. Use the formatString as a stand-in to indicate an error has occurred in the print formatting
		printSize = MyStrLength64(formatString);
		if (twoPassStrPntr->pass == 1)
		{
			Assert(twoPassStrPntr->index <= twoPassStrPntr->str.length);
			u64 spaceLeft = twoPassStrPntr->str.length - twoPassStrPntr->index;
			Assert(printSize <= spaceLeft);
			MyMemCopy(&twoPassStrPntr->str.chars[twoPassStrPntr->index], formatString, printSize);
		}
	}
	
	twoPassStrPntr->index += printSize;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _MISC_TWO_PASS_H
