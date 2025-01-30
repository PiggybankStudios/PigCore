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

#define MAX_NUM_SHADER_UNIFORMS        32
#define MAX_NUM_SHADER_UNIFORM_BLOCKS  2 //NOTE: We currently only have 2 uniform blocks, one for the vertex shader and one for the fragment shader, so 2 is enough

//NOTE: These types are used in the generated shader header files and are passed to
//      the InitShader function, inside unpack these Def types and fill out the real
//      types that will live inside the Shader struct at runtime.
typedef struct ShaderAttributeDef ShaderAttributeDef;
struct ShaderAttributeDef { const char* name; uxx index; };
typedef struct ShaderUniformDef ShaderUniformDef;
struct ShaderUniformDef { const char* name; u8 blockIndex; uxx offset; uxx size; };

typedef struct ShaderAttribute ShaderAttribute;
struct ShaderAttribute
{
	Str8 name;
	uxx index;
	VertAttributeType type; //this is inferred based off the name!
};

typedef enum ShaderUniformType ShaderUniformType;
enum ShaderUniformType
{
	ShaderUniformType_None = 0,
	ShaderUniformType_ProjMatrix,
	ShaderUniformType_ViewMatrix,
	ShaderUniformType_WorldMatrix,
	ShaderUniformType_TintColor,
	ShaderUniformType_Count,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetShaderUniformTypeStr(ShaderUniformType enumValue);
const char* GetShaderUniformMatchStr(ShaderUniformType enumValue);
uxx GetShaderUniformMatchSize(ShaderUniformType enumValue);
#else
const char* GetShaderUniformTypeStr(ShaderUniformType enumValue)
{
	switch (enumValue)
	{
		case ShaderUniformType_None:        return "None";
		case ShaderUniformType_ProjMatrix:  return "ProjMatrix";
		case ShaderUniformType_ViewMatrix:  return "ViewMatrix";
		case ShaderUniformType_WorldMatrix: return "WorldMatrix";
		case ShaderUniformType_TintColor:   return "TintColor";
		default: return UNKNOWN_STR;
	}
}
const char* GetShaderUniformMatchStr(ShaderUniformType enumValue)
{
	switch (enumValue)
	{
		case ShaderUniformType_ProjMatrix:  return "proj";
		case ShaderUniformType_ViewMatrix:  return "view";
		case ShaderUniformType_WorldMatrix: return "world";
		case ShaderUniformType_TintColor:   return "tint";
		default: return UNKNOWN_STR;
	}
}
uxx GetShaderUniformMatchSize(ShaderUniformType enumValue)
{
	switch (enumValue)
	{
		case ShaderUniformType_ProjMatrix:  return sizeof(mat4);
		case ShaderUniformType_ViewMatrix:  return sizeof(mat4);
		case ShaderUniformType_WorldMatrix: return sizeof(mat4);
		case ShaderUniformType_TintColor:   return sizeof(v4r);
		default: return 0;
	}
}
#endif

typedef struct ShaderUniform ShaderUniform;
struct ShaderUniform
{
	ShaderUniformType type;
	Str8 name;
	u8 blockIndex;
	uxx offset;
	uxx size;
};

typedef struct ShaderUniformBlock ShaderUniformBlock;
struct ShaderUniformBlock
{
	bool valueChanged;
	uxx numUniforms;
	Slice value;
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
	uxx numUniforms;
	uxx uniformIndexByType[ShaderUniformType_Count];
	ShaderUniform uniforms[MAX_NUM_SHADER_UNIFORMS];
	ShaderUniformBlock uniformBlocks[MAX_NUM_SHADER_UNIFORM_BLOCKS];
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void FreeShader(Shader* shader);
	Shader InitShader(Arena* arena, const sg_shader_desc* shaderDesc, uxx numAttributes, const ShaderAttributeDef* attributeDefs, uxx numUniforms, const ShaderUniformDef* uniformDefs);
	PIG_CORE_INLINE void SetShaderFilePath(Shader* shader, Str8 filePath);
	PIG_CORE_INLINE void ApplyShaderUniforms(Shader* shader);
	PIG_CORE_INLINE bool SetShaderUniformByType(Shader* shader, ShaderUniformType type, uxx valueSize, const void* valuePntr);
	PIG_CORE_INLINE bool SetShaderProjectionMat(Shader* shader, mat4 matrix);
	PIG_CORE_INLINE bool SetShaderViewMat(Shader* shader, mat4 matrix);
	PIG_CORE_INLINE bool SetShaderWorldMat(Shader* shader, mat4 matrix);
	PIG_CORE_INLINE bool SetShaderTintColorVec(Shader* shader, v4r color);
	PIG_CORE_INLINE bool SetShaderTintColor(Shader* shader, Color32 color);
#endif

#define InitCompiledShader_Internal(outputShaderPntr, arenaPntr, shaderName) do                         \
{                                                                                                       \
	ShaderAttributeDef attributeDefs[shaderName##_SHADER_ATTR_COUNT] = shaderName##_SHADER_ATTR_DEFS;   \
	ShaderUniformDef uniformDefs[shaderName##_SHADER_UNIFORM_COUNT] = shaderName##_SHADER_UNIFORM_DEFS; \
	*(outputShaderPntr) = InitShader(                                                                   \
		(arenaPntr),                                                                                    \
		shaderName##_shader_desc(sg_query_backend()),                                                   \
		shaderName##_SHADER_ATTR_COUNT,                                                                 \
		&attributeDefs[0],                                                                              \
		shaderName##_SHADER_UNIFORM_COUNT,                                                              \
		&uniformDefs[0]                                                                                 \
	);                                                                                                  \
} while(0)
#if DEBUG_BUILD
#define InitCompiledShader(outputShaderPntr, arenaPntr, shaderName) do            \
{                                                                                 \
	InitCompiledShader_Internal((outputShaderPntr), (arenaPntr), shaderName);     \
	SetShaderFilePath((outputShaderPntr), StrLit(shaderName##_SHADER_FILE_PATH)); \
} while(0)
#else
#define InitCompiledShader(outputShaderPntr, arenaPntr, shaderName) InitCompiledShader_Internal((outputShaderPntr), (arenaPntr), shaderName);
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
		for (uxx uIndex = 0; uIndex < shader->numAttributes; uIndex++)
		{
			FreeStr8(shader->arena, &shader->uniforms[uIndex].name);
		}
		for (uxx bIndex = 0; bIndex < ArrayCount(shader->uniformBlocks); bIndex++)
		{
			FreeStr8(shader->arena, &shader->uniformBlocks[bIndex].value);
		}
	}
	ClearPointer(shader);
}

PEXP Shader InitShader(Arena* arena, const sg_shader_desc* shaderDesc, uxx numAttributes, const ShaderAttributeDef* attributeDefs, uxx numUniforms, const ShaderUniformDef* uniformDefs)
{
	NotNull(arena);
	NotNull(shaderDesc);
	Assert(numAttributes > 0);
	Assert(numAttributes <= MAX_NUM_VERT_ATTRIBUTES);
	NotNull(attributeDefs);
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
	result.numAttributes = numAttributes;
	result.numUniforms = numUniforms;
	
	for (uxx aIndex = 0; aIndex < numAttributes; aIndex++)
	{
		const ShaderAttributeDef* attributeDef = &attributeDefs[aIndex];
		ShaderAttribute* attribute = &result.attributes[aIndex];
		NotNull(attributeDef->name);
		attribute->name = AllocStr8Nt(arena, attributeDef->name);
		NotNull(attribute->name.chars);
		attribute->index = attributeDef->index;
		for (uxx tIndex = 1; tIndex < VertAttributeType_Count; tIndex++)
		{
			VertAttributeType type = (VertAttributeType)tIndex;
			if (StrContains(attribute->name, StrLit(GetVertAttributeMatchStr(type)), false)) { attribute->type = type; break; }
		}
	}
	
	for (uxx tIndex = 0; tIndex < ShaderUniformType_Count; tIndex++) { result.uniformIndexByType[tIndex] = UINTXX_MAX; }
	
	for (uxx uIndex = 0; uIndex < numUniforms; uIndex++)
	{
		const ShaderUniformDef* uniformDef = &uniformDefs[uIndex];
		ShaderUniform* uniform = &result.uniforms[uIndex];
		NotNull(uniformDef->name);
		uniform->name = AllocStr8Nt(arena, uniformDef->name);
		NotNull(uniform->name.chars);
		uniform->blockIndex = uniformDef->blockIndex;
		uniform->offset = uniformDef->offset;
		uniform->size = uniformDef->size;
		
		for (uxx tIndex = 1; tIndex < ShaderUniformType_Count; tIndex++)
		{
			ShaderUniformType type = (ShaderUniformType)tIndex;
			if (uniformDef->size == GetShaderUniformMatchSize(type) &&
				StrContains(uniform->name, StrLit(GetShaderUniformMatchStr(type)), false))
			{
				uniform->type = type;
				break;
			}
		}
		if (uniform->type != ShaderUniformType_None && result.uniformIndexByType[uniform->type] == UINTXX_MAX)
		{
			result.uniformIndexByType[uniform->type] = uIndex;
		}
		
		Assert(uniformDef->blockIndex < MAX_NUM_SHADER_UNIFORM_BLOCKS);
		ShaderUniformBlock* uniformBlock = &result.uniformBlocks[uniformDef->blockIndex];
		uniformBlock->value.length = MaxUXX(uniformBlock->value.length, uniformDef->offset + uniformDef->size);
		uniformBlock->numUniforms++;
	}
	
	for (uxx bIndex = 0; bIndex < MAX_NUM_SHADER_UNIFORM_BLOCKS; bIndex++)
	{
		ShaderUniformBlock* uniformBlock = &result.uniformBlocks[bIndex];
		if (uniformBlock->value.length > 0)
		{
			uniformBlock->value.bytes = AllocArray(u8, arena, uniformBlock->value.length);
			MyMemSet(uniformBlock->value.bytes, 0x00, uniformBlock->value.length);
		}
	}
	
	result.error = Result_Success;
	return result;
}

PEXPI void SetShaderFilePath(Shader* shader, Str8 filePath)
{
	NotNull(shader);
	NotNull(shader->arena);
	NotNullStr(filePath);
	#if DEBUG_BUILD
	FreeStr8(shader->arena, &shader->filePath);
	if (!IsEmptyStr(filePath))
	{
		shader->filePath = AllocStr8(shader->arena, filePath);
		NotNull(shader->filePath.chars);
	}
	#else
	UNUSED(shader);
	UNUSED(filePath);
	#endif
}

PEXPI void ApplyShaderUniforms(Shader* shader)
{
	for (uxx bIndex = 0; bIndex < MAX_NUM_SHADER_UNIFORM_BLOCKS; bIndex++)
	{
		if (shader->uniformBlocks[bIndex].valueChanged)
		{
			shader->uniformBlocks[bIndex].valueChanged = false;
			sg_range blockSokolRange = (sg_range){ shader->uniformBlocks[bIndex].value.bytes, shader->uniformBlocks[bIndex].value.length };
			sg_apply_uniforms((int)bIndex, &blockSokolRange);
		}
	}
}

PEXPI bool SetShaderUniformByType(Shader* shader, ShaderUniformType type, uxx valueSize, const void* valuePntr)
{
	if (shader->uniformIndexByType[type] == UINTXX_MAX) { return false; }
	Assert(shader->uniformIndexByType[type] < shader->numUniforms);
	ShaderUniform* uniform = &shader->uniforms[shader->uniformIndexByType[type]];
	Assert(uniform->size == valueSize);
	Assert(uniform->blockIndex < MAX_NUM_SHADER_UNIFORM_BLOCKS);
	ShaderUniformBlock* uniformBlock = &shader->uniformBlocks[uniform->blockIndex];
	NotNull(uniformBlock->value.bytes);
	Assert(uniform->offset + uniform->size <= uniformBlock->value.length);
	MyMemCopy(&uniformBlock->value.bytes[uniform->offset], valuePntr, valueSize);
	uniformBlock->valueChanged = true;
	return true;
}

PEXPI bool SetShaderProjectionMat(Shader* shader, mat4 matrix) { return SetShaderUniformByType(shader, ShaderUniformType_ProjMatrix, sizeof(matrix), &matrix); }
PEXPI bool SetShaderViewMat(Shader* shader, mat4 matrix) { return SetShaderUniformByType(shader, ShaderUniformType_ViewMatrix, sizeof(matrix), &matrix); }
PEXPI bool SetShaderWorldMat(Shader* shader, mat4 matrix) { return SetShaderUniformByType(shader, ShaderUniformType_WorldMatrix, sizeof(matrix), &matrix); }
PEXPI bool SetShaderTintColorVec(Shader* shader, v4r color) { return SetShaderUniformByType(shader, ShaderUniformType_TintColor, sizeof(color), &color); }
PEXPI bool SetShaderTintColor(Shader* shader, Color32 color) { return SetShaderTintColorVec(shader, ToV4rFromColor32(color)); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL

#endif //  _GFX_SHADER_H

#if defined(_GFX_SHADER_H) && defined(_GFX_VERT_BUFFER_H)
#include "cross/cross_shader_and_vert_buffer.h"
#endif
