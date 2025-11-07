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

#if PIG_CORE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION
#endif

#define STBRP_STATIC //TODO: Do we want to expose stb_rect_pack functions to the application when working as a DLL?
#define STBRP_SORT                          qsort //TODO: Do we want to route this to one of our own sorting functions?
#define STBRP_ASSERT(condition)             Assert(condition)
#include "third_party/stb/stb_rect_pack.h"

#endif //  _LIB_STB_RECT_PACK_H
