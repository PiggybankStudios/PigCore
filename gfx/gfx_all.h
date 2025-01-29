/*
File:   gfx_all.h
Author: Taylor Robbins
Date:   01\19\2025
*/

#ifndef _GFX_ALL_H
#define _GFX_ALL_H

#include "base/base_defines_check.h" //required by all the other files
#include "base/base_typedefs.h" //required by gfx_image_loading.h and gfx_vectors.h
#include "struct/struct_vectors.h" //required by gfx_image_loading.h and gfx_vectors.h

#include "gfx/gfx_vertices.h"

#include "mem/mem_arena.h" //required by gfx_shader.h
#include "struct/struct_string.h" //required by gfx_shader.h
#include "misc/misc_result.h" //required by gfx_shader.h and gfx_image_loading.h

#include "gfx/gfx_shader.h"

#include "mem/mem_scratch.h" //required by gfx_image_loading.h

#include "gfx/gfx_image_loading.h"

#endif //  _GFX_ALL_H
