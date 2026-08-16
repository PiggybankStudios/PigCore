/*
File:   tests_resources.c
Author: Taylor Robbins
Date:   08\15\2026
Description: 
	** Sometimes we want to embed the resources (images, fonts, text, etc.)
	** directly into the executable itself rather than hoping we can
	** find and load the files at runtime. This file handles
	** routing requests to load files to an embedded zip file that
	** was created in the build_script.c, which takes the form of a
	** u8 array in build\gen\resources_zip.h and build\gen\resources_zip.c
	** When the resources are NOT embedded then these functions just
	** route to regular OsReadFile functions.
	**
	** On MacOS we have the ability to put resources directly into the
	** app bundle so we don't necassarily need to do this embedding. But
	** we still need to handle the fact that the working directory for
	** GUI applications on MacOS is not fixed, so we have to find
	** the app bundle location first and then look inside
*/

#if USE_EMBEDDED_RESOURCES_ZIP
#include "resources_zip.c"
#endif

void InitAppResources()
{
	ClearStruct(resources);
	resources.isLoadingFromDisk = (USE_EMBEDDED_RESOURCES_ZIP == 0);
	#if USE_EMBEDDED_RESOURCES_ZIP
	Slice zipFileContents = MakeSlice(ArrayCount(resources_zip_bytes), &resources_zip_bytes[0]);
	Result openResult = OpenZipArchive(stdHeap, zipFileContents, &resources.zipFile);
	if (openResult != Result_Success) { NotifyPrint_E("Failed to parse builtin zip file %llu bytes as zip archive: %s", zipFileContents.length, GetResultStr(openResult)); }
	Assert(openResult == Result_Success);
	#endif
}

//TODO: This returns a null-terminated fileContents when using OsReadFile but NOT when using ReadZipArchiveFileAtIndex. That's annoying for the calling code to handle when freeing!
Result TryReadAppResource(Arena* arena, FilePath path, bool convertNewLines, Slice* fileContentsOut)
{
	Assert(arena != nullptr || fileContentsOut == nullptr);
	NotNullStr(path);
	#if USE_EMBEDDED_RESOURCES_ZIP
	{
		if (StrAnyCaseStartsWith(path, StrLit("../resources/"))) { path = StrSliceFrom(path, 13); }
		else { return Result_WrongFolder; }
		
		uxx zipFileIndex = 0;
		if (FindZipArchiveFileNamed(&resources.zipFile, path, &zipFileIndex))
		{
			if (fileContentsOut == nullptr) { return Result_Success; }
			*fileContentsOut = ReadZipArchiveFileAtIndex(&resources.zipFile, arena, zipFileIndex, convertNewLines);
			return Result_Success;
		}
		else
		{
			PrintLine_E("Resource is not in embedded zip: \"%.*s\"", StrPrint(path));
			return Result_FailedToReadFile;
		}
	}
	#elif TARGET_IS_OSX
	{
		//NOTE: On OSX when an application is run from Finder, the working directory is not guaranteed.
		//      We always need to look up where our executable is actually located and use paths relative to that
		
		if (StrAnyCaseStartsWith(path, StrLit("../resources/"))) { path = StrSliceFrom(path, 13); }
		else { return Result_WrongFolder; }
		
		ScratchBegin1(scratch, arena);
		Result getExePathError = Result_None;
		FilePath appBundlePath = OsGetExecutablePath(scratch, &getExePathError);
		if (getExePathError != Result_None) { ScratchEnd(scratch); return getExePathError; }
		NotEmptyStr(appBundlePath);
		
		#if USE_OSX_APP_BUNDLE_RESOURCES
		FilePath resourcePath = JoinPathsInArena3(scratch, appBundlePath, StrLit("Contents/Resources"), path, false);
		#else
		FilePath resourcePath = JoinPathsInArena3(scratch, appBundlePath, StrLit("../../resources"), path, false);
		#endif
		
		if (fileContentsOut != nullptr)
		{
			bool readSuccess = OsReadFile(resourcePath, arena, convertNewLines, fileContentsOut);
			if (!readSuccess) { PrintLine_E("Failed to read resource %s app bundle \"%.*s\"", USE_OSX_APP_BUNDLE_RESOURCES ? "from inside" : "next to", StrPrint(resourcePath)); }
			return readSuccess ? Result_Success : Result_FailedToReadFile;
		}
		else
		{
			return OsDoesFileExist(resourcePath) ? Result_Success : Result_FailedToReadFile;
		}
		
		ScratchEnd(scratch);
	}
	#else
	{
		if (fileContentsOut != nullptr)
		{
			bool readSuccess = OsReadFile(path, arena, convertNewLines, fileContentsOut);
			return readSuccess ? Result_Success : Result_FailedToReadFile;
		}
		else
		{
			return OsDoesFileExist(path) ? Result_Success : Result_FailedToReadFile;
		}
	}
	#endif
}
Slice ReadAppResource(Arena* arena, FilePath path, bool convertNewLines)
{
	Slice result = Slice_Empty;
	Result openResult = TryReadAppResource(arena, path, convertNewLines, &result);
	Assert(openResult == Result_Success);
	return result;
}
