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
#include "struct/struct_var_array.h"
#include "gfx/gfx_font_flow.h"
#include "ui/ui_clay.h"

typedef plex UiLargeTextLine UiLargeTextLine;
plex UiLargeTextLine
{
	Str8 line;
	uxx startIndex;
	uxx endIndex;
	r32 verticalOffset;
	TextMeasure measure;
};

typedef plex UiLargeText UiLargeText;
plex UiLargeText
{
	Arena* arena;
	Str8 text;
	bool selectionActive;
	uxx selectionStart;
	uxx selectionEnd;
	r32 measureMaxWidth; //value of 0 means no word wrapping
	r32 totalHeight;
	VarArray lines; //UiLargeTextLine
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
	void UpdateUiLargeTextView(UiLargeTextView* tview, ClayUIRenderer* renderer, Arena* uiArena, KeyboardState* keyboard, MouseState* mouse, r32 uiScale, Clay_SizingAxis viewWidth, Clay_SizingAxis viewHeight, UiLargeText* text);
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

PEXPI void InitUiLargeTextView(Arena* arena, Str8 idStr, UiLargeTextView* tview)
{
	NotNull(arena);
	NotNull(tview);
	ClearPointer(tview);
	tview->arena = arena;
}

PEXP void UpdateUiLargeTextView(UiLargeTextView* tview,
	ClayUIRenderer* renderer, Arena* uiArena,
	KeyboardState* keyboard, MouseState* mouse,
	r32 uiScale, Clay_SizingAxis viewWidth, Clay_SizingAxis viewHeight,
	UiLargeText* text)
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
	
	v2 contentSize = V2_Zero;
	if (!tview->wordWrapEnabled && containerRec.width > 0) { contentSize.Width = containerRec.Width; }
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
			.scroll = { .vertical = true, .horizontal = !tview->wordWrapEnabled, scrollLag = 5.0f },
		})
		{
			CLAY({ .id = contentId,
				.layout = {
					.sizing = { .width = CLAY_SIZING_FIXED(contentSize.Width), .height = CLAY_SIZING_FIXED(contentSize.Height) },
				},
			})
			{
				rec contentRec = GetClayElementDrawRec(contentId);
			}
		}
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _UI_CLAY_LARGE_TEXTVIEW_H
