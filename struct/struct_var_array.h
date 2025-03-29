/*
File:   struct_var_array.h
Author: Taylor Robbins
Date:   01\04\2025
Description:
	** Defines a VarArray structure which is an array that can reallocate as it
	** grows larger allowing for any number of elements to be pushed on, as long
	** as the Arena backing the array can allocate more memory.
	** This is a C structure, not a C++ templated class like vector<T> so we cannot
	** be really type safe without a lot of work. Instead we use the size of the elements
	** as a rough heuristic to Assert on before doing pointer casting to any particular type,
	** and we use preprocessor macros to tie the pointer cast and sizeof together.
	** The macros also allow us to inject func+file+line information into the call
	** when doing debug builds so we can diagnose which call site caused a problem.
	** To help combat this we will also make some specific 
*/

//TODO: Add VarArraySet which memcpy's some value into a slot

/*
* Usage Example:
*	VarArray numbers;
*	void main()
*	{
*		Arena* arena = ...;
*		InitVarArray(u32, &numbers, arena);
*		while (rand()%100 < 80)
*		{
*			u32* newNum = VarArrayAdd(u32, &numbers);
*			*newNum = (u32)rand();
*		}
*		printf("Generated %llu number(s)\n", numbers.length);
*		VarArrayLoop(&numbers, numIndex)
*		{
*			VarArrayLoopGet(u32, num, &numbers, numIndex);
*			printf("[%llu] %u\n", numIndex, num);
*		}
*		if (numbers.length > 10 && VarArrayGetValue(u32, &numbers, 9) == 42) { printf("You Win!\n"); }
*		FreeVarArray(&numbers);
*	}
*/

#ifndef _STRUCT_VAR_ARRAY_H
#define _STRUCT_VAR_ARRAY_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "mem/mem_arena.h"
#include "std/std_memset.h"

#define VAR_ARRAY_MIN_SIZE    4 //items (a VarArray is actually 0 items until at least one element is pushed on. But at that point it jumps up to this number at least)

#ifndef VAR_ARRAY_DEBUG_INFO
#define VAR_ARRAY_DEBUG_INFO DEBUG_BUILD
#endif

#ifndef VAR_ARRAY_CLEAR_ITEMS_ON_ADD
#define VAR_ARRAY_CLEAR_ITEMS_ON_ADD DEBUG_BUILD
#endif
#ifndef VAR_ARRAY_CLEAR_ITEM_BYTE_VALUE
#define VAR_ARRAY_CLEAR_ITEM_BYTE_VALUE 0xCC
#endif

// +--------------------------------------------------------------+
// |                        Data Structure                        |
// +--------------------------------------------------------------+
typedef struct VarArray VarArray;
struct VarArray
{
	Arena* arena; //doubles as IsInit check
	uxx itemSize;
	uxx itemAlignment;
	uxx maxLength;
	
	#if VAR_ARRAY_DEBUG_INFO
	const char* creationFilePath;
	uxx creationLineNumber;
	const char* creationFuncName;
	#endif
	
	uxx length;
	uxx allocLength;
	void* items;
};

