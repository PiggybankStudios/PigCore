/*
File:   string.h
Author: Taylor Robbins
Date:   01\10\2025
*/

#ifndef _STRING_H
#define _STRING_H

#include <internal/wasm_std_common.h>

CONDITIONAL_EXTERN_C_START

void* _memset(void* pntr, int value, size_t numBytes);
#define memset(pntr, value, numBytes) _memset((pntr), (value), (numBytes))
int memcmp(const void* left, const void* right, size_t numBytes);
void* _memcpy(void* dest, const void* source, size_t numBytes);
#define memcpy(dest, source, numBytes) _memcpy((dest), (source), (numBytes))
void* _memmove(void* dest, const void* source, size_t numBytes);
#define memmove(dest, source, numBytes) _memmove((dest), (source), (numBytes))
char* strcpy(char* dest, const char* source);
// TODO: char* strstr(const char* haystack, const char* needle);

int strcmp(const char* left, const char* right);
int strncmp(const char* left, const char* right, size_t numBytes);
size_t strlen(const char* str);
// TODO: size_t wcslen(const wchar_t* str);

//TODO: char *strchr (const char *, int); //NEEDED for slre.c
//TODO: char *strrchr (const char *, int); //NEEDED for slre.c

CONDITIONAL_EXTERN_C_END

#endif //  _STRING_H
