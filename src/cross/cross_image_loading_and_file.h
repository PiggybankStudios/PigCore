/*
File:   cross_image_loading_and_file.h
Author: Taylor Robbins
Date:   01\20\2025
*/

#ifndef _CROSS_IMAGE_LOADING_AND_FILE_H
#define _CROSS_IMAGE_LOADING_AND_FILE_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	#if PIG_CORE_TRY_PARSE_IMAGE_AVAILABLE
	Result TryLoadImageFromPath(FilePath path, Arena* arena, ImageData* imageDataOut);
	#endif
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

#if PIG_CORE_TRY_PARSE_IMAGE_AVAILABLE

PEXP Result TryLoadImageFromPath(FilePath path, Arena* arena, ImageData* imageDataOut)
{
	NotNullStr(path);
	NotNull(arena);
	ScratchBegin1(scratch, arena);
	
	Slice fileContents = Slice_Empty;
	bool readSuccess = OsReadFile(path, arena, false, &fileContents);
	if (!readSuccess) { ScratchEnd(scratch); return Result_FailedToReadFile; }
	
	Result parseResult = TryParseImageFile(fileContents, arena, imageDataOut);
	
	ScratchEnd(scratch);
	
	return parseResult;
}

#endif //PIG_CORE_TRY_PARSE_IMAGE_AVAILABLE

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_IMAGE_LOADING_AND_FILE_H
