/*
File:   build_config.h
Author: Taylor Robbins
Date:   01\19\2025
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
#define DEBUG_BUILD  1

// This disables hot-reloading support, the platform and game are one unit. Also PigCore gets compiled in directly rather than being used as a dynamic library
#define BUILD_INTO_SINGLE_UNIT  0

// Build .exe binaries for Windows platform
#define BUILD_WINDOWS 1
// Build binaries for Linux platform(s)
#define BUILD_LINUX   0
// Build the WASM binary for operating as a webpage
#define BUILD_WEB     0
// Runs the sokol-shdc.exe on all .glsl files in the source directory to produce .glsl.h and .glsl.c files and then compiles the .glsl.c files to .obj
#define BUILD_SHADERS 1

// Compiles core/piggen/main.c
#define BUILD_PIGGEN            1
// Same as above but only compiles if piggen.exe doesn't already exist in the _build folder
#define BUILD_PIGGEN_IF_NEEDED  1
// Generates code for all projects using piggen.exe (you can turn this off if you're not making changes to generated code and you've already generated it once)
#define RUN_PIGGEN              1

// Builds imgui.obj from imgui.cpp and cimgui.cpp (really it's building core/ui/ui_imgui_main.cpp which includes those)
#define BUILD_IMGUI_OBJ           1
// Same as above but only compiles if the obj doesn't already exist in the _build folder
#define BUILD_IMGUI_OBJ_IF_NEEDED 1

// Builds physx_capi.obj from core/phys/phys_physx_capi_main.cpp
#define BUILD_PHYSX_OBJ           1
// Same as above but only compiles if the obj doesn't already exist in the _build folder
#define BUILD_PHYSX_OBJ_IF_NEEDED 1

// Compiles piggen/main.c to either dynamic or static library
#define BUILD_PIG_CORE_LIB            1
// Same as above but only compiles if the dll doesn't already exist in the _build folder
#define BUILD_PIG_CORE_LIB_IF_NEEDED  1

// Compiles app/platform_main.c to %PROJECT_EXE_NAME%.exe
#define BUILD_APP_EXE  1
// Compiles app/app_main.c to %PROJECT_DLL_NAME%.dll
#define BUILD_APP_DLL  1
// Runs the %PROJECT_EXE_NAME%.exe
#define RUN_APP        0

// Copies the exe and dlls to the _data folder so they can be run alongside the resources folder more easily
// Our debugger projects usually run the exe from the _build folder but with working directory set to the _data folder
#define COPY_TO_DATA_DIRECTORY 1

// Rather than compiling the project(s) it will simply output the
// result of the preprocessor's pass over the code to the build folder
#define DUMP_PREPROCESSOR 0
// After .wasm file(s) are generated, we will run wasm2wat on them to make a .wat
// file (a text format of WebAssembly that is readable, mostly for debugging purposes)
#define CONVERT_WASM_TO_WAT 1
// Use emcc when compiling the WEB files
#define USE_EMSCRIPTEN 0
// Enables auto-profiling on function entry/exit (for clang only). Dumps to a file that can be viewed by spall
#define ENABLE_AUTO_PROFILE 0

// Enables being linked with raylib.lib and it's required libraries
#define BUILD_WITH_RAYLIB     0
// Enables being linked with box2d.lib and it's required libraries
#define BUILD_WITH_BOX2D      0
// Enables using sokol_gfx.h header files (and on non-windows OS' adds required libraries for Sokol to work)
#define BUILD_WITH_SOKOL_GFX  1
// Enables using sokol_app.h header files (and on non-windows OS' adds required libraries for Sokol to work)
#define BUILD_WITH_SOKOL_APP  1
// Enables being linked with SDL.lib and it's required libraries
#define BUILD_WITH_SDL        0
// Enables being linked with openvr_api.lib and it's required libraries
#define BUILD_WITH_OPENVR     0
// Enables using Clay header files
#define BUILD_WITH_CLAY       0
// Enables using Dear ImGui through cimgui.h/cpp
#define BUILD_WITH_IMGUI      0
// Enables being linked with PhysX_static_64.lib as well as physx_capi.obj
#define BUILD_WITH_PHYSX      0

#define PROJECT_READABLE_NAME New Pig Core Project
#define PROJECT_FOLDER_NAME   NewPigCoreProject
#define PROJECT_DLL_NAME      new_project_app_hot_reload
#define PROJECT_EXE_NAME      new_project

#ifndef STRINGIFY_DEFINE
#define STRINGIFY_DEFINE(define) STRINGIFY(define)
#endif
#ifndef STRINGIFY
#define STRINGIFY(text)          #text
#endif
#define PROJECT_READABLE_NAME_STR  STRINGIFY_DEFINE(PROJECT_READABLE_NAME)
#define PROJECT_FOLDER_NAME_STR    STRINGIFY_DEFINE(PROJECT_FOLDER_NAME)
#define PROJECT_DLL_NAME_STR       STRINGIFY_DEFINE(PROJECT_DLL_NAME)
#define PROJECT_EXE_NAME_STR       STRINGIFY_DEFINE(PROJECT_EXE_NAME)

#endif //  _BUILD_CONFIG_H
