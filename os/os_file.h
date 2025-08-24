/*
File:   os_file.h
Author: Taylor Robbins
Date:   01\06\2025
Description:
	** Contains functions that help us open, read, write, delete, and iterate files and folders
*/

#ifndef _OS_FILE_H
#define _OS_FILE_H

#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "std/std_includes.h"
#include "struct/struct_string.h"
#include "mem/mem_scratch.h"
#include "base/base_debug_output.h" //TODO: Remove the need for debug output in this file? Return Result values instead!
#include "os/os_error.h"
#include "misc/misc_result.h"
#include "os/os_path.h"

typedef plex OsFileIter OsFileIter;
plex OsFileIter
{
	Arena* arena;
	FilePath folderPath; //has a trailing slash
	bool includeFiles;
	bool includeFolders;
	
	bool finished;
	uxx index;
	uxx nextIndex;
	
	#if TARGET_IS_WINDOWS
	FilePath folderPathWithWildcard;
	WIN32_FIND_DATAA findData;
	HANDLE handle;
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX)
	DIR* dirHandle;
	#endif
};

typedef enum OsOpenFileMode OsOpenFileMode;
enum OsOpenFileMode
{
	OsOpenFileMode_None = 0,
	OsOpenFileMode_Read, //Opens file for reading if it exists (error otherwise)
	OsOpenFileMode_Create, //Opens a NEW file for writing (fails if the file already exists)
	OsOpenFileMode_Write, //Opens a file for writing (clearing the contents if it already existed)
	OsOpenFileMode_Append, //Opens a file for writing, jumping to the end if it already exists (creating a new file if it doesn't exist)
	//TODO: Do we want a mode where we open for appending but ONLY if the file exists?
	OsOpenFileMode_Count,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetOsOpenFileModeStr(OsOpenFileMode enumValue);
#else
PEXP const char* GetOsOpenFileModeStr(OsOpenFileMode enumValue)
{
	switch (enumValue)
	{
		case OsOpenFileMode_None:      return "None";
		case OsOpenFileMode_Read:      return "Read";
		case OsOpenFileMode_Create:    return "Create";
		case OsOpenFileMode_Write:     return "Write";
		case OsOpenFileMode_Append:    return "Append";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef plex OsFile OsFile;
plex OsFile
{
	Arena* arena;
	bool isOpen;
	bool openedForWriting;
	bool isKnownSize;
	uxx cursorIndex;
	uxx fileSize;
	FilePath path; //has nullterm
	FilePath fullPath; //has nullterm
	
	#if TARGET_IS_WINDOWS
	HANDLE handle;
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX)
	FILE* handle;
	#endif
};

typedef plex OsFileWriteTime OsFileWriteTime;
plex OsFileWriteTime
{
	#if TARGET_IS_WINDOWS
	FILETIME fileTime;
	#elif TARGET_IS_LINUX
	plex timespec timeSpec;
	#else
	u8 placeholder;
	#endif
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	FilePath OsGetFullPath(Arena* arena, FilePath relativePath);
	PIG_CORE_INLINE uxx OsGetFullPathLength(FilePath relativePath);
	bool OsDoesFileOrFolderExist(FilePath path, bool* isFolderOut);
	PIG_CORE_INLINE bool OsDoesPathExist(FilePath path);
	PIG_CORE_INLINE bool OsDoesFileExist(FilePath path);
	PIG_CORE_INLINE bool OsDoesFolderExist(FilePath path);
	PIG_CORE_INLINE void OsFreeFileIter(OsFileIter* fileIter);
	OsFileIter OsIterateFiles(Arena* arena, FilePath path, bool includeFiles, bool includeFolders);
	bool OsIterFileStepEx(OsFileIter* fileIter, bool* isFolderOut, FilePath* pathOut, Arena* pathOutArena, bool giveFullPath);
	PIG_CORE_INLINE bool OsIterFileStep(OsFileIter* fileIter, FilePath* pathOut, Arena* pathOutArena, bool giveFullPath);
	bool OsReadFile(FilePath path, Arena* arena, bool convertNewLines, Slice* contentsOut);
	PIG_CORE_INLINE bool OsReadTextFile(FilePath path, Arena* arena, Str8* contentsOut);
	PIG_CORE_INLINE bool OsReadBinFile(FilePath path, Arena* arena, Slice* contentsOut);
	PIG_CORE_INLINE Slice OsReadFileScratch(FilePath path, bool convertNewLines);
	PIG_CORE_INLINE Str8 OsReadTextFileScratch(FilePath path);
	PIG_CORE_INLINE Slice OsReadBinFileScratch(FilePath path);
	bool OsWriteFile(FilePath path, Str8 fileContents, bool convertNewLines);
	PIG_CORE_INLINE bool OsWriteTextFile(FilePath path, Str8 fileContents);
	PIG_CORE_INLINE bool OsWriteBinFile(FilePath path, Str8 fileContents);
	PIG_CORE_INLINE bool OsCopyFile(FilePath fromPath, FilePath toPath);
	Result OsCreateFolder(FilePath path, bool createParentFoldersIfNeeded);
	void OsCloseFile(OsFile* file);
	bool OsOpenFile(Arena* arena, FilePath path, OsOpenFileMode mode, bool calculateSize, OsFile* openFileOut);
	Result OsReadFromOpenFile(OsFile* file, uxx numBytes, bool convertNewLines, void* bufferOut, uxx* numBytesReadOut);
	PIG_CORE_INLINE Result OsReadFromOpenTextFile(OsFile* file, uxx numBytes, void* bufferOut, uxx* numBytesReadOut);
	PIG_CORE_INLINE Result OsReadFromOpenBinFile(OsFile* file, uxx numBytes, void* bufferOut, uxx* numBytesReadOut);
	bool OsWriteToOpenFile(OsFile* file, Str8 fileContentsPart, bool convertNewLines);
	PIG_CORE_INLINE bool OsWriteToOpenTextFile(OsFile* file, Str8 fileContentsPart);
	PIG_CORE_INLINE bool OsWriteToOpenBinFile(OsFile* file, Str8 fileContentsPart);
	Result OsGetFileWriteTime(FilePath filePath, OsFileWriteTime* timeOut);
	PIG_CORE_INLINE i32 OsCompareFileWriteTime(OsFileWriteTime left, OsFileWriteTime right);
	PIG_CORE_INLINE bool OsAreFileWriteTimesEqual(OsFileWriteTime left, OsFileWriteTime right);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                          Full Path                           |
// +--------------------------------------------------------------+
// Passing nullptr for arena will return a FilePath with no chars pntr but length is filled out
// NOTE: The result is always null-terminated
PEXP FilePath OsGetFullPath(Arena* arena, FilePath relativePath)
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
		
		result.chars = (char*)AllocMem(arena, result.length+1);
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
	#elif TARGET_IS_LINUX || TARGET_IS_OSX
	{
		FilePath relativePathNt = AllocFilePath(scratch, relativePath, true);
		DebugAssert(PATH_MAX <= UINTXX_MAX);
		char* temporaryBuffer = (char*)AllocMem(scratch, (uxx)PATH_MAX);
		
		//TODO: realpath does not work for paths that don't exist yet. We need to do a bunch of work to try and resolve some portion of the relative path, possibly appending the CWD to the front if we run out of pieces to rip off
		// Ultimately this seems like it's going to be a lot of work to get right, so for now we are leaving it as-is. This is not great for a few use cases of OsGetFullPath but they are not crucial for now so it should be fine.
		char* realPathResult = realpath(relativePathNt.chars, temporaryBuffer);
		if (realPathResult != nullptr)
		{
			if (arena != nullptr)
			{
				result = AllocStrAndCopyNt(arena, realPathResult, true);
			}
			else
			{
				result = NewStr8((uxx)MyStrLength(realPathResult), nullptr);
			}
		}
		else
		{
			// If we don't get the full path for a file, it probably doesn't exist, let's try getting the full path for the folder it lives in?
			// If that still doesn't work, we will resort to just returning the relativePath, since likely that will work for the users purpose if they are trying to talk about a file or folder that doesn't exist yet
			FilePath folderPathNt = GetFileFolderPart(relativePathNt);
			if (folderPathNt.length == 0 || folderPathNt.length == relativePathNt.length)
			{
				ScratchEnd(scratch);
				return AllocStrAndCopy(arena, relativePath.length, relativePath.chars, true);
			}
			FilePath fileNamePart = NewFilePath(relativePathNt.length - folderPathNt.length, &relativePathNt.chars[folderPathNt.length]);
			Assert(folderPathNt.chars[folderPathNt.length-1] == '\\' || folderPathNt.chars[folderPathNt.length-1] == '/');
			folderPathNt.length--;
			folderPathNt.chars[folderPathNt.length] = '\0'; //we can do this because relativePathNt was allocated in scratch above
			
			realPathResult = realpath(folderPathNt.chars, temporaryBuffer);
			if (realPathResult != nullptr)
			{
				if (arena != nullptr)
				{
					result = JoinStringsInArenaWithChar(arena, StrLit(realPathResult), '/', fileNamePart, true);
				}
				else
				{
					result = NewStr8((uxx)MyStrLength(realPathResult) + 1 + fileNamePart.length, nullptr);
				}
			}
			else
			{
				ScratchEnd(scratch);
				return AllocStrAndCopy(arena, relativePath.length, relativePath.chars, true);
			}
		}
	}
	#else
	AssertMsg(false, "OsGetFullPath does not support the current platform yet!");
	#endif
		
	ScratchEnd(scratch);
	return result;
}
PEXPI uxx OsGetFullPathLength(FilePath relativePath)
{
	FilePath lengthOnlyPath = OsGetFullPath(nullptr, relativePath);
	return lengthOnlyPath.length;
}

// +--------------------------------------------------------------+
// |                            Exists                            |
// +--------------------------------------------------------------+
PEXP bool OsDoesFileOrFolderExist(FilePath path, bool* isFolderOut)
{
	bool result = false;
	
	#if TARGET_IS_WINDOWS
	{
		ScratchBegin(scratch);
		FilePath fullPath = OsGetFullPath(scratch, path);
		BOOL fileExistsResult = PathFileExistsA(fullPath.chars);
		if (fileExistsResult == TRUE)
		{
			if (isFolderOut != nullptr)
			{
				DWORD fileType = GetFileAttributesA(fullPath.chars);
				if (fileType != INVALID_FILE_ATTRIBUTES)
				{
					*isFolderOut = IsFlagSet(fileType, FILE_ATTRIBUTE_DIRECTORY);
					result = true;
				}
				else { result = false; }
			}
			else { result = true; }
		}
		else
		{
			result = false;
		}
		ScratchEnd(scratch);
	}
	#elif TARGET_IS_LINUX
	{
		ScratchBegin(scratch);
		FilePath fullPath = OsGetFullPath(scratch, path);
		
		int accessResult = access(fullPath.chars, F_OK);
		result = (accessResult == 0);
		
		if (isFolderOut != nullptr && result)
		{
			plex stat statStruct = ZEROED;
			int statResult = stat(fullPath.chars, &statStruct);
			if (statResult == 0)
			{
				*isFolderOut = IsFlagSet(statStruct.st_mode, S_IFDIR);
			}
			else
			{
				PrintLine_E("stat(\"%.*s\") call failed! Can't determine if that path is a folder or file!", StrPrint(fullPath));
				*isFolderOut = false;
			}
		}
		
	}
	// #elif TARGET_IS_OSX
	// {
	// 	//TODO: Implement me!
	// }
	#else
	UNUSED(path);
	UNUSED(isFolderOut);
	AssertMsg(false, "OsDoesFileOrFolderExist does not support the current platform yet!");
	#endif
	
	return result;
}
PEXPI bool OsDoesPathExist(FilePath path)
{
	return OsDoesFileOrFolderExist(path, nullptr);
}
PEXPI bool OsDoesFileExist(FilePath path)
{
	bool isFolder = false;
	bool doesExist = OsDoesFileOrFolderExist(path, &isFolder);
	return (doesExist && !isFolder);
}
PEXPI bool OsDoesFolderExist(FilePath path)
{
	bool isFolder = false;
	bool doesExist = OsDoesFileOrFolderExist(path, &isFolder);
	return (doesExist && isFolder);
}

// +--------------------------------------------------------------+
// |                          Iteration                           |
// +--------------------------------------------------------------+
PEXPI void OsFreeFileIter(OsFileIter* fileIter)
{
	if (fileIter->arena != nullptr)
	{
		if (CanArenaFree(fileIter->arena) && fileIter->folderPath.chars != nullptr) { FreeStr8WithNt(fileIter->arena, &fileIter->folderPath); }
		#if TARGET_IS_WINDOWS
		if (CanArenaFree(fileIter->arena) && fileIter->folderPathWithWildcard.chars != nullptr) { FreeStr8WithNt(fileIter->arena, &fileIter->folderPathWithWildcard); }
		if (fileIter->handle != INVALID_HANDLE_VALUE) { FindClose(fileIter->handle); }
		#elif TARGET_IS_LINUX
		if (fileIter->dirHandle != nullptr) { int closeResult = closedir(fileIter->dirHandle); Assert(closeResult == 0); }
		#endif
	}
	ClearPointer(fileIter);
}

PEXP OsFileIter OsIterateFiles(Arena* arena, FilePath path, bool includeFiles, bool includeFolders)
{
	OsFileIter result = ZEROED;
	result.arena = arena;
	result.includeFiles = includeFiles;
	result.includeFolders = includeFolders;
	result.index = UINTXX_MAX;
	result.nextIndex = 0;
	result.finished = false;
	
	#if TARGET_IS_WINDOWS
	{
		ScratchBegin1(scratch, arena);
		Str8 fullPath = OsGetFullPath(scratch, path);
		NotNullStr(fullPath);
		result.folderPath = AllocFolderPath(arena, fullPath, true); //ensures trailing slash!
		NotNullStr(result.folderPath);
		ChangePathSlashesTo(result.folderPath, '\\');
		//NOTE: File iteration in windows requires that we have a slash on the end and a * wildcard character
		result.folderPathWithWildcard = JoinStringsInArena(arena, result.folderPath, StrLit("*"), true);
		NotNullStr(result.folderPathWithWildcard);
		ScratchEnd(scratch);
	}
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX)
	{
		ScratchBegin1(scratch, arena);
		Str8 fullPath = OsGetFullPath(scratch, path);
		NotNullStr(fullPath);
		result.folderPath = AllocFolderPath(arena, fullPath, true); //ensures trailing slash!
		NotNullStr(result.folderPath);
		ScratchEnd(scratch);
	}
	#else
	UNUSED(path);
	AssertMsg(false, "OsIterateFiles does not support the current platform yet!");
	result.finished = true;
	#endif
	
	return result;
}

// Ex version gives isFolderOut
PEXP bool OsIterFileStepEx(OsFileIter* fileIter, bool* isFolderOut, FilePath* pathOut, Arena* pathOutArena, bool giveFullPath)
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
					OsFreeFileIter(fileIter);
					fileIter->finished = true;
					return false;
				}
			}
			else
			{
				BOOL findNextResult = FindNextFileA(fileIter->handle, &fileIter->findData);
				if (findNextResult == 0)
				{
					OsFreeFileIter(fileIter);
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
						*pathOut = JoinStringsInArena(pathOutArena, fileIter->folderPath, StrLit(fileIter->findData.cFileName), false);
					}
					else
					{
						*pathOut = AllocFilePath(pathOutArena, StrLit(fileIter->findData.cFileName), false);
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
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX)
	{
		ScratchBegin1(scratch, fileIter->arena);
		while (true)
		{
			bool firstIteration = (fileIter->index == UINTXX_MAX);
			fileIter->index = fileIter->nextIndex;
			if (firstIteration)
			{
				fileIter->dirHandle = opendir(fileIter->folderPath.chars);
				if (fileIter->dirHandle == nullptr)
				{
					OsFreeFileIter(fileIter);
					fileIter->finished = true;
					ScratchEnd(scratch);
					return false;
				}
			}
			
			struct dirent* entry = readdir(fileIter->dirHandle);
			if (entry == nullptr)
			{
				OsFreeFileIter(fileIter);
				fileIter->finished = true;
				ScratchEnd(scratch);
				return false;
			}
			
			Str8 fileName = StrLit(entry->d_name);
			if (StrExactEquals(fileName, StrLit(".")) || StrExactEquals(fileName, StrLit(".."))) { continue; } //ignore current and parent folder entries
			
			FilePath fullPath = JoinStringsInArena(scratch, fileIter->folderPath, fileName, true);
			NotNullStr(fullPath);
			if (!fileIter->includeFiles || !fileIter->includeFolders || isFolderOut != nullptr)
			{
				plex stat statStruct = ZEROED;
				int statResult = stat(fullPath.chars, &statStruct);
				if (statResult == 0)
				{
					if (IsFlagSet(statStruct.st_mode, S_IFDIR))
					{
						if (!fileIter->includeFolders) { continue; }
						SetOptionalOutPntr(isFolderOut, true);
					}
					else if (IsFlagSet(statStruct.st_mode, S_IFREG))
					{
						if (!fileIter->includeFiles) { continue; }
						SetOptionalOutPntr(isFolderOut, false);
					}
					else
					{
						PrintLine_W("Unknown file type for \"%.*s\"", StrPrint(fullPath));
						continue;
					}
				}
			}
			
			if (pathOut != nullptr)
			{
				*pathOut = AllocFilePath(pathOutArena, giveFullPath ? fullPath : fileName, false);
				NotNullStr(*pathOut);
			}
			fileIter->nextIndex = fileIter->index+1;
			return true;
		}
		Assert(false); //Shouldn't be possible to get here
		ScratchEnd(scratch);
	}
	#else
	UNUSED(fileIter);
	UNUSED(isFolderOut);
	UNUSED(pathOut);
	UNUSED(pathOutArena);
	UNUSED(giveFullPath);
	AssertMsg(false, "OsIterFileStep does not support the current platform yet!");
	fileIter->finished = true;
	#endif
	
	return false;
}
PEXPI bool OsIterFileStep(OsFileIter* fileIter, FilePath* pathOut, Arena* pathOutArena, bool giveFullPath)
{
	return OsIterFileStepEx(fileIter, nullptr, pathOut, pathOutArena, giveFullPath);
}

