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
#include "base/base_defines_check.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <float.h>
#include <limits.h>
#include <stddef.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <uchar.h> //for char16_t
//TODO: HandmadeMath.h includes <xmmintrin.h> when SSE is supported

#if (TARGET_IS_WINDOWS || USING_CUSTOM_STDLIB)
	#include <intrin.h>
#endif
#if (TARGET_IS_LINUX || TARGET_IS_OSX)
	#include <signal.h>
	// Gives us getpagesize and sleep
	#include <unistd.h>
	// Gives us mmap
	#include <sys/mman.h>
	#include <sys/time.h>
	#include <errno.h>
	#include <dlfcn.h> //needed for dlopen
#endif
#if COMPILER_IS_MSVC && LANGUAGE_IS_C
	#pragma warning(push)
	#pragma warning(disable: 5262) //error C5262: implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases
	// Gives us things like atomic_int type
	#include <stdatomic.h>
	#pragma warning(pop)
#endif
#if LANGUAGE_IS_CPP
	#include <type_traits> //needed for stuff like std::alignment_of<T>()
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

#if TARGET_IS_WINDOWS
	#if BUILD_WITH_RAYLIB
	//NOTE: All these things conflict with raylib.h definitions, so we #define before windows.h to make the header use a different name when #included
	#define Rectangle Win32_Rectangle
	#define CloseWindow Win32_CloseWindow
	#define ShowCursor Win32_ShowCursor
	#define Color Win32_Color
	#endif
	
	//TODO: We probably want WIN32_LEAN_AND_MEAN
	#include <windows.h>
	
	#if BUILD_WITH_RAYLIB
	#undef Rectangle
	#undef CloseWindow
	#undef ShowCursor
	#undef Color
	//NOTE: These 4 also conflict but they are #defines in windows.h, so all that is needed is to #undef them
	#undef DrawText
	#undef LoadImage
	#undef DrawTextEx
	#undef PlaySound
	#endif
#endif

#endif //  _STD_INCLUDES_H
