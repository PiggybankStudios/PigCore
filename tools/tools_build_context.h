/*
File:   tools_build_context.h
Author: Taylor Robbins
Date:   06\23\2025
*/

#ifndef _TOOLS_BUILD_CONTEXT_H
#define _TOOLS_BUILD_CONTEXT_H

typedef struct BuildContext BuildContext;
struct BuildContext
{
	bool DEBUG_BUILD;
	bool BUILD_PIGGEN;
	bool BUILD_SHADERS;
	bool RUN_PIGGEN;
	bool BUILD_IMGUI_OBJ;
	bool BUILD_PHYSX_OBJ;
	bool BUILD_PIG_CORE_DLL;
	bool BUILD_TESTS;
	bool RUN_TESTS;
	bool DUMP_PREPROCESSOR;
	bool CONVERT_WASM_TO_WAT;
	bool USE_EMSCRIPTEN;
	bool ENABLE_AUTO_PROFILE;
	bool RUN_FUZZER;
	bool BUILD_WINDOWS;
	bool BUILD_LINUX;
	bool BUILD_WEB;
	bool BUILD_ORCA;
	bool BUILD_PLAYDATE_DEVICE;
	bool BUILD_PLAYDATE_SIMULATOR;
	bool BUILD_WITH_RAYLIB;
	bool BUILD_WITH_BOX2D;
	bool BUILD_WITH_SOKOL_GFX;
	bool BUILD_WITH_SOKOL_APP;
	bool BUILD_WITH_SDL;
	bool BUILD_WITH_OPENVR;
	bool BUILD_WITH_CLAY;
	bool BUILD_WITH_IMGUI;
	bool BUILD_WITH_PHYSX;
	
	bool isMsvcInitialized;
	bool isEmsdkInitialized;
	
	Str8 emscriptenSdkPath;
	Str8 orcaSdkPath;
	Str8 playdateSdkDir;
	Str8 playdateSdkDir_C_API;
	
	CliArgList cl_CommonFlags;
	CliArgList cl_LangCFlags;
	CliArgList cl_LangCppFlags; //flags that we use when compiling any C++ program using MSVC compiler
	CliArgList clang_CommonFlags; //flags that we use when compiling any C program using Clang
	CliArgList clang_LinuxFlags; //flags for when we are compiling the linux version of a program using Clang
	CliArgList cl_CommonLinkerFlags;
	CliArgList clang_LinuxCommonLibraries;
	CliArgList cl_PigCoreLibraries; //These are all the libraries we need when compiling a binary that contains code from PigCore
	CliArgList clang_PigCoreLibraries; //These are all the libraries we need when compiling a binary that contains code from PigCore
	CliArgList clang_WasmFlags;
	CliArgList clang_WebFlags;
	CliArgList clang_OrcaFlags;
	CliArgList cl_PlaydateSimulatorCompilerFlags;
	CliArgList link_PlaydateSimulatorLinkerFlags;
	CliArgList link_PlaydateSimulatorLibraries;
	CliArgList gcc_PlaydateDeviceCommonFlags;
	CliArgList gcc_PlaydateDeviceCompilerFlags;
	CliArgList gcc_PlaydateDeviceLinkerFlags;
	CliArgList pdc_CommonFlags;
};

#endif //  _TOOLS_BUILD_CONTEXT_H
