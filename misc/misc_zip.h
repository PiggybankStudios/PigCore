/*
File:   misc_zip.h
Author: Taylor Robbins
Date:   01\21\2025
Description:
	** Holds functions that help us parse and load files out of .zip archives
	** This file depends on miniz to do the parsing
*/

#ifndef _MISC_ZIP_H
#define _MISC_ZIP_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "struct/struct_string.h"
#include "os/os_path.h"
#include "misc/misc_result.h"

#define MINIZ_NO_STDIO //to disable all usage and any functions which rely on stdio for file I/O.
#if USING_CUSTOM_STDLIB
#define MINIZ_NO_TIME //if specified then the ZIP archive functions will not be able to get the current time, or get/set file times, and the C run-time funcs that get/set times won't be called.
#endif
// #define MINIZ_NO_DEFLATE_APIS //to disable all compression API's.
// #define MINIZ_NO_INFLATE_APIS //to disable all decompression API's.
// #define MINIZ_NO_ARCHIVE_APIS //to disable all ZIP archive API's.
// #define MINIZ_NO_ARCHIVE_WRITING_APIS //to disable all writing related ZIP archive API's.
// #define MINIZ_NO_ZLIB_APIS //to remove all ZLIB-style compression/decompression API's.
// #define MINIZ_NO_ZLIB_COMPATIBLE_NAMES //to disable zlib names, to prevent conflicts against stock zlib.
// #define MINIZ_NO_MALLOC //to disable all calls to malloc, free, and realloc. 

#define MINIZ_USE_UNALIGNED_LOADS_AND_STORES 1 //TODO: Is this true on all our platforms? Wasm32 specifically?
#define MINIZ_LITTLE_ENDIAN                  1
// #define MINIZ_HAS_64BIT_REGISTERS         //NOTE: This one seems to be defined automatically by miniz.h
#include "third_party/miniz/miniz.h"

#if PIG_CORE_IMPLEMENTATION
	#if COMPILER_IS_MSVC
	#pragma warning(push)
	#pragma warning(disable: 4132) //'s_tdefl_num_probes': const object should be initialized
	#pragma warning(disable: 4127) //conditional expression is constant
	#endif
	#include "third_party/miniz/miniz.c"
	#if COMPILER_IS_MSVC
	#pragma warning(pop)
	#endif
#endif //PIG_CORE_IMPLEMENTATION

