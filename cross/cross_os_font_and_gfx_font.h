/*
File:   cross_os_font_and_gfx_font.h
Author: Taylor Robbins
Date:   02\08\2025
*/

#ifndef _CROSS_OS_FONT_AND_GFX_FONT_H
#define _CROSS_OS_FONT_AND_GFX_FONT_H

#if BUILD_WITH_SOKOL_GFX

typedef plex FontBakeSettings FontBakeSettings;
plex FontBakeSettings
{
	Str8 name;
	r32 size;
	u8 style;
	bool fillKerningTable;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	Result AttachOsTtfFileToFont(PigFont* font, Str8 fontName, r32 fontSize, u8 ttfStyleFlags);
	Result AttachAndMultiBakeFontAtlasesEx(PigFont* font, uxx numSettings, const FontBakeSettings* settings, i32 minAtlasSize, i32 maxAtlasSize, uxx numCharRanges, const FontCharRange* charRanges, uxx numCustomGlyphRanges, const CustomFontCharRange* customGlyphRanges);
	Result AttachAndMultiBakeFontAtlases(PigFont* font, uxx numSettings, const FontBakeSettings* settings, i32 minAtlasSize, i32 maxAtlasSize, uxx numCharRanges, const FontCharRange* charRanges);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP Result AttachOsTtfFileToFont(PigFont* font, Str8 fontName, r32 fontSize, u8 ttfStyleFlags)
{
	NotNull(font);
	NotNull(font->arena);
	
	Slice fileContents = Slice_Empty;
	Result readResult = OsReadPlatformFont(
		font->arena,
		fontName,
		RoundR32i(fontSize),
		IsFlagSet(ttfStyleFlags, FontStyleFlag_Bold),
		IsFlagSet(ttfStyleFlags, FontStyleFlag_Italic),
		&fileContents
	);
	if (readResult != Result_Success) { return readResult; }
	
	return AttachTtfFileToFont(font, fileContents, false, ttfStyleFlags);
}

PEXP Result AttachAndMultiBakeFontAtlasesEx(PigFont* font, uxx numSettings, const FontBakeSettings* settings, i32 minAtlasSize, i32 maxAtlasSize, uxx numCharRanges, const FontCharRange* charRanges, uxx numCustomGlyphRanges, const CustomFontCharRange* customGlyphRanges)
{
	if (numSettings > 0) { NotNull(settings); }
	for (uxx sIndex = 0; sIndex < numSettings; sIndex++)
	{
		const FontBakeSettings* setting = &settings[sIndex];
		Result attachResult = AttachOsTtfFileToFont(font, setting->name, setting->size, setting->style);
		if (attachResult != Result_Success) { return attachResult; }
		Result bakeResult = BakeFontAtlasEx(font, setting->size, setting->style, minAtlasSize, maxAtlasSize, numCharRanges, charRanges, numCustomGlyphRanges, customGlyphRanges);
		if (bakeResult != Result_Success && setting->fillKerningTable) { FillFontKerningTable(font); }
		RemoveAttachedTtfFile(font);
		if (bakeResult != Result_Success) { return bakeResult; }
	}
	return Result_Success;
}

PEXPI Result AttachAndMultiBakeFontAtlases(PigFont* font, uxx numSettings, const FontBakeSettings* settings, i32 minAtlasSize, i32 maxAtlasSize, uxx numCharRanges, const FontCharRange* charRanges)
{
	return AttachAndMultiBakeFontAtlasesEx(font, numSettings, settings, minAtlasSize, maxAtlasSize, numCharRanges, charRanges, 0, nullptr);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _CROSS_OS_FONT_AND_GFX_FONT_H
