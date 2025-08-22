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

//NOTE: We use miniz.h when BUNDLE_RESOURCES_ZIP is enabled
#define MINIZ_NO_STDIO //to disable all usage and any functions which rely on stdio for file I/O.
#define MINIZ_USE_UNALIGNED_LOADS_AND_STORES 1
#define MINIZ_LITTLE_ENDIAN                  1
#include "third_party/miniz/miniz.h"
#include "third_party/miniz/miniz.c"

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
#define FILENAME_PIG_CORE_LIB      "pig_core.lib"
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

typedef plex BundleResourcesContext BundleResourcesContext;
plex BundleResourcesContext
{
	mz_zip_archive zip;
	Str8 relativePath;
	StrArray resourcePaths;
	uxx uncompressedSize;
	uxx archiveAllocSize;
	uxx archiveSize;
	uint8_t* archivePntr;
};

// +==============================+
// |   BundleResourcesCallback    |
// +==============================+
// bool BundleResourcesCallback(Str8 path, bool isFolder, void* contextPntr)
RECURSIVE_DIR_WALK_CALLBACK_DEF(BundleResourcesCallback)
{
	BundleResourcesContext* context = (BundleResourcesContext*)contextPntr;
	if (!isFolder)
	{
		Str8 fileContents = ReadEntireFile(path);
		assert(StrExactStartsWith(path, context->relativePath));
		Str8 inZipPath = StrSliceFrom(path, context->relativePath.length);
		if (inZipPath.length > 0 && IS_SLASH(inZipPath.chars[0])) { inZipPath.length--; inZipPath.chars++; }
		Str8 inZipPathNt = CopyStr8(inZipPath, true);
		FixPathSlashes(inZipPathNt, '/');
		mz_bool addMemSuccess = mz_zip_writer_add_mem(&context->zip, inZipPathNt.chars, fileContents.bytes, (size_t)fileContents.length, (mz_uint)MZ_BEST_COMPRESSION);
		assert(addMemSuccess == MZ_TRUE);
		context->uncompressedSize += fileContents.length;
		AddStr(&context->resourcePaths, inZipPath);
		free(inZipPathNt.chars);
		free(fileContents.chars);
	}
	return true;
}
size_t ZipFileWriteCallback(void* contextPntr, mz_uint64 fileOffset, const void* bufferPntr, size_t numBytes)
{
	// PrintLine("ZipFileWriteCallback(%p, %llu, %p, %zu)", contextPntr, fileOffset, bufferPntr, numBytes);
	BundleResourcesContext* context = (BundleResourcesContext*)contextPntr;
	assert(context != nullptr);
	if (context->archiveAllocSize < fileOffset + numBytes)
	{
		uxx newAllocSize = context->archiveAllocSize;
		if (newAllocSize < 8) { newAllocSize = 8; }
		while (newAllocSize < fileOffset + numBytes) { newAllocSize *= 2; }
		void* newAllocPntr = malloc(newAllocSize);
		if (context->archiveSize > 0) { memcpy(newAllocPntr, context->archivePntr, context->archiveSize); }
		if (context->archivePntr != nullptr) { free(context->archivePntr); }
		context->archivePntr = newAllocPntr;
		context->archiveAllocSize = newAllocSize;
	}
	memcpy(&context->archivePntr[fileOffset], bufferPntr, numBytes);
	if (context->archiveSize < fileOffset + numBytes) { context->archiveSize = fileOffset + numBytes; }
	return numBytes;
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
	
	// Str8 PROJECT_READABLE_NAME = ExtractStrDefine(buildConfigContents, StrLit("PROJECT_READABLE_NAME"));
	// Str8 PROJECT_FOLDER_NAME = ExtractStrDefine(buildConfigContents, StrLit("PROJECT_FOLDER_NAME"));
	Str8 PROJECT_DLL_NAME = ExtractStrDefine(buildConfigContents, StrLit("PROJECT_DLL_NAME"));
	Str8 PROJECT_EXE_NAME = ExtractStrDefine(buildConfigContents, StrLit("PROJECT_EXE_NAME"));
	Str8 filenameAppDll   = JoinStrings2(PROJECT_DLL_NAME, StrLit(".dll"), true);
	Str8 filenameAppSo    = JoinStrings2(PROJECT_DLL_NAME, StrLit(".so"), true);
	Str8 filenameAppExe   = JoinStrings2(PROJECT_EXE_NAME, StrLit(".exe"), true);
	Str8 filenameApp      = JoinStrings2(PROJECT_EXE_NAME, StrLit(""), true);
	
	bool DEBUG_BUILD              = ExtractBoolDefine(buildConfigContents, StrLit("DEBUG_BUILD"));
	bool BUILD_INTO_SINGLE_UNIT   = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_INTO_SINGLE_UNIT"));
	bool PROFILING_ENABLED        = ExtractBoolDefine(buildConfigContents, StrLit("PROFILING_ENABLED"));
	bool BUILD_PIGGEN             = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_PIGGEN"));
	bool BUNDLE_RESOURCES_ZIP     = ExtractBoolDefine(buildConfigContents, StrLit("BUNDLE_RESOURCES_ZIP"));
	bool BUILD_SHADERS            = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_SHADERS"));
	bool RUN_PIGGEN               = ExtractBoolDefine(buildConfigContents, StrLit("RUN_PIGGEN"));
	bool BUILD_TRACY_DLL          = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_TRACY_DLL"));
	bool BUILD_IMGUI_OBJ          = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_IMGUI_OBJ"));
	bool BUILD_PHYSX_OBJ          = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_PHYSX_OBJ"));
	bool BUILD_PIG_CORE_DLL       = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_PIG_CORE_DLL"));
	bool BUILD_APP_EXE            = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_APP_EXE"));
	bool BUILD_APP_DLL            = ExtractBoolDefine(buildConfigContents, StrLit("BUILD_APP_DLL"));
	bool RUN_APP                  = ExtractBoolDefine(buildConfigContents, StrLit("RUN_APP"));
	bool COPY_TO_DATA_DIRECTORY   = ExtractBoolDefine(buildConfigContents, StrLit("COPY_TO_DATA_DIRECTORY"));
	bool DUMP_PREPROCESSOR        = ExtractBoolDefine(buildConfigContents, StrLit("DUMP_PREPROCESSOR"));
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
	if (BUILD_INTO_SINGLE_UNIT && BUILD_APP_DLL && !BUILD_APP_EXE)
	{
		WriteLine_E("BUILD_INTO_SINGLE_UNIT works with BUILD_APP_EXE but only BUILD_APP_DLL is enabled. Assuming we want BUILD_APP_EXE instead");
		BUILD_APP_DLL = false;
		BUILD_APP_EXE = true;
	}
	if (BUILD_INTO_SINGLE_UNIT && BUILD_APP_DLL)
	{
		WriteLine_E("BUILD_INTO_SINGLE_UNIT implies that BUILD_APP_DLL is unnecassary. Only BUILD_APP_EXE matters");
		BUILD_APP_DLL = false;
	}
	if (BUILD_INTO_SINGLE_UNIT && BUILD_APP_EXE && BUILD_PIG_CORE_DLL)
	{
		WriteLine_E("BUILD_INTO_SINGLE_UNIT implies that BUILD_PIG_CORE_DLL is unnecassary. Not building pig_core.dll/so");
		BUILD_PIG_CORE_DLL = false;
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
	CliArgList cl_CommonFlags                    = ZEROED; Fill_cl_CommonFlags(&cl_CommonFlags, DEBUG_BUILD, DUMP_PREPROCESSOR);
	CliArgList cl_LangCFlags                     = ZEROED; Fill_cl_LangCFlags(&cl_LangCFlags);
	CliArgList cl_LangCppFlags                   = ZEROED; Fill_cl_LangCppFlags(&cl_LangCppFlags);
	CliArgList clang_CommonFlags                 = ZEROED; Fill_clang_CommonFlags(&clang_CommonFlags, DEBUG_BUILD, DUMP_PREPROCESSOR);
	CliArgList clang_LinuxFlags                  = ZEROED; Fill_clang_LinuxFlags(&clang_LinuxFlags, DEBUG_BUILD);
	CliArgList cl_CommonLinkerFlags              = ZEROED; Fill_cl_CommonLinkerFlags(&cl_CommonLinkerFlags, DEBUG_BUILD);
	CliArgList clang_LinuxCommonLibraries        = ZEROED; Fill_clang_LinuxCommonLibraries(&clang_LinuxCommonLibraries, BUILD_WITH_SOKOL_APP);
	CliArgList cl_PigCoreLibraries               = ZEROED; Fill_cl_PigCoreLibraries(&cl_PigCoreLibraries, BUILD_WITH_RAYLIB, BUILD_WITH_BOX2D, BUILD_WITH_SDL, BUILD_WITH_OPENVR, BUILD_WITH_IMGUI, BUILD_WITH_PHYSX);
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
	
	AddArgNt(&cl_CommonFlags, CL_INCLUDE_DIR, "[ROOT]/app");
	AddArgNt(&cl_CommonFlags, CL_INCLUDE_DIR, "[ROOT]/core");
	AddArgNt(&cl_CommonLinkerFlags, LINK_LIBRARY_DIR, DEBUG_BUILD ? "[ROOT]/core/third_party/_lib_debug" : "[ROOT]/core/third_party/_lib_release");
	AddArgNt(&clang_CommonFlags, CLANG_INCLUDE_DIR, "[ROOT]/app");
	AddArgNt(&clang_CommonFlags, CLANG_INCLUDE_DIR, "[ROOT]/core");
	AddArgNt(&clang_CommonFlags, CLANG_LIBRARY_DIR, DEBUG_BUILD ? "[ROOT]/core/third_party/_lib_debug" : "[ROOT]/core/third_party/_lib_release");
	
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
			InitializeMsvcIf(StrLit("../core"), &isMsvcInitialized);
			PrintLine("\n[Building %s for Windows...]", FILENAME_PIGGEN_EXE);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/core/piggen/piggen_main.c");
			AddArgNt(&cmd, CL_BINARY_FILE, FILENAME_PIGGEN_EXE);
			AddArgList(&cmd, &cl_CommonFlags);
			AddArgList(&cmd, &cl_LangCFlags);
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
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/core/piggen/piggen_main.c");
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
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/core/piggen/piggen_main.c");
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
		
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/_build/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/_data/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/_traces/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/_media/");
		
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/.git/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/_build/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/_data/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/_media/");
		
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/base/base_defines_check.h");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/piggen/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/tools/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/third_party/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/wasm/std/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/.git/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/_build/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/_scripts/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/_media/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/_template/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/_fuzzing/");
		
		RunCliProgramAndExitOnFailure(StrLit(EXEC_PROGRAM_IN_FOLDER_PREFIX RUNNABLE_FILENAME_PIGGEN), &cmd, StrLit(RUNNABLE_FILENAME_PIGGEN " Failed!"));
	}
	
	// +--------------------------------------------------------------+
	// |                       Bundle Resources                       |
	// +--------------------------------------------------------------+
	if (BUNDLE_RESOURCES_ZIP)
	{
		#if 0
		//TODO: Move away from using python! This is the last script we depend on, currently
		CliArgList cmd = ZEROED;
		AddArgNt(&cmd, CLI_UNQUOTED_ARG, "[ROOT]/core/_scripts/pack_resources.py");
		AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/_data/resources");
		AddArgNt(&cmd, CLI_QUOTED_ARG, FILENAME_RESOURCES_ZIP);
		AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/app/resources_zip.h");
		AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/app/resources_zip.c");
		
		RunCliProgramAndExitOnFailure(StrLit("python"), &cmd, StrLit("pack_resources.py Failed!"));
		AssertFileExist(StrLit(FILENAME_RESOURCES_ZIP), true);
		#else
		BundleResourcesContext context = ZEROED;
		context.zip.m_pWrite = ZipFileWriteCallback;
		context.zip.m_pIO_opaque = &context;
		mz_bool initResult = mz_zip_writer_init(&context.zip, 0);
		if (initResult != MZ_TRUE) { PrintLine_E("zip error: %s", mz_zip_get_error_string(context.zip.m_last_error)); }
		assert(initResult == MZ_TRUE);
		context.relativePath = StrLit("../_data/resources");
		RecursiveDirWalk(StrLit("../_data/resources"), BundleResourcesCallback, &context);
		mz_bool finalizeResult = mz_zip_writer_finalize_archive(&context.zip);
		assert(finalizeResult == MZ_TRUE);
		mz_zip_writer_end(&context.zip);
		PrintLine("Found %u resource files, total %u bytes uncompressed, %u compressed (%.1f%%)", context.resourcePaths.length, context.uncompressedSize, context.archiveSize, ((float)context.archiveSize / (float)context.uncompressedSize) * 100.0);
		
		CreateAndWriteFile(StrLit("resources.zip"), NewStr8(context.archiveSize, context.archivePntr), false);
		
		//Create resources_zip.h
		{
			Str8 cFileContents = ZEROED;
			for (int pass = 0; pass < 2; pass++)
			{
				uxx fileSize = 0;
				
				TwoPassPrint(&cFileContents, &fileSize,
					"/*\n"
					"File:   resources_zip.h\n"
					"Author: WARNING: This file is generated by pig_build.exe! Any hand edits will be lost!\n"
					"*/\n\n"
					"#ifndef _RESOURCES_ZIP_H\n"
					"#define _RESOURCES_ZIP_H\n\n"
				);
				TwoPassPrint(&cFileContents, &fileSize, "u8 resources_zip_bytes[%u];\n\n", context.archiveSize);
				TwoPassPrint(&cFileContents, &fileSize, "#endif //_RESOURCES_ZIP_H\n");
				
				if (pass == 0)
				{
					cFileContents.length = fileSize;
					cFileContents.pntr = malloc(cFileContents.length+1);
					assert(cFileContents.pntr != nullptr);
				}
				else { assert(fileSize == cFileContents.length); cFileContents.chars[cFileContents.length] = '\0'; }
			}
			CreateAndWriteFile(StrLit("../app/resources_zip.h"), cFileContents, true);
			free(cFileContents.chars);
		}
		
		//Create resources_zip.c
		{
			Str8 cFileContents = ZEROED;
			for (int pass = 0; pass < 2; pass++)
			{
				uxx fileSize = 0;
				
				TwoPassPrint(&cFileContents, &fileSize, "// This file is generated by pig_build.exe! Any hand edits will be lost!\n\n");
				TwoPassPrint(&cFileContents, &fileSize, "// Archive Contents (%u file%s, %u bytes uncompressed):\n", context.resourcePaths.length, (context.resourcePaths.length == 1) ? "" : "s", context.uncompressedSize);
				for (uxx rIndex = 0; rIndex < context.resourcePaths.length; rIndex++)
				{
					TwoPassPrint(&cFileContents, &fileSize, "//\t%.*s\n", context.resourcePaths.strings[rIndex].length, context.resourcePaths.strings[rIndex].chars);
				}
				TwoPassPrint(&cFileContents, &fileSize, "\nu8 resources_zip_bytes[%u] = {\n\t", context.archiveSize);
				for (uxx bIndex = 0; bIndex < context.archiveSize; bIndex++)
				{
					if (bIndex > 0) { TwoPassPrint(&cFileContents, &fileSize, ",%s", (bIndex%32) == 0 ? "\n\t" : " "); }
					TwoPassPrint(&cFileContents, &fileSize, "0x%02X", context.archivePntr[bIndex]);
				}
				TwoPassPrint(&cFileContents, &fileSize, "\n};\n");
				
				if (pass == 0)
				{
					cFileContents.length = fileSize;
					cFileContents.pntr = malloc(cFileContents.length+1);
					assert(cFileContents.pntr != nullptr);
				}
				else { assert(fileSize == cFileContents.length); cFileContents.chars[cFileContents.length] = '\0'; }
			}
			CreateAndWriteFile(StrLit("../app/resources_zip.c"), cFileContents, true);
			free(cFileContents.chars);
		}
		
		#endif
	}
	
	// +--------------------------------------------------------------+
	// |                        Build Shaders                         |
	// +--------------------------------------------------------------+
	FindShadersContext findContext = ZEROED;
	CliArgList cl_ShaderObjects = ZEROED;
	CliArgList clang_ShaderObjects = ZEROED;
	if (BUILD_SHADERS || BUILD_WITH_SOKOL_GFX)
	{
		findContext.ignoreListLength = 0;
		findContext.ignoreList = nullptr;
		RecursiveDirWalk(StrLit("../app"), FindShaderFilesCallback, &findContext);
		
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
		if (BUILD_WINDOWS) { InitializeMsvcIf(StrLit("../core"), &isMsvcInitialized); }
		
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
			Str8 shdcExe = JoinStrings2(StrLit("../core/"), StrLit(EXE_SHDC), false);
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
				StrLit("\n#include \"build_config.h\"\n\n#include \"shader_include.h\"\n\n#include \""),
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
			InitializeMsvcIf(StrLit("../core"), &isMsvcInitialized);
			PrintLine("[Building %s for Windows...]", FILENAME_TRACY_DLL);
			
			CliArgList cmd = ZEROED;
			// AddArg(&cmd, CL_COMPILE);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/core/third_party/tracy/TracyClient.cpp");
			AddArgNt(&cmd, CL_INCLUDE_DIR, "[ROOT]/core/third_party/tracy");
			AddArgNt(&cmd, CL_BINARY_FILE, FILENAME_TRACY_DLL);
			AddArgNt(&cmd, CL_DEFINE, "TRACY_ENABLE");
			AddArgNt(&cmd, CL_DEFINE, "TRACY_EXPORTS");
			AddArgNt(&cmd, CL_CONFIGURE_EXCEPTION_HANDLING, "s"); //enable stack-unwinding
			AddArgNt(&cmd, CL_CONFIGURE_EXCEPTION_HANDLING, "c"); //extern "C" functions don't through exceptions
			AddArgList(&cmd, &cl_CommonFlags);
			AddArgList(&cmd, &cl_LangCppFlags);
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
			InitializeMsvcIf(StrLit("../core"), &isMsvcInitialized);
			PrintLine("[Building %s for Windows...]", FILENAME_IMGUI_OBJ);
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, CL_COMPILE);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/core/ui/ui_imgui_main.cpp");
			AddArgNt(&cmd, CL_INCLUDE_DIR, "[ROOT]/core/third_party/imgui");
			AddArgNt(&cmd, CL_OBJ_FILE, FILENAME_IMGUI_OBJ);
			AddArgList(&cmd, &cl_CommonFlags);
			AddArgList(&cmd, &cl_LangCppFlags);
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
			InitializeMsvcIf(StrLit("../core"), &isMsvcInitialized);
			PrintLine("[Building %s for Windows...]", FILENAME_PHYSX_OBJ);
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, CL_COMPILE);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/core/phys/phys_physx_capi_main.cpp");
			AddArgNt(&cmd, CL_INCLUDE_DIR, "[ROOT]/core/third_party/physx");
			AddArgNt(&cmd, CL_OBJ_FILE, FILENAME_PHYSX_OBJ);
			AddArgList(&cmd, &cl_CommonFlags);
			AddArgList(&cmd, &cl_LangCppFlags);
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
			InitializeMsvcIf(StrLit("../core"), &isMsvcInitialized);
			PrintLine("\n[Building %s for Windows...]", FILENAME_PIG_CORE_DLL);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/core/dll/dll_main.c");
			AddArgNt(&cmd, CL_BINARY_FILE, FILENAME_PIG_CORE_DLL);
			AddArgNt(&cmd, CL_DEFINE, "PIG_CORE_DLL_INCLUDE_GFX_SYSTEM_GLOBAL=1");
			AddArgList(&cmd, &cl_CommonFlags);
			AddArgList(&cmd, &cl_LangCFlags);
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
			AddArgNt(&cmd, CLANG_DEFINE, "PIG_CORE_DLL_INCLUDE_GFX_SYSTEM_GLOBAL=1");
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
	// |                  Build PROJECT_EXE_NAME.exe                  |
	// +--------------------------------------------------------------+
	if (RUN_APP && !BUILD_APP_EXE && BUILDING_ON_WINDOWS && !DoesFileExist(filenameAppExe)) { PrintLine("Building %.*s because it's missing", filenameAppExe.length, filenameAppExe.chars); BUILD_APP_EXE = true; BUILD_WINDOWS = true; }
	if (RUN_APP && !BUILD_APP_EXE && !BUILDING_ON_WINDOWS && !DoesFileExist(filenameApp)) { PrintLine("Building %.*s because it's missing", filenameApp.length, filenameApp.chars); BUILD_APP_EXE = true; BUILD_LINUX = true; }
	if (BUILD_APP_EXE)
	{
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit("../core"), &isMsvcInitialized);
			PrintLine("\n[Building %.*s for Windows...]", filenameAppExe.length, filenameAppExe.chars);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/app/platform_main.c"); //NOTE: When BUILD_INTO_SINGLE_UNIT platform_main.c #includes app_main.c (and has PigCore implementations)
			AddArgStr(&cmd, CL_BINARY_FILE, filenameAppExe);
			AddArgList(&cmd, &cl_CommonFlags);
			AddArgList(&cmd, &cl_LangCFlags);
			AddArg(&cmd, CL_LINK);
			AddArgList(&cmd, &cl_CommonLinkerFlags);
			if (!BUILD_INTO_SINGLE_UNIT) { AddArgNt(&cmd, CLI_QUOTED_ARG, FILENAME_PIG_CORE_LIB); }
			if (BUILD_INTO_SINGLE_UNIT) { AddArgList(&cmd, &cl_ShaderObjects); }
			AddArgList(&cmd, &cl_PigCoreLibraries);
			
			Str8 errorStr = JoinStrings3(StrLit("Failed to build "), filenameAppExe, StrLit("!"), false);
			RunCliProgramAndExitOnFailure(StrLit(EXE_MSVC_CL), &cmd, errorStr);
			AssertFileExist(filenameAppExe, true);
			PrintLine("[Built %.*s for Windows!]", filenameAppExe.length, filenameAppExe.chars);
		}
		
		if (BUILD_LINUX)
		{
			PrintLine("\n[Building %.*s for Linux...]", filenameApp.length, filenameApp.chars);
			
			CliArgList cmd = ZEROED;
			cmd.pathSepChar = '/';
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/app/platform_main.c"); //NOTE: When BUILD_INTO_SINGLE_UNIT platform_main.c #includes app_main.c (and has PigCore implementations)
			AddArgStr(&cmd, CLANG_OUTPUT_FILE, filenameApp);
			AddArgList(&cmd, &clang_CommonFlags);
			AddArgList(&cmd, &clang_LinuxFlags);
			AddArgNt(&cmd, CLANG_RPATH_DIR, ".");
			if (!BUILD_INTO_SINGLE_UNIT) { AddArgNt(&cmd, CLI_QUOTED_ARG, FILENAME_PIG_CORE_SO); }
			if (BUILD_INTO_SINGLE_UNIT) { AddArgList(&cmd, &clang_ShaderObjects); }
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
			
			Str8 errorStr = JoinStrings3(StrLit("Failed to build "), filenameApp, StrLit("!"), false);
			RunCliProgramAndExitOnFailure(clangExe, &cmd, errorStr);
			AssertFileExist(filenameApp, true);
			PrintLine("[Built %.*s for Linux!]", filenameApp.length, filenameApp.chars);
			
			#if !BUILDING_ON_LINUX
			chdir("..");
			#endif
		}
		
		//TODO: Add OSX support
	}
	
	// +--------------------------------------------------------------+
	// |                  Build PROJECT_DLL_NAME.dll                  |
	// +--------------------------------------------------------------+
	if (RUN_APP && !BUILD_APP_DLL && BUILDING_ON_WINDOWS && !DoesFileExist(filenameAppDll)) { PrintLine("Building %.*s because it's missing", filenameAppDll.length, filenameAppDll.chars); BUILD_APP_DLL = true; BUILD_WINDOWS = true; }
	if (RUN_APP && !BUILD_APP_DLL && !BUILDING_ON_WINDOWS && !DoesFileExist(filenameAppSo)) { PrintLine("Building %.*s because it's missing", filenameAppSo.length, filenameAppSo.chars); BUILD_APP_DLL = true; BUILD_LINUX = true; }
	if (BUILD_APP_DLL)
	{
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit("../core"), &isMsvcInitialized);
			PrintLine("\n[Building %.*s for Windows...]", filenameAppDll.length, filenameAppDll.chars);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/app/app_main.c");
			AddArgStr(&cmd, CL_BINARY_FILE, filenameAppDll);
			AddArgList(&cmd, &cl_CommonFlags);
			AddArgList(&cmd, &cl_LangCFlags);
			AddArg(&cmd, CL_LINK);
			AddArg(&cmd, LINK_BUILD_DLL);
			AddArgList(&cmd, &cl_CommonLinkerFlags);
			AddArgNt(&cmd, CLI_QUOTED_ARG, FILENAME_PIG_CORE_LIB);
			AddArgList(&cmd, &cl_PigCoreLibraries);
			AddArgList(&cmd, &cl_ShaderObjects);
			
			Str8 errorStr = JoinStrings3(StrLit("Failed to build "), filenameAppDll, StrLit("!"), false);
			RunCliProgramAndExitOnFailure(StrLit(EXE_MSVC_CL), &cmd, errorStr);
			AssertFileExist(filenameAppDll, true);
			PrintLine("[Built %.*s for Windows!]", filenameAppDll.length, filenameAppDll.chars);
		}
		
		if (BUILD_LINUX)
		{
			PrintLine("\n[Building %.*s for Linux...]", filenameAppSo.length, filenameAppSo.chars);
			
			CliArgList cmd = ZEROED;
			cmd.pathSepChar = '/';
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/app/app_main.c");
			AddArgStr(&cmd, CLANG_OUTPUT_FILE, filenameAppSo);
			AddArg(&cmd, CLANG_BUILD_SHARED_LIB);
			AddArg(&cmd, CLANG_fPIC);
			AddArgList(&cmd, &clang_CommonFlags);
			AddArgList(&cmd, &clang_LinuxFlags);
			AddArgNt(&cmd, CLI_QUOTED_ARG, FILENAME_PIG_CORE_SO);
			AddArgList(&cmd, &clang_LinuxCommonLibraries);
			AddArgList(&cmd, &clang_PigCoreLibraries);
			AddArgList(&cmd, &clang_ShaderObjects);
			
			#if BUILDING_ON_LINUX
			Str8 clangExe = StrLit(EXE_CLANG);
			#else
			Str8 clangExe = StrLit(EXE_WSL_CLANG);
			mkdir(FOLDERNAME_LINUX, FOLDER_PERMISSIONS);
			chdir(FOLDERNAME_LINUX);
			cmd.rootDirPath = StrLit("../..");
			#endif
			
			Str8 errorStr = JoinStrings3(StrLit("Failed to build "), filenameAppSo, StrLit("!"), false);
			RunCliProgramAndExitOnFailure(clangExe, &cmd, errorStr);
			AssertFileExist(filenameAppSo, true);
			PrintLine("[Built %.*s for Linux!]", filenameAppSo.length, filenameAppSo.chars);
			
			#if !BUILDING_ON_LINUX
			chdir("..");
			#endif
		}
		
		//TODO: Add OSX support
	}
	
	// +--------------------------------------------------------------+
	// |                   Copy to _data Directory                    |
	// +--------------------------------------------------------------+
	if (COPY_TO_DATA_DIRECTORY)
	{
		Str8 dataFolder = StrLit("../_data");
		PrintLine("Copying files to %.*s...", dataFolder.length, dataFolder.chars);
		#if BUILDING_ON_WINDOWS
		if (BUILD_PIG_CORE_DLL) { CopyFileToFolder(StrLit(FILENAME_PIG_CORE_DLL), dataFolder); }
		if (BUILD_APP_EXE) { CopyFileToFolder(filenameAppExe, dataFolder); }
		if (BUILD_APP_DLL) { CopyFileToFolder(filenameAppDll, dataFolder); }
		if (BUILD_TRACY_DLL) { CopyFileToFolder(StrLit(FILENAME_TRACY_DLL), dataFolder); }
		#elif BUILDING_ON_LINUX
		if (BUILD_PIG_CORE_DLL) { CopyFileToFolder(StrLit(FILENAME_PIG_CORE_SO), dataFolder); }
		if (BUILD_APP_EXE) { CopyFileToFolder(filenameApp, dataFolder); }
		if (BUILD_APP_DLL) { CopyFileToFolder(filenameAppSo, dataFolder); }
		#endif
	}
	
	PrintLine("\n[%s Finished Successfully]", TOOL_EXE_NAME);
	return 0;
}
