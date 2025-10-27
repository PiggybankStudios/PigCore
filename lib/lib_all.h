/*
File:   lib_all.h
Author: Taylor Robbins
Date:   10\27\2025
*/

#ifndef _LIB_ALL_H
#define _LIB_ALL_H

#include "base/base_defines_check.h" //required by all the other files

#include "lib/lib_tracy.h"

#include "base/base_compiler_check.h" //required by most files below
#include "base/base_typedefs.h" //required by most files below
#include "base/base_macros.h" //required by most files below
#include "base/base_assert.h" //required by most files below

#include "lib/lib_sokol_gfx.h"
#include "lib/lib_sokol_app.h"

#include "std/std_basic_math.h" //required by lib_handmade_math.h
#include "std/std_trig.h" //required by lib_handmade_math.h

#include "lib/lib_handmade_math.h"

#include "std/std_malloc.h" //required by lib_stb_image.h and lib_stb_image_write.h
#include "std/std_memset.h" //required by lib_stb_image_write.h and lib_metadesk.h

#include "lib/lib_stb_image.h" //required by lib_freetype.h
#include "lib/lib_stb_image_write.h" //required by lib_freetype.h
#include "lib/lib_freetype.h"

#include "os/os_virtual_mem.h" //required by lib_metadesk.h
#include "os/os_file.h" //required by lib_metadesk.h
#include "struct/struct_string.h" //required by lib_metadesk.h

#include "lib/lib_metadesk.h"

#endif //  _LIB_ALL_H
