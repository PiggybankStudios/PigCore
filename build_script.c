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
#include "tools/tools_emscripten_flags.h"
#include "tools/tools_shdc_flags.h"
#include "tools/tools_pdc_flags.h"

#include "tools/tools_build_context.h"
#include "tools/tools_build_helpers.h"

BuildContext ctx;

#include "tools/tools_build_basics.c"

#define SHADER_IGNORE_LIST { ".git", "_template", "third_party", "_build" }
#define ROOT_DIR           ".."
#define NESTED_ROOT_DIR    "../.."
#define FILENAME_ORCA_SDK_PATH     "orca_sdk_path.txt"
#define FILENAME_PIGGEN_EXE        "piggen.exe"
#define FILENAME_PIGGEN            "piggen"
#define FILENAME_IMGUI_OBJ         "imgui.obj"
#define FILENAME_IMGUI_O           "imgui.o"
#define FILENAME_PHYSX_OBJ         "physx_capi.obj"
#define FILENAME_PHYSX_O           "physx_capi.o"
#define FILENAME_PIG_CORE_DLL      "pig_core.dll"
#define FILENAME_PIG_CORE_SO       "libpig_core.so"
#define FILENAME_TESTS             "tests"
#define FILENAME_TESTS_EXE         "tests.exe"
#define FILENAME_TESTS_OBJ         "tests.obj"
#define FILENAME_APP_WASM          "app.wasm"
#define FILENAME_APP_WAT           "app.wat"
#define FILENAME_INDEX_HTML        "index.html"
#define FILENAME_INDEX_WASM        "index.wasm"
#define FILENAME_INDEX_WAT         "index.wat"
#define FILENAME_MODULE_WASM       "module.wasm"
#define FILENAME_PDEX_ELF          "pdex.elf"
#define FILENAME_PDEX_DLL          "pdex.dll"
#define FILENAME_TESTS_PDX         "tests.pdx"

static inline void PrintUsage()
{
	WriteLine_E("Usage: " TOOL_EXE_NAME " [build_config_path] [is_msvc_compiler_initialized]");
}

