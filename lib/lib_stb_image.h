/*
File:   lib_stb_image.h
Author: Taylor Robbins
Date:   10\27\2025
*/

#ifndef _LIB_STB_IMAGE_H
#define _LIB_STB_IMAGE_H

#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "std/std_malloc.h"
#include "mem/mem_arena.h"

//TODO: stb_image.h uses strtol which we currently don't have an implementation for in our custom standard library!
#if USING_CUSTOM_STDLIB
#define PIG_CORE_TRY_PARSE_IMAGE_AVAILABLE 0
#else
#define PIG_CORE_TRY_PARSE_IMAGE_AVAILABLE 1
#endif

#if PIG_CORE_TRY_PARSE_IMAGE_AVAILABLE

#if PIG_CORE_IMPLEMENTATION
THREAD_LOCAL Arena* StbImageArena = nullptr;
#else
extern THREAD_LOCAL Arena* StbImageArena;
#endif

#if PIG_CORE_IMPLEMENTATION
static void* StbImageMalloc(size_t numBytes)
{
	if (StbImageArena != nullptr)
	{
		return AllocMem(StbImageArena, (uxx)numBytes);
	}
	else { return MyMalloc(numBytes); }
}
static void* StbImageRealloc(void* allocPntr, size_t oldNumBytes, size_t newNumBytes)
{
	if (StbImageArena != nullptr)
	{
		if (allocPntr == nullptr)
		{
			return AllocMem(StbImageArena, (uxx)newNumBytes);
		}
		else
		{
			return ReallocMem(StbImageArena, allocPntr, (uxx)oldNumBytes, (uxx)newNumBytes);
		}
	}
	else { return MyRealloc(allocPntr, newNumBytes); }
}
static void StbImageFree(void* allocPntr)
{
	if (StbImageArena != nullptr)
	{
		if (CanArenaFree(StbImageArena))
		{
			bool arenaAllowedFreeWithoutSize = IsFlagSet(StbImageArena->flags, ArenaFlag_AllowFreeWithoutSize);
			FlagUnset(StbImageArena->flags, ArenaFlag_AllowFreeWithoutSize);
			FreeMem(StbImageArena, allocPntr, 0);
			FlagSetTo(StbImageArena->flags, ArenaFlag_AllowFreeWithoutSize, arenaAllowedFreeWithoutSize);
		}
	}
	else { MyFree(allocPntr); }
}
#endif //PIG_CORE_IMPLEMENTATION

#define STBI_NO_STDIO
// #define STB_IMAGE_STATIC
#define STBI_ASSERT(condition) Assert(condition)
#define STBI_MALLOC(numBytes)                                   StbImageMalloc(numBytes)
#define STBI_REALLOC_SIZED(allocPntr, oldNumBytes, newNumBytes) StbImageRealloc((allocPntr), (oldNumBytes), (newNumBytes))
#define STBI_FREE(allocPntr)                                    StbImageFree(allocPntr)
#if COMPILER_IS_MSVC
#pragma warning(push)
#pragma warning(disable: 5262) //implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases
#endif
#if (COMPILER_IS_CLANG || COMPILER_IS_EMSCRIPTEN)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough" //warning: unannotated fall-through between switch labels
#endif

#if PIG_CORE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "third_party/stb/stb_image.h"

#if COMPILER_IS_MSVC
#pragma warning(pop)
#endif
#if (COMPILER_IS_CLANG || COMPILER_IS_EMSCRIPTEN)
#pragma clang diagnostic pop
#endif

#endif //PIG_CORE_TRY_PARSE_IMAGE_AVAILABLE

#endif //  _LIB_STB_IMAGE_H
