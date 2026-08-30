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
#include "base/base_typedefs.h"
#include "base/base_debug_output.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
#include "misc/misc_standard_names.h"

static void* KbTextShapeMalloc(Arena* arenaPntr, uxx numBytes);
static void KbTextShapeFree(Arena* arenaPntr, void* allocPntr);

#define KB_TEXT_SHAPE_NO_CRT
#define KBTS_MEMSET MyMemSet
#define KBTS_MEMCPY MyMemCopy
#define KBTS_MALLOC(allocatorPntr, numBytes)  KbTextShapeMalloc((Arena*)(allocatorPntr), (numBytes))
#define KBTS_FREE(allocatorPntr, allocPntr)   KbTextShapeFree((Arena*)(allocatorPntr), (allocPntr))

#if PIG_CORE_IMPLEMENTATION
#define KBTS_EXPORT PEXP
#define KB_TEXT_SHAPE_IMPLEMENTATION
#else
#define KBTS_EXPORT //nothing
#endif

#if COMPILER_IS_MSVC
#pragma warning(push)
#pragma warning(disable:5262) //error C5262: implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases
#pragma warning(disable:4100) //warning C4100: 'SubtableCount': unreferenced parameter
#endif
#if COMPILER_IS_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough" //warning: unannotated fall-through between switch labels
#endif
#include "third_party/kb_text_shape/kb_text_shape.h"
#if COMPILER_IS_MSVC
#pragma warning(pop)
#endif
#if COMPILER_IS_CLANG
#pragma clang diagnostic pop
#endif

#if PIG_CORE_IMPLEMENTATION
	static void* KbTextShapeMalloc(Arena* arenaPntr, uxx numBytes)
	{
		void* result = AllocMem(arenaPntr, numBytes);
		PrintLine_D("kb_malloc(%p, %llu) -> %p", arenaPntr, numBytes, result);
		if (numBytes == sizeof(kbts_shape_context))
		{
			Str8 name = GetStandardPeopleFirstName((uxx)result);
			PrintLine_D("kbts_shape_context at %p! %.*s", result, StrPrint(name));
		}
		return result;
	}
	static void KbTextShapeFree(Arena* arenaPntr, void* allocPntr)
	{
		PrintLine_D("kb_free(%p, %p)", arenaPntr, allocPntr);
		FreeMem(arenaPntr, allocPntr, 0);
	}
#endif

#endif //  _LIB_KB_TEXT_SHAPE_H
