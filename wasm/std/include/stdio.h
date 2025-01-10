/*
File:   stdio.h
Author: Taylor Robbins
Date:   01\10\2025
*/

#ifndef _STDIO_H
#define _STDIO_H

#include <internal/wasm_std_common.h>

#include <stdint.h>
#include <stdarg.h>

CONDITIONAL_EXTERN_C_START

// Implementations in wasm_std_stdio.c
int vprintf(const char* formatStr, va_list args);
// TODO: int vfprintf(FILE*, const char*, va_list args);
int vsprintf(char* bufferPntr, const char* formatStr, va_list args);
int vsnprintf(char* bufferPntr, size_t bufferSize, const char* formatStr, va_list args);

int printf(const char* formatStr, ...);
// TODO: int fprintf(FILE*, const char*, ...);
int sprintf(char* bufferPntr, const char* formatStr, ...);
int snprintf(char* bufferPntr, size_t bufferSize, const char* formatStr, ...);

CONDITIONAL_EXTERN_C_END

#endif //  _STDIO_H
