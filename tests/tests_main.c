/*
File:   tests_main.c
Author: Taylor Robbins
Date:   12\31\2024
Description: 
	** Holds the main entry point for tests.exe, a program that relies on a variety
	** of things inside this repository, thus by building it we can make sure the
	** code inside this repository is in a good state without needing an external
	** project, and when run it does a variety of tests to make sure things are
	** working as intended.
*/

#define MEM_ARENA_DEBUG_NAMES           1
#define VAR_ARRAY_DEBUG_INFO            1
#define VAR_ARRAY_CLEAR_ITEMS_ON_ADD    1
#define VAR_ARRAY_CLEAR_ITEM_BYTE_VALUE 0xCC

#include "build_config.h"

#include "base/base_all.h"
#include "std/std_all.h"
#include "os/os_all.h"
#include "mem/mem_all.h"
#include "struct/struct_all.h"
#include "misc/misc_all.h"

#if BUILD_WITH_SOKOL && BUILD_WITH_RAYLIB
#error SOKOL and RAYLIB are not meant to be enabled at the same time. Use one or the other!
#endif

#if BUILD_WITH_RAYLIB
#include "third_party/raylib/raylib.h"
#endif
#if BUILD_WITH_BOX2D
#include "third_party/box2d/box2d.h"
#endif
#if BUILD_WITH_SOKOL
#define SOKOL_D3D11
#define SOKOL_IMPL
#include "third_party/sokol/sokol_gfx.h"
#include "third_party/sokol/sokol_app.h"
#include "third_party/sokol/sokol_log.h"
#include "third_party/sokol/sokol_glue.h"
// #include "dbgui/dbgui.h"
#endif

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
RandomSeries* mainRandom = nullptr;

// +--------------------------------------------------------------+
// |                      tests Source Files                      |
// +--------------------------------------------------------------+
#include "tests/tests_box2d.c"

// +--------------------------------------------------------------+
// |                           Helpers                            |
// +--------------------------------------------------------------+

#if 0
void PrintArena(Arena* arena)
{
	if (arena->committed > 0)
	{
		MyPrint("%s %llu/%llu (%llu virtual) - %llu allocations", arena->debugName, arena->used, arena->committed, arena->size, arena->allocCount);
	}
	else
	{
		MyPrint("%s %llu/%llu - %llu allocations", arena->debugName, arena->used, arena->size, arena->allocCount);
	}
}

void PrintVarArray(VarArray* array)
{
	#if VAR_ARRAY_DEBUG_INFO
	const char* creationFileName = array->creationFilePath;
	for (u64 cIndex = 0; array->creationFilePath != nullptr && array->creationFilePath[cIndex] != '\0'; cIndex++) { char c = array->creationFilePath[cIndex]; if (c == '/' || c == '\\') { creationFileName = &array->creationFilePath[cIndex+1]; } }
	MyPrint("VarArray %llu/%llu items (itemSize=%llu, items=%p, from %s:%llu %s)", array->length, array->allocLength, array->itemSize, array->items, creationFileName, array->creationLineNumber, array->creationFuncName);
	#else
	MyPrint("VarArray %llu/%llu items (itemSize=%llu, items=%p)", array->length, array->allocLength, array->itemSize, array->items);
	#endif
}
void PrintNumbers(VarArray* array)
{
	MyPrintNoLine("[%llu]{", array->length);
	VarArrayLoop(array, nIndex)
	{
		VarArrayLoopGetValue(u32, num, array, nIndex);
		MyPrintNoLine(" %u", num);
	}
	MyPrint(" }");
}
#endif

