/*
File:   ui_clay_large_textview.h
Author: Taylor Robbins
Date:   07\27\2025
Description:
	** When we want to view large chunks of text (esp. more than 64k words, the default word measure limit for Clay itself)
	** we need a special kind of widget that does minimal text layout and re-measuring
	** This scrollable view handles word wrapping and minimal rendering for an arbitrarily
	** large text buffer, as well as smooth and efficient re-measuring when the width
	** of the available area changes or word wrapping is toggled
*/

#ifndef _UI_CLAY_LARGE_TEXTVIEW_H
#define _UI_CLAY_LARGE_TEXTVIEW_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
#include "struct/struct_string.h"
#include "struct/struct_vectors.h"
#include "struct/struct_rectangles.h"
#include "struct/struct_var_array.h"
#include "gfx/gfx_font_flow.h"
#include "ui/ui_clay.h"

typedef plex UiLargeTextLine UiLargeTextLine;
plex UiLargeTextLine
{
	Str8 line;
	uxx startIndex;
	uxx endIndex;
	
	bool measured;
	PigFont* font;
	r32 fontSize;
	u8 fontStyle;
	r32 maxWidth; //value of 0 means no word wrapping
	TextMeasure measure;
	
	r32 verticalOffset;
	r32 height;
};

typedef plex UiLargeText UiLargeText;
plex UiLargeText
{
	Arena* arena;
	Str8 text;
	
	uxx scrollLineIndex;
	r32 scrollLineOffset;
	
	bool selectionActive;
	uxx selectionStart;
	uxx selectionEnd;
	
	VarArray lines; //UiLargeTextLine
	r32 totalHeight;
};

