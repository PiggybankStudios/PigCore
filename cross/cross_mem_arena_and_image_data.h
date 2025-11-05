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

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_MEM_ARENA_AND_IMAGE_DATA_H