#define ARRAY_VISIT_FUNC_DEF(functionName) void functionName(uxx itemIndex, void* item)
typedef ARRAY_VISIT_FUNC_DEF(ArrayVisitFunc_f);

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	#if VAR_ARRAY_DEBUG_INFO
	void InitVarArray_(const char* filePath, uxx lineNumber, const char* funcName, uxx itemSize, uxx itemAlignment, VarArray* array, Arena* arena, uxx initialCountNeeded);
	#else
	void InitVarArray_(uxx itemSize, uxx itemAlignment, VarArray* array, Arena* arena, uxx initialCountNeeded);
	#endif
	PIG_CORE_INLINE bool IsVarArrayInit(const VarArray* array);
	void FreeVarArray(VarArray* array);
	void VarArrayClearEx(VarArray* array, bool deallocate);
	PIG_CORE_INLINE void VarArrayClear(VarArray* array);
	bool VarArrayExpand(VarArray* array, uxx capacityRequired);
	bool VarArrayContains_(uxx itemSize, uxx itemAlignment, VarArray* array, const void* itemInQuestion);
	PIG_CORE_INLINE bool VarArrayGetIndexOf_(uxx itemSize, uxx itemAlignment, VarArray* array, const void* itemInQuestion, uxx* indexOut);
	PIG_CORE_INLINE void VarArrayVisit(VarArray* array, ArrayVisitFunc_f* visitFunc);
	PIG_CORE_INLINE void* VarArrayGet_(uxx itemSize, uxx itemAlignment, const VarArray* array, uxx index, bool assertOnFailure);
	void* VarArrayAdd_(uxx itemSize, uxx itemAlignment, VarArray* array);
	void* VarArrayAddMulti_(uxx itemSize, uxx itemAlignment, VarArray* array, uxx numItems);
	void* VarArrayInsert_(uxx itemSize, uxx itemAlignment, VarArray* array, uxx index);
	void VarArrayRemoveAt_(uxx itemSize, uxx itemAlignment, VarArray* array, uxx index);
	PIG_CORE_INLINE void VarArrayRemove_(uxx itemSize, uxx itemAlignment, VarArray* array, const void* itemToRemove);
	#if VAR_ARRAY_DEBUG_INFO
	void VarArrayCopy_(const char* filePath, uxx lineNumber, const char* funcName, VarArray* destArray, const VarArray* sourceArray, Arena* arena);
	#else
	void VarArrayCopy_(VarArray* destArray, const VarArray* sourceArray, Arena* arena);
	#endif
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#if VAR_ARRAY_DEBUG_INFO
	#if LANGUAGE_IS_C
	#define InitVarArrayWithInitial(type, arrayPntr, arena, initialCountNeeded) InitVarArray_(__FILE__, __LINE__, __func__, (uxx)sizeof(type), (uxx)_Alignof(type), (arrayPntr), (arena), (initialCountNeeded))
	#define InitVarArray(type, arrayPntr, arena) InitVarArray_(__FILE__, __LINE__, __func__, (uxx)sizeof(type), (uxx)_Alignof(type), (arrayPntr), (arena), 0)
	#else
	#define InitVarArrayWithInitial(type, arrayPntr, arena, initialCountNeeded) InitVarArray_(__FILE__, __LINE__, __func__, (uxx)sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (arena), (initialCountNeeded))
	#define InitVarArray(type, arrayPntr, arena) InitVarArray_(__FILE__, __LINE__, __func__, (uxx)sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (arena), 0)
	#endif
#else
	#if LANGUAGE_IS_C
	#define InitVarArrayWithInitial(type, arrayPntr, arena, initialCountNeeded) InitVarArray_((uxx)sizeof(type), (uxx)_Alignof(type), (arrayPntr), (arena), (initialCountNeeded))
	#define InitVarArray(type, arrayPntr, arena) InitVarArray_((uxx)sizeof(type), (uxx)_Alignof(type), (arrayPntr), (arena), 0)
	#else
	#define InitVarArrayWithInitial(type, arrayPntr, arena, initialCountNeeded) InitVarArray_((uxx)sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (arena), (initialCountNeeded))
	#define InitVarArray(type, arrayPntr, arena) InitVarArray_((uxx)sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (arena), 0)
	#endif
#endif

#if LANGUAGE_IS_C
#define VarArrayContains(type, arrayPntr, itemPntrInQuestion) VarArrayContains_((uxx)sizeof(type), (uxx)_Alignof(type), (arrayPntr), (itemPntrInQuestion))
#else
#define VarArrayContains(type, arrayPntr, itemPntrInQuestion) VarArrayContains_((uxx)sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (itemPntrInQuestion))
#endif

#if LANGUAGE_IS_C
#define VarArrayGetIndexOf(type, arrayPntr, itemInQuestion, indexOut) VarArrayGetIndexOf_((uxx)sizeof(type), (uxx)_Alignof(type), (arrayPntr), (itemInQuestion), (indexOut))
#else
#define VarArrayGetIndexOf(type, arrayPntr, itemInQuestion, indexOut) VarArrayGetIndexOf_((uxx)sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (itemInQuestion), (indexOut))
#endif

#define VarArrayLoop(arrayPntr, indexVarName) for (uxx indexVarName = 0; indexVarName < (arrayPntr)->length; indexVarName++)
#define VarArrayLoopGet(type, varName, arrayPntr, indexVarName) type* varName = (((type*)(arrayPntr)->items) + (indexVarName));
#define VarArrayLoopGetValue(type, varName, arrayPntr, indexVarName) type varName = *(((type*)(arrayPntr)->items) + (indexVarName));

