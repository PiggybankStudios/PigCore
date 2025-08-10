/*
File:   ui_clay_textbox.h
Author: Taylor Robbins
Date:   07\20\2025
Description:
	** An implementation of an editable text box for Desktop platforms that depends
	** on Clay and gfx_system.h
*/

#ifndef _UI_CLAY_TEXTBOX_H
#define _UI_CLAY_TEXTBOX_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "struct/struct_var_array.h"
#include "mem/mem_arena.h"
#include "struct/struct_string.h"
#include "ui/ui_clay.h"
#include "gfx/gfx_clay_renderer.h"
#include "input/input_keys.h"
#include "input/input_btn_state.h"
#include "input/input_mouse_btns.h"
#include "struct/struct_vectors.h"
#include "struct/struct_ranges.h"
#include "gfx/gfx_font.h"
#include "gfx/gfx_font_flow.h"

#if BUILD_WITH_CLAY

#define TEXTBOX_INNER_PADDING_X 8 //px
#define TEXTBOX_INNER_PADDING_Y 12 //px

typedef plex UiTextbox UiTextbox;
plex UiTextbox
{
	Arena* arena;
	Str8 idStr;
	ClayId id;
	u8 fontStyle;
	r32 fontSize;
	PigFont* font;
	
	bool isFocused;
	
	bool cursorActive;
	uxx cursorStart;
	uxx cursorEnd;
	bool draggingWithMouse;
	
	FontFlow flow;
	Str8 text;
	VarArray textBuffer; //char
	
	VarArray syntaxRanges; //RichStrStyleChangeRange
	
	bool textChanged;
	bool cursorMoved;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void FreeUiTextbox(UiTextbox* tbox);
	void InitUiTextbox(Arena* arena, Str8 idStr, Str8 initialText, UiTextbox* tbox);
	PIG_CORE_INLINE void UiTextboxSelectAll(UiTextbox* tbox);
	PIG_CORE_INLINE void UiTextboxDeleteBytes(UiTextbox* tbox, uxx startIndex, uxx numBytes);
	PIG_CORE_INLINE void UiTextboxDeleteSelected(UiTextbox* tbox);
	PIG_CORE_INLINE void UiTextboxClear(UiTextbox* tbox);
	PIG_CORE_INLINE void UiTextboxSetText(UiTextbox* tbox, Str8 text);
	PIG_CORE_INLINE uxx UiTextboxFindClosestIndexToPos(UiTextbox* tbox, v2 screenPos);
	PIG_CORE_INLINE void UiTextboxClearSyntaxRanges(UiTextbox* tbox);
	PIG_CORE_INLINE void UiTextboxAddSyntaxRange(UiTextbox* tbox, RangeUXX range, RichStrStyleChange style);
	void DoUiTextbox(UiTextbox* tbox, ClayUIRenderer* renderer, Arena* uiArena, const KeyboardState* keyboard, const MouseState* mouse, UiTextbox** focusedTextbox, PigFont* font, u8 fontStyle, r32 fontSize, r32 uiScale);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void FreeUiTextbox(UiTextbox* tbox)
{
	NotNull(tbox);
	if (tbox->arena != nullptr)
	{
		FreeStr8(tbox->arena, &tbox->idStr);
		FreeVarArray(&tbox->textBuffer);
		FreeVarArray(&tbox->syntaxRanges);
	}
	ClearPointer(tbox);
}

PEXP void InitUiTextbox(Arena* arena, Str8 idStr, Str8 initialText, UiTextbox* tbox)
{
	NotNull(arena);
	NotNull(tbox);
	
	ClearPointer(tbox);
	tbox->arena = arena;
	tbox->idStr = AllocStr8(arena, idStr);
	tbox->id = ToClayId(tbox->idStr);
	InitVarArrayWithInitial(char, &tbox->textBuffer, arena, initialText.length);
	InitVarArray(RichStrStyleChangeRange, &tbox->syntaxRanges, arena);
	tbox->text = NewStr8(0, (char*)tbox->textBuffer.items);
	if (!IsEmptyStr(initialText))
	{
		char* newChars = VarArrayAddMulti(char, &tbox->textBuffer, initialText.length);
		NotNull(newChars);
		MyMemCopy(newChars, initialText.chars, initialText.length);
		tbox->text = NewStr8(initialText.length, newChars);
	}
}

PEXPI void UiTextboxSelectAll(UiTextbox* tbox)
{
	NotNull(tbox);
	NotNull(tbox->arena);
	tbox->cursorActive = true;
	tbox->cursorStart = 0;
	tbox->cursorEnd = tbox->text.length;
}

PEXPI void UiTextboxDeleteBytes(UiTextbox* tbox, uxx startIndex, uxx numBytes)
{
	NotNull(tbox);
	NotNull(tbox->arena);
	Assert(startIndex <= tbox->text.length);
	Assert(startIndex+numBytes <= tbox->text.length);
	if (numBytes == 0) { return; }
	
	if (startIndex+numBytes < tbox->text.length)
	{
		MyMemMove(&tbox->text.chars[startIndex], &tbox->text.chars[startIndex+numBytes], tbox->text.length - (startIndex+numBytes));
	}
	tbox->text.length -= numBytes;
	tbox->textBuffer.length -= numBytes;
	tbox->cursorStart = startIndex;
	tbox->cursorEnd = startIndex;
	tbox->textChanged = true;
	tbox->cursorMoved = true;
}
PEXPI void UiTextboxDeleteSelected(UiTextbox* tbox)
{
	NotNull(tbox);
	NotNull(tbox->arena);
	if (tbox->cursorActive && tbox->cursorEnd != tbox->cursorStart)
	{
		uxx cursorMin = MinUXX(tbox->cursorStart, tbox->cursorEnd);
		uxx cursorMax = MaxUXX(tbox->cursorStart, tbox->cursorEnd);
		UiTextboxDeleteBytes(tbox, cursorMin, cursorMax - cursorMin);
	}
}
PEXPI void UiTextboxClear(UiTextbox* tbox)
{
	UiTextboxDeleteBytes(tbox, 0, tbox->text.length);
}

PEXPI void UiTextboxSetText(UiTextbox* tbox, Str8 text)
{
	NotNull(tbox);
	if (StrExactEquals(tbox->text, text)) { return; }
	VarArrayExpand(&tbox->textBuffer, text.length);
	tbox->textBuffer.length = text.length;
	tbox->text.chars = (char*)tbox->textBuffer.items;
	tbox->text.length = text.length;
	if (text.length > 0) { MyMemCopy(tbox->text.chars, text.chars, text.length); }
	tbox->textChanged = true;
	if (tbox->cursorActive && (tbox->cursorEnd != tbox->text.length || tbox->cursorStart != tbox->text.length))
	{
		tbox->cursorEnd = tbox->text.length;
		tbox->cursorStart = tbox->cursorEnd;
		tbox->cursorMoved = true;
	}
}

PEXPI uxx UiTextboxFindClosestIndexToPos(UiTextbox* tbox, v2 screenPos)
{
	NotNull(tbox);
	uxx cursorIndex = tbox->text.length;
	r32 cursorDistance = LengthV2(Sub(screenPos, tbox->flow.endPos));
	for (uxx gIndex = 0; gIndex < tbox->flow.numGlyphs; gIndex++)
	{
		FontFlowGlyph* glyph = &tbox->flow.glyphs[gIndex];
		r32 distanceToGlyphPos = LengthV2(Sub(screenPos, glyph->position));
		if (distanceToGlyphPos < cursorDistance)
		{
			cursorIndex = glyph->byteIndex;
			cursorDistance = distanceToGlyphPos;
		}
	}
	return cursorIndex;
}

PEXPI void UiTextboxClearSyntaxRanges(UiTextbox* tbox)
{
	NotNull(tbox);
	NotNull(tbox->arena);
	VarArrayClear(&tbox->syntaxRanges);
}
PEXPI void UiTextboxAddSyntaxRange(UiTextbox* tbox, RangeUXX range, RichStrStyleChange style)
{
	NotNull(tbox);
	NotNull(tbox->arena);
	RichStrStyleChangeRange* newRange = VarArrayAdd(RichStrStyleChangeRange, &tbox->syntaxRanges);
	NotNull(newRange);
	ClearPointer(newRange);
	newRange->range = range;
	newRange->style = style;
}

PEXP void DoUiTextbox(UiTextbox* tbox,
	ClayUIRenderer* renderer, Arena* uiArena,
	KeyboardState* keyboard, MouseState* mouse, UiTextbox** focusedTextbox,
	PigFont* font, u8 fontStyle, r32 fontSize, r32 uiScale)
{
	NotNull(tbox);
	NotNull(tbox->arena);
	NotNull(renderer);
	NotNull(uiArena);
	NotNull(keyboard);
	NotNull(mouse);
	NotNull(focusedTextbox);
	
	if (tbox->isFocused != ((*focusedTextbox) == tbox))
	{
		tbox->isFocused = ((*focusedTextbox) == tbox);
		if (!tbox->cursorActive)
		{
			tbox->cursorActive = true;
			tbox->cursorStart = tbox->text.length;
			tbox->cursorEnd = tbox->cursorStart;
		}
	}
	if (!tbox->isFocused && tbox->draggingWithMouse) { tbox->draggingWithMouse = false; }
	if (!tbox->isFocused && tbox->cursorActive && tbox->cursorEnd != tbox->cursorStart) { tbox->cursorStart = tbox->cursorEnd; }
	
	// +==============================+
	// | Mouse Click Selects Textbox  |
	// +==============================+
	if (mouse->isOverWindow && IsMouseBtnPressed(mouse, MouseBtn_Left))
	{
		if (Clay_PointerOver(tbox->id))
		{
			if (!tbox->isFocused)
			{
				tbox->isFocused = true;
				(*focusedTextbox) = tbox;
			}
			
			tbox->cursorActive = true;
			tbox->cursorEnd = UiTextboxFindClosestIndexToPos(tbox, mouse->position);
			tbox->cursorStart = tbox->cursorEnd;
			tbox->cursorMoved = true;
			tbox->draggingWithMouse = true;
		}
		else if (tbox->isFocused) { tbox->isFocused = false; (*focusedTextbox) = nullptr; }
	}
	//TODO: Handle scrolling left/right when dragging
	if (tbox->draggingWithMouse)
	{
		if (IsMouseBtnDown(mouse, MouseBtn_Left))
		{
			tbox->cursorEnd = UiTextboxFindClosestIndexToPos(tbox, mouse->position);
		}
		else { tbox->draggingWithMouse = false; }
	}
	
	// +==============================+
	// |   Escape Unfocuses Textbox   |
	// +==============================+
	if (tbox->isFocused && IsKeyboardKeyPressed(keyboard, Key_Escape))
	{
		tbox->isFocused = false;
		(*focusedTextbox) = nullptr;
	}
	
	// +==============================+
	// |      Handle Arrow Keys       |
	// +==============================+
	//TODO: Handle Alt
	//TODO: Handle key repeats
	if (tbox->isFocused)
	{
		if (IsKeyboardKeyPressed(keyboard, Key_Left))
		{
			if (tbox->cursorActive)
			{
				if (tbox->cursorStart != tbox->cursorEnd && !IsKeyboardKeyDown(keyboard, Key_Shift))
				{
					tbox->cursorEnd = MinUXX(tbox->cursorStart, tbox->cursorEnd);
					tbox->cursorStart = tbox->cursorEnd;
					tbox->cursorMoved = true;
				}
				else if (tbox->cursorEnd > 0)
				{
					if (IsKeyboardKeyDown(keyboard, Key_Control))
					{
						tbox->cursorEnd = FindWordBoundaryStr(tbox->text, tbox->cursorEnd, false);
					}
					else
					{
						u8 prevCodepointSize = GetPrevCodepointForUtf8Str(tbox->text, tbox->cursorEnd, nullptr);
						if (prevCodepointSize == 0) { prevCodepointSize = 1; }
						tbox->cursorEnd -= prevCodepointSize;
					}
					if (!IsKeyboardKeyDown(keyboard, Key_Shift)) { tbox->cursorStart = tbox->cursorEnd; }
					tbox->cursorMoved = true;
				}
			}
			else { tbox->cursorEnd = tbox->text.length; tbox->cursorStart = tbox->cursorEnd; tbox->cursorMoved = true; }
		}
		if (IsKeyboardKeyPressed(keyboard, Key_Right))
		{
			if (tbox->cursorActive)
			{
				if (tbox->cursorStart != tbox->cursorEnd && !IsKeyboardKeyDown(keyboard, Key_Shift))
				{
					tbox->cursorEnd = MaxUXX(tbox->cursorStart, tbox->cursorEnd);
					tbox->cursorStart = tbox->cursorEnd;
					tbox->cursorMoved = true;
				}
				else if (tbox->cursorEnd < tbox->text.length)
				{
					if (IsKeyboardKeyDown(keyboard, Key_Control))
					{
						tbox->cursorEnd = FindWordBoundaryStr(tbox->text, tbox->cursorEnd, true);
					}
					else
					{
						u8 codepointSize = GetCodepointForUtf8Str(tbox->text, tbox->cursorEnd, nullptr);
						if (codepointSize == 0) { codepointSize = 1; }
						tbox->cursorEnd += codepointSize;
					}
					if (!IsKeyboardKeyDown(keyboard, Key_Shift)) { tbox->cursorStart = tbox->cursorEnd; }
					tbox->cursorMoved = true;
				}
			}
			else { tbox->cursorEnd = tbox->text.length; tbox->cursorStart = tbox->cursorEnd; tbox->cursorMoved = true; }
		}
	}
	
	// +==============================+
	// |       Handle Home/End        |
	// +==============================+
	if (tbox->isFocused)
	{
		if (IsKeyboardKeyPressed(keyboard, Key_Home))
		{
			tbox->cursorEnd = 0;
			if (!IsKeyboardKeyDown(keyboard, Key_Shift)) { tbox->cursorStart = tbox->cursorEnd; }
			tbox->cursorMoved = true;
		}
		if (IsKeyboardKeyPressed(keyboard, Key_End))
		{
			tbox->cursorEnd = tbox->text.length;
			if (!IsKeyboardKeyDown(keyboard, Key_Shift)) { tbox->cursorStart = tbox->cursorEnd; }
			tbox->cursorMoved = true;
		}
	}
	
	// +==============================+
	// |        Handle Ctrl+A         |
	// +==============================+
	if (tbox->isFocused && IsKeyboardKeyPressed(keyboard, Key_A) && IsKeyboardKeyDown(keyboard, Key_Control))
	{
		if (!tbox->cursorActive || tbox->cursorStart != 0 || tbox->cursorEnd != tbox->text.length)
		{
			tbox->cursorActive = true;
			tbox->cursorStart = 0;
			tbox->cursorEnd = tbox->text.length;
			tbox->cursorMoved = true;
		}
	}
	
	// +==============================+
	// |        Handle Typing         |
	// +==============================+
	if (tbox->isFocused && tbox->cursorActive)
	{
		for (uxx iIndex = 0; iIndex < keyboard->numCharInputs; iIndex++)
		{
			KeyboardCharInput* charInput = &keyboard->charInputs[iIndex];
			if (charInput->modifierKeys == ModifierKey_None || charInput->modifierKeys == ModifierKey_Shift)
			{
				u8 utf8Bytes[UTF8_MAX_CHAR_SIZE];
				u8 codepointSize = GetUtf8BytesForCode(charInput->codepoint, &utf8Bytes[0], false);
				if (codepointSize > 0)
				{
					if (tbox->cursorStart != tbox->cursorEnd)
					{
						UiTextboxDeleteSelected(tbox);
						DebugAssert(tbox->cursorStart == tbox->cursorEnd);
					}
					
					VarArrayAddMulti(char, &tbox->textBuffer, codepointSize);
					tbox->text.chars = (char*)tbox->textBuffer.items;
					
					if (tbox->cursorEnd < tbox->text.length)
					{
						uxx numCharsToMove = tbox->text.length - tbox->cursorEnd;
						MyMemMove(&tbox->text.chars[tbox->cursorEnd + codepointSize], &tbox->text.chars[tbox->cursorEnd], numCharsToMove);
					}
					MyMemCopy(&tbox->text.chars[tbox->cursorEnd], &utf8Bytes[0], codepointSize);
					tbox->text.length += codepointSize;
					tbox->cursorStart += codepointSize;
					tbox->cursorEnd += codepointSize;
					tbox->textChanged = true;
					tbox->cursorMoved = true;
				}
			}
		}
	}
	
	// +==============================+
	// | Handle Backspace and Delete  |
	// +==============================+
	//TODO: Handle Ctrl
	//TODO: Handle key repeats
	if (tbox->isFocused && tbox->cursorActive)
	{
		if (tbox->cursorEnd != tbox->cursorStart &&
			(IsKeyboardKeyPressed(keyboard, Key_Backspace) || IsKeyboardKeyPressed(keyboard, Key_Delete)))
		{
			UiTextboxDeleteSelected(tbox);
		}
		else if (IsKeyboardKeyPressed(keyboard, Key_Backspace) && tbox->cursorEnd > 0)
		{
			u8 prevCodepointSize = GetPrevCodepointForUtf8Str(tbox->text, tbox->cursorEnd, nullptr);
			if (prevCodepointSize == 0) { prevCodepointSize = 1; }
			UiTextboxDeleteBytes(tbox, tbox->cursorEnd-prevCodepointSize, prevCodepointSize);
		}
		else if (IsKeyboardKeyPressed(keyboard, Key_Delete) && tbox->cursorEnd < tbox->text.length)
		{
			u8 nextCodepointSize = GetCodepointForUtf8Str(tbox->text, tbox->cursorEnd, nullptr);
			if (nextCodepointSize == 0) { nextCodepointSize = 1; }
			UiTextboxDeleteBytes(tbox, tbox->cursorEnd, nextCodepointSize);
		}
	}
	
	//TODO: Update horizontal scroll
	//TODO: Follow cursor movements when cursor leaves viewable area
	//TODO: Handle copy/paste/cut to/from clipboard
	//TODO: Double/Triple Click to Select Word/Line
	
	u16 fontId = GetClayUIRendererFontId(renderer, font, fontStyle);
	FontAtlas* fontAtlas = GetFontAtlas(font, fontSize, fontStyle);
	NotNull(fontAtlas);
	
	if (tbox->flow.numGlyphsAlloc < tbox->text.length)
	{
		uxx newNumGlyphs = tbox->flow.numGlyphsAlloc;
		if (newNumGlyphs < 8) { newNumGlyphs = 8; }
		while (newNumGlyphs < tbox->text.length) { newNumGlyphs *= 2; }
		FontFlowGlyph* newGlyphs = AllocArray(FontFlowGlyph, tbox->arena, newNumGlyphs);
		if (tbox->flow.glyphs != nullptr) { FreeArray(FontFlowGlyph, tbox->arena, tbox->flow.numGlyphsAlloc, tbox->flow.glyphs); }
		tbox->flow.glyphs = newGlyphs;
		tbox->flow.numGlyphsAlloc = newNumGlyphs;
	}
	
	rec textboxRec = GetClayElementDrawRec(tbox->id);
	v2 cursorRelativePos = Sub(tbox->flow.endPos, textboxRec.TopLeft);
	if (tbox->text.length == 0)
	{
		//When there is no text being rendered, we need to figure out the start position of the text
		cursorRelativePos = NewV2(
			(r32)UISCALE_U16(uiScale, TEXTBOX_INNER_PADDING_X),
			textboxRec.Height/2 + fontAtlas->centerOffset
		);
		AlignV2(&cursorRelativePos);
	}
	else
	{
		for (uxx gIndex = 0; gIndex < tbox->flow.numGlyphs; gIndex++)
		{
			FontFlowGlyph* glyph = &tbox->flow.glyphs[gIndex];
			if (glyph->byteIndex == tbox->cursorEnd)
			{
				cursorRelativePos = Sub(glyph->position, textboxRec.TopLeft);
				break;
			}
		}
	}
	
	CLAY({ .id = tbox->id,
		.layout = {
			.sizing = {
				.width = CLAY_SIZING_GROW(0),
				.height = CLAY_SIZING_FIXED(fontAtlas->lineHeight + UISCALE_R32(uiScale, TEXTBOX_INNER_PADDING_Y)),
			},
			.padding = {
				UISCALE_U16(uiScale, TEXTBOX_INNER_PADDING_X), UISCALE_U16(uiScale, TEXTBOX_INNER_PADDING_X),
				UISCALE_U16(uiScale, TEXTBOX_INNER_PADDING_Y), UISCALE_U16(uiScale, TEXTBOX_INNER_PADDING_Y),
			},
			.childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
			.layoutDirection = CLAY_TOP_TO_BOTTOM,
		},
		.cornerRadius = CLAY_CORNER_RADIUS(UISCALE_R32(uiScale, 5)),
		.border = {
			.width = CLAY_BORDER_OUTSIDE(UISCALE_BORDER(uiScale, 1)),
			.color = MonokaiLightGray,
		},
		.backgroundColor = MonokaiDarkGray,
	})
	{
		uxx numStyleRanges = 0;
		RichStrStyleChangeRange* styleRanges = AllocArray(RichStrStyleChangeRange, uiArena, 1 + tbox->syntaxRanges.length);
		NotNull(styleRanges);
		
		if (tbox->cursorActive && tbox->cursorEnd != tbox->cursorStart)
		{
			styleRanges[numStyleRanges].range = NewRangeUXX(tbox->cursorStart, tbox->cursorEnd);
			styleRanges[numStyleRanges].style = NewRichStrStyleChangeEnableFlags(FontStyleFlag_Highlighted);
			numStyleRanges++;
		}
		VarArrayLoop(&tbox->syntaxRanges, rIndex)
		{
			VarArrayLoopGet(RichStrStyleChangeRange, syntaxRange, &tbox->syntaxRanges, rIndex);
			MyMemCopy(&styleRanges[numStyleRanges], syntaxRange, sizeof(RichStrStyleChangeRange));
			numStyleRanges++;
		}
		
		RichStr richText = ToRichStr(tbox->text);
		if (numStyleRanges > 0)
		{
			richText = NewRichStrFromRanges(uiArena, tbox->text, numStyleRanges, &styleRanges[0]);
		}
		
		Str8 encodedRichText = EncodeRichStr(uiArena, richText, false, false);
		CLAY_TEXT(
			encodedRichText,
			CLAY_TEXT_CONFIG({
				.fontId = fontId,
				.fontSize = (u16)fontSize,
				.textColor = MonokaiWhite,
				.wrapMode = CLAY_TEXT_WRAP_NONE,
				.textAlignment = CLAY_TEXT_ALIGN_SHRINK,
				.userData = {
					.contraction = (tbox->cursorActive && tbox->cursorEnd < tbox->text.length/2) ? TextContraction_ClipRight : TextContraction_ClipLeft,
					.flowTarget = &tbox->flow,
					.backgroundColor = MonokaiBack,
					.richText = true,
				},
			})
		);
		
		if (tbox->isFocused && tbox->cursorActive)
		{
			v2 cursorTopLeft = Add(cursorRelativePos, NewV2(UISCALE_R32(uiScale, -1), fontAtlas->maxDescend - fontAtlas->lineHeight));
			CLAY({.id = ToClayIdPrint(uiArena, "%.*sCursor", StrPrint(tbox->idStr)),
				.backgroundColor = MonokaiYellow, //TODO: Change this color
				.layout = {
					.sizing = { .width = CLAY_SIZING_FIXED(UISCALE_R32(uiScale, 2)), .height = CLAY_SIZING_FIXED(fontAtlas->lineHeight) },
				},
				.floating = {
					.attachTo = CLAY_ATTACH_TO_PARENT,
					.offset = cursorTopLeft,
					.zIndex = 5,
					.attachPoints = {
						.parent = CLAY_ATTACH_POINT_LEFT_TOP,
						.element = CLAY_ATTACH_POINT_LEFT_TOP,
					},
				},
			}) {}
		}
	}
	
	ResetFontFlowInfo(&tbox->flow);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_CLAY

#endif //  _UI_CLAY_TEXTBOX_H
