/*
File:   cross_keys_and_btn_state.h
Author: Taylor Robbins
Date:   02\03\2025
*/

#ifndef _CROSS_KEYS_AND_BTN_STATE_H
#define _CROSS_KEYS_AND_BTN_STATE_H

typedef struct KeyboardState KeyboardState;
struct KeyboardState
{
	BtnState keys[Key_Count];
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void InitKeyboardState(KeyboardState* keyboard);
	PIG_CORE_INLINE void RefreshKeyboardState(KeyboardState* keyboard);
	PIG_CORE_INLINE void UpdateKeyboardKey(KeyboardState* keyboard, u64 currentTime, Key key, bool pressed);
	PIG_CORE_INLINE bool IsKeyboardKeyDown(KeyboardState* keyboard, Key key);
	PIG_CORE_INLINE bool IsKeyboardKeyUp(KeyboardState* keyboard, Key key);
	PIG_CORE_INLINE bool IsKeyboardKeyPressed(KeyboardState* keyboard, Key key);
	PIG_CORE_INLINE bool IsKeyboardKeyReleased(KeyboardState* keyboard, Key key);
	PIG_CORE_INLINE bool IsKeyboardKeyPressedRepeating(KeyboardState* keyboard, u64 prevTime, u64 currentTime, Key key, u64 repeatDelay, u64 repeatPeriod);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

//TODO: Should we somehow query the initial state of the all the keys during this init process?
PEXPI void InitKeyboardState(KeyboardState* keyboard)
{
	NotNull(keyboard);
	ClearPointer(keyboard);
	for (uxx keyIndex = 0; keyIndex < Key_Count; keyIndex++)
	{
		InitBtnState(&keyboard->keys[keyIndex], false);
	}
}

PEXPI void RefreshKeyboardState(KeyboardState* keyboard)
{
	NotNull(keyboard);
	for (uxx keyIndex = 0; keyIndex < Key_Count; keyIndex++)
	{
		RefreshBtnState(&keyboard->keys[keyIndex]);
	}
}

PEXPI void UpdateKeyboardKey(KeyboardState* keyboard, u64 currentTime, Key key, bool pressed)
{
	NotNull(keyboard);
	Assert(key < Key_Count);
	UpdateBtnState(&keyboard->keys[key], currentTime, pressed);
}

PEXPI bool IsKeyboardKeyDown(KeyboardState* keyboard, Key key)
{
	NotNull(keyboard);
	Assert(key < Key_Count);
	return keyboard->keys[key].isDown;
}
PEXPI bool IsKeyboardKeyUp(KeyboardState* keyboard, Key key)
{
	NotNull(keyboard);
	Assert(key < Key_Count);
	return !keyboard->keys[key].isDown;
}
PEXPI bool IsKeyboardKeyPressed(KeyboardState* keyboard, Key key)
{
	NotNull(keyboard);
	Assert(key < Key_Count);
	return keyboard->keys[key].wasPressed;
}
PEXPI bool IsKeyboardKeyReleased(KeyboardState* keyboard, Key key)
{
	NotNull(keyboard);
	Assert(key < Key_Count);
	return keyboard->keys[key].wasReleased;
}
PEXPI bool IsKeyboardKeyPressedRepeating(KeyboardState* keyboard, u64 prevTime, u64 currentTime, Key key, u64 repeatDelay, u64 repeatPeriod)
{
	NotNull(keyboard);
	Assert(key < Key_Count);
	if (keyboard->keys[key].isDown)
	{
		u64 prevTimeHeld = TimeSinceBy(prevTime, keyboard->keys[key].lastTransitionTime);
		u64 currentTimeHeld = TimeSinceBy(currentTime, keyboard->keys[key].lastTransitionTime);
		if (currentTimeHeld >= repeatDelay)
		{
			if (prevTimeHeld < repeatDelay) { return true; }
			else { return (((prevTimeHeld - repeatDelay) / repeatPeriod) != ((currentTimeHeld - repeatDelay) / repeatPeriod)); }
		}
	}
	return false;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_KEYS_AND_BTN_STATE_H
