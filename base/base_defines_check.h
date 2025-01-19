/*
File:   base_defines_check.h
Author: Taylor Robbins
Date:   01\19\2025
Description:
	** This file makes sure that all defines that affect the way PigCore is
	** compiled are defined to either 1 or 0
*/

#ifndef _BASE_DEFINES_CHECK_H
#define _BASE_DEFINES_CHECK_H

#ifndef USE_EMSCRIPTEN
#define USE_EMSCRIPTEN 0
#endif

#ifndef BUILD_WITH_RAYLIB
#define BUILD_WITH_RAYLIB 0
#endif

#ifndef BUILD_WITH_BOX2D
#define BUILD_WITH_BOX2D 0
#endif

#ifndef BUILD_WITH_SOKOL
#define BUILD_WITH_SOKOL 0
#endif

#ifndef BUILD_WITH_SDL
#define BUILD_WITH_SDL 0
#endif

#endif //  _BASE_DEFINES_CHECK_H
