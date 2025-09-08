/*
File:   cross_sorting_and_bkt_array.h
Author: Taylor Robbins
Date:   09\08\2025
*/

#ifndef _CROSS_SORTING_AND_BKT_ARRAY_H
#define _CROSS_SORTING_AND_BKT_ARRAY_H

//NOTE: Intentionally no includes here

#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE bool IsBktArraySorted_(uxx itemSize, uxx itemAlignment, BktArray* array, CompareFunc_f* compareFunc, void* contextPntr);
	PIG_CORE_INLINE bool IsBktArraySortedInt_(uxx itemSize, uxx itemAlignment, bool isMemberSigned, uxx memberOffset, uxx memberSize, BktArray* array);
	PIG_CORE_INLINE bool IsBktArraySortedFloat_(uxx itemSize, uxx itemAlignment, uxx memberOffset, uxx memberSize, BktArray* array);
	PIG_CORE_INLINE void QuickSortBktArray_(uxx itemSize, uxx itemAlignment, bool reverseSort, BktArray* array, CompareFunc_f* compareFunc, void* contextPntr);
	PIG_CORE_INLINE void QuickSortBktArrayInt_(uxx itemSize, uxx itemAlignment, bool reverseSort, bool isMemberSigned, uxx memberOffset, uxx memberSize, BktArray* array);
	PIG_CORE_INLINE void QuickSortBktArrayFloat_(uxx itemSize, uxx itemAlignment, bool reverseSort, uxx memberOffset, uxx memberSize, BktArray* array);
#endif

#if LANGUAGE_IS_C
#define IsBktArraySorted(type, arrayPntr, compareFunc, contextPntr) IsBktArraySorted_(sizeof(type),      (uxx)_Alignof(type),                         (arrayPntr), (compareFunc), (contextPntr))
#define IsBktArraySortedIntElem(type, arrayPntr)                    IsBktArraySortedInt_(sizeof(type),   (uxx)_Alignof(type), true,  0, sizeof(type), (arrayPntr))
#define IsBktArraySortedUintElem(type, arrayPntr)                   IsBktArraySortedInt_(sizeof(type),   (uxx)_Alignof(type), false, 0, sizeof(type), (arrayPntr))
#define IsBktArraySortedFloatElem(type, arrayPntr)                  IsBktArraySortedFloat_(sizeof(type), (uxx)_Alignof(type),        0, sizeof(type), (arrayPntr))
#define IsBktArraySortedIntMember(type, memberName, arrayPntr)      IsBktArraySortedInt_(sizeof(type),   (uxx)_Alignof(type), true,  STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define IsBktArraySortedUintMember(type, memberName, arrayPntr)     IsBktArraySortedInt_(sizeof(type),   (uxx)_Alignof(type), false, STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define IsBktArraySortedFloatMember(type, memberName, arrayPntr)    IsBktArraySortedFloat_(sizeof(type), (uxx)_Alignof(type),        STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#else
#define IsBktArraySorted(type, arrayPntr, compareFunc, contextPntr) IsBktArraySorted_(sizeof(type),      (uxx)std::alignment_of<type>(),                         (arrayPntr), (compareFunc), (contextPntr))
#define IsBktArraySortedIntElem(type, arrayPntr)                    IsBktArraySortedInt_(sizeof(type),   (uxx)std::alignment_of<type>(), true,  0, sizeof(type), (arrayPntr))
#define IsBktArraySortedUintElem(type, arrayPntr)                   IsBktArraySortedInt_(sizeof(type),   (uxx)std::alignment_of<type>(), false, 0, sizeof(type), (arrayPntr))
#define IsBktArraySortedFloatElem(type, arrayPntr)                  IsBktArraySortedFloat_(sizeof(type), (uxx)std::alignment_of<type>(),        0, sizeof(type), (arrayPntr))
#define IsBktArraySortedIntMember(type, memberName, arrayPntr)      IsBktArraySortedInt_(sizeof(type),   (uxx)std::alignment_of<type>(), true,  STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define IsBktArraySortedUintMember(type, memberName, arrayPntr)     IsBktArraySortedInt_(sizeof(type),   (uxx)std::alignment_of<type>(), false, STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define IsBktArraySortedFloatMember(type, memberName, arrayPntr)    IsBktArraySortedFloat_(sizeof(type), (uxx)std::alignment_of<type>(),        STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#endif

