/*
File:   std_basic_math.h
Author: Taylor Robbins
Date:   01\05\2025
*/

#ifndef _STD_BASIC_MATH_H
#define _STD_BASIC_MATH_H

#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "std/std_includes.h"

// +==============================+
// |             Min              |
// +==============================+
u8  MinU8(u8   value1, u8  value2) { return (value1 < value2) ? value1 : value2; }
u16 MinU16(u16 value1, u16 value2) { return (value1 < value2) ? value1 : value2; }
u32 MinU32(u32 value1, u32 value2) { return (value1 < value2) ? value1 : value2; }
u64 MinU64(u64 value1, u64 value2) { return (value1 < value2) ? value1 : value2; }
i8  MinI8(i8   value1, i8  value2) { return (value1 < value2) ? value1 : value2; }
i16 MinI16(i16 value1, i16 value2) { return (value1 < value2) ? value1 : value2; }
i32 MinI32(i32 value1, i32 value2) { return (value1 < value2) ? value1 : value2; }
i64 MinI64(i64 value1, i64 value2) { return (value1 < value2) ? value1 : value2; }
r32 MinR32(r32 value1, r32 value2) { return (value1 < value2) ? value1 : value2; }
r64 MinR64(r64 value1, r64 value2) { return (value1 < value2) ? value1 : value2; }

#define Min(value1, value2) (((value1) < (value2)) ? (value1) : (value2))
#define MinX(value1, value2) _Generic(value1, \
	u8:  MinU8((value1),  (value2)),          \
	u16: MinU16((value1), (value2)),          \
	u32: MinU32((value1), (value2)),          \
	u64: MinU64((value1), (value2)),          \
	i8:  MinI8((value1),  (value2)),          \
	i16: MinI16((value1), (value2)),          \
	i32: MinI32((value1), (value2)),          \
	i64: MinI64((value1), (value2)),          \
	r32: MinR32((value1), (value2)),          \
	r64: MinR64((value1), (value2))           \
)
#define MinX3(value1, value2, value3) _Generic(value1, \
	u8:  MinU8((value1),  MinU8((value2),  (value3))), \
	u16: MinU16((value1), MinU16((value2), (value3))), \
	u32: MinU32((value1), MinU32((value2), (value3))), \
	u64: MinU64((value1), MinU64((value2), (value3))), \
	i8:  MinI8((value1),  MinI8((value2),  (value3))), \
	i16: MinI16((value1), MinI16((value2), (value3))), \
	i32: MinI32((value1), MinI32((value2), (value3))), \
	i64: MinI64((value1), MinI64((value2), (value3))), \
	r32: MinR32((value1), MinR32((value2), (value3))), \
	r64: MinR64((value1), MinR64((value2), (value3)))  \
)

// +==============================+
// |             Max              |
// +==============================+
u8  MaxU8(u8   value1, u8  value2) { return (value1 > value2) ? value1 : value2; }
u16 MaxU16(u16 value1, u16 value2) { return (value1 > value2) ? value1 : value2; }
u32 MaxU32(u32 value1, u32 value2) { return (value1 > value2) ? value1 : value2; }
u64 MaxU64(u64 value1, u64 value2) { return (value1 > value2) ? value1 : value2; }
i8  MaxI8(i8   value1, i8  value2) { return (value1 > value2) ? value1 : value2; }
i16 MaxI16(i16 value1, i16 value2) { return (value1 > value2) ? value1 : value2; }
i32 MaxI32(i32 value1, i32 value2) { return (value1 > value2) ? value1 : value2; }
i64 MaxI64(i64 value1, i64 value2) { return (value1 > value2) ? value1 : value2; }
r32 MaxR32(r32 value1, r32 value2) { return (value1 > value2) ? value1 : value2; }
r64 MaxR64(r64 value1, r64 value2) { return (value1 > value2) ? value1 : value2; }

