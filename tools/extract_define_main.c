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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
// #include <float.h>
// #include <limits.h>
// #include <stddef.h>
// #include <stdarg.h>
#include <assert.h>
// #include <string.h>
// #include <math.h>

#ifdef __cplusplus
#define LANGUAGE_IS_C   0
#define LANGUAGE_IS_CPP 1
#else
#define LANGUAGE_IS_C   1
#define LANGUAGE_IS_CPP 0
#endif

#if LANGUAGE_IS_C
#define nullptr ((void*)0)
#define ZEROED {0}
#else
#define ZEROED {}
#endif

typedef unsigned int uxx;

typedef struct Str8 Str8;
struct Str8
{
	uxx length;
	union { char* chars; uint8_t* bytes; void* pntr; };
};

#define StrLitLength(stringLiteral) ((sizeof(stringLiteral) / sizeof((stringLiteral)[0])) - sizeof((stringLiteral)[0]))
#define CheckStrLit(stringLiteral) ("" stringLiteral "")
#define StrLit(stringLiteral) NewStr8(StrLitLength(CheckStrLit(stringLiteral)), (stringLiteral))

static inline Str8 NewStr8(uxx length, const void* pntr)
{
	Str8 result;
	result.length = length;
	result.pntr = (void*)pntr; //throw away const qualifier
	return result;
}

static inline void PrintUsage()
{
	fprintf(stderr, "Usage: extract_define.exe [file_path] [DEFINE_NAME]\n");
}

static inline bool StrExactEquals(Str8 left, Str8 right)
{
	if (left.length != right.length) { return false; }
	if (left.length == 0) { return true; }
	assert(left.chars != nullptr);
	assert(right.chars != nullptr);
	return (memcmp(left.chars, right.chars, left.length) == 0);
}
static inline Str8 StrSlice(Str8 target, uxx startIndex, uxx endIndex)
{
	assert(startIndex <= target.length);
	assert(endIndex <= target.length);
	assert(startIndex <= endIndex);
	return NewStr8(endIndex - startIndex, target.chars + startIndex);
}
static inline Str8 StrSliceFrom(Str8 target, uxx startIndex)
{
	return StrSlice(target, startIndex, target.length);
}
static inline bool IsCharWhitespace(char character)
{
	if (character == ' ') { return true; }
	else if (character == '\t') { return true; }
	else { return false; }
}
static inline bool IsCharIdentifier(char character, bool isFirstChar)
{
	if (character == '_') { return true; }
	if (character >= 'A' && character <= 'Z') { return true; }
	if (character >= 'a' && character <= 'z') { return true; }
	if (!isFirstChar && character >= '0' && character <= '9') { return true; }
	return false;
}
static inline Str8 TrimWhitespace(Str8 target)
{
	assert(target.length == 0 || target.chars != nullptr);
	Str8 result = target;
	while (result.length > 0 && IsCharWhitespace(result.chars[0])) { result.chars++; result.length--; }
	while (result.length > 0 && IsCharWhitespace(result.chars[result.length-1])) { result.length--; }
	return result;
}
static inline uxx FindNextWhitespace(Str8 targetStr, uxx startIndex)
{
	assert(startIndex <= targetStr.length);
	for (uxx bIndex = startIndex; bIndex < targetStr.length; bIndex++)
	{
		if (IsCharWhitespace(targetStr.chars[bIndex])) { return bIndex; }
	}
	return targetStr.length;
}
static inline uxx FindNextNonIdentifierChar(Str8 targetStr, uxx startIndex)
{
	assert(startIndex <= targetStr.length);
	for (uxx bIndex = startIndex; bIndex < targetStr.length; bIndex++)
	{
		if (!IsCharIdentifier(targetStr.chars[bIndex], (bIndex == startIndex))) { return bIndex; }
	}
	return targetStr.length;
}

