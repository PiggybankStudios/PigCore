/*
File:   gfx_pipeline.h
Author: Taylor Robbins
Date:   01\30\2025
Description:
	** This file contains a bunch of structs that wrap Sokol rendering things (like sg_pipeline)
	** but it also brings all the other graphics types together into an API that we can use
	** to track and invoke rendering commands
*/

#ifndef _GFX_PIPELINE_H
#define _GFX_PIPELINE_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "struct/struct_string.h"
#include "misc/misc_result.h"
#include "misc/misc_sokol_gfx_include.h"
#include "gfx/gfx_shader.h"
#include "gfx/gfx_texture.h"
#include "gfx/gfx_vert_buffer.h"

#if BUILD_WITH_SOKOL_GFX

typedef enum GfxPipelineBlendMode GfxPipelineBlendMode;
enum GfxPipelineBlendMode
{
	GfxPipelineBlendMode_None = 0,
	GfxPipelineBlendMode_Opaque,
	GfxPipelineBlendMode_Normal,
	GfxPipelineBlendMode_PremultipliedNormal,
	GfxPipelineBlendMode_Count,
};

#if !PIG_CORE_IMPLEMENTATION
const char* GetGfxPipelineBlendModeStr(GfxPipelineBlendMode enumValue);
#else
PEXP const char* GetGfxPipelineBlendModeStr(GfxPipelineBlendMode enumValue)
{
	switch (enumValue)
	{
		case GfxPipelineBlendMode_None:                return "None";
		case GfxPipelineBlendMode_Opaque:              return "Opaque";
		case GfxPipelineBlendMode_Normal:              return "Normal";
		case GfxPipelineBlendMode_PremultipliedNormal: return "PremultipliedNormal";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef plex GfxPipelineOptions GfxPipelineOptions;
plex GfxPipelineOptions
{
	Shader* shader;
	uxx vertexSize;
	uxx numVertAttributes;
	VertAttribute vertAttributes[MAX_NUM_VERT_ATTRIBUTES];
	bool colorWriteEnabled;
	bool depthWriteEnabled;
	bool depthTestEnabled;
	bool cullingEnabled;
	uxx indexedVerticesSize;
	GfxPipelineBlendMode blendMode;
	//TODO: Add primitive type option?
	//TODO: Add indexed buffer options?
};

typedef plex GfxPipeline GfxPipeline;
plex GfxPipeline
{
	Arena* arena;
	Result error;
	Str8 name;
	GfxPipelineOptions options;
	sg_pipeline handle;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE bool AreEqualGfxPipelineOptions(const GfxPipelineOptions* left, const GfxPipelineOptions* right);
	void FreeGfxPipeline(GfxPipeline* pipeline);
	void MatchVertAttributesToShader(sg_pipeline_desc* pipelineDesc, const Shader* shader, uxx vertexSize, uxx numVertAttributes, const VertAttribute* vertAttributes);
	void FillGfxPipelineOptionsFromVertBuffer(GfxPipelineOptions* options, const VertBuffer* buffer);
	GfxPipeline InitGfxPipeline(Arena* arena, Str8 name, const GfxPipelineOptions* options);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI bool AreEqualGfxPipelineOptions(const GfxPipelineOptions* left, const GfxPipelineOptions* right)
{
	#if 1
	if (left->shader != right->shader) { return false; }
	if (left->vertexSize != right->vertexSize) { return false; }
	if (left->numVertAttributes != right->numVertAttributes) { return false; }
	for (uxx aIndex = 0; aIndex < left->numVertAttributes; aIndex++)
	{
		if (left->vertAttributes[aIndex].type != right->vertAttributes[aIndex].type) { return false; }
		if (left->vertAttributes[aIndex].size != right->vertAttributes[aIndex].size) { return false; }
		if (left->vertAttributes[aIndex].offset != right->vertAttributes[aIndex].offset) { return false; }
	}
	if (left->colorWriteEnabled != right->colorWriteEnabled) { return false; }
	if (left->depthWriteEnabled != right->depthWriteEnabled) { return false; }
	if (left->depthTestEnabled != right->depthTestEnabled) { return false; }
	if (left->cullingEnabled != right->cullingEnabled) { return false; }
	if (left->indexedVerticesSize != right->indexedVerticesSize) { return false; }
	if (left->blendMode != right->blendMode) { return false; }
	return true;
	#else
	return MyMemEquals(left, right, sizeof(GfxPipelineOptions));
	#endif
}

PEXP void FreeGfxPipeline(GfxPipeline* pipeline)
{
	NotNull(pipeline);
	if (pipeline->arena != nullptr)
	{
		sg_destroy_pipeline(pipeline->handle);
		FreeStr8(pipeline->arena, &pipeline->name);
	}
	ClearPointer(pipeline);
}

//NOTE: For now this assumes we have one buffer slot for the pipeline. You cand bind different buffers
//      but they all have to have the same vertex format as the buffer passed here
PEXP void MatchVertAttributesToShader(sg_pipeline_desc* pipelineDesc, const Shader* shader, uxx vertexSize, uxx numVertAttributes, const VertAttribute* vertAttributes)
{
	NotNull(shader);
	NotNull(pipelineDesc);
	Assert(numVertAttributes > 0 && numVertAttributes < MAX_NUM_VERT_ATTRIBUTES);
	Assert(vertexSize > 0);
	NotNull(vertAttributes);
	pipelineDesc->shader = shader->handle;
	pipelineDesc->layout.buffers[0].stride = (int)vertexSize;
	for (uxx bufferAttribIndex = 0; bufferAttribIndex < numVertAttributes; bufferAttribIndex++)
	{
		const VertAttribute* bufferAttrib = &vertAttributes[bufferAttribIndex];
		Assert(bufferAttrib->offset + bufferAttrib->size <= vertexSize);
		for (uxx shaderAttribIndex = 0; shaderAttribIndex < shader->numAttributes; shaderAttribIndex++)
		{
			const ShaderAttribute* shaderAttrib = &shader->attributes[shaderAttribIndex];
			if (bufferAttrib->type == shaderAttrib->type)
			{
				sg_vertex_format sokolFormat = SG_VERTEXFORMAT_INVALID;
				switch (bufferAttrib->size)
				{
					case sizeof(r32)*1: sokolFormat = SG_VERTEXFORMAT_FLOAT; break;
					case sizeof(r32)*2: sokolFormat = SG_VERTEXFORMAT_FLOAT2; break;
					case sizeof(r32)*3: sokolFormat = SG_VERTEXFORMAT_FLOAT3; break;
					case sizeof(r32)*4: sokolFormat = SG_VERTEXFORMAT_FLOAT4; break;
					default: DebugAssertMsg(false, "Unhandled attribute size in FillPipelineDescLayout!"); break;
				}
				pipelineDesc->layout.attrs[shaderAttrib->index].buffer_index = 0;
				pipelineDesc->layout.attrs[shaderAttrib->index].format = sokolFormat;
				pipelineDesc->layout.attrs[shaderAttrib->index].offset = bufferAttrib->offset;
			}
		}
	}
}

PEXP void FillGfxPipelineOptionsFromVertBuffer(GfxPipelineOptions* options, const VertBuffer* buffer)
{
	Assert(buffer->numAttributes <= MAX_NUM_VERT_ATTRIBUTES);
	options->vertexSize = buffer->vertexSize;
	options->numVertAttributes = buffer->numAttributes;
	MyMemCopy(&options->vertAttributes[0], &buffer->attributes[0], sizeof(VertAttribute) * buffer->numAttributes);
}

PEXP GfxPipeline InitGfxPipeline(Arena* arena, Str8 name, const GfxPipelineOptions* options)
{
	NotNull(arena);
	NotNullStr(name);
	NotNull(options);
	NotNull(options->shader);
	NotNull(options->shader->arena);
	Assert(options->numVertAttributes > 0);
	ScratchBegin1(scratch, arena);
	
	GfxPipeline result = ZEROED;
	result.arena = arena;
	if (!IsEmptyStr(name)) { result.name = AllocStr8(arena, name); NotNull(result.name.chars); }
	MyMemCopy(&result.options, options, sizeof(GfxPipelineOptions));
	
	sg_pipeline_desc pipelineDesc = ZEROED;
	Str8 nameNt = AllocStrAndCopy(scratch, name.length, name.chars, true); NotNull(nameNt.chars); //allocate to ensure null-term char
	pipelineDesc.label = nameNt.chars;
	MatchVertAttributesToShader(&pipelineDesc, options->shader, options->vertexSize, options->numVertAttributes, &options->vertAttributes[0]);
	pipelineDesc.depth.pixel_format = _SG_PIXELFORMAT_DEFAULT; //TODO: What format is DEFAULT?
	pipelineDesc.depth.compare = options->depthTestEnabled ? SG_COMPAREFUNC_LESS_EQUAL : SG_COMPAREFUNC_ALWAYS;
	pipelineDesc.depth.write_enabled = options->depthWriteEnabled;
	pipelineDesc.stencil.enabled = false;
	pipelineDesc.color_count = 1;
	pipelineDesc.colors[0].pixel_format = _SG_PIXELFORMAT_DEFAULT; //TODO: What format is DEFAULT?
	pipelineDesc.colors[0].write_mask = options->colorWriteEnabled ? SG_COLORMASK_RGBA : SG_COLORMASK_NONE;
	//TODO: We should do some more testing to make sure these options work for pre-multiplied alpha!
	pipelineDesc.colors[0].blend.enabled = (options->blendMode != GfxPipelineBlendMode_Opaque);
	pipelineDesc.colors[0].blend.src_factor_rgb = (options->blendMode == GfxPipelineBlendMode_PremultipliedNormal) ? SG_BLENDFACTOR_ONE : SG_BLENDFACTOR_SRC_ALPHA;
	pipelineDesc.colors[0].blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	pipelineDesc.colors[0].blend.op_rgb = SG_BLENDOP_ADD;
	pipelineDesc.colors[0].blend.src_factor_alpha = SG_BLENDFACTOR_ONE;
	pipelineDesc.colors[0].blend.dst_factor_alpha = SG_BLENDFACTOR_ZERO;
	pipelineDesc.colors[0].blend.op_alpha = SG_BLENDOP_ADD;
	pipelineDesc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES; //TODO: Add primitive type option?
	switch (options->indexedVerticesSize)
	{
		case 0: pipelineDesc.index_type = SG_INDEXTYPE_NONE; break;
		case sizeof(u16): pipelineDesc.index_type = SG_INDEXTYPE_UINT16; break;
		case sizeof(u32): pipelineDesc.index_type = SG_INDEXTYPE_UINT32; break;
		default: AssertMsg(options->indexedVerticesSize == sizeof(u16) || options->indexedVerticesSize == sizeof(u32) || options->indexedVerticesSize == 0, "Invalid indices size!"); break;
	}
	pipelineDesc.cull_mode = options->cullingEnabled ? SG_CULLMODE_BACK : SG_CULLMODE_NONE;
	pipelineDesc.face_winding = SG_FACEWINDING_CW;
	
	result.handle = sg_make_pipeline(&pipelineDesc);
	if (result.handle.id == SG_INVALID_ID)
	{
		FreeStr8(result.arena, &result.name);
		result.error = Result_SokolError;
		ScratchEnd(scratch);
		return result;
	}
	
	ScratchEnd(scratch);
	result.error = Result_Success;
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _GFX_PIPELINE_H
