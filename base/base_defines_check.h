/*
File:   base_defines_check.h
Author: Taylor Robbins
Date:   01\19\2025
Description:
	** This file makes sure that all defines that affect the way PigCore is
	** compiled are defined to either 1 or 0
*/

#ifndef _BASE_DEFINES_CHECK_H
#define _BASE_DEFINES_CHECK_H

#include "base/base_compiler_check.h"

// When linking with PigCore as a dll, set this define to 0 before including any of the PigCore
// headers to prevent the implementations from being compiled into your compilation unit
#ifndef PIG_CORE_IMPLEMENTATION
#define PIG_CORE_IMPLEMENTATION 1
#endif

// Any function in PigCore that want's to be inlined can have their behavior changed by changing this define
#ifndef PIG_CORE_INLINE
#if TARGET_IS_WINDOWS
#define PIG_CORE_INLINE inline
#else
#define PIG_CORE_INLINE //TODO: Can we do anything to have inline functions that are exported by a shared library??
#endif
#endif

#ifndef PIG_CORE_BUILDING_AS_DLL
#define PIG_CORE_BUILDING_AS_DLL 0
#endif

//NOTE: PEXP stands for "PigCore Export", PEXPI stands for PEXP + PIG_CORE_INLINE
#if PIG_CORE_BUILDING_AS_DLL
	#if COMPILER_IS_MSVC
	#define PEXP __declspec(dllexport)
	#define PEXPI __declspec(dllexport) PIG_CORE_INLINE
	#elif COMPILER_IS_CLANG
	#define PEXP __attribute__((visibility("default")))
	#define PEXPI __attribute__((visibility("default"))) PIG_CORE_INLINE
	#else
	#error This compiler doesn't have an implementation set for PEXP and PEXPI
	#endif
#else
#define PEXP  //nothing
#define PEXPI /*nothing*/  PIG_CORE_INLINE
#endif

// +--------------------------------------------------------------+
// |                  tests build_config Options                  |
// +--------------------------------------------------------------+

#ifndef USE_EMSCRIPTEN
#define USE_EMSCRIPTEN 0
#endif

#ifndef BUILD_WITH_RAYLIB
#define BUILD_WITH_RAYLIB 0
#endif

#ifndef BUILD_WITH_BOX2D
#define BUILD_WITH_BOX2D 0
#endif

#ifndef BUILD_WITH_SOKOL_GFX
#define BUILD_WITH_SOKOL_GFX 0
#endif

#ifndef BUILD_WITH_SOKOL_APP
#define BUILD_WITH_SOKOL_APP 0
#endif

#ifndef BUILD_WITH_SDL
#define BUILD_WITH_SDL 0
#endif

#endif //  _BASE_DEFINES_CHECK_H
