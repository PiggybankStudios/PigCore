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
	UiHoverableSection DoUiHoverable(UiHoverableSection section, UiWidgetContext* context, Str8 idStr, Dir2 preferredTooltipSide, v2 screenSize, bool openOverride);
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define DoUiHoverableInterleaved(sectionVarName, context, idStr, preferredTooltipSide, screenSize, openOverride) for (                           \
	UiHoverableSection sectionVarName = UiHoverableSection_None;                                                                                 \
	(sectionVarName = DoUiHoverable(sectionVarName, context, idStr, preferredTooltipSide, screenSize, openOverride)) != UiHoverableSection_None; \
	/* nothing here */                                                                                                                           \
)
#define DoUiHoverableSection(sectionVarName, enumValue) if (sectionVarName == UiHoverableSection_##enumValue)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP UiHoverableSection DoUiHoverable(UiHoverableSection section, UiWidgetContext* context, Str8 idStr, Dir2 preferredTooltipSide, v2 screenSize, bool openOverride)
{
	NotNull(context);
	NotNull(context->uiArena);
	NotNull(context->mouse);
	UNUSED(preferredTooltipSide); //TODO: Take this into account, as well as the window boundaries relative to the tooltip size
	if (section == UiHoverableSection_None)
	{
		ClayId id = ToClayId(idStr);
		Clay__OpenElement();
		Clay__ConfigureOpenElement((Clay_ElementDeclaration){
			.id = id,
			.layout = {
				.sizing = { .width = CLAY_SIZING_FIT(0), .height = CLAY_SIZING_FIT(0), },
			},
		});
		return UiHoverableSection_HoverArea;
	}
	else if (section == UiHoverableSection_HoverArea)
	{
		Clay__CloseElement();
		
		ClayId id = ToClayId(idStr);
		bool isHovered = (context->mouse->isOverWindow && Clay_PointerOver(id));
		if (isHovered || openOverride)
		{
			ClayId tooltipId = ToClayIdPrint(context->uiArena, "%.*s_Tooltip", StrPrint(idStr));
			
			//NOTE: On the first frame this tooltip appears, we attach it to LEFT_TOP of root to make sure it's offscreen,
			//      next frame we'll know the size and know how to place the tooltip effectively.
			//      This prevents a noticeable "flash" as the tooltip opens and one frame delay after mouse hovers is not noticeable
			Clay_FloatingAttachToElement attachTo = CLAY_ATTACH_TO_ROOT;
			Clay_FloatingAttachPointType parentAttach = CLAY_ATTACH_POINT_LEFT_TOP;
			Clay_FloatingAttachPointType tooltipAttach = CLAY_ATTACH_POINT_RIGHT_BOTTOM;
			
			v2 tooltipSize = GetClayElementDrawRec(tooltipId).Size;
			rec hoverRec = GetClayElementDrawRec(id);
			if (!AreEqualV2(screenSize, V2_Zero) && !AreEqualV2(tooltipSize, V2_Zero) && !AreEqualV2(hoverRec.Size, V2_Zero))
			{
				Dir2 attachSide = preferredTooltipSide;
				u8 sideAlignment = 1; //0 for left/up, 1 for center, 2 for right/down
				
				if (attachSide == Dir2_Down && hoverRec.Y + hoverRec.Height + tooltipSize.Height > screenSize.Height && hoverRec.Y - screenSize.Height >= 0)
				{
					attachSide = Dir2_Up;
				}
				else if (attachSide == Dir2_Up && hoverRec.Y - tooltipSize.Height < 0 && hoverRec.Y + hoverRec.Height + tooltipSize.Height <= screenSize.Height)
				{
					attachSide = Dir2_Down;
				}
				else if (attachSide == Dir2_Right && hoverRec.X + hoverRec.Width + tooltipSize.Width > screenSize.Width && hoverRec.X - screenSize.Width >= 0)
				{
					attachSide = Dir2_Left;
				}
				else if (attachSide == Dir2_Left && hoverRec.X - tooltipSize.Width < 0 && hoverRec.X + hoverRec.Width + tooltipSize.Width <= screenSize.Width)
				{
					attachSide = Dir2_Right;
				}
				
				if (attachSide == Dir2_Left || attachSide == Dir2_Right)
				{
					if (hoverRec.Y + hoverRec.Height/2 + tooltipSize.Height/2 > screenSize.Height)
					{
						sideAlignment = 0; //align up
					}
					else if (hoverRec.Y + hoverRec.Height/2 - tooltipSize.Height/2 < 0)
					{
						sideAlignment = 2; //align down
					}
				}
				else if (attachSide == Dir2_Down || attachSide == Dir2_Up)
				{
					if (hoverRec.X + hoverRec.Width/2 + tooltipSize.Width/2 > screenSize.Width)
					{
						sideAlignment = 0; //align left
					}
					else if (hoverRec.X + hoverRec.Width/2 - tooltipSize.Width/2 < 0)
					{
						sideAlignment = 2; //align right
					}
				}
				
				attachTo = CLAY_ATTACH_TO_PARENT;
				switch (attachSide)
				{
					case Dir2_Right:
					{
						parentAttach  = (sideAlignment == 0 ? CLAY_ATTACH_POINT_RIGHT_BOTTOM : (sideAlignment == 1 ? CLAY_ATTACH_POINT_RIGHT_CENTER : CLAY_ATTACH_POINT_RIGHT_TOP));
						tooltipAttach = (sideAlignment == 0 ? CLAY_ATTACH_POINT_LEFT_BOTTOM  : (sideAlignment == 1 ? CLAY_ATTACH_POINT_LEFT_CENTER  : CLAY_ATTACH_POINT_LEFT_TOP));
					} break;
					case Dir2_Left:
					{
						parentAttach  = (sideAlignment == 0 ? CLAY_ATTACH_POINT_LEFT_BOTTOM  : (sideAlignment == 1 ? CLAY_ATTACH_POINT_LEFT_CENTER  : CLAY_ATTACH_POINT_LEFT_TOP));
						tooltipAttach = (sideAlignment == 0 ? CLAY_ATTACH_POINT_RIGHT_BOTTOM : (sideAlignment == 1 ? CLAY_ATTACH_POINT_RIGHT_CENTER : CLAY_ATTACH_POINT_RIGHT_TOP));
					} break;
					case Dir2_Up:
					{
						parentAttach  = (sideAlignment == 0 ? CLAY_ATTACH_POINT_RIGHT_TOP    : (sideAlignment == 1 ? CLAY_ATTACH_POINT_CENTER_TOP    : CLAY_ATTACH_POINT_LEFT_TOP));
						tooltipAttach = (sideAlignment == 0 ? CLAY_ATTACH_POINT_RIGHT_BOTTOM : (sideAlignment == 1 ? CLAY_ATTACH_POINT_CENTER_BOTTOM : CLAY_ATTACH_POINT_LEFT_BOTTOM));
					} break;
					case Dir2_Down:
					{
						parentAttach  = (sideAlignment == 0 ? CLAY_ATTACH_POINT_RIGHT_BOTTOM : (sideAlignment == 1 ? CLAY_ATTACH_POINT_CENTER_BOTTOM : CLAY_ATTACH_POINT_LEFT_BOTTOM));
						tooltipAttach = (sideAlignment == 0 ? CLAY_ATTACH_POINT_RIGHT_TOP    : (sideAlignment == 1 ? CLAY_ATTACH_POINT_CENTER_TOP    : CLAY_ATTACH_POINT_LEFT_TOP));
					} break;
				}
			}
			
			Clay__OpenElement();
			Clay__ConfigureOpenElement((Clay_ElementDeclaration){
				.id = tooltipId,
				.layout = {
					.sizing = { .width = CLAY_SIZING_FIT(0), .height = CLAY_SIZING_FIT(0), },
				},
				.floating = {
					.attachTo = offscreenMeasure ? CLAY_ATTACH_TO_ROOT : CLAY_ATTACH_TO_PARENT,
					.attachPoints = { .parent = parentAttach, .element = tooltipAttach },
					.pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
				},
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
