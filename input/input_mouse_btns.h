/*
File:   input_mouse_btns.h
Author: Taylor Robbins
Date:   02\03\2025
Description:
	** Holds the MouseBtn enumeration that allows us to talk about all the typical
	** buttons you might see on a mouse (Left, Right, Middle, Scroll, etc.)
	** NOTE: See cross_mouse_btns_btn_state_and_vectors.h for MouseState structure
	** which converts input events related to the mouse into various formats that
	** make it easy to write mouse input handling code in an application.
	** NOTE: See HandleSokolKeyboardMouseAndTouchEvents in input_sokol.h
*/

#ifndef _INPUT_MOUSE_BTNS_H
#define _INPUT_MOUSE_BTNS_H

#include "base/base_defines_check.h"
#include "base/base_macros.h"

//NOTE: The name "CursorShape" conflicts with X11 headers
typedef enum MouseCursorShape MouseCursorShape;
enum MouseCursorShape
{
	MouseCursorShape_Default = 0,
	MouseCursorShape_Point,
	MouseCursorShape_IBeam,
	MouseCursorShape_ResizeHori,
	MouseCursorShape_ResizeVert,
	MouseCursorShape_ResizeDiagNwSe, //TODO: This is the naming conventions others use, can we think of a better one? Maybe "LeanLeft" and "LeanRight"?
	MouseCursorShape_ResizeDiagSwNe, //TODO: This is the naming conventions others use, can we think of a better one? Maybe "LeanLeft" and "LeanRight"?
	MouseCursorShape_Count,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetMouseCursorShapeStr(MouseCursorShape enumValue);
#else
PEXP const char* GetMouseCursorShapeStr(MouseCursorShape enumValue)
{
	switch (enumValue)
	{
		case MouseCursorShape_Default:        return "Default";
		case MouseCursorShape_Point:          return "Point";
		case MouseCursorShape_IBeam:          return "IBeam";
		case MouseCursorShape_ResizeHori:     return "ResizeHori";
		case MouseCursorShape_ResizeVert:     return "ResizeVert";
		case MouseCursorShape_ResizeDiagNwSe: return "ResizeDiagNwSe";
		case MouseCursorShape_ResizeDiagSwNe: return "ResizeDiagSwNe";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef enum MouseBtn MouseBtn;
enum MouseBtn
{
	MouseBtn_None = 0,
	MouseBtn_Left,
	MouseBtn_Right,
	MouseBtn_Middle,
	MouseBtn_Extra1,
	MouseBtn_Extra2,
	MouseBtn_Scroll,
	MouseBtn_ScrollUp,
	MouseBtn_ScrollDown,
	MouseBtn_Count,
};

#if !PIG_CORE_IMPLEMENTATION
const char* GetMouseBtnStr(MouseBtn enumValue);
#else
PEXP const char* GetMouseBtnStr(MouseBtn enumValue)
{
	switch (enumValue)
	{
		case MouseBtn_None:       return "None";
		case MouseBtn_Left:       return "Left";
		case MouseBtn_Right:      return "Right";
		case MouseBtn_Middle:     return "Middle";
		case MouseBtn_Extra1:     return "Extra1";
		case MouseBtn_Extra2:     return "Extra2";
		case MouseBtn_Scroll:     return "Scroll";
		case MouseBtn_ScrollUp:   return "ScrollUp";
		case MouseBtn_ScrollDown: return "ScrollDown";
		default: return UNKNOWN_STR;
	}
}
#endif

#endif //  _INPUT_MOUSE_BTNS_H

#if defined(_INPUT_BTN_STATE_H) && defined(_INPUT_MOUSE_BTNS_H) && defined(_STRUCT_VECTORS_H)
#include "cross/cross_mouse_btns_btn_state_and_vectors.h"
#endif