// +--------------------------------------------------------------+
// |                       Read Entire File                       |
// +--------------------------------------------------------------+
//NOTE: Passing nullptr for arena will output a Str8 that has length set but no chars pointer
// NOTE: The contentsOut is always null-terminated
//TODO: Convert this to return Result!
PEXP bool OsReadFile(FilePath path, Arena* arena, bool convertNewLines, Slice* contentsOut)
{
	//NOTE: This function should be multi-thread safe!
	NotNullStr(path);
	NotNull(contentsOut);
	ClearPointer(contentsOut);
	ScratchBegin1(scratch, arena);
	
	#if TARGET_IS_WINDOWS
	{
		Str8 fullPath = OsGetFullPath(scratch, path); //ensures null-termination
		ChangePathSlashesTo(fullPath, '\\');
		
		//TODO: This check maybe takes a lot of time? Should we just attempt to open the file and be done with it?
		// if (!OsDoesFileExist(fullPath)) { ScratchEnd(scratch); return false; }
		
		HANDLE fileHandle = CreateFileA(
			fullPath.chars,        //lpFileName
			GENERIC_READ,          //dwDesiredAccess
			FILE_SHARE_READ,       //dwShareMode
			NULL,                  //lpSecurityAttributes (NULL: no sub process access)
			OPEN_EXISTING,         //dwCreationDisposition
			FILE_ATTRIBUTE_NORMAL, //dwFlagsAndAttributes
			NULL                   //hTemplateFile
		);
		if (fileHandle == INVALID_HANDLE_VALUE)
		{
			DWORD errorCode = GetLastError();
			if (errorCode == ERROR_FILE_NOT_FOUND)
			{
				PrintLine_E("ERROR: File not found at \"%.*s\". Error code: %s", StrPrint(fullPath), Win32_GetErrorCodeStr(errorCode));
			}
			else
			{
				//The file might have permissions that prevent us from reading it
				PrintLine_E("ERROR: Failed to open file that exists at \"%.*s\". Error code: %s", StrPrint(fullPath), Win32_GetErrorCodeStr(errorCode));
			}
			ScratchEnd(scratch);
			CloseHandle(fileHandle);
			return false;
		}
		
		LARGE_INTEGER fileSizeLargeInt;
		BOOL getFileSizeResult = GetFileSizeEx(
			fileHandle,       //hFile
			&fileSizeLargeInt //lpFileSize
		);
		if (getFileSizeResult == 0)
		{
			DWORD errorCode = GetLastError();
			PrintLine_E("ERROR: Failed to size of file at \"%.*s\". Error code: %s", StrPrint(fullPath), Win32_GetErrorCodeStr(errorCode));
			ScratchEnd(scratch);
			CloseHandle(fileHandle);
			return false;
		}
		
		Assert(fileSizeLargeInt.QuadPart <= UINTXX_MAX);
		uxx fileSize = (uxx)fileSizeLargeInt.QuadPart;
		u8* fileData = (u8*)AllocMem(arena, fileSize+1); //+1 for null-term
		AssertMsg(fileData != nullptr, "Failed to allocate space to hold file contents. The application probably tried to open a massive file");
		Slice result = NewStr8(fileSize, fileData);
		
		if (fileSize > 0)
		{
			//TODO: What about files that are larger than DWORD_MAX? Will we just fail to read these?
			DWORD bytesRead = 0;
			BOOL readFileResult = ReadFile(
				fileHandle,           //hFile
				result.chars,         //lpBuffer
				(DWORD)result.length, //nNumberOfBytesToRead
				&bytesRead,           //lpNumberOfBytesRead
				NULL                  //lpOverlapped
			);
			if (readFileResult == 0)
			{
				DWORD errorCode = GetLastError();
				PrintLine_E("ERROR: Failed to ReadFile contents at \"%.*s\". Error code: %s", StrPrint(fullPath), Win32_GetErrorCodeStr(errorCode));
				FreeStr8WithNt(arena, &result);
				ScratchEnd(scratch);
				CloseHandle(fileHandle);
				return false;
			}
			if (bytesRead < result.length)
			{
				DWORD errorCode = GetLastError();
				PrintLine_E("ERROR: Failed to read all of the file at \"%.*s\". Error code: %s. Read %u/%llu bytes", StrPrint(fullPath), Win32_GetErrorCodeStr(errorCode), bytesRead, result.length);
				FreeStr8WithNt(arena, &result);
				ScratchEnd(scratch);
				CloseHandle(fileHandle);
				return false;
			}
			
			result.chars[result.length] = '\0'; //add null-term
			
			if (convertNewLines)
			{
				Str8 replacedResult = StrReplace(arena, result, StrLit("\r\n"), StrLit("\n"), true);
				if (CanArenaFree(arena)) { FreeStr8WithNt(arena, &result); }
				result = replacedResult;
			}
		}
		else { result.chars[result.length] = '\0'; } //add null-term
		
		*contentsOut = result;
		
		CloseHandle(fileHandle);
	}
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX)
	{
		Str8 fullPath = OsGetFullPath(scratch, path); //ensures null-termination
		FILE* fileHandle = fopen(fullPath.chars, "r");
		if (fileHandle == nullptr)
		{
			//TODO: Should we read the last error code from somewhere?
			PrintLine_E("ERROR: File not found at \"%.*s\"", StrPrint(fullPath));
			ScratchEnd(scratch);
			return false;
		}
		
		fseek(fileHandle, 0, SEEK_END);
		int fileSizeInt = ftell(fileHandle);
		DebugAssert(fileSizeInt >= 0);
		if ((unsigned int)fileSizeInt > (unsigned int)UINTXX_MAX)
		{
			PrintLine_E("File is %d bytes, too big for uxx type!", fileSizeInt);
			fclose(fileHandle);
			ScratchEnd(scratch);
			return false;
		}
		rewind(fileHandle);
		
		contentsOut->length = (uxx)fileSizeInt;
		contentsOut->chars = (char*)AllocMem(convertNewLines ? scratch : arena, contentsOut->length+1);
		AssertMsg(contentsOut->chars != nullptr, "Failed to allocate space to hold file contents. The application probably tried to open a massive file");
		
		size_t readResult = fread(
			contentsOut->chars, //buffer
			1, //size
			(size_t)contentsOut->length, //count
			fileHandle //stream
		);
		DebugAssert(readResult >= 0);
		Assert((uxx)readResult <= contentsOut->length);
		if ((uxx)readResult < contentsOut->length)
		{
			PrintLine_E("Read %llu/%llu bytes! Did fseek(SEEK_END) and ftell() lie to us about the file size?", (u64)readResult, (u64)contentsOut->length);
			fclose(fileHandle);
			if (CanArenaFree(arena) && !convertNewLines) { FreeMem(arena, contentsOut->chars, contentsOut->length+1); }
			ScratchEnd(scratch);
			return false;
		}
		
		contentsOut->chars[contentsOut->length] = '\0';
		
		if (convertNewLines)
		{
			*contentsOut = StrReplace(arena, *contentsOut, StrLit("\r\n"), StrLit("\n"), true);
		}
	}
	#else
	UNUSED(path);
	UNUSED(arena);
	UNUSED(convertNewLines);
	UNUSED(contentsOut);
	AssertMsg(false, "OsReadFile does not support the current platform yet!");
	#endif
	
	ScratchEnd(scratch);
	return true;
}
PEXPI bool OsReadTextFile(FilePath path, Arena* arena, Str8* contentsOut) { return OsReadFile(path, arena, true, contentsOut); }
PEXPI bool OsReadBinFile(FilePath path, Arena* arena, Slice* contentsOut) { return OsReadFile(path, arena, false, contentsOut); }

