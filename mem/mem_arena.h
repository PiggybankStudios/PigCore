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

//TODO: Flag AllowFreeWithoutSize
//TODO: Flag AssertOnFailedAlloc
//TODO: Flag SingleAlloc
//TODO: Flag AllowNullptrFree
//TODO: MaxUsed limitation

#define ALLOC_FUNC_DEF(functionName)   void* functionName(uxx numBytes)
typedef ALLOC_FUNC_DEF(AllocFunc_f);
#define REALLOC_FUNC_DEF(functionName) void* functionName(void* allocPntr, uxx newSize)
typedef REALLOC_FUNC_DEF(ReallocFunc_f);
#define FREE_FUNC_DEF(functionName)    void  functionName(void* allocPntr)
typedef FREE_FUNC_DEF(FreeFunc_f);

typedef enum ArenaType ArenaType;
enum ArenaType
{
	ArenaType_None = 0,
	ArenaType_Alias,
	ArenaType_StdHeap,
	ArenaType_Buffer,
	ArenaType_Funcs,
	ArenaType_Generic,
	ArenaType_GenericPaged,
	ArenaType_Stack,
	ArenaType_StackPaged,
	ArenaType_StackVirtual,
	// ArenaType_FreeListArray, //TODO: We could have an arena that only accepts a particular size of allocations and therefore is faster at finding/freeing/verifying/etc.
	ArenaType_Count,
};
//TODO: We should use piggen to generate this function!
const char* GetArenaTypeStr(ArenaType arenaType)
{
	switch (arenaType)
	{
		case ArenaType_None:          return "None";
		case ArenaType_Alias:         return "Alias";
		case ArenaType_StdHeap:       return "StdHeap";
		case ArenaType_Buffer:        return "Buffer";
		case ArenaType_Funcs:         return "Funcs";
		case ArenaType_Generic:       return "Generic";
		case ArenaType_GenericPaged:  return "GenericPaged";
		case ArenaType_Stack:         return "Stack";
		case ArenaType_StackPaged:    return "StackPaged";
		case ArenaType_StackVirtual:  return "StackVirtual";
		default: return UNKNOWN_STR;
	};
}

typedef struct Arena Arena; //TODO: Generate this forward declaration automatically?
struct Arena
{
	ArenaType type;
	#if MEM_ARENA_DEBUG_NAMES
	const char* debugName;
	#endif
	uxx alignment;
	
	uxx used;
	uxx allocCount;
	uxx size;
	
	Arena* sourceArena;
	void* mainPntr;
	void* otherPntr;
	AllocFunc_f* allocFunc;
	ReallocFunc_f* reallocFunc;
	FreeFunc_f* freeFunc;
};

// +--------------------------------------------------------------+
// |                   Initialization Functions                   |
// +--------------------------------------------------------------+
void InitArenaStdHeap(Arena* arenaOut)
{
	NotNull(arenaOut);
	ClearPointer(arenaOut);
	arenaOut->type = ArenaType_StdHeap;
	#if MEM_ARENA_DEBUG_NAMES
	arenaOut->debugName = "[std_heap]";
	#endif
}

void InitArenaAlias(Arena* arenaOut, Arena* sourceArena)
{
	NotNull(arenaOut);
	NotNull(sourceArena);
	ClearPointer(arenaOut);
	arenaOut->type = ArenaType_Alias;
	#if MEM_ARENA_DEBUG_NAMES
	arenaOut->debugName = "[alias]";
	#endif
	arenaOut->sourceArena = sourceArena;
}

void InitArenaBuffer(Arena* arenaOut, void* bufferPntr, uxx bufferSize)
{
	NotNull(arenaOut);
	NotNull(bufferPntr);
	Assert(bufferSize > 0);
	ClearPointer(arenaOut);
	arenaOut->type = ArenaType_Buffer;
	#if MEM_ARENA_DEBUG_NAMES
	arenaOut->debugName = "[buffer]";
	#endif
	arenaOut->mainPntr = bufferPntr;
	arenaOut->size = bufferSize;
}

void InitArenaStackVirtual(Arena* arenaOut, uxx virtualSize)
{
	NotNull(arenaOut);
	ClearPointer(arenaOut);
	arenaOut->type = ArenaType_StackVirtual;
	#if MEM_ARENA_DEBUG_NAMES
	arenaOut->debugName = "[stack_virtual]";
	#endif
	//TODO: Call VirtualAlloc or mmap!
	UNUSED(virtualSize);
}

