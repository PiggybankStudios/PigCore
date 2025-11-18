/*
File:   file_fmt_sprite_sheet.h
Author: Taylor Robbins
Date:   11\18\2025
Description:
	** A "sprite sheet" is a single image that contains many sub-images arranged in a regular grid.
	** Basically it's a subset of an atlas (which can be any packing of sub-images into a single image)
	** This isn't a terribly complex concept but it's used very often in 2D games
	** and it's nice to have a standardized way to talk about and load these atlases
	** The size of the sprite sheet grid is either defined by the name of the file ending with "wxh",
	** or by a .meta file with the same name being placed next to the image,
	** or embedded into the .png file as a non-standard segment (TODO: Segment name?)
*/

//TODO: Parse .meta file for gridSize
//TODO: Parse .meta file for cell names
//TODO: Track filled cells?
//TODO: Can we generate mipmaps for sprite sheets? We could maybe do proper cell-edge restriction on blending?

//TODO: Make a simple tool to embed metadata in .png
//TODO: Parse embedded metadata in PNG block

#ifndef _FILE_FMT_SPRITE_SHEET_H
#define _FILE_FMT_SPRITE_SHEET_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "os/os_path.h"
#include "struct/struct_string.h"
#include "struct/struct_vectors.h"
#include "struct/struct_rectangles.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "parse/parse_simple_parsers.h"
#include "gfx/gfx_texture.h"
#include "gfx/gfx_image_loading.h"

#if BUILD_WITH_SOKOL_GFX

typedef plex SpriteSheet SpriteSheet;
plex SpriteSheet
{
	Arena* arena;
	Result error;
	#if DEBUG_BUILD
	Str8 name;
	#endif
	union
	{
		v2i gridSize;
		plex { i32 gridWidth, gridHeight; };
	};
	union
	{
		v2i cellSize;
		plex { i32 cellWidth, cellHeight; };
	};
	Texture texture;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeSpriteSheet(SpriteSheet* sheet);
	SpriteSheet InitSpriteSheet(Arena* arena, Str8 debugName, Str8 filePath, Slice imageFileContents, Slice metaFileContents);
	PIG_CORE_INLINE reci GetCellFrameReci(const SpriteSheet* sheet, v2i cellPos);
	PIG_CORE_INLINE rec GetCellFrameRec(const SpriteSheet* sheet, v2i cellPos);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void FreeSpriteSheet(SpriteSheet* sheet)
{
	NotNull(sheet);
	if (sheet->arena != nullptr)
	{
		#if DEBUG_BUILD
		FreeStr8(sheet->arena, &sheet->name);
		#endif
		FreeTexture(&sheet->texture);
	}
	ClearPointer(sheet);
}

PEXP SpriteSheet InitSpriteSheet(Arena* arena, Str8 debugName, Str8 filePath, Slice imageFileContents, Slice metaFileContents)
{
	NotNull(arena);
	ScratchBegin1(scratch, arena);
	SpriteSheet result = ZEROED;
	
	ImageData imageData = ZEROED;
	Result parseResult = TryParseImageFile(imageFileContents, scratch, &imageData);
	if (parseResult != Result_Success)
	{
		result.error = parseResult;
		ScratchEnd(scratch);
		return result;
	}
	
	result.texture = InitTexture(arena, debugName, imageData.size, imageData.pixels, TextureFlag_IsPixelated|TextureFlag_NoMipmaps);
	if (result.texture.error != Result_Success)
	{
		result.error = result.texture.error;
		ScratchEnd(scratch);
		return result;
	}
	
	bool foundGridSize = false;
	v2i gridSize = V2i_Zero_Const;
	
	if (!IsEmptyStr(metaFileContents))
	{
		//TODO: Parse the meta file
	}
	
	//TODO: Look for special png block containing the meta info?
	
	if (!foundGridSize)
	{
		Str8 fileName = GetFileNamePart(filePath, false);
		Str8 sizePart = StrSliceFrom(fileName, fileName.length);
		uxx xCharIndex = fileName.length;
		while(sizePart.length < fileName.length)
		{
			char nextChar = fileName.chars[fileName.length-1 - sizePart.length];
			if (IsCharNumeric(nextChar))
			{
				sizePart.chars--;
				sizePart.length++;
			}
			else if (xCharIndex == fileName.length && (nextChar == 'x' || nextChar == 'X'))
			{
				xCharIndex = fileName.length-1 - sizePart.length;
				sizePart.chars--;
				sizePart.length++;
			}
			else { break; }
		}
		
		if (xCharIndex < fileName.length && sizePart.length >= 3 && xCharIndex > fileName.length - sizePart.length)
		{
			xCharIndex -= (fileName.length - sizePart.length);
			Str8 widthStr = StrSlice(sizePart, 0, xCharIndex);
			Str8 heightStr = StrSliceFrom(sizePart, xCharIndex+1);
			Result parseError = Result_None;
			if (TryParseI32(widthStr, &gridSize.Width, nullptr) && TryParseI32(heightStr, &gridSize.Height, nullptr))
			{
				foundGridSize = true;
			}
		}
	}
	
	if (!foundGridSize)
	{
		DebugAssertMsg(foundGridSize, "Failed to find gridSize for SpriteSheet! This is either defined in the file name or in an accompanying .meta file!");
		FreeTexture(&result.texture);
		ScratchEnd(scratch);
		result.error = Result_MissingData;
		return result;
	}
	
	result.gridSize = gridSize;
	result.cellSize = DivV2i(result.texture.size, result.gridSize);
	if (result.cellSize.Width <= 0 || result.cellSize.Height <= 0)
	{
		DebugAssertMsg(false, "Invalid gridSize for texture size!");
		FreeTexture(&result.texture);
		ScratchEnd(scratch);
		result.error = Result_ValueTooHigh;
		return result;
	}
	
	result.error = Result_Success;
	result.arena = arena;
	#if DEBUG_BUILD
	NotNullStr(debugName);
	if (!IsEmptyStr(debugName))
	{
		result.name = AllocStr8(arena, debugName);
		NotNull(result.name.chars);
	}
	#else
	UNUSED(debugName);
	#endif
	
	ScratchEnd(scratch);
	return result;
}

PEXPI reci GetCellFrameReci(const SpriteSheet* sheet, v2i cellPos)
{
	return MakeReciV(MulV2i(sheet->cellSize, cellPos), sheet->cellSize);
}
PEXPI rec GetCellFrameRec(const SpriteSheet* sheet, v2i cellPos)
{
	return MakeRecV(ToV2Fromi(MulV2i(sheet->cellSize, cellPos)), ToV2Fromi(sheet->cellSize));
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _FILE_FMT_SPRITE_SHEET_H

#if defined(_OS_FILE_H) && defined(_FILE_FMT_SPRITE_SHEET_H)
#include "cross/cross_file_and_sprite_sheet.h"
#endif