// +--------------------------------------------------------------+
// |                             Main                             |
// +--------------------------------------------------------------+
#if BUILD_WITH_SOKOL
int MyMain()
#else
int main()
#endif
{
	MyPrint("Running tests...\n");
	
	// +==============================+
	// |  OS and Compiler Printouts   |
	// +==============================+
	#if 0
	{
		#if COMPILER_IS_MSVC
		MyPrint("Compiled by MSVC");
		#endif
		#if COMPILER_IS_CLANG
		MyPrint("Compiled by Clang");
		#endif
		#if COMPILER_IS_GCC
		MyPrint("Compiled by GCC");
		#endif
		#if TARGET_IS_WINDOWS
		MyPrint("Running on Windows");
		#endif
		#if TARGET_IS_LINUX
		MyPrint("Running on Linux");
		#endif
		#if TARGET_IS_OSX
		MyPrint("Running on OSX");
		#endif
	}
	#endif
	
	// +==============================+
	// |         Basic Arenas         |
	// +==============================+
	#if 1
	Arena stdHeap = ZEROED;
	InitArenaStdHeap(&stdHeap);
	Arena stdAlias = ZEROED;
	InitArenaAlias(&stdAlias, &stdHeap);
	u8 arenaBuffer1[256] = ZEROED;
	Arena bufferArena = ZEROED;
	InitArenaBuffer(&bufferArena, arenaBuffer1, ArrayCount(arenaBuffer1));
	#endif
	
	// +==============================+
	// |     Scratch Arena Tests      |
	// +==============================+
	#if 1
	InitScratchArenasVirtual(Gigabytes(4));
	#else
	Arena scratch1 = ZEROED;
	InitArenaStackVirtual(&scratch1, Gigabytes(4));
	Arena scratch2 = ZEROED;
	InitArenaStackVirtual(&scratch2, Gigabytes(4));
	scratchArenas[0] = &scratch1;
	scratchArenas[1] = &scratch2;
	#endif
	
	// +==============================+
	// |      RandomSeries Tests      |
	// +==============================+
	#if 1
	RandomSeries random;
	InitRandomSeriesDefault(&random);
	SeedRandomSeriesU64(&random, 42); //TODO: Actually seed the random number generator!
	mainRandom = &random;
	#endif
	
	// +==============================+
	// |         Arena Tests          |
	// +==============================+
	#if 0
	{
		u32* allocatedInt1 = AllocMem(&stdHeap, sizeof(u32));
		MyPrint("allocatedInt1: %p", allocatedInt1);
		PrintArena(&stdHeap);
		u32* allocatedInt2 = AllocMem(&stdAlias, sizeof(u32));
		MyPrint("allocatedInt2: %p", allocatedInt2);
		PrintArena(&stdHeap);
		FreeMem(&stdAlias, allocatedInt1, sizeof(u32));
		PrintArena(&stdHeap);
		u32* allocatedInt3 = AllocMem(&stdAlias, sizeof(u32));
		MyPrint("allocatedInt3: %p", allocatedInt3);
		PrintArena(&stdHeap);
		
		u32* allocatedInt4 = AllocMem(&bufferArena, sizeof(u32));
		MyPrint("allocatedInt4: %p", allocatedInt4);
		PrintArena(&bufferArena);
		u32* allocatedInt5 = AllocMem(&bufferArena, sizeof(u32));
		MyPrint("allocatedInt5: %p", allocatedInt5);
		PrintArena(&bufferArena);
		FreeMem(&bufferArena, allocatedInt5, sizeof(u32));
		PrintArena(&bufferArena);
		u32* allocatedInt6 = AllocMem(&bufferArena, sizeof(u32));
		MyPrint("allocatedInt6: %p", allocatedInt6);
		PrintArena(&bufferArena);
		
		u64 mark1 = ArenaGetMark(&scratch1);
		PrintArena(&scratch1);
		u32* num1 = (u32*)AllocMem(&scratch1, sizeof(u32));
		MyPrint("num1 %p", num1);
		PrintArena(&scratch1);
		u32* num2 = (u32*)AllocMem(&scratch1, sizeof(u32));
		MyPrint("num2 %p", num2);
		PrintArena(&scratch1);
		u32* num3 = (u32*)AllocMem(&scratch1, sizeof(u32));
		MyPrint("num3 %p", num3);
		PrintArena(&scratch1);
		FreeMem(&scratch1, num3, sizeof(u32));
		PrintArena(&scratch1);
		ArenaResetToMark(&scratch1, mark1);
		PrintArena(&scratch1);
	}
	#endif
	
	#if TARGET_IS_OSX
	MTLCreateSystemDefaultDevice();
	#endif
	
	// +==============================+
	// |        VarArray Tests        |
	// +==============================+
	#if 0
	{
		VarArray array1;
		InitVarArrayWithInitial(u32, &array1, &stdHeap, 89);
		PrintVarArray(&array1);
		PrintNumbers(&array1);
		
		VarArray array2;
		InitVarArray(u32, &array2, &stdHeap);
		PrintVarArray(&array2);
		PrintNumbers(&array2);
		array2.maxLength = 3;
		
		u32* numPntr1 = VarArrayAdd(u32, &array2);
		*numPntr1 = 7;
		PrintVarArray(&array2);
		PrintNumbers(&array2);
		
		VarArrayAddValue(u32, &array2, 9);
		PrintVarArray(&array2);
		PrintNumbers(&array2);
		
		VarArrayPush(u32, &array2, VarArrayGetValue(u32, &array2, 1));
		PrintVarArray(&array2);
		PrintNumbers(&array2);
		
		u32* numPntr2 = VarArrayAdd(u32, &array2);
		if (numPntr2 != nullptr) { *numPntr2 = 42; }
		PrintVarArray(&array2);
		PrintNumbers(&array2);
		
		VarArray array3;
		VarArrayCopy(&array3, &array2, &stdHeap);
		
		u32 removedNum = VarArrayGetAndRemoveValueAt(u32, &array2, 2);
		MyPrint("Removed array[2] = %u", removedNum);
		PrintVarArray(&array2);
		PrintNumbers(&array2);
		
		VarArrayInsertValue(u32, &array2, 1, VarArrayGetValue(u32, &array2, 0) * VarArrayGetValue(u32, &array2, 1));
		PrintVarArray(&array2);
		PrintNumbers(&array2);
		
		VarArrayRemoveAt(u32, &array2, 1);
		PrintVarArray(&array2);
		PrintNumbers(&array2);
		
		numPntr1 = VarArrayGet(u32, &array2, 0); //refresh our pointer to the value
		VarArrayRemove(u32, &array2, numPntr1);
		PrintVarArray(&array2);
		PrintNumbers(&array2);
		
		FreeVarArray(&array2);
		PrintVarArray(&array2);
		
		PrintVarArray(&array3);
		PrintNumbers(&array3);
	}
	#endif
	
	// +==============================+
	// |          File Tests          |
	// +==============================+
	#if 0
	{
		ScratchBegin(scratch);
		
		FilePath path = AsFilePath(StrNt("../.gitignore"));
		MyPrint("DoesPathHaveTrailingSlash(path) = %s", DoesPathHaveTrailingSlash(path) ? "true" : "false");
		MyPrint("DoesPathHaveExt(path) = %s", DoesPathHaveExt(path) ? "true" : "false");
		MyPrint("\"%.*s\" (path)", StrPrint(path));
		ChangePathSlashesTo(path, '\\');
		MyPrint("\"%.*s\" (ChangePathSlashesTo(path))", StrPrint(path));
		FixPathSlashes(path);
		MyPrint("\"%.*s\" (FixPathSlashes(path))", StrPrint(path));
		MyPrint("\"%.*s\" (GetFileNamePart(path, false))", StrPrint(GetFileNamePart(path, false)));
		MyPrint("\"%.*s\" (GetFileNamePart(path, true))", StrPrint(GetFileNamePart(path, true)));
		MyPrint("\"%.*s\" (GetFileExtPart(path, true, true))", StrPrint(GetFileExtPart(path, true, true)));
		MyPrint("\"%.*s\" (GetFileExtPart(path, false, true))", StrPrint(GetFileExtPart(path, false, true)));
		MyPrint("\"%.*s\" (GetFileExtPart(path, true, false))", StrPrint(GetFileExtPart(path, true, false)));
		MyPrint("\"%.*s\" (GetFileExtPart(path, false, false))", StrPrint(GetFileExtPart(path, false, false)));
		FilePath allocPath = AllocFilePath(scratch, path, true);
		AssertNullTerm(allocPath);
		MyPrint("\"%.*s\" (allocPath)", StrPrint(allocPath));
		FreeFilePathWithNt(scratch, &allocPath);
		FilePath pathFolderPart = GetFileFolderPart(path);
		MyPrint("\"%.*s\" (GetFileFolderPart(path))", StrPrint(pathFolderPart));
		FilePath allocFolderPath = AllocFolderPath(scratch, pathFolderPart, false);
		MyPrint("\"%.*s\" (allocFolderPath)", StrPrint(allocFolderPath));
		FreeFilePath(scratch, &allocFolderPath);
		MyPrint("FullPath will be %llu chars", OsGetFullPathLength(path)); 
		FilePath fullPath = OsGetFullPath(scratch, path);
		MyPrint("\"%.*s\"[%llu] (fullPath)", StrPrint(fullPath), fullPath.length);
		// MyPrint("\"%.*s\" (path)", StrPrint(path));
		MyPrint("\"%.*s\" (GetPathPart(fullPath, -4, true))",  StrPrint(GetPathPart(fullPath, -4, true)));
		MyPrint("\"%.*s\" (GetPathPart(fullPath, -3, true))",  StrPrint(GetPathPart(fullPath, -3, true)));
		MyPrint("\"%.*s\" (GetPathPart(fullPath, -2, true))",  StrPrint(GetPathPart(fullPath, -2, true)));
		MyPrint("\"%.*s\" (GetPathPart(fullPath, -1, true))",  StrPrint(GetPathPart(fullPath, -1, true)));
		MyPrint("\"%.*s\" (GetPathPart(fullPath, 0,  true))",  StrPrint(GetPathPart(fullPath, 0,  true)));
		MyPrint("\"%.*s\" (GetPathPart(fullPath, 1,  true))",  StrPrint(GetPathPart(fullPath, 1,  true)));
		MyPrint("\"%.*s\" (GetPathPart(fullPath, 2,  true))",  StrPrint(GetPathPart(fullPath, 2,  true)));
		MyPrint("\"%.*s\" (GetPathPart(fullPath, 3,  true))",  StrPrint(GetPathPart(fullPath, 3,  true)));
		MyPrint("\"%.*s\" (GetPathPart(fullPath, -4, false))", StrPrint(GetPathPart(fullPath, -4, false)));
		MyPrint("\"%.*s\" (GetPathPart(fullPath, -3, false))", StrPrint(GetPathPart(fullPath, -3, false)));
		MyPrint("\"%.*s\" (GetPathPart(fullPath, -2, false))", StrPrint(GetPathPart(fullPath, -2, false)));
		MyPrint("\"%.*s\" (GetPathPart(fullPath, -1, false))", StrPrint(GetPathPart(fullPath, -1, false)));
		MyPrint("\"%.*s\" (GetPathPart(fullPath, 0,  false))", StrPrint(GetPathPart(fullPath, 0,  false)));
		MyPrint("\"%.*s\" (GetPathPart(fullPath, 1,  false))", StrPrint(GetPathPart(fullPath, 1,  false)));
		MyPrint("\"%.*s\" (GetPathPart(fullPath, 2,  false))", StrPrint(GetPathPart(fullPath, 2,  false)));
		MyPrint("\"%.*s\" (GetPathPart(fullPath, 3,  false))", StrPrint(GetPathPart(fullPath, 3,  false)));
		MyPrint("%llu (CountPathParts(path, true))", CountPathParts(path, true));
		MyPrint("%llu (CountPathParts(path, false))", CountPathParts(path, false));
		MyPrint("%s (OsDoesFileExist(path))", OsDoesFileExist(path) ? "true" : "false");
		MyPrint("%s (OsDoesFolderExist(path))", OsDoesFolderExist(path) ? "true" : "false");
		
		OsFileIter fileIter = OsIterateFiles(scratch, path, true, true);
		u64 fIndex = 0;
		bool isFolder = false;
		FilePath iterFilePath = Str8_Empty;
		while (OsIterFileStepEx(&fileIter, &isFolder, &iterFilePath, scratch, false))
		{
			MyPrint("OsIterFileStep[%llu]: \"%.*s\"%s", fIndex, StrPrint(iterFilePath), isFolder ? " (Folder)" : "");
			fIndex++;
		}
		MyPrint("There are %llu file%s in \"%.*s\"", fIndex, Plural(fIndex, "s"), StrPrint(path));
		
		Str8 fileContents = Str8_Empty;
		if (OsReadFile(path, scratch, true, &fileContents))
		{
			MyPrint("Opened file: %llu chars:", fileContents.length);
			if (fileContents.length < 1024)
			{
				MyPrint("%.*s", StrPrint(fileContents));
			}
			FreeStr8WithNt(scratch, &fileContents);
		}
		
		// bool writeSuccess = OsWriteTextFile(path, StrNt("Hello, we have replaced the file contents with\ngarbage!\n\n:)"));
		// MyPrint("OsWriteTextFile(...): %s", writeSuccess ? "Success" : "Failure");
		
		// OsFile file;
		// if (OsOpenFile(scratch, path, OsOpenFileMode_Append, true, &file))
		// {
		// 	MyPrint("OsOpenFile Succeeded! File is %llu bytes, cursor at %llu", file.fileSize, file.cursorIndex);
		// 	bool writeSuccess = OsWriteToOpenTextFile(&file, StrNt("\n\nHello :)"));
		// 	MyPrint("OsWriteToOpenTextFile(\"\\n\\nHello :)\") %s", writeSuccess ? "Succeeded!" : "Failed!");
		// 	writeSuccess = OsWriteToOpenTextFile(&file, StrNt(""));
		// 	MyPrint("OsWriteToOpenTextFile(\"\") %s", writeSuccess ? "Succeeded!" : "Failed!");
		// 	writeSuccess = OsWriteToOpenTextFile(&file, StrNt("\nTest"));
		// 	MyPrint("OsWriteToOpenTextFile(\"\\nTest\") %s", writeSuccess ? "Succeeded!" : "Failed!");
		// 	OsCloseFile(&file);
		// }
		// else { MyPrint("OsOpenFile Failed!"); }
		
		ScratchEnd(scratch);
	}
	#endif
	
	// +==============================+
	// |      RayLib/Box2D Tests      |
	// +==============================+
	{
		#if BUILD_WITH_BOX2D
		InitBox2DTest();
		#endif
		#if BUILD_WITH_RAYLIB
		{
			InitWindow(800, 600, "Tests (Pigglen)");
			SetWindowMinSize(400, 200);
			SetWindowState(FLAG_WINDOW_RESIZABLE);
			SetTargetFPS(60);
			while (!WindowShouldClose())
			{
				int windowWidth = GetRenderWidth();
				int windowHeight = GetRenderHeight();
				BeginDrawing();
				ClearBackground(RAYWHITE);
				const char* textStr = "Congrats! You created your first window!";
				const int textSize = 20;
				int textWidth = MeasureText(textStr, textSize);
				DrawText(textStr, windowWidth/2 - textWidth/2, windowHeight/2 - textSize/2, textSize, LIGHTGRAY);
				
				#if BUILD_WITH_BOX2D
				UpdateBox2DTest();
				RenderBox2DTest();
				#endif
				
				EndDrawing();
			}
			CloseWindow();
		}
		#endif
	}
	
	MyPrint("All tests completed successfully!");
	return 0;
}