// +--------------------------------------------------------------+
// |                      Capability Queries                      |
// +--------------------------------------------------------------+
bool CanArenaCheckPntrFromArena(const Arena* arena)
{
	DebugNotNull(arena);
	switch (arena->type)
	{
		case ArenaType_Alias: DebugNotNull(arena->sourceArena); return CanArenaCheckPntrFromArena(arena->sourceArena);
		case ArenaType_StdHeap:      return false;
		case ArenaType_Buffer:       return true;
		case ArenaType_Funcs:        return false;
		// case ArenaType_Generic:      return true;
		// case ArenaType_GenericPaged: return true;
		// case ArenaType_Stack:        return true;
		// case ArenaType_StackPaged:   return true;
		case ArenaType_StackVirtual: return true;
		default: return false;
	}
}

bool CanArenaGetSize(const Arena* arena)
{
	DebugNotNull(arena);
	switch (arena->type)
	{
		case ArenaType_Alias: DebugNotNull(arena->sourceArena); return CanArenaGetSize(arena->sourceArena);
		case ArenaType_StdHeap:      return false;
		case ArenaType_Buffer:       return true;
		case ArenaType_Funcs:        return false;
		// case ArenaType_Generic:      return true;
		// case ArenaType_GenericPaged: return true;
		// case ArenaType_Stack:        return false;
		// case ArenaType_StackPaged:   return false;
		case ArenaType_StackVirtual: return false;
		default: return false;
	}
}

bool CanArenaAllocAligned(const Arena* arena)
{
	DebugNotNull(arena);
	switch (arena->type)
	{
		case ArenaType_Alias: DebugNotNull(arena->sourceArena); return CanArenaAllocAligned(arena->sourceArena);
		case ArenaType_StdHeap: return true;
		case ArenaType_Buffer: return true;
		case ArenaType_Funcs: return false;
		// case ArenaType_Generic: return true;
		// case ArenaType_GenericPaged: return true;
		// case ArenaType_Stack: return true;
		// case ArenaType_StackPaged: return true;
		case ArenaType_StackVirtual: return true;
		default: return false;
	}
}

bool CanArenaFree(const Arena* arena)
{
	DebugNotNull(arena);
	switch (arena->type)
	{
		case ArenaType_Alias: DebugNotNull(arena->sourceArena); return CanArenaFree(arena->sourceArena);
		case ArenaType_StdHeap:      return true;
		case ArenaType_Buffer:       return true;
		case ArenaType_Funcs:        return true;
		// case ArenaType_Generic:      return true;
		// case ArenaType_GenericPaged: return true;
		// case ArenaType_Stack:        return true;
		// case ArenaType_StackPaged:   return true;
		case ArenaType_StackVirtual: return false;
		default: return false;
	}
}

bool CanArenaResetToMark(const Arena* arena)
{
	DebugNotNull(arena);
	switch (arena->type)
	{
		case ArenaType_Alias: DebugNotNull(arena->sourceArena); return CanArenaResetToMark(arena->sourceArena);
		case ArenaType_StdHeap:      return false;
		case ArenaType_Buffer:       return false;
		case ArenaType_Funcs:        return false;
		// case ArenaType_Generic:      return true;
		// case ArenaType_GenericPaged: return true;
		// case ArenaType_Stack:        return false;
		// case ArenaType_StackPaged:   return false;
		case ArenaType_StackVirtual: return true;
		default: return false;
	}
}

bool CanArenaSoftGrow(const Arena* arena)
{
	DebugNotNull(arena);
	switch (arena->type)
	{
		case ArenaType_Alias: DebugNotNull(arena->sourceArena); return CanArenaSoftGrow(arena->sourceArena);
		case ArenaType_StdHeap:      return false;
		case ArenaType_Buffer:       return true;
		case ArenaType_Funcs:        return false;
		// case ArenaType_Generic:      return true;
		// case ArenaType_GenericPaged: return true;
		// case ArenaType_Stack:        return false;
		// case ArenaType_StackPaged:   return false;
		case ArenaType_StackVirtual: return true;
		default: return false;
	}
}

