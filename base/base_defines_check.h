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

// When linking with PigCore as a dll, set this define to 0 before including any of the PigCore
// headers to prevent the implementations from being compiled into your compilation unit
#ifndef PIG_CORE_IMPLEMENTATION
#define PIG_CORE_IMPLEMENTATION 1
#endif

// Any function in PigCore that want's to be inlined can have their behavior changed by changing this define
#ifndef PIG_CORE_INLINE
#define PIG_CORE_INLINE inline
#endif

#ifndef PIG_CORE_BUILDING_AS_DLL
#define PIG_CORE_BUILDING_AS_DLL 0
#endif

//NOTE: PEXP stands for "PigCore Export", PEXPI stands for PEXP + PIG_CORE_INLINE
#if (PIG_CORE_BUILDING_AS_DLL && COMPILER_IS_MSVC)
#define PEXP __declspec(dllexport)
#define PEXPI __declspec(dllexport) PIG_CORE_INLINE
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

#ifndef BUILD_WITH_SOKOL
#define BUILD_WITH_SOKOL 0
#endif

#ifndef BUILD_WITH_SDL
#define BUILD_WITH_SDL 0
#endif

#endif //  _BASE_DEFINES_CHECK_H
