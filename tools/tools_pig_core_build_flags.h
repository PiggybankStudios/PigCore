/*
File:   tools_pig_core_build_flags.h
Author: Taylor Robbins
Date:   06\23\2025
Description: 
	** Contains all the flags that PigCore uses to compile in all it's configurations.
	** These are pulled into a separate file so that various Pig Core based programs
	** can use these functions as a starting point for their own build scripts.
*/

#ifndef _TOOLS_PIG_CORE_BUILD_FLAGS_H
#define _TOOLS_PIG_CORE_BUILD_FLAGS_H

void Fill_cl_CommonFlags(CliArgList* cl_CommonFlags, bool DEBUG_BUILD, bool DUMP_PREPROCESSOR, bool DUMP_ASSEMBLY)
{
	AddArg(cl_CommonFlags, DEBUG_BUILD ? CL_STD_LIB_DYNAMIC_DBG : CL_STD_LIB_DYNAMIC);
	AddArg(cl_CommonFlags, CL_FULL_FILE_PATHS); //we need full file paths in errors for Sublime Text to be able to parse the errors and display them in the editor
	AddArg(cl_CommonFlags, CL_NO_LOGO); //Suppress the annoying Microsoft logo and copyright info that the compiler prints out
	AddArgNt(cl_CommonFlags, CL_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "d" : "2");
	if (!DEBUG_BUILD) { AddArgNt(cl_CommonFlags, CL_OPTIMIZATION_LEVEL, "y"); }
	if (!DEBUG_BUILD) { AddArgNt(cl_CommonFlags, CL_OPTIMIZATION_LEVEL, "t"); }
	AddArgNt(cl_CommonFlags, CL_WARNING_LEVEL, "X"); //Treat all warnings as errors
	if (DUMP_ASSEMBLY) { AddArgNt(cl_CommonFlags, CL_GENERATE_ASSEMB_LISTING, "s"); } //Generate assembly listing files with source code included
	AddArgInt(cl_CommonFlags, CL_WARNING_LEVEL, 4); //Use warning level 4, then disable various warnings we don't care about
	AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_LOGICAL_OP_ON_ADDRESS_OF_STR_CONST);
	AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_NAMELESS_STRUCT_OR_UNION);
	AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_STRUCT_WAS_PADDED);
	AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_DECLARATION_HIDES_CLASS_MEMBER);
	AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_FUNC_REMOVED);
	AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_USAGE_OF_DEPRECATED);
	AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_ASSIGNMENT_WITHIN_CONDITIONAL_EXPR);
	AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_NAMED_TYPEDEF_IN_PARENTHESES);
	AddArgInt(cl_CommonFlags, CL_ENABLE_WARNING, CL_WARNING_SWITCH_FALLTHROUGH);
	AddArgNt(cl_CommonFlags, CL_INCLUDE_DIR, "[ROOT]");
	if (DEBUG_BUILD)
	{
		AddArg(cl_CommonFlags, CL_DEBUG_INFO);
		//We don't care about these warnings in debug builds, but we will solve them when we go to build in release mode because they probably indicate mistakes at that point
		AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_SWITCH_ONLY_DEFAULT);
		AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_FUNC_PARAMETER);
		AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_LCOAL_VARIABLE);
		AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_CONDITIONAL_EXPR_IS_CONSTANT);
		AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_LOCAL_VAR_INIT_BUT_NOT_REFERENCED);
		AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_UNREACHABLE_CODE_DETECTED);
	}
	if (DUMP_PREPROCESSOR)
	{
		AddArg(cl_CommonFlags, CL_PRECOMPILE_ONLY);
		AddArg(cl_CommonFlags, CL_PRECOMPILE_PRESERVE_COMMENTS);
	}
}

