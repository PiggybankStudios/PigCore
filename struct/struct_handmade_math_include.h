/*
File:   struct_handmade_math_include.h
Author: Taylor Robbins
Date:   01\15\2025
*/

#ifndef _STRUCT_HANDMADE_MATH_INCLUDE_H
#define _STRUCT_HANDMADE_MATH_INCLUDE_H

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
#define HANDMADE_MATH_USE_RADIANS
#include "third_party/handmade_math/HandmadeMath.h"
#if COMPILER_IS_MSVC
#pragma warning(pop)
#else
#pragma clang diagnostic pop
#endif

//NOTE: Handmade math types use "float" members but we would normally use "r32" typedef. Double check those are the same thing
_Static_assert(sizeof(float) == sizeof(r32));

#endif //  _STRUCT_HANDMADE_MATH_INCLUDE_H
