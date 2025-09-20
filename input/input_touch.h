/*
File:   input_touch.h
Author: Taylor Robbins
Date:   09\19\2025
*/

#ifndef _INPUT_TOUCH_H
#define _INPUT_TOUCH_H

#define MAX_TOUCH_INPUTS 16 //maximum number of simultaneous touch inputs supported
#define TOUCH_ID_INVALID 0 //we use id=0 to indicate the TouchState is empty

typedef plex TouchState TouchState;
plex TouchState
{
	uxx id;
	v2 startPos;
	bool started;
	u64 startTime;
	bool stopped;
	u64 stopTime;
	bool moved;
	u64 moveTime;
	v2 prevPos;
	v2 pos;
	r32 visitRadius;
	rec visitBounds;
};

typedef plex TouchscreenState TouchscreenState;
plex TouchscreenState
{
	uxx nextTouchId;
	uxx numTouches;
	TouchState touches[MAX_TOUCH_INPUTS];
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void InitTouchscreenState(TouchscreenState* touchscreen);
	PIG_CORE_INLINE TouchState* FindTouchById(TouchscreenState* touchscreen, uxx id);
	TouchState* StartNewTouch(TouchscreenState* touchscreen, uxx id, v2 startPos, u64 currentTime);
	void UpdateTouchStatePosition(TouchState* touch, v2 position, u64 currentTime);
	PIG_CORE_INLINE void RefreshTouchscreenState(TouchscreenState* touchscreen);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void InitTouchscreenState(TouchscreenState* touchscreen)
{
	NotNull(touchscreen);
	ClearPointer(touchscreen);
	touchscreen->nextTouchId = 1;
	for (uxx tIndex = 0; tIndex < MAX_TOUCH_INPUTS; tIndex++) { touchscreen->touches[tIndex].id = TOUCH_ID_INVALID; }
}

PEXPI TouchState* FindTouchById(TouchscreenState* touchscreen, uxx id)
{
	DebugAssert(id != TOUCH_ID_INVALID);
	if (id == TOUCH_ID_INVALID) { return nullptr; }
	for (uxx tIndex = 0; tIndex < MAX_TOUCH_INPUTS; tIndex++)
	{
		TouchState* touch = &touchscreen->touches[tIndex];
		if (touch->id == id) { return touch; }
	}
	return nullptr;
}
PEXP TouchState* StartNewTouch(TouchscreenState* touchscreen, uxx id, v2 startPos, u64 currentTime)
{
	DebugAssert(id != TOUCH_ID_INVALID);
	if (id == TOUCH_ID_INVALID) { return nullptr; }
	TouchState* result = nullptr;
	// PrintLine_D("Requested new touch %llu", id);
	for (uxx tIndex = 0; tIndex < MAX_TOUCH_INPUTS; tIndex++)
	{
		TouchState* touch = &touchscreen->touches[tIndex];
		DebugAssertMsg(touch->id != id, "Duplicate touch ID added to TouchscreenState");
		if (touch->id == TOUCH_ID_INVALID && result == nullptr)
		{
			// PrintLine_D("Using slot [%llu] for Touch %llu", tIndex, id);
			result = touch;
			result->id = id;
			result->startTime = currentTime;
			result->started = true;
			result->startPos = startPos;
			result->stopped = false;
			result->stopTime = 0;
			result->moved = false;
			result->moveTime = currentTime;
			result->prevPos = startPos;
			result->pos = startPos;
			result->visitRadius = 0.0f;
			result->visitBounds = NewRec(startPos.X, startPos.Y, 0, 0);
		}
	}
	return result;
}

PEXP void UpdateTouchStatePosition(TouchState* touch, v2 position, u64 currentTime)
{
	NotNull(touch);
	touch->visitRadius = MaxR32(touch->visitRadius, LengthV2(SubV2(position, touch->startPos)));
	touch->visitBounds = BothRec(touch->visitBounds, NewRecV(position, V2_Zero));
	if (!AreEqualV2(touch->pos, position))
	{
		touch->pos = position;
		touch->moveTime = currentTime;
		touch->moved = true;
	}
}

PEXPI void RefreshTouchscreenState(TouchscreenState* touchscreen)
{
	for (uxx tIndex = 0; tIndex < MAX_TOUCH_INPUTS; tIndex++)
	{
		TouchState* touch = &touchscreen->touches[tIndex];
		if (touch->id != TOUCH_ID_INVALID)
		{
			if (touch->stopped) { touch->id = TOUCH_ID_INVALID; continue; }
			touch->moved = false;
			touch->started = false;
			touch->stopped = false;
			touch->prevPos = touch->pos;
		}
	}
	
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _INPUT_TOUCH_H
