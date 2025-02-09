/*
File:   std_math_ex.h
Author: Taylor Robbins
Date:   01\03\2025
Description:
	** This file contains mathematical functions in the style of something that may be
	** provided by the standard library but is not
*/

#ifndef _STD_MATH_EX_H
#define _STD_MATH_EX_H

#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"
#include "base/base_math.h"
#include "std/std_basic_math.h"
#include "std/std_trig.h"

//NOTE: A fixed tolerance is never a perfect solution. But it's usually good enough for our purposes when a float value is within a "reasonable" range
#define DEFAULT_R32_TOLERANCE 0.001f
#define DEFAULT_R64_TOLERANCE 0.001 //TODO: Choose a different tolerance for R64!

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE r32 SawR32(r32 angle);
	PIG_CORE_INLINE r64 SawR64(r64 angle);
	PIG_CORE_INLINE r32 LerpClampR32(r32 val1, r32 val2, r32 amount);
	PIG_CORE_INLINE r64 LerpClampR64(r64 val1, r64 val2, r64 amount);
	PIG_CORE_INLINE r32 InverseLerpR32(r32 start, r32 end, r32 value);
	PIG_CORE_INLINE r64 InverseLerpR64(r64 start, r64 end, r64 value);
	PIG_CORE_INLINE r32 InverseLerpClampR32(r32 start, r32 end, r32 value);
	PIG_CORE_INLINE r64 InverseLerpClampR64(r64 start, r64 end, r64 value);
	PIG_CORE_INLINE r32 FractionalPartR32(r32 value);
	PIG_CORE_INLINE r64 FractionalPartR64(r64 value);
	PIG_CORE_INLINE i8 ConvertSampleR64ToI8(r64 sample);
	PIG_CORE_INLINE i16 ConvertSampleR64ToI16(r64 sample);
	PIG_CORE_INLINE i32 ConvertSampleR64ToI32(r64 sample);
	PIG_CORE_INLINE r64 ConvertSampleI8ToR64(i8 sampleI8);
	PIG_CORE_INLINE r64 ConvertSampleI16ToR64(i16 sampleI16);
	PIG_CORE_INLINE r64 ConvertSampleI32ToR64(i32 sampleI32);
	PIG_CORE_INLINE bool AreSimilarR32(r32 value1, r32 value2, r32 tolerance);
	PIG_CORE_INLINE bool AreSimilarR64(r64 value1, r64 value2, r64 tolerance);
	PIG_CORE_INLINE bool AreSimilarOrGreaterR32(r32 value1, r32 value2, r32 tolerance);
	PIG_CORE_INLINE bool AreSimilarOrGreaterR64(r64 value1, r64 value2, r64 tolerance);
	PIG_CORE_INLINE bool AreSimilarOrLessR32(r32 value1, r32 value2, r32 tolerance);
	PIG_CORE_INLINE bool AreSimilarOrLessR64(r64 value1, r64 value2, r64 tolerance);
	PIG_CORE_INLINE bool AreSimilarBetweenR32(r32 value, r32 min, r32 max, r32 tolerance);
	PIG_CORE_INLINE bool AreSimilarBetweenR64(r64 value, r64 min, r64 max, r64 tolerance);
	PIG_CORE_INLINE r32 OscillateBy(u64 timeSource, r32 min, r32 max, u64 periodMs, u64 offset);
	PIG_CORE_INLINE r32 OscillateSawBy(u64 timeSource, r32 min, r32 max, u64 periodMs, u64 offset);
	PIG_CORE_INLINE r32 AnimateBy(u64 timeSource, r32 min, r32 max, u64 periodMs, u64 offset);
	PIG_CORE_INLINE r32 AnimateAndPauseBy(u64 timeSource, r32 min, r32 max, u64 animationTime, u64 pauseTime, u64 offset);
	PIG_CORE_INLINE u64 AnimateByU64(u64 timeSource, u64 min, u64 max, u64 periodMs, u64 offset);
	PIG_CORE_INLINE u64 AnimateAndPauseByU64(u64 timeSource, u64 min, u64 max, u64 animationTime, u64 pauseTime, u64 offset, bool useFirstFrameForPause);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI r32 SawR32(r32 angle)
{
	r32 x = (angle-Pi32/2) / (Pi32);
	i32 section = FloorR32i(x);
	r32 offset = x - (r32)section;
	if ((section%2) == 0) { return -1 + (offset * 2); }
	else { return 1 - (offset * 2); }
}
PEXPI r64 SawR64(r64 angle)
{
	r64 x = (angle-Pi64/2) / (Pi64);
	i64 section = FloorR64i(x);
	r64 offset = x - (r64)section;
	if ((section%2) == 0) { return -1 + (offset * 2); }
	else { return 1 - (offset * 2); }
}

