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
	** NOTE: See cross_keys_and_btn_state.h for KeyboardState structure which
	** converts input events related to the keyboard into various formats that
	** make it easy to write keyboard input handling code in an application.
	** NOTE: See HandleSokolKeyboardMouseAndTouchEvents in input_sokol.h
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
	ModifierKey_Any     = 0x10,
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
#endif //!PIG_CORE_IMPLEMENTATION

// NOTE: See CUSTOM_CODEPOINT_START in base_unicode.h
#define KEY_ESC_CODEPOINT             0xE001
#define KEY_F1_CODEPOINT              0xE002
#define KEY_F2_CODEPOINT              0xE003
#define KEY_F3_CODEPOINT              0xE004
#define KEY_F4_CODEPOINT              0xE005
#define KEY_F5_CODEPOINT              0xE006
#define KEY_F6_CODEPOINT              0xE007
#define KEY_F7_CODEPOINT              0xE008
#define KEY_F8_CODEPOINT              0xE009
#define KEY_F9_CODEPOINT              0xE00A
#define KEY_F10_CODEPOINT             0xE00B
#define KEY_F11_CODEPOINT             0xE00C
#define KEY_F12_CODEPOINT             0xE00D
#define KEY_SPACE_CODEPOINT           0xE00E
#define KEY_EMPTY_CODEPOINT           0xE00F
#define KEY_EMPTY_WIDE_CODEPOINT      0xE010

#define KEY_TILDE_CODEPOINT           0xE011
#define KEY_1_CODEPOINT               0xE012
#define KEY_2_CODEPOINT               0xE013
#define KEY_3_CODEPOINT               0xE014
#define KEY_4_CODEPOINT               0xE015
#define KEY_5_CODEPOINT               0xE016
#define KEY_6_CODEPOINT               0xE017
#define KEY_7_CODEPOINT               0xE018
#define KEY_8_CODEPOINT               0xE019
#define KEY_9_CODEPOINT               0xE01A
#define KEY_0_CODEPOINT               0xE01B
#define KEY_MINUS_CODEPOINT           0xE01C
#define KEY_PLUS_CODEPOINT            0xE01D
#define KEY_BACKSPACE_CODEPOINT       0xE01E
#define KEY_INSERT_CODEPOINT          0xE01F
#define KEY_DELETE_CODEPOINT          0xE020

#define KEY_TAB_CODEPOINT             0xE021
#define KEY_Q_CODEPOINT               0xE022
#define KEY_W_CODEPOINT               0xE023
#define KEY_E_CODEPOINT               0xE024
#define KEY_R_CODEPOINT               0xE025
#define KEY_T_CODEPOINT               0xE026
#define KEY_Y_CODEPOINT               0xE027
#define KEY_U_CODEPOINT               0xE028
#define KEY_I_CODEPOINT               0xE029
#define KEY_O_CODEPOINT               0xE02A
#define KEY_P_CODEPOINT               0xE02B
#define KEY_OPEN_BRACKET_CODEPOINT    0xE02C
#define KEY_CLOSE_BRACKET_CODEPOINT   0xE02D
#define KEY_PIPE_CODEPOINT            0xE02E
#define KEY_HOME_CODEPOINT            0xE02F
#define KEY_END_CODEPOINT             0xE030

#define KEY_CAPSLOCK_CODEPOINT        0xE031
#define KEY_A_CODEPOINT               0xE032
#define KEY_S_CODEPOINT               0xE033
#define KEY_D_CODEPOINT               0xE034
#define KEY_F_CODEPOINT               0xE035
#define KEY_G_CODEPOINT               0xE036
#define KEY_H_CODEPOINT               0xE037
#define KEY_J_CODEPOINT               0xE038
#define KEY_K_CODEPOINT               0xE039
#define KEY_L_CODEPOINT               0xE03A
#define KEY_COLON_CODEPOINT           0xE03B
#define KEY_QUOTE_CODEPOINT           0xE03C
#define KEY_UP_CODEPOINT              0xE03D
#define KEY_ENTER_CODEPOINT           0xE03E
#define KEY_PAGE_UP_CODEPOINT         0xE03F
#define KEY_PAGE_DOWN_CODEPOINT       0xE040

