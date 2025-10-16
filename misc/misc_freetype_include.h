/*
File:   misc_freetype_include.h
Author: Taylor Robbins
Date:   10\15\2025
*/

#ifndef _MISC_FREETYPE_INCLUDE_H
#define _MISC_FREETYPE_INCLUDE_H

#include "base/base_defines_check.h"

#if BUILD_WITH_FREETYPE

#define FT2_BUILD_LIBRARY
#define FT_CONFIG_MODULES_H "misc/misc_freetype_modules.h"
#define FT_CONFIG_OPTIONS_H "misc/misc_freetype_options.h"
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES

#if !PIG_CORE_IMPLEMENTATION

#include <freetype/freetype.h>

#else

#ifdef len
#undef len
#endif

#if COMPILER_IS_MSVC
#pragma warning(push)
#pragma warning(disable:4244) //'function': conversion from '__int64' to 'const unsigned int', possible loss of data
#pragma warning(disable:4267) //'+=': conversion from 'size_t' to 'TCoord', possible loss of data
#pragma warning(disable:5262) //implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases
#endif

#include "third_party/freetype/src/base/ftsystem.c"
#include "third_party/freetype/src/base/ftinit.c"
#include "third_party/freetype/src/base/ftmm.c"
#include "third_party/freetype/src/base/ftdebug.c"
#include "third_party/freetype/src/base/ftbase.c"
#include "third_party/freetype/src/base/ftbbox.c"
#include "third_party/freetype/src/base/ftglyph.c"
#include "third_party/freetype/src/base/ftbitmap.c"
#include "third_party/freetype/src/sfnt/sfnt.c"
#include "third_party/freetype/src/truetype/truetype.c"
#include "third_party/freetype/src/smooth/smooth.c"
#include "third_party/freetype/src/psnames/psnames.c"
#include "third_party/freetype/src/gzip/ftgzip.c"

#if COMPILER_IS_MSVC
#pragma warning(pop)
#endif

#endif //PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                        Helper Macros                         |
// +--------------------------------------------------------------+

// Convert to/from Fixed Point 26.6 type
#define TO_FT26_FROM_I32(integerValue)    (FT_F26Dot6)(((signed long)(integerValue)) << 6)
#define TO_FT26_FROM_R32(floatValue)      (FT_F26Dot6)((floatValue) * 64.0f)
#define TO_FT26_FROM_R64(floatValue)      (FT_F26Dot6)((floatValue) * 64.0)
#define TO_I32_FROM_FT26(fixedPointValue) (i32)((fixedPointValue) >> 6)
#define TO_R32_FROM_FT26(fixedPointValue) (r32)(((r32)(fixedPointValue)) / 64.0f)
#define TO_R64_FROM_FT26(fixedPointValue) (r64)(((r64)(fixedPointValue)) / 64.0)

#endif //BUILD_WITH_FREETYPE

#endif //  _MISC_FREETYPE_INCLUDE_H
