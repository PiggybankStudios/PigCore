/*
File:   main.c
Author: Taylor Robbins
Date:   12\31\2024
Description: 
	** Holds the main entry point for piggen.exe which is a tool that scrapes C(++)
	** files, searching for special syntax, in order to generate code and modify
	** the source file(s) to #include the generated code
*/

#include "base/base_macros.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#if COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 5262) //error C5262: implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases
#endif
#include <stdatomic.h>
#if COMPILER_MSVC
#pragma warning(pop)
#endif

#if COMPILER_MSVC
#include <process.h> //needed for _beginthread/_endthread
#endif

#if OS_WINDOWS
#include <windows.h> //provides Sleep()
#elif OS_LINUX
#include <unistd.h> //provides sleep()
#endif

#define my(type, a, b) _Generic(*(type*)0, \
	int: ((a) - (b)),          \
	float: ((a) + (b)),        \
	bool: ((a) || (b)))

int main()
{
	#if COMPILER_MSVC
	printf("Compiled by MSVC\n");
	#endif
	#if COMPILER_CLANG
	printf("Compiled by Clang\n");
	#endif
	#if COMPILER_GCC
	printf("Compiled by GCC\n");
	#endif
	#if OS_WINDOWS
	printf("Running on Windows\n");
	#endif
	#if OS_LINUX
	printf("Running on Linux\n");
	#endif
	printf("Running piggen...");
	
	int i1 = 3;
	int i2 = 7;
	float f1 = 3.14159f;
	float f2 = 13.7f;
	bool b1 = true;
	bool b2 = false;
	int output1 = my(float, i1, i2);
	float output2 = my(float, f1, f2);
	bool output3 = my(bool, b1, b2);
	printf("output1 = %d\n", output1);
	printf("output2 = %f\n", output2);
	printf("output3 = %s\n", output3 ? "True" : "False");
	
	// getchar(); //wait for user to press ENTER
	printf("DONE!\n");
	
	return 0;
}
