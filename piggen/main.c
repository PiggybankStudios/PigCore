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
#include "base/base_macros.h"

int main()
{
	printf("Running piggen...");
	// getchar(); //wait for user to press ENTER
	printf("DONE!\n");
	return 0;
}
