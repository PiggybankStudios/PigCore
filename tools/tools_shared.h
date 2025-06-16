/*
File:   tools_shared.h
Author: Taylor Robbins
Date:   06\16\2025
Description: 
	** Holds functions\types\etc. that are shared amongst all the tool scripts
	** This are basically duplicates of a small section of PigCore that need to
	** be separate and stable so that these tools don't depend on the code they
	** are helping compile.
*/

#ifndef _TOOLS_SHARED_H
#define _TOOLS_SHARED_H

// +--------------------------------------------------------------+
// |                     Std Library Includes                     |
// +--------------------------------------------------------------+
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

// +--------------------------------------------------------------+
// |                           Defines                            |
// +--------------------------------------------------------------+
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

// +--------------------------------------------------------------+
// |                            Types                             |
// +--------------------------------------------------------------+
typedef unsigned int uxx;

typedef struct Str8 Str8;
struct Str8
{
	uxx length;
	union { char* chars; uint8_t* bytes; void* pntr; };
};

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define StrLitLength(stringLiteral) ((sizeof(stringLiteral) / sizeof((stringLiteral)[0])) - sizeof((stringLiteral)[0]))
#define CheckStrLit(stringLiteral) ("" stringLiteral "")
#define StrLit(stringLiteral) NewStr8(StrLitLength(CheckStrLit(stringLiteral)), (stringLiteral))

#define WriteLine(messageStr) printf(messageStr "\n")
#define WriteLine_E(messageStr) fprintf(stderr, messageStr "\n")
#define PrintLine(formatStr, ...) printf(formatStr "\n", ##__VA_ARGS__)
#define PrintLine_E(formatStr, ...) fprintf(stderr, formatStr "\n", ##__VA_ARGS__)

// +--------------------------------------------------------------+
// |                        Str Functions                         |
// +--------------------------------------------------------------+
static inline Str8 NewStr8(uxx length, const void* pntr)
{
	Str8 result;
	result.length = length;
	result.pntr = (void*)pntr; //throw away const qualifier
	return result;
}
static inline Str8 NewStr8Nt(const void* nullTermPntr)
{
	Str8 result;
	result.length = (uxx)strlen(nullTermPntr);
	result.pntr = (void*)nullTermPntr; //throw away const qualifier
	return result;
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

static inline bool TryParseBoolArg(Str8 boolStr, bool* valueOut)
{
	if (StrExactEquals(boolStr, StrLit("1"))) { *valueOut = true; return true; }
	if (StrExactEquals(boolStr, StrLit("0"))) { *valueOut = false; return true; }
	if (StrExactEquals(boolStr, StrLit("true"))) { *valueOut = true; return true; }
	if (StrExactEquals(boolStr, StrLit("false"))) { *valueOut = false; return true; }
	return false;
}

static inline Str8 CopyStr8(Str8 strToCopy, bool addNullTerm)
{
	Str8 result;
	result.length = strToCopy.length;
	result.pntr = malloc(result.length + (addNullTerm ? 1 : 0));
	memcpy(result.chars, strToCopy.chars, strToCopy.length);
	if (addNullTerm) { result.chars[result.length] = '\0'; }
	return result;
}

// +--------------------------------------------------------------+
// |                        File Functions                        |
// +--------------------------------------------------------------+
static inline bool TryReadFile(Str8 filePath, Str8* contentsOut)
{
	char* filePathNt = (char*)malloc(filePath.length+1);
	memcpy(filePathNt, filePath.chars, filePath.length);
	filePathNt[filePath.length] = '\0';
	
	//NOTE: We open the file in binary mode because otherwise the result from jumping to SEEK_END to
	//      check the file size does not match the result of fread because the new-lines get converted
	//      in the fread NOT in the result from ftell
	FILE* fileHandle = fopen(filePathNt, "rb");
	free(filePathNt);
	if (fileHandle == nullptr)
	{
		fprintf(stderr, "Couldn't open file at \"%.*s\"!\n", filePath.length, filePath.chars);
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

// +--------------------------------------------------------------+
// |                     Extract Define Logic                     |
// +--------------------------------------------------------------+
static bool IsHeaderLineDefine(Str8 targetDefineName, Str8 line, Str8* valueStrOut)
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

bool TryExtractDefineFrom(Str8 headerFileContents, Str8 defineName, Str8* valueOut)
{
	uxx lineStartIndex = 0;
	for (uxx byteIndex = 0; byteIndex < headerFileContents.length; byteIndex++)
	{
		char character = headerFileContents.chars[byteIndex];
		char nextCharacter = headerFileContents.chars[byteIndex+1]; //requires null-terminator we added above
		if (character == '\n' ||
			(character == '\r' && nextCharacter == '\n') ||
			(character == '\n' && nextCharacter == '\r'))
		{
			bool isTwoCharacterNewLine =
				(character == '\r' && nextCharacter == '\n') ||
				(character == '\n' && nextCharacter == '\r');
			
			Str8 lineStr = NewStr8(byteIndex - lineStartIndex, &headerFileContents.chars[lineStartIndex]);
			
			Str8 defineValue = ZEROED;
			if (IsHeaderLineDefine(defineName, lineStr, &defineValue))
			{
				if (valueOut != nullptr) { *valueOut = defineValue; }
				return true;
			}
			
			if (isTwoCharacterNewLine) { byteIndex++; }
			lineStartIndex = byteIndex+1;
		}
	}
	return false;
}

#endif //  _TOOLS_SHARED_H
