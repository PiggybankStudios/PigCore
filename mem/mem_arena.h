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
#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "base/base_math.h"
#include "std/std_malloc.h"
#include "std/std_memset.h"
#include "os/os_virtual_mem.h"
#include "misc/misc_profiling_tracy_include.h"

#ifndef MEM_ARENA_DEBUG_NAMES
#define MEM_ARENA_DEBUG_NAMES DEBUG_BUILD
#endif

//TODO: MaxUsed limitation

#define ARENA_DEBUG_PADDING_SIZE  32 //bytes
#define ARENA_DEBUG_PADDING_VALUE 0xDA //bytes

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
	ArenaFlag_AddPaddingForDebug   = 0x10,
	ArenaFlag_DontPop              = 0x20,
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
#if !PIG_CORE_IMPLEMENTATION
PEXPI const char* GetArenaTypeStr(ArenaType arenaType);
#else
PIG_CORE_INLINE const char* GetArenaTypeStr(ArenaType arenaType)
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
#endif

typedef plex Arena Arena; //TODO: Generate this forward declaration automatically?
plex Arena
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

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void FreeArena(Arena* arena, Arena* sourceArena);
	void InitArenaStdHeap(Arena* arenaOut);
	void InitArenaFuncs(Arena* arenaOut, AllocFunc_f* allocFunc, FreeFunc_f* freeFunc, ReallocFunc_f* reallocFunc);
	void InitArenaAlias(Arena* arenaOut, Arena* sourceArena);
	void InitArenaBuffer(Arena* arenaOut, void* bufferPntr, uxx bufferSize);
	void InitArenaStack(Arena* arenaOut, uxx stackSize, Arena* sourceArena);
	void InitArenaStackVirtual(Arena* arenaOut, uxx virtualSize);
	void InitArenaStackWasm(Arena* arenaOut);
	bool CanArenaCheckPntrFromArena(const Arena* arena);
	bool CanArenaGetSize(const Arena* arena);
	bool CanArenaAllocAligned(const Arena* arena);
	bool CanArenaFree(const Arena* arena);
	bool CanArenaResetToMark(const Arena* arena);
	bool CanArenaSoftGrow(const Arena* arena);
	bool CanArenaVerifyIntegrity(const Arena* arena);
	bool IsPntrFromArena(const Arena* arena, const void* allocPntr);
	uxx GetAllocSize(const Arena* arena, const void* allocPntr);
	NODISCARD void* AllocMem(Arena* arena, uxx numBytes);
	NODISCARD void* AllocMemAligned(Arena* arena, uxx numBytes, uxx alignmentOverride);
	void FreeMemAligned(Arena* arena, void* allocPntr, uxx allocSize, uxx alignmentOverride);
	PIG_CORE_INLINE void FreeMem(Arena* arena, void* allocPntr, uxx allocSize);
	PIG_CORE_INLINE void FreeMemNoSize(Arena* arena, void* allocPntr);
	NODISCARD void* ReallocMemAligned(Arena* arena, void* allocPntr, uxx oldSize, uxx oldAlignmentOverride, uxx newSize, uxx newAlignmentOverride);
	NODISCARD void* ReallocMem(Arena* arena, void* allocPntr, uxx oldSize, uxx newSize);
	NODISCARD void* ReallocMemNoOldSize(Arena* arena, void* allocPntr, uxx newSize);
	NODISCARD PIG_CORE_INLINE uxx ArenaGetMark(Arena* arena);
	PIG_CORE_INLINE void ArenaResetToMark(Arena* arena, uxx mark);
	uxx ArenaSoftGrowBegin(const Arena* arena, const void* allocPntr, uxx allocSize);
	PIG_CORE_INLINE uxx ArenaSoftGrowBeginNoSize(const Arena* arena, const void* allocPntr);
	void ArenaSoftGrowEnd(Arena* arena, void* allocPntr, uxx allocSize, uxx newSpaceUsed);
	PIG_CORE_INLINE void ArenaSoftGrowEndNoSize(Arena* arena, void* allocPntr, uxx newSpaceUsed);
	bool MemArenaVerifyIntegrity(Arena* arena, bool assertOnFailure);
	PIG_CORE_INLINE bool MemArenaVerifyPaddingAround(const Arena* arena, const void* allocPntr, uxx allocSize, bool assertOnFailure);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define AllocTypeUnaligned(type, arenaPntr)         (type*)AllocMem(       (arenaPntr), (uxx)sizeof(type))
