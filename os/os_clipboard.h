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
	Result OsGetClipboardString(OsWindowHandle windowHandle, Arena* arena, Str8* clipboardStrOut);
	Result OsSetClipboardString(OsWindowHandle windowHandle, Str8 clipboardStr);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP Result OsGetClipboardString(OsWindowHandle windowHandle, Arena* arena, Str8* clipboardStrOut)
{
	Result result = Result_None;
	#if TARGET_IS_WINDOWS
	{
		Assert(windowHandle != OsWidowHandleEmpty);
		if (OpenClipboard(windowHandle))
		{
			HANDLE dataHandle = GetClipboardData(CF_UNICODETEXT);
			if (dataHandle != nullptr)
			{
				Str16 wideStr = Str16Lit((char16_t*)GlobalLock(dataHandle));
				SetOptionalOutPntr(clipboardStrOut, ConvertUcs2StrToUtf8(arena, wideStr, false));
				GlobalUnlock(dataHandle);
				result = Result_Success;
			}
			else { result = Result_EmptyString; }
			CloseClipboard();
		}
		else { result = Result_FailedToOpenClipboard; }
	}
	#elif TARGET_IS_ANDROID
	{
		UNUSED(windowHandle);
		AssertMsg(AndroidNativeActivity != nullptr, "You must set AndroidNativeActivity global before calling OsGetClipboardString!");
		AssertMsg(AndroidJavaVM != nullptr, "You must set AndroidJavaVM global before calling OsGetClipboardString!");
		
		JavaVMAttachBlock(env)
		{
			jstring clipboardJStr = (*env)->NewStringUTF(env, "clipboard");
			jobject clipboardManager = jCall_getSystemService(env, AndroidNativeActivity, clipboardJStr);
			FreeJStr(env, clipboardJStr);
			
			if (jCall_hasPrimaryClip(env, clipboardManager))
			{
				jobject clipData = jCall_getPrimaryClip(env, clipboardManager);
				jobject firstClipDataItem = jCall_getItemAt(env, clipData, 0);
				jobject charSequence = jCall_getText(env, firstClipDataItem);
				jstring contentsJStr = jCall_toString(env, charSequence);
				if (clipboardStrOut != nullptr)
				{
					*clipboardStrOut = ToStr8FromJStr(env, arena, contentsJStr, false);
				}
				FreeJStr(env, contentsJStr);
				(*env)->DeleteLocalRef(env, charSequence);
				(*env)->DeleteLocalRef(env, firstClipDataItem);
				(*env)->DeleteLocalRef(env, clipData);
				result = Result_Success;
			}
			else { result = Result_EmptyString; }
			
			(*env)->DeleteLocalRef(env, clipboardManager);
		}
	}
	#else
	AssertMsg(false, "OsGetClipboardString does not support the current platform yet!");
	result = Result_NotImplemented;
	#endif
	return result;
}

PEXP Result OsSetClipboardString(OsWindowHandle windowHandle, Str8 clipboardStr)
{
	Result result = Result_None;
	#if TARGET_IS_WINDOWS
	do
	{
		Assert(windowHandle != OsWidowHandleEmpty);
		NotNullStr(clipboardStr);
		ScratchBegin(scratch);
		
		uxx dataSize = clipboardStr.length;
		void* dataPntr = clipboardStr.chars;
		bool isDataWide = false;
		if (DoesStrContainMultibyteUtf8Chars(clipboardStr))
		{
			if (scratch == nullptr) { result = Result_NoScratchAvailable; break; }
			Str16 wideStr = ConvertUtf8StrToUcs2(scratch, clipboardStr, true);
			dataPntr = wideStr.chars;
			dataSize = wideStr.length * sizeof(char16_t);
			isDataWide = true;
		}
		
		HGLOBAL globalCopy = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)dataSize + (isDataWide ? sizeof(char16_t) : sizeof(char))); 
		if (globalCopy == nullptr)
		{
			if (scratch != nullptr) { ScratchEnd(scratch); }
			result = Result_FailedToAllocateMemory;
			break;
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
			result = Result_FailedToOpenClipboard;
			break;
		}
		if (EmptyClipboard() == false)
		{
			CloseClipboard();
			result = Result_FailedToEmptyClipboard;
			break;
		}
		
		SetClipboardData((isDataWide ? CF_UNICODETEXT : CF_TEXT), globalCopy);
		
		CloseClipboard();
		result = Result_Success;
	} while(0);
	#elif TARGET_IS_ANDROID
	{
		UNUSED(windowHandle);
		AssertMsg(AndroidNativeActivity != nullptr, "You must set AndroidNativeActivity global before calling OsSetClipboardString!");
		AssertMsg(AndroidJavaVM != nullptr, "You must set AndroidJavaVM global before calling OsSetClipboardString!");
		ScratchBegin(scratch);
		JavaVMAttachBlock(env)
		{
			jstring clipboardJStr = (*env)->NewStringUTF(env, "clipboard");
			jobject clipboardManager = jCall_getSystemService(env, AndroidNativeActivity, clipboardJStr);
			FreeJStr(env, clipboardJStr);
			
			jstring textJStr = NewJStrNt(env, "text");
			jstring contentsJStr = NewJStr(env, clipboardStr);
			jobject clipData = jCall_ClipData_newPlainText(env, textJStr, contentsJStr);
			FreeJStr(env, contentsJStr);
			FreeJStr(env, textJStr);
			
			jCall_setPrimaryClip(env, clipboardManager, clipData);
			(*env)->DeleteLocalRef(env, clipData);
			
			(*env)->DeleteLocalRef(env, clipboardManager);
		}
		ScratchEnd(scratch);
		result = Result_Success;
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
	result = Result_NotImplemented;
	#endif
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _OS_CLIPBOARD_H
