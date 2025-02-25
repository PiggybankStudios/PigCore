/*
File:   app_helpers.c
Author: Taylor Robbins
Date:   02\25\2025
Description: 
	** None
*/

#if BUILD_WITH_SOKOL_GFX

ImageData LoadImageData(Arena* arena, const char* path)
{
	ScratchBegin1(scratch, arena);
	Slice fileContents = Slice_Empty;
	bool readFileResult = OsReadFile(FilePathLit(path), scratch, false, &fileContents);
	Assert(readFileResult);
	ImageData imageData = ZEROED;
	Result parseResult = TryParseImageFile(fileContents, arena, &imageData);
	Assert(parseResult == Result_Success);
	ScratchEnd(scratch);
	return imageData;
}

#if BUILD_WITH_SOKOL_APP
void LoadWindowIcon()
{
	ScratchBegin(scratch);
	ImageData iconImageDatas[6];
	iconImageDatas[0] = LoadImageData(scratch, "resources/image/icon_16.png");
	iconImageDatas[1] = LoadImageData(scratch, "resources/image/icon_24.png");
	iconImageDatas[2] = LoadImageData(scratch, "resources/image/icon_32.png");
	iconImageDatas[3] = LoadImageData(scratch, "resources/image/icon_64.png");
	iconImageDatas[4] = LoadImageData(scratch, "resources/image/icon_120.png");
	iconImageDatas[5] = LoadImageData(scratch, "resources/image/icon_256.png");
	platform->SetWindowIcon(ArrayCount(iconImageDatas), &iconImageDatas[0]);
	ScratchEnd(scratch);
}
#endif //BUILD_WITH_SOKOL_APP

#endif //BUILD_WITH_SOKOL_GFX