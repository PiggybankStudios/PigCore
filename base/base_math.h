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
#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE i32 CeilDivI32(i32 dividend, i32 divisor);
	PIG_CORE_INLINE i64 CeilDivI64(i64 dividend, i64 divisor);
	PIG_CORE_INLINE ixx CeilDivIXX(ixx dividend, ixx divisor);
	PIG_CORE_INLINE u32 CeilDivU32(u32 dividend, u32 divisor);
	PIG_CORE_INLINE u64 CeilDivU64(u64 dividend, u64 divisor);
	PIG_CORE_INLINE uxx CeilDivUXX(uxx dividend, uxx divisor);
	PIG_CORE_INLINE i32 CeilToI32(i32 value, i32 chunkSize);
	PIG_CORE_INLINE i64 CeilToI64(i64 value, i64 chunkSize);
	PIG_CORE_INLINE u32 CeilToU32(u32 value, u32 chunkSize);
	PIG_CORE_INLINE u64 CeilToU64(u64 value, u64 chunkSize);
	PIG_CORE_INLINE i8  SignOfI8(i8   value);
	PIG_CORE_INLINE i16 SignOfI16(i16 value);
	PIG_CORE_INLINE i32 SignOfI32(i32 value);
	PIG_CORE_INLINE i64 SignOfI64(i64 value);
	r32 SignOfR32(r32 value);
	r64 SignOfR64(r64 value);
	PIG_CORE_INLINE u8  ClampU8(u8   value, u8  min, u8  max);
	PIG_CORE_INLINE u16 ClampU16(u16 value, u16 min, u16 max);
	PIG_CORE_INLINE u32 ClampU32(u32 value, u32 min, u32 max);
	PIG_CORE_INLINE u64 ClampU64(u64 value, u64 min, u64 max);
	PIG_CORE_INLINE uxx ClampUXX(uxx value, uxx min, uxx max);
	PIG_CORE_INLINE i8  ClampI8(i8   value, i8  min, i8  max);
	PIG_CORE_INLINE i16 ClampI16(i16 value, i16 min, i16 max);
	PIG_CORE_INLINE i32 ClampI32(i32 value, i32 min, i32 max);
	PIG_CORE_INLINE i64 ClampI64(i64 value, i64 min, i64 max);
	PIG_CORE_INLINE ixx ClampIXX(ixx value, ixx min, ixx max);
	PIG_CORE_INLINE r32 ClampR32(r32 value, r32 min, r32 max);
	PIG_CORE_INLINE r64 ClampR64(r64 value, r64 min, r64 max);
	PIG_CORE_INLINE r32 LerpR32(r32 val1, r32 val2, r32 amount);
	PIG_CORE_INLINE r64 LerpR64(r64 val1, r64 val2, r64 amount);
	PIG_CORE_INLINE u8 ClampCastI8ToU8(i8 value);
	PIG_CORE_INLINE i8 ClampCastU8ToI8(u8 value);
	PIG_CORE_INLINE u16 ClampCastI16ToU16(i16 value);
	PIG_CORE_INLINE i16 ClampCastU16ToI16(u16 value);
	PIG_CORE_INLINE u32 ClampCastI32ToU32(i32 value);
	PIG_CORE_INLINE i32 ClampCastU32ToI32(u32 value);
	PIG_CORE_INLINE u64 ClampCastI64ToU64(i64 value);
	PIG_CORE_INLINE i64 ClampCastU64ToI64(u64 value);
	PIG_CORE_INLINE u8 ClampCastI32ToU8(i32 value);
	PIG_CORE_INLINE i8 ReinterpretCastU8ToI8(u8 value);
	PIG_CORE_INLINE u8 ReinterpretCastI8ToU8(i8 value);
	PIG_CORE_INLINE i16 ReinterpretCastU16ToI16(u16 value);
	PIG_CORE_INLINE u16 ReinterpretCastI16ToU16(i16 value);
	PIG_CORE_INLINE i32 ReinterpretCastU32ToI32(u32 value);
	PIG_CORE_INLINE u32 ReinterpretCastI32ToU32(i32 value);
	PIG_CORE_INLINE i64 ReinterpretCastU64ToI64(u64 value);
	PIG_CORE_INLINE u64 ReinterpretCastI64ToU64(i64 value);
	PIG_CORE_INLINE u8  AbsDiffU8(u8   value1, u8  value2);
	PIG_CORE_INLINE u16 AbsDiffU16(u16 value1, u16 value2);
	PIG_CORE_INLINE u32 AbsDiffU32(u32 value1, u32 value2);
	PIG_CORE_INLINE u64 AbsDiffU64(u64 value1, u64 value2);
	PIG_CORE_INLINE i8  AbsDiffI8(i8   value1, i8  value2);
	PIG_CORE_INLINE i16 AbsDiffI16(i16 value1, i16 value2);
	PIG_CORE_INLINE i32 AbsDiffI32(i32 value1, i32 value2);
	PIG_CORE_INLINE i64 AbsDiffI64(i64 value1, i64 value2);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI i32 CeilDivI32(i32 dividend, i32 divisor)
{
	if ((dividend % divisor) == 0) { return dividend / divisor; }
	else { return (dividend / divisor) + 1; }
}
PEXPI i64 CeilDivI64(i64 dividend, i64 divisor)
{
	if ((dividend % divisor) == 0) { return dividend / divisor; }
	else { return (dividend / divisor) + 1; }
}
PEXPI ixx CeilDivIXX(ixx dividend, ixx divisor)
{
	#if TARGET_IS_32BIT
	return CeilDivI32(dividend, divisor);
	#else
	return CeilDivI64(dividend, divisor);
	#endif
}
PEXPI u32 CeilDivU32(u32 dividend, u32 divisor)
{
	if ((dividend % divisor) == 0) { return dividend / divisor; }
	else { return (dividend / divisor) + 1; }
}
PEXPI u64 CeilDivU64(u64 dividend, u64 divisor)
{
	if ((dividend % divisor) == 0) { return dividend / divisor; }
	else { return (dividend / divisor) + 1; }
}
PEXPI uxx CeilDivUXX(uxx dividend, uxx divisor)
{
	#if TARGET_IS_32BIT
	return CeilDivU32(dividend, divisor);
	#else
	return CeilDivU64(dividend, divisor);
	#endif
}

