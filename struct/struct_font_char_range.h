/*
File:   struct_font_char_range.h
Author: Taylor Robbins
Date:   11\05\2025
*/

//TODO: "char" is a bit of a misnomer here, really we are talking about codepoints. We should clean up our usage of "char", "glyph" and "codepoint"

#ifndef _STRUCT_FONT_CHAR_RANGE_H
#define _STRUCT_FONT_CHAR_RANGE_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "base/base_unicode.h"
#include "struct/struct_image_data.h"
#include "struct/struct_rectangles.h"

//NOTE: CharRanges are inclusive on the upper bound, so endCodepoint is part of the range! Care must be taken when using UNICODE_SOMETHING_END macros since those are exclusive!
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

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define FontCharRange_ASCII                 NewFontCharRange(UNICODE_PRINTABLE_ASCII_START, UNICODE_PRINTABLE_ASCII_END-1)
#define FontCharRange_UppercaseLetters      NewFontCharRange('A', 'Z')
#define FontCharRange_LowercaseLetters      NewFontCharRange('a', 'z')
#define FontCharRange_LatinSupplementAccent NewFontCharRange(UNICODE_LATIN1_SUPPLEMENT_ACCENT_START, UNICODE_LATIN1_SUPPLEMENT_ACCENT_END-1)
#define FontCharRange_LatinExtA             NewFontCharRange(UNICODE_LATIN_EXT_A_START, UNICODE_LATIN_EXT_A_END-1)
#define FontCharRange_Cyrillic              NewFontCharRange(UNICODE_CYRILLIC_START, UNICODE_CYRILLIC_END-1)
#define FontCharRange_Hiragana              NewFontCharRange(UNICODE_HIRAGANA_START, UNICODE_HIRAGANA_END-1)
#define FontCharRange_Katakana              NewFontCharRange(UNICODE_KATAKANA_START, UNICODE_KATAKANA_END-1)

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE FontCharRange NewFontCharRangeSingle(u32 codepoint);
	PIG_CORE_INLINE FontCharRange NewFontCharRange(u32 startCodepoint, u32 endCodepoint);
	PIG_CORE_INLINE FontCharRange NewFontCharRangeLength(u32 startCodepoint, u32 numCodepoints);
	PIG_CORE_INLINE CustomFontCharRange NewCustomFontCharRangeSingle(CustomFontGlyph* glyph);
	PIG_CORE_INLINE CustomFontCharRange NewCustomFontCharRange(uxx numGlyphs, CustomFontGlyph* glyphs);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

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

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_FONT_CHAR_RANGE_H

#if defined(_MEM_ARENA_H) && defined(_STRUCT_FONT_CHAR_RANGE_H)
#include "cross/cross_mem_arena_and_font_char_range.h"
#endif

#if defined(_STRUCT_FONT_CHAR_RANGE_H) && defined(_STRUCT_VAR_ARRAY_H)
#include "cross/cross_font_char_range_and_var_array.h"
#endif
