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
#include "mem/mem_scratch.h"
#include "struct/struct_string.h"
#include "misc/misc_result.h"
#include "gfx/gfx_vertices.h"
#include "gfx/gfx_sokol_include.h"

#if BUILD_WITH_SOKOL

#ifndef SOKOL_GFX_INCLUDED
#error Somehow sokol_gfx.h was not included properly before gfx_shader.h!
#endif

#define MAX_NUM_SHADER_IMAGES          8
#define MAX_NUM_SHADER_SAMPLERS        8
#define MAX_NUM_SHADER_UNIFORMS        32
#define MAX_NUM_SHADER_UNIFORM_BLOCKS  2 //NOTE: We currently only have 2 uniform blocks, one for the vertex shader and one for the fragment shader, so 2 is enough

//NOTE: These types are used in the generated shader header files and are passed to
//      the InitShader function, inside we unpack these Def types and fill out the
//      real types that will live inside the Shader struct at runtime.
typedef struct ShaderImageDef ShaderImageDef;
struct ShaderImageDef { const char* name; uxx index; };
typedef struct ShaderSamplerDef ShaderSamplerDef;
struct ShaderSamplerDef { const char* name; uxx index; };
typedef struct ShaderAttributeDef ShaderAttributeDef;
struct ShaderAttributeDef { const char* name; uxx index; };
typedef struct ShaderUniformDef ShaderUniformDef;
struct ShaderUniformDef { const char* name; u8 blockIndex; uxx offset; uxx size; };

typedef struct ShaderMetadata ShaderMetadata;
struct ShaderMetadata
{
	uxx numImages;
	const ShaderImageDef* imageDefs;
	uxx numSamplers;
	const ShaderSamplerDef* samplerDefs;
	uxx numUniforms;
	const ShaderUniformDef* uniformDefs;
	uxx numAttributes;
	const ShaderAttributeDef* attributeDefs;
};

typedef struct ShaderImage ShaderImage;
struct ShaderImage
{
	Str8 name;
	uxx index;
	uxx sizeUniformIndex;
};

typedef struct ShaderSampler ShaderSampler;
struct ShaderSampler
{
	Str8 name;
	uxx index;
};

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
	ShaderUniformType_SourceRec,
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
		case ShaderUniformType_SourceRec:   return "SourceRec";
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
		case ShaderUniformType_SourceRec:   return "source";
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
		case ShaderUniformType_SourceRec:   return sizeof(v4r);
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
	uxx numImages;
	ShaderImage images[MAX_NUM_SHADER_IMAGES];
	uxx numSamplers;
	ShaderSampler samplers[MAX_NUM_SHADER_SAMPLERS];
	uxx numUniforms;
	uxx uniformIndexByType[ShaderUniformType_Count];
	ShaderUniform uniforms[MAX_NUM_SHADER_UNIFORMS];
	ShaderUniformBlock uniformBlocks[MAX_NUM_SHADER_UNIFORM_BLOCKS];
	uxx numAttributes;
	ShaderAttribute attributes[MAX_NUM_VERT_ATTRIBUTES];
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void FreeShader(Shader* shader);
	Shader InitShader(Arena* arena, const sg_shader_desc* shaderDesc, const ShaderMetadata* shaderMetadata);
	PIG_CORE_INLINE void SetShaderFilePath(Shader* shader, Str8 filePath);
	PIG_CORE_INLINE void ApplyShaderUniforms(Shader* shader);
	PIG_CORE_INLINE bool SetShaderUniformByType(Shader* shader, ShaderUniformType type, uxx valueSize, const void* valuePntr);
	PIG_CORE_INLINE bool SetShaderProjectionMat(Shader* shader, mat4 matrix);
	PIG_CORE_INLINE bool SetShaderViewMat(Shader* shader, mat4 matrix);
	PIG_CORE_INLINE bool SetShaderWorldMat(Shader* shader, mat4 matrix);
	PIG_CORE_INLINE bool SetShaderTintColorRaw(Shader* shader, v4r color);
	PIG_CORE_INLINE bool SetShaderTintColor(Shader* shader, Color32 color);
	PIG_CORE_INLINE bool SetShaderSourceRecRaw(Shader* shader, v4r rectangle); //TODO: Change these to rec type!
	PIG_CORE_INLINE bool SetShaderSourceRec(Shader* shader, v4 rectangle); //TODO: Change these to rec type!
	PIG_CORE_INLINE bool SetShaderUniformByName(Shader* shader, Str8 uniformName, uxx valueSize, const void* valuePntr);
	PIG_CORE_INLINE bool SetShaderUniformByNameR32(Shader* shader, Str8 uniformName, r32 value);
	PIG_CORE_INLINE bool SetShaderUniformByNameMat4(Shader* shader, Str8 uniformName, mat4 matrix);
	PIG_CORE_INLINE bool SetShaderUniformByNameV2(Shader* shader, Str8 uniformName, v2 vector);
	PIG_CORE_INLINE bool SetShaderUniformByNameV3(Shader* shader, Str8 uniformName, v3 vector);
	PIG_CORE_INLINE bool SetShaderUniformByNameV4(Shader* shader, Str8 uniformName, v4 vector);
