/*
File:   cross_mem_arena_and_image_data.h
Author: Taylor Robbins
Date:   11\05\2025
*/

#ifndef _CROSS_MEM_ARENA_AND_IMAGE_DATA_H
#define _CROSS_MEM_ARENA_AND_IMAGE_DATA_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeImageData(Arena* arena, ImageData* imageData);
	PIG_CORE_INLINE ImageData NewImageDataInArena(Arena* arena, v2i size);
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

PEXPI ImageData NewImageDataInArena(Arena* arena, v2i size)
{
	NotNull(arena);
	ImageData result = ZEROED;
	if (size.Width <= 0 || size.Height <= 0) { return result; }
	result.size = size;
	result.numPixels = (uxx)(size.Width * size.Height);
	result.pixels = AllocArray(u32, arena, result.numPixels);
	NotNull(result.pixels);
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_MEM_ARENA_AND_IMAGE_DATA_H
