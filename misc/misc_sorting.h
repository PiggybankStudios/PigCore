/*
File:   misc_sorting.h
Author: Taylor Robbins
Date:   09\07\2025
Description:
	** Contains sorting algorithms that can be used on various data structures
	** The data structure specific implementations are in cross_ files, thie file
	** contains the backing functions that are used for all specific implementations
	** and is included by the application to indicate it wants the specific
	** implementations in the cross files for all data structures it includes
*/

#ifndef _MISC_SORTING_H
#define _MISC_SORTING_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"

//-1 is <    0 is ==   1 is >
#define COMPARE_FUNC_DEF(functionName) i32 functionName(const void* left, const void* right, void* contextPntr)
typedef COMPARE_FUNC_DEF(CompareFunc_f);

#define SORT_API_GET_ELEMENT_SIZE_DEF(functionName) uxx functionName(void* structPntr)
typedef SORT_API_GET_ELEMENT_SIZE_DEF(SortApiGetElementSize_f);

#define SORT_API_GET_NUM_ELEMENTS_DEF(functionName) uxx functionName(void* structPntr)
typedef SORT_API_GET_NUM_ELEMENTS_DEF(SortApiGetNumElements_f);

#define SORT_API_GET_ELEMENT_DEF(functionName) void* functionName(void* structPntr, uxx index)
typedef SORT_API_GET_ELEMENT_DEF(SortApiGetElement_f);

