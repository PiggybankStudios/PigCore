/*
File:   gfx_all.h
Author: Taylor Robbins
Date:   01\19\2025
*/

#ifndef _GFX_ALL_H
#define _GFX_ALL_H

#include "base/base_defines_check.h" //required by all the other files
#include "base/base_typedefs.h" //required by all other files
#include "base/base_assert.h" //required by most other files
#include "struct/struct_vectors.h" //required by all other files

#include "gfx/gfx_vertices.h"

#include "struct/struct_circles.h" //required by gfx_mesh_generation.h
#include "mem/mem_arena.h" //required by gfx_mesh_generation.h, gfx_shader.h, gfx_vert_buffer.h, gfx_font.h and gfx_pipeline.h

#include "gfx/gfx_mesh_generation.h"

#include "struct/struct_string.h" //required by gfx_shader.h, gfx_vert_buffer.h, gfx_pipeline.h, gfx_font.h and gfx_font_flow.h
#include "misc/misc_result.h" //required by gfx_shader.h, gfx_vert_buffer.h, gfx_image_loading.h and gfx_pipeline.h
#include "misc/misc_sokol_gfx_include.h" //required by gfx_error.h, gfx_shader.h, gfx_vert_buffer.h, gfx_pipeline.h and gfx_system.h

#include "gfx/gfx_error.h"
#include "gfx/gfx_vert_buffer.h"

#include "mem/mem_scratch.h" //required by gfx_texture.h, gfx_shader.h, gfx_image_loading.h, gfx_font.h and gfx_pipeline.h

#include "gfx/gfx_texture.h"
#include "gfx/gfx_shader.h"
#include "gfx/gfx_image_loading.h"
#include "gfx/gfx_pipeline.h"

#include "struct/struct_var_array.h" //required by gfx_font.h, gfx_system.h and gfx_clay_renderer.h
#include "std/std_basic_math.h" //required by gfx_font.h
#include "std/std_memset.h" //required by gfx_font.h
#include "base/base_unicode.h" //required by gfx_font.h and gfx_font_flow.h

#include "gfx/gfx_font.h"

#include "struct/struct_rectangles.h" //required by gfx_font_flow.h
#include "struct/struct_color.h" //required by gfx_font_flow.h and gfx_system.h

#include "gfx/gfx_font_flow.h"
#include "gfx/gfx_system.h"

#include "ui/ui_clay.h" //required by gfx_clay_renderer.h

#include "gfx/gfx_clay_renderer.h"

#endif //  _GFX_ALL_H
