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
#include "misc/misc_sokol_gfx_include.h"

#if BUILD_WITH_SOKOL_GFX

#ifndef SOKOL_GFX_INCLUDED
#error Somehow sokol_gfx.h was not included properly before gfx_shader.h!
#endif

#if LANGUAGE_IS_C
_Static_assert(MAX_NUM_VERT_ATTRIBUTES <= SG_MAX_VERTEX_ATTRIBUTES);
#endif

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

typedef plex VertBuffer VertBuffer;
plex VertBuffer
{
	Arena* arena;
	Result error;
	sg_buffer handle;
	sg_buffer indicesHandle;
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
	car { void* verticesPntr; Vertex2D* vertices2D; Vertex3D* vertices3D; };
	car { void* indicesPntr; i32* indicesI32; i16* indicesI16; };
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
	void AddIndicesToVertBufferEx(VertBuffer* buffer, uxx indexSize, uxx numIndices, const void* indicesPntr, bool makeCopy);
	PIG_CORE_INLINE void AddIndicesToVertBufferU8(VertBuffer* buffer, uxx numIndices, const u8* indicesPntr, bool makeCopy);
	PIG_CORE_INLINE void AddIndicesToVertBufferU16(VertBuffer* buffer, uxx numIndices, const u16* indicesPntr, bool makeCopy);
	PIG_CORE_INLINE void AddIndicesToVertBufferU32(VertBuffer* buffer, uxx numIndices, const u32* indicesPntr, bool makeCopy);
	PIG_CORE_INLINE void AddIndicesToVertBufferU64(VertBuffer* buffer, uxx numIndices, const u64* indicesPntr, bool makeCopy);
	void ChangeVerticesInVertBufferEx(VertBuffer* buffer, uxx numVertices, uxx vertexSize, const void* verticesPntr);
	PIG_CORE_INLINE void ChangeVerticesInVertBuffer2D(VertBuffer* buffer, uxx numVertices, const Vertex2D* verticesPntr);
	PIG_CORE_INLINE void ChangeVerticesInVertBuffer3D(VertBuffer* buffer, uxx numVertices, const Vertex3D* verticesPntr);
	void ChangeIndicesInVertBufferEx(VertBuffer* buffer, uxx numIndices, uxx indexSize, const void* indicesPntr);
	PIG_CORE_INLINE void ChangeIndicesInVertBufferU8(VertBuffer* buffer, uxx numIndices, const u8* indicesPntr);
	PIG_CORE_INLINE void ChangeIndicesInVertBufferU16(VertBuffer* buffer, uxx numIndices, const u16* indicesPntr);
	PIG_CORE_INLINE void ChangeIndicesInVertBufferU32(VertBuffer* buffer, uxx numIndices, const u32* indicesPntr);
	PIG_CORE_INLINE void ChangeIndicesInVertBufferU64(VertBuffer* buffer, uxx numIndices, const u64* indicesPntr);
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
	VertBuffer result = ZEROED;
	result.arena = arena;
	result.name = AllocStrAndCopy(arena, name.length, name.chars, true);
	NotNull(result.name.chars);
	result.usage = usage;
	sg_usage sokolUsage = SG_USAGE_IMMUTABLE;
	switch (usage)
	{
		case VertBufferUsage_Static: sokolUsage = SG_USAGE_IMMUTABLE; break;
		case VertBufferUsage_Dynamic: sokolUsage = SG_USAGE_DYNAMIC; break;
		case VertBufferUsage_Streaming: sokolUsage = SG_USAGE_STREAM; break;
	}
	sg_buffer_desc bufferDesc = {
		.type = SG_BUFFERTYPE_VERTEXBUFFER,
		.usage = sokolUsage,
		.label = result.name.chars,
	};
	if (verticesPntr != nullptr) { bufferDesc.data = (sg_range){verticesPntr, verticesSize}; }
	else { bufferDesc.size = verticesSize; }
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
		if (verticesPntr != nullptr) { MyMemCopy(result.verticesPntr, verticesPntr, verticesSize); }
	}
	result.error = Result_Success;
	return result;
}

