/*
File:   struct_all.h
Author: Taylor Robbins
Date:   01\04\2025
Description:
	** Includes all files in this folder, meant as a quick shorthand for applications that just want everything from this folder available
	** This file also serves as a reference for the order of dependencies between files in this folder.
	** This file also includes files from other folders like mem and std because they are required for some of the files in struct
*/

#ifndef _STRUCT_ALL_H
#define _STRUCT_ALL_H

#include "base/base_defines_check.h" //Needed by all other files
#include "base/base_typedefs.h" //Needed by all other files

#include "struct/struct_color.h"

#include "std/std_basic_math.h" //Needed by struct_directions.h and struct_ranges.h

#include "struct/struct_directions.h"

#include "base/base_macros.h" //Needed by many of the files below
#include "base/base_assert.h" //Needed by many of the files below

#include "struct/struct_ranges.h"
#include "struct/struct_typed_array.h"

#include "base/base_char.h" //Needed by struct_string.h
#include "std/std_memset.h" //Needed by struct_string.h and struct_string_buffer.h

#include "struct/struct_string.h"
#include "struct/struct_rich_string.h"
#include "struct/struct_string_buffer.h"

#include "mem/mem_arena.h" //Needed by many of the files below
#include "std/std_memset.h" //Needed by many of the files below

#include "struct/struct_var_array.h"

#include "std/std_trig.h" //Needed by many of the files below

#include "struct/struct_handmade_math_include.h" //Needed by struct_vectors.h, struct_quaternion.h, and struct_matrices.h

#include "struct/struct_vectors.h"
#include "struct/struct_quaternion.h"
#include "struct/struct_rectangles.h"
#include "struct/struct_matrices.h"
#include "struct/struct_lines.h"
#include "struct/struct_circles.h"
#include "struct/struct_faces.h"
#include "struct/struct_image_data.h"

#include "gfx/gfx_vertices.h" //Needed by struct_model_data.h

#include "struct/struct_model_data.h"

#endif //  _STRUCT_ALL_H
