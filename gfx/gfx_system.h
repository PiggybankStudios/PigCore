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
#include "misc/misc_sokol_gfx_include.h"
#include "gfx/gfx_vert_buffer.h"
#include "gfx/gfx_texture.h"
#include "gfx/gfx_shader.h"
#include "gfx/gfx_pipeline.h"

#if BUILD_WITH_SOKOL_GFX

typedef struct GfxSystemState GfxSystemState;
struct GfxSystemState
{
	bool colorWriteEnabled;
	bool depthWriteEnabled;
	bool depthTestEnabled;
	bool cullingEnabled;
	GfxPipelineBlendMode blendMode;
	r32 depth; //for 2D rendering functions
	
	Shader* shader;
	Texture* textures[MAX_NUM_SHADER_IMAGES];
	VertBuffer* vertBuffer;
	
	//This gets cleared whenever something that would need a different pipeline is changed
	//It then gets refilled with an existing or new pipeline when GfxSystem_FlushPipelineGen() is called
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
	
	Texture pixelTexture;
	VertBuffer squareBuffer;
	
	bool frameStarted;
	sg_swapchain swapchain; //given to us in GfxSystem_BeginFrame
	
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
	PIG_CORE_INLINE GfxPipeline* GfxSystem_FindPipelineWithOptions(GfxSystem* system, const GfxPipelineOptions* options);
	PIG_CORE_INLINE GfxPipeline* GfxSystem_FindOrAddPipelineWithOptions(GfxSystem* system, const GfxPipelineOptions* options);
	void GfxSystem_FlushPipelineGen(GfxSystem* system);
	void GfxSystem_FlushBindings(GfxSystem* system);
	PIG_CORE_INLINE void GfxSystem_BeginFrame(GfxSystem* system, sg_swapchain swapchain, Color32 clearColor, r32 clearDepth);
	PIG_CORE_INLINE void GfxSystem_EndFrame(GfxSystem* system);
	PIG_CORE_INLINE void GfxSystem_DrawVerticesEx(GfxSystem* system, uxx startVertex, uxx numVertices);
	PIG_CORE_INLINE void GfxSystem_DrawVertices(GfxSystem* system);
	PIG_CORE_INLINE void GfxSystem_BindShader(GfxSystem* system, Shader* shader);
	PIG_CORE_INLINE void GfxSystem_BindVertBuffer(GfxSystem* system, VertBuffer* buffer);
	PIG_CORE_INLINE void GfxSystem_BindTextureAtIndex(GfxSystem* system, Texture* texture, uxx textureIndex);
	PIG_CORE_INLINE void GfxSystem_BindTexture(GfxSystem* system, Texture* texture);
	PIG_CORE_INLINE void GfxSystem_SetColorWriteEnabled(GfxSystem* system, bool colorWriteEnabled);
	PIG_CORE_INLINE void GfxSystem_SetDepthTestEnabled(GfxSystem* system, bool depthTestEnabled);
	PIG_CORE_INLINE void GfxSystem_SetDepthWriteEnabled(GfxSystem* system, bool depthWriteEnabled);
	PIG_CORE_INLINE void GfxSystem_SetCullingEnabled(GfxSystem* system, bool cullingEnabled);
	PIG_CORE_INLINE void GfxSystem_SetBlendMode(GfxSystem* system, GfxPipelineBlendMode blendMode);
	PIG_CORE_INLINE void GfxSystem_SetDepth(GfxSystem* system, r32 depth);
	PIG_CORE_INLINE void GfxSystem_SetProjectionMat(GfxSystem* system, mat4 matrix);
	PIG_CORE_INLINE void GfxSystem_SetViewMat(GfxSystem* system, mat4 matrix);
	PIG_CORE_INLINE void GfxSystem_SetWorldMat(GfxSystem* system, mat4 matrix);
	PIG_CORE_INLINE void GfxSystem_SetTintColorRaw(GfxSystem* system, v4r colorVec);
	PIG_CORE_INLINE void GfxSystem_SetTintColor(GfxSystem* system, Color32 color);
	PIG_CORE_INLINE void GfxSystem_SetSourceRecRaw(GfxSystem* system, v4r rectangle);
	PIG_CORE_INLINE void GfxSystem_SetSourceRec(GfxSystem* system, rec rectangle);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedRectangleEx(GfxSystem* system, rec rectangle, Color32 color, Texture* texture, rec sourceRec);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedRectangle(GfxSystem* system, rec rectangle, Color32 color, Texture* texture);
	PIG_CORE_INLINE void GfxSystem_DrawRectangle(GfxSystem* system, rec rectangle, Color32 color);
	PIG_CORE_INLINE void GfxSystem_DrawRectangleOutlineEx(GfxSystem* system, rec rectangle, r32 borderThickness, Color32 color, bool outside);
	PIG_CORE_INLINE void GfxSystem_DrawRectangleOutline(GfxSystem* system, rec rectangle, r32 borderThickness, Color32 color);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedObb2Ex(GfxSystem* system, obb2 boundingBox, Color32 color, Texture* texture, rec sourceRec);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedObb2(GfxSystem* system, obb2 boundingBox, Color32 color, Texture* texture);
	PIG_CORE_INLINE void GfxSystem_DrawObb2(GfxSystem* system, obb2 boundingBox, Color32 color);
	PIG_CORE_INLINE void GfxSystem_ClearDepthBuffer(GfxSystem* system, r32 clearDepth);
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
		FreeTexture(&system->pixelTexture);
		FreeVertBuffer(&system->squareBuffer);
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
	systemOut->state.colorWriteEnabled = true;
	systemOut->state.depthWriteEnabled = true;
	systemOut->state.depthTestEnabled = true;
	systemOut->state.cullingEnabled = true;
	systemOut->state.blendMode = GfxPipelineBlendMode_Normal;
	systemOut->state.depth = 1.0f;
	systemOut->state.projectionMat = Mat4_Identity;
	systemOut->state.viewMat = Mat4_Identity;
	systemOut->state.worldMat = Mat4_Identity;
	systemOut->state.tintColor = NewV4r(1.0f, 1.0f, 1.0f, 1.0f);
	systemOut->state.sourceRec = NewV4r(0.0f, 0.0f, 1.0f, 1.0f);
	
