/*
File:   base_compiler_check.h
Author: Taylor Robbins
Date:   01\01\2025
Description:
	** Checks for various #defines to determine which compiler we are being compiled by.
	** Converts that information into a set of our own defines that are easier to remember\use
*/

#ifndef _BASE_COMPILER_CHECK_H
#define _BASE_COMPILER_CHECK_H

#include "build_config.h"
#include "base/base_plex_is_struct.h"

// +--------------------------------------------------------------+
// |                      Determine LANGUAGE                      |
// +--------------------------------------------------------------+
#ifdef __cplusplus
#define LANGUAGE_IS_C   0
#define LANGUAGE_IS_CPP 1
#else
#define LANGUAGE_IS_C   1
#define LANGUAGE_IS_CPP 0
#endif

// +--------------------------------------------------------------+
// |                      Determine COMPILER                      |
// +--------------------------------------------------------------+
//NOTE: Clang lies a bunch, it gives __GNUC__ and _MSC_VER defines even though it is neither! Thus we check it first and preclude the others if we find __clang__
#ifdef __EMSCRIPTEN__
#define COMPILER_IS_EMSCRIPTEN 1
#else
#define COMPILER_IS_EMSCRIPTEN 0
#endif

#if (defined(__clang__) && !COMPILER_IS_EMSCRIPTEN)
#define COMPILER_IS_CLANG 1
#else
#define COMPILER_IS_CLANG 0
#endif

#if (defined(__GNUC__) && !COMPILER_IS_CLANG && !COMPILER_IS_EMSCRIPTEN)
#define COMPILER_IS_GCC 1
#else
#define COMPILER_IS_GCC 0
#endif

//format=VVRR VV=version, RR=Revision, ex. v8.23=0823, LG_Laptop=v19.41.34120
#if (defined(_MSC_VER) && !COMPILER_IS_CLANG && !COMPILER_IS_EMSCRIPTEN)
#define COMPILER_IS_MSVC 1
#else
#define COMPILER_IS_MSVC 0
#endif

//Make sure we have exactly one of the COMPILER defines set to true
#if COMPILER_IS_EMSCRIPTEN + COMPILER_IS_MSVC + COMPILER_IS_CLANG + COMPILER_IS_GCC > 1
#error Failed to differentiate which compiler is running based on the pre-defined compiler flags!
#endif
#if COMPILER_IS_EMSCRIPTEN + COMPILER_IS_MSVC + COMPILER_IS_CLANG + COMPILER_IS_GCC < 1
#error We dont support the current compiler! Should be Clang, GCC, Emscripten, or MSVC
#endif

// +--------------------------------------------------------------+
// |                  Determine TARGET Platform                   |
// +--------------------------------------------------------------+
#if ((defined(__linux__) || defined(__unix__)) && !COMPILER_IS_EMSCRIPTEN)
#define TARGET_IS_LINUX 1
#else
#define TARGET_IS_LINUX 0
#endif

#if defined(TARGET_PLAYDATE) && TARGET_PLAYDATE
#define TARGET_IS_PLAYDATE_SIMULATOR 0
#define TARGET_IS_PLAYDATE_DEVICE 1
#elif defined(TARGET_SIMULATOR) && TARGET_SIMULATOR
#define TARGET_IS_PLAYDATE_SIMULATOR 1
#define TARGET_IS_PLAYDATE_DEVICE 0
#else
#define TARGET_IS_PLAYDATE_SIMULATOR 0
#define TARGET_IS_PLAYDATE_DEVICE 0
#endif

#if defined(_WIN32) && !TARGET_IS_PLAYDATE_SIMULATOR
#define TARGET_IS_WINDOWS 1
#else
#define TARGET_IS_WINDOWS 0
#endif

#ifdef __APPLE__
#define TARGET_IS_OSX 1
#else
#define TARGET_IS_OSX 0
#endif

#ifdef __ORCA__
#define TARGET_IS_ORCA 1
#else
#define TARGET_IS_ORCA 0
#endif

