/*
File:   build_config.h
Author: Taylor Robbins
Date:   12\31\2024
Description:
	** This file contains a bunch of options that control the build.bat.
	** This file is both a C header file that can be #included from a .c file,
	** and it is also scraped by the build.bat to extract values to change the work it performs.
	** Because it is scraped, and not parsed with the full C language spec, we must
	** be careful to keep this file very simple and not introduce any syntax that
	** would confuse the scraper when it's searching for values
	** NOTE: See tools\extract_define_main.c for the source code of the scraper
*/

#ifndef _BUILD_CONFIG_H
#define _BUILD_CONFIG_H

// Controls whether we are making a build that we want to run with a Debugger.
// This often sacrifices runtime speed or code size for extra debug information.
// Debug builds often take less time to compile as well.
#define DEBUG_BUILD   1

// Build .exe binaries for Windows platform
#define BUILD_WINDOWS             1
// Build binaries for Linux platform(s)
#define BUILD_LINUX               0
// Build the WASM binary for operating as a webpage
#define BUILD_WEB                 0
// Build a WASM binary and package as Orca application
#define BUILD_ORCA                0
// Build a binary that can be uploaded to the Playdate device
#define BUILD_PLAYDATE_DEVICE     0
// Build a dll that can be loaded by the Playdate simulator
#define BUILD_PLAYDATE_SIMULATOR  0

// Runs the sokol-shdc.exe on all .glsl files in the source directory to produce .glsl.h and .glsl.c files and then compiles the .glsl.c files to .obj
#define BUILD_SHADERS 0

// Compiles piggen/main.c
#define BUILD_PIGGEN   0
// Generates code for all projects using piggen.exe (you can turn this off if you're not making changes to generated code and you've already generated it once)
#define RUN_PIGGEN     0

#define BUILD_PIG_BUILD 1

// Builds ui/ui_imgui_main.cpp in C++ mode into imgui.obj which will be linked into pig_core.dll and/or tests.exe
#define BUILD_IMGUI_OBJ 0
// Builds phys/phys_physx_capi_main.cpp in C++ mode into physx_capi.obj which will be linked into pig_core.dll and/or tests.exe
#define BUILD_PHYSX_OBJ 0

// Builds dll_main.c into pig_core.dll (mostly as a test to make sure dll compilation is working properly)
#define BUILD_PIG_CORE_DLL 0

// Compiles tests/main.c
#define BUILD_TESTS   1
// Runs the result of compiling tests/main.c, aka the tests.exe
#define RUN_TESTS     0

// Rather than compiling the project(s) it will simply output the
// result of the preprocessor's pass over the code to the build folder
#define DUMP_PREPROCESSOR 0
// After .wasm file(s) are generated, we will run wasm2wat on them to make a .wat
// file (a text format of WebAssembly that is readable, mostly for debugging purposes)
#define CONVERT_WASM_TO_WAT 0
// Use emcc when compiling the WEB files
#define USE_EMSCRIPTEN 0
// Enables auto-profiling on function entry/exit (for clang only). Dumps to a file that can be viewed by spall
#define ENABLE_AUTO_PROFILE 0
// Enables the fuzzing codepath (/fsanitize=fuzzer in MSVC) that helps us test functionality of code by "fuzzing" input data
// and tracking code-paths reached to help us search the possibility space efficiently, looking for crashes or assertions for any input
// NOTE: tests.exe must be run inside a Visual Studio CMD instance so it can find the asan .dll
#define RUN_FUZZER 0

// +===============================+
// | Optional Libraries/Frameworks |
// +===============================+
// Disables various parts of PigCore so that piggen.exe can compile with minimal dependencies
#define BUILD_FOR_PIGGEN     0
// Enables tests.exe being linked with raylib.lib and it's required libraries
#define BUILD_WITH_RAYLIB    0
// Enables tests.exe being linked with box2d.lib and it's required libraries
#define BUILD_WITH_BOX2D     0
// Enables pig_core.dll and tests.exe using sokol_gfx.h (and on non-windows OS' adds required libraries for Sokol to work)
#define BUILD_WITH_SOKOL_GFX 0
// Enables tests.exe using sokol_app.h to create and manage a graphical window
#define BUILD_WITH_SOKOL_APP 0
// Enables tests.exe being linked with SDL.lib and it's required libraries
#define BUILD_WITH_SDL       0
// Enables tests.exe being linked with openvr_api.lib and it's required libraries
#define BUILD_WITH_OPENVR    0
// Enables tests.exe using clay.h to render UI elements
#define BUILD_WITH_CLAY      0
// Enables tests.exe and pig_core.dll being linked with imgui.obj
#define BUILD_WITH_IMGUI     0
// Enables tests.exe and pig_core.dll being linked with phsyx_capi.obj and PhysX_static_64.lib
#define BUILD_WITH_PHYSX     0
// Enables using Metadesk library to parse Metadesk formatted files (md.h and md.c)
#define BUILD_WITH_METADESK  0

#endif //  _BUILD_CONFIG_H
