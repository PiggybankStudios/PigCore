/*
File:   input_touch.h
Author: Taylor Robbins
Date:   09\19\2025
Description:
	** TouchscreenState holds a number of TouchState slots (up to MAX_TOUCH_INPUTS)
	** which can be filled or not (id=TOUCH_ID_INVALID means unfilled)
	** Each filled slot represents an active touch (or one that just ended on this frame)
	** Each frame we update information inside each active touch based on input events
	** collecting information into formats that make it easier to write input handling code.
	** We don't save the entire position history of a touch but we do save visitRadius and
	** visitBounds and we save up to TOUCH_PATH_LENGTH most recent positions (deduplicating
	** when the touch stays at an exact position for more than one frame)
	** For gesture recognition the application should implement a way to track the entire path
	** of the touch, including when each position occurred so velocity can be calculated
	** NOTE: See HandleSokolKeyboardMouseAndTouchEvents in input_sokol.h
*/

#ifndef _INPUT_TOUCH_H
#define _INPUT_TOUCH_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "base/base_macros.h"
#include "std/std_memset.h"
#include "struct/struct_vectors.h"
#include "struct/struct_rectangles.h"

#define MAX_TOUCH_INPUTS 16 //maximum number of simultaneous touch inputs supported
#define TOUCH_ID_INVALID 0 //we use id=0 to indicate the TouchState is empty
#define TOUCH_PATH_LENGTH 16 //positions
#define TOUCH_PATH_INVALID NewV2(INFINITY, INFINITY)

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
	v2 path[TOUCH_PATH_LENGTH];
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
	TouchState* result = nullptr;
	for (uxx tIndex = 0; tIndex < MAX_TOUCH_INPUTS; tIndex++)
	{
		TouchState* touch = &touchscreen->touches[tIndex];
		DebugAssertMsg(touch->id != id, "Duplicate touch ID added to TouchscreenState");
		if (touch->id == TOUCH_ID_INVALID && result == nullptr)
		{
			touchscreen->numTouches++;
			result = touch;
			result->id = (id != TOUCH_ID_INVALID) ? id : touchscreen->nextTouchId;
			if (id == TOUCH_ID_INVALID) { touchscreen->nextTouchId++; }
			else if (touchscreen->nextTouchId <= id) { touchscreen->nextTouchId = id+1; }
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
			for (uxx pIndex = 1; pIndex < TOUCH_PATH_LENGTH; pIndex++) { result->path[pIndex] = TOUCH_PATH_INVALID; }
			result->path[0] = startPos;
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
		MyMemMove(&touch->path[1], &touch->path[0], sizeof(v2) * (TOUCH_PATH_LENGTH-1));
		touch->path[0] = position;
	}
}

PEXPI void RefreshTouchscreenState(TouchscreenState* touchscreen)
{
	for (uxx tIndex = 0; tIndex < MAX_TOUCH_INPUTS; tIndex++)
	{
		TouchState* touch = &touchscreen->touches[tIndex];
		if (touch->id != TOUCH_ID_INVALID)
		{
			if (touch->stopped)
			{
				touch->id = TOUCH_ID_INVALID;
				touchscreen->numTouches--;
				continue;
			}
			touch->moved = false;
			touch->started = false;
			touch->stopped = false;
			touch->prevPos = touch->pos;
		}
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _INPUT_TOUCH_H