PEXPI i32 CeilToI32(i32 value, i32 chunkSize) { return ((value % chunkSize) == 0) ? value : (value + (chunkSize - (value%chunkSize))); }
PEXPI i64 CeilToI64(i64 value, i64 chunkSize) { return ((value % chunkSize) == 0) ? value : (value + (chunkSize - (value%chunkSize))); }
PEXPI u32 CeilToU32(u32 value, u32 chunkSize) { return ((value % chunkSize) == 0) ? value : (value + (chunkSize - (value%chunkSize))); }
PEXPI u64 CeilToU64(u64 value, u64 chunkSize) { return ((value % chunkSize) == 0) ? value : (value + (chunkSize - (value%chunkSize))); }

PEXPI i8  SignOfI8(i8   value) { return (value < 0) ? -1 : ((value > 0) ? 1 : 0); }
PEXPI i16 SignOfI16(i16 value) { return (value < 0) ? -1 : ((value > 0) ? 1 : 0); }
PEXPI i32 SignOfI32(i32 value) { return (value < 0) ? -1 : ((value > 0) ? 1 : 0); }
PEXPI i64 SignOfI64(i64 value) { return (value < 0) ? -1 : ((value > 0) ? 1 : 0); }
PEXP r32 SignOfR32(r32 value) { return (value < 0.0f) ? -1.0f : ((value > 0.0f) ? 1.0f : 0.0f); }
PEXP r64 SignOfR64(r64 value) { return (value < 0.0) ? -1.0 : ((value > 0.0) ? 1.0 : 0.0); }