#define AllocArrayUnaligned(type, arenaPntr, count) (type*)AllocMem(       (arenaPntr), (uxx)(sizeof(type) * (count)))
#if LANGUAGE_IS_C
#define AllocType(type, arenaPntr)                  (type*)AllocMemAligned((arenaPntr),              (uxx)sizeof(type),             (uxx)_Alignof(type))
#define AllocArray(type, arenaPntr, count)          (type*)AllocMemAligned((arenaPntr),              (uxx)(sizeof(type) * (count)), (uxx)_Alignof(type))
#define FreeArray(type, arenaPntr, count, allocPntr)       FreeMemAligned((arenaPntr),  (allocPntr), (uxx)(sizeof(type) * (count)), (uxx)_Alignof(type))
#define FreeType(type, arenaPntr, allocPntr)               FreeMemAligned((arenaPntr),  (allocPntr), (uxx)sizeof(type),             (uxx)_Alignof(type))
#else
#define AllocType(type, arenaPntr)                  (type*)AllocMemAligned((arenaPntr),              (uxx)sizeof(type),             (uxx)std::alignment_of<type>())
#define AllocArray(type, arenaPntr, count)          (type*)AllocMemAligned((arenaPntr),              (uxx)(sizeof(type) * (count)), (uxx)std::alignment_of<type>())
#define FreeArray(type, arenaPntr, count, allocPntr)       FreeMemAligned((arenaPntr),  (allocPntr), (uxx)(sizeof(type) * (count)), (uxx)std::alignment_of<type>())
#define FreeType(type, arenaPntr, allocPntr)               FreeMemAligned((arenaPntr),  (allocPntr), (uxx)sizeof(type),             (uxx)std::alignment_of<type>())
#define AllocAndNewWithArgs(classPntrVarName, arenaPntr, classType, ...) do \
{                                                                           \
	classPntrVarName = AllocType(classType, (arenaPntr));                   \
	new(classPntrVarName) classType(__VA_ARGS__);                           \
} while(0)
#define AllocAndNew(classPntrVarName, arenaPntr, classType) do \
{                                                              \
	classPntrVarName = AllocType(classType, (arenaPntr));      \
	new(classPntrVarName) classType;                           \
} while(0)
#define DeclareAllocAndNewWithArgs(classPntrVarName, arenaPntr, classType, ...) classType* classPntrVarName; do \
{                                                                                                               \
	classPntrVarName = AllocType(classType, (arenaPntr));                                                       \
	new(classPntrVarName) classType(__VA_ARGS__);                                                               \
} while(0)
#define DeclareAllocAndNew(classPntrVarName, arenaPntr, classType) classType* classPntrVarName; do \
{                                                                                                  \
	classPntrVarName = AllocType(classType, (arenaPntr));                                          \
	new(classPntrVarName) classType;                                                               \
} while(0)
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

NODISCARD PEXP void* AllocMem(Arena* arena, uxx numBytes);
void FreeMem(Arena* arena, void* allocPntr, uxx allocSize);

// +--------------------------------------------------------------+
// |                   Initialization Functions                   |
// +--------------------------------------------------------------+
PEXP void FreeArena(Arena* arena, Arena* sourceArena)
{
	NotNull(arena);
	switch (arena->type)
	{
		case ArenaType_Alias: FreeArena(arena->sourceArena, sourceArena); break;
		case ArenaType_Stack: FreeMem(sourceArena, arena->mainPntr, arena->size); break;
		case ArenaType_StackVirtual: OsFreeReservedMemory(arena->mainPntr, arena->size); break;
		default: AssertMsg(arena->type != ArenaType_None && false, "Tried to free unsupported ArenaType!");
	}
	ClearPointer(arena);
}

PEXP void InitArenaStdHeap(Arena* arenaOut)
{
	NotNull(arenaOut);
	ClearPointer(arenaOut);
	arenaOut->type = ArenaType_StdHeap;
	#if MEM_ARENA_DEBUG_NAMES
	arenaOut->debugName = "[std_heap]";
	#endif
}

