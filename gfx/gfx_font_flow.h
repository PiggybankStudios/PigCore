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

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _GFX_FONT_FLOW_H

#if defined(_GFX_FONT_FLOW_H) && defined(_MEM_ARENA_H)
#include "cross/cross_font_flow_and_mem_arena.h"
#endif