#define KEY_SHIFT_CODEPOINT           0xE041
#define KEY_Z_CODEPOINT               0xE042
#define KEY_X_CODEPOINT               0xE043
#define KEY_C_CODEPOINT               0xE044
#define KEY_V_CODEPOINT               0xE045
#define KEY_B_CODEPOINT               0xE046
#define KEY_N_CODEPOINT               0xE047
#define KEY_M_CODEPOINT               0xE048
#define KEY_COMMA_CODEPOINT           0xE049
#define KEY_PERIOD_CODEPOINT          0xE04A
#define KEY_FORWARD_SLASH_CODEPOINT   0xE04B
#define KEY_LEFT_CODEPOINT            0xE04C
#define KEY_DOWN_CODEPOINT            0xE04D
#define KEY_RIGHT_CODEPOINT           0xE04E
#define KEY_ALT_CODEPOINT             0xE04F
#define KEY_CTRL_CODEPOINT            0xE050

#define KEY_EMPTY_SQUARE1_CODEPOINT   0xE051
#define KEY_EMPTY_SQUARE2_CODEPOINT   0xE052
#define KEY_EMPTY_RECTANGLE_CODEPOINT 0xE053
#define KEY_MOUSE_CODEPOINT           0xE054
#define KEY_MOUSE_LEFT_CODEPOINT      0xE055
#define KEY_MOUSE_RIGHT_CODEPOINT     0xE056
#define KEY_MOUSE_MIDDLE_CODEPOINT    0xE057


#define KEY_ESC_CODEPOINT_STR             "\xEE\x80\x81" //0xE001
#define KEY_F1_CODEPOINT_STR              "\xEE\x80\x82" //0xE002
#define KEY_F2_CODEPOINT_STR              "\xEE\x80\x83" //0xE003
#define KEY_F3_CODEPOINT_STR              "\xEE\x80\x84" //0xE004
#define KEY_F4_CODEPOINT_STR              "\xEE\x80\x85" //0xE005
#define KEY_F5_CODEPOINT_STR              "\xEE\x80\x86" //0xE006
#define KEY_F6_CODEPOINT_STR              "\xEE\x80\x87" //0xE007
#define KEY_F7_CODEPOINT_STR              "\xEE\x80\x88" //0xE008
#define KEY_F8_CODEPOINT_STR              "\xEE\x80\x89" //0xE009
#define KEY_F9_CODEPOINT_STR              "\xEE\x80\x8A" //0xE00A
#define KEY_F10_CODEPOINT_STR             "\xEE\x80\x8B" //0xE00B
#define KEY_F11_CODEPOINT_STR             "\xEE\x80\x8C" //0xE00C
#define KEY_F12_CODEPOINT_STR             "\xEE\x80\x8D" //0xE00D
#define KEY_SPACE_CODEPOINT_STR           "\xEE\x80\x8E" //0xE00E
#define KEY_EMPTY_CODEPOINT_STR           "\xEE\x80\x8F" //0xE00F
#define KEY_EMPTY_WIDE_CODEPOINT_STR      "\xEE\x80\x90" //0xE010

#define KEY_TILDE_CODEPOINT_STR           "\xEE\x80\x91" //0xE011
#define KEY_1_CODEPOINT_STR               "\xEE\x80\x92" //0xE012
#define KEY_2_CODEPOINT_STR               "\xEE\x80\x93" //0xE013
#define KEY_3_CODEPOINT_STR               "\xEE\x80\x94" //0xE014
#define KEY_4_CODEPOINT_STR               "\xEE\x80\x95" //0xE015
#define KEY_5_CODEPOINT_STR               "\xEE\x80\x96" //0xE016
#define KEY_6_CODEPOINT_STR               "\xEE\x80\x97" //0xE017
#define KEY_7_CODEPOINT_STR               "\xEE\x80\x98" //0xE018
#define KEY_8_CODEPOINT_STR               "\xEE\x80\x99" //0xE019
#define KEY_9_CODEPOINT_STR               "\xEE\x80\x9A" //0xE01A
#define KEY_0_CODEPOINT_STR               "\xEE\x80\x9B" //0xE01B
#define KEY_MINUS_CODEPOINT_STR           "\xEE\x80\x9C" //0xE01C
#define KEY_PLUS_CODEPOINT_STR            "\xEE\x80\x9D" //0xE01D
#define KEY_BACKSPACE_CODEPOINT_STR       "\xEE\x80\x9E" //0xE01E
#define KEY_INSERT_CODEPOINT_STR          "\xEE\x80\x9F" //0xE01F
#define KEY_DELETE_CODEPOINT_STR          "\xEE\x80\xA0" //0xE020

