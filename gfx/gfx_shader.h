/*
File:   gfx_shader.h
Author: Taylor Robbins
Date:   01\28\2025
Description:
	** Contains the Shader structure which holds a sg_shader from sokol_gfx as well
	** as other metadata about the shader when it was created.
	** NOTE: BUILD_WITH_SOKOL must be true and sokol_gfx.h must be included before this file
*/

#ifndef _GFX_SHADER_H
#define _GFX_SHADER_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "struct/struct_vectors.h"
#include "mem/mem_arena.h"
#include "struct/struct_string.h"
#include "misc/misc_result.h"
#include "gfx/gfx_sokol_include.h"

#if BUILD_WITH_SOKOL

#ifndef SOKOL_GFX_INCLUDED
#error Somehow sokol_gfx.h was not included properly before gfx_shader.h!
#endif

typedef struct Shader Shader;
struct Shader
{
	Arena* arena;
	Str8 name;
	#if DEBUG_BUILD
	Str8 filePath;
	#endif
	Result error;
	sg_shader handle;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void FreeShader(Shader* shader);
	#if DEBUG_BUILD
	Shader InitShader(Arena* arena, const sg_shader_desc* shaderDesc, Str8 filePath);
	#else
	Shader InitShader(Arena* arena, const sg_shader_desc* shaderDesc);
	#endif
#endif

#if DEBUG_BUILD
#define InitCompiledShader(arenaPntr, shaderName) InitShader((arenaPntr), shaderName##_shader_desc(sg_query_backend()), StrLit(shaderName##_FILE_PATH))
#else
#define InitCompiledShader(arenaPntr, shaderName) InitShader((arenaPntr), shaderName##_shader_desc(sg_query_backend()))
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void FreeShader(Shader* shader)
{
	NotNull(shader);
	if (shader->arena != nullptr)
	{
		sg_destroy_shader(shader->handle);
		FreeStr8(shader->arena, &shader->name);
		#if DEBUG_BUILD
		FreeStr8(shader->arena, &shader->filePath);
		#endif
	}
	ClearPointer(shader);
}

#if DEBUG_BUILD
PEXP Shader InitShader(Arena* arena, const sg_shader_desc* shaderDesc, Str8 filePath)
#else
PEXP Shader InitShader(Arena* arena, const sg_shader_desc* shaderDesc)
#endif
{
	NotNull(arena);
	NotNull(shaderDesc);
	Shader result = ZEROED;
	result.arena = arena;
	result.handle = sg_make_shader(shaderDesc);
	if (result.handle.id == SG_INVALID_ID)
	{
		result.error = Result_SokolError;
		return result;
	}
	Assert(result.handle.id != 0);
	result.name = AllocStr8(arena, StrLit(shaderDesc->label));
	NotNull(result.name.chars);
	#if DEBUG_BUILD
	result.filePath = AllocStr8(arena, filePath);
	NotNull(result.filePath.chars);
	#endif
	result.error = Result_Success;
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL

#endif //  _GFX_SHADER_H
