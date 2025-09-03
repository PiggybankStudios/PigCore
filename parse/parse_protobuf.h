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

// The __unpack functions take a ProtobufCBuffer that only contains an append() function pointer. In order to use a pre-allocated buffer we wrap that in a PbBuffer structure that tracks length/allocLength and the pointer to the bytes
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
	PIG_CORE_INLINE void FreePbBuffer(Arena* arena, PbBuffer* buffer);
	PIG_CORE_INLINE PbBuffer NewPbBufferInArena(Arena* arena, uxx numBytes);
	#if DEBUG_BUILD
	PIG_CORE_INLINE Slice ProtobufPackInArena_(const ProtobufCMessageDescriptor* descriptorPntr, Arena* arena, const ProtobufCMessage* message);
	#else
	PIG_CORE_INLINE Slice ProtobufPackInArena_(Arena* arena, const ProtobufCMessage* message);
	#endif
	PIG_CORE_INLINE void* ProtobufUnpackInArena_(const ProtobufCMessageDescriptor* descriptorPntr, Arena* arena, Slice packedSlice);
#endif

#define NewPbBuffer_Const(bufferLength, bufferPntr) { .buffer = { .append = ProtobufBuffer_Append }, .length=0, .allocLength=(bufferLength), .pntr=(bufferPntr) }

#if DEBUG_BUILD
#define ProtobufPackInArena(lowercaseType, arenaPntr, structPntr) ProtobufPackInArena_(&lowercaseType##__descriptor, (arenaPntr), &(structPntr)->base)
#else
#define ProtobufPackInArena(lowercaseType, arenaPntr, structPntr) ProtobufPackInArena_((arenaPntr), &(structPntr)->base)
#endif

#define ProtobufUnpackInArena(type, lowercaseType, arenaPntr, packedSlice) (type*)ProtobufUnpackInArena_(&lowercaseType##__descriptor, (arenaPntr), (packedSlice))

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

PEXPI void FreePbBuffer(Arena* arena, PbBuffer* buffer)
{
	NotNull(arena);
	NotNull(buffer);
	#if LANGUAGE_IS_C
	uxx headerAlignment = (uxx)_Alignof(PbBuffer);
	#else
	uxx headerAlignment = (uxx)std::alignment_of<PbBuffer>();
	#endif
	FreeArray(u8, arena, buffer->allocLength, buffer->pntr);
	ClearPointer(buffer);
}

PEXPI PbBuffer NewPbBufferInArena(Arena* arena, uxx numBytes)
{
	u8* bytesPntr = AllocArray(u8, arena, numBytes);
	return (PbBuffer)NewPbBuffer_Const((bytesPntr != nullptr) ? numBytes : 0, bytesPntr);
}

#if DEBUG_BUILD
PEXPI Slice ProtobufPackInArena_(const ProtobufCMessageDescriptor* descriptorPntr, Arena* arena, const ProtobufCMessage* message)
#else
PEXPI Slice ProtobufPackInArena_(Arena* arena, const ProtobufCMessage* message)
#endif
{
	NotNull(arena);
	NotNull(message);
	#if DEBUG_BUILD
	AssertMsg(message->descriptor == descriptorPntr, "Wrong type passed to ProtobufPackInArena() macro!");
	#endif
	uxx bufferSize = (uxx)protobuf_c_message_get_packed_size(message);
	if (bufferSize == 0) { return Slice_Empty; }
	PbBuffer buffer = NewPbBufferInArena(arena, bufferSize);
	NotNull(buffer.pntr);
	size_t packResult = protobuf_c_message_pack_to_buffer(message, &buffer.buffer);
	DebugAssert((uxx)packResult == bufferSize);
	DebugAssert(buffer.length == buffer.allocLength);
	return NewStr8(bufferSize, buffer.pntr);
}

PEXPI void* ProtobufUnpackInArena_(const ProtobufCMessageDescriptor* descriptorPntr, Arena* arena, Slice packedSlice)
{
	NotNull(descriptorPntr);
	NotNull(arena);
	ProtobufCAllocator allocator = ProtobufAllocatorFromArena(arena);
	ProtobufCMessage* result = protobuf_c_message_unpack(descriptorPntr, &allocator, packedSlice.length, packedSlice.bytes);
	DebugAssert(result == nullptr || result->descriptor == descriptorPntr);
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_PROTOBUF

#endif //  _PARSE_PROTOBUF_H
