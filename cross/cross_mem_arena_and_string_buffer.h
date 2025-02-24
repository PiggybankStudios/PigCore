/*
File:   cross_mem_arena_and_string_buffer.h
Author: Taylor Robbins
Date:   01\31\2025
*/

#ifndef _CROSS_MEM_ARENA_AND_STRING_BUFFER_H
#define _CROSS_MEM_ARENA_AND_STRING_BUFFER_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeStrBuffFromArena(StringBuffer* buffer, Arena* arena);
	PIG_CORE_INLINE StringBuffer NewStrBuffFromArena(Arena* arena, uxx bufferSize);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void FreeStrBuffFromArena(StringBuffer* buffer, Arena* arena)
{
	NotNull(arena);
	NotNull(buffer);
	if (buffer->chars != nullptr)
	{
		FreeArray(char, arena, buffer->maxLength, buffer->chars);
	}
	ClearPointer(buffer);
}

PEXPI StringBuffer NewStrBuffFromArena(Arena* arena, uxx bufferSize)
{
	NotNull(arena);
	Assert(bufferSize > 0);
	StringBuffer result = ZEROED;
	char* allocatedChars = AllocArray(char, arena, bufferSize);
	NotNull(allocatedChars);
	InitStrBuff(&result, bufferSize, allocatedChars);
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_MEM_ARENA_AND_STRING_BUFFER_H
