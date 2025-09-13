/*
File:   struct_sparse_sets.h
Author: Taylor Robbins
Date:   09\13\2025
*/

#ifndef _STRUCT_SPARSE_SETS_H
#define _STRUCT_SPARSE_SETS_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
#include "misc/misc_hash.h"

#define SPARSE_SET_MIN_SIZE    4 //items (a SparseSet is actually 0 items until at least one element is pushed on. But at that point it jumps up to this number at least)

#ifndef SPARSE_SET_CLEAR_ITEMS_ON_ADD
#define SPARSE_SET_CLEAR_ITEMS_ON_ADD DEBUG_BUILD
#endif
#ifndef SPARSE_SET_CLEAR_ITEM_BYTE_VALUE
#define SPARSE_SET_CLEAR_ITEM_BYTE_VALUE 0xCC
#endif

#define SPARSE_SET_CAPACITY_PERCENT 0.75f

#define SparseSetV3i_HeaderSize  sizeof(v3i)
#define SparseSetV3i_EmptyValue  INT32_MAX
#define SparseSetV3i_EmptyV3i    NewV3i(SparseSetV3i_EmptyValue, SparseSetV3i_EmptyValue, SparseSetV3i_EmptyValue)

typedef plex SparseSetV3i SparseSetV3i;
plex SparseSetV3i
{
	Arena* arena;
	uxx itemSize;
	uxx itemAlignment;
	
	uxx length;
	uxx allocLength;
	void* slots;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeSparseSetV3i(SparseSetV3i* set);
	void SparseSetV3iClearEx(SparseSetV3i* set, bool deallocate);
	PIG_CORE_INLINE void SparseSetV3iClear(SparseSetV3i* set);
	void InitSparseSetV3i_(uxx itemSize, uxx itemAlignment, SparseSetV3i* set, Arena* arena);
	PIG_CORE_INLINE void* SparseSetV3iGet_(uxx itemSize, uxx itemAlignment, SparseSetV3i* set, v3i key);
	void* SparseSetV3iAdd_(uxx itemSize, uxx itemAlignment, SparseSetV3i* set, v3i key, bool allowOverwrite);
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define SparseSetV3i_ItemOffset(itemAlignment) AlignOffset(SparseSetV3i_HeaderSize, (itemAlignment))
#define SparseSetV3i_SlotSize(itemSize, itemAlignment) (SparseSetV3i_HeaderSize + SparseSetV3i_ItemOffset(itemAlignment) + (itemSize))
#define SparseSetV3i_GetSlotPntr(itemSize, itemAlignment, slotsPntr, index) ((u8*)(slotsPntr) + ((index) * SparseSetV3i_SlotSize((itemSize), (itemAlignment))))
#define SparseSetV3i_Hash(x, y, z) FnvHashV3((x), (y), (z)) //NOTE: CantorTriplet could be used but performs worse
#define SparseSetV3i_IsEmpty(vector) ((vector).X == SparseSetV3i_EmptyValue && (vector).Y == SparseSetV3i_EmptyValue && (vector).Z == SparseSetV3i_EmptyValue)

#if LANGUAGE_IS_C
#define InitSparseSetV3i(type, setPntr, arenaPntr)   InitSparseSetV3i_((uxx)sizeof(type), (uxx)_Alignof(type), (setPntr), (arenaPntr))
#define SparseSetV3iGet(type, setPntr, key)          ((type*)SparseSetV3iGet_((uxx)sizeof(type), (uxx)_Alignof(type), (setPntr), (key)))
#define SparseSetV3iAdd(type, setPntr, key)          ((type*)SparseSetV3iAdd_((uxx)sizeof(type), (uxx)_Alignof(type), (setPntr), (key), false))
#define SparseSetV3iAddOrReplace(type, setPntr, key) ((type*)SparseSetV3iAdd_((uxx)sizeof(type), (uxx)_Alignof(type), (setPntr), (key), true))
#else
#define InitSparseSetV3i(type, setPntr, arenaPntr)   InitSparseSetV3i_((uxx)sizeof(type), (uxx)std::alignment_of<type>(), (setPntr), (arenaPntr))
#define SparseSetV3iGet(type, setPntr, key)          ((type*)SparseSetV3iGet_((uxx)sizeof(type), (uxx)std::alignment_of<type>(), (setPntr), (key)))
#define SparseSetV3iAdd(type, setPntr, key)          ((type*)SparseSetV3iAdd_((uxx)sizeof(type), (uxx)std::alignment_of<type>(), (setPntr), (key), false))
#define SparseSetV3iAddOrReplace(type, setPntr, key) ((type*)SparseSetV3iAdd_((uxx)sizeof(type), (uxx)std::alignment_of<type>(), (setPntr), (key), true))
#endif

#define SparseSetV3iAddValue(type, setPntr, key, value) do                    \
{                                                                             \
	/* We must evaluate (value) before manipulating the set */                \
	/* because it may access/refer to elements in the set   */                \
	type valueBeforeAdd_NOCONFLICT = (value);                                 \
	type* addedItemPntr_NOCONFLICT = SparseSetV3iAdd(type, (setPntr), (key)); \
	DebugNotNull(addedItemPntr_NOCONFLICT);                                   \
	*addedItemPntr_NOCONFLICT = valueBeforeAdd_NOCONFLICT;                    \
} while(0)
#define SparseSetV3iSetValue(type, setPntr, key, value) do                             \
{                                                                                      \
	/* We must evaluate (value) before manipulating the set */                         \
	/* because it may access/refer to elements in the set   */                         \
	type valueBeforeAdd_NOCONFLICT = (value);                                          \
	type* addedItemPntr_NOCONFLICT = SparseSetV3iAddOrReplace(type, (setPntr), (key)); \
	DebugNotNull(addedItemPntr_NOCONFLICT);                                            \
	*addedItemPntr_NOCONFLICT = valueBeforeAdd_NOCONFLICT;                             \
} while(0)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void FreeSparseSetV3i(SparseSetV3i* set)
{
	NotNull(set);
	if (set->arena != nullptr && set->slots != nullptr)
	{
		FreeMemAligned(set->arena, set->slots, SparseSetV3i_SlotSize(set->itemSize, set->itemAlignment) * set->allocLength, set->itemAlignment);
	}
	ClearPointer(set);
}

PEXP void SparseSetV3iClearEx(SparseSetV3i* set, bool deallocate)
{
	NotNull(set);
	NotNull(set->arena);
	if (deallocate)
	{
		if (set->slots != nullptr) { FreeMemAligned(set->arena, set->slots, SparseSetV3i_SlotSize(set->itemSize, set->itemAlignment) * set->allocLength, set->itemAlignment); }
	}
	else if (set->length > 0)
	{
		for (uxx sIndex = 0; sIndex < set->allocLength; sIndex++)
		{
			*(v3i*)SparseSetV3i_GetSlotPntr(set->itemSize, set->itemAlignment, set->slots, sIndex) = SparseSetV3i_EmptyV3i;
		}
		set->length = 0;
	}
}
PEXPI void SparseSetV3iClear(SparseSetV3i* set) { SparseSetV3iClearEx(set, false); }

PEXP void InitSparseSetV3i_(uxx itemSize, uxx itemAlignment, SparseSetV3i* set, Arena* arena)
{
	NotNull(set);
	NotNull(arena);
	ClearPointer(set);
	set->arena = arena;
	set->itemSize = itemSize;
	set->itemAlignment = itemAlignment;
}

static void SparseSetV3iExpand(SparseSetV3i* set, uxx capacityRequired)
{
	if (capacityRequired <= (uxx)(set->allocLength * SPARSE_SET_CAPACITY_PERCENT)) { return; }
	uxx slotSize = SparseSetV3i_SlotSize(set->itemSize, set->itemAlignment);
	
	uxx newAllocLength = (set->allocLength > SPARSE_SET_MIN_SIZE) ? set->allocLength : SPARSE_SET_MIN_SIZE;
	while (newAllocLength < UINTXX_MAX && (uxx)(newAllocLength * SPARSE_SET_CAPACITY_PERCENT) < capacityRequired) { newAllocLength *= 2; }
	Assert(newAllocLength >= capacityRequired);
	
	void* newSpace = AllocMemAligned(set->arena, newAllocLength * slotSize, set->itemAlignment);
	for (uxx sIndex = 0; sIndex < newAllocLength; sIndex++) { *(v3i*)SparseSetV3i_GetSlotPntr(set->itemSize, set->itemAlignment, newSpace, sIndex) = SparseSetV3i_EmptyV3i; }
	
	//Move all the items, find their new location in the expanded space
	for (uxx sIndex = 0; sIndex < set->allocLength; sIndex++)
	{
		v3i* oldSlotPntr = (v3i*)SparseSetV3i_GetSlotPntr(set->itemSize, set->itemAlignment, set->slots, sIndex);
		if (!SparseSetV3i_IsEmpty(*oldSlotPntr))
		{
			i32 oldSlotHash = SparseSetV3i_Hash(oldSlotPntr->X, oldSlotPntr->Y, oldSlotPntr->Z);
			i32 newExpectedIndex = (oldSlotHash % newAllocLength);
			
			i32 newSlotIndex = newExpectedIndex;
			v3i* newSlotPntr = (v3i*)SparseSetV3i_GetSlotPntr(set->itemSize, set->itemAlignment, newSpace, newSlotIndex);
			while (!AreEqualV3i(*newSlotPntr, *oldSlotPntr) && !SparseSetV3i_IsEmpty(*newSlotPntr))
			{
				newSlotIndex = ((newSlotIndex+1) % newAllocLength);
				if (newSlotIndex == newExpectedIndex) { break; } //we looped around
				newSlotPntr = (v3i*)SparseSetV3i_GetSlotPntr(set->itemSize, set->itemAlignment, newSpace, newSlotIndex);
			}
			
			if (SparseSetV3i_IsEmpty(*newSlotPntr))
			{
				// PrintLine_D("Moving slot[%llu/%llu] into [%llu/%llu]", sIndex, set->allocLength, newSlotIndex, newAllocLength);
				MyMemCopy(newSlotPntr, oldSlotPntr, slotSize);
			}
			else
			{
				AssertMsg(SparseSetV3i_IsEmpty(*newSlotPntr), "Failed to add item to SparseSetV3i after expansion!");
				set->length--;
			}
		}
	}
	
	if (set->slots != nullptr) { FreeMemAligned(set->arena, set->slots, slotSize * set->allocLength, set->itemAlignment); }
	set->slots = newSpace;
	set->allocLength = newAllocLength;
}

PEXPI void* SparseSetV3iGet_(uxx itemSize, uxx itemAlignment, SparseSetV3i* set, v3i key, bool assertOnFailure)
{
	#if DEBUG_BUILD
	NotNull(set);
	NotNull(set->arena);
	AssertMsg(set->itemSize == itemSize, "Invalid itemSize passed to SparseSetV3iGet. Make sure you're accessing the SparseSet with the correct type!");
	AssertMsg(set->itemAlignment == itemAlignment, "Invalid itemAlignment passed to SparseSetV3iGet. Make sure you're accessing the SparseSet with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	DebugAssertMsg(!SparseSetV3i_IsEmpty(key), "SparseSetV3i can't store (INT32_MAX, INT32_MAX, INT32_MAX) since that acts as a special value meaning \"empty\"");
	if (set->allocLength == 0) { return nullptr; }
	
	uxx slotSize = SparseSetV3i_SlotSize(set->itemSize, set->itemAlignment);
	i32 hash = SparseSetV3i_Hash(key.X, key.Y, key.Z);
	i32 expectedIndex = (hash % set->allocLength);
	
	i32 slotIndex = expectedIndex;
	v3i* slotPntr = (v3i*)SparseSetV3i_GetSlotPntr(set->itemSize, set->itemAlignment, set->slots, slotIndex);
	while (!AreEqualV3i(*slotPntr, key) && !SparseSetV3i_IsEmpty(*slotPntr))
	{
		slotIndex = ((slotIndex+1) % set->allocLength);
		if (slotIndex == expectedIndex) { break; } //we looped around
		slotPntr = (v3i*)SparseSetV3i_GetSlotPntr(set->itemSize, set->itemAlignment, set->slots, slotIndex);
	}
	
	if (AreEqualV3i(*slotPntr, key))
	{
		return (u8*)slotPntr + SparseSetV3i_HeaderSize + SparseSetV3i_ItemOffset(set->itemAlignment);
	}
	else
	{
		if (assertOnFailure) { AssertMsg(AreEqualV3i(*slotPntr, key), "No item with key in SparseSetV3i!"); }
		return nullptr;
	}
}

PEXP void* SparseSetV3iAdd_(uxx itemSize, uxx itemAlignment, SparseSetV3i* set, v3i key, bool allowOverwrite)
{
	#if DEBUG_BUILD
	NotNull(set);
	NotNull(set->arena);
	AssertMsg(set->itemSize == itemSize, "Invalid itemSize passed to SparseSetV3iAdd. Make sure you're accessing the SparseSet with the correct type!");
	AssertMsg(set->itemAlignment == itemAlignment, "Invalid itemAlignment passed to SparseSetV3iAdd. Make sure you're accessing the SparseSet with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	
	SparseSetV3iExpand(set, set->length+1);
	
	uxx slotSize = SparseSetV3i_SlotSize(set->itemSize, set->itemAlignment);
	i32 hash = SparseSetV3i_Hash(key.X, key.Y, key.Z);
	i32 expectedIndex = (hash % set->allocLength);
	
	i32 slotIndex = expectedIndex;
	v3i* slotPntr = (v3i*)SparseSetV3i_GetSlotPntr(set->itemSize, set->itemAlignment, set->slots, slotIndex);
	while (!SparseSetV3i_IsEmpty(*slotPntr) && !(allowOverwrite && AreEqualV3i(*slotPntr, key)))
	{
		slotIndex = ((slotIndex+1) % set->allocLength);
		if (slotIndex == expectedIndex) { break; } //we looped around
		slotPntr = (v3i*)SparseSetV3i_GetSlotPntr(set->itemSize, set->itemAlignment, set->slots, slotIndex);
	}
	
	if (SparseSetV3i_IsEmpty(*slotPntr))
	{
		*slotPntr = key;
		set->length++;
		#if SPARSE_SET_CLEAR_ITEMS_ON_ADD
		MyMemSet((u8*)slotPntr + SparseSetV3i_HeaderSize + SparseSetV3i_ItemOffset(set->itemAlignment), SPARSE_SET_CLEAR_ITEM_BYTE_VALUE, set->itemSize);
		#endif
		return (u8*)slotPntr + SparseSetV3i_HeaderSize + SparseSetV3i_ItemOffset(set->itemAlignment);
	}
	else if (allowOverwrite && AreEqualV3i(*slotPntr, key))
	{
		#if SPARSE_SET_CLEAR_ITEMS_ON_ADD
		MyMemSet((u8*)slotPntr + SparseSetV3i_HeaderSize + SparseSetV3i_ItemOffset(set->itemAlignment), SPARSE_SET_CLEAR_ITEM_BYTE_VALUE, set->itemSize);
		#endif
		return (u8*)slotPntr + SparseSetV3i_HeaderSize + SparseSetV3i_ItemOffset(set->itemAlignment);
	}
	else
	{
		AssertMsg(false, "Failed to add item to SparseSetV3i!");
		return nullptr;
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_SPARSE_SETS_H