#define KEY_TAB_CODEPOINT_STR             "\xEE\x80\xA1" //0xE021
#define KEY_Q_CODEPOINT_STR               "\xEE\x80\xA2" //0xE022
#define KEY_W_CODEPOINT_STR               "\xEE\x80\xA3" //0xE023
#define KEY_E_CODEPOINT_STR               "\xEE\x80\xA4" //0xE024
#define KEY_R_CODEPOINT_STR               "\xEE\x80\xA5" //0xE025
#define KEY_T_CODEPOINT_STR               "\xEE\x80\xA6" //0xE026
#define KEY_Y_CODEPOINT_STR               "\xEE\x80\xA7" //0xE027
#define KEY_U_CODEPOINT_STR               "\xEE\x80\xA8" //0xE028
#define KEY_I_CODEPOINT_STR               "\xEE\x80\xA9" //0xE029
#define KEY_O_CODEPOINT_STR               "\xEE\x80\xAA" //0xE02A
#define KEY_P_CODEPOINT_STR               "\xEE\x80\xAB" //0xE02B
#define KEY_OPEN_BRACKET_CODEPOINT_STR    "\xEE\x80\xAC" //0xE02C
#define KEY_CLOSE_BRACKET_CODEPOINT_STR   "\xEE\x80\xAD" //0xE02D
#define KEY_PIPE_CODEPOINT_STR            "\xEE\x80\xAE" //0xE02E
#define KEY_HOME_CODEPOINT_STR            "\xEE\x80\xAF" //0xE02F
#define KEY_END_CODEPOINT_STR             "\xEE\x80\xB0" //0xE030

#define KEY_CAPSLOCK_CODEPOINT_STR        "\xEE\x80\xB1" //0xE031
#define KEY_A_CODEPOINT_STR               "\xEE\x80\xB2" //0xE032
#define KEY_S_CODEPOINT_STR               "\xEE\x80\xB3" //0xE033
#define KEY_D_CODEPOINT_STR               "\xEE\x80\xB4" //0xE034
#define KEY_F_CODEPOINT_STR               "\xEE\x80\xB5" //0xE035
#define KEY_G_CODEPOINT_STR               "\xEE\x80\xB6" //0xE036
#define KEY_H_CODEPOINT_STR               "\xEE\x80\xB7" //0xE037
#define KEY_J_CODEPOINT_STR               "\xEE\x80\xB8" //0xE038
#define KEY_K_CODEPOINT_STR               "\xEE\x80\xB9" //0xE039
#define KEY_L_CODEPOINT_STR               "\xEE\x80\xBA" //0xE03A
#define KEY_COLON_CODEPOINT_STR           "\xEE\x80\xBB" //0xE03B
#define KEY_QUOTE_CODEPOINT_STR           "\xEE\x80\xBC" //0xE03C
#define KEY_UP_CODEPOINT_STR              "\xEE\x80\xBD" //0xE03D
#define KEY_ENTER_CODEPOINT_STR           "\xEE\x80\xBE" //0xE03E
#define KEY_PAGE_UP_CODEPOINT_STR         "\xEE\x80\xBF" //0xE03F
#define KEY_PAGE_DOWN_CODEPOINT_STR       "\xEE\x81\x80" //0xE040

