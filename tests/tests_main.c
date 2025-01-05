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

#include "base/base_all.h"
#include "std/std_all.h"
#include "os/os_all.h"
#include "mem/mem_all.h"
#include "struct/struct_all.h"

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
	MyPrint("VarArray %llu/%llu items (%llu bytes) at %p (from %s:%llu %s)", array->length, array->allocLength, array->itemSize, array->items, array->creationFilePath, array->creationLineNumber, array->creationFuncName);
	#else
	MyPrint("VarArray %llu/%llu items (%llu bytes) at %p", array->length, array->allocLength, array->itemSize, array->items);
	#endif
}

int main()
{
	MyPrint("Running tests...\n");
	
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
	}
	#endif
	
	#if 1
	Arena stdHeap = ZEROED;
	InitArenaStdHeap(&stdHeap);
	Arena stdAlias = ZEROED;
	InitArenaAlias(&stdAlias, &stdHeap);
	u8 arenaBuffer1[256] = ZEROED;
	Arena bufferArena = ZEROED;
	InitArenaBuffer(&bufferArena, arenaBuffer1, ArrayCount(arenaBuffer1));
	Arena scratch1 = ZEROED;
	InitArenaStackVirtual(&scratch1, Gigabytes(4));
	Arena scratch2 = ZEROED;
	InitArenaStackVirtual(&scratch2, Gigabytes(4));
	#endif
	
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
	
	#if 1
	{
		VarArray array1;
		InitVarArray(u32, &array1, &stdHeap);
		PrintVarArray(&array1);
		u32* num1 = VarArrayAdd(u32, &array1);
		PrintVarArray(&array1);
		u32* num2 = VarArrayAdd(u32, &array1);
		PrintVarArray(&array1);
		u32* num3 = VarArrayAdd(u32, &array1);
		PrintVarArray(&array1);
		u32* num4 = VarArrayAdd(u32, &array1);
		PrintVarArray(&array1);
		u32* num5 = VarArrayAdd(u32, &array1);
		PrintVarArray(&array1);
		FreeVarArray(&array1);
		PrintVarArray(&array1);
	}
	#endif
	
	MyPrint("All tests completed successfully!");
	return 0;
}
