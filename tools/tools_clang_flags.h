/*
File:   tools_clang_flags.h
Author: Taylor Robbins
Date:   06\16\2025
Description:
	** Contains defines for all the Clang compiler flags that we may use in our build scripts
*/

#ifndef _TOOLS_CLANG_FLAGS_H
#define _TOOLS_CLANG_FLAGS_H

#define CLANG_COMPILE             "-c"
#define CLANG_DEFINE              "-D \"[VAL]\""
#define CLANG_LANG_VERSION        "-std=[VAL]"
#define CLANG_OUTPUT_FILE         "-o \"[VAL]\""
#define CLANG_OPTIMIZATION_LEVEL  "-O[VAL]"
#define CLANG_DEBUG_INFO          "-g[VAL]"
#define CLANG_DEBUG_INFO_DEFAULT  "-g"
#define CLANG_WARNING_LEVEL       "-W[VAL]"
#define CLANG_ENABLE_WARNING      "-W[VAL]"
#define CLANG_DISABLE_WARNING     "-Wno-[VAL]"
#define CLANG_FULL_FILE_PATHS     "-fdiagnostics-absolute-paths" //Print absolute paths in diagnostics TODO: Figure out how to resolve these back to windows paths for Sublime error linking?
#define CLANG_INCLUDE_DIR         "-I \"[VAL]\"" //Adds an include directory to search in when resolving #includes
#define CLANG_LIBRARY_DIR         "-L \"[VAL]\""
#define CLANG_SYSTEM_LIBRARY      "-l[VAL]"
#define CLANG_PRECOMPILE_ONLY     "-E"
#define CLANG_BUILD_SHARED_LIB    "-shared"
#define CLANG_fPIC                "-fPIC" //TODO: Name this better!
#define CLANG_STDLIB_FOLDER       "--sysroot \"[VAL]\""
#define CLANG_M_FLAG              "-m[VAL]" //TODO: Name this better!
#define CLANG_TARGET_ARCHITECTURE "--target=[VAL]"
//NOTE: Arguments prefixed with -Wl, are passed along to the linker
#define CLANG_NO_ENTRYPOINT       "-Wl,--no-entry"
#define CLANG_EXPORT_DYNAMIC      "-Wl,--export-dynamic"
#define CLANG_EXPORT_SYMBOL       "-Wl,--export=[VAL]"
#define CLANG_ALLOW_UNDEFINED     "-Wl,--allow-undefined"
#define CLANG_NO_STD_LIBRARIES    "--no-standard-libraries"
#define CLANG_NO_STD_INCLUDES     "--no-standard-includes"

#define CLANG_WARNING_SWITCH_MISSING_CASES          "switch" //8 enumeration values not handled in switch: 'ArenaType_None', 'ArenaType_Funcs', 'ArenaType_Generic'...
#define CLANG_WARNING_UNUSED_FUNCTION               "unused-function" //unused function 'MeowExpandSeed'
#define CLANG_WARNING_SHADOWING                     "shadow" //Warn whenever a local variable or type declaration shadows another variable, parameter, type, class member (in C++), or instance variable (in Objective-C) or whenever a built-in function is shadowed
#define CLANG_WARNING_MISSING_FALLTHROUGH_IN_SWITCH "implicit-fallthrough" //Must use [[fallthrough]] on a case label that falls through to the next case

#define EMSCRIPTEN_S_FLAG "-s[VAL]" //TODO: Give this a better name!

#endif //  _TOOLS_CLANG_FLAGS_H
