/*
File:   gfx_font.h
Author: Taylor Robbins
Date:   02\05\2025
Description:
	** A "PigFont" is a data structure built on top of possibly multiple Texture atlases
	** which are packed with rasterized glyphs by stb_truetype.h
	
	** FreeType vs stb_truetype.h: This file will use stb_truetype.h to parse .ttf files and rasterize
	** glyphs by default. But if BUILD_WITH_FREETYPE is enabled we will use FreeType instead. FreeType
	** is a little larger and more complicated than stb_truetype.h so it can be a little more annoying
	** to port to some platforms but it also has better support for various font file formats (like .otf)
	** and it actually produces better rasterizations of glyphs than stb_truetype.h so it is preferred
	** when the FreeType dependency is not a liability. Having support for these two libraries means this
	** file contains 2 codepaths for a lot of logic, making the file a bit larger and also allowing for
	** bugs to creep in to one codepath or the other if we don't utilize both often. For this reason we
	** should always test both code paths when working on font rasterization\flow logic.
	
	** "Active" Fonts: When we want to support rendering a wide variety of text we don't really have the
	** option of pre-rasterizing a set of glyphs to cover all our use cases. Rather than doing font
	** rasterization at initialization time into a fixed number atlases and then dropping the font
	** file, we need to attach one or more font files and leave them attached for the duration of the
	** program. Then when we try to render a particular codepoint we need to find an active atlas and
	** rasterize the glyph into it, or make a new active atlas if one doesn't exist already. This is
	** what we call an "active" font. Note that an "active" font can still have non-active atlases that
	** were pre-baked at initialization time. This allows us to take advantage of some amount of prior
	** knowledge (we almost always are rendering simple latin characters so might as well bake those
	** up-front if we know the size\style beforehand).
	** NOTE: In order for our caching algorithm to work properly we need a programTime to be passed in
	** so we can record when each active glyph\atlas was last accessed and we can evict the oldest ones
	** when we are running out of space. If the font being used is not active, then the programTime argument
	** can be left as 0 for any API that takes it.
	** NOTE: When using an active font you must call FontNewFrame at the beginning of the frame and
	** CommitAllFontTextureUpdates at the end. This is because sokol_gfx.h restricts us to one texture
	** update per-frame for mutable textures. To get around this we defer texture updates until a Commit
	** is requested (which happens in GfxSystem_FontFlowDrawCharCallback) and then any more update that
	** frame will be deferred until the first commit next frame.
*/

//TODO: Move programTime passing to FontNewFrame
//TODO: Choose cell size based on font metrics
//TODO: Allow layout code to ask for font metrics like lineHeight without baking an atlas
//TODO: Evict old glyphs when out of space
//TODO: Evict old atlases when out of use
//TODO: Adhere to atlas limit

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
#include "base/base_unicode.h"
#include "base/base_debug_output.h"
#include "misc/misc_freetype_include.h"

//TODO: Eventually we may want to support using Font stuff in Raylib! That would require making a gfx_texture implementation for Raylib first so we aren't doing that for now
#if BUILD_WITH_SOKOL_GFX

#if PIG_CORE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION
#endif
#define STBRP_SORT                          qsort //TODO: Do we want to route this to one of our own sorting functions?
#define STBRP_ASSERT(condition)             Assert(condition)
#include "third_party/stb/stb_rect_pack.h"

#if !BUILD_WITH_FREETYPE
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
#if COMPILER_IS_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough" //warning: unannotated fall-through between switch labels
#endif
#include "third_party/stb/stb_truetype.h"
#if COMPILER_IS_MSVC
#pragma warning(pop)
#endif
#if COMPILER_IS_CLANG
#pragma clang diagnostic pop
#endif
#endif //!BUILD_WITH_FREETYPE

typedef plex FontCharRange FontCharRange;
plex FontCharRange
{
	u32 startCodepoint;
	u32 endCodepoint;
	uxx glyphArrayStartIndex;
};

typedef plex CustomFontGlyph CustomFontGlyph;
plex CustomFontGlyph
{
	u32 codepoint;
	ImageData imageData;
	reci sourceRec;
};
typedef plex CustomFontCharRange CustomFontCharRange;
plex CustomFontCharRange
{
	u32 startCodepoint;
	u32 endCodepoint;
	CustomFontGlyph* glyphs;
};

#define INVALID_TTF_GLYPH_INDEX INT32_MAX

typedef plex FontGlyph FontGlyph;
plex FontGlyph
{
	u32 codepoint; //for active atlases, a codepoint of 0 means an empty glyph slot
	i32 ttfGlyphIndex;
	reci atlasSourceRec;
	r32 advanceX;
	v2 renderOffset;
	rec logicalRec;
};

typedef plex FontActiveCell FontActiveCell;
plex FontActiveCell
{
	uxx codepoint; //also acts as filled indicator with 0 meaning unfilled
	uxx glyphIndex;
	u64 lastUsedTime;
};

typedef plex FontActiveAtlasTextureUpdate FontActiveAtlasTextureUpdate;
plex FontActiveAtlasTextureUpdate
{
	v2i sourcePos;
	ImageData imageData;
};

typedef plex FontAtlas FontAtlas;
plex FontAtlas
{
	r32 fontSize;
	r32 fontScale; //only used when asking stbtt for size-independent metrics
	u8 styleFlags;
	VarArray charRanges; //FontCharRange
	FontCharRange glyphRange;
	VarArray glyphs; //FontGlyph
	Texture texture;
	r32 lineHeight;
	r32 maxAscend;
	r32 maxDescend;
	r32 centerOffset;
	
	bool isActive;
	u64 lastUsedTime;
	v2i activeCellSize;
	v2i activeCellGridSize;
	FontActiveCell* cells;
	bool pushedTextureUpdates;
	VarArray pendingTextureUpdates; //FontActiveAtlasTextureUpdate
};

typedef plex FontKerningTableEntry FontKerningTableEntry;
plex FontKerningTableEntry
{
	u32 leftTtfGlyphIndex;
	u32 rightTtfGlyphIndex;
	r32 value; //must be multipled by fontScale
};

typedef plex FontKerningTable FontKerningTable;
plex FontKerningTable
{
	uxx numEntries;
	FontKerningTableEntry* entries;
};

typedef plex FontFile FontFile;
plex FontFile
{
	Str8 nameOrPath;
	u8 styleFlags;
	bool inFontArena;
	Slice fileContents;
	#if BUILD_WITH_FREETYPE
	FT_Face freeTypeFace;
	#else
	stbtt_fontinfo ttfInfo;
	#endif
};

#define FONT_MAX_FONT_FILES 8 //maximum number of fallback fonts that can be attached at the same time to a single font

//NOTE: We have a naming conflict with raylib.h if we name this "PigFont" so we are
// naming it PigFont, but typedefing PigFont in non-raylib projects so anything outside
// PigCore that doesn't plan to use Raylib can still use the name "PigFont"
typedef plex PigFont PigFont;
plex PigFont
{
	Arena* arena;
	Str8 name;
	
	bool isActive; //This indicates that the font is allowed to create new active atlases and add/remove glyphs from active atlases as needed
	i32 activeAtlasMinSize;
	i32 activeAtlasMaxSize;
	uxx activeMaxNumAtlases;
	
	uxx numFiles;
	FontFile files[FONT_MAX_FONT_FILES];
	
	VarArray atlases; //FontAtlas
	FontKerningTable kerningTable;
};

//NOTE: Font is defined in Xlib.h as a typedef of XID
#if !BUILD_WITH_RAYLIB && !TARGET_IS_LINUX
typedef PigFont Font;
#endif

