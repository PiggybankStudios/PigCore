/*
File:   gfx_system.h
Author: Taylor Robbins
Date:   01\30\2025
Description:
	** This file brings all the other graphics types together into an API that
	** we can use to track state and invoke rendering commands
	** (sometimes automatically depending on state changes)
*/

#ifndef _GFX_SYSTEM_H
#define _GFX_SYSTEM_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "struct/struct_vectors.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "struct/struct_string.h"
#include "struct/struct_color.h"
#include "struct/struct_var_array.h"
#include "misc/misc_result.h"
#include "gfx/gfx_vertices.h"
#include "gfx/gfx_sokol_include.h"
#include "gfx/gfx_vert_buffer.h"
#include "gfx/gfx_texture.h"
#include "gfx/gfx_shader.h"
#include "gfx/gfx_pipeline.h"

#if BUILD_WITH_SOKOL

typedef struct GfxSystemState GfxSystemState;
struct GfxSystemState
{
	bool depthWriteEnabled;
	bool depthTestEnabled;
	bool cullingEnabled;
	
	Shader* shader;
	Texture* texture;
	VertBuffer* vertBuffer;
	
	//This gets cleared whenever something that would need a different pipeline is changed
	//It then gets refilled with an existing or new pipeline when FlushSystemPipelineGen() is called
	GfxPipeline* pipeline;
	
	mat4 projectionMat;
	mat4 viewMat;
	mat4 worldMat;
	v4r tintColor;
	v4r sourceRec; //TODO: Change this to rec type!
};

typedef struct GfxSystem GfxSystem;
struct GfxSystem
{
	Arena* arena;
	VarArray pipelines; //GfxPipeline
	sg_bindings bindings;
	
	bool bindingsChanged;
	bool uniformsChanged;
	
	GfxSystemState state;
	
	uxx numPipelineChanges;
	uxx numBindingChanges;
	uxx numDrawCalls;
};


// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void ShutdownSokolGraphics();
	void InitSokolGraphics(sg_desc sokolGraphicsDesc);
	void FreeGfxSystem(GfxSystem* system);
	void InitGfxSystem(Arena* arena, GfxSystem* systemOut);
	PIG_CORE_INLINE GfxPipeline* FindSystemPipelineWithOptions(GfxSystem* system, const GfxPipelineOptions* options);
	PIG_CORE_INLINE GfxPipeline* FindOrAddSystemPipelineWithOptions(GfxSystem* system, const GfxPipelineOptions* options);
	void FlushSystemPipelineGen(GfxSystem* system);
	void FlushSystemBindings(GfxSystem* system);
	PIG_CORE_INLINE sg_swapchain CreateSokolSappSwapchain();
	PIG_CORE_INLINE void BeginSystemFrame(GfxSystem* system, sg_swapchain swapchain, Color32 clearColor, r32 clearDepth);
	PIG_CORE_INLINE void EndSystemFrame(GfxSystem* system);
	PIG_CORE_INLINE void DrawSystemVerticesEx(GfxSystem* system, uxx startVertex, uxx numVertices);
	PIG_CORE_INLINE void DrawSystemVertices(GfxSystem* system);
	PIG_CORE_INLINE void BindSystemShader(GfxSystem* system, Shader* shader);
	PIG_CORE_INLINE void BindSystemVertBuffer(GfxSystem* system, VertBuffer* buffer);
	PIG_CORE_INLINE void BindSystemTexture(GfxSystem* system, Texture* texture);
	PIG_CORE_INLINE void SetSystemProjectionMat(GfxSystem* system, mat4 matrix);
	PIG_CORE_INLINE void SetSystemViewMat(GfxSystem* system, mat4 matrix);
	PIG_CORE_INLINE void SetSystemWorldMat(GfxSystem* system, mat4 matrix);
	PIG_CORE_INLINE void SetSystemTintColorRaw(GfxSystem* system, v4r colorVec);
	PIG_CORE_INLINE void SetSystemTintColor(GfxSystem* system, Color32 color);
	PIG_CORE_INLINE void SetSystemSourceRecRaw(GfxSystem* system, v4r rectangle);
	PIG_CORE_INLINE void SetSystemSourceRec(GfxSystem* system, v4 rectangle);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void ShutdownSokolGraphics()
{
	sg_shutdown();
}
PEXP void InitSokolGraphics(sg_desc sokolGraphicsDesc)
{
	sg_setup(&sokolGraphicsDesc);
}

PEXP void FreeGfxSystem(GfxSystem* system)
{
	NotNull(system);
	if (system->arena != nullptr)
	{
		VarArrayLoop(&system->pipelines, pIndex)
		{
			VarArrayLoopGet(GfxPipeline, pipeline, &system->pipelines, pIndex);
			FreeGfxPipeline(pipeline);
		}
		FreeVarArray(&system->pipelines);
	}
	ClearPointer(system);
}