PEXPI VertBuffer InitVertBuffer2D(Arena* arena, Str8 name, VertBufferUsage usage, uxx numVertices, const Vertex2D* verticesPntr, bool makeCopy)
{
	#if LANGUAGE_IS_C
	_Static_assert(sizeof(Vertex2D) == sizeof(r32)*8);
	#endif
	VertAttribute attributes[3] = {
		{ .type = VertAttributeType_Position, .size = sizeof(r32)*2, .offset = sizeof(r32)*0 },
		{ .type = VertAttributeType_TexCoord, .size = sizeof(r32)*2, .offset = sizeof(r32)*2 },
		{ .type = VertAttributeType_Color,    .size = sizeof(r32)*4, .offset = sizeof(r32)*4 },
	};
	return InitVertBufferEx(arena, name, usage, numVertices * sizeof(Vertex2D), verticesPntr, ArrayCount(attributes), &attributes[0], makeCopy);
}
PEXPI VertBuffer InitVertBuffer3D(Arena* arena, Str8 name, VertBufferUsage usage, uxx numVertices, const Vertex3D* verticesPntr, bool makeCopy)
{
	#if LANGUAGE_IS_C
	_Static_assert(sizeof(Vertex3D) == sizeof(r32)*12);
	#endif
	VertAttribute attributes[4] = {
		{ .type = VertAttributeType_Position, .size = sizeof(r32)*3, .offset = sizeof(r32)*0 },
		{ .type = VertAttributeType_Normal,   .size = sizeof(r32)*3, .offset = sizeof(r32)*3 },
		{ .type = VertAttributeType_TexCoord, .size = sizeof(r32)*2, .offset = sizeof(r32)*6 },
		{ .type = VertAttributeType_Color,    .size = sizeof(r32)*4, .offset = sizeof(r32)*8 },
	};
	return InitVertBufferEx(arena, name, usage, numVertices * sizeof(Vertex3D), verticesPntr, ArrayCount(attributes), &attributes[0], makeCopy);
}

PEXP void AddIndicesToVertBufferEx(VertBuffer* buffer, uxx indexSize, uxx numIndices, const void* indicesPntr, bool makeCopy)
{
	NotNull(buffer);
	NotNull(buffer->arena);
	Assert(buffer->hasIndices == false);
	if (numIndices > 0)
	{
		sg_usage sokolUsage = SG_USAGE_IMMUTABLE;
		switch (buffer->usage)
		{
			case VertBufferUsage_Static: sokolUsage = SG_USAGE_IMMUTABLE; break;
			case VertBufferUsage_Dynamic: sokolUsage = SG_USAGE_DYNAMIC; break;
			case VertBufferUsage_Streaming: sokolUsage = SG_USAGE_STREAM; break;
		}
		buffer->hasIndices = true;
		buffer->indexSize = indexSize;
		buffer->numIndices = numIndices;
		sg_buffer_desc bufferDesc = {
			.type = SG_BUFFERTYPE_INDEXBUFFER,
			.usage = sokolUsage,
			.label = buffer->name.chars, //TODO: Should we append like "_indices" or something to the name?
		};
		if (indicesPntr != nullptr) { bufferDesc.data = (sg_range){indicesPntr, (indexSize * numIndices)}; }
		else { bufferDesc.size = (indexSize * numIndices); }
		buffer->indicesHandle = sg_make_buffer(&bufferDesc);
		if (makeCopy)
		{
			buffer->indicesPntr = AllocMem(buffer->arena, indexSize * numIndices);
			NotNull(buffer->indicesPntr);
			if (indicesPntr != nullptr) { MyMemCopy(buffer->indicesPntr, indicesPntr, indexSize * numIndices); }
		}
	}
}
PEXPI void AddIndicesToVertBufferU8(VertBuffer* buffer, uxx numIndices, const u8* indicesPntr, bool makeCopy)
{
	AddIndicesToVertBufferEx(buffer, sizeof(u8), numIndices, indicesPntr, makeCopy);
}
PEXPI void AddIndicesToVertBufferU16(VertBuffer* buffer, uxx numIndices, const u16* indicesPntr, bool makeCopy)
{
	AddIndicesToVertBufferEx(buffer, sizeof(u16), numIndices, indicesPntr, makeCopy);
}
PEXPI void AddIndicesToVertBufferU32(VertBuffer* buffer, uxx numIndices, const u32* indicesPntr, bool makeCopy)
{
	AddIndicesToVertBufferEx(buffer, sizeof(u32), numIndices, indicesPntr, makeCopy);
}
PEXPI void AddIndicesToVertBufferU64(VertBuffer* buffer, uxx numIndices, const u64* indicesPntr, bool makeCopy)
{
	AddIndicesToVertBufferEx(buffer, sizeof(u64), numIndices, indicesPntr, makeCopy);
}

//TODO: Implement InitVertIndexedBufferEx
//TODO: Implement InitVertIndexedBuffer2D
//TODO: Implement InitVertIndexedBuffer3D

