/*
File:   parse_metadesk_include.h
Author: Taylor Robbins
Date:   03\31\2025
*/

#ifndef _PARSE_METADESK_INCLUDE_H
#define _PARSE_METADESK_INCLUDE_H

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "os/os_virtual_mem.h"
#include "os/os_file.h"
#include "struct/struct_string.h"

#if BUILD_WITH_METADESK

#define MD_FUNCTION PEXP
#define MD_GLOBAL static //TODO: Can't expose globals as extern because they are not declared in md.h

#define MD_DEFAULT_BASIC_TYPES 0
typedef i8  MD_i8;
typedef i16 MD_i16;
typedef i32 MD_i32;
typedef i64 MD_i64;
typedef u8  MD_u8;
typedef u16 MD_u16;
typedef u32 MD_u32;
typedef u64 MD_u64;
typedef r32 MD_f32;
typedef r64 MD_f64;

#define MD_Assert(expression) Assert(expression)
#define MD_ArrayCount(array) ArrayCount(array)
#define MD_Min(a, b) (((a) < (b)) ? (a) : (b))
#define MD_Max(a, b) (((a) > (b)) ? (a) : (b))
#define MD_ClampBot(a, b) MD_Max((a), (b))
#define MD_ClampTop(a, b) MD_Min((a), (b))

#define MD_DEFAULT_MEMSET 0
#define MD_IMPL_Memset MyMemSet
#define MD_IMPL_Memmove MyMemMove

#define MD_DEFAULT_FILE_LOAD 0
#define MD_IMPL_LoadEntireFile MdLoadEntireFile

#define MD_DEFAULT_MEMORY 0
#define MD_IMPL_Reserve  MdMemReserve
#define MD_IMPL_Commit   MdMemCommit
#define MD_IMPL_Decommit MdMemDecommit
#define MD_IMPL_Release  MdMemRelease

#define MD_DEFAULT_SPRINTF 0
#define MD_IMPL_Vsnprintf MyVaListPrintf

#define MD_DISABLE_PRINT_HELPERS 1

// TODO: #define MD_DEFAULT_FILE_ITER 0
// TODO: #define MD_DEFAULT_ARENA 0
// TODO: #define MD_DEFAULT_SCRATCH 0

#if COMPILER_IS_MSVC
#pragma warning(push)
#pragma warning(disable: 5262) //implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases
#endif
#if COMPILER_IS_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough" //warning: unannotated fall-through between switch labels
#endif

#include "third_party/metadesk/md.h"

MD_String8 MdLoadEntireFile(MD_Arena* arena, MD_String8 filePath);
void* MdMemReserve(MD_u64 numBytes);
MD_b32 MdMemCommit(void* memoryPntr, MD_u64 numBytes);
void MdMemDecommit(void* memoryPntr, MD_u64 numBytes);
void MdMemRelease(void* memoryPntr, MD_u64 numBytes);

#if PIG_CORE_IMPLEMENTATION
#include "third_party/metadesk/md.c"
#endif

#if COMPILER_IS_MSVC
#pragma warning(pop)
#endif
#if COMPILER_IS_CLANG
#pragma clang diagnostic pop
#endif

#if PIG_CORE_IMPLEMENTATION

thread_local MD_Arena* mdArenaForCallback = nullptr; //TODO: Once we convert Metadesk to use our own Arena* then we won't have to do these shenanigans with thread_local to help our Arena* accepting functions allocate from MD_Arena
// +==============================+
// |     MdAllocFuncCallback      |
// +==============================+
// void* MdAllocCallback(uxx numBytes)
ALLOC_FUNC_DEF(MdAllocCallback)
{
	NotNull(mdArenaForCallback);
	return MD_ArenaPush(mdArenaForCallback, (u64)numBytes);
}

MD_String8 MdLoadEntireFile(MD_Arena* arena, MD_String8 filePath)
{
	mdArenaForCallback = arena;
	Arena callbackArena = ZEROED;
	InitArenaFuncs(&callbackArena, MdAllocCallback, nullptr, nullptr);
	Str8 resultStr = Str8_Empty;
	bool readSuccess = OsReadTextFile(NewStr8((uxx)filePath.size, filePath.str), &callbackArena, &resultStr);
	mdArenaForCallback = nullptr;
	if (readSuccess) { return MD_S8(resultStr.bytes, resultStr.length); }
	else { return MD_S8Lit(""); }
}

void* MdMemReserve(MD_u64 numBytes)
{
	DebugAssert(numBytes <= UINTXX_MAX);
	return OsReserveMemory((uxx)numBytes);
}
MD_b32 MdMemCommit(void* memoryPntr, MD_u64 numBytes)
{
	OsCommitReservedMemory(memoryPntr, (uxx)numBytes);
	return true;
}
void MdMemDecommit(void* memoryPntr, MD_u64 numBytes)
{
	OsDecommitReservedMemory(memoryPntr, (uxx)numBytes);
}
void MdMemRelease(void* memoryPntr, MD_u64 numBytes)
{
	#if TARGET_IS_WINDOWS
	//NOTE: Passing numBytes causes VirtualFree to return ERROR_INVALID_PARAMETER! Apparently if all the pages were decommitted then maybe we HAVE to pass 0 for dwSize?
	numBytes = 0;
	#endif
	OsFreeReservedMemory(memoryPntr, numBytes);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_METADESK

#endif //  _PARSE_METADESK_INCLUDE_H