#define KEY_SHIFT_CODEPOINT_STR           "\xEE\x81\x81" //0xE041
#define KEY_Z_CODEPOINT_STR               "\xEE\x81\x82" //0xE042
#define KEY_X_CODEPOINT_STR               "\xEE\x81\x83" //0xE043
#define KEY_C_CODEPOINT_STR               "\xEE\x81\x84" //0xE044
#define KEY_V_CODEPOINT_STR               "\xEE\x81\x85" //0xE045
#define KEY_B_CODEPOINT_STR               "\xEE\x81\x86" //0xE046
#define KEY_N_CODEPOINT_STR               "\xEE\x81\x87" //0xE047
#define KEY_M_CODEPOINT_STR               "\xEE\x81\x88" //0xE048
#define KEY_COMMA_CODEPOINT_STR           "\xEE\x81\x89" //0xE049
#define KEY_PERIOD_CODEPOINT_STR          "\xEE\x81\x8A" //0xE04A
#define KEY_FORWARD_SLASH_CODEPOINT_STR   "\xEE\x81\x8B" //0xE04B
#define KEY_LEFT_CODEPOINT_STR            "\xEE\x81\x8C" //0xE04C
#define KEY_DOWN_CODEPOINT_STR            "\xEE\x81\x8D" //0xE04D
#define KEY_RIGHT_CODEPOINT_STR           "\xEE\x81\x8E" //0xE04E
#define KEY_ALT_CODEPOINT_STR             "\xEE\x81\x8F" //0xE04F
#define KEY_CTRL_CODEPOINT_STR            "\xEE\x81\x90" //0xE050

#define KEY_EMPTY_SQUARE1_CODEPOINT_STR   "\xEE\x81\x91" //0xE051
#define KEY_EMPTY_SQUARE2_CODEPOINT_STR   "\xEE\x81\x92" //0xE052
#define KEY_EMPTY_RECTANGLE_CODEPOINT_STR "\xEE\x81\x93" //0xE053
#define KEY_MOUSE_CODEPOINT_STR           "\xEE\x81\x94" //0xE054
#define KEY_MOUSE_LEFT_CODEPOINT_STR      "\xEE\x81\x95" //0xE055
#define KEY_MOUSE_RIGHT_CODEPOINT_STR     "\xEE\x81\x96" //0xE056
#define KEY_MOUSE_MIDDLE_CODEPOINT_STR    "\xEE\x81\x97" //0xE057


#define KEY_FIRST_CODEPOINT   KEY_ESC_CODEPOINT
#define KEY_LAST_CODEPOINT    KEY_MOUSE_MIDDLE_CODEPOINT
#define KEY_CODEPOINT_COUNT   (KEY_LAST_CODEPOINT+1 - KEY_FIRST_CODEPOINT)

