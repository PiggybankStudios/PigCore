/*
File:   extract_define_main.c
Author: Taylor Robbins
Date:   05\10\2025
Description: 
	** This file compiles to extract_define.exe which is a small
	** tool that opens a C\C++ header file and searches for a particular
	** #define name, if found it prints out the RHS value of that #define
	** to stdout, where a batch script can store it in an environment variable
	** NOTE: This tool cannot safely include build_config.h or any
	**       PigCore file that relies on build_config.h defines.
	**       That basically means this needs to be a completely standalone C program
	** NOTE: This tool only works well with simple value #defines,
	**       not function-like macros
	** NOTE: This tool does not parse multi-line comment syntax and
	**       will report #defines inside them
	** Usage extract_define.exe [file_path] [DEFINE_NAME]
*/

#include "tools/tools_shared.h"

#if BUILDING_ON_WINDOWS
#define TOOL_EXE_NAME "extract_define.exe"
#else
#define TOOL_EXE_NAME "extract_define"
#endif

static inline void PrintUsage()
{
	WriteLine_E("Usage: " TOOL_EXE_NAME " [file_path] [DEFINE_NAME]");
}

int main(int argc, char* argv[])
{
	assert(argc >= 1); //first argument is the executable name
	if (argc != 3)
	{
		PrintLine_E("Expected 2 arguments, not %d!", argc-1);
		PrintUsage();
		return 1;
	}
	
	const char* filePathNt = argv[1];
	const char* defineNameNt = argv[2];
	Str8 filePath = NewStr8Nt(filePathNt);
	Str8 defineName = NewStr8Nt(defineNameNt);
	
	Str8 fileContents = ReadEntireFile(filePath);
	
	Str8 defineValue = ZEROED;
	if (TryExtractDefineFrom(fileContents, defineName, &defineValue))
	{
		PrintLine("%.*s", defineValue.length, defineValue.chars);
		free(fileContents.chars);
		return 0;
	}
	else
	{
		PrintLine_E("Couldn't find #define %s inside \"%s\"", defineNameNt, filePathNt);
		free(fileContents.chars);
		return 4;
	}
}

