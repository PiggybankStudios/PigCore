/*
File:   cross_zip_and_file.h
Author: Taylor Robbins
Date:   01\21\2025
*/

#ifndef _CROSS_ZIP_AND_FILE_H
#define _CROSS_ZIP_AND_FILE_H

#if !PIG_CORE_IMPLEMENTATION
	Result OpenZipArchivePath(Arena* arena, FilePath filePath, ZipArchive* archiveOut);
	Slice OpenZipArchivePathAndReadFile(Arena* fileContentsArena, FilePath zipFilePath, Str8 archiveFileName);
#endif

#if PIG_CORE_IMPLEMENTATION
PEXP Result OpenZipArchivePath(Arena* arena, FilePath filePath, ZipArchive* archiveOut)
{
	NotNull(arena);
	NotNullStr(filePath);
	ScratchBegin1(scratch, arena);
	Slice zipFileContents = Slice_Empty;
	Result readResult = OsReadFile(filePath, scratch, false, &zipFileContents);
	if (readResult != Result_Success) { ScratchEnd(scratch); return readResult; }
	Result result = OpenZipArchive(arena, zipFileContents, archiveOut);
	ScratchEnd(scratch);
	return result;
}

PEXP Slice OpenZipArchivePathAndReadFile(Arena* fileContentsArena, FilePath zipFilePath, Str8 archiveFileName)
{
	ScratchBegin1(scratch, fileContentsArena);
	ZipArchive archive = ZEROED;
	Result openResult = OpenZipArchivePath(scratch, zipFilePath, &archive);
	if (openResult != Result_Success) { ScratchEnd(scratch); return Slice_Empty; }
	Slice result = ReadZipArchiveFile(&archive, fileContentsArena, archiveFileName);
	ScratchEnd(scratch);
	return result;
}
#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_ZIP_AND_FILE_H
