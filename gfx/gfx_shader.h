/*
File:   gfx_shader.h
Author: Taylor Robbins
Date:   01\28\2025
Description:
	** Contains the Shader structure which holds a sg_shader from sokol_gfx as well
	** as other metadata about the shader when it was created.
*/

#ifndef _GFX_SHADER_H
#define _GFX_SHADER_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "struct/struct_vectors.h"
#include "mem/mem_arena.h"
#include "struct/struct_string.h"
#include "misc/misc_result.h"
#include "gfx/gfx_vertices.h"
#include "gfx/gfx_sokol_include.h"

#if BUILD_WITH_SOKOL

#ifndef SOKOL_GFX_INCLUDED
#error Somehow sokol_gfx.h was not included properly before gfx_shader.h!
#endif

typedef struct ShaderAttribute ShaderAttribute;
struct ShaderAttribute
{
	Str8 name;
	uxx index;
	VertAttributeType type; //this is inferred based off the name!
};

typedef struct Shader Shader;
struct Shader
{
	Arena* arena;
	Result error;
	sg_shader handle;
	Str8 name;
	#if DEBUG_BUILD
	Str8 filePath;
	#endif
	uxx numAttributes;
	ShaderAttribute attributes[MAX_NUM_VERT_ATTRIBUTES];
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void FreeShader(Shader* shader);
	#if DEBUG_BUILD
	Shader InitShader(Arena* arena, const sg_shader_desc* shaderDesc, uxx numAttributes, const char** attributeNames, const uxx* attributeIndices, Str8 filePath);
	#else
	Shader InitShader(Arena* arena, const sg_shader_desc* shaderDesc, uxx numAttributes, const char** attributeNames, const uxx* attributeIndices);
	#endif
	void PipelineDescFromShaderAndBuffer(const Shader* shader, const VertBuffer* buffer, sg_pipeline_desc* pipelineDesc);
#endif

#if DEBUG_BUILD
#define InitCompiledShader(outputShaderPntr, arenaPntr, shaderName) do                           \
{                                                                                                \
	const char* attributeNames[shaderName##_SHADER_ATTR_COUNT] = shaderName##_SHADER_ATTR_NAMES; \
	uxx attributeValues[shaderName##_SHADER_ATTR_COUNT] = shaderName##_SHADER_ATTRS;             \
	*(outputShaderPntr) = InitShader(                                                            \
		(arenaPntr),                                                                             \
		shaderName##_shader_desc(sg_query_backend()),                                            \
		shaderName##_SHADER_ATTR_COUNT,                                                          \
		attributeNames,                                                                          \
		attributeValues,                                                                         \
		StrLit(shaderName##_SHADER_FILE_PATH)                                                    \
	);                                                                                           \
} while(0) 
#else
#define InitCompiledShader(outputShaderPntr, arenaPntr, shaderName) do                           \
{                                                                                                \
	const char* attributeNames[shaderName##_SHADER_ATTR_COUNT] = shaderName##_SHADER_ATTR_NAMES; \
	uxx attributeValues[shaderName##_SHADER_ATTR_COUNT] = shaderName##_SHADER_ATTRS;             \
	*(outputShaderPntr) = InitShader(                                                            \
		(arenaPntr),                                                                             \
		shaderName##_shader_desc(sg_query_backend()),                                            \
		shaderName##_SHADER_ATTR_COUNT,                                                          \
		attributeNames,                                                                          \
		attributeValues                                                                          \
	);                                                                                           \
} while(0)
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
		for (uxx aIndex = 0; aIndex < shader->numAttributes; aIndex++)
		{
			FreeStr8(shader->arena, &shader->attributes[aIndex].name);
		}
	}
	ClearPointer(shader);
}

#if DEBUG_BUILD
PEXP Shader InitShader(Arena* arena, const sg_shader_desc* shaderDesc, uxx numAttributes, const char** attributeNames, const uxx* attributeIndices, Str8 filePath)
#else
PEXP Shader InitShader(Arena* arena, const sg_shader_desc* shaderDesc, uxx numAttributes, const char** attributeNames, const uxx* attributeIndices)
#endif
{
	NotNull(arena);
	NotNull(shaderDesc);
	Assert(numAttributes > 0);
	Assert(numAttributes <= MAX_NUM_VERT_ATTRIBUTES);
	NotNull(attributeNames);
	NotNull(attributeIndices);
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
	result.numAttributes = numAttributes;
	for (uxx aIndex = 0; aIndex < numAttributes; aIndex++)
	{
		NotNull(attributeNames[aIndex]);
		result.attributes[aIndex].name = AllocStr8Nt(arena, attributeNames[aIndex]);
		NotNull(result.attributes[aIndex].name.chars);
		result.attributes[aIndex].index = attributeIndices[aIndex];
		if      (StrContains(result.attributes[aIndex].name, StrLit("position"), false)) { result.attributes[aIndex].type = VertAttributeType_Position; }
		else if (StrContains(result.attributes[aIndex].name, StrLit("color"),    false)) { result.attributes[aIndex].type = VertAttributeType_Color;    }
		else if (StrContains(result.attributes[aIndex].name, StrLit("normal"),   false)) { result.attributes[aIndex].type = VertAttributeType_Normal;   }
		else if (StrContains(result.attributes[aIndex].name, StrLit("tangent"),  false)) { result.attributes[aIndex].type = VertAttributeType_Tangent;  }
		else if (StrContains(result.attributes[aIndex].name, StrLit("tex"),      false)) { result.attributes[aIndex].type = VertAttributeType_TexCoord; }
		else { result.attributes[aIndex].type = VertAttributeType_None; }
	}
	result.error = Result_Success;
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL

#endif //  _GFX_SHADER_H

#if defined(_GFX_SHADER_H) && defined(_GFX_VERT_BUFFER_H)
#include "cross/cross_shader_and_vert_buffer.h"
#endif