// Flags that we use when compiling any C (not C++) program using MSVC compiler
void Fill_cl_LangCFlags(CliArgList* cl_LangCFlags)
{
	AddArgNt(cl_LangCFlags, CL_LANG_VERSION, "clatest"); //Use latest C language spec features
	AddArgNt(cl_LangCFlags, CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
}

// Flags that we use when compiling any C++ program using MSVC compiler
void Fill_cl_LangCppFlags(CliArgList* cl_LangCppFlags)
{	
	AddArgNt(cl_LangCppFlags, CL_LANG_VERSION, "c++20");
	AddArgInt(cl_LangCppFlags, CL_DISABLE_WARNING, CL_WARNING_ENUMERATION_MUST_HAVE_UNDERLYING_TYPE);
	AddArgInt(cl_LangCppFlags, CL_DISABLE_WARNING, CL_WARNING_BITWISE_OP_BETWEEN_ENUMS);
}

// Flags that we use when compiling any C program using Clang
void Fill_clang_CommonFlags(CliArgList* clang_CommonFlags, bool DEBUG_BUILD, bool DUMP_PREPROCESSOR)
{
	AddArg(clang_CommonFlags, CLANG_FULL_FILE_PATHS); //Print absolute paths in diagnostics TODO: Figure out how to resolve these back to windows paths for Sublime error linking?
	AddArgNt(clang_CommonFlags, CLANG_LANG_VERSION, "gnu2x"); //Use C20+ language spec (NOTE: We originally had -std=c2x but that didn't define MAP_ANONYMOUS and mmap was failing)
	AddArgNt(clang_CommonFlags, CLANG_WARNING_LEVEL, "all"); //This enables all the warnings about constructions that some users consider questionable, and that are easy to avoid (or modify to prevent the warning), even in conjunction with macros
	AddArgNt(clang_CommonFlags, CLANG_WARNING_LEVEL, "extra"); //This enables some extra warning flags that are not enabled by -Wall
	AddArgNt(clang_CommonFlags, CLANG_ENABLE_WARNING, CLANG_WARNING_SHADOWING);
	AddArgNt(clang_CommonFlags, CLANG_ENABLE_WARNING, CLANG_WARNING_MISSING_FALLTHROUGH_IN_SWITCH);
	AddArgNt(clang_CommonFlags, CLANG_DISABLE_WARNING, CLANG_WARNING_SWITCH_MISSING_CASES);
	AddArgNt(clang_CommonFlags, CLANG_DISABLE_WARNING, CLANG_WARNING_UNUSED_FUNCTION);
	if (DEBUG_BUILD)
	{
		//We don't care about these warnings in debug builds, but we will solve them when we go to build in release mode because they probably indicate mistakes at that point
		AddArgNt(clang_CommonFlags, CLANG_DISABLE_WARNING, "unused-parameter");
		AddArgNt(clang_CommonFlags, CLANG_DISABLE_WARNING, "unused-variable");
	}
	if (DUMP_PREPROCESSOR) { AddArg(clang_CommonFlags, CLANG_PRECOMPILE_ONLY); }
}

// Flags for when we are compiling the linux version of a program using Clang
void Fill_clang_LinuxFlags(CliArgList* clang_LinuxFlags, bool DEBUG_BUILD)
{
	AddArgNt(clang_LinuxFlags, CLANG_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "0" : "2");
	AddArgNt(clang_LinuxFlags, CLANG_INCLUDE_DIR, "[ROOT]");
	AddArgStr(clang_LinuxFlags, CLANG_LIBRARY_DIR, DEBUG_BUILD ? StrLit("[ROOT]/third_party/_lib_debug") : StrLit("[ROOT]/third_party/_lib_release"));
	AddArg(clang_LinuxFlags, "-mssse3"); //For MeowHash to work we need sse3 support
	AddArg(clang_LinuxFlags, "-maes"); //For MeowHash to work we need aes support
	if (DEBUG_BUILD) { AddArgNt(clang_LinuxFlags, CLANG_DEBUG_INFO, "dwarf-4"); }
}

void Fill_cl_CommonLinkerFlags(CliArgList* cl_CommonLinkerFlags, bool DEBUG_BUILD)
{
	AddArgNt(cl_CommonLinkerFlags, LINK_LIBRARY_DIR, DEBUG_BUILD ? "[ROOT]/third_party/_lib_debug" : "[ROOT]/third_party/_lib_release");
	AddArg(cl_CommonLinkerFlags, LINK_DISABLE_INCREMENTAL);
}

void Fill_clang_LinuxCommonLibraries(CliArgList* clang_LinuxCommonLibraries, bool BUILD_WITH_SOKOL_APP)
{
	AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "m"); //Include the math library (required for stuff like sinf, atan, etc.)
	AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "dl"); //Needed for dlopen and similar functions
	if (BUILD_WITH_SOKOL_APP)
	{
		AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "X11");
		AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "Xi");
		AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "Xcursor");
	}
}

