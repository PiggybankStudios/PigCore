/*
File:   cross_sorting_and_var_array.h
Author: Taylor Robbins
Date:   09\07\2025
*/

#ifndef _CROSS_SORTING_AND_VAR_ARRAY_H
#define _CROSS_SORTING_AND_VAR_ARRAY_H

//NOTE: Intentionally no includes here

#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE bool IsVarArraySorted_(uxx itemSize, uxx itemAlignment, VarArray* array, CompareFunc_f* compareFunc, void* contextPntr);
	PIG_CORE_INLINE bool IsVarArraySortedInt_(uxx itemSize, uxx itemAlignment, bool isMemberSigned, uxx memberOffset, uxx memberSize, VarArray* array);
	PIG_CORE_INLINE bool IsVarArraySortedFloat_(uxx itemSize, uxx itemAlignment, uxx memberOffset, uxx memberSize, VarArray* array);
	PIG_CORE_INLINE void QuickSortVarArray_(uxx itemSize, uxx itemAlignment, bool reverseSort, VarArray* array, CompareFunc_f* compareFunc, void* contextPntr);
	PIG_CORE_INLINE void QuickSortVarArrayInt_(uxx itemSize, uxx itemAlignment, bool reverseSort, bool isMemberSigned, uxx memberOffset, uxx memberSize, VarArray* array);
	PIG_CORE_INLINE void QuickSortVarArrayFloat_(uxx itemSize, uxx itemAlignment, bool reverseSort, uxx memberOffset, uxx memberSize, VarArray* array);
#endif

#if LANGUAGE_IS_C
#define IsVarArraySorted(type, arrayPntr, compareFunc, contextPntr)  IsVarArraySorted_(sizeof(type),      (uxx)_Alignof(type),                         (arrayPntr), (compareFunc), (contextPntr))
#define IsVarArraySortedIntElem(type, arrayPntr)                     IsVarArraySortedInt_(sizeof(type),   (uxx)_Alignof(type), true,  0, sizeof(type), (arrayPntr))
#define IsVarArraySortedUintElem(type, arrayPntr)                    IsVarArraySortedInt_(sizeof(type),   (uxx)_Alignof(type), false, 0, sizeof(type), (arrayPntr))
#define IsVarArraySortedFloatElem(type, arrayPntr)                   IsVarArraySortedFloat_(sizeof(type), (uxx)_Alignof(type),        0, sizeof(type), (arrayPntr))
#define IsVarArraySortedIntMember(type, memberName, arrayPntr)       IsVarArraySortedInt_(sizeof(type),   (uxx)_Alignof(type), true,  STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define IsVarArraySortedUintMember(type, memberName, arrayPntr)      IsVarArraySortedInt_(sizeof(type),   (uxx)_Alignof(type), false, STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define IsVarArraySortedFloatMember(type, memberName, arrayPntr)     IsVarArraySortedFloat_(sizeof(type), (uxx)_Alignof(type),        STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#else
#define IsVarArraySorted(type, arrayPntr, compareFunc, contextPntr)  IsVarArraySorted_(sizeof(type),      (uxx)std::alignment_of<type>(),                         (arrayPntr), (compareFunc), (contextPntr))
#define IsVarArraySortedIntElem(type, arrayPntr)                     IsVarArraySortedInt_(sizeof(type),   (uxx)std::alignment_of<type>(), true,  0, sizeof(type), (arrayPntr))
#define IsVarArraySortedUintElem(type, arrayPntr)                    IsVarArraySortedInt_(sizeof(type),   (uxx)std::alignment_of<type>(), false, 0, sizeof(type), (arrayPntr))
#define IsVarArraySortedFloatElem(type, arrayPntr)                   IsVarArraySortedFloat_(sizeof(type), (uxx)std::alignment_of<type>(),        0, sizeof(type), (arrayPntr))
#define IsVarArraySortedIntMember(type, memberName, arrayPntr)       IsVarArraySortedInt_(sizeof(type),   (uxx)std::alignment_of<type>(), true,  STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define IsVarArraySortedUintMember(type, memberName, arrayPntr)      IsVarArraySortedInt_(sizeof(type),   (uxx)std::alignment_of<type>(), false, STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define IsVarArraySortedFloatMember(type, memberName, arrayPntr)     IsVarArraySortedFloat_(sizeof(type), (uxx)std::alignment_of<type>(),        STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#endif

