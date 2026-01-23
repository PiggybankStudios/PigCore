/*
File:   os_program.h
Author: Taylor Robbins
Date:   01\18\2025
Description:
	** Contains functions that help us get information about our process and other
	** processes running on the OS. Also contains functions like OsGetSettingsSavePath
	** which aren't strictly related to our process, but provide important
	** info for a process to function
*/

#ifndef _OS_PROGRAM_H
#define _OS_PROGRAM_H

#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"
#include "base/base_notifications.h"
#include "std/std_includes.h"
#include "misc/misc_result.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "misc/misc_printing.h"
#include "os/os_path.h"
#include "os/os_file.h"

#if TARGET_IS_WINDOWS
typedef HWND OsWindowHandle;
#define OsWindowHandleEmpty NULL
#elif TARGET_IS_LINUX && BUILD_WITH_SOKOL_APP
typedef Window OsWindowHandle;
#define OsWindowHandleEmpty 0
#elif TARGET_IS_OSX
typedef NSWindow* OsWindowHandle;
#define OsWindowHandleEmpty nullptr
#elif TARGET_IS_ANDROID
typedef plex ANativeWindow* OsWindowHandle;
#define OsWindowHandleEmpty nullptr
#else
typedef void* OsWindowHandle;
#define OsWindowHandleEmpty nullptr
#endif

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	FilePath OsGetExecutablePath(Arena* arena, Result* resultOut);
	FilePath OsGetWorkingDirectory(Arena* arena, Result* resultOut);
	FilePath OsGetSettingsSavePath(Arena* arena, Str8 companyName, Str8 programName, bool createFolders);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

// Always returns path with forward slashes and null-terminating char (unless it fails, then it returns FilePath_Empty)
PEXP FilePath OsGetExecutablePath(Arena* arena, Result* resultOut)
{
	NotNull(arena);
	SetOptionalOutPntr(resultOut, Result_None);
	FilePath resultPath = FilePath_Empty;
	
	#if TARGET_IS_WINDOWS
	{
		#define MAX_EXECUTABLE_PATH_LENGTH 1024
		//TODO: Should we use _pgmptr global variable instead of calling GetModuleFileNameA(NULL, ...)
		
		ScratchBegin1(scratch, arena);
		char* tempBuffer = (char*)AllocMem(scratch, MAX_EXECUTABLE_PATH_LENGTH);
		NotNull(tempBuffer);
		DWORD pathLength = GetModuleFileNameA(
			NULL, //hModule
			tempBuffer, //lpFilename
			MAX_EXECUTABLE_PATH_LENGTH //nSize
		);
		ScratchEnd(scratch);
		
		if (pathLength <= 0)
		{
			//TODO: Check GetLastError to get a more specific error Result
			SetOptionalOutPntr(resultOut, Result_EmptyPath);
			return FilePath_Empty;
		}
		if (pathLength >= MAX_EXECUTABLE_PATH_LENGTH)
		{
			DebugAssertMsg(false, "Our MAX_EXECUTABLE_PATH_LENGTH was not large enough. We should probably bump the constant");
			SetOptionalOutPntr(resultOut, Result_ExePathTooLong);
			return FilePath_Empty;
		}
		
		char* resultBuffer = (char*)AllocMem(arena, (uxx)(pathLength+1));
		if (resultBuffer == nullptr)
		{
			SetOptionalOutPntr(resultOut, Result_FailedToAllocateMemory);
			return FilePath_Empty;
		}
		
		DWORD resultLength = GetModuleFileNameA(
			NULL, //hModule
			resultBuffer, //lpFilename
			pathLength+1 //nSize
		);
		Assert(resultLength == pathLength);
		resultBuffer[resultLength] = '\0';
		
		Assert(resultLength <= UINTXX_MAX);
		resultPath = MakeFilePath((uxx)resultLength, resultBuffer);
		FixPathSlashes(resultPath);
		SetOptionalOutPntr(resultOut, Result_Success);
	}
	#elif TARGET_IS_LINUX
	{
		ScratchBegin1(scratch, arena);
		
		//TODO: Rather than using PATH_MAX here we should call lstat("/proc/self/exe") and look at stat.st_size to find the size of the executable path
		char* scratchBuffer = (char*)AllocMem(scratch, PATH_MAX);
		NotNull(scratchBuffer);
		ssize_t readLinkResult = readlink(
			"/proc/self/exe", //path
			scratchBuffer, //buf
			PATH_MAX //bufsiz
		);
		if (readLinkResult < 0)
		{
			SetOptionalOutPntr(resultOut, Result_Failure); //TODO: check errno and report better errors
			return FilePath_Empty;
		}
		
		Assert((u64)readLinkResult <= UINTXX_MAX);
		resultPath.length = (uxx)readLinkResult;
		resultPath.chars = (char*)AllocMem(arena, resultPath.length+1);
		if (resultPath.chars == nullptr)
		{
			SetOptionalOutPntr(resultOut, Result_FailedToAllocateMemory);
			return FilePath_Empty;
		}
		
		if (resultPath.length > 0) { MyMemCopy(resultPath.chars, scratchBuffer, resultPath.length); }
		resultPath.chars[resultPath.length] = '\0';
		FixPathSlashes(resultPath);
		SetOptionalOutPntr(resultOut, Result_Success);
		
		ScratchEnd(scratch);
	}
	// #elif TARGET_IS_OSX
	// {
	//	//TODO: Implement me!
	// 	SetOptionalOutPntr(resultOut, Result_UnsupportedPlatform);
	// 	return FilePath_Empty;
	// }
	#else
	{
		AssertMsg(false, "OsGetExecutablePath does not support the current platform yet!");
		SetOptionalOutPntr(resultOut, Result_UnsupportedPlatform);
	}
	#endif
	
	return resultPath;
}

