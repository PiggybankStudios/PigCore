/*
File:   input_keys.h
Author: Taylor Robbins
Date:   02\03\2025
Description:
	** Contains the Keys enumeration which is our canonical way of talking about
	** Keyboard keys across all platforms that we support. This enumeration does
	** not match each keyboard type or each operating system's naming convention,
	** rather it describes the front-end for how we want to talk about keys from 
	** the application side of things and the exact way that gets mapped to each
	** scenario can be fiddled with without changing the applications (usually).
	** Note that some key values are not mutually exclusive, for example Key_9
	** represents either Key_Digit9 or Key_Numpad9.
*/

#ifndef _INPUT_KEYS_H
#define _INPUT_KEYS_H

#include "base/base_defines_check.h"
#include "base/base_macros.h"

//Each Key enum value can have multiple physical keys that activate it, this number speicifies
//how many physical keys we can have mapped to a single enum value
#define MAX_ALT_KEY_MAPPINGS  2

typedef enum ModifierKey ModifierKey;
enum ModifierKey
{
	ModifierKey_None    = 0x00,
	ModifierKey_Shift   = 0x01,
	ModifierKey_Control = 0x02,
	ModifierKey_Alt     = 0x04,
	ModifierKey_All     = 0x07,
};

#if !PIG_CORE_IMPLEMENTATION
const char* GetModifierKeyStr(ModifierKey enumValue);
const char* GetModifierKeyShortStr(ModifierKey enumValue);
#else
PEXP const char* GetModifierKeyStr(ModifierKey enumValue)
{
	switch (enumValue)
	{
		case ModifierKey_None:    return "None";
		case ModifierKey_Shift:   return "Shift";
		case ModifierKey_Control: return "Control";
		case ModifierKey_Alt:     return "Alt";
		case ModifierKey_All:     return "All";
		default: return UNKNOWN_STR;
	}
}
PEXP const char* GetModifierKeyShortStr(ModifierKey enumValue)
{
	switch (enumValue)
	{
		case ModifierKey_Shift:   return "Shift";
		case ModifierKey_Control: return "Ctrl";
		case ModifierKey_Alt:     return "Alt";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef enum Key Key;
enum Key
{
	Key_None = 0,
	
	Key_0,
	Key_1,
	Key_2,
	Key_3,
	Key_4,
	Key_5,
	Key_6,
	Key_7,
	Key_8,
	Key_9,
	
	Key_Digit0,
	Key_Digit1,
	Key_Digit2,
	Key_Digit3,
	Key_Digit4,
	Key_Digit5,
	Key_Digit6,
	Key_Digit7,
	Key_Digit8,
	Key_Digit9,
	
	Key_Numpad0,
	Key_Numpad1,
	Key_Numpad2,
	Key_Numpad3,
	Key_Numpad4,
	Key_Numpad5,
	Key_Numpad6,
	Key_Numpad7,
	Key_Numpad8,
	Key_Numpad9,
	Key_NumpadPeriod,
	Key_NumpadDivide,
	Key_NumpadMultiply,
	Key_NumpadSubtract,
	Key_NumpadAdd,
	Key_NumpadEnter,
	
	Key_F1,
	Key_F2,
	Key_F3,
	Key_F4,
	Key_F5,
	Key_F6,
	Key_F7,
	Key_F8,
	Key_F9,
	Key_F10,
	Key_F11,
	Key_F12,
	
	Key_A,
	Key_B,
	Key_C,
	Key_D,
	Key_E,
	Key_F,
	Key_G,
	Key_H,
	Key_I,
	Key_J,
	Key_K,
	Key_L,
	Key_M,
	Key_N,
	Key_O,
	Key_P,
	Key_Q,
	Key_R,
	Key_S,
	Key_T,
	Key_U,
	Key_V,
	Key_W,
	Key_X,
	Key_Y,
	Key_Z,
	
	Key_Tilde,
	Key_Minus,
	Key_Plus,
	Key_Comma,
	Key_Period,
	Key_ForwardSlash,
	Key_Colon,
	Key_Quote,
	Key_OpenBracket,
	Key_CloseBracket,
	Key_Pipe,
	
	Key_Escape,
	Key_Enter, //Both PrimaryEnter and NumpadEnter
	Key_PrimaryEnter,
	Key_Backspace,
	Key_Space,
	Key_CapsLock,
	Key_Tab,
	
	Key_Up,
	Key_Down,
	Key_Left,
	Key_Right,
	
	Key_Insert,
	Key_Delete,
	Key_Home,
	Key_End,
	Key_PageUp,
	Key_PageDown,
	Key_NumLock,
	Key_ScrollLock,
	Key_PrintScreen,
	Key_Pause,
	
	Key_Shift,
	Key_Control,
	Key_Alt,
	Key_Windows, //Windows specific
	Key_Command, //OSX specific
	Key_Option, //OSX specific
	
	Key_LeftShift,
	Key_LeftControl,
	Key_LeftAlt,
	Key_LeftWindows, //Windows specific
	Key_LeftCommand, //OSX specific
	Key_LeftOption, //OSX specific
	
	Key_RightShift,
	Key_RightControl,
	Key_RightAlt,
	Key_RightWindows, //Windows specific
	Key_RightCommand, //OSX specific
	Key_RightOption, //OSX specific
	
	Key_Count,
};

#if !PIG_CORE_IMPLEMENTATION
const char* GetKeyStr(Key key);
#else
PEXP const char* GetKeyStr(Key key)
{
	switch (key)
	{
		case Key_None: return "None";
		case Key_0: return "0";
		case Key_1: return "1";
		case Key_2: return "2";
		case Key_3: return "3";
		case Key_4: return "4";
		case Key_5: return "5";
		case Key_6: return "6";
		case Key_7: return "7";
		case Key_8: return "8";
		case Key_9: return "9";
		case Key_Digit0: return "Digit0";
		case Key_Digit1: return "Digit1";
		case Key_Digit2: return "Digit2";
		case Key_Digit3: return "Digit3";
		case Key_Digit4: return "Digit4";
		case Key_Digit5: return "Digit5";
		case Key_Digit6: return "Digit6";
		case Key_Digit7: return "Digit7";
		case Key_Digit8: return "Digit8";
		case Key_Digit9: return "Digit9";
		case Key_Numpad0: return "Numpad0";
		case Key_Numpad1: return "Numpad1";
		case Key_Numpad2: return "Numpad2";
		case Key_Numpad3: return "Numpad3";
		case Key_Numpad4: return "Numpad4";
		case Key_Numpad5: return "Numpad5";
		case Key_Numpad6: return "Numpad6";
		case Key_Numpad7: return "Numpad7";
		case Key_Numpad8: return "Numpad8";
		case Key_Numpad9: return "Numpad9";
		case Key_NumpadPeriod: return "NumpadPeriod";
		case Key_NumpadDivide: return "NumpadDivide";
		case Key_NumpadMultiply: return "NumpadMultiply";
		case Key_NumpadSubtract: return "NumpadSubtract";
		case Key_NumpadAdd: return "NumpadAdd";
		case Key_NumpadEnter: return "NumpadEnter";
		case Key_F1: return "F1";
		case Key_F2: return "F2";
		case Key_F3: return "F3";
		case Key_F4: return "F4";
		case Key_F5: return "F5";
		case Key_F6: return "F6";
		case Key_F7: return "F7";
		case Key_F8: return "F8";
		case Key_F9: return "F9";
		case Key_F10: return "F10";
		case Key_F11: return "F11";
		case Key_F12: return "F12";
		case Key_A: return "A";
		case Key_B: return "B";
		case Key_C: return "C";
		case Key_D: return "D";
		case Key_E: return "E";
		case Key_F: return "F";
		case Key_G: return "G";
		case Key_H: return "H";
		case Key_I: return "I";
		case Key_J: return "J";
		case Key_K: return "K";
		case Key_L: return "L";
		case Key_M: return "M";
		case Key_N: return "N";
		case Key_O: return "O";
		case Key_P: return "P";
		case Key_Q: return "Q";
		case Key_R: return "R";
		case Key_S: return "S";
		case Key_T: return "T";
		case Key_U: return "U";
		case Key_V: return "V";
		case Key_W: return "W";
		case Key_X: return "X";
		case Key_Y: return "Y";
		case Key_Z: return "Z";
		case Key_Tilde: return "Tilde";
		case Key_Minus: return "Minus";
		case Key_Plus: return "Plus";
		case Key_Comma: return "Comma";
		case Key_Period: return "Period";
		case Key_ForwardSlash: return "ForwardSlash";
		case Key_Colon: return "Colon";
		case Key_Quote: return "Quote";
		case Key_OpenBracket: return "OpenBracket";
		case Key_CloseBracket: return "CloseBracket";
		case Key_Pipe: return "Pipe";
		case Key_Escape: return "Escape";
		case Key_Enter: return "Enter";
		case Key_PrimaryEnter: return "PrimaryEnter";
		case Key_Backspace: return "Backspace";
		case Key_Space: return "Space";
		case Key_CapsLock: return "CapsLock";
		case Key_Tab: return "Tab";
		case Key_Up: return "Up";
		case Key_Down: return "Down";
		case Key_Left: return "Left";
		case Key_Right: return "Right";
		case Key_Insert: return "Insert";
		case Key_Delete: return "Delete";
		case Key_Home: return "Home";
		case Key_End: return "End";
		case Key_PageUp: return "PageUp";
		case Key_PageDown: return "PageDown";
		case Key_NumLock: return "NumLock";
		case Key_ScrollLock: return "ScrollLock";
		case Key_PrintScreen: return "PrintScreen";
		case Key_Pause: return "Pause";
		case Key_Shift: return "Shift";
		case Key_Control: return "Control";
		case Key_Alt: return "Alt";
		case Key_Windows: return "Windows";
		case Key_Command: return "Command";
		case Key_Option: return "Option";
		case Key_LeftShift: return "LeftShift";
		case Key_LeftControl: return "LeftControl";
		case Key_LeftAlt: return "LeftAlt";
		case Key_LeftWindows: return "LeftWindows";
		case Key_LeftCommand: return "LeftCommand";
		case Key_LeftOption: return "LeftOption";
		case Key_RightShift: return "RightShift";
		case Key_RightControl: return "RightControl";
		case Key_RightAlt: return "RightAlt";
		case Key_RightWindows: return "RightWindows";
		case Key_RightCommand: return "RightCommand";
		case Key_RightOption: return "RightOption";
		default: return UNKNOWN_STR;
	}
}

#endif

#endif //  _INPUT_KEYS_H

#if defined(_INPUT_BTN_STATE_H) && defined(_INPUT_KEYS_H)
#include "cross/cross_keys_and_btn_state.h"
#endif
