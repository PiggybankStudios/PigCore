/*
File:   std_angles.h
Author: Taylor Robbins
Date:   01\05\2025
Description:
	** Angles in math have this annoying problem where 2pi == 0 (repeating over a 2pi period)
	** but we often want to know if two angles are the same, how far apart two angles are,
	** which direction to go to get from one angle to another, etc. These problems all
	** revolve around checking and normalizing angle values to withing [0, 2pi) before
	** doing calculations. This file contains a bunch of standard library like functions
	** that do those sorts of proper angle related math.
*/

#ifndef _STD_ANGLES_H
#define _STD_ANGLES_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "std/std_basic_math.h"

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE r32 AngleFixR32(r32 angle);
	PIG_CORE_INLINE r64 AngleFixR64(r64 angle);
	PIG_CORE_INLINE r32 AngleDiffR32(r32 left, r32 right);
	PIG_CORE_INLINE r64 AngleDiffR64(r64 left, r64 right);
	PIG_CORE_INLINE r32 AngleOppositeR32(r32 angle);
	PIG_CORE_INLINE r64 AngleOppositeR64(r64 angle);
	PIG_CORE_INLINE r32 AngleFlipVerticalR32(r32 angle, bool normalize);
	PIG_CORE_INLINE r32 AngleFlipHorizontalR32(r32 angle, bool normalize);
	PIG_CORE_INLINE r64 AngleFlipVerticalR64(r64 angle, bool normalize);
	PIG_CORE_INLINE r64 AngleFlipHorizontalR64(r64 angle, bool normalize);
	PIG_CORE_INLINE r32 AngleLerpR32(r32 angleFrom, r32 angleTo, r32 amount);
	PIG_CORE_INLINE r64 AngleLerpR64(r64 angleFrom, r64 angleTo, r64 amount);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI r32 AngleFixR32(r32 angle)
{
	if (IsInfiniteOrNanR32(angle)) { return angle; }
	r32 result = angle;
	if (result >= TwoPi32) { result = ModR32(result, TwoPi32); }
	if (result < 0) { result = TwoPi32 - ModR32(-result, TwoPi32); }
	return result;
}
PEXPI r64 AngleFixR64(r64 angle)
{
	if (IsInfiniteOrNanR64(angle)) { return angle; }
	r64 result = angle;
	if (result >= TwoPi64) { result = ModR64(result, TwoPi64); }
	if (result < 0) { result = TwoPi64 - ModR64(-result, TwoPi64); }
	return result;
}

PEXPI r32 AngleDiffR32(r32 left, r32 right)
{
	r32 fixedLeft = AngleFixR32(left);
	r32 fixedRight = AngleFixR32(right);
	if (fixedLeft - fixedRight > Pi32) { fixedLeft -= TwoPi32; }
	if (fixedLeft - fixedRight < -Pi32) { fixedLeft += TwoPi32; }
	return fixedLeft - fixedRight;
}
PEXPI r64 AngleDiffR64(r64 left, r64 right)
{
	r64 fixedLeft = AngleFixR64(left);
	r64 fixedRight = AngleFixR64(right);
	if (fixedLeft - fixedRight > Pi64) { fixedLeft -= TwoPi64; }
	if (fixedLeft - fixedRight < -Pi64) { fixedLeft += TwoPi64; }
	return fixedLeft - fixedRight;
}

PEXPI r32 AngleOppositeR32(r32 angle) { return AngleFixR32(angle + Pi32); }
PEXPI r64 AngleOppositeR64(r64 angle) { return AngleFixR64(angle + Pi64); }

PEXPI r32 AngleFlipVerticalR32(r32 angle, bool normalize)
{
	r32 result = TwoPi32 - angle;
	if (normalize) { result = AngleFixR32(result); }
	return result;
}
PEXPI r32 AngleFlipHorizontalR32(r32 angle, bool normalize)
{
	r32 result = Pi32 - angle;
	if (normalize) { result = AngleFixR32(result); }
	return result;
}
PEXPI r64 AngleFlipVerticalR64(r64 angle, bool normalize)
{
	r64 result = TwoPi64 - angle;
	if (normalize) { result = AngleFixR64(result); }
	return result;
}
PEXPI r64 AngleFlipHorizontalR64(r64 angle, bool normalize)
{
	r64 result = Pi64 - angle;
	if (normalize) { result = AngleFixR64(result); }
	return result;
}

PEXPI r32 AngleLerpR32(r32 angleFrom, r32 angleTo, r32 amount)
{
	r32 from = AngleFixR32(angleFrom);
	r32 to = AngleFixR32(angleTo);
	if (to - from > Pi32) { to -= TwoPi32; }
	if (to - from < -Pi32) { to += TwoPi32; }
	return AngleFixR32(from + (to - from) * amount);
}
PEXPI r64 AngleLerpR64(r64 angleFrom, r64 angleTo, r64 amount)
{
	r64 from = AngleFixR64(angleFrom);
	r64 to = AngleFixR64(angleTo);
	if (to - from > Pi64) { to -= TwoPi64; }
	if (to - from < -Pi64) { to += TwoPi64; }
	return AngleFixR64(from + (to - from) * amount);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STD_ANGLES_H
