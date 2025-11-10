/*
File:   ui_clay_widget_context.h
Author: Taylor Robbins
Date:   08\10\2025
Description:
	** UI Widgets often take a bunch of parameters in their "DoX" function so rather than
	** passing each of this pieces of information as a separate parameter, we bundle them
	** all together into a single plex so the application can fill that plex once at the
	** beginning of the frame and then pass it to all widgets that need it
*/

#ifndef _UI_CLAY_WIDGET_CONTEXT_H
#define _UI_CLAY_WIDGET_CONTEXT_H

#include "base/base_defines_check.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "input/input_keys.h"
#include "input/input_mouse_btns.h"
#include "input/input_btn_state.h"
#include "mem/mem_arena.h"
#include "gfx/gfx_clay_renderer.h"
#include "os/os_process_info.h"

#if BUILD_WITH_SOKOL_GFX && BUILD_WITH_CLAY

typedef plex UiWidgetContext UiWidgetContext;
plex UiWidgetContext
{
	Arena* uiArena;
	ClayUIRenderer* renderer;
	KeyboardState* keyboard;
	MouseState* mouse;
	r32 uiScale;
	void** focusedUiElementPntr;
	CursorShape cursorShape;
	OsWindowHandle windowHandle;
	u64 programTime;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE UiWidgetContext NewUiWidgetContext(Arena* uiArena, ClayUIRenderer* renderer, KeyboardState* keyboard, MouseState* mouse, r32 uiScale, void** focusedUiElementPntr, CursorShape cursorShape, OsWindowHandle windowHandle, u64 programTime);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI UiWidgetContext NewUiWidgetContext(Arena* uiArena, ClayUIRenderer* renderer, KeyboardState* keyboard, MouseState* mouse, r32 uiScale, void** focusedUiElementPntr, CursorShape cursorShape, OsWindowHandle windowHandle, u64 programTime)
{
	UiWidgetContext result = ZEROED;
	result.uiArena = uiArena;
	result.renderer = renderer;
	result.keyboard = keyboard;
	result.mouse = mouse;
	result.uiScale = uiScale;
	result.focusedUiElementPntr = focusedUiElementPntr;
	result.cursorShape = cursorShape;
	result.windowHandle = windowHandle;
	result.programTime = programTime;
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX && BUILD_WITH_CLAY

#endif //  _UI_CLAY_WIDGET_CONTEXT_H
