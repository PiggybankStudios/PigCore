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

typedef plex UiListView UiListView;
plex UiListView
{
	Arena* arena;
	Str8 idStr;
	ClayId id;
	bool selectionActive;
	Str8 selectedIdStr;
	bool selectionChanged;
	uxx selectionIndex; //this is only accurate after DoUiListView is called if no changes were made to the items list, otherwise use selectedIdStr as a true indicator of which item is selected
	void* contextPntr;
};

//TODO: Why can't we have a UiListViewItem as the second parameter type?? Even with "struct" prefix it isn't working for some reason
#define UI_LIST_VIEW_ITEM_RENDER_DEF(functionName) void functionName(UiListView* list, void* item, uxx index, bool isSelected, bool isHovered)
typedef UI_LIST_VIEW_ITEM_RENDER_DEF(UiListViewItemRender_f);

typedef plex UiListViewItem UiListViewItem;
plex UiListViewItem
{
	Str8 idStr;
	Str8 displayStr; //only needed if render callback is nullptr
	UiListViewItemRender_f* render;
	void* contextPntr;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void FreeUiListView(UiListView* list);
	void InitUiListView(Arena* arena, Str8 idStr, UiListView* list);
	void DoUiListView(UiListView* list, ClayUIRenderer* renderer, Arena* uiArena, KeyboardState* keyboard, MouseState* mouse, Clay_SizingAxis listWidth, Clay_SizingAxis listHeight, u16 itemGap, uxx numItems, UiListViewItem* items, PigFont* font, u8 fontStyle, r32 fontSize, r32 uiScale);
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
}

//NOTE: Font info is only required if the items do not have a render callback
PEXP void DoUiListView(UiListView* list,
	ClayUIRenderer* renderer, Arena* uiArena,
	KeyboardState* keyboard, MouseState* mouse,
	Clay_SizingAxis listWidth, Clay_SizingAxis listHeight, u16 itemGap,
	uxx numItems, UiListViewItem* items,
	PigFont* font, u8 fontStyle, r32 fontSize, r32 uiScale)
{
	ScratchBegin1(scratch, uiArena);
	CLAY({ .id = list->id,
		.layout = {
			.sizing = { .width = listWidth, .height = listHeight },
			.padding = CLAY_PADDING_ALL(UISCALE_BORDER(uiScale, 1)),
		},
		.backgroundColor = MonokaiDarkGray,
		.border = { .width = CLAY_BORDER_OUTSIDE(UISCALE_BORDER(uiScale, 1)), .color = MonokaiLightGray },
	})
	{
		CLAY({
			.layout = {
				.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
				.childGap = itemGap,
				.layoutDirection = CLAY_TOP_TO_BOTTOM,
			},
			.scroll = { .vertical = true },
		})
		{
			bool foundSelectedItem = false;
			bool clickedOnItem = false;
			for (uxx iIndex = 0; iIndex < numItems; iIndex++)
			{
				UiListViewItem* item = &items[iIndex];
				Str8 itemIdStr = IsEmptyStr(item->idStr) ? PrintInArenaStr(scratch, "Item%llu", iIndex) : item->idStr;
				Str8 fullIdStr = PrintInArenaStr(uiArena, "%.*s_%.*s", StrPrint(list->idStr), StrPrint(itemIdStr));
				ClayId fullId = ToClayIdEx(fullIdStr, iIndex);
				bool isSelected = (list->selectionActive && StrExactEquals(list->selectedIdStr, itemIdStr));
				if (isSelected)
				{
					foundSelectedItem = true;
					list->selectionIndex = iIndex;
				}
				bool isHovered = (mouse->isOverWindow && Clay_PointerOver(list->id) && Clay_PointerOver(fullId));
				
				if (isHovered && IsMouseBtnPressed(mouse, MouseBtn_Left) && !clickedOnItem)
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
						NotNull(font);
						u16 fontId = GetClayUIRendererFontId(renderer, font, fontStyle);
						// FontAtlas* fontAtlas = GetFontAtlas(font, fontSize, fontStyle);
						// NotNull(fontAtlas);
						CLAY_TEXT(
							item->displayStr,
							CLAY_TEXT_CONFIG({
								.fontId = fontId,
								.fontSize = (u16)fontSize,
								.textColor = MonokaiWhite,
								.wrapMode = CLAY_TEXT_WRAP_NONE,
								.textAlignment = CLAY_TEXT_ALIGN_SHRINK,
								.userData = { .contraction = TextContraction_EllipseMiddle },
						}));
					}
				}
			}
			
			bool isListHovered = (mouse->isOverWindow && Clay_PointerOver(list->id));
			if (isListHovered && IsMouseBtnPressed(mouse, MouseBtn_Left) && !clickedOnItem && list->selectionActive)
			{
				FreeStr8(list->arena, &list->selectedIdStr);
				list->selectedIdStr = Str8_Empty;
				list->selectionActive = false;
				list->selectionIndex = numItems;
				list->selectionChanged = true;
			}
			if (!foundSelectedItem && list->selectionActive)
			{
				PrintLine_D("Couldn't find \"%.*s\" out of %llu items", StrPrint(list->selectedIdStr), numItems);
				FreeStr8(list->arena, &list->selectedIdStr);
				list->selectedIdStr = Str8_Empty;
				list->selectionActive = false;
				list->selectionIndex = numItems;
				list->selectionChanged = true;
			}
		}
	}
	
	ScratchEnd(scratch);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _UI_CLAY_LISTVIEW_H