typedef plex UiLargeTextView UiLargeTextView;
plex UiLargeTextView
{
	Arena* arena;
	Str8 idStr;
	ClayId id;
	bool wordWrapEnabled;
	UiLargeText* text;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeUiLargeText(UiLargeText* text);
	PIG_CORE_INLINE void FreeUiLargeTextView(UiLargeTextView* tview);
	PIG_CORE_INLINE void InitUiLargeText(Arena* arena, Str8 textStr, UiLargeText* text);
	PIG_CORE_INLINE void InitUiLargeTextView(Arena* arena, Str8 idStr, UiLargeTextView* tview);
	void DoUiLargeTextView(UiLargeTextView* tview, ClayUIRenderer* renderer, Arena* uiArena, KeyboardState* keyboard, MouseState* mouse, r32 uiScale, Clay_SizingAxis viewWidth, Clay_SizingAxis viewHeight, UiLargeText* text, PigFont* font, r32 fontSize, u8 fontStyle);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void FreeUiLargeText(UiLargeText* text)
{
	NotNull(text);
	if (text->arena != nullptr)
	{
		FreeVarArray(&text->lines);
		FreeStr8(text->arena, &text->text);
	}
	ClearPointer(text);
}

PEXPI void FreeUiLargeTextView(UiLargeTextView* tview)
{
	NotNull(tview);
	if (tview->arena != nullptr)
	{
		FreeStr8(tview->arena, &tview->idStr);
	}
	ClearPointer(tview);
}

PEXPI void InitUiLargeText(Arena* arena, Str8 textStr, UiLargeText* text)
{
	NotNull(arena);
	NotNull(text);
	ClearPointer(text);
	text->arena = arena;
	text->text = AllocStr8(arena, textStr);
	uxx numLines = 1;
	for (uxx cIndex = 0; cIndex < text->text.length; cIndex++)
	{
		if (text->text.chars[cIndex] == '\n') { numLines++; }
		if (text->text.chars[cIndex] == '\r' && cIndex+1 < text->text.length && text->text.chars[cIndex+1] == '\n') { numLines++; cIndex++; }
	}
	InitVarArrayWithInitial(UiLargeTextLine, &text->lines, arena, numLines);
	
	uxx lineStart = 0;
	for (uxx cIndex = 0; cIndex <= text->text.length; cIndex++)
	{
		if (cIndex == text->text.length ||
			text->text.chars[cIndex] == '\n' || 
			(text->text.chars[cIndex] == '\r' && cIndex+1 < text->text.length && text->text.chars[cIndex+1] == '\n'))
		{
			UiLargeTextLine* newLine = VarArrayAdd(UiLargeTextLine, &text->lines);
			NotNull(newLine);
			ClearPointer(newLine);
			newLine->startIndex = lineStart;
			newLine->endIndex = cIndex;
			newLine->line = StrSlice(text->text, newLine->startIndex, newLine->endIndex);
			if (cIndex+1 < text->text.length && text->text.chars[cIndex] == '\r' && text->text.chars[cIndex+1] == '\n') { cIndex++; }
			lineStart = cIndex+1;
		}
	}
	Assert(text->lines.length == numLines);
}

PEXPI void InitUiLargeTextView(Arena* arena, Str8 idStr, UiLargeTextView* tview)
{
	NotNull(arena);
	NotNull(tview);
	ClearPointer(tview);
	tview->arena = arena;
}

PEXP void DoUiLargeTextView(UiLargeTextView* tview,
	ClayUIRenderer* renderer, Arena* uiArena,
	KeyboardState* keyboard, MouseState* mouse,
	r32 uiScale, Clay_SizingAxis viewWidth, Clay_SizingAxis viewHeight,
	UiLargeText* text, PigFont* font, r32 fontSize, u8 fontStyle)
{
	NotNull(tview);
	NotNull(tview->arena);
	NotNull(renderer);
	NotNull(uiArena);
	NotNull(keyboard);
	NotNull(mouse);
	if (text != nullptr && text->arena == nullptr) { text = nullptr; }
	Str8 scrollContainerIdStr = PrintInArenaStr(uiArena, "%.*s_Scroll", StrPrint(tview->idStr));
	Str8 contentIdStr = PrintInArenaStr(uiArena, "%.*s_Content", StrPrint(tview->idStr));
	ClayId scrollContainerId = ToClayId(scrollContainerIdStr);
	ClayId contentId = ToClayId(contentIdStr);
	rec containerRec = GetClayElementDrawRec(scrollContainerId);
	u16 fontId = (font != nullptr) ? GetClayUIRendererFontId(renderer, font, fontStyle) : 0;
	
	if (text != nullptr)
	{
		NotNull(font);
		FontAtlas* fontAtlas = GetFontAtlas(font, fontSize, fontStyle);
		NotNull(fontAtlas);
		
		r32 verticalOffset = 0;
		VarArrayLoop(&text->lines, lIndex)
		{
			VarArrayLoopGet(UiLargeTextLine, line, &text->lines, lIndex);
			if (!line->measured || line->font != font || line->fontSize != fontSize || line->fontStyle != fontStyle)
			{
				//TODO: Once we have support for word wrapping in MeasureText we should pass the line->maxWidth
				r32 maxWidth = tview->wordWrapEnabled ? containerRec.Width : 0.0f;
				line->measure = MeasureTextEx(font, fontSize, fontStyle, false, line->line);
				line->measured = true;
				line->maxWidth = maxWidth;
				line->font = font;
				line->fontSize = fontSize;
				line->fontStyle = fontStyle;
				line->height = MaxR32(line->measure.logicalRec.Height, fontAtlas->lineHeight);
			}
			line->verticalOffset = verticalOffset;
			verticalOffset += line->height;
		}
		text->totalHeight = verticalOffset;
	}
	
	v2 contentSize = V2_Zero;
	if (!tview->wordWrapEnabled && containerRec.Width > 0) { contentSize.Width = containerRec.Width; }
	if (text != nullptr) { contentSize.Height = text->totalHeight; }
	
	CLAY({ .id = tview->id,
		.layout = {
			.sizing = { .width = viewWidth, .height = viewHeight },
			.padding = CLAY_PADDING_ALL(UISCALE_BORDER(uiScale, 1)),
		},
		.backgroundColor = MonokaiDarkGray,
		.border = { .width = CLAY_BORDER_OUTSIDE(UISCALE_BORDER(uiScale, 1)), .color = MonokaiLightGray },
	})
	{
		CLAY({ .id = scrollContainerId,
			.layout = {
				.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
			},
			.scroll = { .vertical = true, .horizontal = !tview->wordWrapEnabled, .scrollLag = 5.0f },
		})
		{
			CLAY({ .id = contentId,
				.layout = {
					.sizing = { .width = CLAY_SIZING_FIXED(contentSize.Width), .height = CLAY_SIZING_FIXED(contentSize.Height) },
				},
			})
			{
				rec contentRec = GetClayElementDrawRec(contentId);
				if (text != nullptr)
				{
					v2 textOffset = contentRec.TopLeft;
					VarArrayLoop(&text->lines, lIndex)
					{
						VarArrayLoopGet(UiLargeTextLine, line, &text->lines, lIndex);
						rec lineRec = NewRecV(
							AddV2(textOffset, NewV2(0, line->verticalOffset)),
							NewV2(tview->wordWrapEnabled ? containerRec.Width : line->measure.logicalRec.Width, line->height)
						);
						
						if (lineRec.Y >= containerRec.Y + containerRec.Height) { break; }
						if (lineRec.Y + lineRec.Height >= containerRec.Y)
						{
							// v2 textPos = AddV2(lineRec.TopLeft, NewV2(0, -line->measure.logicalRec.Y));
							CLAY({
								.layout = {
									.sizing = { .width = CLAY_SIZING_FIXED(lineRec.Width), .height = CLAY_SIZING_FIXED(lineRec.Height) },
								},
								.floating = {
									.attachTo = CLAY_ATTACH_TO_PARENT,
									.attachPoints = { .parent = CLAY_ATTACH_POINT_LEFT_TOP, .element = CLAY_ATTACH_POINT_LEFT_TOP },
									.offset = SubV2(lineRec.TopLeft, contentRec.TopLeft),
								},
							})
							{
								CLAY_TEXT(
									line->line,
									CLAY_TEXT_CONFIG({
										.fontId = fontId,
										.fontSize = (u16)fontSize,
										.textColor = MonokaiWhite,
										.wrapMode = CLAY_TEXT_WRAP_NONE,
										.textAlignment = CLAY_TEXT_ALIGN_LEFT,
								}));
							}
						}
					}
				}
			}
		}
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _UI_CLAY_LARGE_TEXTVIEW_H