//Hard indicates we want to assertOnFailure, opposed to Soft which will return nullptr. Not specifying leads to implicitly using Hard variant
#if LANGUAGE_IS_C
#define VarArrayGetHard(type, arrayPntr, index) ((type*)VarArrayGet_((uxx)sizeof(type), (uxx)_Alignof(type), (arrayPntr), (index), true))
#define VarArrayGetSoft(type, arrayPntr, index) ((type*)VarArrayGet_((uxx)sizeof(type), (uxx)_Alignof(type), (arrayPntr), (index), false))
#else
#define VarArrayGetHard(type, arrayPntr, index) ((type*)VarArrayGet_((uxx)sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (index), true))
#define VarArrayGetSoft(type, arrayPntr, index) ((type*)VarArrayGet_((uxx)sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (index), false))
#endif
#define VarArrayGet(type, arrayPntr, index) VarArrayGetHard(type, (arrayPntr), (index))

//Shorthand for passing 0 for index
#define VarArrayGetFirstHard(type, arrayPntr) VarArrayGetHard(type, (arrayPntr), 0)
#define VarArrayGetFirstSoft(type, arrayPntr) (((arrayPntr)->length > 0) ? VarArrayGetSoft(type, (arrayPntr), 0) : nullptr)
#define VarArrayGetFirst(type, arrayPntr) VarArrayGetFirstHard(type, (arrayPntr)) 

//Shorthand for passing arrayPntr->length-1 for index
#define VarArrayGetLastHard(type, arrayPntr) VarArrayGetHard(type, (arrayPntr), (arrayPntr)->length - 1)
#define VarArrayGetLastSoft(type, arrayPntr) (((arrayPntr)->length > 0) ? VarArrayGetSoft(type, (arrayPntr), (arrayPntr)->length - 1) : nullptr)
#define VarArrayGetLast(type, arrayPntr) VarArrayGetLastHard(type, (arrayPntr)) 

//Getting the value means dereferencing the pointer, this only works with Hard variants since dereference will imply a crash on nullptr
#define VarArrayGetValueHard(type, arrayPntr, index) *VarArrayGetHard(type, (arrayPntr), (index))
#define VarArrayGetValue(type, arrayPntr, index) VarArrayGetValueHard(type, (arrayPntr), (index))

#define VarArrayGetFirstValueHard(type, arrayPntr) *VarArrayGetHard(type, (arrayPntr), 0)
#define VarArrayGetFirstValue(type, arrayPntr) VarArrayGetFirstValueHard(type, (arrayPntr))

#define VarArrayGetLastValueHard(type, arrayPntr) *VarArrayGetHard(type, (arrayPntr), (arrayPntr)->length-1)
#define VarArrayGetLastValue(type, arrayPntr) VarArrayGetLastValueHard(type, (arrayPntr))

#if LANGUAGE_IS_C
#define VarArrayAdd(type, arrayPntr) (type*)VarArrayAdd_((uxx)sizeof(type), (uxx)_Alignof(type), (arrayPntr))
#else
#define VarArrayAdd(type, arrayPntr) (type*)VarArrayAdd_((uxx)sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr))
#endif
#define VarArrayAddValue(type, arrayPntr, value) do                  \
{                                                                    \
	/* We must evaluate (value) before manipulating the array */     \
	/* because it may access/refer to elements in the array   */     \
	type valueBeforeAdd_NOCONFLICT = (value);                        \
	type* addedItemPntr_NOCONFLICT = VarArrayAdd(type, (arrayPntr)); \
	DebugNotNull(addedItemPntr_NOCONFLICT);                          \
	*addedItemPntr_NOCONFLICT = valueBeforeAdd_NOCONFLICT;           \
} while(0)
// This is simply an alias of VarArrayAddValue, but it's here to match the name of VarArrayPop below
#define VarArrayPush(type, arrayPntr, value) VarArrayAddValue(type, (arrayPntr), (value))

#if LANGUAGE_IS_C
#define VarArrayAddMulti(type, arrayPntr, numItems) VarArrayAddMulti_((uxx)sizeof(type), (uxx)_Alignof(type), (arrayPntr), (numItems))
#else
#define VarArrayAddMulti(type, arrayPntr, numItems) VarArrayAddMulti_((uxx)sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (numItems))
#endif

