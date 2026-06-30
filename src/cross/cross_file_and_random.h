/*
File:   cross_file_and_random.h
Author: Taylor Robbins
Date:   04\14\2025
*/

#ifndef _CROSS_FILE_AND_RANDOM_H
#define _CROSS_FILE_AND_RANDOM_H

//NOTE: Intentionally no includes here!

#define TEMPORARY_FILENAME_LENGTH  16 //chars

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	FilePath OsGetTemporaryFolderPath(Arena* arena);
	Str8 OsGetTemporaryFileName(Arena* arena, RandomSeries* randomSeries);
	PIG_CORE_INLINE FilePath OsGetTemporaryFilePath(Arena* arena, RandomSeries* randomSeries);
	PIG_CORE_INLINE bool OsOpenTemporaryFile(Arena* arena, RandomSeries* randomSeries, OsFile* openFileOut);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

//NOTE: The result is always null-terminated, and always has a trailing slash
PEXP FilePath OsGetTemporaryFolderPath(Arena* arena)
{
	FilePath result = FilePath_Empty;
	
	#if TARGET_IS_WINDOWS
	{
		DWORD spaceRequired = GetTempPathA(0, nullptr);
		Assert(spaceRequired > 0);
		result.length = (uxx)spaceRequired-1;
		if (arena == nullptr) { return result; }
		result.chars = (char*)AllocMem(arena, result.length+1);
		NotNull(result.chars);
		
		DWORD pathLengthDword = GetTempPathA(spaceRequired, result.chars);
		Assert(pathLengthDword == spaceRequired-1);
		result.chars[result.length] = '\0';
		FixPathSlashes(result);
		Assert(DoesPathHaveTrailingSlash(result));
	}
	#elif TARGET_IS_LINUX
	{
		ScratchBegin1(scratch, arena);
		char* temporaryDirTemplateString = AllocAndCopyCharsNt(scratch, "/tmp/tmpdir.XXXXXX", true);
		char* temporaryDirString = mkdtemp(temporaryDirTemplateString);
		NotNull(temporaryDirString);
		result = AllocFolderPath(arena, MakeStr8Nt(temporaryDirString), true);
		Assert(DoesPathHaveTrailingSlash(result));
		ScratchEnd(scratch);
	}
	// #elif TARGET_IS_OSX
	// {
	// 	//TODO: Implement me!
	// }
	#else
	UNUSED(arena);
	AssertMsg(false, "OsGetTemporaryFolderPath does not support the current platform yet!");
	#endif
	
	return result;
}

//NOTE: Consider seeding the randomSeries using PID of this process to make sure another program with the same start time doesn't have the same seed
//NOTE: The result is always null-terminated
PEXP Str8 OsGetTemporaryFileName(Arena* arena, RandomSeries* randomSeries)
{
	Str8 result = Str8_Empty;
	result.length = TEMPORARY_FILENAME_LENGTH;
	if (arena == nullptr) { return result; }
	result.chars = (char*)AllocMem(arena, result.length+1);
	NotNull(result.chars);
	
	NotNull(randomSeries);
	char characters[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789";
	for (uxx cIndex = 0; cIndex < result.length; cIndex++)
	{
		result.chars[cIndex] = characters[GetRandU8Range(randomSeries, 0, ArrayCount(characters)-1)];
	}
	result.chars[result.length] = '\0';
	
	return result;
}

//NOTE: The result is always null-terminated
PEXPI FilePath OsGetTemporaryFilePath(Arena* arena, RandomSeries* randomSeries)
{
	uxx folderLength = OsGetTemporaryFolderPath(nullptr).length;
	uxx nameLength = OsGetTemporaryFileName(nullptr, nullptr).length;
	
	FilePath result = FilePath_Empty;
	result.length = folderLength + nameLength;
	if (arena == nullptr) { return result; }
	result.chars = (char*)AllocMem(arena, result.length+1);
	NotNull(result.chars);
	
	Arena folderArena = ZEROED;
	InitArenaBuffer(&folderArena, &result.chars[0], folderLength+1);
	FilePath folderPath = OsGetTemporaryFolderPath(&folderArena);
	Assert(folderPath.chars == &result.chars[0]);
	
	Arena nameArena = ZEROED;
	InitArenaBuffer(&nameArena, &result.chars[folderLength], nameLength+1);
	Str8 fileName = OsGetTemporaryFileName(&nameArena, randomSeries);
	Assert(fileName.chars == &result.chars[folderLength]);
	
	result.chars[result.length] = '\0';
	return result;
}

//TODO: Change this to return Result instead of bool
PEXPI bool OsOpenTemporaryFile(Arena* arena, RandomSeries* randomSeries, OsFile* openFileOut)
{
	ScratchBegin1(scratch, arena);
	FilePath tempFilePath = OsGetTemporaryFilePath(scratch, randomSeries);
	NotEmptyStr(tempFilePath);
	FilePath folderPart = GetFileFolderPart(tempFilePath);
	Result createFoldersResult = OsCreateFolder(folderPart, true);
	if (createFoldersResult != Result_Success) { return false; }
	bool openResult = OsOpenFile(arena, tempFilePath, OsOpenFileMode_Create, false, openFileOut);
	ScratchEnd(scratch);
	return openResult;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_FILE_AND_RANDOM_H
