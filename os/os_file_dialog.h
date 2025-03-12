/*
File:   os_file_dialog.h
Author: Taylor Robbins
Date:   03\01\2025
Description:
	** Contains functions that help us spawn and interact with
	** file\path choosing dialogs that are provided by the OS
*/

#ifndef _OS_FILE_DIALOG_H
#define _OS_FILE_DIALOG_H

#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"
#include "std/std_includes.h"
#include "struct/struct_string.h"
#include "mem/mem_arena.h"
#include "misc/misc_result.h"
#include "os/os_path.h"
#include "os/os_file.h"

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	Result OsDoOpenFileDialog(Arena* arena, FilePath* pathOut);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

#if TARGET_IS_WINDOWS
//TODO: If there are any other files that use Co... functions, we probably need them to shared this one global. For now, OsDoOpenFileDialog is the only one
static bool Win32_HasCoInitialized = false;
#endif

PEXP Result OsDoOpenFileDialog(Arena* arena, FilePath* pathOut)
{
	Result result = Result_None;
	
	#if TARGET_IS_WINDOWS
	{
		//TODO: This implementation has only been lightly tested!
		
		if (!Win32_HasCoInitialized)
		{
			HRESULT initResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			Assert(SUCCEEDED(initResult));
			Win32_HasCoInitialized = true;
		}
		
		IFileOpenDialog* dialogPntr = nullptr;
		HRESULT createInstanceResult = CoCreateInstance((const IID *const)&CLSID_FileOpenDialog, NULL, CLSCTX_ALL, (const IID *const)&IID_IFileOpenDialog, (void**)(&dialogPntr));
		if (!SUCCEEDED(createInstanceResult)) { return Result_Failure; } //TODO: Make a better failure code!
		
		HRESULT showResult = dialogPntr->lpVtbl->Show(dialogPntr, NULL);
		if (!SUCCEEDED(showResult)) { dialogPntr->lpVtbl->Release(dialogPntr); return Result_Canceled; }
		
		IShellItem* shellItem = nullptr;
		HRESULT getResult = dialogPntr->lpVtbl->GetResult(dialogPntr, &shellItem);
		if (!SUCCEEDED(getResult)) { dialogPntr->lpVtbl->Release(dialogPntr); return Result_Failure; } //TOOD: Make a better failure code!
		
		PWSTR filePathPntr16 = nullptr;
		HRESULT getDisplayNameResult = shellItem->lpVtbl->GetDisplayName(shellItem, SIGDN_FILESYSPATH, &filePathPntr16);
		if (!SUCCEEDED(getDisplayNameResult)) { dialogPntr->lpVtbl->Release(dialogPntr); shellItem->lpVtbl->Release(shellItem); return Result_Failure; } //TODO: Make a better failure code!
		
		if (pathOut != nullptr)
		{
			Str16 filePathStr16 = Str16Lit(filePathPntr16);
			*pathOut = ConvertUcs2StrToUtf8(arena, filePathStr16, false);
			NotNullStr(*pathOut);
			FixPathSlashes(*pathOut);
		}
		
		CoTaskMemFree(filePathPntr16);
		shellItem->lpVtbl->Release(shellItem);
		dialogPntr->lpVtbl->Release(dialogPntr);
		result = Result_Success;
	}
	#else
	AssertMsg(false, "OsDoOpenFileDialog does not support the current platform yet!");
	result = Result_UnsupportedPlatform;
	#endif
	
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _OS_FILE_DIALOG_H