#if !PIG_CORE_IMPLEMENTATION
u32 GetCodepointForKey(Key key);
Key GetKeyForCodepoint(u32 codepoint);
const char* GetCodepointStrForKey(Key key);
#else
PEXP u32 GetCodepointForKey(Key key)
{
	switch (key)
	{
		case Key_Escape: return KEY_ESC_CODEPOINT;
		case Key_F1: return KEY_F1_CODEPOINT;
		case Key_F2: return KEY_F2_CODEPOINT;
		case Key_F3: return KEY_F3_CODEPOINT;
		case Key_F4: return KEY_F4_CODEPOINT;
		case Key_F5: return KEY_F5_CODEPOINT;
		case Key_F6: return KEY_F6_CODEPOINT;
		case Key_F7: return KEY_F7_CODEPOINT;
		case Key_F8: return KEY_F8_CODEPOINT;
		case Key_F9: return KEY_F9_CODEPOINT;
		case Key_F10: return KEY_F10_CODEPOINT;
		case Key_F11: return KEY_F11_CODEPOINT;
		case Key_F12: return KEY_F12_CODEPOINT;
		case Key_Space: return KEY_SPACE_CODEPOINT;
		// case Key_Empty: return KEY_EMPTY_CODEPOINT;
		// case Key_EmptyWide: return KEY_EMPTY_WIDE_CODEPOINT;
		case Key_Tilde: return KEY_TILDE_CODEPOINT;
		case Key_1: return KEY_1_CODEPOINT;
		case Key_2: return KEY_2_CODEPOINT;
		case Key_3: return KEY_3_CODEPOINT;
		case Key_4: return KEY_4_CODEPOINT;
		case Key_5: return KEY_5_CODEPOINT;
		case Key_6: return KEY_6_CODEPOINT;
		case Key_7: return KEY_7_CODEPOINT;
		case Key_8: return KEY_8_CODEPOINT;
		case Key_9: return KEY_9_CODEPOINT;
		case Key_0: return KEY_0_CODEPOINT;
		case Key_Minus: return KEY_MINUS_CODEPOINT;
		case Key_Plus: return KEY_PLUS_CODEPOINT;
		case Key_Backspace: return KEY_BACKSPACE_CODEPOINT;
		case Key_Insert: return KEY_INSERT_CODEPOINT;
		case Key_Delete: return KEY_DELETE_CODEPOINT;
		case Key_Tab: return KEY_TAB_CODEPOINT;
		case Key_Q: return KEY_Q_CODEPOINT;
		case Key_W: return KEY_W_CODEPOINT;
		case Key_E: return KEY_E_CODEPOINT;
		case Key_R: return KEY_R_CODEPOINT;
		case Key_T: return KEY_T_CODEPOINT;
		case Key_Y: return KEY_Y_CODEPOINT;
		case Key_U: return KEY_U_CODEPOINT;
		case Key_I: return KEY_I_CODEPOINT;
		case Key_O: return KEY_O_CODEPOINT;
		case Key_P: return KEY_P_CODEPOINT;
		case Key_OpenBracket: return KEY_OPEN_BRACKET_CODEPOINT;
		case Key_CloseBracket: return KEY_CLOSE_BRACKET_CODEPOINT;
		case Key_Pipe: return KEY_PIPE_CODEPOINT;
		case Key_Home: return KEY_HOME_CODEPOINT;
		case Key_End: return KEY_END_CODEPOINT;
		case Key_CapsLock: return KEY_CAPSLOCK_CODEPOINT;
		case Key_A: return KEY_A_CODEPOINT;
		case Key_S: return KEY_S_CODEPOINT;
		case Key_D: return KEY_D_CODEPOINT;
		case Key_F: return KEY_F_CODEPOINT;
		case Key_G: return KEY_G_CODEPOINT;
		case Key_H: return KEY_H_CODEPOINT;
		case Key_J: return KEY_J_CODEPOINT;
		case Key_K: return KEY_K_CODEPOINT;
		case Key_L: return KEY_L_CODEPOINT;
		case Key_Colon: return KEY_COLON_CODEPOINT;
		case Key_Quote: return KEY_QUOTE_CODEPOINT;
		case Key_Up: return KEY_UP_CODEPOINT;
		case Key_Enter: return KEY_ENTER_CODEPOINT;
		case Key_PageUp: return KEY_PAGE_UP_CODEPOINT;
		case Key_PageDown: return KEY_PAGE_DOWN_CODEPOINT;
		case Key_Shift: return KEY_SHIFT_CODEPOINT;
		case Key_Z: return KEY_Z_CODEPOINT;
		case Key_X: return KEY_X_CODEPOINT;
		case Key_C: return KEY_C_CODEPOINT;
		case Key_V: return KEY_V_CODEPOINT;
		case Key_B: return KEY_B_CODEPOINT;
		case Key_N: return KEY_N_CODEPOINT;
		case Key_M: return KEY_M_CODEPOINT;
		case Key_Comma: return KEY_COMMA_CODEPOINT;
		case Key_Period: return KEY_PERIOD_CODEPOINT;
		case Key_ForwardSlash: return KEY_FORWARD_SLASH_CODEPOINT;
		case Key_Left: return KEY_LEFT_CODEPOINT;
		case Key_Down: return KEY_DOWN_CODEPOINT;
		case Key_Right: return KEY_RIGHT_CODEPOINT;
		case Key_Alt: return KEY_ALT_CODEPOINT;
		case Key_Control: return KEY_CTRL_CODEPOINT;
		// case Key_EMPTY_SQUARE1: return KEY_EMPTY_SQUARE1_CODEPOINT;
		// case Key_EMPTY_SQUARE2: return KEY_EMPTY_SQUARE2_CODEPOINT;
		// case Key_EMPTY_RECTANGLE: return KEY_EMPTY_RECTANGLE_CODEPOINT;
		// case Key_MOUSE: return KEY_MOUSE_CODEPOINT;
		// case Key_MOUSE_LEFT: return KEY_MOUSE_LEFT_CODEPOINT;
		// case Key_MOUSE_RIGHT: return KEY_MOUSE_RIGHT_CODEPOINT;
		// case Key_MOUSE_MIDDLE: return KEY_MOUSE_MIDDLE_CODEPOINT;
		default: return 0;
	}
}

