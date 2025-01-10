/*
File:   base_math.h
Author: Taylor Robbins
Date:   01\10\2025
Description:
	** Contains math routines that I use that are similar to std library math routines
	** but DO NOT rely on anything in std_basic_math.h or std_math_ex.h
*/

#ifndef _BASE_MATH_H
#define _BASE_MATH_H

#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"

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

u8  AbsDiffU8(u8   value1, u8  value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }
u16 AbsDiffU16(u16 value1, u16 value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }
u32 AbsDiffU32(u32 value1, u32 value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }
u64 AbsDiffU64(u64 value1, u64 value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }

i8  AbsDiffI8(i8   value1, i8  value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }
i16 AbsDiffI16(i16 value1, i16 value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }
i32 AbsDiffI32(i32 value1, i32 value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }
i64 AbsDiffI64(i64 value1, i64 value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }

#endif //  _BASE_MATH_H