static bool CheckInputLine(Str8 targetDefineName, Str8 line, Str8* valueStrOut)
{
	line = TrimWhitespace(line);
	uxx firstWhitespaceIndex = FindNextWhitespace(line, 0);
	if (firstWhitespaceIndex < line.length)
	{
		Str8 firstToken = StrSlice(line, 0, firstWhitespaceIndex);
		if (StrExactEquals(firstToken, StrLit("#define")))
		{
			line = TrimWhitespace(StrSliceFrom(line, firstWhitespaceIndex+1));
			uxx identifierEndIndex = FindNextNonIdentifierChar(line, 0);
			if (identifierEndIndex < line.length)
			{
				Str8 nameStr = StrSlice(line, 0, identifierEndIndex);
				if (StrExactEquals(nameStr, targetDefineName))
				{
					Str8 valueStr = TrimWhitespace(StrSliceFrom(line, identifierEndIndex+1));
					if (valueStrOut != nullptr) { *valueStrOut = valueStr; }
					return true;
				}
			}
		}
	}
	return false;
}

static inline bool TryReadFile(const char* filePath, Str8* contentsOut)
{
	//NOTE: We open the file in binary mode because otherwise the result from jumping to SEEK_END to
	//      check the file size does not match the result of fread because the new-lines get converted
	//      in the fread NOT in the result from ftell
	FILE* fileHandle = fopen(filePath, "rb");
	if (fileHandle == nullptr)
	{
		fprintf(stderr, "Couldn't open file at \"%s\"!\n", filePath);
		return false;
	}
	
	int seekResult1 = fseek(fileHandle, 0, SEEK_END); assert(seekResult1 == 0);
	long fileSize = ftell(fileHandle); assert(fileSize >= 0); assert(fileSize <= INT_MAX);
	int seekResult2 = fseek(fileHandle, 0, SEEK_SET); assert(seekResult2 == 0);
	
	contentsOut->length = (uxx)fileSize;
	contentsOut->chars = (char*)malloc(fileSize+1);
	assert(contentsOut->chars != nullptr);
	
	int readResult = fread(
		contentsOut->chars,
		1,
		fileSize,
		fileHandle
	);
	contentsOut->chars[fileSize] = '\0';
	if (readResult != (int)fileSize)
	{
		fprintf(stderr, "Failed to read all %d byte%s from file! Only read %d byte%s\n",
			(int)fileSize, (fileSize == 1 ? "" : "s"),
			readResult, (readResult == 1 ? "" : "s")
		);
		free(contentsOut->chars);
		fclose(fileHandle);
		return false;
	}
	
	fclose(fileHandle);
	return true;
}

int main(int argc, char* argv[])
{
	assert(argc >= 1); //first argument is the executable name
	if (argc != 3)
	{
		fprintf(stderr, "Expected 2 arguments, not %d!\n", argc-1);
		PrintUsage();
		return 1;
	}
	
	const char* filePath = argv[1];
	const char* defineName = argv[2];
	Str8 defineNameStr = NewStr8((uxx)strlen(defineName), defineName);
	
	Str8 fileContents = ZEROED;
	if (!TryReadFile(filePath, &fileContents)) { return 2; }
	
	uxx lineStartIndex = 0;
	for (uxx byteIndex = 0; byteIndex < fileContents.length; byteIndex++)
	{
		char character = fileContents.chars[byteIndex];
		char nextCharacter = fileContents.chars[byteIndex+1]; //requires null-terminator we added above
		if (character == '\n' ||
			(character == '\r' && nextCharacter == '\n') ||
			(character == '\n' && nextCharacter == '\r'))
		{
			bool isTwoCharacterNewLine =
				(character == '\r' && nextCharacter == '\n') ||
				(character == '\n' && nextCharacter == '\r');
			
			Str8 lineStr = NewStr8(byteIndex - lineStartIndex, &fileContents.chars[lineStartIndex]);
			
			Str8 defineValue = ZEROED;
			if (CheckInputLine(defineNameStr, lineStr, &defineValue))
			{
				printf("%.*s\n", (int)defineValue.length, defineValue.chars);
				break;
			}
			
			if (isTwoCharacterNewLine) { byteIndex++; }
			lineStartIndex = byteIndex+1;
		}
	}
	
	free(fileContents.chars);
	return 0;
}

