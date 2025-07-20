/*
File:   struct_string_buffer.h
Author: Taylor Robbins
Date:   01\31\2025
Description:
	** A string buffer is a fixed size buffer that is allocated on the stack and has
	** a bunch of helper functions that let you easily append or modify a string
	** inside that buffer. This is useful for scenarios that want to easily build
	** up a string dynamically but know that the maximum size of the string is under
	** some maximum size that is small enough to be acceptable to be on the stack.
NOTE: The StringBuffer will always ensure a null-terminator character is at the end of the string, so you can really only use bufferSize-1 bytes
We use abbreviations and alias macros in this file more heavily because the primary
use cases for this type of data structure benefit greatly from being shorter and simpler
*/

#ifndef _STRUCT_STRING_BUFFER_H
#define _STRUCT_STRING_BUFFER_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "std/std_memset.h"
#include "struct/struct_string.h"

#define STRING_BUFFER_DEFAULT_LENGTH 256 //bytes

typedef enum StrBuffFlag StrBuffFlag;
enum StrBuffFlag
{
	StrBuffFlag_None = 0x00,
	StrBuffFlag_Overflowed       = 0x01,
	StrBuffFlag_AssertOnOverflow = 0x02,
	StrBuffFlag_PrintError       = 0x04,
	StrBuffFlag_All = 0x03,
};

