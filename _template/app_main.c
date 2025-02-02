/*
File:   app_main.c
Author: Taylor Robbins
Date:   01\19\2025
Description: 
	** Contains the dll entry point and all exported functions that the platform
	** layer can lookup by name and call. Also includes all other source files
	** required for the application to compile.
*/

#include "build_config.h"
#define PIG_CORE_IMPLEMENTATION BUILD_INTO_SINGLE_UNIT

#include "base/base_all.h"
#include "std/std_all.h"
#include "os/os_all.h"
#include "misc/misc_all.h"
#include "mem/mem_all.h"
#include "struct/struct_all.h"
#include "gfx/gfx_all.h"

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
#if !BUILD_INTO_SINGLE_UNIT //NOTE: The platform layer already has this global
static Arena stdHeapStruct = ZEROED;
static Arena* stdHeap = nullptr;
#endif

// +--------------------------------------------------------------+
// |                       Main Entry Point                       |
// +--------------------------------------------------------------+
#if (TARGET_IS_WINDOWS && !BUILD_INTO_SINGLE_UNIT)
BOOL WINAPI DllMain(
	HINSTANCE hinstDLL, // handle to DLL module
	DWORD fdwReason,    // reason for calling function
	LPVOID lpReserved)
{
	UNUSED(hinstDLL);
	UNUSED(lpReserved);
	switch(fdwReason)
	{ 
		case DLL_PROCESS_ATTACH:
		{
			InitArenaStdHeap(&stdHeapStruct);
			stdHeap = &stdHeapStruct;
			InitScratchArenasVirtual(Gigabytes(4));
			PrintLine_O("Hello from %s.dll!", PROJECT_DLL_NAME_STR);
		} break;
		
		case DLL_PROCESS_DETACH: PrintLine_W("%s.dll says Goodbye!", PROJECT_DLL_NAME_STR); break;
		case DLL_THREAD_ATTACH: WriteLine_D("dll says hello to a new thread!"); break;
		case DLL_THREAD_DETACH: WriteLine_D("dll says goodbye to a new thread!"); break;
		default: WriteLine_E("dll got an unknown reason in DllMain!"); break;
	}
	
	//If we don't return TRUE here then the LoadLibraryA call will return a failure!
	return TRUE;
}
#endif //(TARGET_IS_WINDOWS && !BUILD_INTO_SINGLE_UNIT)