PEXP Key GetKeyForCodepoint(u32 codepoint)
{
	switch (codepoint)
	{
		case KEY_ESC_CODEPOINT: return Key_Escape;
		case KEY_F1_CODEPOINT: return Key_F1;
		case KEY_F2_CODEPOINT: return Key_F2;
		case KEY_F3_CODEPOINT: return Key_F3;
		case KEY_F4_CODEPOINT: return Key_F4;
		case KEY_F5_CODEPOINT: return Key_F5;
		case KEY_F6_CODEPOINT: return Key_F6;
		case KEY_F7_CODEPOINT: return Key_F7;
		case KEY_F8_CODEPOINT: return Key_F8;
		case KEY_F9_CODEPOINT: return Key_F9;
		case KEY_F10_CODEPOINT: return Key_F10;
		case KEY_F11_CODEPOINT: return Key_F11;
		case KEY_F12_CODEPOINT: return Key_F12;
		case KEY_SPACE_CODEPOINT: return Key_Space;
		// case KEY_EMPTY_CODEPOINT: return Key_Empty;
		// case KEY_EMPTY_WIDE_CODEPOINT: return Key_EmptyWide;
		case KEY_TILDE_CODEPOINT: return Key_Tilde;
		case KEY_1_CODEPOINT: return Key_1;
		case KEY_2_CODEPOINT: return Key_2;
		case KEY_3_CODEPOINT: return Key_3;
		case KEY_4_CODEPOINT: return Key_4;
		case KEY_5_CODEPOINT: return Key_5;
		case KEY_6_CODEPOINT: return Key_6;
		case KEY_7_CODEPOINT: return Key_7;
		case KEY_8_CODEPOINT: return Key_8;
		case KEY_9_CODEPOINT: return Key_9;
		case KEY_0_CODEPOINT: return Key_0;
		case KEY_MINUS_CODEPOINT: return Key_Minus;
		case KEY_PLUS_CODEPOINT: return Key_Plus;
		case KEY_BACKSPACE_CODEPOINT: return Key_Backspace;
		case KEY_INSERT_CODEPOINT: return Key_Insert;
		case KEY_DELETE_CODEPOINT: return Key_Delete;
		case KEY_TAB_CODEPOINT: return Key_Tab;
		case KEY_Q_CODEPOINT: return Key_Q;
		case KEY_W_CODEPOINT: return Key_W;
		case KEY_E_CODEPOINT: return Key_E;
		case KEY_R_CODEPOINT: return Key_R;
		case KEY_T_CODEPOINT: return Key_T;
		case KEY_Y_CODEPOINT: return Key_Y;
		case KEY_U_CODEPOINT: return Key_U;
		case KEY_I_CODEPOINT: return Key_I;
		case KEY_O_CODEPOINT: return Key_O;
		case KEY_P_CODEPOINT: return Key_P;
		case KEY_OPEN_BRACKET_CODEPOINT: return Key_OpenBracket;
		case KEY_CLOSE_BRACKET_CODEPOINT: return Key_CloseBracket;
		case KEY_PIPE_CODEPOINT: return Key_Pipe;
		case KEY_HOME_CODEPOINT: return Key_Home;
		case KEY_END_CODEPOINT: return Key_End;
		case KEY_CAPSLOCK_CODEPOINT: return Key_CapsLock;
		case KEY_A_CODEPOINT: return Key_A;
		case KEY_S_CODEPOINT: return Key_S;
		case KEY_D_CODEPOINT: return Key_D;
		case KEY_F_CODEPOINT: return Key_F;
		case KEY_G_CODEPOINT: return Key_G;
		case KEY_H_CODEPOINT: return Key_H;
		case KEY_J_CODEPOINT: return Key_J;
		case KEY_K_CODEPOINT: return Key_K;
		case KEY_L_CODEPOINT: return Key_L;
		case KEY_COLON_CODEPOINT: return Key_Colon;
		case KEY_QUOTE_CODEPOINT: return Key_Quote;
		case KEY_UP_CODEPOINT: return Key_Up;
		case KEY_ENTER_CODEPOINT: return Key_Enter;
		case KEY_PAGE_UP_CODEPOINT: return Key_PageUp;
		case KEY_PAGE_DOWN_CODEPOINT: return Key_PageDown;
		case KEY_SHIFT_CODEPOINT: return Key_Shift;
		case KEY_Z_CODEPOINT: return Key_Z;
		case KEY_X_CODEPOINT: return Key_X;
		case KEY_C_CODEPOINT: return Key_C;
		case KEY_V_CODEPOINT: return Key_V;
		case KEY_B_CODEPOINT: return Key_B;
		case KEY_N_CODEPOINT: return Key_N;
		case KEY_M_CODEPOINT: return Key_M;
		case KEY_COMMA_CODEPOINT: return Key_Comma;
		case KEY_PERIOD_CODEPOINT: return Key_Period;
		case KEY_FORWARD_SLASH_CODEPOINT: return Key_ForwardSlash;
		case KEY_LEFT_CODEPOINT: return Key_Left;
		case KEY_DOWN_CODEPOINT: return Key_Down;
		case KEY_RIGHT_CODEPOINT: return Key_Right;
		case KEY_ALT_CODEPOINT: return Key_Alt;
		case KEY_CTRL_CODEPOINT: return Key_Control;
		// case KEY_EMPTY_SQUARE1_CODEPOINT: return Key_EMPTY_SQUARE1;
		// case KEY_EMPTY_SQUARE2_CODEPOINT: return Key_EMPTY_SQUARE2;
		// case KEY_EMPTY_RECTANGLE_CODEPOINT: return Key_EMPTY_RECTANGLE;
		// case KEY_MOUSE_CODEPOINT: return Key_MOUSE;
		// case KEY_MOUSE_LEFT_CODEPOINT: return Key_MOUSE_LEFT;
		// case KEY_MOUSE_RIGHT_CODEPOINT: return Key_MOUSE_RIGHT;
		// case KEY_MOUSE_MIDDLE_CODEPOINT: return Key_MOUSE_MIDDLE;
		default: return Key_None;
	}
}

