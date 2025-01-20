/*
File:   mem_scratch.h
Author: Taylor Robbins
Date:   01\06\2025
Description:
	** Scratch arenas are a thread_local collection of 3 arenas of Stack type that
	** all code has access to use implicitly. The expectation is that any code that
	** allocates from the scratch arena takes a mark before doing so and restores
	** to that mark when it's done. In an application with an update loop, there
	** can be a catch-all mark taken at the beginning of the loop that's restored
	** at the end, relieving the requirement code run inside the loop to worry about
	** restoring the mark.
	** NOTE: Any algorithm that allocates for every iteration of a loop might wish
	** to take and restore the mark at the beginning\end of the loop so that the
	** memory usage of the algorithm doesn't grow with the length of the loop.
*/

//There are 3 ways to get a scratch arena:
// 1. GetScratch# - Returns an Arena* but also has an optional uxx* markOut where you should store the mark and call ArenaResetToMark with later
// 2. GetScratchArena# - Returns a ScratchArena which has both Arena* and mark together. Pass to ReleaseScratchArena when done. Use result.arena when passing as a raw Arena*
// 3. ScratchBegin# - A macro for defining both an Arena* variable and a mark in the current scope. Call ScratchEnd in this scope with the same variableName when done.

#ifndef _MEM_SCRATCH_H
#define _MEM_SCRATCH_H

#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "mem/mem_arena.h"

#ifndef SCRATCH_ARENAS_THREAD_LOCAL
#define SCRATCH_ARENAS_THREAD_LOCAL 1
#endif

#define NUM_SCRATCH_ARENAS_PER_THREAD   3

#if SCRATCH_ARENAS_THREAD_LOCAL
thread_local Arena scratchArenasArray[NUM_SCRATCH_ARENAS_PER_THREAD] = ZEROED;
#else
Arena scratchArenasArray[NUM_SCRATCH_ARENAS_PER_THREAD] = ZEROED;
#endif

//NOTE: Init needs to be called once for each thread!

void InitScratchArenas(uxx stackSizePerArena, Arena* sourceArena)
{
	for (uxx aIndex = 0; aIndex < NUM_SCRATCH_ARENAS_PER_THREAD; aIndex++)
	{
		InitArenaStack(&scratchArenasArray[aIndex], stackSizePerArena, sourceArena);
	}
}
//TODO: Add an option for StackPaged once that's implemented
void InitScratchArenasVirtual(uxx virtualSizePerArena)
{
	for (uxx aIndex = 0; aIndex < NUM_SCRATCH_ARENAS_PER_THREAD; aIndex++)
	{
		InitArenaStackVirtual(&scratchArenasArray[aIndex], virtualSizePerArena);
	}
}

typedef struct ScratchArena ScratchArena;
struct ScratchArena
{
	Arena* arena;
	uxx mark;
};

Arena* GetScratch2(Arena* conflict1, Arena* conflict2, uxx* markOut)
{
	Arena* result = (conflict1 != &scratchArenasArray[0] && conflict2 != &scratchArenasArray[0])
		? &scratchArenasArray[0]
		: (conflict1 != &scratchArenasArray[1] && conflict2 != &scratchArenasArray[1])
			? &scratchArenasArray[1]
			: &scratchArenasArray[2];
	SetOptionalOutPntr(markOut, ArenaGetMark(result));
	return result;
}
Arena* GetScratch1(Arena* conflict1, uxx* markOut) { return GetScratch2(conflict1, nullptr, markOut); }
Arena* GetScratch(uxx* markOut) { return GetScratch2(nullptr, nullptr, markOut); }

ScratchArena GetScratchArena2(Arena* conflict1, Arena* conflict2)
{
	ScratchArena result;
	result.arena = GetScratch2(conflict1, conflict2, &result.mark);
	return result;
}
ScratchArena GetScratchArena1(Arena* conflict1) { return GetScratchArena2(conflict1, nullptr); }
ScratchArena GetScratchArena() { return GetScratchArena2(nullptr, nullptr); }

void ReleaseScratchArena(ScratchArena* scratchArena)
{
	NotNull(scratchArena->arena);
	ArenaResetToMark(scratchArena->arena, scratchArena->mark);
	ClearPointer(scratchArena);
}

#define ScratchBegin2(variableName, conflict1, conflict2) uxx variableName##_mark = 0; Arena* variableName = GetScratch2((conflict1), (conflict2), &variableName##_mark)
#define ScratchBegin1(variableName, conflict1) ScratchBegin2(variableName, (conflict1), nullptr)
#define ScratchBegin(variableName) ScratchBegin2(variableName, nullptr, nullptr)
#define ScratchEnd(variableName) do { ArenaResetToMark(variableName, variableName##_mark); variableName = nullptr; variableName##_mark = 0; } while(0)

#endif //  _MEM_SCRATCH_H