#if defined(__wasm32__) && !TARGET_IS_ORCA
#define TARGET_IS_WEB 1
#else
#define TARGET_IS_WEB 0
#endif

//Make sure we have exactly one of the TARGET defines set to true
#if TARGET_IS_LINUX + TARGET_IS_WINDOWS + TARGET_IS_OSX + TARGET_IS_PLAYDATE_DEVICE + TARGET_IS_PLAYDATE_SIMULATOR + TARGET_IS_WEB + TARGET_IS_ORCA > 1
#error Somehow we think we are on more than one TARGET based on the pre-defined compiler flags!
#endif
#if TARGET_IS_LINUX + TARGET_IS_WINDOWS + TARGET_IS_OSX + TARGET_IS_PLAYDATE_DEVICE + TARGET_IS_PLAYDATE_SIMULATOR + TARGET_IS_WEB + TARGET_IS_ORCA < 1
#error Failed to identify which TARGET we are compiling for based on the pre-defined compiler flags!
#endif

// +--------------------------------------------------------------+
// |                    Derived TARGET Aspects                    |
// +--------------------------------------------------------------+
#if (TARGET_IS_PLAYDATE_DEVICE || TARGET_IS_PLAYDATE_SIMULATOR)
#define TARGET_IS_PLAYDATE 1
#else
#define TARGET_IS_PLAYDATE 0
#endif

#if (TARGET_IS_WEB || TARGET_IS_ORCA)
#define TARGET_IS_WASM 1
#else
#define TARGET_IS_WASM 0
#endif

//TODO: Is there a more robust way to determine whether we are compiling 32-bit or 64-bit?
#if (TARGET_IS_WASM || TARGET_IS_PLAYDATE_DEVICE)
#define TARGET_IS_32BIT 1
#define TARGET_IS_64BIT 0
#else
#define TARGET_IS_32BIT 0
#define TARGET_IS_64BIT 1
#endif

#if (TARGET_IS_WASM || TARGET_IS_PLAYDATE)
#define TARGET_HAS_OFFICIAL_STDLIB 0
#else
#define TARGET_HAS_OFFICIAL_STDLIB 1
#endif

#if (TARGET_IS_WEB && !COMPILER_IS_EMSCRIPTEN)
#define USING_CUSTOM_STDLIB 1
#else
#define USING_CUSTOM_STDLIB 0
#endif

// +--------------------------------------------------------------+
// |                      Extern C Handling                       |
// +--------------------------------------------------------------+
//NOTE: winnt.h already defines EXTERN_C on Windows
#if !TARGET_IS_WINDOWS
#define EXTERN_C  extern "C"
#endif

// Hiding the curly brackets inside a macro like this helps keep text editors from indenting
// all our top-level elements in a file while still wrapping the entire file in extern "C"
// NOTE: EXTERN_C_START and EXTERN_C_END are already defined by winnt.h (included by windows.h) so we had to change the name of our version slightly
#define START_EXTERN_C extern "C" {
#define END_EXTERN_C   }

#if LANGUAGE_IS_CPP
#define MAYBE_EXTERN_C       EXTERN_C
#define MAYBE_START_EXTERN_C START_EXTERN_C
#define MAYBE_END_EXTERN_C   END_EXTERN_C
#else
#define MAYBE_EXTERN_C       //nothing
#define MAYBE_START_EXTERN_C //nothing
#define MAYBE_END_EXTERN_C   //nothing
#endif

// +--------------------------------------------------------------+
// |                        Other Defines                         |
// +--------------------------------------------------------------+
#ifndef DEBUG_BUILD
#define DEBUG_BUILD 0
#endif

#if DEBUG_BUILD
	#if !defined(_DEBUG)
	#define _DEBUG
	#endif
	#if defined(NDEBUG)
	#undef NDEBUG
	#endif
#else
	#if defined(_DEBUG)
	#undef _DEBUG
	#endif
	#if !defined(NDEBUG)
	#define NDEBUG
	#endif
#endif

#endif //  _BASE_COMPILER_CHECK_H
