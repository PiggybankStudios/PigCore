/*
File:   os_thread_pool.h
Author: Taylor Robbins
Date:   12\29\2025
*/

#ifndef _OS_THREAD_POOL_H
#define _OS_THREAD_POOL_H

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "base/base_macros.h"
#include "std/std_includes.h"
#include "std/std_memset.h"
#include "os/os_threading.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "struct/struct_bkt_array.h"
#include "misc/misc_result.h"
#include "lib/lib_tracy.h"
#include "base/base_debug_output.h"

//TODO: Add support for prioritizing certain work items over others (and updating those priorities actively on the main thread in a safe way)

#if TARGET_HAS_THREADING

#define THREAD_POOL_ID_INVALID 0

typedef plex ThreadPoolThread ThreadPoolThread;
plex ThreadPoolThread
{
	uxx id;
	plex ThreadPool* pool;
	uxx index;
	Str8 debugName;
	
	ThreadId osId;
	#if TARGET_IS_WINDOWS
	HANDLE osHandle;
	#endif
	
	bool isRunning;
	bool stopRequested;
	Result error;
	void* contextPntr;
};

#define THREAD_POOL_WORK_ITEM_FUNC_DEF(functionName) Result functionName(ThreadPoolThread* thread, plex ThreadPoolWorkItem* workItem)
typedef THREAD_POOL_WORK_ITEM_FUNC_DEF(ThreadPoolWorkItemFunc_f);

typedef plex ThreadPoolWorkItem ThreadPoolWorkItem;
plex ThreadPoolWorkItem
{
	uxx id;
	
	ThreadPoolWorkItemFunc_f* function;
	uxx subjectId;
	uxx subjectSize;
	void* subjectPntr;
	
	bool isWorking;
	uxx workerThreadId;
	Result result;
};

