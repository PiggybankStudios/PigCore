/*
File:   os_file.h
Author: Taylor Robbins
Date:   01\06\2025
Description:
	** Contains functions that help us open, read, write, delete, and iterate files and folders
*/

#ifndef _OS_FILE_H
#define _OS_FILE_H

#include "base/base_typedefs.h"
#include "std/std_includes.h"
#include "struct/struct_string.h"
#include "mem/mem_scratch.h"

// +--------------------------------------------------------------+
// |                          Full Path                           |
// +--------------------------------------------------------------+
// Passing nullptr for arena will return a FilePath with no chars pntr but length is filled out
// NOTE: The result is always null-terminated
FilePath OsGetFullPath(Arena* arena, FilePath relativePath)
{
	NotNullStr(relativePath);
	FilePath result = Str8_Empty_Const;
	ScratchBegin1(scratch, arena);
	
	#if TARGET_IS_WINDOWS
	{
		//NOTE: GetFullPathNameA needs a null-terminated string with backslashes,
		//      so we make a copy of relativePath on the scratch arena and replace forward slashes with backslashes
		FilePath relativePathNt = AllocFilePath(scratch, relativePath, true);
		ChangePathSlashesTo(relativePathNt, '\\');
		
		// Returns required buffer size +1 when the nBufferLength is too small
		DWORD getPathResult1 = GetFullPathNameA(
			relativePathNt.chars, //lpFileName
			0, //nBufferLength
			nullptr, //lpBuffer
			nullptr //lpFilePart
		);
		if (getPathResult1 == 0) { ScratchEnd(scratch); return Str8_Empty; }
		
		result = NewFilePath((uxx)getPathResult1-1, nullptr);
		if (arena == nullptr) { ScratchEnd(scratch); return result; }
		
		result.chars = AllocArray(char, arena, result.length+1);
		if (result.chars == nullptr) { ScratchEnd(scratch); return Str8_Empty; }
		
		// Returns the length of the string (not +1) when nBufferLength is large enough
		DWORD getPathResult2 = GetFullPathNameA(
			relativePathNt.chars, //lpFileName
			(DWORD)(result.length+1), //nBufferLength
			result.chars, //lpBuffer
			nullptr //lpFilePart
		);
		Assert(getPathResult2+1 == getPathResult1);
		Assert(result.chars[result.length] == '\0');
		
		FixPathSlashes(result);
	}
	// #elif TARGET_IS_LINUX
	// {
	// 	//TODO: Implement me!
	// }
	// #elif TARGET_IS_OSX
	// {
	// 	//TODO: Implement me!
	// }
	#else
	AssertMsg(false, "OsGetFullPath does not support the current platform yet!");
	#endif
		
	ScratchEnd(scratch);
	return result;
}
uxx OsGetFullPathLength(FilePath relativePath)
{
	FilePath lengthOnlyPath = OsGetFullPath(nullptr, relativePath);
	return lengthOnlyPath.length;
}

// +--------------------------------------------------------------+
// |                            Exists                            |
// +--------------------------------------------------------------+
bool OsDoesFileOrFolderExist(FilePath path, bool* isFolderOut)
{
	#if TARGET_IS_WINDOWS
	{
		ScratchBegin(scratch);
		FilePath fullPath = OsGetFullPath(scratch, path);
		DWORD fileType = GetFileAttributesA(fullPath.chars);
		ScratchEnd(scratch);
		if (fileType == INVALID_FILE_ATTRIBUTES)
		{
			SetOptionalOutPntr(isFolderOut, false);
			return false;
		}
		else
		{
			SetOptionalOutPntr(isFolderOut, IsFlagSet(fileType, FILE_ATTRIBUTE_DIRECTORY));
			return true;
		}
	}
	// #elif TARGET_IS_LINUX
	// {
	// 	//TODO: Implement me!
	// }
	// #elif TARGET_IS_OSX
	// {
	// 	//TODO: Implement me!
	// }
	#else
	AssertMsg(false, "OsDoesFileOrFolderExist does not support the current platform yet!");
	return false;
	#endif
}
bool OsDoesFileExist(FilePath path)
{
	bool isFolder = false;
	bool doesExist = OsDoesFileOrFolderExist(path, &isFolder);
	return (doesExist && !isFolder);
}
bool OsDoesFolderExist(FilePath path)
{
	bool isFolder = false;
	bool doesExist = OsDoesFileOrFolderExist(path, &isFolder);
	return (doesExist && isFolder);
}

// +--------------------------------------------------------------+
// |                          Iteration                           |
// +--------------------------------------------------------------+
typedef struct OsFileIter OsFileIter;
struct OsFileIter
{
	Arena* arena;
	FilePath folderPath; //has a trailing slash
	FilePath folderPathWithWildcard;
	bool includeFiles;
	bool includeFolders;
	
