/*
File:   ui_clay_label.h
Author: Taylor Robbins
Date:   09\11\2025
*/

#ifndef _UI_CLAY_LABEL_H
#define _UI_CLAY_LABEL_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
#include "struct/struct_string.h"
#include "struct/struct_color.h"
#include "gfx/gfx_texture.h"
#include "gfx/gfx_font.h"
#include "gfx/gfx_font_flow.h"
#include "gfx/gfx_clay_renderer.h"
#include "ui/ui_clay.h"
#include "ui/ui_clay_widget_context.h"

#if BUILD_WITH_CLAY

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void DoUiLabel(UiWidgetContext* context, Str8 idStr, uxx idIndex, Str8 label, Color32 color, PigFont* font, r32 fontSize, u8 fontStyle, bool hasCopyButton, Texture* copyBtnIcon);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void DoUiLabel(UiWidgetContext* context, Str8 idStr, uxx idIndex, Str8 label, Color32 color, PigFont* font, r32 fontSize, u8 fontStyle, bool hasCopyButton, Texture* copyBtnIcon)
{
	NotNull(context);
	NotNull(context->uiArena);
	NotNull(context->mouse);
	Assert(!IsEmptyStr(idStr));
	NotNullStr(label);
	if (!IsEmptyStr(label))
	{
		ClayId containerId = ToClayIdEx(idStr, idIndex);
		Str8 buttonIdStr = JoinStringsInArena(context->uiArena, idStr, StrLit("_CopyBtn"), false);
		ClayId buttonId = ToClayIdEx(buttonIdStr, idIndex);
		bool isContainerHovered = (context->mouse->isOverWindow && Clay_PointerOver(containerId));
		bool isBtnHovered = (context->mouse->isOverWindow && Clay_PointerOver(buttonId));
		
		if (hasCopyButton && isBtnHovered && IsMouseBtnPressed(context->mouse, MouseBtn_Left) && context->windowHandle != OsWindowHandleEmpty)
		{
			Result copyResult = OsSetClipboardString(context->windowHandle, label);
			if (copyResult != Result_Success) { PrintLine_E("Failed to copy label: %s", GetResultStr(copyResult)); }
		}
		
		CLAY({ .id = containerId,
			.layout = {
				.layoutDirection = CLAY_LEFT_TO_RIGHT,
				.childGap = UISCALE_U16(context->uiScale, 2),
			},
		})
		{
			NotNull(font);
			u16 fontId = GetClayUIRendererFontId(context->renderer, font, fontStyle);
			label = IsPntrWithin(context->uiArena->mainPntr, context->uiArena->size, label.chars) ? label : AllocStr8(context->uiArena, label);
			CLAY_TEXT(
				label,
				CLAY_TEXT_CONFIG({
					.fontId = fontId,
					.fontSize = (u16)fontSize,
					.textColor = color,
					.wrapMode = CLAY_TEXT_WRAP_WORDS,
					.textAlignment = CLAY_TEXT_ALIGN_LEFT,
				})
			);
			
			if (hasCopyButton)
			{
				r32 lineHeight = GetFontLineHeight(font, fontSize, fontStyle);
				CLAY({ .id = buttonId,
					.layout = {
						.sizing = { .width=CLAY_SIZING_FIXED(lineHeight), .height=CLAY_SIZING_FIXED(lineHeight) },
						.childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
					},
					.cornerRadius = CLAY_CORNER_RADIUS(UISCALE_R32(context->uiScale, 2)),
					.backgroundColor = ColorWithAlpha(White, isBtnHovered ? 0.5f : 0.0f),
					.border = {
						.color = ColorWithAlpha(White, 0.75f),
						.width = CLAY_BORDER_OUTSIDE(UISCALE_BORDER(context->uiScale, isBtnHovered ? 1 : 0)),
					},
				})
				{
					Color32 iconColor = (isContainerHovered ? White : Transparent);
					if (copyBtnIcon != nullptr)
					{
						CLAY({
							.layout = {
								.sizing = {
									.width = CLAY_SIZING_FIXED((r32)copyBtnIcon->Width),
									.height = CLAY_SIZING_FIXED((r32)copyBtnIcon->Height),
								},
							},
							.image = {
								.imageData = copyBtnIcon,
								.sourceDimensions = {
									.Width = (r32)(copyBtnIcon->Width),
									.Height = (r32)(copyBtnIcon->Height),
								},
							},
							.backgroundColor = iconColor,
						}) {}
					}
					else
					{
						CLAY_TEXT(
							StrLit("C"),
							CLAY_TEXT_CONFIG({
								.fontId = fontId,
								.fontSize = (u16)fontSize,
								.textColor = iconColor,
								.wrapMode = CLAY_TEXT_WRAP_NONE,
								.textAlignment = CLAY_TEXT_ALIGN_LEFT,
							})
						);
					}
				}
			}
		}
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_CLAY

#endif //  _UI_CLAY_LABEL_H