PEXPI r32 LerpClampR32(r32 val1, r32 val2, r32 amount) { return ClampR32(val1 + (val2 - val1) * amount, MinR32(val1, val2), MaxR32(val1, val2)); }
PEXPI r64 LerpClampR64(r64 val1, r64 val2, r64 amount) { return ClampR64(val1 + (val2 - val1) * amount, MinR64(val1, val2), MaxR64(val1, val2)); }

PEXPI r32 InverseLerpR32(r32 start, r32 end, r32 value) { return (value - start) / (end - start); }
PEXPI r64 InverseLerpR64(r64 start, r64 end, r64 value) { return (value - start) / (end - start); }

PEXPI r32 InverseLerpClampR32(r32 start, r32 end, r32 value) { return ClampR32((value - start) / (end - start), 0.0f, 1.0f); }
PEXPI r64 InverseLerpClampR64(r64 start, r64 end, r64 value) { return ClampR64((value - start) / (end - start), 0.0f, 1.0f); }

//TODO: Double check this gives me the correct value!
PEXPI r32 FractionalPartR32(r32 value) { return AbsR32(ModR32(value, 1.0f)); }
PEXPI r64 FractionalPartR64(r64 value) { return AbsR64(ModR64(value, 1.0)); }

// +--------------------------------------------------------------+
// |              Audio Sample Conversion Functions               |
// +--------------------------------------------------------------+
// TODO: Use ClampCast in these conversions!
PEXPI i8 ConvertSampleR64ToI8(r64 sample) { return (i8)RoundR64i(sample * INT8_MAX); }
PEXPI i16 ConvertSampleR64ToI16(r64 sample) { return (i16)RoundR64i(sample * INT16_MAX); }
PEXPI i32 ConvertSampleR64ToI32(r64 sample) { return (i32)RoundR64i(sample * INT32_MAX); }
PEXPI r64 ConvertSampleI8ToR64(i8 sampleI8) { return ((r64)sampleI8 / (r64)INT8_MAX); }
PEXPI r64 ConvertSampleI16ToR64(i16 sampleI16) { return ((r64)sampleI16 / (r64)INT16_MAX); }
PEXPI r64 ConvertSampleI32ToR64(i32 sampleI32) { return ((r64)sampleI32 / (r64)INT32_MAX); }

// +--------------------------------------------------------------+
// |                  Float Comparison Functions                  |
// +--------------------------------------------------------------+
PEXPI bool AreSimilarR32(r32 value1, r32 value2, r32 tolerance) { return (AbsR32(value1 - value2) <= tolerance); }
PEXPI bool AreSimilarR64(r64 value1, r64 value2, r64 tolerance) { return (AbsR64(value1 - value2) <= tolerance); }

PEXPI bool AreSimilarOrGreaterR32(r32 value1, r32 value2, r32 tolerance) { return ((value1 > value2) || AreSimilarR32(value1, value2, tolerance)); }
PEXPI bool AreSimilarOrGreaterR64(r64 value1, r64 value2, r64 tolerance) { return ((value1 > value2) || AreSimilarR64(value1, value2, tolerance)); }

