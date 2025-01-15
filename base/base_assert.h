/*
File:   base_assert.h
Author: Taylor Robbins
Date:   01\03\2025
Description:
	** Contains the Assert macro (and all it's variants and aliases like NotNull)
	** This macro can be configured in to call a callback function defined somewhere
	** else. It can also be configured to be compiled out completely.
*/

//TODO: Add a the configuration options mentioned above! Probably want to call something besides assert.h version of assert()

#ifndef _BASE_ASSERT_H
#define _BASE_ASSERT_H

#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"
#include "std/std_includes.h"

//TODO: Why can't we use a #define around _Static_assert????
#define StaticAssert(condition) _Static_assert(condition)
#if USING_CUSTOM_STDLIB
#define AssertMsg(condition, message) if (!(condition)) { assert_msg(condition, (message)); }
#else
#define AssertMsg(condition, message) if (!(condition)) { assert(condition); }
#endif
#define Assert(condition) AssertMsg(condition, "")
#define NotNull(variable) Assert(variable != nullptr)

#if DEBUG_BUILD
#define DebugStaticAssert(condition) _Static_assert(condition)
#define DebugAssertMsg(condition, message) AssertMsg(condition, message)
#define DebugAssert(condition) Assert(condition)
#define DebugNotNull(variable) NotNull(variable)
#else
#define DebugStaticAssert(condition) //nothing
#define DebugAssertMsg(condition, message) //nothing
#define DebugAssert(condition) //nothing
#define DebugNotNull(variable) //nothing
#endif

#endif //  _BASE_ASSERT_H