// These are all the libraries we need when compiling a Windows binary that contains code from PigCore
void Fill_cl_PigCoreLibraries(CliArgList* cl_PigCoreLibraries, bool BUILD_WITH_RAYLIB, bool BUILD_WITH_BOX2D, bool BUILD_WITH_SDL, bool BUILD_WITH_OPENVR, bool BUILD_WITH_IMGUI, bool BUILD_WITH_PHYSX, bool BUILD_WITH_HTTP)
{
	if (BUILD_WITH_RAYLIB) { AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "raylib.lib"); } //NOTE: raylib.lib MUST be before User32.lib and others
	AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "Gdi32.lib"); //Needed for CreateFontA and other Windows graphics functions
	AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "User32.lib"); //Needed for GetForegroundWindow, GetDC, etc.
	AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "Ole32.lib"); //Needed for Combaseapi.h, CoInitializeEx, CoCreateInstance, etc.
	AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "Shell32.lib"); //Needed for SHGetSpecialFolderPathA
	AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "Shlwapi.lib"); //Needed for PathFileExistsA
	if (BUILD_WITH_RAYLIB)
	{
		AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "Kernel32.lib");
		AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "Winmm.lib");
	}
	if (BUILD_WITH_BOX2D) { AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "box2d.lib"); }
	if (BUILD_WITH_SDL) { AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "SDL2.lib"); }
	if (BUILD_WITH_OPENVR) { AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "openvr_api.lib"); }
	if (BUILD_WITH_PHYSX) { AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "PhysX_static_64.lib"); }
	if (BUILD_WITH_HTTP) { AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "Winhttp.lib"); }
}

// These are all the libraries we need when compiling a Linux binary that contains code from PigCore
void Fill_clang_PigCoreLibraries(CliArgList* clang_PigCoreLibraries, bool BUILD_WITH_BOX2D, bool BUILD_WITH_SOKOL_GFX, bool TARGET_IS_LINUX)
{
	AddArgNt(clang_PigCoreLibraries, CLANG_SYSTEM_LIBRARY, "pthread");
	if (TARGET_IS_LINUX) { AddArgNt(clang_PigCoreLibraries, CLANG_SYSTEM_LIBRARY, "fontconfig"); }
	if (BUILD_WITH_SOKOL_GFX) { AddArgNt(clang_PigCoreLibraries, CLANG_SYSTEM_LIBRARY, "GL"); }
	if (BUILD_WITH_BOX2D) { AddArgNt(clang_PigCoreLibraries, CLANG_SYSTEM_LIBRARY, "box2d"); }
}

void Fill_clang_WasmFlags(CliArgList* clang_WasmFlags, bool DEBUG_BUILD)
{
	AddArgNt(clang_WasmFlags, CLANG_TARGET_ARCHITECTURE, "wasm32");
	AddArgNt(clang_WasmFlags, CLANG_M_FLAG, "bulk-memory");
	AddArgNt(clang_WasmFlags, CLANG_INCLUDE_DIR, "[ROOT]");
	if (DEBUG_BUILD) { AddArg(clang_WasmFlags, CLANG_DEBUG_INFO_DEFAULT); }
	else { AddArgNt(clang_WasmFlags, CLANG_OPTIMIZATION_LEVEL, "2"); }
}

