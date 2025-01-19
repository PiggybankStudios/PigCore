/*
File:   os_path.h
Author: Taylor Robbins
Date:   01\06\2025
Description:
	** Contains functions that operate on the FilePath type (an alias of Str8) in
	** a cross-platform manner (handling forward and backslashes and sometimes
	** syntax like . and ..).
	** This file also contains functions that help us check where our executable
	** is located and where we are running from (the current working folder)
*/

#ifndef _OS_PATH_H
#define _OS_PATH_H

#include "base/base_typedefs.h"
#include "std/std_includes.h"
#include "base/base_assert.h"
#include "struct/struct_string.h"
#include "mem/mem_arena.h"

//NOTE: Although this is named "File" it can also hold a path to a folder too
typedef Str8 FilePath;

#define FilePath_Empty Str8_Empty

#define NewFilePath(length, pntr) (NewStr8((length), (pntr)))
#define FilePathLit(nullTermStr)  (StrLit(nullTermStr))
#define AsFilePath(string)        (string)

#define FreeFilePath(arenaPntr, pathPntr) FreeStr8((arenaPntr), (pathPntr))
#define FreeFilePathWithNt(arenaPntr, pathPntr) FreeStr8WithNt((arenaPntr), (pathPntr))

bool DoesPathHaveTrailingSlash(FilePath path) { return (path.chars != nullptr && path.length > 0 && (path.chars[path.length-1] == '\\' || path.chars[path.length-1] == '/')); }

//These both return the number of slashes that had to be replaced
uxx ChangePathSlashesTo(FilePath path, char slashType)
{
	uxx replaceCount = 0;
	for (uxx cIndex = 0; cIndex < path.length; cIndex++)
	{
		if (path.chars[cIndex] == '/' || path.chars[cIndex] == '\\')
		{
			path.chars[cIndex] = slashType;
			replaceCount++;
		}
	}
	return replaceCount;
}
uxx FixPathSlashes(FilePath path)
{
	//We try to store paths with forward slashes because it doesn't conflict with escape sequences usually
	return ChangePathSlashesTo(path, '/');
}

NODISCARD FilePath AllocFilePath(Arena* arena, Str8 pathStr, bool addNullTerm)
{
	FilePath result = AllocStrAndCopy(arena, pathStr.length, pathStr.chars, addNullTerm);
	FixPathSlashes(result); //we assume we can and should fix slashes since we are allocating the path on an arena
	return result;
}
NODISCARD FilePath AllocFolderPath(Arena* arena, Str8 pathStr, bool addNullTerm)
{
	FilePath result = ZEROED;
	if (pathStr.length == 0 && !addNullTerm) { return Str8_Empty; }
	bool needTrailingSlash = (pathStr.length > 0 && pathStr.chars[pathStr.length-1] != '\\' && pathStr.chars[pathStr.length-1] != '/');
	result.length = pathStr.length + (needTrailingSlash ? 1 : 0);
	result.chars = AllocArray(char, arena, result.length + (addNullTerm ? 1 : 0));
	if (pathStr.length > 0) { MyMemCopy(result.chars, pathStr.chars, pathStr.length); }
	if (needTrailingSlash) { result.chars[result.length-1] = '/'; }
	if (addNullTerm) { result.chars[result.length] = '\0'; }
	FixPathSlashes(result); //we assume we can and should fix slashes since we are allocating the path on an arena
	return result;
}

Str8 GetFileNamePart(FilePath path, bool includeExtension)
{
	uxx lastSlashIndex = 0;
	uxx firstPeriodIndex = path.length;
	for (uxx cIndex = 0; cIndex < path.length; cIndex++)
	{
		if (path.chars[cIndex] == '/' || path.chars[cIndex] == '\\') { lastSlashIndex = cIndex; firstPeriodIndex = path.length; }
		if (path.chars[cIndex] == '.' && cIndex < firstPeriodIndex) { firstPeriodIndex = cIndex; }
	}
	return includeExtension
		? NewStr8(path.length - (lastSlashIndex+1), path.chars + lastSlashIndex+1)
		: NewStr8(firstPeriodIndex - (lastSlashIndex+1), path.chars + lastSlashIndex+1);
}
//NOTE: "subExtensions" are things like "important" in "file.important.txt"
Str8 GetFileExtPart(FilePath path, bool includeSubExtensions, bool includeLeadingPeriod)
{
	uxx periodIndex = path.length;
	for (uxx cIndex = 0; cIndex < path.length; cIndex++)
	{
		if (path.chars[cIndex] == '/' || path.chars[cIndex] == '\\') { periodIndex = path.length; }
		if (path.chars[cIndex] == '.' && (!includeSubExtensions || cIndex < periodIndex)) { periodIndex = cIndex; }
	}
	if (periodIndex >= path.length) { return NewStr8(0, path.chars + path.length); }
	return includeLeadingPeriod
		? NewStr8(path.length - periodIndex, path.chars + periodIndex)
		: NewStr8(path.length - (periodIndex+1), path.chars + periodIndex+1);
}
//NOTE: If you have a path that is already a folder, this will trim the last part of the path unless there is a trailing slash!
FilePath GetFileFolderPart(FilePath path)
{
	uxx lastSlashIndex = path.length;
	for (uxx cIndex = 0; cIndex < path.length; cIndex++)
	{
		if (path.chars[cIndex] == '/' || path.chars[cIndex] == '\\') { lastSlashIndex = cIndex; }
	}
	return NewFilePath(lastSlashIndex < path.length ? lastSlashIndex+1 : lastSlashIndex, path.chars);
}

