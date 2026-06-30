/*
File:   std_trig.h
Author: Taylor Robbins
Date:   01\05\2025
*/

#ifndef _STD_TRIG_H
#define _STD_TRIG_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "std/std_includes.h"

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	r32 SinR32(r32 angle);
	PIG_CORE_INLINE r64 SinR64(r64 angle);
	r32 CosR32(r32 angle);
	PIG_CORE_INLINE r64 CosR64(r64 angle);
	PIG_CORE_INLINE r32 TanR32(r32 angle);
	PIG_CORE_INLINE r64 TanR64(r64 angle);
	PIG_CORE_INLINE r32 AsinR32(r32 value);
	PIG_CORE_INLINE r64 AsinR64(r64 value);
	r32 AcosR32(r32 value);
	PIG_CORE_INLINE r64 AcosR64(r64 value);
	PIG_CORE_INLINE r32 AtanJoinedR32(r32 value);
	PIG_CORE_INLINE r64 AtanJoinedR64(r64 value);
	r32 AtanR32(r32 y, r32 x);
	PIG_CORE_INLINE r64 AtanR64(r64 y, r64 x);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP r32 SinR32(r32 angle) { return sinf(angle); }
PEXPI r64 SinR64(r64 angle) { return sin(angle); }

PEXP r32 CosR32(r32 angle) { return cosf(angle); }
PEXPI r64 CosR64(r64 angle) { return cos(angle); }

PEXPI r32 TanR32(r32 angle) { return tanf(angle); }
PEXPI r64 TanR64(r64 angle) { return tan(angle); }

PEXPI r32 AsinR32(r32 value) { return asinf(value); }
PEXPI r64 AsinR64(r64 value) { return asin(value); }

PEXP r32 AcosR32(r32 value) { return acosf(value); }
PEXPI r64 AcosR64(r64 value) { return acos(value); }

PEXPI r32 AtanJoinedR32(r32 value) { return atanf(value); }
PEXPI r64 AtanJoinedR64(r64 value) { return atan(value); }
PEXP r32 AtanR32(r32 y, r32 x) { return atan2f(y, x); }
PEXPI r64 AtanR64(r64 y, r64 x) { return atan2(y, x); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STD_TRIG_H
