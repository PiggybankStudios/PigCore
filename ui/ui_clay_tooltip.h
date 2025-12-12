/*
File:   ui_clay_tooltip.h
Author: Taylor Robbins
Date:   12\04\2025
	** This is the interaction and rendering portion of tooltips
	** The state structures and registration API is defined in ui_clay_tooltip_registry.h
	** See the description at the top of that file for more information about tooltips
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
#include "misc/misc_standard_colors.h"
#include "gfx/gfx_font_flow.h"
#include "gfx/gfx_clay_renderer.h"
#include "ui/ui_clay.h"
#include "ui/ui_clay_tooltip_registry.h"
#include "ui/ui_clay_widget_context.h"

#if BUILD_WITH_CLAY && BUILD_WITH_SOKOL_GFX

#define TOOLTIP_BACKGROUND_COLOR UiTextWhite
#define TOOLTIP_TEXT_COLOR       UiBackgroundDarkGray

#define TOOLTIP_HOVER_DELAY  750 //ms
#define TOOLTIP_MAX_WIDTH    400 //px (at uiScale=1.0f)
#define TOOLTIP_PADDING_X      6 //px (at uiScale=1.0f)
#define TOOLTIP_PADDING_Y      8 //px (at uiScale=1.0f)
// #define TOOLTIP_MAX_MOVE_DIST      5 //px
#define TOOLTIP_TARGET_UP_OFFSET    5 //px
#define TOOLTIP_TARGET_DOWN_OFFSET  25 //px TODO: Somehow we should ask the OS how tall the cursor is and offset but that much plus a little
#define TOOLTIP_FADEIN_TIME    166 //ms

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void DoUiTooltips(UiWidgetContext* context, TooltipRegistry* registry, v2 screenSize);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void DoUiTooltips(UiWidgetContext* context, TooltipRegistry* registry, v2 screenSize)
{
	NotNull(context);
	NotNull(context->uiArena);
	NotNull(context->mouse);
	NotNull(registry);
	NotNull(registry->arena);
	
	// Check for mouse movement to close open tooltip and reset lastMouseMoveTime
	// TODO: We should probably add some tolerance of small movements that don't close the tooltip until you drift a certain distance away from where the mouse was when the tooltip opened
	if (context->mouse->isOverWindow)
	{
		if (!AreEqualV2(context->mouse->position, context->mouse->prevPosition) ||
			!context->mouse->wasOverWindow ||
			IsMouseBtnPressed(context->mouse, MouseBtn_Left) || IsMouseBtnPressed(context->mouse, MouseBtn_Right) || IsMouseBtnPressed(context->mouse, MouseBtn_Middle))
		{
			if (registry->openTooltipId != TOOLTIP_ID_INVALID) { registry->openTooltipId = TOOLTIP_ID_INVALID; }
			registry->lastMouseMoveTime = context->programTime;
		}
	}
	else if (registry->openTooltipId != TOOLTIP_ID_INVALID) { registry->openTooltipId = TOOLTIP_ID_INVALID; }
	
	// Find which tooltip is currently hovered
	u64 newMouseHoverId = TOOLTIP_ID_INVALID;
	if (context->mouse->isOverWindow)
	{
		VarArrayLoop(&registry->tooltips, tIndex)
		{
			VarArrayLoopGet(RegisteredTooltip, tooltip, &registry->tooltips, tIndex);
			if (tooltip->active)
			{
				if (!IsEmptyStr(tooltip->targetClayIdStr))
				{
					if (Clay_PointerOver(ToClayId(tooltip->targetClayIdStr)) &&
						(tooltip->targetContainerClayId.id == 0 || Clay_PointerOver(tooltip->targetContainerClayId)))
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
	}
	if (newMouseHoverId != registry->hoverTooltipId)
	{
		registry->hoverTooltipId = newMouseHoverId;
		registry->hoverTooltipChangeTime = context->programTime;
		if (registry->openTooltipId != TOOLTIP_ID_INVALID) { registry->openTooltipId = TOOLTIP_ID_INVALID; }
	}
	
	// Open the hovered tooltip if the mouse hasn't move for long enough
	if (registry->hoverTooltipId != TOOLTIP_ID_INVALID &&
		TimeSinceBy(context->programTime, registry->lastMouseMoveTime) >= TOOLTIP_HOVER_DELAY &&
		registry->hoverTooltipId != registry->openTooltipId)
	{
		registry->openTooltipTargetPos = context->mouse->position;
		registry->openTooltipId = registry->hoverTooltipId;
		registry->openTooltipTime = context->programTime;
		RegisteredTooltip* openTooltip = TryFindRegisteredTooltip(registry, registry->openTooltipId);
		NotNull(openTooltip);
	}
	
	// Render the open tooltip
	if (registry->openTooltipId != TOOLTIP_ID_INVALID)
	{
		RegisteredTooltip* openTooltip = TryFindRegisteredTooltip(registry, registry->openTooltipId);
		if (openTooltip != nullptr && openTooltip->active)
		{
			NotNull(openTooltip->font);
			bool attachToElement = !IsEmptyStr(openTooltip->targetClayIdStr);
			ClayId targetId = attachToElement ? ToClayId(openTooltip->targetClayIdStr) : ClayId_Invalid;
			Str8 tooltipIdStr = PrintInArenaStr(context->uiArena, "%.*s%sTooltip%llu", StrPrint(openTooltip->targetClayIdStr), attachToElement ? "_" : "", openTooltip->id);
			ClayId tooltipId = ToClayId(tooltipIdStr);
			Assert(openTooltip->font == nullptr || context->renderer != nullptr);
			v2 attachOffset = AddV2(registry->openTooltipTargetPos, MakeV2(0, TOOLTIP_TARGET_DOWN_OFFSET));
			u64 timeSinceOpen = TimeSinceBy(context->programTime, registry->openTooltipTime);
			r32 tooltipAlpha = (timeSinceOpen < TOOLTIP_FADEIN_TIME) ? (r32)timeSinceOpen / (r32)TOOLTIP_FADEIN_TIME : 1.0f;
			
			u16 fontId = GetClayUIRendererFontId(context->renderer, openTooltip->font, openTooltip->fontStyle);
			Assert(fontId != CLAY_FONT_ID_INVALID);
			r32 wrapWidth = UISCALE_R32(context->uiScale, MinR32(screenSize.Width, TOOLTIP_MAX_WIDTH) - (2 * TOOLTIP_PADDING_X));
			TextMeasure displayStrMeasure = MeasureTextEx(openTooltip->font, openTooltip->fontSize, openTooltip->fontStyle, false, wrapWidth, openTooltip->displayStr);
			v2 tooltipSize = AddV2(displayStrMeasure.logicalRec.Size, MakeV2(2 * UISCALE_R32(context->uiScale, TOOLTIP_PADDING_X), 2 * UISCALE_R32(context->uiScale, TOOLTIP_PADDING_Y)));
			Clay_FloatingAttachPointType attachPoint = CLAY_ATTACH_POINT_CENTER_TOP;
			if (attachOffset.Y > screenSize.Height - tooltipSize.Height)
			{
				attachPoint = CLAY_ATTACH_POINT_CENTER_BOTTOM;
				attachOffset.Y -= (TOOLTIP_TARGET_DOWN_OFFSET + TOOLTIP_TARGET_UP_OFFSET);
			}
			if (attachOffset.X < tooltipSize.Width/2.0f)
			{
				attachOffset.X = tooltipSize.Width/2.0f;
			}
			else if (attachOffset.X > screenSize.Width - tooltipSize.Width/2.0f)
			{
				attachOffset.X = screenSize.Width - tooltipSize.Width/2.0f;
			}
			
			CLAY({ .id = tooltipId,
				.layout = {
					.sizing = { .width = CLAY_SIZING_FIXED(tooltipSize.Width), .height = CLAY_SIZING_FIXED(tooltipSize.Height) },
					.padding = {
						.left = UISCALE_U16(context->uiScale, TOOLTIP_PADDING_X),
						.top = UISCALE_U16(context->uiScale, TOOLTIP_PADDING_Y*0.75f), //TODO: Get rid of this hacky way to make the text look more vertically centered
						// .right = UISCALE_U16(context->uiScale, TOOLTIP_PADDING_X),
						// .bottom = UISCALE_U16(context->uiScale, TOOLTIP_PADDING_Y),
					},
					// .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }, //TODO: Enable this once the clay renderer properly vertically aligns multi-line text
				},
				.floating = {
					.attachTo = CLAY_ATTACH_TO_PARENT,
					.parentId = targetId.id,
					.pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
					.attachPoints = { .parent = CLAY_ATTACH_POINT_LEFT_TOP, .element = attachPoint },
					.zIndex = 10,
					.offset = attachOffset,
				},
				.backgroundColor = ColorWithAlpha(TOOLTIP_BACKGROUND_COLOR, tooltipAlpha),
				.border = {
					.color = ColorWithAlpha(MonokaiWhite, tooltipAlpha),
					.width = CLAY_BORDER_OUTSIDE(UISCALE_BORDER(context->uiScale, 1)),
				},
			})
			{
				CLAY_TEXT(
					AllocStr8(context->uiArena, openTooltip->displayStr),
					CLAY_TEXT_CONFIG({
						.fontId = fontId,
						.fontSize = (u16)openTooltip->fontSize,
						.textColor = ColorWithAlpha(TOOLTIP_TEXT_COLOR, tooltipAlpha),
						.wrapMode = CLAY_TEXT_WRAP_NONE,
						.textAlignment = CLAY_TEXT_ALIGN_LEFT,
						.userData = {
							.wrapWidth = wrapWidth,
						},
				}));
			}
		}
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_CLAY && BUILD_WITH_SOKOL_GFX

#endif //  _UI_CLAY_TOOLTIP_H
