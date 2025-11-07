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

#ifndef BUILD_WITH_SOKOL_GFX
#define BUILD_WITH_SOKOL_GFX 1
#endif
#if !BUILD_WITH_SOKOL_GFX
#error You cannot compile sokol-shdc compiled shaders without BUILD_WITH_SOKOL_GFX
#endif

#define PIG_CORE_IMPLEMENTATION 0
#include "lib/lib_sokol_gfx.h"
#include "struct/struct_vectors.h"
#include "struct/struct_matrices.h"

#define SOKOL_SHDC_IMPL

#endif //  _SHADER_INCLUDE_H