PEXPI Slice OsReadFileScratch(FilePath path, bool convertNewLines)
{
	Arena* scratch = GetScratch(nullptr); //Intentionally throwing away the mark here
	Slice fileContents = Slice_Empty;
	bool readSuccess = OsReadFile(path, scratch, convertNewLines, &fileContents);
	// Assert(readSuccess); //NOTE: Enable me if you want to break on failure to read!
	return (readSuccess ? fileContents : Slice_Empty);
}
PEXPI Str8 OsReadTextFileScratch(FilePath path) { return OsReadFileScratch(path, true); }
PEXPI Slice OsReadBinFileScratch(FilePath path) { return OsReadFileScratch(path, false); }

//TODO: Can we do some sort of asynchronous file read? Like kick off the read and get a callback later?

// +--------------------------------------------------------------+
// |                      Write Entire File                       |
// +--------------------------------------------------------------+
//TODO: Convert this to return Result!
//NOTE: If convertNewLines is true, you should not be passing \r\n instances in your file contents, only \n
PEXP bool OsWriteFile(FilePath path, Str8 fileContents, bool convertNewLines)
{
	NotNullStr(path);
	NotNullStr(fileContents);
	bool result = false;
	
	#if TARGET_IS_WINDOWS
	{
		ScratchBegin(scratch);
		
		if (convertNewLines)
		{
			//TODO: If the contents already have \r\n instances, then this will not work.
			//      Maybe we should use a bit more complicated logic than StrReplace?
			fileContents = StrReplace(scratch, fileContents, StrLit("\n"), StrLit("\r\n"), false);
			NotNullStr(fileContents);
		}
		
		Str8 fullPath = OsGetFullPath(scratch, path);
		NotNullStr(fullPath);
		
		HANDLE fileHandle = CreateFileA(
			fullPath.chars,        //Name of the file
			GENERIC_WRITE,         //Open for writing
			0,                     //Do not share
			NULL,                  //Default security
			CREATE_ALWAYS,         //Always overwrite
			FILE_ATTRIBUTE_NORMAL, //Default file attributes
			0                      //No Template File
		);
		if (fileHandle != INVALID_HANDLE_VALUE)
		{
			//TODO: Should we assert if fileContents.length > max value of DWORD?
			DWORD bytesWritten = 0;
			BOOL writeResult = TRUE;
			if (fileContents.length > 0)
			{
				writeResult = WriteFile(
					fileHandle, //hFile
					fileContents.chars, //lpBuffer
					(DWORD)fileContents.length, //nNumberOfBytesToWrite
					&bytesWritten, //lpNumberOfBytesWritten
					0 //lpOverlapped
				);
			}
			if (writeResult == TRUE)
			{
				if ((uxx)bytesWritten == fileContents.length)
				{
					result = true;
				}
				else
				{
					PrintLine_E("WARNING: Only wrote %u/%llu bytes to file at \"%.*s\"", bytesWritten, fileContents.length, StrPrint(fullPath));
				}
			}
			else
			{
				PrintLine_E("ERROR: Failed to write %llu bytes to file at \"%.*s\"", fileContents.length, StrPrint(fullPath));
			}
			CloseHandle(fileHandle);
		}
		else
		{
			PrintLine_E("ERROR: Failed to open file for writing at \"%.*s\"", StrPrint(fullPath));
		}
		ScratchEnd(scratch);
	}
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX)
	{
		ScratchBegin(scratch);
		//NOTE: When writing files on linux we don't need to convert new-lines. We usually working with strings in memory using \n which is the line-ending style of Linux.
		//      On Windows we convert on load and convert back on save. On Linux we convert on load just to be safe (in-case we load a Windows file from Linux)
		UNUSED(convertNewLines);
		
		Str8 fullPath = OsGetFullPath(scratch, path); //ensures null-termination
		FILE* fileHandle = fopen(fullPath.chars, "w");
		if (fileHandle == nullptr)
		{
			PrintLine_E("ERROR: Failed to open file for writing at \"%.*s\"", StrPrint(fullPath));
			ScratchEnd(scratch);
			return false;
		}
		
		size_t writeResult = fwrite(
			fileContents.pntr, //ptr
			1, //size
			fileContents.length, //count
			fileHandle //stream
		);
		DebugAssert(writeResult >= 0);
		Assert((uxx)writeResult <= fileContents.length);
		if ((uxx)writeResult < fileContents.length)
		{
			PrintLine_E("ERROR: Wrote %llu/%llu bytes to \"%.*s\"", (u64)writeResult, (u64)fileContents.length, StrPrint(fullPath));
			ScratchEnd(scratch);
			fclose(fileHandle);
			return false;
		}
		
		fclose(fileHandle);
		ScratchEnd(scratch);
		result = true;
	}
	#else
	UNUSED(path);
	UNUSED(fileContents);
	UNUSED(convertNewLines);
	AssertMsg(false, "OsWriteFile does not support the current platform yet!");
	#endif
	
	return result;
}
PEXPI bool OsWriteTextFile(FilePath path, Str8 fileContents) { return OsWriteFile(path, fileContents, true); }
PEXPI bool OsWriteBinFile(FilePath path, Str8 fileContents) { return OsWriteFile(path, fileContents, false); }

