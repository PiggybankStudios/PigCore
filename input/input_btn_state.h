/*
File:   input_btn_state.h
Author: Taylor Robbins
Date:   02\03\2025
Description:
	** Contains the BtnState struct which helps us accumulate state changes for a button
	** that happens between frames into a single set of data that can be intrepreted
	** a variety of ways
*/

#ifndef _INPUT_BTN_STATE_H
#define _INPUT_BTN_STATE_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "std/std_memset.h"

typedef struct BtnState BtnState;
struct BtnState
{
	bool isDown;
	bool wasDown;
	bool wasPressed;
	bool wasReleased;
	u8 transitionCount;
	u64 lastTransitionTime;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void InitBtnState(BtnState* state, bool startedDown);
	PIG_CORE_INLINE void RefreshBtnState(BtnState* state);
	PIG_CORE_INLINE bool UpdateBtnState(BtnState* state, u64 currentTime, bool isDown);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void InitBtnState(BtnState* state, bool startedDown)
{
	NotNull(state);
	ClearPointer(state);
	state->isDown = startedDown;
	state->wasDown = state->isDown;
}

PEXPI void RefreshBtnState(BtnState* state)
{
	NotNull(state);
	state->wasDown = state->isDown;
	state->transitionCount = 0;
	state->wasPressed = false;
	state->wasReleased = false;
}

PEXPI bool UpdateBtnState(BtnState* state, u64 currentTime, bool isDown)
{
	NotNull(state);
	if (state->isDown != isDown)
	{
		IncrementU8(state->transitionCount);
		state->lastTransitionTime = currentTime;
		state->isDown = isDown;
		if (isDown) { state->wasPressed = true; }
		else { state->wasReleased = true; }
		return true;
	}
	else { return false; }
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _INPUT_BTN_STATE_H

#if defined(_INPUT_BTN_STATE_H) && defined(_INPUT_KEYS_H)
#include "cross/cross_keys_and_btn_state.h"
#endif

#if defined(_INPUT_BTN_STATE_H) && defined(_INPUT_MOUSE_BTNS_H) && defined(_STRUCT_VECTORS_H)
#include "cross/cross_mouse_btns_btn_state_and_vectors.h"
#endif

#if defined(_UI_IMGUI_H) && defined(_INPUT_SOKOL_H) && defined(_INPUT_BTN_STATE_H)
#include "cross/cross_imgui_input_sokol_and_btn_state.h"
#endif
