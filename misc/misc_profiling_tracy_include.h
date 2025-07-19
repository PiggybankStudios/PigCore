/*
File:   misc_profiling_tracy_include.h
Author: Taylor Robbins
Date:   07\18\2025
*/

#ifndef _MISC_PROFILING_TRACY_INCLUDE_H
#define _MISC_PROFILING_TRACY_INCLUDE_H

#include "base/base_defines_check.h"

#if BUILD_WITH_TRACY

#define ENABLE_TRACY
#if PIG_CORE_BUILDING_AS_DLL
#define TRACY_EXPORTS
#endif
#include "third_party/tracy/tracy/TracyC.h"

#endif //BUILD_WITH_TRACY

#endif //  _MISC_PROFILING_TRACY_INCLUDE_H
