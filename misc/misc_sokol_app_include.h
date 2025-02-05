/*
File:   misc_sokol_app_include.h
Author: Taylor Robbins
Date:   02\03\2025
Description:
	** If sokol_app.h has not been included by the application before including
	** something like input_sokol.h then this file serves as the include site
	** for the HEADER ONLY version of sokol_app.h (i.e. we can only use types, not function calls)
*/

#ifndef _MISC_SOKOL_APP_INCLUDE_H
#define _MISC_SOKOL_APP_INCLUDE_H

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

#endif //  _MISC_SOKOL_APP_INCLUDE_H
