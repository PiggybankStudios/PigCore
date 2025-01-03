/*
File:   mem_arena.h
Author: Taylor Robbins
Date:   01\02\2025
Description:
	** Contains the arena type
*/

#ifndef _MEM_ARENA_H
#define _MEM_ARENA_H

#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "std/std_includes.h"
#include "std/std_malloc.h"
#include "std/std_memset.h"

#ifndef MEM_ARENA_DEBUG_NAMES
#define MEM_ARENA_DEBUG_NAMES DEBUG_BUILD
#endif

typedef enum arena_type_e arena_type_e;
enum arena_type_e
{
	arena_type_none = 0,
	arena_type_alias,
	arena_type_std_heap,
	arena_type_buffer,
	arena_type_funcs,
	arena_type_generic,
	arena_type_generic_paged,
	arena_type_stack,
	arena_type_stack_paged,
	arena_type_stack_virtual,
	arena_type_count,
};
//TODO: We should use piggen to generate this function!
const char* GetArenaTypeStr(arena_type_e arenaType)
{
	switch (arenaType)
	{
		case arena_type_none:          return "none";
		case arena_type_alias:         return "alias";
		case arena_type_std_heap:      return "std_heap";
		case arena_type_buffer:        return "buffer";
		case arena_type_funcs:         return "funcs";
		case arena_type_generic:       return "generic";
		case arena_type_generic_paged: return "generic_paged";
		case arena_type_stack:         return "stack";
		case arena_type_stack_paged:   return "stack_paged";
		case arena_type_stack_virtual: return "stack_virtual";
		default: return UNKNOWN_STR;
	};
}

typedef struct arena_s arena_s; //TODO: Generate this forward declaration automatically?
struct arena_s
{
	arena_type_e type;
	#if MEM_ARENA_DEBUG_NAMES
	const char* debugName;
	#endif
	u64 used;
	u64 size;
	struct arena_s* sourceArena;
	void* mainPntr;
	void* otherPntr;
};

void InitArenaStdHeap(arena_s* arenaOut)
{
	NotNull(arenaOut);
	ClearPointer(arenaOut);
	arenaOut->type = arena_type_std_heap;
	#if MEM_ARENA_DEBUG_NAMES
	arenaOut->debugName = "[std_heap]";
	#endif
}

void InitArenaStackVirtual(arena_s* arenaOut, u64 virtualSize)
{
	NotNull(arenaOut);
	ClearPointer(arenaOut);
	arenaOut->type = arena_type_stack_virtual;
	#if MEM_ARENA_DEBUG_NAMES
	arenaOut->debugName = "[stack_virtual]";
	#endif
	//TODO: Call VirtualAlloc or mmap!
	UNUSED(virtualSize);
}

//TODO: CanVerify

//TODO: CanFree

//TODO: CanGetSize

//TODO: CanPushAndPop

//TODO: IsPntrInside

//TODO: Alloc

//TODO: Free

//TODO: Realloc

//TODO: SoftGrowBegin
//TODO: SoftGrowEnd

//TODO: GetSize

//TODO: PushMark

//TODO: PopMark

//TODO: VerifyInegrity

#endif //  _MEM_ARENA_H
