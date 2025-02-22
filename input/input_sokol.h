/*
File:   input_sokol.h
Author: Taylor Robbins
Date:   02\03\2025
*/

#ifndef _INPUT_SOKOL_H
#define _INPUT_SOKOL_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "input/input_keys.h"
#include "input/input_mouse_btns.h"
#include "input/input_controller_btns.h"
#include "misc/misc_sokol_app_include.h"

#if BUILD_WITH_SOKOL_APP

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	Key GetKeyFromSokolKeycodeEx(sapp_keycode keycode, u8 alternateIndex);
	PIG_CORE_INLINE Key GetKeyFromSokolKeycode(sapp_keycode keycode);
	Key GetNonAltKeyForKey(Key key, u8 primaryIndex);
	MouseBtn GetMouseBtnFromSokolMouseButton(sapp_mousebutton mouseButton);
	bool HandleSokolKeyboardAndMouseEvents(const sapp_event* event, u64 currentTime, KeyboardState* keyboard, MouseState* mouse, bool isMouseLocked);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP Key GetKeyFromSokolKeycodeEx(sapp_keycode keycode, u8 alternateIndex)
{
	#if LANGUAGE_IS_C
	_Static_assert(MAX_ALT_KEY_MAPPINGS == 2);
	#endif
	if (alternateIndex == 0)
	{
		switch (keycode)
		{
			case SAPP_KEYCODE_SPACE: return Key_Space;
			case SAPP_KEYCODE_APOSTROPHE: return Key_Quote;
			case SAPP_KEYCODE_COMMA: return Key_Comma;
			case SAPP_KEYCODE_MINUS: return Key_Minus;
			case SAPP_KEYCODE_PERIOD: return Key_Period;
			case SAPP_KEYCODE_SLASH: return Key_ForwardSlash;
			case SAPP_KEYCODE_0: return Key_Digit0;
			case SAPP_KEYCODE_1: return Key_Digit1;
			case SAPP_KEYCODE_2: return Key_Digit2;
			case SAPP_KEYCODE_3: return Key_Digit3;
			case SAPP_KEYCODE_4: return Key_Digit4;
			case SAPP_KEYCODE_5: return Key_Digit5;
			case SAPP_KEYCODE_6: return Key_Digit6;
			case SAPP_KEYCODE_7: return Key_Digit7;
			case SAPP_KEYCODE_8: return Key_Digit8;
			case SAPP_KEYCODE_9: return Key_Digit9;
			case SAPP_KEYCODE_SEMICOLON: return Key_Colon;
			case SAPP_KEYCODE_EQUAL: return Key_Plus;
			case SAPP_KEYCODE_A: return Key_A;
			case SAPP_KEYCODE_B: return Key_B;
			case SAPP_KEYCODE_C: return Key_C;
			case SAPP_KEYCODE_D: return Key_D;
			case SAPP_KEYCODE_E: return Key_E;
			case SAPP_KEYCODE_F: return Key_F;
			case SAPP_KEYCODE_G: return Key_G;
			case SAPP_KEYCODE_H: return Key_H;
			case SAPP_KEYCODE_I: return Key_I;
			case SAPP_KEYCODE_J: return Key_J;
			case SAPP_KEYCODE_K: return Key_K;
			case SAPP_KEYCODE_L: return Key_L;
			case SAPP_KEYCODE_M: return Key_M;
			case SAPP_KEYCODE_N: return Key_N;
			case SAPP_KEYCODE_O: return Key_O;
			case SAPP_KEYCODE_P: return Key_P;
			case SAPP_KEYCODE_Q: return Key_Q;
			case SAPP_KEYCODE_R: return Key_R;
			case SAPP_KEYCODE_S: return Key_S;
			case SAPP_KEYCODE_T: return Key_T;
			case SAPP_KEYCODE_U: return Key_U;
			case SAPP_KEYCODE_V: return Key_V;
			case SAPP_KEYCODE_W: return Key_W;
			case SAPP_KEYCODE_X: return Key_X;
			case SAPP_KEYCODE_Y: return Key_Y;
			case SAPP_KEYCODE_Z: return Key_Z;
			case SAPP_KEYCODE_LEFT_BRACKET: return Key_OpenBracket;
			case SAPP_KEYCODE_BACKSLASH: return Key_Pipe;
			case SAPP_KEYCODE_RIGHT_BRACKET: return Key_CloseBracket;
			case SAPP_KEYCODE_GRAVE_ACCENT: return Key_Tilde;
			// case SAPP_KEYCODE_WORLD_1: return Key_World_1;
			// case SAPP_KEYCODE_WORLD_2: return Key_World_2;
			case SAPP_KEYCODE_ESCAPE: return Key_Escape;
			case SAPP_KEYCODE_ENTER: return Key_PrimaryEnter;
			case SAPP_KEYCODE_TAB: return Key_Tab;
			case SAPP_KEYCODE_BACKSPACE: return Key_Backspace;
			case SAPP_KEYCODE_INSERT: return Key_Insert;
			case SAPP_KEYCODE_DELETE: return Key_Delete;
			case SAPP_KEYCODE_RIGHT: return Key_Right;
			case SAPP_KEYCODE_LEFT: return Key_Left;
			case SAPP_KEYCODE_DOWN: return Key_Down;
			case SAPP_KEYCODE_UP: return Key_Up;
			case SAPP_KEYCODE_PAGE_UP: return Key_PageUp;
			case SAPP_KEYCODE_PAGE_DOWN: return Key_PageDown;
			case SAPP_KEYCODE_HOME: return Key_Home;
			case SAPP_KEYCODE_END: return Key_End;
			case SAPP_KEYCODE_CAPS_LOCK: return Key_CapsLock;
			case SAPP_KEYCODE_SCROLL_LOCK: return Key_ScrollLock;
			case SAPP_KEYCODE_NUM_LOCK: return Key_NumLock;
			case SAPP_KEYCODE_PRINT_SCREEN: return Key_PrintScreen;
			case SAPP_KEYCODE_PAUSE: return Key_Pause;
			case SAPP_KEYCODE_F1: return Key_F1;
			case SAPP_KEYCODE_F2: return Key_F2;
			case SAPP_KEYCODE_F3: return Key_F3;
			case SAPP_KEYCODE_F4: return Key_F4;
			case SAPP_KEYCODE_F5: return Key_F5;
			case SAPP_KEYCODE_F6: return Key_F6;
			case SAPP_KEYCODE_F7: return Key_F7;
			case SAPP_KEYCODE_F8: return Key_F8;
			case SAPP_KEYCODE_F9: return Key_F9;
			case SAPP_KEYCODE_F10: return Key_F10;
			case SAPP_KEYCODE_F11: return Key_F11;
			case SAPP_KEYCODE_F12: return Key_F12;
			// case SAPP_KEYCODE_F13: return Key_F13;
			// case SAPP_KEYCODE_F14: return Key_F14;
			// case SAPP_KEYCODE_F15: return Key_F15;
			// case SAPP_KEYCODE_F16: return Key_F16;
			// case SAPP_KEYCODE_F17: return Key_F17;
			// case SAPP_KEYCODE_F18: return Key_F18;
			// case SAPP_KEYCODE_F19: return Key_F19;
			// case SAPP_KEYCODE_F20: return Key_F20;
			// case SAPP_KEYCODE_F21: return Key_F21;
			// case SAPP_KEYCODE_F22: return Key_F22;
			// case SAPP_KEYCODE_F23: return Key_F23;
			// case SAPP_KEYCODE_F24: return Key_F24;
			// case SAPP_KEYCODE_F25: return Key_F25;
			case SAPP_KEYCODE_KP_0: return Key_Numpad0;
			case SAPP_KEYCODE_KP_1: return Key_Numpad1;
			case SAPP_KEYCODE_KP_2: return Key_Numpad2;
			case SAPP_KEYCODE_KP_3: return Key_Numpad3;
			case SAPP_KEYCODE_KP_4: return Key_Numpad4;
			case SAPP_KEYCODE_KP_5: return Key_Numpad5;
			case SAPP_KEYCODE_KP_6: return Key_Numpad6;
			case SAPP_KEYCODE_KP_7: return Key_Numpad7;
			case SAPP_KEYCODE_KP_8: return Key_Numpad8;
			case SAPP_KEYCODE_KP_9: return Key_Numpad9;
			case SAPP_KEYCODE_KP_DECIMAL: return Key_NumpadPeriod;
			case SAPP_KEYCODE_KP_DIVIDE: return Key_NumpadDivide;
			case SAPP_KEYCODE_KP_MULTIPLY: return Key_NumpadMultiply;
			case SAPP_KEYCODE_KP_SUBTRACT: return Key_NumpadSubtract;
			case SAPP_KEYCODE_KP_ADD: return Key_NumpadAdd;
			case SAPP_KEYCODE_KP_ENTER: return Key_NumpadEnter;
			// case SAPP_KEYCODE_KP_EQUAL: return Key_NumpadEqual;
			case SAPP_KEYCODE_LEFT_SHIFT: return Key_LeftShift;
			case SAPP_KEYCODE_LEFT_CONTROL: return Key_LeftControl;
			case SAPP_KEYCODE_LEFT_ALT: return Key_LeftAlt;
			case SAPP_KEYCODE_LEFT_SUPER: return Key_LeftCommand;
			case SAPP_KEYCODE_RIGHT_SHIFT: return Key_RightShift;
			case SAPP_KEYCODE_RIGHT_CONTROL: return Key_RightControl;
			case SAPP_KEYCODE_RIGHT_ALT: return Key_RightAlt;
			case SAPP_KEYCODE_RIGHT_SUPER: return Key_RightCommand;
			// case SAPP_KEYCODE_MENU: return Key_Menu;
			default: return Key_None;
		}
	}
	else if (alternateIndex == 1)
	{
		switch (keycode)
		{
			case SAPP_KEYCODE_0: return Key_0;
			case SAPP_KEYCODE_1: return Key_1;
			case SAPP_KEYCODE_2: return Key_2;
			case SAPP_KEYCODE_3: return Key_3;
			case SAPP_KEYCODE_4: return Key_4;
			case SAPP_KEYCODE_5: return Key_5;
			case SAPP_KEYCODE_6: return Key_6;
			case SAPP_KEYCODE_7: return Key_7;
			case SAPP_KEYCODE_8: return Key_8;
			case SAPP_KEYCODE_9: return Key_9;
			case SAPP_KEYCODE_KP_0: return Key_0;
			case SAPP_KEYCODE_KP_1: return Key_1;
			case SAPP_KEYCODE_KP_2: return Key_2;
			case SAPP_KEYCODE_KP_3: return Key_3;
			case SAPP_KEYCODE_KP_4: return Key_4;
			case SAPP_KEYCODE_KP_5: return Key_5;
			case SAPP_KEYCODE_KP_6: return Key_6;
			case SAPP_KEYCODE_KP_7: return Key_7;
			case SAPP_KEYCODE_KP_8: return Key_8;
			case SAPP_KEYCODE_KP_9: return Key_9;
			case SAPP_KEYCODE_ENTER: return Key_Enter;
			case SAPP_KEYCODE_KP_ENTER: return Key_Enter;
			case SAPP_KEYCODE_LEFT_SHIFT: return Key_Shift;
			case SAPP_KEYCODE_RIGHT_SHIFT: return Key_Shift;
			case SAPP_KEYCODE_LEFT_CONTROL: return Key_Control;
			case SAPP_KEYCODE_RIGHT_CONTROL: return Key_Control;
			case SAPP_KEYCODE_LEFT_ALT: return Key_Alt;
			case SAPP_KEYCODE_RIGHT_ALT: return Key_Alt;
			case SAPP_KEYCODE_LEFT_SUPER: return Key_Command;
			case SAPP_KEYCODE_RIGHT_SUPER: return Key_Command;
		}
	}
	return Key_None;
}
PEXPI Key GetKeyFromSokolKeycode(sapp_keycode keycode)
{
	return GetKeyFromSokolKeycodeEx(keycode, 0);
}

