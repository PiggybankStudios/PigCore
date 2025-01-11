/*
File:   mem_arena.h
Author: Taylor Robbins
Date:   01\02\2025
Description:
	** Contains the arena type
*/

#ifndef _MEM_ARENA_H
#define _MEM_ARENA_H

#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "base/base_math.h"
#include "std/std_malloc.h"
#include "std/std_memset.h"
#include "os/os_virtual_mem.h"

#ifndef MEM_ARENA_DEBUG_NAMES
#define MEM_ARENA_DEBUG_NAMES DEBUG_BUILD
#endif

//TODO: MaxUsed limitation

#define ALLOC_FUNC_DEF(functionName)   void* functionName(uxx numBytes)
typedef ALLOC_FUNC_DEF(AllocFunc_f);
#define REALLOC_FUNC_DEF(functionName) void* functionName(void* allocPntr, uxx newSize)
typedef REALLOC_FUNC_DEF(ReallocFunc_f);
#define FREE_FUNC_DEF(functionName)    void  functionName(void* allocPntr)
typedef FREE_FUNC_DEF(FreeFunc_f);

typedef enum ArenaFlag ArenaFlag;
enum ArenaFlag
{
	ArenaFlag_None = 0x00,
	ArenaFlag_AllowFreeWithoutSize = 0x01,
	ArenaFlag_AssertOnFailedAlloc  = 0x02,
	ArenaFlag_SingleAlloc          = 0x04,
	ArenaFlag_AllowNullptrFree     = 0x08,
};

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
	ArenaType_StackWasm,
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
		case ArenaType_StackWasm:     return "StackWasm";
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
	u8 flags;
	
	uxx used;
	uxx committed;
	uxx size;
	uxx allocCount;
	
	Arena* sourceArena;
	void* mainPntr;
	void* otherPntr;
	AllocFunc_f* allocFunc;
	ReallocFunc_f* reallocFunc;
	FreeFunc_f* freeFunc;
};

NODISCARD void* AllocMem(Arena* arena, uxx numBytes);

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

void InitArenaStack(Arena* arenaOut, uxx stackSize, Arena* sourceArena)
{
	NotNull(arenaOut);
	NotNull(sourceArena);
	ClearPointer(arenaOut);
	arenaOut->type = ArenaType_Stack;
	#if MEM_ARENA_DEBUG_NAMES
	arenaOut->debugName = "[stack]";
	#endif
	arenaOut->mainPntr = AllocMem(sourceArena, stackSize);
	NotNull(arenaOut->mainPntr);
	arenaOut->size = stackSize;
}

void InitArenaStackVirtual(Arena* arenaOut, uxx virtualSize)
{
	NotNull(arenaOut);
	ClearPointer(arenaOut);
	arenaOut->type = ArenaType_StackVirtual;
	#if MEM_ARENA_DEBUG_NAMES
	arenaOut->debugName = "[stack_virtual]";
	#endif
	uxx osMemPageSize = OsGetMemoryPageSize();
	Assert(osMemPageSize > 0);
	if ((virtualSize % osMemPageSize) != 0)
	{
		//round up to the nearest whole page size
		virtualSize = ((virtualSize / osMemPageSize) + 1) * osMemPageSize;
	}
	arenaOut->mainPntr = OsReserveMemory(virtualSize);
	NotNull(arenaOut->mainPntr);
	arenaOut->size = virtualSize;
	arenaOut->committed = 0;
}

#if TARGET_IS_WASM
void InitArenaStackWasm(Arena* arenaOut)
{
	NotNull(arenaOut);
	ClearPointer(arenaOut);
	arenaOut->type = ArenaType_StackWasm;
	#if MEM_ARENA_DEBUG_NAMES
	arenaOut->debugName = "[stack_wasm]";
	#endif
	//NOTE: With our own std library implementation, malloc works a little differently, see wasm_std_malloc.c
	u8* heapBeginning = (u8*)MyMalloc(1);
	NotNull(heapBeginning);
	arenaOut->mainPntr = heapBeginning+1;
	//TODO: Technically we have a bit less than this amount since some amount of stuff was probably already allocated before this arena was initialized
	//NOTE: We use UINTXX_MAX here rather than WASM_MEMORY_MAX_SIZE because that constant is 1 more than u32 max value
	arenaOut->size = UINTXX_MAX;
	arenaOut->committed = 0;
}
#endif

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
		case ArenaType_Stack:        return true;
		// case ArenaType_StackPaged:   return true;
		case ArenaType_StackVirtual: return true;
		case ArenaType_StackWasm:    return true;
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
		case ArenaType_Stack:        return false;
		// case ArenaType_StackPaged:   return false;
		case ArenaType_StackVirtual: return false;
		case ArenaType_StackWasm:    return false;
		default: return false;
	}
}

