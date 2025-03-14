/*
File:   gfx_font_flow.h
Author: Taylor Robbins
Date:   02\08\2025
Description:
	** This file acts as the API for using the PigFont structure that
	** is defined in gfx_font.h to layout rastered glyphs from the
	** atlases in the font based on information that was extracted
	** from the ttf for how place each glyph relative to the one before.
	** NOTE: This layout code is relatively straightforward but it
	** needs to disparate use cases so the primary implementation of
	** the logic has a lot of parameters to change it's behavior and
	** hook in to the process.
*/

#ifndef _GFX_FONT_FLOW_H
#define _GFX_FONT_FLOW_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "base/base_unicode.h"
#include "struct/struct_color.h"
#include "struct/struct_vectors.h"
#include "struct/struct_rectangles.h"
#include "struct/struct_string.h"
#include "gfx/gfx_font.h"

//TODO: Eventually we may want to support using Font stuff in Raylib! That would require making a gfx_texture implementation for Raylib first so we aren't doing that for now
#if BUILD_WITH_SOKOL_GFX

typedef struct FontFlowGlyph FontFlowGlyph;
struct FontFlowGlyph
{
	u32 codepoint;
	uxx byteIndex;
	FontAtlas* atlas;
	FontGlyph* glyph;
	v2 position;
	rec drawRec;
	Color32 color;
};

typedef struct FontFlow FontFlow;
struct FontFlow
{
	PigFont* font;
	v2 startPos;
	rec visualRec;
	rec logicalRec;
	uxx numGlyphs;
	uxx numGlyphsAlloc;
	FontFlowGlyph* glyphs;
};

typedef struct FontFlowState FontFlowState;
struct FontFlowState
{
	void* contextPntr;
	PigFont* font;
	v2 position;
	r32 fontSize;
	u8 styleFlags;
	Str8 text;
	uxx byteIndex;
	uxx charIndex;
	uxx glyphIndex;
	v2 underlineStartPos;
	v2 strikethroughStartPos;
	v2 alignPixelSize;
	Color32 color;
	FontAtlas* prevGlyphAtlas;
	FontGlyph* prevGlyph;
};

typedef struct TextMeasure TextMeasure;
struct TextMeasure
{
	rec visualRec;
	union
	{
		rec logicalRec;
		struct { r32 OffsetX, OffsetY; r32 Width, Height; };
	};
};

#define FONT_FLOW_BEFORE_CHAR_DEF(functionName) void functionName(FontFlowState* state, FontFlow* flow, u32 codepoint)
typedef FONT_FLOW_BEFORE_CHAR_DEF(FontFlowBeforeChar_f);

#define FONT_FLOW_DRAW_CHAR_DEF(functionName)   void functionName(FontFlowState* state, FontFlow* flow, rec glyphDrawRec, u32 codepoint, FontAtlas* atlas, FontGlyph* glyph)
typedef FONT_FLOW_DRAW_CHAR_DEF(FontFlowDrawChar_f);

#define FONT_FLOW_AFTER_CHAR_DEF(functionName)  void functionName(FontFlowState* state, FontFlow* flow, rec glyphDrawRec, rec glyphLogicalRec, u32 codepoint, FontAtlas* atlas, FontGlyph* glyph, r32 kerning)
typedef FONT_FLOW_AFTER_CHAR_DEF(FontFlowAfterChar_f);