//NOTE: Sokol does not allow us to change a subset of the vertices, we have to change all the vertices at once!
PEXP void ChangeVerticesInVertBufferEx(VertBuffer* buffer, uxx numVertices, uxx vertexSize, const void* verticesPntr)
{
	NotNull(buffer);
	NotNull(buffer->arena);
	Assert(buffer->usage == VertBufferUsage_Dynamic || buffer->usage == VertBufferUsage_Streaming);
	Assert(numVertices <= buffer->numVertices);
	Assert(vertexSize == buffer->vertexSize);
	if (numVertices == 0) { return; }
	NotNull(verticesPntr);
	
	ScratchBegin1(scratch, buffer->arena);
	uxx allVerticesSize = buffer->vertexSize * buffer->numVertices;
	u8* allVerticesPntr = (u8*)AllocMem(scratch, allVerticesSize);
	NotNull(allVerticesPntr);
	MyMemCopy(allVerticesPntr, verticesPntr, vertexSize * numVertices);
	if (numVertices < buffer->numVertices) { MyMemSet(&allVerticesPntr[vertexSize * numVertices], 0x00, vertexSize * (buffer->numVertices - numVertices)); }
	
	//TODO: Maybe we don't have to update the entire buffer? Maybe we can just update the beginning? Is that more performant since we don't have to pass potentially a lot of zeroes?
	sg_range verticesRange = (sg_range){verticesPntr, allVerticesSize};
	sg_update_buffer(buffer->handle, &verticesRange);
	
	ScratchEnd(scratch);
	
	if (buffer->verticesPntr != nullptr)
	{
		MyMemCopy(buffer->verticesPntr, verticesPntr, vertexSize * numVertices);
	}
}
PEXPI void ChangeVerticesInVertBuffer2D(VertBuffer* buffer, uxx numVertices, const Vertex2D* verticesPntr)
{
	ChangeVerticesInVertBufferEx(buffer, numVertices, sizeof(Vertex2D), verticesPntr);
}
PEXPI void ChangeVerticesInVertBuffer3D(VertBuffer* buffer, uxx numVertices, const Vertex3D* verticesPntr)
{
	ChangeVerticesInVertBufferEx(buffer, numVertices, sizeof(Vertex3D), verticesPntr);
}

//NOTE: Sokol does not allow us to change a subset of the indices, we have to change all the indices at once!
PEXP void ChangeIndicesInVertBufferEx(VertBuffer* buffer, uxx numIndices, uxx indexSize, const void* indicesPntr)
{
	NotNull(buffer);
	NotNull(buffer->arena);
	Assert(buffer->hasIndices);
	Assert(buffer->usage == VertBufferUsage_Dynamic || buffer->usage == VertBufferUsage_Streaming);
	Assert(numIndices <= buffer->numIndices);
	Assert(indexSize == buffer->indexSize);
	if (numIndices == 0) { return; }
	NotNull(indicesPntr);
	
	ScratchBegin1(scratch, buffer->arena);
	uxx allIndicesSize = buffer->indexSize * buffer->numIndices;
	u8* allIndicesPntr = (u8*)AllocMem(scratch, allIndicesSize);
	NotNull(allIndicesPntr);
	MyMemCopy(allIndicesPntr, indicesPntr, indexSize * numIndices);
	if (numIndices < buffer->numIndices) { MyMemSet(&allIndicesPntr[indexSize * numIndices], 0x00, indexSize * (buffer->numIndices - numIndices)); }
	
	//TODO: Maybe we don't have to update the entire buffer? Maybe we can just update the beginning? Is that more performant since we don't have to pass potentially a lot of zeroes?
	sg_range indicesRange = (sg_range){indicesPntr, allIndicesSize};
	sg_update_buffer(buffer->indicesHandle, &indicesRange);
	
	ScratchEnd(scratch);
	
	if (buffer->indicesPntr != nullptr)
	{
		MyMemCopy(buffer->indicesPntr, indicesPntr, indexSize * numIndices);
	}
}
PEXPI void ChangeIndicesInVertBufferU8(VertBuffer* buffer, uxx numIndices, const u8* indicesPntr)
{
	ChangeIndicesInVertBufferEx(buffer, numIndices, sizeof(u8), indicesPntr);
}
PEXPI void ChangeIndicesInVertBufferU16(VertBuffer* buffer, uxx numIndices, const u16* indicesPntr)
{
	ChangeIndicesInVertBufferEx(buffer, numIndices, sizeof(u16), indicesPntr);
}
PEXPI void ChangeIndicesInVertBufferU32(VertBuffer* buffer, uxx numIndices, const u32* indicesPntr)
{
	ChangeIndicesInVertBufferEx(buffer, numIndices, sizeof(u32), indicesPntr);
}
PEXPI void ChangeIndicesInVertBufferU64(VertBuffer* buffer, uxx numIndices, const u64* indicesPntr)
{
	ChangeIndicesInVertBufferEx(buffer, numIndices, sizeof(u64), indicesPntr);
}

PEXPI void BindVertBuffer(sg_bindings* bindings, VertBuffer* buffer, uxx bufferIndex)
{
	NotNull(bindings);
	NotNull(buffer);
	Assert(buffer->handle.id != SG_INVALID_ID);
	bindings->vertex_buffers[bufferIndex] = buffer->handle;
	if (buffer->hasIndices)
	{
		Assert(bufferIndex == 0);
		bindings->index_buffer = buffer->indicesHandle;
		//TODO: Should we ever change bindings->index_buffer_offset?
	}
	else
	{
		bindings->index_buffer.id = SG_INVALID_ID;
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _GFX_VERT_BUFFER_H

#if defined(_GFX_SHADER_H) && defined(_GFX_VERT_BUFFER_H)
#include "cross/cross_shader_and_vert_buffer.h"
#endif
