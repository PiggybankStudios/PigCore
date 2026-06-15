/*
File:   ui_scrollbar.h
Author: Taylor Robbins
Date:   06\15\2026
Description:
	** When making a scrollable container in Pig UI it's often good practice to
	** give the user a visual for the viewable area and the current scroll value
	** relative to the entire content. This file contains a scrollbar gutter+handle
	** that render to the right and/or bottom of a container, with the handle moving
	** as the content scrolls up/down or left/right. The handle can be moved by
	** left-clicking and dragging. Left-clicking in the gutter outside the handle
	** jumps to that scroll value. PageUp/PageDown jump the scrollable container
	** by the current container width/height
*/

#ifndef _UI_SCROLLBAR_H
#define _UI_SCROLLBAR_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "std/std_basic_math.h"
#include "struct/struct_vectors.h"
#include "struct/struct_color.h"
#include "misc/misc_standard_colors.h"
#include "ui/ui_system_core.h"
#include "ui/ui_system.h"

#if BUILD_WITH_PIG_UI

#define PIG_UI_DEFAULT_SCROLLBAR_WIDTH        10 //px
#define PIG_UI_DEFAULT_SCROLLBAR_GUTTER_COLOR MonokaiBack
#define PIG_UI_DEFAULT_SCROLLBAR_COLOR        MonokaiLightGray
#define PIG_UI_SCROLLBAR_MIN_SIZE             25 //px

