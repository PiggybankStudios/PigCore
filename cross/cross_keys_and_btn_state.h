/*
File:   cross_keys_and_btn_state.h
Author: Taylor Robbins
Date:   02\03\2025
NOTE: "Handling" is a simple system for reducing binding conflicts.
      If two bits of code rely on the same input key (Escape for example) then the first bit of code
      will run and set the input as "handled" which will prevent the second bit of code from running.
      This is a simple system, there are no priority options, it's just whoever handles the input first.
*/

#ifndef _CROSS_KEYS_AND_BTN_STATE_H
#define _CROSS_KEYS_AND_BTN_STATE_H

#define KEYBOARD_MAX_CHAR_INPUTS_PER_FRAME 256

typedef plex KeyboardCharInput KeyboardCharInput;
plex KeyboardCharInput
{
	u32 codepoint;
	u8 modifierKeys;
};

typedef plex KeyboardState KeyboardState;
plex KeyboardState
{
	BtnState keys[Key_Count];
	uxx numCharInputs;
	KeyboardCharInput charInputs[KEYBOARD_MAX_CHAR_INPUTS_PER_FRAME];
};

typedef plex KeyboardStateHandling KeyboardStateHandling;
plex KeyboardStateHandling
{
	bool keyHandled[Key_Count];
	bool keyHandledUntilReleased[Key_Count];
	bool charInputHandled[KEYBOARD_MAX_CHAR_INPUTS_PER_FRAME];
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void InitKeyboardState(KeyboardState* keyboard);
	PIG_CORE_INLINE void RefreshKeyboardState(KeyboardState* keyboard);
	PIG_CORE_INLINE void UpdateKeyboardKey(KeyboardState* keyboard, u64 currentTime, Key key, bool pressed, bool isRepeat);
	PIG_CORE_INLINE void AddKeyboardCharInput(KeyboardState* keyboard, u32 codepoint, u8 modifierKeys);
	PIG_CORE_INLINE void RefreshKeyboardStateHandling(const KeyboardState* keyboard, KeyboardStateHandling* handling);
	PIG_CORE_INLINE bool IsKeyboardKeyDown(KeyboardState* keyboard, KeyboardStateHandling* handling, Key key);
	PIG_CORE_INLINE bool IsKeyboardKeyUp(KeyboardState* keyboard, KeyboardStateHandling* handling, Key key);
	PIG_CORE_INLINE bool IsKeyboardKeyPressed(KeyboardState* keyboard, KeyboardStateHandling* handling, Key key, bool followOsRepeatedEvent);
	PIG_CORE_INLINE bool IsKeyboardKeyReleased(KeyboardState* keyboard, KeyboardStateHandling* handling, Key key);
	PIG_CORE_INLINE bool IsKeyboardKeyPressedRepeating(KeyboardState* keyboard, KeyboardStateHandling* handling, u64 prevTime, u64 currentTime, Key key, u64 repeatDelay, u64 repeatPeriod);
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
	keyboard->numCharInputs = 0;
}

PEXPI void UpdateKeyboardKey(KeyboardState* keyboard, u64 currentTime, Key key, bool pressed, bool isRepeat)
{
	NotNull(keyboard);
	Assert(key < Key_Count);
	UpdateBtnState(&keyboard->keys[key], currentTime, pressed, isRepeat);
}

PEXPI void AddKeyboardCharInput(KeyboardState* keyboard, u32 codepoint, u8 modifierKeys)
{
	if (keyboard->numCharInputs < KEYBOARD_MAX_CHAR_INPUTS_PER_FRAME)
	{
		KeyboardCharInput* charInput = &keyboard->charInputs[keyboard->numCharInputs];
		charInput->codepoint = codepoint;
		charInput->modifierKeys = modifierKeys;
		keyboard->numCharInputs++;
	}
}