#if BUILD_WITH_FREETYPE
#if !PIG_CORE_IMPLEMENTATION
extern FT_Library FreeTypeLib;
#else
FT_Library FreeTypeLib = nullptr;
#endif
#endif //BUILD_WITH_FREETYPE

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeFontAtlas(PigFont* font, FontAtlas* atlas);
	PIG_CORE_INLINE void FreeFontKerningTable(Arena* arena, FontKerningTable* kerningTable);
	PIG_CORE_INLINE void RemoveAttachedFontFile(PigFont* font, uxx index);
	PIG_CORE_INLINE void RemoveAttachedFontFiles(PigFont* font);
	void FreeFont(PigFont* font);
	void ClearFontAtlases(PigFont* font);
	PigFont InitFont(Arena* arena, Str8 name);
	PIG_CORE_INLINE void MakeFontActive(PigFont* font, i32 minAtlasSize, i32 maxAtlasSize, uxx maxNumAtlases);
	PIG_CORE_INLINE FontCharRange NewFontCharRangeSingle(u32 codepoint);
	PIG_CORE_INLINE FontCharRange NewFontCharRange(u32 startCodepoint, u32 endCodepoint);
	PIG_CORE_INLINE FontCharRange NewFontCharRangeLength(u32 startCodepoint, u32 numCodepoints);
	PIG_CORE_INLINE CustomFontCharRange NewCustomFontCharRangeSingle(CustomFontGlyph* glyph);
	PIG_CORE_INLINE CustomFontCharRange NewCustomFontCharRange(uxx numGlyphs, CustomFontGlyph* glyph);
	Result TryAttachFontFile(PigFont* font, Str8 nameOrPath, Slice fileContents, u8 styleFlags, bool copyIntoFontArena);
	void FillFontKerningTable(PigFont* font);
	Result BakeFontAtlasEx(PigFont* font, r32 fontSize, u8 extraStyleFlags, i32 minAtlasSize, i32 maxAtlasSize, uxx numCharRanges, const FontCharRange* charRanges, uxx numCustomGlyphRanges, const CustomFontCharRange* customGlyphRanges);
	PIG_CORE_INLINE Result BakeFontAtlas(PigFont* font, r32 fontSize, u8 extraStyleFlags, i32 minAtlasSize, i32 maxAtlasSize, uxx numCharRanges, const FontCharRange* charRanges);
	Result MultiBakeFontAtlasesEx(PigFont* font, uxx numSizes, const r32* fontSizes, u8 extraStyleFlags, i32 minAtlasSize, i32 maxAtlasSize, uxx numCharRanges, const FontCharRange* charRanges, uxx numCustomGlyphRanges, const CustomFontCharRange* customGlyphRanges);
	PIG_CORE_INLINE Result MultiBakeFontAtlases(PigFont* font, uxx numSizes, const r32* fontSizes, u8 extraStyleFlags, i32 minAtlasSize, i32 maxAtlasSize, uxx numCharRanges, const FontCharRange* charRanges);
	PIG_CORE_INLINE FontAtlas* GetDefaultFontAtlas(PigFont* font);
	PIG_CORE_INLINE r32 GetDefaultFontSize(const PigFont* font);
	PIG_CORE_INLINE u8 GetDefaultFontStyleFlags(const PigFont* font);
	PIG_CORE_INLINE bool DoesFontAtlasContainCodepointEx(const FontAtlas* atlas, u32 codepoint, uxx* glyphIndexOut);
	PIG_CORE_INLINE bool DoesFontAtlasContainCodepoint(const FontAtlas* atlas, u32 codepoint);
	FontFile* FindFontFileForCodepoint(PigFont* font, u32 codepoint, r32 fontSize, u8 styleFlags, unsigned int* glyphIndexOut, uxx* fileIndexOut);
	FontAtlas* AddNewActiveAtlas(PigFont* font, u64 programTime, FontFile* fontFile, r32 fontSize, u8 styleFlags);
	void ResizeActiveFontAtlas(PigFont* font, FontAtlas* activeAtlas, v2i newSize);
	FontGlyph* TryAddGlyphToActiveFontAtlas(PigFont* font, u64 programTime, FontFile* fontFile, FontAtlas* activeAtlas, u32 codepoint);
	FontGlyph* GetFontGlyphForCodepoint(PigFont* font, u64 programTime, u32 codepoint, r32 fontSize, u8 styleFlags, bool allowActiveAtlasCreation, FontAtlas** atlasOut);
	PIG_CORE_INLINE FontAtlas* GetFontAtlas(PigFont* font, u64 programTime, r32 fontSize, u8 styleFlags, bool allowActiveAtlasCreation);
	void CommitFontAtlasTextureUpdates(PigFont* font, FontAtlas* activeAtlas);
	PIG_CORE_INLINE void CommitAllFontTextureUpdates(PigFont* font);
	PIG_CORE_INLINE void FontNewFrame(PigFont* font);
	r32 GetFontKerningBetweenGlyphs(const PigFont* font, r32 fontScale, const FontGlyph* leftGlyph, const FontGlyph* rightGlyph);
	r32 GetFontKerningBetweenCodepoints(const PigFont* font, u64 programTime, r32 fontSize, u8 styleFlags, u32 leftCodepoint, u32 rightCodepoint, bool allowActiveAtlasCreation);
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define FontCharRange_ASCII                 NewFontCharRange(UNICODE_PRINTABLE_ASCII_START, UNICODE_PRINTABLE_ASCII_COUNT)
#define FontCharRange_LatinSupplementAccent NewFontCharRangeLength(UNICODE_LATIN1_SUPPLEMENT_ACCENT_START, UNICODE_LATIN1_SUPPLEMENT_ACCENT_COUNT)
#define FontCharRange_LatinExtA             NewFontCharRangeLength(UNICODE_LATIN_EXT_A_START, UNICODE_LATIN_EXT_A_COUNT)
#define FontCharRange_Cyrillic              NewFontCharRangeLength(UNICODE_CYRILLIC_START, UNICODE_CYRILLIC_COUNT)
#define FontCharRange_Hiragana              NewFontCharRangeLength(UNICODE_HIRAGANA_START, UNICODE_HIRAGANA_COUNT)
#define FontCharRange_Katakana              NewFontCharRangeLength(UNICODE_KATAKANA_START, UNICODE_KATAKANA_COUNT)

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
	if (atlas->isActive)
	{
		if (atlas->cells != nullptr)
		{
			FreeArray(FontActiveCell, font->arena, (uxx)(atlas->activeCellGridSize.Width * atlas->activeCellGridSize.Height), atlas->cells);
		}
		VarArrayLoop(&atlas->pendingTextureUpdates, uIndex)
		{
			VarArrayLoopGet(FontActiveAtlasTextureUpdate, update, &atlas->pendingTextureUpdates, uIndex);
			FreeImageData(font->arena, &update->imageData);
		}
		FreeVarArray(&atlas->pendingTextureUpdates);
	}
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

PEXPI void RemoveAttachedFontFile(PigFont* font, uxx index)
{
	NotNull(font);
	NotNull(font->arena);
	Assert(index < font->numFiles);
	FontFile* file = &font->files[index];
	if (file->inFontArena && file->fileContents.chars != nullptr) { FreeStr8(font->arena, &file->fileContents); }
	#if BUILD_WITH_FREETYPE
	if (file->freeTypeFace != nullptr)
	{
		FT_Error doneError = FT_Done_Face(file->freeTypeFace);
		Assert(doneError == 0);
	}
	#endif
	ClearPointer(file);
	if (index+1 < font->numFiles);
	{
		MyMemCopy(&font->files[index], &font->files[index+1], (font->numFiles - (index+1)) * sizeof(FontFile));
	}
	font->numFiles--;
}
PEXPI void RemoveAttachedFontFiles(PigFont* font)
{
	for (uxx fIndex = font->numFiles; fIndex > 0; fIndex--)
	{
		RemoveAttachedFontFile(font, fIndex-1);
	}
}

