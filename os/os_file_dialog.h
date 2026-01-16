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

typedef plex OsOpenFileDialogHandle OsOpenFileDialogHandle;
plex OsOpenFileDialogHandle
{
	Arena* arena;
	
	// If OsIsOpenFileDialogDone returns true than this is filled with a value OR error is set to something besides Result_Success
	FilePath chosenFilePath;
	Result error;
	
	#if TARGET_IS_LINUX
	int zenityExitCode;
	#endif
	
	#if TARGET_IS_LINUX
	DBusError dbusError;
	DBusConnection* dbusConnection;
	Str8 dbusRequestPath; //null-terminated
	#endif
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	Result OsDoOpenFileDialogBlocking(Arena* arena, FilePath* pathOut);
	PIG_CORE_INLINE void OsFreeOpenFileDialogAsyncHandle(OsOpenFileDialogHandle* handle);
	Result OsDoOpenFileDialogAsync(Arena* arena, bool allowBlocking, OsOpenFileDialogHandle* handleOut);
	Result OsCheckOpenFileDialogAsyncHandle(OsOpenFileDialogHandle* handle);
	bool OsIsOpenFileDialogDone(OsOpenFileDialogHandle* handle);
	void OsFreeOpenFileDialogHandle(OsOpenFileDialogHandle* handle);
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
PEXP Result OsDoOpenFileDialogBlocking(Arena* arena, FilePath* pathOut)
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
	AssertMsg(false, "OsDoOpenFileDialogBlocking does not support the current platform yet!");
	result = Result_UnsupportedPlatform;
	#endif
	
	return result;
}

