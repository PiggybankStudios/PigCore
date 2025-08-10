/*
File:   struct_ranges.h
Author: Taylor Robbins
Date:   08\08\2025
Description:
	** Ranges are similar to Vector2 types but conceptually they encode different data
	** and are often used for different purposes (and often want different types like uxx and ixx)
*/

#ifndef _STRUCT_RANGES_H
#define _STRUCT_RANGES_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "base/base_math.h"
#include "std/std_basic_math.h"

typedef car RangeUXX RangeUXX;
car RangeUXX
{
	struct { uxx min, max; };
	struct { uxx left, right; };
	struct { uxx start, end; };
	struct { uxx low, high; };
};

typedef car RangeIXX RangeIXX;
car RangeIXX
{
	struct { ixx min, max; };
	struct { ixx left, right; };
	struct { ixx start, end; };
	struct { ixx low, high; };
};

typedef car RangeR32 RangeR32;
car RangeR32
{
	struct { r32 min, max; };
	struct { r32 left, right; };
	struct { r32 start, end; };
	struct { r32 low, high; };
};
typedef car RangeR64 RangeR64;
car RangeR64
{
	struct { r64 min, max; };
	struct { r64 left, right; };
	struct { r64 start, end; };
	struct { r64 low, high; };
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE RangeUXX NewRangeUXX(uxx value1, uxx value2);
	PIG_CORE_INLINE RangeIXX NewRangeIXX(ixx value1, ixx value2);
	PIG_CORE_INLINE RangeR32 NewRangeR32(r32 value1, r32 value2);
	PIG_CORE_INLINE RangeR64 NewRangeR64(r64 value1, r64 value2);
	PIG_CORE_INLINE RangeUXX NewRangeUXXLength(uxx start, uxx length);
	PIG_CORE_INLINE RangeIXX NewRangeIXXLength(ixx start, ixx length);
	PIG_CORE_INLINE RangeR32 NewRangeR32Length(r32 start, r32 length);
	PIG_CORE_INLINE RangeR64 NewRangeR64Length(r64 start, r64 length);
	PIG_CORE_INLINE RangeUXX FillRangeUXX(uxx value);
	PIG_CORE_INLINE RangeIXX FillRangeIXX(ixx value);
	PIG_CORE_INLINE RangeR32 FillRangeR32(r32 value);
	PIG_CORE_INLINE RangeR64 FillRangeR64(r64 value);
	PIG_CORE_INLINE bool AreEqualRangeUXX(RangeUXX left, RangeUXX right);
	PIG_CORE_INLINE bool AreEqualRangeIXX(RangeIXX left, RangeIXX right);
	PIG_CORE_INLINE bool AreEqualRangeR32(RangeR32 left, RangeR32 right);
	PIG_CORE_INLINE bool AreEqualRangeR64(RangeR64 left, RangeR64 right);
	PIG_CORE_INLINE bool AreSimilarRangeR32(RangeR32 left, RangeR32 right, r32 tolerance);
	PIG_CORE_INLINE bool AreSimilarRangeR64(RangeR64 left, RangeR64 right, r64 tolerance);
	PIG_CORE_INLINE bool DoesOverlapRangeUXX(RangeUXX left, RangeUXX right, bool inclusive);
	PIG_CORE_INLINE bool DoesOverlapRangeIXX(RangeIXX left, RangeIXX right, bool inclusive);
	PIG_CORE_INLINE bool DoesOverlapRangeR32(RangeR32 left, RangeR32 right, bool inclusive);
	PIG_CORE_INLINE bool DoesOverlapRangeR64(RangeR64 left, RangeR64 right, bool inclusive);
	PIG_CORE_INLINE RangeUXX BothRangeUXX(RangeUXX range1, RangeUXX range2);
	PIG_CORE_INLINE RangeIXX BothRangeIXX(RangeIXX range1, RangeIXX range2);
	PIG_CORE_INLINE RangeR32 BothRangeR32(RangeR32 range1, RangeR32 range2);
	PIG_CORE_INLINE RangeR64 BothRangeR64(RangeR64 range1, RangeR64 range2);
	PIG_CORE_INLINE RangeUXX OverlapPartRangeUXX(RangeUXX range1, RangeUXX range2);
	PIG_CORE_INLINE RangeIXX OverlapPartRangeIXX(RangeIXX range1, RangeIXX range2);
	PIG_CORE_INLINE RangeR32 OverlapPartRangeR32(RangeR32 range1, RangeR32 range2);
	PIG_CORE_INLINE RangeR64 OverlapPartRangeR64(RangeR64 range1, RangeR64 range2);
	PIG_CORE_INLINE RangeUXX ClampBelowRangeUXX(RangeUXX range, uxx maxValue);
	PIG_CORE_INLINE RangeIXX ClampBelowRangeIXX(RangeIXX range, ixx maxValue);
	PIG_CORE_INLINE RangeR32 ClampBelowRangeR32(RangeR32 range, r32 maxValue);
	PIG_CORE_INLINE RangeR64 ClampBelowRangeR64(RangeR64 range, r64 maxValue);
	PIG_CORE_INLINE RangeUXX ClampAboveRangeUXX(RangeUXX range, uxx minValue);
	PIG_CORE_INLINE RangeIXX ClampAboveRangeIXX(RangeIXX range, ixx minValue);
	PIG_CORE_INLINE RangeR32 ClampAboveRangeR32(RangeR32 range, r32 minValue);
	PIG_CORE_INLINE RangeR64 ClampAboveRangeR64(RangeR64 range, r64 minValue);
	PIG_CORE_INLINE RangeUXX ClampRangeToRangeUXX(RangeUXX range, RangeUXX limits);
	PIG_CORE_INLINE RangeIXX ClampRangeToRangeIXX(RangeIXX range, RangeIXX limits);
	PIG_CORE_INLINE RangeR32 ClampRangeToRangeR32(RangeR32 range, RangeR32 limits);
	PIG_CORE_INLINE RangeR64 ClampRangeToRangeR64(RangeR64 range, RangeR64 limits);
#endif

// +--------------------------------------------------------------+
// |                           Defines                            |
// +--------------------------------------------------------------+
#define RangeUXX_Zero    NewRangeUXX(0U, 0U)
#define RangeIXX_Zero    NewRangeIXX(0, 0)
#define RangeR32_Zero    NewRangeR32(0.0f, 0.0f)
#define RangeR64_Zero    NewRangeR64(0.0, 0.0)

#define RangeUXX_Zero_Const    {.min=0U,.max=0U}
#define RangeIXX_Zero_Const    {.min=0,.max=0}
#define RangeR32_Zero_Const    {.min=0.0f,.max=0.0f}
#define RangeR64_Zero_Const    {.min=0.0,.max=0.0}

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI RangeUXX NewRangeUXX(uxx value1, uxx value2)
{
	RangeUXX result = RangeUXX_Zero_Const;
	result.min = MinUXX(value1, value2);
	result.max = MaxUXX(value1, value2);
	return result;
}
PEXPI RangeIXX NewRangeIXX(ixx value1, ixx value2)
{
	RangeIXX result = RangeIXX_Zero_Const;
	result.min = MinIXX(value1, value2);
	result.max = MaxIXX(value1, value2);
	return result;
}
PEXPI RangeR32 NewRangeR32(r32 value1, r32 value2)
{
	RangeR32 result = RangeR32_Zero_Const;
	result.min = MinR32(value1, value2);
	result.max = MaxR32(value1, value2);
	return result;
}
PEXPI RangeR64 NewRangeR64(r64 value1, r64 value2)
{
	RangeR64 result = RangeR64_Zero_Const;
	result.min = MinR64(value1, value2);
	result.max = MaxR64(value1, value2);
	return result;
}

PEXPI RangeUXX NewRangeUXXLength(uxx start, uxx length)
{
	RangeUXX result = RangeUXX_Zero_Const;
	result.min = start;
	result.max = start + length;
	return result;
}
PEXPI RangeIXX NewRangeIXXLength(ixx start, ixx length)
{
	RangeIXX result = RangeIXX_Zero_Const;
	result.min = (length >= 0) ? start : start + length;
	result.max = (length >= 0) ? start + length : start;
	return result;
}
PEXPI RangeR32 NewRangeR32Length(r32 start, r32 length)
{
	RangeR32 result = RangeR32_Zero_Const;
	result.min = (length >= 0.0f) ? start : start + length;
	result.max = (length >= 0.0f) ? start + length : start;
	return result;
}
PEXPI RangeR64 NewRangeR64Length(r64 start, r64 length)
{
	RangeR64 result = RangeR64_Zero_Const;
	result.min = (length >= 0.0f) ? start : start + length;
	result.max = (length >= 0.0f) ? start + length : start;
	return result;
}

PEXPI RangeUXX FillRangeUXX(uxx value)
{
	RangeUXX result = RangeUXX_Zero_Const;
	result.min = value;
	result.max = value;
	return result;
}
PEXPI RangeIXX FillRangeIXX(ixx value)
{
	RangeIXX result = RangeIXX_Zero_Const;
	result.min = value;
	result.max = value;
	return result;
}
PEXPI RangeR32 FillRangeR32(r32 value)
{
	RangeR32 result = RangeR32_Zero_Const;
	result.min = value;
	result.max = value;
	return result;
}
PEXPI RangeR64 FillRangeR64(r64 value)
{
	RangeR64 result = RangeR64_Zero_Const;
	result.min = value;
	result.max = value;
	return result;
}

PEXPI bool AreEqualRangeUXX(RangeUXX left, RangeUXX right) { return (left.min == right.min && left.max == right.max); }
PEXPI bool AreEqualRangeIXX(RangeIXX left, RangeIXX right) { return (left.min == right.min && left.max == right.max); }
PEXPI bool AreEqualRangeR32(RangeR32 left, RangeR32 right) { return (left.min == right.min && left.max == right.max); }
PEXPI bool AreEqualRangeR64(RangeR64 left, RangeR64 right) { return (left.min == right.min && left.max == right.max); }

PEXPI bool AreSimilarRangeR32(RangeR32 left, RangeR32 right, r32 tolerance) { return (AreSimilarR32(left.min, right.min, tolerance) && AreSimilarR32(left.max, right.max, tolerance)); }
PEXPI bool AreSimilarRangeR64(RangeR64 left, RangeR64 right, r64 tolerance) { return (AreSimilarR64(left.min, right.min, tolerance) && AreSimilarR64(left.max, right.max, tolerance)); }

PEXPI bool DoesOverlapRangeUXX(RangeUXX left, RangeUXX right, bool inclusive) { return (left.max > right.min || (inclusive && left.max == right.min)) && (right.max > left.min || (inclusive && right.max == left.min)); }
PEXPI bool DoesOverlapRangeIXX(RangeIXX left, RangeIXX right, bool inclusive) { return (left.max > right.min || (inclusive && left.max == right.min)) && (right.max > left.min || (inclusive && right.max == left.min)); }
PEXPI bool DoesOverlapRangeR32(RangeR32 left, RangeR32 right, bool inclusive) { return (left.max > right.min || (inclusive && left.max == right.min)) && (right.max > left.min || (inclusive && right.max == left.min)); }
PEXPI bool DoesOverlapRangeR64(RangeR64 left, RangeR64 right, bool inclusive) { return (left.max > right.min || (inclusive && left.max == right.min)) && (right.max > left.min || (inclusive && right.max == left.min)); }

PEXPI RangeUXX BothRangeUXX(RangeUXX range1, RangeUXX range2)
{
	return NewRangeUXX(MinUXX(range1.min, range2.min), MaxUXX(range1.max, range2.max));
}
PEXPI RangeIXX BothRangeIXX(RangeIXX range1, RangeIXX range2)
{
	return NewRangeIXX(MinIXX(range1.min, range2.min), MaxIXX(range1.max, range2.max));
}
PEXPI RangeR32 BothRangeR32(RangeR32 range1, RangeR32 range2)
{
	return NewRangeR32(MinR32(range1.min, range2.min), MaxR32(range1.max, range2.max));
}
PEXPI RangeR64 BothRangeR64(RangeR64 range1, RangeR64 range2)
{
	return NewRangeR64(MinR64(range1.min, range2.min), MaxR64(range1.max, range2.max));
}

PEXPI RangeUXX OverlapPartRangeUXX(RangeUXX range1, RangeUXX range2)
{
	if (range1.max >= range2.min || range2.max >= range1.min)
	{
		return NewRangeUXX(MaxUXX(range1.min, range2.min), MinUXX(range1.max, range2.max));
	}
	else { return RangeUXX_Zero; }
}
PEXPI RangeIXX OverlapPartRangeIXX(RangeIXX range1, RangeIXX range2)
{
	if (range1.max >= range2.min || range2.max >= range1.min)
	{
		return NewRangeIXX(MaxIXX(range1.min, range2.min), MinIXX(range1.max, range2.max));
	}
	else { return RangeIXX_Zero; }
}
PEXPI RangeR32 OverlapPartRangeR32(RangeR32 range1, RangeR32 range2)
{
	if (range1.max >= range2.min || range2.max >= range1.min)
	{
		return NewRangeR32(MaxR32(range1.min, range2.min), MinR32(range1.max, range2.max));
	}
	else { return RangeR32_Zero; }
}
PEXPI RangeR64 OverlapPartRangeR64(RangeR64 range1, RangeR64 range2)
{
	if (range1.max >= range2.min || range2.max >= range1.min)
	{
		return NewRangeR64(MaxR64(range1.min, range2.min), MinR64(range1.max, range2.max));
	}
	else { return RangeR64_Zero; }
}

PEXPI RangeUXX ClampBelowRangeUXX(RangeUXX range, uxx maxValue)
{
	return NewRangeUXX(MinUXX(range.min, maxValue), MinUXX(range.max, maxValue));
}
PEXPI RangeIXX ClampBelowRangeIXX(RangeIXX range, ixx maxValue)
{
	return NewRangeIXX(MinIXX(range.min, maxValue), MinIXX(range.max, maxValue));
}
PEXPI RangeR32 ClampBelowRangeR32(RangeR32 range, r32 maxValue)
{
	return NewRangeR32(MinR32(range.min, maxValue), MinR32(range.max, maxValue));
}
PEXPI RangeR64 ClampBelowRangeR64(RangeR64 range, r64 maxValue)
{
	return NewRangeR64(MinR64(range.min, maxValue), MinR64(range.max, maxValue));
}

PEXPI RangeUXX ClampAboveRangeUXX(RangeUXX range, uxx minValue)
{
	return NewRangeUXX(MaxUXX(range.min, minValue), MaxUXX(range.max, minValue));
}
PEXPI RangeIXX ClampAboveRangeIXX(RangeIXX range, ixx minValue)
{
	return NewRangeIXX(MaxIXX(range.min, minValue), MaxIXX(range.max, minValue));
}
PEXPI RangeR32 ClampAboveRangeR32(RangeR32 range, r32 minValue)
{
	return NewRangeR32(MaxR32(range.min, minValue), MaxR32(range.max, minValue));
}
PEXPI RangeR64 ClampAboveRangeR64(RangeR64 range, r64 minValue)
{
	return NewRangeR64(MaxR64(range.min, minValue), MaxR64(range.max, minValue));
}

PEXPI RangeUXX ClampRangeToRangeUXX(RangeUXX range, RangeUXX limits)
{
	return NewRangeUXX(ClampUXX(range.min, limits.min, limits.max), ClampUXX(range.max, limits.min, limits.max));
}
PEXPI RangeIXX ClampRangeToRangeIXX(RangeIXX range, RangeIXX limits)
{
	return NewRangeIXX(ClampIXX(range.min, limits.min, limits.max), ClampIXX(range.max, limits.min, limits.max));
}
PEXPI RangeR32 ClampRangeToRangeR32(RangeR32 range, RangeR32 limits)
{
	return NewRangeR32(ClampR32(range.min, limits.min, limits.max), ClampR32(range.max, limits.min, limits.max));
}
PEXPI RangeR64 ClampRangeToRangeR64(RangeR64 range, RangeR64 limits)
{
	return NewRangeR64(ClampR64(range.min, limits.min, limits.max), ClampR64(range.max, limits.min, limits.max));
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_RANGES_H

#if defined(_STRUCT_STRING_H) && defined(_STRUCT_RANGES_H)
#include "cross/cross_string_and_ranges.h"
#endif

#if defined(_STRUCT_RICH_STRING_H) && defined(_MEM_SCRATCH_H) && defined(_STRUCT_RANGES_H)
#include "cross/cross_rich_string_scratch_and_ranges.h"
#endif
