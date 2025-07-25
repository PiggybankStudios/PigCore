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
#include "mem/mem_arena.h"

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
// The below functions define array bounds checking and convenience functions for a provided type.
#define DECLARE_TYPED_ARRAY_FUNCTIONS_DECOR(elementType, arrayStructName, functionDecor)         \
functionDecor void arrayStructName##_Free(arrayStructName* array, Arena* arena);                 \
functionDecor arrayStructName arrayStructName##_Init(uxx initialCapacity, Arena* arena);         \
functionDecor elementType* arrayStructName##_Get(arrayStructName* array, uxx index);             \
functionDecor elementType arrayStructName##_GetValue(arrayStructName* array, uxx index);         \
functionDecor elementType* arrayStructName##_Add(arrayStructName* array, elementType item);      \
functionDecor elementType* arrayStructName##_GetSlice(arrayStructName##Slice* slice, uxx index); \
functionDecor elementType arrayStructName##_RemoveSwapback(arrayStructName* array, uxx index);   \
functionDecor void arrayStructName##_Set(arrayStructName* array, uxx index, elementType value);  \

#define IMPLEMENT_TYPED_ARRAY_FUNCTIONS_DECOR(elementType, arrayStructName, functionDecor)             \
elementType elementType##_DEFAULT = ZEROED;                                                            \
functionDecor void arrayStructName##_Free(arrayStructName* array, Arena* arena)                        \
{                                                                                                      \
	NotNull(array);                                                                                    \
	NotNull(arena);                                                                                    \
	if (array->items != nullptr)                                                                       \
	{                                                                                                  \
		if (CanArenaFree(arena)) { FreeArray(elementType, arena, array->allocLength, array->items); }  \
		array->items = nullptr;                                                                        \
	}                                                                                                  \
	array->allocLength = 0;                                                                            \
	array->length = 0;                                                                                 \
}                                                                                                      \
functionDecor arrayStructName arrayStructName##_Init(uxx initialCapacity, Arena* arena)                \
{                                                                                                      \
	return NEW_STRUCT(arrayStructName) {                                                               \
		.allocLength = initialCapacity,                                                                \
		.length = 0,                                                                                   \
		.items = AllocArray(elementType, arena, initialCapacity)                                       \
	};                                                                                                 \
}                                                                                                      \
functionDecor elementType* arrayStructName##_Get(arrayStructName* array, uxx index)                    \
{                                                                                                      \
	return TypedArrayRangeCheck(index, array->length) ? &array->items[index] : &elementType##_DEFAULT; \
}                                                                                                      \
functionDecor elementType arrayStructName##_GetValue(arrayStructName* array, uxx index)                \
{                                                                                                      \
	return TypedArrayRangeCheck(index, array->length) ? array->items[index] : elementType##_DEFAULT;   \
}                                                                                                      \
functionDecor elementType* arrayStructName##_Add(arrayStructName* array, elementType item)             \
{                                                                                                      \
	if (TypedArrayCapacityCheck(array->length, array->allocLength))                                    \
	{                                                                                                  \
		array->items[array->length++] = item;                                                          \
		return &array->items[array->length - 1];                                                       \
	}                                                                                                  \
	return &elementType##_DEFAULT;                                                                     \
}                                                                                                      \
functionDecor elementType* arrayStructName##_GetSlice(arrayStructName##Slice* slice, uxx index)        \
{                                                                                                      \
    return TypedArrayRangeCheck(index, slice->length) ? &slice->items[index] : &elementType##_DEFAULT; \
}                                                                                                      \
functionDecor elementType arrayStructName##_RemoveSwapback(arrayStructName* array, uxx index)          \
{                                                                                                      \
	if (TypedArrayRangeCheck(index, array->length))                                                    \
	{                                                                                                  \
		array->length--;                                                                               \
		elementType removed = array->items[index];                                                     \
		array->items[index] = array->items[array->length];                                             \
		return removed;                                                                                \
	}                                                                                                  \
	return elementType##_DEFAULT;                                                                      \
}                                                                                                      \
functionDecor void arrayStructName##_Set(arrayStructName* array, uxx index, elementType value)         \
{                                                                                                      \
	if (TypedArrayRangeCheck(index, array->allocLength))                                               \
	{                                                                                                  \
		array->items[index] = value;                                                                   \
		array->length = index < array->length ? array->length : index + 1;                             \
	}                                                                                                  \
}                                                                                                      \