//TODO: Can we do some sort of asynchronous file write function? Like a fire and forget style thing?
//      This would probably require the code to know if an operation on that file is already in
//      progress, otherwise we may run into conflicts with order of operations

PEXPI bool OsCopyFile(FilePath fromPath, FilePath toPath)
{
	ScratchBegin(scratch);
	Slice fileContents = Slice_Empty;
	if (!OsReadBinFile(fromPath, scratch, &fileContents)) { ScratchEnd(scratch); return false; }
	if (!OsWriteBinFile(toPath, fileContents)) { ScratchEnd(scratch); return false; }
	return true;
}

// +--------------------------------------------------------------+
// |                        Create Folders                        |
// +--------------------------------------------------------------+
PEXP Result OsCreateFolder(FilePath path, bool createParentFoldersIfNeeded)
{
	Result result = Result_None;
	
	ScratchBegin(scratch);
	uxx numPathParts = CountPathParts(path, false);
	if (createParentFoldersIfNeeded && numPathParts > 1)
	{
		for (uxx pIndex = 0; pIndex < numPathParts; pIndex++)
		{
			Str8 pathPart = GetPathPart(path, pIndex, false);
			NotEmptyStr(pathPart);
			Assert(IsSizedPntrWithin(path.chars, path.length, pathPart.chars, pathPart.length));
			uxx partEndIndex = (uxx)((pathPart.chars + pathPart.length) - path.chars);
			Str8 partialPath = StrSlice(path, 0, partEndIndex);
			if (!OsDoesFolderExist(partialPath))
			{
				Result createResult = OsCreateFolder(partialPath, false);
				if (createResult != Result_Success) { ScratchEnd(scratch); return createResult; }
			}
		}
	}
	
	#if TARGET_IS_WINDOWS
	{
		if (!OsDoesFolderExist(path))
		{
			Str8 pathNt = AllocStrAndCopy(scratch, path.length, path.chars, true);
			BOOL createResult = CreateDirectoryA(
				pathNt.chars, //lpPathName
				NULL //lpSecurityAttributes
			);
			if (createResult != TRUE)
			{
				ScratchEnd(scratch);
				return Result_Failure;
			}
		}
		
		result = Result_Success;
	}
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX)
	{
		if (!OsDoesFolderExist(path))
		{
			Str8 pathNt = AllocStrAndCopy(scratch, path.length, path.chars, true);
			int mkdirResult = mkdir(pathNt.chars, S_IRWXU|S_IRWXG|S_IRWXO);
			if (mkdirResult != 0)
			{
				ScratchEnd(scratch);
				return Result_Failure; //TODO: Should we produce a better error code using errno?
			}
		}
		
		result = Result_Success;
	}
	#else
	AssertMsg(false, "OsCreateFolder does not support the current platform yet!");
	#endif
	
	ScratchEnd(scratch);
	return result;
}