#if LANGUAGE_IS_C
#define VarArrayInsert(type, arrayPntr, index) (type*)VarArrayInsert_((uxx)sizeof(type), (uxx)_Alignof(type), (arrayPntr), (index))
#else
#define VarArrayInsert(type, arrayPntr, index) (type*)VarArrayInsert_((uxx)sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (index))
#endif
//NOTE: The (value) portion is evaluated AFTER the Insert occurs, it's meaning may change if it's accessing the array or using a pointer from the array!
#define VarArrayInsertValue(type, arrayPntr, index, value) do                       \
{                                                                                   \
	/* We must evaluate (value) before manipulating the array */                    \
	/* because it may access/refer to elements in the array   */                    \
	type valueBeforeInsert_NOCONFLICT = (value);                                    \
	type* insertedItemPntr_NOCONFLICT = VarArrayInsert(type, (arrayPntr), (index)); \
	DebugNotNull(insertedItemPntr_NOCONFLICT);                                      \
	*insertedItemPntr_NOCONFLICT = valueBeforeInsert_NOCONFLICT;                    \
} while(0)

#if LANGUAGE_IS_C
#define VarArrayRemoveAt(type, arrayPntr, index) VarArrayRemoveAt_((uxx)sizeof(type), (uxx)_Alignof(type), (arrayPntr), (index))
#else
#define VarArrayRemoveAt(type, arrayPntr, index) VarArrayRemoveAt_((uxx)sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (index))
#endif
#define VarArrayGetAndRemoveValueAt(type, arrayPntr, index) VarArrayGetValue(type, (arrayPntr), (index)); VarArrayRemoveAt(type, (arrayPntr), (index))
#define VarArrayPop(type, arrayPntr) VarArrayGetValue(type, (arrayPntr), (arrayPntr)->length-1); VarArrayRemoveAt(type, (arrayPntr), (arrayPntr)->length-1)
#define VarArrayRemoveFirst(type, arrayPntr) VarArrayRemoveAt(type, (arrayPntr), 0)
#define VarArrayRemoveLast(type, arrayPntr) VarArrayRemoveAt(type, (arrayPntr), (arrayPntr)->length-1)

#if LANGUAGE_IS_C
#define VarArrayRemove(type, arrayPntr, itemPntr) VarArrayRemove_((uxx)sizeof(type), (uxx)_Alignof(type), (arrayPntr), (itemPntr))
#else
#define VarArrayRemove(type, arrayPntr, itemPntr) VarArrayRemove_((uxx)sizeof(type), (uxx)std::alignment_of<type>(), (arrayPntr), (itemPntr))
#endif

#if VAR_ARRAY_DEBUG_INFO
#define VarArrayCopy(destArray, sourceArray, arenaPntr) VarArrayCopy_(__FILE__, __LINE__, __func__, (destArray), (sourceArray), (arenaPntr))
#else
#define VarArrayCopy(destArray, sourceArray, arenaPntr) VarArrayCopy_((destArray), (sourceArray), (arenaPntr))
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP bool VarArrayExpand(struct VarArray* array, uxx capacityRequired);

// +--------------------------------------------------------------+
// |                     Initialize VarArray                      |
// +--------------------------------------------------------------+
#if VAR_ARRAY_DEBUG_INFO
PEXP void InitVarArray_(const char* filePath, uxx lineNumber, const char* funcName, uxx itemSize, uxx itemAlignment, VarArray* array, Arena* arena, uxx initialCountNeeded)
#else
PEXP void InitVarArray_(uxx itemSize, uxx itemAlignment, VarArray* array, Arena* arena, uxx initialCountNeeded)
#endif
{
	NotNull(array);
	NotNull(arena);
	Assert(itemSize > 0);
	ClearPointer(array); //TODO: At some point in the past I claimed this was slow, we should verify that! Do we really do InitVarArray that often?? Is memset really that slow?
	array->arena = arena;
	array->itemSize = itemSize;
	array->itemAlignment = itemAlignment;
	#if VAR_ARRAY_DEBUG_INFO
	array->creationFilePath = filePath;
	array->creationLineNumber = lineNumber;
	array->creationFuncName = funcName;
	#endif
	VarArrayExpand(array, initialCountNeeded);
}

