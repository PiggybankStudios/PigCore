/*
File:   input_mouse_btns.h
Author: Taylor Robbins
Date:   02\03\2025
*/

#ifndef _INPUT_MOUSE_BTNS_H
#define _INPUT_MOUSE_BTNS_H

#include "base/base_defines_check.h"
#include "base/base_macros.h"

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