// +--------------------------------------------------------------+
// |                Open File For Reading/Writing                 |
// +--------------------------------------------------------------+
PEXP void OsCloseFile(OsFile* file)
{
	NotNull(file);
	#if TARGET_IS_WINDOWS
	{
		if (file->handle != INVALID_HANDLE_VALUE) { CloseHandle(file->handle); }
		if (file->arena != nullptr && CanArenaFree(file->arena))
		{
			FreeFilePathWithNt(file->arena, &file->path);
			FreeFilePathWithNt(file->arena, &file->fullPath);
		}
		ClearPointer(file);
	}
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX)
	{
		if (file->handle != nullptr)
		{
			int closeResult = fclose(file->handle);
			Assert(closeResult == 0);
		}
		if (file->arena != nullptr && CanArenaFree(file->arena))
		{
			FreeFilePathWithNt(file->arena, &file->path);
			FreeFilePathWithNt(file->arena, &file->fullPath);
		}
		ClearPointer(file);
	}
	#else
	AssertMsg(false, "OsCloseFile does not support the current platform yet!");
	#endif
}

//TODO: Convert this to return Result!
PEXP bool OsOpenFile(Arena* arena, FilePath path, OsOpenFileMode mode, bool calculateSize, OsFile* openFileOut)
{
	NotNullStr(path);
	NotNull(openFileOut);
	bool result = false;
	
	#if TARGET_IS_WINDOWS
	{
		ScratchBegin1(scratch, arena);
		Str8 fullPath = OsGetFullPath(scratch, path);
		
		DWORD desiredAccess = GENERIC_READ;
		if (mode != OsOpenFileMode_Read) { desiredAccess |= GENERIC_WRITE; }
		
		DWORD shareMode = 0;
		if (mode == OsOpenFileMode_Read) { shareMode |= FILE_SHARE_READ; }
		
		DWORD creationDisposition = OPEN_EXISTING;
		if (mode == OsOpenFileMode_Append) { creationDisposition = OPEN_ALWAYS; }
		else if (mode == OsOpenFileMode_Write) { creationDisposition = CREATE_ALWAYS; }
		else if (mode == OsOpenFileMode_Create) { creationDisposition = CREATE_NEW; }
		
		DWORD flagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
		
		//TODO: We should convert the path to UCS2 and call CreateFileW!
		HANDLE fileHandle = CreateFileA(
			fullPath.pntr,       //lpFileName
			desiredAccess,       //dwDesiredAccess
			shareMode,           //dwShareMode
			NULL,                //lpSecurityAttributes
			creationDisposition, //dwCreationDisposition
			flagsAndAttributes,  //dwFlagsAndAttributes
			NULL                 //hTemplateFile
		);
		if (fileHandle == INVALID_HANDLE_VALUE)
		{
			PrintLine_E("ERROR: Failed to open file for %s at \"%.*s\"", GetOsOpenFileModeStr(mode), StrPrint(fullPath));
			ScratchEnd(scratch);
			return false;
		}
		
		uxx fileSize = 0;
		uxx cursorIndex = 0;
		if (calculateSize && (mode == OsOpenFileMode_Read || mode == OsOpenFileMode_Append))
		{
			//Seek to the end of the file
			LONG newCursorPosHighOrder = 0;
			DWORD newCursorPos = SetFilePointer(
				fileHandle,             //hFile
				0,                      //lDistanceToMove
				&newCursorPosHighOrder, //lDistanceToMoveHigh
				FILE_END                //dMoveMethod
			);
			if (newCursorPos == INVALID_SET_FILE_POINTER)
			{
				PrintLine_E("ERROR: Failed to seek to the end of the file when opened for %s \"%.*s\"!", GetOsOpenFileModeStr(mode), StrPrint(fullPath));
				CloseHandle(fileHandle);
				ScratchEnd(scratch);
				return false;
			}
			#if TARGET_IS_32BIT
			Assert(newCursorPosHighOrder == 0); //We can't handle 64-bit cursor position with uxx
			fileSize = (uxx)newCursorPos;
			#else
			fileSize = (((uxx)newCursorPosHighOrder << 32) | (uxx)newCursorPos);
			#endif
			cursorIndex = fileSize;
			
			if (mode != OsOpenFileMode_Append)
			{
				//Seek back to the beginning
				DWORD beginMove = SetFilePointer(
					fileHandle, //hFile
					0, //lDistanceToMove,
					NULL, //lDistanceToMoveHigh
					FILE_BEGIN
				);
				Assert(beginMove != INVALID_SET_FILE_POINTER);
				cursorIndex = 0;
			}
		}
		
		ScratchEnd(scratch);
		
		ClearPointer(openFileOut);
		openFileOut->arena = arena;
		openFileOut->isOpen = true;
		openFileOut->handle = fileHandle;
		openFileOut->openedForWriting = (mode != OsOpenFileMode_Read);
		openFileOut->isKnownSize = calculateSize;
		openFileOut->cursorIndex = cursorIndex;
		openFileOut->fileSize = fileSize;
		openFileOut->path = AllocFilePath(arena, path, true);
		NotNullStr(openFileOut->path);
		openFileOut->fullPath = AllocFilePath(arena, fullPath, true);
		NotNullStr(openFileOut->fullPath);
		result = true;
	}
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX)
	{
		ScratchBegin1(scratch, arena);
		Str8 fullPath = OsGetFullPath(scratch, path);
		
		const char* openModeStr = "r";
		switch (mode)
		{
			case OsOpenFileMode_Read:   openModeStr = "r"; break;
			case OsOpenFileMode_Create: openModeStr = "w"; break;
			case OsOpenFileMode_Write:  openModeStr = "r+"; break;
			case OsOpenFileMode_Append: openModeStr = "a"; break;
			default: AssertMsg(false, "Unhandled mode passed to OsOpenFile"); break;
		}
		
		FILE* fileHandle = fopen(fullPath.chars, openModeStr);
		if (fileHandle == nullptr)
		{
			PrintLine_E("ERROR: Failed to open file for %s at \"%.*s\"", GetOsOpenFileModeStr(mode), StrPrint(fullPath));
			ScratchEnd(scratch);
			return false;
		}
		
		openFileOut->handle = fileHandle;
		openFileOut->arena = arena;
		openFileOut->isOpen = true;
		openFileOut->openedForWriting = (mode != OsOpenFileMode_Read);
		openFileOut->isKnownSize = (calculateSize || mode == OsOpenFileMode_Create || mode == OsOpenFileMode_Write);
		openFileOut->cursorIndex = 0;
		openFileOut->fileSize = 0;
		openFileOut->path = AllocStrAndCopy(arena, path.length, path.chars, true);
		openFileOut->fullPath = AllocStrAndCopy(arena, fullPath.length, fullPath.chars, true);
		
		if (calculateSize && (mode == OsOpenFileMode_Read || mode == OsOpenFileMode_Append))
		{
			int seekResult = fseek(fileHandle, 0, SEEK_END);
			Assert(seekResult == 0);
			long fileSize = ftell(fileHandle);
			Assert(fileSize >= 0);
			Assert((unsigned long)fileSize <= UINTXX_MAX);
			openFileOut->fileSize = (uxx)fileSize;
			
			if (mode == OsOpenFileMode_Read)
			{
				seekResult = fseek(fileHandle, 0, SEEK_SET); //back to the beginning of the file
				Assert(seekResult == 0);
			}
			else
			{
				openFileOut->cursorIndex = openFileOut->fileSize;
			}
		}
		
		ScratchEnd(scratch);
		result = true;
	}
	#else
	UNUSED(arena);
	UNUSED(path);
	UNUSED(mode);
	UNUSED(calculateSize);
	UNUSED(openFileOut);
	AssertMsg(false, "OsOpenFile does not support the current platform yet!");
	#endif
	
	return result;
}