PEXP void InitGfxSystem(Arena* arena, GfxSystem* systemOut)
{
	NotNull(arena);
	NotNull(systemOut);
	ClearPointer(systemOut);
	systemOut->arena = arena;
	InitVarArray(GfxPipeline, &systemOut->pipelines, arena);
	systemOut->state.depthTestEnabled = true;
	systemOut->state.depthWriteEnabled = true;
	systemOut->state.cullingEnabled = true;
	systemOut->state.projectionMat = Mat4_Identity;
	systemOut->state.viewMat = Mat4_Identity;
	systemOut->state.worldMat = Mat4_Identity;
	systemOut->state.tintColor = NewV4r(1.0f, 1.0f, 1.0f, 1.0f);
	systemOut->state.sourceRec = NewV4r(0.0f, 0.0f, 1.0f, 1.0f);
}

PEXPI GfxPipeline* FindSystemPipelineWithOptions(GfxSystem* system, const GfxPipelineOptions* options)
{
	VarArrayLoop(&system->pipelines, pIndex)
	{
		VarArrayLoopGet(GfxPipeline, pipeline, &system->pipelines, pIndex);
		if (AreEqualGfxPipelineOptions(&pipeline->options, options)) { return pipeline; }
	}
	return nullptr;
}
PEXPI GfxPipeline* FindOrAddSystemPipelineWithOptions(GfxSystem* system, const GfxPipelineOptions* options)
{
	GfxPipeline* existingPipeline = FindSystemPipelineWithOptions(system, options);
	if (existingPipeline != nullptr) { return existingPipeline; }
	void* oldPipelinesPntr = system->pipelines.items;
	GfxPipeline* newPipeline = VarArrayAdd(GfxPipeline, &system->pipelines);
	NotNull(newPipeline);
	if (system->pipelines.items != oldPipelinesPntr) { system->state.pipeline = nullptr; } //invalidate the pointer stored in the state!
	*newPipeline = InitGfxPipeline(system->arena, StrLit("gfx_system_pipeline"), options);
	return newPipeline;
}

PEXP void FlushSystemPipelineGen(GfxSystem* system)
{
	NotNull(system);
	if (system->state.pipeline == nullptr && system->state.shader != nullptr && system->state.vertBuffer != nullptr)
	{
		GfxPipelineOptions pipelineOptions = ZEROED;
		FillGfxPipelineOptionsFromVertBuffer(&pipelineOptions, system->state.vertBuffer);
		pipelineOptions.shader = system->state.shader;
		pipelineOptions.depthWriteEnabled = system->state.depthWriteEnabled;
		pipelineOptions.depthTestEnabled = system->state.depthTestEnabled;
		pipelineOptions.cullingEnabled = system->state.cullingEnabled;
		system->state.pipeline = FindOrAddSystemPipelineWithOptions(system, &pipelineOptions);
		NotNull(system->state.pipeline);
		sg_apply_pipeline(system->state.pipeline->handle);
		IncrementUXX(system->numPipelineChanges);
		system->bindingsChanged = true;
		system->uniformsChanged = true;
		if (system->state.shader != nullptr)
		{
			for (uxx bIndex = 0; bIndex < MAX_NUM_SHADER_UNIFORM_BLOCKS; bIndex++)
			{
				if (system->state.shader->uniformBlocks[bIndex].value.length > 0)
				{
					system->state.shader->uniformBlocks[bIndex].valueChanged = true;
				}
			}
		}
	}
}

PEXP void FlushSystemBindings(GfxSystem* system)
{
	NotNull(system);
	if (system->bindingsChanged)
	{
		sg_apply_bindings(&system->bindings);
		IncrementUXX(system->numBindingChanges);
		system->bindingsChanged = false;
	}
}

PEXPI void BeginSystemFrame(GfxSystem* system, sg_swapchain swapchain, Color32 clearColor, r32 clearDepth)
{
	NotNull(system);
	
	v4r clearColorVec = ToV4rFromColor32(clearColor);
	sg_pass mainPass = {
		.action = {
			.colors[0] = {
				.load_action = SG_LOADACTION_CLEAR,
				.clear_value = { clearColorVec.R, clearColorVec.G, clearColorVec.B, clearColorVec.A }
			},
			.depth = {
				.load_action = SG_LOADACTION_CLEAR,
				.clear_value = clearDepth,
			},
		},
		.swapchain = swapchain,
	};
	sg_begin_pass(&mainPass);
	
	system->state.shader = nullptr;
	system->bindingsChanged = true;
	system->uniformsChanged = true;
	system->state.pipeline = nullptr;
}

PEXPI void EndSystemFrame(GfxSystem* system)
{
	NotNull(system);
	sg_end_pass();
}

PEXPI void DrawSystemVerticesEx(GfxSystem* system, uxx startVertex, uxx numVertices)
{
	NotNull(system->state.shader);
	NotNull(system->state.vertBuffer);
	FlushSystemPipelineGen(system);
	FlushSystemBindings(system);
	if (system->uniformsChanged)
	{
		ApplyShaderUniforms(system->state.shader);
		system->uniformsChanged = false;
	}
	sg_draw((int)startVertex, (int)numVertices, 1);
	IncrementUXX(system->numDrawCalls);
}
PEXPI void DrawSystemVertices(GfxSystem* system)
{
	NotNull(system->state.vertBuffer);
	DrawSystemVerticesEx(system, 0, system->state.vertBuffer->numVertices);
}

