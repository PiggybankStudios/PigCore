/*
File:   misc_printing.h
Author: Taylor Robbins
Date:   01\15\2025
Description:
	** Contains some helper functions related to printing (aka formatting strings with data)
*/

#ifndef _MISC_PRINTING_H
#define _MISC_PRINTING_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "std/std_includes.h"
#include "std/std_printf.h"
#include "struct/struct_string.h"
#include "mem/mem_arena.h"

//TODO: We should thoroughly test calling vsnprintf multiple times on a single va_list without va_start/va_end in-between
//      This seems to be working fine, but I seem to recall running into problems with it before
//      If it does work fine we should add some macros/functions that allow you do that easily to allocate a printed string into an Arena
//      NOTE: BuilderVanished pointed out this stackoverflow: https://stackoverflow.com/questions/55274350/can-i-use-va-list-twice-as-following

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	char* PrintInArena(Arena* arena, const char* formatString, ...);
	Str8 PrintInArenaStr(Arena* arena, const char* formatString, ...);
	int PrintVa_Measure(const char* formatString, va_list args);
	void PrintVa_Print(const char* formatString, va_list args, char* allocatedSpace, int previousResult);
	void TwoPassPrint(char* resultPntr, u64 resultLength, u64* currentByteIndex, const char* formatString, ...);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
//NOTE: This always adds a null-terminating character to the allocation
#define PrintInArenaVa(arena, resultName, resultLengthName, formatString)               \
char* resultName = nullptr;                                                             \
int resultLengthName = 0;                                                               \
va_list resultName##_args;                                                              \
do                                                                                      \
{                                                                                       \
	va_start(resultName##_args, formatString);                                          \
	resultLengthName = PrintVa_Measure((formatString), resultName##_args);              \
	va_end(resultName##_args);                                                          \
	if (resultLengthName >= 0)                                                          \
	{                                                                                   \
		resultName = (char*)AllocMem((arena), resultLengthName+1);                      \
		if (resultName == nullptr) { break; }                                           \
		va_start(resultName##_args, formatString);                                      \
		PrintVa_Print((formatString), resultName##_args, resultName, resultLengthName); \
		va_end(resultName##_args);                                                      \
	}                                                                                   \
}                                                                                       \
while(0)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

//NOTE: This returns a null-terminated string
PEXP char* PrintInArena(Arena* arena, const char* formatString, ...)
{
	NotNull(arena);
	NotNull(formatString);
	
	char* result = nullptr;
	va_list args;
	
	va_start(args, formatString);
	int length = MyVaListPrintf(result, 0, formatString, args); //Measure first
	Assert(length >= 0);
	va_end(args);
	
	result = (char*)AllocMem(arena, length+1); //Allocate
	if (result == nullptr) { return result; }
	
	va_start(args, formatString);
	MyVaListPrintf(result, (size_t)(length+1), formatString, args); //Real printf
	va_end(args);
	
	result[length] = '\0';
	
	return result;
}
//NOTE: This returns a null-terminated string
PEXP Str8 PrintInArenaStr(Arena* arena, const char* formatString, ...)
{
	NotNull(arena);
	NotNull(formatString);
	
	char* result = nullptr;
	va_list args;
	
	va_start(args, formatString);
	int length = MyVaListPrintf(result, 0, formatString, args); //Measure first
	Assert(length >= 0);
	va_end(args);
	
	result = (char*)AllocMem(arena, length+1); //Allocate
	if (result == nullptr) { return Str8_Empty; }
	
	va_start(args, formatString);
	MyVaListPrintf(result, (size_t)(length+1), formatString, args); //Real printf
	va_end(args);
	
	result[length] = '\0';
	
	return NewStr8((uxx)length, result);
}

PEXP int PrintVa_Measure(const char* formatString, va_list args)
{
	int result = MyVaListPrintf(nullptr, 0, formatString, args);
	return result;
}
//NOTE: This returns a null-terminated string
PEXP void PrintVa_Print(const char* formatString, va_list args, char* allocatedSpace, int previousResult)
{
	Assert(previousResult >= 0);
	NotNull(allocatedSpace);
	int printResult = MyVaListPrintf(allocatedSpace, previousResult+1, formatString, args);
	Assert(printResult == previousResult);
	allocatedSpace[previousResult] = '\0';
}

// Sometimes we want to do a single memory allocation for a collection of strings
// TwoPassPrint is meant to be put inside a loop that runs twice, first pass it only
// does measurements of how many chars are needed for each print, and second pass it
// expects an allocation to have been made and it will start putting the actual
// formatted strings into the allocated buffer
// Example:
// 	Str8 result = Str8_Empty;
// 	for (u8 pass = 0; pass < 2; pass++)
// 	{
// 		u64 charIndex = 0;
// 		VarArrayLoop(&stringArray, sIndex)
// 		{
// 			VarArrayLoopGet(Str8, str, &stringArray, sIndex);
// 			TwoPassPrint(result.chars, result.length, &charIndex, "%s%.*s", (sIndex > 0) ? "-" : "", StrPrint(str));
// 		}
// 		if (pass == 0) { result.chars = (char*)AllocMem(memArena, charIndex+1); result.length = charIndex; }
// 		else { Assert(charIndex == result.length); result.chars[result.length] = '\0'; }
// 	}
PEXP void TwoPassPrint(char* resultPntr, u64 resultLength, u64* currentByteIndex, const char* formatString, ...)
{
	Assert(resultPntr == nullptr || resultLength > 0);
	NotNull(currentByteIndex);
	NotNull(formatString);
	
	u64 printSize = 0;
	va_list args;
	
	va_start(args, formatString);
	int printResult = PrintVa_Measure(formatString, args);
	va_end(args);
	
	if (printResult >= 0)
	{
		printSize = (u64)printResult;
		if (resultPntr != nullptr)
		{
			Assert(*currentByteIndex <= resultLength);
			u64 spaceLeft = resultLength - *currentByteIndex;
			Assert(printSize <= spaceLeft);
			va_start(args, formatString);
			PrintVa_Print(formatString, args, &resultPntr[*currentByteIndex], printResult);
			va_end(args);
		}
	}
	else
	{
		//Print error. Use the formatString as a stand-in to indicate an error has occurred in the print formatting
		printSize = MyStrLength64(formatString);
		if (resultPntr != nullptr)
		{
			Assert(*currentByteIndex <= resultLength);
			u64 spaceLeft = resultLength - *currentByteIndex;
			Assert(printSize <= spaceLeft);
			MyMemCopy(&resultPntr[*currentByteIndex], formatString, printSize);
		}
	}
	
	*currentByteIndex += printSize;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _MISC_PRINTING_H

#if defined(_MEM_SCRATCH_H) && defined(_MISC_PRINTING_H)
#include "cross/cross_scratch_and_printing.h"
#endif
