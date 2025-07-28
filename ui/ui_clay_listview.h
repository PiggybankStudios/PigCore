/*
File:   ui_clay_listview.h
Author: Taylor Robbins
Date:   07\22\2025
*/

#ifndef _UI_CLAY_LISTVIEW_H
#define _UI_CLAY_LISTVIEW_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "mem/mem_arena.h"
#include "struct/struct_string.h"
#include "ui/ui_clay.h"
#include "input/input_keys.h"
#include "input/input_btn_state.h"
#include "input/input_mouse_btns.h"
#include "struct/struct_vectors.h"
#include "gfx/gfx_clay_renderer.h"
#include "gfx/gfx_font.h"

#if BUILD_WITH_CLAY

typedef plex UiListView UiListView;
plex UiListView
{
	Arena* arena;
	Str8 idStr;
	ClayId id;
	r32 smoothScrollDivisor;
	
	bool selectionActive;
	Str8 selectedIdStr;
	bool selectionChanged;
	uxx selectionIndex; //this is only accurate after DoUiListView is called if no changes were made to the items list, otherwise use selectedIdStr as a true indicator of which item is selected
	void* contextPntr;
	
	bool draggingScrollbar;
	bool isDraggingSmooth;
	v2 scrollbarGrabOffset;
};

//TODO: Why can't we have a UiListViewItem as the second parameter type?? Even with "struct" prefix it isn't working for some reason
#define UI_LIST_VIEW_ITEM_RENDER_DEF(functionName) void functionName(UiListView* list, void* item, uxx index, bool isSelected, bool isHovered)
typedef UI_LIST_VIEW_ITEM_RENDER_DEF(UiListViewItemRender_f);

