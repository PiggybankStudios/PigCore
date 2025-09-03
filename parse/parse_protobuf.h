/*
File:   parse_protobuf.h
Author: Taylor Robbins
Date:   09\03\2025
Description:
	** Contains code that helps us serialize to and deserialize from Google's Protocol Buffers (aka protobuf) format
	** NOTE: We use protobuf-c as a C version of the protobuf API instead of the officially supported C++ version
*/

#ifndef _PARSE_PROTOBUF_H
#define _PARSE_PROTOBUF_H

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"

#if BUILD_WITH_PROTOBUF

#if COMPILER_IS_MSVC
#pragma warning(push)
#pragma warning(disable: 4146) //unary minus operator applied to unsigned type, result still unsigned
#pragma warning(disable: 4267) //'function': conversion from 'size_t' to 'uint32_t', possible loss of data
#pragma warning(disable: 4244) //'=': conversion from 'uint32_t' to 'uint8_t', possible loss of data
#endif
#include "third_party/protobuf_c/protobuf-c/protobuf-c.h"
#if PIG_CORE_IMPLEMENTATION
#include "third_party/protobuf_c/protobuf-c/protobuf-c.c"
#endif
#if COMPILER_IS_MSVC
#pragma warning(pop)
#endif

//NOTE: #include parse_proto_google_types.pb-c.h/c in your application in order to use the types within

typedef plex PbBuffer PbBuffer;
plex PbBuffer
{
	ProtobufCBuffer buffer;
	uxx length;
	uxx allocLength;
	u8* pntr; 
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE ProtobufCAllocator ProtobufAllocatorFromArena(Arena* arena);
	PIG_CORE_INLINE PbBuffer NewPbBuffer(uxx bufferLength, u8* bufferPntr);
	PIG_CORE_INLINE void FreeProtobufBuffer(Arena* arena, PbBuffer* buffer);
	PIG_CORE_INLINE PbBuffer* AllocPbBuffer(Arena* arena, uxx numBytes);
#endif

#define NewPbBuffer_Const(bufferLength, bufferPntr) { .buffer = { .append = ProtobufBuffer_Append }, .length=0, .allocLength=(bufferLength), .pntr=(bufferPntr) }

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

static void* ProtobufAllocator_Alloc(void* contextPntr, size_t numBytes)
{
	return AllocMem((Arena*)contextPntr, (uxx)numBytes);
}
static void ProtobufAllocator_Free(void* contextPntr, void* pointer)
{
	if (CanArenaFree((Arena*)contextPntr)) { FreeMem((Arena*)contextPntr, pointer, 0); }
}
static void ProtobufBuffer_Append(ProtobufCBuffer* bufferPntr, size_t dataLength, const u8* dataPntr)
{
	Assert(dataLength <= UINTXX_MAX);
	PbBuffer* buffer = (PbBuffer*)bufferPntr;
	if ((uxx)dataLength > 0)
	{
		Assert(buffer->length + (uxx)dataLength <= buffer->allocLength);
		MyMemCopy(buffer->pntr + buffer->length, dataPntr, (uxx)dataLength);
		buffer->length += (uxx)dataLength;
	}
}

//NOTE: Arena must have ArenaFlag_AllowFreeWithoutSize!
PEXPI ProtobufCAllocator ProtobufAllocatorFromArena(Arena* arena)
{
	ProtobufCAllocator result = ZEROED;
	result.allocator_data = (void*)arena;
	result.alloc = ProtobufAllocator_Alloc;
	result.free = ProtobufAllocator_Free;
	return result;
}

PEXPI PbBuffer NewPbBuffer(uxx bufferLength, u8* bufferPntr)
{
	PbBuffer result = ZEROED;
	result.buffer.append = ProtobufBuffer_Append;
	result.allocLength = bufferLength;
	result.pntr = bufferPntr;
	return result;
}

PEXPI void FreeProtobufBuffer(Arena* arena, PbBuffer* buffer)
{
	NotNull(arena);
	NotNull(buffer);
	#if LANGUAGE_IS_C
	uxx headerAlignment = (uxx)_Alignof(PbBuffer);
	#else
	uxx headerAlignment = (uxx)std::alignment_of<PbBuffer>();
	#endif
	FreeMemAligned(arena, buffer, sizeof(PbBuffer) + buffer->allocLength, headerAlignment);
}

PEXPI PbBuffer* AllocPbBuffer(Arena* arena, uxx numBytes)
{
	#if LANGUAGE_IS_C
	uxx headerAlignment = (uxx)_Alignof(PbBuffer);
	#else
	uxx headerAlignment = (uxx)std::alignment_of<PbBuffer>();
	#endif
	PbBuffer* result = (PbBuffer*)AllocMemAligned(arena, sizeof(PbBuffer) + numBytes, headerAlignment);
	if (result == nullptr) { return nullptr; }
	MyMemSet(result, 0x00, sizeof(PbBuffer) + numBytes);
	result->buffer.append = ProtobufBuffer_Append;
	result->length = 0;
	result->allocLength = numBytes;
	result->pntr = ((u8*)result) + sizeof(PbBuffer);
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_PROTOBUF

#endif //  _PARSE_PROTOBUF_H