#if LANGUAGE_IS_C
#define QuickSortVarArray(type, arrayPntr, compareFunc, contextPntr)          QuickSortVarArray_(sizeof(type),      (uxx)_Alignof(type), false,                         (arrayPntr), (compareFunc), (contextPntr))
#define QuickSortVarArrayIntElem(type, arrayPntr)                             QuickSortVarArrayInt_(sizeof(type),   (uxx)_Alignof(type), false, true,  0, sizeof(type), (arrayPntr))
#define QuickSortVarArrayUintElem(type, arrayPntr)                            QuickSortVarArrayInt_(sizeof(type),   (uxx)_Alignof(type), false, false, 0, sizeof(type), (arrayPntr))
#define QuickSortVarArrayFloat(type, arrayPntr)                               QuickSortVarArrayFloat_(sizeof(type), (uxx)_Alignof(type), false,        0, sizeof(type), (arrayPntr))
#define QuickSortVarArrayIntMember(type, memberName, arrayPntr)               QuickSortVarArrayInt_(sizeof(type),   (uxx)_Alignof(type), false, true,  STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortVarArrayUintMember(type, memberName, arrayPntr)              QuickSortVarArrayInt_(sizeof(type),   (uxx)_Alignof(type), false, false, STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortVarArrayFloatMember(type, memberName, arrayPntr)             QuickSortVarArrayFloat_(sizeof(type), (uxx)_Alignof(type), false,        STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortVarArrayReversed(type, arrayPntr, compareFunc, contextPntr)  QuickSortVarArray_(sizeof(type),      (uxx)_Alignof(type), true,                          (arrayPntr), (compareFunc), (contextPntr))
#define QuickSortVarArrayIntElemReversed(type, arrayPntr)                     QuickSortVarArrayInt_(sizeof(type),   (uxx)_Alignof(type), true,  true,  0, sizeof(type), (arrayPntr))
#define QuickSortVarArrayUintElemReversed(type, arrayPntr)                    QuickSortVarArrayInt_(sizeof(type),   (uxx)_Alignof(type), true,  false, 0, sizeof(type), (arrayPntr))
#define QuickSortVarArrayFloatReversed(type, arrayPntr)                       QuickSortVarArrayFloat_(sizeof(type), (uxx)_Alignof(type), true,         0, sizeof(type), (arrayPntr))
#define QuickSortVarArrayIntMemberReversed(type, memberName, arrayPntr)       QuickSortVarArrayInt_(sizeof(type),   (uxx)_Alignof(type), true,  true,  STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortVarArrayUintMemberReversed(type, memberName, arrayPntr)      QuickSortVarArrayInt_(sizeof(type),   (uxx)_Alignof(type), true,  false, STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortVarArrayFloatMemberReversed(type, memberName, arrayPntr)     QuickSortVarArrayFloat_(sizeof(type), (uxx)_Alignof(type), true,         STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#else
#define QuickSortVarArray(type, arrayPntr, compareFunc, contextPntr)          QuickSortVarArray_(sizeof(type),      (uxx)std::alignment_of<type>(), false,                         (arrayPntr), (compareFunc), (contextPntr))
#define QuickSortVarArrayIntElem(type, arrayPntr)                             QuickSortVarArrayInt_(sizeof(type),   (uxx)std::alignment_of<type>(), false, true,  0, sizeof(type), (arrayPntr))
#define QuickSortVarArrayUintElem(type, arrayPntr)                            QuickSortVarArrayInt_(sizeof(type),   (uxx)std::alignment_of<type>(), false, false, 0, sizeof(type), (arrayPntr))
#define QuickSortVarArrayFloat(type, arrayPntr)                               QuickSortVarArrayFloat_(sizeof(type), (uxx)std::alignment_of<type>(), false,        0, sizeof(type), (arrayPntr))
#define QuickSortVarArrayIntMember(type, memberName, arrayPntr)               QuickSortVarArrayInt_(sizeof(type),   (uxx)std::alignment_of<type>(), false, true,  STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortVarArrayUintMember(type, memberName, arrayPntr)              QuickSortVarArrayInt_(sizeof(type),   (uxx)std::alignment_of<type>(), false, false, STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortVarArrayFloatMember(type, memberName, arrayPntr)             QuickSortVarArrayFloat_(sizeof(type), (uxx)std::alignment_of<type>(), false,        STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortVarArrayReversed(type, arrayPntr, compareFunc, contextPntr)  QuickSortVarArray_(sizeof(type),      (uxx)std::alignment_of<type>(), true,                          (arrayPntr), (compareFunc), (contextPntr))
#define QuickSortVarArrayIntElemReversed(type, arrayPntr)                     QuickSortVarArrayInt_(sizeof(type),   (uxx)std::alignment_of<type>(), true,  true,  0, sizeof(type), (arrayPntr))
#define QuickSortVarArrayUintElemReversed(type, arrayPntr)                    QuickSortVarArrayInt_(sizeof(type),   (uxx)std::alignment_of<type>(), true,  false, 0, sizeof(type), (arrayPntr))
#define QuickSortVarArrayFloatReversed(type, arrayPntr)                       QuickSortVarArrayFloat_(sizeof(type), (uxx)std::alignment_of<type>(), true,         0, sizeof(type), (arrayPntr))
#define QuickSortVarArrayIntMemberReversed(type, memberName, arrayPntr)       QuickSortVarArrayInt_(sizeof(type),   (uxx)std::alignment_of<type>(), true,  true,  STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortVarArrayUintMemberReversed(type, memberName, arrayPntr)      QuickSortVarArrayInt_(sizeof(type),   (uxx)std::alignment_of<type>(), true,  false, STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#define QuickSortVarArrayFloatMemberReversed(type, memberName, arrayPntr)     QuickSortVarArrayFloat_(sizeof(type), (uxx)std::alignment_of<type>(), true,         STRUCT_VAR_OFFSET(type, memberName), STRUCT_VAR_SIZE(type, memberName), (arrayPntr))
#endif

