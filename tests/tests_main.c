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

// #define DEBUG_OUTPUT_PRINT_LEVEL_PREFIX             1
// // #define DEBUG_OUTPUT_LINE_BUFFER_SIZE               14 //512
// #define DEBUG_OUTPUT_ERRORS_ON_LINE_OVERFLOW        1
// #define DEBUG_OUTPUT_TO_WIN32_OUTPUTDEBUGSTRING     1
// #define DEBUG_OUTPUT_ERRORS_ON_FORMAT_FAILURE       1
// #define DEBUG_OUTPUT_CALLBACK_GLOBAL                1
// #define DEBUG_OUTPUT_CALLBACK_ONLY_ON_FINISHED_LINE 0
#define ENABLE_DEBUG_OUTPUT_LEVEL_DEBUG   1
#define ENABLE_DEBUG_OUTPUT_LEVEL_REGULAR 1
#define ENABLE_DEBUG_OUTPUT_LEVEL_INFO    1
#define ENABLE_DEBUG_OUTPUT_LEVEL_NOTIFY  1
#define ENABLE_DEBUG_OUTPUT_LEVEL_OTHER   1
#define ENABLE_DEBUG_OUTPUT_LEVEL_WARNING 1
#define ENABLE_DEBUG_OUTPUT_LEVEL_ERROR   1

#include "build_config.h"

#include "base/base_all.h"
#include "std/std_all.h"
#include "os/os_all.h"
#include "lib/lib_all.h"
#include "mem/mem_all.h"
#include "struct/struct_all.h"
#include "misc/misc_all.h"
#include "input/input_all.h"
#include "file_fmt/file_fmt_all.h"
#include "ui/ui_all.h"
#include "gfx/gfx_all.h"
#include "gfx/gfx_system_global.h"
#include "phys/phys_all.h"
#include "parse/parse_all.h"

#if !TARGET_IS_PLAYDATE
#include "base/base_debug_output_impl.h" //TODO: Get rid of this once the debug output implementation is working on Playdate
#endif

#if BUILD_WITH_RAYLIB
#include "third_party/raylib/raylib.h"
#include "cross/cross_vectors_and_raylib.h"
#include "cross/cross_quaternion_and_raylib.h"
#include "cross/cross_matrices_and_raylib.h"
#include "cross/cross_color_and_raylib.h"
#endif
#if BUILD_WITH_BOX2D
#include "third_party/box2d/box2d.h"
#endif
#if BUILD_WITH_SDL
#if COMPILER_IS_EMSCRIPTEN
#include <SDL/SDL.h>
#else
#include "third_party/SDL/SDL.h"
#endif
#endif

#if BUILD_WITH_PROTOBUF
#include "tests_proto_types.pb-c.h"
#include "tests_proto_types.pb-c.c"
#endif //BUILD_WITH_PROTOBUF

#define SQLITE_ENABLED 0

#if SQLITE_ENABLED
#if DEBUG_BUILD
#define SQLITE_DEBUG 1
#endif
#if COMPILER_IS_MSVC
#pragma warning(push)
#pragma warning(disable: 5262) //implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases
#endif
#if COMPILER_IS_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough" //warning: unannotated fall-through between switch labels
#endif
// #include "third_party/sqlite/sqlite3.h"
#include "third_party/sqlite/sqlite3.c"
#if COMPILER_IS_MSVC
#pragma warning(pop)
#endif
#if COMPILER_IS_CLANG
#pragma clang diagnostic pop
#endif
#endif

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
RandomSeries mainRandomStruct = ZEROED;
RandomSeries* mainRandom = nullptr;
#if !USING_CUSTOM_STDLIB
Arena stdHeapStruct = ZEROED;
Arena* stdHeap = nullptr;
#endif

// +--------------------------------------------------------------+
// |                      tests Source Files                      |
// +--------------------------------------------------------------+
#include "tests/tests_parsing.c"
#include "tests/tests_box2d.c"
#include "tests/tests_sokol.c"
#include "tests/tests_raylib.c"
#include "tests/tests_vr.c"
#include "tests/tests_wasm_std.c"
#include "tests/tests_auto_profile.c"
#include "tests/tests_playdate.c"
#include "tests/tests_sqlite.c"
#include "tests/tests_android.c"

// +--------------------------------------------------------------+
// |                           Helpers                            |
// +--------------------------------------------------------------+

#if 1
void PrintArena(Arena* arena)
{
	if (arena->committed > 0)
	{
		PrintLine_D("%s %llu/%llu (%llu virtual) - %llu allocations", arena->debugName, arena->used, arena->committed, arena->size, arena->allocCount);
	}
	else
	{
		PrintLine_D("%s %llu/%llu - %llu allocations", arena->debugName, arena->used, arena->size, arena->allocCount);
	}
}

void PrintVarArray(VarArray* array)
{
	#if VAR_ARRAY_DEBUG_INFO
	const char* creationFileName = array->creationFilePath;
	for (u64 cIndex = 0; array->creationFilePath != nullptr && array->creationFilePath[cIndex] != '\0'; cIndex++) { char c = array->creationFilePath[cIndex]; if (c == '/' || c == '\\') { creationFileName = &array->creationFilePath[cIndex+1]; } }
	PrintLine_D("VarArray %llu/%llu items (itemSize=%llu, items=%p, from %s:%llu %s)", array->length, array->allocLength, array->itemSize, array->items, creationFileName, array->creationLineNumber, array->creationFuncName);
	#else
	PrintLine_D("VarArray %llu/%llu items (itemSize=%llu, items=%p)", array->length, array->allocLength, array->itemSize, array->items);
	#endif
}
void PrintNumbers(VarArray* array)
{
	Print_D("[%llu]{", array->length);
	VarArrayLoop(array, nIndex)
	{
		VarArrayLoopGetValue(u32, num, array, nIndex);
		Print_D(" %u", num);
	}
	PrintLine_D(" }");
}
#endif

#if 1
// +==============================+
// |   TestsDebugOutputCallback   |
// +==============================+
// void TestsDebugOutputCallback(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel level, bool newLine, const char* message)
DEBUG_OUTPUT_HANDLER_DEF(TestsDebugOutputCallback)
{
	#if 1
	MyPrint("TestsDebugOutputCallback(%s, %s):", GetDbgLevelStr(level), newLine ? "newLine" : "sameLine");
	MyPrint("\tfrom: %s:%u in %s(...)", filePath, lineNumber, funcName);
	MyPrint("\tmessage: \"%s\"", message);
	MyPrint("");
	#else
	MyPrint("TestsDebugOutputCallback:");
	MyPrint("\tfilePath: \"%s\"", filePath);
	MyPrint("\tlineNumber: %u", lineNumber);
	MyPrint("\tfuncName: \"%s\"", funcName);
	MyPrint("\tlevel: DbgLevel_%s", GetDbgLevelStr(level));
	MyPrint("\tnewLine: %s", newLine ? "True" : "False");
	MyPrint("\tmessage: \"%s\"", message);
	MyPrint("");
	#endif
}
#endif

static void PrintRichStr(RichStr richStr)
{
	for (uxx pIndex = 0; pIndex < richStr.numPieces; pIndex++)
	{
		// RichStrPiece* GetRichStrPiece(RichStr* richStr, uxx pieceIndex)
		RichStrPiece* piece = GetRichStrPiece(&richStr, pIndex);
		switch (piece->styleChange.type)
		{
			case RichStrStyleChangeType_None:
			{
				PrintLine_D("\tPiece[%llu]: \"%.*s\"", (u64)pIndex, StrPrint(piece->str));
			} break;
			case RichStrStyleChangeType_FontSize:
			{
				if (piece->styleChange.fontSize == 0.0f)
				{
					PrintLine_D("\tPiece[%llu]: Size=default \"%.*s\"", (u64)pIndex, StrPrint(piece->str));
				}
				else
				{
					PrintLine_D("\tPiece[%llu]: Size=%g \"%.*s\"", (u64)pIndex, piece->styleChange.fontSize, StrPrint(piece->str));
				}
			} break;
			case RichStrStyleChangeType_Alpha:
			{
				if (piece->styleChange.alpha == -1.0f)
				{
					PrintLine_D("\tPiece[%llu]: Alpha=default \"%.*s\"", (u64)pIndex, StrPrint(piece->str));
				}
				else
				{
					PrintLine_D("\tPiece[%llu]: Alpha=%g \"%.*s\"", (u64)pIndex, piece->styleChange.alpha, StrPrint(piece->str));
				}
			} break;
			case RichStrStyleChangeType_FontStyle:
			{
				if (piece->styleChange.enableStyleFlags != 0)
				{
					PrintLine_D("\tPiece[%llu]: Enable %s%s%s \"%.*s\"",
						(u64)pIndex,
						IsFlagSet(piece->styleChange.enableStyleFlags, FontStyleFlag_Bold) ? "Bold" : "",
						IsFlagSet(piece->styleChange.enableStyleFlags, FontStyleFlag_Italic) ? "Italic" : "",
						IsFlagSet(piece->styleChange.enableStyleFlags, FontStyleFlag_Highlighted) ? "Highlight" : "",
						StrPrint(piece->str)
					);
				}
				if (piece->styleChange.disableStyleFlags != 0)
				{
					PrintLine_D("\tPiece[%llu]: Disable %s%s%s \"%.*s\"",
						(u64)pIndex,
						IsFlagSet(piece->styleChange.disableStyleFlags, FontStyleFlag_Bold) ? "Bold" : "",
						IsFlagSet(piece->styleChange.disableStyleFlags, FontStyleFlag_Italic) ? "Italic" : "",
						IsFlagSet(piece->styleChange.disableStyleFlags, FontStyleFlag_Highlighted) ? "Highlight" : "",
						StrPrint(piece->str)
					);
				}
				if (piece->styleChange.defaultStyleFlags != 0)
				{
					PrintLine_D("\tPiece[%llu]: Reset %s%s%s \"%.*s\"",
						(u64)pIndex,
						IsFlagSet(piece->styleChange.defaultStyleFlags, FontStyleFlag_Bold) ? "Bold" : "",
						IsFlagSet(piece->styleChange.defaultStyleFlags, FontStyleFlag_Italic) ? "Italic" : "",
						IsFlagSet(piece->styleChange.defaultStyleFlags, FontStyleFlag_Highlighted) ? "Highlight" : "",
						StrPrint(piece->str)
					);
				}
			} break;
			case RichStrStyleChangeType_Color:
			{
				if (piece->styleChange.color.valueU32 == RICH_STYLE_DEFAULT_COLOR_VALUE)
				{
					PrintLine_D("\tPiece[%llu]: Color=Default \"%.*s\"", (u64)pIndex, StrPrint(piece->str));
				}
				else
				{
					PrintLine_D("\tPiece[%llu]: Color=(r=%u, g=%u, b=%u) \"%.*s\"", (u64)pIndex, piece->styleChange.color.r, piece->styleChange.color.g, piece->styleChange.color.b, StrPrint(piece->str));
				}
			} break;
			case RichStrStyleChangeType_ColorAndAlpha:
			{
				if (piece->styleChange.color.valueU32 == RICH_STYLE_DEFAULT_COLOR_VALUE)
				{
					PrintLine_D("\tPiece[%llu]: ColorAndAlpha=Default \"%.*s\"", (u64)pIndex, StrPrint(piece->str));
				}
				else
				{
					PrintLine_D("\tPiece[%llu]: ColorAndAlpha=(r=%u, g=%u, b=%u, a=%u) \"%.*s\"", (u64)pIndex, piece->styleChange.color.r, piece->styleChange.color.g, piece->styleChange.color.b, piece->styleChange.color.a, StrPrint(piece->str));
				}
			} break;
			default:
			{
				PrintLine_D("\tPiece[%llu]: %s \"%.*s\"", (u64)pIndex, GetRichStrStyleChangeTypeStr(piece->styleChange.type), StrPrint(piece->str));
			} break;
		}
	}
}