//TODO: 

// +--------------------------------------------------------------+
// |                   Bind Resource Functions                    |
// +--------------------------------------------------------------+
PEXPI void BindSystemShader(GfxSystem* system, Shader* shader)
{
	NotNull(system);
	if (system->state.shader != shader)
	{
		if (shader != nullptr)
		{
			SetShaderProjectionMat(shader, system->state.projectionMat);
			SetShaderViewMat(shader, system->state.viewMat);
			SetShaderWorldMat(shader, system->state.worldMat);
			SetShaderTintColorRaw(shader, system->state.tintColor);
			SetShaderSourceRecRaw(shader, system->state.sourceRec);
			if (system->state.texture != nullptr)
			{
				BindTextureAtIndex(&system->bindings, shader, system->state.texture, 0, 0);
				system->bindingsChanged = true;
			}
			for (uxx bIndex = 0; bIndex < MAX_NUM_SHADER_UNIFORM_BLOCKS; bIndex++)
			{
				if (shader->uniformBlocks[bIndex].value.length > 0)
				{
					shader->uniformBlocks[bIndex].valueChanged = true;
				}
			}
		}
		system->state.shader = shader;
		system->uniformsChanged = true;
		system->state.pipeline = nullptr;
	}
}
PEXPI void BindSystemVertBuffer(GfxSystem* system, VertBuffer* buffer)
{
	NotNull(system);
	if (system->state.vertBuffer != buffer)
	{
		if (buffer != nullptr) { BindVertBuffer(&system->bindings, buffer, 0); }
		else { system->bindings.vertex_buffers[0].id = SG_INVALID_ID; }
		system->bindingsChanged = true;
		system->state.vertBuffer = buffer;
		system->state.pipeline = nullptr; //TODO: We don't always need to invalidate the pipeline. We should probably only do this if the attributes are different between current and new VertBuffer!
	}
}
PEXPI void BindSystemTexture(GfxSystem* system, Texture* texture)
{
	NotNull(system);
	if (system->state.texture != texture)
	{
		if (system->state.shader != nullptr)
		{
			if (texture != nullptr) { BindTextureAtIndex(&system->bindings, system->state.shader, texture, 0, 0); }
			else { system->bindings.images[0].id = SG_INVALID_ID; system->bindings.samplers[0].id = SG_INVALID_ID; }
			system->bindingsChanged = true;
		}
		system->state.texture = texture;
	}
}

// +--------------------------------------------------------------+
// |                    Set Uniform Functions                     |
// +--------------------------------------------------------------+
PEXPI void SetSystemProjectionMat(GfxSystem* system, mat4 matrix)
{
	NotNull(system);
	NotNull(system->arena);
	if (!AreEqualMat4(matrix, system->state.projectionMat))
	{
		if (system->state.shader != nullptr) { SetShaderProjectionMat(system->state.shader, matrix); }
		system->state.projectionMat = matrix;
		system->uniformsChanged = true;
	}
}

PEXPI void SetSystemViewMat(GfxSystem* system, mat4 matrix)
{
	NotNull(system);
	NotNull(system->arena);
	if (!AreEqualMat4(matrix, system->state.viewMat))
	{
		if (system->state.shader != nullptr) { SetShaderViewMat(system->state.shader, matrix); }
		system->state.viewMat = matrix;
		system->uniformsChanged = true;
	}
}

PEXPI void SetSystemWorldMat(GfxSystem* system, mat4 matrix)
{
	NotNull(system);
	NotNull(system->arena);
	if (!AreEqualMat4(matrix, system->state.worldMat))
	{
		if (system->state.shader != nullptr) { SetShaderWorldMat(system->state.shader, matrix); }
		system->state.worldMat = matrix;
		system->uniformsChanged = true;
	}
}

PEXPI void SetSystemTintColorRaw(GfxSystem* system, v4r colorVec)
{
	NotNull(system);
	NotNull(system->arena);
	if (!AreEqualV4r(colorVec, system->state.tintColor))
	{
		if (system->state.shader != nullptr) { SetShaderTintColorRaw(system->state.shader, colorVec); }
		system->state.tintColor = colorVec;
		system->uniformsChanged = true;
	}
}
PEXPI void SetSystemTintColor(GfxSystem* system, Color32 color) { SetSystemTintColorRaw(system, ToV4rFromColor32(color)); }

PEXPI void SetSystemSourceRecRaw(GfxSystem* system, v4r rectangle)
{
	NotNull(system);
	NotNull(system->arena);
	if (!AreEqualV4r(rectangle, system->state.sourceRec))
	{
		if (system->state.shader != nullptr) { SetShaderSourceRecRaw(system->state.shader, rectangle); }
		system->state.sourceRec = rectangle;
		system->uniformsChanged = true;
	}
}
PEXPI void SetSystemSourceRec(GfxSystem* system, v4 rectangle) { SetSystemSourceRecRaw(system, ToV4rFrom4(rectangle)); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL

#endif //  _GFX_SYSTEM_H
