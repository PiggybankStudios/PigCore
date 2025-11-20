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
	
	** Naming: Each sub-image in the atlas is called a "cell", the 2D coordinate of that cell is called cellPos
	**   In previous incarnations we called these "frames" which has a good correllation when using the sprite sheet
	**   as a backing format for animations but we use sprite sheets for things besides just animations.
	**   The words "sprite" and "sheet" are not particularly meaningful but they are descriptive and unique
	**   enough and I've been using this term for a while so it'll stay for now.
*/

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
#include "base/base_debug_output.h"
#include "std/std_memset.h"
#include "os/os_path.h"
#include "struct/struct_string.h"
#include "struct/struct_vectors.h"
#include "struct/struct_rectangles.h"
#include "struct/struct_var_array.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "parse/parse_simple_parsers.h"
#include "gfx/gfx_texture.h"
#include "gfx/gfx_image_loading.h"

#if BUILD_WITH_SOKOL_GFX

typedef plex SpriteSheetCell SpriteSheetCell;
plex SpriteSheetCell
{
	v2i cellPos;
	Str8 name;
};

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
	VarArray cells;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeSpriteSheet(SpriteSheet* sheet);
	PIG_CORE_INLINE SpriteSheetCell* TryFindSheetCell(SpriteSheet* sheet, Str8 cellName);
	Result TryParseSpriteSheetMeta(SpriteSheet* sheet, Str8 metaFileContents, bool* foundGridSizeOut);
	SpriteSheet InitSpriteSheetSized(Arena* arena, Str8 debugName, Str8 filePath, Slice imageFileContents, Str8 metaFileContents, v2i gridSize);
	PIG_CORE_INLINE SpriteSheet InitSpriteSheet(Arena* arena, Str8 debugName, Str8 filePath, Slice imageFileContents, Str8 metaFileContents);
	PIG_CORE_INLINE reci GetSheetCellReci(const SpriteSheet* sheet, v2i cellPos);
	PIG_CORE_INLINE rec GetSheetCellRec(const SpriteSheet* sheet, v2i cellPos);
	PIG_CORE_INLINE reci GetNamedSheetCellReci(const SpriteSheet* sheet, Str8 cellName);
	PIG_CORE_INLINE rec GetNamedSheetCellRec(const SpriteSheet* sheet, Str8 cellName);
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
		VarArrayLoop(&sheet->cells, cIndex)
		{
			VarArrayLoopGet(SpriteSheetCell, cell, &sheet->cells, cIndex);
			FreeStr8(sheet->arena, &cell->name);
		}
		FreeVarArray(&sheet->cells);
	}
	ClearPointer(sheet);
}

PEXPI SpriteSheetCell* TryFindSheetCell(SpriteSheet* sheet, Str8 cellName)
{
	NotNull(sheet);
	NotNull(sheet->arena);
	VarArrayLoop(&sheet->cells, cIndex)
	{
		VarArrayLoopGet(SpriteSheetCell, cell, &sheet->cells, cIndex);
		if (StrExactEquals(cell->name, cellName)) { return cell; }
	}
	return nullptr;
}

PEXP Result TryParseSpriteSheetMeta(SpriteSheet* sheet, Str8 metaFileContents, bool* foundGridSizeOut)
{
	NotNull(sheet);
	NotNull(sheet->arena);
	Result result = Result_None;
	SetOptionalOutPntr(foundGridSizeOut, false);
	
	bool foundFilePrefix = false;
	SpriteSheetCell* currentFrame = nullptr;
	
	TextParser parser = MakeTextParser(metaFileContents);
	ParsingToken token = ZEROED;
	while (TextParserGetToken(&parser, &token) && result == Result_None)
	{
		if (token.type == ParsingTokenType_FilePrefix)
		{
			if (foundFilePrefix) { PrintLine_E("Duplicate file header in spriteSheet .meta file: \"%.*s\"", StrPrint(token.str)); result = Result_Duplicate; break; }
			if (!StrExactEquals(token.value, StrLit("Sprite Meta"))) { PrintLine_E("Invalid file header in spriteSheet .meta file: \"%.*s\" (Should be \"Sprite Meta\")", StrPrint(token.value)); result = Result_MissingHeader; break; }
			foundFilePrefix = true;
			continue;
		}
		else if (!foundFilePrefix && token.type != ParsingTokenType_Comment)
		{
			WriteLine_E("Missing file header in spriteSheet .meta file!");
			result = Result_MissingFileHeader;
			break;
		}
		
		switch (token.type)
		{
			case ParsingTokenType_Comment: /* do nothing */ break;
			
			case ParsingTokenType_KeyValuePair:
			{
				if (currentFrame != nullptr)
				{
					if (StrAnyCaseEquals(token.key, StrLit("Name")))
					{
						if (!IsEmptyStr(currentFrame->name)) { PrintLine_E("Duplicate name given for cell (%d, %d) in spriteSheet .meta file!", currentFrame->cellPos.X, currentFrame->cellPos.Y); result = Result_Duplicate; break; }
						currentFrame->name = AllocStr8(sheet->arena, token.value);
					}
					else { PrintLine_W("Unknown key in spriteSheet .meta file: \"%.*s\"", StrPrint(token.key)); }
				}
				else
				{
					if (StrAnyCaseEquals(token.key, StrLit("Size")) || StrAnyCaseEquals(token.key, StrLit("GridSize")))
					{
						v2i gridSize = V2i_Zero;
						Result parseV2Error = Result_None;
						if (TryParseV2i(token.value, &gridSize, &parseV2Error))
						{
							sheet->gridSize = gridSize;
							SetOptionalOutPntr(foundGridSizeOut, true);
						}
						else
						{
							PrintLine_E("Failed to parse %.*s as v2i %s: \"%.*s\"", StrPrint(token.key), GetResultStr(parseV2Error), StrPrint(token.value));
							result = parseV2Error;
							break;
						}
					}
					else { PrintLine_W("Unknown key in spriteSheet .meta file: \"%.*s\"", StrPrint(token.key)); }
				}
			} break;
			
			case ParsingTokenType_None:
			{
				v2i cellPos = V2i_Zero;
				if (TryParseV2i(token.str, &cellPos, nullptr))
				{
					currentFrame = VarArrayAdd(SpriteSheetCell, &sheet->cells);
					NotNull(currentFrame);
					ClearPointer(currentFrame);
					currentFrame->cellPos = cellPos;
				}
				else
				{
					PrintLine_E("Unknown token in spriteSheet .meta file: \"%.*s\"", StrPrint(token.str));
				}
			} break;
			
			default:
			{
				PrintLine_E("Unhandled token type %s in spriteSheet .meta file: \"%.*s\"", GetParsingTokenTypeStr(token.type), StrPrint(token.str));
			} break;
		}
	}
	
	if (!foundFilePrefix) { WriteLine_W("SpriteSheet .meta file is empty or missing file header"); }
	if (result == Result_None) { result = Result_Success; }
	return result;
}

