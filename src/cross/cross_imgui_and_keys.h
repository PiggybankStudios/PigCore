/*
File:   cross_imgui_and_keys.h
Author: Taylor Robbins
Date:   02\21\2025
*/

#ifndef _CROSS_IMGUI_AND_KEYS_H
#define _CROSS_IMGUI_AND_KEYS_H

//NOTE: Intentionally no includes here

#if BUILD_WITH_IMGUI

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	ImGuiKey GetImGuiKey(Key key);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP ImGuiKey GetImGuiKey(Key key)
{
	switch (key)
	{
		case Key_A: return ImGuiKey_A;
		case Key_B: return ImGuiKey_B;
		case Key_C: return ImGuiKey_C;
		case Key_D: return ImGuiKey_D;
		case Key_E: return ImGuiKey_E;
		case Key_F: return ImGuiKey_F;
		case Key_G: return ImGuiKey_G;
		case Key_H: return ImGuiKey_H;
		case Key_I: return ImGuiKey_I;
		case Key_J: return ImGuiKey_J;
		case Key_K: return ImGuiKey_K;
		case Key_L: return ImGuiKey_L;
		case Key_M: return ImGuiKey_M;
		case Key_N: return ImGuiKey_N;
		case Key_O: return ImGuiKey_O;
		case Key_P: return ImGuiKey_P;
		case Key_Q: return ImGuiKey_Q;
		case Key_R: return ImGuiKey_R;
		case Key_S: return ImGuiKey_S;
		case Key_T: return ImGuiKey_T;
		case Key_U: return ImGuiKey_U;
		case Key_V: return ImGuiKey_V;
		case Key_W: return ImGuiKey_W;
		case Key_X: return ImGuiKey_X;
		case Key_Y: return ImGuiKey_Y;
		case Key_Z: return ImGuiKey_Z;
		
		case Key_0: return ImGuiKey_0;
		case Key_1: return ImGuiKey_1;
		case Key_2: return ImGuiKey_2;
		case Key_3: return ImGuiKey_3;
		case Key_4: return ImGuiKey_4;
		case Key_5: return ImGuiKey_5;
		case Key_6: return ImGuiKey_6;
		case Key_7: return ImGuiKey_7;
		case Key_8: return ImGuiKey_8;
		case Key_9: return ImGuiKey_9;
		
		case Key_Numpad0: return ImGuiKey_Keypad0;
		case Key_Numpad1: return ImGuiKey_Keypad1;
		case Key_Numpad2: return ImGuiKey_Keypad2;
		case Key_Numpad3: return ImGuiKey_Keypad3;
		case Key_Numpad4: return ImGuiKey_Keypad4;
		case Key_Numpad5: return ImGuiKey_Keypad5;
		case Key_Numpad6: return ImGuiKey_Keypad6;
		case Key_Numpad7: return ImGuiKey_Keypad7;
		case Key_Numpad8: return ImGuiKey_Keypad8;
		case Key_Numpad9: return ImGuiKey_Keypad9;
		
		case Key_NumpadPeriod: return ImGuiKey_KeypadDecimal;
		case Key_NumpadDivide: return ImGuiKey_KeypadDivide;
		case Key_NumpadMultiply: return ImGuiKey_KeypadMultiply;
		case Key_NumpadSubtract: return ImGuiKey_KeypadSubtract;
		case Key_NumpadAdd: return ImGuiKey_KeypadAdd;
		
		case Key_F1: return ImGuiKey_F1;
		case Key_F2: return ImGuiKey_F2;
		case Key_F3: return ImGuiKey_F3;
		case Key_F4: return ImGuiKey_F4;
		case Key_F5: return ImGuiKey_F5;
		case Key_F6: return ImGuiKey_F6;
		case Key_F7: return ImGuiKey_F7;
		case Key_F8: return ImGuiKey_F8;
		case Key_F9: return ImGuiKey_F9;
		case Key_F10: return ImGuiKey_F10;
		case Key_F11: return ImGuiKey_F11;
		case Key_F12: return ImGuiKey_F12;
		
		case Key_Enter: return ImGuiKey_Enter;
		case Key_Backspace: return ImGuiKey_Backspace;
		case Key_Escape: return ImGuiKey_Escape;
		case Key_Insert: return ImGuiKey_Insert;
		case Key_Delete: return ImGuiKey_Delete;
		case Key_Home: return ImGuiKey_Home;
		case Key_End: return ImGuiKey_End;
		case Key_PageUp: return ImGuiKey_PageUp;
		case Key_PageDown: return ImGuiKey_PageDown;
		case Key_Tab: return ImGuiKey_Tab;
		case Key_CapsLock: return ImGuiKey_CapsLock;
		case Key_NumLock: return ImGuiKey_NumLock;
		
		case Key_Control: return ImGuiKey_LeftCtrl;
		case Key_Alt: return ImGuiKey_LeftAlt;
		case Key_Shift: return ImGuiKey_LeftShift;
		
		case Key_Right: return ImGuiKey_RightArrow;
		case Key_Left: return ImGuiKey_LeftArrow;
		case Key_Up: return ImGuiKey_UpArrow;
		case Key_Down: return ImGuiKey_DownArrow;
		
		case Key_Plus: return ImGuiKey_Equal;
		case Key_Minus: return ImGuiKey_Minus;
		case Key_Pipe: return ImGuiKey_Backslash;
		case Key_OpenBracket: return ImGuiKey_LeftBracket;
		case Key_CloseBracket: return ImGuiKey_RightBracket;
		case Key_Colon: return ImGuiKey_Semicolon;
		case Key_Quote: return ImGuiKey_Apostrophe;
		case Key_Comma: return ImGuiKey_Comma;
		case Key_Period: return ImGuiKey_Period;
		case Key_ForwardSlash: return ImGuiKey_Slash;
		case Key_Tilde: return ImGuiKey_GraveAccent;
		case Key_Space: return ImGuiKey_Space;
		
		default: return ImGuiKey_None;
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_IMGUI

#endif //  _CROSS_IMGUI_AND_KEYS_H