// Always returns path with forward slashes
// Guarantees that the path ends with a forward slash
PEXP FilePath OsGetWorkingDirectory(Arena* arena, Result* resultOut)
{
	NotNull(arena);
	SetOptionalOutPntr(resultOut, Result_None);
	FilePath resultPath = FilePath_Empty;
	
	#if TARGET_IS_WINDOWS
	{
		//NOTE: When passing 0, nullptr then the returned value INCLUDES a byte for null-terminator
		DWORD bufferSizeNeeded = GetCurrentDirectoryA(
			0, //nBufferLength
			nullptr //lpBuffer
		);
		if (bufferSizeNeeded <= 0)
		{
			//TODO: Check GetLastError to get a more specific error Result
			SetOptionalOutPntr(resultOut, Result_EmptyPath);
			return FilePath_Empty;
		}
		
		ScratchBegin1(scratch, arena);
		char* scratchBuffer = (char*)AllocMem(scratch, (uxx)bufferSizeNeeded);
		if (scratchBuffer == nullptr)
		{
			SetOptionalOutPntr(resultOut, Result_FailedToAllocateMemory);
			ScratchEnd(scratch);
			return FilePath_Empty;
		}
		
		//When passing an actual scratchBuffer the returned value DOES NOT INCLUDE a byte for null-terminator
		DWORD resultLength = GetCurrentDirectoryA(
			bufferSizeNeeded, //nBufferLength
			scratchBuffer //lpBuffer
		);
		Assert(resultLength == bufferSizeNeeded-1);
		
		resultPath = AllocFolderPath(arena, MakeStr8((uxx)resultLength, scratchBuffer), true);
		SetOptionalOutPntr(resultOut, Result_Success);
		ScratchEnd(scratch);
	}
	#elif TARGET_IS_LINUX
	{
		ScratchBegin1(scratch, arena);
		
		char* scratchBuffer = (char*)AllocMem(scratch, PATH_MAX);
		NotNull(scratchBuffer);
		
		//TODO: After POSIX.1-2001 we can pass nullptr, 0 to have getcwd call malloc with appropriate space
		char* getCwdResult = getcwd(scratchBuffer, PATH_MAX);
		scratchBuffer[PATH_MAX-1] = '\0'; //getcwd should ensure null-termination but just to be sure let's write that last byte again
		if (getCwdResult == nullptr)
		{
			//TODO: if ERANGE was returned then we should allocate a larger scratch buffer?
			//TODO: Check errno and give better error Result!
			SetOptionalOutPntr(resultOut, Result_Failure);
			ScratchEnd(scratch);
			return FilePath_Empty;
		}
		
		uxx pathLength = (uxx)MyStrLength(getCwdResult);
		bool needTrailingSlash = (pathLength == 0 || !(getCwdResult[pathLength-1] == '\\' || getCwdResult[pathLength-1] == '/'));
		resultPath.length = pathLength + (needTrailingSlash ? 1 : 0);
		DebugAssert(resultPath.length < PATH_MAX);
		resultPath.chars = (char*)AllocMem(arena, resultPath.length+1);
		if (resultPath.chars == nullptr)
		{
			SetOptionalOutPntr(resultOut, Result_FailedToAllocateMemory);
			ScratchEnd(scratch);
			return FilePath_Empty;
		}
		
		if (resultPath.length > 0) { MyMemCopy(resultPath.chars, getCwdResult, pathLength); }
		if (needTrailingSlash) { resultPath.chars[pathLength] = '/'; }
		resultPath.chars[resultPath.length] = '\0';
		FixPathSlashes(resultPath);
		Assert(DoesPathHaveTrailingSlash(resultPath));
		
		SetOptionalOutPntr(resultOut, Result_Success);
		ScratchEnd(scratch);
	}
	// #elif TARGET_IS_OSX
	// {
	// 	//TODO: Implement me!
	// 	SetOptionalOutPntr(resultOut, Result_UnsupportedPlatform);
	// 	return FilePath_Empty;
	// }
	#else
	{
		AssertMsg(false, "OsGetWorkingDirectory does not support the current platform yet!");
		SetOptionalOutPntr(resultOut, Result_UnsupportedPlatform);
	}
	#endif
	
	return resultPath;
}

