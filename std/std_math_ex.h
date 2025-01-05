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
#include "std/std_basic_math.h"
#include "std/std_trig.h"

i32 CeilDivI32(i32 dividend, i32 divisor)
{
	if ((dividend % divisor) == 0) { return dividend / divisor; }
	else { return (dividend / divisor) + 1; }
}
i64 CeilDivI64(i64 dividend, i64 divisor)
{
	if ((dividend % divisor) == 0) { return dividend / divisor; }
	else { return (dividend / divisor) + 1; }
}
ixx CeilDivIXX(ixx dividend, ixx divisor)
{
	#if TARGET_IS_32BIT
	return CeilDivI32(dividend, divisor);
	#else
	return CeilDivI64(dividend, divisor);
	#endif
}
u32 CeilDivU32(u32 dividend, u32 divisor)
{
	if ((dividend % divisor) == 0) { return dividend / divisor; }
	else { return (dividend / divisor) + 1; }
}
u64 CeilDivU64(u64 dividend, u64 divisor)
{
	if ((dividend % divisor) == 0) { return dividend / divisor; }
	else { return (dividend / divisor) + 1; }
}
uxx CeilDivUXX(uxx dividend, uxx divisor)
{
	#if TARGET_IS_32BIT
	return CeilDivU32(dividend, divisor);
	#else
	return CeilDivU64(dividend, divisor);
	#endif
}

i32 CeilToI32(i32 value, i32 chunkSize) { return ((value % chunkSize) == 0) ? value : (value + (chunkSize - (value%chunkSize))); }
i64 CeilToI64(i64 value, i64 chunkSize) { return ((value % chunkSize) == 0) ? value : (value + (chunkSize - (value%chunkSize))); }
u32 CeilToU32(u32 value, u32 chunkSize) { return ((value % chunkSize) == 0) ? value : (value + (chunkSize - (value%chunkSize))); }
u64 CeilToU64(u64 value, u64 chunkSize) { return ((value % chunkSize) == 0) ? value : (value + (chunkSize - (value%chunkSize))); }

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

i8  SignOfI8(i8   value) { return (value < 0) ? -1 : ((value > 0) ? 1 : 0); }
i16 SignOfI16(i16 value) { return (value < 0) ? -1 : ((value > 0) ? 1 : 0); }
i32 SignOfI32(i32 value) { return (value < 0) ? -1 : ((value > 0) ? 1 : 0); }
i64 SignOfI64(i64 value) { return (value < 0) ? -1 : ((value > 0) ? 1 : 0); }
r32 SignOfR32(r32 value) { return (value < 0.0f) ? -1.0f : ((value > 0.0f) ? 1.0f : 0.0f); }
r64 SignOfR64(r64 value) { return (value < 0.0) ? -1.0 : ((value > 0.0) ? 1.0 : 0.0); }

i8  ClampI8(i8   value, i8  min, i8  max) { return (value < min) ? min : ((value > max) ? max : value); }
i16 ClampI16(i16 value, i16 min, i16 max) { return (value < min) ? min : ((value > max) ? max : value); }
i32 ClampI32(i32 value, i32 min, i32 max) { return (value < min) ? min : ((value > max) ? max : value); }
i64 ClampI64(i64 value, i64 min, i64 max) { return (value < min) ? min : ((value > max) ? max : value); }
r32 ClampR32(r32 value, r32 min, r32 max) { return (value < min) ? min : ((value > max) ? max : value); }
r64 ClampR64(r64 value, r64 min, r64 max) { return (value < min) ? min : ((value > max) ? max : value); }

r32 LerpR32(r32 val1, r32 val2, r32 amount) { return (val1 + (val2 - val1) * amount); }
r64 LerpR64(r64 val1, r64 val2, r64 amount) { return (val1 + (val2 - val1) * amount); }

r32 LerpClampR32(r32 val1, r32 val2, r32 amount) { return ClampR32(val1 + (val2 - val1) * amount, MinR32(val1, val2), MaxR32(val1, val2)); }
r64 LerpClampR64(r64 val1, r64 val2, r64 amount) { return ClampR64(val1 + (val2 - val1) * amount, MinR64(val1, val2), MaxR64(val1, val2)); }

r32 InverseLerpR32(r32 start, r32 end, r32 value) { return (value - start) / (end - start); }
r64 InverseLerpR64(r64 start, r64 end, r64 value) { return (value - start) / (end - start); }

r32 InverseLerpClampR32(r32 start, r32 end, r32 value) { return ClampR32((value - start) / (end - start), 0.0f, 1.0f); }
r64 InverseLerpClampR64(r64 start, r64 end, r64 value) { return ClampR64((value - start) / (end - start), 0.0f, 1.0f); }

//TODO: Double check this gives me the correct value!
r32 FractionalPartR32(r32 value) { return AbsR32(ModR32(value, 1.0f)); }
r64 FractionalPartR64(r64 value) { return AbsR64(ModR64(value, 1.0)); }

u8 ClampCastI8ToU8(i8 value) { return (value >= 0) ? (u8)value : 0; }
i8 ClampCastU8ToI8(u8 value) { return (value <= INT8_MAX) ? (i8)value : INT8_MAX; }
u16 ClampCastI16ToU16(i16 value) { return (value >= 0) ? (u16)value : 0; }
i16 ClampCastU16ToI16(u16 value) { return (value <= INT16_MAX) ? (i16)value : INT16_MAX; }
u32 ClampCastI32ToU32(i32 value) { return (value >= 0) ? (u32)value : 0; }
i32 ClampCastU32ToI32(u32 value) { return (value <= INT32_MAX) ? (i32)value : INT32_MAX; }
u64 ClampCastI64ToU64(i64 value) { return (value >= 0) ? (u64)value : 0; }
i64 ClampCastU64ToI64(u64 value) { return (value <= INT64_MAX) ? (i64)value : INT64_MAX; }

//TODO: Do we want ClampCast variants for differently sized integers? Like I32ToI8?

i8 ReinterpretCastU8ToI8(u8 value) { return *((i8*)&value); }
u8 ReinterpretCastI8ToU8(i8 value) { return *((u8*)&value); }
i16 ReinterpretCastU16ToI16(u16 value) { return *((i16*)&value); }
u16 ReinterpretCastI16ToU16(i16 value) { return *((u16*)&value); }
i32 ReinterpretCastU32ToI32(u32 value) { return *((i32*)&value); }
u32 ReinterpretCastI32ToU32(i32 value) { return *((u32*)&value); }
i64 ReinterpretCastU64ToI64(u64 value) { return *((i64*)&value); }
u64 ReinterpretCastI64ToU64(i64 value) { return *((u64*)&value); }

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

u8  AbsDiffU8(u8   value1, u8  value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }
u16 AbsDiffU16(u16 value1, u16 value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }
u32 AbsDiffU32(u32 value1, u32 value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }
u64 AbsDiffU64(u64 value1, u64 value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }

i8  AbsDiffI8(i8   value1, i8  value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }
i16 AbsDiffI16(i16 value1, i16 value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }
i32 AbsDiffI32(i32 value1, i32 value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }
i64 AbsDiffI64(i64 value1, i64 value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }

#endif //  _STD_MATH_EX_H