PEXP Result OsReadFromOpenFile(OsFile* file, uxx numBytes, bool convertNewLines, void* bufferOut, uxx* numBytesReadOut)
{
	NotNull(file);
	NotNull(file->arena);
	NotNull(bufferOut);
	Result result = Result_None;
	
	#if TARGET_IS_WINDOWS
	{
		Assert(!file->isKnownSize || file->cursorIndex <= file->fileSize);
		//TODO: Assert numBytes <= max of DWORD
		DWORD numBytesToRead = (DWORD)numBytes;
		if (file->isKnownSize && file->cursorIndex + numBytes > file->fileSize)
		{
			numBytesToRead = (DWORD)(file->fileSize - file->cursorIndex);
		}
		
		DWORD numBytesRead = 0;
		if (numBytesToRead > 0)
		{
			BOOL readResult = ReadFile(
				file->handle, //hFile
				bufferOut, //lpBuffer
				numBytesToRead, //nNumberOfBytesToRead
				&numBytesRead, //lpNumberOfBytesRead
				NULL //lpOverlapped
			);
			if (readResult == 0)
			{
				// DWORD errorCode = GetLastError(); //TODO: Use this to fill with proper Result
				return Result_FailedToReadFile;
			}
			Assert(numBytesRead <= numBytesToRead);
		}
		
		if (numBytesRead == 0) { return Result_NoMoreBytes; }
		
		file->cursorIndex += (uxx)numBytesRead;
		bool partialRead = ((uxx)numBytesRead < numBytes);
		SetOptionalOutPntr(numBytesReadOut, (uxx)numBytesRead);
		
		if (convertNewLines)
		{
			ScratchBegin(scratch);
			Str8 scratchStr = StrReplace(scratch, NewStr8((uxx)numBytesRead, bufferOut), StrLit("\r\n"), StrLit("\n"), false);
			DebugAssert(scratchStr.length <= (uxx)numBytesRead);
			MyMemCopy(bufferOut, scratchStr.bytes, scratchStr.length);
			SetOptionalOutPntr(numBytesReadOut, scratchStr.length);
			ScratchEnd(scratch);
		}
		
		result = (partialRead ? Result_Partial : Result_Success);
	}
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX)
	{
		Assert(file->isOpen && file->handle != nullptr);
		Assert(!file->isKnownSize || file->cursorIndex <= file->fileSize);
		
		//TODO: Assert numBytes <= max of size_t
		size_t numBytesToRead = (size_t)numBytes;
		if (file->isKnownSize && file->cursorIndex + numBytes > file->fileSize)
		{
			numBytesToRead = (size_t)(file->fileSize - file->cursorIndex);
		}
		
		size_t readResult = fread(
			bufferOut, //buffer
			1, //size
			numBytesToRead, //count
			file->handle //stream
		);
		Assert(readResult <= numBytesToRead);
		
		file->cursorIndex += (uxx)readResult;
		bool partialRead = ((uxx)readResult < numBytes);
		SetOptionalOutPntr(numBytesReadOut, (uxx)readResult);
		
		if (readResult == 0 && ferror(file->handle) != 0)
		{
			return Result_FailedToReadFile;
		}
		
		//NOTE: When reading files on non-Windows environments, we expect the new-line pattern to be \n so there is nothing we need to do
		//TODO: Are there any contexts where the calling code knows the line-endings will be Windows-style and need conversion?
		UNUSED(convertNewLines);
		
		result = partialRead ? Result_Partial : Result_Success;
	}
	#else
	AssertMsg(false, "OsReadFromOpenFile does not support the current platform yet!");
	result = Result_UnsupportedPlatform;
	#endif
	
	return result;
}
PEXPI Result OsReadFromOpenTextFile(OsFile* file, uxx numBytes, void* bufferOut, uxx* numBytesReadOut) { return OsReadFromOpenFile(file, numBytes, true, bufferOut, numBytesReadOut); }
PEXPI Result OsReadFromOpenBinFile(OsFile* file, uxx numBytes, void* bufferOut, uxx* numBytesReadOut) { return OsReadFromOpenFile(file, numBytes, false, bufferOut, numBytesReadOut); }

