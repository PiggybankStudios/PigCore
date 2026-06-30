/*
File:   lib_tracy.h
Author: Taylor Robbins
Date:   07\18\2025
*/

#ifndef _LIB_TRACY_H
#define _LIB_TRACY_H

#include "base/base_defines_check.h"

#if PROFILING_ENABLED
#define TRACY_ENABLE
#endif

#if PIG_CORE_BUILDING_AS_DLL
#define TRACY_EXPORTS
#endif

#include "third_party/tracy/tracy/TracyC.h"

#endif //  _LIB_TRACY_H
