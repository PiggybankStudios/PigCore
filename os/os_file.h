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
#include "base/base_typedefs.h"
#include "std/std_includes.h"
#include "struct/struct_string.h"
#include "mem/mem_scratch.h"
#include "os/os_error.h"
#include "misc/misc_result.h"

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

typedef enum OsOpenFileMode OsOpenFileMode;
enum OsOpenFileMode
{
	OsOpenFileMode_None = 0,
	OsOpenFileMode_Read,
	OsOpenFileMode_Write,
	OsOpenFileMode_Append,
	OsOpenFileMode_Count,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetOsOpenFileModeStr(OsOpenFileMode enumValue);
#else
PEXP const char* GetOsOpenFileModeStr(OsOpenFileMode enumValue)
{
	switch (enumValue)
	{
		case OsOpenFileMode_None:   return "None";
		case OsOpenFileMode_Read:   return "Read";
		case OsOpenFileMode_Write:  return "Write";
		case OsOpenFileMode_Append: return "Append";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef struct OsFile OsFile;
struct OsFile
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
	#endif
};

//NOTE: The name "dll" is a misnomer on non-windows platforms. This can represent a .dll or a .so
typedef struct OsDll OsDll;
struct OsDll
{
	#if TARGET_IS_WINDOWS
	HMODULE handle;
	#elif TARGET_IS_LINUX
	void* handle;
	#else
	int placeholder;
	#endif
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	FilePath OsGetFullPath(Arena* arena, FilePath relativePath);
	PIG_CORE_INLINE uxx OsGetFullPathLength(FilePath relativePath);
	bool OsDoesFileOrFolderExist(FilePath path, bool* isFolderOut);
	PIG_CORE_INLINE bool OsDoesFileExist(FilePath path);
	PIG_CORE_INLINE bool OsDoesFolderExist(FilePath path);
	PIG_CORE_INLINE void OsFreeFileIter(OsFileIter* fileIter);
	OsFileIter OsIterateFiles(Arena* arena, FilePath path, bool includeFiles, bool includeFolders);
	bool OsIterFileStepEx(OsFileIter* fileIter, bool* isFolderOut, FilePath* pathOut, Arena* pathOutArena, bool giveFullPath);
	PIG_CORE_INLINE bool OsIterFileStep(OsFileIter* fileIter, FilePath* pathOut, Arena* pathOutArena, bool giveFullPath);
	bool OsReadFile(FilePath path, Arena* arena, bool convertNewLines, Str8* contentsOut);
	PIG_CORE_INLINE Str8 OsReadTextFileScratch(FilePath path);
	PIG_CORE_INLINE Str8 OsReadBinFileScratch(FilePath path);
	bool OsWriteFile(FilePath path, Str8 fileContents, bool convertNewLines);
	PIG_CORE_INLINE bool OsWriteTextFile(FilePath path, Str8 fileContents);
	PIG_CORE_INLINE bool OsWriteBinFile(FilePath path, Str8 fileContents);
	void OsCloseFile(OsFile* file);
	bool OsOpenFile(Arena* arena, FilePath path, OsOpenFileMode mode, bool calculateSize, OsFile* openFileOut);
	bool OsWriteToOpenFile(OsFile* file, Str8 fileContentsPart, bool convertNewLines);
	PIG_CORE_INLINE bool OsWriteToOpenTextFile(OsFile* file, Str8 fileContentsPart);
	PIG_CORE_INLINE bool OsWriteToOpenBinFile(OsFile* file, Str8 fileContentsPart);
	Result OsLoadDll(FilePath path, OsDll* dllOut);
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
	UNUSED(path);
	UNUSED(isFolderOut);
	AssertMsg(false, "OsDoesFileOrFolderExist does not support the current platform yet!");
	return false;
	#endif
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
	if (fileIter->folderPath.chars != nullptr) { FreeStr8WithNt(fileIter->arena, &fileIter->folderPath); }
	if (fileIter->folderPathWithWildcard.chars != nullptr) { FreeStr8WithNt(fileIter->arena, &fileIter->folderPathWithWildcard); }
	ClearPointer(fileIter);
}

PEXP OsFileIter OsIterateFiles(Arena* arena, FilePath path, bool includeFiles, bool includeFolders)
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
		result.folderPathWithWildcard = JoinStringsInArena(arena, result.folderPath, StrLit("*"), true);
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
	UNUSED(arena);
	UNUSED(path);
	UNUSED(includeFiles);
	UNUSED(includeFolders);
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
	// #elif TARGET_IS_LINUX
	// {
	// 	//TODO: Implement me!
	// }
	// #elif TARGET_IS_OSX
	// {
	// 	//TODO: Implement me!
	// }
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
//TODO: Return Result instead of bool!
PEXP bool OsReadFile(FilePath path, Arena* arena, bool convertNewLines, Str8* contentsOut)
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
				MyPrint("ERROR: File not found at \"%.*s\". Error code: %s", StrPrint(fullPath), Win32_GetErrorCodeStr(errorCode));
			}
			else
			{
				//The file might have permissions that prevent us from reading it
				MyPrint("ERROR: Failed to open file that exists at \"%.*s\". Error code: %s", StrPrint(fullPath), Win32_GetErrorCodeStr(errorCode));
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
			MyPrint("ERROR: Failed to size of file at \"%.*s\". Error code: %s", StrPrint(fullPath), Win32_GetErrorCodeStr(errorCode));
			ScratchEnd(scratch);
			CloseHandle(fileHandle);
			return false;
		}
		
