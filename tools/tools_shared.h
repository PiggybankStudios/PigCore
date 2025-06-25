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
#include <limits.h>
// #include <stddef.h>
// #include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
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

#if defined(_WIN32)
#define BUILDING_ON_WINDOWS 1
#else
#define BUILDING_ON_WINDOWS 0
#endif

#if defined(__linux__) || defined(__unix__)
#define BUILDING_ON_LINUX 1
#else
#define BUILDING_ON_LINUX 0
#endif

#ifdef __APPLE__
#define BUILDING_ON_OSX 1
#else
#define BUILDING_ON_OSX 0
#endif

#if BUILDING_ON_WINDOWS
#include <windows.h>
#endif

#if BUILDING_ON_WINDOWS
#define PATH_SEP_CHAR '\\'
#else
#define PATH_SEP_CHAR '/'
#endif

#if !BUILDING_ON_WINDOWS
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

#if BUILDING_ON_WINDOWS
#define FOLDER_PERMISSIONS 0
#else
#define FOLDER_PERMISSIONS S_IRWXU|S_IRWXG|S_IRWXO
#endif

// +--------------------------------------------------------------+
// |                            Types                             |
// +--------------------------------------------------------------+
typedef unsigned int uxx;

#define UINTXX_MAX UINT_MAX

typedef struct Str8 Str8;
struct Str8
{
	uxx length;
	union { char* chars; uint8_t* bytes; void* pntr; };
};

typedef struct FileIter FileIter;
struct FileIter
{
	bool finished;
	Str8 folderPathNt;
	uxx index;
	uxx nextIndex;
	
	#if BUILDING_ON_WINDOWS
	Str8 folderPathWithWildcard;
	WIN32_FIND_DATAA findData;
	HANDLE handle;
	#elif BUILDING_ON_LINUX
	DIR* dirHandle;
	#endif
};

typedef struct LineParser LineParser;
struct LineParser
{
	uxx byteIndex;
	uxx lineBeginByteIndex;
	uxx lineIndex; //This is not zero based! It's more like a line number you'd see in the gutter of a text editor! It also contains the total number of lines in the input after the iteration has finished
	Str8 inputStr;
	//TODO: Should we add support for Streams again?
};

#define RECURSIVE_DIR_WALK_CALLBACK_DEF(functionName) bool functionName(Str8 path, bool isFolder, void* contextPntr)
typedef RECURSIVE_DIR_WALK_CALLBACK_DEF(RecursiveDirWalkCallback_f);

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define StrLitLength(stringLiteral) ((sizeof(stringLiteral) / sizeof((stringLiteral)[0])) - sizeof((stringLiteral)[0]))
#define CheckStrLit(stringLiteral) ("" stringLiteral "")
#define StrLit(stringLiteral) NewStr8(StrLitLength(CheckStrLit(stringLiteral)), (stringLiteral))
#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

#define WriteLine(messageStr) printf(messageStr "\n")
#define WriteLine_E(messageStr) fprintf(stderr, messageStr "\n")
#define PrintLine(formatStr, ...) printf(formatStr "\n", ##__VA_ARGS__)
#define PrintLine_E(formatStr, ...) fprintf(stderr, formatStr "\n", ##__VA_ARGS__)