#if PIG_CORE_IMPLEMENTATION

PEXPI bool IsVarArraySorted_(uxx itemSize, uxx itemAlignment, VarArray* array, CompareFunc_f* compareFunc, void* contextPntr)
{
	NotNull(array);
	#if DEBUG_BUILD
	Assert(IsVarArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to IsVarArraySorted. Make sure you're accessing the VarArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to IsVarArraySorted. Make sure you're accessing the VarArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	return IsSortedFlat(array->items, array->length, array->itemSize, compareFunc, contextPntr);
}
PEXPI bool IsVarArraySortedInt_(uxx itemSize, uxx itemAlignment, bool isMemberSigned, uxx memberOffset, uxx memberSize, VarArray* array)
{
	NotNull(array);
	#if DEBUG_BUILD
	Assert(IsVarArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to IsVarArraySorted. Make sure you're accessing the VarArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to IsVarArraySorted. Make sure you're accessing the VarArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	return IsSortedFlatOnIntMember_(isMemberSigned, memberOffset, memberSize, array->items, array->length, array->itemSize);
}
PEXPI bool IsVarArraySortedFloat_(uxx itemSize, uxx itemAlignment, uxx memberOffset, uxx memberSize, VarArray* array)
{
	NotNull(array);
	#if DEBUG_BUILD
	Assert(IsVarArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to IsVarArraySorted. Make sure you're accessing the VarArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to IsVarArraySorted. Make sure you're accessing the VarArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	return IsSortedFlatOnFloatMember_(memberOffset, memberSize, array->items, array->length, array->itemSize);
}

PEXPI void QuickSortVarArray_(uxx itemSize, uxx itemAlignment, bool reverseSort, VarArray* array, CompareFunc_f* compareFunc, void* contextPntr)
{
	NotNull(array);
	#if DEBUG_BUILD
	Assert(IsVarArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to QuickSortVarArray. Make sure you're accessing the VarArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to QuickSortVarArray. Make sure you're accessing the VarArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	QuickSortFlat(array->items, array->length, array->itemSize, compareFunc, contextPntr);
}

PEXPI void QuickSortVarArrayInt_(uxx itemSize, uxx itemAlignment, bool reverseSort, bool isMemberSigned, uxx memberOffset, uxx memberSize, VarArray* array)
{
	NotNull(array);
	#if DEBUG_BUILD
	Assert(IsVarArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to QuickSortVarArrayInt. Make sure you're accessing the VarArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to QuickSortVarArrayInt. Make sure you're accessing the VarArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	QuickSortFlatOnIntMember_(reverseSort, isMemberSigned, memberOffset, memberSize, array->items, array->length, array->itemSize);
}
PEXPI void QuickSortVarArrayFloat_(uxx itemSize, uxx itemAlignment, bool reverseSort, uxx memberOffset, uxx memberSize, VarArray* array)
{
	NotNull(array);
	#if DEBUG_BUILD
	Assert(IsVarArrayInit(array));
	AssertMsg(array->itemSize == itemSize, "Invalid itemSize passed to QuickSortVarArrayFloat. Make sure you're accessing the VarArray with the correct type!");
	AssertMsg(array->itemAlignment == itemAlignment, "Invalid itemAlignment passed to QuickSortVarArrayFloat. Make sure you're accessing the VarArray with the correct type!");
	#else
	UNUSED(itemSize);
	UNUSED(itemAlignment);
	#endif
	QuickSortFlatOnFloatMember_(reverseSort, memberOffset, memberSize, array->items, array->length, array->itemSize);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_SORTING_AND_VAR_ARRAY_H
