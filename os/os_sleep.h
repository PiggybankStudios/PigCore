/*
File:   os_sleep.h
Author: Taylor Robbins
Date:   12\29\2025
Description:
	** Provides the OsSleepMs function that allows the current thread to wait until X milliseconds have passed
	** Generally this guarantees that X milliseconds OR MORE has passed, there's not a guarantee of an exact time for the thread to wake up (due to OS scheduler decisions)
*/

#ifndef _OS_SLEEP_H
#define _OS_SLEEP_H

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "std/std_includes.h"

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void OsSleepMs(uxx numMilliseconds);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void OsSleepMs(uxx numMilliseconds)
{
	#if TARGET_IS_WINDOWS
	{
		Sleep((DWORD)numMilliseconds);
	}
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX || TARGET_IS_ANDROID)
	{
		usleep(numMilliseconds * 1000);
	}
	#else
	UNUSED(numMilliseconds);
	AssertMsg(false, "OsSleepMs does not support the current platform yet!");
	#endif
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _OS_SLEEP_H