PEXPI u8  ClampU8(u8   value, u8  min, u8  max) { return (value < min) ? min : ((value > max) ? max : value); }
PEXPI u16 ClampU16(u16 value, u16 min, u16 max) { return (value < min) ? min : ((value > max) ? max : value); }
PEXPI u32 ClampU32(u32 value, u32 min, u32 max) { return (value < min) ? min : ((value > max) ? max : value); }
PEXPI u64 ClampU64(u64 value, u64 min, u64 max) { return (value < min) ? min : ((value > max) ? max : value); }
PEXPI uxx ClampUXX(uxx value, uxx min, uxx max) { return (value < min) ? min : ((value > max) ? max : value); }
PEXPI i8  ClampI8(i8   value, i8  min, i8  max) { return (value < min) ? min : ((value > max) ? max : value); }
PEXPI i16 ClampI16(i16 value, i16 min, i16 max) { return (value < min) ? min : ((value > max) ? max : value); }
PEXPI i32 ClampI32(i32 value, i32 min, i32 max) { return (value < min) ? min : ((value > max) ? max : value); }
PEXPI i64 ClampI64(i64 value, i64 min, i64 max) { return (value < min) ? min : ((value > max) ? max : value); }
PEXPI ixx ClampIXX(ixx value, ixx min, ixx max) { return (value < min) ? min : ((value > max) ? max : value); }
PEXPI r32 ClampR32(r32 value, r32 min, r32 max) { return (value < min) ? min : ((value > max) ? max : value); }
PEXPI r64 ClampR64(r64 value, r64 min, r64 max) { return (value < min) ? min : ((value > max) ? max : value); }

PEXPI r32 LerpR32(r32 val1, r32 val2, r32 amount) { return (val1 + (val2 - val1) * amount); }
PEXPI r64 LerpR64(r64 val1, r64 val2, r64 amount) { return (val1 + (val2 - val1) * amount); }

PEXPI u8 ClampCastI8ToU8(i8 value) { return (value >= 0) ? (u8)value : 0; }
PEXPI i8 ClampCastU8ToI8(u8 value) { return (value <= INT8_MAX) ? (i8)value : INT8_MAX; }
PEXPI u16 ClampCastI16ToU16(i16 value) { return (value >= 0) ? (u16)value : 0; }
PEXPI i16 ClampCastU16ToI16(u16 value) { return (value <= INT16_MAX) ? (i16)value : INT16_MAX; }
PEXPI u32 ClampCastI32ToU32(i32 value) { return (value >= 0) ? (u32)value : 0; }
PEXPI i32 ClampCastU32ToI32(u32 value) { return (value <= INT32_MAX) ? (i32)value : INT32_MAX; }
PEXPI u64 ClampCastI64ToU64(i64 value) { return (value >= 0) ? (u64)value : 0; }
PEXPI i64 ClampCastU64ToI64(u64 value) { return (value <= INT64_MAX) ? (i64)value : INT64_MAX; }
PEXPI u8 ClampCastI32ToU8(i32 value) { return ((value <= UINT8_MAX) ? (value >= 0 ? (u8)value : (u8)0) : UINT8_MAX); }

//TODO: Do we want ClampCast variants for differently sized integers? Like I32ToI8?

PEXPI i8 ReinterpretCastU8ToI8(u8 value) { return *((i8*)&value); }
PEXPI u8 ReinterpretCastI8ToU8(i8 value) { return *((u8*)&value); }
PEXPI i16 ReinterpretCastU16ToI16(u16 value) { return *((i16*)&value); }
PEXPI u16 ReinterpretCastI16ToU16(i16 value) { return *((u16*)&value); }
PEXPI i32 ReinterpretCastU32ToI32(u32 value) { return *((i32*)&value); }
PEXPI u32 ReinterpretCastI32ToU32(i32 value) { return *((u32*)&value); }
PEXPI i64 ReinterpretCastU64ToI64(u64 value) { return *((i64*)&value); }
PEXPI u64 ReinterpretCastI64ToU64(i64 value) { return *((u64*)&value); }

PEXPI u8  AbsDiffU8(u8   value1, u8  value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }
PEXPI u16 AbsDiffU16(u16 value1, u16 value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }
PEXPI u32 AbsDiffU32(u32 value1, u32 value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }
PEXPI u64 AbsDiffU64(u64 value1, u64 value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }

PEXPI i8  AbsDiffI8(i8   value1, i8  value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }
PEXPI i16 AbsDiffI16(i16 value1, i16 value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }
PEXPI i32 AbsDiffI32(i32 value1, i32 value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }
PEXPI i64 AbsDiffI64(i64 value1, i64 value2) { return (value1 >= value2) ? (value1 - value2) : (value2 - value1); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _BASE_MATH_H
