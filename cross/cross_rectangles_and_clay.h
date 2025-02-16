/*
File:   cross_rectangles_and_clay.h
Author: Taylor Robbins
Date:   02\16\2025
*/

#ifndef _CROSS_RECTANGLES_AND_CLAY_H
#define _CROSS_RECTANGLES_AND_CLAY_H

//NOTE: Intentionally no includes here

#if BUILD_WITH_CLAY

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE rec ToRecFromClay(Clay_BoundingBox clayBoundingBox);
	PIG_CORE_INLINE Clay_BoundingBox ToClayBoundingBox(rec rectangle);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI rec ToRecFromClay(Clay_BoundingBox clayBoundingBox) { return NewRec(clayBoundingBox.x, clayBoundingBox.y, clayBoundingBox.width, clayBoundingBox.height); }
PEXPI Clay_BoundingBox ToClayBoundingBox(rec rectangle) { return (Clay_BoundingBox){ .x = rectangle.X, .y = rectangle.Y, .width = rectangle.Width, .height = rectangle.Height }; }

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_CLAY

#endif //  _CROSS_RECTANGLES_AND_CLAY_H
