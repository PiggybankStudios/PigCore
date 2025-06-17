/*
File:   tools_clang_flags.h
Author: Taylor Robbins
Date:   06\16\2025
*/

#ifndef _TOOLS_CLANG_FLAGS_H
#define _TOOLS_CLANG_FLAGS_H

#define CLANG_LANG_VERSION     "-std=[VAL]"
#define CLANG_BINARY_NAME      "-o \"[VAL]\""
#define CLANG_DEBUG_INFO       "-g[VAL]"
#define CLANG_WARNING_LEVEL    "-W[VAL]"
#define CLANG_ENABLE_WARNING   "-W[VAL]"
#define CLANG_DISABLE_WARNING  "-Wno-[VAL]"
#define CLANG_FULL_FILE_PATHS  "-fdiagnostics-absolute-paths" //Print absolute paths in diagnostics TODO: Figure out how to resolve these back to windows paths for Sublime error linking?
#define CLANG_INCLUDE_DIR      "-I \"[VAL]\""
#define CLANG_SYSTEM_LIBRARY   "-l[VAL]"
#define CLANG_PRECOMPILE_ONLY  "-E"

#define CLANG_WARNING_SWITCH_MISSING_CASES          "switch" //8 enumeration values not handled in switch: 'ArenaType_None', 'ArenaType_Funcs', 'ArenaType_Generic'...
#define CLANG_WARNING_UNUSED_FUNCTION               "unused-function" //unused function 'MeowExpandSeed'
#define CLANG_WARNING_SHADOWING                     "shadow" //Warn whenever a local variable or type declaration shadows another variable, parameter, type, class member (in C++), or instance variable (in Objective-C) or whenever a built-in function is shadowed
#define CLANG_WARNING_MISSING_FALLTHROUGH_IN_SWITCH "implicit-fallthrough" //Must use [[fallthrough]] on a case label that falls through to the next case

#endif //  _TOOLS_CLANG_FLAGS_H