bool CanArenaVerifyIntegrity(const Arena* arena)
{
	DebugNotNull(arena);
	switch (arena->type)
	{
		case ArenaType_Alias: DebugNotNull(arena->sourceArena); return CanArenaVerifyIntegrity(arena->sourceArena);
		case ArenaType_StdHeap:      return false;
		case ArenaType_Buffer:       return false;
		case ArenaType_Funcs:        return false;
		// case ArenaType_Generic:      return true;
		// case ArenaType_GenericPaged: return true;
		// case ArenaType_Stack:        return true;
		// case ArenaType_StackPaged:   return true;
		case ArenaType_StackVirtual: return true;
		default: return false;
	}
}

// +--------------------------------------------------------------+
// |            Arena IsPntrFromArena Implementations             |
// +--------------------------------------------------------------+
bool IsPntrFromArena(const Arena* arena, const void* allocPntr)
{
	DebugNotNull(arena);
	Assert(CanArenaCheckPntrFromArena(arena));
	switch (arena->type)
	{
		case ArenaType_Alias: DebugNotNull(arena->sourceArena); return IsPntrFromArena(arena->sourceArena, allocPntr);
		case ArenaType_Buffer: return IsPntrWithin(arena->mainPntr, arena->size, allocPntr);
		// case ArenaType_Generic:      //TODO: Implement me!
		// case ArenaType_GenericPaged: //TODO: Implement me!
		// case ArenaType_Stack:        //TODO: Implement me!
		// case ArenaType_StackPaged:   //TODO: Implement me!
		case ArenaType_StackVirtual: return IsPntrWithin(arena->mainPntr, arena->size, allocPntr);
		default: return false;
	}
}

// +--------------------------------------------------------------+
// |                Arena GetSize Implementations                 |
// +--------------------------------------------------------------+
uxx GetAllocSize(const Arena* arena, const void* allocPntr)
{
	DebugNotNull(arena);
	NotNull(allocPntr);
	UNUSED(arena);
	UNUSED(allocPntr);
	//TODO: Implement me!
	return 0;
}

// +--------------------------------------------------------------+
// |               Arena Allocation Implementations               |
// +--------------------------------------------------------------+
NODISCARD void* AllocMemAligned(Arena* arena, uxx numBytes, uxx alignmentOverride)
{
	DebugNotNull(arena);
	
	void* result = nullptr;
	uxx alignment = (alignmentOverride != 0) ? alignmentOverride : arena->alignment;
	
	switch (arena->type)
	{
		// +==============================+
		// |   ArenaType_Alias AllocMem   |
		// +==============================+
		case ArenaType_Alias: DebugNotNull(arena->sourceArena); return AllocMemAligned(arena->sourceArena, numBytes, alignmentOverride);
		
		// +==============================+
		// |  ArenaType_StdHeap AllocMem  |
		// +==============================+
		case ArenaType_StdHeap:
		{
			uxx alignedNumBytes = numBytes + (alignment > 1 ? alignment-1 : 0);
			result = MyMalloc(alignedNumBytes);
			if (result == nullptr) { break; }
			arena->used += alignedNumBytes;
			IncrementUXX(arena->allocCount);
			if (alignment > 1)
			{
				uxx misalignment = (uxx)(((size_t)result) % (size_t)alignment);
				if (misalignment > 0)
				{
					DebugAssert(numBytes + (alignment - misalignment) <= alignedNumBytes); //TODO: Remove me once we verify the math is working out like we want
					result = (void*)((u8*)result + (alignment - misalignment));
				}
			}
		} break;
		
		// +==============================+
		// |  ArenaType_Buffer AllocMem   |
		// +==============================+
		case ArenaType_Buffer:
		{
			DebugNotNull(arena->mainPntr);
			//TODO: Check if SingleAlloc flag is enabled and fail if we already have 1 allocation
			
			uxx currentMisalignment = (alignment > 1) ? (uxx)((size_t)((u8*)arena->mainPntr + arena->used) % alignment) : 0;
			uxx alignmentBytesNeeded = (currentMisalignment > 0) ? (alignment - currentMisalignment) : 0;
			uxx alignedNumBytes = numBytes + alignmentBytesNeeded;
			
			if (arena->used + alignedNumBytes <= arena->size)
			{
				result = (void*)((u8*)arena->mainPntr + arena->used + alignmentBytesNeeded);
				arena->used += alignedNumBytes;
				IncrementUXX(arena->allocCount);
			}
		} break;
		
		// +==============================+
		// |   ArenaType_Funcs AllocMem   |
		// +==============================+
		case ArenaType_Funcs:
		{
			DebugAssert(alignment == 0);
			DebugNotNull(arena->allocFunc);
			result = arena->allocFunc(numBytes);
			if (result == nullptr) { break; }
			arena->used += numBytes;
			IncrementUXX(arena->allocCount);
		} break;
		
		// +==============================+
		// |  ArenaType_Generic AllocMem  |
		// +==============================+
		// case ArenaType_Generic:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		// +==================================+
		// | ArenaType_GenericPaged AllocMem  |
		// +==================================+
		// case ArenaType_GenericPaged:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		// +==============================+
		// |   ArenaType_Stack AllocMem   |
		// +==============================+
		// case ArenaType_Stack:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		// +===============================+
		// | ArenaType_StackPaged AllocMem |
		// +===============================+
		// case ArenaType_StackPaged:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		// +==================================+
		// | ArenaType_StackVirtual AllocMem  |
		// +==================================+
		// case ArenaType_StackVirtual:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		default:
		{
			AssertMsg(false, "Arena type does not have an AllocMem implementation!");
		} break;
	}
	
	return result;
}
NODISCARD void* AllocMem(Arena* arena, uxx numBytes)
{
	return AllocMemAligned(arena, numBytes, 0);
}

