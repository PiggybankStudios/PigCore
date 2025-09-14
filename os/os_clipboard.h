/*
File:   os_clipboard.h
Author: Taylor Robbins
Date:   09\14\2025
Description:
	** Holds functions that allow us to interact with the Operating Systems clipboard (Copy, Paste, Cut)
*/

#ifndef _OS_CLIPBOARD_H
#define _OS_CLIPBOARD_H

#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "std/std_includes.h"
#include "std/std_memset.h"
#include "base/base_unicode.h"
#include "struct/struct_string.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "misc/misc_result.h"
#include "os/os_process_info.h"

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	Str8 OsGetClipboardString(Arena* arena);
	Result OsSetClipboardString(OsWindowHandle windowHandle, Str8 clipboardStr);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP Str8 OsGetClipboardString(Arena* arena)
{
	#if TARGET_IS_WINDOWS
	{
		//TODO: Implement me!
		return Str8_Empty;
	}
	#else
	AssertMsg(false, "OsGetClipboardString does not support the current platform yet!");
	return Str8_Empty;
	#endif
}

PEXP Result OsSetClipboardString(OsWindowHandle windowHandle, Str8 clipboardStr)
{
	#if TARGET_IS_WINDOWS
	{
		Assert(windowHandle != NULL);
		NotNullStr(clipboardStr);
		ScratchBegin(scratch);
		
		uxx dataSize = clipboardStr.length;
		void* dataPntr = clipboardStr.chars;
		bool isDataWide = false;
		if (DoesStrContainMultibyteUtf8Chars(clipboardStr))
		{
			if (scratch == nullptr) { return Result_NoScratchAvailable; }
			Str16 wideStr = ConvertUtf8StrToUcs2(scratch, clipboardStr, true);
			dataPntr = wideStr.chars;
			dataSize = wideStr.length * sizeof(char16_t);
			isDataWide = true;
		}
		
		HGLOBAL globalCopy = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)dataSize + (isDataWide ? sizeof(char16_t) : sizeof(char))); 
		if (globalCopy == nullptr)
		{
			if (scratch != nullptr) { ScratchEnd(scratch); }
			return Result_FailedToAllocateMemory;
		}
		
		u8* lockPntr = (u8*)GlobalLock(globalCopy);
		MyMemCopy(lockPntr, dataPntr, dataSize);
		MyMemSet(&lockPntr[dataSize], 0x00, (isDataWide ? sizeof(char16_t) : sizeof(char)));
		GlobalUnlock(globalCopy);
		if (scratch != nullptr) { ScratchEnd(scratch); }
		
		if (OpenClipboard(windowHandle) == false)
		{
			HGLOBAL freeResult = GlobalFree(globalCopy);
			Assert(freeResult != NULL);
			return Result_FailedToOpenClipboard;
		}
		if (EmptyClipboard() == false)
		{
			CloseClipboard();
			return Result_FailedToEmptyClipboard;
		}
		
		SetClipboardData((isDataWide ? CF_UNICODETEXT : CF_TEXT), globalCopy);
		
		CloseClipboard();
		return Result_Success;
	}
	// #elif TARGET_IS_LINUX
	//TODO: Implement me!
	// #elif TARGET_IS_OSX
	//TODO: Implement me!
	// #elif TARGET_IS_WEB
	//TODO: Implement me!
	// #elif TARGET_IS_ORCA
	//TODO: Implement me!
	#else
	AssertMsg(false, "OsSetClipboardString does not support the current platform yet!");
	return Result_NotImplemented;
	#endif
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _OS_CLIPBOARD_H
