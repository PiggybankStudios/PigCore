/*
File:   assert.h
Author: Taylor Robbins
Date:   01\10\2025
*/

#ifndef _ASSERT_H
#define _ASSERT_H

#include <internal/wasm_std_common.h>

#if TARGET_IS_WEB
#define assert(condition) do { if (!(condition)) { jsStdAssertFailure(__FILE__, __LINE__, __func__, #condition, nullptr); } } while(0)
//NOTE: assert_msg is not a standard function but we want to be able to pass a message to jsStdAssertFailure so we added this variant
#define assert_msg(condition, message) do { if (!(condition)) { jsStdAssertFailure(__FILE__, __LINE__, __func__, #condition, (message)); } } while(0)
#else
#error The current TARGET doesn't have an AssertFailure implementation!
#endif

#endif //  _ASSERT_H
