/*
File:   gfx_font_flow.h
Author: Taylor Robbins
Date:   02\08\2025
Description:
	** This file acts as the API for using the PigFont structure that
	** is defined in gfx_font.h to layout rastered glyphs from the
	** atlases in the font based on information that was extracted
	** from the ttf for how to place each glyph relative to the one before.
	** NOTE: This layout code is relatively straightforward but it's
	** needed by disparate use cases so the primary implementation of
	** the logic has a lot of parameters to change it's behavior and
	** hook in to the process.
*/

#ifndef _GFX_FONT_FLOW_H
#define _GFX_FONT_FLOW_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "base/base_unicode.h"
#include "mem/mem_scratch.h"
#include "struct/struct_color.h"
#include "struct/struct_vectors.h"
#include "struct/struct_rectangles.h"
#include "struct/struct_string.h"
#include "struct/struct_rich_string.h"
#include "gfx/gfx_font.h"
#include "lib/lib_tracy.h"

//TODO: We should try to optimize DoFontFlow at some point. To layout a full-screen of 12pt text is taking around 39ms on my laptop (optimized build)
//      EarlyFlowLogic: 14ms
//      Other:          10ms
//      GlyphLogic:     8ms
//      CodepointLogic: 5ms
//      PostLogic:      0.5ms

//TODO: Eventually we may want to support using Font stuff in Raylib! That would require making a gfx_texture implementation for Raylib first so we aren't doing that for now
#if BUILD_WITH_SOKOL_GFX

typedef plex FontFlowGlyph FontFlowGlyph;
plex FontFlowGlyph
{
	u32 codepoint;
	uxx byteIndex;
	FontAtlas* atlas;
	FontGlyph* glyph;
	v2 position;
	rec drawRec;
	Color32 color;
};

typedef plex FontFlow FontFlow;
plex FontFlow
{
	PigFont* font;
	v2 startPos;
	v2 endPos;
	rec visualRec;
	rec logicalRec;
	uxx numGlyphs;
	uxx numGlyphsAlloc;
	FontFlowGlyph* glyphs;
};

typedef plex FontFlowState FontFlowState;
plex FontFlowState
{
	void* contextPntr;
	PigFont* font;
	v2 startPos;
	v2 position;
	r32 startFontSize;
	u8 startFontStyle;
	Color32 startColor;
	Color32 backgroundColor; //only used when drawing highlighted text
	RichStr text;
	r32 wrapWidth; //0 = no word wrapping
	
	uxx byteIndex;
	uxx charIndex;
	uxx glyphIndex;
	uxx textPieceIndex;
	uxx textPieceByteIndex;
	RichStrStyle currentStyle;
	v2 alignPixelSize;
	FontAtlas* prevGlyphAtlas;
	FontGlyph* prevGlyph;
	
	bool drawingHighlightRecs;
	uxx highlightRecsDrawnToByteIndex;
	v2 highlightStartPos;
	v2 underlineStartPos;
	v2 strikethroughStartPos;
	
	bool findingNextWordBeforeWrap;
	uxx wordWrapByteIndex;
	bool wordWrapByteIndexIsLineEnd;
	
	//TODO: This is not technically the correct calculation.
	//      Remembering how big this line is doesn't tell us how much to advance
	//      for next line to not overlap with this one. We would need to forward
	//      scan the next line and figure out it's maxLineHeight as well and use
	//      maxDescend of this line and maxAscend of the next line
	r32 maxLineHeightThisLine;
};

typedef plex TextMeasure TextMeasure;
plex TextMeasure
{
	rec visualRec;
	car
	{
		rec logicalRec;
		plex { r32 OffsetX, OffsetY; r32 Width, Height; };
	};
};

#define FONT_FLOW_BEFORE_CHAR_DEF(functionName)     void functionName(FontFlowState* state, FontFlow* flow, u32 codepoint)
typedef FONT_FLOW_BEFORE_CHAR_DEF(FontFlowBeforeChar_f);

#define FONT_FLOW_DRAW_CHAR_DEF(functionName)       void functionName(FontFlowState* state, FontFlow* flow, rec glyphDrawRec, u32 codepoint, FontAtlas* atlas, FontGlyph* glyph, FontGlyphMetrics glyphMetrics)
typedef FONT_FLOW_DRAW_CHAR_DEF(FontFlowDrawChar_f);