#define IS_SLASH(character) ((character) == '\\' || (character) == '/')

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
static inline bool StrExactContains(Str8 haystack, Str8 needle)
{
	assert(needle.length > 0);
	if (haystack.length < needle.length) { return false; }
	for (uxx bIndex = 0; bIndex <= haystack.length - needle.length; bIndex++)
	{
		if (StrExactEquals(StrSlice(haystack, bIndex, bIndex+needle.length), needle)) { return true; }
	}
	return false;
}
static inline bool StrExactStartsWith(Str8 target, Str8 prefix)
{
	assert(prefix.length > 0);
	if (target.length < prefix.length) { return false; }
	return StrExactEquals(StrSlice(target, 0, prefix.length), prefix);
}
static inline bool StrExactEndsWith(Str8 target, Str8 suffix)
{
	assert(suffix.length > 0);
	if (target.length < suffix.length) { return false; }
	return StrExactEquals(StrSlice(target, target.length - suffix.length, target.length), suffix);
}
static inline Str8 GetDirectoryPart(Str8 fullPath, bool includeTrailingSlash)
{
	uxx lastSlashIndex = fullPath.length;
	for (uxx cIndex = 0; cIndex < fullPath.length; cIndex++)
	{
		char character = fullPath.chars[cIndex];
		if (IS_SLASH(character)) { lastSlashIndex = cIndex; }
	}
	if (lastSlashIndex < fullPath.length) { return StrSlice(fullPath, 0, lastSlashIndex + (includeTrailingSlash ? 1 : 0)); }
	else { return fullPath; }
}
static inline Str8 GetFileNamePart(Str8 fullPath, bool includeExtension)
{
	uxx lastSlashIndex = fullPath.length;
	for (uxx cIndex = 0; cIndex < fullPath.length; cIndex++)
	{
		char character = fullPath.chars[cIndex];
		if (IS_SLASH(character)) { lastSlashIndex = cIndex; }
	}
	if (lastSlashIndex < fullPath.length) { return StrSliceFrom(fullPath, lastSlashIndex+1); }
	else { return fullPath; }
}
static inline Str8 GetFileExtPart(Str8 fullPath)
{
	uxx periodIndex = fullPath.length;
	for (uxx cIndex = 0; cIndex < fullPath.length; cIndex++)
	{
		char character = fullPath.chars[cIndex];
		if (IS_SLASH(character)) { periodIndex = fullPath.length; } //reset periodIndex
		else if (character == '.') { periodIndex = cIndex; }
	}
	if (periodIndex < fullPath.length) { return StrSliceFrom(fullPath, periodIndex); }
	else { return StrSliceFrom(fullPath, fullPath.length); }
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

static inline Str8 EscapeString(Str8 unescapedString, bool addNullTerm)
{
	Str8 result = ZEROED;
	for (int pass = 0; pass < 2; pass++)
	{
		uxx byteIndex = 0;
		for (uxx cIndex = 0; cIndex < unescapedString.length; cIndex++)
		{
			char character = unescapedString.chars[cIndex];
			if (character == '\"' || character == '\\' || character == '\'')
			{
				if (result.chars != nullptr)
				{
					result.chars[byteIndex+0] = '\\';
					result.chars[byteIndex+1] = character;
				}
				byteIndex += 2;
			}
			else if (character == '\n' || character == '\r' || character == '\t')
			{
				if (result.chars != nullptr)
				{
					result.chars[byteIndex+0] = '\\';
					if (character == '\n') { result.chars[byteIndex+1] = 'n'; }
					if (character == '\r') { result.chars[byteIndex+1] = 'r'; }
					if (character == '\t') { result.chars[byteIndex+1] = 't'; }
				}
				byteIndex += 2;
			}
			else
			{
				if (result.chars != nullptr) { result.chars[byteIndex] = character; }
				byteIndex++;
			}
		}
		
		if (pass == 0)
		{
			result.length = byteIndex;
			result.pntr = malloc(result.length + (addNullTerm ? 1 : 0));
		}
		else if (addNullTerm) { result.chars[result.length] = '\0'; }
	}
	return result;
}

static inline Str8 JoinStrings2(Str8 left, Str8 right, bool addNullTerm)
{
	Str8 result;
	result.length = left.length + right.length;
	result.pntr = malloc(result.length + (addNullTerm ? 1 : 0));
	memcpy(&result.chars[0], &left.chars[0], left.length);
	memcpy(&result.chars[left.length], &right.chars[0], right.length);
	if (addNullTerm) { result.chars[result.length] = '\0'; }
	return result;
}
static inline Str8 JoinStrings3(Str8 left, Str8 middle, Str8 right, bool addNullTerm)
{
	Str8 result;
	result.length = left.length + middle.length + right.length;
	result.pntr = malloc(result.length + (addNullTerm ? 1 : 0));
	memcpy(&result.chars[0], &left.chars[0], left.length);
	memcpy(&result.chars[left.length], &middle.chars[0], middle.length);
	memcpy(&result.chars[left.length + middle.length], &right.chars[0], right.length);
	if (addNullTerm) { result.chars[result.length] = '\0'; }
	return result;
}

#define CONCAT2(leftNt, rightNt)           JoinStrings2(NewStr8Nt(leftNt), NewStr8Nt(rightNt), true)
#define CONCAT3(leftNt, middleNt, rightNt) JoinStrings3(NewStr8Nt(leftNt), NewStr8Nt(middleNt), NewStr8Nt(rightNt), true)

//Returns the number of target characters that were replaced
static inline uxx StrReplaceChars(Str8 haystack, char targetChar, char replaceChar)
{
	uxx numReplacements = 0;
	for (uxx cIndex = 0; cIndex < haystack.length; cIndex++)
	{
		if (haystack.chars[cIndex] == targetChar)
		{
			haystack.chars[cIndex] = replaceChar;
			numReplacements++;
		}
	}
	return numReplacements;
}

static inline void FixPathSlashes(Str8 path, char slashChar)
{
	StrReplaceChars(path, (slashChar == '/') ? '\\' : '/', slashChar);
}

static inline Str8 StrReplace(Str8 haystack, Str8 target, Str8 replacement, bool addNullTerm)
{
	Str8 result = ZEROED;
	for (uxx cIndex = 0; cIndex + target.length <= haystack.length; cIndex++)
	{
		if (StrExactEquals(StrSlice(haystack, cIndex, cIndex+target.length), target))
		{
			result.length += replacement.length;
			cIndex += target.length-1;
		}
		else { result.length += 1; }
	}
	result.pntr = malloc(result.length + (addNullTerm ? 1 : 0));
	uxx writeIndex = 0;
	for (uxx cIndex = 0; cIndex < haystack.length; cIndex++)
	{
		if (StrExactEquals(StrSlice(haystack, cIndex, cIndex+target.length), target))
		{
			memcpy(&result.chars[writeIndex], replacement.chars, replacement.length);
			writeIndex += replacement.length;
			cIndex += target.length-1;
		}
		else
		{
			result.chars[writeIndex] = haystack.chars[cIndex];
			writeIndex += 1;
		}
	}
	if (addNullTerm) { result.chars[result.length] = '\0'; }
	return result;
}

// +--------------------------------------------------------------+
// |                         Line Parser                          |
// +--------------------------------------------------------------+
static inline LineParser NewLineParser(Str8 inputStr)
{
	LineParser result = ZEROED;
	result.byteIndex = 0;
	result.lineIndex = 0;
	result.inputStr = inputStr;
	return result;
}

static inline bool LineParserGetLine(LineParser* parser, Str8* lineOut)
{
	if (parser->byteIndex >= parser->inputStr.length) { return false; }
	parser->lineIndex++;
	parser->lineBeginByteIndex = parser->byteIndex;
	
	uxx endOfLineByteSize = 0;
	uxx startIndex = parser->byteIndex;
	while (parser->byteIndex < parser->inputStr.length)
	{
		char nextChar = parser->inputStr.chars[parser->byteIndex];
		char nextNextChar = parser->inputStr.chars[parser->byteIndex+1];
		//TODO: Should we handle \n\r sequence? Windows is \r\n and I don't know of any space where \n\r is considered a valid single new-line
		if (nextChar != nextNextChar &&
			(nextChar     == '\n' || nextChar     == '\r') &&
			(nextNextChar == '\n' || nextNextChar == '\r'))
		{
			endOfLineByteSize = 2;
			break;
		}
		else if (nextChar == '\n' || nextChar == '\r')
		{
			endOfLineByteSize = 1;
			break;
		}
		else
		{
			parser->byteIndex++;
		}
	}
	
	Str8 line = NewStr8(parser->byteIndex - startIndex, &parser->inputStr.chars[startIndex]);
	parser->byteIndex += endOfLineByteSize;
	if (lineOut != nullptr) { *lineOut = line; }
	return true;
}

// +--------------------------------------------------------------+
// |                        File Functions                        |
// +--------------------------------------------------------------+
// Result is always null-terminated
// TODO: On linux this will not work properly for paths to folders that don't exist yet
static inline Str8 GetFullPath(Str8 relativePath, char slashChar)
{
	Str8 result = ZEROED;
	
	#if BUILDING_ON_WINDOWS
	{
		Str8 relativePathNt = CopyStr8(relativePath, true);
		FixPathSlashes(relativePathNt, PATH_SEP_CHAR);
		
		// Returns required buffer size +1 when the nBufferLength is too small
		DWORD getPathResult1 = GetFullPathNameA(
			relativePathNt.chars, //lpFileName
			0, //nBufferLength
			nullptr, //lpBuffer
			nullptr //lpFilePart
		);
		assert(getPathResult1 != 0);
		
		result.length = (uxx)getPathResult1-1;
		result.chars = (char*)malloc(result.length + 1);
		
		// Returns the length of the string (not +1) when nBufferLength is large enough
		DWORD getPathResult2 = GetFullPathNameA(
			relativePathNt.chars, //lpFileName
			(DWORD)(result.length+1), //nBufferLength
			result.chars, //lpBuffer
			nullptr //lpFilePart
		);
		assert(getPathResult2+1 == getPathResult1);
		assert(result.chars[result.length] == '\0');
		
		FixPathSlashes(result, slashChar);
		free(relativePathNt.chars);
	}
	#elif (BUILDING_ON_LINUX || BUILDING_ON_OSX)
	{
		Str8 relativePathNt = CopyStr8(relativePath, true);
		FixPathSlashes(relativePathNt, PATH_SEP_CHAR);
		
		char* temporaryBuffer = (char*)malloc(PATH_MAX);
		char* realPathResult = realpath(relativePathNt.chars, temporaryBuffer);
		assert(realPathResult != nullptr);
		
		result.length = (uxx)strlen(realPathResult);
		result.chars = (char*)malloc(result.length + 1);
		memcpy(result.chars, realPathResult, result.length);
		result.chars[result.length] = '\0';
		
		FixPathSlashes(result, slashChar);
		free(temporaryBuffer);
		free(relativePathNt.chars);
	}
	#else
	assert(false && "GetFullPath does not support the current platform yet!");
	#endif
		
	return result;
}

static inline bool TryReadFile(Str8 filePath, Str8* contentsOut)
{
	Str8 filePathNt = CopyStr8(filePath, true);
	FixPathSlashes(filePathNt, PATH_SEP_CHAR);
	
	//NOTE: We open the file in binary mode because otherwise the result from jumping to SEEK_END to
	//      check the file size does not match the result of fread because the new-lines get converted
	//      in the fread NOT in the result from ftell
	FILE* fileHandle = fopen(filePathNt.chars, "rb");
	free(filePathNt.chars);
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
//NOTE: We can't name this "ReadFile" because it conflicts with a Windows function
static inline Str8 ReadEntireFile(Str8 filePath)
{
	Str8 result = ZEROED;
	bool readSuccess = TryReadFile(filePath, &result);
	if (!readSuccess) { exit(3); }
	return result;
}

static inline void CreateAndWriteFile(Str8 filePath, Str8 contents, bool convertNewLines)
{
	Str8 filePathNt = CopyStr8(filePath, true);
	FixPathSlashes(filePathNt, PATH_SEP_CHAR);
	
	#if BUILDING_ON_WINDOWS
	{
		if (convertNewLines) { contents = StrReplace(contents, StrLit("\n"), StrLit("\r\n"), false); }
		HANDLE fileHandle = CreateFileA(
			filePathNt.chars,      //Name of the file
			GENERIC_WRITE,         //Open for writing
			0,                     //Do not share
			NULL,                  //Default security
			CREATE_ALWAYS,         //Always overwrite
			FILE_ATTRIBUTE_NORMAL, //Default file attributes
			0                      //No Template File
		);
		assert(fileHandle != INVALID_HANDLE_VALUE);
		if (contents.length > 0)
		{
			DWORD bytesWritten = 0;
			BOOL writeResult = WriteFile(
				fileHandle, //hFile
				contents.chars, //lpBuffer
				(DWORD)contents.length, //nNumberOfBytesToWrite
				&bytesWritten, //lpNumberOfBytesWritten
				0 //lpOverlapped
			);
			assert(writeResult == TRUE);
			assert((uxx)bytesWritten == contents.length);
		}
		CloseHandle(fileHandle);
		if (convertNewLines) { free(contents.chars); }
	}
	#elif (BUILDING_ON_LINUX || BUILDING_ON_OSX)
	{
		FILE* fileHandle = fopen(filePathNt.chars, "w");
		assert(fileHandle != nullptr);
		if (contents.length > 0)
		{
			size_t writeResult = fwrite(
				contents.pntr, //ptr
				1, //size
				contents.length, //count
				fileHandle //stream
			);
			assert(writeResult >= 0);
			assert((uxx)writeResult == contents.length);
		}
		fclose(fileHandle);
	}
	#else
	assert(false && "CreateAndWriteFile does not support the current platform yet!");
	#endif
	
	free(filePathNt.chars);
}

static inline void AppendToFile(Str8 filePath, Str8 contentsToAppend, bool convertNewLines)
{
	Str8 filePathNt = CopyStr8(filePath, true);
	FixPathSlashes(filePathNt, PATH_SEP_CHAR);
	
	#if BUILDING_ON_WINDOWS
	{
		if (convertNewLines) { contentsToAppend = StrReplace(contentsToAppend, StrLit("\n"), StrLit("\r\n"), false); }
		HANDLE fileHandle = CreateFileA(
			filePathNt.chars,      //Name of the file
			GENERIC_WRITE,         //Open for writing
			0,                     //Do not share
			NULL,                  //Default security
			OPEN_ALWAYS,           //Open if it exists, or create a new file if not
			FILE_ATTRIBUTE_NORMAL, //Default file attributes
			0                      //No Template File
		);
		assert(fileHandle != INVALID_HANDLE_VALUE);
		DWORD moveResult = SetFilePointer(
			fileHandle, //hFile
			0, //lDistanceToMove,
			NULL, //lDistanceToMoveHigh
			FILE_END
		);
		assert(moveResult != INVALID_SET_FILE_POINTER);
		if (contentsToAppend.length > 0)
		{
			DWORD bytesWritten = 0;
			BOOL writeResult = WriteFile(
				fileHandle, //hFile
				contentsToAppend.chars, //lpBuffer
				(DWORD)contentsToAppend.length, //nNumberOfBytesToWrite
				&bytesWritten, //lpNumberOfBytesWritten
				0 //lpOverlapped
			);
			assert(writeResult == TRUE);
			assert((uxx)bytesWritten == contentsToAppend.length);
		}
		CloseHandle(fileHandle);
		if (convertNewLines) { free(contentsToAppend.chars); }
	}
	#elif (BUILDING_ON_LINUX || BUILDING_ON_OSX)
	{
		FILE* fileHandle = fopen(filePathNt.chars, "a");
		assert(fileHandle != nullptr);
		if (contentsToAppend.length > 0)
		{
			size_t writeResult = fwrite(
				contentsToAppend.pntr, //ptr
				1, //size
				contentsToAppend.length, //count
				fileHandle //stream
			);
			assert(writeResult >= 0);
			assert((uxx)writeResult == contentsToAppend.length);
		}
		fclose(fileHandle);
	}
	#else
	assert(false && "AppendToFile does not support the current platform yet!");
	#endif
	
	free(filePathNt.chars);
}
static inline void AppendPrintToFile(Str8 filePath, const char* formatString, ...)
{
	char printBuffer[512];
	
	va_list args;
	va_start(args, formatString);
	int printResult = vsnprintf(&printBuffer[0], ArrayCount(printBuffer), formatString, args);
	va_end(args);
	assert(printResult >= 0);
	assert(printResult < ArrayCount(printBuffer));
	Str8 printedStr = NewStr8((uxx)printResult, &printBuffer[0]);
	AppendToFile(filePath, printedStr, true);
}

static inline void CopyFileToPath(Str8 filePath, Str8 newFilePath)
{
	Str8 fileContents = ZEROED;
	bool readSuccess = TryReadFile(filePath, &fileContents);
	assert(readSuccess);
	CreateAndWriteFile(newFilePath, fileContents, false);
	free(fileContents.chars);
}
static inline void CopyFileToFolder(Str8 filePath, Str8 folderPath)
{
	Str8 fileName = GetFileNamePart(filePath, true);
	const char* joinStr = (folderPath.length == 0 || !IS_SLASH(folderPath.chars[folderPath.length-1])) ? "/" : "";
	Str8 newPath = JoinStrings3(folderPath, NewStr8Nt(joinStr), fileName, false);
	CopyFileToPath(filePath, newPath);
	free(newPath.chars);
}

static inline bool DoesFileExist(Str8 filePath)
{
	char* filePathNt = (char*)malloc(filePath.length+1);
	memcpy(filePathNt, filePath.chars, filePath.length);
	filePathNt[filePath.length] = '\0';
	#if BUILDING_ON_WINDOWS
	{
		BOOL fileExistsResult = PathFileExistsA(filePathNt);
		free(filePathNt);
		return (fileExistsResult == TRUE);
	}
	#elif (BUILDING_ON_LINUX || BUILDING_ON_OSX)
	{
		int accessResult = access(filePathNt, F_OK);
		free(filePathNt);
		return (accessResult == 0);
	}
	#else
	assert(false && "tools_shared.h's DoesFileExist does not support the current platform yet!");
	return false;
	#endif
}

static inline void AssertFileExist(Str8 filePath, bool wasCreatedByBuild)
{
	if (!DoesFileExist(filePath))
	{
		PrintLine_E("Missing file \"%.*s\" %s!", filePath.length, filePath.chars, wasCreatedByBuild ? "was not created" : "was not found");
		exit(6);
	}
}

static inline FileIter StartFileIter(Str8 folderPath)
{
	FileIter result = ZEROED;
	result.index = UINTXX_MAX;
	result.nextIndex = 0;
	result.finished = false;
	bool needsTrailingSlash = (folderPath.length == 0 || (folderPath.chars[folderPath.length-1] != '\\' && folderPath.chars[folderPath.length-1] != '/'));
	result.folderPathNt.length = folderPath.length + (needsTrailingSlash ? 1 : 0);
	result.folderPathNt.chars = (char*)malloc(result.folderPathNt.length + 1);
	memcpy(result.folderPathNt.chars, folderPath.chars, folderPath.length);
	if (needsTrailingSlash) { result.folderPathNt.chars[folderPath.length] = PATH_SEP_CHAR; }
	result.folderPathNt.chars[result.folderPathNt.length] = '\0';
	
	#if BUILDING_ON_WINDOWS
	{
		// ChangePathSlashesTo(result.folderPath, '\\'); //TODO: Should we do this?
		//NOTE: File iteration in windows requires that we have a slash on the end and a * wildcard character
		result.folderPathWithWildcard = JoinStrings2(result.folderPathNt, StrLit("*"), true);
	}
	#elif (BUILDING_ON_LINUX || BUILDING_ON_OSX)
	{
		//nothing to do
	}
	#else
	assert(false && "StartFileIter does not support the current platform yet!");
	result.finished = true;
	#endif
	
	return result;
}

// Ex version gives isFolderOut
static bool StepFileIter(FileIter* fileIter, Str8* pathOut, bool* isFolderOut)
{
	if (fileIter->finished) { return false; }
	
	#if BUILDING_ON_WINDOWS
	{
		while (true)
		{
			bool firstIteration = (fileIter->index == UINTXX_MAX);
			fileIter->index = fileIter->nextIndex;
			if (firstIteration)
			{
				fileIter->handle = FindFirstFileA(fileIter->folderPathWithWildcard.chars, &fileIter->findData);
				if (fileIter->handle == INVALID_HANDLE_VALUE)
				{
					free(fileIter->folderPathNt.chars); fileIter->folderPathNt.chars = nullptr;
					free(fileIter->folderPathWithWildcard.chars); fileIter->folderPathWithWildcard.chars = nullptr;
					fileIter->finished = true;
					return false;
				}
			}
			else
			{
				BOOL findNextResult = FindNextFileA(fileIter->handle, &fileIter->findData);
				if (findNextResult == 0)
				{
					free(fileIter->folderPathNt.chars); fileIter->folderPathNt.chars = nullptr;
					free(fileIter->folderPathWithWildcard.chars); fileIter->folderPathWithWildcard.chars = nullptr;
					fileIter->finished = true;
					return false;
				}
			}
			
			Str8 fileName = NewStr8Nt(fileIter->findData.cFileName);
			
			//ignore current and parent folder entries
			if (StrExactEquals(fileName, StrLit(".")) || StrExactEquals(fileName, StrLit("..")))
			{
				continue;
			}
			
			bool isFolder = (fileIter->findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
			if (pathOut != nullptr)
			{
				*pathOut = JoinStrings2(fileIter->folderPathNt, fileName, true);
				// FixPathSlashes(*pathOut); //TODO: Should we do this?
			}
			if (isFolderOut != nullptr) { *isFolderOut = isFolder; }
			fileIter->nextIndex = fileIter->index+1;
			return true;
		}
	}
	#elif (BUILDING_ON_LINUX || BUILDING_ON_OSX)
	{
		while (true)
		{
			bool firstIteration = (fileIter->index == UINTXX_MAX);
			fileIter->index = fileIter->nextIndex;
			if (firstIteration)
			{
				fileIter->dirHandle = opendir(fileIter->folderPathNt.chars);
				if (fileIter->dirHandle == nullptr)
				{
					free(fileIter->folderPathNt.chars); fileIter->folderPathNt.chars = nullptr;
					fileIter->finished = true;
					return false;
				}
			}
			
			struct dirent* entry = readdir(fileIter->dirHandle);
			if (entry == nullptr)
			{
				free(fileIter->folderPathNt.chars); fileIter->folderPathNt.chars = nullptr;
				fileIter->finished = true;
				return false;
			}
			
			Str8 fileName = NewStr8Nt(entry->d_name);
			if (StrExactEquals(fileName, StrLit(".")) || StrExactEquals(fileName, StrLit(".."))) { continue; } //ignore current and parent folder entries
			
			Str8 fullPath = JoinStrings2(fileIter->folderPathNt, fileName, true);
			if (isFolderOut != nullptr)
			{
				struct stat statStruct = ZEROED;
				int statResult = stat(fullPath.chars, &statStruct);
				if (statResult == 0)
				{
					if ((statStruct.st_mode & S_IFDIR) != 0)
					{
						if (isFolderOut != nullptr) { *isFolderOut = true; }
					}
					else if ((statStruct.st_mode & S_IFREG) != 0)
					{
						if (isFolderOut != nullptr) { *isFolderOut = false; }
					}
					else
					{
						PrintLine_E("Unknown file type for \"%.*s\"", fullPath.length, fullPath.chars);
						continue;
					}
				}
			}
			
			if (pathOut != nullptr) { *pathOut = fullPath; }
			fileIter->nextIndex = fileIter->index+1;
			return true;
		}
	}
	#else
	assert(false && "StepFileIter does not support the current platform yet!");
	fileIter->finished = true;
	#endif
	
	return false;
}

static void RecursiveDirWalk(Str8 rootDir, RecursiveDirWalkCallback_f* callback, void* contextPntr)
{
	FileIter iter = StartFileIter(rootDir);
	Str8 path = ZEROED;
	bool isFolder = false;
	while (StepFileIter(&iter, &path, &isFolder))
	{
		bool callbackResult = callback(path, isFolder, contextPntr);
		if (isFolder && callbackResult)
		{
			RecursiveDirWalk(path, callback, contextPntr);
		}
	}
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
