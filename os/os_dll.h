/*
File:   os_dll.h
Author: Taylor Robbins
Date:   03\01\2025
*/

#ifndef _OS_DLL_H
#define _OS_DLL_H

#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"
#include "std/std_includes.h"
#include "struct/struct_string.h"
#include "mem/mem_scratch.h"
#include "os/os_error.h"
#include "misc/misc_result.h"
#include "os/os_path.h"

//NOTE: The name "dll" is a misnomer on non-windows platforms. This can represent a .dll or a .so
typedef struct OsDll OsDll;
struct OsDll
{
	#if TARGET_IS_WINDOWS
	HMODULE handle;
	#elif TARGET_IS_LINUX
	void* handle;
	#else
	int placeholder;
	#endif
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	Result OsLoadDll(FilePath path, OsDll* dllOut);
	void* OsFindDllFunc(OsDll* dll, Str8 funcName);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

// PEXP void OsCloseDll(OsDll* dll)
// {
//	TODO: Implement me!
// }

PEXP Result OsLoadDll(FilePath path, OsDll* dllOut)
{
	NotNullStr(path);
	NotNull(dllOut);
	Result result = Result_None;
	
	#if TARGET_IS_WINDOWS
	{
		ScratchBegin(scratch);
		FilePath pathNt = AllocFilePath(scratch, path, true);
		ChangePathSlashesTo(pathNt, '\\');
		dllOut->handle = LoadLibraryA(pathNt.chars);
		if (dllOut->handle != NULL) { result = Result_Success; }
		else { result = Result_Failure; }
		ScratchEnd(scratch);
	}
	#elif TARGET_IS_LINUX
	{
		ScratchBegin(scratch);
		FilePath pathNt = AllocFilePath(scratch, path, true);
		ChangePathSlashesTo(pathNt, '/');
		dllOut->handle = dlopen(
			pathNt.chars, //file
			RTLD_NOW //mode TODO: Do we ever want RTLD_LAZY?
		);
		if (dllOut->handle != nullptr) { result = Result_Success; }
		else
		{
			char* errorStr = dlerror();
			PrintLine_E("dlopen failed: \"%s\"", errorStr);
			result = Result_Failure;
		}
		ScratchEnd(scratch);
	}
	#else
	UNUSED(path);
	AssertMsg(false, "OsLoadDll does not support the current platform yet!");
	result = Result_UnsupportedPlatform;
	#endif
	
	return result;
}

PEXP void* OsFindDllFunc(OsDll* dll, Str8 funcName)
{
	NotNull(dll);
	NotNullStr(funcName);
	void* result = nullptr;
	
	#if TARGET_IS_WINDOWS
	{
		ScratchBegin(scratch);
		Str8 funcNameNt = AllocStrAndCopy(scratch, funcName.length, funcName.chars, true);
		result = (void*)GetProcAddress(dll->handle, funcNameNt.chars);
		ScratchEnd(scratch);
	}
	#elif TARGET_IS_LINUX
	{
		ScratchBegin(scratch);
		Str8 funcNameNt = AllocStrAndCopy(scratch, funcName.length, funcName.chars, true);
		result = (void*)dlsym(dll->handle, funcNameNt.chars);
		ScratchEnd(scratch);
	}
	#else
	UNUSED(dll);
	UNUSED(funcName);
	AssertMsg(false, "OsFindDllFunc does not support the current platform yet!");
	#endif
	
	return result;
	
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _OS_DLL_H