// +--------------------------------------------------------------+
// |                             Main                             |
// +--------------------------------------------------------------+
#if !RUN_FUZZER
#if (BUILD_WITH_SOKOL_APP || TARGET_IS_ORCA || TARGET_IS_PLAYDATE || TARGET_IS_ANDROID)
int MyMain(int argc, char* argv[])
#elif (BUILD_WITH_SDL && TARGET_IS_WINDOWS)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char* argv[])
#endif
{
	TracyCZoneN(Zone_Func, "main", true);
	#if (BUILD_WITH_SDL && TARGET_IS_WINDOWS)
	UNUSED(hInstance);
	UNUSED(hPrevInstance);
	UNUSED(lpCmdLine);
	UNUSED(nCmdShow);
	#else
	UNUSED(argc);
	UNUSED(argv);
	#endif
	InitDebugOutputRouter(nullptr);
	WriteLine_N("Running tests...\n");
	
	v2 _v2_zero1 = { .X=0, .Y=0 };
	v2 _v2_zero2 = V2_Zero;
	UNUSED(_v2_zero1);
	UNUSED(_v2_zero2);
	
	// +==============================+
	// |  OS and Compiler Printouts   |
	// +==============================+
	#if 1
	{
		#if COMPILER_IS_MSVC
		WriteLine_N("Compiled by MSVC");
		#endif
		#if COMPILER_IS_CLANG
		WriteLine_N("Compiled by Clang");
		#endif
		#if COMPILER_IS_GCC
		WriteLine_N("Compiled by GCC");
		#endif
		#if TARGET_IS_WINDOWS
		WriteLine_N("Running on Windows");
		#endif
		#if TARGET_IS_LINUX
		WriteLine_N("Running on Linux");
		#endif
		#if TARGET_IS_OSX
		WriteLine_N("Running on OSX");
		#endif
		#if TARGET_IS_ANDROID
		WriteLine_N("Running on Android");
		#endif
		#if TARGET_IS_WEB
		WriteLine_N("Running on WEB");
		#endif
		#if TARGET_IS_PLAYDATE_DEVICE
		WriteLine_N("Running on PLAYDATE_DEVICE");
		#endif
		#if TARGET_IS_PLAYDATE_SIMULATOR
		WriteLine_N("Running on PLAYDATE_SIMULATOR");
		#endif
		#if TARGET_IS_ORCA
		WriteLine_N("Running on ORCA");
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
	InitArenaStdHeap(&stdHeapStruct);
	stdHeap = &stdHeapStruct;
	Arena stdAlias = ZEROED;
	InitArenaAlias(&stdAlias, stdHeap);
	#endif
	u8 arenaBuffer1[256] = ZEROED;
	Arena bufferArena = ZEROED;
	InitArenaBuffer(&bufferArena, arenaBuffer1, ArrayCount(arenaBuffer1));
	
	// +==============================+
	// |     Scratch Arena Tests      |
	// +==============================+
	#if TARGET_IS_PLAYDATE
	InitScratchArenas(Megabytes(1), stdHeap);
	#elif USING_CUSTOM_STDLIB
	InitScratchArenas(Megabytes(256), &wasmMemory);
	#elif TARGET_IS_WASM
	InitScratchArenas(Megabytes(256), stdHeap);
	#else
	InitScratchArenasVirtual(Gigabytes(4));
	#endif
	PrintLine_I("Initialized scratch arenas to %llu bytes", scratchArenasArray[0].size);
	
	// +==============================+
	// |      RandomSeries Tests      |
	// +==============================+
	#if 1
	InitRandomSeriesDefault(&mainRandomStruct);
	u64 randomSeed = OsGetCurrentTimestamp(false);
	PrintLine_D("Random seed: %llu", randomSeed);
	SeedRandomSeriesU64(&mainRandomStruct, randomSeed);
	mainRandom = &mainRandomStruct;
	#endif
	
	#if SQLITE_ENABLED
	TestSqlite();
	#endif
	
	#if BUILD_WITH_RAYLIB
	InitRaylibTests();
	#endif
	
	#if BUILD_WITH_OPENVR
	if (!InitVrTests()) { return 1; }
	#endif
	
	#if TARGET_IS_ANDROID
	DoAndroidTests();
	#endif
	
	// TestParsingFunctions();
	
	// +==============================+
	// |         Arena Tests          |
	// +==============================+
	#if 0
	{
		#if !TARGET_IS_WASM
		u32* allocatedInt1 = AllocMem(stdHeap, sizeof(u32));
		PrintLine_D("allocatedInt1: %p", allocatedInt1);
		PrintArena(stdHeap);
		u32* allocatedInt2 = AllocMem(&stdAlias, sizeof(u32));
		PrintLine_D("allocatedInt2: %p", allocatedInt2);
		PrintArena(stdHeap);
		FreeMem(&stdAlias, allocatedInt1, sizeof(u32));
		PrintArena(stdHeap);
		u32* allocatedInt3 = AllocMem(&stdAlias, sizeof(u32));
		PrintLine_D("allocatedInt3: %p", allocatedInt3);
		PrintArena(stdHeap);
		#endif
		
		u32* allocatedInt4 = AllocMem(&bufferArena, sizeof(u32));
		PrintLine_D("allocatedInt4: %p", allocatedInt4);
		PrintArena(&bufferArena);
		u32* allocatedInt5 = AllocMem(&bufferArena, sizeof(u32));
		PrintLine_D("allocatedInt5: %p", allocatedInt5);
		PrintArena(&bufferArena);
		FreeMem(&bufferArena, allocatedInt5, sizeof(u32));
		PrintArena(&bufferArena);
		u32* allocatedInt6 = AllocMem(&bufferArena, sizeof(u32));
		PrintLine_D("allocatedInt6: %p", allocatedInt6);
		PrintArena(&bufferArena);
		
		ScratchBegin(scratch);
		u64 mark1 = ArenaGetMark(scratch);
		PrintArena(scratch);
		u32* num1 = (u32*)AllocMem(scratch, sizeof(u32));
		PrintLine_D("num1 %p", num1);
		PrintArena(scratch);
		u32* num2 = (u32*)AllocMem(scratch, sizeof(u32));
		PrintLine_D("num2 %p", num2);
		PrintArena(scratch);
		u32* num3 = (u32*)AllocMem(scratch, sizeof(u32));
		PrintLine_D("num3 %p", num3);
		PrintArena(scratch);
		FreeMem(scratch, num3, sizeof(u32));
		PrintArena(scratch);
		ArenaResetToMark(scratch, mark1);
		PrintArena(scratch);
		ScratchEnd(scratch);
	}
	#endif
	
	// +==============================+
	// |      MakeX Macro Tests       |
	// +==============================+
	#if 0
	{
		v2 vert0 = MakeV2(1, 2);
		const v2 vert1 = MakeV2_Const(1, 2);
		
		obb2 test1 = MakeObb2(1, 2, 3, 4, 5);
		obb2 test2 = MakeObb2V(MakeV2(1, 2), MakeV2(3, 4), 5);
		// obb2 test3 = MakeObb2V(MakeV2_Const(1, 2), MakeV2_Const(3, 4), 5);
		obb2 test4 = MakeObb2_Const(1, 2, 3, 4, 5);
		obb2 test5 = MakeObb2V_Const(MakeV2_Const(1, 2), MakeV2_Const(3, 4), 5);
		obb2 test6 = MakeObb2V(vert0, vert1, 5);
		obb2 test7 = MakeObb2V_Const(vert1, vert1, 5);
		obb2 test8 = MakeObb2V_Const(vert1, MakeV2_Const(3, 4), 5);
		const obb2 test9 = MakeObb2V_Const(((v2){.X=1, .Y=2}), ((v2){.X=3, .Y=4}), 5);
		
		mat2 testMat1  = Mat2_Identity;
		mat2 testMat2  = Mat2_Identity_Const;
		mat2 testMat3  = MakeMat2(1, 2, 3, 4);
		mat2 testMat4  = MakeMat2_Const(1, 2, 3, 4);
		mat3 testMat5  = Mat3_Identity;
		mat3 testMat6  = Mat3_Identity_Const;
		mat3 testMat7  = MakeMat3(1, 2, 3, 4, 5, 6, 7, 8, 9);
		mat3 testMat8  = MakeMat3_Const(1, 2, 3, 4, 5, 6, 7, 8, 9);
		mat4 testMat9  = Mat4_Identity;
		mat4 testMat10 = Mat4_Identity_Const;
		mat4 testMat11 = MakeMat4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
		mat4 testMat12 = MakeMat4_Const(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
		
		PrintLine_D("mat4: size=%zu align=%zu", sizeof(mat4), _Alignof(mat4));
		
		PrintLine_D("mat4.Column[0]: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[0]), STRUCT_VAR_SIZE(mat4, Columns[0]));
		PrintLine_D("mat4.Columns[1]: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[1]), STRUCT_VAR_SIZE(mat4, Columns[1]));
		PrintLine_D("mat4.Columns[2]: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[2]), STRUCT_VAR_SIZE(mat4, Columns[2]));
		PrintLine_D("mat4.Columns[3]: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[3]), STRUCT_VAR_SIZE(mat4, Columns[3]));
		
		PrintLine_D("mat4.Columns[0].X: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[0].X), STRUCT_VAR_SIZE(mat4, Columns[0].X));
		PrintLine_D("mat4.Columns[0].Y: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[0].Y), STRUCT_VAR_SIZE(mat4, Columns[0].Y));
		PrintLine_D("mat4.Columns[0].Z: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[0].Z), STRUCT_VAR_SIZE(mat4, Columns[0].Z));
		PrintLine_D("mat4.Columns[0].W: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[0].W), STRUCT_VAR_SIZE(mat4, Columns[0].W));
		
		PrintLine_D("mat4.Columns[1].X: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[1].X), STRUCT_VAR_SIZE(mat4, Columns[1].X));
		PrintLine_D("mat4.Columns[1].Y: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[1].Y), STRUCT_VAR_SIZE(mat4, Columns[1].Y));
		PrintLine_D("mat4.Columns[1].Z: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[1].Z), STRUCT_VAR_SIZE(mat4, Columns[1].Z));
		PrintLine_D("mat4.Columns[1].W: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[1].W), STRUCT_VAR_SIZE(mat4, Columns[1].W));
		
		PrintLine_D("mat4.Columns[2].X: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[2].X), STRUCT_VAR_SIZE(mat4, Columns[2].X));
		PrintLine_D("mat4.Columns[2].Y: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[2].Y), STRUCT_VAR_SIZE(mat4, Columns[2].Y));
		PrintLine_D("mat4.Columns[2].Z: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[2].Z), STRUCT_VAR_SIZE(mat4, Columns[2].Z));
		PrintLine_D("mat4.Columns[2].W: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[2].W), STRUCT_VAR_SIZE(mat4, Columns[2].W));
		
		PrintLine_D("mat4.Columns[3].X: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[3].X), STRUCT_VAR_SIZE(mat4, Columns[3].X));
		PrintLine_D("mat4.Columns[3].Y: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[3].Y), STRUCT_VAR_SIZE(mat4, Columns[3].Y));
		PrintLine_D("mat4.Columns[3].Z: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[3].Z), STRUCT_VAR_SIZE(mat4, Columns[3].Z));
		PrintLine_D("mat4.Columns[3].W: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Columns[3].W), STRUCT_VAR_SIZE(mat4, Columns[3].W));
		
		PrintLine_D("mat4.Elements[0][0]: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Elements[0][0]), STRUCT_VAR_SIZE(mat4, Elements[0][0]));
		PrintLine_D("mat4.Elements[0][1]: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Elements[0][1]), STRUCT_VAR_SIZE(mat4, Elements[0][1]));
		PrintLine_D("mat4.Elements[0][2]: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Elements[0][2]), STRUCT_VAR_SIZE(mat4, Elements[0][2]));
		PrintLine_D("mat4.Elements[0][3]: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Elements[0][3]), STRUCT_VAR_SIZE(mat4, Elements[0][3]));
		
		PrintLine_D("mat4.Elements[0][0]: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Elements[0][0]), STRUCT_VAR_SIZE(mat4, Elements[0][0]));
		PrintLine_D("mat4.Elements[1][0]: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Elements[1][0]), STRUCT_VAR_SIZE(mat4, Elements[1][0]));
		PrintLine_D("mat4.Elements[2][0]: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Elements[2][0]), STRUCT_VAR_SIZE(mat4, Elements[2][0]));
		PrintLine_D("mat4.Elements[3][0]: offset=%zu size=%zu", STRUCT_VAR_OFFSET(mat4, Elements[3][0]), STRUCT_VAR_SIZE(mat4, Elements[3][0]));
	}
	#endif
	
	// +==============================+
	// |          Orca Tests          |
	// +==============================+
	#if 0
	{
		v2 pigV2 = MakeV2(17, 31);
		oc_vec2 orcaVec2 = { .x = 7, .y = 13 };
		v2 toPigV2 = ToV2FromOc(orcaVec2);
		oc_vec2 toOrcaVec2 = ToOcVec2(pigV2);
		PrintLine_D("ToV2FromOc => (%g, %g)", toPigV2.X, toPigV2.Y);
		PrintLine_D("ToOcVec2 => (%g, %g)", toOrcaVec2.x, toOrcaVec2.y);
	}
	#endif
	
	// +==============================+
	// |      Debug Output Tests      |
	// +==============================+
	#if 0
	{
		// #if DEBUG_OUTPUT_CALLBACK_GLOBAL
		// DebugOutputCallback = TestsDebugOutputCallback;
		// #endif
		
		WriteLine_D("Hello Debug!");
		WriteLine_R("Hello Regular!");
		WriteLine_I("Hello Info!");
		WriteLine_N("Hello Notify!");
		WriteLine_O("Hello Other!");
		WriteLine_W("Hello Warning!");
		WriteLine_E("Hello Error!");
		
		u64 testInt = 42;
		PrintLine_D("Print %llu Debug!", testInt++);
		PrintLine_R("Print %llu Regular!", testInt++);
		PrintLine_I("Print %llu Info!", testInt++);
		PrintLine_N("Print %llu Notify!", testInt++);
		PrintLine_O("Print %llu Other!", testInt++);
		PrintLine_W("Print %llu Warning!", testInt++);
		PrintLine_E("Print %llu Error!", testInt++);
		
		for (u64 lIndex = 0; lIndex < DbgLevel_Count; lIndex++)
		{
			PrintLineAt((DbgLevel)lIndex, "OutputAt(%s)", GetDbgLevelStr((DbgLevel)lIndex));
		}
		
		Write_R("Enabled Levels: ");
		Print_D("D[%llu]", testInt++);
		Print_R(", R[%llu]", testInt++);
		Print_I(", I[%llu]", testInt++);
		Print_N(", N[%llu]", testInt++);
		Print_O(", O[%llu]", testInt++);
		Print_W(", W[%llu]", testInt++);
		Print_E(", E[%llu]", testInt++);
		WriteLine_R("");
		
		WriteLine_I("This\nis multiple\nlines!");
	}
	#endif
	
	// +==============================+
	// |         Vector Tests         |
	// +==============================+
	#if 0
	{
		v2 foobarV2 = Div(Add(V2_Half, MakeV2(0, 2)), 3.0f);
		v2 fooV2 = V2_Zero_Const;
		v3 fooV3 = V3_Zero_Const;
		v4 fooV4 = V4_Zero_Const;
		v2i fooV2i = V2i_Zero_Const;
		v3i fooV3i = V3i_Zero_Const;
		v4i fooV4i = V4i_Zero_Const;
		PrintLine_D("sizeof(v2) = %zu", sizeof(v2));
		PrintLine_D("foobar = (%f, %f)", foobarV2.X, foobarV2.Y);
	}
	#endif
	
	// +==============================+
	// |         Matrix Tests         |
	// +==============================+
	#if 0
	{
		v3 pos = MakeV3(1, 2, 3);
		mat2 fooMat2 = NewMat2(1, 2, 3, 4);
		mat3 fooMat3 = MakeScaleXYZMat3(2, 4, 8); //NewMat3(1, 2, 3, 4, 5, 6, 7, 8, 9);
		mat4 fooMat4 = MakeTranslateXYZMat4(1, 1, 1); //NewMat4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
		mat4 barMat4 = NewMat4(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
		mat4 foobarMat4 = MulMat4(fooMat4, barMat4);
		pos = MulMat4AndV3(fooMat4, pos, true);
		// pos = LinearCombineV4Mat4(ToV4From3(pos, 1.0f), fooMat4).XYZ;
		PrintLine_D("sizeof(mat2) = %zu", sizeof(mat2));
		PrintLine_D("sizeof(mat3) = %zu", sizeof(mat3));
		PrintLine_D("sizeof(mat4) = %zu", sizeof(mat4));
		PrintLine_D("pos = (%.0f, %.0f, %.0f)", pos.X, pos.Y, pos.Z);
		PrintLine_D("fooMat2 = (%.0f, %.0f)", fooMat2.Columns[0].X, fooMat2.Columns[1].X);
		PrintLine_D("          (%.0f, %.0f)", fooMat2.Columns[0].Y, fooMat2.Columns[1].Y);
		PrintLine_D("fooMat3 = (%.0f, %.0f, %.0f)", fooMat3.Columns[0].X, fooMat3.Columns[1].X, fooMat3.Columns[2].X);
		PrintLine_D("          (%.0f, %.0f, %.0f)", fooMat3.Columns[0].Y, fooMat3.Columns[1].Y, fooMat3.Columns[2].Y);
		PrintLine_D("          (%.0f, %.0f, %.0f)", fooMat3.Columns[0].Z, fooMat3.Columns[1].Z, fooMat3.Columns[2].Z);
		PrintLine_D("foobarMat4 = (%.0f, %.0f, %.0f, %.0f)", foobarMat4.Columns[0].X, foobarMat4.Columns[1].X, foobarMat4.Columns[2].X, foobarMat4.Columns[3].X);
		PrintLine_D("             (%.0f, %.0f, %.0f, %.0f)", foobarMat4.Columns[0].Y, foobarMat4.Columns[1].Y, foobarMat4.Columns[2].Y, foobarMat4.Columns[3].Y);
		PrintLine_D("             (%.0f, %.0f, %.0f, %.0f)", foobarMat4.Columns[0].Z, foobarMat4.Columns[1].Z, foobarMat4.Columns[2].Z, foobarMat4.Columns[3].Z);
		PrintLine_D("             (%.0f, %.0f, %.0f, %.0f)", foobarMat4.Columns[0].W, foobarMat4.Columns[1].W, foobarMat4.Columns[2].W, foobarMat4.Columns[3].W);
	}
	#endif
	
	// +==============================+
	// |       Quaternion Tests       |
	// +==============================+
	#if 0
	{
		quat foo = Quat_Identity_Const;
		quat bar = ToQuatFromAxis(NormalizeV3(V3_One), HalfPi32);
		mat4 barTransform = ToMat4FromQuat(bar);
		PrintLine_D("foo = (%f, %f, %f, %f)", foo.X, foo.Y, foo.Z, foo.W);
		PrintLine_D("bar = (%f, %f, %f, %f)", bar.X, bar.Y, bar.Z, bar.W);
		PrintLine_D("barTransform = (%.2f, %.2f, %.2f, %.2f)", barTransform.Columns[0].X, barTransform.Columns[1].X, barTransform.Columns[2].X, barTransform.Columns[3].X);
		PrintLine_D("               (%.2f, %.2f, %.2f, %.2f)", barTransform.Columns[0].Y, barTransform.Columns[1].Y, barTransform.Columns[2].Y, barTransform.Columns[3].Y);
		PrintLine_D("               (%.2f, %.2f, %.2f, %.2f)", barTransform.Columns[0].Z, barTransform.Columns[1].Z, barTransform.Columns[2].Z, barTransform.Columns[3].Z);
		PrintLine_D("               (%.2f, %.2f, %.2f, %.2f)", barTransform.Columns[0].W, barTransform.Columns[1].W, barTransform.Columns[2].W, barTransform.Columns[3].W);
	}
	#endif
	
	// +==============================+
	// |       Rectangle Tests        |
	// +==============================+
	#if 0
	{
		rec rec1 = NewRec(1, 2, 10, 20);
		rec rec2 = Rec_Default_Const;
		// rec rec3 = Add(
		// 	Mul(rec1, 0.5f),
		// 	MakeV2(Pi32, -1.5f)
		// );
		rec rec3 = Mul(rec1, 0.5f);
		PrintLine_D("rec1 = (%f, %f, %f, %f)", rec1.X, rec1.Y, rec1.Width, rec1.Height);
		PrintLine_D("rec2 = (%f, %f, %f, %f)", rec2.X, rec2.Y, rec2.Width, rec2.Height);
		PrintLine_D("rec3 = (%f, %f, %f, %f)", rec3.X, rec3.Y, rec3.Width, rec3.Height);
		
		reci reci1 = NewReci(1, 2, 10, 20);
		reci reci2 = Reci_Zero;
		reci reci3 = Reci_Default_Const;
		PrintLine_D("reci1 = (%d, %d, %d, %d)", reci1.X, reci1.Y, reci1.Width, reci1.Height);
		PrintLine_D("reci2 = (%d, %d, %d, %d)", reci2.X, reci2.Y, reci2.Width, reci2.Height);
		PrintLine_D("reci3 = (%d, %d, %d, %d)", reci3.X, reci3.Y, reci3.Width, reci3.Height);
		
		box box1 = NewBox(1, 2, 3, 10, 20, 30);
		box box2 = Box_Default_Const;
		box box3 = Add(Mul(box1, 0.5f), MakeV3(Pi32, -1.5f, 101.2f));
		PrintLine_D("box1 = (%f, %f, %f, %f, %f, %f)", box1.X, box1.Y, box1.Z, box1.Width, box1.Height, box1.Depth);
		PrintLine_D("box2 = (%f, %f, %f, %f, %f, %f)", box2.X, box2.Y, box2.Z, box2.Width, box2.Height, box2.Depth);
		PrintLine_D("box3 = (%f, %f, %f, %f, %f, %f)", box3.X, box3.Y, box3.Z, box3.Width, box3.Height, box3.Depth);
		
		boxi boxi1 = NewBoxi(1, 2, 3, 10, 20, 30);
		boxi boxi2 = Boxi_Zero;
		boxi boxi3 = Boxi_Default_Const;
		PrintLine_D("boxi1 = (%d, %d, %d, %d, %d, %d)", boxi1.X, boxi1.Y, boxi1.Z, boxi1.Width, boxi1.Height, boxi1.Depth);
		PrintLine_D("boxi2 = (%d, %d, %d, %d, %d, %d)", boxi2.X, boxi2.Y, boxi2.Z, boxi2.Width, boxi2.Height, boxi2.Depth);
		PrintLine_D("boxi3 = (%d, %d, %d, %d, %d, %d)", boxi3.X, boxi3.Y, boxi3.Z, boxi3.Width, boxi3.Height, boxi3.Depth);
		
		obb2 obb2_1 = NewObb2(1, 2, 10, 20, QuarterPi32);
		obb2 obb2_2 = Obb2_Zero;
		obb2 obb2_3 = Obb2_Default_Const;
		PrintLine_D("obb2_1 = (%f, %f, %f, %f, %f)", obb2_1.X, obb2_1.Y, obb2_1.Width, obb2_1.Height, ToDegrees32(obb2_1.Rotation));
		PrintLine_D("obb2_2 = (%f, %f, %f, %f, %f)", obb2_2.X, obb2_2.Y, obb2_2.Width, obb2_2.Height, ToDegrees32(obb2_2.Rotation));
		PrintLine_D("obb2_3 = (%f, %f, %f, %f, %f)", obb2_3.X, obb2_3.Y, obb2_3.Width, obb2_3.Height, ToDegrees32(obb2_3.Rotation));
		
		obb3 obb3_1 = NewObb3(1, 2, 3, 10, 20, 30, Quat_Identity);
		obb3 obb3_2 = Obb3_Zero;
		obb3 obb3_3 = Obb3_Default_Const;
		PrintLine_D("obb3_1 = (%f, %f, %f, %f, %f, %f, (%f, %f, %f, %f))", obb3_1.X, obb3_1.Y, obb3_1.Z, obb3_1.Width, obb3_1.Height, obb3_1.Depth, obb3_1.Rotation.X, obb3_1.Rotation.Y, obb3_1.Rotation.Z, obb3_1.Rotation.W);
		PrintLine_D("obb3_2 = (%f, %f, %f, %f, %f, %f, (%f, %f, %f, %f))", obb3_2.X, obb3_2.Y, obb3_2.Z, obb3_2.Width, obb3_2.Height, obb3_2.Depth, obb3_2.Rotation.X, obb3_2.Rotation.Y, obb3_2.Rotation.Z, obb3_2.Rotation.W);
		PrintLine_D("obb3_3 = (%f, %f, %f, %f, %f, %f, (%f, %f, %f, %f))", obb3_3.X, obb3_3.Y, obb3_3.Z, obb3_3.Width, obb3_3.Height, obb3_3.Depth, obb3_3.Rotation.X, obb3_3.Rotation.Y, obb3_3.Rotation.Z, obb3_3.Rotation.W);
		
		rec alignRec = NewRec(15.4f, 13.6f, 10.2f, 3.8f);
		AlignRec(&alignRec);
		PrintLine_D("alignRec = (%f, %f, %f, %f)", alignRec.X, alignRec.Y, alignRec.Width, alignRec.Height);
	}
	#endif
	
	// +==============================+
	// |      StringBuffer Tests      |
	// +==============================+
	#if 0
	{
		// NewStrBuffEx(sb1, 37);
		NewStrBuff(sb1);
		FlagUnset(sb1.flags, StrBuffFlag_AssertOnOverflow);
		SetStrBuff(&sb1, "Hello String Buffer!");
		AppendToStrBuff(&sb1, " (This is a great example...)");
		ReplaceInStrBuff(&sb1, "i", "-");
		ReplaceInStrBuff(&sb1, "g", "ggg");
		RemoveFromStrBuff(&sb1, "ll");
		ReplaceInStrBuff(&sb1, "ff", "f");
		InsertIntoStrBuff(&sb1, 10, "INSERT");
		InsertIntoStrBuff(&sb1, sb1.length, "-END");
		PrintLine_D("sb1.str = [%llu/%llu]\"%.*s\"%s%s", (u64)sb1.length, (u64)sb1.maxLength, StrPrint(sb1.str), IsFlagSet(sb1.flags, StrBuffFlag_Overflowed) ? " (OVERFLOWED!)" : "", sb1.chars[sb1.length] == '\0' ? "" : " (NOT NULL-TERMINATED!)");
		
		NewStrBuffEx(sb2, 37);
		// NewStrBuff(sb2);
		FlagUnset(sb2.flags, StrBuffFlag_AssertOnOverflow);
		AppendPrintToStrBuff(&sb2, "[Line is %llu char%s]", (u64)sb2.length, Plural(sb2.length, "s"));
		AppendPrintToStrBuff(&sb2, "[Line is %llu char%s]", (u64)sb2.length, Plural(sb2.length, "s"));
		AppendPrintToStrBuff(&sb2, "");
		// AppendPrintToStrBuff(&sb2, "[Line is %llu char%s]", (u64)sb2.length, Plural(sb2.length, "s"));
		PrintLine_D("sb2.str = [%llu/%llu]\"%.*s\"%s%s", (u64)sb2.length, (u64)sb2.maxLength, StrPrint(sb2.str), IsFlagSet(sb2.flags, StrBuffFlag_Overflowed) ? " (OVERFLOWED!)" : "", sb2.chars[sb2.length] == '\0' ? "" : " (NOT NULL-TERMINATED!)");
		
		StringBuffer sb3 = NewScratchStrBuff(Kilobytes(64));
		for (uxx wIndex = 0; wIndex < 20; wIndex++)
		{
			if (wIndex > 0) { AppendToStrBuffChar(&sb3, ' '); }
			AppendToStrBuffChar(&sb3, (wIndex > 0) ? 'h' : 'H');
			AppendToStrBuff(&sb3, "ello");
		}
		PrintLine_D("sb3.str = [%llu/%llu]\"%.*s\"%s%s", (u64)sb3.length, (u64)sb3.maxLength, StrPrint(sb3.str), IsFlagSet(sb3.flags, StrBuffFlag_Overflowed) ? " (OVERFLOWED!)" : "", sb3.chars[sb3.length] == '\0' ? "" : " (NOT NULL-TERMINATED!)");
		Str8 slice2 = StrBuffSlice(&sb3, 10, 20);
		PrintLine_D("StrBuffSlice(sb3, 10, 20) = \"%.*s\"", StrPrint(slice2));
		
		StringBuffer sb4 = NewStrBuffFromArena(stdHeap, 100);
		FlagUnset(sb4.flags, StrBuffFlag_AssertOnOverflow);
		for (uxx wIndex = 0; wIndex < 20; wIndex++)
		{
			if (wIndex > 0) { AppendToStrBuffChar(&sb4, ' '); }
			AppendToStrBuffChar(&sb4, (wIndex > 0) ? 'h' : 'H');
			AppendToStrBuff(&sb4, "ello");
		}
		PrintLine_D("sb4.str = [%llu/%llu]\"%.*s\"%s%s", (u64)sb4.length, (u64)sb4.maxLength, StrPrint(sb4.str), IsFlagSet(sb4.flags, StrBuffFlag_Overflowed) ? " (OVERFLOWED!)" : "", sb4.chars[sb4.length] == '\0' ? "" : " (NOT NULL-TERMINATED!)");
	}
	#endif
	
	// +==============================+
	// |        RichStr Tests         |
	// +==============================+
	#if 0
	{
		ScratchBegin(scratch);
		Str8 encodedString = StrLit("Op[rgb=FF00FF]e[highlight]n [alpha=0.5][size=10]\bCo[rgb]lor\b[size]![highlight]");
		// Str8 encodedString = StrLit("\\[");
		PrintLine_D("Original: \"%.*s\"", StrPrint(encodedString));
		
		RichStr richStr = DecodeStrToRichStr(scratch, encodedString);
		PrintLine_D("RichStr is %llu/%llu bytes, %llu piece%s", richStr.fullPiece.str.length, encodedString.length, richStr.numPieces, Plural(richStr.numPieces, "s"));
		PrintRichStr(richStr);
		
		Str8 reEncodedString = EncodeRichStr(scratch, richStr, true, false);
		PrintLine_D("Re-encoded: \"%.*s\"", StrPrint(reEncodedString));
		
		RichStr reDecodedStr = DecodeStrToRichStr(scratch, reEncodedString);
		PrintLine_D("RichStr2 is %llu/%llu bytes, %llu piece%s", reDecodedStr.fullPiece.str.length, reEncodedString.length, reDecodedStr.numPieces, Plural(reDecodedStr.numPieces, "s"));
		PrintRichStr(reDecodedStr);
		
		ScratchEnd(scratch);
	}
	#endif
	
	// +==============================+
	// |        Printing Tests        |
	// +==============================+
	#if 0
	{
		ScratchBegin(scratch);
		char* print1 = PrintInArena(scratch, "Hello %llu World!", GetRandU64Range(mainRandom, 0, 10000));
		PrintLine_D("print1 = \"%s\"", print1);
		Str8 print2 = PrintInArenaStr(scratch, "Hello %llu World! \"%s\"", GetRandU64Range(mainRandom, 0, 10000), print1);
		PrintLine_D("print2 = \"%.*s\"", StrPrint(print2));
		
		Str8 print3 = Str8_Empty;
		u64 randomNum = GetRandU64Range(mainRandom, 0, 32);
		for (u8 pass = 0; pass < 2; pass++)
		{
			u64 charIndex = 0;
			for (u64 nIndex = 0; nIndex < randomNum; nIndex++)
			{
				TwoPassPrint(print3.chars, print3.length, &charIndex, "%s%llu", (nIndex > 0) ? "+" : "", nIndex);
			}
			if (pass == 0) { print3.chars = AllocArray(char, scratch, charIndex+1); print3.length = charIndex; }
			else { Assert(charIndex == print3.length); print3.chars[print3.length] = '\0'; }
		}
		PrintLine_D("print3 = \"%.*s\"", StrPrint(print3));
		
		ScratchEnd(scratch);
	}
	#endif
	
	// +==============================+
	// |        Hashing Tests         |
	// +==============================+
	#if 0
	{
		u8 randomBuffer[32] = { 0xAF, 0x20, 0xCD, 0xC6, 0xDB, 0x9C, 0x59, 0x22, 0xC7, 0xD8, 0xA5, 0x3E, 0x73, 0xD4, 0xB1, 0x1A, 0xDF, 0x90, 0x7D, 0xB6, 0x0B, 0x0C, 0x09, 0x12, 0xF7, 0x48, 0x55, 0x2E, 0xA3, 0x44, 0x61, 0x0B };
		PrintLine_D("Buffer before: %02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X",
			randomBuffer[0],  randomBuffer[1],  randomBuffer[2],  randomBuffer[3],  randomBuffer[4],  randomBuffer[5],  randomBuffer[6],  randomBuffer[7],
			randomBuffer[8],  randomBuffer[9],  randomBuffer[10], randomBuffer[11], randomBuffer[12], randomBuffer[13], randomBuffer[14], randomBuffer[15],
			randomBuffer[16], randomBuffer[17], randomBuffer[18], randomBuffer[19], randomBuffer[20], randomBuffer[21], randomBuffer[22], randomBuffer[23],
			randomBuffer[24], randomBuffer[25], randomBuffer[26], randomBuffer[27], randomBuffer[28], randomBuffer[29], randomBuffer[30], randomBuffer[31]
		);
		Str8 string = StrLit("Hello World!");
		
		u64 fnvHash1 = FnvHashU64(&randomBuffer[0], sizeof(randomBuffer));
		PrintLine_D("fnvHash1 = 0x%016llX", fnvHash1);
		u32 fnvHash2 = FnvHashU32(&randomBuffer[0], sizeof(randomBuffer));
		PrintLine_D("fnvHash2 = 0x%08X", fnvHash2);
		u64 fnvHash3 = FnvHashStrU64(string);
		PrintLine_D("FnvHashStrU64(\"%s\") = 0x%016llX", string.chars, fnvHash3);
		
		#if MEOW_HASH_AVAILABLE
		Hash128 meowHash1 = MeowHash128(&randomBuffer[0], sizeof(randomBuffer));
		PrintLine_D("MeowHash: %08X-%08X-%08X-%08X", meowHash1.parts[0], meowHash1.parts[1], meowHash1.parts[2], meowHash1.parts[3]);
		Hash128 meowHash2 = MeowHashStr128(string);
		PrintLine_D("MeowHash(\"%s\"): %08X-%08X-%08X-%08X", string.chars, meowHash2.parts[0], meowHash2.parts[1], meowHash2.parts[2], meowHash2.parts[3]);
		#endif //MEOW_HASH_AVAILABLE
		
		PrintLine_D("Buffer after:  %02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X",
			randomBuffer[0],  randomBuffer[1],  randomBuffer[2],  randomBuffer[3],  randomBuffer[4],  randomBuffer[5],  randomBuffer[6],  randomBuffer[7],
			randomBuffer[8],  randomBuffer[9],  randomBuffer[10], randomBuffer[11], randomBuffer[12], randomBuffer[13], randomBuffer[14], randomBuffer[15],
			randomBuffer[16], randomBuffer[17], randomBuffer[18], randomBuffer[19], randomBuffer[20], randomBuffer[21], randomBuffer[22], randomBuffer[23],
			randomBuffer[24], randomBuffer[25], randomBuffer[26], randomBuffer[27], randomBuffer[28], randomBuffer[29], randomBuffer[30], randomBuffer[31]
		);
	}
	#endif
	
	// +==============================+
	// |        Unicode Tests         |
	// +==============================+
	#if 0
	{
		ScratchBegin(scratch);
		
		u32 testCodepoints[] = { 0x7F, 0x80, 0xFE, 0xFF, 0xF0, 0x9F, 0xA7, 0xA9, /*kanji*/0x93E1, /*puzzle emoji*/0x1F9E9 };
		for (u64 cIndex = 0; cIndex < ArrayCount(testCodepoints); cIndex++)
		{
			u8 encodedBuffer[UTF8_MAX_CHAR_SIZE];
			u8 numBytes = GetUtf8BytesForCode(testCodepoints[cIndex], &encodedBuffer[0], false);
			Print_D("Codepoint U+%X = [%u]{ ", testCodepoints[cIndex], numBytes);
			for (u8 bIndex = 0; bIndex < numBytes; bIndex++) { Print_D("%s%02X", bIndex > 0 ? ", " : "", encodedBuffer[bIndex]); }
			Write_D("}");
			u32 decodedCodepoint = 0;
			u8 numBytes2 = GetCodepointForUtf8(numBytes, (char*)&encodedBuffer[0], &decodedCodepoint);
			if (numBytes != numBytes2) { PrintLine_D("ERROR: decoded byte count is %u not %u", numBytes2, numBytes); }
			PrintLine_D(" (Decoded=U+%X)", decodedCodepoint);
		}
		char* utf8Str = u8" \u00FF \u93E1 \U0001F9E9 "; //NOTE: Putting the kanji or emoji directly in the string literal doesn't produce the result we want?
		Print_D("utf8Str = %llu bytes: ", MyStrLength64(utf8Str));
		for (u64 bIndex = 0; utf8Str[bIndex] != '\0'; bIndex++)
		{
			Print_D("%s%02X", (bIndex > 0 ? ", " : ""), CharToU8(utf8Str[bIndex]));
		}
		PrintLine_D("");
		
		Str8 haystack = StrLit("This \"is\\\" a simple\" strang"); // This "is\" a simple" strang
		uxx firstChar = FindNextCharInStr(haystack, 0, StrLit(u8"\u00FFpm"));
		PrintLine_D("firstChar = %llu", (u64)firstChar);
		uxx unknownCharIndex = FindNextUnknownCharInStr(haystack, 0, StrLit("T \"\\aehilmnpst")); //should find r or g
		PrintLine_D("unknownCharIndex = %llu", (u64)unknownCharIndex);
		uxx firstChar2 = FindNextCharInStrEx(haystack, 0, StrLit("amg"), true);
		PrintLine_D("firstChar2 = %llu", (u64)firstChar2);
		
		// Str8 escapedString = Str8_Empty;
		// Str8 escapedString = StrLit("\\\\\\");
		Str8 escapedString = StrLit("\\\\-\\\"-\\\'-\\n-\\r-\\t-\\b-\\a-\\g");
		Str8 rawString = StrLit("\\-\"-\'-\n-\r-\t-\b-\a-\\g");
		rawString = UnescapeString(scratch, escapedString);
		PrintLine_D("Raw String: \"%.*s\"", StrPrint(rawString));
		escapedString = EscapeString(scratch, rawString);
		PrintLine_D("Escaped String: \"%.*s\"", StrPrint(escapedString));
		
		ScratchEnd(scratch);
	}
	#endif
	
	// +==============================+
	// |      Zip Archive Tests       |
	// +==============================+
	#if 0
	#if BUILD_WITH_RAYLIB
	v2i zipTextureSize = V2i_Zero;
	Texture2D zipTexture = ZEROED;
	#endif
	{
		ScratchBegin(scratch);
		FilePath zipPath = FilePathLit("images.zip");
		ZipArchive archive = ZEROED;
		Result parseResult = OpenZipArchivePath(scratch, zipPath, &archive);
		Assert(parseResult == Result_Success);
		PrintLine_I("\"%.*s\" has %llu files inside it", StrPrint(zipPath), (u64)archive.numFiles);
		uxx fileIndex = (uxx)GetRandU32Range(mainRandom, 0, (u32)archive.numFiles);
		Str8 fileName1 = GetZipArchiveFilePath(&archive, scratch, fileIndex);
		Slice fileContents1 = ReadZipArchiveBinFileAtIndex(&archive, scratch, fileIndex);
		PrintLine_I("\tFile[%llu] = \"%.*s\" %llu bytes: %02X %02X %02X %02X", (u64)fileIndex, StrPrint(fileName1), (u64)fileContents1.length, fileContents1.bytes[0], fileContents1.bytes[1], fileContents1.bytes[2], fileContents1.bytes[3]);
		#if BUILD_WITH_RAYLIB
		ImageData zipImageData;
		Result loadImageResult = TryParseImageFile(fileContents1, stdHeap, &zipImageData);
		if (loadImageResult != Result_Success) { PrintLine_E("Failed to parse \"%.*s\" texture: %s", StrPrint(fileName1), GetResultStr(loadImageResult)); }
		else
		{
			Image zipImage = ZEROED;
			zipImage.data = zipImageData.pixels;
			zipImage.width = zipImageData.size.Width;
			zipImage.height = zipImageData.size.Height;
			zipImage.mipmaps = 1; //aka no mipmaps
			zipImage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
			zipTexture = LoadTextureFromImage(zipImage);
			zipTextureSize = zipImageData.size;
		}
		#endif //BUILD_WITH_RAYLIB
		CloseZipArchive(&archive);
		ScratchEnd(scratch);
	}
	#endif
	
	// +==============================+
	// |     Simple Parsers Tests     |
	// +==============================+
	#if 0
	{
		Str8 testFileContents = OsReadTextFileScratch(FilePathLit("test.txt"));
		
		LineParser lineParser = NewLineParser(testFileContents);
		Str8 line = Str8_Empty;
		while (LineParserGetLine(&lineParser, &line))
		{
			PrintLine_D("%llu: \"%.*s\"", lineParser.lineIndex, StrPrint(line));
		}
		PrintLine_D("There were %llu lines in the file", lineParser.lineIndex);
		
		TextParser textParser = NewTextParser(testFileContents);
		ParsingToken token = ZEROED;
		u64 tokenIndex = 0;
		while (TextParserGetToken(&textParser, &token))
		{
			switch (token.type)
			{
				case ParsingTokenType_FilePrefix: PrintLine_D("Token[%llu]: FilePrefix \"%.*s\"", tokenIndex, StrPrint(token.value)); break;
				case ParsingTokenType_Directive: PrintLine_D("Token[%llu]: Directive \"%.*s\"", tokenIndex, StrPrint(token.value)); break;
				case ParsingTokenType_KeyValuePair: PrintLine_D("Token[%llu]: KeyValuePair \"%.*s\" = \"%.*s\"", tokenIndex, StrPrint(token.key), StrPrint(token.value)); break;
				default: PrintLine_D("Token[%llu]: %s \"%.*s\"", tokenIndex, GetParsingTokenTypeStr(token.type), StrPrint(token.str)); break;
			}
			tokenIndex++;
		}
	}
	#endif
	
	// +==============================+
	// |         Regex Tests          |
	// +==============================+
	#if 0
	{
		Str8 targetStr = StrLit("Hello World!");
		PrintLine_O("Searching \"%.*s\"", StrPrint(targetStr));
		
		RegexResult result = StrRegexFind(targetStr, StrLit("([^e]+)\\s+([Worl]+)"), true);
		if (result.result == Result_Success)
		{
			PrintLine_D("Regex Matches, %llu capture%s (ending at %llu)", result.numCaptures, Plural(result.numCaptures, "s"), (u64)result.matchEndIndex);
			for (uxx cIndex = 0; cIndex < result.numCaptures; cIndex++)
			{
				PrintLine_D("\tCapture[%llu]: %llu \"%.*s\" (at index %llu)", cIndex, (uxx)result.captures[cIndex].length, StrPrint(result.captures[cIndex]), (u64)result.captureIndices[cIndex]);
			}
		}
		else
		{
			PrintLine_D("Regex did not match: %s", GetResultStr(result.result));
		}
	}
	#endif
	
	#if USING_CUSTOM_STDLIB
	RunWasmStdTests();
	#endif
	
	// #if TARGET_IS_OSX
	// MTLCreateSystemDefaultDevice();
	// #endif
	
	// +==============================+
	// |        VarArray Tests        |
	// +==============================+
	#if 0
	{
		VarArray array1;
		InitVarArrayWithInitial(u32, &array1, stdHeap, 89);
		PrintVarArray(&array1);
		PrintNumbers(&array1);
		
		VarArray array2;
		InitVarArray(u32, &array2, stdHeap);
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
		VarArrayCopy(&array3, &array2, stdHeap);
		
		u32 removedNum = VarArrayGetAndRemoveValueAt(u32, &array2, 2);
		PrintLine_D("Removed array[2] = %u", removedNum);
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
		
		VarArray testArray = ZEROED;
		InitVarArray(char, &testArray, stdHeap);
		void* firstElements = VarArrayAddMulti(char, &testArray, 3);
		PrintLine_D("firstElements: %p", firstElements);
		void* secondElements = VarArrayAddMulti(char, &testArray, 6);
		PrintLine_D("secondElements: %p", secondElements);
		FreeVarArray(&testArray);
		void* allocPntr = MyMallocAligned(64, 1);
		PrintLine_D("Allocated %p with alignment 1", allocPntr);
		MyFreeAligned(allocPntr);
		WriteLine_D("Freed!");
	}
	#endif
	
	// +==============================+
	// |          File Tests          |
	// +==============================+
	#if 0
	{
		WriteLine_O("Running File Tests...");
		ScratchBegin(scratch);
		
		Result exePathError = Result_None;
		FilePath exeFilePath = OsGetExecutablePath(scratch, &exePathError);
		if (IsEmptyStr(exeFilePath)) { PrintLine_E("Failed to get exe path: %s", GetResultStr(exePathError)); }
		else { PrintLine_D("Exe Path: \"%.*s\"", StrPrint(exeFilePath)); }
		
		Result workingDirError = Result_None;
		FilePath workingDir = OsGetWorkingDirectory(scratch, &workingDirError);
		if (IsEmptyStr(workingDir)) { PrintLine_E("Failed to get working directory: %s", GetResultStr(workingDirError)); }
		else { PrintLine_D("Working Directory: \"%.*s\"", StrPrint(workingDir)); }
		
		FilePath path = AllocStr8(scratch, StrLit("../.gitignore"));
		PrintLine_D("DoesPathHaveTrailingSlash(path) = %s", DoesPathHaveTrailingSlash(path) ? "true" : "false");
		PrintLine_D("DoesPathHaveExt(path) = %s", DoesPathHaveExt(path) ? "true" : "false");
		PrintLine_D("\"%.*s\" (path)", StrPrint(path));
		ChangePathSlashesTo(path, '\\');
		PrintLine_D("\"%.*s\" (ChangePathSlashesTo(path))", StrPrint(path));
		FixPathSlashes(path);
		PrintLine_D("\"%.*s\" (FixPathSlashes(path))", StrPrint(path));
		PrintLine_D("\"%.*s\" (GetFileNamePart(path, false))", StrPrint(GetFileNamePart(path, false)));
		PrintLine_D("\"%.*s\" (GetFileNamePart(path, true))", StrPrint(GetFileNamePart(path, true)));
		PrintLine_D("\"%.*s\" (GetFileExtPart(path, true, true))", StrPrint(GetFileExtPart(path, true, true)));
		PrintLine_D("\"%.*s\" (GetFileExtPart(path, false, true))", StrPrint(GetFileExtPart(path, false, true)));
		PrintLine_D("\"%.*s\" (GetFileExtPart(path, true, false))", StrPrint(GetFileExtPart(path, true, false)));
		PrintLine_D("\"%.*s\" (GetFileExtPart(path, false, false))", StrPrint(GetFileExtPart(path, false, false)));
		FilePath allocPath = AllocFilePath(scratch, path, true);
		AssertNullTerm(allocPath);
		PrintLine_D("\"%.*s\" (allocPath)", StrPrint(allocPath));
		FreeFilePathWithNt(scratch, &allocPath);
		FilePath pathFolderPart = GetFileFolderPart(path);
		PrintLine_D("\"%.*s\" (GetFileFolderPart(path))", StrPrint(pathFolderPart));
		FilePath allocFolderPath = AllocFolderPath(scratch, pathFolderPart, false);
		PrintLine_D("\"%.*s\" (allocFolderPath)", StrPrint(allocFolderPath));
		FreeFilePath(scratch, &allocFolderPath);
		PrintLine_D("FullPath will be %llu chars", OsGetFullPathLength(path)); 
		FilePath fullPath = OsGetFullPath(scratch, path);
		PrintLine_D("\"%.*s\"[%llu] (fullPath)", StrPrint(fullPath), fullPath.length);
		// PrintLine_D("\"%.*s\" (path)", StrPrint(path));
		PrintLine_D("\"%.*s\" (GetPathPart(fullPath, -4, true))",  StrPrint(GetPathPart(fullPath, -4, true)));
		PrintLine_D("\"%.*s\" (GetPathPart(fullPath, -3, true))",  StrPrint(GetPathPart(fullPath, -3, true)));
		PrintLine_D("\"%.*s\" (GetPathPart(fullPath, -2, true))",  StrPrint(GetPathPart(fullPath, -2, true)));
		PrintLine_D("\"%.*s\" (GetPathPart(fullPath, -1, true))",  StrPrint(GetPathPart(fullPath, -1, true)));
		PrintLine_D("\"%.*s\" (GetPathPart(fullPath, 0,  true))",  StrPrint(GetPathPart(fullPath, 0,  true)));
		PrintLine_D("\"%.*s\" (GetPathPart(fullPath, 1,  true))",  StrPrint(GetPathPart(fullPath, 1,  true)));
		PrintLine_D("\"%.*s\" (GetPathPart(fullPath, 2,  true))",  StrPrint(GetPathPart(fullPath, 2,  true)));
		PrintLine_D("\"%.*s\" (GetPathPart(fullPath, 3,  true))",  StrPrint(GetPathPart(fullPath, 3,  true)));
		PrintLine_D("\"%.*s\" (GetPathPart(fullPath, -4, false))", StrPrint(GetPathPart(fullPath, -4, false)));
		PrintLine_D("\"%.*s\" (GetPathPart(fullPath, -3, false))", StrPrint(GetPathPart(fullPath, -3, false)));
		PrintLine_D("\"%.*s\" (GetPathPart(fullPath, -2, false))", StrPrint(GetPathPart(fullPath, -2, false)));
		PrintLine_D("\"%.*s\" (GetPathPart(fullPath, -1, false))", StrPrint(GetPathPart(fullPath, -1, false)));
		PrintLine_D("\"%.*s\" (GetPathPart(fullPath, 0,  false))", StrPrint(GetPathPart(fullPath, 0,  false)));
		PrintLine_D("\"%.*s\" (GetPathPart(fullPath, 1,  false))", StrPrint(GetPathPart(fullPath, 1,  false)));
		PrintLine_D("\"%.*s\" (GetPathPart(fullPath, 2,  false))", StrPrint(GetPathPart(fullPath, 2,  false)));
		PrintLine_D("\"%.*s\" (GetPathPart(fullPath, 3,  false))", StrPrint(GetPathPart(fullPath, 3,  false)));
		PrintLine_D("%llu (CountPathParts(path, true))", CountPathParts(path, true));
		PrintLine_D("%llu (CountPathParts(path, false))", CountPathParts(path, false));
		PrintLine_D("%s (OsDoesFileExist(path))", OsDoesFileExist(path) ? "true" : "false");
		PrintLine_D("%s (OsDoesFolderExist(path))", OsDoesFolderExist(path) ? "true" : "false");
		
		OsFileIter fileIter = OsIterateFiles(scratch, path, true, true);
		u64 fIndex = 0;
		bool isFolder = false;
		FilePath iterFilePath = Str8_Empty;
		while (OsIterFileStepEx(&fileIter, &isFolder, &iterFilePath, scratch, false))
		{
			PrintLine_D("OsIterFileStep[%llu]: \"%.*s\"%s", fIndex, StrPrint(iterFilePath), isFolder ? " (Folder)" : "");
			fIndex++;
		}
		PrintLine_D("There are %llu file%s in \"%.*s\"", fIndex, Plural(fIndex, "s"), StrPrint(path));
		
		Str8 fileContents = Str8_Empty;
		if (OsReadFile(path, scratch, true, &fileContents))
		{
			PrintLine_D("Opened file: %llu chars:", fileContents.length);
			if (fileContents.length < 1024)
			{
				PrintLine_D("%.*s", StrPrint(fileContents));
			}
			FreeStr8WithNt(scratch, &fileContents);
		}
		
		// bool writeSuccess = OsWriteTextFile(path, StrLit("Hello, we have replaced the file contents with\ngarbage!\n\n:)"));
		// PrintLine_D("OsWriteTextFile(...): %s", writeSuccess ? "Success" : "Failure");
		
		// OsFile file;
		// if (OsOpenFile(scratch, path, OsOpenFileMode_Append, true, &file))
		// {
		// 	PrintLine_D("OsOpenFile Succeeded! File is %llu bytes, cursor at %llu", file.fileSize, file.cursorIndex);
		// 	bool writeSuccess = OsWriteToOpenTextFile(&file, StrLit("\n\nHello :)"));
		// 	PrintLine_D("OsWriteToOpenTextFile(\"\\n\\nHello :)\") %s", writeSuccess ? "Succeeded!" : "Failed!");
		// 	writeSuccess = OsWriteToOpenTextFile(&file, StrLit(""));
		// 	PrintLine_D("OsWriteToOpenTextFile(\"\") %s", writeSuccess ? "Succeeded!" : "Failed!");
		// 	writeSuccess = OsWriteToOpenTextFile(&file, StrLit("\nTest"));
		// 	PrintLine_D("OsWriteToOpenTextFile(\"\\nTest\") %s", writeSuccess ? "Succeeded!" : "Failed!");
		// 	OsCloseFile(&file);
		// }
		// else { PrintLine_D("OsOpenFile Failed!"); }
		
		OsFile testFile = ZEROED;
		FilePath testPath = FilePathLit("test.txt");
		OsOpenFileMode openMode = OsOpenFileMode_Append;
		bool openSuccess = OsOpenFile(scratch, testPath, openMode, true, &testFile);
		AssertMsg(openSuccess, "OsOpenFile(..., \"test.txt\", OsOpenFileMode_Append, ...) FAILED!");
		PrintLine_D("Successfully opened \"%.*s\" for %s!", StrPrint(testPath), GetOsOpenFileModeStr(openMode));
		// bool OsWriteToOpenTextFile(OsFile* file, Str8 fileContentsPart)
		bool writeSuccess = OsWriteToOpenTextFile(&testFile, StrLit("\n\nHelp! I'm trapped in a computer!"));
		AssertMsg(writeSuccess, "OsWriteToOpenFile failed!");
		OsCloseFile(&testFile);
		
		OsFile tempFile = ZEROED;
		bool openTempSuccess = OsOpenTemporaryFile(scratch, mainRandom, &tempFile);
		AssertMsg(openTempSuccess, "OsOpenTemporaryFile failed!");
		PrintLine_D("Successfully opened temporary file at \"%.*s\"", StrPrint(tempFile.path));
		bool writeTempSuccess = OsWriteToOpenTextFile(&tempFile, StrLit("Help!\n\nI'm\ntrapped\nin\na computer!"));
		AssertMsg(writeTempSuccess, "OsWriteToOpenFile failed!");
		OsCloseFile(&tempFile);
		
		ScratchEnd(scratch);
	}
	#endif
	
	// +==============================+
	// |     FontCharRange Tests      |
	// +==============================+
	#if 0
	{
		ScratchBegin(scratch);
		FontCharRange testRanges[] = {
			NewFontCharRange(8, 8),
			NewFontCharRange(1, 5),
			NewFontCharRange(10, 15),
			NewFontCharRange(13, 14),
			NewFontCharRange(20, 29),
			NewFontCharRange(6, 6),
			NewFontCharRange(24, 25),
			NewFontCharRange(2, 2),
		};
		uxx numTestMergedRanges = 0;
		FontCharRange* testMergedRanges = SortAndMergeFontCharRanges(scratch, ArrayCount(testRanges), &testRanges[0], &numTestMergedRanges);
		PrintLine_D("%llu ranges merged into %llu:", (uxx)ArrayCount(testRanges), numTestMergedRanges);
		for (uxx rIndex = 0; rIndex < numTestMergedRanges; rIndex++)
		{
			PrintLine_D("Range[%llu]: %u-%u", rIndex, testMergedRanges[rIndex].startCodepoint, testMergedRanges[rIndex].endCodepoint);
		}
		ScratchEnd(scratch);
	}
	#endif
	
	// +==============================+
	// |        Protobuf Tests        |
	// +==============================+
	#if BUILD_WITH_PROTOBUF
	{
		ScratchBegin(scratch);
		const u8 oldVersionBytes[] = { 0x09, 0x18, 0x2D, 0x44, 0x54, 0xFB, 0x21, 0x09, 0x40, 0x15, 0xDB, 0x0F, 0x49, 0x40, 0x18, 0x80, 0x08, 0x20, 0xA4, 0x03, 0x28, 0x45, 0x30, 0x6F, 0x38, 0x53, 0x40, 0x89, 0x01, 0x4D, 0xC0, 0xC6, 0x2D, 0x00, 0x51, 0x00, 0xF2, 0x05, 0x2A, 0x01, 0x00, 0x00, 0x00, 0x5D, 0x80, 0x7B, 0xE1, 0xFF, 0x61, 0x00, 0x6C, 0xCA, 0x88, 0xFF, 0xFF, 0xFF, 0xFF, 0x68, 0x01, 0x70, 0x01 };
		Slice oldVersionSlice = NewStr8(ArrayCount(oldVersionBytes), oldVersionBytes);
		
		PrintLine_D("Protobuf-C Version: %s", protobuf_c_version());
		ProtoFileHeader fileHeader = PROTO_FILE_HEADER__INIT;
		fileHeader.v_double = Pi64;
		fileHeader.v_float = Pi32;
		fileHeader.v_int32 = 1024;
		fileHeader.v_int64 = 4202;
		fileHeader.v_uint32 = 69;
		fileHeader.v_uint64 = 111;
		fileHeader.v_sint32 = -42;
		fileHeader.v_sint64 = -69;
		fileHeader.v_fixed32 = Million(3);
		fileHeader.v_fixed64 = Billion(5);
		fileHeader.v_sfixed32 = -(i32)Million(2);
		fileHeader.v_sfixed64 = -(i64)Billion(2);
		fileHeader.v_bool = true;
		fileHeader.v_file_type = PROTO_FILE_HEADER__FILE_TYPE__PNG;
		fileHeader.has_new_field = true;
		fileHeader.new_field = 54321;
		fileHeader.v_string = "Hello from inside!";
		
		#if 1
		Slice packedSlice = ProtobufPackInArena(proto_file_header, scratch, &fileHeader);
		PrintLine_D("fileHeader packed %llu bytes", packedSlice.length);
		#else
		uxx packedSize = (uxx)proto_file_header__get_packed_size(&fileHeader);
		PrintLine_D("fileHeader packed size: %llu bytes", packedSize);
		PbBuffer packedBuffer = NewPbBufferInArena(scratch, packedSize);
		NotNull(packedBuffer.pntr);
		proto_file_header__pack_to_buffer(&fileHeader, &packedBuffer.buffer);
		Assert(packedBuffer.length == packedBuffer.allocLength);
		Slice packedSlice = NewStr8(packedSize, packedBuffer.pntr);
		#endif
		
		// PrintLine_D("Packed fileHeader: %02X %02X %02X %02X ... %02X %02X %02X %02X",
		// 	packedSlice.bytes[0], packedSlice.bytes[1], packedSlice.bytes[2], packedSlice.bytes[3],
		// 	packedSlice.bytes[packedSlice.length-4], packedSlice.bytes[packedSlice.length-3], packedSlice.bytes[packedSlice.length-2], packedSlice.bytes[packedSlice.length-1]
		// );
		// Write_D("{"); for (uxx bIndex = 0; bIndex < packedSlice.length; bIndex++) { Print_D(" 0x%02X,", packedSlice.bytes[bIndex]); } WriteLine_D(" }");
		// packedSlice.bytes[15] = 0x00; //Introduce a corruption to test failed deserialization
		
		// Arena* arenaPntr = stdHeap;
		Arena* arenaPntr = scratch;
		uxx arenaUsageBefore = arenaPntr->used;
		#if 1
		PrintLine_D("Deserializing new %llu bytes...", packedSlice.length);
		ProtoFileHeader* deserHeader = ProtobufUnpackInArena(ProtoFileHeader, proto_file_header, arenaPntr, packedSlice);
		#else
		PrintLine_D("Deserializing old %llu bytes...", oldVersionSlice.length);
		ProtoFileHeader* deserHeader = ProtobufUnpackInArena(ProtoFileHeader, proto_file_header, arenaPntr, oldVersionSlice);
		#endif
		if (deserHeader == nullptr) { WriteLine_E("Failed to deserialize!"); }
		else
		{
			PrintLine_D("Deserialized %llu bytes to %p:", packedSlice.length, deserHeader);
			PrintLine_D("Used %llu bytes from arena", arenaPntr->used - arenaUsageBefore);
			PrintLine_D("v_double = %lf", deserHeader->v_double);
			PrintLine_D("v_float = %f", deserHeader->v_float);
			PrintLine_D("v_int32 = %d", deserHeader->v_int32);
			PrintLine_D("v_int64 = %lld", deserHeader->v_int64);
			PrintLine_D("v_uint32 = %u", deserHeader->v_uint32);
			PrintLine_D("v_uint64 = %llu", deserHeader->v_uint64);
			PrintLine_D("v_sint32 = %d", deserHeader->v_sint32);
			PrintLine_D("v_sint64 = %lld", deserHeader->v_sint64);
			PrintLine_D("v_fixed32 = %u", deserHeader->v_fixed32);
			PrintLine_D("v_fixed64 = %llu", deserHeader->v_fixed64);
			PrintLine_D("v_sfixed32 = %d", deserHeader->v_sfixed32);
			PrintLine_D("v_sfixed64 = %lld", deserHeader->v_sfixed64);
			PrintLine_D("v_bool = %s", deserHeader->v_bool ? "True" : "False");
			PrintLine_D("v_file_type = %d", deserHeader->v_file_type);
			PrintLine_D("new_field = %d", deserHeader->new_field);
			PrintLine_D("v_string = \"%s\"", deserHeader->v_string);
			
			// FlagSet(arenaPntr->flags, ArenaFlag_AllowFreeWithoutSize);
			// ProtobufCAllocator stdAllocator = ProtobufAllocatorFromArena(arenaPntr);
			// proto_file_header__free_unpacked(deserHeader, &stdAllocator);
			// PrintLine_D("Before %llu bytes -> After %llu bytes", arenaUsageBefore, arenaPntr->used);
		}
		
		ScratchEnd(scratch);
	}
	#endif
	
	#if BUILD_WITH_OPENVR
	while (!WindowShouldClose())
	{
		RunVrTests();
	}
	#endif
	
	#if BUILD_WITH_RAYLIB
	RunRaylibTests();
	#endif
	
	// +==============================+
	// |          SDL Tests           |
	// +==============================+
	#if (BUILD_WITH_SDL && COMPILER_IS_EMSCRIPTEN)
	{
		WriteLine_D("Running SDL tests...");
		SDL_Init(SDL_INIT_VIDEO);
		SDL_Surface* screen = SDL_SetVideoMode(256, 256, 32, SDL_SWSURFACE);
		
		if (SDL_MUSTLOCK(screen)) { WriteLine_D("Locking..."); SDL_LockSurface(screen); }
		
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
		TracyCZoneN(Zone_SDL_Init, "SDL_Init", true);
		int initResult = SDL_Init(SDL_INIT_VIDEO);
		TracyCZoneEnd(Zone_SDL_Init);
		Assert(initResult >= 0);
		SDL_Window* window = SDL_CreateWindow("Tests (SDL)", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, 0);
		NotNull(window);
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		NotNull(renderer);
		
		bool windowShouldClose = false;
		while (!windowShouldClose)
		{
			TracyCFrameMark;
			TracyCZoneN(Zone_Update, "Update", true);
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
			TracyCZoneEnd(Zone_Update);
			
			TracyCZoneN(Zone_Present, "Present", true);
			SDL_RenderPresent(renderer);
			TracyCZoneEnd(Zone_Present);
			TracyCZoneN(Zone_Delay, "Delay", true);
			SDL_Delay(16);
			TracyCZoneEnd(Zone_Delay);
		}
	}
	#endif
	
	WriteLine_I("All tests completed successfully!");
	TracyCZoneEnd(Zone_Func);
	return 0;
}
#endif //!RUN_FUZZER

#if TARGET_IS_WEB && !COMPILER_IS_EMSCRIPTEN
WASM_EXPORTED_FUNC(int, ModuleInit, r32 initializeTimestamp)
{
	UNUSED(initializeTimestamp);
	return main(0, nullptr);
}

WASM_EXPORTED_FUNC(void, ModuleUpdate, r64 elapsedMs)
{
	PrintLine_D("Update %lfms", elapsedMs);
	//TODO: Implement me!
}
#endif

#if TARGET_IS_ORCA
oc_vec2 frameSize = {{ 100, 100 }};

oc_surface surface;

unsigned int program;

const char* vshaderSource =
    "attribute vec4 vPosition;\n"
    "uniform mat4 transform;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = transform*vPosition;\n"
    "}\n";

const char* fshaderSource =
    "precision mediump float;\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";

void compile_shader(GLuint shader, const char* source)
{
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);

    int err = glGetError();
    if (err) { WriteLine_E("GL error in compile_shader!"); }
}

ORCA_EXPORT void oc_on_init(void)
{
	char* args[] = { "module.wasm" };
	int mainResult = MyMain(ArrayCount(args), args);
	if (mainResult != 0) { return; }
	
    oc_window_set_title(OC_STR8("tests"));
	
	surface = oc_gles_surface_create();
	oc_gles_surface_make_current(surface);
	
	const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
	PrintLine_I("GLES extensions: \"%s\"\n", extensions);
	
	// int extensionCount = 0;
	// glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);
	// for(int i = 0; i < extensionCount; i++)
	// {
	// 	const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
	// 	PrintLine_I("GLES extension %i: %s\n", i, extension);
	// }
	
	unsigned int vshader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fshader = glCreateShader(GL_FRAGMENT_SHADER);
	program = glCreateProgram();
	
	compile_shader(vshader, vshaderSource);
	compile_shader(fshader, fshaderSource);
	
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);
	glUseProgram(program);
	
	GLfloat vertices[] = {
		-0.866 / 2, -0.5 / 2, 0, 0.866 / 2, -0.5 / 2, 0, 0, 0.5, 0
	};
	
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}

ORCA_EXPORT void oc_on_resize(u32 width, u32 height)
{
    PrintLine_I("frame resize %u, %u", width, height);
    frameSize.x = width;
    frameSize.y = height;
}

ORCA_EXPORT void oc_on_frame_refresh(void)
{
    f32 aspect = frameSize.x / frameSize.y;
    
    oc_gles_surface_make_current(surface);
    
    glClearColor(0, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    static float alpha = 0;
    
    oc_vec2 scaling = oc_surface_contents_scaling(surface);
    
    glViewport(0, 0, frameSize.x * scaling.x, frameSize.y * scaling.y);
    
    GLfloat matrix[] = { cosf(alpha) / aspect, sinf(alpha), 0, 0,
                         -sinf(alpha) / aspect, cosf(alpha), 0, 0,
                         0, 0, 1, 0,
                         0, 0, 0, 1 };
    alpha += 2 * M_PI / 120;
    
    glUniformMatrix4fv(0, 1, false, matrix);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    oc_gles_surface_swap_buffers(surface);
}

#endif //TARGET_IS_ORCA

#if RUN_FUZZER
int LLVMFuzzerTestOneInput(const u8* inputPntr, size_t inputSize)
{
	InitArenaStdHeap(&stdHeapStruct);
	stdHeap = &stdHeapStruct;
	// InitScratchArenasVirtual(Gigabytes(4)); //TODO: We need to free this in order for fuzzing to work properly! If we don't free we end up with too much virtual memory and the OS refuses to give us more!
	uxx arenaUsedBefore = stdHeap->used;
	
	if (inputSize > UINTXX_MAX) { inputSize = UINTXX_MAX; }
	Str8 inputStr = NewStr8((uxx)inputSize, inputPntr);
	
	cTokenizer tokenizer = NewCTokenizer(stdHeap, inputStr);
	cToken* token = NextCToken(&tokenizer);
	while (token != nullptr)
	{
		token = NextCToken(&tokenizer);
	}
	Assert(tokenizer.finished);
	Assert(tokenizer.error != Result_FailedToAllocateMemory);
	Assert(tokenizer.error == Result_Success || tokenizer.error == Result_InvalidUtf8);
	Assert(tokenizer.outputTokenIndex == tokenizer.tokens.length);
	Assert(tokenizer.inputByteIndex == inputStr.length || tokenizer.error == Result_InvalidUtf8);
	
	FreeCTokenizer(&tokenizer);
	
	Assert(stdHeap->used == arenaUsedBefore);
	//TODO: Should we verify scratch arenas are all reset to 0?
	return 0;
	
}
#endif

#if USING_CUSTOM_STDLIB
#include "wasm/std/wasm_std_main.c"
#endif

#if TARGET_IS_PLAYDATE

void HandleSystemEvent(PDSystemEvent event, uint32_t arg)
{
	switch (event)
	{
		case kEventInit:
		{
			char* args[] = { "playdate" };
			int mainResult = MyMain(ArrayCount(args), args);
			if (mainResult != 0) { break; }
			pd->system->setUpdateCallback(PlaydateUpdateCallback, nullptr);
		} break;
		
		default:
		{
			
		} break;
	}
}

MAYBE_START_EXTERN_C
EXPORT_FUNC int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg)
{
	if (event == kEventInit) { pdrealloc = playdate->system->realloc; }
	pd = playdate;
	HandleSystemEvent(event, arg);
	return 0;
}
MAYBE_END_EXTERN_C

DEBUG_OUTPUT_HANDLER_DEF(DebugOutputRouter) { pd->system->logToConsole(message); }
DEBUG_PRINT_HANDLER_DEF(DebugPrintRouter) { pd->system->logToConsole(formatString); }

#endif //TARGET_IS_PLAYDATE

#if TARGET_IS_ANDROID && !BUILD_WITH_SOKOL_APP
void android_main(struct android_app* app)
{
	NotNull(app);
	androidApp = app;
	NotNull(app->activity);
	AndroidNativeActivity = app->activity;
	int mainResult = MyMain(0, nullptr);
	UNUSED(mainResult);
}
#endif //TARGET_IS_ANDROID && !BUILD_WITH_SOKOL_APP
