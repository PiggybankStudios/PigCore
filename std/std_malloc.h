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
#include "base/base_defines_check.h"
#include "std/std_includes.h"

// +--------------------------------------------------------------+
// |                        Regular Alias                         |
// +--------------------------------------------------------------+
#if TARGET_HAS_OFFICIAL_STDLIB

#define MALLOC_ALIGNED_AVAILABLE 1

#define MyMalloc(numBytes) malloc(numBytes)
#if COMPILER_IS_MSVC
#define MyMallocAligned(numBytes, alignment) _aligned_malloc((numBytes), (alignment)) //this is available in C11 and on
#elif 0 //TODO: Figure out why this was returning nullptr!
#define MyMallocAligned(numBytes, alignment) aligned_alloc((numBytes), (alignment)) //this is available in C11 and on in glibc
#else
#define MyMallocAligned(numBytes, alignment) malloc(numBytes)
#endif
#if LANGUAGE_IS_CPP
#define MyMallocType(type) (type*)MyMallocAligned(sizeof(type), std::alignment_of<type>())
#else
#define MyMallocType(type) (type*)MyMallocAligned(sizeof(type), _Alignof(type))
#endif
#define MyRealloc(pntr, numBytes) realloc((pntr), (numBytes))
#define MyFree(pntr) free(pntr)
#if COMPILER_IS_MSVC
#define MyFreeAligned(pntr) _aligned_free(pntr)
#else
#define MyFreeAligned(pntr) MyFree(pntr) //glibc allows us to free aligned allocations with regular free
#endif

// +--------------------------------------------------------------+
// |               WebAssembly Malloc Only Routing                |
// +--------------------------------------------------------------+
#elif TARGET_IS_WASM

#define MALLOC_ALIGNED_AVAILABLE 0

#define MyMalloc(numBytes) malloc(numBytes)
#define MyMallocAligned(numBytes, alignment) malloc(numBytes) //TODO: aligned_malloc needs to be implemented in our custom stdlib!
#if LANGUAGE_IS_CPP
#define MyMallocType(type) (type*)MyMallocAligned(sizeof(type), std::alignment_of<type>())
#else
#define MyMallocType(type) (type*)MyMallocAligned(sizeof(type), _Alignof(type))
#endif
#define MyRealloc(pntr, numBytes) realloc((pntr), (numBytes)) //This will assert if called!
#define MyFree(pntr) free(pntr) //This will assert if called!
#define MyFreeAligned(pntr) MyFree(pntr)

// +--------------------------------------------------------------+
// |                  Playdate pdrealloc Routing                  |
// +--------------------------------------------------------------+
#elif TARGET_IS_PLAYDATE

#define MALLOC_ALIGNED_AVAILABLE 0

#define MyMalloc(numBytes) pdrealloc(nullptr, (numBytes))
#define MyMallocAligned(numBytes, alignment) MyMallocAligned(numBytes) //alignment options are not available in pdrealloc
#define MyMallocType(type) (type*)pdrealloc(nullptr, sizeof(type))
#define MyRealloc(pntr, numBytes) pdrealloc((pntr), (numBytes))
#define MyFree(pntr) pdrealloc((pntr), 0)
#define MyFreeAligned(pntr) MyFree(pntr)

#if PLAYDATE_DEVICE
	#if PIG_CORE_IMPLEMENTATION
	void* _malloc_r(plex _reent* _REENT, size_t nbytes) { return pdrealloc(NULL,nbytes); }
	void* _realloc_r(plex _reent* _REENT, void* ptr, size_t nbytes) { return pdrealloc(ptr,nbytes); }
	void _free_r(plex _reent* _REENT, void* ptr ) { if ( ptr != NULL ) pdrealloc(ptr,0); }
	#else
	PEXP void* _malloc_r(plex _reent* _REENT, size_t nbytes);
	PEXP void* _realloc_r(plex _reent* _REENT, void* ptr, size_t nbytes);
	PEXP void _free_r(plex _reent* _REENT, void* ptr );
	#endif
#endif //PLAYDATE_DEVICE

// +--------------------------------------------------------------+
// |                       Unhandled TARGET                       |
// +--------------------------------------------------------------+
#else
#error This TARGET is not supported yet in std_malloc.h! We must decide what to do will malloc/realloc/free calls!
#endif

#endif //  _STD_MEMORY_H