bool CanArenaAllocAligned(const Arena* arena)
{
	DebugNotNull(arena);
	switch (arena->type)
	{
		case ArenaType_Alias: DebugNotNull(arena->sourceArena); return CanArenaAllocAligned(arena->sourceArena);
		case ArenaType_StdHeap:      return true;
		case ArenaType_Buffer:       return true;
		case ArenaType_Funcs:        return false;
		// case ArenaType_Generic: return true;
		// case ArenaType_GenericPaged: return true;
		case ArenaType_Stack:        return true;
		// case ArenaType_StackPaged: return true;
		case ArenaType_StackVirtual: return true;
		case ArenaType_StackWasm:    return true;
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
		case ArenaType_Stack:        return false;
		// case ArenaType_StackPaged:   return false;
		case ArenaType_StackVirtual: return false;
		case ArenaType_StackWasm:    return false;
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
		case ArenaType_Stack:        return true;
		// case ArenaType_StackPaged:   return true;
		case ArenaType_StackVirtual: return true;
		case ArenaType_StackWasm:    return true;
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
		case ArenaType_Stack:        return true;
		// case ArenaType_StackPaged:   return true;
		case ArenaType_StackVirtual: return true;
		case ArenaType_StackWasm:    return true;
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
		case ArenaType_Stack:        return true;
		// case ArenaType_StackPaged:   return true;
		case ArenaType_StackVirtual: return true;
		case ArenaType_StackWasm:    return true;
		default: return false;
	}
}