//TODO: Convert this to return Result!
//NOTE: If convertNewLines is true, you should not be passing \r\n instances in your file contents, only \n
PEXP bool OsWriteToOpenFile(OsFile* file, Str8 fileContentsPart, bool convertNewLines)
{
	NotNull(file);
	NotNullStr(fileContentsPart);
	if (!file->isOpen) { return false; }
	Assert(file->openedForWriting);
	if (fileContentsPart.length == 0) { return true; } //no bytes to write always succeeds
	bool result = false;
	
	#if TARGET_IS_WINDOWS
	{
		ScratchBegin(scratch);
		Assert(file->handle != INVALID_HANDLE_VALUE);
		
		if (convertNewLines)
		{
			fileContentsPart = StrReplace(scratch, fileContentsPart, StrLit("\n"), StrLit("\r\n"), false);
			NotNullStr(fileContentsPart);
		}
		
		Assert(fileContentsPart.length <= UINT32_MAX); //TODO: Is there a DWORD_MAX constant?
		DWORD numBytesToWrite = (DWORD)fileContentsPart.length;
		DWORD numBytesWritten = 0;
		BOOL writeResult = WriteFile(
			file->handle,          //hFile
			fileContentsPart.pntr, //lpBuffer
			numBytesToWrite,       //nNumberOfBytesToWrite
			&numBytesWritten,      //lpNumberOfnumBytesWritten
			NULL                   //lpOverlapped
		);
		if (writeResult == 0)
		{
			DWORD errorCode = GetLastError();
			PrintLine_E("ERROR: WriteFile failed: 0x%08X (%u)", errorCode, errorCode);
			ScratchEnd(scratch);
			return false;
		}
		
		file->cursorIndex += (uxx)numBytesWritten;
		file->fileSize += (uxx)numBytesWritten;
		
		Assert(numBytesWritten <= numBytesToWrite);
		if (numBytesWritten < numBytesToWrite)
		{
			PrintLine_E("ERROR: Partial write occurred: %u/%u", numBytesWritten, numBytesToWrite);
			ScratchEnd(scratch);
			return false;
		}
		
		result = true;
		ScratchEnd(scratch);
	}
	#elif (TARGET_IS_LINUX || TARGET_IS_OSX)
	{
		Assert(file->handle != nullptr);
		//NOTE: When writing to file on non-Windows environments, we don't actually need to convert new-lines to \r\n
		UNUSED(convertNewLines);
		
		size_t writeResult = fwrite(
			fileContentsPart.pntr, //ptr
			1, //size
			(size_t)fileContentsPart.length, //count
			file->handle //stream
		);
		Assert((uxx)writeResult <= fileContentsPart.length);
		file->cursorIndex += writeResult;
		if (file->isKnownSize) { file->fileSize += writeResult; } //TODO: If we write into the middle of a file does it insert or overwrite?
		
		if (writeResult == 0)
		{
			WriteLine_E("ERROR: fwrite failed!");
			return false;
		}
		if ((uxx)writeResult < fileContentsPart.length)
		{
			PrintLine_E("ERROR: Wrote %llu/%llu bytes to \"%.*s\"", (u64)writeResult, (u64)fileContentsPart.length, StrPrint(file->fullPath));
			return false;
		}
		
		result = true;
	}
	#else
	UNUSED(file);
	UNUSED(fileContentsPart);
	UNUSED(convertNewLines);
	AssertMsg(false, "OsWriteToOpenFile does not support the current platform yet!");
	#endif
	
	return result;
}
PEXPI bool OsWriteToOpenTextFile(OsFile* file, Str8 fileContentsPart) { return OsWriteToOpenFile(file, fileContentsPart, true); }
PEXPI bool OsWriteToOpenBinFile(OsFile* file, Str8 fileContentsPart) { return OsWriteToOpenFile(file, fileContentsPart, false); }

