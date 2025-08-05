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
#include "gfx/gfx_clay_renderer.h"
#include "ui/ui_clay.h"

#if BUILD_WITH_CLAY

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
	r32 wrapWidth; //value of 0 means no word wrapping
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
	r32 prevScrollContainerPositionY;
	
	bool selectionActive;
	uxx selectionStart;
	uxx selectionEnd;
	
	VarArray lines; //UiLargeTextLine
	r32 maxLineWidth;
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
	
	bool draggingHoriScrollbar;
	bool isDraggingHoriSmooth;
	v2 horiScrollbarGrabOffset;
	bool draggingVertScrollbar;
	bool isDraggingVertSmooth;
	v2 vertScrollbarGrabOffset;
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
	tview->idStr = AllocStr8(arena, idStr);
}

PEXP void DoUiLargeTextView(UiLargeTextView* tview,
	ClayUIRenderer* renderer, Arena* uiArena,
	KeyboardState* keyboard, MouseState* mouse,
	r32 uiScale, Clay_SizingAxis viewWidth, Clay_SizingAxis viewHeight,
	UiLargeText* text, PigFont* font, r32 fontSize, u8 fontStyle)
{
	UNUSED(keyboard);
	NotNull(tview);
	NotNull(tview->arena);
	NotNull(renderer);
	NotNull(uiArena);
	NotNull(keyboard);
	NotNull(mouse);
	if (text != nullptr && text->arena == nullptr) { text = nullptr; }
	Str8 scrollContainerIdStr = PrintInArenaStr(uiArena, "%.*s_Scroll", StrPrint(tview->idStr));
	Str8 contentIdStr = PrintInArenaStr(uiArena, "%.*s_Content", StrPrint(tview->idStr));
	Str8 horiGutterIdStr = PrintInArenaStr(uiArena, "%.*s_HScrollGutter", StrPrint(tview->idStr));
	Str8 vertGutterIdStr = PrintInArenaStr(uiArena, "%.*s_VScrollGutter", StrPrint(tview->idStr));
	Str8 horiScrollbarIdStr = PrintInArenaStr(uiArena, "%.*s_HScrollBar", StrPrint(tview->idStr));
	Str8 vertScrollbarIdStr = PrintInArenaStr(uiArena, "%.*s_VScrollBar", StrPrint(tview->idStr));
	ClayId scrollContainerId = ToClayId(scrollContainerIdStr);
	ClayId contentId = ToClayId(contentIdStr);
	ClayId horiGutterId = ToClayId(horiGutterIdStr);
	ClayId vertGutterId = ToClayId(vertGutterIdStr);
	ClayId horiScrollbarId = ToClayId(horiScrollbarIdStr);
	ClayId vertScrollbarId = ToClayId(vertScrollbarIdStr);
	rec containerRec = GetClayElementDrawRec(scrollContainerId);
	rec horiScrollbarDrawRec = GetClayElementDrawRec(horiScrollbarId);
	rec vertScrollbarDrawRec = GetClayElementDrawRec(vertScrollbarId);
	Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(scrollContainerId, false);
	bool isHoriScrollbarHovered = (mouse->isOverWindow && Clay_PointerOver(horiScrollbarId));
	bool isVertScrollbarHovered = (mouse->isOverWindow && Clay_PointerOver(vertScrollbarId));
	u16 fontId = (font != nullptr) ? GetClayUIRendererFontId(renderer, font, fontStyle) : 0;
	
	if (IsMouseBtnPressed(mouse, MouseBtn_Left) && mouse->isOverWindow)
	{
		if (!tview->draggingHoriScrollbar)
		{
			if (isHoriScrollbarHovered)
			{
				tview->draggingHoriScrollbar = true;
				tview->isDraggingHoriSmooth = false;
				tview->horiScrollbarGrabOffset = SubV2(mouse->position, horiScrollbarDrawRec.TopLeft);
			}
			else if (Clay_PointerOver(horiGutterId))
			{
				tview->draggingHoriScrollbar = true;
				tview->isDraggingHoriSmooth = true;
				tview->horiScrollbarGrabOffset = ShrinkV2(horiScrollbarDrawRec.Size, 2);
			}
		}
		if (!tview->draggingVertScrollbar)
		{
			if (isVertScrollbarHovered)
			{
				tview->draggingVertScrollbar = true;
				tview->isDraggingVertSmooth = false;
				tview->vertScrollbarGrabOffset = SubV2(mouse->position, vertScrollbarDrawRec.TopLeft);
			}
			else if (Clay_PointerOver(vertGutterId))
			{
				tview->draggingVertScrollbar = true;
				tview->isDraggingVertSmooth = true;
				tview->vertScrollbarGrabOffset = ShrinkV2(vertScrollbarDrawRec.Size, 2);
			}
		}
	}
	
	if (tview->draggingHoriScrollbar)
	{
		if (scrollData.found && scrollData.contentDimensions.Width <= scrollData.scrollContainerDimensions.Width) { tview->draggingHoriScrollbar = false; }
		else if (!IsMouseBtnDown(mouse, MouseBtn_Left)) { tview->draggingHoriScrollbar = false; }
		else
		{
			rec scrollGutterDrawRec = GetClayElementDrawRec(horiGutterId);
			r32 minX = scrollGutterDrawRec.X;
			r32 maxX = scrollGutterDrawRec.X + scrollGutterDrawRec.Width - horiScrollbarDrawRec.Width;
			if (maxX > minX)
			{
				r32 newScrollbarPos = ClampR32(mouse->position.X - tview->horiScrollbarGrabOffset.X, minX, maxX);
				r32 newScrollbarPercent = (newScrollbarPos - minX) / (maxX - minX);
				scrollData.scrollTarget->X = -((scrollData.contentDimensions.Width - scrollData.scrollContainerDimensions.Width) * newScrollbarPercent);
				if (!tview->isDraggingHoriSmooth) { scrollData.scrollPosition->X = scrollData.scrollTarget->X; }
			}
		}
		if (AbsR32(scrollData.scrollPosition->X - scrollData.scrollTarget->X) < 1.0f) { tview->isDraggingHoriSmooth = false; }
	}
	if (tview->draggingVertScrollbar)
	{
		if (scrollData.found && scrollData.contentDimensions.Height <= scrollData.scrollContainerDimensions.Height) { tview->draggingVertScrollbar = false; }
		else if (!IsMouseBtnDown(mouse, MouseBtn_Left)) { tview->draggingVertScrollbar = false; }
		else
		{
			rec scrollGutterDrawRec = GetClayElementDrawRec(vertGutterId);
			r32 minY = scrollGutterDrawRec.Y;
			r32 maxY = scrollGutterDrawRec.Y + scrollGutterDrawRec.Height - vertScrollbarDrawRec.Height;
			if (maxY > minY)
			{
				r32 newScrollbarPos = ClampR32(mouse->position.Y - tview->vertScrollbarGrabOffset.Y, minY, maxY);
				r32 newScrollbarPercent = (newScrollbarPos - minY) / (maxY - minY);
				scrollData.scrollTarget->Y = -((scrollData.contentDimensions.Height - scrollData.scrollContainerDimensions.Height) * newScrollbarPercent);
				if (!tview->isDraggingVertSmooth) { scrollData.scrollPosition->Y = scrollData.scrollTarget->Y; }
			}
		}
		if (AbsR32(scrollData.scrollPosition->Y - scrollData.scrollTarget->Y) < 1.0f) { tview->isDraggingVertSmooth = false; }
	}
	
	if (text != nullptr)
	{
		NotNull(font);
		FontAtlas* fontAtlas = GetFontAtlas(font, fontSize, fontStyle);
		NotNull(fontAtlas);
		
		if (text->lines.length == 0)
		{
			text->scrollLineIndex = 0;
			text->scrollLineOffset = 0.0f;
		}
		else if (text->scrollLineIndex >= text->lines.length)
		{
			text->scrollLineIndex = text->lines.length-1;
			text->scrollLineOffset = 0.0f;
		}
		
		text->maxLineWidth = 0.0f;
		r32 verticalOffset = 0;
		r32 wrapWidth = tview->wordWrapEnabled ? containerRec.Width : 0.0f;
		VarArrayLoop(&text->lines, lIndex)
		{
			VarArrayLoopGet(UiLargeTextLine, line, &text->lines, lIndex);
			if (!line->measured || line->font != font || line->fontSize != fontSize || line->fontStyle != fontStyle || line->wrapWidth != wrapWidth)
			{
				// PrintLine_D("Measuring line %llu with wrapWidth=%g", lIndex, wrapWidth);
				line->measure = MeasureTextEx(font, fontSize, fontStyle, false, wrapWidth, line->line);
				line->measured = true;
				line->wrapWidth = wrapWidth;
				line->font = font;
				line->fontSize = fontSize;
				line->fontStyle = fontStyle;
				line->height = CeilR32(line->measure.logicalRec.Height / fontAtlas->lineHeight) * fontAtlas->lineHeight;
			}
			if (lIndex == text->scrollLineIndex)
			{
				r32 verticalOffsetDiff = line->verticalOffset - verticalOffset;
				text->prevScrollContainerPositionY += verticalOffsetDiff;
			}
			text->maxLineWidth = MaxR32(text->maxLineWidth, line->measure.logicalRec.Width);
			line->verticalOffset = verticalOffset;
			verticalOffset += line->height;
		}
		text->totalHeight = verticalOffset;
	}
	
	v2 contentSize = V2_Zero;
	if (tview->wordWrapEnabled) { contentSize.Width = containerRec.Width; }
	if (text != nullptr)
	{
		contentSize.Height = text->totalHeight;
		if (!tview->wordWrapEnabled) { contentSize.Width = text->maxLineWidth; }
	}
	
	CLAY({ .id = tview->id,
		.layout = {
			.sizing = { .width = viewWidth, .height = viewHeight },
			.layoutDirection = CLAY_LEFT_TO_RIGHT,
			.padding = CLAY_PADDING_ALL(UISCALE_BORDER(uiScale, 1)),
		},
		.backgroundColor = MonokaiDarkGray,
		.border = { .width = CLAY_BORDER_OUTSIDE(UISCALE_BORDER(uiScale, 1)), .color = MonokaiLightGray },
	})
	{
		CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) }, .layoutDirection = CLAY_TOP_TO_BOTTOM } })
		{
			CLAY({ .id = scrollContainerId,
				.layout = {
					.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
				},
				.scroll = { .vertical = true, .horizontal = !tview->wordWrapEnabled, .scrollLag = 5.0f },
			})
			{
				scrollData = Clay_GetScrollContainerData(scrollContainerId, false);
				// Handle scroll changes moving our scrollLineIndex/scrollLineOffset
				if (scrollData.found && scrollData.scrollPosition->Y != text->prevScrollContainerPositionY)
				{
					r32 scrollChange = scrollData.scrollPosition->Y - text->prevScrollContainerPositionY;
					if (scrollChange > 0.0f)
					{
						while (scrollChange > 0 && text->scrollLineIndex+1 < text->lines.length)
						{
							UiLargeTextLine* line = VarArrayGet(UiLargeTextLine, &text->lines, text->scrollLineIndex);
							if (text->scrollLineOffset + scrollChange >= line->height)
							{
								text->scrollLineIndex++;
								scrollChange -= (line->height - text->scrollLineOffset);
								text->scrollLineOffset = 0.0f;
							}
							else { break; }
						}
						text->scrollLineOffset += scrollChange;
					}
					else
					{
						while (scrollChange < 0 && text->scrollLineIndex > 0)
						{
							if (scrollChange <= text->scrollLineOffset)
							{
								text->scrollLineIndex--;
								scrollChange += text->scrollLineOffset;
								text->scrollLineOffset = VarArrayGet(UiLargeTextLine, &text->lines, text->scrollLineIndex)->height;
							}
							else { break; }
						}
						text->scrollLineOffset = MaxR32(0.0f, text->scrollLineOffset + scrollChange);
					}
				}
				
				// +==============================+
				// |        Render Content        |
				// +==============================+
				CLAY({ .id = contentId,
					.layout = {
						.sizing = {
							.width = tview->wordWrapEnabled ? CLAY_SIZING_GROW(0) : CLAY_SIZING_FIXED(contentSize.Width),
							.height = CLAY_SIZING_FIXED(contentSize.Height)
						},
					},
				})
				{
					rec contentRec = GetClayElementDrawRec(contentId);
					if (text != nullptr)
					{
						v2 textOffset = contentRec.TopLeft;
						//TODO: This loop should eventually jump straight to scrollLineIndex
						VarArrayLoop(&text->lines, lIndex)
						{
							VarArrayLoopGet(UiLargeTextLine, line, &text->lines, lIndex);
							rec lineRec = NewRecV(
								AddV2(textOffset, NewV2(0, line->verticalOffset)),
								NewV2(tview->wordWrapEnabled ? containerRec.Width : line->measure.logicalRec.Width, line->height)
							);
							r32 extraRenderHeight = MaxR32(100, containerRec.Height);
							
							if (lineRec.Y >= containerRec.Y + containerRec.Height + extraRenderHeight) { break; }
							if (lineRec.Y + lineRec.Height >= containerRec.Y - extraRenderHeight)
							{
								//NOTE: When doing word wrapping (no horizontal scrolling) we need to set width to GROW not FIXED because
								//      Clay will push the container off the rightside of the window if it thinks it needs to contain the content
								//      and this causes us to never shrink and re-measure at a smaller wrapWidth
								Clay_SizingAxis lineContainerWidth = tview->wordWrapEnabled ? CLAY_SIZING_GROW(0) : CLAY_SIZING_FIXED(lineRec.Width);
								CLAY({
									.layout = {
										.sizing = { .width = lineContainerWidth, .height = CLAY_SIZING_FIXED(lineRec.Height) },
									},
									.floating = {
										.attachTo = CLAY_ATTACH_TO_PARENT,
										.attachPoints = { .parent = CLAY_ATTACH_POINT_LEFT_TOP, .element = CLAY_ATTACH_POINT_LEFT_TOP },
										.offset = SubV2(lineRec.TopLeft, contentRec.TopLeft),
										.pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
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
											.userData = { .wrapWidth = line->wrapWidth },
									}));
								}
							}
						}
					}
				}
			}
			
			// +==============================+
			// |    Render Hori Scrollbar     |
			// +==============================+
			//NOTE: I don't like changing the size of the innerContainer so we always have a gutter container to reserve space, even if a scrollbar is not needed,
			//      but we only render the scrollbar inside the gutter if the list is taller than the viewable area
			CLAY({ .id = horiGutterId,
				.layout = {
					.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(UISCALE_R32(uiScale, 8 + (1*2))) },
					.padding = { .left = UISCALE_U16(uiScale, 1), .right = UISCALE_U16(uiScale, 1), }
				},
			})
			{
				if (scrollData.found && scrollData.contentDimensions.Width > scrollData.scrollContainerDimensions.Width)
				{
					r32 scrollbarXPercent = 0.0f;
					r32 scrollbarSizePercent = 1.0f;
					if (scrollData.found && scrollData.contentDimensions.Width > scrollData.scrollContainerDimensions.Width)
					{
						scrollbarSizePercent = ClampR32(scrollData.scrollContainerDimensions.Width / scrollData.contentDimensions.Width, 0.0f, 1.0f);
						scrollbarXPercent = ClampR32(-scrollData.scrollPosition->X / (scrollData.contentDimensions.Width - scrollData.scrollContainerDimensions.Width), 0.0f, 1.0f);
					}
					rec scrollGutterDrawRec = GetClayElementDrawRec(horiGutterId);
					v2 scrollBarSize = NewV2(
						MaxR32(MinR32(UISCALE_R32(uiScale, 20), scrollGutterDrawRec.Width*0.25f), scrollGutterDrawRec.Width * scrollbarSizePercent),
						UISCALE_R32(uiScale, 8)
					);
					r32 scrollBarOffsetX = ClampR32((scrollGutterDrawRec.Width - scrollBarSize.Width) * scrollbarXPercent, 0.0f, scrollGutterDrawRec.Width);
					
					CLAY({ .id = horiScrollbarId,
						.layout = {
							.sizing = { .width = CLAY_SIZING_FIXED(scrollBarSize.X), .height = CLAY_SIZING_FIXED(scrollBarSize.Y) },
						},
						.floating = {
							.attachTo = CLAY_ATTACH_TO_PARENT,
							.offset = NewV2(scrollBarOffsetX, UISCALE_R32(uiScale, 1)),
						},
						.backgroundColor = (isHoriScrollbarHovered || tview->draggingHoriScrollbar) ? MonokaiWhite : MonokaiLightGray,
						.cornerRadius = CLAY_CORNER_RADIUS(scrollBarSize.Width/2.0f),
					}) {}
				}
			}
		}
		
		// +==============================+
		// |    Render Vert Scrollbar     |
		// +==============================+
		//NOTE: I don't like changing the size of the innerContainer so we always have a gutter container to reserve space, even if a scrollbar is not needed,
		//      but we only render the scrollbar inside the gutter if the list is taller than the viewable area
		CLAY({ .id = vertGutterId,
			.layout = {
				.sizing = { .width = CLAY_SIZING_FIXED(UISCALE_R32(uiScale, 8 + (1*2))), .height = CLAY_SIZING_GROW(0) },
				.padding = { .left = UISCALE_U16(uiScale, 1), .right = UISCALE_U16(uiScale, 1), }
			},
		})
		{
			if (scrollData.found && scrollData.contentDimensions.Height > scrollData.scrollContainerDimensions.Height)
			{
				r32 scrollbarYPercent = 0.0f;
				r32 scrollbarSizePercent = 1.0f;
				if (scrollData.found && scrollData.contentDimensions.Height > scrollData.scrollContainerDimensions.Height)
				{
					scrollbarSizePercent = ClampR32(scrollData.scrollContainerDimensions.Height / scrollData.contentDimensions.Height, 0.0f, 1.0f);
					scrollbarYPercent = ClampR32(-scrollData.scrollPosition->Y / (scrollData.contentDimensions.Height - scrollData.scrollContainerDimensions.Height), 0.0f, 1.0f);
				}
				rec scrollGutterDrawRec = GetClayElementDrawRec(vertGutterId);
				v2 scrollBarSize = NewV2(
					UISCALE_R32(uiScale, 8),
					MaxR32(MinR32(UISCALE_R32(uiScale, 20), scrollGutterDrawRec.Height*0.25f), scrollGutterDrawRec.Height * scrollbarSizePercent)
				);
				r32 scrollBarOffsetY = ClampR32((scrollGutterDrawRec.Height - scrollBarSize.Height) * scrollbarYPercent, 0.0f, scrollGutterDrawRec.Height);
				
				CLAY({ .id = vertScrollbarId,
					.layout = {
						.sizing = { .width = CLAY_SIZING_FIXED(scrollBarSize.X), .height = CLAY_SIZING_FIXED(scrollBarSize.Y) },
					},
					.floating = {
						.attachTo = CLAY_ATTACH_TO_PARENT,
						.offset = NewV2(UISCALE_R32(uiScale, 1), scrollBarOffsetY),
					},
					.backgroundColor = (isVertScrollbarHovered || tview->draggingVertScrollbar) ? MonokaiWhite : MonokaiLightGray,
					.cornerRadius = CLAY_CORNER_RADIUS(scrollBarSize.Width/2.0f),
				}) {}
			}
		}
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_CLAY

#endif //  _UI_CLAY_LARGE_TEXTVIEW_H
