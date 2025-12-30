/*
File:   lib_sokol_app.h
Author: Taylor Robbins
Date:   02\03\2025
Description:
	** If sokol_app.h has not been included by the application before including
	** something like input_sokol.h then this file serves as the include site
	** for the HEADER ONLY version of sokol_app.h (i.e. we can only use types, not function calls)
*/

#ifndef _LIB_SOKOL_APP_H
#define _LIB_SOKOL_APP_H

#include "build_config.h"
#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "base/base_assert.h"

#if BUILD_WITH_SOKOL_APP && !defined(SOKOL_APP_INCLUDED)

#ifndef SOKOL_ASSERT
#define SOKOL_ASSERT(condition) Assert(condition)
#endif

#ifndef SOKOL_UNREACHABLE
#define SOKOL_UNREACHABLE Assert(false)
#endif

#if DEBUG_BUILD && !defined(SOKOL_DEBUG)
#define SOKOL_DEBUG
#endif

//NOTE: We are NOT defining SOKOL_APP_IMPL here!
#if COMPILER_IS_MSVC
#pragma warning(push)
#pragma warning(disable: 4189) //warning: local variable is initialized but not referenced
#endif
#if COMPILER_IS_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers" //warning: missing field 'revents' initializer [-Wmissing-field-initializers]
#pragma clang diagnostic ignored "-Wdeprecated-declarations" //warning: 'NSOpenGLView' is deprecated: first deprecated in macOS 10.14 - Please use MTKView instead.
#pragma clang diagnostic ignored "-Wunused-variable" //warning: unused variable 'frameResult'
#endif

#include "third_party/sokol/sokol_app.h"

#if COMPILER_IS_MSVC
#pragma warning(pop)
#endif
#if COMPILER_IS_CLANG
#pragma clang diagnostic pop
#endif

#endif //BUILD_WITH_SOKOL_APP

#endif //  _LIB_SOKOL_APP_H
