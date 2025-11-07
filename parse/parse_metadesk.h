/*
File:   parse_metadesk.h
Author: Taylor Robbins
Date:   03\31\2025
*/

#ifndef _PARSE_METADESK_H
#define _PARSE_METADESK_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "mem/mem_arena.h"
#include "struct/struct_string.h"
#include "lib/lib_metadesk.h"

#if BUILD_WITH_METADESK

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Str8 ToStr8FromMd(MD_String8 mdStr8);
	PIG_CORE_INLINE MD_String8 ToMdString8(Str8 str);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI Str8 ToStr8FromMd(MD_String8 mdStr) { DebugAssert(mdStr.size <= UINTXX_MAX); return NewStr8((uxx)mdStr.size, mdStr.str); }
PEXPI MD_String8 ToMdString8(Str8 str) { return MD_S8(str.bytes, str.length); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_METADESK

#endif //  _PARSE_METADESK_H