int main(int argc, char* argv[])
{
	// WriteLine("Running Pig Build!");
	memset(&ctx, 0x00, sizeof(ctx));
	
	ctx.isMsvcInitialized = WasMsvcDevBatchRun();
	ctx.isEmsdkInitialized = WasEmsdkEnvBatchRun();
	
	// +==============================+
	// |       Extract Defines        |
	// +==============================+
	const char* buildConfigPath = ROOT_DIR "\\build_config.h";
	Str8 buildConfigContents = ZEROED;
	if (!TryReadFile(NewStr8Nt(buildConfigPath), &buildConfigContents))
	{
		PrintLine_E("Failed to open file \"%s\"", buildConfigPath);
		return 3;
	}
	
	ctx.DEBUG_BUILD              = ExtractBoolDefine(buildConfigContents, StrLit("DEBUG_BUILD"));
	ctx.BUILD_PIGGEN             = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_PIGGEN"));
	ctx.BUILD_SHADERS            = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_SHADERS"));
	ctx.RUN_PIGGEN               = ExtractBoolDefine(buildConfigContents, StrLit("RUN_PIGGEN"));
	ctx.BUILD_IMGUI_OBJ          = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_IMGUI_OBJ"));
	ctx.BUILD_PHYSX_OBJ          = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_PHYSX_OBJ"));
	ctx.BUILD_PIG_CORE_DLL       = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_PIG_CORE_DLL"));
	ctx.BUILD_TESTS              = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_TESTS"));
	ctx.RUN_TESTS                = ExtractBoolDefine(buildConfigContents, StrLit("RUN_TESTS"));
	ctx.DUMP_PREPROCESSOR        = ExtractBoolDefine(buildConfigContents, StrLit("DUMP_PREPROCESSOR"));
	ctx.CONVERT_WASM_TO_WAT      = ExtractBoolDefine(buildConfigContents, StrLit("CONVERT_WASM_TO_WAT"));
	ctx.USE_EMSCRIPTEN           = ExtractBoolDefine(buildConfigContents, StrLit("USE_EMSCRIPTEN"));
	ctx.ENABLE_AUTO_PROFILE      = ExtractBoolDefine(buildConfigContents, StrLit("ENABLE_AUTO_PROFILE"));
	ctx.RUN_FUZZER               = ExtractBoolDefine(buildConfigContents, StrLit("RUN_FUZZER"));
	ctx.BUILD_WINDOWS            = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WINDOWS"));
	ctx.BUILD_LINUX              = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_LINUX"));
	ctx.BUILD_WEB                = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WEB"));
	ctx.BUILD_ORCA               = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_ORCA"));
	ctx.BUILD_PLAYDATE_DEVICE    = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_PLAYDATE_DEVICE"));
	ctx.BUILD_PLAYDATE_SIMULATOR = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_PLAYDATE_SIMULATOR"));
	ctx.BUILD_WITH_RAYLIB        = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_RAYLIB"));
	ctx.BUILD_WITH_BOX2D         = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_BOX2D"));
	ctx.BUILD_WITH_SOKOL_GFX     = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_SOKOL_GFX"));
	ctx.BUILD_WITH_SOKOL_APP     = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_SOKOL_APP"));
	ctx.BUILD_WITH_SDL           = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_SDL"));
	ctx.BUILD_WITH_OPENVR        = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_OPENVR"));
	ctx.BUILD_WITH_CLAY          = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_CLAY"));
	ctx.BUILD_WITH_IMGUI         = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_IMGUI"));
	ctx.BUILD_WITH_PHYSX         = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_PHYSX"));
	
	free(buildConfigContents.chars);
	
	// +==============================+
	// |          Constants           |
	// +==============================+
	#if BUILDING_ON_WINDOWS
	Str8 shdcExe = StrLit(ROOT_DIR "\\third_party\\_tools\\win32\\sokol-shdc.exe");
	#elif BUILDING_ON_LINUX
	Str8 shdcExe = StrLit(ROOT_DIR "/third_party/_tools/linux/sokol-shdc");
	#elif BUILDING_ON_OSX
	Str8 shdcExe = StrLit(ROOT_DIR "/third_party/_tools/osx/sokol-shdc");
	#endif
	
	// +==============================+
	// |        cl_CommonFlags        |
	// +==============================+
	AddArg(&ctx.cl_CommonFlags, ctx.DEBUG_BUILD ? CL_STD_LIB_DYNAMIC_DBG : CL_STD_LIB_DYNAMIC);
	AddArg(&ctx.cl_CommonFlags, CL_FULL_FILE_PATHS); //we need full file paths in errors for Sublime Text to be able to parse the errors and display them in the editor
	AddArg(&ctx.cl_CommonFlags, CL_NO_LOGO); //Suppress the annoying Microsoft logo and copyright info that the compiler prints out
	AddArgNt(&ctx.cl_CommonFlags, CL_OPTIMIZATION_LEVEL, ctx.DEBUG_BUILD ? "d" : "2");
	if (!ctx.DEBUG_BUILD) { AddArgNt(&ctx.cl_CommonFlags, CL_OPTIMIZATION_LEVEL, "y"); }
	if (!ctx.DEBUG_BUILD) { AddArgNt(&ctx.cl_CommonFlags, CL_OPTIMIZATION_LEVEL, "t"); }
	AddArgNt(&ctx.cl_CommonFlags, CL_WARNING_LEVEL, "X"); //Treat all warnings as errors
	AddArgInt(&ctx.cl_CommonFlags, CL_WARNING_LEVEL, 4); //Use warning level 4, then disable various warnings we don't care about
	AddArgInt(&ctx.cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_LOGICAL_OP_ON_ADDRESS_OF_STR_CONST);
	AddArgInt(&ctx.cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_NAMELESS_STRUCT_OR_UNION);
	AddArgInt(&ctx.cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_STRUCT_WAS_PADDED);
	AddArgInt(&ctx.cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_DECLARATION_HIDES_CLASS_MEMBER);
	AddArgInt(&ctx.cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_FUNC_REMOVED);
	AddArgInt(&ctx.cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_USAGE_OF_DEPRECATED);
	AddArgInt(&ctx.cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_ASSIGNMENT_WITHIN_CONDITIONAL_EXPR);
	AddArgInt(&ctx.cl_CommonFlags, CL_ENABLE_WARNING, CL_WARNING_SWITCH_FALLTHROUGH);
	AddArgNt(&ctx.cl_CommonFlags, CL_INCLUDE_DIR, ROOT_DIR);
	if (ctx.DEBUG_BUILD)
	{
		AddArg(&ctx.cl_CommonFlags, CL_DEBUG_INFO);
		//We don't care about these warnings in debug builds, but we will solve them when we go to build in release mode because they probably indicate mistakes at that point
		AddArgInt(&ctx.cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_SWITCH_ONLY_DEFAULT);
		AddArgInt(&ctx.cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_FUNC_PARAMETER);
		AddArgInt(&ctx.cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_LCOAL_VARIABLE);
		AddArgInt(&ctx.cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_CONDITIONAL_EXPR_IS_CONSTANT);
		AddArgInt(&ctx.cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_LOCAL_VAR_INIT_BUT_NOT_REFERENCED);
		AddArgInt(&ctx.cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_UNREACHABLE_CODE_DETECTED);
	}
	if (ctx.DUMP_PREPROCESSOR)
	{
		AddArg(&ctx.cl_CommonFlags, CL_PRECOMPILE_ONLY);
		AddArg(&ctx.cl_CommonFlags, CL_PRECOMPILE_PRESERVE_COMMENTS);
	}
	
	// +==============================+
	// |        cl_LangCFlags         |
	// +==============================+
	AddArgNt(&ctx.cl_LangCFlags, CL_LANG_VERSION, "clatest"); //Use latest C language spec features
	AddArgNt(&ctx.cl_LangCFlags, CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
	
	// +==============================+
	// |       cl_LangCppFlags        |
	// +==============================+
	AddArgNt(&ctx.cl_LangCppFlags, CL_LANG_VERSION, "c++20");
	AddArgInt(&ctx.cl_LangCppFlags, CL_DISABLE_WARNING, CL_WARNING_ENUMERATION_MUST_HAVE_UNDERLYING_TYPE);
	AddArgInt(&ctx.cl_LangCppFlags, CL_DISABLE_WARNING, CL_WARNING_BITWISE_OP_BETWEEN_ENUMS);
	
	// +==============================+
	// |      clang_CommonFlags       |
	// +==============================+
	AddArg(&ctx.clang_CommonFlags, CLANG_FULL_FILE_PATHS); //Print absolute paths in diagnostics TODO: Figure out how to resolve these back to windows paths for Sublime error linking?
	AddArgNt(&ctx.clang_CommonFlags, CLANG_LANG_VERSION, "gnu2x"); //Use C20+ language spec (NOTE: We originally had -std=c2x but that didn't define MAP_ANONYMOUS and mmap was failing)
	AddArgNt(&ctx.clang_CommonFlags, CLANG_WARNING_LEVEL, "all"); //This enables all the warnings about constructions that some users consider questionable, and that are easy to avoid (or modify to prevent the warning), even in conjunction with macros
	AddArgNt(&ctx.clang_CommonFlags, CLANG_WARNING_LEVEL, "extra"); //This enables some extra warning flags that are not enabled by -Wall
	AddArgNt(&ctx.clang_CommonFlags, CLANG_ENABLE_WARNING, CLANG_WARNING_SHADOWING);
	AddArgNt(&ctx.clang_CommonFlags, CLANG_ENABLE_WARNING, CLANG_WARNING_MISSING_FALLTHROUGH_IN_SWITCH);
	AddArgNt(&ctx.clang_CommonFlags, CLANG_DISABLE_WARNING, CLANG_WARNING_SWITCH_MISSING_CASES);
	AddArgNt(&ctx.clang_CommonFlags, CLANG_DISABLE_WARNING, CLANG_WARNING_UNUSED_FUNCTION);
	if (ctx.DEBUG_BUILD)
	{
		//We don't care about these warnings in debug builds, but we will solve them when we go to build in release mode because they probably indicate mistakes at that point
		AddArgNt(&ctx.clang_CommonFlags, CLANG_DISABLE_WARNING, "unused-parameter");
		AddArgNt(&ctx.clang_CommonFlags, CLANG_DISABLE_WARNING, "unused-variable");
	}
	if (ctx.DUMP_PREPROCESSOR) { AddArg(&ctx.clang_CommonFlags, CLANG_PRECOMPILE_ONLY); }
	
	// +==============================+
	// |       clang_LinuxFlags       |
	// +==============================+
	AddArgNt(&ctx.clang_LinuxFlags, CLANG_INCLUDE_DIR, NESTED_ROOT_DIR);
	AddArg(&ctx.clang_LinuxFlags, "-mssse3"); //For MeowHash to work we need sse3 support
	AddArg(&ctx.clang_LinuxFlags, "-maes"); //For MeowHash to work we need aes support
	if (ctx.DEBUG_BUILD) { AddArgNt(&ctx.clang_LinuxFlags, CLANG_DEBUG_INFO, "dwarf-4"); }
	
	// +==============================+
	// |     cl_CommonLinkerFlags     |
	// +==============================+
	AddArgNt(&ctx.cl_CommonLinkerFlags, LINK_LIBRARY_DIR, ctx.DEBUG_BUILD ? ROOT_DIR "\\third_party\\_lib_debug" : ROOT_DIR "\\third_party\\_lib_release");
	AddArg(&ctx.cl_CommonLinkerFlags, LINK_DISABLE_INCREMENTAL);
	
	// +==============================+
	// |  clang_LinuxCommonLibraries  |
	// +==============================+
	AddArgNt(&ctx.clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "m"); //Include the math library (required for stuff like sinf, atan, etc.)
	AddArgNt(&ctx.clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "dl"); //Needed for dlopen and similar functions
	
	// +==============================+
	// |     cl_PigCoreLibraries      |
	// +==============================+
	if (ctx.BUILD_WITH_RAYLIB) { AddArgNt(&ctx.cl_PigCoreLibraries, CLI_QUOTED_ARG, "raylib.lib"); } //NOTE: raylib.lib MUST be before User32.lib and others
	AddArgNt(&ctx.cl_PigCoreLibraries, CLI_QUOTED_ARG, "Gdi32.lib"); //Needed for CreateFontA and other Windows graphics functions
	AddArgNt(&ctx.cl_PigCoreLibraries, CLI_QUOTED_ARG, "User32.lib"); //Needed for GetForegroundWindow, GetDC, etc.
	AddArgNt(&ctx.cl_PigCoreLibraries, CLI_QUOTED_ARG, "Ole32.lib"); //Needed for Combaseapi.h, CoInitializeEx, CoCreateInstance, etc.
	AddArgNt(&ctx.cl_PigCoreLibraries, CLI_QUOTED_ARG, "Shell32.lib"); //Needed for SHGetSpecialFolderPathA
	AddArgNt(&ctx.cl_PigCoreLibraries, CLI_QUOTED_ARG, "Shlwapi.lib"); //Needed for PathFileExistsA
	if (ctx.BUILD_WITH_RAYLIB)
	{
		AddArgNt(&ctx.cl_PigCoreLibraries, CLI_QUOTED_ARG, "Kernel32.lib");
		AddArgNt(&ctx.cl_PigCoreLibraries, CLI_QUOTED_ARG, "Winmm.lib");
	}
	if (ctx.BUILD_WITH_BOX2D) { AddArgNt(&ctx.cl_PigCoreLibraries, CLI_QUOTED_ARG, "box2d.lib"); }
	if (ctx.BUILD_WITH_SDL) { AddArgNt(&ctx.cl_PigCoreLibraries, CLI_QUOTED_ARG, "SDL2.lib"); }
	if (ctx.BUILD_WITH_OPENVR) { AddArgNt(&ctx.cl_PigCoreLibraries, CLI_QUOTED_ARG, "openvr_api.lib"); }
	if (ctx.BUILD_WITH_IMGUI) { AddArgNt(&ctx.cl_PigCoreLibraries, CLI_QUOTED_ARG, FILENAME_IMGUI_OBJ); }
	if (ctx.BUILD_WITH_PHYSX)
	{
		AddArgNt(&ctx.cl_PigCoreLibraries, CLI_QUOTED_ARG, "PhysX_static_64.lib");
		AddArgNt(&ctx.cl_PigCoreLibraries, CLI_QUOTED_ARG, FILENAME_PHYSX_OBJ);
	}
	
	// +==============================+
	// |    clang_PigCoreLibraries    |
	// +==============================+
	AddArgNt(&ctx.clang_PigCoreLibraries, CLANG_SYSTEM_LIBRARY, "fontconfig");
	if (ctx.BUILD_WITH_SOKOL_GFX) { AddArgNt(&ctx.clang_PigCoreLibraries, CLANG_SYSTEM_LIBRARY, "GL"); }
	
	// +==============================+
	// |      emscriptenSdkPath       |
	// +==============================+
	if (ctx.BUILD_WEB && ctx.USE_EMSCRIPTEN)
	{
		const char* sdkEnvVariable = getenv("EMSCRIPTEN_SDK_PATH");
		if (sdkEnvVariable == nullptr)
		{
			WriteLine_E("Please set the EMSCRIPTEN_SDK_PATH environment variable before trying to build for the web with USE_EMSCRIPTEN");
			exit(7);
		}
		ctx.emscriptenSdkPath = NewStr8Nt(sdkEnvVariable);
		if (ctx.emscriptenSdkPath.chars[ctx.emscriptenSdkPath.length-1] == '\\' || ctx.emscriptenSdkPath.chars[ctx.emscriptenSdkPath.length-1] == '/') { ctx.emscriptenSdkPath.length--; }
		ctx.emscriptenSdkPath = CopyStr8(ctx.emscriptenSdkPath, true);
		PrintLine("Emscripten SDK path: \"%.*s\"", ctx.emscriptenSdkPath.length, ctx.emscriptenSdkPath.chars);
		InitializeEmsdkIf(&ctx.isEmsdkInitialized);
	}
	
	// +==============================+
	// |       clang_WasmFlags        |
	// +==============================+
	AddArgNt(&ctx.clang_WasmFlags, CLANG_TARGET_ARCHITECTURE, "wasm32");
	AddArgNt(&ctx.clang_WasmFlags, CLANG_M_FLAG, "bulk-memory");
	AddArgNt(&ctx.clang_WasmFlags, CLANG_INCLUDE_DIR, NESTED_ROOT_DIR);
	if (ctx.DEBUG_BUILD) { AddArg(&ctx.clang_WasmFlags, CLANG_DEBUG_INFO_DEFAULT); }
	else { AddArgNt(&ctx.clang_WasmFlags, CLANG_OPTIMIZATION_LEVEL, "2"); }
	
	// +==============================+
	// |        clang_WebFlags        |
	// +==============================+
	if (ctx.USE_EMSCRIPTEN)
	{
		AddArgNt(&ctx.clang_WebFlags, EMSCRIPTEN_S_FLAG, "USE_SDL");
		AddArgNt(&ctx.clang_WebFlags, EMSCRIPTEN_S_FLAG, "ALLOW_MEMORY_GROWTH");
	}
	else
	{
		Str8 customStdLibDir = StrLit(NESTED_ROOT_DIR "/wasm/std/include");
		AddArgStr(&ctx.clang_WebFlags, CLANG_INCLUDE_DIR, customStdLibDir);
		AddArg(&ctx.clang_WebFlags, CLANG_NO_ENTRYPOINT);
		AddArg(&ctx.clang_WebFlags, CLANG_ALLOW_UNDEFINED);
		AddArg(&ctx.clang_WebFlags, CLANG_NO_STD_LIBRARIES);
		AddArg(&ctx.clang_WebFlags, CLANG_NO_STD_INCLUDES);
		AddArgNt(&ctx.clang_WebFlags, CLANG_EXPORT_SYMBOL, "__heap_base");
	}
	
	// +==============================+
	// |         orcaSdkPath          |
	// +==============================+
	if (ctx.BUILD_ORCA)
	{
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
		bool readSuccess = TryReadFile(StrLit(FILENAME_ORCA_SDK_PATH), &ctx.orcaSdkPath);
		assert(readSuccess == true);
		assert(ctx.orcaSdkPath.length > 0);
		FixPathSlashes(ctx.orcaSdkPath, '/');
		if (ctx.orcaSdkPath.chars[ctx.orcaSdkPath.length-1] == '/') { ctx.orcaSdkPath.length--; } //no trailing slash
		PrintLine("Orca SDK path: \"%.*s\"", ctx.orcaSdkPath.length, ctx.orcaSdkPath.chars);
	}
	
	// +==============================+
	// |       clang_OrcaFlags        |
	// +==============================+
	AddArg(&ctx.clang_OrcaFlags, CLANG_NO_ENTRYPOINT);
	AddArg(&ctx.clang_OrcaFlags, CLANG_EXPORT_DYNAMIC);
	AddArgStr(&ctx.clang_OrcaFlags, CLANG_STDLIB_FOLDER, JoinStrings2(ctx.orcaSdkPath, StrLit("/orca-libc"), false));
	AddArgStr(&ctx.clang_OrcaFlags, CLANG_INCLUDE_DIR, JoinStrings2(ctx.orcaSdkPath, StrLit("/src"), false));
	AddArgStr(&ctx.clang_OrcaFlags, CLANG_INCLUDE_DIR, JoinStrings2(ctx.orcaSdkPath, StrLit("/src/ext"), false));
	AddArgStr(&ctx.clang_OrcaFlags, CLANG_LIBRARY_DIR, JoinStrings2(ctx.orcaSdkPath, StrLit("/bin"), false));
	AddArgNt(&ctx.clang_OrcaFlags, CLANG_SYSTEM_LIBRARY, "orca_wasm");
	AddArgNt(&ctx.clang_OrcaFlags, CLANG_DEFINE, "__ORCA__"); //#define __ORCA__ so that base_compiler_check.h can set TARGET_IS_ORCA
	
	// +==============================+
	// |        playdateSdkDir        |
	// +==============================+
	if (ctx.BUILD_PLAYDATE_DEVICE || ctx.BUILD_PLAYDATE_SIMULATOR)
	{
		const char* sdkEnvVariable = getenv("PLAYDATE_SDK_PATH");
		if (sdkEnvVariable == nullptr)
		{
			WriteLine_E("Please set the PLAYDATE_SDK_PATH environment variable before trying to build for the Playdate");
			exit(7);
		}
		ctx.playdateSdkDir = NewStr8Nt(sdkEnvVariable);
		if (ctx.playdateSdkDir.chars[ctx.playdateSdkDir.length-1] == '\\' || ctx.playdateSdkDir.chars[ctx.playdateSdkDir.length-1] == '/') { ctx.playdateSdkDir.length--; }
		ctx.playdateSdkDir = CopyStr8(ctx.playdateSdkDir, true);
		PrintLine("Playdate SDK path: \"%.*s\"", ctx.playdateSdkDir.length, ctx.playdateSdkDir.chars);
		ctx.playdateSdkDir_C_API = JoinStrings2(ctx.playdateSdkDir, StrLit("\\C_API"), false);
	}
	
	// +====================================+
	// | cl_PlaydateSimulatorCompilerFlags  |
	// +====================================+
	//TODO: Just use cl_CommonFlags?
	AddArg(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_NO_LOGO);
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_WARNING_LEVEL, "3");
	AddArg(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_NO_WARNINGS_AS_ERRORS);
	AddArg(&ctx.cl_PlaydateSimulatorCompilerFlags, ctx.DEBUG_BUILD ? CL_STD_LIB_DYNAMIC_DBG : CL_STD_LIB_DYNAMIC);
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_OPTIMIZATION_LEVEL, ctx.DEBUG_BUILD ? "d" : "2");
	if (ctx.DEBUG_BUILD) { AddArg(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_DEBUG_INFO); }
	
	//TODO: Just use cl_LangCFlags?
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_LANG_VERSION, "clatest"); //Use latest C language spec features
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
	
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_INCLUDE_DIR, ROOT_DIR);
	if (ctx.BUILD_PLAYDATE_SIMULATOR) { AddArgStr(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_INCLUDE_DIR, ctx.playdateSdkDir_C_API); } //NOTE: ctx.playdateSdkDir_C_API is only filled if BUILD_PLAYDATE_SIMULATOR
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "TARGET_SIMULATOR=1");
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "TARGET_EXTENSION=1");
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "__HEAP_SIZE=8388208");
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "__STACK_SIZE=61800");
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "_WINDLL");
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "_MBCS");
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "WIN32");
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "_WINDOWS");
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "_WINDLL=1");
	AddArg(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_BUFFER_SECURITY_CHECK);
	AddArg(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_DISABLE_MINIMAL_REBUILD);
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_RUNTIME_CHECKS, "1"); //Enable fast runtime checks (Equivalent to "su")
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_CALLING_CONVENTION, "d"); //Use __cdecl calling convention
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_INLINE_EXPANSION_LEVEL, "0"); //Disable inline expansions
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_INTERNAL_COMPILER_ERROR_BEHAVIOR, "prompt"); //TODO: Do we need this?
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_LANG_CONFORMANCE_OPTION, "forScope"); //Enforce Standard C++ for scoping rules (on by default)
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_LANG_CONFORMANCE_OPTION, "inline"); //Remove unreferenced functions or data if they're COMDAT or have internal linkage only (off by default)
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_LANG_CONFORMANCE_OPTION, "wchar_t"); //wchar_t is a native type, not a typedef (on by default)
	AddArgNt(&ctx.cl_PlaydateSimulatorCompilerFlags, CL_FLOATING_POINT_MODEL, "precise"); //"precise" floating-point model; results are predictable
	
	// +====================================+
	// | link_PlaydateSimulatorLinkerFlags  |
	// +====================================+
	AddArg(&ctx.link_PlaydateSimulatorLinkerFlags, LINK_NO_LOGO);
	AddArgNt(&ctx.link_PlaydateSimulatorLinkerFlags, LINK_TARGET_ARCHITECTURE, "X64");
	AddArg(&ctx.link_PlaydateSimulatorLinkerFlags, LINK_DATA_EXEC_COMPAT);
	AddArg(&ctx.link_PlaydateSimulatorLinkerFlags, LINK_ENABLE_ASLR);
	AddArg(&ctx.link_PlaydateSimulatorLinkerFlags, LINK_CONSOLE_APPLICATION);
	AddArgInt(&ctx.link_PlaydateSimulatorLinkerFlags, LINK_TYPELIB_RESOURCE_ID, 1);
	AddArg(&ctx.link_PlaydateSimulatorLinkerFlags, LINK_ENABLE_INCREMENTAL);
	AddArgNt(&ctx.link_PlaydateSimulatorLinkerFlags, LINK_INCREMENTAL_FILE_NAME, "tests.ilk"); //TODO: This should really move down below inside the tests.exe block
	AddArg(&ctx.link_PlaydateSimulatorLinkerFlags, LINK_CREATE_ASSEMBLY_MANIFEST);
	AddArgNt(&ctx.link_PlaydateSimulatorLinkerFlags, LINK_ASSEMBLY_MANIFEST_FILE, "tests.intermediate.manifest"); //TODO: This should really move down below inside the tests.exe block
	AddArgNt(&ctx.link_PlaydateSimulatorLinkerFlags, LINK_LINK_TIME_CODEGEN_FILE, "tests.iobj"); //TODO: This should really move down below inside the tests.exe block
	AddArgNt(&ctx.link_PlaydateSimulatorLinkerFlags, LINK_EMBED_UAC_INFO_EX, "level='asInvoker' uiAccess='false'");
	if (ctx.DEBUG_BUILD) { AddArg(&ctx.link_PlaydateSimulatorLinkerFlags, LINK_DEBUG_INFO); }
	
	// +==================================+
	// | link_PlaydateSimulatorLibraries  |
	// +==================================+
	AddArgNt(&ctx.link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "kernel32.lib");
	AddArgNt(&ctx.link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "user32.lib");
	AddArgNt(&ctx.link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "gdi32.lib");
	AddArgNt(&ctx.link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "winspool.lib");
	AddArgNt(&ctx.link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "shell32.lib");
	AddArgNt(&ctx.link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "ole32.lib");
	AddArgNt(&ctx.link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "oleaut32.lib");
	AddArgNt(&ctx.link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "uuid.lib");
	AddArgNt(&ctx.link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "comdlg32.lib");
	AddArgNt(&ctx.link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "advapi32.lib");
	
	// +===============================+
	// | gcc_PlaydateDeviceCommonFlags |
	// +===============================+
	AddArgNt(&ctx.gcc_PlaydateDeviceCommonFlags, GCC_INCLUDE_DIR, ROOT_DIR);
	if (ctx.BUILD_PLAYDATE_DEVICE) { AddArgStr(&ctx.gcc_PlaydateDeviceCommonFlags, GCC_INCLUDE_DIR, ctx.playdateSdkDir_C_API); } //NOTE: ctx.playdateSdkDir_C_API is only filled if BUILD_PLAYDATE_DEVICE
	AddArgNt(&ctx.gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "TARGET_PLAYDATE=1");
	AddArgNt(&ctx.gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "TARGET_EXTENSION=1");
	AddArgNt(&ctx.gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "__HEAP_SIZE=8388208");
	AddArgNt(&ctx.gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "__STACK_SIZE=61800");
	AddArgNt(&ctx.gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "__FPU_USED=1");
	AddArgNt(&ctx.gcc_PlaydateDeviceCommonFlags, GCC_USE_SPEC_FILE, "nano.specs"); //Required for things like _read, _write, _exit, etc. to not be pulled in as requirements from standard library
	AddArgNt(&ctx.gcc_PlaydateDeviceCommonFlags, GCC_USE_SPEC_FILE, "nosys.specs"); //TODO: Is this helping?
	AddArg(&ctx.gcc_PlaydateDeviceCommonFlags, GCC_TARGET_THUMB);
	AddArgNt(&ctx.gcc_PlaydateDeviceCommonFlags, GCC_TARGET_CPU, "cortex-m7");
	AddArgNt(&ctx.gcc_PlaydateDeviceCommonFlags, GCC_FLOAT_ABI_MODE, "hard"); //Use hardware for floating-point operations
	AddArgNt(&ctx.gcc_PlaydateDeviceCommonFlags, GCC_TARGET_FPU, "fpv5-sp-d16");
	
	// +==================================+
	// | gcc_PlaydateDeviceCompilerFlags  |
	// +==================================+
	AddArgNt(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_DEBUG_INFO_EX, "3");
	AddArgNt(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_DEBUG_INFO_EX, "dwarf-2");
	AddArg(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_STD_LIB_DYNAMIC);
	AddArgNt(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_DEPENDENCY_FILE, "tests.d"); //TODO: This should really move down below inside the tests.exe block
	AddArgInt(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_ALIGN_FUNCS_TO, 16);
	AddArg(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_SEP_DATA_SECTIONS);
	AddArg(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_SEP_FUNC_SECTIONS);
	AddArg(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_EXCEPTIONS);
	AddArg(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_OMIT_FRAME_PNTR);
	AddArg(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_GLOBAL_VAR_NO_COMMON);
	AddArg(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_VERBOSE_ASSEMBLY); //TODO: Should this only be on when DEBUG_BUILD?
	AddArg(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_ONLY_RELOC_WORD_SIZE);
	AddArgNt(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_WARNING_LEVEL, "all");
	// AddArgNt(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_ENABLE_WARNING, "double-promotion");
	AddArgNt(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "unknown-pragmas");
	AddArgNt(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "comment");
	AddArgNt(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "switch");
	AddArgNt(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "nonnull");
	AddArgNt(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "unused");
	AddArgNt(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "missing-braces");
	AddArgNt(&ctx.gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "char-subscripts");
	
	// +===============================+
	// | gcc_PlaydateDeviceLinkerFlags |
	// +===============================+
	AddArg(&ctx.gcc_PlaydateDeviceLinkerFlags, GCC_NO_STD_STARTUP);
	AddArgNt(&ctx.gcc_PlaydateDeviceLinkerFlags, GCC_ENTRYPOINT_NAME, "eventHandler");
	AddArg(&ctx.gcc_PlaydateDeviceLinkerFlags, GCC_DISABLE_RWX_WARNING);
	AddArg(&ctx.gcc_PlaydateDeviceLinkerFlags, GCC_CREF);
	AddArg(&ctx.gcc_PlaydateDeviceLinkerFlags, GCC_GC_SECTIONS);
	AddArg(&ctx.gcc_PlaydateDeviceLinkerFlags, GCC_DISABLE_MISMATCH_WARNING);
	AddArg(&ctx.gcc_PlaydateDeviceLinkerFlags, GCC_EMIT_RELOCATIONS);
	Str8 playdateLinkerScriptPath = JoinStrings2(ctx.playdateSdkDir, StrLit("\\C_API\\buildsupport\\link_map.ld"), false);
	FixPathSlashes(playdateLinkerScriptPath, '\\');
	AddArgStr(&ctx.gcc_PlaydateDeviceLinkerFlags, GCC_LINKER_SCRIPT, playdateLinkerScriptPath);
	
	// +==============================+
	// |       pdc_CommonFlags        |
	// +==============================+
	AddArg(&ctx.pdc_CommonFlags, PDC_QUIET); //Quiet mode, suppress non-error output
	if (ctx.BUILD_PLAYDATE_DEVICE || ctx.BUILD_PLAYDATE_SIMULATOR) { AddArgStr(&ctx.pdc_CommonFlags, PDC_SDK_PATH, ctx.playdateSdkDir); } //NOTE: ctx.playdateSdkDir is only filled if BUILD_PLAYDATE_DEVICE || BUILD_PLAYDATE_SIMULATOR
	
	// +--------------------------------------------------------------+
	// |                       Build piggen.exe                       |
	// +--------------------------------------------------------------+
	if (ctx.RUN_PIGGEN && !ctx.BUILD_PIGGEN && !DoesFileExist(StrLit(FILENAME_PIGGEN_EXE))) { PrintLine("Building %s because it's missing", FILENAME_PIGGEN_EXE); ctx.BUILD_PIGGEN = true; }
	if (ctx.BUILD_PIGGEN)
	{
		// +==============================+
		// |      cl_PiggenLibraries      |
		// +==============================+
		CliArgList cl_PiggenLibraries = ZEROED;
		AddArg(&cl_PiggenLibraries, "Shlwapi.lib"); //Needed for PathFileExistsA
		
		if (ctx.BUILD_WINDOWS)
		{
			InitializeMsvcIf(&ctx.isMsvcInitialized);
			PrintLine("\n[Building %s for Windows...]", FILENAME_PIGGEN_EXE);
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, ROOT_DIR "\\piggen\\piggen_main.c");
			AddArgNt(&cmd, CL_BINARY_FILE, FILENAME_PIGGEN_EXE);
			AddArgList(&cmd, &ctx.cl_CommonFlags);
			AddArgList(&cmd, &ctx.cl_LangCFlags);
			AddArg(&cmd, CL_LINK);
			AddArgList(&cmd, &cl_PiggenLibraries);
			AddArgList(&cmd, &ctx.cl_CommonLinkerFlags);
			
			int statusCode = RunCliProgram(StrLit(EXE_MSVC_CL), &cmd);
			if (statusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_PIGGEN_EXE), true);
				PrintLine("[Built %s for Windows!]", FILENAME_PIGGEN_EXE);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", FILENAME_PIGGEN_EXE, statusCode);
				exit(statusCode);
			}
		}
		if (ctx.BUILD_LINUX)
		{
			PrintLine("\n[Building %s for Linux...]", FILENAME_PIGGEN);
			
			mkdir("linux", 0);
			chdir("linux");
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, NESTED_ROOT_DIR "/piggen/piggen_main.c");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, FILENAME_PIGGEN);
			AddArgList(&cmd, &ctx.clang_CommonFlags);
			AddArgList(&cmd, &ctx.clang_LinuxFlags);
			AddArgList(&cmd, &ctx.clang_LinuxCommonLibraries);
			
			int statusCode = RunCliProgram(StrLit(EXE_WSL_CLANG), &cmd);
			if (statusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_PIGGEN), true);
				PrintLine("[Built %s for Linux!]", FILENAME_PIGGEN);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", FILENAME_PIGGEN, statusCode);
				exit(statusCode);
			}
			
			chdir("..");
		}
	}
	
	// +--------------------------------------------------------------+
	// |                        Run piggen.exe                        |
	// +--------------------------------------------------------------+
	#define FOLDERNAME_GENERATED_CODE "gen"
	if (ctx.RUN_PIGGEN)
	{
		PrintLine("\n[%s]", FILENAME_PIGGEN_EXE);
		
		#define PIGGEN_OUTPUT_FOLDER "-o=\"[VAL]\""
		#define PIGGEN_EXCLUDE_FOLDER "-e=\"[VAL]\""
		
		CliArgList cmd = ZEROED;
		AddArg(&cmd, ROOT_DIR);
		AddArgNt(&cmd, PIGGEN_OUTPUT_FOLDER, FOLDERNAME_GENERATED_CODE "/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, ROOT_DIR "/base/base_defines_check.h");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, ROOT_DIR "/piggen/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, ROOT_DIR "/tools/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, ROOT_DIR "/wasm/std/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, ROOT_DIR "/.git/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, ROOT_DIR "/_scripts/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, ROOT_DIR "/_media/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, ROOT_DIR "/_template/");
		
		int statusCode = RunCliProgram(StrLit(FILENAME_PIGGEN_EXE), &cmd);
		if (statusCode != 0)
		{
			PrintLine_E("%s Failed! Status Code: %d", FILENAME_PIGGEN_EXE, statusCode);
			exit(statusCode);
		}
	}
	
	// +--------------------------------------------------------------+
	// |                        Build Shaders                         |
	// +--------------------------------------------------------------+
	FindShadersContext findContext = ZEROED;
	CliArgList cl_ShaderObjects = ZEROED;
	CliArgList clang_ShaderObjects = ZEROED;
	if (ctx.BUILD_SHADERS || ctx.BUILD_WITH_SOKOL_GFX)
	{
		const char* ignoreList[] = SHADER_IGNORE_LIST;
		findContext.ignoreListLength = ArrayCount(ignoreList);
		findContext.ignoreList = (Str8*)malloc(sizeof(Str8) * findContext.ignoreListLength);
		for (uxx iIndex = 0; iIndex < findContext.ignoreListLength; iIndex++)
		{
			findContext.ignoreList[iIndex] = NewStr8Nt(ignoreList[iIndex]);
		}
		
		RecursiveDirWalk(StrLit(".."), FindShaderFilesCallback, &findContext);
		
		if (ctx.BUILD_WINDOWS)
		{
			for (uxx sIndex = 0; sIndex < findContext.objPaths.length; sIndex++)
			{
				Str8 objPath = findContext.objPaths.strings[sIndex];
				AddArgStr(&cl_ShaderObjects, CLI_QUOTED_ARG, objPath);
				if (!DoesFileExist(objPath) && !ctx.BUILD_SHADERS) { PrintLine("Building shaders because \"%.*s\" is missing!", objPath.length, objPath.chars); ctx.BUILD_SHADERS = true; }
			}
		}
		if (ctx.BUILD_LINUX)
		{
			for (uxx sIndex = 0; sIndex < findContext.oPaths.length; sIndex++)
			{
				Str8 oPath = findContext.oPaths.strings[sIndex];
				AddArgStr(&clang_ShaderObjects, CLI_QUOTED_ARG, oPath);
				Str8 oPathWithFolder = JoinStrings2(StrLit("linux/"), oPath, false);
				if (!DoesFileExist(oPathWithFolder) && !ctx.BUILD_SHADERS) { PrintLine("Building shaders because \"%.*s\" is missing!", oPathWithFolder.length, oPathWithFolder.chars); ctx.BUILD_SHADERS = true; }
			}
		}
		
		if (!ctx.BUILD_SHADERS)
		{
			FreeStrArray(&findContext.shaderPaths);
			FreeStrArray(&findContext.headerPaths);
			FreeStrArray(&findContext.sourcePaths);
			FreeStrArray(&findContext.objPaths);
			FreeStrArray(&findContext.oPaths);
		}
	}
	
	if (ctx.BUILD_SHADERS)
	{
		if (ctx.BUILD_WINDOWS) { InitializeMsvcIf(&ctx.isMsvcInitialized); }
		
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
			
			if (ctx.BUILD_WINDOWS)
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
				AddArgList(&cmd, &ctx.cl_CommonFlags);
				AddArgList(&cmd, &ctx.cl_LangCFlags);
				
				
				int statusCode = RunCliProgram(StrLit(EXE_MSVC_CL), &cmd);
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
			if (ctx.BUILD_LINUX)
			{
				mkdir("linux", 0);
				chdir("linux");
				
				Str8 oPath = findContext.oPaths.strings[sIndex];
				Str8 fixedSourcePath = JoinStrings2(StrLit(ROOT_DIR "/"), sourcePath, false);
				FixPathSlashes(fixedSourcePath, '/');
				Str8 fixedHeaderDirectory = JoinStrings2(StrLit(ROOT_DIR "/"), headerDirectory, false);
				FixPathSlashes(fixedHeaderDirectory, '/');
				
				CliArgList cmd = ZEROED;
				AddArg(&cmd, CLANG_COMPILE);
				AddArgStr(&cmd, CLI_QUOTED_ARG, fixedSourcePath);
				AddArgStr(&cmd, CLANG_OUTPUT_FILE, oPath);
				AddArgStr(&cmd, CLANG_INCLUDE_DIR, fixedHeaderDirectory);
				AddArgList(&cmd, &ctx.clang_CommonFlags);
				AddArgList(&cmd, &ctx.clang_LinuxFlags);
				
				int statusCode = RunCliProgram(StrLit(EXE_WSL_CLANG), &cmd);
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
	if (ctx.BUILD_WITH_IMGUI && !ctx.BUILD_IMGUI_OBJ && ctx.BUILD_WINDOWS && !DoesFileExist(StrLit(FILENAME_IMGUI_OBJ))) { PrintLine("Building %s because it's missing", FILENAME_IMGUI_OBJ); ctx.BUILD_IMGUI_OBJ = true; }
	if (ctx.BUILD_WITH_IMGUI && !ctx.BUILD_IMGUI_OBJ && ctx.BUILD_LINUX && !DoesFileExist(StrLit(FILENAME_IMGUI_O))) { PrintLine("Building %s because it's missing", FILENAME_IMGUI_O); ctx.BUILD_IMGUI_OBJ = true; }
	if (ctx.BUILD_IMGUI_OBJ)
	{
		if (ctx.BUILD_WINDOWS)
		{
			InitializeMsvcIf(&ctx.isMsvcInitialized);
			PrintLine("[Building %s for Windows...]", FILENAME_IMGUI_OBJ);
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, CL_COMPILE);
			AddArgNt(&cmd, CLI_QUOTED_ARG, ROOT_DIR "\\ui\\ui_imgui_main.cpp");
			AddArgNt(&cmd, CL_INCLUDE_DIR, ROOT_DIR "\\third_party\\imgui");
			AddArgNt(&cmd, CL_OBJ_FILE, FILENAME_IMGUI_OBJ);
			AddArgList(&cmd, &ctx.cl_CommonFlags);
			AddArgList(&cmd, &ctx.cl_LangCppFlags);
			AddArg(&cmd, CL_LINK);
			AddArgList(&cmd, &ctx.cl_CommonLinkerFlags);
			
			int statusCode = RunCliProgram(StrLit(EXE_MSVC_CL), &cmd);
			if (statusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_IMGUI_OBJ), true);
				PrintLine("[Built %s for Windows!]", FILENAME_IMGUI_OBJ);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", FILENAME_IMGUI_OBJ, statusCode);
				exit(statusCode);
			}
		}
		if (ctx.BUILD_LINUX)
		{
			//TODO: Implement Linux version!
		}
	}
	
	// +--------------------------------------------------------------+
	// |                     Build physx_capi.obj                     |
	// +--------------------------------------------------------------+
	if (ctx.BUILD_WITH_PHYSX && !ctx.BUILD_PHYSX_OBJ && ctx.BUILD_WINDOWS && !DoesFileExist(StrLit(FILENAME_PHYSX_OBJ))) { PrintLine("Building %s because it's missing", FILENAME_PHYSX_OBJ); ctx.BUILD_PHYSX_OBJ = true; }
	if (ctx.BUILD_WITH_PHYSX && !ctx.BUILD_PHYSX_OBJ && ctx.BUILD_LINUX && !DoesFileExist(StrLit(FILENAME_PHYSX_O))) { PrintLine("Building %s because it's missing", FILENAME_PHYSX_O); ctx.BUILD_PHYSX_OBJ = true; }
	if (ctx.BUILD_PHYSX_OBJ)
	{
		if (ctx.BUILD_WINDOWS)
		{
			InitializeMsvcIf(&ctx.isMsvcInitialized);
			PrintLine("[Building %s for Windows...]", FILENAME_PHYSX_OBJ);
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, CL_COMPILE);
			AddArgNt(&cmd, CLI_QUOTED_ARG, ROOT_DIR "\\phys\\phys_physx_capi_main.cpp");
			AddArgNt(&cmd, CL_INCLUDE_DIR, ROOT_DIR "\\third_party\\physx");
			AddArgNt(&cmd, CL_OBJ_FILE, FILENAME_PHYSX_OBJ);
			AddArgList(&cmd, &ctx.cl_CommonFlags);
			AddArgList(&cmd, &ctx.cl_LangCppFlags);
			AddArg(&cmd, CL_LINK);
			AddArgList(&cmd, &ctx.cl_CommonLinkerFlags);
			
			int statusCode = RunCliProgram(StrLit(EXE_MSVC_CL), &cmd);
			if (statusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_PHYSX_OBJ), true);
				PrintLine("[Built %s for Windows!]", FILENAME_PHYSX_OBJ);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", FILENAME_PHYSX_OBJ, statusCode);
				exit(statusCode);
			}
		}
		if (ctx.BUILD_LINUX)
		{
			//TODO: Implement Linux version!
		}
	}
	
	// +--------------------------------------------------------------+
	// |                      Build pig_core.dll                      |
	// +--------------------------------------------------------------+
	if (ctx.BUILD_PIG_CORE_DLL)
	{
		if (ctx.BUILD_WINDOWS)
		{
			InitializeMsvcIf(&ctx.isMsvcInitialized);
			PrintLine("\n[Building %s for Windows...]", FILENAME_PIG_CORE_DLL);
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, ROOT_DIR "\\dll\\dll_main.c");
			AddArgNt(&cmd, CL_BINARY_FILE, FILENAME_PIG_CORE_DLL);
			AddArgList(&cmd, &ctx.cl_CommonFlags);
			AddArgList(&cmd, &ctx.cl_LangCFlags);
			AddArg(&cmd, CL_LINK);
			AddArg(&cmd, LINK_BUILD_DLL);
			AddArgList(&cmd, &ctx.cl_CommonLinkerFlags);
			AddArgList(&cmd, &ctx.cl_PigCoreLibraries);
			
			int statusCode = RunCliProgram(StrLit(EXE_MSVC_CL), &cmd);
			if (statusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_PIG_CORE_DLL), true);
				PrintLine("[Built %s for Windows!]", FILENAME_PIG_CORE_DLL);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", FILENAME_PIG_CORE_DLL, statusCode);
				exit(statusCode);
			}
		}
		if (ctx.BUILD_LINUX)
		{
			PrintLine("\n[Building %s for Linux...]", FILENAME_PIG_CORE_SO);
			
			mkdir("linux", 0);
			chdir("linux");
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, NESTED_ROOT_DIR "/dll/dll_main.c");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, FILENAME_PIG_CORE_SO);
			AddArg(&cmd, CLANG_BUILD_SHARED_LIB);
			AddArg(&cmd, CLANG_fPIC);
			AddArgList(&cmd, &ctx.clang_CommonFlags);
			AddArgList(&cmd, &ctx.clang_LinuxFlags);
			AddArgList(&cmd, &ctx.clang_LinuxCommonLibraries);
			AddArgList(&cmd, &ctx.clang_PigCoreLibraries);
			
			int statusCode = RunCliProgram(StrLit(EXE_WSL_CLANG), &cmd);
			if (statusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_PIG_CORE_SO), true);
				PrintLine("[Built %s for Linux!]", FILENAME_PIG_CORE_SO);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", FILENAME_PIG_CORE_SO, statusCode);
				exit(statusCode);
			}
			
			chdir("..");
		}
	}
	
	// +--------------------------------------------------------------+
	// |                       Build tests.exe                        |
	// +--------------------------------------------------------------+
	if (ctx.RUN_TESTS && !ctx.BUILD_TESTS && !DoesFileExist(StrLit(FILENAME_TESTS_EXE))) { PrintLine("Building %s because it's missing", FILENAME_TESTS_EXE); ctx.BUILD_TESTS = true; ctx.BUILD_WINDOWS = true; }
	if (ctx.BUILD_TESTS)
	{
		if (ctx.BUILD_WINDOWS)
		{
			InitializeMsvcIf(&ctx.isMsvcInitialized);
			PrintLine("\n[Building %s for Windows...]", FILENAME_TESTS_EXE);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, ROOT_DIR "\\tests\\tests_main.c");
			AddArgNt(&cmd, CL_BINARY_FILE, FILENAME_TESTS_EXE);
			AddArgList(&cmd, &ctx.cl_CommonFlags);
			AddArgList(&cmd, &ctx.cl_LangCFlags);
			AddArg(&cmd, CL_LINK);
			AddArgList(&cmd, &ctx.cl_CommonLinkerFlags);
			AddArgList(&cmd, &ctx.cl_PigCoreLibraries);
			if (ctx.BUILD_WITH_SOKOL_GFX) { AddArgList(&cmd, &cl_ShaderObjects); }
			
			int statusCode = RunCliProgram(StrLit(EXE_MSVC_CL), &cmd);
			if (statusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_TESTS_EXE), true);
				PrintLine("[Built %s for Windows!]", FILENAME_TESTS_EXE);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", FILENAME_TESTS_EXE, statusCode);
				exit(statusCode);
			}
		}
		
		if (ctx.BUILD_LINUX)
		{
			PrintLine("\n[Building %s for Linux...]", FILENAME_TESTS);
			
			mkdir("linux", 0);
			chdir("linux");
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, NESTED_ROOT_DIR "/tests/tests_main.c");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, FILENAME_TESTS);
			AddArgList(&cmd, &ctx.clang_CommonFlags);
			AddArgList(&cmd, &ctx.clang_LinuxFlags);
			AddArgList(&cmd, &ctx.clang_LinuxCommonLibraries);
			AddArgList(&cmd, &ctx.clang_PigCoreLibraries);
			if (ctx.BUILD_WITH_SOKOL_GFX) { AddArgList(&cmd, &clang_ShaderObjects); }
			
			int statusCode = RunCliProgram(StrLit(EXE_WSL_CLANG), &cmd);
			if (statusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_TESTS), true);
				PrintLine("[Built %s for Linux!]", FILENAME_TESTS);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", FILENAME_TESTS, statusCode);
				exit(statusCode);
			}
			
			chdir("..");
		}
		
		if (ctx.BUILD_WEB)
		{
			PrintLine("\n[Building %s for Web...]", FILENAME_APP_WASM);
			
			mkdir("web", 0);
			chdir("web");
			
			// TODO: del *.wasm > NUL 2> NUL
			// TODO: del *.wat > NUL 2> NUL
			// TODO: del *.css > NUL 2> NUL
			// TODO: del *.html > NUL 2> NUL
			// TODO: del *.js > NUL 2> NUL
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, NESTED_ROOT_DIR "/tests/tests_main.c");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, ctx.USE_EMSCRIPTEN ? FILENAME_INDEX_HTML : FILENAME_APP_WASM);
			AddArgList(&cmd, &ctx.clang_CommonFlags);
			AddArgList(&cmd, &ctx.clang_WasmFlags);
			AddArgList(&cmd, &ctx.clang_WebFlags);
			
			int statusCode = RunCliProgram(ctx.USE_EMSCRIPTEN ? StrLit(EXE_EMSCRIPTEN_COMPILER) : StrLit(EXE_CLANG), &cmd);
			if (statusCode == 0)
			{
				if (ctx.USE_EMSCRIPTEN)
				{
					AssertFileExist(StrLit(FILENAME_INDEX_HTML), true);
					AssertFileExist(StrLit(FILENAME_INDEX_WASM), true);
					AssertFileExist(StrLit("index.js"), true);
				}
				else
				{
					AssertFileExist(NewStr8Nt(FILENAME_APP_WASM), true);
				}
				PrintLine("[Built %s for Web!]", FILENAME_APP_WASM);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", FILENAME_APP_WASM, statusCode);
				exit(statusCode);
			}
			
			if (ctx.CONVERT_WASM_TO_WAT)
			{
				CliArgList convertCmd = ZEROED;
				AddArgNt(&convertCmd, CLI_QUOTED_ARG, ctx.USE_EMSCRIPTEN ? FILENAME_INDEX_WASM : FILENAME_APP_WASM);
				AddArgNt(&convertCmd, CLI_PIPE_OUTPUT_TO_FILE, ctx.USE_EMSCRIPTEN ? FILENAME_INDEX_WAT : FILENAME_APP_WAT);
				
				int convertStatusCode = RunCliProgram(StrLit("wasm2wat"), &convertCmd);
				if (convertStatusCode == 0)
				{
					AssertFileExist(ctx.USE_EMSCRIPTEN ? StrLit(FILENAME_INDEX_WAT) : StrLit(FILENAME_APP_WAT), true);
				}
				else
				{
					PrintLine_E("Failed to convert .wasm to .wat! Status Code: %d", convertStatusCode);
					exit(convertStatusCode);
				}
			}
			
			if (!ctx.USE_EMSCRIPTEN)
			{
				StrArray javascriptFiles = ZEROED;
				AddStr(&javascriptFiles, StrLit(NESTED_ROOT_DIR "/wasm/wasm_globals.js"));
				AddStr(&javascriptFiles, StrLit(NESTED_ROOT_DIR "/wasm/std/include/internal/wasm_std_js_api.js"));
				AddStr(&javascriptFiles, StrLit(NESTED_ROOT_DIR "/wasm/wasm_app_js_api.js"));
				AddStr(&javascriptFiles, StrLit(NESTED_ROOT_DIR "/wasm/wasm_main.js"));
				ConcatAllFilesIntoSingleFile(&javascriptFiles, StrLit("combined.js"));
				
				CopyFileToPath(StrLit(NESTED_ROOT_DIR "\\wasm\\wasm_app_style.css"), StrLit("main.css"));
				CopyFileToPath(StrLit(NESTED_ROOT_DIR "\\wasm\\wasm_app_index.html"), StrLit("index.html"));
			}
			
			chdir("..");
		}
		
		if (ctx.BUILD_ORCA)
		{
			PrintLine("\n[Building %s for Orca...]", FILENAME_MODULE_WASM);
			
			mkdir("orca", 0);
			chdir("orca");
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, FILENAME_MODULE_WASM);
			AddArgNt(&cmd, CLI_QUOTED_ARG, NESTED_ROOT_DIR "/tests/tests_main.c");
			AddArgList(&cmd, &ctx.clang_CommonFlags);
			AddArgList(&cmd, &ctx.clang_WasmFlags);
			AddArgList(&cmd, &ctx.clang_OrcaFlags);
			
			int statusCode = RunCliProgram(StrLit(EXE_CLANG), &cmd);
			if (statusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_MODULE_WASM), true);
				PrintLine("[Built %s for Orca!]", FILENAME_MODULE_WASM);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", FILENAME_MODULE_WASM, statusCode);
				exit(statusCode);
			}
			
			CliArgList bundleCmd = ZEROED;
			AddArg(&bundleCmd, "bundle");
			AddArgNt(&bundleCmd, "--name [VAL]", "tests");
			AddArg(&bundleCmd, FILENAME_MODULE_WASM);
			int bundleStatusCode = RunCliProgram(StrLit("orca"), &bundleCmd);
			if (bundleStatusCode == 0)
			{
				PrintLine("[Bundled %s into \"tests\" app!]", FILENAME_MODULE_WASM);
			}
			else
			{
				PrintLine_E("Failed to bundle %s! Orca Status Code: %d", FILENAME_MODULE_WASM, bundleStatusCode);
				exit(bundleStatusCode);
			}
			
			chdir("..");
		}
		
		if (ctx.BUILD_PLAYDATE_DEVICE)
		{
			PrintLine("\n[Building %s for Playdate...]", FILENAME_PDEX_ELF);
			
			CliArgList compileCmd = ZEROED;
			AddArg(&compileCmd, GCC_COMPILE);
			AddArgNt(&compileCmd, CLI_QUOTED_ARG, ROOT_DIR "/tests/tests_main.c");
			AddArgNt(&compileCmd, GCC_OUTPUT_FILE, FILENAME_TESTS_OBJ);
			AddArgList(&compileCmd, &ctx.gcc_PlaydateDeviceCommonFlags);
			AddArgList(&compileCmd, &ctx.gcc_PlaydateDeviceCompilerFlags);
			
			int compileStatusCode = RunCliProgram(StrLit(EXE_ARM_GCC), &compileCmd);
			if (compileStatusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_TESTS_OBJ), true);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", FILENAME_PDEX_ELF, compileStatusCode);
				exit(compileStatusCode);
			}
			
			CliArgList linkCmd = ZEROED;
			AddArgNt(&linkCmd, CLI_QUOTED_ARG, FILENAME_TESTS_OBJ);
			AddArgNt(&linkCmd, GCC_OUTPUT_FILE, FILENAME_PDEX_ELF);
			AddArgList(&linkCmd, &ctx.gcc_PlaydateDeviceCommonFlags);
			AddArgList(&linkCmd, &ctx.gcc_PlaydateDeviceLinkerFlags);
			AddArgNt(&linkCmd, GCC_MAP_FILE, "tests.map");
			
			int linkStatusCode = RunCliProgram(StrLit(EXE_ARM_GCC), &linkCmd);
			if (linkStatusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_PDEX_ELF), true);
				PrintLine("\n[Built %s for Playdate!]", FILENAME_PDEX_ELF);
			}
			else
			{
				PrintLine_E("Failed to build %s! Linker Status Code: %d", FILENAME_PDEX_ELF, linkStatusCode);
				exit(linkStatusCode);
			}
			
			mkdir("playdate_data", 0);
			CopyFileToFolder(StrLit(FILENAME_PDEX_ELF), StrLit("playdate_data"));
		}
		
		if (ctx.BUILD_PLAYDATE_SIMULATOR)
		{
			PrintLine("\n[Building %s for Playdate Simulator...]", FILENAME_PDEX_DLL);
			
			CliArgList compileCmd = ZEROED;
			AddArg(&compileCmd, CL_COMPILE);
			AddArgNt(&compileCmd, CLI_QUOTED_ARG, ROOT_DIR "\\tests\\tests_main.c");
			AddArgNt(&compileCmd, CL_OBJ_FILE, FILENAME_TESTS_OBJ);
			AddArgList(&compileCmd, &ctx.cl_PlaydateSimulatorCompilerFlags);
			
			int compileStatusCode = RunCliProgram(StrLit(EXE_MSVC_CL), &compileCmd);
			if (compileStatusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_TESTS_OBJ), true);
			}
			else
			{
				PrintLine_E("Failed to build %s! Compiler Status Code: %d", FILENAME_PDEX_DLL, compileStatusCode);
				exit(compileStatusCode);
			}
			
			CliArgList linkCmd = ZEROED;
			AddArg(&linkCmd, LINK_BUILD_DLL);
			AddArgNt(&linkCmd, CLI_QUOTED_ARG, FILENAME_TESTS_OBJ);
			AddArgNt(&linkCmd, LINK_OUTPUT_FILE, FILENAME_PDEX_DLL);
			AddArgNt(&linkCmd, LINK_IMPORT_LIBRARY_FILE, "tests.lib"); //TODO: Do we actually need to generate this?
			AddArgNt(&linkCmd, LINK_DEBUG_INFO_FILE, "tests.pdb");
			AddArgList(&linkCmd, &ctx.link_PlaydateSimulatorLinkerFlags);
			AddArgList(&linkCmd, &ctx.link_PlaydateSimulatorLibraries);
			
			int linkStatusCode = RunCliProgram(StrLit(EXE_MSVC_LINK), &linkCmd);
			if (linkStatusCode == 0)
			{
				AssertFileExist(StrLit(FILENAME_PDEX_DLL), true);
				PrintLine("\n[Built %s for Playdate Simulator!]", FILENAME_PDEX_DLL);
			}
			else
			{
				PrintLine_E("Failed to build %s! Linker Status Code: %d", FILENAME_PDEX_DLL, linkStatusCode);
				exit(linkStatusCode);
			}
			
			mkdir("playdate_data", 0);
			CopyFileToFolder(StrLit(FILENAME_PDEX_DLL), StrLit("playdate_data"));
		}
		
		if (ctx.BUILD_PLAYDATE_DEVICE || ctx.BUILD_PLAYDATE_SIMULATOR)
		{
			CopyFileToFolder(StrLit(ROOT_DIR "\\pdxinfo"), StrLit("playdate_data"));
			
			CliArgList packageCmd = ZEROED;
			AddArgList(&packageCmd, &ctx.pdc_CommonFlags);
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
	if (ctx.RUN_TESTS)
	{
		PrintLine("\n[%s]", FILENAME_TESTS_EXE);
		
		CliArgList cmd = ZEROED;
		
		int statusCode = RunCliProgram(StrLit(FILENAME_TESTS_EXE), &cmd);
		if (statusCode != 0)
		{
			PrintLine_E("%s Failed! Status Code: %d", FILENAME_TESTS_EXE, statusCode);
			exit(statusCode);
		}
	}
	
	WriteLine("\n[pig_build.exe Finished Successfully]");
	return 0;
}
