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
#include "struct/struct_string_buffer.h"

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

#if TARGET_IS_LINUX && BUILD_WITH_GTK
static void OsDoOpenFileDialogCallback(GObject* source, GAsyncResult* result, gpointer user_data)
{
	WriteLine_I("Got OpenFileDialog callback!");
	GError* error = nullptr;
	GFile* file = gtk_file_dialog_open_finish((GtkFileDialog*)source, result, &error);
	UNUSED(file);
	UNUSED(error);
}
#endif

//TODO: We should add a way for the application to define file filters and a starting directory
//TODO: Make a version of this function that allows opening multiple files at once
PEXP Result OsDoOpenFileDialog(Arena* arena, FilePath* pathOut)
{
	Assert(arena != nullptr || pathOut == nullptr);
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
			Str16 filePathStr16 = MakeStr16Nt(filePathPntr16);
			*pathOut = ConvertUcs2StrToUtf8(arena, filePathStr16, false);
			NotNullStr(*pathOut);
			FixPathSlashes(*pathOut);
		}
		
		CoTaskMemFree(filePathPntr16);
		shellItem->lpVtbl->Release(shellItem);
		dialogPntr->lpVtbl->Release(dialogPntr);
		result = Result_Success;
	}
	#elif (TARGET_IS_LINUX && BUILD_WITH_GTK)
	{
		GtkFileDialog* dialog = gtk_file_dialog_new();
		gtk_file_dialog_open(dialog, NULL, NULL, OsDoOpenFileDialogCallback, nullptr);
		//TODO: This implementation is incomplete AND only works in a full GTK application. In a non-GTK application, the Callback is never called (can't seem to find how I give GTK an opportunity to update and do callbacks)
		result = Result_Failure;
		
		#if 0
		//NOTE: This code was a suggestion, but gtk_file_chooser_dialog_new is deprecated in GTK 4.10+ and gtk_dialog_run is gone entirely
		{
			#if COMPILER_IS_CLANG
			#pragma clang diagnostic push
			#pragma clang diagnostic ignored "-Wdeprecated-declarations" //warning: 'gtk_file_chooser_dialog_new' is deprecated
			#endif
			GtkWidget* dialogWidget = gtk_file_chooser_dialog_new(
				"Open file", NULL,
				GTK_FILE_CHOOSER_ACTION_OPEN,
				"Cancel", GTK_RESPONSE_CANCEL,
				"Open", GTK_RESPONSE_OK,
				NULL
			);
			if (gtk_dialog_run(GTK_DIALOG(dialogWidget)) == GTK_RESPONSE_OK)
			{
				char* chosenFilePathNt = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialogWidget));
				NotNul(chosenFilePathNt);
				if (pathOut != nullptr)
				{
					*pathOut = AllocStr8Nt(arena, chosenFilePathNt);
					NotNullStr(*pathOut);
					FixPathSlashes(*pathOut);
				}
				result = Result_Success;
			}
			else
			{
				result = Result_Failure; //TODO: Better result code!
			}
			g_object_unref(dialogWidget);
			#if COMPILER_IS_CLANG
			#pragma clang diagnostic pop
			#endif
		}
		#endif
	}
	#elif TARGET_IS_LINUX
	{
		ScratchBegin1(scratch, arena);
		
		//File selection options: (from running `zenity --help-file-selection`)
		//	--file-selection                                  Display file selection dialog
		//	--filename=FILENAME                               Set the filename
		//	--multiple                                        Allow multiple files to be selected
		//	--directory                                       Activate directory-only selection
		//	--save                                            Activate save mode
		//	--separator=SEPARATOR                             Set output separator character
		//	--file-filter=NAME | PATTERN1 PATTERN2 ...        Set a filename filter
		//	--confirm-overwrite                               DEPRECATED; does nothing
		FILE* zenityOutputStream = popen("zenity --file-selection", "r");
		
		int zenityExitCode = 0;
		Str8 chosenFilePath = Str8_Empty;
		if (zenityOutputStream != nullptr)
		{
			//TODO: Can we use something like MAX_PATH instead of hard-coded 2048?
			StringBuffer buffer = NewStrBuffFromArena(scratch, 2048);
			while (!IsStrBuffFull(&buffer) && fgets(&buffer.chars[buffer.length], (buffer.maxLength-1) - buffer.length, zenityOutputStream) != NULL)
			{
				uxx numChars = (uxx)MyStrLength(&buffer.chars[buffer.length]);
				Assert(buffer.length + numChars < buffer.maxLength);
				buffer.length += numChars;
			}
			chosenFilePath = buffer.str;
			zenityExitCode = pclose(zenityOutputStream);
		}
		
		if (zenityExitCode == 0 && chosenFilePath.length > 0)
		{
			chosenFilePath = TrimWhitespaceAndNewLines(chosenFilePath);
			//TODO: Should we validate the path seems valid? If there is anything else that comes out the stdout when the exitCode is 0 then we may confuse output with a file path and return something weird to the application
			
			if (pathOut != nullptr)
			{
				*pathOut = AllocStr8(arena, chosenFilePath);
				NotNullStr(*pathOut);
				FixPathSlashes(*pathOut);
			}
			result = Result_Success;
		}
		else if (zenityExitCode == 127 || zenityExitCode == 127*256) //127 is standard "command not found" result for most shells. Then we multiply by 256 to account for how `wait` reports exit codes
		{
			Notify_W("Zenity is not installed! We can't open a file dialog without it! Please install it through your distro's package manager");
			result = Result_MissingDependency;
		}
		else if (zenityExitCode == 256) //256 means that the user selected "Cancel" in the dialog
		{
			result = Result_Canceled;
		}
		else if (zenityExitCode != 0)
		{
			NotifyPrint_W("Zenity-based dialog exited with code: %d", zenityExitCode);
			result = Result_Failure;
		}
		else
		{
			Notify_W("Zenity-based dialog returned an empty file path!");
			result = Result_EmptyPath;
		}
		
		ScratchEnd(scratch);
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
			Str16 filePathStr16 = MakeStr16Nt(filePathPntr16);
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
