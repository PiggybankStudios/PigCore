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
#include "std/std_includes.h"
#include "std/std_memset.h"
#include "os/os_error.h"
#include "lib/lib_tracy.h"

#if TARGET_HAS_THREADING

#define TIMEOUT_FOREVER UINTXX_MAX

#if TARGET_IS_WINDOWS
typedef DWORD ThreadId;
#elif (TARGET_IS_LINUX || TARGET_IS_OSX || TARGET_IS_ANDROID)
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
#elif (TARGET_IS_LINUX || TARGET_IS_OSX || TARGET_IS_ANDROID)
typedef pthread_mutex_t Mutex;
#else
#error TARGET does not have an implementation for Mutex
#endif

#if TARGET_IS_WINDOWS
#define OS_THREAD_FUNC_DEF(functionName) DWORD functionName(LPVOID contextPntr)
#else
#define OS_THREAD_FUNC_DEF(functionName) void functionName(void* contextPntr)
#endif
typedef OS_THREAD_FUNC_DEF(OsThreadFunc_f);

typedef plex OsThreadHandle OsThreadHandle;
plex OsThreadHandle
{
	ThreadId id;
	#if TARGET_IS_WINDOWS
	HANDLE handle;
	// #elif TARGET_IS_LINUX
	// //TODO: Implement me!
	// #elif TARGET_IS_OSX
	// //TODO: Implement me!
	// #elif TARGET_IS_ANDROID
	// //TODO: Implement me!
	#endif
};

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
	void OsCloseThread(OsThreadHandle* threadHandle);
	OsThreadHandle OsCreateThread(OsThreadFunc_f* threadFunc, void* contextPntr, bool startImmediately);
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
	#elif TARGET_IS_OSX
	{
		result = pthread_self();
	}
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX || TARGET_IS_ANDROID)
	{
		result = syscall(SYS_gettid); //TODO: This is technically pid_t which may not be pthread_t?
		//TODO: Should we do either of these instead?
		// result = pthread_self();
		// int returnCode = pthread_threadid_np(pthread_self(), &result);
		// Assert(returnCode == 0);
	}
	#else
	AssertMsg(false, "OsGetCurrentThreadId does not support the current platform yet!");
	#endif
	return result;
}

PEXPI bool OsIsMainThread() { return (OsGetCurrentThreadId() == MainThreadId); }

// +==============================+
// |       Mutex Functions        |
// +==============================+
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
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX || TARGET_IS_ANDROID)
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
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX || TARGET_IS_ANDROID)
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
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX || TARGET_IS_ANDROID)
	{
		//TODO: Test this code
		if (timeoutMs == TIMEOUT_FOREVER)
		{
			int lockResult = pthread_mutex_lock(mutexPntr);
			DebugAssert(lockResult == 0);
			return (lockResult == 0);
		}
		else if (timeoutMs == 0 || TARGET_IS_OSX)
		{
			int lockResult = pthread_mutex_trylock(mutexPntr);
			DebugAssert(lockResult == 0 || lockResult == EBUSY);
			return (lockResult == 0);
		}
		//NOTE: Apparently pthread_mutex_timedlock is not part of the POSIX standard, so it's not available on OSX's clang
		#if !TARGET_IS_OSX
		else
		{
			plex timespec absTimeout;
			clock_gettime(CLOCK_REALTIME, &absTimeout);
			absTimeout.tv_sec += (timeoutMs / Thousand(1));
			absTimeout.tv_nsec += (timeoutMs % Thousand(1)) * Million(1);
			if ((u64)absTimeout.tv_nsec >= Billion(1)) { absTimeout.tv_sec++; absTimeout.tv_nsec -= Billion(1); }
			int lockResult = pthread_mutex_timedlock(mutexPntr, &absTimeout);
			DebugAssert(lockResult == 0 || lockResult == ETIMEDOUT);
			return (lockResult == 0);
		}
		#else
		{ Assert(false); return true; }
		#endif
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
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX || TARGET_IS_ANDROID)
	{
		int unlockResult = pthread_mutex_unlock(mutexPntr);
		DebugAssert(unlockResult == 0);
	}
	#else
	AssertMsg(false, "UnlockMutex does not support the current platform yet!");
	#endif
}

// +==============================+
// |       Thread Functions       |
// +==============================+
PEXP void OsCloseThread(OsThreadHandle* threadHandle)
{
	NotNull(threadHandle);
	#if TARGET_IS_WINDOWS
	{
		if (threadHandle->handle != NULL)
		{
			//NOTE: TerminateThread is dangerous! If the thread is actively working on anything it could cause problems
			//  For example if it's allocating memory from the std heap it could leave the heap locked and no other thread can allocate memory after that point
			//  Generally we expect either the thread is already exited OR the thread is asleep and doing no real work
			// https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-terminatethread
			BOOL terminateResult = TerminateThread(threadHandle->handle, 1);
			if (terminateResult == 0)
			{
				DWORD errorCode = GetLastError();
				PrintLine_E("TerminateThread error: %s", Win32_GetErrorCodeStr(errorCode));
				Assert(terminateResult != 0);
			}
			BOOL closeResult = CloseHandle(threadHandle->handle);
			if (closeResult == 0)
			{
				DWORD errorCode = GetLastError();
				PrintLine_E("CloseHandle error: %s", Win32_GetErrorCodeStr(errorCode));
				Assert(closeResult != 0);
			}
		}
	}
	#else
	AssertMsg(false, "OsCloseThread does not support the current platform yet!");
	#endif
	ClearPointer(threadHandle);
}

PEXP OsThreadHandle OsCreateThread(OsThreadFunc_f* threadFunc, void* contextPntr, bool startImmediately)
{
	OsThreadHandle result = ZEROED;
	#if TARGET_IS_WINDOWS
	{
		result.handle = CreateThread(
			nullptr,                                   //lpThreadAttributes
			0,                                         //dwStackSize
			threadFunc,                                //lpStartAddress (LPTHREAD_START_ROUTINE)
			(LPVOID)contextPntr,                       //lpParameter,
			(startImmediately ? 0 : CREATE_SUSPENDED), //dwCreationFlags (CREATE_SUSPENDED|STACK_SIZE_PARAM_IS_A_RESERVATION)
			&result.id                                 //lpThreadId
		);
		if (result.handle == NULL)
		{
			DWORD errorCode = GetLastError();
			PrintLine_E("CreateThread error: %s", Win32_GetErrorCodeStr(errorCode));
			Assert(result.handle != NULL);
		}
	}
	#else
	AssertMsg(false, "OsCreateThread does not support the current platform yet!");
	#endif
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //TARGET_HAS_THREADING

#endif //  _OS_THREADING_H
