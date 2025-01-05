/*
File:   std_trig.h
Author: Taylor Robbins
Date:   01\05\2025
*/

#ifndef _STD_TRIG_H
#define _STD_TRIG_H

#include "base/base_typedefs.h"
#include "std/std_includes.h"

r32 SinR32(r32 angle) { return sinf(angle); }
r64 SinR64(r64 angle) { return sin(angle); }

r32 CosR32(r32 angle) { return cosf(angle); }
r64 CosR64(r64 angle) { return cos(angle); }

r32 TanR32(r32 angle) { return tanf(angle); }
r64 TanR64(r64 angle) { return tan(angle); }

r32 AsinR32(r32 value) { return asinf(value); }
r64 AsinR64(r64 value) { return asin(value); }

r32 AcosR32(r32 value) { return acosf(value); }
r64 AcosR64(r64 value) { return acos(value); }

r32 AtanJoinedR32(r32 value) { return atanf(value); }
r64 AtanJoinedR64(r64 value) { return atan(value); }
r32 AtanR32(r32 y, r32 x) { return atan2f(y, x); }
r64 AtanR64(r64 y, r64 x) { return atan2(y, x); }

#endif //  _STD_TRIG_H
