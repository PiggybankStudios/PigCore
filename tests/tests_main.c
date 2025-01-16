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

#include "base/base_compiler_check.h"

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
#if TARGET_IS_WINDOWS
#define SOKOL_D3D11
#elif TARGET_IS_LINUX
#define SOKOL_GLCORE
#elif TARGET_IS_WEB
#define SOKOL_WGPU
#endif
#define SOKOL_IMPL
#if TARGET_IS_WASM
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough" //warning: unannotated fall-through between switch labels [-Wimplicit-fallthrough]
#endif
#include "third_party/sokol/sokol_gfx.h"
#if TARGET_IS_WASM
#pragma clang diagnostic pop
#endif
#if TARGET_IS_LINUX
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers" //warning: missing field 'revents' initializer [-Wmissing-field-initializers]
#endif
#include "third_party/sokol/sokol_app.h"
#if TARGET_IS_LINUX
#pragma clang diagnostic pop
#endif
#include "third_party/sokol/sokol_log.h"
#include "third_party/sokol/sokol_glue.h"
#endif

#if BUILD_WITH_SDL
#if COMPILER_IS_EMSCRIPTEN
#include <SDL/SDL.h>
#else
#include "third_party/SDL/SDL.h"
#endif
#endif

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
RandomSeries* mainRandom = nullptr;

