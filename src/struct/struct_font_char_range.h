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
#define MakeFontCharRangeEx(startCodepointValue, endCodepointValue, glyphArrayStartIndexValue) NEW_STRUCT(FontCharRange){ .startCodepoint=(startCodepointValue), .endCodepoint=(endCodepointValue), .glyphArrayStartIndex=(glyphArrayStartIndexValue) }
#define MakeFontCharRange(startCodepointValue, endCodepointValue)                              MakeFontCharRangeEx(startCodepointValue, endCodepointValue, 0)
#define MakeFontCharRangeLength(startCodepointValue, numCodepoints)                            MakeFontCharRange(startCodepointValue, (startCodepointValue) + (numCodepoints)-1)
#define MakeFontCharRangeSingle(codepoint)                                                     MakeFontCharRange(codepoint, codepoint)

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
#define MakeCustomFontCharRange(startCodepointValue, endCodepointValue, glyphsPntr) NEW_STRUCT(CustomFontCharRange){ .startCodepoint=(startCodepointValue), .endCodepoint=(endCodepointValue), .glyphs=(glyphsPntr) }
#define MakeCustomFontCharRangeSingle(glyphPntr) MakeCustomFontCharRange((glyphPntr)->codepoint, (glyphPntr)->codepoint, (glyphPntr))
#define MakeCustomFontCharRangeArray(numGlyphs, glyphsPntr) MakeCustomFontCharRange((glyphsPntr)[0].codepoint, (glyphsPntr)[numGlyphs-1].codepoint, (glyphsPntr))

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define FontCharRange_ASCII                 MakeFontCharRange(UNICODE_PRINTABLE_ASCII_START, UNICODE_PRINTABLE_ASCII_END-1)
#define FontCharRange_UppercaseLetters      MakeFontCharRange('A', 'Z')
#define FontCharRange_LowercaseLetters      MakeFontCharRange('a', 'z')
#define FontCharRange_LatinSupplementAccent MakeFontCharRange(UNICODE_LATIN1_SUPPLEMENT_ACCENT_START, UNICODE_LATIN1_SUPPLEMENT_ACCENT_END-1)
#define FontCharRange_LatinExtA             MakeFontCharRange(UNICODE_LATIN_EXT_A_START, UNICODE_LATIN_EXT_A_END-1)
#define FontCharRange_Cyrillic              MakeFontCharRange(UNICODE_CYRILLIC_START, UNICODE_CYRILLIC_END-1)
#define FontCharRange_Hiragana              MakeFontCharRange(UNICODE_HIRAGANA_START, UNICODE_HIRAGANA_END-1)
#define FontCharRange_Katakana              MakeFontCharRange(UNICODE_KATAKANA_START, UNICODE_KATAKANA_END-1)

#endif //  _STRUCT_FONT_CHAR_RANGE_H

#if defined(_MISC_SORTING_H) && defined(_STRUCT_FONT_CHAR_RANGE_H)
#include "cross/cross_sorting_and_font_char_range.h"
#endif

#if defined(_STRUCT_FONT_CHAR_RANGE_H) && defined(_STRUCT_VAR_ARRAY_H)
#include "cross/cross_font_char_range_and_var_array.h"
#endif
