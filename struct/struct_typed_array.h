/*
File:   struct_typed_array.h
Author: Taylor Robbins
Date:   03\28\2025
Description:
	** Defines macros that help generate a VarArray-like structure and API for a
	** type using a single macro invocation. This allows each structure and API
	** to be truly type aware and safe, but it also requires each type that is going
	** to have an array for it be defined ONLY once, and the generated code can be
	** a bit hard to debug. Generally we prefer using VarArray instead of these typed
	** arrays but sometimes these typed arrays are more useful. Also the clay.h layout
	** library uses these arrays pretty extensively
*/

#ifndef _STRUCT_TYPED_ARRAY_H
#define _STRUCT_TYPED_ARRAY_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"

// Clay_Arena is a memory arena structure that is used by clay to manage its internal allocations.
// Rather than creating it by hand, it's easier to use Clay_CreateArenaWithCapacityAndMemory()
typedef struct
{
    uintptr_t nextAllocation;
    size_t capacity;
    char* memory;
} Clay_Arena;

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void* AllocateTypedArray(i32 initialCapacity, u32 itemSize, Clay_Arena* arena);
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
// The below functions define array bounds checking and convenience functions for a provided type.
#define DEFINE_TYPED_ARRAY_FUNCTIONS(type, arrayStructName)                                       \
type type##_DEFAULT = ZEROED;                                                                     \
arrayStructName arrayStructName##_Allocate_Arena(i32 initialCapacity, Clay_Arena* arena)          \
{                                                                                                 \
	return NEW_STRUCT(arrayStructName) {                                                          \
		.allocLength = initialCapacity,                                                           \
		.length = 0,                                                                              \
		.items = (type*)AllocateTypedArray(initialCapacity, sizeof(type), arena)                  \
	};                                                                                            \
}                                                                                                 \
type* arrayStructName##_Get(arrayStructName* array, i32 index)                                    \
{                                                                                                 \
	return Clay__Array_RangeCheck(index, array->length) ? &array->items[index] : &type##_DEFAULT; \
}                                                                                                 \
type arrayStructName##_GetValue(arrayStructName* array, i32 index)                                \
{                                                                                                 \
	return Clay__Array_RangeCheck(index, array->length) ? array->items[index] : type##_DEFAULT;   \
}                                                                                                 \
type* arrayStructName##_Add(arrayStructName* array, type item)                                    \
{                                                                                                 \
	if (Clay__Array_AddCapacityCheck(array->length, array->allocLength))                          \
	{                                                                                             \
		array->items[array->length++] = item;                                                     \
		return &array->items[array->length - 1];                                                  \
	}                                                                                             \
	return &type##_DEFAULT;                                                                       \
}                                                                                                 \
type* arrayStructName##Slice_Get(arrayStructName##Slice* slice, i32 index)                        \
{                                                                                                 \
    return Clay__Array_RangeCheck(index, slice->length) ? &slice->items[index] : &type##_DEFAULT; \
}                                                                                                 \
type arrayStructName##_RemoveSwapback(arrayStructName* array, i32 index)                          \
{                                                                                                 \
	if (Clay__Array_RangeCheck(index, array->length))                                             \
	{                                                                                             \
		array->length--;                                                                          \
		type removed = array->items[index];                                                       \
		array->items[index] = array->items[array->length];                                        \
		return removed;                                                                           \
	}                                                                                             \
	return type##_DEFAULT;                                                                        \
}                                                                                                 \
void arrayStructName##_Set(arrayStructName* array, i32 index, type value)                         \
{                                                                                                 \
	if (Clay__Array_RangeCheck(index, array->allocLength))                                        \
	{                                                                                             \
		array->items[index] = value;                                                              \
		array->length = index < array->length ? array->length : index + 1;                        \
	}                                                                                             \
}

//TODO: Somehow we should make it so we can add semicolon after this macro and not have the compiler complain! This would make our syntax highlighting better in Sublime
#define DEFINE_TYPED_ARRAY(elementType, arrayStructName)     \
typedef struct                                               \
{                                                            \
    i32 allocLength;                                         \
    i32 length;                                              \
    elementType* items;                                      \
} arrayStructName;                                           \
typedef struct                                               \
{                                                            \
    i32 length;                                              \
    elementType* items;                                      \
} arrayStructName##Slice;                                    \
DEFINE_TYPED_ARRAY_FUNCTIONS(elementType, arrayStructName)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void* AllocateTypedArray(i32 initialCapacity, u32 itemSize, Clay_Arena* arena)
{
    size_t totalSizeBytes = initialCapacity * itemSize;
    uintptr_t nextAllocOffset = arena->nextAllocation + (64 - (arena->nextAllocation % 64));
    if (nextAllocOffset + totalSizeBytes <= arena->capacity)
    {
        arena->nextAllocation = nextAllocOffset + totalSizeBytes;
        return (void*)((uintptr_t)arena->memory + (uintptr_t)nextAllocOffset);
    }
    else
    {
    	AssertMsg(false, "Clay attempted to allocate memory in its arena, but ran out of capacity. Try increasing the capacity of the arena passed to Clay_Initialize()");
	    return nullptr;
    }
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_TYPED_ARRAY_H
