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
#include "tools/tools_shdc_flags.h"
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
	Str8 wslClangCompiler = StrLit("wsl clang-18"); //we are using the WSL instance with clang-18 installed to compile for Linux
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
	AddArg(&clang_LinuxFlags, "-mssse3");
	AddArg(&clang_LinuxFlags, "-maes");
	if (DEBUG_BUILD) { AddArgNt(&clang_LinuxFlags, CLANG_DEBUG_INFO, "dwarf-4"); }
	
	// +==============================+
	// |     cl_CommonLinkerFlags     |
	// +==============================+
	CliArgList cl_CommonLinkerFlags = ZEROED;
	AddArgNt(&cl_CommonLinkerFlags, LINK_LIBRARY_DIR, DEBUG_BUILD ? "..\\third_party\\_lib_debug" : "..\\third_party\\_lib_release");
	AddArg(&cl_CommonLinkerFlags, LINK_NOT_INCREMENTAL);
	
	// +==============================+
	// |  clang_LinuxCommonLibraries  |
	// +==============================+
	CliArgList clang_LinuxCommonLibraries = ZEROED; //"linux_linker_flags"
	AddArgNt(&clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "m"); //Include the math library (required for stuff like sinf, atan, etc.)
	AddArgNt(&clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "dl"); //Needed for dlopen and similar functions
	
	// +==============================+
	// |     cl_PigCoreLibraries      |
	// +==============================+
	CliArgList cl_PigCoreLibraries = ZEROED; //These are all the libraries we need when compiling a binary that contains code from PigCore
	AddArg(&cl_PigCoreLibraries, "Gdi32.lib"); //Needed for CreateFontA and other Windows graphics functions
	AddArg(&cl_PigCoreLibraries, "User32.lib"); //Needed for GetForegroundWindow, GetDC, etc.
	AddArg(&cl_PigCoreLibraries, "Ole32.lib"); //Needed for Combaseapi.h, CoInitializeEx, CoCreateInstance, etc.
	AddArg(&cl_PigCoreLibraries, "Shell32.lib"); //Needed for SHGetSpecialFolderPathA
	AddArg(&cl_PigCoreLibraries, "Shlwapi.lib"); //Needed for PathFileExistsA
	if (BUILD_WITH_RAYLIB)
	{
		AddArg(&cl_PigCoreLibraries, "Kernel32.lib");
		AddArg(&cl_PigCoreLibraries, "Winmm.lib");
	}
	if (BUILD_WITH_BOX2D) { AddArg(&cl_PigCoreLibraries, "box2d.lib"); }
	if (BUILD_WITH_SDL) { AddArg(&cl_PigCoreLibraries, "SDL2.lib"); }
	if (BUILD_WITH_OPENVR) { AddArg(&cl_PigCoreLibraries, "openvr_api.lib"); }
	if (BUILD_WITH_PHYSX) { AddArg(&cl_PigCoreLibraries, "PhysX_static_64.lib"); }
	
	// +==============================+
	// |    clang_PigCoreLibraries    |
	// +==============================+
	CliArgList clang_PigCoreLibraries = ZEROED; //These are all the libraries we need when compiling a binary that contains code from PigCore
	AddArgNt(&clang_PigCoreLibraries, CLANG_SYSTEM_LIBRARY, "fontconfig");
	if (BUILD_WITH_SOKOL_GFX) { AddArgNt(&clang_PigCoreLibraries, CLANG_SYSTEM_LIBRARY, "GL"); }
	
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
			AddArgNt(&cmd, CL_BINARY_NAME, FILENAME_PIGGEN);
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
			AddArgNt(&cmd, CLANG_BINARY_NAME, LINUX_FILENAME_PIGGEN);
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
				AddArgStr(&cmd, CL_OBJ_NAME, objPath);
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
				AddArgStr(&cmd, CLANG_OBJECT_NAME, oPath);
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
			AddArgNt(&cmd, CL_OBJ_NAME, FILENAME_IMGUI);
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
			AddArgNt(&cmd, CL_OBJ_NAME, FILENAME_PHYSX);
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
			AddArgNt(&cmd, CL_BINARY_NAME, FILENAME_PIGCORE);
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
			AddArgNt(&cmd, CLANG_BINARY_NAME, LINUX_FILENAME_PIGCORE);
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
	if (RUN_TESTS && !BUILD_TESTS && !DoesFileExist(StrLit(FILENAME_TESTS))) { PrintLine("Building %s because it's missing", FILENAME_TESTS); BUILD_TESTS = true; }
	if (BUILD_TESTS)
	{
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(&isMsvcInitialized);
			PrintLine("\n[Building %s for Windows...]", FILENAME_TESTS);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, "..\\tests\\tests_main.c");
			AddArgNt(&cmd, CL_BINARY_NAME, FILENAME_TESTS);
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
			AddArgNt(&cmd, CLANG_BINARY_NAME, LINUX_FILENAME_TESTS);
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
