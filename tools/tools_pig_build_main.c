/*
File:   tools_pig_build_main.c
Author: Taylor Robbins
Date:   06\16\2025
Description: 
	** This is our C based build script that handles invoking the
	** compiler and other CLI tools to build the rest of the repository.
	** This tool must be easy to compile and must be entirely self-contained
	** This tool will open the build_config.h and scrape it at when
	** ran and decide what to build based on whats set in there.
	** We don't directly #include build_config.h because we don't want
	** to-recompile this tool every time one of the options changes
	** NOTE: We don't really spend any time freeing things in this program
	** since it's lifespan is short and it's memory requirements are tiny
*/

#define TOOL_EXE_NAME "pig_build.exe"

#include "tools/tools_shared.h"
#include "tools/tools_str_array.h"
#include "tools/tools_cli.h"
#include "tools/tools_msvc_flags.h"
#include "tools/tools_clang_flags.h"
#include "tools/tools_gcc_flags.h"
#include "tools/tools_shdc_flags.h"
#include "tools/tools_pdc_flags.h"
#include "tools/tools_pig_build_helpers.h"

static inline void PrintUsage()
{
	WriteLine_E("Usage: " TOOL_EXE_NAME " [build_config_path] [is_msvc_compiler_initialized]");
}

#define SHADER_IGNORE_LIST { ".git", "_template", "third_party", "_build" }

typedef struct FindShadersContext FindShadersContext;
struct FindShadersContext
{
	StrArray shaderPaths;
	StrArray headerPaths;
	StrArray sourcePaths;
	StrArray objPaths;
	StrArray oPaths;
};

RECURSIVE_DIR_WALK_CALLBACK_DEF(FindShaderFilesCallback)
{
	if (isFolder)
	{
		const char* shaderIgnoreList[] = SHADER_IGNORE_LIST;
		for (uxx iIndex = 0; iIndex < ArrayCount(shaderIgnoreList); iIndex++)
		{
			if (StrExactContains(path, NewStr8Nt(shaderIgnoreList[iIndex]))) { return false; }
		}
	}
	
	FindShadersContext* context = (FindShadersContext*)contextPntr;
	if (!isFolder && StrExactEndsWith(path, StrLit(".glsl")))
	{
		Str8 shaderName = GetFileNamePart(path, false);
		AddStr(&context->shaderPaths, path);
		AddStr(&context->headerPaths, JoinStrings2(path, StrLit(".h"), true));
		AddStr(&context->sourcePaths, JoinStrings2(path, StrLit(".c"), true));
		AddStr(&context->objPaths, JoinStrings2(shaderName, StrLit(".obj"), true));
		AddStr(&context->oPaths, JoinStrings2(shaderName, StrLit(".o"), true));
	}
	return true;
}

