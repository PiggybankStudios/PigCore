/*
File:   std_memset.h
Author: Taylor Robbins
Date:   01\02\2025
Description:
	** Contains routing aliases for memset, memcmp, memcpy, memmove, strcpy, strcmp, strncmp, strlen, wcslen, and strstr
*/

#ifndef _STD_MEMSET_H
#define _STD_MEMSET_H

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "std/std_includes.h"

#define MyMemSet(dest, value, length)      memset((dest), (value), (length))
#define MyMemCompare(pntr1, pntr2, length) memcmp((pntr1), (pntr2), (length))
#define MyMemEquals(pntr1, pntr2, length)  (MyMemCompare((pntr1), (pntr2), (length)) == 0)
#define MyMemCopy(dest, source, length)    memcpy((dest), (source), (length))
#define MyMemMove(dest, source, length)    memmove((dest), (source), (length))
#define MyStrCopyNt(dest, source)          strcpy((dest), (source))
#define MyStrCompareNt(str1, str2)         strcmp((str1), (str2))
#define MyStrCompare(str1, str2, length)   strncmp((str1), (str2), (length))
#define MyStrLength(str)                   strlen(str)
#define MyStrLength32(str)                 ((u32)strlen(str))
#define MyStrLength64(str)                 ((u64)strlen(str))
#if TARGET_IS_WINDOWS
	#define MyWideStrLength(str)           wcslen((const wchar_t*)str)
	#define MyWideStrLength32(str)         ((u32)wcslen(str))
	#define MyWideStrLength64(str)         ((u64)wcslen(str))
#else
	#if !PIG_CORE_IMPLEMENTATION
	size_t MyWideStrLength(const char16_t* str);
	#else
	PEXPI size_t MyWideStrLength(const char16_t* str) { size_t result = 0; while (str[result] != 0) { result++; } return result; }
	#endif
	#define MyWideStrLength32(str) ((u32)MyWideStrLength(str))
	#define MyWideStrLength64(str) ((u64)MyWideStrLength(str))
#endif
#define MyStrStrNt(str1, str2)             strstr((str1), (str2))

// +--------------------------------------------------------------+
// |                      Convenience Macros                      |
// +--------------------------------------------------------------+
#define ClearArray(Array)      MyMemSet((Array), '\0', sizeof((Array)))
#define ClearStruct(Structure) MyMemSet(&(Structure), '\0', sizeof((Structure)))
#define ClearPointer(Pointer)  MyMemSet((Pointer), '\0', sizeof(*(Pointer)));

#define SwapPntrVars(varType, pntrVar1, pntrVar2) do                              \
{                                                                                 \
	varType tempVarWithLongNameThatWontConflict;                                  \
	MyMemCopy(&tempVarWithLongNameThatWontConflict, (pntrVar2), sizeof(varType)); \
	MyMemCopy((pntrVar2), (pntrVar1), sizeof(varType));                           \
	MyMemCopy((pntrVar1), &tempVarWithLongNameThatWontConflict, sizeof(varType)); \
	varType* tempPntrWithLongNameThatWontConflict;                                \
	tempPntrWithLongNameThatWontConflict = (pntrVar2);                            \
	(pntrVar2) = (pntrVar1);                                                      \
	(pntrVar1) = tempPntrWithLongNameThatWontConflict;                            \
} while(0)

#define SwapPntrs(varType, pntr1, pntr2) do                                    \
{                                                                              \
	varType tempVarWithLongNameThatWontConflict;                               \
	MyMemCopy(&tempVarWithLongNameThatWontConflict, (pntr2), sizeof(varType)); \
	MyMemCopy((pntr2), (pntr1), sizeof(varType));                              \
	MyMemCopy((pntr1), &tempVarWithLongNameThatWontConflict, sizeof(varType)); \
} while(0)

#define SwapValues(type, variable1, variable2) do           \
{                                                           \
	type tempVarWithLongNameThatWontConflict = (variable1); \
	(variable1) = (variable2);                              \
	variable2 = tempVarWithLongNameThatWontConflict;        \
} while (0)

#endif //  _STD_MEMSET_H
