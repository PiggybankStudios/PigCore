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

#include "tools/tools_shared.h"
#include "tools/tools_str_array.h"
#include "tools/tools_cli.h"

#include "tools/tools_msvc_flags.h"
#include "tools/tools_clang_flags.h"
#include "tools/tools_gcc_flags.h"
#include "tools/tools_emscripten_flags.h"
#include "tools/tools_shdc_flags.h"
#include "tools/tools_pdc_flags.h"

#include "tools/tools_build_helpers.h"
#include "tools/tools_pig_core_build_flags.h"

#define BUILD_CONFIG_PATH       "../build_config.h"

#define FOLDERNAME_GENERATED_CODE  "gen"
#define FOLDERNAME_LINUX           "linux"
#define FOLDERNAME_OSX             "osx"
#define FOLDERNAME_WEB             "web"
#define FOLDERNAME_ORCA            "orca"

#define FILENAME_PIGGEN_EXE        "piggen.exe"
#define FILENAME_PIGGEN            "piggen"
#define FILENAME_TRACY_DLL         "tracy.dll"
#define FILENAME_TRACY_LIB         "tracy.lib"
#define FILENAME_TRACY_SO          "tracy.so"
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

#if BUILDING_ON_WINDOWS
#define TOOL_EXE_NAME      "pig_build.exe"
#else
#define TOOL_EXE_NAME      "pig_build"
#endif

static inline void PrintUsage()
{
	WriteLine_E("Usage: " TOOL_EXE_NAME " [build_config_path] [is_msvc_compiler_initialized]");
}