#if LANGUAGE_IS_C
#define QuickSortBktArray(type, arrayPntr, compareFunc, contextPntr)          QuickSortBktArray_(sizeof(type),      (uxx)_Alignof(type), false,                         (arrayPntr), (compareFunc), (contextPntr))
#define QuickSortBktArrayIntElem(type, arrayPntr)                             QuickSortBktArrayInt_(sizeof(type),   (uxx)_Alignof(type), false, true,  0, sizeof(type), (arrayPntr))
#define QuickSortBktArrayUintElem(type, arrayPntr)                            QuickSortBktArrayInt_(sizeof(type),   (uxx)_Alignof(type), false, false, 0, sizeof(type), (arrayPntr))
#define QuickSortBktArrayFloat(type, arrayPntr)                               QuickSortBktArrayFloat_(sizeof(type), (uxx)_Alignof(type), false,        0, sizeof(type), (arrayPntr))
#define QuickSortBktArrayIntMember(type, memberName, arrayPntr)               QuickSortBktArrayInt_(sizeof(type),   (uxx)_Alignof(type), false, true,  STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortBktArrayUintMember(type, memberName, arrayPntr)              QuickSortBktArrayInt_(sizeof(type),   (uxx)_Alignof(type), false, false, STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortBktArrayFloatMember(type, memberName, arrayPntr)             QuickSortBktArrayFloat_(sizeof(type), (uxx)_Alignof(type), false,        STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortBktArrayReversed(type, arrayPntr, compareFunc, contextPntr)  QuickSortBktArray_(sizeof(type),      (uxx)_Alignof(type), true,                          (arrayPntr), (compareFunc), (contextPntr))
#define QuickSortBktArrayIntElemReversed(type, arrayPntr)                     QuickSortBktArrayInt_(sizeof(type),   (uxx)_Alignof(type), true,  true,  0, sizeof(type), (arrayPntr))
#define QuickSortBktArrayUintElemReversed(type, arrayPntr)                    QuickSortBktArrayInt_(sizeof(type),   (uxx)_Alignof(type), true,  false, 0, sizeof(type), (arrayPntr))
#define QuickSortBktArrayFloatReversed(type, arrayPntr)                       QuickSortBktArrayFloat_(sizeof(type), (uxx)_Alignof(type), true,         0, sizeof(type), (arrayPntr))
#define QuickSortBktArrayIntMemberReversed(type, memberName, arrayPntr)       QuickSortBktArrayInt_(sizeof(type),   (uxx)_Alignof(type), true,  true,  STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortBktArrayUintMemberReversed(type, memberName, arrayPntr)      QuickSortBktArrayInt_(sizeof(type),   (uxx)_Alignof(type), true,  false, STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortBktArrayFloatMemberReversed(type, memberName, arrayPntr)     QuickSortBktArrayFloat_(sizeof(type), (uxx)_Alignof(type), true,         STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#else
#define QuickSortBktArray(type, arrayPntr, compareFunc, contextPntr)          QuickSortBktArray_(sizeof(type),      (uxx)std::alignment_of<type>(), false,                         (arrayPntr), (compareFunc), (contextPntr))
#define QuickSortBktArrayIntElem(type, arrayPntr)                             QuickSortBktArrayInt_(sizeof(type),   (uxx)std::alignment_of<type>(), false, true,  0, sizeof(type), (arrayPntr))
#define QuickSortBktArrayUintElem(type, arrayPntr)                            QuickSortBktArrayInt_(sizeof(type),   (uxx)std::alignment_of<type>(), false, false, 0, sizeof(type), (arrayPntr))
#define QuickSortBktArrayFloat(type, arrayPntr)                               QuickSortBktArrayFloat_(sizeof(type), (uxx)std::alignment_of<type>(), false,        0, sizeof(type), (arrayPntr))
#define QuickSortBktArrayIntMember(type, memberName, arrayPntr)               QuickSortBktArrayInt_(sizeof(type),   (uxx)std::alignment_of<type>(), false, true,  STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortBktArrayUintMember(type, memberName, arrayPntr)              QuickSortBktArrayInt_(sizeof(type),   (uxx)std::alignment_of<type>(), false, false, STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortBktArrayFloatMember(type, memberName, arrayPntr)             QuickSortBktArrayFloat_(sizeof(type), (uxx)std::alignment_of<type>(), false,        STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortBktArrayReversed(type, arrayPntr, compareFunc, contextPntr)  QuickSortBktArray_(sizeof(type),      (uxx)std::alignment_of<type>(), true,                          (arrayPntr), (compareFunc), (contextPntr))
#define QuickSortBktArrayIntElemReversed(type, arrayPntr)                     QuickSortBktArrayInt_(sizeof(type),   (uxx)std::alignment_of<type>(), true,  true,  0, sizeof(type), (arrayPntr))
#define QuickSortBktArrayUintElemReversed(type, arrayPntr)                    QuickSortBktArrayInt_(sizeof(type),   (uxx)std::alignment_of<type>(), true,  false, 0, sizeof(type), (arrayPntr))
#define QuickSortBktArrayFloatReversed(type, arrayPntr)                       QuickSortBktArrayFloat_(sizeof(type), (uxx)std::alignment_of<type>(), true,         0, sizeof(type), (arrayPntr))
#define QuickSortBktArrayIntMemberReversed(type, memberName, arrayPntr)       QuickSortBktArrayInt_(sizeof(type),   (uxx)std::alignment_of<type>(), true,  true,  STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortBktArrayUintMemberReversed(type, memberName, arrayPntr)      QuickSortBktArrayInt_(sizeof(type),   (uxx)std::alignment_of<type>(), true,  false, STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortBktArrayFloatMemberReversed(type, memberName, arrayPntr)     QuickSortBktArrayFloat_(sizeof(type), (uxx)std::alignment_of<type>(), true,         STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#endif

