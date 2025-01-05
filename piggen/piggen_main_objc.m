/*
File:   piggen_main.m
Author: Taylor Robbins
Date:   01/04/2025
Description:
	** When compiling on OSX with Clang we use the .m file extension
	** to indicate we want Clang to compile in Objective-C mode.
	** Objective-C is required to interact with most libraries on OSX (like Metal)
*/

#include <Metal/Metal.h>

#include "piggen/piggen_main.c"
