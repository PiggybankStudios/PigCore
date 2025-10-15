/*
File:   misc_freetype_include.h
Author: Taylor Robbins
Date:   10\15\2025
*/

#ifndef _MISC_FREETYPE_INCLUDE_H
#define _MISC_FREETYPE_INCLUDE_H

#include "base/base_defines_check.h"

#if BUILD_WITH_FREETYPE

#if !PIG_CORE_IMPLEMENTATION

// #include "freetype.h"

#else

#define FT2_BUILD_LIBRARY

#include "third_party/freetype/src/base/ftsystem.c"
#include "third_party/freetype/src/base/ftinit.c"
#include "third_party/freetype/src/base/ftdebug.c"
#include "third_party/freetype/src/base/ftbase.c"
#include "third_party/freetype/src/base/ftbbox.c"
#include "third_party/freetype/src/base/ftglyph.c"
#include "third_party/freetype/src/base/ftbitmap.c"
#include "third_party/freetype/src/sfnt/sfnt.c"
#include "third_party/freetype/src/truetype/truetype.c"
#include "third_party/freetype/src/smooth/smooth.c"
#include "third_party/freetype/src/psnames/psnames.c"

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_FREETYPE

#endif //  _MISC_FREETYPE_INCLUDE_H
