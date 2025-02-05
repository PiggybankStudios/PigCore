/*
File:   misc_sokol_gfx_include.h
Author: Taylor Robbins
Date:   01\28\2025
Description:
	** If sokol_gfx.h has not been included by the application before including
	** something like gfx_shader.h then this file serves as the include site
*/

#ifndef _MISC_SOKOL_GFX_INCLUDE_H
#define _MISC_SOKOL_GFX_INCLUDE_H

#include "build_config.h"
#include "base/base_defines_check.h"
#include "base/base_assert.h"

#if BUILD_WITH_SOKOL && !defined(SOKOL_GFX_INCLUDED)

#ifndef SOKOL_ASSERT
#define SOKOL_ASSERT(condition) Assert(condition)
#endif

#ifndef SOKOL_UNREACHABLE
#define SOKOL_UNREACHABLE Assert(false)
#endif

#if DEBUG_BUILD && !defined(SOKOL_DEBUG)
#define SOKOL_DEBUG
#endif

//NOTE: Change this over in shader_include.h as well!
#if TARGET_IS_WINDOWS
	#if 1
	#ifndef SOKOL_D3D11
	#define SOKOL_D3D11
	#endif
	#else
	#ifndef SOKOL_GLCORE
	#define SOKOL_GLCORE
	#endif
	#endif
#elif TARGET_IS_LINUX
	#ifndef SOKOL_GLCORE
	#define SOKOL_GLCORE
	#endif
#elif TARGET_IS_OSX
	#ifndef SOKOL_METAL
	#define SOKOL_METAL
	#endif
#elif TARGET_IS_WEB
	#ifndef SOKOL_WGPU
	#define SOKOL_WGPU
	#endif
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

#endif //  _MISC_SOKOL_GFX_INCLUDE_H