PEXP void InitArenaFuncs(Arena* arenaOut, AllocFunc_f* allocFunc, FreeFunc_f* freeFunc, ReallocFunc_f* reallocFunc)
{
	NotNull(arenaOut);
	NotNull(allocFunc);
	ClearPointer(arenaOut);
	arenaOut->type = ArenaType_Funcs;
	#if MEM_ARENA_DEBUG_NAMES
	arenaOut->debugName = "[funcs]";
	#endif
	arenaOut->allocFunc = allocFunc;
	arenaOut->freeFunc = freeFunc;
	arenaOut->reallocFunc = reallocFunc;
}

PEXP void InitArenaAlias(Arena* arenaOut, Arena* sourceArena)
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

PEXP void InitArenaBuffer(Arena* arenaOut, void* bufferPntr, uxx bufferSize)
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

PEXP void InitArenaStack(Arena* arenaOut, uxx stackSize, Arena* sourceArena)
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

PEXP void InitArenaStackVirtual(Arena* arenaOut, uxx virtualSize)
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

#if USING_CUSTOM_STDLIB
PEXP void InitArenaStackWasm(Arena* arenaOut)
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
PEXP bool CanArenaCheckPntrFromArena(const Arena* arena)
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

PEXP bool CanArenaGetSize(const Arena* arena)
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

PEXP bool CanArenaAllocAligned(const Arena* arena)
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

PEXP bool CanArenaFree(const Arena* arena)
{
	DebugNotNull(arena);
	switch (arena->type)
	{
		case ArenaType_Alias: DebugNotNull(arena->sourceArena); return CanArenaFree(arena->sourceArena);
		case ArenaType_StdHeap:      return true;
		case ArenaType_Buffer:       return true;
		case ArenaType_Funcs:        return (arena->freeFunc != nullptr);
		// case ArenaType_Generic:      return true;
		// case ArenaType_GenericPaged: return true;
		case ArenaType_Stack:        return false;
		// case ArenaType_StackPaged:   return false;
		case ArenaType_StackVirtual: return false;
		case ArenaType_StackWasm:    return false;
		default: return false;
	}
}

PEXP bool CanArenaResetToMark(const Arena* arena)
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

PEXP bool CanArenaSoftGrow(const Arena* arena)
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