PEXPI void RefreshKeyboardStateHandling(const KeyboardState* keyboard, KeyboardStateHandling* handling)
{
	NotNull(keyboard);
	NotNull(handling);
	for (uxx keyIndex = 0; keyIndex < Key_Count; keyIndex++)
	{
		handling->keyHandled[keyIndex] = false;
		if (handling->keyHandledUntilReleased[keyIndex])
		{
			if (keyboard->keys[keyIndex].isDown || keyboard->keys[keyIndex].wasReleased) { handling->keyHandled[keyIndex] = true; }
			if (!keyboard->keys[keyIndex].isDown) { handling->keyHandledUntilReleased[keyIndex] = false; }
		}
	}
	for (uxx cIndex = 0; cIndex < KEYBOARD_MAX_CHAR_INPUTS_PER_FRAME; cIndex++)
	{
		handling->charInputHandled[cIndex] = false;
	}
}

PEXPI bool IsKeyboardKeyDown(KeyboardState* keyboard, KeyboardStateHandling* handling, Key key)
{
	NotNull(keyboard);
	Assert(key < Key_Count);
	if (handling != nullptr && handling->keyHandled[key]) { return false; }
	bool result = keyboard->keys[key].isDown;
	if (result && handling != nullptr) { handling->keyHandled[key] = true; }
	return result;
}
PEXPI bool IsKeyboardKeyUp(KeyboardState* keyboard, KeyboardStateHandling* handling, Key key)
{
	NotNull(keyboard);
	Assert(key < Key_Count);
	if (handling != nullptr && handling->keyHandled[key]) { return false; }
	bool result = !keyboard->keys[key].isDown;
	if (result && handling != nullptr) { handling->keyHandled[key] = true; }
	return result;
}
PEXPI bool IsKeyboardKeyPressed(KeyboardState* keyboard, KeyboardStateHandling* handling, Key key, bool followOsRepeatedEvent)
{
	NotNull(keyboard);
	Assert(key < Key_Count);
	if (handling != nullptr && handling->keyHandled[key]) { return false; }
	bool result = keyboard->keys[key].wasPressed || (followOsRepeatedEvent && keyboard->keys[key].wasRepeated);
	if (result && handling != nullptr) { handling->keyHandled[key] = true; }
	return result;
}
PEXPI bool IsKeyboardKeyReleased(KeyboardState* keyboard, KeyboardStateHandling* handling, Key key)
{
	NotNull(keyboard);
	Assert(key < Key_Count);
	if (handling != nullptr && handling->keyHandled[key]) { return false; }
	bool result = keyboard->keys[key].wasReleased;
	if (result && handling != nullptr) { handling->keyHandled[key] = true; }
	return result;
}
PEXPI bool IsKeyboardKeyPressedRepeating(KeyboardState* keyboard, KeyboardStateHandling* handling, u64 prevTime, u64 currentTime, Key key, u64 repeatDelay, u64 repeatPeriod)
{
	NotNull(keyboard);
	Assert(key < Key_Count);
	if (handling != nullptr && handling->keyHandled[key]) { return false; }
	if (keyboard->keys[key].isDown)
	{
		u64 prevTimeHeld = TimeSinceBy(prevTime, keyboard->keys[key].lastTransitionTime);
		u64 currentTimeHeld = TimeSinceBy(currentTime, keyboard->keys[key].lastTransitionTime);
		if (currentTimeHeld >= repeatDelay)
		{
			if (prevTimeHeld < repeatDelay)
			{
				if (handling != nullptr) { handling->keyHandled[key] = true; }
				return true;
			}
			else
			{
				bool isRepeating = (((prevTimeHeld - repeatDelay) / repeatPeriod) != ((currentTimeHeld - repeatDelay) / repeatPeriod));
				if (isRepeating && handling != nullptr) { handling->keyHandled[key] = true; }
				return isRepeating;
			}
		}
	}
	return false;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_KEYS_AND_BTN_STATE_H
