/*
File:   gfx_vert_buffer.h
Author: Taylor Robbins
Date:   01\29\2025
Description:
	** Contains the VertBuffer structure which holds a sg_buffer from sokol_gfx as well
	** as other metadata about the vertices and buffer when it was created.
*/

#ifndef _GFX_VERT_BUFFER_H
#define _GFX_VERT_BUFFER_H

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

_Static_assert(MAX_NUM_VERT_ATTRIBUTES <= SG_MAX_VERTEX_ATTRIBUTES);

typedef enum VertBufferUsage VertBufferUsage;
enum VertBufferUsage
{
	VertBufferUsage_None = 0,
	VertBufferUsage_Static,
	VertBufferUsage_Dynamic,
	VertBufferUsage_Streaming,
	VertBufferUsage_Count,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetVertBufferUsageStr(VertBufferUsage enumValue);
#else
PEXP const char* GetVertBufferUsageStr(VertBufferUsage enumValue)
{
	switch (enumValue)
	{
		case VertBufferUsage_None:      return "None";
		case VertBufferUsage_Static:    return "Static";
		case VertBufferUsage_Dynamic:   return "Dynamic";
		case VertBufferUsage_Streaming: return "Streaming";
		default: return "Unknown";
	}
}
#endif

typedef struct VertBuffer VertBuffer;
struct VertBuffer
{
	Arena* arena;
	Result error;
	sg_buffer handle;
	Str8 name;
	VertBufferUsage usage;
	uxx numAttributes;
	VertAttribute attributes[MAX_NUM_VERT_ATTRIBUTES];
	bool hasIndices;
	uxx vertexSize;
	uxx indexSize;
	uxx numVertices;
	uxx numIndices;
	//These are only filled if makeCopy was true when buffer was created
	union { void* verticesPntr; Vertex2D* vertices2D; Vertex3D* vertices3D; };
	union { void* indicesPntr; i32* indicesI32; i16* indicesI16; };
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void FreeVertBuffer(VertBuffer* buffer);
	void FreeVertBuffer(VertBuffer* buffer);
	VertBuffer InitVertBufferEx(Arena* arena, Str8 name, VertBufferUsage usage, uxx verticesSize, const void* verticesPntr, uxx numAttributes, const VertAttribute* attributes, bool makeCopy);
	PIG_CORE_INLINE VertBuffer InitVertBuffer2D(Arena* arena, Str8 name, VertBufferUsage usage, uxx numVertices, const Vertex2D* verticesPntr, bool makeCopy);
	PIG_CORE_INLINE VertBuffer InitVertBuffer3D(Arena* arena, Str8 name, VertBufferUsage usage, uxx numVertices, const Vertex3D* verticesPntr, bool makeCopy);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void FreeVertBuffer(VertBuffer* buffer)
{
	NotNull(buffer);
	if (buffer->arena != nullptr)
	{
		sg_destroy_buffer(buffer->handle);
		FreeStr8WithNt(buffer->arena, &buffer->name);
		if (buffer->verticesPntr != nullptr)
		{
			FreeMem(buffer->arena, buffer->verticesPntr, buffer->numVertices * buffer->vertexSize);
		}
		if (buffer->indicesPntr != nullptr)
		{
			FreeMem(buffer->arena, buffer->indicesPntr, buffer->numIndices * buffer->indexSize);
		}
	}
	ClearPointer(buffer);
}

PEXP VertBuffer InitVertBufferEx(Arena* arena, Str8 name, VertBufferUsage usage, uxx verticesSize, const void* verticesPntr, uxx numAttributes, const VertAttribute* attributes, bool makeCopy)
{
	NotNull(arena);
	NotNullStr(name);
	Assert(verticesSize > 0);
	NotNull(verticesPntr);
	VertBuffer result = ZEROED;
	result.arena = arena;
	result.name = AllocStrAndCopy(arena, name.length, name.chars, true);
	NotNull(result.name.chars);
	sg_usage sokolUsage = SG_USAGE_IMMUTABLE;
	switch (usage)
	{
		case VertBufferUsage_Static: sokolUsage = SG_USAGE_IMMUTABLE; break;
		case VertBufferUsage_Dynamic: sokolUsage = SG_USAGE_DYNAMIC; break;
		case VertBufferUsage_Streaming: sokolUsage = SG_USAGE_STREAM; break;
	}
	sg_buffer_desc bufferDesc = {
		.data = (sg_range){verticesPntr, verticesSize},
		.usage = sokolUsage,
		.label = result.name.chars,
	};
	result.handle = sg_make_buffer(&bufferDesc);
	if (result.handle.id == SG_INVALID_ID)
	{
		FreeStr8WithNt(arena, &result.name);
		result.error = Result_SokolError;
		return result;
	}
	Assert(numAttributes > 0);
	Assert(numAttributes <= MAX_NUM_VERT_ATTRIBUTES);
	MyMemCopy(&result.attributes, attributes, sizeof(VertAttribute) * numAttributes);
	result.numAttributes = numAttributes;
	result.vertexSize = 0;
	for (uxx aIndex = 0; aIndex < numAttributes; aIndex++)
	{
		AssertMsg((attributes[aIndex].size % sizeof(r32)) == 0, "All attributes in a vertBuffer should be floats!");
		result.vertexSize = MaxUXX(result.vertexSize, attributes[aIndex].offset + attributes[aIndex].size);
	}
	DebugAssert(result.vertexSize > 0);
	AssertMsg((verticesSize % result.vertexSize) == 0, "verticesSize did not match size of vertex!");
	result.numVertices = verticesSize / result.vertexSize;
	if (makeCopy)
	{
		result.verticesPntr = AllocMem(arena, verticesSize);
		if (result.verticesPntr == nullptr)
		{
			FreeStr8WithNt(arena, &result.name);
			result.error = Result_FailedToAllocateMemory;
			return result;
		}
		MyMemCopy(result.verticesPntr, verticesPntr, verticesSize);
	}
	result.error = Result_Success;
	return result;
}
PEXPI VertBuffer InitVertBuffer2D(Arena* arena, Str8 name, VertBufferUsage usage, uxx numVertices, const Vertex2D* verticesPntr, bool makeCopy)
{
	_Static_assert(sizeof(Vertex2D) == sizeof(r32)*8);
	VertAttribute attributes[3] = {
		{ .type = VertAttributeType_Position, .size = sizeof(r32)*2, .offset = sizeof(r32)*0 },
		{ .type = VertAttributeType_TexCoord, .size = sizeof(r32)*2, .offset = sizeof(r32)*2 },
		{ .type = VertAttributeType_Color,    .size = sizeof(r32)*4, .offset = sizeof(r32)*4 },
	};
	return InitVertBufferEx(arena, name, usage, numVertices * sizeof(Vertex2D), verticesPntr, ArrayCount(attributes), &attributes[0], makeCopy);
}
PEXPI VertBuffer InitVertBuffer3D(Arena* arena, Str8 name, VertBufferUsage usage, uxx numVertices, const Vertex3D* verticesPntr, bool makeCopy)
{
	_Static_assert(sizeof(Vertex3D) == sizeof(r32)*9);
	VertAttribute attributes[3] = {
		{ .type = VertAttributeType_Position, .size = sizeof(r32)*3, .offset = sizeof(r32)*0 },
		{ .type = VertAttributeType_TexCoord, .size = sizeof(r32)*2, .offset = sizeof(r32)*3 },
		{ .type = VertAttributeType_Color,    .size = sizeof(r32)*4, .offset = sizeof(r32)*5 },
	};
	return InitVertBufferEx(arena, name, usage, numVertices * sizeof(Vertex3D), verticesPntr, ArrayCount(attributes), &attributes[0], makeCopy);
}

//TODO: Implement InitVertIndexedBufferEx
//TODO: Implement InitVertIndexedBuffer2D
//TODO: Implement InitVertIndexedBuffer3D

PEXPI void BindVertBuffer(sg_bindings* bindings, VertBuffer* buffer, uxx bufferIndex)
{
	NotNull(bindings);
	NotNull(buffer);
	Assert(buffer->handle.id != SG_INVALID_ID);
	bindings->vertex_buffers[0] = buffer->handle;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL

#endif //  _GFX_VERT_BUFFER_H

#if defined(_GFX_SHADER_H) && defined(_GFX_VERT_BUFFER_H)
#include "cross/cross_shader_and_vert_buffer.h"
#endif
