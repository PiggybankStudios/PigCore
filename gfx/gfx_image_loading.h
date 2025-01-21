/*
File:   gfx_image_loading.h
Author: Taylor Robbins
Date:   01\19\2025
Description:
	** Contains functions that help us load various kinds of image files from disk
	** and parse them (using stb_image.h for PNGs for now)
*/

#ifndef _GFX_IMAGE_LOADING_H
#define _GFX_IMAGE_LOADING_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "struct/struct_vectors.h"
#include "misc/misc_result.h"
#include "mem/mem_scratch.h"

typedef struct ImageData ImageData;
struct ImageData
{
	v2i size;
	uxx numPixels;
	u32* pixels;
};

#if PIG_CORE_IMPLEMENTATION
thread_local Arena* StbImageScratchArena = nullptr;
#else
extern thread_local Arena* StbImageScratchArena;
#endif

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	#if !USING_CUSTOM_STDLIB
	Result TryParseImageFile(Slice fileContents, Arena* arena, ImageData* imageDataOut);
	#endif
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

//TODO: stb_image.h uses strtol which we currently don't have an implementation for in our custom standard library!
#if !USING_CUSTOM_STDLIB

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

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STB_IMAGE_STATIC
#define STBI_ASSERT(condition) Assert(condition)
#define STBI_MALLOC(numBytes)                                   StbImageMalloc(numBytes)
#define STBI_REALLOC_SIZED(allocPntr, oldNumBytes, newNumBytes) StbImageRealloc((allocPntr), (oldNumBytes), (newNumBytes))
#define STBI_FREE(allocPntr)                                    StbImageFree(allocPntr)
#if COMPILER_IS_MSVC
#pragma warning(push)
#pragma warning(disable: 5262) //implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases
#endif
#if COMPILER_IS_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough" //warning: unannotated fall-through between switch labels
#endif

#include "third_party/stb/stb_image.h"

#if COMPILER_IS_MSVC
#pragma warning(pop)
#endif
#if COMPILER_IS_CLANG
#pragma clang diagnostic pop
#endif

PEXP Result TryParseImageFile(Slice fileContents, Arena* arena, ImageData* imageDataOut)
{
	NotNullStr(fileContents);
	NotNull(imageDataOut);
	ScratchBegin1(scratch, arena);
	StbImageScratchArena = scratch;
	int imageWidth, imageHeight, fileChannelCount;
	const int numChannels = 4;
	u8* imageData = stbi_load_from_memory(fileContents.bytes, (int)fileContents.length, &imageWidth, &imageHeight, &fileChannelCount, numChannels);
	
	if (imageData != nullptr)
	{
		Assert(imageWidth > 0 && imageHeight > 0);
		Assert(fileChannelCount == numChannels);
		PrintLine_D("imageData: %p %dx%d", imageData, imageWidth, imageHeight);
		
		ClearPointer(imageDataOut);
		imageDataOut->size = NewV2i(imageWidth, imageHeight);
		imageDataOut->numPixels = (uxx)(imageWidth * imageHeight);
		imageDataOut->pixels = AllocArray(u32, arena, imageDataOut->numPixels);
		if (imageDataOut->pixels == nullptr)
		{
			return Result_FailedToAllocateMemory;
		}
		MyMemCopy(imageDataOut->pixels, imageData, sizeof(u32) * imageDataOut->numPixels);
		
		stbi_image_free(imageData);
	}
	else
	{
		ScratchEnd(scratch);
		StbImageScratchArena = nullptr;
		return Result_ParsingFailure;
	}
	
	ScratchEnd(scratch);
	StbImageScratchArena = nullptr;
	
	return Result_Success;
}

#endif //!USING_CUSTOM_STDLIB

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _GFX_IMAGE_LOADING_H