#if TARGET_IS_LINUX
// DBusHandleMessageFunction = https://dbus.freedesktop.org/doc/api/html/group__DBusConnection.html#ga5d7721ab952bd87d9a84f26d61709ad6
static DBusHandlerResult OsOpenFileDialogSignalHandler(DBusConnection* connection, DBusMessage* message, void* userData)
{
	NotNull(userData);
	OsOpenFileDialogHandle* handle = (OsOpenFileDialogHandle*)userData;
	// PrintLine_D("Got D-Bus signal on connection %p message %p with data %p", connection, message, userData);
	
	if (dbus_message_is_signal(message, "org.freedesktop.portal.Request", "Response"))
	{
		// WriteLine_D("Signal is a Request Response!");
		const char* responsePathNt = dbus_message_get_path(message);
		NotNull(responsePathNt);
		Str8 responsePath = MakeStr8Nt(responsePathNt);
		if (StrExactEquals(responsePath, handle->dbusRequestPath))
		{
			// PrintLine_D("Response has matching path for our request! \"%s\"", responsePathNt);
			
			BeginBreakableBlock(errorProneBlock)
			{
				DBusMessageIter messageArgs;
				dbus_bool_t iterInitResult = dbus_message_iter_init(message, &messageArgs);
				if (!iterInitResult) { WriteLine_E("Failed to initialize message iterator!"); BreakBlock(errorProneBlock); }
				
				int firstArgType = dbus_message_iter_get_arg_type(&messageArgs);
				if (firstArgType != DBUS_TYPE_UINT32) { PrintLine_E("Unexpected arg type %d. Expected UINT32(%d)", firstArgType, (int)DBUS_TYPE_UINT32); BreakBlock(errorProneBlock); }
				u32 responseCode = 0;
				dbus_message_iter_get_basic(&messageArgs, &responseCode);
				// PrintLine_I("Response Code: %u (0x%08X)", responseCode, responseCode);
				
				if (responseCode == 1) //user cancelled
				{
					handle->error = Result_Canceled;
					BreakBlock(errorProneBlock);
				}
				else if (responseCode != 0)
				{
					PrintLine_W("Unknown open file dialog response code %u. Expected 0 or 1 for success or cancelled", responseCode);
					handle->error = Result_Unknown;
					BreakBlock(errorProneBlock);
				}
				
				dbus_bool_t nextResult = dbus_message_iter_next(&messageArgs);
				if (!nextResult) { WriteLine_E("Failed to iterate to second argument!"); BreakBlock(errorProneBlock); }
				
				int secondArgType = dbus_message_iter_get_arg_type(&messageArgs);
				if (secondArgType != DBUS_TYPE_ARRAY) { PrintLine_E("Unexpected arg type %d. Expected ARRAY(%d)", secondArgType, (int)DBUS_TYPE_ARRAY); BreakBlock(errorProneBlock); }
				
				DBusMessageIter arrayIter;
				dbus_message_iter_recurse(&messageArgs, &arrayIter);
				while (dbus_message_iter_get_arg_type(&arrayIter) == DBUS_TYPE_DICT_ENTRY)
				{
					DBusMessageIter entryIter;
					dbus_message_iter_recurse(&arrayIter, &entryIter);
					const char* entryKeyNt = nullptr;
					dbus_message_iter_get_basic(&entryIter, &entryKeyNt);
					Str8 entryKey = MakeStr8Nt(entryKeyNt);
					if (StrExactEquals(entryKey, StrLit("uris")))
					{
						dbus_message_iter_next(&entryIter);
						DBusMessageIter valueIter;
						dbus_message_iter_recurse(&entryIter, &valueIter);
						int valueType = dbus_message_iter_get_arg_type(&valueIter);
						if (valueType == DBUS_TYPE_ARRAY)
						{
							DBusMessageIter uriArrayIter;
							dbus_message_iter_recurse(&valueIter, &uriArrayIter);
							while (dbus_message_iter_get_arg_type(&uriArrayIter) == DBUS_TYPE_STRING)
							{
								const char* chosenFilePathNt = nullptr;
								dbus_message_iter_get_basic(&uriArrayIter, &chosenFilePathNt);
								
								if (chosenFilePathNt != nullptr && chosenFilePathNt[0] != '\0')
								{
									handle->chosenFilePath = MakeStr8Nt(chosenFilePathNt);
									if (StrAnyCaseStartsWith(handle->chosenFilePath, StrLit("file://")))
									{
										handle->chosenFilePath = StrSliceFrom(handle->chosenFilePath, 7);
									}
									handle->chosenFilePath = AllocStr8(handle->arena, handle->chosenFilePath);
									NotNullStr(handle->chosenFilePath);
									FixPathSlashes(handle->chosenFilePath);
									handle->error = Result_Success;
									break;
								}
								
								if (!dbus_message_iter_next(&arrayIter)) { break; }
							}
							if (handle->error == Result_Success) { break; }
						}
					}
					else
					{
						// PrintLine_D("Unhandled dictionary entry \"%.*s\"", StrPrint(entryKey));
					}
					
					if (!dbus_message_iter_next(&arrayIter)) { break; }
				}
				
				if (handle->error != Result_Success)
				{
					WriteLine_E("Failed to find the selected file path in the response argument dictionary");
					handle->error = Result_Unknown; //TODO: Better error code
				}
			} EndBreakableBlock(errorProneBlock);
			
			dbus_connection_close(handle->dbusConnection);
			dbus_connection_unref(handle->dbusConnection);
			handle->dbusConnection = nullptr;
			if (handle->error == Result_None || handle->error == Result_Ongoing) { handle->error = Result_Unknown; }
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		else
		{
			PrintLine_W("Response has incorrect path \"%s\". We expect \"%.*s\"", responsePathNt, StrPrint(handle->dbusRequestPath));
		}
	}
	else
	{
		const char* interfaceName = dbus_message_get_interface(message);
		const char* memberName = dbus_message_get_member(message);
		// PrintLine_D("Got signal for interface \"%s\" function \"%s\"", interfaceName, memberName);
	}
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
#endif //TARGET_IS_LINUX

PEXPI void OsFreeOpenFileDialogAsyncHandle(OsOpenFileDialogHandle* handle)
{
	NotNull(handle);
	if (handle->arena != nullptr)
	{
		if (handle->chosenFilePath.chars != nullptr) { FreeStr8(handle->arena, &handle->chosenFilePath); }
		
		#if TARGET_IS_LINUX
		if (dbus_error_is_set(&handle->dbusError)) { dbus_error_free(&handle->dbusError); }
		if (handle->dbusConnection != nullptr)
		{
			dbus_connection_remove_filter(handle->dbusConnection, OsOpenFileDialogSignalHandler, (void*)handle);
			dbus_connection_close(handle->dbusConnection);
		}
		if (handle->dbusRequestPath.chars != nullptr) { FreeStr8WithNt(handle->arena, &handle->dbusRequestPath); }
		#endif //TARGET_IS_LINUX
	}
	ClearPointer(handle);
}

PEXP Result OsDoOpenFileDialogAsync(Arena* arena, bool allowBlocking, OsOpenFileDialogHandle* handleOut)
{
	NotNull(arena);
	NotNull(handleOut);
	ClearPointer(handleOut);
	
	#if TARGET_IS_LINUX
	BeginBreakableBlock(errorBlock)
	{
		handleOut->arena = arena;
		
		WriteLine_D("Initializing D-Bus connection to xdg.desktop.portal for OsDoOpenFileDialogAsync...");
		dbus_error_init(&handleOut->dbusError);
		handleOut->dbusConnection = dbus_bus_get_private(DBUS_BUS_SESSION, &handleOut->dbusError);
		if (dbus_error_is_set(&handleOut->dbusError))
		{
			PrintLine_E("DBUS Connection Error: %s", handleOut->dbusError.message);
			handleOut->error = Result_DBusError;
			BreakBlock(errorBlock);
		}
		if (handleOut->dbusConnection == nullptr)
		{
			WriteLine_E("DBUS Connection Failed!");
			handleOut->error = Result_Unknown; //TODO: Better error code
			BreakBlock(errorBlock);
		}
		
		// PrintLine_D("Registering D-Bus connection %p...", handleOut->dbusConnection);
		dbus_bus_register(handleOut->dbusConnection, &handleOut->dbusError);
		if (dbus_error_is_set(&handleOut->dbusError))
		{
			PrintLine_E("D-Bus Register error: %s", handleOut->dbusError.message);
			handleOut->error = Result_DBusError;
			BreakBlock(errorBlock);
		}
		
		//Register a signal "filter" which will catch the response that contains the file the user chose
		// PrintLine_D("Registering D-Bus signal handler with userData %p...", handleOut);
		dbus_bool_t addFilterResult = dbus_connection_add_filter(handleOut->dbusConnection, OsOpenFileDialogSignalHandler, (void*)handleOut, nullptr);
		if (!addFilterResult)
		{
			WriteLine_E("Failed to register D-Bus signal handler!");
			handleOut->error = Result_Unknown; //TODO: Better error code
			BreakBlock(errorBlock);
		}
		
		// https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.FileChooser.html
		// OpenFile(IN parent_window s, IN title s, IN options a{sv}, OUT handle o)
		// WriteLine_D("Finding FileChooser.OpenFile...");
		DBusMessage* requestMsg = dbus_message_new_method_call(
			"org.freedesktop.portal.Desktop", //destination
			"/org/freedesktop/portal/desktop", //path
			"org.freedesktop.portal.FileChooser", //interface
			"OpenFile" //method
		);
		if (requestMsg == nullptr)
		{
			WriteLine_E("Failed to create D-Bus request message!");
			handleOut->error = Result_Unknown; //TODO: Better error code
			BreakBlock(errorBlock);
		}
		
		// WriteLine_D("Preparing OpenFile args...");
		DBusMessageIter requestArgs;
		DBusMessageIter requestOpts;
		dbus_message_iter_init_append(requestMsg, &requestArgs);
		const char* parentWindow = "";
		const char* windowTitle = "All Files";
		if (!dbus_message_iter_append_basic(&requestArgs, DBUS_TYPE_STRING, &parentWindow)) { handleOut->error = Result_Unknown; BreakBlock(errorBlock); }
		if (!dbus_message_iter_append_basic(&requestArgs, DBUS_TYPE_STRING, &windowTitle)) { handleOut->error = Result_Unknown; BreakBlock(errorBlock); }
		if (!dbus_message_iter_open_container(&requestArgs, DBUS_TYPE_ARRAY, "{sv}", &requestOpts)) { handleOut->error = Result_Unknown; BreakBlock(errorBlock); }
		//TODO: Fill in any of the options in this array: handle_token, accept_label, modal, multiple, directory, filters, current_filter, choices, current_folder
		if (!dbus_message_iter_close_container(&requestArgs, &requestOpts)) { handleOut->error = Result_Unknown; BreakBlock(errorBlock); }
		
		// WriteLine_D("Registering response listener...");
		dbus_bus_add_match(
			handleOut->dbusConnection,
			"type='signal'"
			",interface='org.freedesktop.portal.Request'"
			",member='Response'",
			&handleOut->dbusError
		);
		if (dbus_error_is_set(&handleOut->dbusError))
		{
			PrintLine_E("D-Bus error: %s", handleOut->dbusError.message);
			handleOut->error = Result_DBusError;
			BreakBlock(errorBlock);
		}
		
		// The dialog process is asynchronous but this call returns immediately with a "path" that we can use to identify the final Response later (in the signal filter callback)
		// Thus "blocking" here doesn't mean blocking for the duration of the dialog being open, only for the duration of the D-Bus message and reply for the initial request
		// (we're gonna call this initial reply an acknowledgement or "ack")
		// WriteLine_D("Starting call...");
		DBusMessage* dbusAck = dbus_connection_send_with_reply_and_block(handleOut->dbusConnection, requestMsg, DBUS_TIMEOUT_USE_DEFAULT, &handleOut->dbusError);
		dbus_message_unref(requestMsg);
		if (dbus_error_is_set(&handleOut->dbusError))
		{
			PrintLine_E("D-Bus Send error: %s", handleOut->dbusError.message);
			handleOut->error = Result_DBusError;
			BreakBlock(errorBlock);
		}
		if (dbusAck == nullptr)
		{
			handleOut->error = Result_Unknown;
			BreakBlock(errorBlock);
		}
		
		DBusMessageIter dbusAckArgs;
		dbus_bool_t initResult = dbus_message_iter_init(dbusAck, &dbusAckArgs);
		if (!initResult)
		{
			WriteLine_E("Failed to initialize DBusMessageIter on ack from OpenFile call");
			dbus_message_unref(dbusAck);
			handleOut->error = Result_Unknown;
			BreakBlock(errorBlock);
		}
		int dbusAckArgType = dbus_message_iter_get_arg_type(&dbusAckArgs);
		if (dbusAckArgType != DBUS_TYPE_OBJECT_PATH)
		{
			PrintLine_E("Ack for DBus OpenFile request did not contain an object path like we expected. Type: %d", dbusAckArgType);
			dbus_message_unref(dbusAck);
			handleOut->error = Result_Unknown;
			BreakBlock(errorBlock);
		}
		
		const char* dbusRequestPathNt = nullptr;
		dbus_message_iter_get_basic(&dbusAckArgs, &dbusRequestPathNt);
		NotNull(dbusRequestPathNt);
		handleOut->dbusRequestPath = AllocStr8Nt(arena, dbusRequestPathNt);
		// PrintLine_D("D-Bus request path: \"%.*s\"", StrPrint(handleOut->dbusRequestPath));
		
		// WriteLine_D("Waiting for response...");
		handleOut->error = Result_Ongoing;
	} EndBreakableBlock(errorBlock);
	
	#else
	if (allowBlocking)
	{
		handleOut->arena = arena;
		handleOut->error = OsDoOpenFileDialogBlocking(arena, &handleOut->chosenFilePath);
	}
	else
	{
		AssertMsg(false, "OsDoOpenFileDialogAsync does not support the current platform yet!");
		handleOut->error = Result_UnsupportedPlatform;
	}
	#endif
	
	if (handleOut->error != Result_Success && handleOut->error != Result_Ongoing)
	{
		Result error = handleOut->error;
		OsFreeOpenFileDialogAsyncHandle(handleOut);
		handleOut->error = error;
	}
	return handleOut->error;
}

PEXP Result OsCheckOpenFileDialogAsyncHandle(OsOpenFileDialogHandle* handle)
{
	NotNull(handle);
	if (handle->error != Result_Ongoing && handle->error != Result_None) { return handle->error; }
	if (handle->arena == nullptr) { handle->error = Result_Uninitialized; return handle->error; }
	
	#if TARGET_IS_LINUX
	{
		dbus_bool_t connectionStillAlive = dbus_connection_read_write_dispatch(handle->dbusConnection, 0);
		if (!connectionStillAlive)
		{
			if (handle->dbusConnection != nullptr && (handle->error == Result_None || handle->error == Result_Ongoing))
			{
				WriteLine_E("D-Bus Connection closed without signal handler collecting a result!");
				handle->error = Result_Disconnected;
				dbus_connection_close(handle->dbusConnection); //TODO: Do we need to do this?
				dbus_connection_unref(handle->dbusConnection); //TODO: Do we need to do this?
				handle->dbusConnection = nullptr;
			}
		}
	}
	#else
	AssertMsg(false, "OsCheckOpenFileDialogAsyncHandle does not support the current platform yet!");
	handle->error = Result_UnsupportedPlatform;
	#endif
	
	return handle->error;
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
