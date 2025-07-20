/*
File:   gfx_font.h
Author: Taylor Robbins
Date:   02\05\2025
Description:
	** A "PigFont" is a data structure built on top of possibly multiple Texture atlases
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
#include "struct/struct_rich_string.h"
#include "gfx/gfx_texture.h"

//TODO: Eventually we may want to support using Font stuff in Raylib! That would require making a gfx_texture implementation for Raylib first so we aren't doing that for now
#if BUILD_WITH_SOKOL_GFX

#if PIG_CORE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#endif
#define STBRP_SORT                          qsort //TODO: Do we want to route this to one of our own sorting functions?
#define STBRP_ASSERT(condition)             Assert(condition)
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
#if COMPILER_IS_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough" //warning: unannotated fall-through between switch labels
#endif
#include "third_party/stb/stb_rect_pack.h"
#include "third_party/stb/stb_truetype.h"
#if COMPILER_IS_MSVC
#pragma warning(pop)
#endif
#if COMPILER_IS_CLANG
#pragma clang diagnostic pop
#endif

typedef struct FontCharRange FontCharRange;
struct FontCharRange
{
	u32 startCodepoint;
	u32 endCodepoint;
	uxx glyphArrayStartIndex;
};

typedef struct CustomFontGlyph CustomFontGlyph;
struct CustomFontGlyph
{
	u32 codepoint;
	ImageData imageData;
	reci sourceRec;
};
typedef struct CustomFontCharRange CustomFontCharRange;
struct CustomFontCharRange
{
	u32 startCodepoint;
	u32 endCodepoint;
	CustomFontGlyph* glyphs;
};

#define INVALID_TTF_GLYPH_INDEX INT32_MAX

typedef struct FontGlyph FontGlyph;
struct FontGlyph
{
	u32 codepoint;
	i32 ttfGlyphIndex;
	reci atlasSourceRec;
	r32 advanceX;
	v2 renderOffset;
	rec logicalRec;
};

typedef struct FontAtlas FontAtlas;
struct FontAtlas
{
	r32 fontSize;
	r32 fontScale; //only used when asking stbtt for size-independent metrics
	u8 styleFlags;
	VarArray charRanges; //FontCharRange
	FontCharRange glyphRange;
	VarArray glyphs;
	Texture texture;
	r32 lineHeight;
	r32 maxAscend;
	r32 maxDescend;
	r32 centerOffset;
};

typedef struct FontKerningTableEntry FontKerningTableEntry;
struct FontKerningTableEntry
{
	u32 leftTtfGlyphIndex;
	u32 rightTtfGlyphIndex;
	r32 value; //must be multipled by fontScale
};

typedef struct FontKerningTable FontKerningTable;
struct FontKerningTable
{
	uxx numEntries;
	FontKerningTableEntry* entries;
};

//NOTE: We have a naming conflict with raylib.h if we name this "PigFont" so we are
// naming it PigFont, but typedefing PigFont in non-raylib projects so anything outside
// PigCore that doesn't plan to use Raylib can still use the name "PigFont"
typedef struct PigFont PigFont;
struct PigFont
{
	Arena* arena;
	Str8 name;
	
	Slice ttfFile;
	u8 ttfStyleFlags;
	stbtt_fontinfo ttfInfo;
	
	VarArray atlases; //FontAtlas
	FontKerningTable kerningTable;
};

//NOTE: Font is defined in Xlib.h as a typedef of XID
#if !BUILD_WITH_RAYLIB && !TARGET_IS_LINUX
typedef PigFont Font;
#endif

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeFontAtlas(PigFont* font, FontAtlas* atlas);
	PIG_CORE_INLINE void FreeFontKerningTable(Arena* arena, FontKerningTable* kerningTable);
	void FreeFont(PigFont* font);
	void ClearFontAtlases(PigFont* font);
	PigFont InitFont(Arena* arena, Str8 name);
	PIG_CORE_INLINE FontCharRange NewFontCharRangeSingle(u32 codepoint);
	PIG_CORE_INLINE FontCharRange NewFontCharRange(u32 startCodepoint, u32 endCodepoint);
	PIG_CORE_INLINE FontCharRange NewFontCharRangeLength(u32 startCodepoint, u32 numCodepoints);
	PIG_CORE_INLINE CustomFontCharRange NewCustomFontCharRangeSingle(CustomFontGlyph* glyph);
	PIG_CORE_INLINE CustomFontCharRange NewCustomFontCharRange(uxx numGlyphs, CustomFontGlyph* glyph);
	PIG_CORE_INLINE void RemoveAttachedTtfFile(PigFont* font);
	PIG_CORE_INLINE void InitFontTtfInfo(PigFont* font);
	void AttachTtfFileToFont(PigFont* font, Slice ttfFileContents);
	void FillFontKerningTable(PigFont* font);
	Result BakeFontAtlasEx(PigFont* font, r32 fontSize, u8 extraStyleFlags, v2i atlasSize, uxx numCharRanges, const FontCharRange* charRanges, uxx numCustomGlyphRanges, const CustomFontCharRange* customGlyphRanges);
	PIG_CORE_INLINE Result BakeFontAtlas(PigFont* font, r32 fontSize, u8 extraStyleFlags, v2i atlasSize, uxx numCharRanges, const FontCharRange* charRanges);
	PIG_CORE_INLINE FontAtlas* GetDefaultFontAtlas(PigFont* font);
	PIG_CORE_INLINE r32 GetDefaultFontSize(const PigFont* font);
	PIG_CORE_INLINE u8 GetDefaultFontStyleFlags(const PigFont* font);
	PIG_CORE_INLINE bool DoesFontAtlasContainCodepointEx(const FontAtlas* atlas, u32 codepoint, uxx* glyphIndexOut);
	PIG_CORE_INLINE bool DoesFontAtlasContainCodepoint(const FontAtlas* atlas, u32 codepoint);
	FontGlyph* GetFontGlyphForCodepoint(PigFont* font, u32 codepoint, r32 fontSize, u8 styleFlags, FontAtlas** atlasOut);
	PIG_CORE_INLINE FontAtlas* GetFontAtlas(PigFont* font, r32 fontSize, u8 styleFlags);
	r32 GetFontKerningBetweenGlyphs(const PigFont* font, r32 fontScale, const FontGlyph* leftGlyph, const FontGlyph* rightGlyph);
	r32 GetFontKerningBetweenCodepoints(const PigFont* font, r32 fontSize, u8 styleFlags, u32 leftCodepoint, u32 rightCodepoint);
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define FontCharRange_ASCII    NewFontCharRange(0x20, 0x7E)
#define FontCharRange_LatinExt NewFontCharRangeLength(0xA0, 96)
#define FontCharRange_Cyrillic NewFontCharRangeLength(0x400, 256)
#define FontCharRange_Hiragana NewFontCharRangeLength(0x3041, 95)
#define FontCharRange_Katakana NewFontCharRangeLength(0x30A0, 95)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void FreeFontAtlas(PigFont* font, FontAtlas* atlas)
{
	NotNull(font);
	NotNull(atlas);
	UNUSED(font);
	FreeVarArray(&atlas->charRanges);
	FreeVarArray(&atlas->glyphs);
	FreeTexture(&atlas->texture);
	ClearPointer(atlas);
}

PEXPI void FreeFontKerningTable(Arena* arena, FontKerningTable* kerningTable)
{
	NotNull(arena);
	NotNull(kerningTable);
	if (kerningTable->entries != nullptr)
	{
		FreeArray(FontKerningTableEntry, arena, kerningTable->numEntries, kerningTable->entries);
	}
	ClearPointer(kerningTable);
}

PEXP void FreeFont(PigFont* font)
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
		FreeFontKerningTable(font->arena, &font->kerningTable);
	}
	ClearPointer(font);
}

PEXP void ClearFontAtlases(PigFont* font)
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

PEXP PigFont InitFont(Arena* arena, Str8 name)
{
	NotNull(arena);
	NotNullStr(name);
	PigFont result = ZEROED;
	result.arena = arena;
	InitVarArray(FontAtlas, &result.atlases, arena);
	result.name = AllocStr8(arena, name);
	return result;
}

PEXPI FontCharRange NewFontCharRangeSingle(u32 codepoint)
{
	FontCharRange result = ZEROED;
	result.startCodepoint = codepoint;
	result.endCodepoint = codepoint;
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

PEXPI CustomFontCharRange NewCustomFontCharRangeSingle(CustomFontGlyph* glyph)
{
	NotNull(glyph);
	CustomFontCharRange result = ZEROED;
	result.startCodepoint = glyph->codepoint;
	result.endCodepoint = glyph->codepoint;
	result.glyphs = glyph;
	return result;
}
PEXPI CustomFontCharRange NewCustomFontCharRange(uxx numGlyphs, CustomFontGlyph* glyphs)
{
	NotNull(glyphs);
	Assert(numGlyphs > 0);
	CustomFontCharRange result = ZEROED;
	u32 prevCodepoint = 0;
	for (uxx gIndex = 0; gIndex < numGlyphs; gIndex++)
	{
		CustomFontGlyph* glyph = &glyphs[gIndex];
		if (gIndex == 0) { result.startCodepoint = glyph->codepoint; }
		else { AssertMsg(prevCodepoint == glyph->codepoint-1, "Codepoints in glyphs must be consecutive when calling NewCustomFontCharRange"); }
		if (gIndex+1 == numGlyphs) { result.endCodepoint = glyph->codepoint; }
		prevCodepoint = glyph->codepoint;
	}
	result.glyphs = glyphs;
	return result;
}

PEXPI void RemoveAttachedTtfFile(PigFont* font)
{
	NotNull(font);
	NotNull(font->arena);
	if (font->ttfFile.chars != nullptr)
	{
		FreeStr8(font->arena, &font->ttfFile);
	}
	ClearStruct(font->ttfInfo);
}

PEXPI void InitFontTtfInfo(PigFont* font)
{
	NotNull(font);
	NotEmptyStr(font->ttfFile);
	int firstFontOffset = stbtt_GetFontOffsetForIndex(font->ttfFile.bytes, 0);
	Assert(firstFontOffset >= 0);
	int initFontResult = stbtt_InitFont(&font->ttfInfo, font->ttfFile.bytes, firstFontOffset);
	Assert(initFontResult != 0);
	UNUSED(initFontResult);
	// int numOfFontsInTtf = stbtt_GetNumberOfFonts(font->ttfFile.bytes);
	// PrintLine_D("There %s %d font%s in this ttf file", PluralEx(numOfFontsInTtf, "is", "are"), numOfFontsInTtf, Plural(numOfFontsInTtf, "s"));
}

PEXP void AttachTtfFileToFont(PigFont* font, Slice ttfFileContents, u8 ttfStyleFlags)
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
	InitFontTtfInfo(font);
}

PEXP void FillFontKerningTable(PigFont* font)
{
	NotNull(font);
	NotNull(font->arena);
	NotEmptyStr(font->ttfFile);
	
	FreeFontKerningTable(font->arena, &font->kerningTable);
	
	int tableLength = stbtt_GetKerningTableLength(&font->ttfInfo);
	Assert(tableLength >= 0);
	if (tableLength == 0) { return; }
	
	ScratchBegin1(scratch, font->arena);
	
	stbtt_kerningentry* stbEntries = AllocArray(stbtt_kerningentry, scratch, (uxx)tableLength);
	NotNull(stbEntries);
	int getResult = stbtt_GetKerningTable(&font->ttfInfo, stbEntries, tableLength);
	Assert(getResult >= 0);
	Assert(getResult <= tableLength);
	if (getResult == 0) { ScratchEnd(scratch); return; }
	
	font->kerningTable.numEntries = (uxx)getResult;
	font->kerningTable.entries = AllocArray(FontKerningTableEntry, font->arena, font->kerningTable.numEntries);
	NotNull(font->kerningTable.entries);
	for (uxx eIndex = 0; eIndex < font->kerningTable.numEntries; eIndex++)
	{
		const stbtt_kerningentry* stbEntry = &stbEntries[eIndex];
		FontKerningTableEntry* kerningEntry = &font->kerningTable.entries[eIndex];
		Assert(stbEntry->glyph1 >= 0);
		Assert(stbEntry->glyph2 >= 0);
		kerningEntry->leftTtfGlyphIndex = (u32)stbEntry->glyph1;
		kerningEntry->rightTtfGlyphIndex = (u32)stbEntry->glyph2;
		kerningEntry->value = (r32)stbEntry->advance;
	}
	
	ScratchEnd(scratch);
}

PEXP Result BakeFontAtlasEx(PigFont* font, r32 fontSize, u8 extraStyleFlags, v2i atlasSize, uxx numCharRanges, const FontCharRange* charRanges, uxx numCustomGlyphRanges, const CustomFontCharRange* customGlyphRanges)
{
	NotNull(font);
	NotNull(font->arena);
	NotEmptyStr(font->ttfFile);
	Assert(atlasSize.Width > 0 && atlasSize.Height > 0);
	Assert(numCharRanges > 0);
	NotNull(charRanges);
	ScratchBegin1(scratch, font->arena);
	Assert(numCustomGlyphRanges == 0 || customGlyphRanges != nullptr);
	
	uxx stbOutPixelSize = sizeof(u8);
	uxx numPixels = (uxx)(atlasSize.Width * atlasSize.Height);
	Color32* pixelsPntr = AllocArray(Color32, scratch, numPixels); //NOTE: We allocate Color32 size so we can expand the 1-byte output from stb_truetype into a 4-channel pixel
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
	Assert(beginResult != 0);
	UNUSED(beginResult);
	
	stbtt_pack_range* stbRanges = AllocArray(stbtt_pack_range, scratch, numCharRanges);
	NotNull(stbRanges);
	MyMemSet(stbRanges, 0x00, sizeof(stbtt_pack_range) * numCharRanges);
	u32 minCodepoint = UINT32_MAX;
	u32 maxCodepoint = 0;
	uxx numGlyphsInRanges = 0;
	for (uxx rIndex = 0; rIndex < numCharRanges; rIndex++)
	{
		const FontCharRange* charRange = &charRanges[rIndex];
		Assert(charRange->endCodepoint >= charRange->startCodepoint);
		stbtt_pack_range* stbRange = &stbRanges[rIndex];
		stbRange->font_size = fontSize;
		stbRange->first_unicode_codepoint_in_range = (int)charRange->startCodepoint;
		// int *array_of_unicode_codepoints;       // if non-zero, then this is an array of unicode codepoints
		stbRange->num_chars = (int)(charRange->endCodepoint+1 - charRange->startCodepoint);
		uxx numGlyphsInCharRange = (charRange->endCodepoint - charRange->startCodepoint)+1;
		stbRange->chardata_for_range = AllocArray(stbtt_packedchar, scratch, numGlyphsInCharRange);
		NotNull(stbRange->chardata_for_range);
		MyMemSet(stbRange->chardata_for_range, 0x00, sizeof(stbtt_packedchar) * numGlyphsInCharRange);
		minCodepoint = MinU32(minCodepoint, charRange->startCodepoint);
		maxCodepoint = MaxU32(maxCodepoint, charRange->endCodepoint);
		numGlyphsInRanges += numGlyphsInCharRange;
	}
	uxx numGlyphsInCustomRanges = 0;
	for (uxx rIndex = 0; rIndex < numCustomGlyphRanges; rIndex++)
	{
		const CustomFontCharRange* customRange = &customGlyphRanges[rIndex];
		Assert(customRange->endCodepoint >= customRange->startCodepoint);
		NotNull(customRange->glyphs);
		//TODO: We should probably check to make sure the codepoints don't exist in the regular charRanges that were given!
		uxx numGlyphsInCustomRange = (customRange->endCodepoint - customRange->startCodepoint)+1;
		minCodepoint = MinU32(minCodepoint, customRange->startCodepoint);
		maxCodepoint = MaxU32(maxCodepoint, customRange->endCodepoint);
		numGlyphsInCustomRanges += numGlyphsInCustomRange;
	}
	uxx numGlyphsTotal = numGlyphsInRanges + numGlyphsInCustomRanges;
	
	//NOTE: This used to be stbtt_PackFontRanges
	stbtt_fontinfo fontInfo;
	fontInfo.userdata = packContext.user_allocator_context;
	stbtt_InitFont(&fontInfo, font->ttfFile.bytes, stbtt_GetFontOffsetForIndex(font->ttfFile.bytes, 0));
	
	stbrp_rect* rects = AllocArray(stbrp_rect, scratch, numGlyphsTotal);
	NotNull(rects);
	int numRects = stbtt_PackFontRangesGatherRects(&packContext, &fontInfo, stbRanges, (int)numCharRanges, rects);
	Assert(numRects >= 0 && (uxx)numRects == numGlyphsInRanges);
	UNUSED(numRects);
	uxx customGlyphIndex = 0;
	for (uxx rIndex = 0; rIndex < numCustomGlyphRanges; rIndex++)
	{
		const CustomFontCharRange* customRange = &customGlyphRanges[rIndex];
		uxx numGlyphsInCustomRange = (customRange->endCodepoint - customRange->startCodepoint)+1;
		for (uxx gIndex = 0; gIndex < numGlyphsInCustomRange; gIndex++)
		{
			const CustomFontGlyph* customGlyph = &customRange->glyphs[gIndex];
			DebugAssert(customGlyphIndex < numGlyphsInCustomRanges);
			stbrp_rect* customGlyphRec = &rects[numGlyphsInRanges + customGlyphIndex];
			reci sourceRec = AreEqual(customGlyph->sourceRec, Reci_Zero_Const)
				? NewReci(0, 0, customGlyph->imageData.size.Width, customGlyph->imageData.size.Height)
				: customGlyph->sourceRec;
			Assert(sourceRec.X >= 0 && sourceRec.Y >= 0);
			Assert(sourceRec.Width > 0 && sourceRec.Height > 0);
			Assert(sourceRec.X + sourceRec.Width <= customGlyph->imageData.size.Width);
			Assert(sourceRec.Y + sourceRec.Height <= customGlyph->imageData.size.Height);
			customGlyphRec->w = (int)sourceRec.Width;
			customGlyphRec->h = (int)sourceRec.Height;
			customGlyphIndex++;
		}
	}
	
	stbtt_PackFontRangesPackRects(&packContext, rects, (int)numGlyphsTotal);
	
	int packResult = stbtt_PackFontRangesRenderIntoRects(&packContext, &fontInfo, stbRanges, (int)numCharRanges, rects);
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
	
	// Copy custom glyph image data into the atlas
	customGlyphIndex = 0;
	for (uxx rIndex = 0; rIndex < numCustomGlyphRanges; rIndex++)
	{
		const CustomFontCharRange* customRange = &customGlyphRanges[rIndex];
		uxx numGlyphsInCustomRange = (customRange->endCodepoint - customRange->startCodepoint)+1;
		for (uxx gIndex = 0; gIndex < numGlyphsInCustomRange; gIndex++)
		{
			const CustomFontGlyph* customGlyph = &customRange->glyphs[gIndex];
			DebugAssert(customGlyphIndex < numGlyphsInCustomRanges);
			stbrp_rect* customGlyphRec = &rects[numGlyphsInRanges + customGlyphIndex];
			reci sourceRec = AreEqual(customGlyph->sourceRec, Reci_Zero_Const)
				? NewReci(0, 0, customGlyph->imageData.size.Width, customGlyph->imageData.size.Height)
				: customGlyph->sourceRec;
			Assert(customGlyphRec->w == (int)sourceRec.Width);
			Assert(customGlyphRec->h == (int)sourceRec.Height);
			Assert(customGlyphRec->x >= 0 && customGlyphRec->y >= 0);
			Assert(customGlyphRec->x + customGlyphRec->w <= (int)atlasSize.Width);
			Assert(customGlyphRec->y + customGlyphRec->h <= (int)atlasSize.Height);
			for (i32 yOffset = 0; yOffset < sourceRec.Height; yOffset++)
			{
				v2i targetPos = NewV2i((i32)customGlyphRec->x, (i32)customGlyphRec->y + yOffset);
				u32* targetPntr = (u32*)&pixelsPntr[INDEX_FROM_COORD2D(targetPos.X, targetPos.Y, atlasSize.Width, atlasSize.Height)];
				u32* sourcePntr = &customGlyph->imageData.pixels[INDEX_FROM_COORD2D(sourceRec.X, sourceRec.Y + yOffset, customGlyph->imageData.size.Width, customGlyph->imageData.size.Height)];
				MyMemCopy(targetPntr, sourcePntr, sizeof(u32) * sourceRec.Width);
			}
			customGlyphIndex++;
		}
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
	newAtlas->fontScale = stbtt_ScaleForPixelHeight(&font->ttfInfo, fontSize);
	newAtlas->styleFlags = (font->ttfStyleFlags | extraStyleFlags);
	newAtlas->glyphRange.startCodepoint = minCodepoint;
	newAtlas->glyphRange.endCodepoint = maxCodepoint;
	
	int ascent, descent, lineGap;
	stbtt_GetFontVMetrics(&font->ttfInfo, &ascent, &descent, &lineGap);
	newAtlas->maxAscend = (r32)ascent * newAtlas->fontScale;
	newAtlas->maxDescend = (r32)(-descent) * newAtlas->fontScale;
	
	//The ascent value returned by GetFontVMetrics is often way higher than all the characters we normally print
	//Rather than using that value, we'd prefer to use the ascent of a character like 'W' to get a more accurate idea of how far up the font will extend
	//This helps look more visually appealing with positioning text vertically centered in a small space (like in a UI button)
	int wBoxX0, wBoxY0, wBoxX1, wBoxY1;
	int getBoxResult = stbtt_GetCodepointBox(&font->ttfInfo, CharToU32('W'), &wBoxX0, &wBoxY0, &wBoxX1, &wBoxY1);
	if (getBoxResult > 0)
	{
		newAtlas->maxAscend = MinR32(newAtlas->maxAscend, (r32)wBoxY1 * newAtlas->fontScale);
	}
	
	newAtlas->lineHeight = newAtlas->maxAscend + newAtlas->maxDescend + ((r32)lineGap * newAtlas->fontScale);
	newAtlas->centerOffset = newAtlas->maxAscend / 2.0f;
	
	InitVarArrayWithInitial(FontCharRange, &newAtlas->charRanges, font->arena, numCharRanges + numCustomGlyphRanges);
	InitVarArrayWithInitial(FontGlyph, &newAtlas->glyphs, font->arena, numGlyphsTotal);
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
			glyph->ttfGlyphIndex = stbtt_FindGlyphIndex(&font->ttfInfo, glyph->codepoint);
			// if (glyph->ttfGlyphIndex < 0) { PrintLine_D("Codepoint 0x%08X has ttfGlyphIndex %d", glyph->codepoint, glyph->ttfGlyphIndex); }
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
			glyph->logicalRec = NewRec(stbCharInfo->xoff, -newAtlas->maxAscend, (r32)glyph->atlasSourceRec.Width, newAtlas->maxAscend);
			if (glyph->logicalRec.Width == 0)
			{
				glyph->logicalRec.Width = glyph->advanceX;
			}
			// TODO: What are these floats for? stbCharInfo->xoff2 stbCharInfo->yoff2
		}
	}
	
	// Fill out glyph information for all custom glyphs
	customGlyphIndex = 0;
	for (uxx rIndex = 0; rIndex < numCustomGlyphRanges; rIndex++)
	{
		const CustomFontCharRange* customRange = &customGlyphRanges[rIndex];
		uxx numGlyphsInCustomRange = (customRange->endCodepoint - customRange->startCodepoint)+1;
		FontCharRange* newRange = VarArrayAdd(FontCharRange, &newAtlas->charRanges);
		ClearPointer(newRange);
		newRange->startCodepoint = customRange->startCodepoint;
		newRange->endCodepoint = customRange->endCodepoint;
		newRange->glyphArrayStartIndex = newAtlas->glyphs.length;
		
		FontGlyph* newGlyphs = VarArrayAddMulti(FontGlyph, &newAtlas->glyphs, numGlyphsInCustomRange);
		NotNull(newGlyphs);
		for (uxx gIndex = 0; gIndex < numGlyphsInCustomRange; gIndex++)
		{
			const CustomFontGlyph* customGlyph = &customRange->glyphs[gIndex];
			DebugAssert(customGlyphIndex < numGlyphsInCustomRanges);
			stbrp_rect* packedGlyphRec = &rects[numGlyphsInRanges + customGlyphIndex];
			FontGlyph* newGlyph = &newGlyphs[gIndex];
			ClearPointer(newGlyph);
			newGlyph->codepoint = customGlyph->codepoint;
			newGlyph->ttfGlyphIndex = INVALID_TTF_GLYPH_INDEX;
			newGlyph->atlasSourceRec = NewReci((i32)packedGlyphRec->x, (i32)packedGlyphRec->y, (i32)packedGlyphRec->w, (i32)packedGlyphRec->h);
			newGlyph->advanceX = (r32)newGlyph->atlasSourceRec.Width;
			newGlyph->renderOffset = NewV2(0, RoundR32(-newAtlas->maxAscend + (newAtlas->maxAscend + newAtlas->maxDescend)/2.0f - newGlyph->atlasSourceRec.Height/2.0f));
			newGlyph->logicalRec = NewRec(0, -newAtlas->maxAscend, (r32)newGlyph->atlasSourceRec.Width, newAtlas->maxAscend);
			customGlyphIndex++;
		}
	}
	
	ScratchEnd(scratch);
	
	return Result_Success;
}
PEXPI Result BakeFontAtlas(PigFont* font, r32 fontSize, u8 extraStyleFlags, v2i atlasSize, uxx numCharRanges, const FontCharRange* charRanges)
{
	return BakeFontAtlasEx(font, fontSize, extraStyleFlags, atlasSize, numCharRanges, charRanges, 0, nullptr);
}

PEXPI FontAtlas* GetDefaultFontAtlas(PigFont* font)
{
	NotNull(font);
	NotNull(font->arena);
	if (font->atlases.length > 0) { return VarArrayGetFirst(FontAtlas, &font->atlases); }
	else { return nullptr; }
}
PEXPI r32 GetDefaultFontSize(const PigFont* font)
{
	FontAtlas* defaultAtlas = GetDefaultFontAtlas((PigFont*)font);
	return (defaultAtlas != nullptr) ? defaultAtlas->fontSize : 0.0f;
}
PEXPI u8 GetDefaultFontStyleFlags(const PigFont* font)
{
	FontAtlas* defaultAtlas = GetDefaultFontAtlas((PigFont*)font);
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

//Pass 0 for codepoint to lookup and atlas without a particular glyph in mind
PEXP FontGlyph* GetFontGlyphForCodepoint(PigFont* font, u32 codepoint, r32 fontSize, u8 styleFlags, FontAtlas** atlasOut)
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
			if (codepoint == 0 || DoesFontAtlasContainCodepointEx(atlas, codepoint, &glyphIndex))
			{
				multipleMatches = (matchingAtlas != nullptr && AreSimilarR32(sizeDiff, matchingSizeDiff, DEFAULT_R32_TOLERANCE));
				matchingSizeDiff = sizeDiff;
				matchingAtlas = atlas;
				if (codepoint != 0) { result = VarArrayGetHard(FontGlyph, &atlas->glyphs, glyphIndex); }
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
					if (codepoint == 0 || DoesFontAtlasContainCodepointEx(atlas, codepoint, &glyphIndex))
					{
						multipleMatches = (matchingAtlas != nullptr && styleDiffs == matchingStyleDiffs);
						matchingStyleDiffs = styleDiffs;
						matchingAtlas = atlas;
						if (codepoint != 0) { result = VarArrayGetHard(FontGlyph, &atlas->glyphs, glyphIndex); }
					}
				}
			}
		}
		
		NotNull(matchingAtlas);
	}
	
	if (multipleMatches)
	{
		//TODO: If we still find more than one match, is there any other criteria we should check?
	}
	
	SetOptionalOutPntr(atlasOut, matchingAtlas);
	return result;
}

PEXPI FontAtlas* GetFontAtlas(PigFont* font, r32 fontSize, u8 styleFlags)
{
	FontAtlas* result = nullptr;
	GetFontGlyphForCodepoint(font, 0, fontSize, styleFlags, &result);
	return result;
}

PEXP r32 GetFontKerningBetweenGlyphs(const PigFont* font, r32 fontScale, const FontGlyph* leftGlyph, const FontGlyph* rightGlyph)
{
	NotNull(font);
	NotNull(leftGlyph);
	NotNull(rightGlyph);
	if (font->kerningTable.numEntries == 0) { return 0.0f; }
	if (leftGlyph->ttfGlyphIndex < 0 || rightGlyph->ttfGlyphIndex < 0) { return 0.0f; }
	
	//TODO: We should do a binary search here to speed things up!
	for (uxx eIndex = 0; eIndex < font->kerningTable.numEntries; eIndex++)
	{
		const FontKerningTableEntry* entry = &font->kerningTable.entries[eIndex];
		if (entry->leftTtfGlyphIndex == (uxx)leftGlyph->ttfGlyphIndex && entry->rightTtfGlyphIndex == (uxx)rightGlyph->ttfGlyphIndex)
		{
			return entry->value * fontScale;
		}
	}
	
	return 0.0f;
}
PEXP r32 GetFontKerningBetweenCodepoints(const PigFont* font, r32 fontSize, u8 styleFlags, u32 leftCodepoint, u32 rightCodepoint)
{
	NotNull(font);
	
	FontAtlas* leftGlyphAtlas = nullptr;
	FontGlyph* leftGlyph = GetFontGlyphForCodepoint((PigFont*)font, leftCodepoint, fontSize, styleFlags, &leftGlyphAtlas);
	if (leftGlyph == nullptr || leftGlyphAtlas == nullptr) { return 0.0f; }
	if (leftGlyph->ttfGlyphIndex < 0) { return 0.0f; }
	
	FontAtlas* rightGlyphAtlas = nullptr;
	FontGlyph* rightGlyph = GetFontGlyphForCodepoint((PigFont*)font, rightCodepoint, fontSize, styleFlags, &rightGlyphAtlas);
	if (rightGlyph == nullptr || rightGlyphAtlas == nullptr) { return 0.0f; }
	if (rightGlyph->ttfGlyphIndex < 0) { return 0.0f; }
	
	if (leftGlyphAtlas->fontScale != rightGlyphAtlas->fontScale) { return 0.0f; }
	return GetFontKerningBetweenGlyphs(font, leftGlyphAtlas->fontScale, leftGlyph, rightGlyph);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _GFX_FONT_H

#if defined(_OS_FONT_H) && defined(_GFX_FONT_H)
#include "cross/cross_os_font_and_gfx_font.h"
#endif