// +--------------------------------------------------------------+
// |                      tests Source Files                      |
// +--------------------------------------------------------------+
#include "tests/tests_box2d.c"
#include "tests/tests_wasm_std.c"

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
#elif (BUILD_WITH_SDL && TARGET_IS_WINDOWS)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char* argv[])
#endif
{
	#if BUILD_WITH_SOKOL
	#elif (BUILD_WITH_SDL && TARGET_IS_WINDOWS)
	UNUSED(hInstance);
	UNUSED(hPrevInstance);
	UNUSED(lpCmdLine);
	UNUSED(nCmdShow);
	#else
	UNUSED(argc);
	UNUSED(argv);
	#endif
	MyPrint("Running tests...\n");
	
	// +==============================+
	// |  OS and Compiler Printouts   |
	// +==============================+
	#if 1
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
		#if TARGET_IS_WEB
		MyPrint("Running on WEB");
		#endif
		#if TARGET_IS_ORCA
		MyPrint("Running on ORCA");
		#endif
	}
	#endif
	
	// +==============================+
	// |         Basic Arenas         |
	// +==============================+
	#if USING_CUSTOM_STDLIB
	Arena wasmMemory = ZEROED;
	InitArenaStackWasm(&wasmMemory);
	FlagSet(wasmMemory.flags, ArenaFlag_AssertOnFailedAlloc);
	#else
	Arena stdHeap = ZEROED;
	InitArenaStdHeap(&stdHeap);
	Arena stdAlias = ZEROED;
	InitArenaAlias(&stdAlias, &stdHeap);
	#endif
	u8 arenaBuffer1[256] = ZEROED;
	Arena bufferArena = ZEROED;
	InitArenaBuffer(&bufferArena, arenaBuffer1, ArrayCount(arenaBuffer1));
	
	// +==============================+
	// |     Scratch Arena Tests      |
	// +==============================+
	#if USING_CUSTOM_STDLIB
	InitScratchArenas(Megabytes(256), &wasmMemory);
	#elif TARGET_IS_WASM
	InitScratchArenas(Megabytes(256), &stdHeap);
	#else
	InitScratchArenasVirtual(Gigabytes(4));
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
		#if !TARGET_IS_WASM
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
		#endif
		
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
	
	// +==============================+
	// |         Vector Tests         |
	// +==============================+
	#if 1
	{
		v2 foobarV2 = Div(Add(V2_Half, NewV2(0, 2)), 3.0f);
		v2 fooV2 = V2_Zero_Const;
		v3 fooV3 = V3_Zero_Const;
		v4 fooV4 = V4_Zero_Const;
		v2i fooV2i = V2i_Zero_Const;
		v3i fooV3i = V3i_Zero_Const;
		v4i fooV4i = V4i_Zero_Const;
		MyPrint("sizeof(v2) = %zu", sizeof(v2));
		MyPrint("foobar = (%f, %f)", foobarV2.X, foobarV2.Y);
	}
	#endif
	
	// +==============================+
	// |         Matrix Tests         |
	// +==============================+
	#if 1
	{
		v3 pos = NewV3(1, 2, 3);
		mat2 fooMat2 = NewMat2(1, 2, 3, 4);
		mat3 fooMat3 = MakeScaleXYZMat3(2, 4, 8); //NewMat3(1, 2, 3, 4, 5, 6, 7, 8, 9);
		mat4 fooMat4 = MakeTranslateXYZMat4(1, 1, 1); //NewMat4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
		mat4 barMat4 = NewMat4(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
		mat4 foobarMat4 = MulMat4(fooMat4, barMat4);
		pos = MulMat4AndV3(fooMat4, pos, true);
		// pos = LinearCombineV4Mat4(ToV4From3(pos, 1.0f), fooMat4).XYZ;
		MyPrint("sizeof(mat2) = %zu", sizeof(mat2));
		MyPrint("sizeof(mat3) = %zu", sizeof(mat3));
		MyPrint("sizeof(mat4) = %zu", sizeof(mat4));
		MyPrint("pos = (%.0f, %.0f, %.0f)", pos.X, pos.Y, pos.Z);
		MyPrint("fooMat2 = (%.0f, %.0f)", fooMat2.Columns[0].X, fooMat2.Columns[1].X);
		MyPrint("          (%.0f, %.0f)", fooMat2.Columns[0].Y, fooMat2.Columns[1].Y);
		MyPrint("fooMat3 = (%.0f, %.0f, %.0f)", fooMat3.Columns[0].X, fooMat3.Columns[1].X, fooMat3.Columns[2].X);
		MyPrint("          (%.0f, %.0f, %.0f)", fooMat3.Columns[0].Y, fooMat3.Columns[1].Y, fooMat3.Columns[2].Y);
		MyPrint("          (%.0f, %.0f, %.0f)", fooMat3.Columns[0].Z, fooMat3.Columns[1].Z, fooMat3.Columns[2].Z);
		MyPrint("foobarMat4 = (%.0f, %.0f, %.0f, %.0f)", foobarMat4.Columns[0].X, foobarMat4.Columns[1].X, foobarMat4.Columns[2].X, foobarMat4.Columns[3].X);
		MyPrint("             (%.0f, %.0f, %.0f, %.0f)", foobarMat4.Columns[0].Y, foobarMat4.Columns[1].Y, foobarMat4.Columns[2].Y, foobarMat4.Columns[3].Y);
		MyPrint("             (%.0f, %.0f, %.0f, %.0f)", foobarMat4.Columns[0].Z, foobarMat4.Columns[1].Z, foobarMat4.Columns[2].Z, foobarMat4.Columns[3].Z);
		MyPrint("             (%.0f, %.0f, %.0f, %.0f)", foobarMat4.Columns[0].W, foobarMat4.Columns[1].W, foobarMat4.Columns[2].W, foobarMat4.Columns[3].W);
	}
	#endif
	
	// +==============================+
	// |       Quaternion Tests       |
	// +==============================+
	#if 1
	{
		quat foo = Quat_Identity_Const;
		quat bar = ToQuatFromAxis(NormalizeV3(V3_One), HalfPi32);
		mat4 barTransform = ToMat4FromQuat(bar);
		MyPrint("foo = (%f, %f, %f, %f)", foo.X, foo.Y, foo.Z, foo.W);
		MyPrint("bar = (%f, %f, %f, %f)", bar.X, bar.Y, bar.Z, bar.W);
		MyPrint("barTransform = (%.2f, %.2f, %.2f, %.2f)", barTransform.Columns[0].X, barTransform.Columns[1].X, barTransform.Columns[2].X, barTransform.Columns[3].X);
		MyPrint("               (%.2f, %.2f, %.2f, %.2f)", barTransform.Columns[0].Y, barTransform.Columns[1].Y, barTransform.Columns[2].Y, barTransform.Columns[3].Y);
		MyPrint("               (%.2f, %.2f, %.2f, %.2f)", barTransform.Columns[0].Z, barTransform.Columns[1].Z, barTransform.Columns[2].Z, barTransform.Columns[3].Z);
		MyPrint("               (%.2f, %.2f, %.2f, %.2f)", barTransform.Columns[0].W, barTransform.Columns[1].W, barTransform.Columns[2].W, barTransform.Columns[3].W);
	}
	#endif
	
	#if USING_CUSTOM_STDLIB
	RunWasmStdTests();
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
	
	// +==============================+
	// |          SDL Tests           |
	// +==============================+
	#if (BUILD_WITH_SDL && COMPILER_IS_EMSCRIPTEN)
	{
		MyPrint("Running SDL tests...");
		SDL_Init(SDL_INIT_VIDEO);
		SDL_Surface* screen = SDL_SetVideoMode(256, 256, 32, SDL_SWSURFACE);
		
		if (SDL_MUSTLOCK(screen)) { MyPrint("Locking..."); SDL_LockSurface(screen); }
		
		for (int yOffset = 0; yOffset < 256; yOffset++)
		{
			for (int xOffset = 0; xOffset < 256; xOffset++)
			{
				int alpha = (xOffset + yOffset) % 255;
				*((u32*)screen->pixels + yOffset*256 + xOffset) = SDL_MapRGBA(screen->format, xOffset, yOffset, 255-yOffset, alpha);
			}
		}
		
		if (SDL_MUSTLOCK(screen)) { SDL_UnlockSurface(screen); }
		SDL_Flip(screen);
		
		SDL_Quit();
	}
	#elif BUILD_WITH_SDL
	{
		int initResult = SDL_Init(SDL_INIT_VIDEO);
		Assert(initResult >= 0);
		SDL_Window* window = SDL_CreateWindow("Tests (SDL)", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, 0);
		NotNull(window);
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		NotNull(renderer);
		
		bool windowShouldClose = false;
		while (!windowShouldClose)
		{
			SDL_SetRenderDrawColor(renderer, 96, 128, 255, 255);
			SDL_RenderClear(renderer);
			
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
					case SDL_QUIT: windowShouldClose = true; break;
					default: break;
				}
			}
			
			SDL_RenderPresent(renderer);
			SDL_Delay(16);
		}
	}
	#endif
	
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

#if TARGET_IS_WASM && !COMPILER_IS_EMSCRIPTEN
WASM_EXPORTED_FUNC(int, ModuleInit, r32 initializeTimestamp)
{
	UNUSED(initializeTimestamp);
	return main(0, nullptr);
}

WASM_EXPORTED_FUNC(void, ModuleUpdate, r64 elapsedMs)
{
	MyPrint("Update %lfms", elapsedMs);
	//TODO: Implement me!
}
#endif

#if USING_CUSTOM_STDLIB
#include "wasm/std/wasm_std_main.c"
#endif