int main(int argc, char* argv[])
{
	// PrintLine("Running %s...", TOOL_EXE_NAME);
	
	bool isMsvcInitialized = WasMsvcDevBatchRun();
	bool isEmsdkInitialized = WasEmsdkEnvBatchRun();
	
	// +==============================+
	// |       Extract Defines        |
	// +==============================+
	Str8 buildConfigContents = ReadEntireFile(StrLit(BUILD_CONFIG_PATH));
	
	bool DEBUG_BUILD              = ExtractBoolDefine(buildConfigContents, StrLit("DEBUG_BUILD"));
	bool PROFILING_ENABLED        = ExtractBoolDefine(buildConfigContents, StrLit("PROFILING_ENABLED"));
	bool BUILD_PIGGEN             = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_PIGGEN"));
	bool BUILD_SHADERS            = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_SHADERS"));
	bool RUN_PIGGEN               = ExtractBoolDefine(buildConfigContents, StrLit("RUN_PIGGEN"));
	bool BUILD_TRACY_DLL          = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_TRACY_DLL"));
	bool BUILD_IMGUI_OBJ          = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_IMGUI_OBJ"));
	bool BUILD_PHYSX_OBJ          = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_PHYSX_OBJ"));
	bool BUILD_PIG_CORE_DLL       = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_PIG_CORE_DLL"));
	bool BUILD_TESTS              = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_TESTS"));
	bool RUN_TESTS                = ExtractBoolDefine(buildConfigContents, StrLit("RUN_TESTS"));
	bool GENERATE_PROTOBUF        = ExtractBoolDefine(buildConfigContents, StrLit("GENERATE_PROTOBUF"));
	bool DUMP_PREPROCESSOR        = ExtractBoolDefine(buildConfigContents, StrLit("DUMP_PREPROCESSOR"));
	bool DUMP_ASSEMBLY            = ExtractBoolDefine(buildConfigContents, StrLit("DUMP_ASSEMBLY"));
	bool CONVERT_WASM_TO_WAT      = ExtractBoolDefine(buildConfigContents, StrLit("CONVERT_WASM_TO_WAT"));
	bool USE_EMSCRIPTEN           = ExtractBoolDefine(buildConfigContents, StrLit("USE_EMSCRIPTEN"));
	// bool ENABLE_AUTO_PROFILE      = ExtractBoolDefine(buildConfigContents, StrLit("ENABLE_AUTO_PROFILE"));
	// bool RUN_FUZZER               = ExtractBoolDefine(buildConfigContents, StrLit("RUN_FUZZER"));
	bool BUILD_WINDOWS            = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WINDOWS"));
	bool BUILD_LINUX              = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_LINUX"));
	bool BUILD_OSX                = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_OSX"));
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
	bool BUILD_WITH_HTTP          = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_HTTP"));
	bool BUILD_WITH_PROTOBUF      = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_WITH_PROTOBUF"));
	
	free(buildConfigContents.chars);
	
	// +==============================+
	// | Enforce Option Restrictions  |
	// +==============================+
	if (BUILD_WINDOWS && !BUILDING_ON_WINDOWS)
	{
		PrintLine_E("BUILD_WINDOWS does not working when building on non-Windows platforms");
		BUILD_WINDOWS = false;
	}
	if (BUILD_OSX && !BUILDING_ON_OSX)
	{
		PrintLine_E("BUILD_OSX does not working when building on non-OSX platforms");
		BUILD_OSX = false;
	}
	
	// +==============================+
	// |        Find SDK Paths        |
	// +==============================+
	Str8 emscriptenSdkPath = ZEROED;
	if (BUILD_WEB && USE_EMSCRIPTEN)
	{
		emscriptenSdkPath = GetEmscriptenSdkPath();
		PrintLine("Emscripten SDK path: \"%.*s\"", emscriptenSdkPath.length, emscriptenSdkPath.chars);
		InitializeEmsdkIf(StrLit(".."), &isEmsdkInitialized);
	}
	
	Str8 orcaSdkPath = ZEROED;
	if (BUILD_ORCA)
	{
		orcaSdkPath = GetOrcaSdkPath();
		PrintLine("Orca SDK path: \"%.*s\"", orcaSdkPath.length, orcaSdkPath.chars);
	}
	
	Str8 playdateSdkDir = ZEROED;
	Str8 playdateSdkDir_C_API = ZEROED;
	if (BUILD_PLAYDATE_DEVICE || BUILD_PLAYDATE_SIMULATOR)
	{
		playdateSdkDir = GetPlaydateSdkPath();
		PrintLine("Playdate SDK path: \"%.*s\"", playdateSdkDir.length, playdateSdkDir.chars);
		playdateSdkDir_C_API = JoinStrings2(playdateSdkDir, StrLit("/C_API"), false);
	}
	
	// +==============================+
	// |       Fill CliArgLists       |
	// +==============================+
	CliArgList cl_CommonFlags                    = ZEROED; Fill_cl_CommonFlags(&cl_CommonFlags, DEBUG_BUILD, DUMP_PREPROCESSOR, DUMP_ASSEMBLY);
	CliArgList cl_LangCFlags                     = ZEROED; Fill_cl_LangCFlags(&cl_LangCFlags);
	CliArgList cl_LangCppFlags                   = ZEROED; Fill_cl_LangCppFlags(&cl_LangCppFlags);
	CliArgList clang_CommonFlags                 = ZEROED; Fill_clang_CommonFlags(&clang_CommonFlags, DEBUG_BUILD, DUMP_PREPROCESSOR);
	CliArgList clang_LinuxFlags                  = ZEROED; Fill_clang_LinuxFlags(&clang_LinuxFlags, DEBUG_BUILD);
	CliArgList cl_CommonLinkerFlags              = ZEROED; Fill_cl_CommonLinkerFlags(&cl_CommonLinkerFlags, DEBUG_BUILD);
	CliArgList clang_LinuxCommonLibraries        = ZEROED; Fill_clang_LinuxCommonLibraries(&clang_LinuxCommonLibraries, BUILD_WITH_SOKOL_APP);
	CliArgList cl_PigCoreLibraries               = ZEROED; Fill_cl_PigCoreLibraries(&cl_PigCoreLibraries, BUILD_WITH_RAYLIB, BUILD_WITH_BOX2D, BUILD_WITH_SDL, BUILD_WITH_OPENVR, BUILD_WITH_IMGUI, BUILD_WITH_PHYSX, BUILD_WITH_HTTP);
	CliArgList clang_PigCoreLibraries            = ZEROED; Fill_clang_PigCoreLibraries(&clang_PigCoreLibraries, BUILD_WITH_BOX2D, BUILD_WITH_SOKOL_GFX, !BUILDING_ON_OSX);
	CliArgList clang_WasmFlags                   = ZEROED; Fill_clang_WasmFlags(&clang_WasmFlags, DEBUG_BUILD);
	CliArgList clang_WebFlags                    = ZEROED; Fill_clang_WebFlags(&clang_WebFlags, USE_EMSCRIPTEN);
	CliArgList clang_OrcaFlags                   = ZEROED; Fill_clang_OrcaFlags(&clang_OrcaFlags, orcaSdkPath);
	CliArgList cl_PlaydateSimulatorCompilerFlags = ZEROED; Fill_cl_PlaydateSimulatorCompilerFlags(&cl_PlaydateSimulatorCompilerFlags, DEBUG_BUILD, playdateSdkDir_C_API);
	CliArgList link_PlaydateSimulatorLinkerFlags = ZEROED; Fill_link_PlaydateSimulatorLinkerFlags(&link_PlaydateSimulatorLinkerFlags, DEBUG_BUILD);
	CliArgList link_PlaydateSimulatorLibraries   = ZEROED; Fill_link_PlaydateSimulatorLibraries(&link_PlaydateSimulatorLibraries);
	CliArgList gcc_PlaydateDeviceCommonFlags     = ZEROED; Fill_gcc_PlaydateDeviceCommonFlags(&gcc_PlaydateDeviceCommonFlags, playdateSdkDir_C_API);
	CliArgList gcc_PlaydateDeviceCompilerFlags   = ZEROED; Fill_gcc_PlaydateDeviceCompilerFlags(&gcc_PlaydateDeviceCompilerFlags);
	CliArgList gcc_PlaydateDeviceLinkerFlags     = ZEROED; Fill_gcc_PlaydateDeviceLinkerFlags(&gcc_PlaydateDeviceLinkerFlags, playdateSdkDir);
	CliArgList pdc_CommonFlags                   = ZEROED; Fill_pdc_CommonFlags(&pdc_CommonFlags, playdateSdkDir);
	
	if (BUILD_WITH_PROTOBUF)
	{
		//NOTE: Generated .pb-c.h files contain an #include that doesn't go through "third_party/protobuf_c/" so we add this as an
		//      include directory explicitly and from there it can find <protobuf-c/protobuf-c.h>
		AddArgNt(&cl_CommonFlags, CL_INCLUDE_DIR, "[ROOT]/third_party/protobuf_c");
		AddArgNt(&clang_CommonFlags, CLANG_INCLUDE_DIR, "[ROOT]/third_party/protobuf_c");
	}
	
	// +--------------------------------------------------------------+
	// |                   Generate Protobuf Files                    |
	// +--------------------------------------------------------------+
	if (GENERATE_PROTOBUF)
	{
		WriteLine("\n[Generating Protobuf...]");
		
		#define PROTOC_C_OUT_PATH "--c_out=\"[VAL]\""
		#define PROTOC_PLUGIN_EXE_PATH "--plugin=\"[VAL]\""
		#define PROTOC_PROTO_PATH "--proto_path=\"[VAL]\""
		#define PROTOC_ERROR_FORMAT "--error_format=[VAL]"
		
		#if BUILDING_ON_WINDOWS
		Str8 protocExe = StrLit("wsl protoc");
		#else
		Str8 protocExe = StrLit("protoc");
		#endif
		
		CliArgList proto_CommonFlags = ZEROED;
		AddArgNt(&proto_CommonFlags, PROTOC_PLUGIN_EXE_PATH, "[ROOT]/third_party/_tools/linux/protoc-gen-c");
		AddArgNt(&proto_CommonFlags, PROTOC_ERROR_FORMAT, "msvs");
		AddArgNt(&proto_CommonFlags, PROTOC_PROTO_PATH, "[ROOT]");
		
		//NOTE: For some reason when [ROOT] folder is given as the first proto_path it likes to make a folder next to the .proto file with the name of the folder it resides in (like making "parse" folder next to "parse/parse_proto_google_types.proto")
		//      To counteract this, we add the primary folder proto_path first THEN add proto_CommonFlags which includes [ROOT] as a place to look for import resolves
		
		//TODO: The functions inside the generated files are not dllexport and will not be available to apps linking with pig_core.dll!
		
		//TODO: Rather than manually running on a specific set of .proto files, we should resursively search the folders and find all .proto files
		{
			CliArgList cmd = ZEROED;
			cmd.pathSepChar = '/';
			AddArgNt(&cmd, PROTOC_PROTO_PATH, "[ROOT]/parse");
			AddArgList(&cmd, &proto_CommonFlags);
			AddArgNt(&cmd, PROTOC_C_OUT_PATH, "[ROOT]/parse");
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/parse/parse_proto_google_types.proto");
			RunCliProgramAndExitOnFailure(protocExe, &cmd, StrLit("protoc Failed on parse_proto_google_types.proto!"));
		}
		{
			CliArgList cmd = ZEROED;
			cmd.pathSepChar = '/';
			AddArgNt(&cmd, PROTOC_PROTO_PATH, "[ROOT]/tests");
			AddArgList(&cmd, &proto_CommonFlags);
			AddArgNt(&cmd, PROTOC_C_OUT_PATH, "[ROOT]/tests");
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/tests/tests_proto_types.proto");
			RunCliProgramAndExitOnFailure(protocExe, &cmd, StrLit("protoc Failed on tests_proto_types.proto!"));
		}
	}
	
	// +--------------------------------------------------------------+
	// |                       Build piggen.exe                       |
	// +--------------------------------------------------------------+
	#if BUILDING_ON_WINDOWS
	#define RUNNABLE_FILENAME_PIGGEN FILENAME_PIGGEN_EXE
	#else
	#define RUNNABLE_FILENAME_PIGGEN FILENAME_PIGGEN
	#endif
	if (RUN_PIGGEN && !BUILD_PIGGEN && !DoesFileExist(StrLit(RUNNABLE_FILENAME_PIGGEN))) { PrintLine("Building %s because it's missing", RUNNABLE_FILENAME_PIGGEN); BUILD_PIGGEN = true; }
	if (BUILD_PIGGEN)
	{
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit(".."), &isMsvcInitialized);
			PrintLine("\n[Building %s for Windows...]", FILENAME_PIGGEN_EXE);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/piggen/piggen_main.c");
			AddArgNt(&cmd, CL_BINARY_FILE, FILENAME_PIGGEN_EXE);
			AddArgList(&cmd, &cl_CommonFlags);
			AddArgList(&cmd, &cl_LangCFlags);
			if (DUMP_ASSEMBLY) { AddArgNt(&cmd, CL_ASSEMB_LISTING_FILE, "piggen.asm"); }
			AddArg(&cmd, CL_LINK);
			AddArgList(&cmd, &cl_CommonLinkerFlags);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "Shlwapi.lib"); //Needed for PathFileExistsA
			
			RunCliProgramAndExitOnFailure(StrLit(EXE_MSVC_CL), &cmd, StrLit("Failed to build " FILENAME_PIGGEN_EXE "!"));
			AssertFileExist(StrLit(FILENAME_PIGGEN_EXE), true);
			PrintLine("[Built %s for Windows!]", FILENAME_PIGGEN_EXE);
		}
		if (BUILD_LINUX)
		{
			PrintLine("\n[Building %s for Linux...]", FILENAME_PIGGEN);
			
			CliArgList cmd = ZEROED;
			cmd.pathSepChar = '/';
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/piggen/piggen_main.c");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, FILENAME_PIGGEN);
			AddArgList(&cmd, &clang_CommonFlags);
			AddArgList(&cmd, &clang_LinuxFlags);
			AddArgList(&cmd, &clang_LinuxCommonLibraries);
			
			#if BUILDING_ON_LINUX
			Str8 clangExe = StrLit(EXE_CLANG);
			#else
			Str8 clangExe = StrLit(EXE_WSL_CLANG);
			mkdir(FOLDERNAME_LINUX, FOLDER_PERMISSIONS);
			chdir(FOLDERNAME_LINUX);
			cmd.rootDirPath = StrLit("../..");
			#endif
			
			RunCliProgramAndExitOnFailure(clangExe, &cmd, StrLit("Failed to build " FILENAME_PIGGEN "!"));
			AssertFileExist(StrLit(FILENAME_PIGGEN), true);
			PrintLine("[Built %s for Linux!]", FILENAME_PIGGEN);
			
			#if !BUILDING_ON_LINUX
			chdir("..");
			#endif
		}
		if (BUILD_OSX)
		{
			PrintLine("\n[Building %s for OSX...]", FILENAME_PIGGEN);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/piggen/piggen_main.c");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, FILENAME_PIGGEN);
			AddArgList(&cmd, &clang_CommonFlags);
			AddArgList(&cmd, &clang_LinuxFlags); //TODO: If this works, we should rename this list
			AddArgList(&cmd, &clang_LinuxCommonLibraries); //TODO: If this works, we should rename this list
			
			RunCliProgramAndExitOnFailure(StrLit(EXE_CLANG), &cmd, StrLit("Failed to build " FILENAME_PIGGEN "!"));
			AssertFileExist(StrLit(FILENAME_PIGGEN), true);
			PrintLine("[Built %s for OSX!]", FILENAME_PIGGEN);
		}
	}
	
	// +--------------------------------------------------------------+
	// |                        Run piggen.exe                        |
	// +--------------------------------------------------------------+
	if (RUN_PIGGEN)
	{
		PrintLine("\n[%s]", RUNNABLE_FILENAME_PIGGEN);
		
		#define PIGGEN_OUTPUT_FOLDER "-o=\"[VAL]\""
		#define PIGGEN_EXCLUDE_FOLDER "-e=\"[VAL]\""
		
		CliArgList cmd = ZEROED;
		AddArgNt(&cmd, CLI_QUOTED_ARG, "..");
		AddArgNt(&cmd, PIGGEN_OUTPUT_FOLDER, FOLDERNAME_GENERATED_CODE "/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/base/base_defines_check.h");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/piggen/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/tools/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/third_party/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/wasm/std/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/.git/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/_build/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/_scripts/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/_media/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/_template/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/_fuzzing/");
		
		RunCliProgramAndExitOnFailure(StrLit(EXEC_PROGRAM_IN_FOLDER_PREFIX RUNNABLE_FILENAME_PIGGEN), &cmd, StrLit(RUNNABLE_FILENAME_PIGGEN " Failed!"));
	}
	
	// +--------------------------------------------------------------+
	// |                        Build Shaders                         |
	// +--------------------------------------------------------------+
	FindShadersContext findContext = ZEROED;
	CliArgList cl_ShaderObjects = ZEROED;
	CliArgList clang_ShaderObjects = ZEROED;
	if (BUILD_SHADERS || BUILD_WITH_SOKOL_GFX)
	{
		const char* ignoreList[] = { ".git", "_template", "third_party", "_build" };
		findContext.ignoreListLength = ArrayCount(ignoreList);
		findContext.ignoreList = (Str8*)malloc(sizeof(Str8) * findContext.ignoreListLength);
		for (uxx iIndex = 0; iIndex < findContext.ignoreListLength; iIndex++)
		{
			findContext.ignoreList[iIndex] = NewStr8Nt(ignoreList[iIndex]);
		}
		
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
				Str8 oPathWithFolder = BUILDING_ON_LINUX ? CopyStr8(oPath, false) : JoinStrings2(StrLit(FOLDERNAME_LINUX "/"), oPath, false);
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
		if (BUILD_WINDOWS) { InitializeMsvcIf(StrLit(".."), &isMsvcInitialized); }
		
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
			Str8 realHeaderPath = StrReplace(headerPath, StrLit("[ROOT]"), StrLit(".."), false);
			Str8 realShaderPath = StrReplace(shaderPath, StrLit("[ROOT]"), StrLit(".."), false);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, SHDC_FORMAT, "sokol_impl");
			AddArgNt(&cmd, SHDC_ERROR_FORMAT, "msvc");
			// AddArg(&cmd, SHDC_REFLECTION);
			AddArgNt(&cmd, SHDC_SHADER_LANGUAGES, "hlsl5:glsl430:metal_macos");
			AddArgStr(&cmd, SHDC_INPUT, shaderPath);
			AddArgStr(&cmd, SHDC_OUTPUT, headerPath);
			
			PrintLine("Generating \"%.*s\"...", realHeaderPath.length, realHeaderPath.chars);
			Str8 shdcExe = JoinStrings2(StrLit("../"), StrLit(EXE_SHDC), false);
			FixPathSlashes(shdcExe, PATH_SEP_CHAR);
			RunCliProgramAndExitOnFailure(shdcExe, &cmd, StrLit(EXE_SHDC_NAME " failed on TODO:!"));
			AssertFileExist(realHeaderPath, true);
			
			ScrapeShaderHeaderFileAndAddExtraInfo(realHeaderPath, realShaderPath);
			free(realHeaderPath.chars);
			free(realShaderPath.chars);
		}
		
		//Then compile each header file to an .o/.obj file
		for (uxx sIndex = 0; sIndex < findContext.shaderPaths.length; sIndex++)
		{
			Str8 headerPath = findContext.headerPaths.strings[sIndex];
			Str8 sourcePath = findContext.sourcePaths.strings[sIndex];
			Str8 headerFileName = GetFileNamePart(headerPath, true);
			Str8 headerDirectory = GetDirectoryPart(headerPath, true);
			Str8 realSourcePath = StrReplace(sourcePath, StrLit("[ROOT]"), StrLit(".."), false);
			
			//We need a .c file that #includes shader_include.h (which defines SOKOL_SHDC_IMPL) and then the shader header file
			Str8 sourceFileContents = JoinStrings3(
				StrLit("\n#include \"shader_include.h\"\n\n#include \""),
				headerFileName,
				StrLit("\"\n"),
				false
			);
			PrintLine("Generating \"%.*s\"...", realSourcePath.length, realSourcePath.chars);
			CreateAndWriteFile(realSourcePath, sourceFileContents, true);
			
			if (BUILD_WINDOWS)
			{
				Str8 objPath = findContext.objPaths.strings[sIndex];
				
				CliArgList cmd = ZEROED;
				AddArg(&cmd, CL_COMPILE);
				AddArgStr(&cmd, CLI_QUOTED_ARG, sourcePath);
				AddArgStr(&cmd, CL_OBJ_FILE, objPath);
				AddArgStr(&cmd, CL_INCLUDE_DIR, headerDirectory);
				AddArgList(&cmd, &cl_CommonFlags);
				AddArgList(&cmd, &cl_LangCFlags);
				
				RunCliProgramAndExitOnFailure(StrLit(EXE_MSVC_CL), &cmd, StrLit("Failed to build TODO: for Windows!"));
				AssertFileExist(objPath, true);
			}
			if (BUILD_LINUX)
			{
				Str8 oPath = findContext.oPaths.strings[sIndex];
				//TODO: The path we store in the findContext needs to have [ROOT] at the beginning somehow so we can get rid of this logic
				// Str8 fixedSourcePath = BUILDING_ON_LINUX ? CopyStr8(sourcePath, false) : JoinStrings2(StrLit("../"), sourcePath, false);
				// FixPathSlashes(fixedSourcePath, '/');
				// Str8 fixedHeaderDirectory = BUILDING_ON_LINUX ? CopyStr8(headerDirectory, false) : JoinStrings2(StrLit("../"), headerDirectory, false);
				// FixPathSlashes(fixedHeaderDirectory, '/');
				
				CliArgList cmd = ZEROED;
				cmd.pathSepChar = '/';
				AddArg(&cmd, CLANG_COMPILE);
				AddArgStr(&cmd, CLI_QUOTED_ARG, sourcePath);
				AddArgStr(&cmd, CLANG_OUTPUT_FILE, oPath);
				AddArgStr(&cmd, CLANG_INCLUDE_DIR, headerDirectory);
				AddArgList(&cmd, &clang_CommonFlags);
				AddArgList(&cmd, &clang_LinuxFlags);
				
				#if BUILDING_ON_LINUX
				Str8 clangExe = StrLit(EXE_CLANG);
				#else
				Str8 clangExe = StrLit(EXE_WSL_CLANG);
				mkdir(FOLDERNAME_LINUX, FOLDER_PERMISSIONS);
				chdir(FOLDERNAME_LINUX);
				cmd.rootDirPath = StrLit("../..");
				#endif
				
				RunCliProgramAndExitOnFailure(clangExe, &cmd, StrLit("Failed to build TODO: for Linux!"));
				AssertFileExist(oPath, true);
				
				#if !BUILDING_ON_LINUX
				chdir("..");
				#endif
			}
		}
		
		FreeStrArray(&findContext.shaderPaths);
		FreeStrArray(&findContext.headerPaths);
		FreeStrArray(&findContext.sourcePaths);
		FreeStrArray(&findContext.objPaths);
		FreeStrArray(&findContext.oPaths);
	}
	
	// +--------------------------------------------------------------+
	// |                       Build tracy.dll                        |
	// +--------------------------------------------------------------+
	if (PROFILING_ENABLED && !BUILD_TRACY_DLL && BUILD_WINDOWS && !DoesFileExist(StrLit(FILENAME_TRACY_DLL))) { PrintLine("Building %s because it's missing", FILENAME_TRACY_DLL); BUILD_TRACY_DLL = true; }
	if (PROFILING_ENABLED && !BUILD_TRACY_DLL && BUILD_LINUX && !DoesFileExist(StrLit(FILENAME_TRACY_SO))) { PrintLine("Building %s because it's missing", FILENAME_TRACY_SO); BUILD_TRACY_DLL = true; }
	if (BUILD_TRACY_DLL)
	{
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit(".."), &isMsvcInitialized);
			PrintLine("[Building %s for Windows...]", FILENAME_TRACY_DLL);
			
			CliArgList cmd = ZEROED;
			// AddArg(&cmd, CL_COMPILE);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/third_party/tracy/TracyClient.cpp");
			AddArgNt(&cmd, CL_INCLUDE_DIR, "[ROOT]/third_party/tracy");
			AddArgNt(&cmd, CL_BINARY_FILE, FILENAME_TRACY_DLL);
			AddArgNt(&cmd, CL_DEFINE, "TRACY_ENABLE");
			AddArgNt(&cmd, CL_DEFINE, "TRACY_EXPORTS");
			AddArgNt(&cmd, CL_CONFIGURE_EXCEPTION_HANDLING, "s"); //enable stack-unwinding
			AddArgNt(&cmd, CL_CONFIGURE_EXCEPTION_HANDLING, "c"); //extern "C" functions don't through exceptions
			AddArgList(&cmd, &cl_CommonFlags);
			AddArgList(&cmd, &cl_LangCppFlags);
			if (DUMP_ASSEMBLY) { AddArgNt(&cmd, CL_ASSEMB_LISTING_FILE, "tracy.asm"); }
			AddArg(&cmd, CL_LINK);
			AddArg(&cmd, LINK_BUILD_DLL);
			AddArgList(&cmd, &cl_CommonLinkerFlags);
			
			RunCliProgramAndExitOnFailure(StrLit(EXE_MSVC_CL), &cmd, StrLit("Failed to build " FILENAME_TRACY_DLL "!"));
			AssertFileExist(StrLit(FILENAME_TRACY_DLL), true);
			PrintLine("[Built %s for Windows!]", FILENAME_TRACY_DLL);
		}
		if (BUILD_LINUX)
		{
			//TODO: Implement Linux version!
		}
	}
	if (PROFILING_ENABLED) { AddArgNt(&cl_PigCoreLibraries, CLI_QUOTED_ARG, FILENAME_TRACY_LIB); }
	
	// +--------------------------------------------------------------+
	// |                       Build imgui.obj                        |
	// +--------------------------------------------------------------+
	if (BUILD_WITH_IMGUI && !BUILD_IMGUI_OBJ && BUILD_WINDOWS && !DoesFileExist(StrLit(FILENAME_IMGUI_OBJ))) { PrintLine("Building %s because it's missing", FILENAME_IMGUI_OBJ); BUILD_IMGUI_OBJ = true; }
	if (BUILD_WITH_IMGUI && !BUILD_IMGUI_OBJ && BUILD_LINUX && !DoesFileExist(StrLit(FILENAME_IMGUI_O))) { PrintLine("Building %s because it's missing", FILENAME_IMGUI_O); BUILD_IMGUI_OBJ = true; }
	if (BUILD_IMGUI_OBJ)
	{
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit(".."), &isMsvcInitialized);
			PrintLine("[Building %s for Windows...]", FILENAME_IMGUI_OBJ);
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, CL_COMPILE);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]\\ui\\ui_imgui_main.cpp");
			AddArgNt(&cmd, CL_INCLUDE_DIR, "[ROOT]\\third_party\\imgui");
			AddArgNt(&cmd, CL_OBJ_FILE, FILENAME_IMGUI_OBJ);
			AddArgList(&cmd, &cl_CommonFlags);
			AddArgList(&cmd, &cl_LangCppFlags);
			if (DUMP_ASSEMBLY) { AddArgNt(&cmd, CL_ASSEMB_LISTING_FILE, "imgui.asm"); }
			AddArg(&cmd, CL_LINK);
			AddArgList(&cmd, &cl_CommonLinkerFlags);
			
			RunCliProgramAndExitOnFailure(StrLit(EXE_MSVC_CL), &cmd, StrLit("Failed to build " FILENAME_IMGUI_OBJ "!"));
			AssertFileExist(StrLit(FILENAME_IMGUI_OBJ), true);
			PrintLine("[Built %s for Windows!]", FILENAME_IMGUI_OBJ);
		}
		if (BUILD_LINUX)
		{
			//TODO: Implement Linux version!
		}
	}
	if (BUILD_WITH_IMGUI) { AddArgNt(&cl_PigCoreLibraries, CLI_QUOTED_ARG, FILENAME_IMGUI_OBJ); }
	
	// +--------------------------------------------------------------+
	// |                     Build physx_capi.obj                     |
	// +--------------------------------------------------------------+
	if (BUILD_WITH_PHYSX && !BUILD_PHYSX_OBJ && BUILD_WINDOWS && !DoesFileExist(StrLit(FILENAME_PHYSX_OBJ))) { PrintLine("Building %s because it's missing", FILENAME_PHYSX_OBJ); BUILD_PHYSX_OBJ = true; }
	if (BUILD_WITH_PHYSX && !BUILD_PHYSX_OBJ && BUILD_LINUX && !DoesFileExist(StrLit(FILENAME_PHYSX_O))) { PrintLine("Building %s because it's missing", FILENAME_PHYSX_O); BUILD_PHYSX_OBJ = true; }
	if (BUILD_PHYSX_OBJ)
	{
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit(".."), &isMsvcInitialized);
			PrintLine("[Building %s for Windows...]", FILENAME_PHYSX_OBJ);
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, CL_COMPILE);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]\\phys\\phys_physx_capi_main.cpp");
			AddArgNt(&cmd, CL_INCLUDE_DIR, "[ROOT]\\third_party\\physx");
			AddArgNt(&cmd, CL_OBJ_FILE, FILENAME_PHYSX_OBJ);
			AddArgList(&cmd, &cl_CommonFlags);
			AddArgList(&cmd, &cl_LangCppFlags);
			if (DUMP_ASSEMBLY) { AddArgNt(&cmd, CL_ASSEMB_LISTING_FILE, "physx.asm"); }
			AddArg(&cmd, CL_LINK);
			AddArgList(&cmd, &cl_CommonLinkerFlags);
			
			RunCliProgramAndExitOnFailure(StrLit(EXE_MSVC_CL), &cmd, StrLit("Failed to build " FILENAME_PHYSX_OBJ "!"));
			AssertFileExist(StrLit(FILENAME_PHYSX_OBJ), true);
			PrintLine("[Built %s for Windows!]", FILENAME_PHYSX_OBJ);
		}
		if (BUILD_LINUX)
		{
			//TODO: Implement Linux version!
		}
	}
	if (BUILD_WITH_PHYSX) { AddArgNt(&cl_PigCoreLibraries, CLI_QUOTED_ARG, FILENAME_PHYSX_OBJ); }
	
	// +--------------------------------------------------------------+
	// |                      Build pig_core.dll                      |
	// +--------------------------------------------------------------+
	if (BUILD_PIG_CORE_DLL)
	{
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit(".."), &isMsvcInitialized);
			PrintLine("\n[Building %s for Windows...]", FILENAME_PIG_CORE_DLL);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]\\dll\\dll_main.c");
			AddArgNt(&cmd, CL_BINARY_FILE, FILENAME_PIG_CORE_DLL);
			AddArgList(&cmd, &cl_CommonFlags);
			AddArgList(&cmd, &cl_LangCFlags);
			if (DUMP_ASSEMBLY) { AddArgNt(&cmd, CL_ASSEMB_LISTING_FILE, "pig_core.asm"); }
			AddArg(&cmd, CL_LINK);
			AddArg(&cmd, LINK_BUILD_DLL);
			AddArgList(&cmd, &cl_CommonLinkerFlags);
			AddArgList(&cmd, &cl_PigCoreLibraries);
			
			RunCliProgramAndExitOnFailure(StrLit(EXE_MSVC_CL), &cmd, StrLit("Failed to build " FILENAME_PIG_CORE_DLL "!"));
			AssertFileExist(StrLit(FILENAME_PIG_CORE_DLL), true);
			PrintLine("[Built %s for Windows!]", FILENAME_PIG_CORE_DLL);
		}
		if (BUILD_LINUX)
		{
			PrintLine("\n[Building %s for Linux...]", FILENAME_PIG_CORE_SO);
			
			CliArgList cmd = ZEROED;
			cmd.pathSepChar = '/';
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/dll/dll_main.c");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, FILENAME_PIG_CORE_SO);
			AddArg(&cmd, CLANG_BUILD_SHARED_LIB);
			AddArg(&cmd, CLANG_fPIC);
			AddArgList(&cmd, &clang_CommonFlags);
			AddArgList(&cmd, &clang_LinuxFlags);
			AddArgList(&cmd, &clang_LinuxCommonLibraries);
			AddArgList(&cmd, &clang_PigCoreLibraries);
			
			#if BUILDING_ON_LINUX
			Str8 clangExe = StrLit(EXE_CLANG);
			#else
			Str8 clangExe = StrLit(EXE_WSL_CLANG);
			mkdir(FOLDERNAME_LINUX, FOLDER_PERMISSIONS);
			chdir(FOLDERNAME_LINUX);
			cmd.rootDirPath = StrLit("../..");
			#endif
			
			RunCliProgramAndExitOnFailure(clangExe, &cmd, StrLit("Failed to build " FILENAME_PIG_CORE_SO "!"));
			AssertFileExist(StrLit(FILENAME_PIG_CORE_SO), true);
			PrintLine("[Built %s for Linux!]", FILENAME_PIG_CORE_SO);
			
			#if !BUILDING_ON_LINUX
			chdir("..");
			#endif
		}
	}
	
	// +--------------------------------------------------------------+
	// |                       Build tests.exe                        |
	// +--------------------------------------------------------------+
	if (RUN_TESTS && !BUILD_TESTS && !DoesFileExist(StrLit(FILENAME_TESTS_EXE))) { PrintLine("Building %s because it's missing", FILENAME_TESTS_EXE); BUILD_TESTS = true; BUILD_WINDOWS = true; }
	if (BUILD_TESTS)
	{
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit(".."), &isMsvcInitialized);
			PrintLine("\n[Building %s for Windows...]", FILENAME_TESTS_EXE);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]\\tests\\tests_main.c");
			AddArgNt(&cmd, CL_BINARY_FILE, FILENAME_TESTS_EXE);
			AddArgList(&cmd, &cl_CommonFlags);
			AddArgList(&cmd, &cl_LangCFlags);
			if (DUMP_ASSEMBLY) { AddArgNt(&cmd, CL_ASSEMB_LISTING_FILE, "tests.asm"); }
			AddArg(&cmd, CL_LINK);
			AddArgList(&cmd, &cl_CommonLinkerFlags);
			AddArgList(&cmd, &cl_PigCoreLibraries);
			if (BUILD_WITH_SOKOL_GFX) { AddArgList(&cmd, &cl_ShaderObjects); }
			
			RunCliProgramAndExitOnFailure(StrLit(EXE_MSVC_CL), &cmd, StrLit("Failed to build " FILENAME_TESTS_EXE "!"));
			AssertFileExist(StrLit(FILENAME_TESTS_EXE), true);
			PrintLine("[Built %s for Windows!]", FILENAME_TESTS_EXE);
		}
		
		if (BUILD_LINUX)
		{
			PrintLine("\n[Building %s for Linux...]", FILENAME_TESTS);
			
			CliArgList cmd = ZEROED;
			cmd.pathSepChar = '/';
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/tests/tests_main.c");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, FILENAME_TESTS);
			AddArgList(&cmd, &clang_CommonFlags);
			AddArgList(&cmd, &clang_LinuxFlags);
			AddArgList(&cmd, &clang_LinuxCommonLibraries);
			AddArgList(&cmd, &clang_PigCoreLibraries);
			if (BUILD_WITH_SOKOL_GFX) { AddArgList(&cmd, &clang_ShaderObjects); }
			
			#if BUILDING_ON_LINUX
			Str8 clangExe = StrLit(EXE_CLANG);
			#else
			Str8 clangExe = StrLit(EXE_WSL_CLANG);
			mkdir(FOLDERNAME_LINUX, FOLDER_PERMISSIONS);
			chdir(FOLDERNAME_LINUX);
			cmd.rootDirPath = StrLit("../..");
			#endif
			
			RunCliProgramAndExitOnFailure(clangExe, &cmd, StrLit("Failed to build " FILENAME_TESTS "!"));
			AssertFileExist(StrLit(FILENAME_TESTS), true);
			PrintLine("[Built %s for Linux!]", FILENAME_TESTS);
			
			#if !BUILDING_ON_LINUX
			chdir("..");
			#endif
		}
		
		if (BUILD_OSX)
		{
			PrintLine("\n[Building %s for OSX...]", FILENAME_TESTS);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/tests/tests_main.c");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, FILENAME_TESTS);
			AddArgList(&cmd, &clang_CommonFlags);
			AddArgList(&cmd, &clang_LinuxFlags);
			AddArgList(&cmd, &clang_LinuxCommonLibraries);
			AddArgList(&cmd, &clang_PigCoreLibraries);
			if (BUILD_WITH_SOKOL_GFX) { AddArgList(&cmd, &clang_ShaderObjects); }
			
			RunCliProgramAndExitOnFailure(StrLit(EXE_CLANG), &cmd, StrLit("Failed to build " FILENAME_TESTS "!"));
			AssertFileExist(StrLit(FILENAME_TESTS), true);
			PrintLine("[Built %s for OSX!]", FILENAME_TESTS);
		}
		
		if (BUILD_WEB)
		{
			PrintLine("\n[Building %s for Web...]", FILENAME_APP_WASM);
			
			mkdir(FOLDERNAME_WEB, FOLDER_PERMISSIONS);
			chdir(FOLDERNAME_WEB);
			
			// TODO: del *.wasm > NUL 2> NUL
			// TODO: del *.wat > NUL 2> NUL
			// TODO: del *.css > NUL 2> NUL
			// TODO: del *.html > NUL 2> NUL
			// TODO: del *.js > NUL 2> NUL
			
			CliArgList cmd = ZEROED;
			cmd.rootDirPath = StrLit("../..");
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/tests/tests_main.c");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, USE_EMSCRIPTEN ? FILENAME_INDEX_HTML : FILENAME_APP_WASM);
			AddArgList(&cmd, &clang_CommonFlags);
			AddArgList(&cmd, &clang_WasmFlags);
			AddArgList(&cmd, &clang_WebFlags);
			
			RunCliProgramAndExitOnFailure(USE_EMSCRIPTEN ? StrLit(EXE_EMSCRIPTEN_COMPILER) : StrLit(EXE_CLANG), &cmd, StrLit("Failed to build " FILENAME_APP_WASM "!"));
			if (USE_EMSCRIPTEN)
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
			
			if (CONVERT_WASM_TO_WAT)
			{
				CliArgList convertCmd = ZEROED;
				AddArgNt(&convertCmd, CLI_QUOTED_ARG, USE_EMSCRIPTEN ? FILENAME_INDEX_WASM : FILENAME_APP_WASM);
				AddArgNt(&convertCmd, CLI_PIPE_OUTPUT_TO_FILE, USE_EMSCRIPTEN ? FILENAME_INDEX_WAT : FILENAME_APP_WAT);
				
				int convertStatusCode = RunCliProgram(StrLit("wasm2wat"), &convertCmd);
				if (convertStatusCode == 0)
				{
					AssertFileExist(USE_EMSCRIPTEN ? StrLit(FILENAME_INDEX_WAT) : StrLit(FILENAME_APP_WAT), true);
				}
				else
				{
					PrintLine_E("Failed to convert .wasm to .wat! Status Code: %d", convertStatusCode);
					exit(convertStatusCode);
				}
			}
			
			if (!USE_EMSCRIPTEN)
			{
				StrArray javascriptFiles = ZEROED;
				AddStr(&javascriptFiles, StrLit("../../wasm/wasm_globals.js"));
				AddStr(&javascriptFiles, StrLit("../../wasm/std/include/internal/wasm_std_js_api.js"));
				AddStr(&javascriptFiles, StrLit("../../wasm/wasm_app_js_api.js"));
				AddStr(&javascriptFiles, StrLit("../../wasm/wasm_main.js"));
				ConcatAllFilesIntoSingleFile(&javascriptFiles, StrLit("combined.js"));
				
				CopyFileToPath(StrLit("..\\..\\wasm\\wasm_app_style.css"), StrLit("main.css"));
				CopyFileToPath(StrLit("..\\..\\wasm\\wasm_app_index.html"), StrLit("index.html"));
			}
			
			chdir("..");
		}
		
		if (BUILD_ORCA)
		{
			PrintLine("\n[Building %s for Orca...]", FILENAME_MODULE_WASM);
			
			mkdir(FOLDERNAME_ORCA, FOLDER_PERMISSIONS);
			chdir(FOLDERNAME_ORCA);
			
			CliArgList cmd = ZEROED;
			cmd.rootDirPath = StrLit("../..");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, FILENAME_MODULE_WASM);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/tests/tests_main.c");
			AddArgList(&cmd, &clang_CommonFlags);
			AddArgList(&cmd, &clang_WasmFlags);
			AddArgList(&cmd, &clang_OrcaFlags);
			
			RunCliProgramAndExitOnFailure(StrLit(EXE_CLANG), &cmd, StrLit("Failed to build " FILENAME_MODULE_WASM "!"));
			AssertFileExist(StrLit(FILENAME_MODULE_WASM), true);
			PrintLine("[Built %s for Orca!]", FILENAME_MODULE_WASM);
			
			CliArgList bundleCmd = ZEROED;
			AddArg(&bundleCmd, "bundle");
			AddArgNt(&bundleCmd, "--name [VAL]", "tests");
			AddArg(&bundleCmd, FILENAME_MODULE_WASM);
			RunCliProgramAndExitOnFailure(StrLit("orca"), &bundleCmd, StrLit("Failed to bundle " FILENAME_MODULE_WASM "!"));
			PrintLine("[Bundled %s into \"tests\" app!]", FILENAME_MODULE_WASM);
			
			chdir("..");
		}
		
		if (BUILD_PLAYDATE_DEVICE)
		{
			PrintLine("\n[Building %s for Playdate...]", FILENAME_PDEX_ELF);
			
			CliArgList compileCmd = ZEROED;
			AddArg(&compileCmd, GCC_COMPILE);
			AddArgNt(&compileCmd, CLI_QUOTED_ARG, "[ROOT]/tests/tests_main.c");
			AddArgNt(&compileCmd, GCC_OUTPUT_FILE, FILENAME_TESTS_OBJ);
			AddArgList(&compileCmd, &gcc_PlaydateDeviceCommonFlags);
			AddArgList(&compileCmd, &gcc_PlaydateDeviceCompilerFlags);
			
			RunCliProgramAndExitOnFailure(StrLit(EXE_ARM_GCC), &compileCmd, StrLit("Failed to build " FILENAME_TESTS_OBJ "!"));
			AssertFileExist(StrLit(FILENAME_TESTS_OBJ), true);
			
			CliArgList linkCmd = ZEROED;
			AddArgNt(&linkCmd, CLI_QUOTED_ARG, FILENAME_TESTS_OBJ);
			AddArgNt(&linkCmd, GCC_OUTPUT_FILE, FILENAME_PDEX_ELF);
			AddArgList(&linkCmd, &gcc_PlaydateDeviceCommonFlags);
			AddArgList(&linkCmd, &gcc_PlaydateDeviceLinkerFlags);
			AddArgNt(&linkCmd, GCC_MAP_FILE, "tests.map");
			
			RunCliProgramAndExitOnFailure(StrLit(EXE_ARM_GCC), &linkCmd, StrLit("Failed to build " FILENAME_PDEX_ELF "!"));
			AssertFileExist(StrLit(FILENAME_PDEX_ELF), true);
			PrintLine("[Built %s for Playdate!]", FILENAME_PDEX_ELF);
			
			mkdir("playdate_data", FOLDER_PERMISSIONS);
			CopyFileToFolder(StrLit(FILENAME_PDEX_ELF), StrLit("playdate_data"));
		}
		
		if (BUILD_PLAYDATE_SIMULATOR)
		{
			PrintLine("\n[Building %s for Playdate Simulator...]", FILENAME_PDEX_DLL);
			
			CliArgList compileCmd = ZEROED;
			AddArg(&compileCmd, CL_COMPILE);
			AddArgNt(&compileCmd, CLI_QUOTED_ARG, "[ROOT]\\tests\\tests_main.c");
			AddArgNt(&compileCmd, CL_OBJ_FILE, FILENAME_TESTS_OBJ);
			AddArgList(&compileCmd, &cl_PlaydateSimulatorCompilerFlags);
			
			RunCliProgramAndExitOnFailure(StrLit(EXE_MSVC_CL), &compileCmd, StrLit("Failed to build " FILENAME_TESTS_OBJ "!"));
			AssertFileExist(StrLit(FILENAME_TESTS_OBJ), true);
			
			CliArgList linkCmd = ZEROED;
			AddArg(&linkCmd, LINK_BUILD_DLL);
			AddArgNt(&linkCmd, CLI_QUOTED_ARG, FILENAME_TESTS_OBJ);
			AddArgNt(&linkCmd, LINK_OUTPUT_FILE, FILENAME_PDEX_DLL);
			AddArgNt(&linkCmd, LINK_IMPORT_LIBRARY_FILE, "tests.lib"); //TODO: Do we actually need to generate this?
			AddArgNt(&linkCmd, LINK_DEBUG_INFO_FILE, "tests.pdb");
			AddArgList(&linkCmd, &link_PlaydateSimulatorLinkerFlags);
			AddArgList(&linkCmd, &link_PlaydateSimulatorLibraries);
			
			RunCliProgramAndExitOnFailure(StrLit(EXE_MSVC_LINK), &linkCmd, StrLit("Failed to build " FILENAME_PDEX_DLL "!"));
			AssertFileExist(StrLit(FILENAME_PDEX_DLL), true);
			PrintLine("[Built %s for Playdate Simulator!]", FILENAME_PDEX_DLL);
			
			mkdir("playdate_data", FOLDER_PERMISSIONS);
			CopyFileToFolder(StrLit(FILENAME_PDEX_DLL), StrLit("playdate_data"));
		}
		
		if (BUILD_PLAYDATE_DEVICE || BUILD_PLAYDATE_SIMULATOR)
		{
			CopyFileToFolder(StrLit("..\\pdxinfo"), StrLit("playdate_data"));
			
			CliArgList cmd = ZEROED;
			AddArgList(&cmd, &pdc_CommonFlags);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "playdate_data");
			AddArgNt(&cmd, CLI_QUOTED_ARG, FILENAME_TESTS_PDX);
			
			RunCliProgramAndExitOnFailure(StrLit("pdc"), &cmd, StrLit("Failed to package " FILENAME_TESTS_PDX "!"));
			AssertFileExist(StrLit(FILENAME_TESTS_PDX), true); //TODO: Is this going to work on a folder?
			PrintLine("[Packaged %s for Playdate!]", FILENAME_TESTS_PDX);
		}
	}
	
	// +--------------------------------------------------------------+
	// |                        Run tests.exe                         |
	// +--------------------------------------------------------------+
	if (RUN_TESTS)
	{
		#if BUILDING_ON_WINDOWS
		#define RUNNABLE_FILENAME_TESTS FILENAME_TESTS_EXE
		#else
		#define RUNNABLE_FILENAME_TESTS FILENAME_TESTS
		#endif
		PrintLine("\n[%s]", RUNNABLE_FILENAME_TESTS);
		CliArgList cmd = ZEROED;
		RunCliProgramAndExitOnFailure(StrLit(EXEC_PROGRAM_IN_FOLDER_PREFIX RUNNABLE_FILENAME_TESTS), &cmd, StrLit(RUNNABLE_FILENAME_TESTS " Exited With Error!"));
	}
	
	PrintLine("\n[%s Finished Successfully]", TOOL_EXE_NAME);
	return 0;
}