typedef plex SortApi SortApi;
plex SortApi
{
	SortApiGetElementSize_f* GetElementSize;
	SortApiGetNumElements_f* GetNumElements;
	SortApiGetElement_f* GetElement;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	bool IsSortedFuncs(void* structPntr, const SortApi* sortApi, CompareFunc_f* compareFunc, void* contextPntr);
	PIG_CORE_INLINE bool IsSortedFuncsOnIntMember_(bool isMemberSigned, uxx memberOffset, uxx memberSize, void* structPntr, const SortApi* sortApi);
	PIG_CORE_INLINE bool IsSortedFuncsOnFloatMember_(uxx memberOffset, uxx memberSize, void* structPntr, const SortApi* sortApi);
	bool IsSortedFlat(const void* arrayPntr, uxx numElements, uxx elementSize, CompareFunc_f* compareFunc, void* contextPntr);
	PIG_CORE_INLINE bool IsSortedFlatOnIntMember_(bool isMemberSigned, uxx memberOffset, uxx memberSize, const void* arrayPntr, uxx numElements, uxx elementSize);
	PIG_CORE_INLINE bool IsSortedFlatOnFloatMember_(uxx memberOffset, uxx memberSize, const void* arrayPntr, uxx numElements, uxx elementSize);
	void QuickSortFuncs(void* structPntr, SortApi* sortApi, CompareFunc_f* compareFunc, void* contextPntr);
	PIG_CORE_INLINE void QuickSortFuncsOnIntMember_(bool reverseSort, bool isMemberSigned, uxx memberOffset, uxx memberSize, void* structPntr, SortApi* sortApi);
	PIG_CORE_INLINE void QuickSortFuncsOnFloatMember_(bool reverseSort, uxx memberOffset, uxx memberSize, void* structPntr, SortApi* sortApi);
	void QuickSortFlat(void* arrayPntr, uxx numElements, uxx elementSize, CompareFunc_f* compareFunc, void* contextPntr);
	PIG_CORE_INLINE void QuickSortFlatOnIntMember_(bool reverseSort, bool isMemberSigned, uxx memberOffset, uxx memberSize, void* arrayPntr, uxx numElements, uxx elementSize);
	PIG_CORE_INLINE void QuickSortFlatOnFloatMember_(bool reverseSort, uxx memberOffset, uxx memberSize, void* arrayPntr, uxx numElements, uxx elementSize);
#endif

#define IsSortedFuncsOnIntMember(type, memberName, sortApiPntr)                          IsSortedFuncsOnIntMember_(true,  STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (sortApiPntr))
#define IsSortedFuncsOnUintMember(type, memberName, sortApiPntr)                         IsSortedFuncsOnIntMember_(false, STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (sortApiPntr))
#define IsSortedFuncsOnFloatMember(type, memberName, sortApiPntr)                        IsSortedFuncsOnFloatMember_(     STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (sortApiPntr))
#define IsSortedFlatOnUintMember(type, memberName, arrayPntr, numElements, elementSize)  IsSortedFlatOnIntMember_(false,  STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr), (numElements), (elementSize))
#define IsSortedFlatOnIntMember(type, memberName, arrayPntr, numElements, elementSize)   IsSortedFlatOnIntMember_(true,   STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr), (numElements), (elementSize))
#define IsSortedFlatOnFloatMember(type, memberName, arrayPntr, numElements, elementSize) IsSortedFlatOnFloatMember_(      STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr), (numElements), (elementSize))

#define QuickSortFuncsOnUintMember(type, memberName, arrayPntr, numElements, elementSize)          QuickSortFuncsOnIntMember_(false, false, STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr), (numElements), (elementSize))
#define QuickSortFuncsOnIntMember(type, memberName, arrayPntr, numElements, elementSize)           QuickSortFuncsOnIntMember_(false, true,  STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr), (numElements), (elementSize))
#define QuickSortFuncsOnFloatMember(type, memberName, arrayPntr, numElements, elementSize)         QuickSortFuncsOnFloatMember_(false,      STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr), (numElements), (elementSize))
#define QuickSortFuncsOnUintMemberReversed(type, memberName, arrayPntr, numElements, elementSize)  QuickSortFuncsOnIntMember_(true,  false, STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr), (numElements), (elementSize))
#define QuickSortFuncsOnIntMemberReversed(type, memberName, arrayPntr, numElements, elementSize)   QuickSortFuncsOnIntMember_(true,  true,  STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr), (numElements), (elementSize))
#define QuickSortFuncsOnFloatMemberReversed(type, memberName, arrayPntr, numElements, elementSize) QuickSortFuncsOnFloatMember_(true,       STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr), (numElements), (elementSize))
#define QuickSortFlatOnUintMember(type, memberName, arrayPntr, numElements, elementSize)           QuickSortFlatOnIntMember_(false, false,  STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr), (numElements), (elementSize))
#define QuickSortFlatOnIntMember(type, memberName, arrayPntr, numElements, elementSize)            QuickSortFlatOnIntMember_(false, true,   STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr), (numElements), (elementSize))
#define QuickSortFlatOnFloatMember(type, memberName, arrayPntr, numElements, elementSize)          QuickSortFlatOnFloatMember_(false,       STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr), (numElements), (elementSize))
#define QuickSortFlatOnUintMemberReversed(type, memberName, arrayPntr, numElements, elementSize)   QuickSortFlatOnIntMember_(true,  false,  STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr), (numElements), (elementSize))
#define QuickSortFlatOnIntMemberReversed(type, memberName, arrayPntr, numElements, elementSize)    QuickSortFlatOnIntMember_(true,  true,   STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr), (numElements), (elementSize))
#define QuickSortFlatOnFloatMemberReversed(type, memberName, arrayPntr, numElements, elementSize)  QuickSortFlatOnFloatMember_(true,        STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr), (numElements), (elementSize))

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

// +==============================+
// |  Member-Based Sort Helpers   |
// +==============================+
typedef plex SortOnMember_Context SortOnMember_Context;
plex SortOnMember_Context
{
	bool isMemberSigned;
	bool reverseSort;
	uxx memberOffset;
	uxx memberSize;
};

// i32 SortOnIntMember_Compare(const void* left, const void* right, void* contextPntr)
static COMPARE_FUNC_DEF(SortOnIntMember_Compare)
{
	DebugNotNull(left);
	DebugNotNull(right);
	DebugNotNull(contextPntr);
	SortOnMember_Context* context = (SortOnMember_Context*)contextPntr;
	const u8* leftMember = (const u8*)left + context->memberOffset;
	const u8* rightMember = (const u8*)right + context->memberOffset;
	if (context->memberSize == sizeof(u8) && !context->isMemberSigned)
	{
		return (*(const u8*)leftMember) < (*(const u8*)rightMember) ? (context->reverseSort ? 1 : -1)
			: ((*(const u8*)leftMember) > (*(const u8*)rightMember) ? (context->reverseSort ? -1 : 1) : 0);
	}
	else if (context->memberSize == sizeof(i8) && context->isMemberSigned)
	{
		return (*(const i8*)leftMember) < (*(const i8*)rightMember) ? (context->reverseSort ? 1 : -1)
			: ((*(const i8*)leftMember) > (*(const i8*)rightMember) ? (context->reverseSort ? -1 : 1) : 0);
	}
	else if (context->memberSize == sizeof(u16) && !context->isMemberSigned)
	{
		return (*(const u16*)leftMember) < (*(const u16*)rightMember) ? (context->reverseSort ? 1 : -1)
			: ((*(const u16*)leftMember) > (*(const u16*)rightMember) ? (context->reverseSort ? -1 : 1) : 0);
	}
	else if (context->memberSize == sizeof(i16) && context->isMemberSigned)
	{
		return (*(const i16*)leftMember) < (*(const i16*)rightMember) ? (context->reverseSort ? 1 : -1)
			: ((*(const i16*)leftMember) > (*(const i16*)rightMember) ? (context->reverseSort ? -1 : 1) : 0);
	}
	else if (context->memberSize == sizeof(u32) && !context->isMemberSigned)
	{
		return (*(const u32*)leftMember) < (*(const u32*)rightMember) ? (context->reverseSort ? 1 : -1)
			: ((*(const u32*)leftMember) > (*(const u32*)rightMember) ? (context->reverseSort ? -1 : 1) : 0);
	}
	else if (context->memberSize == sizeof(i32) && context->isMemberSigned)
	{
		return (*(const i32*)leftMember) < (*(const i32*)rightMember) ? (context->reverseSort ? 1 : -1)
			: ((*(const i32*)leftMember) > (*(const i32*)rightMember) ? (context->reverseSort ? -1 : 1) : 0);
	}
	else if (context->memberSize == sizeof(u64) && !context->isMemberSigned)
	{
		return (*(const u64*)leftMember) < (*(const u64*)rightMember) ? (context->reverseSort ? 1 : -1)
			: ((*(const u64*)leftMember) > (*(const u64*)rightMember) ? (context->reverseSort ? -1 : 1) : 0);
	}
	else if (context->memberSize == sizeof(i64) && context->isMemberSigned)
	{
		return (*(const i64*)leftMember) < (*(const i64*)rightMember) ? (context->reverseSort ? 1 : -1)
			: ((*(const i64*)leftMember) > (*(const i64*)rightMember) ? (context->reverseSort ? -1 : 1) : 0);
	}
	else { Assert(false); return 0; }
}
// i32 SortOnFloatMember_Compare(const void* left, const void* right, void* contextPntr)
static COMPARE_FUNC_DEF(SortOnFloatMember_Compare)
{
	DebugNotNull(left);
	DebugNotNull(right);
	DebugNotNull(contextPntr);
	SortOnMember_Context* context = (SortOnMember_Context*)contextPntr;
	const u8* leftMember = (const u8*)left + context->memberOffset;
	const u8* rightMember = (const u8*)right + context->memberOffset;
	if (context->memberSize == sizeof(r32))
	{
		return (*(const r32*)leftMember) < (*(const r32*)rightMember) ? (context->reverseSort ? 1 : -1)
			: ((*(const r32*)leftMember) > (*(const r32*)rightMember) ? (context->reverseSort ? -1 : 1) : 0);
	}
	else if (context->memberSize == sizeof(r64))
	{
		return (*(const r64*)leftMember) < (*(const r64*)rightMember) ? (context->reverseSort ? 1 : -1)
			: ((*(const r64*)leftMember) > (*(const r64*)rightMember) ? (context->reverseSort ? -1 : 1) : 0);
	}
	else { Assert(false); return 0; }
}

// +==============================+
// |           IsSorted           |
// +==============================+
PEXP bool IsSortedFuncs(void* structPntr, const SortApi* sortApi, CompareFunc_f* compareFunc, void* contextPntr)
{
	NotNull(structPntr);
	NotNull(sortApi);
	NotNull(sortApi->GetNumElements);
	NotNull(sortApi->GetElement);
	NotNull(compareFunc);
	uxx numElements = sortApi->GetNumElements(structPntr);
	void* prevElement = nullptr;
	for (uxx eIndex = 0; eIndex < numElements; eIndex++)
	{
		void* nextElement = sortApi->GetElement(structPntr, eIndex);
		if (prevElement != nullptr)
		{
			i32 sortResult = compareFunc(prevElement, nextElement, contextPntr);
			if (sortResult > 0) { return false; }
		}
		prevElement = nextElement;
	}
	return true;
}
PEXPI bool IsSortedFuncsOnIntMember_(bool isMemberSigned, uxx memberOffset, uxx memberSize, void* structPntr, const SortApi* sortApi)
{
	SortOnMember_Context context = ZEROED;
	context.reverseSort = false;
	context.isMemberSigned = isMemberSigned;
	context.memberOffset = memberOffset;
	context.memberSize = memberSize;
	return IsSortedFuncs(structPntr, sortApi, SortOnIntMember_Compare, &context);
}
PEXPI bool IsSortedFuncsOnFloatMember_(uxx memberOffset, uxx memberSize, void* structPntr, const SortApi* sortApi)
{
	SortOnMember_Context context = ZEROED;
	context.reverseSort = false;
	context.memberOffset = memberOffset;
	context.memberSize = memberSize;
	return IsSortedFuncs(structPntr, sortApi, SortOnFloatMember_Compare, &context);
}

PEXP bool IsSortedFlat(const void* arrayPntr, uxx numElements, uxx elementSize, CompareFunc_f* compareFunc, void* contextPntr)
{
	NotNull(arrayPntr);
	Assert(elementSize > 0);
	NotNull(compareFunc);
	for (uxx eIndex = 0; eIndex+1 < numElements; eIndex++)
	{
		const u8* leftElement = (const u8*)arrayPntr + (elementSize * eIndex);
		const u8* rightElement = (const u8*)arrayPntr + (elementSize * (eIndex+1));
		i32 compareResult = compareFunc(leftElement, rightElement, contextPntr);
		if (compareResult > 0) { return false; }
	}
	return true;
}

PEXPI bool IsSortedFlatOnIntMember_(bool isMemberSigned, uxx memberOffset, uxx memberSize, const void* arrayPntr, uxx numElements, uxx elementSize)
{
	SortOnMember_Context context = ZEROED;
	context.reverseSort = false;
	context.isMemberSigned = isMemberSigned;
	context.memberOffset = memberOffset;
	context.memberSize = memberSize;
	return IsSortedFlat(arrayPntr, numElements, elementSize, SortOnIntMember_Compare, &context);
}

PEXPI bool IsSortedFlatOnFloatMember_(uxx memberOffset, uxx memberSize, const void* arrayPntr, uxx numElements, uxx elementSize)
{
	SortOnMember_Context context = ZEROED;
	context.reverseSort = false;
	context.memberOffset = memberOffset;
	context.memberSize = memberSize;
	return IsSortedFlat(arrayPntr, numElements, elementSize, SortOnFloatMember_Compare, &context);
}

// +==============================+
// |          Quick Sort          |
// +==============================+
static ixx QuickSortFuncsPartition(void* structPntr, SortApi* sortApi, uxx elementSize, uxx startIndex, uxx numElements, void* workingSpace, CompareFunc_f* compareFunc, void* contextPntr)
{
	u8* tempSpace = ((u8*)workingSpace) + 0;
	u8* pivotCopy = ((u8*)workingSpace) + elementSize;
	u8* pivotPntr = (u8*)sortApi->GetElement(structPntr, startIndex + numElements-1);
	NotNull(pivotPntr);
	MyMemCopy(pivotCopy, pivotPntr, elementSize);
	
	ixx smallIndex = -1;
	u8* smallPntr = nullptr;
	for (u64 eIndex = 0; eIndex < numElements-1; eIndex++)
	{
		u8* elementPntr = (u8*)sortApi->GetElement(structPntr, startIndex + eIndex);
		NotNull(elementPntr);
		i32 compareResult = compareFunc((const void*)elementPntr, (const void*)pivotCopy, contextPntr);
		if (compareResult < 0)
		{
			smallIndex++;
			smallPntr = (u8*)sortApi->GetElement(structPntr, (uxx)(startIndex + smallIndex));
			NotNull(smallPntr);
			
			//swap smallPntr and elementPntr
			MyMemCopy(tempSpace, smallPntr, elementSize);
			MyMemCopy(smallPntr, elementPntr, elementSize);
			MyMemCopy(elementPntr, tempSpace, elementSize);
		}
	}
	
	smallIndex++;
	smallPntr = (u8*)sortApi->GetElement(structPntr, (uxx)(startIndex + smallIndex));
	NotNull(smallPntr);
	Assert((u64)smallIndex < numElements);
	MyMemCopy(pivotPntr, smallPntr, elementSize);
	MyMemCopy(smallPntr, pivotCopy, elementSize);
	
	return smallIndex;
}

//This is the recursive part of QuickSortFuncs
static void QuickSortFuncs_(void* structPntr, SortApi* sortApi, uxx elementSize, uxx startIndex, uxx numElements, u8* workingSpace, CompareFunc_f* compareFunc, void* contextPntr)
{
	if (numElements <= 1) { return; } //nothing to sort
	//parition the array
	ixx pi = QuickSortFuncsPartition(structPntr, sortApi, elementSize, 0, numElements, workingSpace, compareFunc, contextPntr);
	//sort lower partition
	QuickSortFuncs_(structPntr, sortApi, elementSize, 0, (uxx)pi, workingSpace, compareFunc, contextPntr);
	//sort upper partition
	QuickSortFuncs_(structPntr, sortApi, elementSize, (uxx)(pi+1), (uxx)(numElements - (pi+1)), workingSpace, compareFunc, contextPntr);
}

// Operates on a data structure through functions in SortApi. This allows the sorting algorithm to interact with complex data structures
PEXP void QuickSortFuncs(void* structPntr, SortApi* sortApi, CompareFunc_f* compareFunc, void* contextPntr)
{
	NotNull(structPntr);
	NotNull(sortApi);
	NotNull(sortApi->GetNumElements);
	NotNull(sortApi->GetElement);
	NotNull(compareFunc);
	ScratchBegin(scratch);
	//NOTE: workingSpace must be a space large enough to hold two elements. This space is used to perform swaps and to hold the pivot element
	uxx numElements = sortApi->GetNumElements(structPntr);
	if (numElements < 2) { return; } //nothing to sort
	uxx elementSize = sortApi->GetElementSize(structPntr);
	Assert(elementSize > 0);
	u8* workingSpace = (u8*)AllocMem(scratch, elementSize*2);
	NotNull(workingSpace);
	#if DEBUG_BUILD
	MyMemSet(workingSpace, 0x00, elementSize*2);
	#endif
	QuickSortFuncs_(structPntr, sortApi, elementSize, 0, numElements, workingSpace, compareFunc, contextPntr);
	ScratchEnd(scratch);
}

PEXPI void QuickSortFuncsOnIntMember_(bool reverseSort, bool isMemberSigned, uxx memberOffset, uxx memberSize, void* structPntr, SortApi* sortApi)
{
	SortOnMember_Context context = ZEROED;
	context.isMemberSigned = isMemberSigned;
	context.reverseSort = reverseSort;
	context.memberOffset = memberOffset;
	context.memberSize = memberSize;
	QuickSortFuncs(structPntr, sortApi, SortOnIntMember_Compare, &context);
}
PEXPI void QuickSortFuncsOnFloatMember_(bool reverseSort, uxx memberOffset, uxx memberSize, void* structPntr, SortApi* sortApi)
{
	SortOnMember_Context context = ZEROED;
	context.reverseSort = reverseSort;
	context.memberOffset = memberOffset;
	context.memberSize = memberSize;
	QuickSortFuncs(structPntr, sortApi, SortOnFloatMember_Compare, &context);
}

static ixx QuickSortFlatPartition(void* arrayPntr, uxx numElements, uxx elementSize, void* workingSpace, CompareFunc_f* compareFunc, void* contextPntr)
{
	u8* bytePntr = (u8*)arrayPntr;
	
	u8* tempSpace = ((u8*)workingSpace) + 0;
	u8* pivotCopy = ((u8*)workingSpace) + elementSize;
	u8* pivotPntr = bytePntr + elementSize*(numElements-1);
	MyMemCopy(pivotCopy, pivotPntr, elementSize);
	
	ixx smallIndex = -1;
	u8* smallPntr = bytePntr - elementSize;
	for (u64 eIndex = 0; eIndex < numElements-1; eIndex++)
	{
		u8* elementPntr = bytePntr + elementSize*(eIndex);
		i32 compareResult = compareFunc((const void*)elementPntr, (const void*)pivotCopy, contextPntr);
		if (compareResult < 0)
		{
			smallIndex++;
			smallPntr += elementSize;
			
			//swap smallPntr and elementPntr
			MyMemCopy(tempSpace, smallPntr, elementSize);
			MyMemCopy(smallPntr, elementPntr, elementSize);
			MyMemCopy(elementPntr, tempSpace, elementSize);
		}
	}
	
	smallIndex++;
	smallPntr += elementSize;
	Assert((u64)smallIndex < numElements);
	MyMemCopy(pivotPntr, smallPntr, elementSize);
	MyMemCopy(smallPntr, pivotCopy, elementSize);
	
	return smallIndex;
}

//This is the recursive part of QuickSortFlat
static void QuickSortFlat_(void* arrayPntr, uxx numElements, uxx elementSize, u8* workingSpace, CompareFunc_f* compareFunc, void* contextPntr)
{
	if (numElements <= 1) { return; } //nothing to sort
	//parition the array
	ixx pi = QuickSortFlatPartition(arrayPntr, numElements, elementSize, workingSpace, compareFunc, contextPntr);
	//sort lower partition
	QuickSortFlat_(arrayPntr, (uxx)pi, elementSize, workingSpace, compareFunc, contextPntr);
	//sort upper partition
	QuickSortFlat_((void*)((u8*)arrayPntr + elementSize*(pi+1)), (uxx)(numElements - (pi+1)), elementSize, workingSpace, compareFunc, contextPntr);
}

// Operates on an array of elements that are "flat" in memory. This means we can assume how we find each element based on a simple offset calculation
// For more complex data structures we need to go through the SortApi used in the QuickSortFuncs variant
PEXP void QuickSortFlat(void* arrayPntr, uxx numElements, uxx elementSize, CompareFunc_f* compareFunc, void* contextPntr)
{
	NotNull(arrayPntr);
	Assert(elementSize > 0);
	NotNull(compareFunc);
	ScratchBegin(scratch);
	//NOTE: workingSpace must be a space large enough to hold two elements. This space is used to perform swaps and to hold the pivot element
	u8* workingSpace = (u8*)AllocMem(scratch, elementSize*2);
	NotNull(workingSpace);
	#if DEBUG_BUILD
	MyMemSet(workingSpace, 0x00, elementSize*2);
	#endif
	QuickSortFlat_(arrayPntr, numElements, elementSize, workingSpace, compareFunc, contextPntr);
	ScratchEnd(scratch);
}

PEXPI void QuickSortFlatOnIntMember_(bool reverseSort, bool isMemberSigned, uxx memberOffset, uxx memberSize, void* arrayPntr, uxx numElements, uxx elementSize)
{
	SortOnMember_Context context = ZEROED;
	context.isMemberSigned = isMemberSigned;
	context.reverseSort = reverseSort;
	context.memberOffset = memberOffset;
	context.memberSize = memberSize;
	QuickSortFlat(arrayPntr, numElements, elementSize, SortOnIntMember_Compare, &context);
}
PEXPI void QuickSortFlatOnFloatMember_(bool reverseSort, uxx memberOffset, uxx memberSize, void* arrayPntr, uxx numElements, uxx elementSize)
{
	SortOnMember_Context context = ZEROED;
	context.reverseSort = reverseSort;
	context.memberOffset = memberOffset;
	context.memberSize = memberSize;
	QuickSortFlat(arrayPntr, numElements, elementSize, SortOnFloatMember_Compare, &context);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _MISC_SORTING_H

#if defined(_MISC_SORTING_H) && defined(_STRUCT_VAR_ARRAY_H)
#include "cross/cross_sorting_and_var_array.h"
#endif

#if defined(_MISC_SORTING_H) && defined(_STRUCT_BKT_ARRAY_H)
#include "cross/cross_sorting_and_bkt_array.h"
#endif