typedef struct ZipArchive ZipArchive;
struct ZipArchive
{
	Arena* arena;
	bool isWriter;
	mz_zip_archive zip;
	uxx numFiles;
	u64 size;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void CloseZipArchive(ZipArchive* archive);
	Result OpenZipArchive(Arena* arena, Slice zipFileContents, ZipArchive* archiveOut);
	FilePath GetZipArchiveFilePath(ZipArchive* archive, Arena* pathArena, uxx fileIndex);
	bool FindZipArchiveFileNamed(ZipArchive* archive, Str8 fileName, uxx* fileIndexOut);
	Slice ReadZipArchiveFileAtIndex(ZipArchive* archive, Arena* fileContentsArena, uxx fileIndex, bool convertNewLines);
	PIG_CORE_INLINE Str8 ReadZipArchiveTextFileAtIndex(ZipArchive* archive, Arena* fileContentsArena, uxx fileIndex);
	PIG_CORE_INLINE Slice ReadZipArchiveBinFileAtIndex(ZipArchive* archive, Arena* fileContentsArena, uxx fileIndex);
	PIG_CORE_INLINE Slice ReadZipArchiveFile(ZipArchive* archive, Arena* fileContentsArena, Str8 fileName, bool convertNewLines);
	PIG_CORE_INLINE Str8 ReadZipArchiveTextFile(ZipArchive* archive, Arena* fileContentsArena, Str8 fileName);
	PIG_CORE_INLINE Slice ReadZipArchiveBinFile(ZipArchive* archive, Arena* fileContentsArena, Str8 fileName);
	Slice OpenZipArchiveAndReadFile(Arena* fileContentsArena, Slice zipFileContents, Str8 fileName, bool convertNewLines);
	PIG_CORE_INLINE Str8 OpenZipArchiveAndReadTextFile(Arena* fileContentsArena, Slice zipFileContents, Str8 fileName);
	PIG_CORE_INLINE Slice OpenZipArchiveAndReadBinFile(Arena* fileContentsArena, Slice zipFileContents, Str8 fileName);
	void CreateZipArchive(Arena* arena, ZipArchive* archiveOut);
	Result AddZipArchiveFile(ZipArchive* archive, FilePath fileName, Slice fileContents, bool convertNewLines);
	PIG_CORE_INLINE Result AddZipArchiveTextFile(ZipArchive* archive, FilePath fileName, Str8 fileContents);
	PIG_CORE_INLINE Result AddZipArchiveBinFile(ZipArchive* archive, FilePath fileName, Slice fileContents);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void CloseZipArchive(ZipArchive* archive)
{
	NotNull(archive);
	if (archive->arena != nullptr)
	{
		mz_bool endResult = mz_zip_end(&archive->zip);
		Assert(endResult == MZ_TRUE);
	}
	ClearPointer(archive);
}

PEXP Result OpenZipArchive(Arena* arena, Slice zipFileContents, ZipArchive* archiveOut)
{
	NotNull(arena);
	NotNullStr(zipFileContents);
	NotNull(archiveOut);
	ClearPointer(archiveOut);
	mz_bool initResult = mz_zip_reader_init_mem(&archiveOut->zip, zipFileContents.bytes, zipFileContents.length, 0); //No MZ_ZIP_FLAGs
	if (initResult == MZ_FALSE)
	{
		mz_zip_error initError = mz_zip_get_last_error(&archiveOut->zip);
		PrintLine_E("Failed to parse zip file: \"%s\"", mz_zip_get_error_string(initError));
		//TODO: Do we need to call mz_zip_reader_end here?
		return Result_Failure; //TODO: Make this into a better error code
	}
	DebugAssert(mz_zip_get_mode(&archiveOut->zip) == MZ_ZIP_MODE_READING);
	DebugAssert(mz_zip_get_type(&archiveOut->zip) == MZ_ZIP_TYPE_MEMORY);
	
	archiveOut->arena = arena;
	archiveOut->numFiles = (uxx)mz_zip_reader_get_num_files(&archiveOut->zip);
	archiveOut->size = mz_zip_get_archive_size(&archiveOut->zip);
	
	#if 0
	ScratchBegin1(scratch, arena);
	for (uxx fIndex = 0; fIndex < (uxx)numFiles; fIndex++)
	{
		uxx scratchMark = ArenaGetMark(scratch);
		mz_uint fileNameByteLength = mz_zip_reader_get_filename(&archiveOut->zip, (mz_uint)fIndex, nullptr, 0);
		Str8 fileName;
		fileName.length = fileNameByteLength-1;
		fileName.chars = (char*)AllocMem(scratch, fileNameByteLength);
		NotNull(fileName.chars);
		mz_zip_reader_get_filename(&archiveOut->zip, (mz_uint)fIndex, fileName.chars, (mz_uint)(fileName.length+1));
		
		mz_bool isDirectory = mz_zip_reader_is_file_a_directory(&archiveOut->zip, (mz_uint)fIndex);
		mz_bool isEncrypted = mz_zip_reader_is_file_encrypted(&archiveOut->zip, (mz_uint)fIndex);
		mz_bool isSupported = mz_zip_reader_is_file_supported(&archiveOut->zip, (mz_uint)fIndex);
		mz_zip_archive_file_stat fileStats = ZEROED;
		mz_bool readStatSuccess = mz_zip_reader_file_stat(&archiveOut->zip, (mz_uint)fIndex, &fileStats);
		
		// mz_uint64 m_central_dir_ofs; //Byte offset of this entry in the archive's central directory. Note we currently only support up to UINT_MAX or less bytes in the central dir.
		// mz_uint16 m_version_made_by; // These fields are copied directly from the zip's central dir.
		// mz_uint16 m_version_needed;
		// mz_uint16 m_bit_flag;
		// mz_uint16 m_method;
		// mz_uint32 m_crc32; // CRC-32 of uncompressed data.
		// mz_uint64 m_comp_size; //File's compressed size.
		// mz_uint64 m_uncomp_size; //File's uncompressed size. Note, I've seen some old archives where directory entries had 512 bytes for their uncompressed sizes, but when you try to unpack them you actually get 0 bytes.
		// mz_uint16 m_internal_attr; //Zip internal and external file attributes.
		// mz_uint32 m_external_attr;
		// mz_uint64 m_local_header_ofs; //Entry's local header file offset in bytes.
		// mz_uint32 m_comment_size; //Size of comment in bytes.
		// mz_bool m_is_directory; //MZ_TRUE if the entry appears to be a directory.
		// mz_bool m_is_encrypted; //MZ_TRUE if the entry uses encryption/strong encryption (which miniz_zip doesn't support)
		// mz_bool m_is_supported; //MZ_TRUE if the file is not encrypted, a patch file, and if it uses a compression method we support.
		// char m_filename[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE]; //Filename. If string ends in '/' it's a subdirectory entry. Guaranteed to be zero terminated, may be truncated to fit.
		// char m_comment[MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE]; //Comment field. Guaranteed to be zero terminated, may be truncated to fit.
		PrintLine_I("[%llu] %s \"%.*s\":", fIndex, isDirectory ? "Dir" : "File", StrPrint(fileName), isEncrypted ? " (Encrypted)" : "", isSupported ? "" : " (Unsupported!)");
		PrintLine_D("\tLocalHeaderOffset: %llu", fileStats.m_local_header_ofs);
		if (!isDirectory)
		{
			PrintLine_D("\tCRC: 0x%08X", fileStats.m_crc32);
			PrintLine_D("\tSize: %llu compressed, %llu uncompressed (%.1f%% smaller)", fileStats.m_comp_size, fileStats.m_uncomp_size, 100.0f - ((r32)fileStats.m_comp_size / (r32)fileStats.m_uncomp_size) * 100.0f);
		}
		if (fileStats.m_comment_size > 0) { PrintLine_D("\tComment: \"%s\"", &fileStats.m_comment[0]); }
		
		if (!isDirectory)
		{
			Slice fileMemory;
			fileMemory.length = (uxx)fileStats.m_uncomp_size;
			fileMemory.bytes = (u8*)AllocMem(scratch, (uxx)fileStats.m_uncomp_size);
			NotNull(fileMemory.bytes);
			mz_bool readFileSuccess = mz_zip_reader_extract_to_mem(&archiveOut->zip, (mz_uint)fIndex, fileMemory.bytes, fileMemory.length, 0);
			Assert(readFileSuccess);
			PrintLine_D("\tContents: %02X %02X %02X %02X", fileMemory.bytes[0], fileMemory.bytes[1], fileMemory.bytes[2], fileMemory.bytes[3]);
		}
		
		ArenaResetToMark(scratch, scratchMark);
	}
	ScratchEnd(scratch);
	#endif
	
	// 
	
	return Result_Success;
}

//NOTE: Always returns a null-terminated path
PEXP FilePath GetZipArchiveFilePath(ZipArchive* archive, Arena* pathArena, uxx fileIndex)
{
	NotNull(archive);
	NotNull(archive->arena);
	Assert(fileIndex < archive->numFiles);
	
	mz_uint fileNameByteLength = mz_zip_reader_get_filename(&archive->zip, (mz_uint)fileIndex, nullptr, 0);
	
	Str8 result;
	result.length = fileNameByteLength-1;
	result.chars = (char*)AllocMem(pathArena, fileNameByteLength);
	if (result.chars == nullptr) { return FilePath_Empty; }
	mz_uint secondFileNameResult = mz_zip_reader_get_filename(&archive->zip, (mz_uint)fileIndex, result.chars, (mz_uint)(result.length+1));
	Assert(secondFileNameResult == fileNameByteLength);
	
	return result;
}

//TODO: Should we add a case-insensitive search option?
PEXP bool FindZipArchiveFileNamed(ZipArchive* archive, Str8 fileName, uxx* fileIndexOut)
{
	NotNull(archive);
	NotNull(archive->arena);
	ScratchBegin(scratch);
	for (uxx fIndex = 0; fIndex < archive->numFiles; fIndex++)
	{
		uxx scratchMark = ArenaGetMark(scratch);
		FilePath filePath = GetZipArchiveFilePath(archive, scratch, fIndex);
		//TODO: Replace with StrEndsWith call
		if (filePath.length >= fileName.length && MyStrCompare(filePath.chars + filePath.length - fileName.length, fileName.chars, fileName.length) == 0)
		{
			SetOptionalOutPntr(fileIndexOut, fIndex);
			ScratchEnd(scratch);
			return true;
		}
		ArenaResetToMark(scratch, scratchMark);
	}
	ScratchEnd(scratch);
	return false;
}

PEXP Slice ReadZipArchiveFileAtIndex(ZipArchive* archive, Arena* fileContentsArena, uxx fileIndex, bool convertNewLines)
{
	NotNull(archive);
	NotNull(archive->arena);
	NotNull(fileContentsArena);
	Assert(fileIndex < archive->numFiles);
	
	mz_zip_archive_file_stat fileStats = ZEROED;
	mz_bool readStatSuccess = mz_zip_reader_file_stat(&archive->zip, (mz_uint)fileIndex, &fileStats);
	Assert(readStatSuccess == MZ_TRUE);
	
	ScratchBegin1(scratch, fileContentsArena);
	Slice result = Slice_Empty;
	result.length = (uxx)fileStats.m_uncomp_size;
	result.bytes = (u8*)AllocMem(convertNewLines ? scratch : fileContentsArena, (uxx)result.length);
	NotNull(result.bytes);
	mz_bool readFileSuccess = mz_zip_reader_extract_to_mem(&archive->zip, (mz_uint)fileIndex, result.bytes, result.length, 0);
	if (!readFileSuccess)
	{
		//TODO: Take a look at archive->zip.m_last_error to determine the failure reason!
		FreeStr8(fileContentsArena, &result);
		ScratchEnd(scratch);
		return Slice_Empty;
	}
	Assert(readFileSuccess);
	
	if (convertNewLines) { result = StrReplace(fileContentsArena, result, StrLit("\r\n"), StrLit("\n"), false); }
	
	ScratchEnd(scratch);
	return result;
}
PEXPI Str8 ReadZipArchiveTextFileAtIndex(ZipArchive* archive, Arena* fileContentsArena, uxx fileIndex) { return ReadZipArchiveFileAtIndex(archive, fileContentsArena, fileIndex, true); }
PEXPI Slice ReadZipArchiveBinFileAtIndex(ZipArchive* archive, Arena* fileContentsArena, uxx fileIndex) { return ReadZipArchiveFileAtIndex(archive, fileContentsArena, fileIndex, false); }

PEXPI Slice ReadZipArchiveFile(ZipArchive* archive, Arena* fileContentsArena, Str8 fileName, bool convertNewLines)
{
	uxx fileIndex = 0;
	if (!FindZipArchiveFileNamed(archive, fileName, &fileIndex)) { return Slice_Empty; }
	return ReadZipArchiveFileAtIndex(archive, fileContentsArena, fileIndex, convertNewLines);
}
PEXPI Str8 ReadZipArchiveTextFile(ZipArchive* archive, Arena* fileContentsArena, Str8 fileName) { return ReadZipArchiveFile(archive, fileContentsArena, fileName, true); }
PEXPI Slice ReadZipArchiveBinFile(ZipArchive* archive, Arena* fileContentsArena, Str8 fileName) { return ReadZipArchiveFile(archive, fileContentsArena, fileName, false); }

PEXP Slice OpenZipArchiveAndReadFile(Arena* fileContentsArena, Slice zipFileContents, Str8 fileName, bool convertNewLines)
{
	ScratchBegin1(scratch, fileContentsArena);
	ZipArchive archive = ZEROED;
	Result openResult = OpenZipArchive(scratch, zipFileContents, &archive);
	if (openResult != Result_Success) { ScratchEnd(scratch); return Slice_Empty; }
	Slice result = ReadZipArchiveFile(&archive, fileContentsArena, fileName, convertNewLines);
	ScratchEnd(scratch);
	return result;
}
PEXPI Str8 OpenZipArchiveAndReadTextFile(Arena* fileContentsArena, Slice zipFileContents, Str8 fileName) { return OpenZipArchiveAndReadFile(fileContentsArena, zipFileContents, fileName, true); }
PEXPI Slice OpenZipArchiveAndReadBinFile(Arena* fileContentsArena, Slice zipFileContents, Str8 fileName) { return OpenZipArchiveAndReadFile(fileContentsArena, zipFileContents, fileName, false); }


size_t ZipFileWriteCallback(void* contextPntr, mz_uint64 fileOffset, const void* bufferPntr, size_t numBytes)
{
	PrintLine_D("ZipFileWriteCallback(%p, %llu, %p, %zu)", contextPntr, fileOffset, bufferPntr, numBytes);
	ZipArchive* archive = (ZipArchive*)contextPntr;
	if (archive != nullptr)
	{
		PrintLine_D("ZipFileWriteCallback archive: %p %s %llu file%s", archive->arena, archive->isWriter ? "Writer" : "Reader", (u64)archive->numFiles, Plural(archive->numFiles, "s"));
	}
	//TODO: Implement me!
	return numBytes;
}

//NOTE: ZipArchive contains a function pointer, so maintaining an open writable ZipArchive is a bad idea if hot-reloading is happening
PEXP void CreateZipArchive(Arena* arena, ZipArchive* archiveOut)
{
	NotNull(arena);
	NotNull(archiveOut);
	ClearPointer(archiveOut);
	archiveOut->isWriter = true;
	archiveOut->arena = arena;
	
	archiveOut->zip.m_pWrite = ZipFileWriteCallback;
	archiveOut->zip.m_pIO_opaque = archiveOut;
	mz_bool initResult = mz_zip_writer_init(&archiveOut->zip, 0);
	Assert(initResult == MZ_TRUE);
}

PEXP Result AddZipArchiveFile(ZipArchive* archive, FilePath fileName, Slice fileContents, bool convertNewLines)
{
	NotNull(archive);
	NotNull(archive->arena);
	NotEmptyStr(fileName);
	NotNullStr(fileContents);
	ScratchBegin(scratch);
	
	if (convertNewLines && fileContents.length > 0)
	{
		fileContents = StrReplace(scratch, fileContents, StrLit("\n"), StrLit("\r\n"), true);
		NotNull(fileContents.chars);
	}
	// Allocate a null-terminated version of fileName
	FilePath fileNameNt = AllocFilePath(scratch, fileName, true);
	mz_bool addMemSuccess = mz_zip_writer_add_mem(&archive->zip, fileNameNt.chars, fileContents.bytes, (size_t)fileContents.length, (mz_uint)MZ_DEFAULT_COMPRESSION); //TODO: Should we tune this compression level? Maybe choose MZ_BEST_SPEED sometimes?
	Assert(addMemSuccess); //TODO: Do we want to return a failure result for this? Why would it fail?
	IncrementUXX(archive->numFiles);
	
	ScratchEnd(scratch);
	return Result_Success;
}
PEXPI Result AddZipArchiveTextFile(ZipArchive* archive, FilePath fileName, Str8 fileContents) { return AddZipArchiveFile(archive, fileName, fileContents, true); }
PEXPI Result AddZipArchiveBinFile(ZipArchive* archive, FilePath fileName, Slice fileContents) { return AddZipArchiveFile(archive, fileName, fileContents, false); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _MISC_ZIP_H

#if defined(_MISC_ZIP_H) && defined(_OS_FILE_H)
#include "cross/cross_zip_and_file.h"
#endif
