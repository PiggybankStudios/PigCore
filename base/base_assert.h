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

#include <assert.h>

#define AssertMsg(condition, message) if (!(condition)) { assert(condition); }
#define Assert(condition) AssertMsg(condition, "")

#define NotNull(variable) Assert(variable != nullptr)

#endif //  _BASE_ASSERT_H