void Fill_clang_WebFlags(CliArgList* clang_WebFlags, bool USE_EMSCRIPTEN)
{	
	if (USE_EMSCRIPTEN)
	{
		AddArgNt(clang_WebFlags, EMSCRIPTEN_S_FLAG, "USE_SDL");
		AddArgNt(clang_WebFlags, EMSCRIPTEN_S_FLAG, "ALLOW_MEMORY_GROWTH");
	}
	else
	{
		AddArgNt(clang_WebFlags, CLANG_INCLUDE_DIR, "[ROOT]/wasm/std/include");
		AddArg(clang_WebFlags, CLANG_NO_ENTRYPOINT);
		AddArg(clang_WebFlags, CLANG_ALLOW_UNDEFINED);
		AddArg(clang_WebFlags, CLANG_NO_STD_LIBRARIES);
		AddArg(clang_WebFlags, CLANG_NO_STD_INCLUDES);
		AddArgNt(clang_WebFlags, CLANG_EXPORT_SYMBOL, "__heap_base");
	}
}

void Fill_clang_OrcaFlags(CliArgList* clang_OrcaFlags, Str8 orcaSdkPath)
{
	AddArg(clang_OrcaFlags, CLANG_NO_ENTRYPOINT);
	AddArg(clang_OrcaFlags, CLANG_EXPORT_DYNAMIC);
	AddArgStr(clang_OrcaFlags, CLANG_STDLIB_FOLDER, JoinStrings2(orcaSdkPath, StrLit("/orca-libc"), false));
	AddArgStr(clang_OrcaFlags, CLANG_INCLUDE_DIR, JoinStrings2(orcaSdkPath, StrLit("/src"), false));
	AddArgStr(clang_OrcaFlags, CLANG_INCLUDE_DIR, JoinStrings2(orcaSdkPath, StrLit("/src/ext"), false));
	AddArgStr(clang_OrcaFlags, CLANG_LIBRARY_DIR, JoinStrings2(orcaSdkPath, StrLit("/bin"), false));
	AddArgNt(clang_OrcaFlags, CLANG_SYSTEM_LIBRARY, "orca_wasm");
	AddArgNt(clang_OrcaFlags, CLANG_DEFINE, "__ORCA__"); //#define __ORCA__ so that base_compiler_check.h can set TARGET_IS_ORCA
}

void Fill_cl_PlaydateSimulatorCompilerFlags(CliArgList* cl_PlaydateSimulatorCompilerFlags, bool DEBUG_BUILD, Str8 playdateSdkDir_C_API)
{
	//TODO: Just use cl_CommonFlags?
	AddArg(cl_PlaydateSimulatorCompilerFlags, CL_NO_LOGO);
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_WARNING_LEVEL, "3");
	AddArg(cl_PlaydateSimulatorCompilerFlags, CL_NO_WARNINGS_AS_ERRORS);
	AddArg(cl_PlaydateSimulatorCompilerFlags, DEBUG_BUILD ? CL_STD_LIB_DYNAMIC_DBG : CL_STD_LIB_DYNAMIC);
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "d" : "2");
	if (DEBUG_BUILD) { AddArg(cl_PlaydateSimulatorCompilerFlags, CL_DEBUG_INFO); }
	
	//TODO: Just use cl_LangCFlags?
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_LANG_VERSION, "clatest"); //Use latest C language spec features
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
	
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_INCLUDE_DIR, "[ROOT]");
	if (playdateSdkDir_C_API.length > 0) { AddArgStr(cl_PlaydateSimulatorCompilerFlags, CL_INCLUDE_DIR, playdateSdkDir_C_API); }
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "TARGET_SIMULATOR=1");
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "TARGET_EXTENSION=1");
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "__HEAP_SIZE=8388208");
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "__STACK_SIZE=61800");
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "_WINDLL");
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "_MBCS");
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "WIN32");
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "_WINDOWS");
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "_WINDLL=1");
	AddArg(cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_BUFFER_SECURITY_CHECK);
	AddArg(cl_PlaydateSimulatorCompilerFlags, CL_DISABLE_MINIMAL_REBUILD);
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_RUNTIME_CHECKS, "1"); //Enable fast runtime checks (Equivalent to "su")
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_CALLING_CONVENTION, "d"); //Use __cdecl calling convention
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_INLINE_EXPANSION_LEVEL, "0"); //Disable inline expansions
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_INTERNAL_COMPILER_ERROR_BEHAVIOR, "prompt"); //TODO: Do we need this?
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_LANG_CONFORMANCE_OPTION, "forScope"); //Enforce Standard C++ for scoping rules (on by default)
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_LANG_CONFORMANCE_OPTION, "inline"); //Remove unreferenced functions or data if they're COMDAT or have internal linkage only (off by default)
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_LANG_CONFORMANCE_OPTION, "wchar_t"); //wchar_t is a native type, not a typedef (on by default)
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_FLOATING_POINT_MODEL, "precise"); //"precise" floating-point model; results are predictable
}

