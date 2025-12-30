/*
File:   ui_clay_resizable_split.h
Author: Taylor Robbins
Date:   08\13\2025
Description:
	** A UiResizableSplit manages splitting a container either horizontally or vertically
	** and portioning the two pieces based on a percentage value of the total width\height
	** This value can be changed by the user by clicking and dragging the split bar
	** left\right or up\down to choose a new percentage split
*/

#ifndef _UI_CLAY_RESIZABLE_SPLIT_H
#define _UI_CLAY_RESIZABLE_SPLIT_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "struct/struct_string.h"
#include "ui/ui_clay.h"
#include "ui/ui_clay_widget_context.h"

#if BUILD_WITH_CLAY

typedef enum UiResizableSplitSection UiResizableSplitSection;
enum UiResizableSplitSection
{
	UiResizableSplitSection_None = 0,
	UiResizableSplitSection_Left,
	UiResizableSplitSection_Right,
	UiResizableSplitSection_Top = UiResizableSplitSection_Left,
	UiResizableSplitSection_Bottom = UiResizableSplitSection_Right,
};

typedef plex UiResizableSplit UiResizableSplit;
plex UiResizableSplit
{
	Arena* arena;
	Str8 idStr;
	bool horizontal;
	u16 dividerPadding; //auto-scaled by context->uiScale in Do function
	r32 splitPercent;
	r32 minSplitPercent;
	r32 maxSplitPercent;
	r32 minFirstSplitSize;
	r32 minSecondSplitSize;
	bool resizing;
	r32 resizingMouseOffset;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeUiResizableSplit(UiResizableSplit* split);
	PIG_CORE_INLINE void InitUiResizableSplit(Arena* arena, Str8 idStr, bool horizontal, u16 dividerPadding, r32 defaultSplitPercent, UiResizableSplit* split);
	UiResizableSplitSection DoUiResizableSplit(UiResizableSplitSection section, UiWidgetContext* context, UiResizableSplit* split);
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define DoUiResizableSplitInterleaved(sectionVarName, context, splitPntr) for (                                    \
	UiResizableSplitSection sectionVarName = UiResizableSplitSection_None;                                         \
	(sectionVarName = DoUiResizableSplit(sectionVarName, (context), (splitPntr))) != UiResizableSplitSection_None; \
	/* nothing here */                                                                                             \
)
#define DoUiResizableSplitSection(sectionVarName, enumValue) if (sectionVarName == UiResizableSplitSection_##enumValue)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void FreeUiResizableSplit(UiResizableSplit* split)
{
	NotNull(split);
	if (split->arena != nullptr)
	{
		FreeStr8(split->arena, &split->idStr);
	}
	ClearPointer(split);
}

PEXPI void InitUiResizableSplit(Arena* arena, Str8 idStr, bool horizontal, u16 dividerPadding, r32 defaultSplitPercent, UiResizableSplit* split)
{
	NotNull(arena);
	NotNull(split);
	NotEmptyStr(idStr);
	ClearPointer(split);
	split->arena = arena;
	split->idStr = AllocStr8(arena, idStr);
	NotNull(split->idStr.chars);
	split->horizontal = horizontal;
	split->dividerPadding = dividerPadding;
	split->splitPercent = defaultSplitPercent;
	split->minSplitPercent = 0.0f;
	split->maxSplitPercent = 1.0f;
	split->minFirstSplitSize = 0.0f;
	split->minSecondSplitSize = 0.0f;
}

PEXP UiResizableSplitSection DoUiResizableSplit(UiResizableSplitSection section, UiWidgetContext* context, UiResizableSplit* split)
{
	NotNull(context);
	NotNull(context->mouse);
	NotNull(split);
	NotNull(split->arena);
	Str8 firstSectionIdStr = PrintInArenaStr(context->uiArena, "%.*s_%s", StrPrint(split->idStr), split->horizontal ? "Left" : "Top");
	Str8 secondSectionIdStr = PrintInArenaStr(context->uiArena, "%.*s_%s", StrPrint(split->idStr), split->horizontal ? "Right" : "Bottom");
	Str8 dividerIdStr = PrintInArenaStr(context->uiArena, "%.*s_Divider", StrPrint(split->idStr));
	ClayId outerId = ToClayId(split->idStr);
	ClayId firstSectionId = ToClayId(firstSectionIdStr);
	ClayId secondSectionId = ToClayId(secondSectionIdStr);
	ClayId dividerId = ToClayId(dividerIdStr);
	
	if (section == UiResizableSplitSection_None)
	{
		Clay__OpenElement();
		Clay__ConfigureOpenElement((Clay_ElementDeclaration){ .id = outerId,
			.layout = {
				.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
				.layoutDirection = split->horizontal ? CLAY_LEFT_TO_RIGHT : CLAY_TOP_TO_BOTTOM,
				.childGap = UISCALE_U16(context->uiScale, split->dividerPadding),
			},
		});
		
		Clay__OpenElement();
		Clay__ConfigureOpenElement((Clay_ElementDeclaration){ .id = firstSectionId,
			.layout = {
				.sizing = {
					.width  = split->horizontal ? CLAY_SIZING_PERCENT(split->splitPercent) : CLAY_SIZING_GROW(0),
					.height = split->horizontal ? CLAY_SIZING_GROW(0) : CLAY_SIZING_PERCENT(split->splitPercent),
				},
				.layoutDirection = split->horizontal ? CLAY_TOP_TO_BOTTOM : CLAY_LEFT_TO_RIGHT,
			},
		});
		return UiResizableSplitSection_Left;
	}
	else if (section == UiResizableSplitSection_Left)
	{
		Clay__CloseElement();
		
		Clay__OpenElement();
		Clay__ConfigureOpenElement((Clay_ElementDeclaration){ .id = secondSectionId,
			.layout = {
				.sizing = {
					.width = split->horizontal ? CLAY_SIZING_PERCENT(1.0f - split->splitPercent) : CLAY_SIZING_GROW(0),
					.height = split->horizontal ? CLAY_SIZING_GROW(0) : CLAY_SIZING_PERCENT(1.0f - split->splitPercent),
				},
				.layoutDirection = split->horizontal ? CLAY_TOP_TO_BOTTOM : CLAY_LEFT_TO_RIGHT,
			},
		});
		return UiResizableSplitSection_Right;
	}
	else if (section == UiResizableSplitSection_Right)
	{
		Clay__CloseElement();
		
		rec outerRec = GetClayElementDrawRec(outerId);
		rec secondSectionRec = GetClayElementDrawRec(secondSectionId);
		if (!AreEqualV2(secondSectionRec.Size, V2_Zero))
		{
			r32 dividerPadding = UISCALE_R32(context->uiScale, (r32)split->dividerPadding);
			r32 handleWidth = MaxR32(UISCALE_R32(context->uiScale, 4), dividerPadding);
			CLAY({ .id = dividerId,
				.layout = {
					.sizing = {
						.width  = CLAY_SIZING_FIXED(split->horizontal ? handleWidth : secondSectionRec.Width),
						.height = CLAY_SIZING_FIXED(split->horizontal ? secondSectionRec.Height : handleWidth),
					},
				},
				.floating = {
					.attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
					.parentId = secondSectionId.id,
					.offset = MakeV2(split->horizontal ? -(dividerPadding/2.0f) : 0, split->horizontal ? 0 : -(dividerPadding/2.0f)),
					.attachPoints = {
						.parent = CLAY_ATTACH_POINT_LEFT_TOP,
						.element = split->horizontal ? CLAY_ATTACH_POINT_CENTER_TOP : CLAY_ATTACH_POINT_LEFT_CENTER,
					},
				},
			}) {}
			
			if (context->mouse->isOverWindow && Clay_PointerOver(dividerId))
			{
				context->cursorShape = (split->horizontal ? MouseCursorShape_ResizeHori : MouseCursorShape_ResizeVert);
				if (IsMouseBtnPressed(context->mouse, MouseBtn_Left))
				{
					split->resizing = true;
					split->resizingMouseOffset = split->horizontal ? (context->mouse->position.X - secondSectionRec.X) : (context->mouse->position.Y - secondSectionRec.Y);
				}
			}
			
			if (split->resizing)
			{
				if (IsMouseBtnDown(context->mouse, MouseBtn_Left))
				{
					context->cursorShape = (split->horizontal ? MouseCursorShape_ResizeHori : MouseCursorShape_ResizeVert);
					split->splitPercent = ClampR32(
						split->horizontal ? (context->mouse->position.X - outerRec.X) / outerRec.Width : (context->mouse->position.Y - outerRec.Y) / outerRec.Height,
						split->minSplitPercent, split->maxSplitPercent
					);
				}
				else { split->resizing = false; }
			}
		}
		else { split->resizing = false; }
		
		if (outerRec.Width > 0 && outerRec.Height > 0)
		{
			r32 minPercent = MaxR32(split->minSplitPercent, split->minFirstSplitSize / (split->horizontal ? outerRec.Width : outerRec.Height));
			r32 maxPercent = MinR32(split->maxSplitPercent, 1.0f - (split->minSecondSplitSize / (split->horizontal ? outerRec.Width : outerRec.Height)));
			if (minPercent >= maxPercent) //handle degenerate scenarios (like outerRec.Width < split->minFirstSplitSize)
			{
				minPercent = (minPercent + maxPercent)/2.0f;
				if (minPercent < 0.0f || minPercent > 1.0f) { minPercent = 0.5f; }
				maxPercent = minPercent;
			}
			split->splitPercent = ClampR32(split->splitPercent, minPercent, maxPercent);
		}
		
		Clay__CloseElement();
		
		return UiResizableSplitSection_None;
	}
	
	return UiResizableSplitSection_None;
}

#endif

#endif //BUILD_WITH_CLAY

#endif //  _UI_CLAY_RESIZABLE_SPLIT_H
