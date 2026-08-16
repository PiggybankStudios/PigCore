/*
File:   tests_resources.h
Author: Taylor Robbins
Date:   08\15\2026
*/

#ifndef _TESTS_RESOURCES_H
#define _TESTS_RESOURCES_H

#if USE_EMBEDDED_RESOURCES_ZIP
#include "resources_zip.h"
#endif

typedef struct AppResources AppResources;
struct AppResources
{
	bool isLoadingFromDisk;
	#if USE_EMBEDDED_RESOURCES_ZIP
	ZipArchive zipFile;
	#endif
};

Result TryReadAppResource(Arena* arena, FilePath path, bool convertNewLines, Slice* fileContentsOut);
Slice ReadAppResource(Arena* arena, FilePath path, bool convertNewLines);

//Global
AppResources resources;

#endif //  _TESTS_RESOURCES_H
