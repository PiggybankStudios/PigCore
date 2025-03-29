/*
File:   gfx_clay_renderer.h
Author: Taylor Robbins
Date:   02\16\2025
Description:
	** Contains the a renderer for Clay UI that is based on gfx_system.h (see ui_clay.h)
*/

#ifndef _GFX_CLAY_RENDERER_H
#define _GFX_CLAY_RENDERER_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "struct/struct_vectors.h"
#include "struct/struct_rectangles.h"
#include "struct/struct_var_array.h"
#include "gfx/gfx_font.h"
#include "gfx/gfx_font_flow.h"
#include "gfx/gfx_system.h"
#include "ui/ui_clay.h"

#if BUILD_WITH_SOKOL_GFX && BUILD_WITH_CLAY

typedef struct ClayUIRendererFont ClayUIRendererFont;
struct ClayUIRendererFont
{
	u16 id;
	PigFont* pntr;
	u8 styleFlags;
};

typedef struct ClayUIRenderer ClayUIRenderer;
struct ClayUIRenderer
{
	Arena* arena;
	ClayUI clay;
	VarArray fonts; //ClayUIRendererFont
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Clay_ImageElementConfig ToClayImage(Texture* texture);
	CLAY_MEASURE_TEXT_DEF(ClayUIRendererMeasureText);
	void InitClayUIRenderer(Arena* arena, v2 windowSize, ClayUIRenderer* rendererOut);
	PIG_CORE_INLINE u16 AddClayUIRendererFont(ClayUIRenderer* renderer, PigFont* fontPntr, u8 styleFlags);
	PIG_CORE_INLINE u16 GetClayUIRendererFontId(ClayUIRenderer* renderer, PigFont* fontPntr, u8 styleFlags);
	void RenderClayCommandArray(ClayUIRenderer* renderer, GfxSystem* system, Clay_RenderCommandArray* commands);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI Clay_ImageElementConfig ToClayImage(Texture* texture)
{
	Clay_ImageElementConfig result = ZEROED;
	result.imageData = texture;
	result.sourceDimensions = ToV2Fromi(texture->size);
	return result;
}

// +==============================+
// |  ClayUIRendererMeasureText   |
// +==============================+
// v2 ClayUIRendererMeasureText(Str8 text, Clay_TextElementConfig *config, CLAY_MEASURE_USERDATA_TYPE userData)
PEXP CLAY_MEASURE_TEXT_DEF(ClayUIRendererMeasureText)
{
	ScratchBegin(scratch);
	NotNull(userData);
	ClayUIRenderer* renderer = (ClayUIRenderer*)userData;
	Str8 textStr = NewStr8((uxx)text.length, text.chars);
	RichStr richTextStr = DecodeStrToRichStr(scratch, textStr);
	Assert(config->fontId < renderer->fonts.length);
	ClayUIRendererFont* font = VarArrayGetHard(ClayUIRendererFont, &renderer->fonts, (uxx)config->fontId);
	r32 fontSize = (r32)config->fontSize;
	FontAtlas* fontAtlas = GetFontAtlas(font->pntr, fontSize, font->styleFlags);
	NotNull(fontAtlas);
	//NOTE: Clay has no way of knowing the lineHeight, so if we don't tell it otherwise it will place text a little too close to each other vertically
	TextMeasure measure = MeasureRichTextEx(font->pntr, fontSize, font->styleFlags, richTextStr);
	
	if (measure.Height < fontAtlas->lineHeight) { measure.Height = fontAtlas->lineHeight; }
	//NOTE: Our measurement can return non-whole numbers, but Clay just truncates these to int, so the CeilR32s here are important!
	v2 result = NewV2(CeilR32(measure.Width - measure.OffsetX), CeilR32(measure.Height));
	ScratchEnd(scratch);
	return result;
}

PEXP void InitClayUIRenderer(Arena* arena, v2 windowSize, ClayUIRenderer* rendererOut)
{
	NotNull(arena);
	NotNull(rendererOut);
	
	ClearPointer(rendererOut);
	rendererOut->arena = arena;
	InitVarArray(ClayUIRendererFont, &rendererOut->fonts, arena);
	InitClayUI(arena, windowSize, ClayUIRendererMeasureText, rendererOut, &rendererOut->clay);
}

PEXPI u16 AddClayUIRendererFont(ClayUIRenderer* renderer, PigFont* fontPntr, u8 styleFlags)
{
	NotNull(renderer);
	NotNull(renderer->clay.context);
	NotNull(fontPntr);
	Assert(renderer->fonts.length <= UINT16_MAX);
	SetClayContext(&renderer->clay);
	u16 newId = (u16)renderer->fonts.length;
	ClayUIRendererFont* newFont = VarArrayAdd(ClayUIRendererFont, &renderer->fonts);
	NotNull(newFont);
	ClearPointer(newFont);
	newFont->id = newId;
	newFont->pntr = fontPntr;
	newFont->styleFlags = styleFlags;
	return newId;
}

PEXPI u16 GetClayUIRendererFontId(ClayUIRenderer* renderer, PigFont* fontPntr, u8 styleFlags)
{
	NotNull(renderer);
	NotNull(renderer->clay.context);
	NotNull(fontPntr);
	SetClayContext(&renderer->clay);
	VarArrayLoop(&renderer->fonts, fIndex)
	{
		VarArrayLoopGet(ClayUIRendererFont, font, &renderer->fonts, fIndex);
		if (font->pntr == fontPntr && font->styleFlags == styleFlags) { return font->id; }
	}
	return UINT16_MAX;
}

PEXPI void RenderClayCommandArray(ClayUIRenderer* renderer, GfxSystem* system, Clay_RenderCommandArray* commands)
{
	NotNull(renderer);
	NotNull(renderer->clay.context);
	NotNull(system);
	NotNull(commands);
	Assert(commands->length >= 0);
	SetClayContext(&renderer->clay);
	ScratchBegin(scratch);
	
	for (uxx cIndex = 0; cIndex < (uxx)commands->length; cIndex++)
	{
		Clay_RenderCommand* command = &commands->items[cIndex];
		rec drawRec = command->boundingBox;
		switch (command->commandType)
		{
			// +===============================+
			// | CLAY_RENDER_COMMAND_TYPE_TEXT |
			// +===============================+
			case CLAY_RENDER_COMMAND_TYPE_TEXT:
			{
				uxx scratchMark = ArenaGetMark(scratch);
				Str8 text = NewStr8(command->renderData.text.stringContents.length, command->renderData.text.stringContents.chars);
				RichStr richText = DecodeStrToRichStr(scratch, text);
				u16 fontId = command->renderData.text.fontId;
				r32 fontSize = (r32)command->renderData.text.fontSize;
				Assert(fontId < renderer->fonts.length);
				Color32 drawColor = command->renderData.text.textColor;
				ClayUIRendererFont* font = VarArrayGetHard(ClayUIRendererFont, &renderer->fonts, (uxx)fontId);
				FontAtlas* fontAtlas = GetFontAtlas(font->pntr, fontSize, font->styleFlags);
				NotNull(fontAtlas);
				reci oldClipRec = ZEROED;
				v2 textOffset = V2_Zero;
				if (command->renderData.text.userData.contraction == TextContraction_ClipLeft ||
					command->renderData.text.userData.contraction == TextContraction_ClipRight ||
					richText.numPieces > 1) //TODO: We don't support ellipses style contractions with RichStr right now!
				{
					rec textClipRec = NewRec(
						FloorR32(drawRec.X),
						FloorR32(drawRec.Y + drawRec.Height/2 + fontAtlas->centerOffset - fontAtlas->maxAscend),
						CeilR32(drawRec.Width),
						CeilR32(fontAtlas->maxAscend + fontAtlas->maxDescend)
					);
					if (command->renderData.text.userData.contraction == TextContraction_ClipLeft)
					{
						TextMeasure measure = MeasureRichTextEx(font->pntr, fontSize, font->styleFlags, richText);
						if (measure.Width > drawRec.Width)
						{
							textOffset.X -= (measure.Width - drawRec.Width);
						}
					}
					// GfxSystem_DrawRectangle(system, textClipRec, ColorWithAlpha(White, 0.2f));
					oldClipRec = GfxSystem_AddClipRec(system, ToReciFromf(textClipRec));
				}
				else if (command->renderData.text.userData.contraction == TextContraction_EllipseLeft)
				{
					text = ShortenTextStartToFitWidth(scratch, font->pntr, fontSize, font->styleFlags, text, drawRec.Width, StrLit(UNICODE_ELLIPSIS_STR));
					richText = ToRichStr(text);
				}
				else if (command->renderData.text.userData.contraction == TextContraction_EllipseMiddle)
				{
					text = ShortenTextToFitWidth(scratch, font->pntr, fontSize, font->styleFlags, text, drawRec.Width, StrLit(UNICODE_ELLIPSIS_STR), text.length/2);
					richText = ToRichStr(text);
				}
				else if (command->renderData.text.userData.contraction == TextContraction_EllipseRight)
				{
					text = ShortenTextEndToFitWidth(scratch, font->pntr, fontSize, font->styleFlags, text, drawRec.Width, StrLit(UNICODE_ELLIPSIS_STR));
					richText = ToRichStr(text);
				}
				else if (command->renderData.text.userData.contraction == TextContraction_EllipseFilePath)
				{
					text = ShortenFilePathToFitWidth(scratch, font->pntr, fontSize, font->styleFlags, text, drawRec.Width, StrLit(UNICODE_ELLIPSIS_STR));
					richText = ToRichStr(text);
				}
				v2 textPos = NewV2(drawRec.X + textOffset.X, drawRec.Y + textOffset.Y + drawRec.Height/2 + fontAtlas->centerOffset);
				AlignV2(&textPos);
				Result drawResult = GfxSystem_DrawRichTextWithFont(system, font->pntr, fontSize, font->styleFlags, richText, textPos, drawColor);
				Assert(drawResult == Result_Success || drawResult == Result_InvalidUtf8);
				if (command->renderData.text.userData.contraction == TextContraction_ClipLeft ||
					command->renderData.text.userData.contraction == TextContraction_ClipRight)
				{
					GfxSystem_SetClipRec(system, oldClipRec);
				}
				ArenaResetToMark(scratch, scratchMark);
			} break;
			
			// +================================+
			// | CLAY_RENDER_COMMAND_TYPE_IMAGE |
			// +================================+
			case CLAY_RENDER_COMMAND_TYPE_IMAGE:
			{
				Texture* texturePntr = (Texture*)command->renderData.image.imageData;
				Color32 drawColor = command->renderData.image.backgroundColor;
				if (drawColor.valueU32 == 0) { drawColor = White; } //default value means "untinted"
				GfxSystem_DrawTexturedRectangle(system, drawRec, drawColor, texturePntr);
			} break;
			
			// +========================================+
			// | CLAY_RENDER_COMMAND_TYPE_SCISSOR_START |
			// +========================================+
			case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
			{
				// r32 oldDepth = system->state.depth;
				// GfxSystem_SetDepth(system, 0.0f);
				// GfxSystem_DrawRectangleOutlineEx(system, drawRec, 1, MonokaiRed, false);
				GfxSystem_SetClipRec(system, ToReciFromf(drawRec));
				// GfxSystem_SetDepth(system, oldDepth);
			} break;
			
			// +======================================+
			// | CLAY_RENDER_COMMAND_TYPE_SCISSOR_END |
			// +======================================+
			case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
			{
				GfxSystem_DisableClipRec(system);
			} break;
			
			// +====================================+
			// | CLAY_RENDER_COMMAND_TYPE_RECTANGLE |
			// +====================================+
			case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
			{
				Color32 drawColor = command->renderData.rectangle.backgroundColor;
				GfxSystem_DrawRoundedRectangleEx(system,
					drawRec,
					command->renderData.rectangle.cornerRadius.topLeft,
					command->renderData.rectangle.cornerRadius.topRight,
					command->renderData.rectangle.cornerRadius.bottomRight,
					command->renderData.rectangle.cornerRadius.bottomLeft,
					drawColor
				);
			} break;
			
			// +==================================+
			// | CLAY_RENDER_COMMAND_TYPE_BORDER  |
			// +==================================+
			case CLAY_RENDER_COMMAND_TYPE_BORDER:
			{
				Color32 drawColor = command->renderData.border.color;
				if (command->renderData.border.cornerRadius.topLeft != 0 ||
					command->renderData.border.cornerRadius.topRight != 0 ||
					command->renderData.border.cornerRadius.bottomLeft != 0 ||
					command->renderData.border.cornerRadius.bottomRight != 0)
				{
					//TODO: Should we just assert that all the widths are the same? We probably don't need to support variable side widths AND corner radius at the same time, right?
					r32 borderThickness = MaxR32(MaxR32(command->renderData.border.width.left, command->renderData.border.width.right), MaxR32(command->renderData.border.width.top, command->renderData.border.width.bottom));
					GfxSystem_DrawRoundedRectangleOutlineEx(system,
						drawRec,
						borderThickness,
						command->renderData.border.cornerRadius.topLeft,
						command->renderData.border.cornerRadius.topRight,
						command->renderData.border.cornerRadius.bottomRight,
						command->renderData.border.cornerRadius.bottomLeft,
						drawColor,
						command->userData.outsideBorder
					);
				}
				else
				{
					GfxSystem_DrawRectangleOutlineSidesEx(system,
						drawRec,
						command->renderData.border.width.left,
						command->renderData.border.width.right,
						command->renderData.border.width.top,
						command->renderData.border.width.bottom,
						drawColor,
						command->userData.outsideBorder
					);
				}
			} break;
			
			// +==================================+
			// | CLAY_RENDER_COMMAND_TYPE_CUSTOM  |
			// +==================================+
			case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
			{
				//TODO: Implement me!
			} break;
			
			default: AssertMsg(false, "Unhandled Clay RenderCommand Type!"); break;
		}
	}
	
	ScratchEnd(scratch);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX && BUILD_WITH_CLAY

#endif //  _GFX_CLAY_RENDERER_H
