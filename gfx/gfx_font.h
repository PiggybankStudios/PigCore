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
#define STBTT_free(pointer, user)           do { if ((pointer) != nullptr && CanArenaFree((Arena*)(user))) { FreeMem((Arena*)(user), (pointer), 0); } } while(0)
#define STBTT_assert(expession)             do { Assert(expession); } while(0)
#define STBTT_strlen(str)                   (uxx)MyStrLength64(str)
#define STBTT_memcpy(dest, source, length)  MyMemCopy((dest), (source), (length))
#define STBTT_memset(dest, value, length)   MyMemSet((dest), (value), (length))
#if COMPILER_IS_MSVC
#pragma warning(push)
#pragma warning(disable: 5262) //implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases
#endif
#include "third_party/stb/stb_truetype.h"
#if COMPILER_IS_MSVC
#pragma warning(pop)
#endif

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
	u8 styleFlags;
	VarArray charRanges; //FontCharRange
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
	u8 ttfStyleFlags;
	VarArray atlases; //FontAtlas
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void FreeFontAtlas(Font* font, FontAtlas* atlas);
	void FreeFont(Font* font);
	void ClearFontAtlases(Font* font);
	Font InitFont(Arena* arena, Str8 name);
	PIG_CORE_INLINE FontCharRange NewFontCharRange(u32 startCodepoint, u32 endCodepoint);
	PIG_CORE_INLINE FontCharRange NewFontCharRangeLength(u32 startCodepoint, u32 numCodepoints);
	PIG_CORE_INLINE void RemoveAttachedTtfFile(Font* font);
	void AttachTtfFileToFont(Font* font, Slice ttfFileContents);
	Result BakeFontAtlas(Font* font, r32 fontSize, u8 extraStyleFlags, v2i atlasSize, uxx numCharRanges, const FontCharRange* charRanges);
	PIG_CORE_INLINE FontAtlas* GetDefaultFontAtlas(Font* font);
	PIG_CORE_INLINE r32 GetDefaultFontSize(const Font* font);
	PIG_CORE_INLINE u8 GetDefaultFontStyleFlags(const Font* font);
	PIG_CORE_INLINE bool DoesFontAtlasContainCodepointEx(const FontAtlas* atlas, u32 codepoint, uxx* glyphIndexOut);
	PIG_CORE_INLINE bool DoesFontAtlasContainCodepoint(const FontAtlas* atlas, u32 codepoint);
	FontGlyph* GetFontGlyphForCodepoint(Font* font, u32 codepoint, r32 fontSize, u8 styleFlags, FontAtlas** atlasOut);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void FreeFontAtlas(Font* font, FontAtlas* atlas)
{
	NotNull(font);
	NotNull(atlas);
	FreeVarArray(&atlas->charRanges);
	FreeVarArray(&atlas->glyphs);
	FreeTexture(&atlas->texture);
	ClearPointer(atlas);
}

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
			FreeFontAtlas(font, atlas);
		}
		FreeVarArray(&font->atlases);
	}
	ClearPointer(font);
}

PEXP void ClearFontAtlases(Font* font)
{
	NotNull(font);
	NotNull(font->arena);
	VarArrayLoop(&font->atlases, aIndex)
	{
		VarArrayLoopGet(FontAtlas, atlas, &font->atlases, aIndex);
		FreeFontAtlas(font, atlas);
	}
	VarArrayClear(&font->atlases);
}

PEXP Font InitFont(Arena* arena, Str8 name)
{
	NotNull(arena);
	NotNullStr(name);
	Font result = ZEROED;
	result.arena = arena;
	InitVarArray(FontAtlas, &result.atlases, arena);
	result.name = AllocStr8(arena, name);
	return result;
}

PEXPI FontCharRange NewFontCharRange(u32 startCodepoint, u32 endCodepoint)
{
	FontCharRange result = ZEROED;
	result.startCodepoint = startCodepoint;
	result.endCodepoint = endCodepoint;
	return result;
}
PEXPI FontCharRange NewFontCharRangeLength(u32 startCodepoint, u32 numCodepoints)
{
	Assert(numCodepoints > 0);
	return NewFontCharRange(startCodepoint, startCodepoint + numCodepoints-1);
}

PEXPI void RemoveAttachedTtfFile(Font* font)
{
	NotNull(font);
	NotNull(font->arena);
	if (font->ttfFile.chars != nullptr)
	{
		FreeStr8(font->arena, &font->ttfFile);
	}
}

