/*
File:   shader_include.h
Author: Taylor Robbins
Date:   01\29\2025
Description:
	** This file gets included by all the generated shader .c source files
*/

#ifndef _SHADER_INCLUDE_H
#define _SHADER_INCLUDE_H

#include "build_config.h"

#ifdef BUILD_WITH_SOKOL
#undef BUILD_WITH_SOKOL
#endif
#define BUILD_WITH_SOKOL 1
#define PIG_CORE_IMPLEMENTATION 0
#include "misc/misc_sokol_gfx_include.h"
#include "struct/struct_vectors.h"
#include "struct/struct_matrices.h"

#define SOKOL_SHDC_IMPL

#endif //  _SHADER_INCLUDE_H
