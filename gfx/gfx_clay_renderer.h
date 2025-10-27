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
#include "lib/lib_tracy.h"

#if BUILD_WITH_SOKOL_GFX && BUILD_WITH_CLAY

typedef plex ClayUIRendererFont ClayUIRendererFont;
plex ClayUIRendererFont
{
	u16 id;
	PigFont* pntr;
	u8 styleFlags;
};

typedef plex ClayUIRenderer ClayUIRenderer;
plex ClayUIRenderer
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
	NotNull(config);
	NotNull(userData);
	ClayUIRenderer* renderer = (ClayUIRenderer*)userData;
	RichStr richText = config->userData.richText ? DecodeStrToRichStr(scratch, text) : ToRichStr(text);
	Assert(config->fontId < renderer->fonts.length);
	ClayUIRendererFont* font = VarArrayGetHard(ClayUIRendererFont, &renderer->fonts, (uxx)config->fontId);
	r32 fontSize = (r32)config->fontSize;
	FontAtlas* fontAtlas = GetFontAtlas(font->pntr, fontSize, font->styleFlags);
	NotNull(fontAtlas);
	//NOTE: Clay has no way of knowing the lineHeight, so if we don't tell it otherwise it will place text a little too close to each other vertically
	//NOTE: Clay asks us for sizes of words, not entire strings. And it expects the composition of each word + spaces together should match the size of the whole string measured together
	//      Our text measuring code does not treat advanceX as part of the logical space, so if the final character in a word is a glyph that has advanceX > glyphWidth then it will return
	//      glyphWidth when measured as a word, but the advanceX will be used to shift the following space/words in the full string resulting in a larger full-text size
	//      The cater to Clay we need to include the advanceX of the last character in the measurement
	const bool includeAdvanceX = true;
	TextMeasure measure = MeasureRichTextEx(font->pntr, fontSize, font->styleFlags, includeAdvanceX, config->userData.wrapWidth, richText);
	
	if (measure.Height < fontAtlas->lineHeight) { measure.Height = fontAtlas->lineHeight; }
	//NOTE: Our measurement can return non-whole numbers, but Clay just truncates these to int, so the CeilR32s here are important!
	v2 result = NewV2(CeilR32(measure.Width - measure.OffsetX), CeilR32(MaxR32(measure.logicalRec.Height, measure.visualRec.Height)));
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
				TracyCZoneN(Zone_COMMAND_TEXT, "TEXT", true);
				// GfxSystem_DrawRectangle(system, drawRec, ColorWithAlpha(MonokaiGreen, 0.25f));
				
				uxx scratchMark = ArenaGetMark(scratch);
				Str8 text = NewStr8(command->renderData.text.stringContents.length, command->renderData.text.stringContents.chars);
				RichStr richText = command->renderData.text.userData.richText ? DecodeStrToRichStr(scratch, text) : ToRichStr(text);
				u16 fontId = command->renderData.text.fontId;
				r32 fontSize = (r32)command->renderData.text.fontSize;
				Assert(fontId < renderer->fonts.length);
				Color32 drawColor = command->renderData.text.textColor;
				ClayUIRendererFont* font = VarArrayGetHard(ClayUIRendererFont, &renderer->fonts, (uxx)fontId);
				FontAtlas* fontAtlas = GetFontAtlas(font->pntr, fontSize, font->styleFlags);
				NotNull(fontAtlas);
				reci oldClipRec = ZEROED;
				r32 textOffsetX = 0.0f;
				if (command->renderData.text.userData.wrapWidth == 0.0f)
				{
					if (command->renderData.text.userData.contraction == TextContraction_ClipLeft ||
						command->renderData.text.userData.contraction == TextContraction_ClipRight ||
						richText.numPieces > 1) //TODO: We don't support ellipses style contractions with RichStr right now!
					{
						rec textClipRec = NewRec(
							drawRec.X,
							drawRec.Y + fontAtlas->lineHeight/2 + fontAtlas->centerOffset - fontAtlas->maxAscend,
							drawRec.Width,
							fontAtlas->lineHeight
						);
						AlignRec(&textClipRec);
						if (command->renderData.text.userData.contraction == TextContraction_ClipLeft)
						{
							TextMeasure measure = MeasureRichTextEx(font->pntr, fontSize, font->styleFlags, false, 0, richText);
							if (measure.Width > drawRec.Width)
							{
								textOffsetX -= (measure.Width - drawRec.Width);
							}
						}
						// GfxSystem_DrawRectangle(system, textClipRec, ColorWithAlpha(MonokaiPurple, 0.2f));
						oldClipRec = GfxSystem_AddClipRec(system, ToReciFromf(textClipRec));
					}
					else if (command->renderData.text.userData.contraction == TextContraction_EllipseLeft)
					{
						text = ShortenTextStartToFitWidth(scratch, font->pntr, fontSize, font->styleFlags, text, CeilR32(drawRec.Width), StrLit(UNICODE_ELLIPSIS_STR));
						richText = ToRichStr(text);
					}
					else if (command->renderData.text.userData.contraction == TextContraction_EllipseMiddle)
					{
						text = ShortenTextToFitWidth(scratch, font->pntr, fontSize, font->styleFlags, text, CeilR32(drawRec.Width), StrLit(UNICODE_ELLIPSIS_STR), text.length/2);
						richText = ToRichStr(text);
					}
					else if (command->renderData.text.userData.contraction == TextContraction_EllipseRight)
					{
						text = ShortenTextEndToFitWidth(scratch, font->pntr, fontSize, font->styleFlags, text, CeilR32(drawRec.Width), StrLit(UNICODE_ELLIPSIS_STR));
						richText = ToRichStr(text);
					}
					else if (command->renderData.text.userData.contraction == TextContraction_EllipseFilePath)
					{
						text = ShortenFilePathToFitWidth(scratch, font->pntr, fontSize, font->styleFlags, text, CeilR32(drawRec.Width), StrLit(UNICODE_ELLIPSIS_STR));
						richText = ToRichStr(text);
					}
				}
				v2 textPos = NewV2(drawRec.X + textOffsetX, drawRec.Y + fontAtlas->lineHeight/2 + fontAtlas->centerOffset);
				AlignV2(&textPos);
				
				FontFlowState state = ZEROED;
				state.contextPntr = (void*)system;
				state.font = font->pntr;
				state.text = richText;
				state.startFontSize = fontSize;
				state.startFontStyle = font->styleFlags;
				state.startColor = drawColor;
				state.alignPixelSize = system->state.alignPixelSize;
				state.wrapWidth = command->renderData.text.userData.wrapWidth;
				state.position = textPos;
				state.backgroundColor = (command->renderData.text.userData.backgroundColor.a != 0) ? command->renderData.text.userData.backgroundColor : system->state.textBackgroundColor;
				FontFlowCallbacks callbacks = ZEROED;
				callbacks.drawChar = GfxSystem_FontFlowDrawCharCallback;
				callbacks.drawHighlight = GfxSystem_FontFlowDrawHighlightCallback;
				FontFlow* flowTarget = (command->renderData.text.userData.flowTarget != nullptr) 
					? command->renderData.text.userData.flowTarget
					: &system->prevFontFlow;
				
				Result drawResult = DoFontFlow(&state, &callbacks, flowTarget);
				Assert(drawResult == Result_Success || drawResult == Result_InvalidUtf8);
				
				// GfxSystem_DrawRectangle(system, NewRecV(textPos, V2_One), MonokaiRed);
				
				if (command->renderData.text.userData.contraction == TextContraction_ClipLeft ||
					command->renderData.text.userData.contraction == TextContraction_ClipRight)
				{
					GfxSystem_SetClipRec(system, oldClipRec);
				}
				ArenaResetToMark(scratch, scratchMark);
				TracyCZoneEnd(Zone_COMMAND_TEXT);
			} break;
			
			// +================================+
			// | CLAY_RENDER_COMMAND_TYPE_IMAGE |
			// +================================+
			case CLAY_RENDER_COMMAND_TYPE_IMAGE:
			{
				TracyCZoneN(Zone_COMMAND_IMAGE, "IMAGE", true);
				Texture* texturePntr = (Texture*)command->renderData.image.imageData;
				Color32 drawColor = command->renderData.image.backgroundColor;
				if (drawColor.valueU32 == 0) { drawColor = White; } //default value means "untinted"
				GfxSystem_DrawTexturedRectangle(system, drawRec, drawColor, texturePntr);
				TracyCZoneEnd(Zone_COMMAND_IMAGE);
			} break;
			
			// +========================================+
			// | CLAY_RENDER_COMMAND_TYPE_SCISSOR_START |
			// +========================================+
			case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
			{
				TracyCZoneN(Zone_COMMAND_SCISSOR_START, "SCISSOR_START", true);
				//NOTE: Negative or infinite values for clip rectangle cause OpenGL errors so we guard against that
				//TODO: We should change Clay so it never produces SCISSOR_START commands with 0 or infinite size
				if (drawRec.Width > 0 && drawRec.Height > 0 && !IsInfiniteOrNanR32(drawRec.Width) && !IsInfiniteOrNanR32(drawRec.Height))
				{
					// r32 oldDepth = system->state.depth;
					// GfxSystem_SetDepth(system, 0.0f);
					// GfxSystem_DrawRectangleOutlineEx(system, drawRec, 1, MonokaiRed, false);
					GfxSystem_SetClipRec(system, ToReciFromf(drawRec));
					// GfxSystem_SetDepth(system, oldDepth);
				}
				TracyCZoneEnd(Zone_COMMAND_SCISSOR_START);
			} break;
			
			// +======================================+
			// | CLAY_RENDER_COMMAND_TYPE_SCISSOR_END |
			// +======================================+
			case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
			{
				TracyCZoneN(Zone_COMMAND_SCISSOR_END, "SCISSOR_END", true);
				GfxSystem_DisableClipRec(system);
				TracyCZoneEnd(Zone_COMMAND_SCISSOR_END);
			} break;
			
			// +====================================+
			// | CLAY_RENDER_COMMAND_TYPE_RECTANGLE |
			// +====================================+
			case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
			{
				TracyCZoneN(Zone_COMMAND_RECTANGLE, "RECTANGLE", true);
				AlignRec(&drawRec);
				if (!IsInfiniteOrNanR32(drawRec.Width) && !IsInfiniteOrNanR32(drawRec.Height))
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
				}
				TracyCZoneEnd(Zone_COMMAND_RECTANGLE);
			} break;
			
			// +==================================+
			// | CLAY_RENDER_COMMAND_TYPE_BORDER  |
			// +==================================+
			case CLAY_RENDER_COMMAND_TYPE_BORDER:
			{
				TracyCZoneN(Zone_COMMAND_BORDER, "BORDER", true);
				AlignRec(&drawRec);
				
				if (!IsInfiniteOrNanR32(drawRec.Width) && !IsInfiniteOrNanR32(drawRec.Height))
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
				}
				TracyCZoneEnd(Zone_COMMAND_BORDER);
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
