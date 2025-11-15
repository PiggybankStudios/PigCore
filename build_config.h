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
#define /*DONT SHOW IN CSWITCH*/ _BUILD_CONFIG_H

#define BUILD_PIG_BUILD 0

// Controls whether we are making a build that we want to run with a Debugger.
// This often sacrifices runtime speed or code size for extra debug information.
// Debug builds often take less time to compile as well.
#define DEBUG_BUILD   1
// Enables linking with tracy.lib to enable profiling through Tracy
#define PROFILING_ENABLED 1

// Build .exe binaries for Windows platform
#define BUILD_WINDOWS             1
// Build binaries for Linux platform(s)
#define BUILD_LINUX               0
// Build binaries for OSX platform
#define BUILD_OSX                 0
// Build the WASM binary for operating as a webpage
#define BUILD_WEB                 0
// Build the native .so files for Android
#define BUILD_ANDROID             0
// Package the .so files and other content into an .apk for Android
#define BUILD_ANDROID_APK         0
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

// Builds third_party/tracy/TracyClient.cpp in C++ mode into tracy.obj which will be linked into pig_core.dll
#define BUILD_TRACY_DLL 0

// Builds ui/ui_imgui_main.cpp in C++ mode into imgui.obj which will be linked into pig_core.dll and/or tests.exe
#define BUILD_IMGUI_OBJ 0
// Builds phys/phys_physx_capi_main.cpp in C++ mode into physx_capi.obj which will be linked into pig_core.dll and/or tests.exe
#define BUILD_PHYSX_OBJ 0

// Builds dll_main.c into pig_core.dll (mostly as a test to make sure dll compilation is working properly)
#define BUILD_PIG_CORE_DLL 0

// Compiles tests/main.c
#define BUILD_TESTS       1
// Runs the result of compiling tests/main.c, aka the tests.exe
#define RUN_TESTS         0
// Installs the Android .apk onto a running Virtual Device (AVD) through abd.exe
#define INSTALL_TESTS_APK 0

// Runs protoc --c_out (which uses protoc-gen-c plugin) to generate pb-c.h and pb-c.c files from all .proto files
#define GENERATE_PROTOBUF 0
// Rather than compiling the project(s) it will simply output the
// result of the preprocessor's pass over the code to the build folder
#define DUMP_PREPROCESSOR 0
// Generates assembly listing files for all compilation units
#define DUMP_ASSEMBLY 0
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
// Tells the sokol_gfx.h include to use OpenGL instead of D3D11 on Windows (NOTE: Smooth resizing only works in OpenGL mode right now!)
#define PREFER_OPENGL_OVER_D3D11 0

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
#define BUILD_WITH_SOKOL_GFX 1
// Enables tests.exe using sokol_app.h to create and manage a graphical window
#define BUILD_WITH_SOKOL_APP 1
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
// Enables support for making HTTP (and HTTPS) requests using the OS' API (WinHTTP for Windows, ? for Linux, etc.)
#define BUILD_WITH_HTTP      0
// Enables support for encoding/decoding Googles's protocol buffers using protobuf-c library
#define BUILD_WITH_PROTOBUF  0
// Enables building with the FreeType library which provides better font rasterizing support than stb_truetype.h (the default dependency)
#define BUILD_WITH_FREETYPE  1

// +==============================+
// |        String Defines        |
// +==============================+
// #define ANDROID_SIGNING_KEY_PATH     C:/Users/robbitay/.android/debug.keystore
// #define ANDROID_SIGNING_PASSWORD     android
#define ANDROID_SIGNING_KEY_PATH     F:/android_keystore.jks
#define ANDROID_SIGNING_PASS_PATH    Q:/android_keystore_password.txt

//folder name inside %ANDROID_SDK%/ndk/
#define ANDROID_NDK_VERSION          29.0.13599879
//folder name inside %ANDROID_SDK%/platforms/
#define ANDROID_PLATFORM_FOLDERNAME  android-36
//folder name inside %ANDROID_SDK%/build-tools/
#define ANDROID_BUILD_TOOLS_VERSION  36.0.0
#define ANDROID_PACKAGE_PATH         com.piggybank.pigcore.tests
#define ANDROID_ACTIVITY_PATH        com.piggybank.pigcore.tests/android.app.NativeActivity

#ifndef STRINGIFY_DEFINE
#define STRINGIFY_DEFINE(define) STRINGIFY(define)
#endif
#ifndef STRINGIFY
#define STRINGIFY(text)          #text
#endif

#define ANDROID_SIGNING_KEY_PATH_STR  STRINGIFY_DEFINE(ANDROID_SIGNING_KEY_PATH)
#ifdef ANDROID_SIGNING_PASSWORD
#define ANDROID_SIGNING_PASSWORD_STR  STRINGIFY_DEFINE(ANDROID_SIGNING_PASSWORD)
#endif
#ifdef ANDROID_SIGNING_PASS_PATH
#define ANDROID_SIGNING_PASS_PATH_STR STRINGIFY_DEFINE(ANDROID_SIGNING_PASS_PATH)
#endif
#define ANDROID_NDK_VERSION_STR         STRINGIFY_DEFINE(ANDROID_NDK_VERSION)
#define ANDROID_PLATFORM_FOLDERNAME_STR STRINGIFY_DEFINE(ANDROID_PLATFORM_FOLDERNAME)
#define ANDROID_BUILD_TOOLS_VERSION_STR STRINGIFY_DEFINE(ANDROID_BUILD_TOOLS_VERSION)
#define ANDROID_PACKAGE_PATH_STR        STRINGIFY_DEFINE(ANDROID_PACKAGE_PATH)
#define ANDROID_ACTIVITY_PATH_STR       STRINGIFY_DEFINE(ANDROID_ACTIVITY_PATH)

#endif //  _BUILD_CONFIG_H
