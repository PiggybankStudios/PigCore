/*
File:   os_program.h
Author: Taylor Robbins
Date:   01\18\2025
Description:
	** Contains functions that help us get information about our process and other
	** processes running on the OS.
*/

#ifndef _OS_PROGRAM_H
#define _OS_PROGRAM_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "std/std_includes.h"
#include "misc/misc_result.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "os/os_path.h"

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	FilePath OsGetExecutablePath(Arena* arena, Result* resultOut);
	FilePath OsGetWorkingDirectory(Arena* arena, Result* resultOut);
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
		resultPath = NewFilePath((uxx)resultLength, resultBuffer);
		FixPathSlashes(resultPath);
		SetOptionalOutPntr(resultOut, Result_Success);
	}
	// #elif TARGET_IS_LINUX
	// {
	// 	//TODO: Implement me!
	// 	SetOptionalOutPntr(resultOut, Result_UnsupportedPlatform);
	// 	return FilePath_Empty;
	// }
	// #elif TARGET_IS_OSX
	// {
	//	//TODO: Implement me!
	// 	SetOptionalOutPntr(resultOut, Result_UnsupportedPlatform);
	// 	return FilePath_Empty;
	// }
	#else
	{
		AssertMsg(false, "OsGetExecutablePath does not support the current platform yet!")
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
		
		resultPath = AllocFolderPath(arena, NewStr8((uxx)resultLength, scratchBuffer), true);
		SetOptionalOutPntr(resultOut, Result_Success);
		ScratchEnd(scratch);
	}
	// #elif TARGET_IS_LINUX
	// {
	// 	//TODO: Implement me!
	// 	SetOptionalOutPntr(resultOut, Result_UnsupportedPlatform);
	// 	return FilePath_Empty;
	// }
	// #elif TARGET_IS_OSX
	// {
	// 	//TODO: Implement me!
	// 	SetOptionalOutPntr(resultOut, Result_UnsupportedPlatform);
	// 	return FilePath_Empty;
	// }
	#else
	{
		AssertMsg(false, "OsGetWorkingDirectory does not support the current platform yet!")
		SetOptionalOutPntr(resultOut, Result_UnsupportedPlatform);
	}
	#endif
	
	return resultPath;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _OS_PROGRAM_H