typedef plex UiListViewItem UiListViewItem;
plex UiListViewItem
{
	Str8 idStr;
	
	Str8 displayStr;
	PigFont* font;
	u8 fontStyle;
	r32 fontSize;
	TextContraction contraction; //default is TextContraction_EllipseMiddle
	
	UiListViewItemRender_f* render;
	void* contextPntr;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void FreeUiListView(UiListView* list);
	void InitUiListView(Arena* arena, Str8 idStr, UiListView* list);
	void DoUiListView(UiListView* list, ClayUIRenderer* renderer, Arena* uiArena, KeyboardState* keyboard, MouseState* mouse, r32 uiScale, Clay_SizingAxis listWidth, Clay_SizingAxis listHeight, u16 itemGap, uxx numItems, UiListViewItem* items);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void FreeUiListView(UiListView* list)
{
	NotNull(list);
	if (list->arena != nullptr) { FreeStr8(list->arena, &list->idStr); }
	ClearPointer(list);
}

PEXP void InitUiListView(Arena* arena, Str8 idStr, UiListView* list)
{
	NotNull(arena);
	NotNull(list);
	ClearPointer(list);
	list->arena = arena;
	list->idStr = AllocStr8(arena, idStr);
	list->id = ToClayId(list->idStr);
	list->smoothScrollDivisor = 5.0f;
}

//NOTE: Font info is only required if the items do not have a render callback
PEXP void DoUiListView(UiListView* list,
	ClayUIRenderer* renderer, Arena* uiArena,
	KeyboardState* keyboard, MouseState* mouse,
	r32 uiScale, Clay_SizingAxis listWidth, Clay_SizingAxis listHeight, u16 itemGap,
	uxx numItems, UiListViewItem* items)
{
	UNUSED(keyboard);
	ScratchBegin1(scratch, uiArena);
	ClayId innerContainerId = ToClayIdPrint(uiArena, "%.*s_Inner", StrPrint(list->idStr));
	ClayId gutterId = ToClayIdPrint(uiArena, "%.*s_ScrollGutter", StrPrint(list->idStr));
	ClayId scrollbarId = ToClayIdPrint(uiArena, "%.*s_ScrollBar", StrPrint(list->idStr));
	rec scrollbarDrawRec = GetClayElementDrawRec(scrollbarId);
	Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(innerContainerId, false);
	bool isScrollbarHovered = (mouse->isOverWindow && Clay_PointerOver(scrollbarId));
	bool clickedInScrollArea = false;
	
	if (IsMouseBtnPressed(mouse, MouseBtn_Left) && mouse->isOverWindow && !list->draggingScrollbar)
	{
		if (isScrollbarHovered)
		{
			list->draggingScrollbar = true;
			list->isDraggingSmooth = false;
			list->scrollbarGrabOffset = SubV2(mouse->position, scrollbarDrawRec.TopLeft);
			clickedInScrollArea = true;
		}
		else if (Clay_PointerOver(gutterId))
		{
			list->draggingScrollbar = true;
			list->isDraggingSmooth = true;
			list->scrollbarGrabOffset = ShrinkV2(scrollbarDrawRec.Size, 2);
			clickedInScrollArea = true;
		}
	}
	
	if (list->draggingScrollbar)
	{
		if (scrollData.found && scrollData.contentDimensions.Height <= scrollData.scrollContainerDimensions.Height) { list->draggingScrollbar = false; }
		else if (!IsMouseBtnDown(mouse, MouseBtn_Left)) { list->draggingScrollbar = false; }
		else
		{
			rec scrollGutterDrawRec = GetClayElementDrawRec(gutterId);
			r32 minY = scrollGutterDrawRec.Y;
			r32 maxY = scrollGutterDrawRec.Y + scrollGutterDrawRec.Height - scrollbarDrawRec.Height;
			if (maxY > minY)
			{
				r32 newScrollbarPos = ClampR32(mouse->position.Y - list->scrollbarGrabOffset.Y, minY, maxY);
				r32 newScrollbarPercent = (newScrollbarPos - minY) / (maxY - minY);
				scrollData.scrollTarget->Y = -((scrollData.contentDimensions.Height - scrollData.scrollContainerDimensions.Height) * newScrollbarPercent);
				if (!list->isDraggingSmooth) { scrollData.scrollPosition->Y = scrollData.scrollTarget->Y; }
			}
		}
		if (scrollData.scrollPosition->Y == scrollData.scrollTarget->Y) { list->isDraggingSmooth = false; }
	}
	
	CLAY({ .id = list->id,
		.layout = {
			.sizing = { .width = listWidth, .height = listHeight },
			.padding = CLAY_PADDING_ALL(UISCALE_BORDER(uiScale, 1)),
		},
		.backgroundColor = MonokaiDarkGray,
		.border = { .width = CLAY_BORDER_OUTSIDE(UISCALE_BORDER(uiScale, 1)), .color = MonokaiLightGray },
	})
	{
		// +==============================+
		// |         Render List          |
		// +==============================+
		CLAY({ .id = innerContainerId,
			.layout = {
				.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
				.childGap = itemGap,
				.layoutDirection = CLAY_TOP_TO_BOTTOM,
			},
			.scroll = { .vertical = true, .scrollLag = list->smoothScrollDivisor },
		})
		{
			bool foundSelectedItem = false;
			bool clickedOnItem = false;
			for (uxx iIndex = 0; iIndex < numItems; iIndex++)
			{
				UiListViewItem* item = &items[iIndex];
				Str8 itemIdStr = IsEmptyStr(item->idStr) ? PrintInArenaStr(scratch, "Item%llu", iIndex) : item->idStr;
				Str8 fullIdStr = PrintInArenaStr(uiArena, "%.*s_Item_%.*s", StrPrint(list->idStr), StrPrint(itemIdStr));
				ClayId fullId = ToClayIdEx(fullIdStr, iIndex);
				bool isSelected = (list->selectionActive && StrExactEquals(list->selectedIdStr, itemIdStr));
				if (isSelected)
				{
					foundSelectedItem = true;
					list->selectionIndex = iIndex;
				}
				bool isHovered = (mouse->isOverWindow && Clay_PointerOver(list->id) && Clay_PointerOver(fullId));
				
				if (isHovered && IsMouseBtnPressed(mouse, MouseBtn_Left) && !clickedOnItem && !clickedInScrollArea)
				{
					clickedOnItem = true;
					if (!list->selectionActive || !StrExactEquals(list->selectedIdStr, itemIdStr))
					{
						if (list->selectionActive) { FreeStr8(list->arena, &list->selectedIdStr); }
						list->selectedIdStr = AllocStr8(list->arena, itemIdStr);
						list->selectionActive = true;
						list->selectionIndex = iIndex;
						foundSelectedItem = true;
						list->selectionChanged = true;
					}
				}
				
				CLAY({ .id = fullId,
					.layout = {
						.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0) },
						.layoutDirection = CLAY_LEFT_TO_RIGHT,
						.padding = CLAY_PADDING_ALL(UISCALE_U16(uiScale, 4)),
						.childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
					},
					.backgroundColor = (isSelected ? MonokaiLightGray : (isHovered ? MonokaiBack : Transparent)),
				})
				{
					if (item->render != nullptr)
					{
						item->render(list, item, iIndex, isSelected, isHovered);
					}
					else
					{
						NotNull(renderer);
						NotNull(item->font);
						u16 fontId = GetClayUIRendererFontId(renderer, item->font, item->fontStyle);
						CLAY_TEXT(
							item->displayStr,
							CLAY_TEXT_CONFIG({
								.fontId = fontId,
								.fontSize = (u16)item->fontSize,
								.textColor = isSelected ? MonokaiDarkGray : MonokaiWhite,
								.wrapMode = CLAY_TEXT_WRAP_NONE,
								.textAlignment = CLAY_TEXT_ALIGN_SHRINK,
								.userData = { .contraction = item->contraction },
						}));
					}
				}
			}
			
			// Add an empty container to the bottom of the list to make sure we have some space that the user can click to deselect
			r32 emptyRowHeight = MinR32(scrollData.scrollContainerDimensions.Height*0.25f, UISCALE_R32(uiScale, 30.0f));
			CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(emptyRowHeight) } } }) {}
			
			bool isListHovered = (mouse->isOverWindow && Clay_PointerOver(list->id));
			if (isListHovered && IsMouseBtnPressed(mouse, MouseBtn_Left) && !clickedOnItem && !clickedInScrollArea && list->selectionActive)
			{
				FreeStr8(list->arena, &list->selectedIdStr);
				list->selectedIdStr = Str8_Empty;
				list->selectionActive = false;
				list->selectionIndex = numItems;
				list->selectionChanged = true;
			}
			if (!foundSelectedItem && list->selectionActive)
			{
				// PrintLine_D("Couldn't find \"%.*s\" out of %llu items", StrPrint(list->selectedIdStr), numItems);
				FreeStr8(list->arena, &list->selectedIdStr);
				list->selectedIdStr = Str8_Empty;
				list->selectionActive = false;
				list->selectionIndex = numItems;
				list->selectionChanged = true;
			}
		}
		
		// +==============================+
		// |       Render Scrollbar       |
		// +==============================+
		//NOTE: I don't like changing the size of the innerContainer so we always have a gutter container to reserve space, even if a scrollbar is not needed,
		//      but we only render the scrollbar inside the gutter if the list is taller than the viewable area
		CLAY({ .id = gutterId,
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
				rec scrollGutterDrawRec = GetClayElementDrawRec(gutterId);
				v2 scrollBarSize = NewV2(
					UISCALE_R32(uiScale, 8),
					scrollGutterDrawRec.Height * scrollbarSizePercent
				);
				r32 scrollBarOffsetY = ClampR32((scrollGutterDrawRec.Height - scrollBarSize.Height) * scrollbarYPercent, 0.0f, scrollGutterDrawRec.Height);
				
				CLAY({ .id = scrollbarId,
					.layout = {
						.sizing = { .width = CLAY_SIZING_FIXED(scrollBarSize.X), .height = CLAY_SIZING_FIXED(scrollBarSize.Y) },
					},
					.floating = {
						.attachTo = CLAY_ATTACH_TO_PARENT,
						.offset = NewV2(UISCALE_R32(uiScale, 1), scrollBarOffsetY),
					},
					.backgroundColor = (isScrollbarHovered || list->draggingScrollbar) ? MonokaiWhite : MonokaiLightGray,
					.cornerRadius = CLAY_CORNER_RADIUS(scrollBarSize.Width/2.0f),
				}) {}
			}
		}
	}
	
	ScratchEnd(scratch);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_CLAY

#endif //  _UI_CLAY_LISTVIEW_H