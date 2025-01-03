/*
File:   base_typedefs.h
Author: Taylor Robbins
Date:   01\02\2025
Description:
	** These are some fundamental typedefs that all our projects in C use
*/

#ifndef _BASE_TYPEDEFS_H
#define _BASE_TYPEDEFS_H

#include "base/base_compiler_check.h"

#include <stdint.h>

// We use an "i" to indicate it can hold integer numbers
typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

// We use a "u" to distinguish these as only holding unsigned numbers (they are still integers, but "i" is already taken)
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

// We use an "r" to indicate it can hold real numbers
typedef float       r32;
typedef double      r64;

//These types work similar to size_t but are directly controlled by our defines, and have a stricter guarantee for their type
// This should allow application code that doesn't care about 32-bit platforms to assume these types are 64-bit and use the
// i64, u64, r64 types directly (also the naming convention is more agreeable than "size_t")
// Current 32-bit platforms: web, orca, and playdate
#if TARGET_IS_32BIT
typedef i32 ixx;
typedef u32 uxx;
typedef r32 rxx;
#define UINTXX_MAX UINT32_MAX
#define INTXX_MIN  INT32_MIN
#define INTXX_MAX  INT32_MAX
#elif TARGET_IS_64BIT
typedef i64 ixx;
typedef u64 uxx;
typedef r64 rxx;
#define UINTXX_MAX UINT64_MAX
#define INTXX_MIN  INT64_MIN
#define INTXX_MAX  INT64_MAX
#else
#error Unsupported PLATFORM_XBIT define!
#endif

#if COMPILER_IS_MSVC
#define nullptr 0
#endif

#endif //  _BASE_TYPEDEFS_H
