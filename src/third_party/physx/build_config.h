/*
File:   build_config.h
Author: Taylor Robbins
Date:   02\23\2025
Description:
	** This file contains a bunch of options that control the build.bat.
	** This file is both a C header file that can be #included from a .c file,
	** and it is also scraped by the build.bat to extract values to change the work it performs.
	** Because it is scraped, and not parsed with the full C language spec, we must
	** be careful to keep this file very simple and not introduce any syntax that
	** would confuse the scraper when it's searching for values
*/

#ifndef _BUILD_CONFIG_H
#define _BUILD_CONFIG_H

// Controls whether we are making a build that we want to run with a Debugger.
// This often sacrifices runtime speed or code size for extra debug information.
// Debug builds often take less time to compile as well.
#define DEBUG_BUILD  0

// Build .obj binaries for Windows platform
#define BUILD_WINDOWS 1
// Build .o binaries for Linux platform(s)
#define BUILD_LINUX   0

#define BUILD_PVDRUNTIME          1
#define BUILD_PHYSX_FOUNDATION    1
#define BUILD_PHYSX_COMMON        1
#define BUILD_PHYSX_PVD           1
#define BUILD_SCENEQUERY          1
#define BUILD_LOWLEVEL            1
#define BUILD_LOWLEVELAABB        1
#define BUILD_LOWLEVELDYNAMICS    1
#define BUILD_SIMCONTROLLER       1
#define BUILD_PHYSX_TASK          1
#define BUILD_PHYSX_API           1
#define BUILD_PHYSX_EXTENSIONS    1

#define BUILD_STATIC_LIB          1

#endif //  _BUILD_CONFIG_H