PEXP void FreeFont(PigFont* font)
{
	NotNull(font);
	if (font->arena != nullptr)
	{
		FreeStr8(font->arena, &font->name);
		RemoveAttachedFontFiles(font);
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
	result.isActive = false;
	return result;
}

PEXPI void MakeFontActive(PigFont* font, i32 minAtlasSize, i32 maxAtlasSize, uxx maxNumAtlases)
{
	NotNull(font);
	NotNull(font->arena);
	Assert(minAtlasSize > 0);
	Assert(maxAtlasSize > 0);
	Assert(minAtlasSize <= maxAtlasSize);
	font->isActive = true;
	font->activeAtlasMinSize = minAtlasSize;
	font->activeAtlasMaxSize = maxAtlasSize;
	font->activeMaxNumAtlases = maxNumAtlases;
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

PEXP Result TryAttachFontFile(PigFont* font, Str8 nameOrPath, Slice fileContents, u8 styleFlags, bool copyIntoFontArena)
{
	NotNull(font);
	NotNull(font->arena);
	NotNullStr(nameOrPath);
	NotNullStr(fileContents);
	NotEmptyStr(fileContents);
	if (font->numFiles >= FONT_MAX_FONT_FILES) { return Result_TooMany; }
	FontFile* newFile = &font->files[font->numFiles];
	ClearPointer(newFile);
	newFile->nameOrPath = AllocStr8(font->arena, nameOrPath);
	newFile->styleFlags = styleFlags;
	newFile->fileContents = copyIntoFontArena ? AllocStr8(font->arena, fileContents) : fileContents;
	newFile->inFontArena = copyIntoFontArena;
	
	Result result = Result_None;
	#if BUILD_WITH_FREETYPE
	do
	{
		if (FreeTypeLib == nullptr)
		{
			TracyCZoneN(_InitFreeType, "FT_Init_FreeType", true);
			FT_Error initError = FT_Init_FreeType(&FreeTypeLib);
			TracyCZoneEnd(_InitFreeType);
			if (initError != 0)
			{
				DebugAssertMsg(initError == 0, "Failed to initialize FreeType library!");
				result = Result_InitFailed;
				break;
			}
			NotNull(FreeTypeLib);
		}
		
		FT_Open_Args freeTypeFaceArgs = ZEROED;
		freeTypeFaceArgs.flags = FT_OPEN_MEMORY;
		freeTypeFaceArgs.memory_base = newFile->fileContents.pntr;
		freeTypeFaceArgs.memory_size = (FT_Long)newFile->fileContents.length;
		TracyCZoneN(_FreeTypeOpenFace, "FT_Open_Face", true);
		FT_Error openError = FT_Open_Face(FreeTypeLib, &freeTypeFaceArgs, 0, &newFile->freeTypeFace);
		TracyCZoneEnd(_FreeTypeOpenFace);
		if (openError != 0)
		{
			PrintLine_E("FreeType file parsing error (%llu byte file): %s", newFile->fileContents.length, FT_Error_String(openError));
			DebugAssertMsg(openError == 0, "Failed to parse font file with FreeType!");
			result = Result_ParsingFailure;
			break;
		}
		NotNull(newFile->freeTypeFace);
		
		#if 0
		PrintLine_D("Scanning font file \"%.*s\" %s%s for available chars:", StrPrint(nameOrPath), IsFlagSet(styleFlags, FontStyleFlag_Bold) ? "Bold" : "", IsFlagSet(styleFlags, FontStyleFlag_Italic) ? "Italic" : "");
		FT_ULong rangeStart = 0;
		FT_ULong previousCharCode = 0;
		FT_ULong charCode = 0;
		FT_UInt glyphIndex = 0;
		charCode = FT_Get_First_Char(newFile->freeTypeFace, &glyphIndex);
		while (glyphIndex != 0)
		{
			if (charCode != previousCharCode+1)
			{
				PrintLine_D("\t[0x%08X-0x%08X] (%u chars)", rangeStart, previousCharCode, (previousCharCode+1) - rangeStart);
				rangeStart = charCode;
			}
			previousCharCode = charCode;
			
			// if (charCode >= 0x20 && charCode <= 0x7E) { PrintLine_D("\tContains 0x%08X \'%c\'", charCode, (char)charCode); }
			// else { PrintLine_D("\tContains 0x%08X", charCode); }
			charCode = FT_Get_Next_Char(newFile->freeTypeFace, charCode, &glyphIndex);
		}
		if (rangeStart != previousCharCode) { PrintLine_D("\t[0x%08X-0x%08X] (%u chars)", rangeStart, previousCharCode, (previousCharCode+1) - rangeStart); }
		#endif
		
	} while(false);
	#else //!BUILD_WITH_FREETYPE
	{
		int firstFontOffset = stbtt_GetFontOffsetForIndex(newFile->fileContents.bytes, 0);
		Assert(firstFontOffset >= 0); //TODO: Turn this into an error?
		int initFontResult = stbtt_InitFont(&newFile->ttfInfo, newFile->fileContents.bytes, firstFontOffset);
		Assert(initFontResult != 0); //TODO: Turn this into an error?
		// int numOfFontsInTtf = stbtt_GetNumberOfFonts(newFile->fileContents.bytes);
		// PrintLine_D("There %s %d font%s in this ttf file", PluralEx(numOfFontsInTtf, "is", "are"), numOfFontsInTtf, Plural(numOfFontsInTtf, "s"));
	}
	#endif //BUILD_WITH_FREETYPE
	
	if (result == Result_None)
	{
		result = Result_Success;
		font->numFiles++;
	}
	else
	{
		FreeStr8(font->arena, &newFile->nameOrPath);
		if (copyIntoFontArena) { FreeStr8(font->arena, &newFile->fileContents); }
	}
	return result;
}

PEXP void FillFontKerningTable(PigFont* font)
{
	NotNull(font);
	NotNull(font->arena);
	Assert(font->numFiles > 0);
	FontFile* fontFile = &font->files[0];
	#if BUILD_WITH_FREETYPE
	UNUSED(fontFile);
	//TODO: FreeType support for Kerning? Should we brute-force find all combinations of kerning values?
	#else
	{
		FreeFontKerningTable(font->arena, &font->kerningTable);
		
		int tableLength = stbtt_GetKerningTableLength(&fontFile->ttfInfo);
		Assert(tableLength >= 0);
		if (tableLength == 0) { return; }
		
		ScratchBegin1(scratch, font->arena);
		
		stbtt_kerningentry* stbEntries = AllocArray(stbtt_kerningentry, scratch, (uxx)tableLength);
		NotNull(stbEntries);
		int getResult = stbtt_GetKerningTable(&fontFile->ttfInfo, stbEntries, tableLength);
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
	#endif
}

PEXP Result BakeFontAtlasEx(PigFont* font, r32 fontSize, u8 extraStyleFlags, i32 minAtlasSize, i32 maxAtlasSize, uxx numCharRanges, const FontCharRange* charRanges, uxx numCustomGlyphRanges, const CustomFontCharRange* customGlyphRanges)
{
	NotNull(font);
	NotNull(font->arena);
	Assert(minAtlasSize > 0 && maxAtlasSize > 0);
	Assert(numCharRanges > 0);
	NotNull(charRanges);
	TracyCZoneN(_funcZone, "BakeFontAtlasEx", true);
	ScratchBegin1(scratch, font->arena);
	Assert(numCustomGlyphRanges == 0 || customGlyphRanges != nullptr);
	Result result = Result_None;
	
	Assert(font->numFiles > 0);
	FontFile* fontFile = &font->files[0];
	
	u32 minCodepoint = UINT32_MAX;
	u32 maxCodepoint = 0;
	uxx numCodepointsInCharRanges = 0;
	uxx numCodepointsInCustomRanges = 0;
	for (uxx rIndex = 0; rIndex < numCharRanges; rIndex++)
	{
		const FontCharRange* charRange = &charRanges[rIndex];
		Assert(charRange->endCodepoint >= charRange->startCodepoint);
		numCodepointsInCharRanges += (charRange->endCodepoint - charRange->startCodepoint)+1;
		minCodepoint = MinU32(minCodepoint, charRange->startCodepoint);
		maxCodepoint = MaxU32(maxCodepoint, charRange->endCodepoint);
	}
	for (uxx rIndex = 0; rIndex < numCustomGlyphRanges; rIndex++)
	{
		const CustomFontCharRange* charRange = &customGlyphRanges[rIndex];
		Assert(charRange->endCodepoint >= charRange->startCodepoint);
		NotNull(charRange->glyphs);
		numCodepointsInCustomRanges += (charRange->endCodepoint - charRange->startCodepoint)+1;
		minCodepoint = MinU32(minCodepoint, charRange->startCodepoint);
		maxCodepoint = MaxU32(maxCodepoint, charRange->endCodepoint);
	}
	uxx numCodepointsTotal = numCodepointsInCharRanges + numCodepointsInCustomRanges;
	
	#if BUILD_WITH_FREETYPE
	
	do
	{
		NotNull(fontFile->freeTypeFace);
		const int packingPadding = 1; //px
		
		FT_F26Dot6 freeTypeFontSize = TO_FT26_FROM_R32(fontSize);
		const u32 freeTypeFontDpi = 72;
		FT_Error setCharSizeError = FT_Set_Char_Size(fontFile->freeTypeFace, freeTypeFontSize, freeTypeFontSize, freeTypeFontDpi, freeTypeFontDpi);
		Assert(setCharSizeError == 0);
		
		uxx numGlyphsInAtlas = 0;
		stbrp_rect* packRects = AllocArray(stbrp_rect, scratch, numCodepointsTotal);
		NotNull(packRects);
		{
			uxx packedRecIndex = 0;
			for (uxx rIndex = 0; rIndex < numCharRanges; rIndex++)
			{
				const FontCharRange* charRange = &charRanges[rIndex];
				for (u32 codepoint = charRange->startCodepoint; codepoint <= charRange->endCodepoint; codepoint++)
				{
					//TODO: Fill in a packRect using information from FreeType about a particular codepoint
					FT_UInt glyphIndex = FT_Get_Char_Index(fontFile->freeTypeFace, codepoint);
					if (glyphIndex == 0)
					{
						PrintLine_E("Font doesn't contain glyph for codepoint 0x%08X!", codepoint);
						DebugAssert(glyphIndex != 0);
						result = Result_NotFound;
						break;
					}
					FT_Error loadGlyphError = FT_Load_Glyph(fontFile->freeTypeFace, glyphIndex, FT_LOAD_DEFAULT); //TODO: Use FT_LOAD_COLOR for colored emoji! Also check FT_HAS_COLOR(face)
					if (loadGlyphError != 0)
					{
						PrintLine_E("Failed to load glyph for codepoint 0x%08X: %s", codepoint, loadGlyphError);
						DebugAssert(loadGlyphError == 0);
						result = Result_InvalidCharacter;
						break;
					}
					NotNull(fontFile->freeTypeFace->glyph);
					
					if (fontFile->freeTypeFace->glyph->metrics.width > 0 && fontFile->freeTypeFace->glyph->metrics.height > 0)
					{
						DebugAssert(packedRecIndex < numCodepointsTotal);
						packRects[packedRecIndex].w = TO_I32_FROM_FT26(fontFile->freeTypeFace->glyph->metrics.width) + packingPadding*2;
						packRects[packedRecIndex].h = TO_I32_FROM_FT26(fontFile->freeTypeFace->glyph->metrics.height) + packingPadding*2;
						// PrintLine_D("Codepoint U+%X is %dx%d glyph at %g (%d)", codepoint, packRects[packedRecIndex].w - packingPadding, packRects[packedRecIndex].h - packingPadding, fontSize, freeTypeFontSize);
						packedRecIndex++;
					}
				}
			}
			if (result != Result_None) { break; }
			for (uxx rIndex = 0; rIndex < numCustomGlyphRanges; rIndex++)
			{
				const CustomFontCharRange* charRange = &customGlyphRanges[rIndex];
				NotNull(charRange->glyphs);
				uxx numGlyphsInCustomRange = (charRange->endCodepoint - charRange->startCodepoint)+1;
				for (uxx gIndex = 0; gIndex < numGlyphsInCustomRange; gIndex++)
				{
					const CustomFontGlyph* customGlyph = &charRange->glyphs[gIndex];
					DebugAssert(customGlyph->imageData.size.Width > 0 && customGlyph->imageData.size.Height > 0);
					
					if (customGlyph->imageData.size.Width > 0 && customGlyph->imageData.size.Height > 0)
					{
						DebugAssert(packedRecIndex < numCodepointsTotal);
						packRects[packedRecIndex].w = (int)customGlyph->imageData.size.Width + packingPadding*2;
						packRects[packedRecIndex].h = (int)customGlyph->imageData.size.Height + packingPadding*2;
						packedRecIndex++;
					}
				}
			}
			DebugAssert(packedRecIndex <= numCodepointsTotal);
			numGlyphsInAtlas = packedRecIndex;
		}
		
		bool packedSuccessfully = false;
		i32 atlasSideLength = minAtlasSize;
		while (atlasSideLength <= maxAtlasSize)
		{
			uxx scratchMark = ArenaGetMark(scratch);
			uxx numPackNodes = (uxx)atlasSideLength;
			stbrp_node* packNodes = AllocArray(stbrp_node, scratch, numPackNodes);
			stbrp_context packContext = ZEROED;
			stbrp_init_target(&packContext, (int)atlasSideLength, (int)atlasSideLength, packNodes, (int)numPackNodes);
			int packResult = stbrp_pack_rects(&packContext, packRects, (int)numGlyphsInAtlas);
			ArenaResetToMark(scratch, scratchMark);
			if (packResult == 1) { packedSuccessfully = true; break; }
			atlasSideLength *= 2;
		}
		if (!packedSuccessfully) { result = Result_NotEnoughSpace; break; }
		
		v2i atlasSize = FillV2i(atlasSideLength);
		uxx numPixels = (uxx)(atlasSize.Width * atlasSize.Height);
		Color32* pixelsPntr = AllocArray(Color32, scratch, numPixels);
		NotNull(pixelsPntr);
		MyMemSet(pixelsPntr, 0x00, sizeof(Color32) * numPixels);
		
		FontAtlas* newAtlas = VarArrayAdd(FontAtlas, &font->atlases);
		NotNull(newAtlas);
		ClearPointer(newAtlas);
		newAtlas->fontSize = fontSize;
		newAtlas->fontScale = 1.0f; //TODO: Can we get this from FreeType? Do we need it (without kerning)?
		newAtlas->styleFlags = (fontFile->styleFlags | extraStyleFlags);
		newAtlas->glyphRange.startCodepoint = minCodepoint;
		newAtlas->glyphRange.endCodepoint = maxCodepoint;
		newAtlas->maxAscend = TO_R32_FROM_FT26(fontFile->freeTypeFace->size->metrics.ascender);
		newAtlas->maxDescend = TO_R32_FROM_FT26(fontFile->freeTypeFace->size->metrics.descender);
		newAtlas->lineHeight = TO_R32_FROM_FT26(fontFile->freeTypeFace->size->metrics.height);
		newAtlas->centerOffset = newAtlas->maxAscend - (newAtlas->lineHeight / 2.0f); //TODO: Fill the centerOffset using the W measure method that we did below?
		InitVarArrayWithInitial(FontCharRange, &newAtlas->charRanges, font->arena, numCharRanges + numCustomGlyphRanges);
		InitVarArrayWithInitial(FontGlyph, &newAtlas->glyphs, font->arena, numCodepointsTotal);
		
		{
			uxx packedRecIndex = 0;
			for (uxx rIndex = 0; rIndex < numCharRanges; rIndex++)
			{
				const FontCharRange* charRange = &charRanges[rIndex];
				
				FontCharRange* newCharRange = VarArrayAdd(FontCharRange, &newAtlas->charRanges);
				NotNull(newCharRange);
				ClearPointer(newCharRange);
				newCharRange->startCodepoint = charRange->startCodepoint;
				newCharRange->endCodepoint = charRange->endCodepoint;
				newCharRange->glyphArrayStartIndex = newAtlas->glyphs.length;
				
				for (u32 codepoint = charRange->startCodepoint; codepoint <= charRange->endCodepoint; codepoint++)
				{
					//TODO: Fill in a packRect using information from FreeType about a particular codepoint
					FT_UInt glyphIndex = FT_Get_Char_Index(fontFile->freeTypeFace, codepoint);
					Assert(glyphIndex != 0);
					FT_Error loadGlyphError = FT_Load_Glyph(fontFile->freeTypeFace, glyphIndex, FT_LOAD_DEFAULT); //TODO: Use FT_LOAD_COLOR for colored emoji! Also check FT_HAS_COLOR(face)
					Assert(loadGlyphError == 0);
					NotNull(fontFile->freeTypeFace->glyph);
					FT_Error renderGlyphError = FT_Render_Glyph(fontFile->freeTypeFace->glyph, FT_RENDER_MODE_NORMAL);
					Assert(renderGlyphError == 0);
					
					FontGlyph* newGlyph = VarArrayAdd(FontGlyph, &newAtlas->glyphs);
					NotNull(newGlyph);
					ClearPointer(newGlyph);
					newGlyph->codepoint = codepoint;
					newGlyph->ttfGlyphIndex = glyphIndex;
					newGlyph->advanceX = TO_R32_FROM_FT26(fontFile->freeTypeFace->glyph->advance.x);
					newGlyph->renderOffset.X = (r32)fontFile->freeTypeFace->glyph->bitmap_left;
					newGlyph->renderOffset.Y = -(r32)fontFile->freeTypeFace->glyph->bitmap_top;
					newGlyph->logicalRec = NewRec(0, -newAtlas->maxAscend, newGlyph->advanceX, newAtlas->maxAscend);
					
					if (fontFile->freeTypeFace->glyph->bitmap.width > 0 && fontFile->freeTypeFace->glyph->bitmap.rows > 0)
					{
						Assert(packedRecIndex < numGlyphsInAtlas);
						stbrp_rect packedRec = packRects[packedRecIndex];
						packedRecIndex++;
						Assert(packedRec.was_packed);
						
						Assert(fontFile->freeTypeFace->glyph->bitmap.width == (unsigned int)packedRec.w - packingPadding*2);
						Assert(fontFile->freeTypeFace->glyph->bitmap.rows == (unsigned int)packedRec.h - packingPadding*2);
						Assert(fontFile->freeTypeFace->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY);
						
						newGlyph->atlasSourceRec = NewReci(packedRec.x + packingPadding, packedRec.y + packingPadding, packedRec.w - packingPadding*2, packedRec.h - packingPadding*2);
						newGlyph->logicalRec.Width = MaxR32(newGlyph->renderOffset.X + (r32)newGlyph->atlasSourceRec.Width, newGlyph->advanceX);
						
						// PrintLine_D("Codepoint U+%X is %dx%d offset=(%g, %g) advance=%g", codepoint, newGlyph->atlasSourceRec.Width, newGlyph->atlasSourceRec.Height, newGlyph->renderOffset.X, newGlyph->renderOffset.Y, newGlyph->advanceX);
						// PrintLine_D("Codepoint U+%X packed (%d, %d, %d, %d) in %dx%d atlas[%llu]",
						// 	codepoint,
						// 	newGlyph->atlasSourceRec.X, newGlyph->atlasSourceRec.Y, newGlyph->atlasSourceRec.Width, newGlyph->atlasSourceRec.Height,
						// 	atlasSize.Width, atlasSize.Height,
						// 	font->atlases.length-1
						// );
						
						for (int yOffset = 0; yOffset < packedRec.h - packingPadding*2; yOffset++)
						{
							for (int xOffset = 0; xOffset < packedRec.w - packingPadding*2; xOffset++)
							{
								u8 inValue = fontFile->freeTypeFace->glyph->bitmap.buffer[INDEX_FROM_COORD2D(xOffset, yOffset, fontFile->freeTypeFace->glyph->bitmap.pitch, fontFile->freeTypeFace->glyph->bitmap.height)];
								Color32* outPixel = &pixelsPntr[INDEX_FROM_COORD2D(packedRec.x + packingPadding + xOffset, packedRec.y + packingPadding + yOffset, atlasSize.Width, atlasSize.Height)];
								outPixel->r = 255;
								outPixel->g = 255;
								outPixel->b = 255;
								outPixel->a = inValue;
							}
						}
					}
				}
			}
			for (uxx rIndex = 0; rIndex < numCustomGlyphRanges; rIndex++)
			{
				const CustomFontCharRange* charRange = &customGlyphRanges[rIndex];
				NotNull(charRange->glyphs);
				
				FontCharRange* newCharRange = VarArrayAdd(FontCharRange, &newAtlas->charRanges);
				NotNull(newCharRange);
				ClearPointer(newCharRange);
				newCharRange->startCodepoint = charRange->startCodepoint;
				newCharRange->endCodepoint = charRange->endCodepoint;
				newCharRange->glyphArrayStartIndex = newAtlas->glyphs.length;
				
				uxx numGlyphsInCustomRange = (charRange->endCodepoint - charRange->startCodepoint)+1;
				for (uxx gIndex = 0; gIndex < numGlyphsInCustomRange; gIndex++)
				{
					const CustomFontGlyph* customGlyph = &charRange->glyphs[gIndex];
					v2i glyphSize = customGlyph->imageData.size;
					
					FontGlyph* newGlyph = VarArrayAdd(FontGlyph, &newAtlas->glyphs);
					NotNull(newGlyph);
					ClearPointer(newGlyph);
					newGlyph->codepoint = customGlyph->codepoint;
					newGlyph->ttfGlyphIndex = 0;
					newGlyph->advanceX = (r32)glyphSize.Width;
					newGlyph->renderOffset = NewV2(0, RoundR32(-newAtlas->maxAscend + (newAtlas->maxAscend + newAtlas->maxDescend)/2.0f - glyphSize.Height/2.0f));
					newGlyph->logicalRec = NewRec(0, -newAtlas->maxAscend, (r32)glyphSize.Width, newAtlas->maxAscend);
					
					if (glyphSize.Width > 0 && glyphSize.Height > 0)
					{
						Assert(packedRecIndex < numGlyphsInAtlas);
						stbrp_rect packedRec = packRects[packedRecIndex];
						packedRecIndex++;
						Assert(packedRec.was_packed);
						
						newGlyph->atlasSourceRec = NewReci(packedRec.x, packedRec.y, packedRec.w - packingPadding*2, packedRec.h - packingPadding*2);
						
						for (i32 rowIndex = 0; rowIndex < customGlyph->imageData.size.Height; rowIndex++)
						{
							const Color32* inRowPntr = (const Color32*)&customGlyph->imageData.pixels[INDEX_FROM_COORD2D(0, rowIndex, glyphSize.Width, glyphSize.Height)];
							Color32* outRowPntr = &pixelsPntr[INDEX_FROM_COORD2D(packedRec.x + packingPadding + 0, packedRec.y + packingPadding + rowIndex, atlasSize.Width, atlasSize.Height)];
							MyMemCopy(outRowPntr, inRowPntr, sizeof(Color32) * glyphSize.Width);
						}
					}
				}
			}
			Assert(packedRecIndex == numGlyphsInAtlas);
		}
		
		Str8 textureName = PrintInArenaStr(scratch, "%.*s_atlas[%llu]", StrPrint(font->name), (u64)(font->atlases.length-1));
		newAtlas->texture = InitTexture(font->arena, textureName, atlasSize, pixelsPntr, TextureFlag_NoMipmaps);
		if (newAtlas->texture.error != Result_Success)
		{
			result = newAtlas->texture.error;
			FreeFontAtlas(font, newAtlas);
			VarArrayRemoveLast(FontAtlas, &font->atlases);
			break;
		}
		
		result = Result_Success;
	} while(false);
	
	#else //!BUILD_WITH_FREETYPE
	
	do
	{
		stbtt_pack_range* stbRanges = AllocArray(stbtt_pack_range, scratch, numCharRanges);
		NotNull(stbRanges);
		MyMemSet(stbRanges, 0x00, sizeof(stbtt_pack_range) * numCharRanges);
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
		}
		
		//NOTE: This used to be stbtt_PackFontRanges
		stbtt_fontinfo fontInfo;
		fontInfo.userdata = scratch;
		stbtt_InitFont(&fontInfo, fontFile->fileContents.bytes, stbtt_GetFontOffsetForIndex(fontFile->fileContents.bytes, 0));
		
		stbrp_rect* rects = AllocArray(stbrp_rect, scratch, numCodepointsTotal);
		NotNull(rects);
		int numRects = 0;
		uxx numPixels = 0;
		Color32* pixelsPntr = nullptr;
		
		bool packedSuccessfully = false;
		i32 atlasSideLength = minAtlasSize;
		while (atlasSideLength <= maxAtlasSize)
		{
			uxx scratchMark = ArenaGetMark(scratch);
			
			numPixels = (uxx)(atlasSideLength * atlasSideLength);
			pixelsPntr = AllocArray(Color32, scratch, numPixels);
			NotNull(pixelsPntr);
			MyMemSet(pixelsPntr, 0x00, sizeof(Color32) * numPixels);
			
			stbtt_pack_context packContext = ZEROED;
			int beginResult = stbtt_PackBegin(
				&packContext, //context
				(u8*)pixelsPntr, //pixels
				(int)atlasSideLength, (int)atlasSideLength, //width, height
				(int)(atlasSideLength * sizeof(u8)), //stride_in_bytes
				1, //padding (between chars)
				scratch //alloc_context
			);
			Assert(beginResult != 0);
			
			numRects = stbtt_PackFontRangesGatherRects(&packContext, &fontInfo, stbRanges, (int)numCharRanges, rects);
			Assert(numRects >= 0 && (uxx)numRects == numCodepointsInCharRanges);
			uxx customGlyphIndex = 0;
			for (uxx rIndex = 0; rIndex < numCustomGlyphRanges; rIndex++)
			{
				const CustomFontCharRange* customRange = &customGlyphRanges[rIndex];
				uxx numGlyphsInCustomRange = (customRange->endCodepoint - customRange->startCodepoint)+1;
				for (uxx gIndex = 0; gIndex < numGlyphsInCustomRange; gIndex++)
				{
					const CustomFontGlyph* customGlyph = &customRange->glyphs[gIndex];
					DebugAssert(customGlyphIndex < numCodepointsInCustomRanges);
					stbrp_rect* customGlyphRec = &rects[numCodepointsInCharRanges + customGlyphIndex];
					reci sourceRec = AreEqual(customGlyph->sourceRec, (reci)Reci_Zero_Const)
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
			
			stbtt_PackFontRangesPackRects(&packContext, rects, (int)numCodepointsTotal);
			int packResult = stbtt_PackFontRangesRenderIntoRects(&packContext, &fontInfo, stbRanges, (int)numCharRanges, rects);
			if (packResult > 0) { packedSuccessfully = true; break; }
			ArenaResetToMark(scratch, scratchMark); //only reset scratch if we failed to pack! Thus pixelsPntr can live on for use below
			atlasSideLength *= 2;
		}
		if (!packedSuccessfully) { result = Result_NotEnoughSpace; break; }
		v2i atlasSize = FillV2i(atlasSideLength);
		
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
		uxx customGlyphCopyIndex = 0;
		for (uxx rIndex = 0; rIndex < numCustomGlyphRanges; rIndex++)
		{
			const CustomFontCharRange* customRange = &customGlyphRanges[rIndex];
			uxx numGlyphsInCustomRange = (customRange->endCodepoint - customRange->startCodepoint)+1;
			for (uxx gIndex = 0; gIndex < numGlyphsInCustomRange; gIndex++)
			{
				const CustomFontGlyph* customGlyph = &customRange->glyphs[gIndex];
				DebugAssert(customGlyphCopyIndex < numCodepointsInCustomRanges);
				stbrp_rect* customGlyphRec = &rects[numCodepointsInCharRanges + customGlyphCopyIndex];
				reci sourceRec = AreEqual(customGlyph->sourceRec, (reci)Reci_Zero_Const)
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
				customGlyphCopyIndex++;
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
			result = newAtlas->texture.error;
			break;
		}
		
		newAtlas->fontSize = fontSize;
		newAtlas->fontScale = stbtt_ScaleForPixelHeight(&fontFile->ttfInfo, fontSize);
		newAtlas->styleFlags = (fontFile->styleFlags | extraStyleFlags);
		newAtlas->glyphRange.startCodepoint = minCodepoint;
		newAtlas->glyphRange.endCodepoint = maxCodepoint;
		
		int ascent, descent, lineGap;
		stbtt_GetFontVMetrics(&fontFile->ttfInfo, &ascent, &descent, &lineGap);
		newAtlas->maxAscend = (r32)ascent * newAtlas->fontScale;
		newAtlas->maxDescend = (r32)(-descent) * newAtlas->fontScale;
		newAtlas->lineHeight = newAtlas->maxAscend + newAtlas->maxDescend + ((r32)lineGap * newAtlas->fontScale);
		
		//TODO: This is sort of a hack and causes problems with things like highlight/clip rectangles that need to really encompass the true maxAscend
		//      So for now we are going to only use this value to inform the centerOffset
		//The ascent value returned by GetFontVMetrics is often way higher than all the characters we normally print
		//Rather than using that value, we'd prefer to use the ascent of a character like 'W' to get a more accurate idea of how far up the font will extend
		//This helps look more visually appealing with positioning text vertically centered in a small space (like in a UI button)
		int wBoxX0, wBoxY0, wBoxX1, wBoxY1;
		int getBoxResult = stbtt_GetCodepointBox(&fontFile->ttfInfo, CharToU32('W'), &wBoxX0, &wBoxY0, &wBoxX1, &wBoxY1);
		if (getBoxResult > 0)
		{
			r32 pretendMaxAscend = MinR32(newAtlas->maxAscend, (r32)wBoxY1 * newAtlas->fontScale);
			newAtlas->centerOffset = pretendMaxAscend / 2.0f;
		}
		
		InitVarArrayWithInitial(FontCharRange, &newAtlas->charRanges, font->arena, numCharRanges + numCustomGlyphRanges);
		InitVarArrayWithInitial(FontGlyph, &newAtlas->glyphs, font->arena, numCodepointsTotal);
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
				glyph->ttfGlyphIndex = stbtt_FindGlyphIndex(&fontFile->ttfInfo, glyph->codepoint);
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
		uxx customGlyphInfoIndex = 0;
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
				DebugAssert(customGlyphInfoIndex < numCodepointsInCustomRanges);
				stbrp_rect* packedGlyphRec = &rects[numCodepointsInCharRanges + customGlyphInfoIndex];
				FontGlyph* newGlyph = &newGlyphs[gIndex];
				ClearPointer(newGlyph);
				newGlyph->codepoint = customGlyph->codepoint;
				newGlyph->ttfGlyphIndex = INVALID_TTF_GLYPH_INDEX;
				newGlyph->atlasSourceRec = NewReci((i32)packedGlyphRec->x, (i32)packedGlyphRec->y, (i32)packedGlyphRec->w, (i32)packedGlyphRec->h);
				newGlyph->advanceX = (r32)newGlyph->atlasSourceRec.Width;
				newGlyph->renderOffset = NewV2(0, RoundR32(-newAtlas->maxAscend + (newAtlas->maxAscend + newAtlas->maxDescend)/2.0f - newGlyph->atlasSourceRec.Height/2.0f));
				newGlyph->logicalRec = NewRec(0, -newAtlas->maxAscend, (r32)newGlyph->atlasSourceRec.Width, newAtlas->maxAscend);
				customGlyphInfoIndex++;
			}
		}
		
		result = Result_Success;
	} while(false);
	
	#endif //BUILD_WITH_FREETYPE
	
	ScratchEnd(scratch);
	TracyCZoneEnd(_funcZone);
	return result;
}
PEXPI Result BakeFontAtlas(PigFont* font, r32 fontSize, u8 extraStyleFlags, i32 minAtlasSize, i32 maxAtlasSize, uxx numCharRanges, const FontCharRange* charRanges)
{
	return BakeFontAtlasEx(font, fontSize, extraStyleFlags, minAtlasSize, maxAtlasSize, numCharRanges, charRanges, 0, nullptr);
}

PEXP Result MultiBakeFontAtlasesEx(PigFont* font, uxx numSizes, const r32* fontSizes, u8 extraStyleFlags, i32 minAtlasSize, i32 maxAtlasSize, uxx numCharRanges, const FontCharRange* charRanges, uxx numCustomGlyphRanges, const CustomFontCharRange* customGlyphRanges)
{
	if (numSizes > 0) { NotNull(fontSizes); }
	for (uxx sIndex = 0; sIndex < numSizes; sIndex++)
	{
		Result bakeResult = BakeFontAtlasEx(font, fontSizes[sIndex], extraStyleFlags, minAtlasSize, maxAtlasSize, numCharRanges, charRanges, numCustomGlyphRanges, customGlyphRanges);
		if (bakeResult != Result_Success) { return bakeResult; }
	}
	return Result_Success;
}
PEXPI Result MultiBakeFontAtlases(PigFont* font, uxx numSizes, const r32* fontSizes, u8 extraStyleFlags, i32 minAtlasSize, i32 maxAtlasSize, uxx numCharRanges, const FontCharRange* charRanges)
{
	return MultiBakeFontAtlasesEx(font, numSizes, fontSizes, extraStyleFlags, minAtlasSize, maxAtlasSize, numCharRanges, charRanges, 0, nullptr);
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

PEXP FontFile* FindFontFileForCodepoint(PigFont* font, u32 codepoint, r32 fontSize, u8 styleFlags, unsigned int* glyphIndexOut, uxx* fileIndexOut)
{
	NotNull(font);
	NotNull(font->arena);
	for (uxx fIndex = 0; fIndex < font->numFiles; fIndex++)
	{
		FontFile* fontFile = &font->files[fIndex];
		if ((fontFile->styleFlags & FontStyleFlag_FontFileFlags) == (styleFlags & FontStyleFlag_FontFileFlags))
		{
			#if BUILD_WITH_FREETYPE
			{
				FT_UInt glyphIndex = FT_Get_Char_Index(fontFile->freeTypeFace, codepoint);
				if (glyphIndex != 0)
				{
					SetOptionalOutPntr(glyphIndexOut, glyphIndex);
					SetOptionalOutPntr(fileIndexOut, fIndex);
					return fontFile;
				}
			}
			#else //!BUILD_WITH_FREETYPE
			{
				Unimplemented(); //TODO: Implement me!
			}
			#endif //BUILD_WITH_FREETYPE
		}
	}
	return nullptr;
}

PEXP FontAtlas* AddNewActiveAtlas(PigFont* font, u64 programTime, FontFile* fontFile, r32 fontSize, u8 styleFlags)
{
	NotNull(font);
	Assert(font->isActive);
	if (font->activeMaxNumAtlases != 0 && font->atlases.length >= font->activeMaxNumAtlases) { return nullptr; }
	
	v2i atlasSize = FillV2i(font->activeAtlasMinSize);
	FontAtlas* newAtlas = VarArrayAdd(FontAtlas, &font->atlases);
	NotNull(newAtlas);
	ClearPointer(newAtlas);
	newAtlas->fontSize = fontSize;
	newAtlas->fontScale = 1.0f; //TODO: Fill me?
	newAtlas->styleFlags = (styleFlags & FontStyleFlag_FontAtlasFlags);
	newAtlas->glyphRange = NewFontCharRangeSingle(0);
	InitVarArrayWithInitial(FontCharRange, &newAtlas->charRanges, font->arena, 1);
	InitVarArrayWithInitial(FontGlyph, &newAtlas->glyphs, font->arena, 1);
	InitVarArray(FontActiveAtlasTextureUpdate, &newAtlas->pendingTextureUpdates, font->arena);
	
	ScratchBegin1(scratch, font->arena);
	uxx numAtlasPixels = (uxx)(atlasSize.Width * atlasSize.Height);
	u32* atlasPixels = AllocArray(u32, scratch, numAtlasPixels);
	MyMemSet(atlasPixels, 0x00, sizeof(u32) * numAtlasPixels);
	Str8 atlasTextureName = PrintInArenaStr(scratch, "%.*s_atlas[%llu]", StrPrint(font->name), (u64)(font->atlases.length-1));
	newAtlas->texture = InitTexture(font->arena, atlasTextureName, atlasSize, atlasPixels, TextureFlag_Mutable|TextureFlag_HasCopy|TextureFlag_NoMipmaps);
	Assert(newAtlas->texture.error == Result_Success);
	newAtlas->pushedTextureUpdates = true;
	ScratchEnd(scratch);
	
	#if BUILD_WITH_FREETYPE
	{
		FT_F26Dot6 freeTypeFontSize = TO_FT26_FROM_R32(fontSize);
		const u32 freeTypeFontDpi = 72;
		FT_Error setCharSizeError = FT_Set_Char_Size(fontFile->freeTypeFace, freeTypeFontSize, freeTypeFontSize, freeTypeFontDpi, freeTypeFontDpi);
		newAtlas->maxAscend = TO_R32_FROM_FT26(fontFile->freeTypeFace->size->metrics.ascender);
		newAtlas->maxDescend = TO_R32_FROM_FT26(fontFile->freeTypeFace->size->metrics.descender);
		newAtlas->lineHeight = TO_R32_FROM_FT26(fontFile->freeTypeFace->size->metrics.height);
		newAtlas->centerOffset = newAtlas->maxAscend - (newAtlas->lineHeight / 2.0f); //TODO: Fill the centerOffset using the W measure method that we did with stb_truetype.h?
	}
	#else //!BUILD_WITH_FREETYPE
	{
		Unimplemented(); //TODO: Implement me!
		//TODO: r32 lineHeight;
		//TODO: r32 maxAscend;
		//TODO: r32 maxDescend;
		//TODO: r32 centerOffset;
	}
	#endif //BUILD_WITH_FREETYPE
	
	newAtlas->isActive = true;
	newAtlas->activeCellSize = NewV2i(10, 16); //TODO: How do we decide this?
	newAtlas->activeCellGridSize.Width = FloorR32i((r32)atlasSize.Width / (r32)newAtlas->activeCellSize.Width);
	newAtlas->activeCellGridSize.Height = FloorR32i((r32)atlasSize.Height / (r32)newAtlas->activeCellSize.Height);
	uxx numCells = (uxx)(newAtlas->activeCellGridSize.Width * newAtlas->activeCellGridSize.Height);
	newAtlas->cells = AllocArray(FontActiveCell, font->arena, numCells);
	NotNull(newAtlas->cells);
	MyMemSet(newAtlas->cells, 0x00, sizeof(FontActiveCell) * numCells);
	
	newAtlas->lastUsedTime = programTime;
	return newAtlas;
}

PEXP void ResizeActiveFontAtlas(PigFont* font, FontAtlas* activeAtlas, v2i newSize)
{
	NotNull(font);
	NotNull(activeAtlas);
	Assert(activeAtlas->isActive);
	Assert(newSize.Width >= activeAtlas->texture.Width && newSize.Height >= activeAtlas->texture.Height);
	if (newSize.Width == activeAtlas->texture.Width && newSize.Height == activeAtlas->texture.Height) { return; }
	ScratchBegin1(scratch, font->arena);
	
	uxx atlasIndex = 0;
	bool foundIndex = VarArrayGetIndexOf(FontAtlas, &font->atlases, activeAtlas, &atlasIndex);
	Assert(foundIndex);
	PrintLine_D("Resizing atlas[%llu] %dx%d -> %dx%d (%llu glyph%s)", atlasIndex, activeAtlas->texture.Width, activeAtlas->texture.Height, newSize.Width, newSize.Height, activeAtlas->glyphs.length, Plural(activeAtlas->glyphs.length, "s"));
	uxx newNumPixels = (uxx)(newSize.Width * newSize.Height);
	Color32* newPixels = AllocArray(Color32, scratch, newNumPixels);
	NotNull(newPixels);
	MyMemSet(newPixels, 0x00, sizeof(Color32) * newNumPixels);
	for (uxx rowIndex = 0; rowIndex < activeAtlas->texture.Height; rowIndex++)
	{
		const Color32* sourceRow = (Color32*)&activeAtlas->texture.pixelsU32[INDEX_FROM_COORD2D(0, rowIndex, activeAtlas->texture.Width, activeAtlas->texture.Height)];
		Color32* destRow = &newPixels[INDEX_FROM_COORD2D(0, rowIndex, newSize.Width, newSize.Height)];
		MyMemCopy(destRow, sourceRow, sizeof(Color32) * activeAtlas->texture.Width);
	}
	if (activeAtlas->pendingTextureUpdates.length > 0)
	{
		PrintLine_D("Applying %llu texture update%s during resize", activeAtlas->pendingTextureUpdates.length, Plural(activeAtlas->pendingTextureUpdates.length, "s"));
		VarArrayLoop(&activeAtlas->pendingTextureUpdates, uIndex)
		{
			VarArrayLoopGet(FontActiveAtlasTextureUpdate, update, &activeAtlas->pendingTextureUpdates, uIndex);
			PrintLine_D("%dx%d pixels copied to (%d,%d)", update->imageData.size.Width, update->imageData.size.Height, update->sourcePos.X, update->sourcePos.Y);
			for (uxx rowIndex = 0; rowIndex < update->imageData.size.Height; rowIndex++)
			{
				const Color32* srcRowPntr = (Color32*)&update->imageData.pixels[INDEX_FROM_COORD2D(0, rowIndex, update->imageData.size.Width, update->imageData.size.Height)];
				Color32* destRowPntr = &newPixels[INDEX_FROM_COORD2D(update->sourcePos.X + 0, update->sourcePos.Y + rowIndex, newSize.Width, newSize.Height)];
				MyMemCopy(destRowPntr, srcRowPntr, sizeof(Color32) * update->imageData.size.Width);
			}
			FreeImageData(font->arena, &update->imageData);
		}
		VarArrayClear(&activeAtlas->pendingTextureUpdates);
	}
	Str8 atlasTextureName = PrintInArenaStr(scratch, "%.*s_atlas[%llu]", StrPrint(font->name), atlasIndex);
	Texture newTexture = InitTexture(font->arena, atlasTextureName, newSize, newPixels, TextureFlag_Mutable|TextureFlag_HasCopy|TextureFlag_NoMipmaps);
	Assert(newTexture.error == Result_Success);
	FreeTexture(&activeAtlas->texture);
	MyMemCopy(&activeAtlas->texture, &newTexture, sizeof(Texture));
	activeAtlas->pushedTextureUpdates = true;
	
	v2i newGridSize = NewV2i(
		FloorR32i((r32)newSize.Width / (r32)activeAtlas->activeCellSize.Width),
		FloorR32i((r32)newSize.Height / (r32)activeAtlas->activeCellSize.Height)
	);
	uxx newNumCells = (uxx)(newGridSize.Width * newGridSize.Height);
	FontActiveCell* newCells = AllocArray(FontActiveCell, font->arena, newNumCells);
	NotNull(newCells);
	MyMemSet(newCells, 0x00, sizeof(FontActiveCell) * newNumCells);
	for (uxx rowIndex = 0; rowIndex < activeAtlas->activeCellGridSize.Height; rowIndex++)
	{
		FontActiveCell* oldRow = &activeAtlas->cells[INDEX_FROM_COORD2D(0, rowIndex, activeAtlas->activeCellGridSize.Width, activeAtlas->activeCellGridSize.Height)];
		FontActiveCell* newRow = &newCells[INDEX_FROM_COORD2D(0, rowIndex, newGridSize.Width, newGridSize.Height)];
		MyMemCopy(newRow, oldRow, sizeof(FontActiveCell) * activeAtlas->activeCellGridSize.Width);
	}
	activeAtlas->activeCellGridSize = newGridSize;
	
	ScratchEnd(scratch);
}

PEXP FontGlyph* TryAddGlyphToActiveFontAtlas(PigFont* font, u64 programTime, FontFile* fontFile, FontAtlas* activeAtlas, u32 codepoint)
{
	NotNull(font);
	NotNull(fontFile);
	NotNull(activeAtlas);
	
	#if BUILD_WITH_FREETYPE
	FT_F26Dot6 freeTypeFontSize = TO_FT26_FROM_R32(activeAtlas->fontSize);
	const u32 freeTypeFontDpi = 72;
	FT_Error setCharSizeError = FT_Set_Char_Size(fontFile->freeTypeFace, freeTypeFontSize, freeTypeFontSize, freeTypeFontDpi, freeTypeFontDpi);
	Assert(setCharSizeError == 0);
	FT_UInt fontFileGlyphIndex = FT_Get_Char_Index(fontFile->freeTypeFace, codepoint);
	Assert(fontFileGlyphIndex != 0);
	FT_Error loadGlyphError = FT_Load_Glyph(fontFile->freeTypeFace, fontFileGlyphIndex, FT_LOAD_DEFAULT); //TODO: Use FT_LOAD_COLOR for colored emoji! Also check FT_HAS_COLOR(face)
	Assert(loadGlyphError == 0);
	v2i glyphSize = NewV2i(
		TO_I32_FROM_FT26(fontFile->freeTypeFace->glyph->metrics.width),
		TO_I32_FROM_FT26(fontFile->freeTypeFace->glyph->metrics.height)
	);
	v2i glyphCellSize = NewV2i(
		CeilDivI32(glyphSize.Width, activeAtlas->activeCellSize.Width),
		CeilDivI32(glyphSize.Height, activeAtlas->activeCellSize.Height)
	);
	#else
	Unimplemented(); //TODO: Implement me!
	#endif
	
	bool foundSpace = (glyphSize.Width == 0 || glyphSize.Height == 0);
	v2i cellPos = V2i_Zero_Const;
	while (!foundSpace)
	{
		if (glyphCellSize.Width <= activeAtlas->activeCellGridSize.Width && glyphCellSize.Height <= activeAtlas->activeCellGridSize.Height)
		{
			for (i32 cellY = 0; cellY + glyphCellSize.Height <= activeAtlas->activeCellGridSize.Height && !foundSpace; cellY++)
			{
				for (i32 cellX = 0; cellX + glyphCellSize.Width <= activeAtlas->activeCellGridSize.Width && !foundSpace; cellX++)
				{
					bool foundFilledCell = false;
					for (i32 offsetY = 0; offsetY < glyphCellSize.Height && !foundFilledCell; offsetY++)
					{
						for (i32 offsetX = 0; offsetX < glyphCellSize.Width && !foundFilledCell; offsetX++)
						{
							v2i gridPos = NewV2i(cellX + offsetX, cellY + offsetY);
							FontActiveCell* cell = &activeAtlas->cells[INDEX_FROM_COORD2D(gridPos.X, gridPos.Y, activeAtlas->activeCellGridSize.Width, activeAtlas->activeCellGridSize.Height)];
							if (cell->codepoint != 0) { foundFilledCell = true; break; }
						}
					}
					
					if (!foundFilledCell)
					{
						cellPos = NewV2i(cellX, cellY);
						foundSpace = true;
						break;
					}
				}
			}
			if (foundSpace) { break; }
		}
		
		if (activeAtlas->texture.Width < font->activeAtlasMaxSize)
		{
			v2i newSize = NewV2i(
				MinI32(font->activeAtlasMaxSize, activeAtlas->texture.Width*2),
				MinI32(font->activeAtlasMaxSize, activeAtlas->texture.Height*2)
			);
			ResizeActiveFontAtlas(font, activeAtlas, newSize);
		}
		else
		{
			//TODO: Find old glyphs and evict them!
			break;
		}
	}
	
	if (foundSpace)
	{
		PrintLine_D("Placing \'%c\' 0x%08X at cell(%d, %d) %dx%d in grid(%d, %d)", (char)codepoint, codepoint, cellPos.X, cellPos.Y, glyphCellSize.Width, glyphCellSize.Height, activeAtlas->activeCellGridSize.Width, activeAtlas->activeCellGridSize.Height);
		
		#if BUILD_WITH_FREETYPE
		FT_Error renderGlyphError = FT_Render_Glyph(fontFile->freeTypeFace->glyph, FT_RENDER_MODE_NORMAL);
		Assert(renderGlyphError == 0);
		#else
		Unimplemented(); //TODO: Implement me!
		#endif
		
		//Add new FontGlyph
		uxx glyphSortedInsertIndex = activeAtlas->glyphs.length;
		VarArrayLoop(&activeAtlas->glyphs, gIndex)
		{
			VarArrayLoopGet(FontGlyph, fontGlyph, &activeAtlas->glyphs, gIndex);
			if (fontGlyph->codepoint >= codepoint) { glyphSortedInsertIndex = gIndex; break; }
		}
		// PrintLine_D("Inserting glyph at index %llu", glyphSortedInsertIndex);
		FontGlyph* newGlyph = VarArrayInsert(FontGlyph, &activeAtlas->glyphs, glyphSortedInsertIndex);
		NotNull(newGlyph);
		ClearPointer(newGlyph);
		newGlyph->codepoint = codepoint;
		newGlyph->ttfGlyphIndex = 0; //TODO: Should we fill this?
		newGlyph->atlasSourceRec = NewReci(
			cellPos.X * activeAtlas->activeCellSize.Width,
			cellPos.Y * activeAtlas->activeCellSize.Height,
			glyphSize.Width,
			glyphSize.Height
		);
		#if BUILD_WITH_FREETYPE
		newGlyph->advanceX = TO_R32_FROM_FT26(fontFile->freeTypeFace->glyph->advance.x);
		newGlyph->renderOffset.X = (r32)fontFile->freeTypeFace->glyph->bitmap_left;
		newGlyph->renderOffset.Y = -(r32)fontFile->freeTypeFace->glyph->bitmap_top;
		newGlyph->logicalRec = NewRec(0, -activeAtlas->maxAscend, newGlyph->advanceX, activeAtlas->maxAscend);
		#else
		Unimplemented(); //TODO: Implement me!
		#endif
		
		// Bump glyphArrayStartIndex on any charRange that points to a glyph after our insertion index
		VarArrayLoop(&activeAtlas->charRanges, rIndex)
		{
			VarArrayLoopGet(FontCharRange, charRange, &activeAtlas->charRanges, rIndex);
			if (charRange->glyphArrayStartIndex >= glyphSortedInsertIndex)
			{
				// PrintLine_D("charRange[%llu] 0x%08X-0x%08X now starts at glyph index %llu", rIndex, charRange->startCodepoint, charRange->endCodepoint, charRange->glyphArrayStartIndex+1);
				charRange->glyphArrayStartIndex++;
			}
		}
		
		// Extend glyphRange
		if (activeAtlas->glyphs.length == 1)
		{
			activeAtlas->glyphRange = NewFontCharRangeSingle(codepoint);
		}
		else if (codepoint < activeAtlas->glyphRange.startCodepoint)
		{
			activeAtlas->glyphRange.startCodepoint = codepoint;
		}
		else if (codepoint > activeAtlas->glyphRange.endCodepoint)
		{
			activeAtlas->glyphRange.endCodepoint = codepoint;
		}
		
		// Extend or add to charRanges
		bool extendedRange = false;
		VarArrayLoop(&activeAtlas->charRanges, rIndex)
		{
			VarArrayLoopGet(FontCharRange, charRange, &activeAtlas->charRanges, rIndex);
			if (codepoint >= charRange->startCodepoint && codepoint <= charRange->endCodepoint) { extendedRange = true; break; }
			else if (charRange->startCodepoint > 0 && codepoint == charRange->startCodepoint-1)
			{
				charRange->startCodepoint = codepoint;
				charRange->glyphArrayStartIndex = glyphSortedInsertIndex;
				extendedRange = true;
				break;
			}
			else if (charRange->endCodepoint < UINT32_MAX && codepoint == charRange->endCodepoint+1)
			{
				charRange->endCodepoint = codepoint;
				extendedRange = true;
				break;
			}
		}
		//TODO: We should combine touching/overlapping ranges!
		if (!extendedRange)
		{
			uxx rangeSortedInsertIndex = activeAtlas->charRanges.length;
			VarArrayLoop(&activeAtlas->charRanges, rIndex)
			{
				VarArrayLoopGet(FontCharRange, charRange, &activeAtlas->charRanges, rIndex);
				if (charRange->startCodepoint > codepoint) { rangeSortedInsertIndex = rIndex; break; }
			}
			FontCharRange* newCharRange = VarArrayInsert(FontCharRange, &activeAtlas->charRanges, rangeSortedInsertIndex);
			ClearPointer(newCharRange);
			newCharRange->startCodepoint = codepoint;
			newCharRange->endCodepoint = codepoint;
			newCharRange->glyphArrayStartIndex = glyphSortedInsertIndex;
		}
		
		if (glyphSize.Width > 0 && glyphSize.Height > 0)
		{
			// Update FontActiveCell(s)
			for (uxx yOffset = 0; yOffset < glyphCellSize.Height; yOffset++)
			{
				for (uxx xOffset = 0; xOffset < glyphCellSize.Width; xOffset++)
				{
					FontActiveCell* cell = &activeAtlas->cells[INDEX_FROM_COORD2D(cellPos.X + xOffset, cellPos.Y + yOffset, activeAtlas->activeCellGridSize.Width, activeAtlas->activeCellGridSize.Height)];
					cell->codepoint = codepoint;
					cell->glyphIndex = glyphSortedInsertIndex;
					cell->lastUsedTime = programTime;
				}
			}
		
			// Store a FontActiveAtlasTextureUpdate
			#if BUILD_WITH_FREETYPE
			{
				Assert(fontFile->freeTypeFace->glyph->bitmap.width == (unsigned int)glyphSize.Width);
				Assert(fontFile->freeTypeFace->glyph->bitmap.rows == (unsigned int)glyphSize.Height);
				Assert(fontFile->freeTypeFace->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY);
				FontActiveAtlasTextureUpdate* newUpdate = VarArrayAdd(FontActiveAtlasTextureUpdate, &activeAtlas->pendingTextureUpdates);
				NotNull(newUpdate);
				ClearPointer(newUpdate);
				newUpdate->sourcePos = newGlyph->atlasSourceRec.TopLeft;
				newUpdate->imageData.size = glyphSize;
				newUpdate->imageData.numPixels = (uxx)(glyphSize.Width * glyphSize.Height);
				newUpdate->imageData.pixels = AllocArray(u32, font->arena, newUpdate->imageData.numPixels);
				NotNull(newUpdate->imageData.pixels);
				for (uxx yOffset = 0; yOffset < glyphSize.Height; yOffset++)
				{
					for (uxx xOffset = 0; xOffset < glyphSize.Width; xOffset++)
					{
						u8 inValue = fontFile->freeTypeFace->glyph->bitmap.buffer[INDEX_FROM_COORD2D(xOffset, yOffset, fontFile->freeTypeFace->glyph->bitmap.pitch, fontFile->freeTypeFace->glyph->bitmap.height)];
						Color32* pixelPntr = (Color32*)&newUpdate->imageData.pixels[INDEX_FROM_COORD2D(xOffset, yOffset, glyphSize.Width, glyphSize.Height)];
						pixelPntr->r = 255;
						pixelPntr->g = 255;
						pixelPntr->b = 255;
						pixelPntr->a = inValue;
					}
				}
			}
			#else
			{
				Unimplemented(); //TODO: Implement me!
			}
			#endif
		}
		
		return newGlyph;
	}
	else { return nullptr; }
}

//Pass 0 for codepoint to lookup and atlas without a particular glyph in mind
PEXP FontGlyph* GetFontGlyphForCodepoint(PigFont* font, u64 programTime, u32 codepoint, r32 fontSize, u8 styleFlags, bool allowActiveAtlasCreation, FontAtlas** atlasOut)
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
	
	if (matchingAtlas != nullptr)
	{
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
	}
	
	//If we didn't find an exact match atlas that contains the codepoint, then let's try creating the glyph in an active atlas
	if (allowActiveAtlasCreation && font->isActive && (matchingAtlas == nullptr || !AreSimilarR32(matchingAtlas->fontSize, fontSize, DEFAULT_R32_TOLERANCE) || (matchingAtlas->styleFlags & FontStyleFlag_FontAtlasFlags) != (styleFlags & FontStyleFlag_FontAtlasFlags)))
	{
		//Find an active font atlas if possible
		FontAtlas* matchingActiveAtlas = nullptr;
		VarArrayLoop(&font->atlases, aIndex)
		{
			VarArrayLoopGet(FontAtlas, atlas, &font->atlases, aIndex);
			if (atlas->isActive && AreSimilarR32(atlas->fontSize, fontSize, DEFAULT_R32_TOLERANCE) && (atlas->styleFlags & FontStyleFlag_FontAtlasFlags) == (styleFlags & FontStyleFlag_FontAtlasFlags))
			{
				matchingActiveAtlas = atlas;
				break;
			}
		}
		
		bool needToCreateNewAtlas = (matchingActiveAtlas == nullptr);
		bool needToRasterizeGlyph = false;
		if (codepoint != 0)
		{
			if (needToCreateNewAtlas) { needToRasterizeGlyph = true; }
			else { needToRasterizeGlyph = (matchingActiveAtlas->isActive && !DoesFontAtlasContainCodepointEx(matchingActiveAtlas, codepoint, nullptr)); }
		}
		
		//Find a sourceFontFile
		FontFile* sourceFontFile = nullptr;
		if (needToCreateNewAtlas || needToRasterizeGlyph)
		{
			unsigned int fontFileGlyphIndex = 0;
			FontFile* fontFile = FindFontFileForCodepoint(font, codepoint, fontSize, styleFlags, &fontFileGlyphIndex, nullptr);
			if (fontFile != nullptr)
			{
				#if BUILD_WITH_FREETYPE
				{
					FT_F26Dot6 freeTypeFontSize = TO_FT26_FROM_R32(fontSize);
					const u32 freeTypeFontDpi = 72;
					FT_Error setCharSizeError = FT_Set_Char_Size(fontFile->freeTypeFace, freeTypeFontSize, freeTypeFontSize, freeTypeFontDpi, freeTypeFontDpi);
					Assert(setCharSizeError == 0);
					FT_Error loadGlyphError = FT_Load_Glyph(fontFile->freeTypeFace, fontFileGlyphIndex, FT_LOAD_DEFAULT);
					if (loadGlyphError == 0)
					{
						matchingAtlas = matchingActiveAtlas;
						sourceFontFile = fontFile;
					}
				}
				#else //!BUILD_WITH_FREETYPE
				{
					Unimplemented(); //TODO: Implement me!
				}
				#endif //BUILD_WITH_FREETYPE
			}
			else
			{
				//We don't have any attached fonts that have a glyph for the desired codepoint so we can't rasterize it even if we wanted to
				needToCreateNewAtlas = false;
				needToRasterizeGlyph = false;
			}
		}
		
		if (needToCreateNewAtlas && sourceFontFile != nullptr)
		{
			PrintLine_D("Adding new active atlas for codepoint 0x%08X at size=%g style=%s%s", codepoint, fontSize, IsFlagSet(styleFlags, FontStyleFlag_Bold) ? "Bold" : "", IsFlagSet(styleFlags, FontStyleFlag_Italic) ? "Italic" : "");
			FontAtlas* newAtlas = AddNewActiveAtlas(font, programTime, sourceFontFile, fontSize, styleFlags);
			if (newAtlas != nullptr)
			{
				matchingAtlas = newAtlas;
				result = nullptr;
			}
		}
		
		if (needToRasterizeGlyph && matchingAtlas != nullptr && sourceFontFile != nullptr)
		{
			uxx matchingAtlasIndex = 0;
			VarArrayGetIndexOf(FontAtlas, &font->atlases, matchingAtlas, &matchingAtlasIndex);
			PrintLine_D("Adding new glyph to atlas[%llu] for codepoint 0x%08X at size=%g style=%s%s", matchingAtlasIndex, codepoint, fontSize, IsFlagSet(styleFlags, FontStyleFlag_Bold) ? "Bold" : "", IsFlagSet(styleFlags, FontStyleFlag_Italic) ? "Italic" : "");
			result = TryAddGlyphToActiveFontAtlas(font, programTime, sourceFontFile, matchingAtlas, codepoint);
		}
	}
	
	if (matchingAtlas != nullptr && matchingAtlas->isActive) { matchingAtlas->lastUsedTime = programTime; }
	SetOptionalOutPntr(atlasOut, matchingAtlas);
	return result;
}

PEXPI FontAtlas* GetFontAtlas(PigFont* font, u64 programTime, r32 fontSize, u8 styleFlags, bool allowActiveAtlasCreation)
{
	FontAtlas* result = nullptr;
	GetFontGlyphForCodepoint(font, programTime, 0, fontSize, styleFlags, allowActiveAtlasCreation, &result);
	return result;
}

PEXP void CommitFontAtlasTextureUpdates(PigFont* font, FontAtlas* activeAtlas)
{
	NotNull(font);
	NotNull(activeAtlas);
	if (activeAtlas->isActive && activeAtlas->pendingTextureUpdates.length > 0 && !activeAtlas->pushedTextureUpdates)
	{
		ScratchBegin1(scratch, font->arena);
		uxx numPixels = (uxx)(activeAtlas->texture.Width * activeAtlas->texture.Height);
		Color32* newPixels = AllocArray(Color32, scratch, numPixels);
		NotNull(newPixels);
		MyMemCopy(newPixels, activeAtlas->texture.pixelsU32, sizeof(Color32) * numPixels);
		VarArrayLoop(&activeAtlas->pendingTextureUpdates, uIndex)
		{
			VarArrayLoopGet(FontActiveAtlasTextureUpdate, update, &activeAtlas->pendingTextureUpdates, uIndex);
			for (uxx rowIndex = 0; rowIndex < update->imageData.size.Height; rowIndex++)
			{
				const Color32* srcRowPntr = (Color32*)&update->imageData.pixels[INDEX_FROM_COORD2D(0, rowIndex, update->imageData.size.Width, update->imageData.size.Height)];
				Color32* destRowPntr = &newPixels[INDEX_FROM_COORD2D(update->sourcePos.X + 0, update->sourcePos.Y + rowIndex, activeAtlas->texture.Width, activeAtlas->texture.Height)];
				MyMemCopy(destRowPntr, srcRowPntr, sizeof(Color32) * update->imageData.size.Width);
			}
			FreeImageData(font->arena, &update->imageData);
		}
		VarArrayClear(&activeAtlas->pendingTextureUpdates);
		UpdateTexture(&activeAtlas->texture, newPixels);
		activeAtlas->pushedTextureUpdates = true;
		ScratchEnd(scratch);
	}
}
PEXPI void CommitAllFontTextureUpdates(PigFont* font)
{
	VarArrayLoop(&font->atlases, aIndex)
	{
		VarArrayLoopGet(FontAtlas, atlas, &font->atlases, aIndex);
		CommitFontAtlasTextureUpdates(font, atlas);
	}
}

PEXPI void FontNewFrame(PigFont* font)
{
	VarArrayLoop(&font->atlases, aIndex)
	{
		VarArrayLoopGet(FontAtlas, atlas, &font->atlases, aIndex);
		if (atlas->isActive) { atlas->pushedTextureUpdates = false; }
	}
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
PEXP r32 GetFontKerningBetweenCodepoints(const PigFont* font, u64 programTime, r32 fontSize, u8 styleFlags, u32 leftCodepoint, u32 rightCodepoint, bool allowActiveAtlasCreation)
{
	NotNull(font);
	
	FontAtlas* leftGlyphAtlas = nullptr;
	FontGlyph* leftGlyph = GetFontGlyphForCodepoint((PigFont*)font, programTime, leftCodepoint, fontSize, styleFlags, allowActiveAtlasCreation, &leftGlyphAtlas);
	if (leftGlyph == nullptr || leftGlyphAtlas == nullptr) { return 0.0f; }
	if (leftGlyph->ttfGlyphIndex < 0) { return 0.0f; }
	
	FontAtlas* rightGlyphAtlas = nullptr;
	FontGlyph* rightGlyph = GetFontGlyphForCodepoint((PigFont*)font, programTime, rightCodepoint, fontSize, styleFlags, allowActiveAtlasCreation, &rightGlyphAtlas);
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