PEXP Key GetNonAltKeyForKey(Key key, u8 primaryIndex)
{
	switch (key)
	{
		case Key_Enter: return (primaryIndex == 0) ? Key_PrimaryEnter : ((primaryIndex == 1) ? Key_NumpadEnter : Key_None);
		case Key_0: return (primaryIndex == 0) ? Key_Digit0 : ((primaryIndex == 1) ? Key_Numpad0 : Key_None);
		case Key_1: return (primaryIndex == 0) ? Key_Digit1 : ((primaryIndex == 1) ? Key_Numpad1 : Key_None);
		case Key_2: return (primaryIndex == 0) ? Key_Digit2 : ((primaryIndex == 1) ? Key_Numpad2 : Key_None);
		case Key_3: return (primaryIndex == 0) ? Key_Digit3 : ((primaryIndex == 1) ? Key_Numpad3 : Key_None);
		case Key_4: return (primaryIndex == 0) ? Key_Digit4 : ((primaryIndex == 1) ? Key_Numpad4 : Key_None);
		case Key_5: return (primaryIndex == 0) ? Key_Digit5 : ((primaryIndex == 1) ? Key_Numpad5 : Key_None);
		case Key_6: return (primaryIndex == 0) ? Key_Digit6 : ((primaryIndex == 1) ? Key_Numpad6 : Key_None);
		case Key_7: return (primaryIndex == 0) ? Key_Digit7 : ((primaryIndex == 1) ? Key_Numpad7 : Key_None);
		case Key_8: return (primaryIndex == 0) ? Key_Digit8 : ((primaryIndex == 1) ? Key_Numpad8 : Key_None);
		case Key_9: return (primaryIndex == 0) ? Key_Digit9 : ((primaryIndex == 1) ? Key_Numpad9 : Key_None);
		case Key_Shift: return (primaryIndex == 0) ? Key_LeftShift : ((primaryIndex == 1) ? Key_RightShift : Key_None); 
		case Key_Control: return (primaryIndex == 0) ? Key_LeftControl : ((primaryIndex == 1) ? Key_RightControl : Key_None); 
		case Key_Alt: return (primaryIndex == 0) ? Key_LeftAlt : ((primaryIndex == 1) ? Key_RightAlt : Key_None); 
		case Key_Command: return (primaryIndex == 0) ? Key_LeftCommand : ((primaryIndex == 1) ? Key_RightCommand : Key_None); 
		default: return Key_None;
	}
}

