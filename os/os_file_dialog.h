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
	Result OsDoSaveFileDialog(uxx numExtensions, Str8Pair* extensions, uxx defaultExtensionIndex, Arena* arena, FilePath* pathOut);
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
		if (!SUCCEEDED(getResult)) { dialogPntr->lpVtbl->Release(dialogPntr); return Result_Failure; } //TODO: Make a better failure code!
		
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
	UNUSED(arena);
	UNUSED(pathOut);
	AssertMsg(false, "OsDoOpenFileDialog does not support the current platform yet!");
	result = Result_UnsupportedPlatform;
	#endif
	
	return result;
}

PEXP Result OsDoSaveFileDialog(uxx numExtensions, Str8Pair* extensions, uxx defaultExtensionIndex, Arena* arena, FilePath* pathOut)
{
	Assert(numExtensions == 0 || defaultExtensionIndex < numExtensions);
	Result result = Result_None;
	
	#if TARGET_IS_WINDOWS
	{
		//TODO: This implementation has only been lightly tested!
		ScratchBegin1(scratch, arena);
		
		if (!Win32_HasCoInitialized)
		{
			HRESULT initResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			Assert(SUCCEEDED(initResult));
			Win32_HasCoInitialized = true;
		}
		
		IFileSaveDialog* dialogPntr = nullptr;
		HRESULT createInstanceResult = CoCreateInstance((const IID *const)&CLSID_FileSaveDialog, NULL, CLSCTX_ALL, (const IID *const)&IID_IFileSaveDialog, (void**)(&dialogPntr));
		if (!SUCCEEDED(createInstanceResult)) { ScratchEnd(scratch); return Result_Failure; } //TODO: Make a better failure code!
		
		//TODO: Call SetDefaultFolder(dialogPntr, L"SOMETHING") https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ifiledialog-setdefaultfolder
		//      Or call SetFolder(dialogPntr, IShellItem* folderItem) (use SHCreateItemFromParsingName(L"Path", NULL, &IID_IShellItem, (void**)&folderItem))
		//TODO: Call SetFileName(dialogPntr, L"SOMETHING")
		
		if (numExtensions > 0)
		{
			COMDLG_FILTERSPEC* filterSpecs = AllocArray(COMDLG_FILTERSPEC, scratch, numExtensions);
			NotNull(filterSpecs);
			for (uxx eIndex = 0; eIndex < numExtensions; eIndex++)
			{
				Assert(!IsEmptyStr(extensions[eIndex].key));
				Assert(!IsEmptyStr(extensions[eIndex].value));
				filterSpecs[eIndex].pszName = ConvertUtf8StrToUcs2(scratch, extensions[eIndex].key, true).chars;
				filterSpecs[eIndex].pszSpec = ConvertUtf8StrToUcs2(scratch, extensions[eIndex].value, true).chars;
				if (eIndex == defaultExtensionIndex)
				{
					Str8 extensionPart = GetFileExtPart(extensions[eIndex].value, false, false);
					if (!StrExactEquals(extensionPart, StrLit("*")))
					{
						Str16 extensionPart16 = ConvertUtf8StrToUcs2(scratch, extensionPart, true);
						NotNull(extensionPart16.chars);
						HRESULT setDefaultExtensionResult = dialogPntr->lpVtbl->SetDefaultExtension(dialogPntr, extensionPart16.chars);
						Assert(SUCCEEDED(setDefaultExtensionResult));
					}
				}
			}
			HRESULT setFileTypesResult = dialogPntr->lpVtbl->SetFileTypes(dialogPntr, (UINT)numExtensions, filterSpecs);
			Assert(SUCCEEDED(setFileTypesResult));
			HRESULT setFileTypeIndexResult = dialogPntr->lpVtbl->SetFileTypeIndex(dialogPntr, (UINT)(defaultExtensionIndex+1)); //NOTE: for some reason this index is 1-based not 0-based
			Assert(SUCCEEDED(setFileTypeIndexResult));
		}
		else
		{
			COMDLG_FILTERSPEC filterSpecs[1] = { L"All Files", L"*.*" };
			HRESULT setFileTypesResult = dialogPntr->lpVtbl->SetFileTypes(dialogPntr, ArrayCount(filterSpecs), &filterSpecs[0]);
			Assert(SUCCEEDED(setFileTypesResult));
			HRESULT setFileTypeIndexResult = dialogPntr->lpVtbl->SetFileTypeIndex(dialogPntr, 1);
			Assert(SUCCEEDED(setFileTypeIndexResult));
		}
		
		#if 0
		if (StrExactStartsWith(defaultExtension, StrLit("."))) { defaultExtension = StrSliceFrom(defaultExtension, 1); } //remove leading period if given, it's not accepted by Windows API
		if (!IsEmptyStr(defaultExtension))
		{
			Str16 extension16 = ConvertUtf8StrToUcs2(scratch, defaultExtension, true);
			NotNull(extension16.chars);
			HRESULT setExtensionResult = dialogPntr->lpVtbl->SetDefaultExtension(dialogPntr, extension16.chars);
			if (!SUCCEEDED(setExtensionResult)) { dialogPntr->lpVtbl->Release(dialogPntr); ScratchEnd(scratch); return Result_Failure; } //TODO: Make a better failure code!
		}
		#endif
		
		HRESULT showResult = dialogPntr->lpVtbl->Show(dialogPntr, NULL);
		if (!SUCCEEDED(showResult)) { dialogPntr->lpVtbl->Release(dialogPntr); ScratchEnd(scratch); return Result_Canceled; }
		
		IShellItem* shellItem = nullptr;
		HRESULT getResult = dialogPntr->lpVtbl->GetResult(dialogPntr, &shellItem);
		if (!SUCCEEDED(getResult)) { dialogPntr->lpVtbl->Release(dialogPntr); ScratchEnd(scratch); return Result_Failure; } //TODO: Make a better failure code!
		
		PWSTR filePathPntr16 = nullptr;
		HRESULT getDisplayNameResult = shellItem->lpVtbl->GetDisplayName(shellItem, SIGDN_FILESYSPATH, &filePathPntr16);
		if (!SUCCEEDED(getDisplayNameResult)) { dialogPntr->lpVtbl->Release(dialogPntr); shellItem->lpVtbl->Release(shellItem); ScratchEnd(scratch); return Result_Failure; } //TODO: Make a better failure code!
		
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
		ScratchEnd(scratch);
	}
	#else
	UNUSED(numExtensions);
	UNUSED(extensions);
	UNUSED(defaultExtensionIndex);
	UNUSED(arena);
	UNUSED(pathOut);
	AssertMsg(false, "OsDoSaveFileDialog does not support the current platform yet!");
	result = Result_UnsupportedPlatform;
	#endif
	
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _OS_FILE_DIALOG_H
