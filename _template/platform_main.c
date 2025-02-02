/*
File:   platform_main.c
Author: Taylor Robbins
Date:   01\19\2025
Description: 
	** Holds the main entry point for the application and #includes all source files
	** that are needed for the platform layer to be compiled to an executable.
	** If BUILD_INTO_SINGLE_UNIT then this file #includes app_main.c as well.
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
#include "gfx/gfx_system_global.h"

#if BUILD_WITH_RAYLIB
#include "third_party/raylib/raylib.h"
#endif

#define ENABLE_RAYLIB_LOGS_DEBUG   0
#define ENABLE_RAYLIB_LOGS_INFO    0
#define ENABLE_RAYLIB_LOGS_WARNING 1
#define ENABLE_RAYLIB_LOGS_ERROR   1

// +--------------------------------------------------------------+
// |                         Header Files                         |
// +--------------------------------------------------------------+
// TODO: Add header files here

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
Arena* stdHeap = nullptr;

// +--------------------------------------------------------------+
// |                         Source Files                         |
// +--------------------------------------------------------------+
// TODO: Add source files here

#if BUILD_WITH_RAYLIB
void RaylibLogCallback(int logLevel, const char* text, va_list args)
{
	DbgLevel dbgLevel;
	switch (logLevel)
	{
		case LOG_TRACE:   dbgLevel = DbgLevel_Debug;   break;
		case LOG_DEBUG:   dbgLevel = DbgLevel_Debug;   break;
		case LOG_INFO:    dbgLevel = DbgLevel_Info;    break;
		case LOG_WARNING: dbgLevel = DbgLevel_Warning; break;
		case LOG_ERROR:   dbgLevel = DbgLevel_Error;   break;
		case LOG_FATAL:   dbgLevel = DbgLevel_Error;   break;
		default: dbgLevel = DbgLevel_Regular; break;
	}
	if (dbgLevel == DbgLevel_Debug && !ENABLE_RAYLIB_LOGS_DEBUG) { return; }
	if (dbgLevel == DbgLevel_Info && !ENABLE_RAYLIB_LOGS_INFO) { return; }
	if (dbgLevel == DbgLevel_Warning && !ENABLE_RAYLIB_LOGS_WARNING) { return; }
	if (dbgLevel == DbgLevel_Error && !ENABLE_RAYLIB_LOGS_ERROR) { return; }
	
	ScratchBegin(scratch);
	va_list argsCopy;
	va_copy(argsCopy, args);
	char* formattedText = nullptr;
	int formattedTextLength = MyVaListPrintf(nullptr, 0, text, args);
	if (formattedTextLength >= 0)
	{
		formattedText = AllocArray(char, scratch, formattedTextLength+1);
		if (formattedText != nullptr)
		{
			MyVaListPrintf(formattedText, formattedTextLength+1, text, argsCopy);
			formattedText[formattedTextLength] = '\0';
		}
	}
	va_end(argsCopy);
	if (formattedText != nullptr)
	{
		WriteLineAt(dbgLevel, formattedText);
	}
	else
	{
		WriteLine_E("RaylibLogCallback PRINT FAILURE!");
		WriteLineAt(dbgLevel, text);
	}
	ScratchEnd(scratch);
}
#endif //BUILD_WITH_RAYLIB

// +--------------------------------------------------------------+
// |                       Main Entry Point                       |
// +--------------------------------------------------------------+
int main()
{
	Arena stdHeapLocal = ZEROED;
	InitArenaStdHeap(&stdHeapLocal);
	stdHeap = &stdHeapLocal;
	InitScratchArenasVirtual(Gigabytes(4));
	
	ScratchBegin(loadScratch);
	
	#if BUILD_WITH_RAYLIB
	SetTraceLogCallback(RaylibLogCallback);
	InitWindow(800, 600, PROJECT_READABLE_NAME_STR);
	SetWindowMinSize(400, 200);
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(60);
	#endif //BUILD_WITH_RAYLIB
	
	#if BUILD_INTO_SINGLE_UNIT
	WriteLine_N("Compiled as single unit!");
	#else
	OsDll appDll;
	{
		#if TARGET_IS_WINDOWS
		FilePath dllPath = StrLit(PROJECT_DLL_NAME_STR ".dll");
		#elif TARGET_IS_LINUX
		FilePath dllPath = StrLit("./" PROJECT_DLL_NAME_STR ".so");
		#else
		#error Current TARGET doesn't have an implementation for shared library suffix!
		#endif
		Result loadDllResult = OsLoadDll(dllPath, &appDll);
		if (loadDllResult != Result_Success) { PrintLine_E("Failed to load \"%.*s\": %s", StrPrint(dllPath), GetResultStr(loadDllResult)); }
		Assert(loadDllResult == Result_Success);
	}
	#endif
	
	ScratchEnd(loadScratch);
	
	// +--------------------------------------------------------------+
	// |                        Main Game Loop                        |
	// +--------------------------------------------------------------+
	#if BUILD_WITH_RAYLIB
	while (!WindowShouldClose())
	{
		//Grab all scratch arenas so we can ensure they get reset at the end of each frame
		ScratchBegin(scratch1);
		ScratchBegin1(scratch2, scratch1);
		ScratchBegin2(scratch3, scratch1, scratch2);
		
		int windowWidth = GetRenderWidth();
		int windowHeight = GetRenderHeight();
		BeginDrawing();
		ClearBackground(RAYWHITE);
		Str8 textStr = StrLit(PROJECT_READABLE_NAME_STR);
		int textWidth = MeasureText(textStr.chars, (int)textStr.length);
		DrawText(textStr.chars, windowWidth/2 - textWidth/2, windowHeight/2 - (int)textStr.length/2, (int)textStr.length, LIGHTGRAY);
		
		EndDrawing();
		
		ScratchEnd(scratch1);
		ScratchEnd(scratch2);
		ScratchEnd(scratch3);
	}
	
	#if !BUILD_INTO_SINGLE_UNIT
	CloseWindow(); //NOTE: Apparently this breaks because it conflicts with some dll from Windows when we compile into a single binary
	#endif
	#endif //BUILD_WITH_RAYLIB
	
	return 0;
}

#if BUILD_INTO_SINGLE_UNIT
#include "app/app_main.c"
#endif