PEXP SpriteSheet InitSpriteSheetSized(Arena* arena, Str8 debugName, Str8 filePath, Slice imageFileContents, Str8 metaFileContents, v2i gridSize)
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
	
	result.arena = arena;
	InitVarArray(SpriteSheetCell, &result.cells, arena);
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
	
	do
	{
		bool foundGridSize = !AreEqualV2i(gridSize, FillV2i(-1));
		v2i actualGridSize = gridSize;
		
		if (IsEmptyStr(metaFileContents))
		{
			//TODO: Look for special png block containing the meta info?
		}
		
		if (!IsEmptyStr(metaFileContents))
		{
			bool foundGridSizeInMetaFile = false;
			Result parseMetaError = TryParseSpriteSheetMeta(&result, metaFileContents, &foundGridSizeInMetaFile);
			if (parseMetaError == Result_Success)
			{
				if (foundGridSizeInMetaFile)
				{
					if (foundGridSize) { WriteLine_W("GridSize is defined in sprite sheet .meta file AND given by calling code!"); }
					foundGridSize = true;
					actualGridSize = result.gridSize;
				}
			}
			else
			{
				result.error = parseMetaError;
				break;
			}
		}
		
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
				v2i parsedGridSize = V2i_Zero;
				if (TryParseI32(widthStr, &parsedGridSize.Width, nullptr) &&
					TryParseI32(heightStr, &parsedGridSize.Height, nullptr))
				{
					actualGridSize = parsedGridSize;
					foundGridSize = true;
				}
			}
		}
		
		if (!foundGridSize)
		{
			DebugAssertMsg(foundGridSize, "Failed to find gridSize for SpriteSheet! This is either defined in the file name or in an accompanying .meta file!");
			result.error = Result_MissingData;
			break;
		}
		
		result.gridSize = actualGridSize;
		result.cellSize = DivV2i(result.texture.size, result.gridSize);
		if (result.cellSize.Width <= 0 || result.cellSize.Height <= 0)
		{
			DebugAssertMsg(false, "Invalid gridSize for texture size!");
			result.error = Result_ValueTooHigh;
			break;
		}
		
		result.error = Result_Success;
	} while(0);
	
	ScratchEnd(scratch);
	if (result.error != Result_Success)
	{
		Result error = result.error;
		FreeSpriteSheet(&result);
		result.error = error;
	}
	return result;
}
PEXPI SpriteSheet InitSpriteSheet(Arena* arena, Str8 debugName, Str8 filePath, Slice imageFileContents, Str8 metaFileContents)
{
	return InitSpriteSheetSized(arena, debugName, filePath, imageFileContents, metaFileContents, FillV2i(-1));
}

PEXPI reci GetSheetCellReci(const SpriteSheet* sheet, v2i cellPos)
{
	return MakeReciV(MulV2i(sheet->cellSize, cellPos), sheet->cellSize);
}
PEXPI rec GetSheetCellRec(const SpriteSheet* sheet, v2i cellPos)
{
	return MakeRecV(ToV2Fromi(MulV2i(sheet->cellSize, cellPos)), ToV2Fromi(sheet->cellSize));
}

PEXPI reci GetNamedSheetCellReci(const SpriteSheet* sheet, Str8 cellName)
{
	const SpriteSheetCell* cell = TryFindSheetCell((SpriteSheet*)sheet, cellName);
	if (cell == nullptr) { return Reci_Zero; }
	return MakeReciV(MulV2i(sheet->cellSize, cell->cellPos), sheet->cellSize);
}
PEXPI rec GetNamedSheetCellRec(const SpriteSheet* sheet, Str8 cellName)
{
	SpriteSheetCell* cell = TryFindSheetCell((SpriteSheet*)sheet, cellName);
	if (cell == nullptr) { return Rec_Zero; }
	return MakeRecV(ToV2Fromi(MulV2i(sheet->cellSize, cell->cellPos)), ToV2Fromi(sheet->cellSize));
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _FILE_FMT_SPRITE_SHEET_H

#if defined(_OS_FILE_H) && defined(_FILE_FMT_SPRITE_SHEET_H)
#include "cross/cross_file_and_sprite_sheet.h"
#endif