	Color32 pixel = White;
	systemOut->pixelTexture = InitTexture(arena, StrLit("pixel"), V2i_One, &pixel, TextureFlag_IsRepeating|TextureFlag_NoMipmaps);
	
	Vertex2D squareVertices[] = {
		{ .X=0.0f, .Y=0.0f,   .tX=0.0f, .tY=0.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		{ .X=1.0f, .Y=0.0f,   .tX=1.0f, .tY=0.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		{ .X=0.0f, .Y=1.0f,   .tX=0.0f, .tY=1.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		
		{ .X=1.0f, .Y=1.0f,   .tX=1.0f, .tY=1.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		{ .X=0.0f, .Y=1.0f,   .tX=0.0f, .tY=1.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		{ .X=1.0f, .Y=0.0f,   .tX=1.0f, .tY=0.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
	};
	systemOut->squareBuffer = InitVertBuffer2D(arena, StrLit("square"), VertBufferUsage_Static, ArrayCount(squareVertices), &squareVertices[0], false);
	Assert(systemOut->squareBuffer.error == Result_Success);
}

PEXPI GfxPipeline* GfxSystem_FindPipelineWithOptions(GfxSystem* system, const GfxPipelineOptions* options)
{
	VarArrayLoop(&system->pipelines, pIndex)
	{
		VarArrayLoopGet(GfxPipeline, pipeline, &system->pipelines, pIndex);
		if (AreEqualGfxPipelineOptions(&pipeline->options, options)) { return pipeline; }
	}
	return nullptr;
}
PEXPI GfxPipeline* GfxSystem_FindOrAddPipelineWithOptions(GfxSystem* system, const GfxPipelineOptions* options)
{
	GfxPipeline* existingPipeline = GfxSystem_FindPipelineWithOptions(system, options);
	if (existingPipeline != nullptr) { return existingPipeline; }
	void* oldPipelinesPntr = system->pipelines.items;
	GfxPipeline* newPipeline = VarArrayAdd(GfxPipeline, &system->pipelines);
	NotNull(newPipeline);
	if (system->pipelines.items != oldPipelinesPntr) { system->state.pipeline = nullptr; } //invalidate the pointer stored in the state!
	*newPipeline = InitGfxPipeline(system->arena, StrLit("gfx_system_pipeline"), options);
	return newPipeline;
}

PEXP void GfxSystem_FlushPipelineGen(GfxSystem* system)
{
	NotNull(system);
	if (system->state.pipeline == nullptr && system->state.shader != nullptr && system->state.vertBuffer != nullptr)
	{
		GfxPipelineOptions pipelineOptions = ZEROED;
		FillGfxPipelineOptionsFromVertBuffer(&pipelineOptions, system->state.vertBuffer);
		pipelineOptions.shader = system->state.shader;
		pipelineOptions.colorWriteEnabled = system->state.colorWriteEnabled;
		pipelineOptions.depthWriteEnabled = system->state.depthWriteEnabled;
		pipelineOptions.depthTestEnabled = system->state.depthTestEnabled;
		pipelineOptions.cullingEnabled = system->state.cullingEnabled;
		pipelineOptions.blendMode = system->state.blendMode;
		system->state.pipeline = GfxSystem_FindOrAddPipelineWithOptions(system, &pipelineOptions);
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

PEXP void GfxSystem_FlushBindings(GfxSystem* system)
{
	NotNull(system);
	if (system->bindingsChanged)
	{
		sg_apply_bindings(&system->bindings);
		IncrementUXX(system->numBindingChanges);
		system->bindingsChanged = false;
	}
}

PEXPI void GfxSystem_BeginFrame(GfxSystem* system, sg_swapchain swapchain, Color32 clearColor, r32 clearDepth)
{
	NotNull(system);
	Assert(!system->frameStarted);
	system->swapchain = swapchain;
	
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
		.swapchain = system->swapchain,
		.label = "mainPass",
	};
	sg_begin_pass(&mainPass);
	
	system->state.shader = nullptr;
	system->bindingsChanged = true;
	system->uniformsChanged = true;
	system->state.pipeline = nullptr;
	
	system->frameStarted = true;
}

PEXPI void GfxSystem_EndFrame(GfxSystem* system)
{
	NotNull(system);
	Assert(system->frameStarted);
	sg_end_pass();
	
	system->frameStarted = false;
}

PEXPI void GfxSystem_DrawVerticesEx(GfxSystem* system, uxx startVertex, uxx numVertices)
{
	NotNull(system->state.shader);
	NotNull(system->state.vertBuffer);
	GfxSystem_FlushPipelineGen(system);
	GfxSystem_FlushBindings(system);
	if (system->uniformsChanged)
	{
		ApplyShaderUniforms(system->state.shader);
		system->uniformsChanged = false;
	}
	sg_draw((int)startVertex, (int)numVertices, 1);
	IncrementUXX(system->numDrawCalls);
}
PEXPI void GfxSystem_DrawVertices(GfxSystem* system)
{
	NotNull(system->state.vertBuffer);
	GfxSystem_DrawVerticesEx(system, 0, system->state.vertBuffer->numVertices);
}

//TODO: 

// +--------------------------------------------------------------+
// |                   Bind Resource Functions                    |
// +--------------------------------------------------------------+
PEXPI void GfxSystem_BindShader(GfxSystem* system, Shader* shader)
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
			for (uxx tIndex = 0; tIndex < MAX_NUM_SHADER_IMAGES; tIndex++)
			{
				if (system->state.textures[tIndex] != nullptr)
				{
					BindTextureAtIndex(&system->bindings, shader, system->state.textures[tIndex], tIndex, tIndex);
				}
			}
			for (uxx bIndex = 0; bIndex < MAX_NUM_SHADER_UNIFORM_BLOCKS; bIndex++)
			{
				if (shader->uniformBlocks[bIndex].value.length > 0)
				{
					shader->uniformBlocks[bIndex].valueChanged = true;
				}
			}
			system->bindingsChanged = true;
		}
		system->state.shader = shader;
		system->uniformsChanged = true;
		system->state.pipeline = nullptr;
	}
}
PEXPI void GfxSystem_BindVertBuffer(GfxSystem* system, VertBuffer* buffer)
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
PEXPI void GfxSystem_BindTextureAtIndex(GfxSystem* system, Texture* texture, uxx textureIndex)
{
	NotNull(system);
	Assert(textureIndex < MAX_NUM_SHADER_IMAGES);
	if (system->state.textures[textureIndex] != texture)
	{
		if (system->state.shader != nullptr)
		{
			if (texture != nullptr) { BindTextureAtIndex(&system->bindings, system->state.shader, texture, textureIndex, textureIndex); }
			else { system->bindings.images[textureIndex].id = SG_INVALID_ID; system->bindings.samplers[textureIndex].id = SG_INVALID_ID; }
			system->bindingsChanged = true;
		}
		system->state.textures[textureIndex] = texture;
	}
}
PEXPI void GfxSystem_BindTexture(GfxSystem* system, Texture* texture)
{
	GfxSystem_BindTextureAtIndex(system, texture, 0);
}

// +--------------------------------------------------------------+
// |               Change Pipeline Option Functions               |
// +--------------------------------------------------------------+
PEXPI void GfxSystem_SetColorWriteEnabled(GfxSystem* system, bool colorWriteEnabled)
{
	NotNull(system);
	NotNull(system->arena);
	if (colorWriteEnabled != system->state.colorWriteEnabled)
	{
		system->state.colorWriteEnabled = colorWriteEnabled;
		system->state.pipeline = nullptr;
	}
}
PEXPI void GfxSystem_SetDepthTestEnabled(GfxSystem* system, bool depthTestEnabled)
{
	NotNull(system);
	NotNull(system->arena);
	if (depthTestEnabled != system->state.depthTestEnabled)
	{
		system->state.depthTestEnabled = depthTestEnabled;
		system->state.pipeline = nullptr;
	}
}
PEXPI void GfxSystem_SetDepthWriteEnabled(GfxSystem* system, bool depthWriteEnabled)
{
	NotNull(system);
	NotNull(system->arena);
	if (depthWriteEnabled != system->state.depthWriteEnabled)
	{
		system->state.depthWriteEnabled = depthWriteEnabled;
		system->state.pipeline = nullptr;
	}
}
PEXPI void GfxSystem_SetCullingEnabled(GfxSystem* system, bool cullingEnabled)
{
	NotNull(system);
	NotNull(system->arena);
	if (cullingEnabled != system->state.cullingEnabled)
	{
		system->state.cullingEnabled = cullingEnabled;
		system->state.pipeline = nullptr;
	}
}
PEXPI void GfxSystem_SetBlendMode(GfxSystem* system, GfxPipelineBlendMode blendMode)
{
	NotNull(system);
	NotNull(system->arena);
	if (blendMode != system->state.blendMode)
	{
		system->state.blendMode = blendMode;
		system->state.pipeline = nullptr;
	}
}

// +--------------------------------------------------------------+
// |            Set Non-Uniform State Value Functions             |
// +--------------------------------------------------------------+
PEXPI void GfxSystem_SetDepth(GfxSystem* system, r32 depth)
{
	NotNull(system);
	NotNull(system->arena);
	if (depth != system->state.depth)
	{
		system->state.depth = depth;
		system->state.pipeline = nullptr;
	}
}

// +--------------------------------------------------------------+
// |                    Set Uniform Functions                     |
// +--------------------------------------------------------------+
PEXPI void GfxSystem_SetProjectionMat(GfxSystem* system, mat4 matrix)
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

PEXPI void GfxSystem_SetViewMat(GfxSystem* system, mat4 matrix)
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

PEXPI void GfxSystem_SetWorldMat(GfxSystem* system, mat4 matrix)
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

PEXPI void GfxSystem_SetTintColorRaw(GfxSystem* system, v4r colorVec)
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
PEXPI void GfxSystem_SetTintColor(GfxSystem* system, Color32 color) { GfxSystem_SetTintColorRaw(system, ToV4rFromColor32(color)); }

PEXPI void GfxSystem_SetSourceRecRaw(GfxSystem* system, v4r rectangleV4r)
{
	NotNull(system);
	NotNull(system->arena);
	if (!AreEqualV4r(rectangleV4r, system->state.sourceRec))
	{
		if (system->state.shader != nullptr) { SetShaderSourceRecRaw(system->state.shader, rectangleV4r); }
		system->state.sourceRec = rectangleV4r;
		system->uniformsChanged = true;
	}
}
PEXPI void GfxSystem_SetSourceRec(GfxSystem* system, rec rectangle) { GfxSystem_SetSourceRecRaw(system, ToV4rFromRec(rectangle)); }

// +--------------------------------------------------------------+
// |                      Drawing Functions                       |
// +--------------------------------------------------------------+
PEXPI void GfxSystem_DrawTexturedRectangleEx(GfxSystem* system, rec rectangle, Color32 color, Texture* texture, rec sourceRec)
{
	if (texture != nullptr)
	{
		GfxSystem_BindTexture(system, texture);
		GfxSystem_SetSourceRec(system, sourceRec);
	}
	else
	{
		GfxSystem_BindTexture(system, &system->pixelTexture);
		GfxSystem_SetSourceRec(system, Rec_Default);
	}
	
	mat4 worldMat = Mat4_Identity;
	TransformMat4(&worldMat, MakeScaleXYZMat4(rectangle.Width, rectangle.Height, 1.0f));
	TransformMat4(&worldMat, MakeTranslateXYZMat4(rectangle.X, rectangle.Y, system->state.depth));
	GfxSystem_SetWorldMat(system, worldMat);
	
	GfxSystem_SetTintColor(system, color);
	
	GfxSystem_BindVertBuffer(system, &system->squareBuffer);
	GfxSystem_DrawVertices(system);
}
PEXPI void GfxSystem_DrawTexturedRectangle(GfxSystem* system, rec rectangle, Color32 color, Texture* texture)
{
	rec sourceRec = (texture != nullptr) ? NewRec(0, 0, (r32)texture->Width, (r32)texture->Height) : Rec_Default_Const;
	GfxSystem_DrawTexturedRectangleEx(system, rectangle, color, texture, sourceRec);
}
PEXPI void GfxSystem_DrawRectangle(GfxSystem* system, rec rectangle, Color32 color)
{
	GfxSystem_DrawTexturedRectangleEx(system, rectangle, color, nullptr, Rec_Default_Const);
}

PEXPI void GfxSystem_DrawRectangleOutlineEx(GfxSystem* system, rec rectangle, r32 borderThickness, Color32 color, bool outside)
{
	GfxSystem_DrawRectangle(system, NewRec(rectangle.X, rectangle.Y - borderThickness, rectangle.Width, borderThickness), color);
	GfxSystem_DrawRectangle(system, NewRec(rectangle.X, rectangle.Y + rectangle.Height, rectangle.Width, borderThickness), color);
	GfxSystem_DrawRectangle(system, NewRec(rectangle.X - borderThickness, rectangle.Y, borderThickness, rectangle.Height), color);
	GfxSystem_DrawRectangle(system, NewRec(rectangle.X + rectangle.Width, rectangle.Y, borderThickness, rectangle.Height), color);
}
PEXPI void GfxSystem_DrawRectangleOutline(GfxSystem* system, rec rectangle, r32 borderThickness, Color32 color)
{
	GfxSystem_DrawRectangleOutlineEx(system, rectangle, borderThickness, color, true);
}

PEXPI void GfxSystem_DrawTexturedObb2Ex(GfxSystem* system, obb2 boundingBox, Color32 color, Texture* texture, rec sourceRec)
{
	if (texture != nullptr)
	{
		GfxSystem_BindTexture(system, texture);
		GfxSystem_SetSourceRec(system, sourceRec);
	}
	else
	{
		GfxSystem_BindTexture(system, &system->pixelTexture);
		GfxSystem_SetSourceRec(system, Rec_Default);
	}
	
	mat4 worldMat = Mat4_Identity;
	TransformMat4(&worldMat, MakeTranslateXYZMat4(-0.5f, -0.5f, 0.0f));
	TransformMat4(&worldMat, MakeScaleXYZMat4(boundingBox.Width, boundingBox.Height, 1.0f));
	TransformMat4(&worldMat, MakeRotateZMat4(boundingBox.Rotation));
	TransformMat4(&worldMat, MakeTranslateXYZMat4(boundingBox.X, boundingBox.Y, system->state.depth));
	GfxSystem_SetWorldMat(system, worldMat);
	
	GfxSystem_SetTintColor(system, color);
	
	GfxSystem_BindVertBuffer(system, &system->squareBuffer);
	GfxSystem_DrawVertices(system);
}
PEXPI void GfxSystem_DrawTexturedObb2(GfxSystem* system, obb2 boundingBox, Color32 color, Texture* texture)
{
	rec sourceRec = (texture != nullptr) ? NewRec(0, 0, (r32)texture->Width, (r32)texture->Height) : Rec_Default_Const;
	GfxSystem_DrawTexturedObb2Ex(system, boundingBox, color, texture, sourceRec);
}
PEXPI void GfxSystem_DrawObb2(GfxSystem* system, obb2 boundingBox, Color32 color)
{
	GfxSystem_DrawTexturedObb2Ex(system, boundingBox, color, nullptr, Rec_Default_Const);
}

PEXPI void GfxSystem_ClearDepthBuffer(GfxSystem* system, r32 clearDepth)
{
	NotNull(system);
	Assert(system->frameStarted);
	NotNull(system->state.shader);
	
	mat4 oldProjectionMat = system->state.projectionMat;
	mat4 oldViewMat = system->state.viewMat;
	bool oldColorWriteEnabled = system->state.colorWriteEnabled;
	bool oldDepthWriteEnabled = system->state.depthWriteEnabled;
	bool oldDepthTestEnabled = system->state.depthTestEnabled;
	bool oldCullingEnabled = system->state.cullingEnabled;
	r32 oldDepth = system->state.depth;
	
	GfxSystem_SetProjectionMat(system, Mat4_Identity);
	GfxSystem_SetViewMat(system, Mat4_Identity);
	GfxSystem_SetColorWriteEnabled(system, false);
	GfxSystem_SetDepthTestEnabled(system, false);
	GfxSystem_SetDepthWriteEnabled(system, true);
	GfxSystem_SetCullingEnabled(system, false);
	GfxSystem_SetDepth(system, 1.0f);
	
	GfxSystem_DrawRectangle(system, NewRec(-1, -1, 2, 2), MonokaiPurple);
	
	GfxSystem_SetDepth(system, oldDepth);
	GfxSystem_SetCullingEnabled(system, oldCullingEnabled);
	GfxSystem_SetColorWriteEnabled(system, oldColorWriteEnabled);
	GfxSystem_SetDepthTestEnabled(system, oldDepthWriteEnabled);
	GfxSystem_SetDepthWriteEnabled(system, oldDepthTestEnabled);
	GfxSystem_SetProjectionMat(system, oldProjectionMat);
	GfxSystem_SetViewMat(system, oldViewMat);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _GFX_SYSTEM_H