//TODO: Implement OsMoveFileCursorRelative
//TODO: Implement OsMoveFileCursor

// +--------------------------------------------------------------+
// |                       File Write Time                        |
// +--------------------------------------------------------------+
PEXP Result OsGetFileWriteTime(FilePath filePath, OsFileWriteTime* timeOut)
{
	NotNullStr(filePath);
	Result result = Result_None;
	#if TARGET_IS_WINDOWS
	{
		ScratchBegin(scratch);
		Str8 filePathNt = AllocStrAndCopy(scratch, filePath.length, filePath.chars, true);
		NotNull(filePathNt.chars);
		WIN32_FILE_ATTRIBUTE_DATA attributeData;
		if (GetFileAttributesExA(filePathNt.chars, GetFileExInfoStandard, &attributeData))
		{
			if (timeOut != nullptr) { timeOut->fileTime = attributeData.ftLastWriteTime; }
			result = Result_Success;
		}
		else { result = Result_NotFound; }
		ScratchEnd(scratch);
	}
	#elif TARGET_IS_LINUX
	{
		ScratchBegin(scratch);
		Str8 filePathNt = AllocStrAndCopy(scratch, filePath.length, filePath.chars, true);
		plex stat statStruct = ZEROED;
		int statResult = stat(filePathNt.chars, &statStruct);
		ScratchEnd(scratch);
		if (statResult != 0) { return Result_NotFound; }
		if (timeOut != nullptr)
		{
			ClearPointer(timeOut);
			timeOut->timeSpec = statStruct.st_mtim;
		}
		result = Result_Success;
	}
	#else
	AssertMsg(false, "OsGetFileWriteTime does not support the current platform yet!");
	result = Result_UnsupportedPlatform;
	#endif
	return result;
}

PEXPI i32 OsCompareFileWriteTime(OsFileWriteTime left, OsFileWriteTime right)
{
	#if TARGET_IS_WINDOWS
	{
		ULARGE_INTEGER leftTime = ZEROED;
		leftTime.u.HighPart = left.fileTime.dwHighDateTime;
		leftTime.u.LowPart = left.fileTime.dwLowDateTime;
		ULARGE_INTEGER rightTime = ZEROED;
		rightTime.u.HighPart = right.fileTime.dwHighDateTime;
		rightTime.u.LowPart = right.fileTime.dwLowDateTime;
		if (leftTime.QuadPart > rightTime.QuadPart) { return 1; }
		else if (leftTime.QuadPart < rightTime.QuadPart) { return -1; }
		else { return 0; }
	}
	#elif TARGET_IS_LINUX
	{
		if (left.timeSpec.tv_sec > right.timeSpec.tv_sec) { return 1; }
		else if (left.timeSpec.tv_sec < right.timeSpec.tv_sec) { return -1; }
		{
			if (left.timeSpec.tv_nsec > right.timeSpec.tv_nsec) { return 1; }
			else if (left.timeSpec.tv_nsec < right.timeSpec.tv_nsec) { return 1; }
			else { return 0; }
		}
	}
	#else
	AssertMsg(false, "OsCompareFileWriteTime does not support the current platform yet!");
	return 0;
	#endif
}
PEXPI bool OsAreFileWriteTimesEqual(OsFileWriteTime left, OsFileWriteTime right)
{
	return (OsCompareFileWriteTime(left, right) == 0);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _OS_FILE_H

#if defined(_OS_FILE_H) && defined(_MISC_RANDOM_H)
#include "cross/cross_file_and_random.h"
#endif

#if defined(_GFX_IMAGE_LOADING_H) && defined(_OS_FILE_H)
#include "cross/cross_image_loading_and_file.h"
#endif

#if defined(_MISC_ZIP_H) && defined(_OS_FILE_H)
#include "cross/cross_zip_and_file.h"
#endif

#if defined(_FILE_FMT_GLTF_H) && defined(_OS_FILE_H)
#include "cross/cross_gltf_and_os_file.h"
#endif
