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
#include "gfx/gfx_font.h"
#include "gfx/gfx_font_flow.h"
#include "gfx/gfx_pipeline.h"

#if BUILD_WITH_SOKOL_GFX

#define GFX_SYSTEM_CIRCLE_NUM_SIDES 36 // aka 10 degree increments

typedef struct GfxSystemState GfxSystemState;
struct GfxSystemState
{
	bool colorWriteEnabled;
	bool depthWriteEnabled;
	bool depthTestEnabled;
	bool cullingEnabled;
	uxx indexedVerticesSize;
	GfxPipelineBlendMode blendMode;
	r32 depth; //for 2D rendering functions
	reci clipRec;
	
	Shader* shader;
	Texture* textures[MAX_NUM_SHADER_IMAGES];
	VertBuffer* vertBuffer;
	
	PigFont* font;
	r32 fontSize;
	u8 fontStyleFlags;
	v2 alignPixelSize;
	
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
	VertBuffer circleBuffer;
	FontFlow prevFontFlow;
	
	bool frameStarted;
	v2i screenSize;
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
	PIG_CORE_INLINE void GfxSystem_BeginFrame(GfxSystem* system, sg_swapchain swapchain, v2i screenSize, Color32 clearColor, r32 clearDepth);
	PIG_CORE_INLINE void GfxSystem_EndFrame(GfxSystem* system);
	PIG_CORE_INLINE void GfxSystem_DrawVerticesEx(GfxSystem* system, uxx startVertexOrIndex, uxx numVerticesOrIndices);
	PIG_CORE_INLINE void GfxSystem_DrawVertices(GfxSystem* system);
	PIG_CORE_INLINE void GfxSystem_BindShader(GfxSystem* system, Shader* shader);
	PIG_CORE_INLINE void GfxSystem_BindVertBuffer(GfxSystem* system, VertBuffer* buffer);
	PIG_CORE_INLINE void GfxSystem_BindTextureAtIndex(GfxSystem* system, Texture* texture, uxx textureIndex);
	PIG_CORE_INLINE void GfxSystem_BindTexture(GfxSystem* system, Texture* texture);
	PIG_CORE_INLINE void GfxSystem_BindFontEx(GfxSystem* system, PigFont* font, r32 fontSize, u8 fontStyleFlags);
	PIG_CORE_INLINE void GfxSystem_BindFontAtSize(GfxSystem* system, PigFont* font, r32 fontSize);
	PIG_CORE_INLINE void GfxSystem_BindFont(GfxSystem* system, PigFont* font);
	PIG_CORE_INLINE void GfxSystem_SetClipRec(GfxSystem* system, reci clipRec);
	PIG_CORE_INLINE void GfxSystem_DisableClipRec(GfxSystem* system);
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
	void GfxSystem_DrawTexturedRectangleEx(GfxSystem* system, rec rectangle, Color32 color, Texture* texture, rec sourceRec);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedRectangle(GfxSystem* system, rec rectangle, Color32 color, Texture* texture);
	PIG_CORE_INLINE void GfxSystem_DrawRectangle(GfxSystem* system, rec rectangle, Color32 color);
	PIG_CORE_INLINE void GfxSystem_DrawRectangleOutlineSidesEx(GfxSystem* system, rec rectangle, r32 leftThickness, r32 rightThickness, r32 topThickness, r32 bottomThickness, Color32 color, bool outside);
	PIG_CORE_INLINE void GfxSystem_DrawRectangleOutlineSides(GfxSystem* system, rec rectangle, r32 leftThickness, r32 rightThickness, r32 topThickness, r32 bottomThickness, Color32 color);
	PIG_CORE_INLINE void GfxSystem_DrawRectangleOutlineEx(GfxSystem* system, rec rectangle, r32 borderThickness, Color32 color, bool outside);
	PIG_CORE_INLINE void GfxSystem_DrawRectangleOutline(GfxSystem* system, rec rectangle, r32 borderThickness, Color32 color);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedObb2Ex(GfxSystem* system, obb2 boundingBox, Color32 color, Texture* texture, rec sourceRec);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedObb2(GfxSystem* system, obb2 boundingBox, Color32 color, Texture* texture);
	PIG_CORE_INLINE void GfxSystem_DrawObb2(GfxSystem* system, obb2 boundingBox, Color32 color);
	void GfxSystem_DrawTexturedCirclePieceEx(GfxSystem* system, Circle circle, r32 angleMin, r32 angleMax, Color32 color, Texture* texture, rec sourceRec);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedCirclePiece(GfxSystem* system, Circle circle, r32 angleMin, r32 angleMax, Color32 color, Texture* texture);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedCircleEx(GfxSystem* system, Circle circle, Color32 color, Texture* texture, rec sourceRec);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedCircle(GfxSystem* system, Circle circle, Color32 color, Texture* texture);
	PIG_CORE_INLINE void GfxSystem_DrawCirclePiece(GfxSystem* system, Circle circle, r32 angleMin, r32 angleMax, Color32 color);
	PIG_CORE_INLINE void GfxSystem_DrawCircle(GfxSystem* system, Circle circle, Color32 color);
	void GfxSystem_DrawTexturedRoundedRectangleEx(GfxSystem* system, rec rectangle, r32 radiusTL, r32 radiusTR, r32 radiusBR, r32 radiusBL, Color32 color, Texture* texture, rec sourceRec);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedRoundedRectangle(GfxSystem* system, rec rectangle, r32 radius, Color32 color, Texture* texture, rec sourceRec);
	PIG_CORE_INLINE void GfxSystem_DrawRoundedRectangleEx(GfxSystem* system, rec rectangle, r32 radiusTL, r32 radiusTR, r32 radiusBR, r32 radiusBL, Color32 color);
	PIG_CORE_INLINE void GfxSystem_DrawRoundedRectangle(GfxSystem* system, rec rectangle, r32 radius, Color32 color);
	PIG_CORE_INLINE void GfxSystem_ClearDepthBuffer(GfxSystem* system, r32 clearDepth);
	Result GfxSystem_DrawTextWithFont(GfxSystem* system, PigFont* font, r32 fontSize, u8 styleFlags, Str8 text, v2 position, Color32 color);
	PIG_CORE_INLINE Result GfxSystem_DrawTextAtSize(GfxSystem* system, r32 fontSize, Str8 text, v2 position, Color32 color);
	PIG_CORE_INLINE Result GfxSystem_DrawTextBold(GfxSystem* system, Str8 text, v2 position, Color32 color);
	PIG_CORE_INLINE Result GfxSystem_DrawTextItalic(GfxSystem* system, Str8 text, v2 position, Color32 color);
	PIG_CORE_INLINE Result GfxSystem_DrawText(GfxSystem* system, Str8 text, v2 position, Color32 color);
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
		FreeVertBuffer(&system->circleBuffer);
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
	systemOut->state.indexedVerticesSize = 0;
	systemOut->state.blendMode = GfxPipelineBlendMode_Normal;
	systemOut->state.depth = 1.0f;
	systemOut->state.clipRec = Reci_Default;
	systemOut->state.alignPixelSize = V2_One;
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
	