	bool finished;
	uxx index;
	uxx nextIndex;
	
	#if TARGET_IS_WINDOWS
	WIN32_FIND_DATAA findData;
	HANDLE handle;
	#endif
};

void OsFreeFileIter(OsFileIter* fileIter)
{
	if (fileIter->folderPath.chars != nullptr) { FreeStr8WithNt(fileIter->arena, &fileIter->folderPath); }
	if (fileIter->folderPathWithWildcard.chars != nullptr) { FreeStr8WithNt(fileIter->arena, &fileIter->folderPathWithWildcard); }
	ClearPointer(fileIter);
}

OsFileIter OsIterateFiles(Arena* arena, FilePath path, bool includeFiles, bool includeFolders)
{
	OsFileIter result = ZEROED;
	
	#if TARGET_IS_WINDOWS
	{
		ScratchBegin1(scratch, arena);
		result.includeFiles = includeFiles;
		result.includeFolders = includeFolders;
		result.index = UINTXX_MAX;
		result.nextIndex = 0;
		result.finished = false;
		Str8 fullPath = OsGetFullPath(scratch, path);
		NotNullStr(fullPath);
		result.folderPath = AllocFolderPath(arena, fullPath, true); //ensures trailing slash!
		NotNullStr(result.folderPath);
		ChangePathSlashesTo(result.folderPath, '\\');
		//NOTE: File iteration in windows requires that we have a slash on the end and a * wildcard character
		result.folderPathWithWildcard = JoinStringsInArena(arena, result.folderPath, StrNt("*"), true);
		NotNullStr(result.folderPathWithWildcard);
		MyPrint("result.folderPath: \"%.*s\"", StrPrint(result.folderPath));
		MyPrint("result.folderPathWithWildcard: \"%.*s\"", StrPrint(result.folderPathWithWildcard));
		ScratchEnd(scratch);
	}
	// #elif TARGET_IS_LINUX
	// {
	// 	//TODO: Implement me!
	// }
	// #elif TARGET_IS_OSX
	// {
	// 	//TODO: Implement me!
	// }
	#else
	AssertMsg(false, "OsIterateFiles does not support the current platform yet!");
	result.finished = true;
	#endif
	
	return result;
}

// Ex version gives isFolderOut
bool OsIterFileStepEx(OsFileIter* fileIter, bool* isFolderOut, FilePath* pathOut, Arena* pathOutArena, bool giveFullPath)
{
	NotNull(fileIter);
	if (pathOut != nullptr) { Assert(pathOutArena != nullptr); }
	if (fileIter->finished) { return false; }
	
	#if TARGET_IS_WINDOWS
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
					fileIter->finished = true;
					return false;
				}
			}
			else
			{
				BOOL findNextResult = FindNextFileA(fileIter->handle, &fileIter->findData);
				if (findNextResult == 0)
				{
					fileIter->finished = true;
					return false;
				}
			}
			
			//Skip the generic "this" folder listing
			if (fileIter->findData.cFileName[0] == '.' && fileIter->findData.cFileName[1] == '\0')
			{
				continue;
			}
			//Skip the parent folder listing
			if (fileIter->findData.cFileName[0] == '.' && fileIter->findData.cFileName[1] == '.' && fileIter->findData.cFileName[2] == '\0')
			{
				continue;
			}
			
			bool isFolder = IsFlagSet(fileIter->findData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY);
			if ((isFolder && fileIter->includeFolders) || (!isFolder && fileIter->includeFiles))
			{
				if (pathOut != nullptr)
				{
					if (giveFullPath)
					{
						*pathOut = JoinStringsInArena(pathOutArena, fileIter->folderPath, StrNt(fileIter->findData.cFileName), false);
					}
					else
					{
						*pathOut = AllocFilePath(pathOutArena, StrNt(fileIter->findData.cFileName), false);
					}
					NotNullStr(*pathOut);
					FixPathSlashes(*pathOut);
				}
				SetOptionalOutPntr(isFolderOut, isFolder);
				fileIter->nextIndex = fileIter->index+1;
				return true;
			}
		}
		Assert(false); //Shouldn't be possible to get here
	}
	// #elif TARGET_IS_LINUX
	// {
	// 	//TODO: Implement me!
	// }
	// #elif TARGET_IS_OSX
	// {
	// 	//TODO: Implement me!
	// }
	#else
	AssertMsg(false, "OsIterFileStep does not support the current platform yet!");
	result.finished = true;
	#endif
	
	return false;
}
bool OsIterFileStep(OsFileIter* fileIter, FilePath* pathOut, Arena* pathOutArena, bool giveFullPath)
{
	return OsIterFileStepEx(fileIter, nullptr, pathOut, pathOutArena, giveFullPath);
}

#endif //  _OS_FILE_H
