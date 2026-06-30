/*
File:   app_resources.h
Author: Taylor Robbins
Date:   07\06\2025
*/

#ifndef _APP_RESOURCES_H
#define _APP_RESOURCES_H

#if USE_BUNDLED_RESOURCES
#include "resources_zip.h"
#endif

typedef struct AppResources AppResources;
struct AppResources
{
	bool isLoadingFromDisk;
	#if USE_BUNDLED_RESOURCES
	ZipArchive zipFile;
	#endif
};

#endif //  _APP_RESOURCES_H
