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
#include "base/base_compiler_check.h"

//NOTE: Copy of backend descision from gfx_sokol_include.h
#if TARGET_IS_WINDOWS
#define SOKOL_D3D11
// #define SOKOL_GLCORE
#elif TARGET_IS_LINUX
#define SOKOL_GLCORE
#elif TARGET_IS_OSX
#define SOKOL_METAL
#elif TARGET_IS_WEB
#define SOKOL_WGPU
#endif

#include "third_party/sokol/sokol_gfx.h"

#define PIG_CORE_IMPLEMENTATION 0
#include "struct/struct_vectors.h"
#include "struct/struct_matrices.h"

#define SOKOL_SHDC_IMPL

#endif //  _SHADER_INCLUDE_H