void Fill_link_PlaydateSimulatorLinkerFlags(CliArgList* link_PlaydateSimulatorLinkerFlags, bool DEBUG_BUILD)
{
	AddArg(link_PlaydateSimulatorLinkerFlags, LINK_NO_LOGO);
	AddArgNt(link_PlaydateSimulatorLinkerFlags, LINK_TARGET_ARCHITECTURE, "X64");
	AddArg(link_PlaydateSimulatorLinkerFlags, LINK_DATA_EXEC_COMPAT);
	AddArg(link_PlaydateSimulatorLinkerFlags, LINK_ENABLE_ASLR);
	AddArg(link_PlaydateSimulatorLinkerFlags, LINK_CONSOLE_APPLICATION);
	AddArgInt(link_PlaydateSimulatorLinkerFlags, LINK_TYPELIB_RESOURCE_ID, 1);
	AddArg(link_PlaydateSimulatorLinkerFlags, LINK_ENABLE_INCREMENTAL);
	AddArgNt(link_PlaydateSimulatorLinkerFlags, LINK_INCREMENTAL_FILE_NAME, "tests.ilk"); //TODO: This should really move down below inside the tests.exe block
	AddArg(link_PlaydateSimulatorLinkerFlags, LINK_CREATE_ASSEMBLY_MANIFEST);
	AddArgNt(link_PlaydateSimulatorLinkerFlags, LINK_ASSEMBLY_MANIFEST_FILE, "tests.intermediate.manifest"); //TODO: This should really move down below inside the tests.exe block
	AddArgNt(link_PlaydateSimulatorLinkerFlags, LINK_LINK_TIME_CODEGEN_FILE, "tests.iobj"); //TODO: This should really move down below inside the tests.exe block
	AddArgNt(link_PlaydateSimulatorLinkerFlags, LINK_EMBED_UAC_INFO_EX, "level='asInvoker' uiAccess='false'");
	if (DEBUG_BUILD) { AddArg(link_PlaydateSimulatorLinkerFlags, LINK_DEBUG_INFO); }
}

void Fill_link_PlaydateSimulatorLibraries(CliArgList* link_PlaydateSimulatorLibraries)
{
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "kernel32.lib");
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "user32.lib");
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "gdi32.lib");
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "winspool.lib");
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "shell32.lib");
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "ole32.lib");
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "oleaut32.lib");
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "uuid.lib");
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "comdlg32.lib");
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "advapi32.lib");
}