#if PIG_CORE_IMPLEMENTATION

static SORT_API_GET_ELEMENT_SIZE_DEF(BktArraySortApi_GetElementSize) { return ((BktArray*)structPntr)->itemSize; }
static SORT_API_GET_NUM_ELEMENTS_DEF(BktArraySortApi_GetNumElements) { return ((BktArray*)structPntr)->length; }
static SORT_API_GET_ELEMENT_DEF(BktArraySortApi_GetElement) { BktArray* bktArray = (BktArray*)structPntr; return BktArrayGet_(bktArray->itemSize, bktArray->itemAlignment, bktArray, index, true); }
#define BktArraySortApi_Const {                       \
	.GetElementSize = BktArraySortApi_GetElementSize, \
	.GetNumElements = BktArraySortApi_GetNumElements, \
	.GetElement = BktArraySortApi_GetElement          \
}

PEXPI bool IsBktArraySorted_(uxx itemSize, uxx itemAlignment, BktArray* array, CompareFunc_f* compareFunc, void* contextPntr)
{
	#if DEBUG_BUILD
	NotNull(array);
	Assert(IsBktArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to IsBktArraySorted. Make sure you're accessing the BktArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to IsBktArraySorted. Make sure you're accessing the BktArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	SortApi sortApi = BktArraySortApi_Const;
	return IsSortedFuncs((void*)array, &sortApi, compareFunc, contextPntr);
}

PEXPI bool IsBktArraySortedInt_(uxx itemSize, uxx itemAlignment, bool isMemberSigned, uxx memberOffset, uxx memberSize, BktArray* array)
{
	#if DEBUG_BUILD
	NotNull(array);
	Assert(IsBktArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to IsBktArraySortedInt. Make sure you're accessing the BktArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to IsBktArraySortedInt. Make sure you're accessing the BktArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	SortApi sortApi = BktArraySortApi_Const;
	return IsSortedFuncsOnIntMember_(isMemberSigned, memberOffset, memberSize, array, &sortApi);
}

PEXPI bool IsBktArraySortedFloat_(uxx itemSize, uxx itemAlignment, uxx memberOffset, uxx memberSize, BktArray* array)
{
	#if DEBUG_BUILD
	NotNull(array);
	Assert(IsBktArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to IsBktArraySortedFloat. Make sure you're accessing the BktArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to IsBktArraySortedFloat. Make sure you're accessing the BktArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	SortApi sortApi = BktArraySortApi_Const;
	return IsSortedFuncsOnFloatMember_(memberOffset, memberSize, array, &sortApi);
}

PEXPI void QuickSortBktArray_(uxx itemSize, uxx itemAlignment, bool reverseSort, BktArray* array, CompareFunc_f* compareFunc, void* contextPntr)
{
	NotNull(array);
	#if DEBUG_BUILD
	Assert(IsBktArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to QuickSortBktArray. Make sure you're accessing the BktArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to QuickSortBktArray. Make sure you're accessing the BktArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	SortApi sortApi = BktArraySortApi_Const;
	QuickSortFuncs((void*)array, &sortApi, compareFunc, contextPntr);
}

PEXPI void QuickSortBktArrayInt_(uxx itemSize, uxx itemAlignment, bool reverseSort, bool isMemberSigned, uxx memberOffset, uxx memberSize, BktArray* array)
{
	NotNull(array);
	#if DEBUG_BUILD
	Assert(IsBktArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to QuickSortBktArrayInt. Make sure you're accessing the BktArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to QuickSortBktArrayInt. Make sure you're accessing the BktArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	SortApi sortApi = BktArraySortApi_Const;
	QuickSortFuncsOnIntMember_(reverseSort, isMemberSigned, memberOffset, memberSize, (void*)array, &sortApi);
}
PEXPI void QuickSortBktArrayFloat_(uxx itemSize, uxx itemAlignment, bool reverseSort, uxx memberOffset, uxx memberSize, BktArray* array)
{
	NotNull(array);
	#if DEBUG_BUILD
	Assert(IsBktArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to QuickSortBktArrayFloat. Make sure you're accessing the BktArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to QuickSortBktArrayFloat. Make sure you're accessing the BktArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	SortApi sortApi = BktArraySortApi_Const;
	QuickSortFuncsOnFloatMember_(reverseSort, memberOffset, memberSize, (void*)array, &sortApi);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_SORTING_AND_BKT_ARRAY_H
