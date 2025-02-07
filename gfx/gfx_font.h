/*
File:   gfx_font.h
Author: Taylor Robbins
Date:   02\05\2025
Description:
	** A "Font" is a data structure built on top of possibly multiple Texture atlases
	** which are packed with rasterized glyphs by stb_truetype.h
*/

#ifndef _GFX_FONT_H
#define _GFX_FONT_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "std/std_basic_math.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "struct/struct_string.h"
#include "struct/struct_var_array.h"
#include "gfx/gfx_texture.h"

#if PIG_CORE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#endif
#define STBTT_STATIC
#define STBTT_ifloor(x)                     FloorR32i(x)
#define STBTT_iceil(x)                      CeilR32i(x)
#define STBTT_sqrt(x)                       SqrtR32(x)
#define STBTT_pow(x, exp)                   PowR32((x), (exp))
#define STBTT_fabs(x)                       AbsR32(x)
#define STBTT_malloc(numBytes, user)        AllocMem((Arena*)(user), (numBytes))
#define STBTT_free(pointer, user)           do { if ((pointer) != nullptr) { FreeMem((Arena*)(user), (pointer), 0) } } while(0)
#define STBTT_assert(expession)             Assert(expession)
#define STBTT_strlen(str)                   (uxx)MyStrLength64(str)
#define STBTT_memcpy(dest, source, length)  MyMemCopy((dest), (source), (length))
#define STBTT_memset(dest, value, length)   MyMemSet((dest), (value), (length))
#include "third_party/stb/stb_truetype.h"

typedef enum FontStyleFlag FontStyleFlag;
enum FontStyleFlag
{
	FontStyleFlag_None          = 0x00,
	FontStyleFlag_Bold          = 0x01,
	FontStyleFlag_Italic        = 0x02,
	FontStyleFlag_Inverted      = 0x04,
	FontStyleFlag_Underline     = 0x08,
	FontStyleFlag_Strikethrough = 0x10,
	FontStyleFlag_Outline       = 0x20,
	FontStyleFlag_All           = 0x3F,
};

typedef struct FontCharRange FontCharRange;
struct FontCharRange
{
	u32 startCodepoint;
	u32 endCodepoint;
	uxx glyphArrayStartIndex;
};

typedef struct FontGlyph FontGlyph;
struct FontGlyph
{
	u32 codepoint;
	reci atlasSourceRec;
	r32 advanceX;
	v2 renderOffset;
	rec logicalRec;
};

typedef struct FontAtlas FontAtlas;
struct FontAtlas
{
	r32 fontSize;
	u8 fontStyleFlags;
	VarArray charRanges;
	FontCharRange glyphRange;
	VarArray glyphs;
	Texture texture;
};

typedef struct Font Font;
struct Font
{
	Arena* arena;
	Str8 name;
	Slice ttfFile;
	VarArray atlases; //FontAtlas
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void FreeFont(Font* font);
	void InitFont(Font* font, Arena* arena);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void FreeFont(Font* font)
{
	NotNull(font);
	if (font->arena != nullptr)
	{
		FreeStr8(font->arena, &font->name);
		FreeStr8(font->arena, &font->ttfFile);
		VarArrayLoop(&font->atlases, aIndex)
		{
			VarArrayLoopGet(FontAtlas, atlas, &font->atlases, aIndex);
			FreeVarArray(&atlas->charRanges);
			FreeVarArray(&atlas->glyphs);
			FreeTexture(&atlas->texture);
		}
		FreeVarArray(atlases);
	}
	ClearPointer(font);
}

PEXP void InitFont(Font* font, Arena* arena)
{
	NotNull(font);
	NotNull(arena);
	ClearPointer(font);
	font->arena = arena;
	InitVarArray(FontAtlas, &font->atlases, arena);
}

PEXP void AttachTtfFileToFont(Font* font, Slice ttfFileContents)
{
	NotNull(font);
	NotNullStr(ttfFileContents);
	NotEmptyStr(ttfFileContents);
	FreeStr8(font->arena, &font->ttfFile);
	font->ttfFile.length = ttfFileContents.length;
	font->ttfFile.chars = AllocMem(font->arena, ttfFileContents.length);
	NotNull(font->ttfFile.chars);
	MyMemCopy(font->ttfFile.chars, ttfFileContents.chars, ttfFileContents.length);
}

PEXP Result BakeFontAtlas(Font* font, r32 fontSize, u8 fontStyleFlags, v2i atlasSize)
{
	NotNull(font);
	NotNull(font->arena);
	Assert(atlasSize.Width > 0 && atlasSize.Height > 0);
	ScratchBegin1(scratch, font->arena);
	
	uxx pixelSize = sizeof(u8);
	uxx numPixels = (uxx)(atlasSize.Width * atlasSize.Height);
	u32* pixelsPntr = AllocArray(u32, font->arena, numPixels);
	NotNull(pixelsPntr);
	MyMemSet(pixelsPntr, 0x00, pixelSize * numPixels);
	
	stbtt_pack_context packContext = ZEROED;
	int beginResult = stbtt_PackBegin(
		&packContext, //context
		pixelsPntr, //pixels
		(int)atlasSize.Width, (int)atlasSize.Height, //width, height
		(int)(atlasSize.Width * pixelSize), //stride_in_bytes
		1, //padding (between chars)
		scratch //alloc_context
	);
	
	ScratchEnd(scratch);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _GFX_FONT_H
