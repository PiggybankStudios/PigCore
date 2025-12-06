/*
File:   ui_clay_tooltip.h
Author: Taylor Robbins
Date:   12\04\2025
*/

#ifndef _UI_CLAY_TOOLTIP_H
#define _UI_CLAY_TOOLTIP_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
#include "struct/struct_string.h"
#include "gfx/gfx_font_flow.h"
#include "gfx/gfx_clay_renderer.h"
#include "ui/ui_clay.h"
#include "ui/ui_clay_tooltip_registry.h"
#include "ui/ui_clay_widget_context.h"

#if BUILD_WITH_CLAY && BUILD_WITH_SOKOL_GFX

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void DoUiTooltips(UiWidgetContext* context, TooltipRegistry* registry);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void DoUiTooltips(UiWidgetContext* context, TooltipRegistry* registry)
{
	NotNull(context);
	NotNull(context->uiArena);
	NotNull(context->mouse);
	NotNull(registry);
	NotNull(registry->arena);
	
	// Check for mouse movement to close open tooltip and reset lastMouseMoveTime
	// TODO: We should probably add some tolerance of small movements that don't close the tooltip until you drift a certain distance away from where the mouse was when the tooltip opened
	if (!AreEqualV2(context->mouse->position, context->mouse->prevPosition)) //TODO: Check if the mouse is not over the window as well!
	{
		if (registry->openTooltipId != TOOLTIP_ID_INVALID) { registry->openTooltipId = TOOLTIP_ID_INVALID; }
		registry->lastMouseMoveTime = context->programTime;
	}
	
	// Find which tooltip is currently hovered
	u64 newMouseHoverId = TOOLTIP_ID_INVALID;
	VarArrayLoop(&registry->tooltips, tIndex)
	{
		VarArrayLoopGet(RegisteredTooltip, tooltip, &registry->tooltips, tIndex);
		if (tooltip->active)
		{
			if (!IsEmptyStr(tooltip->targetClayIdStr))
			{
				//TODO: Add support for IsMouseOverInContainer?
				//TODO: Also check if mouse is over window!
				if (Clay_PointerOver(ToClayId(tooltip->targetClayIdStr)))
				{
					newMouseHoverId = tooltip->id;
				}
			}
			else
			{
				if (IsInsideRec(tooltip->targetRec, context->mouse->position))
				{
					newMouseHoverId = tooltip->id;
				}
			}
		}
	}
	if (newMouseHoverId != registry->hoverTooltipId)
	{
		registry->hoverTooltipId = newMouseHoverId;
		registry->hoverTooltipChangeTime = context->programTime;
		if (registry->openTooltipId != TOOLTIP_ID_INVALID) { registry->openTooltipId = TOOLTIP_ID_INVALID; }
	}
	
	if (registry->openTooltipId != TOOLTIP_ID_INVALID)
	{
		VarArrayLoop(&registry->tooltips, tIndex)
		{
			VarArrayLoopGet(RegisteredTooltip, tooltip, &registry->tooltips, tIndex);
			if (tooltip->active && registry->openTooltipId == tooltip->id)
			{
				bool attachToSpecific = !IsEmptyStr(tooltip->targetClayIdStr);
				ClayId targetId = attachToSpecific ? ToClayId(tooltip->targetClayIdStr) : ClayId_Invalid;
				Str8 tooltipIdStr = PrintInArenaStr(context->uiArena, "%.*s%sTooltip%llu", StrPrint(tooltip->targetClayIdStr), attachToSpecific ? "_" : "", tooltip->id);
				ClayId tooltipId = ToClayId(tooltipIdStr);
				Assert(tooltip->font == nullptr || context->renderer != nullptr);
				u16 fontId = (tooltip->font != nullptr) ? GetClayUIRendererFontId(context->renderer, tooltip->font, tooltip->fontStyle) : 0;
				
				CLAY({ .id = tooltipId,
					.layout = {
						.sizing = { .width = CLAY_SIZING_FIT(0), .height = CLAY_SIZING_FIT(0) },
						.padding = {
							.left = UISCALE_U16(context->uiScale, 8),
							.right = UISCALE_U16(context->uiScale, 8),
							.top = UISCALE_U16(context->uiScale, 4),
							.bottom = UISCALE_U16(context->uiScale, 4),
						},
						.childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
					},
					.floating = {
						.attachTo = attachToSpecific ? CLAY_ATTACH_TO_ELEMENT_WITH_ID : CLAY_ATTACH_TO_PARENT,
						.parentId = targetId.id,
					},
					.backgroundColor = MonokaiDarkGray,
					.border = {
						.color = MonokaiWhite,
						.width = CLAY_BORDER_OUTSIDE(UISCALE_BORDER(context->uiScale, 1)),
					},
				})
				{
					if (fontId != 0)
					{
						CLAY_TEXT(
							AllocStr8(context->uiArena, tooltip->displayStr),
							CLAY_TEXT_CONFIG({
								.fontId = fontId,
								.fontSize = (u16)tooltip->fontSize,
								.textColor = MonokaiWhite,
								.wrapMode = CLAY_TEXT_WRAP_NONE,
								.textAlignment = CLAY_TEXT_ALIGN_LEFT,
						}));
					}
					//TODO: Should we render something if we couldn't find a font to render text with? Maybe we need like a "defaultFont" in the UiWidgetContext
				}
				
				break;
			}
		}
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_CLAY && BUILD_WITH_SOKOL_GFX

#endif //  _UI_CLAY_TOOLTIP_H
