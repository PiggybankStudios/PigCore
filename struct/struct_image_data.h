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
#include "struct/struct_vectors.h"

typedef plex ImageData ImageData;
plex ImageData
{
	v2i size;
	uxx numPixels;
	u32* pixels;
};
#define MakeImageDataEx(sizeV2i, numPixelsValue, pixelsPntr) NEW_STRUCT(ImageData){ .size=(sizeV2i), .numPixels=(numPixelsValue), .pixels=(pixelsPntr) }
#define MakeImageData(sizeV2i, pixelsPntr) NEW_STRUCT(ImageData){ .size=(sizeV2i), .numPixels=(uxx)((sizeV2i).Width * (sizeV2i).Height), .pixels=(pixelsPntr) }

#endif //  _STRUCT_IMAGE_DATA_H

#if defined(_MEM_ARENA_H) && defined(_STRUCT_IMAGE_DATA_H)
#include "cross/cross_mem_arena_and_image_data.h"
#endif