bool DoesPathHaveExt(FilePath path)
{
	bool result = false;
	for (uxx cIndex = 0; cIndex < path.length; cIndex++)
	{
		if (path.chars[cIndex] == '/' || path.chars[cIndex] == '\\') { result = false; }
		if (path.chars[cIndex] == '.') { result = true; }
	}
	return result;
}

uxx CountPathParts(FilePath path, bool includeEmptyBeginOrEnd)
{
	uxx result = 0;
	uxx finalSlash = path.length;
	for (uxx cIndex = 0; cIndex < path.length; cIndex++)
	{
		if (path.chars[cIndex] == '\\' || path.chars[cIndex] == '/')
		{
			if (includeEmptyBeginOrEnd || cIndex > 0)
			{
				result++;
				finalSlash = cIndex;
			}
		}
	}
	if (path.length > 0 && (includeEmptyBeginOrEnd || finalSlash < path.length-1)) { result++; }
	return result;
}

// Pass negative value for partIndex to index from the end
Str8 GetPathPart(FilePath path, ixx partIndex, bool includeEmptyBeginOrEnd)
{
	if (partIndex >= 0) //index is from beginning
	{
		uxx partBegin = 0;
		uxx currentPartIndex = 0;
		for (uxx cIndex = 0; cIndex <= path.length; cIndex++)
		{
			if (cIndex == path.length || path.chars[cIndex] == '\\' || path.chars[cIndex] == '/')
			{
				if (includeEmptyBeginOrEnd || cIndex > 0 || cIndex != partBegin)
				{
					if (currentPartIndex == (uxx)partIndex)
					{
						return NewStr8(cIndex - partBegin, path.chars + partBegin);
					}
					else
					{
						currentPartIndex++;
					}
				}
				partBegin = cIndex+1;
			}
		}
		return NewStr8(0, path.chars + path.length);
	}
	else //index is from beginning
	{
		uxx partBegin = path.length;
		uxx currentPartIndex = 1;
		for (ixx cIndex = path.length; cIndex >= 0; cIndex--)
		{
			if (cIndex == 0 || path.chars[cIndex-1] == '\\' || path.chars[cIndex-1] == '/')
			{
				if (includeEmptyBeginOrEnd || (uxx)cIndex < path.length || partBegin != (uxx)cIndex)
				{
					if (currentPartIndex == (uxx)(-partIndex))
					{
						return NewStr8((uxx)((ixx)partBegin - cIndex), path.chars + cIndex);
					}
					else
					{
						currentPartIndex++;
					}
				}
				partBegin = cIndex-1;
			}
		}
		return NewStr8(0, path.chars);
	}
}

// Removes any parts that are the "up one folder" syntax: ..
// as long as there is a preceeding part of the path that can be removed at the same time
#if 0
// TODO: This function is going to be a little harder to implement than I originally thought
Str8 CollapsePathParts(FilePath path)
{
	uxx newLength = path.length;
	for (uxx cIndex = 0; cIndex+3 <= path.length; cIndex++)
	{
		if (path.chars[cIndex] == '\\' || path.chars[cIndex] == '/')
		{
			if (path.chars[cIndex+1] == '.' && path.chars[cIndex+2] == '.' &&
				(cIndex+3 == path.length || path.chars[cIndex+3] == '\\' || path.chars[cIndex+3] == '/'))
			{
				bool foundPrevslash = false;
				for (uxx cIndex2 = cIndex; cIndex2 > 0; cIndex2--)
				{
					if (path.chars[cIndex2-1] == '\\' || path.chars[cIndex2-1] == '/')
					{
						foundPrevslash = true;
						newLength -= (cIndex+3) - (cIndex2-1);
						cIndex += 3;
					}
				}
			}
		}
	}
}
#endif

#endif //  _OS_PATH_H
