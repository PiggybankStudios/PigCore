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
#include "misc/misc_printing.h"
#include "struct/struct_string.h"
#include "struct/struct_rich_string.h"
#include "struct/struct_color.h"
#include "struct/struct_var_array.h"
#include "misc/misc_result.h"
#include "gfx/gfx_vertices.h"
#include "lib/lib_sokol_gfx.h"
#include "gfx/gfx_vert_buffer.h"
#include "gfx/gfx_texture.h"
#include "gfx/gfx_shader.h"
#include "gfx/gfx_font.h"
#include "gfx/gfx_font_flow.h"
#include "gfx/gfx_pipeline.h"

#if BUILD_WITH_SOKOL_GFX

#define GFX_SYSTEM_CIRCLE_NUM_SIDES     32 // aka 11.25 degree increments
#define GFX_SYSTEM_RING_NUM_THICKNESSES 10 // aka 10% increments
#define GFX_SYSTEM_RING_NUM_SIDES       32 // aka 11.25 degree increments

typedef plex GfxSystemState GfxSystemState;
plex GfxSystemState
{
	bool colorWriteEnabled;
	bool depthWriteEnabled;
	bool depthTestEnabled;
	bool cullingEnabled;
	uxx indexedVerticesSize;
	GfxPipelineBlendMode blendMode;
	r32 depth; //for 2D rendering functions
	reci clipRec;
	Color32 textBackgroundColor; //only used when drawing highlighted text
	
	Shader* shader;
	Texture* textures[MAX_NUM_SHADER_IMAGES];
	VertBuffer* vertBuffer;
	uxx vertexOffset;
	
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

typedef plex GfxSystem GfxSystem;
plex GfxSystem
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
	VertBuffer ringBuffers[GFX_SYSTEM_RING_NUM_THICKNESSES];
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
	PIG_CORE_INLINE void GfxSystem_SetVertexOffset(GfxSystem* system, uxx vertexOffset);
	PIG_CORE_INLINE void GfxSystem_BindTextureAtIndex(GfxSystem* system, Texture* texture, uxx textureIndex);
	PIG_CORE_INLINE void GfxSystem_BindTexture(GfxSystem* system, Texture* texture);
	PIG_CORE_INLINE void GfxSystem_BindFontEx(GfxSystem* system, PigFont* font, r32 fontSize, u8 fontStyleFlags);
	PIG_CORE_INLINE void GfxSystem_BindFontAtSize(GfxSystem* system, PigFont* font, r32 fontSize);
	PIG_CORE_INLINE void GfxSystem_BindFont(GfxSystem* system, PigFont* font);
	PIG_CORE_INLINE void GfxSystem_SetClipRec(GfxSystem* system, reci clipRec);
	PIG_CORE_INLINE reci GfxSystem_AddClipRec(GfxSystem* system, reci clipRec);
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
	PIG_CORE_INLINE void GfxSystem_DrawTexturedRectangleOutlineSidesEx(GfxSystem* system, rec rectangle, r32 leftThickness, r32 rightThickness, r32 topThickness, r32 bottomThickness, Color32 color, bool outside, Texture* texture, rec sourceRec);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedRectangleOutlineSides(GfxSystem* system, rec rectangle, r32 leftThickness, r32 rightThickness, r32 topThickness, r32 bottomThickness, Color32 color, Texture* texture);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedRectangleOutlineEx(GfxSystem* system, rec rectangle, r32 borderThickness, Color32 color, bool outside, Texture* texture, rec sourceRec);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedRectangleOutline(GfxSystem* system, rec rectangle, r32 borderThickness, Color32 color, Texture* texture);
	PIG_CORE_INLINE void GfxSystem_DrawRectangleOutlineSidesEx(GfxSystem* system, rec rectangle, r32 leftThickness, r32 rightThickness, r32 topThickness, r32 bottomThickness, Color32 color, bool outside);
	PIG_CORE_INLINE void GfxSystem_DrawRectangleOutlineSides(GfxSystem* system, rec rectangle, r32 leftThickness, r32 rightThickness, r32 topThickness, r32 bottomThickness, Color32 color);
	PIG_CORE_INLINE void GfxSystem_DrawRectangleOutlineEx(GfxSystem* system, rec rectangle, r32 borderThickness, Color32 color, bool outside);
	PIG_CORE_INLINE void GfxSystem_DrawRectangleOutline(GfxSystem* system, rec rectangle, r32 borderThickness, Color32 color);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedObb2Ex(GfxSystem* system, obb2 boundingBox, Color32 color, Texture* texture, rec sourceRec);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedObb2(GfxSystem* system, obb2 boundingBox, Color32 color, Texture* texture);
	PIG_CORE_INLINE void GfxSystem_DrawObb2(GfxSystem* system, obb2 boundingBox, Color32 color);
	PIG_CORE_INLINE void GfxSystem_DrawLine(GfxSystem* system, v2 startPos, v2 endPos, r32 thickness, Color32 color);
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
	void GfxSystem_DrawTexturedRingPieceEx(GfxSystem* system, Circle circle, r32 thickness, r32 angleMin, r32 angleMax, Color32 color, Texture* texture, rec sourceRec);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedRingPiece(GfxSystem* system, Circle circle, r32 thickness, r32 angleMin, r32 angleMax, Color32 color, Texture* texture);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedRingEx(GfxSystem* system, Circle circle, r32 thickness, Color32 color, Texture* texture, rec sourceRec);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedRing(GfxSystem* system, Circle circle, r32 thickness, Color32 color, Texture* texture);
	PIG_CORE_INLINE void GfxSystem_DrawRingPiece(GfxSystem* system, Circle circle, r32 thickness, r32 angleMin, r32 angleMax, Color32 color);
	PIG_CORE_INLINE void GfxSystem_DrawRing(GfxSystem* system, Circle circle, r32 thickness, Color32 color);
	void GfxSystem_DrawTexturedRoundedRectangleOutlineEx(GfxSystem* system, rec rectangle, r32 thickness, r32 radiusTL, r32 radiusTR, r32 radiusBR, r32 radiusBL, Color32 color, bool outside, Texture* texture, rec sourceRec);
	PIG_CORE_INLINE void GfxSystem_DrawTexturedRoundedRectangleOutline(GfxSystem* system, rec rectangle, r32 thickness, r32 radius, Color32 color, bool outside, Texture* texture, rec sourceRec);
	PIG_CORE_INLINE void GfxSystem_DrawRoundedRectangleOutlineEx(GfxSystem* system, rec rectangle, r32 thickness, r32 radiusTL, r32 radiusTR, r32 radiusBR, r32 radiusBL, Color32 color, bool outside);
	PIG_CORE_INLINE void GfxSystem_DrawRoundedRectangleOutline(GfxSystem* system, rec rectangle, r32 thickness, r32 radius, Color32 color);
	PIG_CORE_INLINE void GfxSystem_ClearDepthBuffer(GfxSystem* system, r32 clearDepth);
	PIG_CORE_INLINE void GfxSystem_SetTextBackgroundColor(GfxSystem* system, Color32 color);
	Result GfxSystem_DrawWrappedRichTextWithFont(GfxSystem* system, PigFont* font, r32 fontSize, u8 styleFlags, RichStr text, v2 position, r32 wrapWidth, Color32 color);
	PIG_CORE_INLINE Result GfxSystem_DrawRichTextWithFont(GfxSystem* system, PigFont* font, r32 fontSize, u8 styleFlags, RichStr text, v2 position, Color32 color);
	PIG_CORE_INLINE Result GfxSystem_DrawTextWithFont(GfxSystem* system, PigFont* font, r32 fontSize, u8 styleFlags, Str8 text, v2 position, Color32 color);
	PIG_CORE_INLINE Result GfxSystem_DrawTextAtSize(GfxSystem* system, r32 fontSize, Str8 text, v2 position, Color32 color);
	PIG_CORE_INLINE Result GfxSystem_DrawTextBold(GfxSystem* system, Str8 text, v2 position, Color32 color);
	PIG_CORE_INLINE Result GfxSystem_DrawTextItalic(GfxSystem* system, Str8 text, v2 position, Color32 color);
	PIG_CORE_INLINE Result GfxSystem_DrawText(GfxSystem* system, Str8 text, v2 position, Color32 color);
	PIG_CORE_INLINE Result GfxSystem_DrawWrappedText(GfxSystem* system, Str8 text, v2 position, r32 wrapWidth, Color32 color);
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
	
	Vertex2D ringVertices[GFX_SYSTEM_RING_NUM_SIDES*6];
	for (uxx tIndex = 0; tIndex < GFX_SYSTEM_RING_NUM_THICKNESSES; tIndex++)
	{
		const r32 outerRadius = 0.5f;
		r32 innerRadius = (outerRadius / GFX_SYSTEM_RING_NUM_THICKNESSES) * (r32)(tIndex+1);
		for (uxx sIndex = 0; sIndex < GFX_SYSTEM_RING_NUM_SIDES; sIndex++)
		{
			r32 angle0 = (r32)(sIndex+0) * (TwoPi32 / GFX_SYSTEM_RING_NUM_SIDES);
			r32 angle1 = (r32)(sIndex+1) * (TwoPi32 / GFX_SYSTEM_RING_NUM_SIDES);
			v2 positionAndTexCoordInner0 = NewV2(0.5f + CosR32(angle0)*innerRadius, 0.5f + SinR32(angle0)*innerRadius);
			v2 positionAndTexCoordOuter0 = NewV2(0.5f + CosR32(angle0)*outerRadius, 0.5f + SinR32(angle0)*outerRadius);
			v2 positionAndTexCoordInner1 = NewV2(0.5f + CosR32(angle1)*innerRadius, 0.5f + SinR32(angle1)*innerRadius);
			v2 positionAndTexCoordOuter1 = NewV2(0.5f + CosR32(angle1)*outerRadius, 0.5f + SinR32(angle1)*outerRadius);
			ringVertices[sIndex*6 + 0] = NewVertex2D(positionAndTexCoordInner0, positionAndTexCoordInner0, NewV4(1, 1, 1, 1));
			ringVertices[sIndex*6 + 1] = NewVertex2D(positionAndTexCoordOuter0, positionAndTexCoordOuter0, NewV4(1, 1, 1, 1));
			ringVertices[sIndex*6 + 2] = NewVertex2D(positionAndTexCoordOuter1, positionAndTexCoordOuter1, NewV4(1, 1, 1, 1));
			ringVertices[sIndex*6 + 3] = NewVertex2D(positionAndTexCoordInner0, positionAndTexCoordInner0, NewV4(1, 1, 1, 1));
			ringVertices[sIndex*6 + 4] = NewVertex2D(positionAndTexCoordOuter1, positionAndTexCoordOuter1, NewV4(1, 1, 1, 1));
			ringVertices[sIndex*6 + 5] = NewVertex2D(positionAndTexCoordInner1, positionAndTexCoordInner1, NewV4(1, 1, 1, 1));
		}
		systemOut->ringBuffers[tIndex] = InitVertBuffer2D(arena, ScratchPrintStr("ring%llu", (u64)tIndex+1), VertBufferUsage_Static, ArrayCount(ringVertices), &ringVertices[0], false);
		Assert(systemOut->ringBuffers[tIndex].error == Result_Success);
	}
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
	
	sg_apply_viewport(0, 0, (int)screenSize.Width, (int)screenSize.Height, true);
	
	system->state.clipRec = NewReciV(V2i_Zero, screenSize);
	system->state.textBackgroundColor = NewColorU32(0x00000000);
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
	sg_commit();
	
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
PEXPI void GfxSystem_SetVertexOffset(GfxSystem* system, uxx vertexOffset)
{
	NotNull(system);
	if (system->state.vertexOffset != vertexOffset)
	{
		DebugAssert(vertexOffset <= INT_MAX);
		system->bindings.vertex_buffer_offsets[0] = (int)vertexOffset;
		system->state.vertexOffset = vertexOffset;
		system->bindingsChanged = true;
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
PEXPI reci GfxSystem_AddClipRec(GfxSystem* system, reci clipRec)
{
	NotNull(system);
	reci oldClipRec = system->state.clipRec;
	if (!AreEqual(system->state.clipRec, clipRec))
	{
		reci overlapRec = OverlapPartReci(system->state.clipRec, clipRec);
		sg_apply_scissor_rect(overlapRec.X, overlapRec.Y, overlapRec.Width, overlapRec.Height, true);
		system->state.clipRec = overlapRec;
	}
	return oldClipRec;
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
	rec sourceRec = (texture != nullptr) ? NewRec(0, 0, (r32)texture->Width, (r32)texture->Height) : (rec)Rec_Zero_Const;
	GfxSystem_DrawTexturedRectangleEx(system, rectangle, color, texture, sourceRec);
}
PEXPI void GfxSystem_DrawRectangle(GfxSystem* system, rec rectangle, Color32 color)
{
	GfxSystem_DrawTexturedRectangleEx(system, rectangle, color, nullptr, (rec)Rec_Zero_Const);
}

PEXPI void GfxSystem_DrawTexturedRectangleOutlineSidesEx(GfxSystem* system, rec rectangle, r32 leftThickness, r32 rightThickness, r32 topThickness, r32 bottomThickness, Color32 color, bool outside, Texture* texture, rec sourceRec)
{
	if (outside)
	{
		rec inflatedRec = NewRec(rectangle.X - leftThickness, rectangle.Y - topThickness, rectangle.Width + leftThickness + rightThickness, rectangle.Height + topThickness + bottomThickness);
		GfxSystem_DrawTexturedRectangleOutlineSidesEx(system, inflatedRec, leftThickness, rightThickness, topThickness, bottomThickness, color, false, texture, sourceRec);
		return;
	}
	if (leftThickness + rightThickness >= rectangle.Width)
	{
		GfxSystem_DrawTexturedRectangleEx(system, rectangle, color, texture, sourceRec);
		return;
	}
	if (topThickness + bottomThickness >= rectangle.Height)
	{
		GfxSystem_DrawTexturedRectangleEx(system, rectangle, color, texture, sourceRec);
		return;
	}
	if (topThickness > 0) //Top Side
	{
		rec sideRec = NewRec(rectangle.X, rectangle.Y, rectangle.Width, topThickness);
		GfxSystem_DrawTexturedRectangleEx(system, sideRec, color, texture, RelativeRec(rectangle, sideRec, sourceRec));
	}
	if (bottomThickness > 0) //Bottom Side
	{
		rec sideRec = NewRec(rectangle.X, rectangle.Y + rectangle.Height - bottomThickness, rectangle.Width, bottomThickness);
		GfxSystem_DrawTexturedRectangleEx(system, sideRec, color, texture, RelativeRec(rectangle, sideRec, sourceRec));
	}
	if (leftThickness > 0) //Left Side
	{
		rec sideRec = NewRec(rectangle.X, rectangle.Y + topThickness, leftThickness, rectangle.Height - (topThickness+bottomThickness));
		GfxSystem_DrawTexturedRectangleEx(system, sideRec, color, texture, RelativeRec(rectangle, sideRec, sourceRec));
	}
	if (rightThickness > 0) //Right Side
	{
		rec sideRec = NewRec(rectangle.X + rectangle.Width - rightThickness, rectangle.Y + topThickness, rightThickness, rectangle.Height - (topThickness+bottomThickness));
		GfxSystem_DrawTexturedRectangleEx(system, sideRec, color, texture, RelativeRec(rectangle, sideRec, sourceRec));
	}
}
PEXPI void GfxSystem_DrawTexturedRectangleOutlineSides(GfxSystem* system, rec rectangle, r32 leftThickness, r32 rightThickness, r32 topThickness, r32 bottomThickness, Color32 color, Texture* texture)
{
	rec sourceRec = (texture != nullptr) ? NewRec(0, 0, (r32)texture->Width, (r32)texture->Height) : (rec)Rec_Zero_Const;
	GfxSystem_DrawTexturedRectangleOutlineSidesEx(system, rectangle, leftThickness, rightThickness, topThickness, bottomThickness, color, true, texture, sourceRec);
}
PEXPI void GfxSystem_DrawTexturedRectangleOutlineEx(GfxSystem* system, rec rectangle, r32 borderThickness, Color32 color, bool outside, Texture* texture, rec sourceRec)
{
	GfxSystem_DrawTexturedRectangleOutlineSidesEx(system, rectangle, borderThickness, borderThickness, borderThickness, borderThickness, color, outside, texture, sourceRec);
}
PEXPI void GfxSystem_DrawTexturedRectangleOutline(GfxSystem* system, rec rectangle, r32 borderThickness, Color32 color, Texture* texture)
{
	rec sourceRec = (texture != nullptr) ? NewRec(0, 0, (r32)texture->Width, (r32)texture->Height) : (rec)Rec_Zero_Const;
	GfxSystem_DrawTexturedRectangleOutlineSidesEx(system, rectangle, borderThickness, borderThickness, borderThickness, borderThickness, color, true, texture, sourceRec);
}
PEXPI void GfxSystem_DrawRectangleOutlineSidesEx(GfxSystem* system, rec rectangle, r32 leftThickness, r32 rightThickness, r32 topThickness, r32 bottomThickness, Color32 color, bool outside)
{
	GfxSystem_DrawTexturedRectangleOutlineSidesEx(system, rectangle, leftThickness, rightThickness, topThickness, bottomThickness, color, outside, nullptr, (rec)Rec_Zero_Const);
}
PEXPI void GfxSystem_DrawRectangleOutlineSides(GfxSystem* system, rec rectangle, r32 leftThickness, r32 rightThickness, r32 topThickness, r32 bottomThickness, Color32 color)
{
	GfxSystem_DrawTexturedRectangleOutlineSidesEx(system, rectangle, leftThickness, rightThickness, topThickness, bottomThickness, color, true, nullptr, (rec)Rec_Zero_Const);
}
PEXPI void GfxSystem_DrawRectangleOutlineEx(GfxSystem* system, rec rectangle, r32 borderThickness, Color32 color, bool outside)
{
	GfxSystem_DrawTexturedRectangleOutlineSidesEx(system, rectangle, borderThickness, borderThickness, borderThickness, borderThickness, color, outside, nullptr, (rec)Rec_Zero_Const);
}
PEXPI void GfxSystem_DrawRectangleOutline(GfxSystem* system, rec rectangle, r32 borderThickness, Color32 color)
{
	GfxSystem_DrawTexturedRectangleOutlineSidesEx(system, rectangle, borderThickness, borderThickness, borderThickness, borderThickness, color, true, nullptr, (rec)Rec_Zero_Const);
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
	TransformMat4(&worldMat, MakeRotate2DMat4(boundingBox.Rotation));
	TransformMat4(&worldMat, MakeTranslateXYZMat4(boundingBox.X, boundingBox.Y, system->state.depth));
	GfxSystem_SetWorldMat(system, worldMat);
	
	GfxSystem_SetTintColor(system, color);
	
	GfxSystem_BindVertBuffer(system, &system->squareBuffer);
	GfxSystem_DrawVertices(system);
}
PEXPI void GfxSystem_DrawTexturedObb2(GfxSystem* system, obb2 boundingBox, Color32 color, Texture* texture)
{
	rec sourceRec = (texture != nullptr) ? NewRec(0, 0, (r32)texture->Width, (r32)texture->Height) : (rec)Rec_Zero_Const;
	GfxSystem_DrawTexturedObb2Ex(system, boundingBox, color, texture, sourceRec);
}
PEXPI void GfxSystem_DrawObb2(GfxSystem* system, obb2 boundingBox, Color32 color)
{
	GfxSystem_DrawTexturedObb2Ex(system, boundingBox, color, nullptr, (rec)Rec_Zero_Const);
}

PEXPI void GfxSystem_DrawLine(GfxSystem* system, v2 startPos, v2 endPos, r32 thickness, Color32 color)
{
	if (AreEqualV2(startPos, endPos)) { return; }
	v2 lineMidpoint = Div(Add(startPos, endPos), 2.0f);
	v2 lineSize = NewV2(LengthV2(Sub(endPos, startPos)), thickness);
	r32 lineRotation = AtanR32(endPos.Y - startPos.Y, endPos.X - startPos.X);
	obb2 lineBox = NewObb2V(lineMidpoint, lineSize, lineRotation);
	GfxSystem_DrawObb2(system, lineBox, color);
}

PEXP void GfxSystem_DrawTexturedCirclePieceEx(GfxSystem* system, Circle circle, r32 angleMin, r32 angleMax, Color32 color, Texture* texture, rec sourceRec)
{
	NotNull(system);
	
	uxx startSideIndex = (uxx)RoundR32i((AngleFixR32(angleMin) / TwoPi32) * GFX_SYSTEM_CIRCLE_NUM_SIDES);
	uxx endSideIndex = (uxx)RoundR32i((AngleFixR32(angleMax) / TwoPi32) * GFX_SYSTEM_CIRCLE_NUM_SIDES);
	if (endSideIndex == startSideIndex)
	{
		//If angleMin>=angleMax, treat it as if we are drawing a full circle, since it is ambigious, otherwise if the angles are slightly different then don't draw anything because it's less than one segment angle
		if (AreSimilarOrGreaterR32(AngleFixR32(angleMin), AngleFixR32(angleMax), DEFAULT_R32_TOLERANCE))
		{
			startSideIndex = 0;
			endSideIndex = GFX_SYSTEM_CIRCLE_NUM_SIDES;
		}
		else { return; }
	}
	if (endSideIndex < startSideIndex)
	{
		//NOTE: To draw an arc that passes over 0/TwoPi angle, we really need to do two draw calls for either side. So we just recurse one level to accomplish this cleanly
		r32 minAngleForSingleSegment = (TwoPi32 / GFX_SYSTEM_CIRCLE_NUM_SIDES) / 2.0f;
		if (AngleFixR32(angleMin) <= TwoPi32 - minAngleForSingleSegment) { GfxSystem_DrawTexturedCirclePieceEx(system, circle, angleMin, TwoPi32-DEFAULT_R32_TOLERANCE, color, texture, sourceRec); }
		if (AngleFixR32(angleMax) >= minAngleForSingleSegment) { GfxSystem_DrawTexturedCirclePieceEx(system, circle, 0, angleMax, color, texture, sourceRec); }
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
	rec sourceRec = (texture != nullptr) ? NewRec(0, 0, (r32)texture->Width, (r32)texture->Height) : (rec)Rec_Zero_Const;
	GfxSystem_DrawTexturedCirclePieceEx(system, circle, angleMin, angleMax, color, texture, sourceRec);
}
PEXPI void GfxSystem_DrawTexturedCircleEx(GfxSystem* system, Circle circle, Color32 color, Texture* texture, rec sourceRec)
{
	GfxSystem_DrawTexturedCirclePieceEx(system, circle, 0, TwoPi32-DEFAULT_R32_TOLERANCE, color, texture, sourceRec);
}
PEXPI void GfxSystem_DrawTexturedCircle(GfxSystem* system, Circle circle, Color32 color, Texture* texture)
{
	rec sourceRec = (texture != nullptr) ? NewRec(0, 0, (r32)texture->Width, (r32)texture->Height) : (rec)Rec_Zero_Const;
	GfxSystem_DrawTexturedCirclePieceEx(system, circle, 0, TwoPi32-DEFAULT_R32_TOLERANCE, color, texture, sourceRec);
}
PEXPI void GfxSystem_DrawCirclePiece(GfxSystem* system, Circle circle, r32 angleMin, r32 angleMax, Color32 color)
{
	GfxSystem_DrawTexturedCirclePieceEx(system, circle, angleMin, angleMax, color, nullptr, (rec)Rec_Zero_Const);
}
PEXPI void GfxSystem_DrawCircle(GfxSystem* system, Circle circle, Color32 color)
{
	GfxSystem_DrawTexturedCirclePieceEx(system, circle, 0, TwoPi32-DEFAULT_R32_TOLERANCE, color, nullptr, (rec)Rec_Zero_Const);
}

PEXP void GfxSystem_DrawTexturedRoundedRectangleEx(GfxSystem* system, rec rectangle, r32 radiusTL, r32 radiusTR, r32 radiusBR, r32 radiusBL, Color32 color, Texture* texture, rec sourceRec)
{
	NotNull(system);
	bool isDegenerate = false;
	if (radiusTL + radiusBL > rectangle.Height) { isDegenerate = true; radiusTL = rectangle.Height/2; radiusBL = rectangle.Height/2; }
	if (radiusTR + radiusBR > rectangle.Height) { isDegenerate = true; radiusTR = rectangle.Height/2; radiusBR = rectangle.Height/2; }
	if (radiusTL + radiusTR > rectangle.Width)  { isDegenerate = true; radiusTL = rectangle.Width/2; radiusTR = rectangle.Width/2; }
	if (radiusBL + radiusBR > rectangle.Width)  { isDegenerate = true; radiusBL = rectangle.Width/2; radiusBR = rectangle.Width/2; }
	if (radiusTL != 0 || radiusTR != 0 || radiusBL != 0 || radiusBR != 0)
	{
		if (radiusTL > 0 || radiusTR > 0) //Top Rec
		{
			rec sideRec = NewRec(rectangle.X + radiusTL, rectangle.Y, rectangle.Width - radiusTR - radiusTL, MaxR32(radiusTR, radiusTL));
			GfxSystem_DrawTexturedRectangleEx(system, sideRec, color, texture, RelativeRec(rectangle, sideRec, sourceRec));
		}
		if (radiusBL > 0 || radiusBR > 0) //Bottom Rec
		{
			rec sideRec = NewRec(rectangle.X + radiusBL, rectangle.Y + rectangle.Height - MaxR32(radiusBL, radiusBR), rectangle.Width - radiusBR - radiusBL, MaxR32(radiusBL, radiusBR));
			GfxSystem_DrawTexturedRectangleEx(system, sideRec, color, texture, RelativeRec(rectangle, sideRec, sourceRec));
		}
		if (radiusTL > 0 || radiusBL > 0) //Left Rec
		{
			rec sideRec = NewRec(rectangle.X, rectangle.Y + radiusTL, MaxR32(radiusTL, radiusBL), rectangle.Height - radiusBL - radiusTL);
			GfxSystem_DrawTexturedRectangleEx(system, sideRec, color, texture, RelativeRec(rectangle, sideRec, sourceRec));
		}
		if (radiusTR > 0 || radiusBR > 0) //Right Rec
		{
			rec sideRec = NewRec(rectangle.X + rectangle.Width - MaxR32(radiusTR, radiusBR), rectangle.Y + radiusTR, MaxR32(radiusTR, radiusBR), rectangle.Height - radiusBR - radiusTR);
			GfxSystem_DrawTexturedRectangleEx(system, sideRec, color, texture, RelativeRec(rectangle, sideRec, sourceRec));
		}
		
		if (!isDegenerate) //Center Rec
		{
			rec centerRec = NewRec(rectangle.X + MaxR32(radiusTL, radiusBL), rectangle.Y + MaxR32(radiusTL, radiusTR), rectangle.Width - MaxR32(radiusTL, radiusBL) - MaxR32(radiusTR, radiusBR), rectangle.Height - MaxR32(radiusTL, radiusTR) - MaxR32(radiusBL, radiusBR));
			GfxSystem_DrawTexturedRectangleEx(system, centerRec, color, texture, RelativeRec(rectangle, centerRec, sourceRec));
		}
		
		if (radiusBR > 0) //BottomRight Quarter Circle
		{
			Circle cornerCircle = NewCircle(rectangle.X + rectangle.Width - radiusBR, rectangle.Y + rectangle.Height - radiusBR, radiusBR);
			rec cornerFullCircleRec = NewRec(cornerCircle.X - cornerCircle.R, cornerCircle.Y - cornerCircle.R, 2 * cornerCircle.R, 2 * cornerCircle.R);
			GfxSystem_DrawTexturedCirclePieceEx(system, cornerCircle, 0, HalfPi32, color, texture, RelativeRec(rectangle, cornerFullCircleRec, sourceRec));
		}
		if (radiusBL > 0) //BottomLeft Quarter Circle
		{
			Circle cornerCircle = NewCircle(rectangle.X + radiusBL, rectangle.Y + rectangle.Height - radiusBL, radiusBL);
			rec cornerFullCircleRec = NewRec(cornerCircle.X - cornerCircle.R, cornerCircle.Y - cornerCircle.R, 2 * cornerCircle.R, 2 * cornerCircle.R);
			GfxSystem_DrawTexturedCirclePieceEx(system, cornerCircle, HalfPi32, Pi32, color, texture, RelativeRec(rectangle, cornerFullCircleRec, sourceRec));
		}
		if (radiusTL > 0) //TopLeft Quarter Circle
		{
			Circle cornerCircle = NewCircle(rectangle.X + radiusTL, rectangle.Y + radiusTL, radiusTL);
			rec cornerFullCircleRec = NewRec(cornerCircle.X - cornerCircle.R, cornerCircle.Y - cornerCircle.R, 2 * cornerCircle.R, 2 * cornerCircle.R);
			GfxSystem_DrawTexturedCirclePieceEx(system, cornerCircle, Pi32, ThreeHalfsPi32, color, texture, RelativeRec(rectangle, cornerFullCircleRec, sourceRec));
		}
		if (radiusTR > 0) //TopRight Quarter Circle
		{
			Circle cornerCircle = NewCircle(rectangle.X + rectangle.Width - radiusTR, rectangle.Y + radiusTR, radiusTR);
			rec cornerFullCircleRec = NewRec(cornerCircle.X - cornerCircle.R, cornerCircle.Y - cornerCircle.R, 2 * cornerCircle.R, 2 * cornerCircle.R);
			GfxSystem_DrawTexturedCirclePieceEx(system, cornerCircle, ThreeHalfsPi32, TwoPi32, color, texture, RelativeRec(rectangle, cornerFullCircleRec, sourceRec));
		}
	}
	else
	{
		GfxSystem_DrawTexturedRectangleEx(system, rectangle, color, texture, sourceRec);
	}
}
PEXPI void GfxSystem_DrawTexturedRoundedRectangle(GfxSystem* system, rec rectangle, r32 radius, Color32 color, Texture* texture, rec sourceRec)
{
	GfxSystem_DrawTexturedRoundedRectangleEx(system, rectangle, radius, radius, radius, radius, color, texture, sourceRec);
}
PEXPI void GfxSystem_DrawRoundedRectangleEx(GfxSystem* system, rec rectangle, r32 radiusTL, r32 radiusTR, r32 radiusBR, r32 radiusBL, Color32 color)
{
	GfxSystem_DrawTexturedRoundedRectangleEx(system, rectangle, radiusTL, radiusTR, radiusBR, radiusBL, color, nullptr, (rec)Rec_Zero_Const);
}
PEXPI void GfxSystem_DrawRoundedRectangle(GfxSystem* system, rec rectangle, r32 radius, Color32 color)
{
	GfxSystem_DrawTexturedRoundedRectangleEx(system, rectangle, radius, radius, radius, radius, color, nullptr, (rec)Rec_Zero_Const);
}

PEXP void GfxSystem_DrawTexturedRingPieceEx(GfxSystem* system, Circle circle, r32 thickness, r32 angleMin, r32 angleMax, Color32 color, Texture* texture, rec sourceRec)
{
	NotNull(system);
	Assert(thickness >= 0.0f);
	
	r32 innerRadius = MaxR32(0.0f, circle.Radius - thickness);
	r32 innerRadiusPercent = innerRadius / circle.Radius;
	if (innerRadiusPercent <= (1.0f / GFX_SYSTEM_RING_NUM_THICKNESSES) / 2.0f)
	{
		GfxSystem_DrawTexturedCirclePieceEx(system, circle, angleMin, angleMax, color, texture, sourceRec);
		return;
	}
	uxx ringBufferIndex = (uxx)RoundR32i(innerRadiusPercent / (1.0f / GFX_SYSTEM_RING_NUM_THICKNESSES)) - 1;
	Assert(ringBufferIndex < GFX_SYSTEM_RING_NUM_THICKNESSES);
	
	uxx startSideIndex = (uxx)RoundR32i((AngleFixR32(angleMin) / TwoPi32) * GFX_SYSTEM_RING_NUM_SIDES);
	uxx endSideIndex = (uxx)RoundR32i((AngleFixR32(angleMax) / TwoPi32) * GFX_SYSTEM_RING_NUM_SIDES);
	if (endSideIndex == startSideIndex)
	{
		//If angleMin>=angleMax, treat it as if we are drawing a full circle, since it is ambigious, otherwise if the angles are slightly different then don't draw anything because it's less than one segment angle
		if (AreSimilarOrGreaterR32(AngleFixR32(angleMin), AngleFixR32(angleMax), DEFAULT_R32_TOLERANCE))
		{
			startSideIndex = 0;
			endSideIndex = GFX_SYSTEM_RING_NUM_SIDES;
		}
		else { return; }
	}
	if (endSideIndex < startSideIndex)
	{
		//NOTE: To draw an arc that passes over 0/TwoPi angle, we really need to do two draw calls for either side. So we just recurse one level to accomplish this cleanly
		r32 minAngleForSingleSegment = (TwoPi32 / GFX_SYSTEM_RING_NUM_SIDES) / 2.0f;
		if (AngleFixR32(angleMin) <= TwoPi32 - minAngleForSingleSegment) { GfxSystem_DrawTexturedRingPieceEx(system, circle, thickness, angleMin, TwoPi32-DEFAULT_R32_TOLERANCE, color, texture, sourceRec); }
		if (AngleFixR32(angleMax) >= minAngleForSingleSegment) { GfxSystem_DrawTexturedRingPieceEx(system, circle, thickness, 0, angleMax, color, texture, sourceRec); }
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
	
	GfxSystem_BindVertBuffer(system, &system->ringBuffers[ringBufferIndex]);
	GfxSystem_DrawVerticesEx(system, startSideIndex*6, (endSideIndex - startSideIndex)*6);
}
PEXPI void GfxSystem_DrawTexturedRingPiece(GfxSystem* system, Circle circle, r32 thickness, r32 angleMin, r32 angleMax, Color32 color, Texture* texture)
{
	rec sourceRec = (texture != nullptr) ? NewRec(0, 0, (r32)texture->Width, (r32)texture->Height) : (rec)Rec_Zero_Const;
	GfxSystem_DrawTexturedRingPieceEx(system, circle, thickness, angleMin, angleMax, color, texture, sourceRec);
}
PEXPI void GfxSystem_DrawTexturedRingEx(GfxSystem* system, Circle circle, r32 thickness, Color32 color, Texture* texture, rec sourceRec)
{
	GfxSystem_DrawTexturedRingPieceEx(system, circle, thickness, 0, TwoPi32-DEFAULT_R32_TOLERANCE, color, texture, sourceRec);
}
PEXPI void GfxSystem_DrawTexturedRing(GfxSystem* system, Circle circle, r32 thickness, Color32 color, Texture* texture)
{
	rec sourceRec = (texture != nullptr) ? NewRec(0, 0, (r32)texture->Width, (r32)texture->Height) : (rec)Rec_Zero_Const;
	GfxSystem_DrawTexturedRingPieceEx(system, circle, thickness, 0, TwoPi32-DEFAULT_R32_TOLERANCE, color, texture, sourceRec);
}
PEXPI void GfxSystem_DrawRingPiece(GfxSystem* system, Circle circle, r32 thickness, r32 angleMin, r32 angleMax, Color32 color)
{
	GfxSystem_DrawTexturedRingPieceEx(system, circle, thickness, angleMin, angleMax, color, nullptr, (rec)Rec_Zero_Const);
}
PEXPI void GfxSystem_DrawRing(GfxSystem* system, Circle circle, r32 thickness, Color32 color)
{
	GfxSystem_DrawTexturedRingPieceEx(system, circle, thickness, 0, TwoPi32-DEFAULT_R32_TOLERANCE, color, nullptr, (rec)Rec_Zero_Const);
}

PEXP void GfxSystem_DrawTexturedRoundedRectangleOutlineEx(GfxSystem* system, rec rectangle, r32 thickness, r32 radiusTL, r32 radiusTR, r32 radiusBR, r32 radiusBL, Color32 color, bool outside, Texture* texture, rec sourceRec)
{
	NotNull(system);
	if (outside)
	{
		if (radiusTL > 0) { radiusTL += thickness; }
		if (radiusTR > 0) { radiusTR += thickness; }
		if (radiusBR > 0) { radiusBR += thickness; }
		if (radiusBL > 0) { radiusBL += thickness; }
		GfxSystem_DrawTexturedRoundedRectangleOutlineEx(system, NewRec(rectangle.X - thickness, rectangle.Y - thickness, rectangle.Width + thickness*2, rectangle.Height + thickness*2), thickness, radiusTL, radiusTR, radiusBR, radiusBL, color, false, texture, sourceRec);
		return;
	}
	if (radiusTL + radiusBL > rectangle.Height) { radiusTL = rectangle.Height/2; radiusBL = rectangle.Height/2; }
	if (radiusTR + radiusBR > rectangle.Height) { radiusTR = rectangle.Height/2; radiusBR = rectangle.Height/2; }
	if (radiusTL + radiusTR > rectangle.Width) { radiusTL = rectangle.Width/2; radiusTR = rectangle.Width/2; }
	if (radiusBL + radiusBR > rectangle.Width) { radiusBL = rectangle.Width/2; radiusBR = rectangle.Width/2; }
	if (radiusTL != 0 || radiusTR != 0 || radiusBL != 0 || radiusBR != 0)
	{
		rec topRec = NewRec(rectangle.X + radiusTL, rectangle.Y, rectangle.Width - radiusTR - radiusTL, thickness);
		GfxSystem_DrawTexturedRectangleEx(system, topRec, color, texture, RelativeRec(rectangle, topRec, sourceRec)); //top side
		rec leftRec = NewRec(rectangle.X, rectangle.Y + radiusTL, thickness, rectangle.Height - radiusBL - radiusTL);
		GfxSystem_DrawTexturedRectangleEx(system, leftRec, color, texture, RelativeRec(rectangle, leftRec, sourceRec)); //left side
		rec bottomRec = NewRec(rectangle.X + radiusBL, rectangle.Y + rectangle.Height - thickness, rectangle.Width - radiusBR - radiusBL, thickness);
		GfxSystem_DrawTexturedRectangleEx(system, bottomRec, color, texture, RelativeRec(rectangle, bottomRec, sourceRec)); //bottom side
		rec rightRec = NewRec(rectangle.X + rectangle.Width - thickness, rectangle.Y + radiusTR, thickness, rectangle.Height - radiusBR - radiusTR);
		GfxSystem_DrawTexturedRectangleEx(system, rightRec, color, texture, RelativeRec(rectangle, rightRec, sourceRec)); //right side
		
		if (radiusBR > 0) //BottomRight Ring Piece
		{
			Circle cornerCircle = NewCircle(rectangle.X + rectangle.Width - radiusBR, rectangle.Y + rectangle.Height - radiusBR, radiusBR);
			rec cornerFullCircleRec = NewRec(cornerCircle.X - cornerCircle.R, cornerCircle.Y - cornerCircle.R, 2 * cornerCircle.R, 2 * cornerCircle.R);
			GfxSystem_DrawTexturedRingPieceEx(system, cornerCircle, thickness, 0, HalfPi32, color, texture, RelativeRec(rectangle, cornerFullCircleRec, sourceRec));
		}
		if (radiusBL > 0) //BottomLeft Ring Piece
		{
			Circle cornerCircle = NewCircle(rectangle.X + radiusBL, rectangle.Y + rectangle.Height - radiusBL, radiusBL);
			rec cornerFullCircleRec = NewRec(cornerCircle.X - cornerCircle.R, cornerCircle.Y - cornerCircle.R, 2 * cornerCircle.R, 2 * cornerCircle.R);
			GfxSystem_DrawTexturedRingPieceEx(system, cornerCircle, thickness, HalfPi32, Pi32, color, texture, RelativeRec(rectangle, cornerFullCircleRec, sourceRec));
		}
		if (radiusTL > 0) //TopLeft Ring Piece
		{
			Circle cornerCircle = NewCircle(rectangle.X + radiusTL, rectangle.Y + radiusTL, radiusTL);
			rec cornerFullCircleRec = NewRec(cornerCircle.X - cornerCircle.R, cornerCircle.Y - cornerCircle.R, 2 * cornerCircle.R, 2 * cornerCircle.R);
			GfxSystem_DrawTexturedRingPieceEx(system, cornerCircle, thickness, Pi32, ThreeHalfsPi32, color, texture, RelativeRec(rectangle, cornerFullCircleRec, sourceRec));
		}
		if (radiusTR > 0) //TopRight Ring Piece
		{
			Circle cornerCircle = NewCircle(rectangle.X + rectangle.Width - radiusTR, rectangle.Y + radiusTR, radiusTR);
			rec cornerFullCircleRec = NewRec(cornerCircle.X - cornerCircle.R, cornerCircle.Y - cornerCircle.R, 2 * cornerCircle.R, 2 * cornerCircle.R);
			GfxSystem_DrawTexturedRingPieceEx(system, cornerCircle, thickness, ThreeHalfsPi32, TwoPi32, color, texture, RelativeRec(rectangle, cornerFullCircleRec, sourceRec));
		}
	}
	else
	{
		GfxSystem_DrawTexturedRectangleOutlineEx(system, rectangle, thickness, color, outside, texture, sourceRec);
	}
}
PEXPI void GfxSystem_DrawTexturedRoundedRectangleOutline(GfxSystem* system, rec rectangle, r32 thickness, r32 radius, Color32 color, bool outside, Texture* texture, rec sourceRec)
{
	GfxSystem_DrawTexturedRoundedRectangleOutlineEx(system, rectangle, thickness, radius, radius, radius, radius, color, outside, texture, sourceRec);
}
PEXPI void GfxSystem_DrawRoundedRectangleOutlineEx(GfxSystem* system, rec rectangle, r32 thickness, r32 radiusTL, r32 radiusTR, r32 radiusBR, r32 radiusBL, Color32 color, bool outside)
{
	GfxSystem_DrawTexturedRoundedRectangleOutlineEx(system, rectangle, thickness, radiusTL, radiusTR, radiusBR, radiusBL, color, outside, nullptr, (rec)Rec_Zero_Const);
}
PEXPI void GfxSystem_DrawRoundedRectangleOutline(GfxSystem* system, rec rectangle, r32 thickness, r32 radius, Color32 color)
{
	GfxSystem_DrawTexturedRoundedRectangleOutlineEx(system, rectangle, thickness, radius, radius, radius, radius, color, true, nullptr, (rec)Rec_Zero_Const);
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

PEXPI void GfxSystem_SetTextBackgroundColor(GfxSystem* system, Color32 color)
{
	NotNull(system);
	system->state.textBackgroundColor = color;
}

// +====================================+
// | GfxSystem_FontFlowDrawCharCallback |
// +====================================+
// void GfxSystem_FontFlowDrawCharCallback(FontFlowState* state, FontFlow* flow, rec glyphDrawRec, u32 codepoint, FontAtlas* atlas, FontGlyph* glyph, FontGlyphMetrics glyphMetrics)
FONT_FLOW_DRAW_CHAR_DEF(GfxSystem_FontFlowDrawCharCallback)
{
	NotNull(state);
	NotNull(state->contextPntr);
	NotNull(atlas);
	NotNull(glyph);
	UNUSED(flow);
	UNUSED(codepoint);
	UNUSED(glyphMetrics);
	GfxSystem* system = (GfxSystem*)state->contextPntr;
	Color32 drawColor = state->currentStyle.color;
	if (IsFlagSet(state->currentStyle.fontStyle, FontStyleFlag_Highlighted) && state->backgroundColor.a != 0)
	{
		drawColor = state->backgroundColor;
	}
	CommitFontAtlasTextureUpdates(state->font, atlas);
	rec atlasSourceRec = ToRecFromi(NewReciV(glyph->atlasSourcePos, glyph->metrics.glyphSize));
	GfxSystem_DrawTexturedRectangleEx(system, glyphDrawRec, drawColor, &atlas->texture, atlasSourceRec);
}

// +==========================================+
// | GfxSystem_FontFlowDrawHighlightCallback  |
// +==========================================+
// void GfxSystem_FontFlowDrawHighlightCallback(FontFlowState* state, FontFlow* flow, rec highlightRec)
FONT_FLOW_DRAW_HIGHLIGHT_DEF(GfxSystem_FontFlowDrawHighlightCallback)
{
	UNUSED(flow);
	NotNull(state);
	NotNull(state->contextPntr);
	GfxSystem* system = (GfxSystem*)state->contextPntr;
	GfxSystem_DrawRectangle(system, highlightRec, state->currentStyle.color);
}

PEXP Result GfxSystem_DrawWrappedRichTextWithFont(GfxSystem* system, PigFont* font, r32 fontSize, u8 styleFlags, RichStr text, v2 position, r32 wrapWidth, Color32 color)
{
	NotNull(system);
	NotNull(font);
	NotNullStr(text.fullPiece.str);
	TracyCZoneN(_funcZone, "GfxSystem_DrawText", true);
	
	FontFlowState state = ZEROED;
	state.contextPntr = (void*)system;
	state.font = font;
	state.text = text;
	state.startFontSize = fontSize;
	state.startFontStyle = styleFlags;
	state.startColor = color;
	state.alignPixelSize = system->state.alignPixelSize;
	state.position = position;
	state.wrapWidth = wrapWidth;
	state.backgroundColor = system->state.textBackgroundColor;
	FontFlowCallbacks callbacks = ZEROED;
	callbacks.drawChar = GfxSystem_FontFlowDrawCharCallback;
	callbacks.drawHighlight = GfxSystem_FontFlowDrawHighlightCallback;
	
	Result result = DoFontFlow(&state, &callbacks, &system->prevFontFlow);
	
	TracyCZoneEnd(_funcZone);
	return result;
}
PEXP Result GfxSystem_DrawRichTextWithFont(GfxSystem* system, PigFont* font, r32 fontSize, u8 styleFlags, RichStr text, v2 position, Color32 color)
{
	return GfxSystem_DrawWrappedRichTextWithFont(system, font, fontSize, styleFlags, text, position, 0.0f, color);
}
PEXPI Result GfxSystem_DrawTextWithFont(GfxSystem* system, PigFont* font, r32 fontSize, u8 styleFlags, Str8 text, v2 position, Color32 color)
{
	return GfxSystem_DrawWrappedRichTextWithFont(system, font, fontSize, styleFlags, ToRichStr(text), position, 0.0f, color);
}
PEXPI Result GfxSystem_DrawTextAtSize(GfxSystem* system, r32 fontSize, Str8 text, v2 position, Color32 color)
{
	NotNull(system);
	NotNull(system->state.font);
	return GfxSystem_DrawWrappedRichTextWithFont(system, system->state.font, fontSize, system->state.fontStyleFlags, ToRichStr(text), position, 0.0f, color);
}
PEXPI Result GfxSystem_DrawTextBold(GfxSystem* system, Str8 text, v2 position, Color32 color)
{
	NotNull(system);
	NotNull(system->state.font);
	return GfxSystem_DrawWrappedRichTextWithFont(system, system->state.font, system->state.fontSize, (system->state.fontStyleFlags | FontStyleFlag_Bold), ToRichStr(text), position, 0.0f, color);
}
PEXPI Result GfxSystem_DrawTextItalic(GfxSystem* system, Str8 text, v2 position, Color32 color)
{
	NotNull(system);
	NotNull(system->state.font);
	return GfxSystem_DrawWrappedRichTextWithFont(system, system->state.font, system->state.fontSize, (system->state.fontStyleFlags | FontStyleFlag_Italic), ToRichStr(text), position, 0.0f, color);
}
PEXPI Result GfxSystem_DrawText(GfxSystem* system, Str8 text, v2 position, Color32 color)
{
	NotNull(system);
	NotNull(system->state.font);
	return GfxSystem_DrawWrappedRichTextWithFont(system, system->state.font, system->state.fontSize, system->state.fontStyleFlags, ToRichStr(text), position, 0.0f, color);
}
PEXPI Result GfxSystem_DrawWrappedText(GfxSystem* system, Str8 text, v2 position, r32 wrapWidth, Color32 color)
{
	NotNull(system);
	NotNull(system->state.font);
	return GfxSystem_DrawWrappedRichTextWithFont(system, system->state.font, system->state.fontSize, system->state.fontStyleFlags, ToRichStr(text), position, wrapWidth, color);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _GFX_SYSTEM_H

#if defined(_UI_IMGUI_H) && defined(_GFX_SYSTEM_H)
#include "cross/cross_imgui_and_gfx_system.h"
#endif
