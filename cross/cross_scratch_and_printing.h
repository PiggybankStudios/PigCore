/*
File:   cross_scratch_and_printing.h
Author: Taylor Robbins
Date:   02\17\2025
*/

#ifndef _CROSS_SCRATCH_AND_PRINTING_H
#define _CROSS_SCRATCH_AND_PRINTING_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Str8 ScratchPrintStr(const char* formatString, ...);
	PIG_CORE_INLINE char* ScratchPrint(const char* formatString, ...);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

//Always returns a null-terminated string
PEXPI Str8 ScratchPrintStr(const char* formatString, ...)
{
	NotNull(formatString);
	ScratchBegin(scratch);
	
	char* result = nullptr;
	va_list args;
	
	va_start(args, formatString);
	int length = MyVaListPrintf(result, 0, formatString, args); //Measure first
	AssertMsg(length >= 0, "ScratchPrintStr format failed!");
	va_end(args);
	
	result = (char*)AllocMem(scratch, length+1); //Allocate
	if (result == nullptr) { return Str8_Empty; }
	
	va_start(args, formatString);
	MyVaListPrintf(result, (size_t)(length+1), formatString, args); //Real printf
	va_end(args);
	
	result[length] = '\0';
	
	//Intentionally no ScratchEnd
	return MakeStr8((uxx)length, result);
}

PEXPI char* ScratchPrint(const char* formatString, ...)
{
	NotNull(formatString);
	ScratchBegin(scratch);
	
	char* result = nullptr;
	va_list args;
	
	va_start(args, formatString);
	int length = MyVaListPrintf(result, 0, formatString, args); //Measure first
	AssertMsg(length >= 0, "ScratchPrint format failed!");
	va_end(args);
	
	result = (char*)AllocMem(scratch, length+1); //Allocate
	if (result == nullptr) { return nullptr; }
	
	va_start(args, formatString);
	MyVaListPrintf(result, (size_t)(length+1), formatString, args); //Real printf
	va_end(args);
	
	result[length] = '\0';
	
	//Intentionally no ScratchEnd
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_SCRATCH_AND_PRINTING_H
