/*
File:   cross_os_font_and_gfx_font.h
Author: Taylor Robbins
Date:   02\08\2025
*/

#ifndef _CROSS_OS_FONT_AND_GFX_FONT_H
#define _CROSS_OS_FONT_AND_GFX_FONT_H

#if BUILD_WITH_SOKOL_GFX

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	Result AttachOsTtfFileToFont(PigFont* font, Str8 fontName, r32 fontSize, u8 ttfStyleFlags);
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
	
	FreeStr8(font->arena, &font->ttfFile);
	font->ttfFile = fileContents;
	font->ttfStyleFlags = ttfStyleFlags;
	
	InitFontTtfInfo(font);
	
	return Result_Success;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _CROSS_OS_FONT_AND_GFX_FONT_H
