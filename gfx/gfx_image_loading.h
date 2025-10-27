/*
File:   gfx_image_loading.h
Author: Taylor Robbins
Date:   01\19\2025
Description:
	** Contains functions that help us load various kinds of image files from disk and parse them
	** We use stb_image.h so we support loading: .jpeg, .png, .bmp, .gif, .tga, and more
*/

//TODO: We should probably move this file to file_fmt folder!

#ifndef _GFX_IMAGE_LOADING_H
#define _GFX_IMAGE_LOADING_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "struct/struct_vectors.h"
#include "misc/misc_result.h"
#include "mem/mem_scratch.h"
#include "struct/struct_image_data.h"
#include "misc/misc_profiling_tracy_include.h"
#include "misc/misc_stb_image_include.h"

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	#if PIG_CORE_TRY_PARSE_IMAGE_AVAILABLE
	Result TryParseImageFile(Slice fileContents, Arena* arena, ImageData* imageDataOut);
	#endif
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

#if PIG_CORE_TRY_PARSE_IMAGE_AVAILABLE

PEXP Result TryParseImageFile(Slice fileContents, Arena* arena, ImageData* imageDataOut)
{
	TracyCZoneN(funcZone, "TryParseImageFile", true);
	NotNullStr(fileContents);
	NotNull(imageDataOut);
	ScratchBegin1(scratch, arena);
	StbImageArena = scratch;
	int imageWidth, imageHeight, fileChannelCount;
	const int numChannels = 4;
	TracyCZoneN(_stbLoadFromMemory, "stbi_load_from_memory", true);
	u8* imageData = stbi_load_from_memory(fileContents.bytes, (int)fileContents.length, &imageWidth, &imageHeight, &fileChannelCount, numChannels);
	TracyCZoneEnd(_stbLoadFromMemory);
	
	if (imageData != nullptr)
	{
		Assert(imageWidth > 0 && imageHeight > 0);
		// PrintLine_D("imageData: %p %dx%d (%llu pixels)", imageData, imageWidth, imageHeight, (uxx)(imageWidth * imageHeight));
		
		ClearPointer(imageDataOut);
		imageDataOut->size = NewV2i(imageWidth, imageHeight);
		imageDataOut->numPixels = (uxx)(imageWidth * imageHeight);
		imageDataOut->pixels = AllocArray(u32, arena, imageDataOut->numPixels);
		if (imageDataOut->pixels == nullptr)
		{
			ScratchEnd(scratch);
			TracyCZoneEnd(funcZone);
			return Result_FailedToAllocateMemory;
		}
		TracyCZoneN(_CopyPixels, "CopyPixels", true);
		MyMemCopy(imageDataOut->pixels, imageData, sizeof(u32) * imageDataOut->numPixels);
		TracyCZoneEnd(_CopyPixels);
		
		TracyCZoneN(_stbImageFree, "stbi_image_free", true);
		stbi_image_free(imageData);
		TracyCZoneEnd(_stbImageFree);
	}
	else
	{
		ScratchEnd(scratch);
		StbImageArena = nullptr;
		TracyCZoneEnd(funcZone);
		return Result_ParsingFailure;
	}
	
	ScratchEnd(scratch);
	StbImageArena = nullptr;
	
	TracyCZoneEnd(funcZone);
	return Result_Success;
}

#endif //PIG_CORE_TRY_PARSE_IMAGE_AVAILABLE

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _GFX_IMAGE_LOADING_H

#if defined(_GFX_IMAGE_LOADING_H) && defined(_OS_FILE_H)
#include "cross/cross_image_loading_and_file.h"
#endif