PEXP void AttachTtfFileToFont(Font* font, Slice ttfFileContents, u8 ttfStyleFlags)
{
	NotNull(font);
	NotNullStr(ttfFileContents);
	NotEmptyStr(ttfFileContents);
	FreeStr8(font->arena, &font->ttfFile);
	font->ttfFile.length = ttfFileContents.length;
	font->ttfFile.chars = AllocMem(font->arena, ttfFileContents.length);
	NotNull(font->ttfFile.chars);
	MyMemCopy(font->ttfFile.chars, ttfFileContents.chars, ttfFileContents.length);
	font->ttfStyleFlags = ttfStyleFlags;
}

PEXP Result BakeFontAtlas(Font* font, r32 fontSize, u8 extraStyleFlags, v2i atlasSize, uxx numCharRanges, const FontCharRange* charRanges)
{
	NotNull(font);
	NotNull(font->arena);
	Assert(!IsEmptyStr(font->ttfFile));
	Assert(atlasSize.Width > 0 && atlasSize.Height > 0);
	Assert(numCharRanges > 0);
	NotNull(charRanges);
	ScratchBegin1(scratch, font->arena);
	
	uxx stbOutPixelSize = sizeof(u8);
	uxx numPixels = (uxx)(atlasSize.Width * atlasSize.Height);
	Color32* pixelsPntr = AllocArray(Color32, font->arena, numPixels); //NOTE: We allocate Color32 size so we can expand the 1-byte output from stb_truetype into a 4-channel pixel
	NotNull(pixelsPntr);
	MyMemSet(pixelsPntr, 0x00, sizeof(u32) * numPixels);
	
	stbtt_pack_context packContext = ZEROED;
	int beginResult = stbtt_PackBegin(
		&packContext, //context
		(u8*)pixelsPntr, //pixels
		(int)atlasSize.Width, (int)atlasSize.Height, //width, height
		(int)(atlasSize.Width * stbOutPixelSize), //stride_in_bytes
		1, //padding (between chars)
		scratch //alloc_context
	);
	
	stbtt_pack_range* stbRanges = AllocArray(stbtt_pack_range, scratch, numCharRanges);
	NotNull(stbRanges);
	MyMemSet(stbRanges, 0x00, sizeof(stbtt_pack_range) * numCharRanges);
	u32 minCodepoint = UINT32_MAX;
	u32 maxCodepoint = 0;
	uxx numGlyphs = 0;
	for (uxx rIndex = 0; rIndex < numCharRanges; rIndex++)
	{
		const FontCharRange* charRange = &charRanges[rIndex];
		stbtt_pack_range* stbRange = &stbRanges[rIndex];
		stbRange->font_size = fontSize;
		stbRange->first_unicode_codepoint_in_range = (int)charRange->startCodepoint;
		// int *array_of_unicode_codepoints;       // if non-zero, then this is an array of unicode codepoints
		stbRange->num_chars = (int)(charRange->endCodepoint+1 - charRange->startCodepoint);
		uxx numGlyphsInCharRange = (charRange->endCodepoint - charRange->startCodepoint)+1;
		stbRange->chardata_for_range = AllocArray(stbtt_packedchar, scratch, numGlyphsInCharRange);
		NotNull(stbRange->chardata_for_range);
		#if DEBUG_BUILD
		MyMemSet(stbRange->chardata_for_range, 0xCC, sizeof(stbtt_packedchar) * numGlyphsInCharRange);
		#endif
		minCodepoint = MinU32(minCodepoint, charRange->startCodepoint);
		maxCodepoint = MaxU32(maxCodepoint, charRange->endCodepoint);
		numGlyphs += numGlyphsInCharRange;
	}
	
	int packResult = stbtt_PackFontRanges(
		&packContext, //spc
		font->ttfFile.bytes, //fontdata
		0, //font_index,
		stbRanges, //ranges,
		(int)numCharRanges //num_ranges
	);
	if (packResult <= 0)
	{
		ScratchEnd(scratch);
		return Result_NotEnoughSpace;
	}
	
	// Walk backwards from end, expanding each 1-byte/single channel pixel value into a Color32 (4-byte/4 channel)
	for (uxx pixelIndex = numPixels; pixelIndex > 0; pixelIndex--)
	{
		Color32* writePntr = &pixelsPntr[pixelIndex-1];
		u8* readPntr = &((u8*)pixelsPntr)[pixelIndex-1];
		writePntr->a = *readPntr;
		writePntr->r = 255;
		writePntr->g = 255;
		writePntr->b = 255;
	}
	
	FontAtlas* newAtlas = VarArrayAdd(FontAtlas, &font->atlases);
	NotNull(newAtlas);
	ClearPointer(newAtlas);
	Str8 textureName = PrintInArenaStr(scratch, "%.*s_atlas[%llu]", StrPrint(font->name), (u64)(font->atlases.length-1));
	newAtlas->texture = InitTexture(font->arena, textureName, atlasSize, pixelsPntr, TextureFlag_NoMipmaps);
	if (newAtlas->texture.error != Result_Success)
	{
		VarArrayPop(FontCharRange, &font->atlases);
		ScratchEnd(scratch);
		return newAtlas->texture.error;
	}
	
	newAtlas->fontSize = fontSize;
	newAtlas->styleFlags = (font->ttfStyleFlags | extraStyleFlags);
	newAtlas->glyphRange.startCodepoint = minCodepoint;
	newAtlas->glyphRange.endCodepoint = maxCodepoint;
	
	InitVarArrayWithInitial(FontCharRange, &newAtlas->charRanges, font->arena, numCharRanges);
	InitVarArrayWithInitial(FontGlyph, &newAtlas->glyphs, font->arena, numGlyphs);
	for (uxx rIndex = 0; rIndex < numCharRanges; rIndex++)
	{
		const FontCharRange* charRange = &charRanges[rIndex];
		stbtt_pack_range* stbRange = &stbRanges[rIndex];
		FontCharRange* altasRange = VarArrayAdd(FontCharRange, &newAtlas->charRanges);
		MyMemCopy(altasRange, charRange, sizeof(FontCharRange));
		uxx numGlyphsInCharRange = (charRange->endCodepoint - charRange->startCodepoint)+1;
		altasRange->glyphArrayStartIndex = newAtlas->glyphs.length;
		FontGlyph* newGlyphs = VarArrayAddMulti(FontGlyph, &newAtlas->glyphs, numGlyphsInCharRange);
		NotNull(newGlyphs);
		MyMemSet(newGlyphs, 0x00, sizeof(FontGlyph) * numGlyphsInCharRange);
		NotNull(stbRange->chardata_for_range);
		for (uxx gIndex = 0; gIndex < numGlyphsInCharRange; gIndex++)
		{
			const stbtt_packedchar* stbCharInfo = &stbRange->chardata_for_range[gIndex];
			FontGlyph* glyph = &newGlyphs[gIndex];
			glyph->codepoint = charRange->startCodepoint + (u32)gIndex;
			DebugAssert(stbCharInfo->x0 <= stbCharInfo->x1);
			DebugAssert(stbCharInfo->y0 <= stbCharInfo->y1);
			DebugAssert(stbCharInfo->x0 >= 0);
			DebugAssert(stbCharInfo->x0 <= atlasSize.Width);
			DebugAssert(stbCharInfo->x1 >= 0);
			DebugAssert(stbCharInfo->x1 <= atlasSize.Width);
			DebugAssert(stbCharInfo->y0 >= 0);
			DebugAssert(stbCharInfo->y0 <= atlasSize.Height);
			DebugAssert(stbCharInfo->y1 >= 0);
			DebugAssert(stbCharInfo->y1 <= atlasSize.Height);
			glyph->atlasSourceRec = NewReci((i32)stbCharInfo->x0, (i32)stbCharInfo->y0, (i32)(stbCharInfo->x1 - stbCharInfo->x0), (i32)(stbCharInfo->y1 - stbCharInfo->y0));
			glyph->advanceX = stbCharInfo->xadvance;
			glyph->renderOffset = NewV2(stbCharInfo->xoff, stbCharInfo->yoff);
			//TODO: Logical rec maybe should be a different size than the renderable glyph?
			glyph->logicalRec = NewRec(-stbCharInfo->xoff, -stbCharInfo->yoff, (r32)glyph->atlasSourceRec.Width, (r32)glyph->atlasSourceRec.Height);
			// TODO: What are these floats for? stbCharInfo->xoff2 stbCharInfo->yoff2
		}
	}
	
	ScratchEnd(scratch);
	
	return Result_Success;
}