PEXP bool CanArenaVerifyIntegrity(const Arena* arena)
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
PEXP bool IsPntrFromArena(const Arena* arena, const void* allocPntr)
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
PEXP uxx GetAllocSize(const Arena* arena, const void* allocPntr)
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
NODISCARD PEXP void* AllocMemAligned(Arena* arena, uxx numBytes, uxx alignmentOverride)
{
	TracyCZoneN(Zone_Func, "AllocMemAligned", true);
	DebugNotNull(arena);
	
	void* result = nullptr;
	uxx alignment = (alignmentOverride != UINTXX_MAX) ? alignmentOverride : arena->alignment;
	
	if (IsFlagSet(arena->flags, ArenaFlag_AddPaddingForDebug) && numBytes > 0)
	{
		numBytes += ARENA_DEBUG_PADDING_SIZE*2;
	}
	
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
			#if MALLOC_ALIGNED_AVAILABLE
			{
				result = (alignment > 1) ? MyMallocAligned(numBytes, alignment) : MyMalloc(numBytes);
				if (result == nullptr)
				{
					if (IsFlagSet(arena->flags, ArenaFlag_AssertOnFailedAlloc)) { AssertMsg(false, "Failed to allocate in StdHeap Arena!"); }
					break;
				}
				arena->used += numBytes;
				IncrementUXX(arena->allocCount);
			}
			#else
			{
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
			}
			#endif
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
			
			uxx currentMisalignment = (alignment > 1) ? (uxx)((size_t)((u8*)arena->mainPntr + arena->used) % alignment) : 0;
			uxx alignmentBytesNeeded = (currentMisalignment > 0) ? (alignment - currentMisalignment) : 0;
			uxx alignedNumBytes = numBytes + alignmentBytesNeeded;
			
			if (arena->used + alignedNumBytes <= arena->size)
			{
				if (arena->used + alignedNumBytes > arena->committed)
				{
					uxx osMemPageSize = OsGetMemoryPageSize();
					Assert(osMemPageSize > 0);
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
					if (newCommittedArea == nullptr) { break; }
					AssertMsg(newCommittedArea == (u8*)arena->mainPntr + arena->committed, "WASM malloc did not return next chunk of memory sequentially! Someone else must have called malloc somewhere! You can only have one StackWasm arena active at a time and no-one can call std malloc besides that one arena!");
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
	
	if (IsFlagSet(arena->flags, ArenaFlag_AddPaddingForDebug) && result != nullptr)
	{
		MyMemSet(result, ARENA_DEBUG_PADDING_VALUE, ARENA_DEBUG_PADDING_SIZE);
		MyMemSet(((u8*)result) + numBytes - ARENA_DEBUG_PADDING_SIZE, ARENA_DEBUG_PADDING_VALUE, ARENA_DEBUG_PADDING_SIZE);
		result = ((u8*)result) + ARENA_DEBUG_PADDING_SIZE;
	}
	
	TracyCZoneEnd(Zone_Func);
	return result;
}
NODISCARD PEXP void* AllocMem(Arena* arena, uxx numBytes) //pre-declared at top of file
{
	return AllocMemAligned(arena, numBytes, UINTXX_MAX);
}

// +--------------------------------------------------------------+
// |                  Arena Free Implementations                  |
// +--------------------------------------------------------------+
PEXP void FreeMemAligned(Arena* arena, void* allocPntr, uxx allocSize, uxx alignmentOverride)
{
	TracyCZoneN(Zone_Func, "FreeMemAligned", true);
	DebugNotNull(arena);
	if (allocPntr == nullptr && !IsFlagSet(arena->flags, ArenaFlag_AllowNullptrFree))
	{
		AssertMsg(allocPntr != nullptr, "Tried to free nullptr from Arena!");
		TracyCZoneEnd(Zone_Func);
		return;
	}
	if (allocSize == 0 && !IsFlagSet(arena->flags, ArenaFlag_AllowFreeWithoutSize))
	{
		AssertMsg(allocSize != 0, "Tried to free from Arena without size!");
		TracyCZoneEnd(Zone_Func);
		return;
	}
	DebugNotNull(allocPntr);
	
	uxx alignment = (alignmentOverride != UINTXX_MAX) ? alignmentOverride : arena->alignment;
	
	if (IsFlagSet(arena->flags, ArenaFlag_AddPaddingForDebug) && allocSize > 0)
	{
		allocSize += ARENA_DEBUG_PADDING_SIZE*2;
		allocPntr = ((u8*)allocPntr) - ARENA_DEBUG_PADDING_SIZE;
	}
	
	switch (arena->type)
	{
		// +=============================+
		// |   ArenaType_Alias FreeMem   |
		// +=============================+
		case ArenaType_Alias:
		{
			DebugNotNull(arena->sourceArena);
			FreeMemAligned(arena->sourceArena, allocPntr, allocSize, alignmentOverride);
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
			#if MALLOC_ALIGNED_AVAILABLE
			if (alignment > 1) { MyFreeAligned(allocPntr); }
			else { MyFree(allocPntr); }
			#else
			MyFree(allocPntr);
			#endif
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
			NotNull(arena->freeFunc);
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
				//TODO: Do we want to uncommit committed pages?
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
	
	TracyCZoneEnd(Zone_Func);
}
PEXPI void FreeMem(Arena* arena, void* allocPntr, uxx allocSize)
{
	FreeMemAligned(arena, allocPntr, allocSize, UINTXX_MAX);
}
PEXPI void FreeMemNoSize(Arena* arena, void* allocPntr)
{
	FreeMemAligned(arena, allocPntr, 0, UINTXX_MAX);
}

// +--------------------------------------------------------------+
// |                Arena Realloc Implementations                 |
// +--------------------------------------------------------------+
//TODO: Should we have alignment option here?
NODISCARD PEXP void* ReallocMemAligned(Arena* arena, void* allocPntr, uxx oldSize, uxx oldAlignmentOverride, uxx newSize, uxx newAlignmentOverride)
{
	TracyCZoneN(Zone_Func, "ReallocMemAligned", true);
	DebugNotNull(arena);
	void* result = nullptr;
	
	uxx oldAlignment = (oldAlignmentOverride != UINTXX_MAX) ? oldAlignmentOverride : arena->alignment;
	uxx newAlignment = (newAlignmentOverride != UINTXX_MAX) ? newAlignmentOverride : arena->alignment;
	
	// Degenerate cases where we either do nothing, Alloc, or Free
	if (oldSize == newSize && oldAlignment == newAlignment) { TracyCZoneEnd(Zone_Func); return allocPntr; }
	if (allocPntr == nullptr)
	{
		Assert(oldSize == 0);
		result = AllocMemAligned(arena, newSize, newAlignmentOverride);
		TracyCZoneEnd(Zone_Func);
		return result;
	}
	if (newSize == 0)
	{
		FreeMemAligned(arena, allocPntr, oldSize, oldAlignmentOverride);
		TracyCZoneEnd(Zone_Func);
		return nullptr;
	}
	
	if (oldSize == 0 && !IsFlagSet(arena->flags, ArenaFlag_AllowFreeWithoutSize)) { AssertMsg(oldSize != 0, "Tried to Realloc in Arena without oldSize!"); TracyCZoneEnd(Zone_Func); return nullptr; }
	
	if (IsFlagSet(arena->flags, ArenaFlag_AddPaddingForDebug))
	{
		oldSize += ARENA_DEBUG_PADDING_SIZE*2;
		allocPntr = ((u8*)allocPntr) - ARENA_DEBUG_PADDING_SIZE;
		newSize += ARENA_DEBUG_PADDING_SIZE*2;
	}
	
	switch (arena->type)
	{
		// +==============================+
		// |  ArenaType_Alias ReallocMem  |
		// +==============================+
		case ArenaType_Alias:
		{
			DebugNotNull(arena->sourceArena);
			result = ReallocMemAligned(arena->sourceArena, allocPntr, oldSize, oldAlignment, newSize, newAlignment);
			if (result == nullptr && !IsFlagSet(arena->flags, ArenaFlag_AssertOnFailedAlloc)) { AssertMsg(result != nullptr, "Realloc in Alias Arena failed!"); break; }
		} break;
		
		// +==============================+
		// | ArenaType_StdHeap ReallocMem |
		// +==============================+
		case ArenaType_StdHeap:
		{
			if (newAlignment == oldAlignment && newAlignment == 0)
			{
				result = MyRealloc(allocPntr, newSize);
				if (result == nullptr && IsFlagSet(arena->flags, ArenaFlag_AssertOnFailedAlloc)) { AssertMsg(result != nullptr, "Realloc in StdHeap Arena failed!"); break; }
				if (result != nullptr)
				{
					if (newSize > oldSize) { arena->used += newSize - oldSize; }
					else { arena->used -= oldSize - newSize; }
				}
				else { arena->used -= oldSize; }
			}
			else
			{
				result = (newAlignment > 1) ? MyMallocAligned(newSize, newAlignment) : MyMalloc(newSize);
				if (result == nullptr && IsFlagSet(arena->flags, ArenaFlag_AssertOnFailedAlloc)) { }
				if (result != nullptr)
				{
					if (oldSize > 0)
					{
						MyMemCopy(result, allocPntr, (newSize < oldSize) ? newSize : oldSize);
						if (oldAlignment > 1) { MyFreeAligned(allocPntr); }
						else { MyFree(allocPntr); }
					}
					if (newSize > oldSize) { arena->used += newSize - oldSize; }
					else { arena->used -= oldSize - newSize; }
				}
				else { arena->used -= oldSize; }
			}
		} break;
		
		// +==============================+
		// | ArenaType_Buffer ReallocMem  |
		// +==============================+
		case ArenaType_Buffer:
		{
			AssertMsg(false, "ReallocMem is unimplemented for ArenaType_Buffer"); //TODO: Implement me!
		} break;
		
		// +==============================+
		// |  ArenaType_Funcs ReallocMem  |
		// +==============================+
		case ArenaType_Funcs:
		{
			if (arena->reallocFunc != nullptr)
			{
				result = arena->reallocFunc(allocPntr, newSize);
			}
			else
			{
				NotNull(arena->freeFunc);
				void* newSpace = arena->allocFunc(newSize);
				if (newSpace == nullptr) { break; }
				Assert(allocPntr == nullptr || oldSize > 0);
				if (allocPntr != nullptr) { MyMemCopy(newSpace, allocPntr, oldSize); }
				result = newSpace;
			}
		} break;
		
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
		case ArenaType_Stack:
		{
			DebugNotNull(arena->mainPntr);
			uxx allocIndex = (uxx)((u8*)allocPntr - (u8*)arena->mainPntr);
			// If the allocation is the last thing on the arena, then we can actually just grow it
			if (oldSize > 0 && allocIndex + oldSize == arena->used && IsAlignedTo(allocPntr, newAlignment))
			{
				if (allocIndex + newSize <= arena->size)
				{
					arena->used = allocIndex + newSize;
					result = allocPntr;
				}
			}
			//Otherwise a Realloc is the same as a call to Alloc, the old allocation will be "forgotten"
			else if (newSize > 0)
			{
				result = AllocMemAligned(arena, newSize, newAlignmentOverride);
				if (oldSize > 0) { MyMemCopy(result, allocPntr, (oldSize <= newSize) ? oldSize : newSize); }
			}
			if (result == nullptr && newSize > 0 && IsFlagSet(arena->flags, ArenaFlag_AssertOnFailedAlloc)) { AssertMsg(false, "Failed to reallocate in Stack Arena!"); }
		} break;
		
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
		case ArenaType_StackVirtual:
		{
			//TODO: Should we be silently ignoring scenarios where we can't free things?
			DebugNotNull(arena->mainPntr);
			uxx allocIndex = (uxx)((u8*)allocPntr - (u8*)arena->mainPntr);
			// If the allocation is the last thing on the arena, then we can actually just grow it
			if (oldSize > 0 && allocIndex + oldSize == arena->used && IsAlignedTo(allocPntr, newAlignment))
			{
				if (newSize > oldSize)
				{
					void* newAlloc = AllocMemAligned(arena, newSize - oldSize, 0); //we're re-using the page committing logic by calling AllocMemAligned here
					Assert((u8*)newAlloc == (u8*)arena->mainPntr + allocIndex + oldSize);
					arena->used = allocIndex + newSize;
					result = allocPntr;
				}
				else //if (newSize < oldSize)
				{
					//TODO: Do we want to uncommit committed pages?
					arena->used = allocIndex + newSize;
					result = allocPntr;
				}
			}
			//Otherwise a Realloc is the same as a call to Alloc, the old allocation will be "forgotten"
			else if (newSize > 0)
			{
				result = AllocMemAligned(arena, newSize, newAlignmentOverride);
				if (oldSize > 0) { MyMemCopy(result, allocPntr, (oldSize <= newSize) ? oldSize : newSize); }
			}
			if (result == nullptr && newSize > 0 && IsFlagSet(arena->flags, ArenaFlag_AssertOnFailedAlloc)) { AssertMsg(false, "Failed to reallocate in StackVirtual Arena!"); }
		} break;
		
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
	
	if (IsFlagSet(arena->flags, ArenaFlag_AddPaddingForDebug) && result != nullptr)
	{
		MyMemSet(result, ARENA_DEBUG_PADDING_VALUE, ARENA_DEBUG_PADDING_SIZE);
		MyMemSet(((u8*)result) + newSize - ARENA_DEBUG_PADDING_SIZE, ARENA_DEBUG_PADDING_VALUE, ARENA_DEBUG_PADDING_SIZE);
		result = ((u8*)result) + ARENA_DEBUG_PADDING_SIZE;
	}
	
	TracyCZoneEnd(Zone_Func);
	return result;
}
NODISCARD PEXP void* ReallocMem(Arena* arena, void* allocPntr, uxx oldSize, uxx newSize)
{
	return ReallocMemAligned(arena, allocPntr, oldSize, UINTXX_MAX, newSize, UINTXX_MAX);
}
NODISCARD PEXP void* ReallocMemNoOldSize(Arena* arena, void* allocPntr, uxx newSize)
{
	return ReallocMem(arena, allocPntr, 0, newSize);
}

// +--------------------------------------------------------------+
// |                Arena Push/Pop Implementations                |
// +--------------------------------------------------------------+
NODISCARD PEXPI uxx ArenaGetMark(Arena* arena)
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
PEXPI void ArenaResetToMark(Arena* arena, uxx mark)
{
	DebugNotNull(arena);
	Assert(CanArenaResetToMark(arena));
	switch (arena->type)
	{
		case ArenaType_Alias:
		{
			DebugNotNull(arena->sourceArena);
			if (!IsFlagSet(arena->flags, ArenaFlag_DontPop))
			{
				ArenaResetToMark(arena->sourceArena, mark);
			}
		} break;
		case ArenaType_Stack:
		{
			if (!IsFlagSet(arena->flags, ArenaFlag_DontPop))
			{
				arena->used = mark;
				if (mark == 0) { arena->allocCount = 0; }
			}
		} break;
		// case ArenaType_StackPaged: //TODO: Implement me!
		case ArenaType_StackVirtual:
		{
			if (!IsFlagSet(arena->flags, ArenaFlag_DontPop))
			{
				//TODO: Do we want to uncommit committed pages?
				arena->used = mark;
				if (mark == 0) { arena->allocCount = 0; }
			}
		} break;
		case ArenaType_StackWasm:
		{
			if (!IsFlagSet(arena->flags, ArenaFlag_DontPop))
			{
				//NOTE: Memory usage for our WASM module is not actually going down. We can't ever really free memory in WASM
				arena->used = mark;
				if (mark == 0) { arena->allocCount = 0; }
			}
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
PEXP uxx ArenaSoftGrowBegin(const Arena* arena, const void* allocPntr, uxx allocSize)
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
PEXPI uxx ArenaSoftGrowBeginNoSize(const Arena* arena, const void* allocPntr)
{
	return ArenaSoftGrowBegin(arena, allocPntr, 0);
}

// This will Assert if the newSpaceUsed value is invalid!
PEXP void ArenaSoftGrowEnd(Arena* arena, void* allocPntr, uxx allocSize, uxx newSpaceUsed)
{
	DebugNotNull(arena);
	UNUSED(arena);
	UNUSED(allocPntr);
	UNUSED(allocSize);
	UNUSED(newSpaceUsed);
	//TODO: Implement me!
}
PEXPI void ArenaSoftGrowEndNoSize(Arena* arena, void* allocPntr, uxx newSpaceUsed)
{
	ArenaSoftGrowEnd(arena, allocPntr, 0, newSpaceUsed);
}

// +--------------------------------------------------------------+
// |            Arena VerifyIntegrity Implementations             |
// +--------------------------------------------------------------+
PEXP bool MemArenaVerifyIntegrity(Arena* arena, bool assertOnFailure)
{
	DebugNotNull(arena);
	UNUSED(arena);
	UNUSED(assertOnFailure);
	//TODO: Implement me!
	return false;
}

PEXPI bool MemArenaVerifyPaddingAround(const Arena* arena, const void* allocPntr, uxx allocSize, bool assertOnFailure)
{
	NotNull(arena);
	NotNull(allocPntr);
	Assert(allocSize > 0);
	Assert(IsFlagSet(arena->flags, ArenaFlag_AddPaddingForDebug));
	UNUSED(arena); //TODO: Do we want to verify that the allocPntr actually lives inside a memory region that is owned by the arena?
	
	u8 expectedPaddingValues[ARENA_DEBUG_PADDING_SIZE];
	for (uxx bIndex = 0; bIndex < ARENA_DEBUG_PADDING_SIZE; bIndex++) { expectedPaddingValues[bIndex] = ARENA_DEBUG_PADDING_VALUE; }
	
	u8* beforePadding = ((u8*)allocPntr) - ARENA_DEBUG_PADDING_SIZE;
	if (!MyMemEquals(beforePadding, &expectedPaddingValues[0], ARENA_DEBUG_PADDING_SIZE))
	{
		if (assertOnFailure) { AssertMsg(MyMemEquals(beforePadding, &expectedPaddingValues[0], ARENA_DEBUG_PADDING_SIZE), "Allocation leading padding as corrupted!"); }
		return false;
	}
	
	u8* afterPadding = ((u8*)allocPntr) + allocSize;
	if (!MyMemEquals(afterPadding, &expectedPaddingValues[0], ARENA_DEBUG_PADDING_SIZE))
	{
		if (assertOnFailure) { AssertMsg(MyMemEquals(afterPadding, &expectedPaddingValues[0], ARENA_DEBUG_PADDING_SIZE), "Allocation trailing padding as corrupted!"); }
		return false;
	}
	
	return true;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _MEM_ARENA_H

#if defined(_MEM_ARENA_H) && defined(_STRUCT_STRING_H)
#include "cross/cross_mem_arena_and_string.h"
#endif

#if defined(_MEM_ARENA_H) && defined(_STRUCT_STRING_BUFFER_H)
#include "cross/cross_mem_arena_and_string_buffer.h"
#endif

#if defined(_MEM_ARENA_H) && defined(_STRUCT_STRING_H) && defined(_BASE_UNICODE_H)
#include "cross/cross_mem_arena_string_and_unicode.h"
#endif

#if defined(_GFX_FONT_FLOW_H) && defined(_MEM_ARENA_H)
#include "cross/cross_font_flow_and_mem_arena.h"
#endif