// +--------------------------------------------------------------+
// |            Arena IsPntrFromArena Implementations             |
// +--------------------------------------------------------------+
//TODO: This may be a bit misleading. If one arena sources it's memory from another one,
//      they will both say the allocation is from them.
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
		case ArenaType_Stack: return IsPntrWithin(arena->mainPntr, arena->size, allocPntr);
		// case ArenaType_StackPaged:   //TODO: Implement me!
		case ArenaType_StackVirtual: return IsPntrWithin(arena->mainPntr, arena->size, allocPntr);
		case ArenaType_StackWasm: return IsPntrWithin(arena->mainPntr, arena->size, allocPntr);
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
		case ArenaType_Alias:
		{
			if (IsFlagSet(arena->flags, ArenaFlag_SingleAlloc) && arena->allocCount >= 1) { AssertMsg(false, "Second allocation attempted from Buffer Arena with SingleAlloc flag!"); break; }
			DebugNotNull(arena->sourceArena);
			result = AllocMemAligned(arena->sourceArena, numBytes, alignmentOverride);
			arena->used = arena->sourceArena->used;
			arena->committed = arena->sourceArena->committed;
			arena->size = arena->sourceArena->size;
			arena->allocCount = arena->sourceArena->allocCount;
			if (result == nullptr && IsFlagSet(arena->flags, ArenaFlag_AssertOnFailedAlloc)) { AssertMsg(false, "Failed to allocate in Alias Arena!"); }
		} break;
		
		// +==============================+
		// |  ArenaType_StdHeap AllocMem  |
		// +==============================+
		case ArenaType_StdHeap:
		{
			if (IsFlagSet(arena->flags, ArenaFlag_SingleAlloc) && arena->allocCount >= 1) { AssertMsg(false, "Second allocation attempted from Buffer Arena with SingleAlloc flag!"); break; }
			uxx alignedNumBytes = numBytes + (alignment > 1 ? alignment-1 : 0);
			result = MyMalloc(alignedNumBytes);
			if (result == nullptr)
			{
				if (IsFlagSet(arena->flags, ArenaFlag_AssertOnFailedAlloc)) { AssertMsg(false, "Failed to allocate in StdHeap Arena!"); }
				break;
			}
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
			if (IsFlagSet(arena->flags, ArenaFlag_SingleAlloc) && arena->allocCount >= 1) { AssertMsg(false, "Second allocation attempted from Buffer Arena with SingleAlloc flag!"); break; }
			
			uxx currentMisalignment = (alignment > 1) ? (uxx)((size_t)((u8*)arena->mainPntr + arena->used) % alignment) : 0;
			uxx alignmentBytesNeeded = (currentMisalignment > 0) ? (alignment - currentMisalignment) : 0;
			uxx alignedNumBytes = numBytes + alignmentBytesNeeded;
			
			if (arena->used + alignedNumBytes <= arena->size)
			{
				result = (void*)((u8*)arena->mainPntr + arena->used + alignmentBytesNeeded);
				arena->used += alignedNumBytes;
				IncrementUXX(arena->allocCount);
			}
			else if (IsFlagSet(arena->flags, ArenaFlag_AssertOnFailedAlloc)) { AssertMsg(false, "Failed to allocate in Buffer Arena!"); }
		} break;
		
		// +==============================+
		// |   ArenaType_Funcs AllocMem   |
		// +==============================+
		case ArenaType_Funcs:
		{
			DebugAssert(alignment == 0);
			DebugNotNull(arena->allocFunc);
			result = arena->allocFunc(numBytes);
			if (result == nullptr)
			{
				if (IsFlagSet(arena->flags, ArenaFlag_AssertOnFailedAlloc)) { AssertMsg(false, "Failed to allocate in Funcs Arena!"); }
				break;
			}
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
		case ArenaType_Stack:
		{
			DebugNotNull(arena->mainPntr);
			
			uxx currentMisalignment = (alignment > 1) ? (uxx)((size_t)((u8*)arena->mainPntr + arena->used) % alignment) : 0;
			uxx alignmentBytesNeeded = (currentMisalignment > 0) ? (alignment - currentMisalignment) : 0;
			uxx alignedNumBytes = numBytes + alignmentBytesNeeded;
			
			if (arena->used + alignedNumBytes <= arena->size)
			{
				if (arena->used + alignedNumBytes <= arena->size)
				{
					result = (void*)((u8*)arena->mainPntr + arena->used);
					arena->used += alignedNumBytes;
					IncrementUXX(arena->allocCount);
				}
				else if (IsFlagSet(arena->flags, ArenaFlag_AssertOnFailedAlloc)) { AssertMsg(false, "Failed to allocate in ArenaType_Stack Arena!"); }
			}
			else if (IsFlagSet(arena->flags, ArenaFlag_AssertOnFailedAlloc)) { AssertMsg(false, "Failed to allocate in ArenaType_Stack Arena!"); }
		} break;
		
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
		case ArenaType_StackVirtual:
		{
			DebugNotNull(arena->mainPntr);
			uxx osMemPageSize = OsGetMemoryPageSize();
			Assert(osMemPageSize > 0);
			
			uxx currentMisalignment = (alignment > 1) ? (uxx)((size_t)((u8*)arena->mainPntr + arena->used) % alignment) : 0;
			uxx alignmentBytesNeeded = (currentMisalignment > 0) ? (alignment - currentMisalignment) : 0;
			uxx alignedNumBytes = numBytes + alignmentBytesNeeded;
			
			if (arena->used + alignedNumBytes <= arena->size)
			{
				if (arena->used + alignedNumBytes > arena->committed)
				{
					uxx numTotalPagesNeeded = CeilDivUXX(arena->used + alignedNumBytes, osMemPageSize);
					uxx numNewPagesNeeded = numTotalPagesNeeded - (arena->committed / osMemPageSize);
					OsCommitReservedMemory((u8*)arena->mainPntr + arena->committed, numNewPagesNeeded * osMemPageSize);
					arena->committed += (numNewPagesNeeded * osMemPageSize);
				}
				
				result = (void*)((u8*)arena->mainPntr + arena->used);
				arena->used += alignedNumBytes;
				IncrementUXX(arena->allocCount);
			}
			else if (IsFlagSet(arena->flags, ArenaFlag_AssertOnFailedAlloc)) { AssertMsg(false, "Failed to allocate in StackVirtual Arena!"); }
		} break;
		
		// +==============================+
		// | ArenaType_StackWasm AllocMem |
		// +==============================+
		case ArenaType_StackWasm:
		{
			DebugNotNull(arena->mainPntr);
			uxx osMemPageSize = OsGetMemoryPageSize();
			Assert(osMemPageSize > 0);
			
			uxx currentMisalignment = (alignment > 1) ? (uxx)((size_t)((u8*)arena->mainPntr + arena->used) % alignment) : 0;
			uxx alignmentBytesNeeded = (currentMisalignment > 0) ? (alignment - currentMisalignment) : 0;
			uxx alignedNumBytes = numBytes + alignmentBytesNeeded;
			
			if (arena->used + alignedNumBytes <= arena->size)
			{
				if (arena->used + alignedNumBytes > arena->committed)
				{
					uxx numNewBytesNeeded = (arena->used + alignedNumBytes) - arena->committed;
					u8* newCommittedArea = (u8*)MyMalloc(numNewBytesNeeded);
					AssertMsg(newCommittedArea != nullptr, "Ran out of WASM memory! Stdlib malloc() return nullptr!");
					if (newCommittedArea == nullptr) { return nullptr; }
					AssertMsg(newCommittedArea == arena->mainPntr + arena->committed, "WASM malloc did not return next chunk of memory sequentially! Someone else must have called malloc somewhere! You can only have one StackWasm arena active at a time and no-one can call std malloc besides that one arena!");
					arena->committed += numNewBytesNeeded;
				}
				
				result = (void*)((u8*)arena->mainPntr + arena->used);
				arena->used += alignedNumBytes;
				IncrementUXX(arena->allocCount);
			}
			else if (IsFlagSet(arena->flags, ArenaFlag_AssertOnFailedAlloc)) { AssertMsg(false, "Failed to allocate in ArenaType_StackWasm Arena!"); }
		} break;
		
		default:
		{
			AssertMsg(false, "Arena type does not have an AllocMem implementation!");
		} break;
	}
	
	return result;
}
NODISCARD void* AllocMem(Arena* arena, uxx numBytes) //pre-declared at top of file
{
	return AllocMemAligned(arena, numBytes, 0);
}

