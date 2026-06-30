/*
File:   platform_main.h
Author: Taylor Robbins
Date:   02\25\2025
*/

#ifndef _PLATFORM_MAIN_H
#define _PLATFORM_MAIN_H

typedef struct PlatformData PlatformData;
struct PlatformData
{
	Arena stdHeap;
	Arena stdHeapAllowFreeWithoutSize;
	
	AppApi appApi;
	#if !BUILD_INTO_SINGLE_UNIT
	OsDll appDll;
	#endif
	void* appMemoryPntr;
	
	AppInput appInputs[2];
	AppInput* oldAppInput;
	AppInput* currentAppInput;
};

#endif //  _PLATFORM_MAIN_H
