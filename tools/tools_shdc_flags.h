/*
File:   tools_shdc_flags.h
Author: Taylor Robbins
Date:   06\19\2025
Description:
	** Contains defines for all the sokol-shdc.exe flags that we may use in pig_build.exe
*/

#ifndef _TOOLS_SHDC_FLAGS_H
#define _TOOLS_SHDC_FLAGS_H

#if BUILDING_ON_WINDOWS
#define EXE_SHDC        "../third_party/_tools/win32/sokol-shdc.exe"
#define EXE_SHDC_NAME   "sokol-shdc.exe"
#elif BUILDING_ON_LINUX
#define EXE_SHDC        "../third_party/_tools/linux/sokol-shdc"
#define EXE_SHDC_NAME   "sokol-shdc"
#elif BUILDING_ON_OSX
#define EXE_SHDC        "../third_party/_tools/osx/sokol-shdc"
#define EXE_SHDC_NAME   "sokol-shdc"
#endif

#define SHDC_FORMAT           "--format=[VAL]"
#define SHDC_ERROR_FORMAT     "--errfmt=[VAL]"
#define SHDC_REFLECTION       "--reflection"
#define SHDC_SHADER_LANGUAGES "--slang=[VAL]"
#define SHDC_INPUT            "--input=\"[VAL]\""
#define SHDC_OUTPUT           "--output=\"[VAL]\""

#endif //  _TOOLS_SHDC_FLAGS_H
