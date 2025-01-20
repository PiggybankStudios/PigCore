/*
File:   cross_easing_and_math_ex.h
Author: Taylor Robbins
Date:   01\15\2025
*/

#ifndef _CROSS_EASING_AND_MATH_EX_H
#define _CROSS_EASING_AND_MATH_EX_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE r32 OscillatePhaseBy(u64 timeSource, r32 min, r32 max, u64 periodMs, u64 offset);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI r32 OscillatePhaseBy(u64 timeSource, r32 min, r32 max, u64 periodMs, u64 offset)
{
	r32 lerpValue = (SawR32((((timeSource + offset) % periodMs) / (r32)periodMs) * 2*Pi32) + 1.0f) / 2.0f;
	lerpValue = Ease(EasingStyle_CubicOut, lerpValue);
	return min + (max - min) * lerpValue;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_EASING_AND_MATH_EX_H
