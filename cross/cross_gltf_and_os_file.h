/*
File:   cross_gltf_and_os_file.h
Author: Taylor Robbins
Date:   02\15\2025
*/

#ifndef _CROSS_GLTF_AND_OS_FILE_H
#define _CROSS_GLTF_AND_OS_FILE_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Result TryLoadGltfFile(FilePath filePath, Arena* arena, ModelData* modelDataOut);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

// +==============================+
// |      cgltfReadFileFunc       |
// +==============================+
// cgltf_result cgltfReadFileFunc(const cgltf_memory_options* memoryOptions, const cgltf_file_options* fileOptions, const char* path, cgltf_size* size, void** data)
static CGLTF_READ_FILE_FUNC_DEF(cgltfReadFileFunc)
{
	UNUSED(memoryOptions);
	NotNull(fileOptions);
	NotNull(fileOptions->user_data);
	Arena* arena = (Arena*)fileOptions->user_data;
	NotNull(path);
	// PrintLine_D("glTF is reading \"%s\"...", path);
	FilePath filePath = FilePathLit(path);
	Slice fileContents = Slice_Empty;
	bool readFileResult = OsReadFile(filePath, arena, false, &fileContents);
	if (readFileResult == false) { WriteLine_E("glTF failed to read file!"); return cgltf_result_file_not_found; }
	SetOptionalOutPntr(size, (cgltf_size)fileContents.length);
	SetOptionalOutPntr(data, fileContents.pntr);
	return cgltf_result_success;
}
// +==============================+
// |     cgltfReleaseFileFunc     |
// +==============================+
// void cgltfReleaseFileFunc(const cgltf_memory_options* memoryOptions, const cgltf_file_options* fileOptions, void* data)
static CGLTF_RELEASE_FILE_FUNC_DEF(cgltfReleaseFileFunc)
{
	UNUSED(memoryOptions);
	NotNull(fileOptions);
	NotNull(fileOptions->user_data);
	Arena* arena = (Arena*)fileOptions->user_data;
	if (data != nullptr && CanArenaFree(arena)) { FreeMem(arena, data, 0); }
}


PEXPI Result TryLoadGltfFile(FilePath filePath, Arena* arena, ModelData* modelDataOut)
{
	ScratchBegin1(scratch, arena);
	Slice fileContents = Slice_Empty;
	bool readResult = OsReadFile(filePath, scratch, false, &fileContents);
	if (readResult == false) { ScratchEnd(scratch); return Result_FailedToReadFile; }
	FilePath folderPath = GetFileFolderPart(filePath);
	Result result = TryParseGltfFileEx(fileContents, arena, modelDataOut, cgltfReadFileFunc, cgltfReleaseFileFunc, folderPath);
	ScratchEnd(scratch);
	return result;
}

#endif

#endif //  _CROSS_GLTF_AND_OS_FILE_H
