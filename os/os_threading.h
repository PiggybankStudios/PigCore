/*
File:   os_threading.h
Author: Taylor Robbins
Date:   07\24\2025
Description:
	** Contains wrappers for threading-related types and functions
*/

#ifndef _OS_THREADING_H
#define _OS_THREADING_H

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "base/base_macros.h"
#include "misc/misc_profiling_tracy_include.h"

#if TARGET_HAS_THREADING

#define TIMEOUT_FOREVER UINTXX_MAX

#if TARGET_IS_WINDOWS
typedef DWORD ThreadId;
#elif (TARGET_IS_LINUX || TARGET_IS_OSX)
typedef pthread_t ThreadId;
#else
#error TARGET does not have an implementation for ThreadId
#endif

#if TARGET_IS_WINDOWS
	#if !PIG_CORE_IMPLEMENTATION
	extern __declspec(dllimport) ThreadId MainThreadId;
	#else
	__declspec(dllexport) ThreadId MainThreadId = ZEROED;
	#endif
#else
	#if !PIG_CORE_IMPLEMENTATION
	extern ThreadId MainThreadId;
	#else
	ThreadId MainThreadId = ZEROED;
	#endif
#endif

#if TARGET_IS_WINDOWS
typedef HANDLE Mutex;
#elif (TARGET_IS_LINUX || TARGET_IS_OSX)
typedef pthread_mutex_t Mutex;
#else
#error TARGET does not have an implementation for Mutex
#endif

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE ThreadId OsGetCurrentThreadId();
	PIG_CORE_INLINE bool OsIsMainThread();
	PIG_CORE_INLINE void InitMutex(Mutex* mutexPntr);
	PIG_CORE_INLINE void DestroyMutex(Mutex* mutexPntr);
	PIG_CORE_INLINE bool LockMutex(Mutex* mutexPntr, uxx timeoutMs);
	#if PROFILING_ENABLED
	PIG_CORE_INLINE bool LockMutexAndEndTracyZone(Mutex* mutexPntr, uxx timeoutMs, TracyCZoneCtx zone);
	#endif
	PIG_CORE_INLINE void UnlockMutex(Mutex* mutexPntr);
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define LockMutexBlock(mutexPntr, timeout) DeferIfBlockCondEnd(LockMutex((mutexPntr), (timeout)), UnlockMutex(mutexPntr))
#if PROFILING_ENABLED
#define LockMutexBlockWithTracyZone(mutexPntr, timeout, zoneName, zoneDisplayStr) TracyCZoneN(zoneName, zoneDisplayStr, true); DeferIfBlockCondEnd(LockMutexAndEndTracyZone((mutexPntr), (timeout), zoneName), UnlockMutex(mutexPntr))
#else
#define LockMutexBlockWithTracyZone(mutexPntr, timeout, zoneName, zoneDisplayStr) LockMutexBlock((mutexPntr), (timeout))
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI ThreadId OsGetCurrentThreadId()
{
	ThreadId result = ZEROED;
	#if TARGET_IS_WINDOWS
	{
		result = GetCurrentThreadId();
	}
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX)
	{
    	int returnCode = pthread_threadid_np(pthread_self(), &result);
    	Assert(returnCode == 0);
	}
	#else
	AssertMsg(false, "OsGetCurrentThreadId does not support the current platform yet!");
	#endif
	return result;
}

PEXPI bool OsIsMainThread() { return (OsGetCurrentThreadId() == MainThreadId); }

PEXPI void InitMutex(Mutex* mutexPntr)
{
	DebugNotNull(mutexPntr);
	#if TARGET_IS_WINDOWS
	{
		*mutexPntr = CreateMutexA(
			nullptr, //lpMutexAttributes
			false, //bInitialOwner
			nullptr //lpName TODO: Should we fill this out for debug purposes? Or maybe for actual functional purposes?
		);
		DebugAssert(*mutexPntr != NULL);
	}
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX)
	{
		//TODO: Test this code
		int initResult = pthread_mutex_init(mutexPntr, NULL);
		DebugAssert(initResult == 0);
	}
	#else
	AssertMsg(false, "InitMutex does not support the current platform yet!");
	#endif
}

