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

#ifndef ASSERTIONS_ENABLED
#define ASSERTIONS_ENABLED DEBUG_BUILD
#endif

#ifndef PIG_CORE_BUILDING_AS_DLL
#define PIG_CORE_BUILDING_AS_DLL 0
#endif

#ifndef BUILD_FOR_PIGGEN
#define BUILD_FOR_PIGGEN 0
#endif

// This is not ever set for a real compilation, but is set by compile_commands.json so that the LSP (like clangd) will have this define set when it does it's parsing/indexing of the codebase
// There are some language features or includes that we want to suppress because clangd doesn't like or understand them. Thus we put them inside a #if !COMPILER_IS_LSP section
#ifndef COMPILER_IS_LSP
#define COMPILER_IS_LSP 0
#endif

//NOTE: PEXP stands for "PigCore Export", PEXPI stands for PEXP + PIG_CORE_INLINE
#if PIG_CORE_BUILDING_AS_DLL
	#if COMPILER_IS_MSVC
	#define PEXP __declspec(dllexport)
	#define PEXPI __declspec(dllexport) PIG_CORE_INLINE
	#elif COMPILER_IS_CLANG || IS_COMPILER_GCC
	#define PEXP __attribute__((visibility("default")))
	#define PEXPI __attribute__((visibility("default"))) PIG_CORE_INLINE
	#else
	#error This compiler doesnt have an implementation set for PEXP and PEXPI
	#endif
#else
#define PEXP  //nothing
#define PEXPI /*nothing*/  PIG_CORE_INLINE
#endif

// +--------------------------------------------------------------+
// |                        PIGGEN Macros                         |
// +--------------------------------------------------------------+
#ifndef PIGGEN
#define PIGGEN 0
#endif
#if (PIGGEN)
#error PIGGEN was set to a true value! This will cause all the PIGGEN blocks to get compiled as C code which is not intended!
#endif
#ifndef PIGGEN_TAG
#define PIGGEN_TAG(...) //nothing
#endif
#ifndef PIGGEN_INLINE
#define PIGGEN_INLINE(...) //nothing
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

#ifndef BUILD_WITH_CLAY
#define BUILD_WITH_CLAY 0
#endif

#ifndef BUILD_WITH_IMGUI
#define BUILD_WITH_IMGUI 0
#endif

#ifndef BUILD_WITH_ODE
#define BUILD_WITH_ODE 0
#endif

#ifndef BUILD_WITH_PHYSX
#define BUILD_WITH_PHYSX 0
#endif

#ifndef BUILD_WITH_METADESK
#define BUILD_WITH_METADESK 0
#endif

#ifndef PREFER_OPENGL_OVER_D3D11
#define PREFER_OPENGL_OVER_D3D11 0
#endif

#ifndef BUILD_WITH_HTTP
#define BUILD_WITH_HTTP 0
#endif

#ifndef BUILD_WITH_PROTOBUF
#define BUILD_WITH_PROTOBUF 0
#endif

#ifndef BUILD_WITH_FREETYPE
#define BUILD_WITH_FREETYPE 0
#endif

#ifndef BUILD_WITH_GTK
#define BUILD_WITH_GTK 0
#endif

#ifndef BUILD_WITH_PIG_UI
#define BUILD_WITH_PIG_UI 0
#endif

// +--------------------------------------------------------------+
// |                       Check Conflicts                        |
// +--------------------------------------------------------------+
#if BUILD_WITH_RAYLIB && (BUILD_WITH_SOKOL_APP || BUILD_WITH_SOKOL_GFX)
#error Raylib and Sokol are not meant to be used together!
#endif
#if BUILD_WITH_RAYLIB && BUILD_WITH_SDL
#error Raylib and SDL are not meant to be used together!
#endif
#if BUILD_WITH_SDL && (BUILD_WITH_SOKOL_APP || BUILD_WITH_SOKOL_GFX)
#error SDL and Sokol are not meant to be used together!
#endif
#if BUILD_WITH_PHYSX && BUILD_WITH_ODE
#error PhysX and ODE are not meant to be used together!
#endif
#if BUILD_WITH_PIG_UI && (!BUILD_WITH_SOKOL_APP || !BUILD_WITH_SOKOL_GFX)
#error BUILD_WITH_PIG_UI can only be used with sokol_gfx.h + sokol_app.h right now!
#endif

#endif //  _BASE_DEFINES_CHECK_H
