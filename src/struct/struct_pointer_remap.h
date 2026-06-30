/*
File:   struct_pointer_remap.h
Author: Taylor Robbins
Date:   08\21\2025
Description:
	** Often when a particular manipulation to a data structure is made we need to 
	** loop over all pointers that were pointing to that data structure and update
	** them in some way. The PointerRemap plex allows us to save the information about
	** which pointers go where so we can make the manipulation and produce the Remap
	** and then the calling code can visit all the sites where pointers live and
	** update them appropriately.
	
	** PointerRemaps can also be used to convert relative pointers (aka indices)
	** into real pointers, often after loading information from disk where the values
	** in the file had to be saved as offsets rather than pointers to memory.
	
	** TODO: This concept was introduced while working on BktArray as a way to
	** potentially allow storing pointers and doing fixup when operations like
	** Insert or Remove are used. However none of the BktArray functions actually
	** produce a PointerRemap right now so it remains to be seen if we actually
	** want this idea used all over the place. The work of producing the Remap may
	** be significant enough that we don't want it baked in as an optional output
	** of these operations, so you'd need a separate implementation of these functions
	** that produces the remap, which is not ideal.
*/

#ifndef _STRUCT_POINTER_REMAP_H
#define _STRUCT_POINTER_REMAP_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"

typedef plex PointerRemapRange PointerRemapRange;
plex PointerRemapRange
{
	uintptr_t from;
	uintptr_t to;
	size_t size;
};

