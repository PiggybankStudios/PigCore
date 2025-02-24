/*
File:   struct_image_data.h
Author: Taylor Robbins
Date:   02\15\2025
*/

#ifndef _STRUCT_IMAGE_DATA_H
#define _STRUCT_IMAGE_DATA_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
#include "struct/struct_vectors.h"

typedef struct ImageData ImageData;
struct ImageData
{
	v2i size;
	uxx numPixels;
	u32* pixels;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeImageData(Arena* arena, ImageData* imageData);
	PIG_CORE_INLINE ImageData NewImageData(v2i size, u32* pixels);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void FreeImageData(Arena* arena, ImageData* imageData)
{
	NotNull(arena);
	NotNull(imageData);
	if (imageData->pixels != nullptr)
	{
		FreeArray(u32, arena, imageData->numPixels, imageData->pixels);
	}
	ClearPointer(imageData);
}

PEXPI ImageData NewImageData(v2i size, u32* pixels)
{
	ImageData result = ZEROED;
	result.size = size;
	result.numPixels = (uxx)(size.Width * size.Height);
	result.pixels = pixels;
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_IMAGE_DATA_H