#define FONT_FLOW_DRAW_HIGHLIGHT_DEF(functionName)  void functionName(FontFlowState* state, FontFlow* flow, rec highlightRec)
typedef FONT_FLOW_DRAW_HIGHLIGHT_DEF(FontFlowDrawHighlight_f);

#define FONT_FLOW_AFTER_CHAR_DEF(functionName)      void functionName(FontFlowState* state, FontFlow* flow, rec glyphDrawRec, rec glyphLogicalRec, u32 codepoint, FontAtlas* atlas, FontGlyph* glyph, r32 kerning)
typedef FONT_FLOW_AFTER_CHAR_DEF(FontFlowAfterChar_f);

typedef plex FontFlowCallbacks FontFlowCallbacks;
plex FontFlowCallbacks
{
	FontFlowBeforeChar_f* beforeChar;
	FontFlowDrawChar_f* drawChar;
	FontFlowDrawHighlight_f* drawHighlight;
	FontFlowAfterChar_f* afterChar;
};

typedef plex TextLayout TextLayout;
plex TextLayout
{
	Arena* arena;
	FontFlow flow;
	uxx numGlyphs;
	uxx numGlyphsAlloc;
	FontFlowGlyph* glyphs;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	Result DoFontFlow(FontFlowState* state, FontFlowCallbacks* callbacks, FontFlow* flowOut);
	PIG_CORE_INLINE TextMeasure MeasureRichTextFlow(const PigFont* font, r32 fontSize, u8 styleFlags, bool includeAdvanceX, r32 wrapWidth, RichStr text, FontFlow* flowOut);
	PIG_CORE_INLINE TextMeasure MeasureRichTextEx(const PigFont* font, r32 fontSize, u8 styleFlags, bool includeAdvanceX, r32 wrapWidth, RichStr text);
	PIG_CORE_INLINE TextMeasure MeasureRichText(const PigFont* font, r32 wrapWidth, RichStr text);
	PIG_CORE_INLINE TextMeasure MeasureTextFlow(const PigFont* font, r32 fontSize, u8 styleFlags, bool includeAdvanceX, r32 wrapWidth, Str8 text, FontFlow* flowOut);
	PIG_CORE_INLINE TextMeasure MeasureTextEx(const PigFont* font, r32 fontSize, u8 styleFlags, bool includeAdvanceX, r32 wrapWidth, Str8 text);
	PIG_CORE_INLINE TextMeasure MeasureText(const PigFont* font, r32 wrapWidth, Str8 text);
	//TODO: Made RichStr versions of ShortenTextToFitWidth functions!
	uxx ShortenTextToFitWidthEx(const PigFont* font, r32 fontSize, u8 styleFlags, Str8 text, r32 maxWidth, Str8 ellipsesStr, uxx ellipsesIndex, Str8* beforeEllipseStrOut, Str8* afterEllipseStrOut);
	PIG_CORE_INLINE Str8 ShortenTextToFitWidth(Arena* arena, const PigFont* font, r32 fontSize, u8 styleFlags, Str8 text, r32 maxWidth, Str8 ellipsesStr, uxx ellipsesIndex);
	PIG_CORE_INLINE Str8 ShortenTextStartToFitWidth(Arena* arena, const PigFont* font, r32 fontSize, u8 styleFlags, Str8 text, r32 maxWidth, Str8 ellipsesStr);
	PIG_CORE_INLINE Str8 ShortenTextEndToFitWidth(Arena* arena, const PigFont* font, r32 fontSize, u8 styleFlags, Str8 text, r32 maxWidth, Str8 ellipsesStr);
	PIG_CORE_INLINE Str8 ShortenFilePathToFitWidth(Arena* arena, const PigFont* font, r32 fontSize, u8 styleFlags, FilePath filePath, r32 maxWidth, Str8 ellipsesStr);
	PIG_CORE_INLINE void ResetFontFlowInfo(FontFlow* flow);
	PIG_CORE_INLINE void FreeTextLayout(TextLayout* layout);
	Result DoTextLayoutInArena(Arena* arena, FontFlowState* state, TextLayout* layoutOut);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP Result DoFontFlow(FontFlowState* state, FontFlowCallbacks* callbacks, FontFlow* flowOut);

static Result DoFontFlow_HighlightRecs(const FontFlowState* flowState, FontFlowCallbacks* callbacks, uxx* endIndexOut)
{
	NotNull(flowState);
	FontFlowState tempState;
	MyMemCopy(&tempState, flowState, sizeof(FontFlowState));
	tempState.drawingHighlightRecs = true;
	tempState.highlightStartPos = tempState.position;
	Result result = DoFontFlow(&tempState, callbacks, nullptr);
	SetOptionalOutPntr(endIndexOut, tempState.byteIndex);
	return result;
}

static Result DoFontFlow_FindNextWordWrapIndex(const FontFlowState* realState, FontFlowCallbacks* callbacks, uxx* wrapIndexOut)
{
	NotNull(realState);
	FontFlowState tempState;
	MyMemCopy(&tempState, realState, sizeof(FontFlowState));
	tempState.findingNextWordBeforeWrap = true;
	Result result = DoFontFlow(&tempState, callbacks, nullptr);
	SetOptionalOutPntr(wrapIndexOut, tempState.byteIndex);
	return result;
}

static void DoFontFlow_DrawHighlightRec(FontFlowState* state, FontFlowCallbacks* callbacks, FontFlow* flowOut)
{
	FontLineMetrics metrics = GetFontLineMetrics(state->font, state->currentStyle.fontSize, state->currentStyle.fontStyle);
	rec highlightRec = NewRec(
		state->highlightStartPos.X,
		state->highlightStartPos.Y - metrics.centerOffset - metrics.lineHeight/2.0f - 1,
		state->position.X - state->highlightStartPos.X,
		metrics.lineHeight+2
	);
	AlignRecToV2(&highlightRec, state->alignPixelSize);
	if (callbacks != nullptr && callbacks->drawHighlight != nullptr)
	{
		callbacks->drawHighlight(state, flowOut, highlightRec);
	}
	state->highlightStartPos = state->position;
}

PEXP Result DoFontFlow(FontFlowState* state, FontFlowCallbacks* callbacks, FontFlow* flowOut)
{
	NotNull(state);
	NotNullStr(state->text.fullPiece.str);
	NotNull(state->font);
	TracyCZoneN(_funcZone, "DoFontFlow", !state->drawingHighlightRecs && !state->findingNextWordBeforeWrap);
	TracyCZoneN(_funcZone_DrawHighlightRecs, "DrawHighlightRecs(DoFontFlow)", state->drawingHighlightRecs);
	TracyCZoneN(_funcZone_FindingNextWordBeforeWrap, "FindingNextWordBeforeWrap(DoFontFlow)", state->findingNextWordBeforeWrap);
	Result result = Result_Success;
	
	//Initial copying of state between start/current and flowOut, doesn't need to happen if we're drawing highlight recs or finding word wrap index because the parent DoFontFlow already did this
	if (!state->drawingHighlightRecs && !state->findingNextWordBeforeWrap)
	{
		state->startPos = state->position;
		state->currentStyle.fontSize = state->startFontSize;
		state->currentStyle.fontStyle = state->startFontStyle;
		state->currentStyle.color = state->startColor;
		
		if (flowOut != nullptr)
		{
			// ClearPointer(flowOut); //NOTE: Don't clear the FontFlow since the numGlyphsAlloc/glyphs is filled by the calling code if they want us to store per-glyph information
			flowOut->font = state->font;
			flowOut->startPos = state->position;
			flowOut->endPos = state->position;
			flowOut->visualRec = NewRecV(state->position, V2_Zero);
			flowOut->logicalRec = NewRecV(state->position, V2_Zero);
			FontLineMetrics lineMetrics = GetFontLineMetrics(state->font, state->startFontSize, state->startFontStyle);
			flowOut->logicalRec.Y -= lineMetrics.maxAscend;
			flowOut->logicalRec.Height = lineMetrics.maxAscend;
			flowOut->numGlyphs = 0;
		}
	}
	
	uxx lastWordEndIndex = UINTXX_MAX;
	u32 prevCodepoint = UINT32_MAX;
	
	while (state->byteIndex < state->text.fullPiece.str.length)
	{
		TracyCZoneN(_earlyFlowLogic, "EarlyFlowLogic", true);
		RichStrPiece* currentPiece = GetRichStrPiece(&state->text, state->textPieceIndex);
		DebugAssert(currentPiece != nullptr);
		
		// If any of these things are changing in the next str piece then we need to draw a piece of the active highlight before we continue
		bool isLineEnding = (state->byteIndex >= state->wordWrapByteIndex && !state->findingNextWordBeforeWrap);
		bool isStartingNewPiece = (state->textPieceByteIndex == 0);
		bool isHighlightedChanging = (isStartingNewPiece && IsFontStyleFlagChangingInRichStrStyleChange(&state->currentStyle, state->startFontStyle, currentPiece->styleChange, FontStyleFlag_Highlighted));
		if (state->drawingHighlightRecs && IsFlagSet(state->currentStyle.fontStyle, FontStyleFlag_Highlighted) && (isStartingNewPiece || isLineEnding))
		{
			if (isLineEnding ||
				isHighlightedChanging ||
				currentPiece->styleChange.type == RichStrStyleChangeType_Color ||
				currentPiece->styleChange.type == RichStrStyleChangeType_ColorAndAlpha ||
				currentPiece->styleChange.type == RichStrStyleChangeType_FontSize)
			{
				DoFontFlow_DrawHighlightRec(state, callbacks, flowOut);
			}
		}
		
		if (isStartingNewPiece)
		{
			TracyCZoneN(_applyRichStyleChangeZone, "ApplyRichStyleChange", true);
			ApplyRichStyleChange(&state->currentStyle, currentPiece->styleChange, state->startFontSize, state->startFontStyle, state->startColor);
			TracyCZoneEnd(_applyRichStyleChangeZone);
		}
		
		//If highlighting is starting then we should do some look-ahead and render the highlight rectangles FIRST then render the text on top using backgroundColor
		if (!state->drawingHighlightRecs && !state->findingNextWordBeforeWrap &&
			isHighlightedChanging && IsFlagSet(state->currentStyle.fontStyle, FontStyleFlag_Highlighted) &&
			callbacks != nullptr && callbacks->drawHighlight != nullptr)
		{
			if (state->byteIndex >= state->highlightRecsDrawnToByteIndex)
			{
				Result drawHighlightResult = DoFontFlow_HighlightRecs(state, callbacks, &state->highlightRecsDrawnToByteIndex);
				if (drawHighlightResult != Result_Success && drawHighlightResult != Result_InvalidUtf8)
				{
					result = drawHighlightResult;
					TracyCZoneEnd(_earlyFlowLogic);
					break;
				}
			}
		}
		
		if (isLineEnding)
		{
			if (state->wordWrapByteIndexIsLineEnd)
			{
				state->position.X = state->startPos.X;
				if (state->maxLineHeightThisLine > 0)
				{
					state->position.Y += state->maxLineHeightThisLine;
				}
				else
				{
					state->position.Y += GetFontLineHeight(state->font, state->currentStyle.fontSize, state->currentStyle.fontStyle);
				}
				state->maxLineHeightThisLine = 0.0f;
				state->highlightStartPos = state->position;
			}
			
			Result findWrapResult = DoFontFlow_FindNextWordWrapIndex(state, callbacks, &state->wordWrapByteIndex);
			if (findWrapResult != Result_Success && findWrapResult != Result_InvalidUtf8)
			{
				result = findWrapResult;
				TracyCZoneEnd(_earlyFlowLogic);
				break;
			}
			state->wordWrapByteIndexIsLineEnd = true;
		}
		TracyCZoneEnd(_earlyFlowLogic);
		
		if (state->textPieceByteIndex < currentPiece->str.length)
		{
			TracyCZoneN(_codepointLogic, "CodepointLogic", true);
			u32 codepoint = 0;
			u8 utf8ByteSize = GetCodepointForUtf8Str(currentPiece->str, state->textPieceByteIndex, &codepoint);
			if (utf8ByteSize == 0)
			{
				//TODO: Should we handle invalid UTF-8 differently?
				codepoint = CharToU32(currentPiece->str.chars[state->textPieceByteIndex]);
				utf8ByteSize = 1;
				if (result == Result_Success) { result = Result_InvalidUtf8; }
			}
			
			if (codepoint == '\n' && state->findingNextWordBeforeWrap) //TODO: Should we handle \r\n new-line sequence?
			{
				state->byteIndex += utf8ByteSize;
				TracyCZoneEnd(_codepointLogic);
				break;
			}
			if (prevCodepoint != UINT32_MAX && state->findingNextWordBeforeWrap && state->wrapWidth > 0.0f && IsWordBoundary(prevCodepoint, codepoint))
			{
				lastWordEndIndex = state->byteIndex;
			}
			
			if (callbacks != nullptr && callbacks->beforeChar != nullptr && !state->drawingHighlightRecs && !state->findingNextWordBeforeWrap)
			{
				callbacks->beforeChar(state, flowOut, codepoint);
			}
			if (state->byteIndex >= state->text.fullPiece.str.length) { TracyCZoneEnd(_codepointLogic); break; }
			
			r32 kerning = 0.0f;
			rec glyphDrawRec = Rec_Zero;
			rec glyphLogicalRec = Rec_Zero;
			
			u32 fontCodepoint = codepoint;
			FontAtlas* fontAtlas = nullptr;
			FontGlyph* fontGlyph = nullptr;
			
			u32 substituteCodepoints[] = { codepoint, UNICODE_UNKNOWN_CHAR_CODEPOINT, CharToU32('?') };
			uxx substituteIndex = 0;
			while (fontGlyph == nullptr && substituteIndex < ArrayCount(substituteCodepoints))
			{
				fontGlyph = TryGetFontGlyphForCodepoint(state->font, substituteCodepoints[substituteIndex], state->currentStyle.fontSize, state->currentStyle.fontStyle, true, &fontAtlas);
				// if (fontGlyph == nullptr) { MyBreak(); }
				if (fontGlyph != nullptr) { fontCodepoint = substituteCodepoints[substituteIndex]; break; }
				else if (IsCodepointWhitespace(codepoint, true)) { break; } //don't do substitution characters for whitespace (esp. not new-line character)
				// PrintLine_D("Couldn't find a glyph for codepoint U+%X \'%s\'", substituteCodepoints[substituteIndex], DebugGetCodepointName(substituteCodepoints[substituteIndex]));
				substituteIndex++;
			}
			TracyCZoneEnd(_codepointLogic);
			
			if (fontGlyph != nullptr || IsCodepointWhitespace(codepoint, false))
			{
				TracyCZoneN(_glyphLogic, "GlyphLogic", true);
				FontGlyphMetrics glyphMetrics = ZEROED;
				if (fontGlyph != nullptr) { glyphMetrics = fontGlyph->metrics; }
				if (fontAtlas != nullptr && !AreSimilarR32(fontAtlas->fontSize, state->currentStyle.fontSize, DEFAULT_R32_TOLERANCE))
				{
					//TODO: If we can figure out how to make TryGetFontGlyphMetrics (FT_Load_Glyph is slow) then we should do the more accurate logic instead
					#if 1
					r32 lineHeight = GetFontLineHeight(state->font, state->currentStyle.fontSize, state->currentStyle.fontStyle);
					r32 predictedScale = lineHeight / fontAtlas->metrics.lineHeight;
					#else
					r32 predictedScale = 1.0f;
					if (!TryGetFontGlyphMetrics(state->font, fontCodepoint, state->currentStyle.fontSize, state->currentStyle.fontStyle, &glyphMetrics))
					{
						FontLineMetrics lineMetrics = ZEROED;
						if (TryGetFontLineMetrics(state->font, state->currentStyle.fontSize, state->currentStyle.fontStyle, &lineMetrics))
						{
							predictedScale = lineMetrics.lineHeight / fontAtlas->metrics.lineHeight;
						}
					}
					#endif
					
					if (predictedScale != 1.0f)
					{
						glyphMetrics.glyphSize = NewV2i(
							RoundR32i(glyphMetrics.glyphSize.Width * predictedScale),
							RoundR32i(glyphMetrics.glyphSize.Height * predictedScale)
						);
						glyphMetrics.renderOffset = ScaleV2(glyphMetrics.renderOffset, predictedScale);
						glyphMetrics.advanceX *= predictedScale;
						glyphMetrics.logicalRec = ScaleRec(glyphMetrics.logicalRec, predictedScale);
					}
				}
				
				state->maxLineHeightThisLine = MaxR32(state->maxLineHeightThisLine, fontAtlas != nullptr ? fontAtlas->metrics.lineHeight : 0.0f);
				
				if (state->prevGlyphAtlas != nullptr && fontGlyph != nullptr && state->prevGlyphAtlas->metrics.fontScale == fontAtlas->metrics.fontScale) //TODO: Should we check the style flags match?
				{
					kerning = GetFontKerningBetweenGlyphs(state->font, fontAtlas->metrics.fontScale, state->prevGlyph, fontGlyph);
					state->position.X += kerning;
					// if (kerning != 0.0f) { PrintLine_D("Kern between \'%c\' and \'%c\' = %f", (char)state->prevGlyph->codepoint, (char)codepoint, kerning); }
				}
				
				glyphDrawRec = NewRecV(AddV2(state->position, glyphMetrics.renderOffset), ToV2Fromi(glyphMetrics.glyphSize));
				glyphLogicalRec = NewRecV(AddV2(state->position, glyphMetrics.logicalRec.TopLeft), glyphMetrics.logicalRec.Size);
				if (state->alignPixelSize.X != 0) { glyphDrawRec.X = RoundR32(glyphDrawRec.X * state->alignPixelSize.X) / state->alignPixelSize.X; }
				if (state->alignPixelSize.Y != 0) { glyphDrawRec.Y = RoundR32(glyphDrawRec.Y * state->alignPixelSize.Y) / state->alignPixelSize.Y; }
				
				// If the character can't fit within wrapWidth, then figure out where to break the line
				if (state->findingNextWordBeforeWrap && state->wrapWidth > 0.0f &&
					glyphLogicalRec.X + glyphLogicalRec.Width >= state->startPos.X + state->wrapWidth)
				{
					//Either wrap at the last word boundary, or if there was no word boundary this line then wrap before this character
					//TODO: Sublime seems to not use the last word boundary if it only was like 1-3 characters before the boundary
					//		(and there was plenty of more horizontal space to be used) Maybe we should do something similar?
					if (lastWordEndIndex != UINTXX_MAX)
					{
						state->byteIndex = lastWordEndIndex;
					}
					
					// Consume any whitespace and up to one new-line character as part of the line break
					while (state->byteIndex < state->text.fullPiece.str.length)
					{
						uxx pieceByteIndex = 0;
						RichStrPiece* richStrPiece = GetRichStrPieceForByteIndex(&state->text, state->byteIndex, &pieceByteIndex);
						NotNull(richStrPiece);
						u32 maybeSpaceCodepoint = 0;
						u8 maybeSpaceUtf8ByteSize = GetCodepointForUtf8Str(richStrPiece->str, pieceByteIndex, &maybeSpaceCodepoint);
						if (IsCharWhitespace(maybeSpaceCodepoint, false))
						{
							state->byteIndex += maybeSpaceUtf8ByteSize;
						}
						else if (maybeSpaceCodepoint == '\n')
						{
							state->byteIndex += maybeSpaceUtf8ByteSize;
							break;
						}
						else { break; }
					}
					
					TracyCZoneEnd(_glyphLogic);
					break;
				}
				
				if (callbacks != nullptr && callbacks->drawChar != nullptr && !state->drawingHighlightRecs && !state->findingNextWordBeforeWrap && fontGlyph != nullptr)
				{
					callbacks->drawChar(state, flowOut, glyphDrawRec, codepoint, fontAtlas, fontGlyph, glyphMetrics);
				}
				
				//TODO: Draw Strikethrough
				//TODO: Draw Underline
				//TODO: Draw Highlight
				
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
						//TODO: Should we store glyphMetrics here? Since fontGlyph can be nullptr now? 
						flowGlyph->position = state->position;
						flowGlyph->drawRec = glyphDrawRec;
						flowGlyph->color = state->currentStyle.color;
					}
					
					flowOut->logicalRec = BothRec(flowOut->logicalRec, glyphLogicalRec);
					if (flowOut->numGlyphs == 0) { flowOut->visualRec = glyphDrawRec; }
					else { flowOut->visualRec = BothRec(flowOut->visualRec, glyphDrawRec); }
					flowOut->numGlyphs++;
				}
				
				state->position.X += glyphMetrics.advanceX;
				state->glyphIndex++;
				TracyCZoneEnd(_glyphLogic);
			}
			else
			{
				//TODO: What should we do if we don't find the glyph? Render a default character maybe?
			}
			
			TracyCZoneN(_postLogic, "PostLogic", true);
			state->charIndex++;
			state->byteIndex += utf8ByteSize;
			state->textPieceByteIndex += utf8ByteSize;
			
			if (callbacks != nullptr && callbacks->afterChar != nullptr && !state->drawingHighlightRecs && !state->findingNextWordBeforeWrap)
			{
				callbacks->afterChar(state, flowOut, glyphDrawRec, glyphLogicalRec, codepoint, fontAtlas, fontGlyph, kerning);
			}
			
			if (fontGlyph != nullptr)
			{
				state->prevGlyphAtlas = fontAtlas;
				state->prevGlyph = fontGlyph;
			}
			
			prevCodepoint = codepoint;
			TracyCZoneEnd(_postLogic);
		}
		else
		{
			state->textPieceIndex++;
			state->textPieceByteIndex = 0;
		}
	}
	
	if (flowOut != nullptr) { flowOut->endPos = state->position; }
	
	if (state->drawingHighlightRecs && IsFlagSet(state->currentStyle.fontStyle, FontStyleFlag_Highlighted))
	{
		DoFontFlow_DrawHighlightRec(state, callbacks, flowOut);
	}
	
	TracyCZoneEnd(_funcZone_FindingNextWordBeforeWrap);
	TracyCZoneEnd(_funcZone_DrawHighlightRecs);
	TracyCZoneEnd(_funcZone);
	return result;
}