typedef plex StringBuffer StringBuffer;
plex StringBuffer
{
	u8 flags;
	uxx maxLength;
	union
	{
		Str8 str;
		plex
		{
			uxx length;
			union { char* chars; u8* bytes; void* pntr; };
		};
	};
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void InitStrBuff(StringBuffer* bufferOut, uxx bufferSize, void* bufferPntr);
	PIG_CORE_INLINE void ClearStrBuff(StringBuffer* buffer);
	PIG_CORE_INLINE void SetStrBuffStr(StringBuffer* buffer, Str8 str);
	PIG_CORE_INLINE void AppendToStrBuffStr(StringBuffer* buffer, Str8 str);
	PIG_CORE_INLINE void AppendToStrBuffChar(StringBuffer* buffer, char character);
	PIG_CORE_INLINE Str8 StrBuffSlice(StringBuffer* buffer, uxx startIndex, uxx endIndex);
	PIG_CORE_INLINE Str8 StrBuffSliceFrom(StringBuffer* buffer, uxx startIndex);
	PIG_CORE_INLINE Str8 StrBuffSliceLength(StringBuffer* buffer, uxx startIndex, uxx length);
	uxx ReplaceInStrBuffStr(StringBuffer* buffer, Str8 target, Str8 replacement);
	PIG_CORE_INLINE uxx RemoveFromStrBuffStr(StringBuffer* buffer, Str8 target);
	PIG_CORE_INLINE void InsertIntoStrBuffStr(StringBuffer* buffer, uxx index, Str8 insertStr);
#endif

#define NewStrBuffEx(variableName, bufferSize) u8 variableName##_buffer[bufferSize]; StringBuffer variableName; do \
{                                                                                                                  \
	InitStrBuff(&variableName, bufferSize, &variableName##_buffer[0]);                                             \
} while(0)
#define NewStrBuff(variableName) NewStrBuffEx(variableName, STRING_BUFFER_DEFAULT_LENGTH)

#define SetStrBuff(bufferPntr, nullTermStr) SetStrBuffStr((bufferPntr), StrLit(nullTermStr))
#define AppendToStrBuff(bufferPntr, nullTermStr) AppendToStrBuffStr((bufferPntr), StrLit(nullTermStr))
#define ReplaceInStrBuff(bufferPntr, targetNullTermStr, replacementNullTermStr) ReplaceInStrBuffStr((bufferPntr), StrLit(targetNullTermStr), StrLit(replacementNullTermStr))
#define RemoveFromStrBuff(bufferPntr, targetNullTermStr) RemoveFromStrBuffStr((bufferPntr), StrLit(targetNullTermStr))
#define InsertIntoStrBuff(bufferPntr, index, insertNullTermStr) InsertIntoStrBuffStr((bufferPntr), (index), StrLit(insertNullTermStr))

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void InitStrBuff(StringBuffer* bufferOut, uxx bufferSize, void* bufferPntr)
{
	NotNull(bufferOut);
	Assert(bufferSize > 0);
	NotNull(bufferPntr);
	ClearPointer(bufferOut);
	bufferOut->length = 0;
	bufferOut->maxLength = bufferSize;
	bufferOut->pntr = bufferPntr;
	bufferOut->chars[0] = '\0';
	bufferOut->flags = DEBUG_BUILD ? StrBuffFlag_AssertOnOverflow : StrBuffFlag_None;
}

// +==============================+
// |        Get Functions         |
// +==============================+
PEXPI Str8 StrBuffSlice(StringBuffer* buffer, uxx startIndex, uxx endIndex)
{
	NotNull(buffer);
	NotNull(buffer->chars);
	Assert(startIndex <= buffer->length);
	Assert(endIndex <= buffer->length);
	Assert(endIndex >= startIndex);
	return NewStr8(endIndex - startIndex, &buffer->chars[startIndex]);
}
PEXPI Str8 StrBuffSliceFrom(StringBuffer* buffer, uxx startIndex)
{
	NotNull(buffer);
	NotNull(buffer->chars);
	Assert(startIndex <= buffer->length);
	return NewStr8(buffer->length - startIndex, &buffer->chars[startIndex]);
}
PEXPI Str8 StrBuffSliceLength(StringBuffer* buffer, uxx startIndex, uxx length)
{
	NotNull(buffer);
	NotNull(buffer->chars);
	Assert(startIndex <= buffer->length);
	Assert(startIndex + length <= buffer->length);
	return NewStr8(length, &buffer->chars[startIndex]);
}

// +==============================+
// |    Manipulation Functions    |
// +==============================+
PEXPI void ClearStrBuff(StringBuffer* buffer)
{
	NotNull(buffer);
	Assert(buffer->maxLength > 0);
	buffer->length = 0;
	buffer->chars[0] = '\0';
}

PEXPI void SetStrBuffStr(StringBuffer* buffer, Str8 str)
{
	NotNull(buffer);
	NotNullStr(str);
	if (str.length < buffer->maxLength)
	{
		MyMemCopy(&buffer->chars[0], str.chars, str.length);
		buffer->length = str.length;
		buffer->chars[buffer->length] = '\0';
	}
	else
	{
		FlagSet(buffer->flags, StrBuffFlag_Overflowed);
		if (IsFlagSet(buffer->flags, StrBuffFlag_AssertOnOverflow)) { AssertMsg(str.length < buffer->maxLength, "StringBuffer overflow in SetStringBuffer!"); }
		MyMemCopy(&buffer->chars[0], str.chars, buffer->maxLength-1);
		buffer->length = buffer->maxLength-1;
		buffer->chars[buffer->length] = '\0';
	}
}

PEXPI void AppendToStrBuffStr(StringBuffer* buffer, Str8 str)
{
	NotNull(buffer);
	NotNullStr(str);
	if (buffer->length + str.length < buffer->maxLength)
	{
		MyMemCopy(&buffer->chars[buffer->length], str.chars, str.length);
		buffer->length += str.length;
		buffer->chars[buffer->length] = '\0';
	}
	else
	{
		FlagSet(buffer->flags, StrBuffFlag_Overflowed);
		if (IsFlagSet(buffer->flags, StrBuffFlag_AssertOnOverflow)) { AssertMsg(buffer->length + str.length < buffer->maxLength, "StringBuffer overflow in AppendStringBuffer!"); }
		if (buffer->length < buffer->maxLength-1)
		{
			MyMemCopy(&buffer->chars[buffer->length], str.chars, (buffer->maxLength-1) - buffer->length);
			buffer->length = buffer->maxLength-1;
			buffer->chars[buffer->length] = '\0';
		}
	}
}
PEXPI void AppendToStrBuffChar(StringBuffer* buffer, char character)
{
	AppendToStrBuffStr(buffer, NewStr8(1, &character));
}

//TODO: Add an AnyCase version?
//NOTE: Returns the number of replacements performed
PEXP uxx ReplaceInStrBuffStr(StringBuffer* buffer, Str8 target, Str8 replacement)
{
	NotNull(buffer);
	NotNull(buffer->chars);
	NotNullStr(target);
	NotNullStr(replacement);
	if (target.length == 0) { return 0; } //no work to do if there is no target
	uxx numReplacements = 0;
	
	for (uxx cIndex = 0; cIndex + target.length <= buffer->length; cIndex++)
	{
		if (MyMemEquals(&buffer->chars[cIndex], target.chars, target.length))
		{
			if (replacement.length > target.length) //if replacing with larger string
			{
				//Move all characters after this point up
				uxx offset = (replacement.length - target.length);
				if (buffer->length + offset < buffer->maxLength)
				{
					MyMemMove(&buffer->chars[cIndex + replacement.length], &buffer->chars[cIndex + target.length], buffer->length - (cIndex + target.length));
					buffer->length += offset;
				}
				else
				{
					FlagSet(buffer->flags, StrBuffFlag_Overflowed);
					if (IsFlagSet(buffer->flags, StrBuffFlag_AssertOnOverflow)) { AssertMsg(buffer->length + offset < buffer->maxLength, "StringBuffer overflow in ReplaceInStrBuffStr!"); }
					uxx numCharsThatWillFit = (buffer->maxLength-1) - (cIndex + replacement.length);
					MyMemMove(&buffer->chars[cIndex + replacement.length], &buffer->chars[cIndex + target.length], numCharsThatWillFit);
					buffer->length = buffer->maxLength-1;
				}
			}
			else if (replacement.length < target.length) //if replacing with a smaller string
			{
				//Move all characters after this point down
				uxx offset = (target.length - replacement.length);
				MyMemMove(&buffer->chars[cIndex + replacement.length], &buffer->chars[cIndex + target.length], buffer->length - (cIndex + target.length));
				buffer->length -= offset;
			}
			
			if (replacement.length > 0)
			{
				MyMemCopy(&buffer->chars[cIndex], replacement.chars, replacement.length);
			}
			if (replacement.length > 1) { cIndex += replacement.length-1; }
			
			numReplacements++;
		}
	}
	
	DebugAssert(buffer->length < buffer->maxLength);
	buffer->chars[buffer->length] = '\0';
	
	return numReplacements;
}
PEXPI uxx RemoveFromStrBuffStr(StringBuffer* buffer, Str8 target)
{
	return ReplaceInStrBuffStr(buffer, target, Str8_Empty);
}

PEXPI void InsertIntoStrBuffStr(StringBuffer* buffer, uxx index, Str8 insertStr)
{
	NotNull(buffer);
	NotNull(buffer->chars);
	NotNullStr(insertStr);
	Assert(index <= buffer->length);
	if (insertStr.length == 0) { return; } //if insertStr is empty, no work to do
	
	if (buffer->length + insertStr.length < buffer->maxLength)
	{
		if (index < buffer->length)
		{
			MyMemMove(&buffer->chars[index + insertStr.length], &buffer->chars[index], buffer->length - index);
		}
		MyMemCopy(&buffer->chars[index], insertStr.chars, insertStr.length);
		buffer->length += insertStr.length;
	}
	else
	{
		FlagSet(buffer->flags, StrBuffFlag_Overflowed);
		if (IsFlagSet(buffer->flags, StrBuffFlag_AssertOnOverflow)) { AssertMsg(buffer->length + insertStr.length < buffer->maxLength, "StringBuffer overflow in InsertIntoStrBuffStr!"); }
		uxx numCharsThatWillFit = (buffer->maxLength-1) - index;
		if (numCharsThatWillFit > 0)
		{
			uxx numInsertCharsThatWillFit = MinUXX(numCharsThatWillFit, insertStr.length);
			uxx numCharsAfterInsert = (buffer->maxLength-1) - (index + numInsertCharsThatWillFit);
			if (numCharsAfterInsert > 0)
			{
				MyMemMove(&buffer->chars[index + numInsertCharsThatWillFit], &buffer->chars[index], numCharsAfterInsert);
			}
			MyMemCopy(&buffer->chars[index], insertStr.chars, numInsertCharsThatWillFit);
		}
		buffer->length = buffer->maxLength-1;
	}
	
	buffer->chars[buffer->length] = '\0';
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_STRING_BUFFER_H

#if defined(_MEM_ARENA_H) && defined(_STRUCT_STRING_BUFFER_H)
#include "cross/cross_mem_arena_and_string_buffer.h"
#endif

#if defined(_MEM_SCRATCH_H) && defined(_STRUCT_STRING_BUFFER_H)
#include "cross/cross_mem_scratch_and_string_buffer.h"
#endif

#if defined(_STD_PRINTF_H) && defined(_STRUCT_STRING_BUFFER_H)
#include "cross/cross_printf_and_string_buffer.h"
#endif
