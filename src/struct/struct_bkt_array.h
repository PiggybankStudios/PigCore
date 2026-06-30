/*
File:   struct_bkt_array.h
Author: Taylor Robbins
Date:   08\19\2025
Description:
	** A BktArray (Bkt=Bucket) is similar to VarArray but rather
	** than having one contiguous buffer of memory, it allocates
	** a linked list of buckets with some space in each bucket.
	** This allows us to guarantee that BktArrayAdd is never going
	** to move existing items in the array, so pointers to items
	** can be held through calls to BktArrayAdd.
	** Removals and insertions can still shift some items though
	** the shifting is bounded to a single bucket so it MAY be safe
	** to hold a pointer to an item in another bucket but it's not
	** easy to guarantee which pointers are safe so it's best to
	** treat them as barriers for all pointers.
	
	** NOTE: Read description in struct_var_array.h for more info
*/

#ifndef _STRUCT_BKT_ARRAY_H
#define _STRUCT_BKT_ARRAY_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "mem/mem_arena.h"
#include "std/std_memset.h"

typedef plex BktArrayBkt BktArrayBkt;
plex BktArrayBkt
{
	BktArrayBkt* next;
	uxx length;
	uxx allocLength;
};

typedef plex BktArray BktArray;
plex BktArray
{
	Arena* arena;
	uxx itemSize;
	uxx itemAlignment;
	uxx defaultBucketSize;
	uxx length;
	uxx allocLength;
	uxx numBuckets;
	BktArrayBkt* firstBucket;
	BktArrayBkt* lastBucket; //This is the last bucket that may have space, not necassarily the last bucket in the linked list
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void FreeBktArray(BktArray* array);
	void BktArrayClear(BktArray* array, bool deallocate);
	void InitBktArrayWithInitial_(uxx itemSize, uxx itemAlignment, BktArray* array, Arena* arena, uxx defaultBucketSize, uxx initialCountNeeded);
	PIG_CORE_INLINE bool IsBktArrayInit(const BktArray* array);
	PIG_CORE_INLINE void* BktArrayGet_(uxx itemSize, uxx itemAlignment, BktArray* array, uxx index, bool assertOnFailure);
	PIG_CORE_INLINE uxx BktArrayGetIndexOf_(uxx itemSize, uxx itemAlignment, const BktArray* array, const void* itemInQuestion);
	PIG_CORE_INLINE bool BktArrayContains_(uxx itemSize, uxx itemAlignment, const BktArray* array, const void* itemInQuestion);
	PIG_CORE_INLINE void* BktArrayAdd_(uxx itemSize, uxx itemAlignment, BktArray* array);
	PIG_CORE_INLINE void* BktArrayAddSomewhere_(uxx itemSize, uxx itemAlignment, BktArray* array);
	void* BktArrayAddMulti_(uxx itemSize, uxx itemAlignment, BktArray* array, uxx numItems);
	PIG_CORE_INLINE void* BktArrayAddArray_(uxx itemSize, uxx itemAlignment, BktArray* destArray, const BktArray* srcArray);
	void BktArrayRemoveAt_(uxx itemSize, uxx itemAlignment, BktArray* array, uxx index);
	PIG_CORE_INLINE void BktArrayRemove_(uxx itemSize, uxx itemAlignment, BktArray* array, const void* itemToRemove);
	PIG_CORE_INLINE void BktArrayCopy(Arena* arena, BktArray* destArray, const BktArray* srcArray);
	void* BktArrayInsert_(uxx itemSize, uxx itemAlignment, BktArray* array, uxx index);
	PIG_CORE_INLINE void BktArrayCondenseInto(BktArray* array, Arena* intoArena, bool freeMemory);
	PIG_CORE_INLINE void BktArrayCondense(BktArray* array);
	PIG_CORE_INLINE void BktArrayDropEmptyBuckets(BktArray* array);
	PIG_CORE_INLINE uxx BktArrayGetBucketIndexAt(BktArray* array, uxx itemIndex, uxx* innerIndexOut);
	PIG_CORE_INLINE uxx BktArrayGetBucketIndex(BktArray* array, const void* itemPntr, uxx* innerIndexOut);
	PIG_CORE_INLINE BktArrayBkt* BktArrayGetBucket(BktArray* array, uxx bucketIndex);
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#if LANGUAGE_IS_C	
#define InitBktArrayWithInitial(type, arrayPntr, arenaPntr, defaultBucketSize, initialCountNeeded) InitBktArrayWithInitial_(sizeof(type), (uxx)_Alignof(type), (arrayPntr), (arenaPntr), (defaultBucketSize), (initialCountNeeded))
#else
#define InitBktArrayWithInitial(type, arrayPntr, arenaPntr, defaultBucketSize, initialCountNeeded) InitBktArrayWithInitial_(sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (arenaPntr), (defaultBucketSize), (initialCountNeeded))
#endif
#define InitBktArray(type, arrayPntr, arenaPntr, defaultBucketSize) InitBktArrayWithInitial(type, (arrayPntr), (arenaPntr), (defaultBucketSize), 0)

//Hard indicates we want to assertOnFailure, opposed to Soft which will return nullptr. Not specifying leads to implicitly using Hard variant
#if LANGUAGE_IS_C
#define BktArrayGetHard(type, arrayPntr, index) (type*)BktArrayGet_(sizeof(type), (uxx)_Alignof(type), (arrayPntr), (index), true)
#define BktArrayGetSoft(type, arrayPntr, index) (type*)BktArrayGet_(sizeof(type), (uxx)_Alignof(type), (arrayPntr), (index), false)
#else
#define BktArrayGetHard(type, arrayPntr, index) (type*)BktArrayGet_(sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (index), true)
#define BktArrayGetSoft(type, arrayPntr, index) (type*)BktArrayGet_(sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (index), false)
#endif
#define BktArrayGet(type, arrayPntr, index) BktArrayGetHard(type, (arrayPntr), (index))

#if LANGUAGE_IS_C
#define BktArrayGetIndexOf(type, arrayPntr, itemInQuestion) BktArrayGetIndexOf_(sizeof(type), (uxx)_Alignof(type), (arrayPntr), (itemInQuestion))
#define BktArrayContains(type, arrayPntr, itemInQuestion) BktArrayContains_(sizeof(type), (uxx)_Alignof(type), (arrayPntr), (itemInQuestion))
#else
#define BktArrayGetIndexOf(type, arrayPntr, itemInQuestion) BktArrayGetIndexOf_(sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (itemInQuestion))
#define BktArrayContains(type, arrayPntr, itemInQuestion) BktArrayContains_(sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (itemInQuestion))
#endif

#if LANGUAGE_IS_C
#define BktArrayAdd(type, arrayPntr) (type*)BktArrayAdd_(sizeof(type), (uxx)_Alignof(type), (arrayPntr))
#define BktArrayAddSomewhere(type, arrayPntr) (type*)BktArrayAddSomewhere_(sizeof(type), (uxx)_Alignof(type), (arrayPntr))
#else
#define BktArrayAdd(type, arrayPntr) (type*)BktArrayAdd_(sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr))
#define BktArrayAddSomewhere(type, arrayPntr) (type*)BktArrayAddSomewhere_(sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr))
#endif
#define BktArrayAddValue(type, arrayPntr, value) do                  \
{                                                                    \
	/* We must evaluate (value) before manipulating the array */     \
	/* because it may access/refer to elements in the array   */     \
	type valueBeforeAdd_NOCONFLICT = (value);                        \
	type* addedItemPntr_NOCONFLICT = BktArrayAdd(type, (arrayPntr)); \
	DebugNotNull(addedItemPntr_NOCONFLICT);                          \
	*addedItemPntr_NOCONFLICT = valueBeforeAdd_NOCONFLICT;           \
} while(0)
#define BktArrayAddValueSomewhere(type, arrayPntr, value) do                  \
{                                                                             \
	/* We must evaluate (value) before manipulating the array */              \
	/* because it may access/refer to elements in the array   */              \
	type valueBeforeAdd_NOCONFLICT = (value);                                 \
	type* addedItemPntr_NOCONFLICT = BktArrayAddSomewhere(type, (arrayPntr)); \
	DebugNotNull(addedItemPntr_NOCONFLICT);                                   \
	*addedItemPntr_NOCONFLICT = valueBeforeAdd_NOCONFLICT;                    \
} while(0)
// This is simply an alias of BktArrayAddValue, but it's here to match the name of BktArrayPop below
#define BktArrayPush(type, arrayPntr, value) BktArrayAddValue(type, (arrayPntr), (value))

#if LANGUAGE_IS_C
#define BktArrayAddMulti(type, arrayPntr, numItems) (type*)BktArrayAddMulti_(sizeof(type), (uxx)_Alignof(type), (arrayPntr), (numItems))
#define BktArrayAddArray(type, destArrayPntr, srcArrayPntr) (type*)BktArrayAddArray_(sizeof(type), (uxx)_Alignof(type), (destArrayPntr), (srcArrayPntr))
#else
#define BktArrayAddMulti(type, arrayPntr, numItems) (type*)BktArrayAddMulti_(sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (numItems))
#define BktArrayAddArray(type, destArrayPntr, srcArrayPntr) (type*)BktArrayAddArray_(sizeof(type), (uxx)std::alignment_of<type>(), (destArrayPntr), (srcArrayPntr))
#endif
#define BktArrayAddValues(type, arrayPntr, numValues, valuesPntr) do                    \
{                                                                                       \
	type* addedItemsPntr_NOCONFLICT = BktArrayAddMulti(type, (arrayPntr), (numValues)); \
	DebugNotNull(addedItemsPntr_NOCONFLICT);                                            \
	MyMemCopy(addedItemsPntr_NOCONFLICT, (valuesPntr), sizeof(type) * (numValues));     \
} while(0)

#if LANGUAGE_IS_C
#define BktArrayRemoveAt(type, arrayPntr, index) BktArrayRemoveAt_(sizeof(type), (uxx)_Alignof(type), (arrayPntr), (index))
#define BktArrayRemove(type, arrayPntr, itemToRemovePntr) BktArrayRemove_(sizeof(type), (uxx)_Alignof(type), (arrayPntr), (itemToRemovePntr))
#else
#define BktArrayRemoveAt(type, arrayPntr, index) BktArrayRemoveAt_(sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (index))
#define BktArrayRemove(type, arrayPntr, itemToRemovePntr) BktArrayRemove_(sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (itemToRemovePntr))
#endif

#if LANGUAGE_IS_C
#define BktArrayInsert(type, arrayPntr, index) (type*)BktArrayInsert_(sizeof(type), (uxx)_Alignof(type), (arrayPntr), (index))
#else
#define BktArrayInsert(type, arrayPntr, index) (type*)BktArrayInsert_(sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (index))
#endif
#define BktArrayInsertValue(type, arrayPntr, index, value) do                    \
{                                                                                \
	/* We must evaluate (value) before manipulating the array */                 \
	/* because it may access/refer to elements in the array   */                 \
	type valueBeforeInsert_NOCONFLICT = (value);                                 \
	type* addedItemPntr_NOCONFLICT = BktArrayInsert(type, (arrayPntr), (index)); \
	DebugNotNull(addedItemPntr_NOCONFLICT);                                      \
	*addedItemPntr_NOCONFLICT = valueBeforeInsert_NOCONFLICT;                    \
} while(0)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

#define BktArrayGetHeaderPadding(itemAlignment) AlignOffset(sizeof(BktArrayBkt), (itemAlignment))
#define BktArrayBktGetItemPntr(arrayPntr, bucketPntr, index) (((u8*)(bucketPntr)) + sizeof(BktArrayBkt) + BktArrayGetHeaderPadding((arrayPntr)->itemAlignment) + ((index) * (arrayPntr)->itemSize))
#define BktArrayAllocSize(arrayPntr, allocLength) sizeof(BktArrayBkt) + BktArrayGetHeaderPadding((arrayPntr)->itemAlignment) + ((allocLength) * (arrayPntr)->itemSize)

PEXP void FreeBktArray(BktArray* array)
{
	NotNull(array);
	if (array->arena != nullptr)
	{
		BktArrayBkt* bucket = array->firstBucket;
		for (uxx bIndex = 0; bIndex < array->numBuckets; bIndex++)
		{
			NotNull(bucket);
			BktArrayBkt* nextBucket = bucket->next;
			FreeMemAligned(array->arena,
				bucket,
				BktArrayAllocSize(array, bucket->allocLength),
				array->itemAlignment
			);
			bucket = nextBucket;
		}
		Assert(bucket == nullptr);
	}
	ClearPointer(array);
}

PEXP void BktArrayClear(BktArray* array, bool deallocate)
{
	NotNull(array);
	array->length = 0;
	if (deallocate)
	{
		BktArrayBkt* bucket = array->firstBucket;
		for (uxx bIndex = 0; bIndex < array->numBuckets; bIndex++)
		{
			NotNull(bucket);
			BktArrayBkt* nextBucket = bucket->next;
			FreeMemAligned(array->arena,
				bucket,
				BktArrayAllocSize(array, bucket->allocLength),
				array->itemAlignment
			);
			bucket = nextBucket;
		}
		Assert(bucket == nullptr);
		array->firstBucket = nullptr;
		array->lastBucket = nullptr;
		array->numBuckets = 0;
		array->allocLength = 0;
	}
	else
	{
		BktArrayBkt* bucket = array->firstBucket;
		for (uxx bIndex = 0; bIndex < array->numBuckets; bIndex++)
		{
			NotNull(bucket);
			bucket->length = 0;
			bucket = bucket->next;
		}
		array->lastBucket = array->firstBucket;
	}
}

PEXP void InitBktArrayWithInitial_(uxx itemSize, uxx itemAlignment, BktArray* array, Arena* arena, uxx defaultBucketSize, uxx initialCountNeeded)
{
	NotNull(array);
	NotNull(arena);
	Assert(defaultBucketSize > 0);
	ClearPointer(array);
	array->arena = arena;
	array->itemSize = itemSize;
	array->itemAlignment = itemAlignment;
	array->defaultBucketSize = defaultBucketSize;
	if (initialCountNeeded > 0)
	{
		uxx bucketSize = MaxUXX(array->defaultBucketSize, initialCountNeeded);
		BktArrayBkt* newBucket = (BktArrayBkt*)AllocMemAligned(array->arena, BktArrayAllocSize(array, bucketSize), array->itemAlignment);
		NotNull(newBucket);
		ClearPointer(newBucket);
		newBucket->allocLength = bucketSize;
		array->firstBucket = newBucket;
		array->lastBucket = newBucket;
		array->numBuckets = 1;
		array->allocLength = bucketSize;
	}
}

PEXPI bool IsBktArrayInit(const BktArray* array)
{
	return (array->arena != nullptr);
}

PEXPI void* BktArrayGet_(uxx itemSize, uxx itemAlignment, BktArray* array, uxx index, bool assertOnFailure)
{
	#if DEBUG_BUILD
	NotNull(array);
	Assert(IsBktArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to BktArrayGet. Make sure you're accessing the BktArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to BktArrayGet. Make sure you're accessing the BktArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	if (index >= array->length)
	{
		if (assertOnFailure) { AssertMsg(index < array->length, "BktArrayGet out of bounds!"); }
		return nullptr;
	}
	
	void* result = nullptr;
	BktArrayBkt* bucket = array->firstBucket;
	uxx currentIndex = 0;
	while (result == nullptr)
	{
		NotNull(bucket);
		if (index - currentIndex < bucket->length)
		{
			result = BktArrayBktGetItemPntr(array, bucket, (index - currentIndex));
			break;
		}
		else
		{
			currentIndex += bucket->length;
			bucket = bucket->next;
		}
	}
	
	return result;
}

PEXPI uxx BktArrayGetIndexOf_(uxx itemSize, uxx itemAlignment, const BktArray* array, const void* itemInQuestion)
{
	#if DEBUG_BUILD
	NotNull(array);
	Assert(IsBktArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to BktArrayGetIndexOf. Make sure you're accessing the BktArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to BktArrayGetIndexOf. Make sure you're accessing the BktArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	if (itemInQuestion == nullptr) { return array->length; }
	BktArrayBkt* bucket = array->firstBucket;
	uxx index = 0;
	while (bucket != nullptr)
	{
		u8* itemsBase = BktArrayBktGetItemPntr(array, bucket, 0);
		if (IsPntrWithin(itemsBase, (bucket->allocLength * array->itemSize), itemInQuestion))
		{
			uxx offsetFromBase = (uxx)((u8*)itemInQuestion - itemsBase);
			DebugAssert(offsetFromBase % array->itemSize == 0);
			return index + (offsetFromBase / array->itemSize);
		}
		index += bucket->length;
		bucket = bucket->next;
	}
	return array->length;
}

PEXPI bool BktArrayContains_(uxx itemSize, uxx itemAlignment, const BktArray* array, const void* itemInQuestion) { return (BktArrayGetIndexOf_(itemSize, itemAlignment, array, itemInQuestion) < array->length); }

PEXPI void* BktArrayAdd_(uxx itemSize, uxx itemAlignment, BktArray* array)
{
	#if DEBUG_BUILD
	NotNull(array);
	Assert(IsBktArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to BktArrayAdd. Make sure you're accessing the BktArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to BktArrayAdd. Make sure you're accessing the BktArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	BktArrayBkt* bucket = array->lastBucket;
	while (bucket != nullptr && bucket->length >= bucket->allocLength) { bucket = bucket->next; }
	
	if (bucket == nullptr)
	{
		BktArrayBkt* newBucket = (BktArrayBkt*)AllocMemAligned(array->arena, BktArrayAllocSize(array, array->defaultBucketSize), array->itemAlignment);
		if (newBucket == nullptr) { return nullptr; }
		ClearPointer(newBucket);
		newBucket->allocLength = array->defaultBucketSize;
		if (array->firstBucket == nullptr) { array->firstBucket = newBucket; }
		else
		{
			BktArrayBkt* lastBucket = array->lastBucket;
			while (lastBucket->next != nullptr) { lastBucket = lastBucket->next; }
			lastBucket->next = newBucket;
		}
		array->lastBucket = newBucket;
		array->numBuckets++;
		array->allocLength += newBucket->allocLength;
		bucket = newBucket;
	}
	
	void* result = BktArrayBktGetItemPntr(array, bucket, bucket->length);
	bucket->length++;
	array->length++;
	array->lastBucket = (bucket->length == bucket->allocLength && bucket->next != nullptr) ? bucket->next : bucket;
	
	return result;
}

PEXPI void* BktArrayAddSomewhere_(uxx itemSize, uxx itemAlignment, BktArray* array)
{
	#if DEBUG_BUILD
	NotNull(array);
	Assert(IsBktArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to BktArrayAddSomewhere. Make sure you're accessing the BktArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to BktArrayAddSomewhere. Make sure you're accessing the BktArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	
	void* result = nullptr;
	BktArrayBkt* bucket = array->firstBucket;
	while (bucket != nullptr)
	{
		if (bucket->length < bucket->allocLength)
		{
			result = BktArrayBktGetItemPntr(array, bucket, bucket->length);
			bucket->length++;
			array->length++;
			break;
		}
		bucket = bucket->next;
	}
	
	if (result == nullptr)
	{
		DebugAssert(array->length == array->allocLength);
		result = BktArrayAdd_(itemSize, itemAlignment, array);
	}
	
	return result;
}

//NOTE: This function returns a single pointer, meaning all items need to be in the same bucket, we may "waste" space if existing buckets don't have enough space for all items in which case we'll allocate a new bucket even when there was some space left
PEXP void* BktArrayAddMulti_(uxx itemSize, uxx itemAlignment, BktArray* array, uxx numItems)
{
	#if DEBUG_BUILD
	NotNull(array);
	Assert(IsBktArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to BktArrayAddMulti. Make sure you're accessing the BktArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to BktArrayAddMulti. Make sure you're accessing the BktArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	
	bool skippedEmptyBuckets = false;
	BktArrayBkt* bucket = array->lastBucket;
	while (bucket != nullptr && bucket->length + numItems > bucket->allocLength)
	{
		skippedEmptyBuckets |= (bucket->length == 0);
		bucket = bucket->next;
	}
	
	if (bucket == nullptr)
	{
		uxx bucketSize = MaxUXX(array->defaultBucketSize, numItems);
		BktArrayBkt* newBucket = (BktArrayBkt*)AllocMemAligned(array->arena, BktArrayAllocSize(array, bucketSize), array->itemAlignment);
		if (newBucket == nullptr) { return nullptr; }
		ClearPointer(newBucket);
		newBucket->allocLength = bucketSize;
		if (array->firstBucket == nullptr) { array->firstBucket = newBucket; }
		else
		{
			BktArrayBkt* lastBucket = array->lastBucket;
			DebugAssert(lastBucket->next == nullptr || lastBucket->next->length == 0);
			newBucket->next = lastBucket->next;
			lastBucket->next = newBucket;
		}
		array->lastBucket = newBucket;
		array->numBuckets++;
		array->allocLength += newBucket->allocLength;
		bucket = newBucket;
	}
	
	void* result = BktArrayBktGetItemPntr(array, bucket, bucket->length);
	bucket->length += numItems;
	array->length += numItems;
	
	//NOTE: Empty buckets can be safely moved to the end of the linked list which makes them potentially useful later for calls to Add (or smaller calls to AddMulti)
	if (skippedEmptyBuckets)
	{
		BktArrayBkt* currBucket = array->firstBucket;
		BktArrayBkt* prevBucket = nullptr;
		while (currBucket != bucket)
		{
			BktArrayBkt* nextBucket = currBucket->next;
			if (currBucket->length == 0)
			{
				if (prevBucket != nullptr) { prevBucket->next = currBucket->next; }
				else { array->firstBucket = currBucket->next; }
				currBucket->next = bucket->next;
				bucket->next = currBucket;
			}
			else { prevBucket = currBucket; }
			currBucket = nextBucket;
		}
	}
	
	array->lastBucket = (bucket->length == bucket->allocLength && bucket->next != nullptr) ? bucket->next : bucket;
	
	return result;
}

PEXPI void* BktArrayAddArray_(uxx itemSize, uxx itemAlignment, BktArray* destArray, const BktArray* srcArray)
{
	#if DEBUG_BUILD
	NotNull(destArray);
	Assert(IsBktArrayInit(destArray));
	NotNull(srcArray);
	Assert(IsBktArrayInit(srcArray));
	AssertMsg(destArray->itemSize == itemSize, "Invalid itemSize passed to BktArrayAddMulti. Make sure you're accessing the BktArray with the correct type!");
	AssertMsg(destArray->itemAlignment == itemAlignment, "Invalid itemAlignment passed to BktArrayAddMulti. Make sure you're accessing the BktArray with the correct type!");
	AssertMsg(srcArray->itemSize == itemSize, "Invalid itemSize of srcArray passed to BktArrayAddMulti. Make sure the two arrays are the same type!");
	AssertMsg(srcArray->itemAlignment == itemAlignment, "Invalid itemAlignment of srcArray passed to BktArrayAddMulti. Make sure the two arrays are the same type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	
	if (srcArray->length == 0) { return nullptr; }
	
	uxx numItems = srcArray->length; //NOTE: Important that we evaluate this first, since srcArray may be destArray
	u8* result = (u8*)BktArrayAddMulti_(itemSize, itemAlignment, destArray, numItems);
	if (result == nullptr) { return nullptr; }
	//TODO: We could potentially optimize this function for scenarios where srcArray isn't 1 bucket but the 
	if (srcArray->numBuckets == 1)
	{
		MyMemCopy(result, BktArrayBktGetItemPntr(srcArray, srcArray->firstBucket, 0), destArray->itemSize * numItems);
	}
	else
	{
		uxx index = 0;
		const BktArrayBkt* srcBucket = srcArray->firstBucket;
		while (srcBucket != nullptr && index < numItems)
		{
			uxx numItemsToCopy = MinUXX(numItems - index, srcBucket->length);
			MyMemCopy(result + (itemSize * index), BktArrayBktGetItemPntr(srcArray, srcBucket, 0), itemSize * numItemsToCopy);
			index += numItemsToCopy;
			srcBucket = srcBucket->next;
		}
		DebugAssert(index == numItems);
	}
	return result;
}

PEXP void BktArrayRemoveAt_(uxx itemSize, uxx itemAlignment, BktArray* array, uxx index)
{
	#if DEBUG_BUILD
	Assert(array != nullptr && true);
	Assert(IsBktArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to BktArrayRemoveAt. Make sure you're accessing the BktArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to BktArrayRemoveAt. Make sure you're accessing the BktArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	if (index >= array->length) { AssertMsg(index < array->length, "BktArrayRemoveAt out of bounds!"); return; }
	
	//If removing from the end and the lastBucket isn't empty then we can quickly remove!
	if (index == array->length-1 && array->lastBucket != nullptr && array->lastBucket->length > 0)
	{
		array->lastBucket->length--;
		array->length--;
		if (array->length == 0)
		{
			array->lastBucket = array->firstBucket;
		}
		else if (array->lastBucket->length == 0)
		{
			BktArrayBkt* beforeLastBucket = array->firstBucket;
			while (beforeLastBucket != nullptr && beforeLastBucket->next != array->lastBucket) { beforeLastBucket = beforeLastBucket->next; }
			DebugNotNull(beforeLastBucket);
			array->lastBucket = beforeLastBucket;
		}
		return;
	}
	
	BktArrayBkt* bucket = array->firstBucket;
	BktArrayBkt* prevBucket = nullptr;
	uxx baseIndex = 0;
	while (bucket != nullptr)
	{
		uxx removeIndex = (index - baseIndex);
		if (removeIndex < bucket->length)
		{
			u8* removePntr = BktArrayBktGetItemPntr(array, bucket, removeIndex);
			if (removeIndex+1 < bucket->length)
			{
				MyMemMove(removePntr, removePntr + array->itemSize, (bucket->length - (removeIndex+1)) * array->itemSize);
			}
			bucket->length--;
			array->length--;
			if (bucket->next == array->lastBucket && array->lastBucket->length == 0)
			{
				array->lastBucket = bucket;
			}
			if (bucket->length == 0)
			{
				if (array->lastBucket != bucket)
				{
					if (prevBucket != nullptr) { prevBucket->next = bucket->next; }
					else { array->firstBucket = bucket->next; }
					bucket->next = array->lastBucket->next;
					array->lastBucket->next = bucket;
				}
				else if (prevBucket != nullptr)
				{
					array->lastBucket = prevBucket;
				}
			}
			break;
		}
		baseIndex += bucket->length;
		prevBucket = bucket;
		bucket = bucket->next;
	}
	NotNull(bucket);
}

PEXPI void BktArrayRemove_(uxx itemSize, uxx itemAlignment, BktArray* array, const void* itemToRemove) { BktArrayRemoveAt_(itemSize, itemAlignment, array, BktArrayGetIndexOf_(itemSize, itemAlignment, array, itemToRemove)); }

PEXPI void BktArrayCopy(Arena* arena, BktArray* destArray, const BktArray* srcArray)
{
	NotNull(arena);
	NotNull(destArray);
	NotNull(srcArray);
	Assert(IsBktArrayInit(srcArray));
	InitBktArrayWithInitial_(srcArray->itemSize, srcArray->itemAlignment, destArray, arena, srcArray->defaultBucketSize, srcArray->length);
	BktArrayAddArray_(srcArray->itemSize, srcArray->itemAlignment, destArray, srcArray);
}

//TODO: If we keep inserting at 0 this performs terribly once we have two full buckets at the beginning. We end up generating a new empty bucket at the end on every insert
PEXP void* BktArrayInsert_(uxx itemSize, uxx itemAlignment, BktArray* array, uxx index)
{
	#if DEBUG_BUILD
	NotNull(array);
	Assert(IsBktArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to BktArrayInsert. Make sure you're accessing the BktArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to BktArrayInsert. Make sure you're accessing the BktArray with the correct type!");
	Assert(index <= array->length);
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	if (index > array->length) { return nullptr; }
	#endif
	if (index == array->length) { return BktArrayAdd_(itemSize, itemAlignment, array); }
	
	BktArrayBkt* prevBucket = nullptr;
	BktArrayBkt* bucket = array->firstBucket;
	uxx baseIndex = 0;
	while (bucket != nullptr && baseIndex < index)
	{
		if (baseIndex + bucket->length > index) { break; }
		baseIndex += bucket->length;
		prevBucket = bucket;
		bucket = bucket->next;
	}
	DebugAssertMsg(bucket != nullptr, "We reached the end of the bucket linked list in BktArrayInsert even through insertion index wasn't at the end of the array");
	
	void* result = nullptr;
	uxx insertIndex = (index - baseIndex);
	if (bucket->length < bucket->allocLength)
	{
		//insert into the bucket, push later elements up
		u8* itemsBase = BktArrayBktGetItemPntr(array, bucket, 0);
		if (insertIndex < bucket->length) { MyMemMove(itemsBase + (array->itemSize * (insertIndex+1)), itemsBase + (array->itemSize * insertIndex), array->itemSize * (bucket->length - insertIndex)); }
		bucket->length++;
		array->length++;
		if (array->lastBucket == bucket && bucket->next != nullptr) { array->lastBucket = bucket->next; }
		result = itemsBase + (array->itemSize * insertIndex);
	}
	else
	{
		BktArrayBkt* nextBucket = bucket->next;
		if (nextBucket != nullptr && nextBucket->length < nextBucket->allocLength)
		{
			//Make space in nextBucket for one more item
			u8* nextItemsBase = BktArrayBktGetItemPntr(array, nextBucket, 0);
			if (nextBucket->length > 0)
			{
				MyMemMove(nextItemsBase + (array->itemSize * 1), nextItemsBase, array->itemSize * nextBucket->length);
			}
			nextBucket->length++;
			array->length++;
			
			if (insertIndex < bucket->length)
			{
				//push one item into beginning of next bucket, move items in this bucket up by one
				u8* itemsBase = BktArrayBktGetItemPntr(array, bucket, 0);
				MyMemCopy(nextItemsBase, itemsBase + (array->itemSize * (bucket->length-1)), array->itemSize);
				if (insertIndex+1 < bucket->length)
				{
					MyMemMove(itemsBase + (array->itemSize * (insertIndex+1)), itemsBase + (array->itemSize * insertIndex), array->itemSize * (bucket->length - (insertIndex+1)));
				}
				result = itemsBase + (array->itemSize * insertIndex);
			}
			else
			{
				//push this item into the beginning of the next bucket
				result = nextItemsBase;
			}
			
			if (nextBucket->length == nextBucket->allocLength && nextBucket->next != nullptr && array->lastBucket == nextBucket)
			{
				array->lastBucket = nextBucket->next;
			}
		}
		else
		{
			//create a new bucket in-between this and next, move items after the index into the new bucket, leaving the current bucket partially filled
			uxx numItemsToMove = (bucket->length - insertIndex);
			uxx bucketSize = MaxUXX(1 + numItemsToMove, array->defaultBucketSize);
			BktArrayBkt* newBucket = (BktArrayBkt*)AllocMemAligned(array->arena, BktArrayAllocSize(array, bucketSize), array->itemAlignment);
			if (newBucket == nullptr) { return nullptr; }
			ClearPointer(newBucket);
			newBucket->allocLength = bucketSize;
			newBucket->length = 1 + numItemsToMove;
			newBucket->next = bucket->next;
			bucket->next = newBucket;
			u8* itemsBase = BktArrayBktGetItemPntr(array, bucket, 0);
			u8* newItemsBase = BktArrayBktGetItemPntr(array, newBucket, 0);
			if (numItemsToMove > 0)
			{
				MyMemCopy(newItemsBase + (array->itemSize * 1), itemsBase + (array->itemSize * insertIndex), array->itemSize * numItemsToMove);
			}
			bucket->length = insertIndex;
			if (array->lastBucket == bucket) { array->lastBucket = newBucket; }
			if (bucket->length == 0)
			{
				//If we emptied the bucket, move it to the end so it can be used
				if (prevBucket != nullptr) { prevBucket->next = newBucket; }
				else { array->firstBucket = newBucket; }
				bucket->next = array->lastBucket->next;
				array->lastBucket->next = bucket;
				if (array->lastBucket->length == array->lastBucket->allocLength) { array->lastBucket = bucket; }
			}
			array->numBuckets++;
			array->allocLength += newBucket->allocLength;
			array->length++;
			result = newItemsBase;
		}
	}
	
	return result;
}

//TODO: InsertMulti? InsertArray?

PEXPI void BktArrayCondenseInto(BktArray* array, Arena* intoArena, bool freeMemory)
{
	DebugNotNull(array);
	DebugAssert(IsBktArrayInit(array));
	if (intoArena == nullptr) { intoArena = array->arena; }
	
	BktArrayBkt* newBucket = nullptr;
	if (array->length > 0)
	{
		uxx newBucketSize = array->length;
		newBucket = (BktArrayBkt*)AllocMemAligned(intoArena, BktArrayAllocSize(array, newBucketSize), array->itemAlignment);
		NotNull(newBucket);
		ClearPointer(newBucket);
		newBucket->allocLength = newBucketSize;
		u8* newItemsBase = BktArrayBktGetItemPntr(array, newBucket, 0);
		uxx writeIndex = 0;
		BktArrayBkt* bucket = array->firstBucket;
		while (bucket != nullptr)
		{
			if (bucket->length > 0)
			{
				Assert(writeIndex + bucket->length <= newBucketSize);
				u8* itemsBase = BktArrayBktGetItemPntr(array, bucket, 0);
				MyMemCopy(newItemsBase + (array->itemSize * writeIndex), itemsBase, array->itemSize * bucket->length);
				writeIndex += bucket->length;
			}
			bucket = bucket->next;
		}
		Assert(writeIndex == array->length);
		newBucket->length = writeIndex;
	}
	
	if (CanArenaFree(array->arena) && freeMemory)
	{
		BktArrayBkt* bucket = array->firstBucket;
		while (bucket != nullptr)
		{
			BktArrayBkt* nextBucket = bucket->next;
			FreeMemAligned(array->arena, bucket, BktArrayAllocSize(array, bucket->allocLength), array->itemAlignment);
			bucket = nextBucket;
		}
	}
	
	array->arena = intoArena;
	array->firstBucket = newBucket;
	array->lastBucket = newBucket;
	array->numBuckets = (newBucket != nullptr) ? 1 : 0;
	array->allocLength = (newBucket != nullptr) ? newBucket->allocLength : 0;
}
PEXPI void BktArrayCondense(BktArray* array) { BktArrayCondenseInto(array, nullptr, true); }

PEXPI void BktArrayDropEmptyBuckets(BktArray* array)
{
	NotNull(array);
	Assert(IsBktArrayInit(array));
	BktArrayBkt* prevBucket = nullptr;
	BktArrayBkt* bucket = array->firstBucket;
	while (bucket != nullptr)
	{
		BktArrayBkt* nextBucket = bucket->next;
		if (bucket->length == 0)
		{
			if (prevBucket != nullptr) { prevBucket->next = bucket->next; }
			else { array->firstBucket = bucket->next; }
			if (array->lastBucket == bucket) { array->lastBucket = bucket->next; }
			array->numBuckets--;
			array->allocLength -= bucket->allocLength;
			if (CanArenaFree(array->arena)) { FreeMemAligned(array->arena, bucket, BktArrayAllocSize(array, bucket->allocLength), array->itemAlignment); }
		}
		else { prevBucket = bucket; }
		bucket = nextBucket;
	}
}

PEXPI uxx BktArrayGetBucketIndexAt(BktArray* array, uxx itemIndex, uxx* innerIndexOut)
{
	NotNull(array);
	Assert(IsBktArrayInit(array));
	Assert(itemIndex < array->length);
	BktArrayBkt* bucket = array->firstBucket;
	uxx baseIndex = 0;
	uxx bucketIndex = 0;
	while (bucket != nullptr)
	{
		if (itemIndex < baseIndex + bucket->length)
		{
			SetOptionalOutPntr(innerIndexOut, itemIndex - baseIndex);
			return bucketIndex;
		}
		baseIndex += bucket->length;
		bucket = bucket->next;
		bucketIndex++;
	}
	SetOptionalOutPntr(innerIndexOut, 0);
	return array->numBuckets;
}
PEXPI uxx BktArrayGetBucketIndex(BktArray* array, const void* itemPntr, uxx* innerIndexOut)
{
	NotNull(array);
	Assert(IsBktArrayInit(array));
	BktArrayBkt* bucket = array->firstBucket;
	uxx bucketIndex = 0;
	while (bucket != nullptr)
	{
		u8* itemsBase = BktArrayBktGetItemPntr(array, bucket, 0);
		if (IsPntrWithin(itemsBase, array->itemSize * bucket->allocLength, itemPntr))
		{
			SetOptionalOutPntr(innerIndexOut, (uxx)((u8*)itemPntr - itemsBase) / array->itemSize);
			return bucketIndex;
		}
		bucket = bucket->next;
		bucketIndex++;
	}
	SetOptionalOutPntr(innerIndexOut, 0);
	return array->numBuckets;
}

PEXPI BktArrayBkt* BktArrayGetBucket(BktArray* array, uxx bucketIndex)
{
	NotNull(array);
	Assert(IsBktArrayInit(array));
	if (bucketIndex >= array->numBuckets) { return nullptr; }
	BktArrayBkt* bucket = array->firstBucket;
	uxx currBucketIndex = 0;
	while (bucket != nullptr)
	{
		if (currBucketIndex == bucketIndex) { return bucket; }
		bucket = bucket->next;
		currBucketIndex++;
	}
	return nullptr;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_BKT_ARRAY_H

#if defined(_MISC_SORTING_H) && defined(_STRUCT_BKT_ARRAY_H)
#include "cross/cross_sorting_and_bkt_array.h"
#endif