// +--------------------------------------------------------------+
// |                  Arena Free Implementations                  |
// +--------------------------------------------------------------+
void FreeMem(Arena* arena, void* allocPntr, uxx size)
{
	DebugNotNull(arena);
	//TODO: Check AllowNullptrFree and skip this assertion if true
	DebugNotNull(allocPntr);
	
	switch (arena->type)
	{
		// +=============================+
		// |   ArenaType_Alias FreeMem   |
		// +=============================+
		case ArenaType_Alias: DebugNotNull(arena->sourceArena); FreeMem(arena->sourceArena, allocPntr, size); break;
		
		// +=============================+
		// |  ArenaType_StdHeap FreeMem  |
		// +=============================+
		case ArenaType_StdHeap:
		{
			//TODO: Check AllowFreeWithoutSize flag!
			//TODO: Is this going to complain for aligned allocations??
			MyFree(allocPntr);
			arena->used -= size;
			Decrement(arena->allocCount);
		} break;
		
		// +=============================+
		// |  ArenaType_Buffer FreeMem   |
		// +=============================+
		case ArenaType_Buffer:
		{
			DebugNotNull(arena->mainPntr);
			Assert(IsSizedPntrWithin(arena->mainPntr, arena->size, allocPntr, size));
			uxx allocIndex = (uxx)((u8*)allocPntr - (u8*)arena->mainPntr);
			if (size > 0)
			{
				Assert(allocIndex + size == arena->used);
				arena->used -= size;
			}
			else
			{
				//TODO: Check AllowFreeWithoutSize flag!
				arena->used = allocIndex;
			}
			Decrement(arena->allocCount);
		} break;
		
		// +=============================+
		// |   ArenaType_Funcs FreeMem   |
		// +=============================+
		case ArenaType_Funcs:
		{
			DebugNotNull(arena->freeFunc);
			arena->freeFunc(allocPntr);
			arena->used -= size;
			Decrement(arena->allocCount);
		} break;
		
		// +=============================+
		// |  ArenaType_Generic FreeMem  |
		// +=============================+
		// case ArenaType_Generic:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		// +=================================+
		// | ArenaType_GenericPaged FreeMem  |
		// +=================================+
		// case ArenaType_GenericPaged:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		// +=============================+
		// |   ArenaType_Stack FreeMem   |
		// +=============================+
		// case ArenaType_Stack:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		// +==============================+
		// | ArenaType_StackPaged FreeMem |
		// +==============================+
		// case ArenaType_StackPaged:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		// +=================================+
		// | ArenaType_StackVirtual FreeMem  |
		// +=================================+
		// case ArenaType_StackVirtual:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		default:
		{
			AssertMsg(false, "Arena type does not have an AllocMem implementation!");
		} break;
	}
}
void FreeMemNoSize(Arena* arena, void* allocPntr)
{
	FreeMem(arena, allocPntr, 0);
}

