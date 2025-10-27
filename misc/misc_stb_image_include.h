/*
File:   misc_stb_image_include.h
Author: Taylor Robbins
Date:   10\27\2025
*/

#ifndef _MISC_STB_IMAGE_INCLUDE_H
#define _MISC_STB_IMAGE_INCLUDE_H

#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"

//TODO: stb_image.h uses strtol which we currently don't have an implementation for in our custom standard library!
#if USING_CUSTOM_STDLIB
#define PIG_CORE_TRY_PARSE_IMAGE_AVAILABLE 0
#else
#define PIG_CORE_TRY_PARSE_IMAGE_AVAILABLE 1
#endif

#if PIG_CORE_TRY_PARSE_IMAGE_AVAILABLE

#if PIG_CORE_IMPLEMENTATION
THREAD_LOCAL Arena* StbImageScratchArena = nullptr;
#else
extern THREAD_LOCAL Arena* StbImageScratchArena;
#endif

#if PIG_CORE_IMPLEMENTATION
static void* StbImageMalloc(size_t numBytes)
{
	NotNull(StbImageScratchArena);
	return AllocMem(StbImageScratchArena, (uxx)numBytes);
}
static void* StbImageRealloc(void* allocPntr, size_t oldNumBytes, size_t newNumBytes)
{
	NotNull(StbImageScratchArena);
	if (allocPntr == nullptr)
	{
		return AllocMem(StbImageScratchArena, (uxx)newNumBytes);
	}
	else
	{
		return ReallocMem(StbImageScratchArena, allocPntr, (uxx)oldNumBytes, (uxx)newNumBytes);
	}
}
static void StbImageFree(void* allocPntr)
{
	NotNull(StbImageScratchArena);
	UNUSED(allocPntr);
	//NOTE: We don't need to call FreeMem since we are allocating from a Stack type arena (the scratch arenas)
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

#endif //  _MISC_STB_IMAGE_INCLUDE_H