	Vertex2D circleVertices[GFX_SYSTEM_CIRCLE_NUM_SIDES*3];
	for (uxx sIndex = 0; sIndex < GFX_SYSTEM_CIRCLE_NUM_SIDES; sIndex++)
	{
		r32 angle0 = (r32)(sIndex+0) * (TwoPi32 / GFX_SYSTEM_CIRCLE_NUM_SIDES);
		r32 angle1 = (r32)(sIndex+1) * (TwoPi32 / GFX_SYSTEM_CIRCLE_NUM_SIDES);
		v2 positionAndTexCoord0 = NewV2(0.5f + CosR32(angle0)*0.5f, 0.5f + SinR32(angle0)*0.5f);
		v2 positionAndTexCoord1 = NewV2(0.5f + CosR32(angle1)*0.5f, 0.5f + SinR32(angle1)*0.5f);
		circleVertices[sIndex*3 + 0] = NewVertex2D(positionAndTexCoord0, positionAndTexCoord0, NewV4(1, 1, 1, 1));
		circleVertices[sIndex*3 + 1] = NewVertex2D(positionAndTexCoord1, positionAndTexCoord1, NewV4(1, 1, 1, 1));
		circleVertices[sIndex*3 + 2] = NewVertex2D(V2_Half, V2_Half, NewV4(1, 1, 1, 1));
	}
	systemOut->circleBuffer = InitVertBuffer2D(arena, StrLit("circle"), VertBufferUsage_Static, ArrayCount(circleVertices), &circleVertices[0], false);
	Assert(systemOut->circleBuffer.error == Result_Success);
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
		pipelineOptions.indexedVerticesSize = system->state.indexedVerticesSize;
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

PEXPI void GfxSystem_BeginFrame(GfxSystem* system, sg_swapchain swapchain, v2i screenSize, Color32 clearColor, r32 clearDepth)
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
	