//TODO: Somehow we should make it so we can add semicolon after this macro and not have the compiler complain! This would make our syntax highlighting better in Sublime
#define DECLARE_TYPED_ARRAY_DECOR(elementType, arrayStructName, functionDecor)   \
typedef plex                                                                     \
{                                                                                \
    uxx length;                                                                  \
    uxx allocLength;                                                             \
    elementType* items;                                                          \
} arrayStructName;                                                               \
typedef plex                                                                     \
{                                                                                \
    uxx length;                                                                  \
    elementType* items;                                                          \
} arrayStructName##Slice;                                                        \
DECLARE_TYPED_ARRAY_FUNCTIONS_DECOR(elementType, arrayStructName, functionDecor) \

#define IMPLEMENT_TYPED_ARRAY_DECOR(elementType, arrayStructName, functionDecor)   \
IMPLEMENT_TYPED_ARRAY_FUNCTIONS_DECOR(elementType, arrayStructName, functionDecor) \

#define DECLARE_AND_IMPLEMENT_TYPED_ARRAY_DECOR(elementType, arrayStructName, functionDecor) \
DECLARE_TYPED_ARRAY_DECOR(elementType, arrayStructName, functionDecor)                       \
IMPLEMENT_TYPED_ARRAY_DECOR(elementType, arrayStructName, functionDecor)                     \

#define DECLARE_TYPED_ARRAY_FUNCTIONS(type, arrayStructName)            DECLARE_TYPED_ARRAY_FUNCTIONS_DECOR(type, arrayStructName, )
#define IMPLEMENT_TYPED_ARRAY_FUNCTIONS(type, arrayStructName)          IMPLEMENT_TYPED_ARRAY_FUNCTIONS_DECOR(type, arrayStructName, )
#define DECLARE_TYPED_ARRAY(elementType, arrayStructName)               DECLARE_TYPED_ARRAY_DECOR(elementType, arrayStructName, )
#define IMPLEMENT_TYPED_ARRAY(elementType, arrayStructName)             IMPLEMENT_TYPED_ARRAY_DECOR(elementType, arrayStructName, )
#define DECLARE_AND_IMPLEMENT_TYPED_ARRAY(elementType, arrayStructName) DECLARE_AND_IMPLEMENT_TYPED_ARRAY_DECOR(elementType, arrayStructName, )

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	bool TypedArrayRangeCheck(uxx index, uxx length);
	bool TypedArrayCapacityCheck(uxx length, uxx allocLength);
	DECLARE_TYPED_ARRAY(bool, boolArray)
	DECLARE_TYPED_ARRAY(i32, i32Array)
	DECLARE_TYPED_ARRAY(u32, u32Array)
	DECLARE_TYPED_ARRAY(i64, i64Array)
	DECLARE_TYPED_ARRAY(u64, u64Array)
	DECLARE_TYPED_ARRAY(ixx, ixxArray)
	DECLARE_TYPED_ARRAY(uxx, uxxArray)
	DECLARE_TYPED_ARRAY(char, charArray)
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP bool TypedArrayRangeCheck(uxx index, uxx length)
{
	if (index < length) { return true; }
	AssertMsg(index < length, "Clay attempted to make an out of bounds array access. This is an internal error and is likely a bug.");
	return false;
}

PEXP bool TypedArrayCapacityCheck(uxx length, uxx allocLength)
{
	if (length < allocLength) { return true; }
	AssertMsg(length < allocLength, "Clay attempted to make an out of bounds array access. This is an internal error and is likely a bug.");
	return false;
}

DECLARE_AND_IMPLEMENT_TYPED_ARRAY_DECOR(bool, boolArray, PEXP)
DECLARE_AND_IMPLEMENT_TYPED_ARRAY_DECOR(i32, i32Array, PEXP)
DECLARE_AND_IMPLEMENT_TYPED_ARRAY_DECOR(u32, u32Array, PEXP)
DECLARE_AND_IMPLEMENT_TYPED_ARRAY_DECOR(i64, i64Array, PEXP)
DECLARE_AND_IMPLEMENT_TYPED_ARRAY_DECOR(u64, u64Array, PEXP)
DECLARE_AND_IMPLEMENT_TYPED_ARRAY_DECOR(ixx, ixxArray, PEXP)
DECLARE_AND_IMPLEMENT_TYPED_ARRAY_DECOR(uxx, uxxArray, PEXP)
DECLARE_AND_IMPLEMENT_TYPED_ARRAY_DECOR(char, charArray, PEXP)

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_TYPED_ARRAY_H

#if defined(_STRUCT_STRING_H) && defined(_STRUCT_TYPED_ARRAY_H)
#include "cross/cross_string_and_typed_array.h"
#endif
