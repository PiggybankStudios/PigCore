/*
File:   cross_assert_and_debug_output.h
Author: Taylor Robbins
Date:   08\15\2026
*/

#ifndef _CROSS_ASSERT_AND_DEBUG_OUTPUT_H
#define _CROSS_ASSERT_AND_DEBUG_OUTPUT_H

//NOTE: Intentionally no includes here

//NOTE: This macro runs the condition twice on failure!
//      The formatStr MUST be a string literal
#define AssertFmt(condition, formatStr, ...) if (!(condition)) { PrintLine_E("Assert(" #condition ") Failed! Message:\n" formatStr, ##__VA_ARGS__); AssertMsg((condition), CheckStrLit(formatStr)) }
#if DEBUG_BUILD
#define DebugAssertFmt(condition, formatStr, ...) if (!(condition)) { PrintLine_E("Assert(" #condition ") Failed! Message:\n" formatStr, ##__VA_ARGS__); AssertMsg((condition), CheckStrLit(formatStr)) }
#else
#define DebugAssertFmt(condition, formatStr, ...) //nothing
#endif

#endif //  _CROSS_ASSERT_AND_DEBUG_OUTPUT_H