PEXP const char* GetCodepointStrForKey(Key key)
{
	switch (key)
	{
		case Key_Escape: return KEY_ESC_CODEPOINT_STR;
		case Key_F1: return KEY_F1_CODEPOINT_STR;
		case Key_F2: return KEY_F2_CODEPOINT_STR;
		case Key_F3: return KEY_F3_CODEPOINT_STR;
		case Key_F4: return KEY_F4_CODEPOINT_STR;
		case Key_F5: return KEY_F5_CODEPOINT_STR;
		case Key_F6: return KEY_F6_CODEPOINT_STR;
		case Key_F7: return KEY_F7_CODEPOINT_STR;
		case Key_F8: return KEY_F8_CODEPOINT_STR;
		case Key_F9: return KEY_F9_CODEPOINT_STR;
		case Key_F10: return KEY_F10_CODEPOINT_STR;
		case Key_F11: return KEY_F11_CODEPOINT_STR;
		case Key_F12: return KEY_F12_CODEPOINT_STR;
		case Key_Space: return KEY_SPACE_CODEPOINT_STR;
		// case Key_Empty: return KEY_EMPTY_CODEPOINT_STR;
		// case Key_EmptyWide: return KEY_EMPTY_WIDE_CODEPOINT_STR;
		case Key_Tilde: return KEY_TILDE_CODEPOINT_STR;
		case Key_1: return KEY_1_CODEPOINT_STR;
		case Key_2: return KEY_2_CODEPOINT_STR;
		case Key_3: return KEY_3_CODEPOINT_STR;
		case Key_4: return KEY_4_CODEPOINT_STR;
		case Key_5: return KEY_5_CODEPOINT_STR;
		case Key_6: return KEY_6_CODEPOINT_STR;
		case Key_7: return KEY_7_CODEPOINT_STR;
		case Key_8: return KEY_8_CODEPOINT_STR;
		case Key_9: return KEY_9_CODEPOINT_STR;
		case Key_0: return KEY_0_CODEPOINT_STR;
		case Key_Minus: return KEY_MINUS_CODEPOINT_STR;
		case Key_Plus: return KEY_PLUS_CODEPOINT_STR;
		case Key_Backspace: return KEY_BACKSPACE_CODEPOINT_STR;
		case Key_Insert: return KEY_INSERT_CODEPOINT_STR;
		case Key_Delete: return KEY_DELETE_CODEPOINT_STR;
		case Key_Tab: return KEY_TAB_CODEPOINT_STR;
		case Key_Q: return KEY_Q_CODEPOINT_STR;
		case Key_W: return KEY_W_CODEPOINT_STR;
		case Key_E: return KEY_E_CODEPOINT_STR;
		case Key_R: return KEY_R_CODEPOINT_STR;
		case Key_T: return KEY_T_CODEPOINT_STR;
		case Key_Y: return KEY_Y_CODEPOINT_STR;
		case Key_U: return KEY_U_CODEPOINT_STR;
		case Key_I: return KEY_I_CODEPOINT_STR;
		case Key_O: return KEY_O_CODEPOINT_STR;
		case Key_P: return KEY_P_CODEPOINT_STR;
		case Key_OpenBracket: return KEY_OPEN_BRACKET_CODEPOINT_STR;
		case Key_CloseBracket: return KEY_CLOSE_BRACKET_CODEPOINT_STR;
		case Key_Pipe: return KEY_PIPE_CODEPOINT_STR;
		case Key_Home: return KEY_HOME_CODEPOINT_STR;
		case Key_End: return KEY_END_CODEPOINT_STR;
		case Key_CapsLock: return KEY_CAPSLOCK_CODEPOINT_STR;
		case Key_A: return KEY_A_CODEPOINT_STR;
		case Key_S: return KEY_S_CODEPOINT_STR;
		case Key_D: return KEY_D_CODEPOINT_STR;
		case Key_F: return KEY_F_CODEPOINT_STR;
		case Key_G: return KEY_G_CODEPOINT_STR;
		case Key_H: return KEY_H_CODEPOINT_STR;
		case Key_J: return KEY_J_CODEPOINT_STR;
		case Key_K: return KEY_K_CODEPOINT_STR;
		case Key_L: return KEY_L_CODEPOINT_STR;
		case Key_Colon: return KEY_COLON_CODEPOINT_STR;
		case Key_Quote: return KEY_QUOTE_CODEPOINT_STR;
		case Key_Up: return KEY_UP_CODEPOINT_STR;
		case Key_Enter: return KEY_ENTER_CODEPOINT_STR;
		case Key_PageUp: return KEY_PAGE_UP_CODEPOINT_STR;
		case Key_PageDown: return KEY_PAGE_DOWN_CODEPOINT_STR;
		case Key_Shift: return KEY_SHIFT_CODEPOINT_STR;
		case Key_Z: return KEY_Z_CODEPOINT_STR;
		case Key_X: return KEY_X_CODEPOINT_STR;
		case Key_C: return KEY_C_CODEPOINT_STR;
		case Key_V: return KEY_V_CODEPOINT_STR;
		case Key_B: return KEY_B_CODEPOINT_STR;
		case Key_N: return KEY_N_CODEPOINT_STR;
		case Key_M: return KEY_M_CODEPOINT_STR;
		case Key_Comma: return KEY_COMMA_CODEPOINT_STR;
		case Key_Period: return KEY_PERIOD_CODEPOINT_STR;
		case Key_ForwardSlash: return KEY_FORWARD_SLASH_CODEPOINT_STR;
		case Key_Left: return KEY_LEFT_CODEPOINT_STR;
		case Key_Down: return KEY_DOWN_CODEPOINT_STR;
		case Key_Right: return KEY_RIGHT_CODEPOINT_STR;
		case Key_Alt: return KEY_ALT_CODEPOINT_STR;
		case Key_Control: return KEY_CTRL_CODEPOINT_STR;
		// case Key_EMPTY_SQUARE1: return KEY_EMPTY_SQUARE1_CODEPOINT_STR;
		// case Key_EMPTY_SQUARE2: return KEY_EMPTY_SQUARE2_CODEPOINT_STR;
		// case Key_EMPTY_RECTANGLE: return KEY_EMPTY_RECTANGLE_CODEPOINT_STR;
		// case Key_MOUSE: return KEY_MOUSE_CODEPOINT_STR;
		// case Key_MOUSE_LEFT: return KEY_MOUSE_LEFT_CODEPOINT_STR;
		// case Key_MOUSE_RIGHT: return KEY_MOUSE_RIGHT_CODEPOINT_STR;
		// case Key_MOUSE_MIDDLE: return KEY_MOUSE_MIDDLE_CODEPOINT_STR;
		default: return nullptr;
	}
}
#endif //!PIG_CORE_IMPLEMENTATION

#endif //  _INPUT_KEYS_H

#if defined(_UI_IMGUI_H) && defined(_INPUT_KEYS_H)
#include "cross/cross_imgui_and_keys.h"
#endif