#if BUILD_WITH_SOKOL
sg_pass_action sokolPassAction;

void AppInit(void)
{
	sg_setup(&(sg_desc){
		.environment = sglue_environment(),
		.logger.func = slog_func,
	});
	sokolPassAction = (sg_pass_action){
		.colors[0] = {
			.load_action = SG_LOADACTION_CLEAR,
			.clear_value = { 1.0f, 0.0f, 0.0f, 1.0f }
		}
	};
}

void AppFrame(void)
{
	float newGreen = sokolPassAction.colors[0].clear_value.g + 0.01f;
	sokolPassAction.colors[0].clear_value.g = (newGreen > 1.0f) ? 0.0f : newGreen;
	sg_begin_pass(&(sg_pass){ .action = sokolPassAction, .swapchain = sglue_swapchain() });
	sg_end_pass();
	sg_commit();
}

void AppCleanup(void)
{
	sg_shutdown();
}

void AppEvent(const sapp_event* event)
{
	UNUSED(event);
	
}


sapp_desc sokol_main(int argc, char* argv[])
{
	UNUSED(argc);
	UNUSED(argv);
	MyMain();
	return (sapp_desc){
		.init_cb = AppInit,
		.frame_cb = AppFrame,
		.cleanup_cb = AppCleanup,
		.event_cb = AppEvent,
		.width = 400,
		.height = 300,
		.window_title = "Simple Sokol App!",
		.icon.sokol_default = true,
		.logger.func = slog_func,
	};
}
#endif