int main(int argc, char* argv[])
{
	// WriteLine("Running Pig Build!");
	
	bool isMsvcInitialized = WasMsvcDevBatchRun();
	bool isEmsdkInitialized = WasEmsdkEnvBatchRun();
	
	// +==============================+
	// |       Extract Defines        |
	// +==============================+
	const char* buildConfigPath = "..\\build_config.h";
	Str8 buildConfigContents = ZEROED;
	if (!TryReadFile(NewStr8Nt(buildConfigPath), &buildConfigContents))
	{
		PrintLine_E("Failed to open file \"%s\"", buildConfigPath);
		return 3;
	}
	
	bool DEBUG_BUILD              = ExtractBoolDefine(buildConfigContents, StrLit("DEBUG_BUILD"));
	bool BUILD_PIGGEN             = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_PIGGEN"));
	// bool BUILD_PIG_BUILD          = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_PIG_BUILD"));
	bool BUILD_SHADERS            = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_SHADERS"));
	bool RUN_PIGGEN               = ExtractBoolDefine(buildConfigContents, StrLit("RUN_PIGGEN"));
	bool BUILD_IMGUI_OBJ          = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_IMGUI_OBJ"));
	bool BUILD_PHYSX_OBJ          = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_PHYSX_OBJ"));
	bool BUILD_PIG_CORE_DLL       = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_PIG_CORE_DLL"));
	bool BUILD_TESTS              = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_TESTS"));
	bool RUN_TESTS                = ExtractBoolDefine(buildConfigContents, StrLit("RUN_TESTS"));
	bool DUMP_PREPROCESSOR        = ExtractBoolDefine(buildConfigContents, StrLit("DUMP_PREPROCESSOR"));
	bool CONVERT_WASM_TO_WAT      = ExtractBoolDefine(buildConfigContents, StrLit("CONVERT_WASM_TO_WAT"));
	bool USE_EMSCRIPTEN           = ExtractBoolDefine(buildConfigContents, StrLit("USE_EMSCRIPTEN"));
	bool ENABLE_AUTO_PROFILE      = ExtractBoolDefine(buildConfigContents, StrLit("ENABLE_AUTO_PROFILE"));
	bool RUN_FUZZER               = ExtractBoolDefine(buildConfigContents, StrLit("RUN_FUZZER"));
	bool BUILD_WINDOWS            = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WINDOWS"));
	bool BUILD_LINUX              = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_LINUX"));
	bool BUILD_WEB                = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WEB"));
	bool BUILD_ORCA               = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_ORCA"));
	bool BUILD_PLAYDATE_DEVICE    = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_PLAYDATE_DEVICE"));
	bool BUILD_PLAYDATE_SIMULATOR = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_PLAYDATE_SIMULATOR"));
	bool BUILD_WITH_RAYLIB        = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_RAYLIB"));
	bool BUILD_WITH_BOX2D         = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_BOX2D"));
	bool BUILD_WITH_SOKOL_GFX     = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_SOKOL_GFX"));
	bool BUILD_WITH_SOKOL_APP     = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_SOKOL_APP"));
	bool BUILD_WITH_SDL           = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_SDL"));
	bool BUILD_WITH_OPENVR        = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_OPENVR"));
	// bool BUILD_WITH_CLAY          = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_CLAY"));
	bool BUILD_WITH_IMGUI         = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_IMGUI"));
	bool BUILD_WITH_PHYSX         = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_PHYSX"));
	
	free(buildConfigContents.chars);
	
	// +==============================+
	// |          Constants           |
	// +==============================+
	const char* rootDir = "..";
	const char* linux_rootDir = "../.."; //we are inside the "linux" folder when compiler linux binaries
	Str8 msvcCompiler = StrLit("cl");
	Str8 msvcLinker = StrLit("LINK");
	Str8 clangCompiler = StrLit("clang");
	Str8 wslClangCompiler = StrLit("wsl clang-18"); //we are using the WSL instance with clang-18 installed to compile for Linux
	Str8 armGccCompiler = StrLit("arm-none-eabi-gcc"); //we use this when compiling for the Playdate device
	Str8 emscriptenCompiler = StrLit("emcc");
	#if BUILDING_ON_WINDOWS
	Str8 shdcExe = StrLit("..\\third_party\\_tools\\win32\\sokol-shdc.exe");
	#elif BUILDING_ON_LINUX
	Str8 shdcExe = StrLit("../../third_party/_tools/linux/sokol-shdc");
	#elif BUILDINGON_ON_OSX
	Str8 shdcExe = StrLit("../../third_party/_tools/osx/sokol-shdc");
	#endif
	
	// +==============================+
	// |        cl_CommonFlags        |
	// +==============================+
	CliArgList cl_CommonFlags = ZEROED; //"common_cl_flags" flags that we use when compiling any C or C++ program using MSVC compiler
	AddArg(&cl_CommonFlags, DEBUG_BUILD ? CL_STD_LIB_DYNAMIC_DBG : CL_STD_LIB_DYNAMIC);
	AddArg(&cl_CommonFlags, CL_FULL_FILE_PATHS); //we need full file paths in errors for Sublime Text to be able to parse the errors and display them in the editor
	AddArg(&cl_CommonFlags, CL_NO_LOGO); //Suppress the annoying Microsoft logo and copyright info that the compiler prints out
	AddArgNt(&cl_CommonFlags, CL_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "d" : "2");
	if (!DEBUG_BUILD) { AddArgNt(&cl_CommonFlags, CL_OPTIMIZATION_LEVEL, "y"); }
	if (!DEBUG_BUILD) { AddArgNt(&cl_CommonFlags, CL_OPTIMIZATION_LEVEL, "t"); }
	AddArgNt(&cl_CommonFlags, CL_WARNING_LEVEL, "X"); //Treat all warnings as errors
	AddArgInt(&cl_CommonFlags, CL_WARNING_LEVEL, 4); //Use warning level 4, then disable various warnings we don't care about
	AddArgInt(&cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_LOGICAL_OP_ON_ADDRESS_OF_STR_CONST);
	AddArgInt(&cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_NAMELESS_STRUCT_OR_UNION);
	AddArgInt(&cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_STRUCT_WAS_PADDED);
	AddArgInt(&cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_DECLARATION_HIDES_CLASS_MEMBER);
	AddArgInt(&cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_FUNC_REMOVED);
	AddArgInt(&cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_USAGE_OF_DEPRECATED);
	AddArgInt(&cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_ASSIGNMENT_WITHIN_CONDITIONAL_EXPR);
	AddArgInt(&cl_CommonFlags, CL_ENABLE_WARNING, CL_WARNING_SWITCH_FALLTHROUGH);
	AddArgNt(&cl_CommonFlags, CL_INCLUDE_DIR, rootDir);
	if (DEBUG_BUILD)
	{
		AddArg(&cl_CommonFlags, CL_DEBUG_INFO);
		//We don't care about these warnings in debug builds, but we will solve them when we go to build in release mode because they probably indicate mistakes at that point
		AddArgInt(&cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_SWITCH_ONLY_DEFAULT);
		AddArgInt(&cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_FUNC_PARAMETER);
		AddArgInt(&cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_LCOAL_VARIABLE);
		AddArgInt(&cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_CONDITIONAL_EXPR_IS_CONSTANT);
		AddArgInt(&cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_LOCAL_VAR_INIT_BUT_NOT_REFERENCED);
		AddArgInt(&cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_UNREACHABLE_CODE_DETECTED);
	}
	if (DUMP_PREPROCESSOR)
	{
		AddArg(&cl_CommonFlags, CL_PRECOMPILE_ONLY);
		AddArg(&cl_CommonFlags, CL_PRECOMPILE_PRESERVE_COMMENTS);
	}
	
	// +==============================+
	// |        cl_LangCFlags         |
	// +==============================+
	CliArgList cl_LangCFlags = ZEROED; //"c_cl_flags" flags that we use when compiling any C program using MSVC compiler
	AddArgNt(&cl_LangCFlags, CL_LANG_VERSION, "clatest"); //Use latest C language spec features
	AddArgNt(&cl_LangCFlags, CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
	
	// +==============================+
	// |       cl_LangCppFlags        |
	// +==============================+
	CliArgList cl_LangCppFlags = ZEROED; //"cpp_cl_flags" flags that we use when compiling any C++ program using MSVC compiler
	AddArgNt(&cl_LangCppFlags, CL_LANG_VERSION, "c++20");
	AddArgInt(&cl_LangCppFlags, CL_DISABLE_WARNING, CL_WARNING_ENUMERATION_MUST_HAVE_UNDERLYING_TYPE);
	AddArgInt(&cl_LangCppFlags, CL_DISABLE_WARNING, CL_WARNING_BITWISE_OP_BETWEEN_ENUMS);
	
	// +==============================+
	// |      clang_CommonFlags       |
	// +==============================+
	CliArgList clang_CommonFlags = ZEROED; //"common_clang_flags" flags that we use when compiling any C program using Clang
	AddArg(&clang_CommonFlags, CLANG_FULL_FILE_PATHS); //Print absolute paths in diagnostics TODO: Figure out how to resolve these back to windows paths for Sublime error linking?
	AddArgNt(&clang_CommonFlags, CLANG_LANG_VERSION, "gnu2x"); //Use C20+ language spec (NOTE: We originally had -std=c2x but that didn't define MAP_ANONYMOUS and mmap was failing)
	AddArgNt(&clang_CommonFlags, CLANG_WARNING_LEVEL, "all"); //This enables all the warnings about constructions that some users consider questionable, and that are easy to avoid (or modify to prevent the warning), even in conjunction with macros
	AddArgNt(&clang_CommonFlags, CLANG_WARNING_LEVEL, "extra"); //This enables some extra warning flags that are not enabled by -Wall
	AddArgNt(&clang_CommonFlags, CLANG_ENABLE_WARNING, CLANG_WARNING_SHADOWING);
	AddArgNt(&clang_CommonFlags, CLANG_ENABLE_WARNING, CLANG_WARNING_MISSING_FALLTHROUGH_IN_SWITCH);
	AddArgNt(&clang_CommonFlags, CLANG_DISABLE_WARNING, CLANG_WARNING_SWITCH_MISSING_CASES);
	AddArgNt(&clang_CommonFlags, CLANG_DISABLE_WARNING, CLANG_WARNING_UNUSED_FUNCTION);
	if (DEBUG_BUILD)
	{
		
		//We don't care about these warnings in debug builds, but we will solve them when we go to build in release mode because they probably indicate mistakes at that point
		AddArgNt(&clang_CommonFlags, CLANG_DISABLE_WARNING, "unused-parameter");
		AddArgNt(&clang_CommonFlags, CLANG_DISABLE_WARNING, "unused-variable");
	}
	if (DUMP_PREPROCESSOR) { AddArg(&clang_CommonFlags, CLANG_PRECOMPILE_ONLY); }
	
	// +==============================+
	// |       clang_LinuxFlags       |
	// +==============================+
	CliArgList clang_LinuxFlags = ZEROED; //"linux_clang_flags" flags for when we are compiling the linux version of a program using Clang
	AddArgNt(&clang_LinuxFlags, CLANG_INCLUDE_DIR, linux_rootDir);
	AddArg(&clang_LinuxFlags, "-mssse3"); //For MeowHash to work we need sse3 support
	AddArg(&clang_LinuxFlags, "-maes"); //For MeowHash to work we need aes support
	if (DEBUG_BUILD) { AddArgNt(&clang_LinuxFlags, CLANG_DEBUG_INFO, "dwarf-4"); }
	
	// +==============================+
	// |     cl_CommonLinkerFlags     |
	// +==============================+
	CliArgList cl_CommonLinkerFlags = ZEROED;
	AddArgNt(&cl_CommonLinkerFlags, LINK_LIBRARY_DIR, DEBUG_BUILD ? "..\\third_party\\_lib_debug" : "..\\third_party\\_lib_release");
	AddArg(&cl_CommonLinkerFlags, LINK_DISABLE_INCREMENTAL);
	
	// +==============================+
	// |  clang_LinuxCommonLibraries  |
	// +==============================+
	CliArgList clang_LinuxCommonLibraries = ZEROED; //"linux_linker_flags"
	AddArgNt(&clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "m"); //Include the math library (required for stuff like sinf, atan, etc.)
	AddArgNt(&clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "dl"); //Needed for dlopen and similar functions
	
	// +==============================+
	// |     cl_PigCoreLibraries      |
	// +==============================+
	CliArgList cl_PigCoreLibraries = ZEROED; //These are all the libraries we need when compiling a binary that contains code from PigCore (both pig_core.dll and tests.exe)
	if (BUILD_WITH_RAYLIB) { AddArgNt(&cl_PigCoreLibraries, CLI_QUOTED_ARG, "raylib.lib"); } //NOTE: raylib.lib MUST be before User32.lib and others
	AddArgNt(&cl_PigCoreLibraries, CLI_QUOTED_ARG, "Gdi32.lib"); //Needed for CreateFontA and other Windows graphics functions
	AddArgNt(&cl_PigCoreLibraries, CLI_QUOTED_ARG, "User32.lib"); //Needed for GetForegroundWindow, GetDC, etc.
	AddArgNt(&cl_PigCoreLibraries, CLI_QUOTED_ARG, "Ole32.lib"); //Needed for Combaseapi.h, CoInitializeEx, CoCreateInstance, etc.
	AddArgNt(&cl_PigCoreLibraries, CLI_QUOTED_ARG, "Shell32.lib"); //Needed for SHGetSpecialFolderPathA
	AddArgNt(&cl_PigCoreLibraries, CLI_QUOTED_ARG, "Shlwapi.lib"); //Needed for PathFileExistsA
	if (BUILD_WITH_RAYLIB)
	{
		AddArgNt(&cl_PigCoreLibraries, CLI_QUOTED_ARG, "Kernel32.lib");
		AddArgNt(&cl_PigCoreLibraries, CLI_QUOTED_ARG, "Winmm.lib");
	}
	if (BUILD_WITH_BOX2D) { AddArgNt(&cl_PigCoreLibraries, CLI_QUOTED_ARG, "box2d.lib"); }
	if (BUILD_WITH_SDL) { AddArgNt(&cl_PigCoreLibraries, CLI_QUOTED_ARG, "SDL2.lib"); }
	if (BUILD_WITH_OPENVR) { AddArgNt(&cl_PigCoreLibraries, CLI_QUOTED_ARG, "openvr_api.lib"); }
	if (BUILD_WITH_PHYSX) { AddArgNt(&cl_PigCoreLibraries, CLI_QUOTED_ARG, "PhysX_static_64.lib"); }
	
	// +==============================+
	// |    clang_PigCoreLibraries    |
	// +==============================+
	CliArgList clang_PigCoreLibraries = ZEROED; //These are all the libraries we need when compiling a binary that contains code from PigCore (both pig_core.so and tests)
	AddArgNt(&clang_PigCoreLibraries, CLANG_SYSTEM_LIBRARY, "fontconfig");
	if (BUILD_WITH_SOKOL_GFX) { AddArgNt(&clang_PigCoreLibraries, CLANG_SYSTEM_LIBRARY, "GL"); }
	
	// +==============================+
	// |      emscriptenSdkPath       |
	// +==============================+
	Str8 emscriptenSdkPath = ZEROED;
	if (BUILD_WEB && USE_EMSCRIPTEN)
	{
		const char* sdkEnvVariable = getenv("EMSCRIPTEN_SDK_PATH");
		if (sdkEnvVariable == nullptr)
		{
			WriteLine_E("Please set the EMSCRIPTEN_SDK_PATH environment variable before trying to build for the web with USE_EMSCRIPTEN");
			exit(7);
		}
		emscriptenSdkPath = NewStr8Nt(sdkEnvVariable);
		if (emscriptenSdkPath.chars[emscriptenSdkPath.length-1] == '\\' || emscriptenSdkPath.chars[emscriptenSdkPath.length-1] == '/') { emscriptenSdkPath.length--; }
		emscriptenSdkPath = CopyStr8(emscriptenSdkPath, true);
		PrintLine("Emscripten SDK path: \"%.*s\"", emscriptenSdkPath.length, emscriptenSdkPath.chars);
		InitializeEmsdkIf(&isEmsdkInitialized);
	}
	
	// +==============================+
	// |       clang_WasmFlags        |
	// +==============================+
	CliArgList clang_WasmFlags = ZEROED; //"wasm_clang_flags"
	AddArgNt(&clang_WasmFlags, CLANG_TARGET_ARCHITECTURE, "wasm32");
	AddArgNt(&clang_WasmFlags, CLANG_M_FLAG, "bulk-memory");
	AddArgNt(&clang_WasmFlags, CLANG_INCLUDE_DIR, linux_rootDir);
	if (DEBUG_BUILD) { AddArg(&clang_WasmFlags, CLANG_DEBUG_INFO_DEFAULT); }
	else { AddArgNt(&clang_WasmFlags, CLANG_OPTIMIZATION_LEVEL, "2"); }
	
	// +==============================+
	// |        clang_WebFlags        |
	// +==============================+
	CliArgList clang_WebFlags = ZEROED;
	if (USE_EMSCRIPTEN)
	{
		AddArgNt(&clang_WebFlags, EMSCRIPTEN_S_FLAG, "USE_SDL");
		AddArgNt(&clang_WebFlags, EMSCRIPTEN_S_FLAG, "ALLOW_MEMORY_GROWTH");
	}
	else
	{
		Str8 customStdLibDir = JoinStrings2(NewStr8Nt(linux_rootDir), StrLit("/wasm/std/include"), false);
		PrintLine("customStdLibDir: \"%.*s\"", customStdLibDir.length, customStdLibDir.chars);
		AddArgStr(&clang_WebFlags, CLANG_INCLUDE_DIR, customStdLibDir);
		AddArg(&clang_WebFlags, CLANG_NO_ENTRYPOINT);
		AddArg(&clang_WebFlags, CLANG_ALLOW_UNDEFINED);
		AddArg(&clang_WebFlags, CLANG_NO_STD_LIBRARIES);
		AddArg(&clang_WebFlags, CLANG_NO_STD_INCLUDES);
		AddArgNt(&clang_WebFlags, CLANG_EXPORT_SYMBOL, "__heap_base");
	}
	
	// +==============================+
	// |         orcaSdkPath          |
	// +==============================+
	Str8 orcaSdkPath = ZEROED;
	if (BUILD_ORCA)
	{
		#define FILENAME_ORCA_SDK_PATH "orca_sdk_path.txt"
		CliArgList cmd = ZEROED;
		AddArg(&cmd, "sdk-path");
		AddArgNt(&cmd, CLI_PIPE_OUTPUT_TO_FILE, FILENAME_ORCA_SDK_PATH);
		int statusCode = RunCliProgram(StrLit("orca"), &cmd);
		if (statusCode != 0)
		{
			PrintLine_E("Failed to run \"orca sdk-path\"! Status code: %d", statusCode);
			WriteLine_E("Make sure Orca SDK is installed and is added to the PATH!");
			exit(statusCode);
		}
		AssertFileExist(StrLit(FILENAME_ORCA_SDK_PATH), false);
		bool readSuccess = TryReadFile(StrLit(FILENAME_ORCA_SDK_PATH), &orcaSdkPath);
		assert(readSuccess == true);
		assert(orcaSdkPath.length > 0);
		FixPathSlashes(orcaSdkPath, '/');
		if (orcaSdkPath.chars[orcaSdkPath.length-1] == '/') { orcaSdkPath.length--; } //no trailing slash
		PrintLine("Orca SDK path: \"%.*s\"", orcaSdkPath.length, orcaSdkPath.chars);
	}
	
	// +==============================+
	// |        playdateSdkDir        |
	// +==============================+
	Str8 playdateSdkDir = ZEROED;
	Str8 playdateSdkDir_C_API = ZEROED;
	if (BUILD_PLAYDATE_DEVICE || BUILD_PLAYDATE_SIMULATOR)
	{
		const char* sdkEnvVariable = getenv("PLAYDATE_SDK_PATH");
		if (sdkEnvVariable == nullptr)
		{
			WriteLine_E("Please set the PLAYDATE_SDK_PATH environment variable before trying to build for the Playdate");
			exit(7);
		}
		playdateSdkDir = NewStr8Nt(sdkEnvVariable);
		if (playdateSdkDir.chars[playdateSdkDir.length-1] == '\\' || playdateSdkDir.chars[playdateSdkDir.length-1] == '/') { playdateSdkDir.length--; }
		playdateSdkDir = CopyStr8(playdateSdkDir, true);
		PrintLine("Playdate SDK path: \"%.*s\"", playdateSdkDir.length, playdateSdkDir.chars);
		playdateSdkDir_C_API = JoinStrings2(playdateSdkDir, StrLit("\\C_API"), false);
	}
	
	// +==============================+
	// |       clang_OrcaFlags        |
	// +==============================+
	CliArgList clang_OrcaFlags = ZEROED; //"orca_clang_flags"
	AddArg(&clang_OrcaFlags, CLANG_NO_ENTRYPOINT);
	AddArg(&clang_OrcaFlags, CLANG_EXPORT_DYNAMIC);
	AddArgStr(&clang_OrcaFlags, CLANG_STDLIB_FOLDER, JoinStrings2(orcaSdkPath, StrLit("/orca-libc"), false));
	AddArgStr(&clang_OrcaFlags, CLANG_INCLUDE_DIR, JoinStrings2(orcaSdkPath, StrLit("/src"), false));
	AddArgStr(&clang_OrcaFlags, CLANG_INCLUDE_DIR, JoinStrings2(orcaSdkPath, StrLit("/src/ext"), false));
	AddArgStr(&clang_OrcaFlags, CLANG_LIBRARY_DIR, JoinStrings2(orcaSdkPath, StrLit("/bin"), false));
	AddArgNt(&clang_OrcaFlags, CLANG_SYSTEM_LIBRARY, "orca_wasm");
	AddArgNt(&clang_OrcaFlags, CLANG_DEFINE, "__ORCA__"); //#define __ORCA__ so that base_compiler_check.h can set TARGET_IS_ORCA
	
	// +====================================+
	// | cl_PlaydateSimulatorCompilerFlags  |
	// +====================================+
	CliArgList cl_PlaydateSimulatorCompilerFlags = ZEROED;
	
	//TODO: Just use cl_CommonFlags?
	AddArg(&cl_PlaydateSimulatorCompilerFlags, CL_NO_LOGO);
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_WARNING_LEVEL, "3");
	AddArg(&cl_PlaydateSimulatorCompilerFlags, CL_NO_WARNINGS_AS_ERRORS);
	AddArg(&cl_PlaydateSimulatorCompilerFlags, DEBUG_BUILD ? CL_STD_LIB_DYNAMIC_DBG : CL_STD_LIB_DYNAMIC);
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "d" : "2");
	if (DEBUG_BUILD) { AddArg(&cl_PlaydateSimulatorCompilerFlags, CL_DEBUG_INFO); }
	
	//TODO: Just use cl_LangCFlags?
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_LANG_VERSION, "clatest"); //Use latest C language spec features
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
	
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_INCLUDE_DIR, rootDir);
	if (BUILD_PLAYDATE_SIMULATOR) { AddArgStr(&cl_PlaydateSimulatorCompilerFlags, CL_INCLUDE_DIR, playdateSdkDir_C_API); } //NOTE: playdateSdkDir_C_API is only filled if BUILD_PLAYDATE_SIMULATOR
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "TARGET_SIMULATOR=1");
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "TARGET_EXTENSION=1");
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "__HEAP_SIZE=8388208");
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "__STACK_SIZE=61800");
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "_WINDLL");
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "_MBCS");
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "WIN32");
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "_WINDOWS");
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "_WINDLL=1");
	AddArg(&cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_BUFFER_SECURITY_CHECK);
	AddArg(&cl_PlaydateSimulatorCompilerFlags, CL_DISABLE_MINIMAL_REBUILD);
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_RUNTIME_CHECKS, "1"); //Enable fast runtime checks (Equivalent to "su")
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_CALLING_CONVENTION, "d"); //Use __cdecl calling convention
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_INLINE_EXPANSION_LEVEL, "0"); //Disable inline expansions
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_INTERNAL_COMPILER_ERROR_BEHAVIOR, "prompt"); //TODO: Do we need this?
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_LANG_CONFORMANCE_OPTION, "forScope"); //Enforce Standard C++ for scoping rules (on by default)
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_LANG_CONFORMANCE_OPTION, "inline"); //Remove unreferenced functions or data if they're COMDAT or have internal linkage only (off by default)
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_LANG_CONFORMANCE_OPTION, "wchar_t"); //wchar_t is a native type, not a typedef (on by default)
	AddArgNt(&cl_PlaydateSimulatorCompilerFlags, CL_FLOATING_POINT_MODEL, "precise"); //"precise" floating-point model; results are predictable
	
	// +====================================+
	// | link_PlaydateSimulatorLinkerFlags  |
	// +====================================+
	CliArgList link_PlaydateSimulatorLinkerFlags = ZEROED;
	AddArg(&link_PlaydateSimulatorLinkerFlags, LINK_NO_LOGO);
	AddArgNt(&link_PlaydateSimulatorLinkerFlags, LINK_TARGET_ARCHITECTURE, "X64");
	AddArg(&link_PlaydateSimulatorLinkerFlags, LINK_DATA_EXEC_COMPAT);
	AddArg(&link_PlaydateSimulatorLinkerFlags, LINK_ENABLE_ASLR);
	AddArg(&link_PlaydateSimulatorLinkerFlags, LINK_CONSOLE_APPLICATION);
	AddArgInt(&link_PlaydateSimulatorLinkerFlags, LINK_TYPELIB_RESOURCE_ID, 1);
	AddArg(&link_PlaydateSimulatorLinkerFlags, LINK_ENABLE_INCREMENTAL);
	AddArgNt(&link_PlaydateSimulatorLinkerFlags, LINK_INCREMENTAL_FILE_NAME, "tests.ilk"); //TODO: This should really move down below inside the tests.exe block
	AddArg(&link_PlaydateSimulatorLinkerFlags, LINK_CREATE_ASSEMBLY_MANIFEST);
	AddArgNt(&link_PlaydateSimulatorLinkerFlags, LINK_ASSEMBLY_MANIFEST_FILE, "tests.intermediate.manifest"); //TODO: This should really move down below inside the tests.exe block
	AddArgNt(&link_PlaydateSimulatorLinkerFlags, LINK_LINK_TIME_CODEGEN_FILE, "tests.iobj"); //TODO: This should really move down below inside the tests.exe block
	AddArgNt(&link_PlaydateSimulatorLinkerFlags, LINK_EMBED_UAC_INFO_EX, "level='asInvoker' uiAccess='false'");
	if (DEBUG_BUILD) { AddArg(&link_PlaydateSimulatorLinkerFlags, LINK_DEBUG_INFO); }
	
	// +==================================+
	// | link_PlaydateSimulatorLibraries  |
	// +==================================+
	CliArgList link_PlaydateSimulatorLibraries = ZEROED;
	AddArgNt(&link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "kernel32.lib");
	AddArgNt(&link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "user32.lib");
	AddArgNt(&link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "gdi32.lib");
	AddArgNt(&link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "winspool.lib");
	AddArgNt(&link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "shell32.lib");
	AddArgNt(&link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "ole32.lib");
	AddArgNt(&link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "oleaut32.lib");
	AddArgNt(&link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "uuid.lib");
	AddArgNt(&link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "comdlg32.lib");
	AddArgNt(&link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "advapi32.lib");
	
	// +===============================+
	// | gcc_PlaydateDeviceCommonFlags |
	// +===============================+
	CliArgList gcc_PlaydateDeviceCommonFlags = ZEROED;
	AddArgNt(&gcc_PlaydateDeviceCommonFlags, GCC_INCLUDE_DIR, rootDir);
	if (BUILD_PLAYDATE_DEVICE) { AddArgStr(&gcc_PlaydateDeviceCommonFlags, GCC_INCLUDE_DIR, playdateSdkDir_C_API); } //NOTE: playdateSdkDir_C_API is only filled if BUILD_PLAYDATE_DEVICE
	AddArgNt(&gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "TARGET_PLAYDATE=1");
	AddArgNt(&gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "TARGET_EXTENSION=1");
	AddArgNt(&gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "__HEAP_SIZE=8388208");
	AddArgNt(&gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "__STACK_SIZE=61800");
	AddArgNt(&gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "__FPU_USED=1");
	AddArgNt(&gcc_PlaydateDeviceCommonFlags, GCC_USE_SPEC_FILE, "nano.specs"); //Required for things like _read, _write, _exit, etc. to not be pulled in as requirements from standard library
	AddArgNt(&gcc_PlaydateDeviceCommonFlags, GCC_USE_SPEC_FILE, "nosys.specs"); //TODO: Is this helping?
	AddArg(&gcc_PlaydateDeviceCommonFlags, GCC_TARGET_THUMB);
	AddArgNt(&gcc_PlaydateDeviceCommonFlags, GCC_TARGET_CPU, "cortex-m7");
	AddArgNt(&gcc_PlaydateDeviceCommonFlags, GCC_FLOAT_ABI_MODE, "hard"); //Use hardware for floating-point operations
	AddArgNt(&gcc_PlaydateDeviceCommonFlags, GCC_TARGET_FPU, "fpv5-sp-d16");
	
	// +==================================+
	// | gcc_PlaydateDeviceCompilerFlags  |
	// +==================================+
	CliArgList gcc_PlaydateDeviceCompilerFlags = ZEROED;
	AddArgNt(&gcc_PlaydateDeviceCompilerFlags, GCC_DEBUG_INFO_EX, "3");
	AddArgNt(&gcc_PlaydateDeviceCompilerFlags, GCC_DEBUG_INFO_EX, "dwarf-2");
	AddArg(&gcc_PlaydateDeviceCompilerFlags, GCC_STD_LIB_DYNAMIC);
	AddArgNt(&gcc_PlaydateDeviceCompilerFlags, GCC_DEPENDENCY_FILE, "tests.d"); //TODO: This should really move down below inside the tests.exe block
	AddArgInt(&gcc_PlaydateDeviceCompilerFlags, GCC_ALIGN_FUNCS_TO, 16);
	AddArg(&gcc_PlaydateDeviceCompilerFlags, GCC_SEP_DATA_SECTIONS);
	AddArg(&gcc_PlaydateDeviceCompilerFlags, GCC_SEP_FUNC_SECTIONS);
	AddArg(&gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_EXCEPTIONS);
	AddArg(&gcc_PlaydateDeviceCompilerFlags, GCC_OMIT_FRAME_PNTR);
	AddArg(&gcc_PlaydateDeviceCompilerFlags, GCC_GLOBAL_VAR_NO_COMMON);
	AddArg(&gcc_PlaydateDeviceCompilerFlags, GCC_VERBOSE_ASSEMBLY); //TODO: Should this only be on when DEBUG_BUILD?
	AddArg(&gcc_PlaydateDeviceCompilerFlags, GCC_ONLY_RELOC_WORD_SIZE);
	AddArgNt(&gcc_PlaydateDeviceCompilerFlags, GCC_WARNING_LEVEL, "all");
	// AddArgNt(&gcc_PlaydateDeviceCompilerFlags, GCC_ENABLE_WARNING, "double-promotion");
	AddArgNt(&gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "unknown-pragmas");
	AddArgNt(&gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "comment");
	AddArgNt(&gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "switch");
	AddArgNt(&gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "nonnull");
	AddArgNt(&gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "unused");
	AddArgNt(&gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "missing-braces");
	AddArgNt(&gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "char-subscripts");
	
	// +===============================+
	// | gcc_PlaydateDeviceLinkerFlags |
	// +===============================+
	CliArgList gcc_PlaydateDeviceLinkerFlags = ZEROED;
	AddArg(&gcc_PlaydateDeviceLinkerFlags, GCC_NO_STD_STARTUP);
	AddArgNt(&gcc_PlaydateDeviceLinkerFlags, GCC_ENTRYPOINT_NAME, "eventHandler");
	AddArg(&gcc_PlaydateDeviceLinkerFlags, GCC_DISABLE_RWX_WARNING);
	AddArg(&gcc_PlaydateDeviceLinkerFlags, GCC_CREF);
	AddArg(&gcc_PlaydateDeviceLinkerFlags, GCC_GC_SECTIONS);
	AddArg(&gcc_PlaydateDeviceLinkerFlags, GCC_DISABLE_MISMATCH_WARNING);
	AddArg(&gcc_PlaydateDeviceLinkerFlags, GCC_EMIT_RELOCATIONS);
	Str8 playdateLinkerScriptPath = JoinStrings2(playdateSdkDir, StrLit("\\C_API\\buildsupport\\link_map.ld"), false);
	FixPathSlashes(playdateLinkerScriptPath, '\\');
	AddArgStr(&gcc_PlaydateDeviceLinkerFlags, GCC_LINKER_SCRIPT, playdateLinkerScriptPath);
	
	
	// +==============================+
	// |       pdc_CommonFlags        |
	// +==============================+
	CliArgList pdc_CommonFlags = ZEROED;
	AddArg(&pdc_CommonFlags, PDC_QUIET); //Quiet mode, suppress non-error output
	if (BUILD_PLAYDATE_DEVICE || BUILD_PLAYDATE_SIMULATOR) { AddArgStr(&pdc_CommonFlags, PDC_SDK_PATH, playdateSdkDir); } //NOTE: playdateSdkDir is only filled if BUILD_PLAYDATE_DEVICE || BUILD_PLAYDATE_SIMULATOR
	
	// +--------------------------------------------------------------+
	// |                       Build piggen.exe                       |
	// +--------------------------------------------------------------+
	#define FILENAME_PIGGEN "piggen.exe"
	#define LINUX_FILENAME_PIGGEN "piggen"
	if (RUN_PIGGEN && !BUILD_PIGGEN && !DoesFileExist(StrLit(FILENAME_PIGGEN))) { PrintLine("Building %s because it's missing", FILENAME_PIGGEN); BUILD_PIGGEN = true; }
	if (BUILD_PIGGEN)
	{
		// +==============================+
		// |      cl_PiggenLibraries      |
		// +==============================+
		CliArgList cl_PiggenLibraries = ZEROED;
		AddArg(&cl_PiggenLibraries, "Shlwapi.lib"); //Needed for PathFileExistsA
		
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(&isMsvcInitialized);
			PrintLine("\n[Building %s for Windows...]", FILENAME_PIGGEN);
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, "..\\piggen\\piggen_main.c");
			AddArgNt(&cmd, CL_BINARY_FILE, FILENAME_PIGGEN);
			AddArgList(&cmd, &cl_CommonFlags);
			AddArgList(&cmd, &cl_LangCFlags);
			AddArg(&cmd, CL_LINK);
			AddArgList(&cmd, &cl_PiggenLibraries);
			AddArgList(&cmd, &cl_CommonLinkerFlags);
			
			int statusCode = RunCliProgram(msvcCompiler, &cmd);
			if (statusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_PIGGEN), true);
				PrintLine("[Built %s for Windows!]", FILENAME_PIGGEN);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", FILENAME_PIGGEN, statusCode);
				exit(statusCode);
			}
		}
		if (BUILD_LINUX)
		{
			PrintLine("\n[Building %s for Linux...]", LINUX_FILENAME_PIGGEN);
			
			mkdir("linux", 0);
			chdir("linux");
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, "../../piggen/piggen_main.c");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, LINUX_FILENAME_PIGGEN);
			AddArgList(&cmd, &clang_CommonFlags);
			AddArgList(&cmd, &clang_LinuxFlags);
			AddArgList(&cmd, &clang_LinuxCommonLibraries);
			
			int statusCode = RunCliProgram(wslClangCompiler, &cmd);
			if (statusCode == 0)
			{
				AssertFileExist(StrLit(LINUX_FILENAME_PIGGEN), true);
				PrintLine("[Built %s for Linux!]", LINUX_FILENAME_PIGGEN);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", LINUX_FILENAME_PIGGEN, statusCode);
				exit(statusCode);
			}
			
			chdir("..");
		}
	}
	
	// +--------------------------------------------------------------+
	// |                        Run piggen.exe                        |
	// +--------------------------------------------------------------+
	#define FOLDERNAME_GENERATED_CODE "gen"
	if (RUN_PIGGEN)
	{
		PrintLine("\n[%s]", FILENAME_PIGGEN);
		
		#define PIGGEN_OUTPUT_FOLDER "-o=\"[VAL]\""
		#define PIGGEN_EXCLUDE_FOLDER "-e=\"[VAL]\""
		
		CliArgList cmd = ZEROED;
		AddArg(&cmd, rootDir);
		AddArgNt(&cmd, PIGGEN_OUTPUT_FOLDER, FOLDERNAME_GENERATED_CODE "/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "../base/base_defines_check.h");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "../piggen/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "../tools/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "../wasm/std/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "../.git/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "../_scripts/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "../_media/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "../_template/");
		
		int statusCode = RunCliProgram(StrLit(FILENAME_PIGGEN), &cmd);
		if (statusCode != 0)
		{
			PrintLine_E("%s Failed! Status Code: %d", FILENAME_PIGGEN, statusCode);
			exit(statusCode);
		}
	}
	
	// +--------------------------------------------------------------+
	// |                        Build Shaders                         |
	// +--------------------------------------------------------------+
	FindShadersContext findContext = ZEROED;
	CliArgList cl_ShaderObjects = ZEROED;
	CliArgList clang_ShaderObjects = ZEROED;
	if (BUILD_SHADERS || BUILD_WITH_SOKOL_GFX)
	{
		RecursiveDirWalk(StrLit(".."), FindShaderFilesCallback, &findContext);
		
		if (BUILD_WINDOWS)
		{
			for (uxx sIndex = 0; sIndex < findContext.objPaths.length; sIndex++)
			{
				Str8 objPath = findContext.objPaths.strings[sIndex];
				AddArgStr(&cl_ShaderObjects, CLI_QUOTED_ARG, objPath);
				if (!DoesFileExist(objPath) && !BUILD_SHADERS) { PrintLine("Building shaders because \"%.*s\" is missing!", objPath.length, objPath.chars); BUILD_SHADERS = true; }
			}
		}
		if (BUILD_LINUX)
		{
			for (uxx sIndex = 0; sIndex < findContext.oPaths.length; sIndex++)
			{
				Str8 oPath = findContext.oPaths.strings[sIndex];
				AddArgStr(&clang_ShaderObjects, CLI_QUOTED_ARG, oPath);
				Str8 oPathWithFolder = JoinStrings2(StrLit("linux/"), oPath, false);
				if (!DoesFileExist(oPathWithFolder) && !BUILD_SHADERS) { PrintLine("Building shaders because \"%.*s\" is missing!", oPathWithFolder.length, oPathWithFolder.chars); BUILD_SHADERS = true; }
			}
		}
		
		if (!BUILD_SHADERS)
		{
			FreeStrArray(&findContext.shaderPaths);
			FreeStrArray(&findContext.headerPaths);
			FreeStrArray(&findContext.sourcePaths);
			FreeStrArray(&findContext.objPaths);
			FreeStrArray(&findContext.oPaths);
		}
	}
	
	if (BUILD_SHADERS)
	{
		if (BUILD_WINDOWS) { InitializeMsvcIf(&isMsvcInitialized); }
		
		PrintLine("Found %u shader%s", findContext.shaderPaths.length, findContext.shaderPaths.length == 1 ? "" : "s");
		// for (uxx sIndex = 0; sIndex < findContext.shaderPaths.length; sIndex++)
		// {
		// 	PrintLine("Shader[%u]", sIndex);
		// 	PrintLine("\t\"%.*s\"", findContext.shaderPaths.strings[sIndex].length, findContext.shaderPaths.strings[sIndex].chars);
		// 	PrintLine("\t\"%.*s\"", findContext.headerPaths.strings[sIndex].length, findContext.headerPaths.strings[sIndex].chars);
		// 	PrintLine("\t\"%.*s\"", findContext.sourcePaths.strings[sIndex].length, findContext.sourcePaths.strings[sIndex].chars);
		// 	PrintLine("\t\"%.*s\"", findContext.objPaths.strings[sIndex].length, findContext.objPaths.strings[sIndex].chars);
		// 	PrintLine("\t\"%.*s\"", findContext.oPaths.strings[sIndex].length, findContext.oPaths.strings[sIndex].chars);
		// }
		
		// First use shdc.exe to generate header files for each .glsl file
		for (uxx sIndex = 0; sIndex < findContext.shaderPaths.length; sIndex++)
		{
			Str8 shaderPath = findContext.shaderPaths.strings[sIndex];
			Str8 headerPath = findContext.headerPaths.strings[sIndex];
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, SHDC_FORMAT, "sokol_impl");
			AddArgNt(&cmd, SHDC_ERROR_FORMAT, "msvc");
			// AddArg(&cmd, SHDC_REFLECTION);
			AddArgNt(&cmd, SHDC_SHADER_LANGUAGES, "hlsl5:glsl430:metal_macos");
			AddArgStr(&cmd, SHDC_INPUT, shaderPath);
			AddArgStr(&cmd, SHDC_OUTPUT, headerPath);
			
			PrintLine("Generating \"%.*s\"...", headerPath.length, headerPath.chars);
			int statusCode = RunCliProgram(shdcExe, &cmd);
			if (statusCode != 0)
			{
				Str8 shdcFilename = GetFileNamePart(shdcExe, true);
				PrintLine_E("%.*s failed on %.*s! Status Code: %d",
					shdcFilename.length, shdcFilename.chars,
					shaderPath.length, shaderPath.chars,
					statusCode
				);
				exit(statusCode);
			}
			else
			{
				AssertFileExist(headerPath, true);
				ScrapeShaderHeaderFileAndAddExtraInfo(headerPath, shaderPath);
			}
		}
		
		//Then compile each header file to an .o/.obj file
		for (uxx sIndex = 0; sIndex < findContext.shaderPaths.length; sIndex++)
		{
			Str8 headerPath = findContext.headerPaths.strings[sIndex];
			Str8 sourcePath = findContext.sourcePaths.strings[sIndex];
			Str8 headerFileName = GetFileNamePart(headerPath, true);
			Str8 headerDirectory = GetDirectoryPart(headerPath, true);
			
			//We need a .c file that #includes shader_include.h (which defines SOKOL_SHDC_IMPL) and then the shader header file
			Str8 sourceFileContents = JoinStrings3(
				StrLit("\n#include \"shader_include.h\"\n\n#include \""),
				headerFileName,
				StrLit("\"\n"),
				false
			);
			PrintLine("Generating \"%.*s\"...", sourcePath.length, sourcePath.chars);
			CreateAndWriteFile(sourcePath, sourceFileContents, true);
			
			if (BUILD_WINDOWS)
			{
				Str8 objPath = findContext.objPaths.strings[sIndex];
				Str8 fixedSourcePath = CopyStr8(sourcePath, false);
				FixPathSlashes(fixedSourcePath, '\\');
				Str8 fixedHeaderDirectory = CopyStr8(headerDirectory, false);
				FixPathSlashes(fixedHeaderDirectory, '\\');
				
				CliArgList cmd = ZEROED;
				AddArg(&cmd, CL_COMPILE);
				AddArgStr(&cmd, CLI_QUOTED_ARG, fixedSourcePath);
				AddArgStr(&cmd, CL_OBJ_FILE, objPath);
				AddArgStr(&cmd, CL_INCLUDE_DIR, fixedHeaderDirectory);
				AddArgList(&cmd, &cl_CommonFlags);
				AddArgList(&cmd, &cl_LangCFlags);
				
				
				int statusCode = RunCliProgram(msvcCompiler, &cmd);
				if (statusCode == 0)
				{
					AssertFileExist(objPath, true);
				}
				else
				{
					PrintLine_E("Failed to build %.*s for WINDOWS! Compiler Status Code: %d", sourcePath.length, sourcePath.chars, statusCode);
					exit(statusCode);
				}
			}
			if (BUILD_LINUX)
			{
				mkdir("linux", 0);
				chdir("linux");
				
				Str8 oPath = findContext.oPaths.strings[sIndex];
				Str8 fixedSourcePath = JoinStrings2(StrLit("../"), sourcePath, false);
				FixPathSlashes(fixedSourcePath, '/');
				Str8 fixedHeaderDirectory = JoinStrings2(StrLit("../"), headerDirectory, false);
				FixPathSlashes(fixedHeaderDirectory, '/');
				
				CliArgList cmd = ZEROED;
				AddArg(&cmd, CLANG_COMPILE);
				AddArgStr(&cmd, CLI_QUOTED_ARG, fixedSourcePath);
				AddArgStr(&cmd, CLANG_OUTPUT_FILE, oPath);
				AddArgStr(&cmd, CLANG_INCLUDE_DIR, fixedHeaderDirectory);
				AddArgList(&cmd, &clang_CommonFlags);
				AddArgList(&cmd, &clang_LinuxFlags);
				
				int statusCode = RunCliProgram(wslClangCompiler, &cmd);
				if (statusCode == 0)
				{
					AssertFileExist(oPath, true);
				}
				else
				{
					PrintLine_E("Failed to build %.*s for LINUX! Compiler Status Code: %d", sourcePath.length, sourcePath.chars, statusCode);
					exit(statusCode);
				}
				
				chdir("..");
			}
		}
		
		FreeStrArray(&findContext.shaderPaths);
		FreeStrArray(&findContext.headerPaths);
		FreeStrArray(&findContext.sourcePaths);
		FreeStrArray(&findContext.objPaths);
		FreeStrArray(&findContext.oPaths);
	} 
	
	// +--------------------------------------------------------------+
	// |                       Build imgui.obj                        |
	// +--------------------------------------------------------------+
	#define FILENAME_IMGUI "imgui.obj"
	#define LINUX_FILENAME_IMGUI "imgui.o"
	if (BUILD_WITH_IMGUI && !BUILD_IMGUI_OBJ && BUILD_WINDOWS && !DoesFileExist(StrLit(FILENAME_IMGUI))) { PrintLine("Building %s because it's missing", FILENAME_IMGUI); BUILD_IMGUI_OBJ = true; }
	if (BUILD_WITH_IMGUI && !BUILD_IMGUI_OBJ && BUILD_LINUX && !DoesFileExist(StrLit(LINUX_FILENAME_IMGUI))) { PrintLine("Building %s because it's missing", LINUX_FILENAME_IMGUI); BUILD_IMGUI_OBJ = true; }
	if (BUILD_IMGUI_OBJ)
	{
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(&isMsvcInitialized);
			PrintLine("[Building %s for Windows...]", FILENAME_IMGUI);
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, CL_COMPILE);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "..\\ui\\ui_imgui_main.cpp");
			AddArgNt(&cmd, CL_INCLUDE_DIR, "..\\third_party\\imgui");
			AddArgNt(&cmd, CL_OBJ_FILE, FILENAME_IMGUI);
			AddArgList(&cmd, &cl_CommonFlags);
			AddArgList(&cmd, &cl_LangCppFlags);
			AddArg(&cmd, CL_LINK);
			AddArgList(&cmd, &cl_CommonLinkerFlags);
			
			int statusCode = RunCliProgram(msvcCompiler, &cmd);
			if (statusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_IMGUI), true);
				PrintLine("[Built %s for Windows!]", FILENAME_IMGUI);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", FILENAME_IMGUI, statusCode);
				exit(statusCode);
			}
		}
		if (BUILD_LINUX)
		{
			//TODO: Implement Linux version!
		}
	}
	
	// +--------------------------------------------------------------+
	// |                     Build physx_capi.obj                     |
	// +--------------------------------------------------------------+
	#define FILENAME_PHYSX "physx_capi.obj"
	#define LINUX_FILENAME_PHYSX "physx_capi.o"
	//TODO: Check if imgui.obj exists, and auto-build even if not requested
	if (BUILD_WITH_PHYSX && !BUILD_PHYSX_OBJ && BUILD_WINDOWS && !DoesFileExist(StrLit(FILENAME_PHYSX))) { PrintLine("Building %s because it's missing", FILENAME_PHYSX); BUILD_WITH_PHYSX = true; }
	if (BUILD_WITH_PHYSX && !BUILD_PHYSX_OBJ && BUILD_LINUX && !DoesFileExist(StrLit(LINUX_FILENAME_PHYSX))) { PrintLine("Building %s because it's missing", LINUX_FILENAME_PHYSX); BUILD_WITH_PHYSX = true; }
	if (BUILD_PHYSX_OBJ)
	{
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(&isMsvcInitialized);
			PrintLine("[Building %s for Windows...]", FILENAME_PHYSX);
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, CL_COMPILE);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "..\\phys\\phys_physx_capi_main.cpp");
			AddArgNt(&cmd, CL_INCLUDE_DIR, "..\\third_party\\physx");
			AddArgNt(&cmd, CL_OBJ_FILE, FILENAME_PHYSX);
			AddArgList(&cmd, &cl_CommonFlags);
			AddArgList(&cmd, &cl_LangCppFlags);
			AddArg(&cmd, CL_LINK);
			AddArgList(&cmd, &cl_CommonLinkerFlags);
			
			int statusCode = RunCliProgram(msvcCompiler, &cmd);
			if (statusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_PHYSX), true);
				PrintLine("[Built %s for Windows!]", FILENAME_PHYSX);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", FILENAME_PHYSX, statusCode);
				exit(statusCode);
			}
		}
		if (BUILD_LINUX)
		{
			//TODO: Implement Linux version!
		}
	}
	if (BUILD_WITH_PHYSX) { AddArgNt(&cl_PigCoreLibraries, CLI_QUOTED_ARG, FILENAME_PHYSX); }
	
	// +--------------------------------------------------------------+
	// |                      Build pig_core.dll                      |
	// +--------------------------------------------------------------+
	#define FILENAME_PIGCORE "pig_core.dll"
	#define LINUX_FILENAME_PIGCORE "libpig_core.so"
	if (BUILD_PIG_CORE_DLL)
	{
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(&isMsvcInitialized);
			PrintLine("\n[Building %s for Windows...]", FILENAME_PIGCORE);
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, "..\\dll\\dll_main.c");
			AddArgNt(&cmd, CL_BINARY_FILE, FILENAME_PIGCORE);
			AddArgList(&cmd, &cl_CommonFlags);
			AddArgList(&cmd, &cl_LangCFlags);
			AddArg(&cmd, CL_LINK);
			AddArg(&cmd, LINK_BUILD_DLL);
			AddArgList(&cmd, &cl_CommonLinkerFlags);
			AddArgList(&cmd, &cl_PigCoreLibraries);
			
			int statusCode = RunCliProgram(msvcCompiler, &cmd);
			if (statusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_PIGCORE), true);
				PrintLine("[Built %s for Windows!]", FILENAME_PIGCORE);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", FILENAME_PIGCORE, statusCode);
				exit(statusCode);
			}
		}
		if (BUILD_LINUX)
		{
			PrintLine("\n[Building %s for Linux...]", LINUX_FILENAME_PIGCORE);
			
			mkdir("linux", 0);
			chdir("linux");
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, "../../dll/dll_main.c");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, LINUX_FILENAME_PIGCORE);
			AddArg(&cmd, CLANG_BUILD_SHARED_LIB);
			AddArg(&cmd, CLANG_fPIC);
			AddArgList(&cmd, &clang_CommonFlags);
			AddArgList(&cmd, &clang_LinuxFlags);
			AddArgList(&cmd, &clang_LinuxCommonLibraries);
			AddArgList(&cmd, &clang_PigCoreLibraries);
			
			int statusCode = RunCliProgram(wslClangCompiler, &cmd);
			if (statusCode == 0)
			{
				AssertFileExist(StrLit(LINUX_FILENAME_PIGCORE), true);
				PrintLine("[Built %s for Linux!]", LINUX_FILENAME_PIGCORE);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", LINUX_FILENAME_PIGCORE, statusCode);
				exit(statusCode);
			}
			
			chdir("..");
		}
	}
	
	// +--------------------------------------------------------------+
	// |                       Build tests.exe                        |
	// +--------------------------------------------------------------+
	#define FILENAME_TESTS "tests.exe"
	#define LINUX_FILENAME_TESTS "tests"
	#define WEB_FILENAME_TESTS "app.wasm"
	#define WEB_FILENAME_INDEX_HTML "index.html"
	#define ORCA_FILENAME_TESTS "module.wasm"
	#define FILENAME_TESTS_OBJ "tests.obj"
	#define PLAYDATE_FILENAME_TESTS "pdex.elf"
	#define PLAYDATESIM_FILENAME_TESTS "pdex.dll"
	#define FILENAME_TESTS_PDX "tests.pdx"
	if (RUN_TESTS && !BUILD_TESTS && !DoesFileExist(StrLit(FILENAME_TESTS))) { PrintLine("Building %s because it's missing", FILENAME_TESTS); BUILD_TESTS = true; BUILD_WINDOWS = true; }
	if (BUILD_TESTS)
	{
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(&isMsvcInitialized);
			PrintLine("\n[Building %s for Windows...]", FILENAME_TESTS);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, "..\\tests\\tests_main.c");
			AddArgNt(&cmd, CL_BINARY_FILE, FILENAME_TESTS);
			AddArgList(&cmd, &cl_CommonFlags);
			AddArgList(&cmd, &cl_LangCFlags);
			AddArg(&cmd, CL_LINK);
			AddArgList(&cmd, &cl_CommonLinkerFlags);
			AddArgList(&cmd, &cl_PigCoreLibraries);
			AddArgList(&cmd, &cl_ShaderObjects);
			
			int statusCode = RunCliProgram(msvcCompiler, &cmd);
			if (statusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_TESTS), true);
				PrintLine("[Built %s for Windows!]", FILENAME_TESTS);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", FILENAME_TESTS, statusCode);
				exit(statusCode);
			}
		}
		
		if (BUILD_LINUX)
		{
			PrintLine("\n[Building %s for Linux...]", LINUX_FILENAME_TESTS);
			
			mkdir("linux", 0);
			chdir("linux");
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, "../../tests/tests_main.c");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, LINUX_FILENAME_TESTS);
			AddArgList(&cmd, &clang_CommonFlags);
			AddArgList(&cmd, &clang_LinuxFlags);
			AddArgList(&cmd, &clang_LinuxCommonLibraries);
			AddArgList(&cmd, &clang_PigCoreLibraries);
			AddArgList(&cmd, &clang_ShaderObjects);
			
			int statusCode = RunCliProgram(wslClangCompiler, &cmd);
			if (statusCode == 0)
			{
				AssertFileExist(StrLit(LINUX_FILENAME_TESTS), true);
				PrintLine("[Built %s for Linux!]", LINUX_FILENAME_TESTS);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", LINUX_FILENAME_TESTS, statusCode);
				exit(statusCode);
			}
			
			chdir("..");
		}
		
		if (BUILD_WEB)
		{
			PrintLine("\n[Building %s for Web...]", WEB_FILENAME_TESTS);
			
			mkdir("web", 0);
			chdir("web");
			
			// TODO: del *.wasm > NUL 2> NUL
			// TODO: del *.wat > NUL 2> NUL
			// TODO: del *.css > NUL 2> NUL
			// TODO: del *.html > NUL 2> NUL
			// TODO: del *.js > NUL 2> NUL
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, "../../tests/tests_main.c");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, USE_EMSCRIPTEN ? WEB_FILENAME_INDEX_HTML : WEB_FILENAME_TESTS);
			AddArgList(&cmd, &clang_CommonFlags);
			AddArgList(&cmd, &clang_WasmFlags);
			AddArgList(&cmd, &clang_WebFlags);
			
			int statusCode = RunCliProgram(USE_EMSCRIPTEN ? emscriptenCompiler : clangCompiler, &cmd);
			if (statusCode == 0)
			{
				if (USE_EMSCRIPTEN)
				{
					AssertFileExist(StrLit(WEB_FILENAME_INDEX_HTML), true);
					AssertFileExist(StrLit("index.wasm"), true);
					AssertFileExist(StrLit("index.js"), true);
				}
				else
				{
					AssertFileExist(NewStr8Nt(WEB_FILENAME_TESTS), true);
				}
				PrintLine("[Built %s for Web!]", WEB_FILENAME_TESTS);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", WEB_FILENAME_TESTS, statusCode);
				exit(statusCode);
			}
			
			//TODO: CONVERT_WASM_TO_WAT
			
			if (!USE_EMSCRIPTEN)
			{
				StrArray javascriptFiles = ZEROED;
				AddStr(&javascriptFiles, StrLit("..\\..\\wasm\\wasm_globals.js"));
				AddStr(&javascriptFiles, StrLit("..\\..\\wasm\\std\\include\\internal\\wasm_std_js_api.js"));
				AddStr(&javascriptFiles, StrLit("..\\..\\wasm\\wasm_app_js_api.js"));
				AddStr(&javascriptFiles, StrLit("..\\..\\wasm\\wasm_main.js"));
				ConcatAllFilesIntoSingleFile(&javascriptFiles, StrLit("combined.js"));
				
				CopyFileToPath(StrLit("..\\..\\wasm\\wasm_app_style.css"), StrLit("main.css"));
				CopyFileToPath(StrLit("..\\..\\wasm\\wasm_app_index.html"), StrLit("index.html"));
			}
			
			chdir("..");
		}
		
		if (BUILD_ORCA)
		{
			PrintLine("\n[Building %s for Orca...]", ORCA_FILENAME_TESTS);
			
			mkdir("orca", 0);
			chdir("orca");
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, ORCA_FILENAME_TESTS);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "../../tests/tests_main.c");
			AddArgList(&cmd, &clang_CommonFlags);
			AddArgList(&cmd, &clang_WasmFlags);
			AddArgList(&cmd, &clang_OrcaFlags);
			
			int statusCode = RunCliProgram(clangCompiler, &cmd);
			if (statusCode == 0)
			{
				AssertFileExist(StrLit(ORCA_FILENAME_TESTS), true);
				PrintLine("[Built %s for Orca!]", ORCA_FILENAME_TESTS);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", ORCA_FILENAME_TESTS, statusCode);
				exit(statusCode);
			}
			
			CliArgList bundleCmd = ZEROED;
			AddArg(&bundleCmd, "bundle");
			AddArgNt(&bundleCmd, "--name [VAL]", "tests");
			AddArg(&bundleCmd, ORCA_FILENAME_TESTS);
			int bundleStatusCode = RunCliProgram(StrLit("orca"), &bundleCmd);
			if (bundleStatusCode == 0)
			{
				PrintLine("[Bundled %s into \"tests\" app!]", ORCA_FILENAME_TESTS);
			}
			else
			{
				PrintLine_E("Failed to bundle %s! Orca Status Code: %d", ORCA_FILENAME_TESTS, bundleStatusCode);
				exit(bundleStatusCode);
			}
			
			chdir("..");
		}
		
		if (BUILD_PLAYDATE_DEVICE)
		{
			PrintLine("\n[Building %s for Playdate...]", PLAYDATE_FILENAME_TESTS);
			
			CliArgList compileCmd = ZEROED;
			AddArg(&compileCmd, GCC_COMPILE);
			AddArgNt(&compileCmd, CLI_QUOTED_ARG, "../tests/tests_main.c");
			AddArgNt(&compileCmd, GCC_OUTPUT_FILE, FILENAME_TESTS_OBJ);
			AddArgList(&compileCmd, &gcc_PlaydateDeviceCommonFlags);
			AddArgList(&compileCmd, &gcc_PlaydateDeviceCompilerFlags);
			
			int compileStatusCode = RunCliProgram(armGccCompiler, &compileCmd);
			if (compileStatusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_TESTS_OBJ), true);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", PLAYDATE_FILENAME_TESTS, compileStatusCode);
				exit(compileStatusCode);
			}
			
			CliArgList linkCmd = ZEROED;
			AddArgNt(&linkCmd, CLI_QUOTED_ARG, FILENAME_TESTS_OBJ);
			AddArgNt(&linkCmd, GCC_OUTPUT_FILE, PLAYDATE_FILENAME_TESTS);
			AddArgList(&linkCmd, &gcc_PlaydateDeviceCommonFlags);
			AddArgList(&linkCmd, &gcc_PlaydateDeviceLinkerFlags);
			AddArgNt(&linkCmd, GCC_MAP_FILE, "tests.map");
			
			int linkStatusCode = RunCliProgram(armGccCompiler, &linkCmd);
			if (linkStatusCode == 0)
			{
				AssertFileExist(StrLit(PLAYDATE_FILENAME_TESTS), true);
				PrintLine("\n[Built %s for Playdate!]", PLAYDATE_FILENAME_TESTS);
			}
			else
			{
				PrintLine_E("Failed to build %s! Linker Status Code: %d", PLAYDATE_FILENAME_TESTS, linkStatusCode);
				exit(linkStatusCode);
			}
			
			mkdir("playdate_data", 0);
			CopyFileToFolder(StrLit(PLAYDATE_FILENAME_TESTS), StrLit("playdate_data"));
		}
		
		if (BUILD_PLAYDATE_SIMULATOR)
		{
			PrintLine("\n[Building %s for Playdate Simulator...]", PLAYDATESIM_FILENAME_TESTS);
			
			CliArgList compileCmd = ZEROED;
			AddArg(&compileCmd, CL_COMPILE);
			AddArgNt(&compileCmd, CLI_QUOTED_ARG, "..\\tests\\tests_main.c");
			AddArgNt(&compileCmd, CL_OBJ_FILE, FILENAME_TESTS_OBJ);
			AddArgList(&compileCmd, &cl_PlaydateSimulatorCompilerFlags);
			
			int compileStatusCode = RunCliProgram(msvcCompiler, &compileCmd);
			if (compileStatusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_TESTS_OBJ), true);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", PLAYDATESIM_FILENAME_TESTS, compileStatusCode);
				exit(compileStatusCode);
			}
			
			CliArgList linkCmd = ZEROED;
			AddArg(&linkCmd, LINK_BUILD_DLL);
			AddArgNt(&linkCmd, CLI_QUOTED_ARG, FILENAME_TESTS_OBJ);
			AddArgNt(&linkCmd, LINK_OUTPUT_FILE, PLAYDATESIM_FILENAME_TESTS);
			AddArgNt(&linkCmd, LINK_IMPORT_LIBRARY_FILE, "tests.lib"); //TODO: Do we actually need to generate this?
			AddArgNt(&linkCmd, LINK_DEBUG_INFO_FILE, "tests.pdb");
			AddArgList(&linkCmd, &link_PlaydateSimulatorLinkerFlags);
			AddArgList(&linkCmd, &link_PlaydateSimulatorLibraries);
			
			int linkStatusCode = RunCliProgram(msvcLinker, &linkCmd);
			if (linkStatusCode == 0)
			{
				AssertFileExist(StrLit(PLAYDATESIM_FILENAME_TESTS), true);
				PrintLine("\n[Built %s for Playdate Simulator!]", PLAYDATESIM_FILENAME_TESTS);
			}
			else
			{
				PrintLine_E("Failed to build %s! Linker Status Code: %d", PLAYDATESIM_FILENAME_TESTS, linkStatusCode);
				exit(linkStatusCode);
			}
			
			mkdir("playdate_data", 0);
			CopyFileToFolder(StrLit(PLAYDATESIM_FILENAME_TESTS), StrLit("playdate_data"));
		}
		
		if (BUILD_PLAYDATE_DEVICE || BUILD_PLAYDATE_SIMULATOR)
		{
			CopyFileToFolder(StrLit("..\\pdxinfo"), StrLit("playdate_data"));
			
			CliArgList packageCmd = ZEROED;
			AddArgList(&packageCmd, &pdc_CommonFlags);
			AddArgNt(&packageCmd, CLI_QUOTED_ARG, "playdate_data");
			AddArgNt(&packageCmd, CLI_QUOTED_ARG, FILENAME_TESTS_PDX);
			
			int packageStatusCode = RunCliProgram(StrLit("pdc"), &packageCmd);
			if (packageStatusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_TESTS_PDX), true); //TODO: Is this going to work on a folder?
				PrintLine("\n[Packaged %s for Playdate!]", FILENAME_TESTS_PDX);
			}
			else
			{
				PrintLine_E("Failed to package %s! Status Code: %d", FILENAME_TESTS_PDX, packageStatusCode);
				exit(packageStatusCode);
			}
		}
	}
	
	// +--------------------------------------------------------------+
	// |                        Run tests.exe                         |
	// +--------------------------------------------------------------+
	if (RUN_TESTS)
	{
		PrintLine("\n[%s]", FILENAME_TESTS);
		
		CliArgList cmd = ZEROED;
		
		int statusCode = RunCliProgram(StrLit(FILENAME_TESTS), &cmd);
		if (statusCode != 0)
		{
			PrintLine_E("%s Failed! Status Code: %d", FILENAME_TESTS, statusCode);
			exit(statusCode);
		}
	}
	
	WriteLine("\n[pig_build.exe Finished Successfully]");
	return 0;
}
