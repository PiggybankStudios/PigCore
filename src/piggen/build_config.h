/*
File:   build_config.h
Author: Taylor Robbins
Date:   04\01\2025
Description:
	** The build.bat uses the build_config.h that it's the root directory, not this one,
	** but when piggen_main.c is compiled it needs to include some PigCore files that will
	** in-turn include build_config.h, which has been configured for tests.exe and pig_core.dll
	** so we need an alternate build_config.h that will serve as a replacement for the one
	** in the root directory, and should be found first, by virtue of it being in the same
	** directory as the file that's including build_config.h.
	** This version doesn't need to serve any of the options for build.bat, only the options
	** that change the compilation of PigCore files
*/

#ifndef _BUILD_CONFIG_H
#define _BUILD_CONFIG_H

#define DEBUG_BUILD 1

#define BUILD_FOR_PIGGEN     1
#define BUILD_WITH_RAYLIB    0
#define BUILD_WITH_BOX2D     0
#define BUILD_WITH_SOKOL_GFX 0
#define BUILD_WITH_SOKOL_APP 0
#define BUILD_WITH_SDL       0
#define BUILD_WITH_OPENVR    0
#define BUILD_WITH_CLAY      0
#define BUILD_WITH_IMGUI     0
#define BUILD_WITH_PHYSX     0
#define BUILD_WITH_METADESK  1

#endif //  _BUILD_CONFIG_H
