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

// +--------------------------------------------------------------+
// |                      Determine COMPILER                      |
// +--------------------------------------------------------------+
//NOTE: Clang lies a bunch, it gives __GNUC__ and _MSC_VER defines even though it is neither! Thus we check it first and preclude the others if we find __clang__
#ifdef __clang__
#define COMPILER_IS_CLANG 1
#else
#define COMPILER_IS_CLANG 0
#endif

#if defined(__GNUC__) && !COMPILER_IS_CLANG
#define COMPILER_IS_GCC 1
#else
#define COMPILER_IS_GCC 0
#endif

//format=VVRR VV=version, RR=Revision, ex. v8.23=0823, LG_Laptop=v19.41.34120
#if defined(_MSC_VER) && !COMPILER_IS_CLANG
#define COMPILER_IS_MSVC 1
#else
#define COMPILER_IS_MSVC 0
#endif

//Make sure we have exactly one of the COMPILER defines set to true
#if COMPILER_IS_MSVC + COMPILER_IS_CLANG + COMPILER_IS_GCC > 1
#error Failed to differentiate which compiler is running based on the pre-defined compiler flags!
#endif
#if COMPILER_IS_MSVC + COMPILER_IS_CLANG + COMPILER_IS_GCC < 1
#error We dont support the current compiler! Should be Clang, GCC, or MSVC
#endif

// +--------------------------------------------------------------+
// |                  Determine TARGET Platform                   |
// +--------------------------------------------------------------+
#if (defined(__linux__) || defined(__unix__))
#define TARGET_IS_LINUX 1
#else
#define TARGET_IS_LINUX 0
#endif

#ifdef _WIN32
#define TARGET_IS_WINDOWS 1
#else
#define TARGET_IS_WINDOWS 0
#endif

#ifdef __APPLE__
#define TARGET_IS_OSX 1
#else
#define TARGET_IS_OSX 0
#endif

//TODO: Detect if we are compiling for PLAYDATE!
#define TARGET_IS_PLAYDATE_SIMULATOR 0
#define TARGET_IS_PLAYDATE_DEVICE 0
//TODO: Detect if we are compiling for WebAssembly!
#define TARGET_IS_WEB 0
//TODO: Detect if we are compiling for Orca!
#define TARGET_IS_ORCA 0

//Make sure we have exactly one of the TARGET defines set to true
#if TARGET_IS_LINUX + TARGET_IS_WINDOWS + TARGET_IS_OSX + TARGET_IS_PLAYDATE + TARGET_IS_PLAYDATE_SIMULATOR + TARGET_IS_WEB + TARGET_IS_ORCA > 1
#error Somehow we think we are on more than one TARGET based on the pre-defined compiler flags!
#endif
#if TARGET_IS_LINUX + TARGET_IS_WINDOWS + TARGET_IS_OSX + TARGET_IS_PLAYDATE + TARGET_IS_PLAYDATE_SIMULATOR + TARGET_IS_WEB + TARGET_IS_ORCA < 1
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
#define TARGET_IS_32BIT 0
#else
#define TARGET_IS_64BIT 1
#endif

#if (TARGET_IS_WASM || TARGET_IS_PLAYDATE)
#define TARGET_HAS_OFFICIAL_STDLIB 0
#else
#define TARGET_HAS_OFFICIAL_STDLIB 1
#endif

#endif //  _BASE_COMPILER_CHECK_H
