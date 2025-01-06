/*
File:   std_includes.h
Author: Taylor Robbins
Date:   01\02\2025
Descriptions:
	** Handles the majority of #includes of standard C library header files based on
	** TARGET, COMPILER, and features enabled in build_config.h.
	** Standard library header includes CAN happen outside this file but they are
	** generally discouraged unless we can't move the include into this file for some reason
*/

#ifndef _STD_INCLUDES_H
#define _STD_INCLUDES_H

#include "build_config.h"
#include "base/base_compiler_check.h"

#ifndef DONT_INCLUDE_WINDOWS_H
#define DONT_INCLUDE_WINDOWS_H 0
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <stddef.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <float.h>
// #include <assert.h> //TODO: Do we want this?
#if (TARGET_IS_WINDOWS || TARGET_IS_WEB)
	#include <intrin.h>
#endif
#if (TARGET_IS_LINUX || TARGET_IS_OSX)
	#include <signal.h>
	// Gives us getpagesize and sleep
	#include <unistd.h>
	// Gives us mmap
	#include <sys/mman.h>
#endif
#if COMPILER_IS_MSVC
	#pragma warning(push)
	#pragma warning(disable: 5262) //error C5262: implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases
	#endif
	// Gives us things like atomic_int type
	#include <stdatomic.h>
	#if COMPILER_IS_MSVC
	#pragma warning(pop)
#endif
#if COMPILER_IS_MSVC
	// Needed for _beginthread/_endthread
	#include <process.h>
#endif

#if TARGET_IS_ORCA
	#include <orca.h>
	#include "gy_orca_aliases.h"
#endif

#if TARGET_IS_PLAYDATE
	#include "pd_api.h"
	extern PlaydateAPI* pd;
	void* (*pdrealloc)(void* pntr, size_t size);
#endif

#if (TARGET_IS_WINDOWS && !DONT_INCLUDE_WINDOWS_H)
	// Gives us Sleep
	#include <windows.h>
#endif

#endif //  _STD_INCLUDES_H
