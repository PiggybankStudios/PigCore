/*
File:   cross_imgui_input_sokol_btn_state_and_vectors.h
Author: Taylor Robbins
Date:   02\21\2025
*/

#ifndef _CROSS_IMGUI_INPUT_SOKOL_BTN_STATE_AND_VECTORS_H
#define _CROSS_IMGUI_INPUT_SOKOL_BTN_STATE_AND_VECTORS_H

//NOTE: Intentionally no includes here

#if BUILD_WITH_IMGUI

#define IMGUI_MOUSE_SCROLL_SCALE 0.25f

typedef plex ImguiInput ImguiInput;
plex ImguiInput
{
	r32 elapsedMs;
	KeyboardState* keyboard;
	MouseState* mouse;
	bool isMouseOverOther;
	bool isWindowFocused;
	bool windowFocusedChanged;
	bool isTyping;
};

typedef plex ImguiOutput ImguiOutput;
plex ImguiOutput
{
	bool isMouseOverImgui;
	bool isImguiTypingFocused;
	#if BUILD_WITH_SOKOL_APP
	sapp_mouse_cursor cursorType;
	#endif
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void UpdateImguiInput(ImguiUI* imgui, const ImguiInput* input, ImguiOutput* output);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void UpdateImguiInput(ImguiUI* imgui, const ImguiInput* input, ImguiOutput* output)
{
	NotNull(imgui);
	NotNull(imgui->io);
	NotNull(input);
	NotNull(output);
	ClearPointer(output);
	
	imgui->io->DeltaTime = (input->elapsedMs / 1000.0f);
	
	ImGuiIO_AddMousePosEvent(imgui->io, input->mouse->position.X, input->mouse->position.Y);
	if (!input->isMouseOverOther)
	{
		if (input->mouse->scrollDelta.X != 0 || input->mouse->scrollDelta.Y != 0)
		{
			ImGuiIO_AddMouseWheelEvent(imgui->io, input->mouse->scrollDelta.X * IMGUI_MOUSE_SCROLL_SCALE, input->mouse->scrollDelta.Y * IMGUI_MOUSE_SCROLL_SCALE);
		}
		
		if (IsMouseBtnPressed(input->mouse, MouseBtn_Left)) { ImGuiIO_AddMouseButtonEvent(imgui->io, 0, true); }
		if (IsMouseBtnReleased(input->mouse, MouseBtn_Left)) { ImGuiIO_AddMouseButtonEvent(imgui->io, 0, false); }
		if (IsMouseBtnPressed(input->mouse, MouseBtn_Right)) { ImGuiIO_AddMouseButtonEvent(imgui->io, 1, true); }
		if (IsMouseBtnReleased(input->mouse, MouseBtn_Right)) { ImGuiIO_AddMouseButtonEvent(imgui->io, 1, false); }
		if (IsMouseBtnPressed(input->mouse, MouseBtn_Middle)) { ImGuiIO_AddMouseButtonEvent(imgui->io, 2, true); }
		if (IsMouseBtnReleased(input->mouse, MouseBtn_Middle)) { ImGuiIO_AddMouseButtonEvent(imgui->io, 2, false); }
	}
	
	if (!input->isTyping)
	{
		ImGuiIO_AddKeyEvent(imgui->io, ImGuiMod_Ctrl, IsKeyboardKeyDown(input->keyboard, Key_Control));
		ImGuiIO_AddKeyEvent(imgui->io, ImGuiMod_Alt, IsKeyboardKeyDown(input->keyboard, Key_Alt));
		ImGuiIO_AddKeyEvent(imgui->io, ImGuiMod_Shift, IsKeyboardKeyDown(input->keyboard, Key_Shift));
		
		for (uxx keyIndex = 0; keyIndex < Key_Count; keyIndex++)
		{
			Key key = (Key)keyIndex;
			ImGuiKey imKey = GetImGuiKey(key);
			if (imKey != ImGuiKey_None)
			{
				if (IsKeyboardKeyPressed(input->keyboard, key)) { ImGuiIO_AddKeyEvent(imgui->io, imKey, true); }
				if (IsKeyboardKeyReleased(input->keyboard, key)) { ImGuiIO_AddKeyEvent(imgui->io, imKey, false); }
			}
		}
		
		for (uxx cIndex = 0; cIndex < input->keyboard->numCharInputs; cIndex++)
		{
			const KeyboardCharInput* charInput = &input->keyboard->charInputs[cIndex];
			ImGuiIO_AddInputCharacter(imgui->io, charInput->codepoint);
		}
	}
	
	if (input->windowFocusedChanged) { ImGuiIO_AddFocusEvent(imgui->io, input->isWindowFocused); }
	
	#if BUILD_WITH_SOKOL_APP
	if (input->isMouseOverOther)
	{
		output->cursorType = SAPP_MOUSECURSOR_DEFAULT;
		switch (igGetMouseCursor())
		{
			case ImGuiMouseCursor_Arrow:      output->cursorType = SAPP_MOUSECURSOR_ARROW; break;
			case ImGuiMouseCursor_TextInput:  output->cursorType = SAPP_MOUSECURSOR_IBEAM; break;
			case ImGuiMouseCursor_ResizeAll:  output->cursorType = SAPP_MOUSECURSOR_RESIZE_ALL; break;
			case ImGuiMouseCursor_ResizeNS:   output->cursorType = SAPP_MOUSECURSOR_RESIZE_NS; break;
			case ImGuiMouseCursor_ResizeEW:   output->cursorType = SAPP_MOUSECURSOR_RESIZE_EW; break;
			case ImGuiMouseCursor_ResizeNESW: output->cursorType = SAPP_MOUSECURSOR_RESIZE_NESW; break;
			case ImGuiMouseCursor_ResizeNWSE: output->cursorType = SAPP_MOUSECURSOR_RESIZE_NWSE; break;
			case ImGuiMouseCursor_Hand:       output->cursorType = SAPP_MOUSECURSOR_POINTING_HAND; break;
			case ImGuiMouseCursor_NotAllowed: output->cursorType = SAPP_MOUSECURSOR_NOT_ALLOWED; break;
		}
	}
	#endif
	
	output->isMouseOverImgui = imgui->io->WantCaptureMouse;
	output->isImguiTypingFocused = imgui->io->WantTextInput;
	//TODO: How do we handle imgui->io->WantCaptureKeyboard?
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_IMGUI

#endif //  _CROSS_IMGUI_INPUT_SOKOL_BTN_STATE_AND_VECTORS_H