		Assert(fileSizeLargeInt.QuadPart <= UINTXX_MAX);
		uxx fileSize = (uxx)fileSizeLargeInt.QuadPart;
		u8* fileData = AllocArray(u8, arena, fileSize+1); //+1 for null-term
		AssertMsg(fileData != nullptr, "Failed to allocate space to hold file contents. The application probably tried to open a massive file");
		Str8 result = NewStr8(fileSize, (char*)fileData);
		
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
				MyPrint("ERROR: Failed to ReadFile contents at \"%.*s\". Error code: %s", StrPrint(fullPath), Win32_GetErrorCodeStr(errorCode));
				FreeStr8WithNt(arena, &result);
				ScratchEnd(scratch);
				CloseHandle(fileHandle);
				return false;
			}
			if (bytesRead < result.length)
			{
				DWORD errorCode = GetLastError();
				MyPrint("ERROR: Failed to read all of the file at \"%.*s\". Error code: %s. Read %u/%llu bytes", StrPrint(fullPath), Win32_GetErrorCodeStr(errorCode), bytesRead, result.length);
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
	// #elif TARGET_IS_LINUX
	// {
	// 	//TODO: Implement me!
	// }
	// #elif TARGET_IS_OSX
	// {
	// 	//TODO: Implement me!
	// }
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
PEXPI Str8 OsReadTextFileScratch(FilePath path)
{
	Arena* scratch = GetScratch(nullptr); //Intentionally throwing away the mark here
	Str8 fileContents = Str8_Empty;
	bool readSuccess = OsReadFile(path, scratch, true, &fileContents);
	// Assert(readSuccess); //NOTE: Enable me if you want to break on failure to read!
	return readSuccess ? fileContents : Str8_Empty;
}
PEXPI Str8 OsReadBinFileScratch(FilePath path)
{
	Arena* scratch = GetScratch(nullptr); //Intentionally throwing away the mark here
	Str8 fileContents = Str8_Empty;
	bool readSuccess = OsReadFile(path, scratch, false, &fileContents);
	// Assert(readSuccess); //NOTE: Enable me if you want to break on failure to read!
	return readSuccess ? fileContents : Str8_Empty;
}

//TODO: Can we do some sort of asynchronous file read? Like kick off the read and get a callback later?

// +--------------------------------------------------------------+
// |                      Write Entire File                       |
// +--------------------------------------------------------------+
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
					MyPrint("WARNING: Only wrote %u/%llu bytes to file at \"%.*s\"", bytesWritten, fileContents.length, StrPrint(fullPath));
				}
			}
			else
			{
				MyPrint("ERROR: Failed to write %llu bytes to file at \"%.*s\"", fileContents.length, StrPrint(fullPath));
			}
			CloseHandle(fileHandle);
		}
		else
		{
			MyPrint("ERROR: Failed to open file for writing at \"%.*s\"", StrPrint(fullPath));
		}
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

// +--------------------------------------------------------------+
// |                Open File For Reading/Writing                 |
// +--------------------------------------------------------------+
PEXP void OsCloseFile(OsFile* file)
{
	NotNull(file);
	if (file->arena != nullptr && CanArenaFree(file->arena))
	{
		FreeFilePathWithNt(file->arena, &file->path);
		FreeFilePathWithNt(file->arena, &file->fullPath);
	}
	ClearPointer(file);
}

