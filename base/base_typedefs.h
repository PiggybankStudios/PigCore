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
#include <stdbool.h>

// NOTE: Both "long" and "long long" in Clang are 8 bytes, so int64_t/uint64_t are "long"
//       Meanwhile in MSVC "long" is 4 bytes while "long long" is 8 bytes, so int64_t/uint64_t are "long long"
//       Format arguments like %llu will complain if this is technically a "long" and not a "long long"
//       So to make sure we can always use %llu arguments across Windows and Linux we specifically typedef long long, not int64_t/uint64_t from stdint.h.

// We use an "i" to indicate it can hold integer numbers
typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef long long   i64;
#if LANGUAGE_IS_C
_Static_assert(sizeof(i64) == 8, "i64 is not 8 bytes!"); //TODO: Change to StaticAssert once we diangose that
#endif

// We use a "u" to distinguish these as only holding unsigned numbers (they are still integers, but "i" is already taken)
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef unsigned long long u64;
#if LANGUAGE_IS_C
_Static_assert(sizeof(u64) == 8, "u64 is not 8 bytes!"); //TODO: Change to StaticAssert once we diangose that
#endif

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

#if LANGUAGE_IS_C
#define nullptr ((void*)0)
#endif

#define VOID_FUNC_DEF(functionName) void functionName()
typedef VOID_FUNC_DEF(void_func_f);
#define BOOL_FUNC_DEF(functionName) bool functionName()
typedef BOOL_FUNC_DEF(bool_func_f);
#define I32_FUNC_DEF(functionName) i32 functionName()
typedef I32_FUNC_DEF(i32_func_f);
#define VOID_PNTR_FUNC_DEF(functionName) void* functionName()
typedef VOID_PNTR_FUNC_DEF(void_pntr_func_f);

#endif //  _BASE_TYPEDEFS_H
