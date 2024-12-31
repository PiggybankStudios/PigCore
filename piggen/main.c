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

int main()
{
	printf("Hello World! This is piggen.exe\n");
	getchar(); //wait for user to press ENTER
	return 0;
}
