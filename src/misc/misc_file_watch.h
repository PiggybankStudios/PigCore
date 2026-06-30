/*
File:   misc_file_watch.h
Author: Taylor Robbins
Date:   08\23\2025
Description:
	** Holds functions that help us manage a list of "File Watches"
	** which basically are just a recording of the last file write time
	** along with some info to help us decide how often to check the write
	** time and to handle the file existing or not
*/

#ifndef _MISC_FILE_WATCH_H
#define _MISC_FILE_WATCH_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "mem/mem_arena.h"
#include "struct/struct_string.h"
#include "os/os_file.h"

typedef enum OsFileWatchChange OsFileWatchChange;
enum OsFileWatchChange
{
	OsFileWatchChange_None = 0,
	OsFileWatchChange_Modified,
	OsFileWatchChange_Created,
	OsFileWatchChange_Deleted,
	OsFileWatchChange_Count,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetOsFileWatchChangeStr(OsFileWatchChange enumValue);
#else
PEXP const char* GetOsFileWatchChangeStr(OsFileWatchChange enumValue)
{
	switch (enumValue)
	{
		case OsFileWatchChange_None:     return "None";
		case OsFileWatchChange_Modified: return "Modified";
		case OsFileWatchChange_Created:  return "Created";
		case OsFileWatchChange_Deleted:  return "Deleted";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef plex OsFileWatch OsFileWatch;
plex OsFileWatch
{
	Arena* arena;
	uxx checkPeriod;
	FilePath path;
	FilePath fullPath;
	u64 lastCheck;
	
	bool fileExists;
	OsFileWriteTime writeTime;
	u64 lastChangeTime;
	OsFileWatchChange change; //check and clear this!
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void OsFreeFileWatch(OsFileWatch* watch);
	PIG_CORE_INLINE void OsInitFileWatch(Arena* arena, FilePath path, uxx checkPeriod, u64 programTime, OsFileWatch* watchOut);
	PIG_CORE_INLINE bool OsUpdateFileWatch(OsFileWatch* watch, u64 programTime);
	PIG_CORE_INLINE void OsResetFileWatch(OsFileWatch* watch, u64 programTime);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void OsFreeFileWatch(OsFileWatch* watch)
{
	NotNull(watch);
	if (watch->arena != nullptr)
	{
		FreeStr8(watch->arena, &watch->path);
		FreeStr8(watch->arena, &watch->fullPath);
	}
	ClearPointer(watch);
}

PEXPI void OsInitFileWatch(Arena* arena, FilePath path, uxx checkPeriod, u64 programTime, OsFileWatch* watchOut)
{
	NotNull(arena);
	NotNullStr(path);
	NotNull(watchOut);
	ClearPointer(watchOut);
	watchOut->arena = arena;
	watchOut->path = AllocStr8(arena, path);
	watchOut->fullPath = OsGetFullPath(arena, path);
	watchOut->checkPeriod = checkPeriod;
	watchOut->lastCheck = programTime;
	watchOut->fileExists = OsDoesFileExist(watchOut->fullPath);
	if (watchOut->fileExists)
	{
		Result getWriteTimeResult = OsGetFileWriteTime(watchOut->fullPath, &watchOut->writeTime);
		Assert(getWriteTimeResult == Result_Success);
	}
}

PEXPI bool OsUpdateFileWatch(OsFileWatch* watch, u64 programTime)
{
	NotNull(watch);
	bool fileChanged = false;
	if (watch->arena != nullptr)
	{
		if (watch->checkPeriod == 0 || TimeSinceBy(programTime, watch->lastCheck) >= watch->checkPeriod)
		{
			bool existsNow = OsDoesFileExist(watch->fullPath);
			if (existsNow != watch->fileExists)
			{
				watch->fileExists = existsNow;
				fileChanged = true;
				watch->change = (existsNow ? OsFileWatchChange_Created : OsFileWatchChange_Deleted);
				watch->lastChangeTime = programTime;
				if (existsNow)
				{
					Result getWriteTimeResult = OsGetFileWriteTime(watch->fullPath, &watch->writeTime);
					Assert(getWriteTimeResult == Result_Success);
				}
			}
			else if (watch->fileExists)
			{
				OsFileWriteTime newWriteTime;
				Result getWriteTimeResult = OsGetFileWriteTime(watch->fullPath, &newWriteTime);
				Assert(getWriteTimeResult == Result_Success);
				if (!OsAreFileWriteTimesEqual(watch->writeTime, newWriteTime))
				{
					fileChanged = true;
					watch->change = OsFileWatchChange_Modified;
					watch->lastChangeTime = programTime;
				}
				watch->writeTime = newWriteTime;
			}
			watch->lastCheck = programTime;
		}
	}
	return fileChanged;
}

PEXPI void OsResetFileWatch(OsFileWatch* watch, u64 programTime)
{
	UNUSED(programTime); //TODO: Do we actually need this passed for some reason?
	NotNull(watch);
	NotNull(watch->arena);
	watch->change = OsFileWatchChange_None;
	watch->fileExists = OsDoesFileExist(watch->fullPath);
	if (watch->fileExists)
	{
		Result getWriteTimeResult = OsGetFileWriteTime(watch->fullPath, &watch->writeTime);
		Assert(getWriteTimeResult == Result_Success);
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _MISC_FILE_WATCH_H