	system->state.clipRec = NewReciV(V2i_Zero, screenSize);
	system->screenSize = screenSize;
	
	system->state.shader = nullptr;
	system->state.vertBuffer = nullptr;
	for (uxx tIndex = 0; tIndex < MAX_NUM_SHADER_IMAGES; tIndex++) { system->state.textures[tIndex] = nullptr; }
	system->state.font = nullptr;
	system->state.pipeline = nullptr;
	system->bindingsChanged = true;
	system->uniformsChanged = true;
	
	system->frameStarted = true;
}

PEXPI void GfxSystem_EndFrame(GfxSystem* system)
{
	NotNull(system);
	Assert(system->frameStarted);
	sg_end_pass();
	
	system->frameStarted = false;
}

PEXPI void GfxSystem_DrawVerticesEx(GfxSystem* system, uxx startVertexOrIndex, uxx numVerticesOrIndices)
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
	sg_draw((int)startVertexOrIndex, (int)numVerticesOrIndices, 1);
	IncrementUXX(system->numDrawCalls);
}
PEXPI void GfxSystem_DrawVertices(GfxSystem* system)
{
	NotNull(system->state.vertBuffer);
	if (system->state.vertBuffer->hasIndices)
	{
		GfxSystem_DrawVerticesEx(system, 0, system->state.vertBuffer->numIndices);
	}
	else
	{
		GfxSystem_DrawVerticesEx(system, 0, system->state.vertBuffer->numVertices);
	}
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
		uxx newIndexedVerticesSize = (buffer->hasIndices ? buffer->indexSize : 0);
		if (system->state.indexedVerticesSize != newIndexedVerticesSize)
		{
			system->state.indexedVerticesSize = newIndexedVerticesSize;
			system->state.pipeline = nullptr;
		}
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

PEXPI void GfxSystem_BindFontEx(GfxSystem* system, PigFont* font, r32 fontSize, u8 fontStyleFlags)
{
	NotNull(system);
	if (system->state.font != font) { system->state.font = font; }
	if (system->state.fontSize != fontSize) { system->state.fontSize = fontSize; }
	if (system->state.fontStyleFlags != fontStyleFlags) { system->state.fontStyleFlags = fontStyleFlags; }
}
PEXPI void GfxSystem_BindFontAtSize(GfxSystem* system, PigFont* font, r32 fontSize)
{
	u8 fontStyleFlags = FontStyleFlag_None;
	if (font != nullptr && font->atlases.length > 0)
	{
		//TODO: This logic is wrong for choosing the styleFlags, we really should find the closest match similar to how GetFontGlyphForCodepoint works
		FontAtlas* firstAtlas = VarArrayGetFirst(FontAtlas, &font->atlases);
		fontStyleFlags = firstAtlas->styleFlags;
	}
	GfxSystem_BindFontEx(system, font, fontSize, fontStyleFlags);
}
PEXPI void GfxSystem_BindFont(GfxSystem* system, PigFont* font)
{
	r32 fontSize = 16.0f;
	u8 fontStyleFlags = FontStyleFlag_None;
	if (font != nullptr && font->atlases.length > 0)
	{
		FontAtlas* firstAtlas = VarArrayGetFirst(FontAtlas, &font->atlases);
		fontSize = firstAtlas->fontSize;
		fontStyleFlags = firstAtlas->styleFlags;
	}
	GfxSystem_BindFontEx(system, font, fontSize, fontStyleFlags);
}

// +--------------------------------------------------------------+
// |               Change Pipeline Option Functions               |
// +--------------------------------------------------------------+
PEXPI void GfxSystem_SetClipRec(GfxSystem* system, reci clipRec)
{
	NotNull(system);
	if (!AreEqual(system->state.clipRec, clipRec))
	{
		sg_apply_scissor_rect(clipRec.X, clipRec.Y, clipRec.Width, clipRec.Height, true);
		system->state.clipRec = clipRec;
	}
}
PEXPI void GfxSystem_DisableClipRec(GfxSystem* system) { GfxSystem_SetClipRec(system, NewReciV(V2i_Zero, system->screenSize)); }

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
PEXP void GfxSystem_DrawTexturedRectangleEx(GfxSystem* system, rec rectangle, Color32 color, Texture* texture, rec sourceRec)
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

PEXPI void GfxSystem_DrawRectangleOutlineSidesEx(GfxSystem* system, rec rectangle, r32 leftThickness, r32 rightThickness, r32 topThickness, r32 bottomThickness, Color32 color, bool outside)
{
	if (outside)
	{
		if (topThickness > 0) { GfxSystem_DrawRectangle(system, NewRec(rectangle.X, rectangle.Y - topThickness, rectangle.Width, topThickness), color); } //top 
		if (bottomThickness > 0) { GfxSystem_DrawRectangle(system, NewRec(rectangle.X, rectangle.Y + rectangle.Height, rectangle.Width, bottomThickness), color); } //bottom
		if (leftThickness > 0) { GfxSystem_DrawRectangle(system, NewRec(rectangle.X - leftThickness, rectangle.Y - topThickness, leftThickness, rectangle.Height + topThickness+bottomThickness), color); } //left
		if (rightThickness > 0) { GfxSystem_DrawRectangle(system, NewRec(rectangle.X + rectangle.Width, rectangle.Y - topThickness, rightThickness, rectangle.Height + topThickness+bottomThickness), color); } //right
	}
	else
	{
		if (topThickness > 0) { GfxSystem_DrawRectangle(system, NewRec(rectangle.X, rectangle.Y, rectangle.Width, topThickness), color); } //top
		if (bottomThickness > 0) { GfxSystem_DrawRectangle(system, NewRec(rectangle.X, rectangle.Y + rectangle.Height - bottomThickness, rectangle.Width, bottomThickness), color); } //bottom
		if (leftThickness > 0) { GfxSystem_DrawRectangle(system, NewRec(rectangle.X, rectangle.Y + topThickness, leftThickness, rectangle.Height - (topThickness+bottomThickness)), color); } //left
		if (rightThickness > 0) { GfxSystem_DrawRectangle(system, NewRec(rectangle.X + rectangle.Width - rightThickness, rectangle.Y + topThickness, rightThickness, rectangle.Height - (topThickness+bottomThickness)), color); } //right
	}
}
PEXPI void GfxSystem_DrawRectangleOutlineSides(GfxSystem* system, rec rectangle, r32 leftThickness, r32 rightThickness, r32 topThickness, r32 bottomThickness, Color32 color)
{
	GfxSystem_DrawRectangleOutlineSidesEx(system, rectangle, leftThickness, rightThickness, topThickness, bottomThickness, color, true);
}
PEXPI void GfxSystem_DrawRectangleOutlineEx(GfxSystem* system, rec rectangle, r32 borderThickness, Color32 color, bool outside)
{
	GfxSystem_DrawRectangleOutlineSidesEx(system, rectangle, borderThickness, borderThickness, borderThickness, borderThickness, color, outside);
}
PEXPI void GfxSystem_DrawRectangleOutline(GfxSystem* system, rec rectangle, r32 borderThickness, Color32 color)
{
	GfxSystem_DrawRectangleOutlineSidesEx(system, rectangle, borderThickness, borderThickness, borderThickness, borderThickness, color, true);
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

PEXP void GfxSystem_DrawTexturedCirclePieceEx(GfxSystem* system, Circle circle, r32 angleMin, r32 angleMax, Color32 color, Texture* texture, rec sourceRec)
{
	NotNull(system);
	
	uxx startSideIndex = (uxx)RoundR32i((AngleFixR32(angleMin) / TwoPi32) * GFX_SYSTEM_CIRCLE_NUM_SIDES);
	uxx endSideIndex = (uxx)RoundR32i((AngleFixR32(angleMax) / TwoPi32) * GFX_SYSTEM_CIRCLE_NUM_SIDES);
	if (endSideIndex == startSideIndex) { return; } //TODO: Should we render anything when the angle is too small? Maybe always render a segment?
	if (endSideIndex < startSideIndex)
	{
		//NOTE: To draw an arc that passes over 0/TwoPi angle, we really need to do two draw calls for either side. So we just recurse one level to accomplish this cleanly
		GfxSystem_DrawTexturedCirclePieceEx(system, circle, angleMin, TwoPi32-DEFAULT_R32_TOLERANCE, color, texture, sourceRec);
		GfxSystem_DrawTexturedCirclePieceEx(system, circle, 0, angleMax, color, texture, sourceRec);
		return;
	}
	
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
	TransformMat4(&worldMat, MakeScaleXYZMat4(circle.R*2.0f, circle.R*2.0f, 1.0f));
	TransformMat4(&worldMat, MakeTranslateXYZMat4(circle.X - circle.R, circle.Y - circle.R, system->state.depth));
	GfxSystem_SetWorldMat(system, worldMat);
	
	GfxSystem_SetTintColor(system, color);
	
	GfxSystem_BindVertBuffer(system, &system->circleBuffer);
	GfxSystem_DrawVerticesEx(system, startSideIndex*3, (endSideIndex - startSideIndex)*3);
}
PEXPI void GfxSystem_DrawTexturedCirclePiece(GfxSystem* system, Circle circle, r32 angleMin, r32 angleMax, Color32 color, Texture* texture)
{
	rec sourceRec = (texture != nullptr) ? NewRec(0, 0, (r32)texture->Width, (r32)texture->Height) : Rec_Default_Const;
	GfxSystem_DrawTexturedCirclePieceEx(system, circle, angleMin, angleMax, color, texture, sourceRec);
}
PEXPI void GfxSystem_DrawTexturedCircleEx(GfxSystem* system, Circle circle, Color32 color, Texture* texture, rec sourceRec)
{
	GfxSystem_DrawTexturedCirclePieceEx(system, circle, 0, TwoPi32-DEFAULT_R32_TOLERANCE, color, texture, sourceRec);
}
PEXPI void GfxSystem_DrawTexturedCircle(GfxSystem* system, Circle circle, Color32 color, Texture* texture)
{
	rec sourceRec = (texture != nullptr) ? NewRec(0, 0, (r32)texture->Width, (r32)texture->Height) : Rec_Default_Const;
	GfxSystem_DrawTexturedCirclePieceEx(system, circle, 0, TwoPi32-DEFAULT_R32_TOLERANCE, color, texture, sourceRec);
}
PEXPI void GfxSystem_DrawCirclePiece(GfxSystem* system, Circle circle, r32 angleMin, r32 angleMax, Color32 color)
{
	GfxSystem_DrawTexturedCirclePieceEx(system, circle, angleMin, angleMax, color, nullptr, Rec_Default_Const);
}
PEXPI void GfxSystem_DrawCircle(GfxSystem* system, Circle circle, Color32 color)
{
	GfxSystem_DrawTexturedCirclePieceEx(system, circle, 0, TwoPi32-DEFAULT_R32_TOLERANCE, color, nullptr, Rec_Default_Const);
}

PEXP void GfxSystem_DrawTexturedRoundedRectangleEx(GfxSystem* system, rec rectangle, r32 radiusTL, r32 radiusTR, r32 radiusBR, r32 radiusBL, Color32 color, Texture* texture, rec sourceRec)
{
	NotNull(system);
	if (radiusTL + radiusBL > rectangle.Height) { radiusTL = rectangle.Height/2; radiusBL = rectangle.Height/2; }
	if (radiusTR + radiusBR > rectangle.Height) { radiusTR = rectangle.Height/2; radiusBR = rectangle.Height/2; }
	if (radiusTL + radiusTR > rectangle.Width) { radiusTL = rectangle.Width/2; radiusTR = rectangle.Width/2; }
	if (radiusBL + radiusBR > rectangle.Width) { radiusBL = rectangle.Width/2; radiusBR = rectangle.Width/2; }
	//TODO: We need to manipulate sourceRec properly for each section we are drawing here!
	if (radiusTL != 0 || radiusTR != 0 || radiusBL != 0 || radiusBR != 0)
	{
		if (radiusTL > 0 || radiusTR > 0) //Top Rec
		{
			GfxSystem_DrawTexturedRectangleEx(system, NewRec(rectangle.X + radiusTL, rectangle.Y, rectangle.Width - radiusTR - radiusTL, MaxR32(radiusTR, radiusTL)), color, texture, sourceRec);
		}
		if (radiusBL > 0 || radiusBR > 0) //Bottom Rec
		{
			GfxSystem_DrawTexturedRectangleEx(system, NewRec(rectangle.X + radiusBL, rectangle.Y + rectangle.Height - MaxR32(radiusBL, radiusBR), rectangle.Width - radiusBR - radiusBL, MaxR32(radiusBL, radiusBR)), color, texture, sourceRec);
		}
		if (radiusTL > 0 || radiusBL > 0) //Left Rec
		{
			GfxSystem_DrawTexturedRectangleEx(system, NewRec(rectangle.X, rectangle.Y + radiusTL, MaxR32(radiusTL, radiusBL), rectangle.Height - radiusBL - radiusTL), color, texture, sourceRec);
		}
		if (radiusTR > 0 || radiusBR > 0) //Right Rec
		{
			GfxSystem_DrawTexturedRectangleEx(system, NewRec(rectangle.X + rectangle.Width - MaxR32(radiusTR, radiusBR), rectangle.Y + radiusTR, MaxR32(radiusTR, radiusBR), rectangle.Height - radiusBR - radiusTR), color, texture, sourceRec);
		}
		
		//Center Rec
		GfxSystem_DrawTexturedRectangleEx(system, NewRec(rectangle.X + MaxR32(radiusTL, radiusBL), rectangle.Y + MaxR32(radiusTL, radiusTR), rectangle.Width - MaxR32(radiusTL, radiusBL) - MaxR32(radiusTR, radiusBR), rectangle.Height - MaxR32(radiusTL, radiusTR) - MaxR32(radiusBL, radiusBR)), color, texture, sourceRec);
		
		if (radiusBR > 0) //BottomRight Circle Piece
		{
			GfxSystem_DrawTexturedCirclePieceEx(system, NewCircle(rectangle.X + rectangle.Width - radiusBR, rectangle.Y + rectangle.Height - radiusBR, radiusBR), 0, HalfPi32, color, texture, sourceRec);
		}
		if (radiusBL > 0) //BottomLeft Circle Piece
		{
			GfxSystem_DrawTexturedCirclePieceEx(system, NewCircle(rectangle.X + radiusBL, rectangle.Y + rectangle.Height - radiusBL, radiusBL), HalfPi32, Pi32, color, texture, sourceRec);
		}
		if (radiusTL > 0) //TopLeft Circle Piece
		{
			GfxSystem_DrawTexturedCirclePieceEx(system, NewCircle(rectangle.X + radiusTL, rectangle.Y + radiusTL, radiusTL), Pi32, ThreeHalfsPi32, color, texture, sourceRec);
		}
		if (radiusTR > 0) //TopRight Circle Piece
		{
			GfxSystem_DrawTexturedCirclePieceEx(system, NewCircle(rectangle.X + rectangle.Width - radiusTR, rectangle.Y + radiusTR, radiusTR), ThreeHalfsPi32, TwoPi32, color, texture, sourceRec);
		}
	}
	else
	{
		GfxSystem_DrawRectangle(system, rectangle, color);
	}
}
PEXPI void GfxSystem_DrawTexturedRoundedRectangle(GfxSystem* system, rec rectangle, r32 radius, Color32 color, Texture* texture, rec sourceRec)
{
	GfxSystem_DrawTexturedRoundedRectangleEx(system, rectangle, radius, radius, radius, radius, color, texture, sourceRec);
}
PEXPI void GfxSystem_DrawRoundedRectangleEx(GfxSystem* system, rec rectangle, r32 radiusTL, r32 radiusTR, r32 radiusBR, r32 radiusBL, Color32 color)
{
	GfxSystem_DrawTexturedRoundedRectangleEx(system, rectangle, radiusTL, radiusTR, radiusBR, radiusBL, color, nullptr, Rec_Default_Const);
}
PEXPI void GfxSystem_DrawRoundedRectangle(GfxSystem* system, rec rectangle, r32 radius, Color32 color)
{
	GfxSystem_DrawTexturedRoundedRectangleEx(system, rectangle, radius, radius, radius, radius, color, nullptr, Rec_Default_Const);
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
	GfxSystem_SetDepth(system, clearDepth);
	
	GfxSystem_DrawRectangle(system, NewRec(-1, -1, 2, 2), MonokaiPurple);
	
	GfxSystem_SetDepth(system, oldDepth);
	GfxSystem_SetCullingEnabled(system, oldCullingEnabled);
	GfxSystem_SetColorWriteEnabled(system, oldColorWriteEnabled);
	GfxSystem_SetDepthTestEnabled(system, oldDepthWriteEnabled);
	GfxSystem_SetDepthWriteEnabled(system, oldDepthTestEnabled);
	GfxSystem_SetProjectionMat(system, oldProjectionMat);
	GfxSystem_SetViewMat(system, oldViewMat);
}

// +====================================+
// | GfxSystem_FontFlowDrawCharCallback |
// +====================================+
// void GfxSystem_FontFlowDrawCharCallback(FontFlowState* state, FontFlow* flow, rec glyphDrawRec, u32 codepoint, FontAtlas* atlas, FontGlyph* glyph)
FONT_FLOW_DRAW_CHAR_DEF(GfxSystem_FontFlowDrawCharCallback)
{
	NotNull(state);
	NotNull(state->contextPntr);
	NotNull(atlas);
	NotNull(glyph);
	UNUSED(flow);
	UNUSED(codepoint);
	GfxSystem* system = (GfxSystem*)state->contextPntr;
	GfxSystem_DrawTexturedRectangleEx(system, glyphDrawRec, state->color, &atlas->texture, ToRecFromi(glyph->atlasSourceRec));
}

PEXP Result GfxSystem_DrawTextWithFont(GfxSystem* system, PigFont* font, r32 fontSize, u8 styleFlags, Str8 text, v2 position, Color32 color)
{
	NotNull(system);
	NotNull(font);
	NotNullStr(text);
	
	FontFlowState state = ZEROED;
	state.contextPntr = (void*)system;
	state.font = font;
	state.text = text;
	state.fontSize = fontSize;
	state.styleFlags = styleFlags;
	state.alignPixelSize = system->state.alignPixelSize;
	state.color = color;
	state.position = position;
	FontFlowCallbacks callbacks = ZEROED;
	callbacks.drawChar = GfxSystem_FontFlowDrawCharCallback;
	
	Result result = DoFontFlow(&state, &callbacks, &system->prevFontFlow);
	
	return result;
}
PEXPI Result GfxSystem_DrawTextAtSize(GfxSystem* system, r32 fontSize, Str8 text, v2 position, Color32 color)
{
	NotNull(system);
	NotNull(system->state.font);
	return GfxSystem_DrawTextWithFont(system, system->state.font, fontSize, system->state.fontStyleFlags, text, position, color);
}
PEXPI Result GfxSystem_DrawTextBold(GfxSystem* system, Str8 text, v2 position, Color32 color)
{
	NotNull(system);
	NotNull(system->state.font);
	return GfxSystem_DrawTextWithFont(system, system->state.font, system->state.fontSize, (system->state.fontStyleFlags | FontStyleFlag_Bold), text, position, color);
}
PEXPI Result GfxSystem_DrawTextItalic(GfxSystem* system, Str8 text, v2 position, Color32 color)
{
	NotNull(system);
	NotNull(system->state.font);
	return GfxSystem_DrawTextWithFont(system, system->state.font, system->state.fontSize, (system->state.fontStyleFlags | FontStyleFlag_Italic), text, position, color);
}
PEXPI Result GfxSystem_DrawText(GfxSystem* system, Str8 text, v2 position, Color32 color)
{
	NotNull(system);
	NotNull(system->state.font);
	return GfxSystem_DrawTextWithFont(system, system->state.font, system->state.fontSize, system->state.fontStyleFlags, text, position, color);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _GFX_SYSTEM_H
