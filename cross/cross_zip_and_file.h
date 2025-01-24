/*
File:   cross_zip_and_file.h
Author: Taylor Robbins
Date:   01\21\2025
*/

#ifndef _CROSS_ZIP_AND_FILE_H
#define _CROSS_ZIP_AND_FILE_H

#if !PIG_CORE_IMPLEMENTATION
	Result OpenZipArchivePath(Arena* arena, FilePath filePath, ZipArchive* archiveOut);
	Slice OpenZipArchivePathAndReadFile(Arena* fileContentsArena, FilePath zipFilePath, FilePath archiveFileName, bool convertNewLines);
	PIG_CORE_INLINE Str8 OpenZipArchivePathAndReadTextFile(Arena* fileContentsArena, FilePath zipFilePath, FilePath archiveFileName);
	PIG_CORE_INLINE Slice OpenZipArchivePathAndReadBinFile(Arena* fileContentsArena, FilePath zipFilePath, FilePath archiveFileName);
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

PEXP Slice OpenZipArchivePathAndReadFile(Arena* fileContentsArena, FilePath zipFilePath, FilePath archiveFileName, bool convertNewLines)
{
	ScratchBegin1(scratch, fileContentsArena);
	ZipArchive archive = ZEROED;
	Result openResult = OpenZipArchivePath(scratch, zipFilePath, &archive);
	if (openResult != Result_Success) { ScratchEnd(scratch); return Slice_Empty; }
	Slice result = ReadZipArchiveFile(&archive, fileContentsArena, archiveFileName, convertNewLines);
	ScratchEnd(scratch);
	return result;
}
PEXPI Str8 OpenZipArchivePathAndReadTextFile(Arena* fileContentsArena, FilePath zipFilePath, FilePath archiveFileName) { return OpenZipArchivePathAndReadFile(fileContentsArena, zipFilePath, archiveFileName, true); }
PEXPI Slice OpenZipArchivePathAndReadBinFile(Arena* fileContentsArena, FilePath zipFilePath, FilePath archiveFileName) { return OpenZipArchivePathAndReadFile(fileContentsArena, zipFilePath, archiveFileName, false); }


#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_ZIP_AND_FILE_H
