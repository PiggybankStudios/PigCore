/*
File:   lib_handmade_math.h
Author: Taylor Robbins
Date:   01\15\2025
*/

#ifndef _LIB_HANDMADE_MATH_H
#define _LIB_HANDMADE_MATH_H

#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "std/std_basic_math.h"
#include "std/std_trig.h"

// #define HANDMADE_MATH_NO_SSE //TODO: We could enable this and deleted Vec4Raw, since it would allow HMM_Vec4 to be 4-byte aligned rather than 16
#define HANDMADE_MATH_USE_RADIANS
#define HANDMADE_MATH_PROVIDE_MATH_FUNCTIONS
#define HMM_SINF SinR32
#define HMM_COSF CosR32
#define HMM_TANF TanR32
#define HMM_SQRTF SqrtR32
#define HMM_ACOSF AcosR32

#if COMPILER_IS_MSVC
//NOTE: HandmadeMath.h disables warning 4201 when compiling under MSVC
#pragma warning(push)
#else
//NOTE: HandmadeMath.h disables -Wfloat-equal, -Wmissing-braces, -Wgnu-anonymous-struct and -Wmissing-field-initializers warnings
#pragma clang diagnostic push
#endif

#include "third_party/handmade_math/HandmadeMath.h"

#if COMPILER_IS_MSVC
#pragma warning(pop)
#else
#pragma clang diagnostic pop
#endif

//NOTE: Handmade math types use "float" members but we would normally use "r32" typedef. Double check those are the same thing
#if STATIC_ASSERT_AVAILABLE
_Static_assert(sizeof(float) == sizeof(r32), "float is not the same size as r32!");
#endif

#endif //  _LIB_HANDMADE_MATH_H