#endif

#define InitCompiledShader_Internal(outputShaderPntr, arenaPntr, shaderName) do                                   \
{                                                                                                                 \
	ShaderImageDef imageDefs[] = shaderName##_SHADER_IMAGE_DEFS;                                                  \
	ShaderSamplerDef samplerDefs[] = shaderName##_SHADER_SAMPLER_DEFS;                                            \
	ShaderUniformDef uniformDefs[] = shaderName##_SHADER_UNIFORM_DEFS;                                            \
	ShaderAttributeDef attributeDefs[] = shaderName##_SHADER_ATTR_DEFS;                                           \
	ShaderMetadata shaderMetadata = ZEROED;                                                                       \
	shaderMetadata.numImages = shaderName##_SHADER_IMAGE_COUNT;                                                   \
	shaderMetadata.imageDefs = &imageDefs[0];                                                                     \
	shaderMetadata.numSamplers = shaderName##_SHADER_SAMPLER_COUNT;                                               \
	shaderMetadata.samplerDefs = &samplerDefs[0];                                                                 \
	shaderMetadata.numUniforms = shaderName##_SHADER_UNIFORM_COUNT;                                               \
	shaderMetadata.uniformDefs = &uniformDefs[0];                                                                 \
	shaderMetadata.numAttributes = shaderName##_SHADER_ATTR_COUNT;                                                \
	shaderMetadata.attributeDefs = &attributeDefs[0];                                                             \
	*(outputShaderPntr) = InitShader((arenaPntr), shaderName##_shader_desc(sg_query_backend()), &shaderMetadata); \
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

PEXP Shader InitShader(Arena* arena, const sg_shader_desc* shaderDesc, const ShaderMetadata* shaderMetadata)
{
	NotNull(arena);
	NotNull(shaderDesc);
	NotNull(shaderMetadata);
	Assert(shaderMetadata->numImages <= MAX_NUM_SHADER_IMAGES);
	Assert(shaderMetadata->numSamplers <= MAX_NUM_SHADER_SAMPLERS);
	Assert(shaderMetadata->numUniforms <= MAX_NUM_SHADER_UNIFORMS);
	Assert(shaderMetadata->numAttributes > 0 && shaderMetadata->numAttributes <= MAX_NUM_VERT_ATTRIBUTES);
	Assert(shaderMetadata->numImages == 0 || shaderMetadata->imageDefs != nullptr);
	Assert(shaderMetadata->numSamplers == 0 || shaderMetadata->samplerDefs != nullptr);
	Assert(shaderMetadata->numUniforms == 0 || shaderMetadata->uniformDefs != nullptr);
	NotNull(shaderMetadata->attributeDefs);
	ScratchBegin1(scratch, arena);
	
	Shader result = ZEROED;
	result.arena = arena;
	result.handle = sg_make_shader(shaderDesc);
	if (result.handle.id == SG_INVALID_ID)
	{
		result.error = Result_SokolError;
		ScratchEnd(scratch);
		return result;
	}
	result.name = AllocStr8(arena, StrLit(shaderDesc->label));
	NotNull(result.name.chars);
	result.numImages = shaderMetadata->numImages;
	result.numSamplers = shaderMetadata->numSamplers;
	result.numUniforms = shaderMetadata->numUniforms;
	result.numAttributes = shaderMetadata->numAttributes;
	
	// +==============================+
	// |        Handle Images         |
	// +==============================+
	for (uxx iIndex = 0; iIndex < shaderMetadata->numImages; iIndex++)
	{
		const ShaderImageDef* imageDef = &shaderMetadata->imageDefs[iIndex];
		ShaderImage* image = &result.images[iIndex];
		NotNull(imageDef->name);
		image->name = AllocStr8Nt(arena, imageDef->name);
		NotNull(image->name.chars);
		image->index = imageDef->index;
	}
	
	// +==============================+
	// |       Handle Samplers        |
	// +==============================+
	for (uxx sIndex = 0; sIndex < shaderMetadata->numSamplers; sIndex++)
	{
		const ShaderSamplerDef* samplerDef = &shaderMetadata->samplerDefs[sIndex];
		ShaderSampler* sampler = &result.samplers[sIndex];
		NotNull(samplerDef->name);
		sampler->name = AllocStr8Nt(arena, samplerDef->name);
		NotNull(sampler->name.chars);
		sampler->index = samplerDef->index;
	}
	
	// +==============================+
	// |       Handle Uniforms        |
	// +==============================+
	for (uxx tIndex = 0; tIndex < ShaderUniformType_Count; tIndex++) { result.uniformIndexByType[tIndex] = UINTXX_MAX; }
	
	for (uxx uIndex = 0; uIndex < shaderMetadata->numUniforms; uIndex++)
	{
		const ShaderUniformDef* uniformDef = &shaderMetadata->uniformDefs[uIndex];
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
	
	//Look for a uniform with the same name as each image but with "_size" suffix
	for (uxx iIndex = 0; iIndex < result.numImages; iIndex++)
	{
		ShaderImage* image = &result.images[iIndex];
		image->sizeUniformIndex = result.numUniforms;
		if (!IsEmptyStr(image->name))
		{
			Str8 sizeUniformName = JoinStringsInArena(scratch, image->name, StrLit("_size"), false);
			for (uxx uIndex = 0; uIndex < result.numUniforms; uIndex++)
			{
				ShaderUniform* uniform = &result.uniforms[uIndex];
				if (StrExactEquals(uniform->name, sizeUniformName))
				{
					image->sizeUniformIndex = uIndex;
					break;
				}
			}
		}
	}
	
	// +==============================+
	// |      Handle Attributes       |
	// +==============================+
	for (uxx aIndex = 0; aIndex < shaderMetadata->numAttributes; aIndex++)
	{
		const ShaderAttributeDef* attributeDef = &shaderMetadata->attributeDefs[aIndex];
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
	
	ScratchEnd(scratch);
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
PEXPI bool SetShaderTintColorRaw(Shader* shader, v4r color) { return SetShaderUniformByType(shader, ShaderUniformType_TintColor, sizeof(color), &color); }
PEXPI bool SetShaderTintColor(Shader* shader, Color32 color) { return SetShaderTintColorRaw(shader, ToV4rFromColor32(color)); }
PEXPI bool SetShaderSourceRecRaw(Shader* shader, v4r rectangle) { return SetShaderUniformByType(shader, ShaderUniformType_SourceRec, sizeof(rectangle), &rectangle); } //TODO: Change these to rec type!
PEXPI bool SetShaderSourceRec(Shader* shader, v4 rectangle) { return SetShaderSourceRecRaw(shader, ToV4rFrom4(rectangle)); } //TODO: Change these to rec type!

PEXPI bool SetShaderUniformByName(Shader* shader, Str8 uniformName, uxx valueSize, const void* valuePntr)
{
	for (uxx uIndex = 0; uIndex < shader->numUniforms; uIndex++)
	{
		ShaderUniform* uniform = &shader->uniforms[uIndex];
		if (StrExactEquals(uniform->name, uniformName))
		{
			Assert(uniform->size == valueSize);
			Assert(uniform->blockIndex < MAX_NUM_SHADER_UNIFORM_BLOCKS);
			ShaderUniformBlock* uniformBlock = &shader->uniformBlocks[uniform->blockIndex];
			NotNull(uniformBlock->value.bytes);
			Assert(uniform->offset + uniform->size <= uniformBlock->value.length);
			MyMemCopy(&uniformBlock->value.bytes[uniform->offset], valuePntr, valueSize);
			uniformBlock->valueChanged = true;
			return true;
		}
	}
	return false;
}
PEXPI bool SetShaderUniformByNameMat4(Shader* shader, Str8 uniformName, mat4 matrix) { return SetShaderUniformByName(shader, uniformName, sizeof(matrix), &matrix); }
PEXPI bool SetShaderUniformByNameR32(Shader* shader, Str8 uniformName, r32 value) { return SetShaderUniformByName(shader, uniformName, sizeof(value), &value); }
PEXPI bool SetShaderUniformByNameV2(Shader* shader, Str8 uniformName, v2 vector) { return SetShaderUniformByName(shader, uniformName, sizeof(vector), &vector); }
PEXPI bool SetShaderUniformByNameV3(Shader* shader, Str8 uniformName, v3 vector) { return SetShaderUniformByName(shader, uniformName, sizeof(vector), &vector); }
PEXPI bool SetShaderUniformByNameV4(Shader* shader, Str8 uniformName, v4 vector) { return SetShaderUniformByName(shader, uniformName, sizeof(vector), &vector); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL

#endif //  _GFX_SHADER_H

#if defined(_GFX_SHADER_H) && defined(_GFX_VERT_BUFFER_H)
#include "cross/cross_shader_and_vert_buffer.h"
#endif

#if defined(_GFX_SHADER_H) && defined(_GFX_TEXTURE_H)
#include "cross/cross_shader_and_texture.h"
#endif
