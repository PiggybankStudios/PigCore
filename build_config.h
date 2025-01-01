/*
File:   build_config.h
Author: Taylor Robbins
Date:   12\31\2024
Description:
	** This file contains a bunch of options that control the build.bat.
	** This file is both a C header file that can be #included from a .c file,
	** and it is also scraped by the build.bat to extract values to change the work it performs.
	** Because it is scraped, and not parsed with the full C language spec, we must
	** be careful to keep this file very simple and not introduce any syntax that
	** would confuse the scraper when it's searching for values
*/

#ifndef _BUILD_CONFIG_H
#define _BUILD_CONFIG_H

// Controls whether we are making a build that we want to run with a Debugger.
// This often sacrifices runtime speed or code size for extra debug information.
// Debug builds often take less time to compile as well.
#define DEBUG_BUILD   0

// Compiles piggen/main.c
#define BUILD_PIGGEN   1
// Generates code for all projects using piggen.exe (you can turn this off if you're not making changes to generated code and you've already generated it once)
#define RUN_PIGGEN     1

// Compiles tests/main.c
#define BUILD_TESTS   1
// Runs the result of compiling tests/main.c, aka the tests.exe
#define RUN_TESTS     1

// Rather than compiling the project(s) it will simply output the
// result of the preprocessor's pass over the code to the build folder
#define DUMP_PREPROCESSOR 0

#endif //  _BUILD_CONFIG_H