// +--------------------------------------------------------------+
// |                Arena Realloc Implementations                 |
// +--------------------------------------------------------------+
NODISCARD void* ReallocMem(Arena* arena, void* allocPntr, uxx oldSize, uxx newSize)
{
	DebugNotNull(arena);
	UNUSED(arena);
	UNUSED(allocPntr);
	UNUSED(oldSize);
	UNUSED(newSize);
	//TODO: Implement me!
	return nullptr;
}
NODISCARD void* ReallocMemNoOldSize(Arena* arena, void* allocPntr, uxx newSize)
{
	return ReallocMem(arena, allocPntr, 0, newSize);
}

// +--------------------------------------------------------------+
// |                Arena Push/Pop Implementations                |
// +--------------------------------------------------------------+
NODISCARD uxx ArenaGetMark(Arena* arena)
{
	DebugNotNull(arena);
	Assert(CanArenaResetToMark(arena));
	switch (arena->type)
	{
		case ArenaType_Alias: DebugNotNull(arena->sourceArena); return ArenaGetMark(arena->sourceArena);
		// case ArenaType_Stack: //TODO: Implement me!
		// case ArenaType_StackPaged: //TODO: Implement me!
		case ArenaType_StackVirtual: return arena->used;
		default: Assert(false); return 0;
	}
}
void ArenaResetToMark(Arena* arena, uxx mark)
{
	DebugNotNull(arena);
	Assert(CanArenaResetToMark(arena));
	switch (arena->type)
	{
		case ArenaType_Alias: DebugNotNull(arena->sourceArena); ArenaResetToMark(arena->sourceArena, mark); break;
		// case ArenaType_Stack: //TODO: Implement me!
		// case ArenaType_StackPaged: //TODO: Implement me!
		case ArenaType_StackVirtual: arena->used = mark; break;
		default: Assert(false); break;
	}
}

// +--------------------------------------------------------------+
// |                Arena SoftGrow Implementations                |
// +--------------------------------------------------------------+
//NOTE: SoftGrowing is useful when performing argument formatting on the end of a string,
// you can call SoftGrowBegin, if there is space available you attempt a call to printf
// style function with that size as the max. If the printf fails (or if there was no
// space to begin with) then you ReallocMem with the appropriate space to fit both existing
// chars and new printf size. This is especially efficient for StringBuilder when repeatedly
// formatting strings and appending on the end when the builder is allocated at the end
// of an arena, or esp. when it's in a stack where reallocation is wasteful.

// Returns the amount of room that is available immediately following the allocation
// Any value <= to the returned value here can be passed to SoftGrowEnd and is guaranteed to succeed
// (assuming no new allocations happened in-between soft grow begin/end)
uxx ArenaSoftGrowBegin(const Arena* arena, const void* allocPntr, uxx allocSize)
{
	DebugNotNull(arena);
	Assert(CanArenaSoftGrow(arena));
	UNUSED(arena);
	UNUSED(allocPntr);
	UNUSED(allocSize);
	//TODO: Implement this!
	return 0;
}
// The size can be assumed in some arenas, by choosing to not pass a size you accept
// some arenas returning 0 because they cannot determine the size of the allocation
uxx ArenaSoftGrowBeginNoSize(const Arena* arena, const void* allocPntr)
{
	return ArenaSoftGrowBegin(arena, allocPntr, 0);
}

// This will Assert if the newSpaceUsed value is invalid!
void ArenaSoftGrowEnd(Arena* arena, void* allocPntr, uxx allocSize, uxx newSpaceUsed)
{
	DebugNotNull(arena);
	UNUSED(arena);
	UNUSED(allocPntr);
	UNUSED(allocSize);
	UNUSED(newSpaceUsed);
	//TODO: Implement me!
}
void ArenaSoftGrowEndNoSize(Arena* arena, void* allocPntr, uxx newSpaceUsed)
{
	ArenaSoftGrowEnd(arena, allocPntr, 0, newSpaceUsed);
}

// +--------------------------------------------------------------+
// |            Arena VerifyIntegrity Implementations             |
// +--------------------------------------------------------------+
bool MemArenaVerifyIntegrity(Arena* arena, bool assertOnFailure)
{
	DebugNotNull(arena);
	UNUSED(arena);
	UNUSED(assertOnFailure);
	//TODO: Implement me!
	return false;
}

#endif //  _MEM_ARENA_H