typedef plex PointerRemap PointerRemap;
plex PointerRemap
{
	Arena* arena;
	uxx numRanges;
	uxx numRangesAlloc;
	PointerRemapRange* ranges;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreePointerRemap(PointerRemap* remap);
	PIG_CORE_INLINE void InitPointerRemapWithInitial(PointerRemap* remapOut, Arena* arena, uxx numRangesExpected);
	PIG_CORE_INLINE void InitPointerRemap(PointerRemap* remapOut, Arena* arena);
	PIG_CORE_INLINE bool IsSizedPointerInRemap(const PointerRemap* remap, const void* pointer, size_t size);
	PIG_CORE_INLINE bool IsPointerInRemap(const PointerRemap* remap, const void* pointer, size_t size);
	PIG_CORE_INLINE void* RemapPointer_(PointerRemap* remap, const void* pointer, bool assertOnFailure);
	void AddRemapRange(PointerRemap* remap, uintptr_t from, uintptr_t to, size_t size, bool combineIfPossible);
	PIG_CORE_INLINE void AddRemapPointer(PointerRemap* remap, const void* fromPntr, const void* toPntr);
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define RemapSizedPointerHard(type, remap, pointer, size) (type*)RemapPointer_((remap), (pointer), (size), true)
#define RemapSizedPointerSoft(type, remap, pointer, size) (type*)RemapPointer_((remap), (pointer), (size), false)
#define RemapSizedPointer(type, remap, pointer, size) RemapPointerHard(type, (remap), (pointer), (size))

#define RemapPointerHard(type, remap, pointer) (type*)RemapPointer_((remap), (pointer), 0, true)
#define RemapPointerSoft(type, remap, pointer) (type*)RemapPointer_((remap), (pointer), 0, false)
#define RemapPointer(type, remap, pointer) RemapPointerHard(type, (remap), (pointer))

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void FreePointerRemap(PointerRemap* remap)
{
	NotNull(remap);
	if (remap->arena != nullptr)
	{
		if (remap->numRangesAlloc > 0)
		{
			NotNull(remap->ranges);
			FreeArray(PointerRemapRange, remap->arena, remap->numRangesAlloc, remap->ranges);
		}
	}
	ClearPointer(remap);
}

PEXPI void InitPointerRemapWithInitial(PointerRemap* remapOut, Arena* arena, uxx numRangesExpected)
{
	NotNull(remapOut);
	ClearPointer(remapOut);
	remapOut->arena = arena;
	if (numRangesExpected > 0)
	{
		remapOut->numRangesAlloc = numRangesExpected;
		remapOut->ranges = AllocArray(PointerRemapRange, arena, numRangesExpected);
		NotNull(remapOut->ranges);
	}
}
PEXPI void InitPointerRemap(PointerRemap* remapOut, Arena* arena) { InitPointerRemapWithInitial(remapOut, arena, 0); }

PEXPI bool IsSizedPointerInRemap(const PointerRemap* remap, const void* pointer, size_t size)
{
	DebugNotNull(remap);
	DebugNotNull(remap->arena);
	uintptr_t pointerInt = (uintptr_t)pointer;
	for (uxx rIndex = 0; rIndex < remap->numRanges; rIndex++)
	{
		PointerRemapRange* range = &remap->ranges[rIndex];
		if (pointerInt >= range->from && pointerInt + size <= range->from + range->size) { return true; }
	}
	return false;
}
PEXPI bool IsPointerInRemap(const PointerRemap* remap, const void* pointer) { return IsSizedPointerInRemap(remap, pointer, 0); }

PEXPI void* RemapPointer_(PointerRemap* remap, const void* pointer, size_t size, bool assertOnFailure)
{
	DebugNotNull(remap);
	DebugNotNull(remap->arena);
	uintptr_t pointerInt = (uintptr_t)pointer;
	for (uxx rIndex = 0; rIndex < remap->numRanges; rIndex++)
	{
		PointerRemapRange* range = &remap->ranges[rIndex];
		if (pointerInt >= range->from && pointerInt + size <= range->from + range->size)
		{
			return (void*)(range->to + (pointerInt - range->from));
		}
	}
	if (assertOnFailure) { AssertMsg(false, "Pointer is not part of the PointerRemap!"); }
	return nullptr;
}

PEXP void AddRemapRange(PointerRemap* remap, uintptr_t from, uintptr_t to, size_t size, bool combineIfPossible)
{
	DebugNotNull(remap);
	DebugNotNull(remap->arena);
	
	if (combineIfPossible)
	{
		uxx combinedIndex = remap->numRanges;
		for (uxx rIndex = 0; rIndex < remap->numRanges; rIndex++)
		{
			PointerRemapRange* range = &remap->ranges[rIndex];
			if (from + size >= range->from && from <= range->from + range->size)
			{
				size_t fromOffset = from - range->from;
				if (to + size >= range->to && to <= range->to + range->size)
				{
					size_t toOffset = to - range->to;
					if (toOffset == fromOffset)
					{
						range->from = (range->from < from) ? range->from : from;
						range->to = (range->to < to) ? range->to : to;
						range->size = (range->size > fromOffset + size) ? range->size : fromOffset + size;
						combinedIndex = rIndex;
						break;
					}
				}
			}
		}
		if (combinedIndex < remap->numRanges)
		{
			//TODO: We should probably check if this new larger range touches any other ranges as well and combine again!
			return;
		}
	}
	
	if (remap->numRanges >= remap->numRangesAlloc)
	{
		uxx newAllocCount = MaxUXX(remap->numRangesAlloc, 4);
		while (newAllocCount < remap->numRanges+1) { newAllocCount *= 2; }
		#if LANGUAGE_IS_C
		uxx alignment = _Alignof(PointerRemapRange);
		#else
		uxx alignment = std::alignment_of<PointerRemapRange>();
		#endif
		remap->ranges = ReallocMemAligned(remap->arena, remap->ranges, sizeof(PointerRemapRange) * remap->numRangesAlloc, alignment, sizeof(PointerRemapRange) * newAllocCount, alignment);
		remap->numRangesAlloc = newAllocCount;
	}
	
	PointerRemapRange* newRange = &remap->ranges[remap->numRanges];
	newRange->from = from;
	newRange->to = to;
	newRange->size = size;
}

PEXPI void AddRemapPointer(PointerRemap* remap, const void* fromPntr, const void* toPntr) { AddRemapRange(remap, (uintptr_t)fromPntr, (uintptr_t)toPntr, 0, false); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_POINTER_REMAP_H