PEXPI bool IsVarArrayInit(const VarArray* array)
{
	return (array->arena != nullptr);
}

// +--------------------------------------------------------------+
// |                        Free VarArray                         |
// +--------------------------------------------------------------+
PEXP void FreeVarArray(VarArray* array)
{
	NotNull(array);
	Assert(IsVarArrayInit(array));
	if (array->allocLength > 0)
	{
		NotNull(array->items);
		if (CanArenaAllocAligned(array->arena))
		{
			FreeMemAligned(array->arena, array->items, array->itemSize * array->allocLength, array->itemAlignment);
		}
		else
		{
			FreeMem(array->arena, array->items, array->itemSize * array->allocLength);
		}
	}
	ClearPointer(array);
}

// +--------------------------------------------------------------+
// |                        Clear VarArray                        |
// +--------------------------------------------------------------+
PEXP void VarArrayClearEx(VarArray* array, bool deallocate)
{
	NotNull(array);
	if (deallocate && array->allocLength > 0)
	{
		DebugNotNull(array->arena);
		if (CanArenaAllocAligned(array->arena))
		{
			FreeMemAligned(array->arena, array->items, array->itemSize * array->allocLength, array->itemAlignment);
		}
		else
		{
			FreeMem(array->arena, array->items, array->itemSize * array->allocLength);
		}
		array->items = nullptr;
		array->allocLength = 0;
	}
	array->length = 0;
}
PEXPI void VarArrayClear(VarArray* array) { VarArrayClearEx(array, false); }

// +--------------------------------------------------------------+
// |                       Expand VarArray                        |
// +--------------------------------------------------------------+
PEXP bool VarArrayExpand(VarArray* array, uxx capacityRequired) //pre-declared at top of file
{
	NotNull(array);
	NotNull(array->arena);
	Assert(array->itemSize > 0);
	if (array->allocLength >= capacityRequired) { return true; }
	if (array->maxLength > 0 && capacityRequired > array->maxLength) { return false; }
	
	uxx newLength = 0;
	newLength = array->allocLength > 0 ? array->allocLength : VAR_ARRAY_MIN_SIZE;
	while (newLength < capacityRequired)
	{
		//double in size, but don't overflow uxx type
		newLength = (newLength <= UINTXX_MAX/2) ? (newLength*2) : UINTXX_MAX;
	}
	if (array->maxLength > 0 && newLength > array->maxLength) { newLength = array->maxLength; }
	DebugAssert(newLength >= capacityRequired);
	DebugAssert(newLength <= (UINT64_MAX / array->itemSize));
	
	void* newSpace = nullptr;
	if (CanArenaAllocAligned(array->arena))
	{
		newSpace = AllocMemAligned(array->arena, newLength * array->itemSize, array->itemAlignment);
	}
	else
	{
		newSpace = AllocMem(array->arena, array->allocLength * array->itemSize);
	}
	
	if (newSpace == nullptr)
	{
		// PrintLine_E("Failed to expand variable array %s to %llu items at %llu bytes each", (array->name.pntr != nullptr) ? array->name.pntr : "[unnamed]", newLength, array->itemSize);
		AssertMsg(false, "Failed to expand VarArray!");
		return false;
	}
	
	if (array->length > 0)
	{
		MyMemCopy(newSpace, array->items, array->length * array->itemSize);
	}
	if (array->items != nullptr)
	{
		if (CanArenaAllocAligned(array->arena))
		{
			FreeMemAligned(array->arena, array->items, array->itemSize * array->allocLength, array->itemAlignment);
		}
		else
		{
			FreeMem(array->arena, array->items, array->itemSize * array->allocLength);
		}
	}
	
	array->items = newSpace;
	array->allocLength = newLength;
	return true;
}

// +--------------------------------------------------------------+
// |                        Query VarArray                        |
// +--------------------------------------------------------------+
PEXP bool VarArrayContains_(uxx itemSize, uxx itemAlignment, VarArray* array, const void* itemInQuestion)
{
	NotNull(array);
	#if DEBUG_BUILD
	Assert(array->itemSize > 0);
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to VarArrayContains. Make sure you're accessing the VarArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to VarArrayContains. Make sure you're accessing the VarArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	if (itemInQuestion == nullptr) { return false; }
	if (array->items == nullptr) { return false; }
	if (!IsPntrWithin(array->items, array->length * array->itemSize, itemInQuestion)) { return false; }
	#if DEBUG_BUILD
	//Make sure the item is aligned properly
	uxx offsetFromBase = (uxx)(((const u8*)itemInQuestion) - (u8*)array->items);
	Assert((offsetFromBase % array->itemSize) == 0);
	#endif
	return true;
}

