/*
File:   piggen_main.c
Author: Taylor Robbins
Date:   12\31\2024
Description: 
	** Holds the main entry point for piggen.exe which is a tool that scrapes C(++)
	** files, searching for special syntax, in order to generate code and modify
	** the source file(s) to #include the generated code
*/

#include "build_config.h"
#ifdef BUILD_WITH_RAYLIB
#undef BUILD_WITH_RAYLIB
#endif
#define BUILD_WITH_RAYLIB 0
#ifdef BUILD_WITH_SOKOL_GFX
#undef BUILD_WITH_SOKOL_GFX
#endif
#define BUILD_WITH_SOKOL_GFX 0
#ifdef BUILD_WITH_SOKOL_APP
#undef BUILD_WITH_SOKOL_APP
#endif
#define BUILD_WITH_SOKOL_APP 0

#include "base/base_all.h"
#include "std/std_all.h"
#include "os/os_all.h"
#include "mem/mem_all.h"
#include "struct/struct_all.h"

int main()
{
	MyPrintNoLine("Running piggen...");
	// getchar(); //wait for user to press ENTER
	MyPrint("DONE!");
	return 0;
}