void Fill_gcc_PlaydateDeviceCommonFlags(CliArgList* gcc_PlaydateDeviceCommonFlags, Str8 playdateSdkDir_C_API)
{
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_INCLUDE_DIR, "[ROOT]");
	if (playdateSdkDir_C_API.length > 0) { AddArgStr(gcc_PlaydateDeviceCommonFlags, GCC_INCLUDE_DIR, playdateSdkDir_C_API); }
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "TARGET_PLAYDATE=1");
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "TARGET_EXTENSION=1");
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "__HEAP_SIZE=8388208");
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "__STACK_SIZE=61800");
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "__FPU_USED=1");
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_USE_SPEC_FILE, "nano.specs"); //Required for things like _read, _write, _exit, etc. to not be pulled in as requirements from standard library
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_USE_SPEC_FILE, "nosys.specs"); //TODO: Is this helping?
	AddArg(gcc_PlaydateDeviceCommonFlags, GCC_TARGET_THUMB);
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_TARGET_CPU, "cortex-m7");
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_FLOAT_ABI_MODE, "hard"); //Use hardware for floating-point operations
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_TARGET_FPU, "fpv5-sp-d16");
}

void Fill_gcc_PlaydateDeviceCompilerFlags(CliArgList* gcc_PlaydateDeviceCompilerFlags)
{
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DEBUG_INFO_EX, "3");
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DEBUG_INFO_EX, "dwarf-2");
	AddArg(gcc_PlaydateDeviceCompilerFlags, GCC_STD_LIB_DYNAMIC);
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DEPENDENCY_FILE, "tests.d"); //TODO: This should really move down below inside the tests.exe block
	AddArgInt(gcc_PlaydateDeviceCompilerFlags, GCC_ALIGN_FUNCS_TO, 16);
	AddArg(gcc_PlaydateDeviceCompilerFlags, GCC_SEP_DATA_SECTIONS);
	AddArg(gcc_PlaydateDeviceCompilerFlags, GCC_SEP_FUNC_SECTIONS);
	AddArg(gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_EXCEPTIONS);
	AddArg(gcc_PlaydateDeviceCompilerFlags, GCC_OMIT_FRAME_PNTR);
	AddArg(gcc_PlaydateDeviceCompilerFlags, GCC_GLOBAL_VAR_NO_COMMON);
	AddArg(gcc_PlaydateDeviceCompilerFlags, GCC_VERBOSE_ASSEMBLY); //TODO: Should this only be on when DEBUG_BUILD?
	AddArg(gcc_PlaydateDeviceCompilerFlags, GCC_ONLY_RELOC_WORD_SIZE);
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_WARNING_LEVEL, "all");
	// AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_ENABLE_WARNING, "double-promotion");
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "unknown-pragmas");
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "comment");
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "switch");
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "nonnull");
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "unused");
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "missing-braces");
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "char-subscripts");
}

void Fill_gcc_PlaydateDeviceLinkerFlags(CliArgList* gcc_PlaydateDeviceLinkerFlags, Str8 playdateSdkDir)
{
	AddArg(gcc_PlaydateDeviceLinkerFlags, GCC_NO_STD_STARTUP);
	AddArgNt(gcc_PlaydateDeviceLinkerFlags, GCC_ENTRYPOINT_NAME, "eventHandler");
	AddArg(gcc_PlaydateDeviceLinkerFlags, GCC_DISABLE_RWX_WARNING);
	AddArg(gcc_PlaydateDeviceLinkerFlags, GCC_CREF);
	AddArg(gcc_PlaydateDeviceLinkerFlags, GCC_GC_SECTIONS);
	AddArg(gcc_PlaydateDeviceLinkerFlags, GCC_DISABLE_MISMATCH_WARNING);
	AddArg(gcc_PlaydateDeviceLinkerFlags, GCC_EMIT_RELOCATIONS);
	AddArgStr(gcc_PlaydateDeviceLinkerFlags, GCC_LINKER_SCRIPT, JoinStrings2(playdateSdkDir, StrLit("/C_API/buildsupport/link_map.ld"), false));
}

void Fill_pdc_CommonFlags(CliArgList* pdc_CommonFlags, Str8 playdateSdkDir)
{
	AddArg(pdc_CommonFlags, PDC_QUIET); //Quiet mode, suppress non-error output
	if (playdateSdkDir.length > 0) { AddArgStr(pdc_CommonFlags, PDC_SDK_PATH, playdateSdkDir); }
}

#endif //  _TOOLS_PIG_CORE_BUILD_FLAGS_H