#define AllocType(type, arenaPntr)                  (type*)AllocMemAligned((arenaPntr), (uxx)sizeof(type),             (uxx)_Alignof(type))
#define AllocTypeUnaligned(type, arenaPntr)         (type*)AllocMem(       (arenaPntr), (uxx)sizeof(type),             0)
#define AllocArray(type, arenaPntr, count)          (type*)AllocMemAligned((arenaPntr), (uxx)(sizeof(type) * (count)), (uxx)_Alignof(type))
#define AllocArrayUnaligned(type, arenaPntr, count) (type*)AllocMem(       (arenaPntr), (uxx)(sizeof(type) * (count)), 0)

// +--------------------------------------------------------------+
// |                  Arena Free Implementations                  |
// +--------------------------------------------------------------+
void FreeMem(Arena* arena, void* allocPntr, uxx allocSize)
{
	DebugNotNull(arena);
	if (allocPntr == nullptr && !IsFlagSet(arena->flags, ArenaFlag_AllowNullptrFree)) { AssertMsg(allocPntr != nullptr, "Tried to free nullptr from Arena!"); return; }
	if (allocSize == 0 && !IsFlagSet(arena->flags, ArenaFlag_AllowFreeWithoutSize)) { AssertMsg(allocSize != 0, "Tried to free from Arena without size!"); return; }
	DebugNotNull(allocPntr);
	
	switch (arena->type)
	{
		// +=============================+
		// |   ArenaType_Alias FreeMem   |
		// +=============================+
		case ArenaType_Alias:
		{
			DebugNotNull(arena->sourceArena);
			FreeMem(arena->sourceArena, allocPntr, allocSize);
			arena->used = arena->sourceArena->used;
			arena->committed = arena->sourceArena->committed;
			arena->size = arena->sourceArena->size;
			arena->allocCount = arena->sourceArena->allocCount;
		} break;
		
		// +=============================+
		// |  ArenaType_StdHeap FreeMem  |
		// +=============================+
		case ArenaType_StdHeap:
		{
			//TODO: Is this going to complain for aligned allocations??
			MyFree(allocPntr);
			arena->used -= allocSize;
			Decrement(arena->allocCount);
		} break;
		
		// +=============================+
		// |  ArenaType_Buffer FreeMem   |
		// +=============================+
		case ArenaType_Buffer:
		{
			DebugNotNull(arena->mainPntr);
			Assert(IsSizedPntrWithin(arena->mainPntr, arena->size, allocPntr, allocSize));
			uxx allocIndex = (uxx)((u8*)allocPntr - (u8*)arena->mainPntr);
			if (allocSize > 0)
			{
				Assert(allocIndex + allocSize == arena->used);
				arena->used -= allocSize;
			}
			else
			{
				// If this arena has the AllowFreeWithoutSize then we are blindly trusting the pointer is pointing to the last allocation
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
			arena->used -= allocSize;
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
		case ArenaType_Stack:
		{
			DebugNotNull(arena->mainPntr);
			AssertMsg(allocSize > 0, "Stacks do not allowing freeing unless you know the size of the allocation!");
			Assert(IsSizedPntrWithin(arena->mainPntr, arena->size, allocPntr, allocSize));
			// The only case we support freeing is if you have the size AND you are the last allocation on the stack!
			// Even then you're not guaranteed to return to the same usage you had before allocating because alignment
			// requirements might have added a bit on the front
			if ((u8*)allocPntr == ((u8*)arena->mainPntr + arena->used - allocSize))
			{
				arena->used -= allocSize;
				Decrement(arena->allocCount);
			}
			else { AssertMsg(false, "Stacks do not allow arbitrary freeing! You can only free the LAST thing on the stack!"); }
		} break;
		
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
		//NOTE: Freeing on stacks is not fully supported. You are expected to use Marks instead to free memory
		case ArenaType_StackVirtual:
		{
			DebugNotNull(arena->mainPntr);
			AssertMsg(allocSize > 0, "Stacks do not allowing freeing unless you know the size of the allocation!");
			Assert(IsSizedPntrWithin(arena->mainPntr, arena->size, allocPntr, allocSize));
			// The only case we support freeing is if you have the size AND you are the last allocation on the stack!
			// Even then you're not guaranteed to return to the same usage you had before allocating because alignment
			// requirements might have added a bit on the front
			if ((u8*)allocPntr == ((u8*)arena->mainPntr + arena->used - allocSize))
			{
				arena->used -= allocSize;
				Decrement(arena->allocCount);
			}
			else { AssertMsg(false, "Stacks do not allow arbitrary freeing! You can only free the LAST thing on the stack!"); }
		} break;
		
		// +==============================+
		// | ArenaType_StackWasm FreeMem  |
		// +==============================+
		case ArenaType_StackWasm:
		{
			DebugNotNull(arena->mainPntr);
			AssertMsg(allocSize > 0, "Stacks do not allowing freeing unless you know the size of the allocation!");
			Assert(IsSizedPntrWithin(arena->mainPntr, arena->size, allocPntr, allocSize));
			// The only case we support freeing is if you have the size AND you are the last allocation on the stack!
			// Even then you're not guaranteed to return to the same usage you had before allocating because alignment
			// requirements might have added a bit on the front
			if ((u8*)allocPntr == ((u8*)arena->mainPntr + arena->used - allocSize))
			{
				arena->used -= allocSize;
				Decrement(arena->allocCount);
			}
			else { AssertMsg(false, "Stacks do not allow arbitrary freeing! You can only free the LAST thing on the stack!"); }
		} break;
		
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
//TODO: Should we have alignment option here?
NODISCARD void* ReallocMem(Arena* arena, void* allocPntr, uxx oldSize, uxx newSize)
{
	DebugNotNull(arena);
	NotNull(allocPntr);
	void* result = nullptr;
	
	// Degenerate cases where we either do nothing, Alloc, or Free
	if (oldSize == newSize) { return allocPntr; }
	if (oldSize == 0 && allocPntr == nullptr)
	{
		result = AllocMem(arena, newSize);
		return result;
	}
	if (newSize == 0)
	{
		FreeMem(arena, allocPntr, oldSize);
		return nullptr;
	}
	
	if (oldSize == 0 && !IsFlagSet(arena->flags, ArenaFlag_AllowFreeWithoutSize)) { AssertMsg(oldSize != 0, "Tried to Realloc in Arena without oldSize!"); return nullptr; }
	
	switch (arena->type)
	{
		// +==============================+
		// |  ArenaType_Alias ReallocMem  |
		// +==============================+
		case ArenaType_Alias:
		{
			DebugNotNull(arena->sourceArena);
			result = ReallocMem(arena->sourceArena, allocPntr, oldSize, newSize);
			if (result == nullptr && !IsFlagSet(arena->flags, ArenaFlag_AssertOnFailedAlloc)) { AssertMsg(result != nullptr, "Realloc in Alias Arena failed!"); break; }
		} break;
		
		// +==============================+
		// | ArenaType_StdHeap ReallocMem |
		// +==============================+
		case ArenaType_StdHeap:
		{
			result = MyRealloc(allocPntr, newSize);
			if (result == nullptr && !IsFlagSet(arena->flags, ArenaFlag_AssertOnFailedAlloc)) { AssertMsg(result != nullptr, "Realloc in StdHeap Arena failed!"); break; }
			if (result != nullptr)
			{
				if (newSize > oldSize) { arena->used += newSize - oldSize; }
				else { arena->used -= oldSize - newSize; }
			}
			else { arena->used -= oldSize; }
		} break;
		
		// +==============================+
		// | ArenaType_Buffer ReallocMem  |
		// +==============================+
		case ArenaType_Buffer:
		{
			
		} break;
		
		// +==============================+
		// |  ArenaType_Funcs ReallocMem  |
		// +==============================+
		// case ArenaType_Funcs:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		// +==============================+
		// | ArenaType_Generic ReallocMem |
		// +==============================+
		// case ArenaType_Generic:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		// +====================================+
		// | ArenaType_GenericPaged ReallocMem  |
		// +====================================+
		// case ArenaType_GenericPaged:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		// +==============================+
		// |  ArenaType_Stack ReallocMem  |
		// +==============================+
		// case ArenaType_Stack:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		// +==================================+
		// | ArenaType_StackPaged ReallocMem  |
		// +==================================+
		// case ArenaType_StackPaged:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		// +====================================+
		// | ArenaType_StackVirtual ReallocMem  |
		// +====================================+
		// case ArenaType_StackVirtual:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		// +================================+
		// | ArenaType_StackWasm ReallocMem |
		// +================================+
		// case ArenaType_StackWasm:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		default:
		{
			AssertMsg(false, "Arena type does not have a ReallocMem implementation!");
		} break;
	}
	
	return result;
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
		case ArenaType_Stack: return arena->used;
		// case ArenaType_StackPaged: //TODO: Implement me!
		case ArenaType_StackVirtual: return arena->used;
		case ArenaType_StackWasm: return arena->used;
		default: AssertMsg(false, "Arena type does not have a ArenaGetMark implementation!"); return 0;
	}
}
void ArenaResetToMark(Arena* arena, uxx mark)
{
	DebugNotNull(arena);
	Assert(CanArenaResetToMark(arena));
	switch (arena->type)
	{
		case ArenaType_Alias: DebugNotNull(arena->sourceArena); ArenaResetToMark(arena->sourceArena, mark); break;
		case ArenaType_Stack:
		{
			arena->used = mark;
			if (mark == 0) { arena->allocCount = 0; }
		} break;
		// case ArenaType_StackPaged: //TODO: Implement me!
		case ArenaType_StackVirtual:
		{
			//TODO: Do we want to uncommit committed pages?
			arena->used = mark;
			if (mark == 0) { arena->allocCount = 0; }
		} break;
		case ArenaType_StackWasm:
		{
			//NOTE: Memory usage for our WASM module is not actually going down. We can't ever really free memory in WASM
			arena->used = mark;
			if (mark == 0) { arena->allocCount = 0; }
		} break;
		default: AssertMsg(false, "Arena type does not have a ArenaResetToMark implementation!"); break;
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

#if defined(_MEM_ARENA_H) && defined(_STRUCT_STRING_H)
#include "cross/cross_mem_arena_and_string.h"
#endif