PEXPI FontAtlas* GetDefaultFontAtlas(Font* font)
{
	NotNull(font);
	NotNull(font->arena);
	if (font->atlases.length > 0) { return VarArrayGetFirst(FontAtlas, &font->atlases); }
	else { return nullptr; }
}
PEXPI r32 GetDefaultFontSize(const Font* font)
{
	FontAtlas* defaultAtlas = GetDefaultFontAtlas((Font*)font);
	return (defaultAtlas != nullptr) ? defaultAtlas->fontSize : 0.0f;
}
PEXPI u8 GetDefaultFontStyleFlags(const Font* font)
{
	FontAtlas* defaultAtlas = GetDefaultFontAtlas((Font*)font);
	return (defaultAtlas != nullptr) ? defaultAtlas->styleFlags : FontStyleFlag_None;
}

PEXPI bool DoesFontAtlasContainCodepointEx(const FontAtlas* atlas, u32 codepoint, uxx* glyphIndexOut)
{
	if (atlas->glyphRange.startCodepoint <= codepoint && atlas->glyphRange.endCodepoint >= codepoint)
	{
		VarArrayLoop(&atlas->charRanges, rIndex)
		{
			VarArrayLoopGet(FontCharRange, charRange, &atlas->charRanges, rIndex);
			if (charRange->startCodepoint <= codepoint && charRange->endCodepoint >= codepoint)
			{
				SetOptionalOutPntr(glyphIndexOut, charRange->glyphArrayStartIndex + (uxx)(codepoint - charRange->startCodepoint));
				return true;
			}
		}
	}
	return false;
}
PEXPI bool DoesFontAtlasContainCodepoint(const FontAtlas* atlas, u32 codepoint)
{
	return DoesFontAtlasContainCodepointEx(atlas, codepoint, nullptr);
}

