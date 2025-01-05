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
*		if (numbers.length > 10 && *VarArrayGet(u32, &numbers, 9) == 42) { printf("You Win!\n"); }
*		FreeVarArray(&numbers);
*	}
*/

#ifndef _STRUCT_VAR_ARRAY_H
#define _STRUCT_VAR_ARRAY_H

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

bool VarArrayExpand(struct VarArray* array, uxx capacityRequired);

// +--------------------------------------------------------------+
// |                     Initialize VarArray                      |
// +--------------------------------------------------------------+
#if VAR_ARRAY_DEBUG_INFO
void InitVarArray_(const char* filePath, uxx lineNumber, const char* funcName, uxx itemSize, uxx itemAlignment, VarArray* array, Arena* arena, uxx initialCountNeeded)
#else
void InitVarArray_(uxx itemSize, uxx itemAlignment, VarArray* array, Arena* arena, uxx initialCountNeeded)
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
#if VAR_ARRAY_DEBUG_INFO
#define InitVarArrayWithInitial(type, array, arena, initialCountNeeded) InitVarArray_(__FILE__, __LINE__, __func__, (uxx)sizeof(type), (uxx)_Alignof(type), (array), (arena), (initialCountNeeded))
#define InitVarArray(type, array, arena) InitVarArray_(__FILE__, __LINE__, __func__, (uxx)sizeof(type), (uxx)_Alignof(type), (array), (arena), 0)
#else
#define InitVarArrayWithInitial(type, array, arena, initialCountNeeded) InitVarArray_((uxx)sizeof(type), (uxx)_Alignof(type), (array), _Alignof(type), (initialCountNeeded))
#define InitVarArray(type, array, arena) InitVarArray_((uxx)sizeof(type), (uxx)_Alignof(type), (array), _Alignof(type), 0)
#endif

bool IsVarArrayInit(const VarArray* array)
{
	return (array->arena != nullptr);
}

// +--------------------------------------------------------------+
// |                        Free VarArray                         |
// +--------------------------------------------------------------+
void FreeVarArray(VarArray* array)
{
	NotNull(array);
	Assert(IsVarArrayInit(array));
	if (array->allocLength > 0)
	{
		NotNull(array->items);
		FreeMem(array->arena, array->items, array->itemSize * array->allocLength);
	}
	ClearPointer(array);
}

// +--------------------------------------------------------------+
// |                        Clear VarArray                        |
// +--------------------------------------------------------------+
void VarArrayClearEx(VarArray* array, bool deallocate)
{
	NotNull(array);
	if (deallocate && array->allocLength > 0)
	{
		DebugNotNull(array->arena);
		FreeMem(array->arena, array->items, array->allocLength * array->itemSize);
		array->items = nullptr;
		array->allocLength = 0;
	}
	array->length = 0;
}
void VarArrayClear(VarArray* array) { VarArrayClearEx(array, false); }

// +--------------------------------------------------------------+
// |                       Expand VarArray                        |
// +--------------------------------------------------------------+
bool VarArrayExpand(VarArray* array, uxx capacityRequired) //pre-declared at top of file
{
	NotNull(array);
	NotNull(array->arena);
	Assert(array->itemSize > 0);
	if (array->allocLength >= capacityRequired) { return false; }
	
	uxx newLength = 0;
	newLength = array->allocLength > 0 ? array->allocLength : VAR_ARRAY_MIN_SIZE;
	while (newLength < capacityRequired)
	{
		//double in size, but don't overflow uxx type
		newLength = (newLength <= UINTXX_MAX/2) ? (newLength*2) : UINTXX_MAX;
	}
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
		FreeMem(array->arena, array->items, array->allocLength * array->itemSize);
	}
	
	array->items = newSpace;
	array->allocLength = newLength;
	return true;
}

