/*
File:   piggen_main.c
Author: Taylor Robbins
Date:   12\31\2024
Description: 
	** Holds the main entry point for piggen.exe which is a tool that scrapes C(++)
	** files, searching for special syntax, in order to generate code and modify
	** the source file(s) to #include the generated code
*/

#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "std/std_includes.h"
#include "std/std_malloc.h"
#include "std/std_memset.h"
#include "std/std_printf.h"
#include "mem/mem_arena.h"

int main()
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
	MyPrintNoLine("Running piggen...");
	
	u8 arenaBuffer1[256] = ZEROED;
	
	Arena stdHeap = ZEROED;
	InitArenaStdHeap(&stdHeap);
	Arena stdAlias = ZEROED;
	InitArenaAlias(&stdAlias, &stdHeap);
	Arena bufferArena = ZEROED;
	InitArenaBuffer(&bufferArena, arenaBuffer1, ArrayCount(arenaBuffer1));
	Arena scratch1 = ZEROED;
	InitArenaStackVirtual(&scratch1, Gigabytes(4));
	Arena scratch2 = ZEROED;
	InitArenaStackVirtual(&scratch2, Gigabytes(4));
	
	u32* allocatedInt1 = AllocMem(&stdHeap, sizeof(u32));
	MyPrint("allocatedInt1: %p", allocatedInt1);
	u32* allocatedInt2 = AllocMem(&stdAlias, sizeof(u32));
	MyPrint("allocatedInt2: %p", allocatedInt2);
	FreeMem(&stdAlias, allocatedInt1, sizeof(u32));
	u32* allocatedInt3 = AllocMem(&stdAlias, sizeof(u32));
	MyPrint("allocatedInt3: %p", allocatedInt3);
	
	u32* allocatedInt4 = AllocMem(&bufferArena, sizeof(u32));
	MyPrint("allocatedInt4: %p", allocatedInt4);
	u32* allocatedInt5 = AllocMem(&bufferArena, sizeof(u32));
	MyPrint("allocatedInt5: %p", allocatedInt5);
	FreeMem(&bufferArena, allocatedInt5, sizeof(u32));
	u32* allocatedInt6 = AllocMem(&bufferArena, sizeof(u32));
	MyPrint("allocatedInt6: %p", allocatedInt6);
	
	// getchar(); //wait for user to press ENTER
	MyPrint("DONE!");
	
	return 0;
}
