/*
File:   cross_mem_scratch_and_string_buffer.h
Author: Taylor Robbins
Date:   01\31\2025
*/

#ifndef _CROSS_MEM_SCRATCH_AND_STRING_BUFFER_H
#define _CROSS_MEM_SCRATCH_AND_STRING_BUFFER_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE StringBuffer NewScratchStrBuff2(uxx bufferSize, const Arena* conflict1, const Arena* conflict2);
#endif //!PIG_CORE_IMPLEMENTATION

#define NewScratchStrBuff(bufferSize) NewScratchStrBuff2((bufferSize), nullptr, nullptr)
#define NewScratchStrBuff1(bufferSize, conflict1) NewScratchStrBuff2((bufferSize), (conflict1), nullptr)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI StringBuffer NewScratchStrBuff2(uxx bufferSize, const Arena* conflict1, const Arena* conflict2)
{
	Assert(bufferSize > 0);
	ScratchBegin2(scratch, conflict1, conflict2);
	//NOTE: Intentionally no ScratchEnd here! This assumes your program resets all scratch arenas as part of a frame loop or similar, otherwise you will leak memory here
	return NewStrBuffFromArena(scratch, bufferSize);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_MEM_SCRATCH_AND_STRING_BUFFER_H