PEXPI bool AreSimilarOrLessR32(r32 value1, r32 value2, r32 tolerance) { return ((value1 < value2) || AreSimilarR32(value1, value2, tolerance)); }
PEXPI bool AreSimilarOrLessR64(r64 value1, r64 value2, r64 tolerance) { return ((value1 < value2) || AreSimilarR64(value1, value2, tolerance)); }

PEXPI bool AreSimilarBetweenR32(r32 value, r32 min, r32 max, r32 tolerance) { return ((value > min && value < max) || AreSimilarR32(value, min, tolerance) || AreSimilarR32(value, max, tolerance)); }
PEXPI bool AreSimilarBetweenR64(r64 value, r64 min, r64 max, r64 tolerance) { return ((value > min && value < max) || AreSimilarR64(value, min, tolerance) || AreSimilarR64(value, max, tolerance)); }

// +--------------------------------------------------------------+
// |               Oscillate and Animate Functions                |
// +--------------------------------------------------------------+
PEXPI r32 OscillateBy(u64 timeSource, r32 min, r32 max, u64 periodMs, u64 offset)
{
	r32 lerpValue = (SinR32((((timeSource + offset) % periodMs) / (r32)periodMs) * 2*Pi32) + 1.0f) / 2.0f;
	return min + (max - min) * lerpValue;
}
PEXPI r32 OscillateSawBy(u64 timeSource, r32 min, r32 max, u64 periodMs, u64 offset)
{
	r32 lerpValue = (SawR32((((timeSource + offset) % periodMs) / (r32)periodMs) * 2*Pi32) + 1.0f) / 2.0f;
	return min + (max - min) * lerpValue;
}

PEXPI r32 AnimateBy(u64 timeSource, r32 min, r32 max, u64 periodMs, u64 offset)
{
	r32 lerpValue = ((timeSource + offset) % periodMs) / (r32)periodMs;
	return min + (max - min) * lerpValue;
}
PEXPI r32 AnimateAndPauseBy(u64 timeSource, r32 min, r32 max, u64 animationTime, u64 pauseTime, u64 offset)
{
	u64 lerpIntValue = (u64)((timeSource + offset) % (animationTime + pauseTime));
	if (lerpIntValue >= animationTime) { lerpIntValue = animationTime; }
	return min + (max - min) * (lerpIntValue / (r32)animationTime);
}

PEXPI u64 AnimateByU64(u64 timeSource, u64 min, u64 max, u64 periodMs, u64 offset)
{
	Assert(periodMs > 0);
	if (min == max) { return min; }
	bool reversed = false;
	if (max < min) { SwapVariables(u64, min, max); reversed = true; }
	u64 bucketTime = periodMs / (max-min);
	u64 inLoopTime = ((timeSource + offset) % periodMs);
	if (reversed) { inLoopTime = periodMs-1 - inLoopTime; }
	u64 result = ClampU64(inLoopTime / bucketTime, min, max-1);
	return result;
}
PEXPI u64 AnimateAndPauseByU64(u64 timeSource, u64 min, u64 max, u64 animationTime, u64 pauseTime, u64 offset, bool useFirstFrameForPause)
{
	Assert(animationTime > 0);
	if (min == max) { return min; }
	bool reversed = false;
	if (max < min) { SwapVariables(u64, min, max); reversed = true; }
	u64 bucketTime = animationTime / (max - min);
	u64 inLoopTime = ((timeSource + offset) % (animationTime + pauseTime));
	if (inLoopTime > animationTime) { inLoopTime = (useFirstFrameForPause ? 0 : animationTime-1); }
	if (reversed) { inLoopTime = animationTime-1 - inLoopTime; }
	u64 result = ClampU64(inLoopTime / bucketTime, min, max-1);
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STD_MATH_EX_H

#if defined(_MISC_EASING_H) && defined(_STD_MATH_EX_H)
#include "cross/cross_easing_and_math_ex.h"
#endif
