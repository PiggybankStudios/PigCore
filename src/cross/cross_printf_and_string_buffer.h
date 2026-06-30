/*
File:   cross_printf_and_string_buffer.h
Author: Taylor Robbins
Date:   01\31\2025
*/

#ifndef _CROSS_PRINTF_AND_STRING_BUFFER_H
#define _CROSS_PRINTF_AND_STRING_BUFFER_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void PrintInStrBuffVa(StringBuffer* buffer, const char* formatString, va_list args);
	PIG_CORE_INLINE void AppendPrintToStrBuffVa(StringBuffer* buffer, const char* formatString, va_list args);
	PIG_CORE_INLINE void PrintInStrBuff(StringBuffer* buffer, const char* formatString, ...);
	PIG_CORE_INLINE void AppendPrintToStrBuff(StringBuffer* buffer, const char* formatString, ...);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void AppendPrintToStrBuffVa(StringBuffer* buffer, const char* formatString, va_list args)
{
	NotNull(buffer);
	NotNull(formatString);
	if (formatString[0] == '\0') { return; } //no work to do if there's no formatString
	if (buffer->length >= buffer->maxLength-1)
	{
		FlagSet(buffer->flags, StrBuffFlag_Overflowed);
		if (IsFlagSet(buffer->flags, StrBuffFlag_AssertOnOverflow)) { AssertMsg(buffer->length < buffer->maxLength-1, "StringBuffer overflow in PrintInStrBuff!"); }
		return;
	}
	
	uxx availableSpace = (buffer->maxLength-1) - buffer->length;
	
	int printfResult = MyVaListPrintf(&buffer->chars[buffer->length], (size_t)availableSpace, formatString, args);
	
	if (printfResult >= 0)
	{
		if ((uxx)printfResult <= availableSpace)
		{
			buffer->length += (uxx)printfResult;
		}
		else
		{
			FlagSet(buffer->flags, StrBuffFlag_Overflowed);
			if (IsFlagSet(buffer->flags, StrBuffFlag_AssertOnOverflow)) { AssertMsg((uxx)printfResult <= availableSpace, "StringBuffer overflow in PrintInStrBuff!"); }
			buffer->length = buffer->maxLength-1;
		}
	}
	else
	{
		FlagSet(buffer->flags, StrBuffFlag_PrintError);
		//TODO: Should we append the formatString here? Or maybe append some error message like FORMAT_ERROR?
	}
	
	buffer->chars[buffer->length] = '\0';
}

PEXPI void PrintInStrBuffVa(StringBuffer* buffer, const char* formatString, va_list args)
{
	NotNull(buffer);
	ClearStrBuff(buffer);
	AppendPrintToStrBuffVa(buffer, formatString, args);
}

PEXPI void PrintInStrBuff(StringBuffer* buffer, const char* formatString, ...)
{
	NotNull(buffer);
	NotNull(formatString);
	ClearStrBuff(buffer);
	va_list args;
	va_start(args, formatString);
	AppendPrintToStrBuffVa(buffer, formatString, args);
	va_end(args);
}

PEXPI void AppendPrintToStrBuff(StringBuffer* buffer, const char* formatString, ...)
{
	NotNull(buffer);
	NotNull(formatString);
	va_list args;
	va_start(args, formatString);
	AppendPrintToStrBuffVa(buffer, formatString, args);
	va_end(args);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_PRINTF_AND_STRING_BUFFER_H
