/*
File:   os_font.h
Author: Taylor Robbins
Date:   02\05\2025
Description:
	** Contains functions that help us ask the operating system for font files like .ttf
*/

#ifndef _OS_FONT_H
#define _OS_FONT_H

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "struct/struct_string.h"

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	Result OsReadPlatformFont(Arena* arena, Str8 fontName, i32 fontSize, bool bold, bool italic, Slice* fileContentsOut);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP Result OsReadPlatformFont(Arena* arena, Str8 fontName, i32 fontSize, bool bold, bool italic, Slice* fileContentsOut)
{
	NotNull(arena);
	NotNull(fileContentsOut);
	NotNullStr(fontName);
	Result result = Result_None;
	#if TARGET_IS_WINDOWS
	{
		ScratchBegin1(scratch, arena);
		NotNullStr(fontName);
		Str8 fontNameNt = AllocStrAndCopy(scratch, fontName.length, fontName.chars, true);
		
		//TODO: Call EnumFontFamiliesEx first to determine which font name exactly we will be getting
		
		//NOTE: This comes from wingdi.h (included by Windows.h) and resides in Gdi32.dll
		HFONT fontHandle = CreateFontA(
			fontSize,                   //height
			0,                          //width
			0,                          //escapement
			0,                          //orientation
			bold ? FW_BOLD : FW_NORMAL, //weight
			italic,                     //italic
			false,                      //underline TODO: Should we ask for this?
			false,                      //strikeout TODO: Should we ask for this?
			ANSI_CHARSET,               //charset
			OUT_DEFAULT_PRECIS,         //output precision
			CLIP_DEFAULT_PRECIS,        //clipping precision
			DEFAULT_QUALITY,            //quality
			DEFAULT_PITCH|FF_DONTCARE,  //pitch and family
			fontNameNt.chars            //Face Name
		);
		if (fontHandle == NULL)
		{
			// PrintLine_E("Failed to find font with name \"%s\"", fontNameNt.chars);
			ScratchEnd(scratch);
			return Result_NotFound;
		}
		
		HWND windowHandle = GetForegroundWindow();
		if (windowHandle == NULL)
		{
			BOOL deleteResult = DeleteObject(fontHandle);
			Assert(deleteResult != 0);
			ScratchEnd(scratch);
			return Result_NoForegroundWindow;
		}
		
		HDC winDeviceContext = GetDC(windowHandle);
		if (winDeviceContext == NULL)
		{
			BOOL deleteResult = DeleteObject(fontHandle);
			Assert(deleteResult != 0);
			ScratchEnd(scratch);
			return Result_NoDeviceContext;
		}
		
		HGDIOBJ selectResult = SelectObject(winDeviceContext, fontHandle);
		if (selectResult == NULL)
		{
			// PrintLine_E("Failed to select platform font after creation! \"%s\"", fontNameNt.pntr);
			BOOL deleteResult = DeleteObject(fontHandle);
			Assert(deleteResult != 0);
			ScratchEnd(scratch);
			return Result_Failure; //TODO: Should we give this failure a better name?
		}
		
		DWORD getFontDataResult = GetFontData(
			winDeviceContext, //hardware device context
			0,                //table
			0,                //offset
			nullptr,          //buffer
			0                 //buffer size
		);
		if (getFontDataResult == GDI_ERROR)
		{
			// PrintLine_E("Failed to get/measure platform font contents for font \"%s\"", fontNameNt.pntr);
			BOOL deleteResult = DeleteObject(fontHandle);
			Assert(deleteResult != 0);
			ScratchEnd(scratch);
			return Result_FailedToReadFile;
		}
		if (getFontDataResult <= 0)
		{
			// PrintLine_E("There was no file contents for font \"%s\"", fontNameNt.pntr);
			BOOL deleteResult = DeleteObject(fontHandle);
			Assert(deleteResult != 0);
			ScratchEnd(scratch);
			return Result_EmptyFile;
		}
		
		fileContentsOut->length = (uxx)getFontDataResult;
		fileContentsOut->bytes = AllocArray(u8, arena, fileContentsOut->length);
		if (fileContentsOut->bytes == nullptr)
		{
			// PrintLine_E("Failed to allocate space to store font file data for font \"%s\": %llu bytes", fontNameNt.pntr, fontDataSize);
			BOOL deleteResult = DeleteObject(fontHandle);
			Assert(deleteResult != 0);
			ScratchEnd(scratch);
			return Result_FailedToAllocateMemory;
		}
		
		getFontDataResult = GetFontData(
			winDeviceContext,              //hardware device context
			0,                             //table
			0,                             //offset
			fileContentsOut->bytes,        //buffer
			(DWORD)fileContentsOut->length //buffer size
		);
		Assert(getFontDataResult == (DWORD)fileContentsOut->length);
		
		BOOL deleteResult = DeleteObject(fontHandle);
		Assert(deleteResult != 0);
		
		result = Result_Success;
		ScratchEnd(scratch);
	}
	#else
	AssertMsg(false, "OsReadPlatformFont does not support the current platform yet!");
	#endif
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _OS_FONT_H