PEXP FontGlyph* GetFontGlyphForCodepoint(Font* font, u32 codepoint, r32 fontSize, u8 styleFlags, FontAtlas** atlasOut)
{
	NotNull(font);
	
	bool multipleMatches = false;
	FontAtlas* matchingAtlas = nullptr;
	r32 matchingSizeDiff = 0.0f;
	FontGlyph* result = nullptr;
	VarArrayLoop(&font->atlases, aIndex)
	{
		VarArrayLoopGet(FontAtlas, atlas, &font->atlases, aIndex);
		r32 sizeDiff = AbsR32(atlas->fontSize - fontSize);
		if (matchingAtlas == nullptr || sizeDiff <= matchingSizeDiff)
		{
			uxx glyphIndex = 0;
			if (DoesFontAtlasContainCodepointEx(atlas, codepoint, &glyphIndex))
			{
				multipleMatches = (matchingAtlas != nullptr && AreSimilarR32(sizeDiff, matchingSizeDiff, DEFAULT_R32_TOLERANCE));
				matchingSizeDiff = sizeDiff;
				matchingAtlas = atlas;
				result = VarArrayGetHard(FontGlyph, &atlas->glyphs, glyphIndex);
			}
		}
	}
	
	if (matchingAtlas == nullptr) { return nullptr; }
	
	if (multipleMatches)
	{
		//TODO: If we find more than one bake with the same fontSize, we should differentiate based on which one has closer style flags
		multipleMatches = false;
		matchingAtlas = nullptr;
		result = nullptr;
		uxx matchingStyleDiffs = 0;
		
		VarArrayLoop(&font->atlases, aIndex)
		{
			VarArrayLoopGet(FontAtlas, atlas, &font->atlases, aIndex);
			r32 sizeDiff = AbsR32(atlas->fontSize - fontSize);
			if (AreSimilarR32(sizeDiff, matchingSizeDiff, DEFAULT_R32_TOLERANCE))
			{
				uxx styleDiffs = 0;
				if (IsFlagSet(atlas->styleFlags, FontStyleFlag_Inverted) != IsFlagSet(styleFlags, FontStyleFlag_Inverted)) { styleDiffs += 4; }
				if (IsFlagSet(atlas->styleFlags, FontStyleFlag_Bold) != IsFlagSet(styleFlags, FontStyleFlag_Bold)) { styleDiffs += 1; }
				if (IsFlagSet(atlas->styleFlags, FontStyleFlag_Italic) != IsFlagSet(styleFlags, FontStyleFlag_Italic)) { styleDiffs += 1; }
				//TODO: Should we care about Underline, Strikethrough or Outline?
				
				if (matchingAtlas == nullptr || styleDiffs <= matchingStyleDiffs)
				{
					uxx glyphIndex = 0;
					if (DoesFontAtlasContainCodepointEx(atlas, codepoint, &glyphIndex))
					{
						multipleMatches = (matchingAtlas != nullptr && styleDiffs == matchingStyleDiffs);
						matchingStyleDiffs = styleDiffs;
						matchingAtlas = atlas;
						result = VarArrayGetHard(FontGlyph, &atlas->glyphs, glyphIndex);
					}
				}
			}
		}
		
		NotNull(result);
	}
	
	if (multipleMatches)
	{
		//TODO: If we still find more than one match, is there any other criteria we should check?
	}
	
	SetOptionalOutPntr(atlasOut, matchingAtlas);
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _GFX_FONT_H

#if defined(_OS_FONT_H) && defined(_GFX_FONT_H)
#include "cross/cross_os_font_and_gfx_font.h"
#endif
