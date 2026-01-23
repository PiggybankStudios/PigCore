/*
File:   cross_threading_and_mem_arena.h
Author: Taylor Robbins
Date:   01\20\2026
*/

#ifndef _CROSS_THREADING_AND_MEM_ARENA_H
#define _CROSS_THREADING_AND_MEM_ARENA_H

//NOTE: Intentionally no includes here

#if TARGET_HAS_THREADING

//The thread name serves two purposes
//  1. We tell the operating system what name we prefer to see in the debugger or other inspection tools
//  2. Internally we can print out the name of the current thread in debug messages or similar logging
//TODO: We cannot get the name of any thread from any other thread, we only have a thread local variable that holds the current threads name. We may want to make some sort of lookup table that all threads can access
#if !PIG_CORE_IMPLEMENTATION
extern THREAD_LOCAL Str8 CurrentThreadName;
#else
THREAD_LOCAL Str8 CurrentThreadName = Str8_Empty;
#endif

#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void OsSetThreadName(Arena* arena, Str8 name);
#endif

#if PIG_CORE_IMPLEMENTATION

PEXPI void OsSetThreadName(Arena* arena, Str8 name)
{
	if (arena != nullptr && CurrentThreadName.chars != nullptr && CanArenaFree(arena))
	{
		FreeStr8WithNt(arena, &CurrentThreadName);
	}
	
	if (!IsEmptyStr(name))
	{
		Assert(arena != nullptr || name.chars[name.length] == '\0'); //Ensure it's null-terminated if it's not gonna get allocated by this function
		CurrentThreadName = (arena != nullptr) ? AllocStrAndCopy(arena, name.length, name.chars, true) : name;
		NotNull(CurrentThreadName.chars);
		
		#if TARGET_IS_LINUX
		{
			char oldChar = '\0';
			if (CurrentThreadName.length >= 16)
			{
				oldChar = CurrentThreadName.chars[15];
				CurrentThreadName.chars[15] = '\0';
			}
			//NOTE: OsGetCurrentThreadId did not work here? Caused a segmentation fault, but pthread_self does not
			int setThreadNameResult = pthread_setname_np(pthread_self(), CurrentThreadName.chars);
			Assert(setThreadNameResult == 0);
			if (CurrentThreadName.length >= 16)
			{
				CurrentThreadName.chars[15] = oldChar;
			}
		}
		#endif //TARGET_IS_LINUX
		
		#if PROFILING_ENABLED
		TracyCSetThreadName(CurrentThreadName.chars);
		#endif
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //TARGET_HAS_THREADING

#endif //  _CROSS_THREADING_AND_MEM_ARENA_H