//NOTE: A ThreadPool should not be moved to a different location in memory because ThreadPoolThreads store a pointer back to their pool (we only get one contextPntr to pass to the thread main so we need some way to find the pool when all we have is the thread)
typedef plex ThreadPool ThreadPool;
plex ThreadPool
{
	Arena* arena;
	Str8 debugName;
	ThreadId mainThreadId;
	bool threadsHaveScratch;
	bool threadScratchIsVirtual;
	uxx threadScratchSize;
	
	uxx nextThreadId;
	BktArray threads; //ThreadPoolThread
	
	uxx nextWorkItemId;
	BktArray workQueue; //ThreadPoolWorkItem
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeThreadPoolThread(ThreadPool* pool, ThreadPoolThread* thread);
	PIG_CORE_INLINE void FreeThreadPool(ThreadPool* pool);
	PIG_CORE_INLINE void InitThreadPool(Arena* arena, Str8 debugName, bool threadsHaveScratch, bool threadScratchIsVirtual, uxx threadScratchSize, ThreadPool* poolOut);
	ThreadPoolThread* StartThreadPoolThread(ThreadPool* pool, void* contextPntr);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

#if TARGET_IS_WINDOWS
DWORD ThreadPoolThread_Main(LPVOID contextPntr);
#else
void ThreadPoolThread_Main(void* contextPntr);
#endif

PEXPI void FreeThreadPoolThread(ThreadPool* pool, ThreadPoolThread* thread)
{
	NotNull(pool);
	NotNull(pool->arena);
	NotNull(thread);
	FreeStr8WithNt(pool->arena, &thread->debugName);
	ClearPointer(thread);
}

PEXPI void FreeThreadPool(ThreadPool* pool)
{
	NotNull(pool);
	if (pool->arena != nullptr)
	{
		FreeStr8(pool->arena, &pool->debugName);
		for (uxx tIndex = 0; tIndex < pool->threads.length; tIndex++)
		{
			ThreadPoolThread* thread = BktArrayGet(ThreadPoolThread, &pool->threads, tIndex);
			FreeThreadPoolThread(pool, thread);
		}
		FreeBktArray(&pool->threads);
		FreeBktArray(&pool->workQueue);
	}
	ClearPointer(pool);
}

PEXPI void InitThreadPool(Arena* arena, Str8 debugName, bool threadsHaveScratch, bool threadScratchIsVirtual, uxx threadScratchSize, ThreadPool* poolOut)
{
	NotNull(arena);
	NotNull(poolOut);
	ClearPointer(poolOut);
	poolOut->arena = arena;
	poolOut->debugName = AllocStr8(arena, debugName);
	poolOut->mainThreadId = OsGetCurrentThreadId();
	poolOut->threadsHaveScratch = threadsHaveScratch;
	poolOut->threadScratchIsVirtual = threadScratchIsVirtual;
	poolOut->threadScratchSize = threadScratchSize;
	poolOut->nextThreadId = 1;
	InitBktArray(ThreadPoolThread, &poolOut->threads, arena, 8);
	poolOut->nextWorkItemId = 1;
	InitBktArray(ThreadPoolWorkItem, &poolOut->workQueue, arena, 32);
}

PEXP ThreadPoolThread* StartThreadPoolThread(ThreadPool* pool, void* contextPntr)
{
	NotNull(pool);
	NotNull(pool->arena);
	Assert(OsGetCurrentThreadId() == pool->mainThreadId);

	ThreadPoolThread* newThread = BktArrayAdd(ThreadPoolThread, &pool->threads);
	NotNull(newThread);
	ClearPointer(newThread);
	newThread->index = pool->threads.length-1;
	newThread->id = pool->nextThreadId;
	pool->nextThreadId++;
	newThread->debugName = PrintInArenaStr(pool->arena, "%.*s[%llu]", StrPrint(pool->debugName), newThread->index);
	newThread->pool = pool;
	newThread->contextPntr = contextPntr;
	
	newThread->isRunning = false;
	
	#if TARGET_IS_WINDOWS
	{
		newThread->osHandle = CreateThread(
			nullptr,               //lpThreadAttributes
			0,                     //dwStackSize
			ThreadPoolThread_Main, //lpStartAddress (LPTHREAD_START_ROUTINE)
			(LPVOID)newThread,     //lpParameter,
			0,                     //dwCreationFlags (CREATE_SUSPENDED|STACK_SIZE_PARAM_IS_A_RESERVATION)
			&newThread->osId       //lpThreadId
		);
		
		if (newThread->osHandle == NULL)
		{
			DWORD errorCode = GetLastError();
			PrintLine_E("Failed to start ThreadPoolThread! CreateThread error: %s", Win32_GetErrorCodeStr(errorCode));
			BktArrayRemoveAt(ThreadPoolThread, &pool->threads, pool->threads.length-1);
			return nullptr;
		}
	}
	// #elif TARGET_IS_LINUX
	//TODO: Implement me!
	// #elif TARGET_IS_OSX
	//TODO: Implement me!
	#else
	AssertMsg(false, "StartThreadPoolThread does not support the current platform yet!");
	#endif
	
	//TODO: We could wait for isRunning to become true before continuing?
	
	return newThread;
}

// +--------------------------------------------------------------+
// |                    ThreadPoolThread_Main                     |
// +--------------------------------------------------------------+
#if TARGET_IS_WINDOWS
DWORD ThreadPoolThread_Main(LPVOID contextPntr)
#else
void ThreadPoolThread_Main(void* contextPntr)
#endif
{
	ThreadPoolThread* thread = (ThreadPoolThread*)contextPntr;
	thread->isRunning = true;
	
	#if PROFILING_ENABLED
	TracyCSetThreadName(thread->debugName.chars);
	#endif
	
	#if SCRATCH_ARENAS_THREAD_LOCAL
	if (thread->pool->threadsHaveScratch)
	{
		if (thread->pool->threadScratchIsVirtual)
		{
			InitScratchArenasVirtual(thread->pool->threadScratchSize);
		}
		else
		{
			InitScratchArenas(thread->pool->threadScratchSize, thread->pool->arena);
		}
	}
	#endif
	
	PrintLine_N("Thread[%llu] id:%llu is starting!", thread->index, thread->id);
	uxx iterIndex = 0;
	while (!thread->stopRequested)
	{
		//TODO: Implement me!
		PrintLine_D("Thread[%llu] id:%llu - iterating %llu...", thread->index, thread->id, iterIndex);
		iterIndex++;
		#if TARGET_IS_WINDOWS
		Sleep(1000);
		#endif
	}
	PrintLine_W("Thread[%llu] id:%llu is ending!", thread->index, thread->id);
	
	#if SCRATCH_ARENAS_THREAD_LOCAL
	if (thread->pool->threadsHaveScratch)
	{
		if (thread->pool->threadScratchIsVirtual)
		{
			FreeScratchArenasVirtual();
		}
		else
		{
			FreeScratchArenas(thread->pool->arena);
		}
	}
	#endif
	
	if (thread->error == Result_None)
	{
		if (thread->stopRequested) { thread->error = Result_Stopped; }
		else { thread->error = Result_Finished; }
	}
	
	thread->isRunning = false;
	
	#if TARGET_IS_WINDOWS
	return 0;
	#endif
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //TARGET_HAS_THREADING

#endif //  _OS_THREAD_POOL_H
