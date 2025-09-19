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

#if BUILD_WITH_SOKOL_GFX && !defined(SOKOL_GFX_INCLUDED)

#ifndef SOKOL_ASSERT
#define SOKOL_ASSERT(condition) Assert(condition)
#endif

#ifndef SOKOL_UNREACHABLE
#define SOKOL_UNREACHABLE Assert(false)
#endif

#if DEBUG_BUILD && !defined(SOKOL_DEBUG)
#define SOKOL_DEBUG
#endif

#if !defined(SOKOL_D3D11) && !defined(SOKOL_GLCORE) && !defined(SOKOL_GLES3) && !defined(SOKOL_METAL) && !defined(SOKOL_WGPU) && !defined(SOKOL_DUMMY_BACKEND)
#if TARGET_IS_WINDOWS
	#if PREFER_OPENGL_OVER_D3D11
	#define SOKOL_GLCORE
	#else
	#define SOKOL_D3D11
	#endif
#elif TARGET_IS_LINUX
	#define SOKOL_GLCORE
#elif TARGET_IS_OSX
	#define SOKOL_METAL
#elif TARGET_IS_WEB
	#define SOKOL_WGPU
#elif TARGET_IS_ANDROID
	#define SOKOL_GLES3
#endif
#endif //!defined(SOKOL_D3D11) && !defined(SOKOL_GLCORE) && !defined(SOKOL_GLES3) && !defined(SOKOL_METAL) && !defined(SOKOL_WGPU) && !defined(SOKOL_DUMMY_BACKEND)

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

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _MISC_SOKOL_GFX_INCLUDE_H

#if defined(_MISC_SOKOL_GFX_INCLUDE_H) && defined(_STRUCT_MATRICES_H)
#include "cross/cross_sokol_gfx_and_matrices.h"
#endif