PEXP MouseBtn GetMouseBtnFromSokolMouseButton(sapp_mousebutton mouseButton)
{
	switch (mouseButton)
	{
		case SAPP_MOUSEBUTTON_LEFT: return MouseBtn_Left;
		case SAPP_MOUSEBUTTON_RIGHT: return MouseBtn_Right;
		case SAPP_MOUSEBUTTON_MIDDLE: return MouseBtn_Middle;
		default: return MouseBtn_None;
	}
}

// Returns true if the event is handled as a change to KeyboardState or MouseState
PEXP bool HandleSokolKeyboardAndMouseEvents(const sapp_event* event, u64 currentTime, KeyboardState* keyboard, MouseState* mouse, bool isMouseLocked)
{
	NotNull(event);
	NotNull(keyboard);
	NotNull(mouse);
	bool handled = false;
	switch (event->type)
	{
		case SAPP_EVENTTYPE_KEY_DOWN: [[fallthrough]];
		case SAPP_EVENTTYPE_KEY_UP:
		{
			#if LANGUAGE_IS_C
			_Static_assert(MAX_ALT_KEY_MAPPINGS == 2);
			#endif
			bool isKeyDown = (event->type == SAPP_EVENTTYPE_KEY_DOWN);
			Key primaryKey = GetKeyFromSokolKeycodeEx(event->key_code, 0);
			Key altKey = GetKeyFromSokolKeycodeEx(event->key_code, 1);
			if (primaryKey != Key_None) { UpdateKeyboardKey(keyboard, currentTime, primaryKey, isKeyDown); }
			if (altKey != Key_None)
			{
				// When two keycodes are mapped to one Key, we have to avoid producing an early release event if both keys were
				// held down at the same time and then only one key was released. The GetNonAltKeyForKey function helps us find
				// any other keys that feed into this shared alt key and track them back to a Key enum value that is not shared
				// and therefore holds the value for whether that other key is being held currently.
				Key otherPrimaryKey = GetNonAltKeyForKey(altKey, 0);
				if (otherPrimaryKey == primaryKey) { otherPrimaryKey = GetNonAltKeyForKey(altKey, 1); }
				bool isOtherPrimaryKeyDown = false;
				if (otherPrimaryKey != Key_None) { isOtherPrimaryKeyDown = keyboard->keys[otherPrimaryKey].isDown; }
				UpdateKeyboardKey(keyboard, currentTime, altKey, isKeyDown || isOtherPrimaryKeyDown);
			}
			handled = true;
		} break;
		
		case SAPP_EVENTTYPE_MOUSE_DOWN: [[fallthrough]];
		case SAPP_EVENTTYPE_MOUSE_UP:
		{
			MouseBtn mouseBtn = GetMouseBtnFromSokolMouseButton(event->mouse_button);
			if (mouseBtn != MouseBtn_None) { UpdateMouseBtn(mouse, currentTime, mouseBtn, (event->type == SAPP_EVENTTYPE_MOUSE_DOWN)); }
			handled = true;
		} break;
		
		case SAPP_EVENTTYPE_MOUSE_SCROLL:
		{
			UpdateMouseScroll(mouse, currentTime, NewV2(event->scroll_x, event->scroll_y));
			handled = true;
		} break;
		
		case SAPP_EVENTTYPE_MOUSE_MOVE:
		{
			if (isMouseLocked)
			{
				UpdateMouseLockedDelta(mouse, currentTime, NewV2(event->mouse_dx, event->mouse_dy));
			}
			else
			{
				UpdateMousePosition(mouse, currentTime, NewV2(event->mouse_x, event->mouse_y));
			}
			handled = true;
		} break;
		
		case SAPP_EVENTTYPE_MOUSE_ENTER: [[fallthrough]];
		case SAPP_EVENTTYPE_MOUSE_LEAVE:
		{
			UpdateMouseOverWindow(mouse, currentTime, (event->type == SAPP_EVENTTYPE_MOUSE_ENTER));
			handled = true;
		} break;
		
		case SAPP_EVENTTYPE_CHAR:
		{
			u8 modifierKeys = ModifierKey_None;
			if (IsFlagSet(event->modifiers, SAPP_MODIFIER_SHIFT)) { FlagSet(modifierKeys, ModifierKey_Shift);   }
			if (IsFlagSet(event->modifiers, SAPP_MODIFIER_CTRL))  { FlagSet(modifierKeys, ModifierKey_Control); }
			if (IsFlagSet(event->modifiers, SAPP_MODIFIER_ALT))   { FlagSet(modifierKeys, ModifierKey_Alt);     }
			AddKeyboardCharInput(keyboard, event->char_code, modifierKeys);
		} break;
		
		default: break;
	}
	return handled;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_APP

#endif //  _INPUT_SOKOL_H
 
#if defined(_UI_IMGUI_H) && defined(_INPUT_SOKOL_H) && defined(_INPUT_BTN_STATE_H)
#include "cross/cross_imgui_input_sokol_and_btn_state.h"
#endif
