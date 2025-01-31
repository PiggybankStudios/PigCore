/*
File:   gfx_sokol_include.h
Author: Taylor Robbins
Date:   01\28\2025
Description:
	** If sokol_gfx.h and\or sokol_app.h are not included by the application before
	** including something like gfx_shader.h then this file serves as the 
*/

#ifndef _GFX_SOKOL_INCLUDE_H
#define _GFX_SOKOL_INCLUDE_H

#include "build_config.h"
#include "base/base_defines_check.h"
#include "base/base_assert.h"

#if BUILD_WITH_SOKOL && (!defined(SOKOL_GFX_INCLUDED) || !defined(SOKOL_APP_INCLUDED))

#define SOKOL_ASSERT(condition) Assert(condition)
#define SOKOL_UNREACHABLE Assert(false)
#if DEBUG_BUILD
#define SOKOL_DEBUG
#endif

//NOTE: Change this over in shader_include.h as well!
#if TARGET_IS_WINDOWS
#define SOKOL_D3D11
// #define SOKOL_GLCORE
#elif TARGET_IS_LINUX
#define SOKOL_GLCORE
#elif TARGET_IS_OSX
#define SOKOL_METAL
#elif TARGET_IS_WEB
#define SOKOL_WGPU
#endif

#if PIG_CORE_IMPLEMENTATION
#define SOKOL_GFX_IMPL
#endif
#if TARGET_IS_WASM
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough" //warning: unannotated fall-through between switch labels [-Wimplicit-fallthrough]
#endif
#include "third_party/sokol/sokol_gfx.h"
#if TARGET_IS_WASM
#pragma clang diagnostic pop
#endif

#endif //BUILD_WITH_SOKOL

#endif //  _GFX_SOKOL_INCLUDE_H
