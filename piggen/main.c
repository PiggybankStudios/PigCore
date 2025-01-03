/*
File:   main.c
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

int main()
{
	#if COMPILER_IS_MSVC
	printf("Compiled by MSVC\n");
	#endif
	#if COMPILER_IS_CLANG
	printf("Compiled by Clang\n");
	#endif
	#if COMPILER_IS_GCC
	printf("Compiled by GCC\n");
	#endif
	#if TARGET_IS_WINDOWS
	printf("Running on Windows\n");
	#endif
	#if TARGET_IS_LINUX
	printf("Running on Linux\n");
	#endif
	#if TARGET_IS_OSX
	printf("Running on OSX\n");
	#endif
	printf("Running piggen...");
	
	// getchar(); //wait for user to press ENTER
	printf("DONE!\n");
	
	return 0;
}
