/*
File:   struct_stream.h
Author: Taylor Robbins
Date:   09\06\2025
Description:
	** A "stream" is something that can written to and\or read from and could be backed
	** by a buffer in memory, a file on disk, or a network socket
*/

#ifndef _STRUCT_STREAM_H
#define _STRUCT_STREAM_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "struct/struct_string.h"
#include "os/os_file.h"

typedef enum DataStreamType DataStreamType;
enum DataStreamType
{
	DataStreamType_None = 0,
	DataStreamType_Buffer,
	DataStreamType_File,
	DataStreamType_Socket,
	DataStreamType_Count,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetDataStreamTypeStr(DataStreamType enumValue);
#else
PEXP const char* GetDataStreamTypeStr(DataStreamType enumValue)
{
	switch (enumValue)
	{
		case DataStreamType_None:   return "None";
		case DataStreamType_Buffer: return "Buffer";
		case DataStreamType_File:   return "File";
		case DataStreamType_Socket: return "Socket";
		case DataStreamType_Count:  return "Count";
		default: return "Unknown";
	}
}
#endif

typedef plex DataStream DataStream;
plex DataStream
{
	DataStreamType type;
	Result error;
	uxx cursor;
	uxx size; //UINTXX_MAX means unknown size
	union
	{
		Slice buffer;
		OsFile* filePntr;
		//TODO: Network Socket
	};
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE DataStream ToDataStreamFromBuffer(Slice buffer);
	PIG_CORE_INLINE DataStream ToDataStreamFromFile(OsFile* osFilePntr);
	PIG_CORE_INLINE bool IsDataStreamMemoryBacked(const DataStream* stream);
	PIG_CORE_INLINE bool IsDataStreamFinished(const DataStream* stream);
	u8* TryReadFromDataStream(DataStream* stream, uxx numBytes, Arena* dataArena);
	PIG_CORE_INLINE u8* TryReadFromDataStreamOrZeros(DataStream* stream, uxx numBytes, Arena* dataArena);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI DataStream ToDataStreamFromBuffer(Slice buffer)
{
	DataStream result = ZEROED;
	result.type = DataStreamType_Buffer;
	result.error = Result_None;
	result.cursor = 0;
	result.size = buffer.length;
	result.buffer = buffer;
	return result;
}

PEXPI DataStream ToDataStreamFromFile(OsFile* osFilePntr)
{
	Assert(osFilePntr != nullptr && osFilePntr->isOpen);
	DataStream result = ZEROED;
	result.type = DataStreamType_File;
	result.error = Result_None;
	result.cursor = osFilePntr->cursorIndex;
	result.size = osFilePntr->isKnownSize ? osFilePntr->fileSize : UINTXX_MAX;
	result.filePntr = osFilePntr;
	return result;
}

PEXPI bool IsDataStreamMemoryBacked(const DataStream* stream) { return (stream->type == DataStreamType_Buffer); }
PEXPI bool IsDataStreamFinished(const DataStream* stream) { return (stream->type == DataStreamType_None || (stream->size != UINTXX_MAX && stream->cursor >= stream->size)); }

PEXP u8* TryReadFromDataStream(DataStream* stream, uxx numBytes, Arena* dataArena)
{
	NotNull(stream);
	if (numBytes == 0) { return nullptr; }
	u8* result = nullptr;
	switch (stream->type)
	{
		case DataStreamType_Buffer:
		{
			if (stream->cursor + numBytes > stream->size) { stream->error = Result_EndOfBuffer; return nullptr; }
			NotNull(stream->buffer.bytes);
			if (dataArena != nullptr)
			{
				result = (u8*)AllocMem(dataArena, numBytes);
				if (result == nullptr) { stream->error = Result_FailedToAllocateMemory; return nullptr; }
				MyMemCopy(result, &stream->buffer.bytes[stream->cursor], numBytes);
			}
			else { result = &stream->buffer.bytes[stream->cursor]; }
			stream->cursor += numBytes;
		} break;
		
		case DataStreamType_File:
		{
			NotNull(stream->filePntr);
			NotNull(dataArena);
			stream->size = (stream->filePntr->isKnownSize ? stream->filePntr->fileSize : UINTXX_MAX);
			stream->cursor = stream->filePntr->cursorIndex;
			if (stream->size != UINTXX_MAX && stream->cursor + numBytes > stream->size) { stream->error = Result_EndOfFile; return nullptr; }
			
			result = (u8*)AllocMem(dataArena, numBytes);
			if (result == nullptr) { stream->error = Result_FailedToAllocateMemory; return nullptr; }
			
			uxx numBytesRead = 0;
			Result readResult = OsReadFromOpenFile(stream->filePntr, numBytes, false, result, &numBytesRead);
			stream->size = (stream->filePntr->isKnownSize ? stream->filePntr->fileSize : UINTXX_MAX);
			stream->cursor = stream->filePntr->cursorIndex;
			if (readResult != Result_Success)
			{
				if (CanArenaFree(dataArena)) { FreeMem(dataArena, result, numBytes); }
				stream->error = readResult;
				return nullptr;
			}
			else if (numBytesRead < numBytes)
			{
				if (CanArenaFree(dataArena)) { FreeMem(dataArena, result, numBytes); }
				stream->error = Result_EndOfFile;
				stream->size = stream->filePntr->cursorIndex;
				return nullptr;
			}
		} break;
		
		default: Assert(false); break;
	}
	return result;
}

PEXPI u8* TryReadFromDataStreamOrZeros(DataStream* stream, uxx numBytes, Arena* dataArena)
{
	NotNull(dataArena);
	u8* result = TryReadFromDataStream(stream, numBytes, dataArena);
	if (numBytes > 0 && result == nullptr)
	{
		result = (u8*)AllocMem(dataArena, numBytes);
		NotNull(result);
		MyMemSet(result, 0x00, numBytes);
	}
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_STREAM_H

#if defined(_STRUCT_STREAM_H) && defined(_PARSE_BINARY_H)
#include "cross/cross_stream_and_parse_binary.h"
#endif
