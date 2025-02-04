/*
File:   input_sokol_include.h
Author: Taylor Robbins
Date:   02\03\2025
Description:
	** If sokol_app.h has not been included by the application before including
	** something like input_sokol.h then this file serves as the include site
	** for the HEADER ONLY version of sokol_app.h (i.e. we can only use types, not function calls)
*/

#ifndef _INPUT_SOKOL_INCLUDE_H
#define _INPUT_SOKOL_INCLUDE_H

#include "build_config.h"
#include "base/base_defines_check.h"
#include "base/base_assert.h"

#if BUILD_WITH_SOKOL && !defined(SOKOL_APP_INCLUDED)

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

//NOTE: We are NOT defining SOKOL_APP_IMPL here!
#if TARGET_IS_LINUX
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers" //warning: missing field 'revents' initializer [-Wmissing-field-initializers]
#endif
#include "third_party/sokol/sokol_app.h"
#if TARGET_IS_LINUX
#pragma clang diagnostic pop
#endif

#endif //BUILD_WITH_SOKOL

#endif //  _INPUT_SOKOL_INCLUDE_H