PEXP bool OsOpenFile(Arena* arena, FilePath path, OsOpenFileMode mode, bool calculateSize, OsFile* openFileOut)
{
	NotNullStr(path);
	NotNull(openFileOut);
	bool result = false;
	
	#if TARGET_IS_WINDOWS
	{
		ScratchBegin1(scratch, arena);
		Str8 fullPath = OsGetFullPath(scratch, path);
		bool forWriteOrAppend = (mode == OsOpenFileMode_Write || mode == OsOpenFileMode_Append);
		bool forWriting = (mode == OsOpenFileMode_Write);
		
		HANDLE fileHandle = CreateFileA(
			fullPath.pntr,                                     //lpFileName
			(forWriteOrAppend ? GENERIC_WRITE : GENERIC_READ), //dwDesiredAccess
			(forWriteOrAppend ? 0 : FILE_SHARE_READ),          //dwShareMode
			NULL,                                              //lpSecurityAttributes
			(forWriting ? CREATE_ALWAYS : OPEN_ALWAYS),        //dwCreationDisposition
			FILE_ATTRIBUTE_NORMAL,                             //dwFlagsAndAttributes
			NULL                                               //hTemplateFile
		);
		if (fileHandle == INVALID_HANDLE_VALUE)
		{
			MyPrint("ERROR: Failed to %s file at \"%.*s\"", (forWriting ? "Create" : "Open"), StrPrint(fullPath));
			ScratchEnd(scratch);
			return false;
		}
		
		uxx fileSize = 0;
		uxx cursorIndex = 0;
		if (calculateSize)
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
				MyPrint("ERROR: Failed to seek to the end of the file when opened for %s \"%.*s\"!", GetOsOpenFileModeStr(mode), StrPrint(fullPath));
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
			
			if (!forWriteOrAppend)
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
		openFileOut->openedForWriting = forWriteOrAppend;
		openFileOut->isKnownSize = calculateSize;
		openFileOut->cursorIndex = cursorIndex;
		openFileOut->fileSize = fileSize;
		openFileOut->path = AllocFilePath(arena, path, true);
		NotNullStr(openFileOut->path);
		openFileOut->fullPath = AllocFilePath(arena, fullPath, true);
		NotNullStr(openFileOut->fullPath);
		result = true;
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
	UNUSED(arena);
	UNUSED(path);
	UNUSED(mode);
	UNUSED(calculateSize);
	UNUSED(openFileOut);
	AssertMsg(false, "OsOpenFile does not support the current platform yet!");
	#endif
	
	return result;
}

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
			MyPrint("ERROR: WriteFile failed: 0x%08X (%u)", errorCode, errorCode);
			ScratchEnd(scratch);
			return false;
		}
		
		file->cursorIndex += (uxx)numBytesWritten;
		file->fileSize += (uxx)numBytesWritten;
		
		Assert(numBytesWritten <= numBytesToWrite);
		if (numBytesWritten < numBytesToWrite)
		{
			MyPrint("ERROR: Partial write occurred: %u/%u", numBytesWritten, numBytesToWrite);
			ScratchEnd(scratch);
			return false;
		}
		
		result = true;
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
// |                         DLL Loading                          |
// +--------------------------------------------------------------+
PEXP Result OsLoadDll(FilePath path, OsDll* dllOut)
{
	NotNullStr(path);
	NotNull(dllOut);
	Result result = Result_None;
	
	#if TARGET_IS_WINDOWS
	{
		ScratchBegin(scratch);
		FilePath pathNt = AllocFilePath(scratch, path, true);
		ChangePathSlashesTo(pathNt, '\\');
		dllOut->handle = LoadLibraryA(pathNt.chars);
		if (dllOut->handle != NULL) { result = Result_Success; }
		else { result = Result_Failure; }
		ScratchEnd(scratch);
	}
	#elif TARGET_IS_LINUX
	{
		ScratchBegin(scratch);
		FilePath pathNt = AllocFilePath(scratch, path, true);
		ChangePathSlashesTo(pathNt, '/');
		dllOut->handle = dlopen(
			pathNt.chars, //file
			RTLD_NOW //mode TODO: Do we ever want RTLD_LAZY?
		);
		if (dllOut->handle != nullptr) { result = Result_Success; }
		else
		{
			char* errorStr = dlerror();
			PrintLine_E("dlopen failed: \"%s\"", errorStr);
			result = Result_Failure;
		}
		ScratchEnd(scratch);
	}
	#else
	UNUSED(path);
	AssertMsg(false, "OsLoadDll does not support the current platform yet!");
	result = Result_UnsupportedPlatform;
	#endif
	
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _OS_FILE_H
