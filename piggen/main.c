/*
File:   main.c
Author: Taylor Robbins
Date:   12\31\2024
Description: 
	** Holds the main entry point for piggen.exe which is a tool that scrapes C(++)
	** files, searching for special syntax, in order to generate code and modify
	** the source file(s) to #include the generated code
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "base/base_macros.h"

typedef struct
{
	int a;
	int b;
} MyType_t;

// typedef int bool;
// #define true ((bool)1)
// #define false ((bool)0)

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
		
	unsigned char something[] =
	#embed "piggen/test.txt";
	for (int i = 0; i < ArrayCount(something); i++)
	{
		printf("[%d] = 0x%02X\n", i, something[i]);
	}
	
	MyType_t test = {0};
	typeof(test) test2;
	test2.a = 2;
	test2.b = 3;
	printf("test={ a=%d, b=%d }\n", test.a, test.b);
	printf("test2={ a=%d, b=%d }\n", test2.a, test2.b);
	// printf("test2=%d\n", test2);
	
	bool test3 = false;
	printf("test3=%s\n", test3 ? "true" : "false");
	
	// unsigned _BitInt(5) smallInt = 1;
	// while (smallInt != 0) { printf("smallInt=%u\n", (unsigned int)smallInt); smallInt++; }
	
	// getchar(); //wait for user to press ENTER
	printf("DONE!\n");
	
	return 0;
}