PEXPI bool VarArrayGetIndexOf_(uxx itemSize, uxx itemAlignment, VarArray* array, const void* itemInQuestion, uxx* indexOut)
{
	if (!VarArrayContains_(itemSize, itemAlignment, array, itemInQuestion)) { return false; }
	uxx offsetFromBase = (uxx)(((const u8*)itemInQuestion) - (u8*)array->items);
	DebugAssert((offsetFromBase % array->itemSize) == 0);
	DebugNotNull(indexOut); //Call VarArrayContains instead of VarArrayGetIndexOf if you don't need the index!
	*indexOut = (offsetFromBase / array->itemSize);
	return true;
}

// +--------------------------------------------------------------+
// |                 Visit/Iterate Over VarArray                  |
// +--------------------------------------------------------------+
PEXPI void VarArrayVisit(VarArray* array, ArrayVisitFunc_f* visitFunc)
{
	DebugNotNull(array);
	DebugNotNull(visitFunc);
	for (uxx iIndex = 0; iIndex < array->length; iIndex++)
	{
		visitFunc(iIndex, (u8*)array->items + (iIndex * array->itemSize));
	}
}

// +--------------------------------------------------------------+
// |                    Get Item From VarArray                    |
// +--------------------------------------------------------------+
PEXPI void* VarArrayGet_(uxx itemSize, uxx itemAlignment, const VarArray* array, uxx index, bool assertOnFailure)
{
	NotNull(array);
	#if DEBUG_BUILD
	Assert(IsVarArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to VarArrayGet. Make sure you're accessing the VarArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to VarArrayGet. Make sure you're accessing the VarArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	if (index >= array->length)
	{
		if (assertOnFailure)
		{
			// GyLibPrintLine_E("Tried to get item %llu/%llu in VarArray %s of itemSize %llu", index, array->length, (array->name.pntr != nullptr) ? array->name.pntr : "[unnamed]", array->itemSize);
			AssertMsg(false, "VarArrayGet out of bounds!");
		}
		return nullptr;
	}
	DebugNotNull(array->items);
	void* result = (((u8*)array->items) + (index * array->itemSize));
	return result;
}

// +--------------------------------------------------------------+
// |                     Add Item to VarArray                     |
// +--------------------------------------------------------------+
//NOTE: This always asserts on failure to allocate since VarArrayExpand defaults to asserting
PEXP void* VarArrayAdd_(uxx itemSize, uxx itemAlignment, VarArray* array)
{
	#if DEBUG_BUILD
	NotNull(array);
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to VarArrayAdd. Make sure you're accessing the VarArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to VarArrayAdd. Make sure you're accessing the VarArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	if (array->maxLength > 0 && array->length >= array->maxLength) { return nullptr; }
	
	VarArrayExpand(array, array->length+1);
	DebugAssert(array->allocLength >= array->length+1);
	
	void* result = (((u8*)array->items) + (array->itemSize * array->length));
	#if VAR_ARRAY_CLEAR_ITEMS_ON_ADD
	MyMemSet(result, VAR_ARRAY_CLEAR_ITEM_BYTE_VALUE, array->itemSize);
	#endif
	array->length++;
	
	return result;
}

PEXP void* VarArrayAddMulti_(uxx itemSize, uxx itemAlignment, VarArray* array, uxx numItems)
{
	#if DEBUG_BUILD
	NotNull(array);
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to VarArrayAddMulti. Make sure you're accessing the VarArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to VarArrayAddMulti. Make sure you're accessing the VarArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	
	if (numItems == 0) { return nullptr; }
	if (!VarArrayExpand(array, array->length+numItems)) { return nullptr; }
	
	void* result = nullptr;
	uxx numItemsBefore = array->length;
	for (uxx iIndex = 0; iIndex < numItems; iIndex++)
	{
		void* newItem = VarArrayAdd_(itemSize, itemAlignment, array);
		if (newItem == nullptr)
		{
			array->length = numItemsBefore;
			return nullptr;
		}
		else if (result == nullptr) { result = newItem; }
	}
	
	return result;
}

// +--------------------------------------------------------------+
// |                  Insert Item Into VarArray                   |
// +--------------------------------------------------------------+
PEXP void* VarArrayInsert_(uxx itemSize, uxx itemAlignment, VarArray* array, uxx index)
{
	DebugNotNull(array);
	DebugAssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to VarArrayInsert. Make sure you're accessing the VarArray with the correct type!");
	DebugAssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to VarArrayInsert. Make sure you're accessing the VarArray with the correct type!");
	Assert(index <= array->length);
	if (array->maxLength > 0 && array->length >= array->maxLength) { return nullptr; }
	
	if (index == array->length) { return VarArrayAdd_(itemSize, itemAlignment, array); }
	
	VarArrayExpand(array, array->length+1);
	DebugAssert(array->allocLength >= array->length+1);
	
	//Move all items above the index up by one slot
	uxx insertionIndex = (index * array->itemSize);
	u8* insertionPntr = ((u8*)array->items) + insertionIndex;
	MyMemMove(insertionPntr + 1*array->itemSize, insertionPntr, (size_t)((array->length*array->itemSize) - insertionIndex));
	
	void* result = insertionPntr;
	#if VAR_ARRAY_CLEAR_ITEMS_ON_ADD
	MyMemSet(result, VAR_ARRAY_CLEAR_ITEM_BYTE_VALUE, array->itemSize);
	#endif
	array->length++;
	
	return result;
}

// +--------------------------------------------------------------+
// |               Remove Item at Index in VarArray               |
// +--------------------------------------------------------------+
PEXP void VarArrayRemoveAt_(uxx itemSize, uxx itemAlignment, VarArray* array, uxx index)
{
	#if DEBUG_BUILD
	NotNull(array);
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to VarArrayRemove. Make sure you're accessing the VarArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to VarArrayRemove. Make sure you're accessing the VarArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	AssertMsg(index < array->length, "VarArrayRemove index out of bounds!");
	//move all items above index down by one
	if (index < array->length-1)
	{
		u8* removePntr = ((u8*)array->items) + (index * array->itemSize);
		uxx numItemsToShift = array->length - (index+1);
		MyMemMove(removePntr, removePntr + 1*array->itemSize, numItemsToShift * array->itemSize);
	}
	array->length--;
}

// +--------------------------------------------------------------+
// |               Remove Item by Pntr in VarArray                |
// +--------------------------------------------------------------+
PEXPI void VarArrayRemove_(uxx itemSize, uxx itemAlignment, VarArray* array, const void* itemToRemove)
{
	uxx itemIndex = 0;
	bool itemInArray = VarArrayGetIndexOf_(itemSize, itemAlignment, array, itemToRemove, &itemIndex);
	Assert(itemInArray);
	VarArrayRemoveAt_(itemSize, itemAlignment, array, itemIndex);
}

// +--------------------------------------------------------------+
// |                 Copy VarArray Into New Arena                 |
// +--------------------------------------------------------------+
#if VAR_ARRAY_DEBUG_INFO
PEXP void VarArrayCopy_(const char* filePath, uxx lineNumber, const char* funcName, VarArray* destArray, const VarArray* sourceArray, Arena* arena)
#else
PEXP void VarArrayCopy_(VarArray* destArray, const VarArray* sourceArray, Arena* arena)
#endif
{
	NotNull(arena);
	NotNull(destArray);
	NotNull(sourceArray);
	Assert(destArray != sourceArray);
	#if VAR_ARRAY_DEBUG_INFO
	InitVarArray_(filePath, lineNumber, funcName, sourceArray->itemSize, sourceArray->itemAlignment, destArray, arena, sourceArray->length);
	#else
	InitVarArray_(sourceArray->itemSize, sourceArray->itemAlignment, destArray, arena, sourceArray->length);
	#endif
	destArray->maxLength = sourceArray->maxLength;
	if (sourceArray->length > 0)
	{
		DebugAssert(destArray->allocLength >= sourceArray->length);
		NotNull(destArray->items);
		destArray->length = sourceArray->length;
		MyMemCopy(destArray->items, sourceArray->items, sourceArray->length * sourceArray->itemSize);
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_VAR_ARRAY_H