typedef plex UiScrollbarState UiScrollbarState;
plex UiScrollbarState
{
	bool isHovered;
	bool isDragging;
	bool isDraggingSmooth;
	v2 draggingOffset;
	r32 gutterWidth;
	Color32 barColor;
	Color32 gutterColor;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void ContainerWithVerticalScrollbar_(UiId scrollViewId, UiScrollbarState* state, UiElemConfig splitterContainerConfig, UiElemConfig scrollViewConfig);
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define ContainerWithVerticalScrollbar(scrollViewId, statePntr, splitterContainerConfig, scrollViewConfig) \
	DeferBlockWithStart(ContainerWithVerticalScrollbar_((scrollViewId), (statePntr), (splitterContainerConfig), (scrollViewConfig)), CloseUiElementMulti(2))

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void ContainerWithVerticalScrollbar_(UiId scrollViewId, UiScrollbarState* state, UiElemConfig splitterContainerConfig, UiElemConfig scrollViewConfig)
{
	NotNull(state);
	r32 gutterWidth = state->gutterWidth;
	Color32 gutterColor = state->gutterColor;
	Color32 barColor = state->barColor;
	if (gutterWidth == 0.0f) { gutterWidth = PIG_UI_DEFAULT_SCROLLBAR_WIDTH; }
	if (gutterColor.valueU32 == PigUiDefaultColor_Value) { gutterColor = PIG_UI_DEFAULT_SCROLLBAR_GUTTER_COLOR; }
	if (barColor.valueU32 == PigUiDefaultColor_Value) { barColor = PIG_UI_DEFAULT_SCROLLBAR_COLOR; }
	const r32 gutterLeftRightPadding = 1;
	
	UiId splitterId = UiIdSuffixLit(scrollViewId, "_ScrollbarSplitter");
	UiId barId = UiIdSuffixLit(scrollViewId, "_Scrollbar");
	UiId gutterId = UiIdSuffixLit(scrollViewId, "_ScrollbarGutter");
	UiElement* oldScrollViewElem = GetUiElementByIdInPrevFrame(scrollViewId, true);
	UiElement* oldGutterElem = GetUiElementByIdInPrevFrame(gutterId, true);
	UiElement* oldBarElem = GetUiElementByIdInPrevFrame(barId, true);
	state->isHovered = IsUiElementHovered(barId);
	bool isGutterHovered = IsUiElementHovered(gutterId);
	v2 gutterSize = (oldGutterElem != nullptr) ? ShrinkV2(oldGutterElem->layoutRec.Size, UiCtx->scale) : V2_One;
	v2 viewSize = (oldScrollViewElem != nullptr) ? ShrinkV2(oldScrollViewElem->layoutRec.Size, UiCtx->scale) : V2_One;
	v2 contentSize = (oldScrollViewElem != nullptr) ? ShrinkV2(oldScrollViewElem->contentSize, UiCtx->scale) : V2_Zero;
	r32 viewablePercentage = (contentSize.Height > viewSize.Height) ? (viewSize.Height / contentSize.Height) : 1.0f;
	v2 barSize = MakeV2(
		gutterSize.Width - (gutterLeftRightPadding*2),
		MinR32(MaxR32(PIG_UI_SCROLLBAR_MIN_SIZE, RoundR32(gutterSize.Height * viewablePercentage)), gutterSize.Height)
	);
	
	bool setNewScrollValue = false;
	r32 newScrollPercentage = 0.0f;
	
	if (UiCtx->mouse != nullptr)
	{
		if (state->isHovered && IsMouseBtnPressed(UiCtx->mouse, nullptr, MouseBtn_Left)) //TODO: Pass MouseStateHandling*
		{
			state->isDragging = true;
			state->isDraggingSmooth = false;
			state->draggingOffset = V2_Zero;
			if (oldBarElem != nullptr) { state->draggingOffset = SubV2(UiCtx->mouse->position, oldBarElem->layoutRec.TopLeft); }
		}
		else if (isGutterHovered && IsMouseBtnPressed(UiCtx->mouse, nullptr, MouseBtn_Left)) //TODO: Pass MouseStateHandling*
		{
			state->isDragging = true;
			state->isDraggingSmooth = true;
			state->draggingOffset = V2_Zero;
			if (oldBarElem != nullptr) { state->draggingOffset = ShrinkV2(oldBarElem->layoutRec.Size, 2); }
		}
		
		if (state->isDragging)
		{
			if (!IsMouseBtnDown(UiCtx->mouse, nullptr, MouseBtn_Left)) { state->isDragging = false; }
			else
			{
				v2 draggedBarPosition = SubV2(UiCtx->mouse->position, state->draggingOffset);
				if (oldGutterElem != nullptr) { draggedBarPosition = SubV2(draggedBarPosition, oldGutterElem->layoutRec.TopLeft); }
				draggedBarPosition = ShrinkV2(draggedBarPosition, UiCtx->scale);
				r32 maxBarY = gutterSize.Height - barSize.Height;
				newScrollPercentage = (maxBarY > 0.0f) ? draggedBarPosition.Y / maxBarY : 0.0f;
				newScrollPercentage = ClampR32(newScrollPercentage, 0.0f, 1.0f);
				setNewScrollValue = true;
			}
			
			//TODO: Handle the mouseBtn!
		}
	}
	
	
	splitterContainerConfig.id = splitterId;
	splitterContainerConfig.direction = UiLayoutDir_RightToLeft;
	OpenUiElement(splitterContainerConfig);
	
	UIELEM({ .id = gutterId,
		.direction = UiLayoutDir_TopDown,
		.alignment = UI_ALIGN_TOP_CENTER(),
		.sizing = { .width=UI_FIXED(gutterWidth), .height=UI_PERCENT(1.0f) },
		.padding = { .inner = { .Left=gutterLeftRightPadding, .Right=gutterLeftRightPadding } },
		.color = gutterColor,
	})
	{
		r32 scrollPercentage = (oldScrollViewElem != nullptr && oldScrollViewElem->scrollMax.Y > 0) ? oldScrollViewElem->scroll.Y / oldScrollViewElem->scrollMax.Y : 0.0f;
		UIELEM_LEAF({ .id = barId,
			.sizing = UI_FIXED2(barSize.Width, barSize.Height),
			.padding = { .outer = { .Top=RoundR32(scrollPercentage * (gutterSize.Height - barSize.Height)) } },
			.color = barColor,
			.cornerRadius = FillV4r(barSize.Width/2),
		});
	}
	
	scrollViewConfig.id = scrollViewId;
	scrollViewConfig.sizing.width = UI_EXPAND();
	UiElement* scrollViewElem = OpenUiElement(scrollViewConfig);
	
	if (setNewScrollValue && scrollViewElem != nullptr)
	{
		scrollViewElem->scrollGoto.Y = newScrollPercentage * scrollViewElem->scrollMax.Y;
		if (!state->isDraggingSmooth) { scrollViewElem->scroll.Y = scrollViewElem->scrollGoto.Y; }
		if (state->isDraggingSmooth && AreSimilarR32(scrollViewElem->scroll.Y, scrollViewElem->scrollGoto.Y, DEFAULT_R32_TOLERANCE)) { state->isDraggingSmooth = false; }
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_PIG_UI

#endif //  _UI_SCROLLBAR_H