// +--------------------------------------------------------------+
// |                        Query VarArray                        |
// +--------------------------------------------------------------+
bool VarArrayContains_(uxx itemSize, VarArray* array, const void* itemInQuestion)
{
	NotNull(array);
	DebugAssert(array->itemSize > 0);
	DebugAssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to VarArrayContains. Make sure you're accessing the VarArray with the correct type!");
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
#define VarArrayContains(type, array, itemInQuestion) VarArrayContains_((uxx)sizeof(type), (array), (itemInQuestion))

bool VarArrayGetIndexOf_(uxx itemSize, VarArray* array, const void* itemInQuestion, uxx* indexOut)
{
	if (!VarArrayContains_(itemSize, array, itemInQuestion)) { return false; }
	uxx offsetFromBase = (uxx)(((const u8*)itemInQuestion) - (u8*)array->items);
	DebugAssert((offsetFromBase % array->itemSize) == 0);
	DebugNotNull(indexOut);
	*indexOut = (offsetFromBase / array->itemSize);
	return true;
}
#define VarArrayGetIndexOf(type, array, itemInQuestion, indexOut) VarArrayGetIndexOf_((uxx)sizeof(type), (array), (itemInQuestion), (indexOut))

// +--------------------------------------------------------------+
// |                 Visit/Iterate Over VarArray                  |
// +--------------------------------------------------------------+
void VarArrayVisit(VarArray* array, ArrayVisitFunc_f* visitFunc)
{
	DebugNotNull(array);
	DebugNotNull(visitFunc);
	for (uxx iIndex = 0; iIndex < array->length; iIndex++)
	{
		visitFunc(iIndex, (u8*)array->items + (iIndex * array->itemSize));
	}
}

#define VarArrayLoop(arrayPntr, indexVarName) for (uxx indexVarName = 0; indexVarName < (arrayPntr)->length; indexVarName++)
#define VarArrayLoopGet(type, varName, arrayPntr, indexVarName) type* varName = ((type*)(arrayPntr)->items) + (indexVarName);

// +--------------------------------------------------------------+
// |                    Get Item From VarArray                    |
// +--------------------------------------------------------------+
void* VarArrayGet_(uxx itemSize, const VarArray* array, uxx index, bool assertOnFailure)
{
	NotNull(array);
	DebugAssert(IsVarArrayInit(array));
	DebugAssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to VarArrayGet. Make sure you're accessing the VarArray with the correct type!");
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
#define VarArrayGetHard(type, array, index) ((type*)VarArrayGet_((uxx)sizeof(type), (array), (index), true))
#define VarArrayGetSoft(type, array, index) ((type*)VarArrayGet_((uxx)sizeof(type), (array), (index), false))
#define VarArrayGet(type, array, index) VarArrayGetHard(type, (array), (index))

#define VarArrayGetFirstHard(type, array) VarArrayGetHard(type, (array), 0)
#define VarArrayGetFirstSoft(type, array) (((array)->length > 0) ? VarArrayGetSoft(type, (array), 0) : nullptr)
#define VarArrayGetFirst(type, array) VarArrayGetFirstHard(type, (array)) 

#define VarArrayGetLastHard(type, array) VarArrayGetHard(type, (array), (array)->length - 1)
#define VarArrayGetLastSoft(type, array) (((array)->length > 0) ? VarArrayGetSoft(type, (array), (array)->length - 1) : nullptr)
#define VarArrayGetLast(type, array) VarArrayGetLastHard(type, (array)) 

// +--------------------------------------------------------------+
// |                     Add Item to VarArray                     |
// +--------------------------------------------------------------+
//NOTE: This always asserts on failure to allocate since VarArrayExpand defaults to asserting
void* VarArrayAdd_(uxx itemSize, uxx itemAlignment, VarArray* array)
{
	DebugNotNull(array);
	DebugAssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to VarArrayAdd. Make sure you're accessing the VarArray with the correct type!");
	DebugAssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to VarArrayAdd. Make sure you're accessing the VarArray with the correct type!");
	
	VarArrayExpand(array, array->length+1);
	DebugAssert(array->allocLength >= array->length+1);
	
	void* result = (((u8*)array->items) + (array->itemSize * array->length));
	#if VAR_ARRAY_CLEAR_ITEMS_ON_ADD
	MyMemSet(result, VAR_ARRAY_CLEAR_ITEM_BYTE_VALUE, array->itemSize);
	#endif
	array->length++;
	
	return result;
}

#define VarArrayAdd(type, array) (type*)VarArrayAdd_((uxx)sizeof(type), (uxx)_Alignof(type), (array))

#endif //  _STRUCT_VAR_ARRAY_H
