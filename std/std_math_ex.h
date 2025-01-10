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

r32 SawR32(r32 angle)
{
	r32 x = (angle-Pi32/2) / (Pi32);
	i32 section = FloorR32i(x);
	r32 offset = x - (r32)section;
	if ((section%2) == 0) { return -1 + (offset * 2); }
	else { return 1 - (offset * 2); }
}
r64 SawR64(r64 angle)
{
	r64 x = (angle-Pi64/2) / (Pi64);
	i64 section = FloorR64i(x);
	r64 offset = x - (r64)section;
	if ((section%2) == 0) { return -1 + (offset * 2); }
	else { return 1 - (offset * 2); }
}

r32 LerpClampR32(r32 val1, r32 val2, r32 amount) { return ClampR32(val1 + (val2 - val1) * amount, MinR32(val1, val2), MaxR32(val1, val2)); }
r64 LerpClampR64(r64 val1, r64 val2, r64 amount) { return ClampR64(val1 + (val2 - val1) * amount, MinR64(val1, val2), MaxR64(val1, val2)); }

r32 InverseLerpR32(r32 start, r32 end, r32 value) { return (value - start) / (end - start); }
r64 InverseLerpR64(r64 start, r64 end, r64 value) { return (value - start) / (end - start); }

r32 InverseLerpClampR32(r32 start, r32 end, r32 value) { return ClampR32((value - start) / (end - start), 0.0f, 1.0f); }
r64 InverseLerpClampR64(r64 start, r64 end, r64 value) { return ClampR64((value - start) / (end - start), 0.0f, 1.0f); }

//TODO: Double check this gives me the correct value!
r32 FractionalPartR32(r32 value) { return AbsR32(ModR32(value, 1.0f)); }
r64 FractionalPartR64(r64 value) { return AbsR64(ModR64(value, 1.0)); }

// TODO: Use ClampCast in these conversions!
i8 ConvertSampleR64ToI8(r64 sample) { return (i8)RoundR64i(sample * INT8_MAX); }
i16 ConvertSampleR64ToI16(r64 sample) { return (i16)RoundR64i(sample * INT16_MAX); }
i32 ConvertSampleR64ToI32(r64 sample) { return (i32)RoundR64i(sample * INT32_MAX); }
r64 ConvertSampleI8ToR64(i8 sampleI8) { return ((r64)sampleI8 / (r64)INT8_MAX); }
r64 ConvertSampleI16ToR64(i16 sampleI16) { return ((r64)sampleI16 / (r64)INT16_MAX); }
r64 ConvertSampleI32ToR64(i32 sampleI32) { return ((r64)sampleI32 / (r64)INT32_MAX); }

//NOTE: A fixed tolerance is never a perfect solution. But it's usually good enough for our purposes when a float value is within a "reasonable" range
#define DEFAULT_R32_TOLERANCE 0.001f
#define DEFAULT_R64_TOLERANCE 0.001 //TODO: Choose a different tolerance for R64!

bool BasicallyEqualR32(r32 value1, r32 value2, r32 tolerance) { return (AbsR32(value1 - value2) <= tolerance); }
bool BasicallyEqualR64(r64 value1, r64 value2, r64 tolerance) { return (AbsR64(value1 - value2) <= tolerance); }

bool BasicallyGreaterThanR32(r32 value1, r32 value2, r32 tolerance) { return ((value1 > value2) || BasicallyEqualR32(value1, value2, tolerance)); }
bool BasicallyGreaterThanR64(r64 value1, r64 value2, r64 tolerance) { return ((value1 > value2) || BasicallyEqualR64(value1, value2, tolerance)); }

bool BasicallyLessThanR32(r32 value1, r32 value2, r32 tolerance) { return ((value1 < value2) || BasicallyEqualR32(value1, value2, tolerance)); }
bool BasicallyLessThanR64(r64 value1, r64 value2, r64 tolerance) { return ((value1 < value2) || BasicallyEqualR64(value1, value2, tolerance)); }

bool BasicallyBetweenR32(r32 value, r32 min, r32 max, r32 tolerance) { return ((value > min && value < max) || BasicallyEqualR32(value, min, tolerance) || BasicallyEqualR32(value, max, tolerance)); }
bool BasicallyBetweenR64(r64 value, r64 min, r64 max, r64 tolerance) { return ((value > min && value < max) || BasicallyEqualR64(value, min, tolerance) || BasicallyEqualR64(value, max, tolerance)); }

#endif //  _STD_MATH_EX_H
