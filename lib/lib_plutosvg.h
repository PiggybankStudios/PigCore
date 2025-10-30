/*
File:   lib_plutosvg.h
Author: Taylor Robbins
Date:   10\29\2025
*/

#ifndef _LIB_PLUTOSVG_H
#define _LIB_PLUTOSVG_H

//TODO: We need to go through Pluto SVG source code and replace all the direct calls to malloc/realloc/free with our aliases (or maybe with Arena API usages?)

#include "base/base_defines_check.h"
#include "lib/lib_stb_image.h" // Used in plutovg-surface.c
#include "lib/lib_stb_image_write.h" // Used in plutovg-surface.c
#include "lib/lib_stb_truetype.h" // Used in plutovg-font.c
#include "lib/lib_freetype.h" // Pluto SVG uses some types from FreeType itself

//TODO: Should we expose Pluto SVG functions when building as a DLL?
#if BUILD_WITH_FREETYPE

#define PLUTOVG_BUILD_STATIC
#define PLUTOSVG_BUILD_STATIC

#include "plutovg.h"
#include "plutosvg.h"
#include "plutosvg-ft.h"

#if PIG_CORE_IMPLEMENTATION

#if COMPILER_IS_MSVC
#pragma warning(push)
#pragma warning(disable:4244) //'function': conversion from '__int64' to 'int', possible loss of data
#pragma warning(disable:4267) //'function': conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable:5262) //implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases
#pragma warning(disable:4018) //'>': signed/unsigned mismatch
#pragma warning(disable:4459) //declaration of 'crc_table' hides global declaration
#pragma warning(disable:4100) //'npoints': unreferenced formal parameter
#endif

#include "plutovg-surface.c"
#include "plutovg-rasterize.c"
#include "plutovg-path.c"
#include "plutovg-paint.c"
#include "plutovg-matrix.c"
#include "plutovg-ft-raster.c"
// #include "plutovg-ft-stroker.c" //this is just a copy of FreeType's src\base\ftstroke.c
// #include "plutovg-ft-math.c" //this is just a copy of FreeType's src\base\fttrigon.c
#include "plutovg-font.c"
#include "plutovg-canvas.c"
#include "plutovg-blend.c"

#include "plutosvg.c"

#if COMPILER_IS_MSVC
#pragma warning(pop)
#endif

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_FREETYPE

#endif //  _LIB_PLUTOSVG_H