typedef struct FontFlowCallbacks FontFlowCallbacks;
struct FontFlowCallbacks
{
	FontFlowBeforeChar_f* beforeChar;
	FontFlowDrawChar_f* drawChar;
	FontFlowAfterChar_f* afterChar;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	Result DoFontFlow(FontFlowState* state, FontFlowCallbacks* callbacks, FontFlow* flowOut);
	PIG_CORE_INLINE TextMeasure MeasureTextEx(const PigFont* font, r32 fontSize, u8 styleFlags, Str8 text);
	PIG_CORE_INLINE TextMeasure MeasureText(const PigFont* font, Str8 text);
	uxx ShortenTextToFitWidthEx(const PigFont* font, r32 fontSize, u8 styleFlags, Str8 text, r32 maxWidth, Str8 ellipsesStr, uxx ellipsesIndex, Str8* beforeEllipseStrOut, Str8* afterEllipseStrOut);
	PIG_CORE_INLINE Str8 ShortenTextToFitWidth(Arena* arena, const PigFont* font, r32 fontSize, u8 styleFlags, Str8 text, r32 maxWidth, Str8 ellipsesStr, uxx ellipsesIndex);
	PIG_CORE_INLINE Str8 ShortenTextStartToFitWidth(Arena* arena, const PigFont* font, r32 fontSize, u8 styleFlags, Str8 text, r32 maxWidth, Str8 ellipsesStr);
	PIG_CORE_INLINE Str8 ShortenTextEndToFitWidth(Arena* arena, const PigFont* font, r32 fontSize, u8 styleFlags, Str8 text, r32 maxWidth, Str8 ellipsesStr);
	PIG_CORE_INLINE Str8 ShortenFilePathToFitWidth(Arena* arena, const PigFont* font, r32 fontSize, u8 styleFlags, FilePath filePath, r32 maxWidth, Str8 ellipsesStr);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP Result DoFontFlow(FontFlowState* state, FontFlowCallbacks* callbacks, FontFlow* flowOut)
{
	NotNull(state);
	NotNullStr(state->text);
	NotNull(state->font);
	Result result = Result_Success;
	
	if (flowOut != nullptr)
	{
		// ClearPointer(flowOut); //NOTE: Don't clear the FontFlow since the numGlyphsAlloc/glyphs is filled by the calling code if they want us to store per-glyph information
		flowOut->font = state->font;
		flowOut->startPos = state->position;
		flowOut->visualRec = NewRecV(state->position, V2_Zero);
		flowOut->logicalRec = NewRecV(state->position, V2_Zero);
		FontAtlas* firstAtlas = GetFontAtlas(state->font, state->fontSize, state->styleFlags);
		if (firstAtlas != nullptr)
		{
			flowOut->logicalRec.Y -= firstAtlas->maxAscend;
			flowOut->logicalRec.Height = firstAtlas->maxAscend;
		}
		flowOut->numGlyphs = 0;
	}
	
	while (state->byteIndex < state->text.length)
	{
		u32 codepoint = 0;
		u8 utf8ByteSize = GetCodepointForUtf8Str(state->text, state->byteIndex, &codepoint);
		if (utf8ByteSize == 0)
		{
			//TODO: Should we handle invalid UTF-8 differently?
			codepoint = CharToU32(state->text.chars[state->byteIndex]);
			utf8ByteSize = 1;
			if (result == Result_Success) { result = Result_InvalidUtf8; }
		}
		
		if (callbacks != nullptr && callbacks->beforeChar != nullptr) { callbacks->beforeChar(state, flowOut, codepoint); }
		if (state->byteIndex >= state->text.length) { break; }
		
		r32 kerning = 0.0f;
		rec glyphDrawRec = Rec_Zero;
		rec glyphLogicalRec = Rec_Zero;
		FontAtlas* fontAtlas = nullptr;
		FontGlyph* fontGlyph = GetFontGlyphForCodepoint(state->font, codepoint, state->fontSize, state->styleFlags, &fontAtlas);
		if (fontGlyph != nullptr)
		{
			if (state->prevGlyphAtlas != nullptr && state->prevGlyphAtlas->fontScale == fontAtlas->fontScale) //TODO: Should we check the style flags match?
			{
				kerning = GetFontKerningBetweenGlyphs(state->font, fontAtlas->fontScale, state->prevGlyph, fontGlyph);
				state->position.X += kerning;
				// if (kerning != 0.0f) { PrintLine_D("Kern between \'%c\' and \'%c\' = %f", (char)state->prevGlyph->codepoint, (char)codepoint, kerning); }
			}
			
			glyphDrawRec = NewRecV(Add(state->position, fontGlyph->renderOffset), ToV2Fromi(fontGlyph->atlasSourceRec.Size));
			glyphLogicalRec = NewRecV(Add(state->position, fontGlyph->logicalRec.TopLeft), fontGlyph->logicalRec.Size);
			if (state->alignPixelSize.X != 0) { glyphDrawRec.X = RoundR32(glyphDrawRec.X * state->alignPixelSize.X) / state->alignPixelSize.X; }
			if (state->alignPixelSize.Y != 0) { glyphDrawRec.Y = RoundR32(glyphDrawRec.Y * state->alignPixelSize.Y) / state->alignPixelSize.Y; }
			
			if (callbacks != nullptr && callbacks->drawChar != nullptr)
			{
				callbacks->drawChar(state, flowOut, glyphDrawRec, codepoint, fontAtlas, fontGlyph);
			}
			
			//TODO: Draw Strikethrough
			//TODO: Draw Underline
			
			if (flowOut != nullptr)
			{
				if (state->glyphIndex < flowOut->numGlyphsAlloc)
				{
					NotNull(flowOut->glyphs);
					FontFlowGlyph* flowGlyph = &flowOut->glyphs[state->glyphIndex];
					ClearPointer(flowGlyph);
					flowGlyph->codepoint = codepoint;
					flowGlyph->byteIndex = state->byteIndex;
					flowGlyph->atlas = fontAtlas;
					flowGlyph->glyph = fontGlyph;
					flowGlyph->position = state->position;
					flowGlyph->drawRec = glyphDrawRec;
					flowGlyph->color = state->color;
				}
				
				flowOut->logicalRec = BothRec(flowOut->logicalRec, glyphLogicalRec);
				if (flowOut->numGlyphs == 0) { flowOut->visualRec = glyphDrawRec; }
				else { flowOut->visualRec = BothRec(flowOut->visualRec, glyphDrawRec); }
				flowOut->numGlyphs++;
			}
			
			state->position.X += fontGlyph->advanceX;
			state->glyphIndex++;
		}
		else
		{
			//TODO: What should we do if we don't find the glyph? Render a default character maybe?
		}
		
		state->charIndex++;
		state->byteIndex += utf8ByteSize;
		
		if (callbacks != nullptr && callbacks->afterChar != nullptr) { callbacks->afterChar(state, flowOut, glyphDrawRec, glyphLogicalRec, codepoint, fontAtlas, fontGlyph, kerning); }
		
		if (fontGlyph != nullptr)
		{
			state->prevGlyphAtlas = fontAtlas;
			state->prevGlyph = fontGlyph;
		}
	}
	
	return result;
}

PEXPI TextMeasure MeasureTextEx(const PigFont* font, r32 fontSize, u8 styleFlags, Str8 text)
{
	FontFlowState state = ZEROED;
	state.font = (PigFont*)font;
	state.position = V2_Zero_Const;
	state.fontSize = fontSize;
	state.styleFlags = styleFlags;
	state.text = text;
	state.alignPixelSize = V2_One; //TODO: Should this be a function parameter?
	
	FontFlow flow = ZEROED;
	Result flowResult = DoFontFlow(&state, nullptr, &flow);
	Assert(flowResult == Result_Success || flowResult == Result_InvalidUtf8);
	
	TextMeasure result = ZEROED;
	result.visualRec = flow.visualRec;
	result.logicalRec = flow.logicalRec;
	return result;
}
PEXPI TextMeasure MeasureText(const PigFont* font, Str8 text)
{
	Assert(font->atlases.length > 0);
	FontAtlas* firstAtlas = VarArrayGetFirst(FontAtlas, &font->atlases);
	return MeasureTextEx(font, firstAtlas->fontSize, firstAtlas->styleFlags, text);
}

//NOTE: ellipsesIndex is an index into the pre-shortened string, it will replace characters from both left and right so it may end up at an index earlier in the string if it pulls characters from the left
// Returns number of characters that were removed from the text in order to make it fit
PEXP uxx ShortenTextToFitWidthEx(const PigFont* font, r32 fontSize, u8 styleFlags, Str8 text, r32 maxWidth, Str8 ellipsesStr, uxx ellipsesIndex, Str8* beforeEllipseStrOut, Str8* afterEllipseStrOut)
{
	NotNull(font);
	NotNullStr(text);
	NotNullStr(ellipsesStr);
	Assert(ellipsesIndex <= text.length);
	Assert(!IsInfiniteOrNanR32(maxWidth));
	if (maxWidth <= 0)
	{
		SetOptionalOutPntr(beforeEllipseStrOut, NewStr8(0, &text.chars[0]));
		SetOptionalOutPntr(afterEllipseStrOut, NewStr8(0, &text.chars[text.length-1]));
		return text.length;
	}
	ScratchBegin(scratch);
	FontFlowState state = ZEROED;
	state.font = (PigFont*)font;
	state.position = V2_Zero_Const;
	state.fontSize = fontSize;
	state.styleFlags = styleFlags;
	state.text = text;
	state.alignPixelSize = V2_One;
	
	FontFlow flow = ZEROED;
	flow.numGlyphsAlloc = text.length;
	flow.glyphs = AllocArray(FontFlowGlyph, scratch, text.length);
	NotNull(flow.glyphs);
	Result flowResult = DoFontFlow(&state, nullptr, &flow);
	Assert(flowResult == Result_Success || flowResult == Result_InvalidUtf8);
	AssertMsg(flow.numGlyphs <= flow.numGlyphsAlloc, "We shouldn't have more glyphs than there are number of bytes in the string!");
	
	if (flow.logicalRec.Width <= maxWidth)
	{
		SetOptionalOutPntr(beforeEllipseStrOut, text);
		SetOptionalOutPntr(afterEllipseStrOut, NewStr8(0, &text.chars[text.length-1]));
		ScratchEnd(scratch);
		return 0;
	}
	
	r32 ellipsesWidth = 0.0f;
	if (!IsEmptyStr(ellipsesStr))
	{
		TextMeasure ellipsesMeasure = MeasureTextEx(font, fontSize, styleFlags, ellipsesStr);
		ellipsesWidth = ellipsesMeasure.Width - ellipsesMeasure.OffsetX;
	}
	
	Str8 leftPortion = StrSlice(text, 0, ellipsesIndex);
	Str8 rightPortion = StrSliceFrom(text, ellipsesIndex);
	FontFlowGlyph* leftLastGlyph = leftPortion.length > 0 ? &flow.glyphs[leftPortion.length-1] : nullptr;
	r32 leftWidth = (leftLastGlyph != nullptr) ? (leftLastGlyph->drawRec.X + leftLastGlyph->drawRec.Width - flow.logicalRec.X) : 0.0f;
	FontFlowGlyph* rightFirstGlyph = rightPortion.length > 0 ? &flow.glyphs[text.length - rightPortion.length] : nullptr;
	r32 rightWidth = (rightFirstGlyph != nullptr) ? ((flow.logicalRec.X + flow.logicalRec.Width) - rightFirstGlyph->drawRec.X) : 0.0f;
	uxx numCharsRemoved = 0;
	bool removeLeft = true;
	while (leftPortion.length + rightPortion.length > 0 && leftWidth + ellipsesWidth + rightWidth > maxWidth)
	{
		if (removeLeft && leftPortion.length == 0) { removeLeft = false; }
		if (!removeLeft && rightPortion.length == 0) { removeLeft = true; }
		if (removeLeft)
		{
			//TODO: This isn't going to work when we have multi-byte UTF-8 characters that account for a single glyph. We should think about how we are indexing into the glyphs to find the one we are about to remove!
			FontFlowGlyph* leftGlyph = &flow.glyphs[leftPortion.length-1];
			leftWidth = leftGlyph->drawRec.X - flow.logicalRec.X;
			leftPortion.length--;
			numCharsRemoved++;
		}
		else
		{
			//TODO: This isn't going to work when we have multi-byte UTF-8 characters that account for a single glyph. We should think about how we are indexing into the glyphs to find the one we are about to remove!
			FontFlowGlyph* rightGlyph = &flow.glyphs[text.length - rightPortion.length];
			rightWidth = (flow.logicalRec.X + flow.logicalRec.Width) - (rightGlyph->drawRec.X + rightGlyph->drawRec.Width);
			rightPortion.chars++;
			rightPortion.length--;
			numCharsRemoved++;
		}
		removeLeft = !removeLeft; //alternate taking characters from left and right sides
	}
	
	SetOptionalOutPntr(beforeEllipseStrOut, leftPortion);
	SetOptionalOutPntr(afterEllipseStrOut, rightPortion);
	
	ScratchEnd(scratch);
	return numCharsRemoved;
}
PEXPI Str8 ShortenTextToFitWidth(Arena* arena, const PigFont* font, r32 fontSize, u8 styleFlags, Str8 text, r32 maxWidth, Str8 ellipsesStr, uxx ellipsesIndex)
{
	NotNull(arena);
	Str8 beforeEllipseStr = Str8_Empty;
	Str8 afterEllipseStr = Str8_Empty;
	uxx numCharsRemoved = ShortenTextToFitWidthEx(font, fontSize, styleFlags, text, maxWidth, ellipsesStr, ellipsesIndex, &beforeEllipseStr, &afterEllipseStr);
	if (numCharsRemoved > 0)
	{
		return PrintInArenaStr(arena, "%.*s%.*s%.*s", StrPrint(beforeEllipseStr), StrPrint(ellipsesStr), StrPrint(afterEllipseStr));
	}
	else
	{
		return AllocStr8(arena, text);
	}
}
PEXPI Str8 ShortenTextStartToFitWidth(Arena* arena, const PigFont* font, r32 fontSize, u8 styleFlags, Str8 text, r32 maxWidth, Str8 ellipsesStr)
{
	return ShortenTextToFitWidth(arena, font, fontSize, styleFlags, text, maxWidth, ellipsesStr, 0);
}
PEXPI Str8 ShortenTextEndToFitWidth(Arena* arena, const PigFont* font, r32 fontSize, u8 styleFlags, Str8 text, r32 maxWidth, Str8 ellipsesStr)
{
	return ShortenTextToFitWidth(arena, font, fontSize, styleFlags, text, maxWidth, ellipsesStr, text.length);
}
PEXPI Str8 ShortenFilePathToFitWidth(Arena* arena, const PigFont* font, r32 fontSize, u8 styleFlags, FilePath filePath, r32 maxWidth, Str8 ellipsesStr)
{
	
	Str8 fileNamePart = GetFileNamePart(filePath, true);
	uxx fileNameStartIndex = (uxx)(fileNamePart.chars - filePath.chars);
	uxx ellipsesIndex = fileNameStartIndex / 2;
	return ShortenTextToFitWidth(arena, font, fontSize, styleFlags, filePath, maxWidth, ellipsesStr, ellipsesIndex);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _GFX_FONT_FLOW_H

#if defined(_GFX_FONT_FLOW_H) && defined(_MEM_ARENA_H)
#include "cross/cross_font_flow_and_mem_arena.h"
#endif
