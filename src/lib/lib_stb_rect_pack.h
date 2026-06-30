/*
File:   lib_stb_rect_pack.h
Author: Taylor Robbins
Date:   10\29\2025
*/

#ifndef _LIB_STB_RECT_PACK_H
#define _LIB_STB_RECT_PACK_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "misc/misc_sorting.h"

#if PIG_CORE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION
#endif

// The normal qsort function in stdlib doesn't take a contextPntr.
// To support the compare function in stb_rect_pack.h we use this function as the
// primary compare function and store the stb_rect_pack.h one inside the contextPntr
// We can then cast it and call it without passing a contextPntr
#if !PIG_CORE_IMPLEMENTATION
COMPARE_FUNC_DEF(StripContextCompareFunc);
#else //PIG_CORE_IMPLEMENTATION
COMPARE_FUNC_DEF(StripContextCompareFunc)
{
	int (*realCompareFunc)(const void*, const void*) = (int (*)(const void*, const void*))contextPntr;
	return realCompareFunc(left, right);
}
#endif //PIG_CORE_IMPLEMENTATION

#define STBRP_STATIC //TODO: Do we want to expose stb_rect_pack functions to the application when working as a DLL?
#define STBRP_SORT(arrayPntr, numElements, elementSize, compareFunc) QuickSortFlat((arrayPntr), (numElements), (elementSize), StripContextCompareFunc, (void*)(compareFunc))
#define STBRP_ASSERT(condition)             Assert(condition)
#include "third_party/stb/stb_rect_pack.h"

#endif //  _LIB_STB_RECT_PACK_H