PEXPI TextMeasure MeasureRichTextFlow(const PigFont* font, r32 fontSize, u8 styleFlags, bool includeAdvanceX, r32 wrapWidth, RichStr text, FontFlow* flowOut)
{
	NotNull(flowOut);
	TracyCZoneN(_funcZone, "MeasureRichTextFlow", true);
	FontFlowState state = ZEROED;
	state.font = (PigFont*)font;
	state.position = V2_Zero;
	state.startFontSize = fontSize;
	state.startFontStyle = styleFlags;
	state.text = text;
	state.wrapWidth = wrapWidth;
	state.alignPixelSize = V2_One; //TODO: Should this be a function parameter?
	
	Result flowResult = DoFontFlow(&state, nullptr, flowOut);
	Assert(flowResult == Result_Success || flowResult == Result_InvalidUtf8);
	
	TextMeasure result = ZEROED;
	result.visualRec = flowOut->visualRec;
	result.logicalRec = flowOut->logicalRec;
	if (includeAdvanceX && result.logicalRec.Width < flowOut->endPos.X - result.logicalRec.X)
	{
		result.logicalRec.Width = flowOut->endPos.X - result.logicalRec.X;
	}
	TracyCZoneEnd(_funcZone);
	return result;
}
PEXPI TextMeasure MeasureRichTextEx(const PigFont* font, r32 fontSize, u8 styleFlags, bool includeAdvanceX, r32 wrapWidth, RichStr text)
{
	FontFlow flow = ZEROED;
	return MeasureRichTextFlow(font, fontSize, styleFlags, includeAdvanceX, wrapWidth, text, &flow);
}
PEXPI TextMeasure MeasureRichText(const PigFont* font, r32 wrapWidth, RichStr text)
{
	Assert(font->atlases.length > 0);
	FontAtlas* firstAtlas = VarArrayGetFirst(FontAtlas, &font->atlases);
	return MeasureRichTextEx(font, firstAtlas->fontSize, firstAtlas->styleFlags, false, wrapWidth, text);
}
PEXPI TextMeasure MeasureTextFlow(const PigFont* font, r32 fontSize, u8 styleFlags, bool includeAdvanceX, r32 wrapWidth, Str8 text, FontFlow* flowOut)
{
	return MeasureRichTextFlow(font, fontSize, styleFlags, includeAdvanceX, wrapWidth, ToRichStr(text), flowOut);
}
PEXPI TextMeasure MeasureTextEx(const PigFont* font, r32 fontSize, u8 styleFlags, bool includeAdvanceX, r32 wrapWidth, Str8 text)
{
	return MeasureRichTextEx(font, fontSize, styleFlags, includeAdvanceX, wrapWidth, ToRichStr(text));
}
PEXPI TextMeasure MeasureText(const PigFont* font, r32 wrapWidth, Str8 text)
{
	Assert(font->atlases.length > 0);
	FontAtlas* firstAtlas = VarArrayGetFirst(FontAtlas, &font->atlases);
	return MeasureRichTextEx(font, firstAtlas->fontSize, firstAtlas->styleFlags, false, wrapWidth, ToRichStr(text));
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
	TracyCZoneN(_funcZone, "ShortenTextToFitWidthEx", true);
	if (maxWidth <= 0)
	{
		SetOptionalOutPntr(beforeEllipseStrOut, NewStr8(0, &text.chars[0]));
		SetOptionalOutPntr(afterEllipseStrOut, NewStr8(0, &text.chars[text.length-1]));
		TracyCZoneEnd(_funcZone);
		return text.length;
	}
	ScratchBegin(scratch);
	FontFlowState state = ZEROED;
	state.font = (PigFont*)font;
	state.position = V2_Zero;
	state.startFontSize = fontSize;
	state.startFontStyle = styleFlags;
	state.text = ToRichStr(text);
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
		TracyCZoneEnd(_funcZone);
		return 0;
	}
	
	r32 ellipsesWidth = 0.0f;
	if (!IsEmptyStr(ellipsesStr))
	{
		TextMeasure ellipsesMeasure = MeasureTextEx(font, fontSize, styleFlags, false, 0, ellipsesStr);
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
	TracyCZoneEnd(_funcZone);
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

PEXPI void ResetFontFlowInfo(FontFlow* flow)
{
	NotNull(flow);
	uxx numGlyphsAlloc = flow->numGlyphsAlloc;
	FontFlowGlyph* glyphs = flow->glyphs;
	ClearPointer(flow);
	flow->numGlyphsAlloc = numGlyphsAlloc;
	flow->glyphs = glyphs;
}

PEXPI void FreeTextLayout(TextLayout* layout)
{
	if (layout->arena != nullptr)
	{
		if (layout->glyphs != nullptr) { FreeArray(FontFlowGlyph, layout->arena, layout->numGlyphsAlloc, layout->glyphs); }
	}
	ClearPointer(layout);
}

PEXP Result DoTextLayoutInArena(Arena* arena, FontFlowState* state, TextLayout* layoutOut)
{
	NotNull(arena);
	NotNull(state);
	NotNull(layoutOut);
	
	ClearPointer(layoutOut);
	layoutOut->arena = arena;
	
	uxx numCodepoints = 0;
	while (state->byteIndex < state->text.fullPiece.str.length)
	{
		u32 codepoint = 0;
		u8 utf8ByteSize = GetCodepointForUtf8Str(state->text.fullPiece.str, state->byteIndex, &codepoint);
		if (utf8ByteSize == 0)
		{
			//TODO: Should we handle invalid UTF-8 differently?
			codepoint = CharToU32(state->text.fullPiece.str.chars[state->byteIndex]);
			utf8ByteSize = 1;
		}
		numCodepoints++;
		state->byteIndex += utf8ByteSize;
	}
	state->byteIndex = 0;
	
	layoutOut->numGlyphsAlloc = numCodepoints;
	if (numCodepoints > 0)
	{
		layoutOut->glyphs = AllocArray(FontFlowGlyph, arena, layoutOut->numGlyphsAlloc);
		if (layoutOut->glyphs == nullptr) { return Result_FailedToAllocateMemory; }
		#if DEBUG_BUILD
		MyMemSet(layoutOut->glyphs, 0xCC, sizeof(FontFlowGlyph) * layoutOut->numGlyphsAlloc);
		#endif
	}
	layoutOut->flow.numGlyphsAlloc = layoutOut->numGlyphsAlloc;
	layoutOut->flow.glyphs = layoutOut->glyphs;
	
	Result flowResult = DoFontFlow(state, nullptr, &layoutOut->flow);
	Assert(layoutOut->flow.numGlyphs <= layoutOut->flow.numGlyphsAlloc);
	layoutOut->numGlyphs = layoutOut->flow.numGlyphs;
	
	return flowResult;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _GFX_FONT_FLOW_H