PEXP FilePath OsGetSettingsSavePath(Arena* arena, Str8 companyName, Str8 programName, bool createFolders)
{
	NotNull(arena);
	NotNullStr(companyName);
	NotNullStr(programName);
	FilePath result = FilePath_Empty;
	
	#if TARGET_IS_WINDOWS
	{
		Assert(!IsEmptyStr(companyName) || !IsEmptyStr(programName));
		//TODO: Should we assert that companyName and programName have valid characters to use in folder names on the current OS?
		ScratchBegin1(scratch, arena);
		char* pathBuffer = (char*)AllocMem(arena, MAX_PATH);
		NotNull(pathBuffer);
		
		BOOL getFolderPathResult = SHGetSpecialFolderPathA(
			NULL, //hwnd
			&pathBuffer[0], //pszPath
			CSIDL_APPDATA, //csidl
			false //fCreate
		);
		if (getFolderPathResult == FALSE)
		{
			NotifyPrint_E("SHGetSpecialFolderPathA(CSIDL_APPDATA) failed!");
			ScratchEnd(scratch);
			return FilePath_Empty;
		}
		// Assert(BufferIsNullTerminated(MAX_PATH, pathBuffer)); TODO: Re-enable me once we have BufferIsNullTerminated?
		
		result = MakeFilePath((uxx)MyStrLength(&pathBuffer[0]), pathBuffer);
		Assert(!IsEmptyStr(result));
		FixPathSlashes(result);
		
		if (!IsEmptyStr(companyName) && !IsEmptyStr(programName))
		{
			result = PrintInArenaStr(scratch, "%.*s%s%.*s/%.*s",
				StrPrint(result),
				DoesPathHaveTrailingSlash(result) ? "" : "/",
				StrPrint(companyName),
				StrPrint(programName)
			);
		}
		else if (!IsEmptyStr(companyName))
		{
			result = PrintInArenaStr(scratch, "%.*s%s%.*s",
				StrPrint(result),
				DoesPathHaveTrailingSlash(result) ? "" : "/",
				StrPrint(companyName)
			);
		}
		else if (!IsEmptyStr(programName))
		{
			result = PrintInArenaStr(scratch, "%.*s%s%.*s",
				StrPrint(result),
				DoesPathHaveTrailingSlash(result) ? "" : "/",
				StrPrint(programName)
			);
		}
		else if (DoesPathHaveTrailingSlash(result))
		{
			result = StrSlice(result, 0, result.length-1);
		}
		
		result = AllocStr8(arena, result);
		NotNull(result.chars);
		
		if (createFolders)
		{
			Result createFolderResult = OsCreateFolder(result, true);
			if (createFolderResult != Result_Success)
			{
				PrintLine_E("Failed to create \"%.*s\"!", StrPrint(result));
				FreeStr8(arena, &result);
				ScratchEnd(scratch);
				return FilePath_Empty;
			}
		}
		
		ScratchEnd(scratch);
	}
	#elif TARGET_IS_LINUX
	{
		Assert(!IsEmptyStr(companyName) || !IsEmptyStr(programName));
		ScratchBegin1(scratch, arena);
		
		// This should return something line ~/.config, inside that folder we should create our own folder
		// The user could change where this points to if they want though, so we aren't guaranteed anything about where this folder might be
		// See https://wiki.archlinux.org/title/XDG_Base_Directory
		// TODO: getenv is technically not thread safe. Maybe we should do this once, on a single thread, and then cache it for all threads?
		char* configFolderPathNt = getenv("XDG_CONFIG_HOME");
		if (configFolderPathNt == nullptr || configFolderPathNt[0] == '\0')
		{
			WriteLine_W("$XDG_CONFIG_HOME environment var is not set! Falling back to $HOME!");
			configFolderPathNt = getenv("HOME");
			if (configFolderPathNt == nullptr || configFolderPathNt[0] == '\0')
			{
				AssertMsg(configFolderPathNt != nullptr && configFolderPathNt[0] != '\0', "$XDG_CONFIG_HOME and $HOME environment vars are both not set! We don't know where to save our settings and other persistent info");
				return FilePath_Empty;
			}
		}
		Str8 configFolderPath = MakeStr8Nt(configFolderPathNt);
		
		if (DoesPathHaveTrailingSlash(configFolderPath)) { configFolderPath.length--; }
		result = PrintInArenaStr(arena, "%.*s/%.*s", StrPrint(configFolderPath), StrPrint(programName));
		NotNullStr(result);
		FixPathSlashes(result);
		
		if (createFolders)
		{
			Result createFolderResult = OsCreateFolder(result, true);
			if (createFolderResult != Result_Success)
			{
				PrintLine_E("Failed to create \"%.*s\"!", StrPrint(result));
				FreeStr8(arena, &result);
				ScratchEnd(scratch);
				return FilePath_Empty;
			}
		}
		
		ScratchEnd(scratch);
	}
	#elif TARGET_IS_ANDROID
	{
		AssertMsg(AndroidNativeActivity != nullptr, "You must set AndroidNativeActivity global before calling OsGetSettingsSavePath!");
		AssertMsg(AndroidJavaVM != nullptr, "You must set AndroidJavaVM global before calling OsGetSettingsSavePath!");
		UNUSED(companyName);
		UNUSED(programName);
		UNUSED(createFolders);
		
		JavaVMAttachBlock(env)
		{
			jobject fileObj = jCall_getFilesDir(env, AndroidNativeActivity);
			jstring pathString = jCall_getAbsolutePath(env, fileObj);
			result = ToStr8FromJStr(env, arena, pathString, false);
			FixPathSlashes(result);
			(*env)->DeleteLocalRef(env, pathString);
		}
	}
	#else
	AssertMsg(false, "OsGetSettingsSavePath does not support the current platform yet!");
	#endif
	
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _OS_PROGRAM_H
