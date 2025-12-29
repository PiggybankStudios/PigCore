/*
File:   os_thread_pool.h
Author: Taylor Robbins
Date:   12\29\2025
Description:
	** TODO: Write this!
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
#include "struct/struct_string.h"
#include "struct/struct_bkt_array.h"
#include "struct/struct_work_subject.h"
#include "misc/misc_result.h"
#include "lib/lib_tracy.h"
#include "base/base_debug_output.h"

//TODO: Use a semaphore instead of a mutex for work claiming? Or maybe a lockless intrinsic like atomics?
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
};

#define THREAD_POOL_WORK_ITEM_FUNC_DEF(functionName) Result functionName(ThreadPoolThread* thread, plex ThreadPoolWorkItem* workItem)
typedef THREAD_POOL_WORK_ITEM_FUNC_DEF(ThreadPoolWorkItemFunc_f);

typedef plex ThreadPoolWorkItem ThreadPoolWorkItem;
plex ThreadPoolWorkItem
{
	uxx id;
	
	ThreadPoolWorkItemFunc_f* function;
	WorkSubject subject;
	
	bool isWorking;
	bool isDone;
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
	BktArray workItems; //ThreadPoolWorkItem
	Mutex workItemsMutex;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeThreadPoolThread(ThreadPool* pool, ThreadPoolThread* thread);
	PIG_CORE_INLINE void FreeThreadPoolWorkItem(ThreadPool* pool, ThreadPoolWorkItem* workItem);
	PIG_CORE_INLINE void FreeThreadPool(ThreadPool* pool);
	PIG_CORE_INLINE void InitThreadPool(Arena* arena, Str8 debugName, bool threadsHaveScratch, bool threadScratchIsVirtual, uxx threadScratchSize, ThreadPool* poolOut);
	ThreadPoolThread* AddThreadToPool(ThreadPool* pool);
	ThreadPoolWorkItem* AddWorkItemToThreadPool(ThreadPool* pool, ThreadPoolWorkItemFunc_f* workItemFunc, WorkSubject* subject);
	PIG_CORE_INLINE ThreadPoolWorkItem* GetFinishedThreadPoolWorkItem(ThreadPool* pool); //NOTE: Remember to call FreeThreadPoolWorkItem when done!
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

PEXPI void FreeThreadPoolWorkItem(ThreadPool* pool, ThreadPoolWorkItem* workItem)
{
	NotNull(pool);
	NotNull(workItem);
	FreeWorkSubject(&workItem->subject);
	ClearPointer(workItem);
	workItem->id = THREAD_POOL_ID_INVALID;
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
		for (uxx wIndex = 0; wIndex < pool->workItems.length; wIndex++)
		{
			ThreadPoolWorkItem* workItem = BktArrayGet(ThreadPoolWorkItem, &pool->workItems, wIndex);
			FreeThreadPoolWorkItem(pool, workItem);
		}
		FreeBktArray(&pool->workItems);
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
	InitBktArray(ThreadPoolWorkItem, &poolOut->workItems, arena, 32);
	InitMutex(&poolOut->workItemsMutex);
}

PEXP ThreadPoolThread* AddThreadToPool(ThreadPool* pool)
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

PEXP ThreadPoolWorkItem* AddWorkItemToThreadPool(ThreadPool* pool, ThreadPoolWorkItemFunc_f* workItemFunc, WorkSubject* subject)
{
	NotNull(pool);
	NotNull(pool->arena);
	NotNull(workItemFunc);
	ThreadPoolWorkItem* result = nullptr;
	LockMutex(&pool->workItemsMutex, TIMEOUT_FOREVER);
	{
		ThreadPoolWorkItem* openWorkItemSlot = nullptr;
		for (uxx wIndex = 0; wIndex < pool->workItems.length; wIndex++)
		{
			ThreadPoolWorkItem* workItem = BktArrayGet(ThreadPoolWorkItem, &pool->workItems, wIndex);
			if (workItem->id == THREAD_POOL_ID_INVALID)
			{
				openWorkItemSlot = workItem;
				break;
			}
		}
		result = openWorkItemSlot;
		if (result == nullptr)
		{
			result = BktArrayAdd(ThreadPoolWorkItem, &pool->workItems);
			NotNull(result);
			ClearPointer(result);
		}
		
		result->id = pool->nextWorkItemId;
		pool->nextWorkItemId++;
		result->function = workItemFunc;
		if (subject != nullptr) { MyMemCopy(&result->subject, subject, sizeof(WorkSubject)); }
		result->isWorking = false;
		result->isDone = false;
		result->workerThreadId = THREAD_POOL_ID_INVALID;
		result->result = Result_None;
	}
	UnlockMutex(&pool->workItemsMutex);
	return result;
}

//NOTE: Remember to call FreeThreadPoolWorkItem on the item when the result has been processed, otherwise the workItems array will get very long!
PEXPI ThreadPoolWorkItem* GetFinishedThreadPoolWorkItem(ThreadPool* pool)
{
	NotNull(pool);
	NotNull(pool->arena);
	for (uxx wIndex = 0; wIndex < pool->workItems.length; wIndex++)
	{
		ThreadPoolWorkItem* workItem = BktArrayGet(ThreadPoolWorkItem, &pool->workItems, wIndex);
		if (workItem->id != THREAD_POOL_ID_INVALID && workItem->isDone)
		{
			return workItem;
		}
	}
	return nullptr;
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
		TracyCZoneN(_scratchInitZone, "ScratchInit", true);
		if (thread->pool->threadScratchIsVirtual)
		{
			InitScratchArenasVirtual(thread->pool->threadScratchSize);
		}
		else
		{
			InitScratchArenas(thread->pool->threadScratchSize, thread->pool->arena);
		}
		TracyCZoneEnd(_scratchInitZone);
	}
	#endif
	
	PrintLine_N("%.*s (id=%llu) is starting!", StrPrint(thread->debugName), thread->id);
	uxx iterIndex = 0;
	while (!thread->stopRequested)
	{
		TracyCZoneN(_awakeZone, "Awake", true);
		ThreadPoolWorkItem* claimedWorkItem = nullptr;
		for (uxx wIndex = 0; wIndex < thread->pool->workItems.length; wIndex++)
		{
			ThreadPoolWorkItem* workItem = BktArrayGet(ThreadPoolWorkItem, &thread->pool->workItems, wIndex);
			if (workItem->id != THREAD_POOL_ID_INVALID && !workItem->isWorking && !workItem->isDone && workItem->workerThreadId == THREAD_POOL_ID_INVALID)
			{
				if (LockMutex(&thread->pool->workItemsMutex, 0))
				{
					// Double check the isWorking/isDone flags after locking the mutex.
					// This item may have been claimed by another thread while we were waiting for the mutex to lock
					if (!workItem->isWorking && !workItem->isDone && workItem->workerThreadId == THREAD_POOL_ID_INVALID)
					{
						claimedWorkItem = workItem;
						claimedWorkItem->isWorking = true;
						claimedWorkItem->workerThreadId = thread->id;
					}
					UnlockMutex(&thread->pool->workItemsMutex);
					break;
				}
			}
		}
		TracyCZoneEnd(_awakeZone);
		
		if (claimedWorkItem != nullptr)
		{
			TracyCZoneN(_workingZone, "Working", true);
			claimedWorkItem->result = claimedWorkItem->function(thread, claimedWorkItem);
			claimedWorkItem->isDone = true;
			claimedWorkItem->isWorking = false;
			TracyCZoneEnd(_workingZone);
		}
		
		if (claimedWorkItem == nullptr)
		{
			TracyCZoneN(_sleepZone, "Sleeping", true);
			// if ((iterIndex % 60) == 0) { PrintLine_D("Thread[%llu] id:%llu - sleeping %llu...", thread->index, thread->id, iterIndex); }
			iterIndex++;
			#if TARGET_IS_WINDOWS
			Sleep(100);
			#endif
			TracyCZoneEnd(_sleepZone);
		}
	}
	PrintLine_W("%.*s (id=%llu) is end!", StrPrint(thread->debugName), thread->id);
	
	#if SCRATCH_ARENAS_THREAD_LOCAL
	if (thread->pool->threadsHaveScratch)
	{
		TracyCZoneN(_scratchFreeZone, "ScratchFree", true);
		if (thread->pool->threadScratchIsVirtual)
		{
			FreeScratchArenasVirtual();
		}
		else
		{
			FreeScratchArenas(thread->pool->arena);
		}
		TracyCZoneEnd(_scratchFreeZone);
	}
	#endif
	
	if (thread->error == Result_None)
	{
		if (thread->stopRequested) { thread->error = Result_Stopped; }
		else { thread->error = Result_Success; }
	}
	
	thread->isRunning = false;
	
	#if TARGET_IS_WINDOWS
	return 0;
	#endif
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //TARGET_HAS_THREADING

#endif //  _OS_THREAD_POOL_H
