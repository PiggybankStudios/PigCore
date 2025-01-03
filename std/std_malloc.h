/*
File:   std_malloc.h
Author: Taylor Robbins
Date:   01\02\2025
Description:
	** Contains routing aliases for malloc, realloc, and free called MyMalloc, MyRealloc, and MyFree
*/

#ifndef _STD_MEMORY_H
#define _STD_MEMORY_H

#include "base/base_compiler_check.h"
#include "std/std_includes.h"

// +--------------------------------------------------------------+
// |                        Regular Alias                         |
// +--------------------------------------------------------------+
#if TARGET_HAS_OFFICIAL_STDLIB

#define MyMalloc(numBytes) malloc(numBytes)
#define MyMallocType(type) (type*)malloc(sizeof(type))
#define MyRealloc(pntr, numBytes) realloc((pntr), (numBytes))
#define MyFree(pntr) free(pntr)

// +--------------------------------------------------------------+
// |                  Playdate pdrealloc Routing                  |
// +--------------------------------------------------------------+
#elif TARGET_IS_PLAYDATE

#define MyMalloc(numBytes) pdrealloc(nullptr, (numBytes))
#define MyMallocType(type) (type*)pdrealloc(nullptr, sizeof(type))
#define MyRealloc(pntr, numBytes) pdrealloc((pntr), (numBytes))
#define MyFree(pntr) pdrealloc((pntr), 0)

#if PLAYDATE_DEVICE
void* _malloc_r(struct _reent* _REENT, size_t nbytes) { return pdrealloc(NULL,nbytes); }
void* _realloc_r(struct _reent* _REENT, void* ptr, size_t nbytes) { return pdrealloc(ptr,nbytes); }
void _free_r(struct _reent* _REENT, void* ptr ) { if ( ptr != NULL ) pdrealloc(ptr,0); }
#endif //PLAYDATE_DEVICE

// +--------------------------------------------------------------+
// |                       Unhandled TARGET                       |
// +--------------------------------------------------------------+
#else
#error This TARGET is not supported yet in std_memory.h! We must decide what to do will malloc/realloc/free calls!
#endif

#endif //  _STD_MEMORY_H
