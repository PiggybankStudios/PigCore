/*
File:   ui_clay_checkbox.h
Author: Taylor Robbins
Date:   07\31\2025
Description:
	** A textbox is a simple widget that has a clickable box that toggles a boolean
	** value and often there's a label besides the box to describe what the value is
*/

#ifndef _UI_CLAY_CHECKBOX_H
#define _UI_CLAY_CHECKBOX_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
#include "struct/struct_string.h"
#include "gfx/gfx_font_flow.h"
#include "ui/ui_clay.h"

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void DoUiCheckbox(Str8 idStr, bool* isChecked, ClayUIRenderer* renderer, Arena* uiArena, MouseState* mouse, r32 uiScale, r32 boxSize, Texture* checkboxTexture, Str8 label, Dir2 labelSide, PigFont* font, r32 fontSize, u8 fontStyle);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void DoUiCheckbox(Str8 idStr, bool* isChecked,
	ClayUIRenderer* renderer, Arena* uiArena, MouseState* mouse,
	r32 uiScale, r32 boxSize, Texture* checkboxTexture,
	Str8 label, Dir2 labelSide, PigFont* font, r32 fontSize, u8 fontStyle)
{
	NotNullStr(idStr);
	NotNull(isChecked);
	NotNull(uiArena);
	NotNullStr(label);
	Assert(IsEmptyStr(label) || font != nullptr);
	Assert(font == nullptr || renderer != nullptr);
	u16 fontId = (font != nullptr) ? GetClayUIRendererFontId(renderer, font, fontStyle) : 0;
	bool isVerticallyAligned = (labelSide == Dir2_Up || labelSide == Dir2_Down);
	
	Str8 containerIdStr = AllocStr8(uiArena, idStr);
	Str8 boxIdStr = PrintInArenaStr(uiArena, "%.*s_Box", StrPrint(idStr));
	ClayId containerId = ToClayId(containerIdStr);
	ClayId boxId = ToClayId(boxIdStr);
	bool isHovered = (mouse->isOverWindow && Clay_PointerOver(containerId));
	Color32 greenColor = NewColor(44, 91, 54, 255);
	Color32 borderColor = ColorLerpSimple((*isChecked) ? greenColor : MonokaiDarkGray, MonokaiWhite, isHovered ? 0.7f : 0.2f);
	
	if (isHovered && IsMouseBtnPressed(mouse, MouseBtn_Left))
	{
		*isChecked = !(*isChecked);
	}
	
	CLAY({ .id = containerId,
		.layout = {
			.sizing = { .width = CLAY_SIZING_FIT(0), .height = CLAY_SIZING_FIT(0) },
			.layoutDirection = (isVerticallyAligned ? CLAY_TOP_TO_BOTTOM : CLAY_LEFT_TO_RIGHT),
			.childGap = UISCALE_U16(uiScale, isVerticallyAligned ? 2 : 6),
		},
	})
	{
		#define LABEL_CODE() do { if (!IsEmptyStr(label))                                                   \
		{                                                                                                   \
			CLAY_TEXT(                                                                                      \
				AllocStr8(uiArena, label),                                                                  \
				CLAY_TEXT_CONFIG({                                                                          \
					.fontId = fontId,                                                                       \
					.fontSize = (u16)fontSize,                                                              \
					.textColor = MonokaiWhite,                                                              \
					.wrapMode = CLAY_TEXT_WRAP_NONE,                                                        \
					.textAlignment = (isVerticallyAligned ? CLAY_TEXT_ALIGN_CENTER : CLAY_TEXT_ALIGN_LEFT), \
			}));                                                                                            \
		} } while(0)
		
		bool doesLabelComeBefore = (labelSide == Dir2_Left || labelSide == Dir2_Up);
		if (doesLabelComeBefore) { LABEL_CODE(); }
		
		u16 borderSize = UISCALE_BORDER(uiScale, 1);
		CLAY({ .id = boxId,
			.layout = {
				.sizing = { .width = CLAY_SIZING_FIXED(UISCALE_R32(uiScale, boxSize*2)), .height = CLAY_SIZING_FIXED(UISCALE_R32(uiScale, boxSize)) },
				.childAlignment = { .x = (*isChecked) ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_CENTER },
				.padding = CLAY_PADDING_ALL(borderSize*2),
			},
			.backgroundColor = (*isChecked) ? greenColor : Transparent,
			.border = { .width = CLAY_BORDER_OUTSIDE(borderSize), .color = borderColor },
			.userData = { .outsideBorder = true },
			.cornerRadius = CLAY_CORNER_RADIUS(UISCALE_R32(uiScale, boxSize/2)),
		})
		{
			if (checkboxTexture != nullptr && (*isChecked))
			{
				
			}
			
			r32 dotSize = UISCALE_R32(uiScale, boxSize - (borderSize*4));
			CLAY({
				.layout = {
					.sizing = { .width = CLAY_SIZING_FIXED(dotSize), .height = CLAY_SIZING_FIXED(dotSize) },
				},
				.backgroundColor = (*isChecked) ? MonokaiWhite : MonokaiLightGray,
				.cornerRadius = CLAY_CORNER_RADIUS(dotSize/2),
			}) {}
		}
		
		if (!doesLabelComeBefore) { LABEL_CODE(); }
		#undef LABEL_CODE
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _UI_CLAY_CHECKBOX_H
