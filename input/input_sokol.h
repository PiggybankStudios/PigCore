/*
File:   input_sokol.h
Author: Taylor Robbins
Date:   02\03\2025
Description:
	** Holds functions that help us convert sokol_app.h input events into our PigCore
	** specific formats (like KeyboardState, MouseState, and TouchscreenState).
	** In particular this file holds HandleSokolKeyboardAndMouseEvents which is a single
	** function that routes events related to mouse\keyboard\touchscreen to the appropriate
	** functions and returns true when an event was handled.
*/

#ifndef _INPUT_SOKOL_H
#define _INPUT_SOKOL_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_notifications.h"
#include "input/input_keys.h"
#include "input/input_mouse_btns.h"
#include "input/input_controller_btns.h"
#include "input/input_touch.h"
#include "lib/lib_sokol_app.h"

#if BUILD_WITH_SOKOL_APP

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	Key GetKeyFromSokolKeycodeEx(sapp_keycode keycode, u8 alternateIndex);
	PIG_CORE_INLINE Key GetKeyFromSokolKeycode(sapp_keycode keycode);
	Key GetNonAltKeyForKey(Key key, u8 primaryIndex);
	MouseBtn GetMouseBtnFromSokolMouseButton(sapp_mousebutton mouseButton);
	bool HandleSokolKeyboardMouseAndTouchEvents(const sapp_event* event, u64 currentTime, v2i screenSize, KeyboardState* keyboard, MouseState* mouse, TouchscreenState* touchscreen, bool isMouseLocked);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP Key GetKeyFromSokolKeycodeEx(sapp_keycode keycode, u8 alternateIndex)
{
	#if STATIC_ASSERT_AVAILABLE
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
PEXP bool HandleSokolKeyboardMouseAndTouchEvents(const sapp_event* event, u64 currentTime, v2i screenSize, KeyboardState* keyboard, MouseState* mouse, TouchscreenState* touchscreen, bool isMouseLocked)
{
	NotNull(event);
	NotNull(keyboard);
	NotNull(mouse);
	bool handled = false;
	switch (event->type)
	{
		// +==============================+
		// |  SAPP_EVENTTYPE_KEY_DOWN/UP  |
		// +==============================+
		case SAPP_EVENTTYPE_KEY_DOWN: [[fallthrough]];
		case SAPP_EVENTTYPE_KEY_UP:
		{
			#if STATIC_ASSERT_AVAILABLE
			_Static_assert(MAX_ALT_KEY_MAPPINGS == 2);
			#endif
			bool isKeyDown = (event->type == SAPP_EVENTTYPE_KEY_DOWN);
			Key primaryKey = GetKeyFromSokolKeycodeEx(event->key_code, 0);
			Key altKey = GetKeyFromSokolKeycodeEx(event->key_code, 1);
			if (primaryKey != Key_None) { UpdateKeyboardKey(keyboard, currentTime, primaryKey, isKeyDown, event->key_repeat); }
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
				UpdateKeyboardKey(keyboard, currentTime, altKey, isKeyDown || isOtherPrimaryKeyDown, event->key_repeat);
			}
			handled = true;
		} break;
		
		// +==============================+
		// | SAPP_EVENTTYPE_MOUSE_DOWN/UP |
		// +==============================+
		case SAPP_EVENTTYPE_MOUSE_DOWN: [[fallthrough]];
		case SAPP_EVENTTYPE_MOUSE_UP:
		{
			MouseBtn mouseBtn = GetMouseBtnFromSokolMouseButton(event->mouse_button);
			if (mouseBtn != MouseBtn_None) { UpdateMouseBtn(mouse, currentTime, mouseBtn, (event->type == SAPP_EVENTTYPE_MOUSE_DOWN)); }
			handled = true;
		} break;
		
		// +==============================+
		// | SAPP_EVENTTYPE_MOUSE_SCROLL  |
		// +==============================+
		case SAPP_EVENTTYPE_MOUSE_SCROLL:
		{
			UpdateMouseScroll(mouse, currentTime, NewV2(event->scroll_x, event->scroll_y));
			handled = true;
		} break;
		
		// +==============================+
		// |  SAPP_EVENTTYPE_MOUSE_MOVE   |
		// +==============================+
		case SAPP_EVENTTYPE_MOUSE_MOVE:
		{
			if (isMouseLocked)
			{
				if (!mouse->isOverWindow) { mouse->isOverWindow = true; } //if the mouse is locked, we assume it's over the window
				UpdateMouseLockedDelta(mouse, currentTime, NewV2(event->mouse_dx, event->mouse_dy));
			}
			else
			{
				UpdateMousePosition(mouse, currentTime, NewV2(event->mouse_x, event->mouse_y));
				
				//NOTE: This check is needed because isOverWindow starts as false even if the mouse is inside the window when our application starts.
				// With this, we consider it inside the window the minute it moves. We don't need to handle mouse leaving the window since we should
				// get events for mouse leaving reliably
				if (!mouse->isOverWindow)
				{
					if (event->mouse_x >= 0 && event->mouse_y >= 0 && event->mouse_x < screenSize.Width && event->mouse_y < screenSize.Height)
					{
						mouse->isOverWindow = true;
					}
				}
			}
			handled = true;
		} break;
		
		// +==================================+
		// | SAPP_EVENTTYPE_MOUSE_ENTER/LEAVE |
		// +==================================+
		case SAPP_EVENTTYPE_MOUSE_ENTER: [[fallthrough]];
		case SAPP_EVENTTYPE_MOUSE_LEAVE:
		{
			UpdateMouseOverWindow(mouse, currentTime, (event->type == SAPP_EVENTTYPE_MOUSE_ENTER));
			handled = true;
		} break;
		
		// +==============================+
		// |     SAPP_EVENTTYPE_CHAR      |
		// +==============================+
		case SAPP_EVENTTYPE_CHAR:
		{
			u8 modifierKeys = ModifierKey_None;
			if (IsFlagSet(event->modifiers, SAPP_MODIFIER_SHIFT)) { FlagSet(modifierKeys, ModifierKey_Shift);   }
			if (IsFlagSet(event->modifiers, SAPP_MODIFIER_CTRL))  { FlagSet(modifierKeys, ModifierKey_Control); }
			if (IsFlagSet(event->modifiers, SAPP_MODIFIER_ALT))   { FlagSet(modifierKeys, ModifierKey_Alt);     }
			AddKeyboardCharInput(keyboard, event->char_code, modifierKeys);
			handled = true;
		} break;
		
		// +==============================+
		// | SAPP_EVENTTYPE_TOUCHES_BEGAN |
		// +==============================+
		case SAPP_EVENTTYPE_TOUCHES_BEGAN:
		{
			if (touchscreen != nullptr)
			{
				// PrintLine_W("Got TOUCHES_BEGAN %d", event->num_touches);
				for (int tIndex = 0; tIndex < event->num_touches; tIndex++)
				{
					const sapp_touchpoint* sokolTouch = &event->touches[tIndex];
					DebugAssert(sokolTouch->identifier < UINT32_MAX);
					uxx touchId = (uxx)sokolTouch->identifier+1;
					v2 touchPos = NewV2(sokolTouch->pos_x, sokolTouch->pos_y);
					//TODO: Should we use sokolTouch->android_tooltype (i.e. to check if it's a stylus, mouse, or touch)
					// PrintLine_W("Finding touch %llu", touchId);
					TouchState* touch = FindTouchById(touchscreen, touchId);
					if (touch == nullptr && sokolTouch->changed)
					{
						// PrintLine_W("Starting new touch %llu", touchId);
						touch = StartNewTouch(touchscreen, touchId, touchPos, currentTime);
						if (touch == nullptr) { NotifyPrint_E("Ran out of touch slots in TouchscreenState!"); continue; }
					}
					if (touch != nullptr) { UpdateTouchStatePosition(touch, touchPos, currentTime); }
				}
				handled = true;
			}
		} break;
		
		// +==============================+
		// | SAPP_EVENTTYPE_TOUCHES_ENDED |
		// +==============================+
		case SAPP_EVENTTYPE_TOUCHES_ENDED:
		{
			if (touchscreen != nullptr)
			{
				// PrintLine_W("Got TOUCHES_ENDED %d", event->num_touches);
				for (int tIndex = 0; tIndex < event->num_touches; tIndex++)
				{
					const sapp_touchpoint* sokolTouch = &event->touches[tIndex];
					DebugAssert(sokolTouch->identifier < UINT32_MAX);
					uxx touchId = (uxx)sokolTouch->identifier+1;
					TouchState* touch = FindTouchById(touchscreen, touchId);
					if (touch != nullptr)
					{
						v2 touchPos = NewV2(sokolTouch->pos_x, sokolTouch->pos_y);
						UpdateTouchStatePosition(touch, touchPos, currentTime);
						if (sokolTouch->changed)
						{
							// PrintLine_W("Ended touch %llu", touch->id);
							touch->stopped = true;
							touch->stopTime = currentTime;
						}
					}
					else if (sokolTouch->changed) { PrintLine_W("Got ENDED event for unknown touch %llu!", touchId); }
				}
				handled = true;
			}
		} break;
		
		// +==================================+
		// | SAPP_EVENTTYPE_TOUCHES_CANCELLED |
		// +==================================+
		case SAPP_EVENTTYPE_TOUCHES_CANCELLED:
		{
			if (touchscreen != nullptr)
			{
				// PrintLine_W("Got TOUCHES_CANCELLED %d", event->num_touches);
				for (int tIndex = 0; tIndex < event->num_touches; tIndex++)
				{
					const sapp_touchpoint* sokolTouch = &event->touches[tIndex];
					DebugAssert(sokolTouch->identifier < UINT32_MAX);
					uxx touchId = (uxx)sokolTouch->identifier+1;
					TouchState* touch = FindTouchById(touchscreen, touchId);
					if (touch != nullptr)
					{
						v2 touchPos = NewV2(sokolTouch->pos_x, sokolTouch->pos_y);
						UpdateTouchStatePosition(touch, touchPos, currentTime);
						if (sokolTouch->changed)
						{
							// PrintLine_W("Cancelled touch %llu", touch->id);
							touch->stopped = true;
							touch->stopTime = currentTime;
						}
					}
					else if (sokolTouch->changed) { PrintLine_W("Got CANCELLED event for unknown touch %llu!", touchId); }
				}
				handled = true;
			}
		} break;
		
		// +==============================+
		// | SAPP_EVENTTYPE_TOUCHES_MOVED |
		// +==============================+
		case SAPP_EVENTTYPE_TOUCHES_MOVED:
		{
			if (touchscreen != nullptr)
			{
				for (int tIndex = 0; tIndex < event->num_touches; tIndex++)
				{
					const sapp_touchpoint* sokolTouch = &event->touches[tIndex];
					DebugAssert(sokolTouch->identifier < UINT32_MAX);
					uxx touchId = (uxx)sokolTouch->identifier+1;
					TouchState* touch = FindTouchById(touchscreen, touchId);
					if (touch != nullptr) { UpdateTouchStatePosition(touch, NewV2(sokolTouch->pos_x, sokolTouch->pos_y), currentTime); }
				}
				handled = true;
			}
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
