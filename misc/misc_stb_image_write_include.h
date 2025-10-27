/*
File:   misc_stb_image_write_include.h
Author: Taylor Robbins
Date:   10\27\2025
*/

#ifndef _MISC_STB_IMAGE_WRITE_INCLUDE_H
#define _MISC_STB_IMAGE_WRITE_INCLUDE_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "std/std_malloc.h"
#include "mem/mem_arena.h"

#if PIG_CORE_IMPLEMENTATION
THREAD_LOCAL Arena* StbImageWriteArena = nullptr;
#else
extern THREAD_LOCAL Arena* StbImageWriteArena;
#endif

#if PIG_CORE_IMPLEMENTATION
static void* StbImageWriteMalloc(size_t numBytes)
{
	if (StbImageWriteArena != nullptr) { return AllocMem(StbImageWriteArena, (uxx)numBytes); }
	else { return MyMalloc(numBytes); }
}
static void* StbImageWriteRealloc(void* allocPntr, size_t oldNumBytes, size_t newNumBytes)
{
	if (StbImageWriteArena != nullptr)
	{
		if (allocPntr == nullptr) { return AllocMem(StbImageWriteArena, (uxx)newNumBytes); }
		else { return ReallocMem(StbImageWriteArena, allocPntr, (uxx)oldNumBytes, (uxx)newNumBytes); }
	}
	else { return MyRealloc(allocPntr, newNumBytes); }
}
static void StbImageWriteFree(void* allocPntr)
{
	if (StbImageWriteArena != nullptr)
	{
		if (CanArenaFree(StbImageWriteArena))
		{
			bool arenaAllowedFreeWithoutSize = IsFlagSet(StbImageWriteArena->flags, ArenaFlag_AllowFreeWithoutSize);
			FlagUnset(StbImageWriteArena->flags, ArenaFlag_AllowFreeWithoutSize);
			FreeMem(StbImageWriteArena, allocPntr, 0);
			FlagSetTo(StbImageWriteArena->flags, ArenaFlag_AllowFreeWithoutSize, arenaAllowedFreeWithoutSize);
		}
	}
	else { MyFree(allocPntr); }
}
#endif //PIG_CORE_IMPLEMENTATION

#define STBI_WRITE_NO_STDIO
#define STBIW_ASSERT(condition) Assert(condition)
#define STBIW_MALLOC(numBytes)                           StbImageWriteMalloc(numBytes)
#define STBIW_REALLOC_SIZED(allocPntr, oldSize, newSize) StbImageWriteRealloc((allocPntr), (oldSize), (newSize))
#define STBIW_FREE(allocPntr)                            StbImageWriteFree(allocPntr)
#define STBIW_MEMMOVE(dest, source, length) MyMemMove((dest), (source), (length))
// #define STBIW_ZLIB_COMPRESS

#if PIG_CORE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include "third_party/stb/stb_image_write.h"

#endif //  _MISC_STB_IMAGE_WRITE_INCLUDE_H
