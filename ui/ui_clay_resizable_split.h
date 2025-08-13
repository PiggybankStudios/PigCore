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

typedef enum UiResizableSplitSection UiResizableSplitSection;
enum UiResizableSplitSection
{
	UiResizableSplitSection_None = 0,
	UiResizableSplitSection_Left,
	UiResizableSplitSection_Right,
	UiResizableSplitSection_Top = UiResizableSplitSection_Left,
	UiResizableSplitSection_Bottom = UiResizableSplitSection_Right,
};

PEXP UiResizableSplitSection DoUiResizableSplit(UiResizableSplitSection section, UiWidgetContext* context, Str8 idStr, bool horizontal, r32* splitPercent)
{
	if (section == UiResizableSplitSection_None)
	{
		Str8 firstSectionIdStr = PrintInArenaStr(context->uiArena, "%.*s_%s", StrPrint(idStr), horizontal ? "Left" : "Top");
		
		Clay__OpenElement();
		Clay__ConfigureOpenElement({ .id = ToClayId(firstSectionIdStr),
			.layout = {
				.sizing = {
					.width  = horizontal ? CLAY_SIZING_PERCENT(*splitPercent) : CLAY_SIZING_GROW(0),
					.height = horizontal ? CLAY_SIZING_GROW(0) : CLAY_SIZING_PERCENT(*splitPercent),
				},
				.layoutDirection = horizontal ? CLAY_TOP_TO_BOTTOM : CLAY_LEFT_TO_RIGHT,
			},
		});
	}
	else if (section == UiResizableSplitSection_Left)
	{
		Clay__CloseElement();
		
		Str8 secondSectionIdStr = PrintInArenaStr(context->uiArena, "%.*s_%s", StrPrint(idStr), horizontal ? "Right" : "Bottom");
		ClayId secondSectionId = ToClayId(secondSectionIdStr);
		rec secondSectionRec = GetClayElementDrawRec(secondSectionId);
		
		if (!AreEqualV2(secondSectionRec.Size, V2_Zero))
		{
			CLAY({ .id = ToClayId(idStr),
				.layout = {
					.sizing = {
						.width  = CLAY_SIZING_FIXED(horizontal ? 4 : secondSectionRec.Width),
						.height = CLAY_SIZING_FIXED(horizontal ? secondSectionRec.Height : 4),
					},
				},
				.floating = {
					.attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
					.elementId = secondSectionId.id,
					.attachPoints = { .parent = CLAY_ATTACH_POINT_LEFT_TOP, .element = CLAY_ATTACH_POINT_CENTER_TOP },
				},
			}) {}
		}
		
		Clay__OpenElement();
		Clay__ConfigureOpenElement({ .id = secondSectionId,
			.layout = {
				.sizing = {
					.width = horizontal ? CLAY_SIZING_PERCENT(1.0f - (*splitPercent)) : CLAY_SIZING_GROW(0),
					.height = horizontal ? CLAY_SIZING_GROW(0) : CLAY_SIZING_PERCENT(1.0f - (*splitPercent)),
				},
				.layoutDirection = horizontal ? CLAY_TOP_TO_BOTTOM : CLAY_LEFT_TO_RIGHT,
			},
		});
	}
	else if (section == UiResizableSplitSection_Right)
	{
		Clay__CloseElement();
	}
}

#endif //  _UI_CLAY_RESIZABLE_SPLIT_H
