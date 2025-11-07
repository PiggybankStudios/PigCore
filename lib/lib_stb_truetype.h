/*
File:   lib_stb_truetype.h
Author: Taylor Robbins
Date:   10\29\2025
*/

#ifndef _LIB_STB_TRUETYPE_H
#define _LIB_STB_TRUETYPE_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "std/std_basic_math.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
#include "lib/lib_stb_rect_pack.h" // We always use stb_rect_pack.h algorithm for stb_truetype.h

#if PIG_CORE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#endif

#define STBTT_STATIC //TODO: Do we want to expose stb_truetype functions to the application when working as a DLL?
#define STBTT_ifloor(x)                     FloorR32i(x)
#define STBTT_iceil(x)                      CeilR32i(x)
#define STBTT_sqrt(x)                       SqrtR32(x)
#define STBTT_pow(x, exp)                   PowR32((x), (exp))
#define STBTT_fabs(x)                       AbsR32(x)
#define STBTT_malloc(numBytes, user)        AllocMem((Arena*)(user), (numBytes))
#define STBTT_free(pointer, user)           do { if ((pointer) != nullptr && CanArenaFree((Arena*)(user))) { FreeMem((Arena*)(user), (pointer), 0); } } while(0)
#define STBTT_assert(expession)             do { Assert(expession); } while(0)
#define STBTT_strlen(str)                   (uxx)MyStrLength64(str)
#define STBTT_memcpy(dest, source, length)  MyMemCopy((dest), (source), (length))
#define STBTT_memset(dest, value, length)   MyMemSet((dest), (value), (length))

#if COMPILER_IS_MSVC
#pragma warning(push)
#pragma warning(disable: 5262) //implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases
#endif

#if COMPILER_IS_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough" //warning: unannotated fall-through between switch labels
#endif

#include "third_party/stb/stb_truetype.h"

#if COMPILER_IS_MSVC
#pragma warning(pop)
#endif

#if COMPILER_IS_CLANG
#pragma clang diagnostic pop
#endif

#endif //  _LIB_STB_TRUETYPE_H