PEXPI void DestroyMutex(Mutex* mutexPntr)
{
	DebugNotNull(mutexPntr);
	#if TARGET_IS_WINDOWS
	{
		DebugAssert(*mutexPntr != NULL);
		BOOL closeResult = CloseHandle(*mutexPntr);
		Assert(closeResult != 0);
		*mutexPntr = NULL;
	}
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX)
	{
		//TODO: Test this code
		int destroyResult = pthread_mutex_destroy(mutexPntr);
		DebugAssert(destroyResult == 0);
	}
	#else
	AssertMsg(false, "DestroyMutex does not support the current platform yet!");
	#endif
}

PEXPI bool LockMutex(Mutex* mutexPntr, uxx timeoutMs)
{
	DebugNotNull(mutexPntr);
	#if TARGET_IS_WINDOWS
	{
		DebugAssert(*mutexPntr != NULL);
		DWORD timeoutDword = (DWORD)timeoutMs;
		if (timeoutMs == TIMEOUT_FOREVER) { timeoutDword = INFINITE; }
		else { DebugAssert(timeoutMs <= UINT32_MAX); }
		DWORD lockResult = WaitForSingleObject(*mutexPntr, timeoutDword);
		DebugAssertMsg(timeoutDword != INFINITE || lockResult == WAIT_OBJECT_0, "Failed to lock mutex with INFINITE timeout!");
		return (lockResult == WAIT_OBJECT_0);
	}
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX)
	{
		//TODO: Test this code
		if (timeoutMs == 0)
		{
			int lockResult = pthread_mutex_trylock(mutexPntr);
			DebugAssert(lockResult == 0 || lockResult == EBUSY);
			return (lockResult == 0);
		}
		else if (timeoutMs == TIMEOUT_FOREVER)
		{
			int lockResult = pthread_mutex_lock(mutexPntr);
			DebugAssert(lockResult == 0);
			return (lockResult == 0);
		}
		else
		{
			plex timespec absTimeout;
			clock_gettime(CLOCK_REALTIME, &absTimeout);
			absTimeout.tv_sec += (timeoutMs / Thousand(1));
			absTimeout.tv_nsec += (timeoutMs % Thousand(1)) * Million(1);
			if (absTimeout.tv_nsec >= Billion(1)) { absTimeout.tv_sec++; absTimeout.tv_nsec -= Billion(1); }
			int lockResult = pthread_mutex_timedlock(mutexPntr, &absTimeout);
			DebugAssert(lockResult == 0 || lockResult == ETIMEDOUT);
			return (lockResult == 0);
		}
	}
	#else
	AssertMsg(false, "LockMutex does not support the current platform yet!");
	#endif
}

#if PROFILING_ENABLED
PEXPI bool LockMutexAndEndTracyZone(Mutex* mutexPntr, uxx timeoutMs, TracyCZoneCtx zone)
{
	bool result = LockMutex(mutexPntr, timeoutMs);
	TracyCZoneEnd(zone);
	return result;
}
#endif //PROFILING_ENABLED

PEXPI void UnlockMutex(Mutex* mutexPntr)
{
	#if TARGET_IS_WINDOWS
	{
		BOOL releaseResult = ReleaseMutex(*mutexPntr);
		DebugAssert(releaseResult != 0);
	}
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX)
	{
		int unlockResult = pthread_mutex_unlock(mutexPntr);
		DebugAssert(unlockResult == 0);
	}
	#else
	AssertMsg(false, "UnlockMutex does not support the current platform yet!");
	#endif
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //TARGET_HAS_THREADING

#endif //  _OS_THREADING_H
