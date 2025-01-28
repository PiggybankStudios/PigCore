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

#include "base/base_compiler_check.h"

#include "base/base_all.h"
#include "std/std_all.h"
#include "os/os_all.h"
#include "mem/mem_all.h"
#include "struct/struct_all.h"
#include "misc/misc_all.h"
#include "gfx/gfx_all.h"

#if BUILD_WITH_SOKOL && BUILD_WITH_RAYLIB
#error SOKOL and RAYLIB are not meant to be enabled at the same time. Use one or the other!
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
#include "tests/tests_vr.c"
#include "tests/tests_wasm_std.c"
#include "tests/tests_auto_profile.c"

// +--------------------------------------------------------------+
// |                           Helpers                            |
// +--------------------------------------------------------------+

#if 0
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
	WriteLine_N("Running tests...\n");
	
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
		#if TARGET_IS_WEB
		WriteLine_N("Running on WEB");
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
	SeedRandomSeriesU64(&random, OsGetCurrentTimestamp(false)); //TODO: Actually seed the random number generator!
	mainRandom = &random;
	#endif
	
	#if BUILD_WITH_RAYLIB
	InitWindow(800, 600, "Tests (PigCore)");
	SetWindowMinSize(400, 200);
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(60);
	#endif
	
	#if BUILD_WITH_OPENVR
	if (!InitVrTests()) { return 1; }
	#endif
	
	// +==============================+
	// |         Arena Tests          |
	// +==============================+
	#if 0
	{
		#if !TARGET_IS_WASM
		u32* allocatedInt1 = AllocMem(&stdHeap, sizeof(u32));
		PrintLine_D("allocatedInt1: %p", allocatedInt1);
		PrintArena(&stdHeap);
		u32* allocatedInt2 = AllocMem(&stdAlias, sizeof(u32));
		PrintLine_D("allocatedInt2: %p", allocatedInt2);
		PrintArena(&stdHeap);
		FreeMem(&stdAlias, allocatedInt1, sizeof(u32));
		PrintArena(&stdHeap);
		u32* allocatedInt3 = AllocMem(&stdAlias, sizeof(u32));
		PrintLine_D("allocatedInt3: %p", allocatedInt3);
		PrintArena(&stdHeap);
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
		
		u64 mark1 = ArenaGetMark(&scratch1);
		PrintArena(&scratch1);
		u32* num1 = (u32*)AllocMem(&scratch1, sizeof(u32));
		PrintLine_D("num1 %p", num1);
		PrintArena(&scratch1);
		u32* num2 = (u32*)AllocMem(&scratch1, sizeof(u32));
		PrintLine_D("num2 %p", num2);
		PrintArena(&scratch1);
		u32* num3 = (u32*)AllocMem(&scratch1, sizeof(u32));
		PrintLine_D("num3 %p", num3);
		PrintArena(&scratch1);
		FreeMem(&scratch1, num3, sizeof(u32));
		PrintArena(&scratch1);
		ArenaResetToMark(&scratch1, mark1);
		PrintArena(&scratch1);
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
		v2 foobarV2 = Div(Add(V2_Half, NewV2(0, 2)), 3.0f);
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
		v3 pos = NewV3(1, 2, 3);
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
		Result loadImageResult = TryParseImageFile(fileContents1, &stdHeap, &zipImageData);
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
	}
	#endif
	
	// +==============================+
	// |          File Tests          |
	// +==============================+
	#if 0
	{
		ScratchBegin(scratch);
		
		Result exePathError = Result_None;
		FilePath exeFilePath = OsGetExecutablePath(scratch, &exePathError);
		if (IsEmptyStr(exeFilePath)) { PrintLine_E("Failed to get exe path: %s", GetResultStr(exePathError)); }
		else { PrintLine_D("Exe Path: \"%.*s\"", StrPrint(exeFilePath)); }
		
		Result workingDirError = Result_None;
		FilePath workingDir = OsGetWorkingDirectory(scratch, &workingDirError);
		if (IsEmptyStr(workingDir)) { PrintLine_E("Failed to get working directory: %s", GetResultStr(workingDirError)); }
		else { PrintLine_D("Working Directory: \"%.*s\"", StrPrint(workingDir)); }
		
		FilePath path = FilePathLit("../.gitignore");
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
		
		ScratchEnd(scratch);
	}
	#endif
	
	#if BUILD_WITH_OPENVR
	while (!WindowShouldClose())
	{
		RunVrTests();
	}
	#endif
	
	#define RAYLIB_3D 1
	#if BUILD_WITH_BOX2D && !RAYLIB_3D
	InitBox2DTest();
	#endif
	
	// +==============================+
	// |      RayLib/Box2D Tests      |
	// +==============================+
	#if BUILD_WITH_RAYLIB && !BUILD_WITH_OPENVR
	{
		#if RAYLIB_3D
		Camera3D camera = ZEROED;
		camera.position = NewVector3(1, 2, -10);
		camera.target = NewVector3(0, 0, 0);
		camera.up = Vector3FromV3(V3_Up);
		camera.fovy = 60; //ToDegrees32(QuarterPi32);
		camera.projection = CAMERA_PERSPECTIVE;
	    DisableCursor();
		#else
		Camera2D camera = ZEROED;
		camera.target = NewVector2((r32)GetRenderWidth()/2, (r32)GetRenderHeight()/2);
		camera.offset = NewVector2((r32)GetRenderWidth()/2, (r32)GetRenderHeight()/2);
		camera.rotation = 0;
		camera.zoom = 1.0f;
		#endif
		
		while (!WindowShouldClose())
		{
			v2i windowSize = NewV2i(GetRenderWidth(), GetRenderHeight());
			
			#if RAYLIB_3D
			if (IsKeyPressed(KEY_ESCAPE)) { CloseWindow(); }
			if (IsKeyDown(KEY_Z)) { camera.target = NewVector3(0.0f, 0.0f, 0.0f); }
			UpdateCamera(&camera, CAMERA_FIRST_PERSON);
			#endif
			
			BeginDrawing();
			ClearBackground(RAYWHITE);
			
			#if RAYLIB_3D
			{
				BeginMode3D(camera);
				
				DrawCube(camera.target, 0.5f, 0.5f, 0.5f, PURPLE);
				
				// Draw grid on the ground
				for (i32 gridIndex = -100; gridIndex <= 100; gridIndex++)
				{
					const r32 gridThickness = 0.01f;
					const r32 gridSize = 400;
					const r32 gridStep = 1.0f;
					DrawCubeV(NewVector3(0 + gridStep*gridIndex, 0, 0), NewVector3(gridThickness, gridThickness, gridSize), ColorFromColor32(MonokaiGray1));
					DrawCubeV(NewVector3(0, 0, 0 + gridStep*gridIndex), NewVector3(gridSize, gridThickness, gridThickness), ColorFromColor32(MonokaiGray1));
				}
				
				DrawCubeV(NewVector3(10, 0, 0), NewVector3(20, 0.1f, 0.1f), ColorFromColor32(MonokaiRed));
				DrawCubeV(NewVector3(0, 10, 0), NewVector3(0.1f, 20, 0.1f), ColorFromColor32(MonokaiGreen));
				DrawCubeV(NewVector3(0, 0, 10), NewVector3(0.1f, 0.1f, 20), ColorFromColor32(MonokaiBlue));
				
				EndMode3D();
			}
			#else //!RAYLIB_3D
			{
				BeginMode2D(camera);
				
				const char* textStr = "Congrats! You created your first window!";
				const int textSize = 20;
				int textWidth = MeasureText(textStr, textSize);
				DrawText(textStr, windowSize.Width/2 - textWidth/2, windowSize.Height/2 - textSize/2, textSize, LIGHTGRAY);
				
				#if BUILD_WITH_BOX2D
				UpdateBox2DTest();
				RenderBox2DTest();
				#endif
				
				#if 0
				r32 textureScale = MinR32((r32)windowSize.Width / (r32)zipTextureSize.Width, (r32)windowSize.Height / (r32)zipTextureSize.Height);
				v2 textureSize = Mul(ToV2Fromi(zipTextureSize), textureScale);
				Vector2 topLeft = (Vector2){
					.x = (r32)windowSize.Width/2 - textureSize.Width/2,
					.y = (r32)windowSize.Height/2 - textureSize.Height/2
				};
				DrawTextureEx(zipTexture, topLeft, 0.0f, textureScale, WHITE);
				#endif
				
				EndMode2D();
			}
			#endif //RAYLIB_3D
			
			EndDrawing();
		}
		CloseWindow();
	}
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
	
	WriteLine_I("All tests completed successfully!");
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
	PrintLine_D("Update %lfms", elapsedMs);
	//TODO: Implement me!
}
#endif

#if USING_CUSTOM_STDLIB
#include "wasm/std/wasm_std_main.c"
#endif