#define Max(value1, value2) (((value1) > (value2)) ? (value1) : (value2))
#define MaxX(value1, value2) _Generic(value1, \
	u8:  MaxU8((value1),  (value2)),          \
	u16: MaxU16((value1), (value2)),          \
	u32: MaxU32((value1), (value2)),          \
	u64: MaxU64((value1), (value2)),          \
	i8:  MaxI8((value1),  (value2)),          \
	i16: MaxI16((value1), (value2)),          \
	i32: MaxI32((value1), (value2)),          \
	i64: MaxI64((value1), (value2)),          \
	r32: MaxR32((value1), (value2)),          \
	r64: MaxR64((value1), (value2))           \
)
#define MaxX3(value1, value2, value3) _Generic(value1, \
	u8:  MaxU8((value1),  MaxU8((value2),  (value3))), \
	u16: MaxU16((value1), MaxU16((value2), (value3))), \
	u32: MaxU32((value1), MaxU32((value2), (value3))), \
	u64: MaxU64((value1), MaxU64((value2), (value3))), \
	i8:  MaxI8((value1),  MaxI8((value2),  (value3))), \
	i16: MaxI16((value1), MaxI16((value2), (value3))), \
	i32: MaxI32((value1), MaxI32((value2), (value3))), \
	i64: MaxI64((value1), MaxI64((value2), (value3))), \
	r32: MaxR32((value1), MaxR32((value2), (value3))), \
	r64: MaxR64((value1), MaxR64((value2), (value3)))  \
)

r32 AbsR32(r32 value) { return fabsf(value); }
r64 AbsR64(r64 value) { return fabs(value); }
i8 AbsI8(i8 value) { return (value < 0) ? -value : value; }
i16 AbsI16(i16 value) { return (value < 0) ? -value : value; }
i32 AbsI32(i32 value) { return (value < 0 ? -value : value); }
i64 AbsI64(i64 value) { return (value < 0 ? -value : value); }

r32 RoundR32(r32 value) { return roundf(value); }
i32 RoundR32i(r32 value) { return (i32)roundf(value); }
r64 RoundR64(r64 value) { return round(value); }
i64 RoundR64i(r64 value) { return (i64)round(value); }

r32 FloorR32(r32 value) { return floorf(value); }
i32 FloorR32i(r32 value) { return (i32)floorf(value); }
r64 FloorR64(r64 value) { return floor(value); }
i64 FloorR64i(r64 value) { return (i64)floor(value); }

r32 CeilR32(r32 value) { return ceilf(value); }
i32 CeilR32i(r32 value) { return (i32)ceilf(value); }
r64 CeilR64(r64 value) { return ceil(value); }
i64 CeilR64i(r64 value) { return (i64)ceil(value); }

r32 PowR32(r32 value, r32 power) { return powf(value, power); }
r64 PowR64(r64 value, r64 power) { return pow(value, power); }

r32 LnR32(r32 value) { return logf(value); }
r64 LnR64(r64 value) { return log(value); }
r32 Log2R32(r32 value) { return log2f(value); }
r64 Log2R64(r64 value) { return log2(value); }
r32 Log10R32(r32 value) { return log10f(value); }
r64 Log10R64(r64 value) { return log10(value); }

r32 SqrtR32(r32 value) { return sqrtf(value); }
r64 SqrtR64(r64 value) { return sqrt(value); }

r32 CbrtR32(r32 value) { return cbrtf(value); }
r64 CbrtR64(r64 value) { return cbrt(value); }

r32 ModR32(r32 numerator, r32 denominator) { return fmodf(numerator, denominator); }
r64 ModR64(r64 numerator, r64 denominator) { return fmod(numerator, denominator); }

//TODO: math.h doesn't have isinff or isnanf?? But this site claims they do? https://linux.die.net/man/3/isinff
bool IsInfiniteR32(r32 value) { return (isinf(value) || isnan(value)); }
bool IsInfiniteR64(r64 value) { return (isinf(value) || isnan(value)); }

#endif //  _STD_BASIC_MATH_H
