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
#include "os/os_sleep.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "struct/struct_string.h"
#include "struct/struct_bkt_array.h"
#include "struct/struct_work_subject.h"
#include "misc/misc_result.h"
#include "lib/lib_tracy.h"
#include "base/base_debug_output.h"
#include "base/base_notifications.h"

//TODO: Use a semaphore instead of a mutex for work claiming? Or maybe a lockless intrinsic like atomics?
//TODO: Implement for Linux, OSX, and Android!
//TODO: Add support for prioritizing certain work items over others (and updating those priorities actively on the main thread in a safe way)
//TODO: Add a "dependency" system so that certain work items can wait for other items to finish before they begin (and somehow use the result of the work item?)

#if TARGET_HAS_THREADING

#define THREAD_POOL_ID_INVALID         0
#define THREAD_POOL_MAX_STOP_WAIT_TIME 1500 //ms
#define THREAD_POOL_SLEEP_INTERVAL     100 //ms

typedef plex ThreadPoolThread ThreadPoolThread;
plex ThreadPoolThread
{
	uxx id;
	plex ThreadPool* pool;
	uxx index;
	Str8 debugName;
	
	OsThreadHandle osThread;
	
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
	PIG_CORE_INLINE void StopAllThreadsInPool(ThreadPool* pool);
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

OS_THREAD_FUNC_DEF(ThreadPoolThread_Main);

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

PEXPI void StopAllThreadsInPool(ThreadPool* pool)
{
	bool waitingForThreadsToStop = false;
	for (uxx tIndex = 0; tIndex < pool->threads.length; tIndex++)
	{
		ThreadPoolThread* thread = BktArrayGet(ThreadPoolThread, &pool->threads, tIndex);
		if (thread->id != THREAD_POOL_ID_INVALID && thread->isRunning)
		{
			thread->stopRequested = true;
			waitingForThreadsToStop = true;
		}
	}
	
	if (waitingForThreadsToStop)
	{
		for (uxx tIndex = 0; tIndex < pool->threads.length; tIndex++)
		{
			ThreadPoolThread* thread = BktArrayGet(ThreadPoolThread, &pool->threads, tIndex);
			if (thread->id != THREAD_POOL_ID_INVALID)
			{
				//TODO: How do we join the thread more gracefully?
				bool printedToConsole = false;
				uxx numMillisecondsWaited = 0;
				//TODO: Can we make this a volatile read? To make sure the read happens on every iteration of the loop?
				while (thread->isRunning && numMillisecondsWaited < THREAD_POOL_MAX_STOP_WAIT_TIME)
				{
					if (!printedToConsole) { Print_D("Waiting for thread %llu to stop...", thread->id); printedToConsole = true; }
					OsSleepMs(10);
					numMillisecondsWaited += 10;
				}
				
				if (thread->isRunning && numMillisecondsWaited >= THREAD_POOL_MAX_STOP_WAIT_TIME)
				{
					NotifyPrint_E("Failed to stop thread %llu! (After waiting %llums) Dangerously terminating the thread!", thread->id, numMillisecondsWaited);
					thread->isRunning = false;
				}
				else { PrintLine_D("Stopped thread %llu!", thread->id); }
			}
		}
	}
	
	for (uxx tIndex = 0; tIndex < pool->threads.length; tIndex++)
	{
		ThreadPoolThread* thread = BktArrayGet(ThreadPoolThread, &pool->threads, tIndex);
		if (thread->id != THREAD_POOL_ID_INVALID)
		{
			OsCloseThread(&thread->osThread);
			FreeThreadPoolThread(pool, thread);
		}
	}
	BktArrayClear(&pool->threads, true);
}

PEXPI void FreeThreadPool(ThreadPool* pool)
{
	NotNull(pool);
	if (pool->arena != nullptr)
	{
		StopAllThreadsInPool(pool);
		FreeStr8(pool->arena, &pool->debugName);
		for (uxx tIndex = 0; tIndex < pool->threads.length; tIndex++)
		{
			ThreadPoolThread* thread = BktArrayGet(ThreadPoolThread, &pool->threads, tIndex);
			Assert(!thread->isRunning);
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
	newThread->osThread = OsCreateThread(ThreadPoolThread_Main, (void*)newThread, true);
	
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
//NOTE: This is declared above as well so it can be passed to OsCreateThread in AddThreadToPool
// DWORD ThreadPoolThread_Main(LPVOID contextPntr)
OS_THREAD_FUNC_DEF(ThreadPoolThread_Main)
{
	ThreadPoolThread* thread = (ThreadPoolThread*)contextPntr;
	thread->isRunning = true;
	
	#if PROFILING_ENABLED
	TracyCSetThreadName(thread->debugName.chars);
	#endif
	
	#if SCRATCH_ARENAS_THREAD_LOCAL
	if (thread->pool->threadsHaveScratch)
	{
		TracyCZoneN(Zone_ScratchInit, "ScratchInit", true);
		if (thread->pool->threadScratchIsVirtual)
		{
			InitScratchArenasVirtual(thread->pool->threadScratchSize);
		}
		else
		{
			InitScratchArenas(thread->pool->threadScratchSize, thread->pool->arena);
		}
		TracyCZoneEnd(Zone_ScratchInit);
	}
	#endif
	
	// PrintLine_N("%.*s (id=%llu) is starting!", StrPrint(thread->debugName), thread->id);
	uxx numSleeps = 0;
	while (!thread->stopRequested)
	{
		TracyCZoneN(Zone_Awake, "Awake", true);
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
		TracyCZoneEnd(Zone_Awake);
		
		if (claimedWorkItem != nullptr)
		{
			TracyCZoneN(Zone_Working, "Working", true);
			
			#if SCRATCH_ARENAS_THREAD_LOCAL
			Arena* scratch1 = nullptr; uxx scratch1_mark = 0;
			Arena* scratch2 = nullptr; uxx scratch2_mark = 0;
			Arena* scratch3 = nullptr; uxx scratch3_mark = 0;
			if (thread->pool->threadsHaveScratch)
			{
				scratch1 = GetScratch(&scratch1_mark);
				scratch2 = GetScratch1(scratch1, &scratch2_mark);
				scratch3 = GetScratch2(scratch1, scratch2, &scratch3_mark);
			}
			#endif //SCRATCH_ARENAS_THREAD_LOCAL
			
			claimedWorkItem->result = claimedWorkItem->function(thread, claimedWorkItem);
			claimedWorkItem->isDone = true;
			claimedWorkItem->isWorking = false;
			
			#if SCRATCH_ARENAS_THREAD_LOCAL
			if (thread->pool->threadsHaveScratch)
			{
				ArenaResetToMark(scratch3, scratch3_mark);
				ArenaResetToMark(scratch2, scratch2_mark);
				ArenaResetToMark(scratch1, scratch1_mark);
			}
			#endif //SCRATCH_ARENAS_THREAD_LOCAL
			
			TracyCZoneEnd(Zone_Working);
		}
		
		if (claimedWorkItem == nullptr)
		{
			TracyCZoneNC(Zone_Sleeping, "Sleeping", 0xFF333333UL, true);
			
			//TODO: Use a semaphore or atomic to help us get rid of the Sleep here!
			// if ((numSleeps % 60) == 0) { PrintLine_D("Thread[%llu] id:%llu - sleeping %llu...", thread->index, thread->id, numSleeps/60); }
			OsSleepMs(THREAD_POOL_SLEEP_INTERVAL);
			numSleeps++;
			
			TracyCZoneEnd(Zone_Sleeping);
		}
	}
	// PrintLine_W("%.*s (id=%llu) is ending!", StrPrint(thread->debugName), thread->id);
	
	#if SCRATCH_ARENAS_THREAD_LOCAL
	if (thread->pool->threadsHaveScratch)
	{
		TracyCZoneN(Zone_ScratchFree, "ScratchFree", true);
		if (thread->pool->threadScratchIsVirtual)
		{
			FreeScratchArenasVirtual();
		}
		else
		{
			FreeScratchArenas(thread->pool->arena);
		}
		TracyCZoneEnd(Zone_ScratchFree);
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
