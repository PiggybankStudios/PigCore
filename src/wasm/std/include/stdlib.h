/*
File:   stdlib.h
Author: Taylor Robbins
Date:   01\10\2025
*/

#ifndef _STDLIB_H
#define _STDLIB_H

#include <internal/wasm_std_common.h>
#include <stdint.h>

CONDITIONAL_EXTERN_C_START

// Implementations in wasm_std_misc.c
int abs(int value);
_Noreturn void exit(int exitCode);
_Noreturn void abort();
//NOTE: This is not a standard function but we want to pass a message to jsStdAbort so we added this
_Noreturn void abort_msg(const char* message);

// Implementations in wasm_std_malloc.c
void* malloc(size_t numBytes);
void* calloc(size_t numElements, size_t elemSize);
void* realloc(void* prevAllocPntr, size_t newSize);
void free(void* allocPntr);
void* aligned_alloc(size_t numBytes, size_t alignmentSize);

// TODO: double atof(const char* str);
// TODO: void* alloca(size_t numBytes);

// typedef int (StdCompareFunc_f)(const void* left, const void* right);
// typedef int (StdCompareFuncEx_f)(const void* left, const void* right, void* compareFunc);
//TODO: void qsort(void* basePntr, size_t numItems, size_t itemSize, StdCompareFunc_f* compareFunc);

CONDITIONAL_EXTERN_C_END

#endif //  _STDLIB_H
