/*
File:   lib_kb_text_shape.h
Author: Taylor Robbins
Date:   08\09\2026
Decription:
	** kb_text_shape.h is a single header include library that helps
	** use handle text segmentation and shaping
	** (similar to HarfBuzz but smaller and simpler and written in C)
*/

#ifndef _LIB_KB_TEXT_SHAPE_H
#define _LIB_KB_TEXT_SHAPE_H

#include "base/base_defines_check.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"

#define KB_TEXT_SHAPE_NO_CRT
#define KBTS_MEMSET MyMemSet
#define KBTS_MEMCPY MyMemCopy
#define KBTS_MALLOC(allocatorPntr, numBytes) AllocMem((Arena*)(allocatorPntr), (numBytes))
#define KBTS_FREE(allocatorPntr, allocPntr)   FreeMem((Arena*)(allocatorPntr), (allocPntr), 0)

// #define KB_TEXT_SHAPE_STATIC

#if PIG_CORE_IMPLEMENTATION
#define KB_TEXT_SHAPE_IMPLEMENTATION
#endif

#if COMPILER_IS_MSVC
#pragma warning(push)
#pragma warning(disable:5262) //error C5262: implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases
#endif
#include "third_party/kb_text_shape/kb_text_shape.h"
#if COMPILER_IS_MSVC
#pragma warning(pop)
#endif

#endif //  _LIB_KB_TEXT_SHAPE_H
