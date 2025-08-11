/*
File:   ui_clay_hoverable.h
Author: Taylor Robbins
Date:   08\10\2025
*/

#ifndef _UI_CLAY_HOVERABLE_H
#define _UI_CLAY_HOVERABLE_H

#include "base/base_defines_check.h"
#include "base/base_macros.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "ui/ui_clay_widget_context.h"
#include "struct/struct_directions.h"
#include "struct/struct_string.h"

typedef enum UiHoverableSection UiHoverableSection;
enum UiHoverableSection
{
	UiHoverableSection_None = 0,
	UiHoverableSection_HoverArea,
	UiHoverableSection_Tooltip,
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	UiHoverableSection DoUiHoverable(UiHoverableSection section, UiWidgetContext* context, Str8 idStr, Dir2 preferredTooltipSide);
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define DoUiHoverableInterleaved(sectionVarName, context, idStr, preferredTooltipSide) for (                           \
	UiHoverableSection sectionVarName = UiHoverableSection_None;                                                       \
	(sectionVarName = DoUiHoverable(sectionVarName, context, idStr, preferredTooltipSide)) != UiHoverableSection_None; \
	/* nothing here */                                                                                                 \
)
#define DoUiHoverableSection(sectionVarName, enumValue) if (sectionVarName == UiHoverableSection_##enumValue)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP UiHoverableSection DoUiHoverable(UiHoverableSection section, UiWidgetContext* context, Str8 idStr, Dir2 preferredTooltipSide)
{
	NotNull(context);
	NotNull(context->uiArena);
	NotNull(context->mouse);
	UNUSED(preferredTooltipSide); //TODO: Take this into account, as well as the window boundaries relative to the tooltip size
	if (section == UiHoverableSection_None)
	{
		ClayId id = ToClayId(idStr);
		// bool isHovered = (context->mouse->isOverWindow && Clay_PointerOver(id));
		
		Clay__OpenElement();
		Clay__ConfigureOpenElement((Clay_ElementDeclaration){
			.id = id,
			.layout = {
				.sizing = { .width = CLAY_SIZING_FIT(0), .height = CLAY_SIZING_FIT(0), },
			},
			// .backgroundColor = isHovered ? ColorWithAlpha(MonokaiWhite, 0.5f) : Transparent,
		});
		return UiHoverableSection_HoverArea;
	}
	else if (section == UiHoverableSection_HoverArea)
	{
		Clay__CloseElement();
		
		ClayId id = ToClayId(idStr);
		ClayId tooltipId = ToClayIdPrint(context->uiArena, "%.*s_Tooltip", StrPrint(idStr));
		bool isHovered = (context->mouse->isOverWindow && Clay_PointerOver(id));
		
		if (isHovered)
		{
			Clay__OpenElement();
			Clay__ConfigureOpenElement((Clay_ElementDeclaration){
				.id = tooltipId,
				.layout = {
					.sizing = { .width = CLAY_SIZING_FIT(0), .height = CLAY_SIZING_FIT(0), },
					// .padding = CLAY_PADDING_ALL(UISCALE_U16(context->uiScale, 8)),
				},
				// .border = {
				// 	.width = CLAY_BORDER_OUTSIDE(UISCALE_BORDER(context->uiScale, 1)),
				// 	.color = MonokaiWhite,
				// },
				.floating = {
					.attachTo = CLAY_ATTACH_TO_PARENT,
					.attachPoints = { .parent = CLAY_ATTACH_POINT_RIGHT_BOTTOM, .element = CLAY_ATTACH_POINT_RIGHT_TOP },
					.pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
				},
				// .backgroundColor = MonokaiDarkGray,
			});
			return UiHoverableSection_Tooltip;
		}
		else { return UiHoverableSection_None; }
	}
	else if (section == UiHoverableSection_Tooltip)
	{
		Clay__CloseElement();
		return UiHoverableSection_None;
	}
	return UiHoverableSection_None;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _UI_CLAY_HOVERABLE_H
