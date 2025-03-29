/*
File:   cross_string_and_typed_array.h
Author: Taylor Robbins
Date:   03\29\2025
*/

#ifndef _CROSS_STRING_AND_TYPED_ARRAY_H
#define _CROSS_STRING_AND_TYPED_ARRAY_H

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	DECLARE_TYPED_ARRAY(Str8, Str8Array)
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

DECLARE_AND_IMPLEMENT_TYPED_ARRAY_DECOR(Str8, Str8Array, PEXP)

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_STRING_AND_TYPED_ARRAY_H
