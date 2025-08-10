/*
File:   cross_mouse_btns_btn_state_and_vectors.h
Author: Taylor Robbins
Date:   02\03\2025
*/

#ifndef _CROSS_MOUSE_BTNS_BTN_STATE_AND_VECTORS_H
#define _CROSS_MOUSE_BTNS_BTN_STATE_AND_VECTORS_H

typedef plex MouseState MouseState;
plex MouseState
{
	v2 position;
	v2 prevPosition;
	v2 lockedPosDelta;
	u64 lastMoveTime;
	bool isOverWindow;
	bool wasOverWindow;
	bool isLocked;
	v2 scrollValue;
	v2 scrollDelta;
	BtnState btns[MouseBtn_Count];
	v2 clickStartPositions[MouseBtn_Count];
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void InitMouseState(MouseState* mouse);
	PIG_CORE_INLINE void RefreshMouseState(MouseState* mouse, bool isMouseLocked, v2 lockedMouseAbsolutePosition);
	PIG_CORE_INLINE void UpdateMousePosition(MouseState* mouse, u64 currentTime, v2 newPosition);
	PIG_CORE_INLINE void UpdateMouseLockedDelta(MouseState* mouse, u64 currentTime, v2 lockedPosDelta);
	PIG_CORE_INLINE void UpdateMouseScroll(MouseState* mouse, u64 currentTime, v2 scrollDelta);
	PIG_CORE_INLINE void UpdateMouseOverWindow(MouseState* mouse, u64 currentTime, bool isOverWindow);
	PIG_CORE_INLINE void UpdateMouseBtn(MouseState* mouse, u64 currentTime, MouseBtn btn, bool pressed);
	PIG_CORE_INLINE bool IsMouseBtnDown(MouseState* mouse, MouseBtn btn);
	PIG_CORE_INLINE bool IsMouseBtnUp(MouseState* mouse, MouseBtn btn);
	PIG_CORE_INLINE bool IsMouseBtnPressed(MouseState* mouse, MouseBtn btn);
	PIG_CORE_INLINE bool IsMouseBtnReleased(MouseState* mouse, MouseBtn btn);
	PIG_CORE_INLINE bool IsMouseBtnPressedRepeating(MouseState* mouse, u64 prevTime, u64 currentTime, MouseBtn btn, u64 repeatDelay, u64 repeatPeriod);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

//TODO: Should we somehow query the initial state of the all the keys during this init process?
PEXPI void InitMouseState(MouseState* mouse)
{
	NotNull(mouse);
	ClearPointer(mouse);
	for (uxx btnIndex = 0; btnIndex < MouseBtn_Count; btnIndex++)
	{
		InitBtnState(&mouse->btns[btnIndex], false);
	}
}

PEXPI void RefreshMouseState(MouseState* mouse, bool isMouseLocked, v2 lockedMouseAbsolutePosition)
{
	NotNull(mouse);
	mouse->isLocked = isMouseLocked;
	if (isMouseLocked) { mouse->position = lockedMouseAbsolutePosition; }
	mouse->prevPosition = mouse->position;
	mouse->lockedPosDelta = V2_Zero;
	mouse->scrollDelta = V2_Zero;
	mouse->wasOverWindow = mouse->isOverWindow;
	for (uxx btnIndex = 0; btnIndex < MouseBtn_Count; btnIndex++)
	{
		RefreshBtnState(&mouse->btns[btnIndex]);
	}
}

PEXPI void UpdateMousePosition(MouseState* mouse, u64 currentTime, v2 newPosition)
{
	NotNull(mouse);
	if (!AreEqualV2(mouse->position, newPosition))
	{
		mouse->position = newPosition;
		mouse->lastMoveTime = currentTime;
	}
}
PEXPI void UpdateMouseLockedDelta(MouseState* mouse, u64 currentTime, v2 lockedPosDelta)
{
	NotNull(mouse);
	if (!AreEqualV2(lockedPosDelta, V2_Zero))
	{
		mouse->lockedPosDelta = Add(mouse->lockedPosDelta, lockedPosDelta);
		mouse->lastMoveTime = currentTime;
	}
}

PEXPI void UpdateMouseScroll(MouseState* mouse, u64 currentTime, v2 scrollDelta)
{
	UNUSED(currentTime);
	NotNull(mouse);
	if (!AreEqualV2(scrollDelta, V2_Zero))
	{
		mouse->scrollDelta = Add(mouse->scrollDelta, scrollDelta);
		mouse->scrollValue = Add(mouse->scrollValue, scrollDelta);
	}
}

PEXPI void UpdateMouseOverWindow(MouseState* mouse, u64 currentTime, bool isOverWindow)
{
	UNUSED(currentTime);
	NotNull(mouse);
	if (mouse->isOverWindow != isOverWindow)
	{
		mouse->isOverWindow = isOverWindow;
	}
}

PEXPI void UpdateMouseBtn(MouseState* mouse, u64 currentTime, MouseBtn btn, bool pressed)
{
	NotNull(mouse);
	Assert(btn < MouseBtn_Count);
	if (UpdateBtnState(&mouse->btns[btn], currentTime, pressed, false))
	{
		mouse->clickStartPositions[btn] = mouse->position;
	}
}

PEXPI bool IsMouseBtnDown(MouseState* mouse, MouseBtn btn)
{
	NotNull(mouse);
	Assert(btn < MouseBtn_Count);
	return mouse->btns[btn].isDown;
}
PEXPI bool IsMouseBtnUp(MouseState* mouse, MouseBtn btn)
{
	NotNull(mouse);
	Assert(btn < MouseBtn_Count);
	return !mouse->btns[btn].isDown;
}
PEXPI bool IsMouseBtnPressed(MouseState* mouse, MouseBtn btn)
{
	NotNull(mouse);
	Assert(btn < MouseBtn_Count);
	return mouse->btns[btn].wasPressed;
}
PEXPI bool IsMouseBtnReleased(MouseState* mouse, MouseBtn btn)
{
	NotNull(mouse);
	Assert(btn < MouseBtn_Count);
	return mouse->btns[btn].wasReleased;
}
PEXPI bool IsMouseBtnPressedRepeating(MouseState* mouse, u64 prevTime, u64 currentTime, MouseBtn btn, u64 repeatDelay, u64 repeatPeriod)
{
	NotNull(mouse);
	Assert(btn < MouseBtn_Count);
	if (mouse->btns[btn].isDown)
	{
		u64 prevTimeHeld = TimeSinceBy(prevTime, mouse->btns[btn].lastTransitionTime);
		u64 currentTimeHeld = TimeSinceBy(currentTime, mouse->btns[btn].lastTransitionTime);
		if (currentTimeHeld >= repeatDelay)
		{
			if (prevTimeHeld < repeatDelay) { return true; }
			else { return (((prevTimeHeld - repeatDelay) / repeatPeriod) != ((currentTimeHeld - repeatDelay) / repeatPeriod)); }
		}
	}
	return false;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_MOUSE_BTNS_BTN_STATE_AND_VECTORS_H
