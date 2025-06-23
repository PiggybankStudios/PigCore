/*
File:   tools_emscripten_flags.h
Author: Taylor Robbins
Date:   06\23\2025
*/

#ifndef _TOOLS_EMSCRIPTEN_FLAGS_H
#define _TOOLS_EMSCRIPTEN_FLAGS_H

#define EXE_EMSCRIPTEN_COMPILER "emcc"

//NOTE: Emscripten's compiler accepts all the same flags that Clang does

#define EMSCRIPTEN_S_FLAG "-s[VAL]" //TODO: Give this a better name!

#endif //  _TOOLS_EMSCRIPTEN_FLAGS_H
