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

//TODO: We should probably call _mkdir() (or _wmkdir()?) instead of mkdir() on Windows! https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/mkdir

#define PIG_BUILD_PRINT_SYS_CMDS 0
#include "pig_build.h"

#define BUILD_CONFIG_PATH       "../build_config.h"

#define FOLDERNAME_GENERATED_CODE  "gen"
#define FOLDERNAME_LINUX           "linux"
#define FOLDERNAME_OSX             "osx"
#define FOLDERNAME_WEB             "web"
#define FOLDERNAME_ANDROID         "android"
#define FOLDERNAME_ORCA            "orca"

#define FILENAME_PIGGEN_EXE            "piggen.exe"
#define FILENAME_PIGGEN                "piggen"
#define FILENAME_TRACY_DLL             "tracy.dll"
#define FILENAME_TRACY_LIB             "tracy.lib"
#define FILENAME_TRACY_SO              "tracy.so"
#define FILENAME_IMGUI_OBJ             "imgui.obj"
#define FILENAME_IMGUI_O               "imgui.o"
#define FILENAME_PHYSX_OBJ             "physx_capi.obj"
#define FILENAME_PHYSX_O               "physx_capi.o"
#define FILENAME_PIG_CORE_DLL          "pig_core.dll"
#define FILENAME_PIG_CORE_SO           "libpig_core.so"
#define FILENAME_TESTS                 "tests"
#define FILENAME_TESTS_EXE             "tests.exe"
#define FILENAME_TESTS_APK             "tests.apk"
#define FILENAME_TESTS_SO              "libtests.so"
#define FILENAME_TESTS_OBJ             "tests.obj"
#define FILENAME_ANDROID_RESOURCES_ZIP "resources.zip"
// #define FILENAME_DUMMY                 "dummy"
// #define FILENAME_DUMMY_EXE             "dummy.exe"
#define FILENAME_DUMMY_JAVA            "Dummy.java"
#define FILENAME_DUMMY_CLASS           "Dummy.class"
#define FILENAME_CLASSES_DEX           "classes.dex"
#define FILENAME_APP_WASM              "app.wasm"
#define FILENAME_APP_WAT               "app.wat"
#define FILENAME_INDEX_HTML            "index.html"
#define FILENAME_INDEX_WASM            "index.wasm"
#define FILENAME_INDEX_WAT             "index.wat"
#define FILENAME_MODULE_WASM           "module.wasm"
#define FILENAME_PDEX_ELF              "pdex.elf"
#define FILENAME_PDEX_DLL              "pdex.dll"
#define FILENAME_TESTS_PDX             "tests.pdx"

void PrintUsage()
{
	WriteLine_E("Usage: " BUILD_SCRIPT_EXE_NAME " [DEBUG_BUILD={1/0}] [BUILD_TESTS={1/0}] ...");
}

bool GetBoolConfig(const char* defineName, Str8 buildConfigContents, int argc, char** argv, StrArray* tagsArrayOut)
{
	Str8 defineNameStr = MakeStr8Nt(defineName);
	bool result = ExtractBoolDefine(buildConfigContents, defineNameStr);
	
	//Check for the define being passed on the command-line
	if (argc > 1)
	{
		for (int aIndex = 1; aIndex < argc; aIndex++)
		{
			Str8 argStr = MakeStr8Nt(argv[aIndex]);
			if (StrExactStartsWith(argStr, defineNameStr))
			{
				bool argValue = true;
				Str8 valuePart = StrSliceFrom(argStr, defineNameStr.length);
				valuePart = TrimWhitespace(valuePart);
				if (valuePart.length > 0)
				{
					if (StrExactStartsWith(valuePart, StrLit("=")))
					{
						valuePart = StrSliceFrom(valuePart, 1);
						valuePart = TrimWhitespace(valuePart);
						if (StrExactStartsWith(valuePart, StrLit("true")) || StrExactStartsWith(valuePart, StrLit("1")))
						{
							argValue = true;
						}
						else if (StrExactStartsWith(valuePart, StrLit("false")) || StrExactStartsWith(valuePart, StrLit("0")))
						{
							argValue = false;
						}
						else { continue; }
					}
					else { continue; }
				}
				result = argValue;
			}
		}
	}
	
	if (result)
	{
		AddStr(tagsArrayOut, defineNameStr);
	}
	
	return result;
}
//TODO: Add GetStrConfig to do ExtractStrDefine and check cmd-line args for config value

int main(int argc, char* argv[])
{
	RecompileIfNeeded();
	PrintLine("[" BUILD_SCRIPT_EXE_NAME "...]");
	
	bool isMsvcInitialized = WasMsvcDevBatchRun();
	bool isEmsdkInitialized = WasEmsdkEnvBatchRun();
	
	// +==============================+
	// |       Extract Defines        |
	// +==============================+
	StrArray buildConfigTags = ZEROED;
	Str8 buildConfigContents = ReadEntireFile(StrLit(BUILD_CONFIG_PATH));
	
	bool DEBUG_BUILD                       = GetBoolConfig("DEBUG_BUILD",                       buildConfigContents, argc, argv, &buildConfigTags);
	bool PROFILING_ENABLED                 = GetBoolConfig("PROFILING_ENABLED",                 buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_PIGGEN                      = GetBoolConfig("BUILD_PIGGEN",                      buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_SHADERS                     = GetBoolConfig("BUILD_SHADERS",                     buildConfigContents, argc, argv, &buildConfigTags);
	bool GENERATE_COMPILE_COMMANDS_FOR_LSP = GetBoolConfig("GENERATE_COMPILE_COMMANDS_FOR_LSP", buildConfigContents, argc, argv, &buildConfigTags);
	bool RUN_PIGGEN                        = GetBoolConfig("RUN_PIGGEN",                        buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_TRACY_DLL                   = GetBoolConfig("BUILD_TRACY_DLL",                   buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_IMGUI_OBJ                   = GetBoolConfig("BUILD_IMGUI_OBJ",                   buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_PHYSX_OBJ                   = GetBoolConfig("BUILD_PHYSX_OBJ",                   buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_PIG_CORE_DLL                = GetBoolConfig("BUILD_PIG_CORE_DLL",                buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_TESTS                       = GetBoolConfig("BUILD_TESTS",                       buildConfigContents, argc, argv, &buildConfigTags);
	bool RUN_TESTS                         = GetBoolConfig("RUN_TESTS",                         buildConfigContents, argc, argv, &buildConfigTags);
	bool INSTALL_TESTS_APK                 = GetBoolConfig("INSTALL_TESTS_APK",                 buildConfigContents, argc, argv, &buildConfigTags);
	bool GENERATE_PROTOBUF                 = GetBoolConfig("GENERATE_PROTOBUF",                 buildConfigContents, argc, argv, &buildConfigTags);
	bool DUMP_PREPROCESSOR                 = GetBoolConfig("DUMP_PREPROCESSOR",                 buildConfigContents, argc, argv, &buildConfigTags);
	bool DUMP_ASSEMBLY                     = GetBoolConfig("DUMP_ASSEMBLY",                     buildConfigContents, argc, argv, &buildConfigTags);
	bool CONVERT_WASM_TO_WAT               = GetBoolConfig("CONVERT_WASM_TO_WAT",               buildConfigContents, argc, argv, &buildConfigTags);
	bool USE_EMSCRIPTEN                    = GetBoolConfig("USE_EMSCRIPTEN",                    buildConfigContents, argc, argv, &buildConfigTags);
	// bool ENABLE_AUTO_PROFILE               = GetBoolConfig("ENABLE_AUTO_PROFILE",            buildConfigContents, argc, argv, &buildConfigTags);
	// bool RUN_FUZZER                        = GetBoolConfig("RUN_FUZZER",                     buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_WINDOWS                     = GetBoolConfig("BUILD_WINDOWS",                     buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_LINUX                       = GetBoolConfig("BUILD_LINUX",                       buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_OSX                         = GetBoolConfig("BUILD_OSX",                         buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_WEB                         = GetBoolConfig("BUILD_WEB",                         buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_ANDROID                     = GetBoolConfig("BUILD_ANDROID",                     buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_ANDROID_APK                 = GetBoolConfig("BUILD_ANDROID_APK",                 buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_ORCA                        = GetBoolConfig("BUILD_ORCA",                        buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_PLAYDATE_DEVICE             = GetBoolConfig("BUILD_PLAYDATE_DEVICE",             buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_PLAYDATE_SIMULATOR          = GetBoolConfig("BUILD_PLAYDATE_SIMULATOR",          buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_WITH_RAYLIB                 = GetBoolConfig("BUILD_WITH_RAYLIB",                 buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_WITH_BOX2D                  = GetBoolConfig("BUILD_WITH_BOX2D",                  buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_WITH_SOKOL_GFX              = GetBoolConfig("BUILD_WITH_SOKOL_GFX",              buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_WITH_SOKOL_APP              = GetBoolConfig("BUILD_WITH_SOKOL_APP",              buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_WITH_SDL                    = GetBoolConfig("BUILD_WITH_SDL",                    buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_WITH_OPENVR                 = GetBoolConfig("BUILD_WITH_OPENVR",                 buildConfigContents, argc, argv, &buildConfigTags);
	// bool BUILD_WITH_CLAY                   = GetBoolConfig("BUILD_WITH_CLAY",                buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_WITH_IMGUI                  = GetBoolConfig("BUILD_WITH_IMGUI",                  buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_WITH_PHYSX                  = GetBoolConfig("BUILD_WITH_PHYSX",                  buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_WITH_HTTP                   = GetBoolConfig("BUILD_WITH_HTTP",                   buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_WITH_PROTOBUF               = GetBoolConfig("BUILD_WITH_PROTOBUF",               buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_WITH_FREETYPE               = GetBoolConfig("BUILD_WITH_FREETYPE",               buildConfigContents, argc, argv, &buildConfigTags);
	bool BUILD_WITH_GTK                    = GetBoolConfig("BUILD_WITH_GTK",                    buildConfigContents, argc, argv, &buildConfigTags);
	
	Str8 ANDROID_SIGNING_KEY_PATH = CopyStr8(ExtractStrDefine(buildConfigContents, StrLit("ANDROID_SIGNING_KEY_PATH")), false);
	Str8 ANDROID_SIGNING_PASSWORD = ZEROED;
	if (TryExtractDefineFrom(buildConfigContents, StrLit("ANDROID_SIGNING_PASSWORD"), &ANDROID_SIGNING_PASSWORD)) { ANDROID_SIGNING_PASSWORD = CopyStr8(ANDROID_SIGNING_PASSWORD, false); }
	Str8 ANDROID_SIGNING_PASS_PATH = ZEROED;
	if (TryExtractDefineFrom(buildConfigContents, StrLit("ANDROID_SIGNING_PASS_PATH"), &ANDROID_SIGNING_PASS_PATH)) { ANDROID_SIGNING_PASS_PATH = CopyStr8(ANDROID_SIGNING_PASS_PATH, false); }
	Str8 ANDROID_NDK_VERSION = CopyStr8(ExtractStrDefine(buildConfigContents, StrLit("ANDROID_NDK_VERSION")), false);
	Str8 ANDROID_PLATFORM_FOLDERNAME = CopyStr8(ExtractStrDefine(buildConfigContents, StrLit("ANDROID_PLATFORM_FOLDERNAME")), false);
	Str8 ANDROID_BUILD_TOOLS_VERSION = CopyStr8(ExtractStrDefine(buildConfigContents, StrLit("ANDROID_BUILD_TOOLS_VERSION")), false);
	Str8 ANDROID_ACTIVITY_PATH = CopyStr8(ExtractStrDefine(buildConfigContents, StrLit("ANDROID_ACTIVITY_PATH")), false);
	
	free(buildConfigContents.chars);
	
	// +==============================+
	// | Parse Command-Line Arguments |
	// +==============================+
	if (argc > 1)
	{
		PrintLine("Got %d argument%s", argc-1, (argc-1 == 1) ? "" : "s");
		for (int aIndex = 1; aIndex < argc; aIndex++)
		{
			PrintLine("Arg[%d]: %s", aIndex-1, argv[aIndex]);
			//TODO: We should parse these arguments and use them as overrides to the #defines we loaded above!
		}
		PrintUsage(); //TODO: Only print this out if we find an argument we don't understand
		PrintLine_E("ERROR: Command-line arguments are not supported yet!");
		return 1;
	}
	
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
		PrintLine("Emscripten SDK path: \"%.*s\"", StrPrint(emscriptenSdkPath));
		InitializeEmsdkIf(StrLit(".."), &isEmsdkInitialized);
	}
	
	Str8 androidSdkDir = ZEROED;
	Str8 androidSdkBuildToolsDir = ZEROED;
	Str8 androidSdkPlatformDir = ZEROED;
	Str8 androidNdkDir = ZEROED;
	Str8 androidNdkToolchainDir = ZEROED;
	if (BUILD_ANDROID)
	{
		androidSdkDir = GetAndroidSdkPath();
		PrintLine("Android SDK path: \"%.*s\"", StrPrint(androidSdkDir));
		androidSdkBuildToolsDir = JoinStrings3(androidSdkDir, StrLit("/build-tools/"), ANDROID_BUILD_TOOLS_VERSION, false);
		androidSdkPlatformDir = JoinStrings3(androidSdkDir, StrLit("/platforms/"), ANDROID_PLATFORM_FOLDERNAME, false);
		androidNdkDir = JoinStrings3(androidSdkDir, StrLit("/ndk/"), ANDROID_NDK_VERSION, false);
		//TODO: "windows-x86_64" is going to be different when compiling on Linux or OSX, we should figure out how we want that configured once we get there
		androidNdkToolchainDir = JoinStrings3(androidNdkDir, StrLit("/toolchains/llvm/prebuilt/"), StrLit("windows-x86_64"), false);
		//TODO: We should check to see if all these folders actually exist and give a nice error to the user when they need to install something or change the build_config.h
	}
	
	Str8 orcaSdkPath = ZEROED;
	if (BUILD_ORCA)
	{
		orcaSdkPath = GetOrcaSdkPath();
		PrintLine("Orca SDK path: \"%.*s\"", StrPrint(orcaSdkPath));
	}
	
	Str8 playdateSdkDir = ZEROED;
	Str8 playdateSdkDir_C_API = ZEROED;
	if (BUILD_PLAYDATE_DEVICE || BUILD_PLAYDATE_SIMULATOR)
	{
		playdateSdkDir = GetPlaydateSdkPath();
		PrintLine("Playdate SDK path: \"%.*s\"", StrPrint(playdateSdkDir));
		playdateSdkDir_C_API = JoinStrings2(playdateSdkDir, StrLit("/C_API"), false);
	}
	
	// +==============================+
	// |       Fill CliArgLists       |
	// +==============================+
	Str8 pigCoreThirdPartyPath = StrLit("[ROOT]/third_party");
	CliArgList pigCoreCompilerFlags = ZEROED;
	CliArgList pigCoreLinkerFlags = ZEROED;
	FillPigCoreFlags(&pigCoreCompilerFlags, &pigCoreLinkerFlags,
		pigCoreThirdPartyPath,
		androidNdkDir, androidNdkToolchainDir,
		orcaSdkPath,
		playdateSdkDir, playdateSdkDir_C_API
	);
	
	//We'll put shader objects, imgui.obj/o, tracy.dll/so, and physx_capi.obj/o into this list
	CliArgList thingsToLink = ZEROED;
	
	AddTaggedArgNt(&pigCoreCompilerFlags, EXE_MSVC_CL "|Piggen|DUMP_ASSEMBLY",          CL_ASSEMB_LISTING_FILE, "piggen.asm");
	AddTaggedArgNt(&pigCoreCompilerFlags, EXE_MSVC_CL "|PigCore|Library|DUMP_ASSEMBLY", CL_ASSEMB_LISTING_FILE, "pig_core.asm");
	AddTaggedArgNt(&pigCoreCompilerFlags, EXE_MSVC_CL "|PigCoreTests|DUMP_ASSEMBLY",    CL_ASSEMB_LISTING_FILE, "tests.asm");
	
	//NOTE: Generated .pb-c.h files contain an #include that doesn't go through "third_party/protobuf_c/" so we add this as an
	//      include directory explicitly and from there it can find <protobuf-c/protobuf-c.h>
	AddTaggedArgNt(&pigCoreCompilerFlags, EXE_MSVC_CL "|BUILD_WITH_PROTOBUF", CL_INCLUDE_DIR, "[ROOT]/third_party/protobuf_c");
	AddTaggedArgNt(&pigCoreCompilerFlags, EXE_CLANG "|BUILD_WITH_PROTOBUF", CLANG_INCLUDE_DIR, "[ROOT]/third_party/protobuf_c");
	
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
			RunCliProgramAndExitOnFailure(protocExe, "", &cmd, StrLit("protoc Failed on parse_proto_google_types.proto!"));
		}
		{
			CliArgList cmd = ZEROED;
			cmd.pathSepChar = '/';
			AddArgNt(&cmd, PROTOC_PROTO_PATH, "[ROOT]/tests");
			AddArgList(&cmd, &proto_CommonFlags);
			AddArgNt(&cmd, PROTOC_C_OUT_PATH, "[ROOT]/tests");
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/tests/tests_proto_types.proto");
			RunCliProgramAndExitOnFailure(protocExe, "", &cmd, StrLit("protoc Failed on tests_proto_types.proto!"));
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
		StrArray piggenTags = ZEROED;
		AddStr(&piggenTags, StrLit("Piggen"));
		AddStr(&piggenTags, BUILDING_ON_OSX ? StrLit("LangObjectiveC") : StrLit("LangC"));
		
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit(".."), &isMsvcInitialized);
			PrintLine("\n[Building %s for Windows...]", FILENAME_PIGGEN_EXE);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/piggen/piggen_main.c");
			AddArgNt(&cmd, CL_BINARY_FILE, FILENAME_PIGGEN_EXE);
			AddArgList(&cmd, &pigCoreCompilerFlags);
			AddArg(&cmd, CL_LINK);
			AddArgList(&cmd, &pigCoreLinkerFlags);
			AddArgList(&cmd, &thingsToLink);
			
			StrArray tags = ZEROED;
			AddStrArray(&tags, &piggenTags);
			AddStr(&tags, StrLit("cl"));
			AddStr(&tags, StrLit("Windows"));
			AddStrArray(&tags, &buildConfigTags);
			
			RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_MSVC_CL), &tags, &cmd, StrLit("Failed to build " FILENAME_PIGGEN_EXE "!"));
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
			AddArgList(&cmd, &pigCoreCompilerFlags);
			AddArgList(&cmd, &pigCoreLinkerFlags);
			AddArgList(&cmd, &thingsToLink);
			
			StrArray tags = ZEROED;
			AddStrArray(&tags, &piggenTags);
			AddStr(&tags, StrLit("clang"));
			AddStr(&tags, StrLit("Linux"));
			AddStr(&tags, StrLit("LinuxOrOsx"));
			AddStrArray(&tags, &buildConfigTags);
			
			#if BUILDING_ON_LINUX
			Str8 clangExe = StrLit(EXE_CLANG);
			#else
			Str8 clangExe = StrLit(EXE_WSL_CLANG);
			mkdir(FOLDERNAME_LINUX, FOLDER_PERMISSIONS);
			chdir(FOLDERNAME_LINUX);
			cmd.rootDirPath = StrLit("../..");
			#endif
			
			RunCliProgramTagArrayAndExitOnFailure(clangExe, &tags, &cmd, StrLit("Failed to build " FILENAME_PIGGEN "!"));
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
			AddArgList(&cmd, &pigCoreCompilerFlags);
			AddArgList(&cmd, &pigCoreLinkerFlags);
			AddArgList(&cmd, &thingsToLink);
			
			StrArray tags = ZEROED;
			AddStrArray(&tags, &piggenTags);
			AddStr(&tags, StrLit("clang"));
			AddStr(&tags, StrLit("OSX"));
			AddStr(&tags, StrLit("LinuxOrOsx"));
			AddStrArray(&tags, &buildConfigTags);
			
			RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_CLANG), &tags, &cmd, StrLit("Failed to build " FILENAME_PIGGEN "!"));
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
		
		RunCliProgramAndExitOnFailure(StrLit(EXEC_PROGRAM_IN_FOLDER_PREFIX RUNNABLE_FILENAME_PIGGEN), "", &cmd, StrLit(RUNNABLE_FILENAME_PIGGEN " Failed!"));
	}
	
	// +--------------------------------------------------------------+
	// |                        Build Shaders                         |
	// +--------------------------------------------------------------+
	FindShadersContext findContext = ZEROED;
	CliArgList clang_AndroidShaderObjects[AndroidTargetArchitechture_Count] = ZEROED;
	if (BUILD_SHADERS || BUILD_WITH_SOKOL_GFX)
	{
		const char* ignoreList[] = { ".git", "_template", "third_party", "_build" };
		findContext.ignoreListLength = ArrayCount(ignoreList);
		findContext.ignoreList = (Str8*)malloc(sizeof(Str8) * findContext.ignoreListLength);
		for (u64 iIndex = 0; iIndex < findContext.ignoreListLength; iIndex++)
		{
			findContext.ignoreList[iIndex] = MakeStr8Nt(ignoreList[iIndex]);
		}
		
		RecursiveDirWalk(StrLit(".."), FindShaderFilesCallback, &findContext);
		
		if (BUILD_WINDOWS)
		{
			for (u64 sIndex = 0; sIndex < findContext.objPaths.length; sIndex++)
			{
				Str8 objPath = findContext.objPaths.strings[sIndex];
				AddTaggedArgStr(&thingsToLink, EXE_MSVC_CL "|PigCore|Windows|BUILD_WITH_SOKOL_GFX", CLI_QUOTED_ARG, objPath);
				if (!DoesFileExist(objPath) && !BUILD_SHADERS) { PrintLine("Building shaders because \"%.*s\" is missing!", StrPrint(objPath)); BUILD_SHADERS = true; }
			}
		}
		if (BUILD_LINUX)
		{
			for (u64 sIndex = 0; sIndex < findContext.oPaths.length; sIndex++)
			{
				Str8 oPath = findContext.oPaths.strings[sIndex];
				AddTaggedArgStr(&thingsToLink, EXE_CLANG "|PigCore|Linux|BUILD_WITH_SOKOL_GFX", CLI_QUOTED_ARG, oPath);
				Str8 oPathWithFolder = BUILDING_ON_LINUX ? CopyStr8(oPath, false) : JoinStrings2(StrLit(FOLDERNAME_LINUX "/"), oPath, false);
				if (!DoesFileExist(oPathWithFolder) && !BUILD_SHADERS) { PrintLine("Building shaders because \"%.*s\" is missing!", StrPrint(oPathWithFolder)); BUILD_SHADERS = true; }
			}
		}
		if (BUILD_OSX)
		{
			for (u64 sIndex = 0; sIndex < findContext.oPaths.length; sIndex++)
			{
				Str8 oPath = findContext.oPaths.strings[sIndex];
				AddTaggedArgStr(&thingsToLink, EXE_CLANG "|PigCore|OSX|BUILD_WITH_SOKOL_GFX", CLI_QUOTED_ARG, oPath);
				Str8 oPathWithFolder = BUILDING_ON_OSX ? CopyStr8(oPath, false) : JoinStrings2(StrLit(FOLDERNAME_OSX "/"), oPath, false);
				if (!DoesFileExist(oPathWithFolder) && !BUILD_SHADERS) { PrintLine("Building shaders because \"%.*s\" is missing!", StrPrint(oPathWithFolder)); BUILD_SHADERS = true; }
			}
		}
		if (BUILD_ANDROID)
		{
			for (u64 sIndex = 0; sIndex < findContext.oPaths.length; sIndex++)
			{
				for (u64 archIndex = 1; archIndex < AndroidTargetArchitechture_Count; archIndex++)
				{
					AndroidTargetArchitechture architecture = (AndroidTargetArchitechture)archIndex;
					Str8 archFolderName = MakeStr8Nt(GetAndroidTargetArchitechtureFolderName(architecture));
					Str8 archFolderPath = JoinStrings3(StrLit("lib/"), archFolderName, StrLit("/"), false);
					Str8 oPath = findContext.oPaths.strings[sIndex];
					AddArgStr(&clang_AndroidShaderObjects[archIndex], CLI_QUOTED_ARG, oPath);
					Str8 oRelativePath = JoinStrings3(StrLit(FOLDERNAME_ANDROID "/"), archFolderPath, oPath, false);
					if (!DoesFileExist(oRelativePath) && !BUILD_SHADERS) { PrintLine("Building shaders because \"%.*s\" is missing!", StrPrint(oRelativePath)); BUILD_SHADERS = true; }
				}
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
		
		PrintLine("Found %llu shader%s", findContext.shaderPaths.length, findContext.shaderPaths.length == 1 ? "" : "s");
		// for (u64 sIndex = 0; sIndex < findContext.shaderPaths.length; sIndex++)
		// {
		// 	PrintLine("Shader[%u]", sIndex);
		// 	PrintLine("\t\"%.*s\"", StrPrint(findContext.shaderPaths.strings[sIndex]));
		// 	PrintLine("\t\"%.*s\"", StrPrint(findContext.headerPaths.strings[sIndex]));
		// 	PrintLine("\t\"%.*s\"", StrPrint(findContext.sourcePaths.strings[sIndex]));
		// 	PrintLine("\t\"%.*s\"", StrPrint(findContext.objPaths.strings[sIndex]));
		// 	PrintLine("\t\"%.*s\"", StrPrint(findContext.oPaths.strings[sIndex]));
		// }
		
		// First use shdc.exe to generate header files for each .glsl file
		for (u64 sIndex = 0; sIndex < findContext.shaderPaths.length; sIndex++)
		{
			Str8 shaderPath = findContext.shaderPaths.strings[sIndex];
			Str8 headerPath = findContext.headerPaths.strings[sIndex];
			Str8 realHeaderPath = StrReplace(headerPath, StrLit("[ROOT]"), StrLit(".."), false);
			Str8 realShaderPath = StrReplace(shaderPath, StrLit("[ROOT]"), StrLit(".."), false);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, SHDC_FORMAT, "sokol_impl");
			AddArgNt(&cmd, SHDC_ERROR_FORMAT, "msvc");
			// AddArg(&cmd, SHDC_REFLECTION);
			AddArgNt(&cmd, SHDC_SHADER_LANGUAGES, "hlsl5:glsl430:glsl300es:metal_macos");
			AddArgStr(&cmd, SHDC_INPUT, shaderPath);
			AddArgStr(&cmd, SHDC_OUTPUT, headerPath);
			
			PrintLine("Generating \"%.*s\"...", StrPrint(realHeaderPath));
			Str8 shdcExe = JoinStrings2(StrLit("../"), StrLit(EXE_SHDC), false);
			FixPathSlashes(shdcExe, PATH_SEP_CHAR);
			RunCliProgramAndExitOnFailure(shdcExe, "", &cmd, StrLit(EXE_SHDC_NAME " failed on TODO:!"));
			AssertFileExist(realHeaderPath, true);
			
			ScrapeShaderHeaderFileAndAddExtraInfo(realHeaderPath, realShaderPath);
			free(realHeaderPath.chars);
			free(realShaderPath.chars);
		}
		
		//Then compile each header file to an .o/.obj file
		for (u64 sIndex = 0; sIndex < findContext.shaderPaths.length; sIndex++)
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
			PrintLine("Generating \"%.*s\"...", StrPrint(realSourcePath));
			CreateAndWriteFile(realSourcePath, sourceFileContents, true);
			
			StrArray shaderTags = ZEROED;
			AddStr(&shaderTags, StrLit("Shader"));
			AddStr(&shaderTags, BUILDING_ON_OSX ? StrLit("LangObjectiveC") : StrLit("LangC"));
			AddStr(&shaderTags, StrLit("Object"));
			
			if (BUILD_WINDOWS)
			{
				Str8 objPath = findContext.objPaths.strings[sIndex];
				
				CliArgList cmd = ZEROED;
				AddArg(&cmd, CL_COMPILE);
				AddArgStr(&cmd, CLI_QUOTED_ARG, sourcePath);
				AddArgStr(&cmd, CL_OBJ_FILE, objPath);
				AddArgStr(&cmd, CL_INCLUDE_DIR, headerDirectory);
				AddArgList(&cmd, &pigCoreCompilerFlags);
				
				StrArray tags = ZEROED;
				AddStrArray(&tags, &shaderTags);
				AddStr(&tags, StrLit("cl"));
				AddStr(&tags, StrLit("Windows"));
				AddStrArray(&tags, &buildConfigTags);
				
				Str8 errorMessage = JoinStrings3(StrLit("Fald to build "), objPath, StrLit(" for Windows!"), false);
				RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_MSVC_CL), &tags, &cmd, errorMessage);
				AssertFileExist(objPath, true);
			}
			if (BUILD_LINUX)
			{
				Str8 oPath = findContext.oPaths.strings[sIndex];
				
				CliArgList cmd = ZEROED;
				cmd.pathSepChar = '/';
				AddArg(&cmd, CLANG_COMPILE);
				AddArgStr(&cmd, CLI_QUOTED_ARG, sourcePath);
				AddArgStr(&cmd, CLANG_OUTPUT_FILE, oPath);
				AddArgStr(&cmd, CLANG_INCLUDE_DIR, headerDirectory);
				AddArgList(&cmd, &pigCoreCompilerFlags);
				
				StrArray tags = ZEROED;
				AddStrArray(&tags, &shaderTags);
				AddStr(&tags, StrLit("clang"));
				AddStr(&tags, StrLit("Linux"));
				AddStr(&tags, StrLit("LinuxOrOsx"));
				AddStrArray(&tags, &buildConfigTags);
				
				#if BUILDING_ON_LINUX
				Str8 clangExe = StrLit(EXE_CLANG);
				#else
				Str8 clangExe = StrLit(EXE_WSL_CLANG);
				mkdir(FOLDERNAME_LINUX, FOLDER_PERMISSIONS);
				chdir(FOLDERNAME_LINUX);
				cmd.rootDirPath = StrLit("../..");
				#endif
				
				Str8 errorMessage = JoinStrings3(StrLit("Fald to build "), oPath, StrLit(" for Linux!"), false);
				RunCliProgramTagArrayAndExitOnFailure(clangExe, &tags, &cmd, errorMessage);
				AssertFileExist(oPath, true);
				
				#if !BUILDING_ON_LINUX
				chdir("..");
				#endif
			}
			if (BUILD_OSX)
			{
				Str8 oPath = findContext.oPaths.strings[sIndex];
				
				CliArgList cmd = ZEROED;
				cmd.pathSepChar = '/';
				AddArg(&cmd, CLANG_COMPILE);
				AddArgStr(&cmd, CLI_QUOTED_ARG, sourcePath);
				AddArgStr(&cmd, CLANG_OUTPUT_FILE, oPath);
				AddArgStr(&cmd, CLANG_INCLUDE_DIR, headerDirectory);
				AddArgNt(&cmd, CLANG_DISABLE_WARNING, "unused-command-line-argument");
				AddArgList(&cmd, &pigCoreCompilerFlags);
				
				StrArray tags = ZEROED;
				AddStrArray(&tags, &shaderTags);
				AddStr(&tags, StrLit("clang"));
				AddStr(&tags, StrLit("OSX"));
				AddStr(&tags, StrLit("LinuxOrOsx"));
				AddStrArray(&tags, &buildConfigTags);
				
				Str8 errorMessage = JoinStrings3(StrLit("Fald to build "), oPath, StrLit(" for OSX!"), false);
				RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_CLANG), &tags, &cmd, errorMessage);
				AssertFileExist(oPath, true);
			}
			if (BUILD_ANDROID)
			{
				mkdir(FOLDERNAME_ANDROID, FOLDER_PERMISSIONS);
				chdir(FOLDERNAME_ANDROID);
				mkdir("lib", FOLDER_PERMISSIONS);
				chdir("lib");
				
				for (u64 archIndex = 1; archIndex < AndroidTargetArchitechture_Count; archIndex++)
				{
					AndroidTargetArchitechture architecture = (AndroidTargetArchitechture)archIndex;
					mkdir(GetAndroidTargetArchitechtureFolderName(architecture), FOLDER_PERMISSIONS);
					chdir(GetAndroidTargetArchitechtureFolderName(architecture));
					Str8 architectureStr = MakeStr8Nt(GetAndroidTargetArchitechtureTargetStr(architecture));
					
					Str8 oPath = findContext.oPaths.strings[sIndex];
					
					CliArgList cmd = ZEROED;
					cmd.pathSepChar = '/';
					cmd.rootDirPath = StrLit("../../../..");
					AddArg(&cmd, CLANG_COMPILE);
					AddArgStr(&cmd, CLI_QUOTED_ARG, sourcePath);
					AddArgStr(&cmd, CLANG_OUTPUT_FILE, oPath);
					AddArgStr(&cmd, CLANG_INCLUDE_DIR, headerDirectory);
					AddArgStr(&cmd, CLANG_TARGET_ARCHITECTURE, architectureStr);
					AddArgList(&cmd, &pigCoreCompilerFlags);
					
					StrArray tags = ZEROED;
					AddStrArray(&tags, &shaderTags);
					AddStr(&tags, StrLit("clang"));
					AddStr(&tags, StrLit("Android"));
					AddStr(&tags, architectureStr);
					AddStrArray(&tags, &buildConfigTags);
					
					Str8 errorMessage = JoinStrings3(StrLit("Fald to build "), oPath, StrLit(" for Android!"), false);
					RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_CLANG), &tags, &cmd, errorMessage);
					AssertFileExist(oPath, true);
					
					chdir("..");
				}
				
				chdir("..");
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
	// |                       Build tracy.dll                        |
	// +--------------------------------------------------------------+
	if (PROFILING_ENABLED && !BUILD_TRACY_DLL && BUILD_WINDOWS && !DoesFileExist(StrLit(FILENAME_TRACY_DLL))) { PrintLine("Building %s because it's missing", FILENAME_TRACY_DLL); BUILD_TRACY_DLL = true; }
	if (PROFILING_ENABLED && !BUILD_TRACY_DLL && BUILD_LINUX && !DoesFileExist(StrLit(FILENAME_TRACY_SO))) { PrintLine("Building %s because it's missing", FILENAME_TRACY_SO); BUILD_TRACY_DLL = true; }
	if (BUILD_TRACY_DLL)
	{
		StrArray tracyTags = ZEROED;
		AddStr(&tracyTags, StrLit("Tracy"));
		AddStr(&tracyTags, StrLit("LangCpp"));
		AddStr(&tracyTags, StrLit("Library"));
		
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit(".."), &isMsvcInitialized);
			PrintLine("[Building %s for Windows...]", FILENAME_TRACY_DLL);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/third_party/tracy/TracyClient.cpp");
			AddArgNt(&cmd, CL_BINARY_FILE, FILENAME_TRACY_DLL);
			AddArgList(&cmd, &pigCoreCompilerFlags);
			AddArg(&cmd, CL_LINK);
			AddArg(&cmd, LINK_BUILD_DLL);
			AddArgList(&cmd, &pigCoreLinkerFlags);
			AddArgList(&cmd, &thingsToLink);
			
			StrArray tags = ZEROED;
			AddStrArray(&tags, &tracyTags);
			AddStr(&tags, StrLit("cl"));
			AddStr(&tags, StrLit("Windows"));
			AddStrArray(&tags, &buildConfigTags);
			
			RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_MSVC_CL), &tags, &cmd, StrLit("Failed to build " FILENAME_TRACY_DLL "!"));
			AssertFileExist(StrLit(FILENAME_TRACY_DLL), true);
			PrintLine("[Built %s for Windows!]", FILENAME_TRACY_DLL);
		}
		if (BUILD_LINUX)
		{
			PrintLine("\n[Building %s for Linux...]", FILENAME_TRACY_SO);
			
			CliArgList cmd = ZEROED;
			cmd.pathSepChar = '/';
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/third_party/tracy/TracyClient.cpp");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, FILENAME_TRACY_SO);
			AddArg(&cmd, CLANG_BUILD_SHARED_LIB);
			AddArg(&cmd, CLANG_fPIC);
			AddArgList(&cmd, &pigCoreCompilerFlags);
			AddArgList(&cmd, &pigCoreLinkerFlags);
			AddArgList(&cmd, &thingsToLink);
			
			StrArray tags = ZEROED;
			AddStrArray(&tags, &tracyTags);
			AddStr(&tags, StrLit("clang"));
			AddStr(&tags, StrLit("Linux"));
			AddStr(&tags, StrLit("LinuxOrOsx"));
			AddStrArray(&tags, &buildConfigTags);
			
			#if BUILDING_ON_LINUX
			Str8 clangExe = StrLit(EXE_CLANG);
			#else
			Str8 clangExe = StrLit(EXE_WSL_CLANG);
			mkdir(FOLDERNAME_LINUX, FOLDER_PERMISSIONS);
			chdir(FOLDERNAME_LINUX);
			cmd.rootDirPath = StrLit("../..");
			#endif
			
			RunCliProgramTagArrayAndExitOnFailure(clangExe, &tags, &cmd, StrLit("Failed to build " FILENAME_TRACY_SO "!"));
			AssertFileExist(StrLit(FILENAME_TRACY_SO), true);
			PrintLine("[Built %s for Linux!]", FILENAME_TRACY_SO);
			
			#if !BUILDING_ON_LINUX
			chdir("..");
			#endif
		}
	}
	AddTaggedArgNt(&thingsToLink, EXE_MSVC_CL "|Windows|Program|PROFILING_ENABLED", CLI_QUOTED_ARG, FILENAME_TRACY_LIB);
	AddTaggedArgNt(&thingsToLink, EXE_CLANG "|Linux|Program|PROFILING_ENABLED", CLI_QUOTED_ARG, FILENAME_TRACY_SO);
	//TODO: Add Linux .dylib file as linkerFlag
	
	// +--------------------------------------------------------------+
	// |                       Build imgui.obj                        |
	// +--------------------------------------------------------------+
	if (BUILD_WITH_IMGUI && !BUILD_IMGUI_OBJ && BUILD_WINDOWS && !DoesFileExist(StrLit(FILENAME_IMGUI_OBJ))) { PrintLine("Building %s because it's missing", FILENAME_IMGUI_OBJ); BUILD_IMGUI_OBJ = true; }
	if (BUILD_WITH_IMGUI && !BUILD_IMGUI_OBJ && BUILD_LINUX && !DoesFileExist(StrLit(FILENAME_IMGUI_O))) { PrintLine("Building %s because it's missing", FILENAME_IMGUI_O); BUILD_IMGUI_OBJ = true; }
	if (BUILD_IMGUI_OBJ)
	{
		StrArray imguiTags = ZEROED;
		AddStr(&imguiTags, StrLit("Imgui"));
		AddStr(&imguiTags, StrLit("LangCpp"));
		AddStr(&imguiTags, StrLit("Object"));
		
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit(".."), &isMsvcInitialized);
			PrintLine("[Building %s for Windows...]", FILENAME_IMGUI_OBJ);
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, CL_COMPILE);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/ui/ui_imgui_main.cpp");
			AddArgNt(&cmd, CL_OBJ_FILE, FILENAME_IMGUI_OBJ);
			AddArgList(&cmd, &pigCoreCompilerFlags);
			
			StrArray tags = ZEROED;
			AddStrArray(&tags, &imguiTags);
			AddStr(&tags, StrLit("cl"));
			AddStr(&tags, StrLit("Windows"));
			AddStrArray(&tags, &buildConfigTags);
			
			RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_MSVC_CL), &tags, &cmd, StrLit("Failed to build " FILENAME_IMGUI_OBJ "!"));
			AssertFileExist(StrLit(FILENAME_IMGUI_OBJ), true);
			PrintLine("[Built %s for Windows!]", FILENAME_IMGUI_OBJ);
		}
		if (BUILD_LINUX)
		{
			PrintLine("[Building %s for Linux...]", FILENAME_IMGUI_O);
			
			CliArgList cmd = ZEROED;
			cmd.pathSepChar = '/';
			AddArg(&cmd, CLANG_COMPILE);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/ui/ui_imgui_main.cpp");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, FILENAME_IMGUI_O);
			AddArgList(&cmd, &pigCoreCompilerFlags);
			
			StrArray tags = ZEROED;
			AddStrArray(&tags, &imguiTags);
			AddStr(&tags, StrLit("clang"));
			AddStr(&tags, StrLit("Linux"));
			AddStr(&tags, StrLit("LinuxOrOsx"));
			AddStrArray(&tags, &buildConfigTags);
			
			#if BUILDING_ON_LINUX
			Str8 clangExe = StrLit(EXE_CLANG);
			#else
			Str8 clangExe = StrLit(EXE_WSL_CLANG);
			mkdir(FOLDERNAME_LINUX, FOLDER_PERMISSIONS);
			chdir(FOLDERNAME_LINUX);
			cmd.rootDirPath = StrLit("../..");
			#endif
			
			RunCliProgramTagArrayAndExitOnFailure(clangExe, &tags, &cmd, StrLit("Failed to build " FILENAME_IMGUI_O "!"));
			AssertFileExist(StrLit(FILENAME_IMGUI_O), true);
			PrintLine("[Built %s for Linux!]", FILENAME_IMGUI_O);
			
			#if !BUILDING_ON_LINUX
			chdir("..");
			#endif
		}
	}
	AddTaggedArgNt(&thingsToLink, EXE_MSVC_CL "|Windows|Program|BUILD_WITH_IMGUI", CLI_QUOTED_ARG, FILENAME_IMGUI_OBJ);
	AddTaggedArgNt(&thingsToLink, EXE_CLANG "|LinuxOrOsx|Program|BUILD_WITH_IMGUI", CLI_QUOTED_ARG, FILENAME_IMGUI_O);
	
	// +--------------------------------------------------------------+
	// |                     Build physx_capi.obj                     |
	// +--------------------------------------------------------------+
	if (BUILD_WITH_PHYSX && !BUILD_PHYSX_OBJ && BUILD_WINDOWS && !DoesFileExist(StrLit(FILENAME_PHYSX_OBJ))) { PrintLine("Building %s because it's missing", FILENAME_PHYSX_OBJ); BUILD_PHYSX_OBJ = true; }
	if (BUILD_WITH_PHYSX && !BUILD_PHYSX_OBJ && BUILD_LINUX && !DoesFileExist(StrLit(FILENAME_PHYSX_O))) { PrintLine("Building %s because it's missing", FILENAME_PHYSX_O); BUILD_PHYSX_OBJ = true; }
	if (BUILD_PHYSX_OBJ)
	{
		StrArray physxTags = ZEROED;
		AddStr(&physxTags, StrLit("PhysX"));
		AddStr(&physxTags, StrLit("LangCpp"));
		AddStr(&physxTags, StrLit("Object"));
		
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit(".."), &isMsvcInitialized);
			PrintLine("[Building %s for Windows...]", FILENAME_PHYSX_OBJ);
			
			CliArgList cmd = ZEROED;
			AddArg(&cmd, CL_COMPILE);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/phys/phys_physx_capi_main.cpp");
			AddArgNt(&cmd, CL_OBJ_FILE, FILENAME_PHYSX_OBJ);
			AddArgList(&cmd, &pigCoreCompilerFlags);
			AddArg(&cmd, CL_LINK);
			AddArgList(&cmd, &pigCoreLinkerFlags);
			AddArgList(&cmd, &thingsToLink);
			
			StrArray tags = ZEROED;
			AddStrArray(&tags, &physxTags);
			AddStr(&tags, StrLit("cl"));
			AddStr(&tags, StrLit("Windows"));
			AddStrArray(&tags, &buildConfigTags);
			
			RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_MSVC_CL), &tags, &cmd, StrLit("Failed to build " FILENAME_PHYSX_OBJ "!"));
			AssertFileExist(StrLit(FILENAME_PHYSX_OBJ), true);
			PrintLine("[Built %s for Windows!]", FILENAME_PHYSX_OBJ);
		}
		if (BUILD_LINUX)
		{
			//TODO: Implement Linux version!
		}
	}
	AddTaggedArgNt(&thingsToLink, EXE_MSVC_CL "|Windows|Program|BUILD_WITH_PHYSX", CLI_QUOTED_ARG, FILENAME_PHYSX_OBJ);
	// AddTaggedArgNt(&thingsToLink, EXE_MSVC_CL "|LinuxOrOsx|Program|BUILD_WITH_PHYSX", CLI_QUOTED_ARG, FILENAME_PHYSX_O); //TODO: Uncomment me once we actually build this on LinuxOrOsx!
	
	// +--------------------------------------------------------------+
	// |                      Build pig_core.dll                      |
	// +--------------------------------------------------------------+
	if (BUILD_PIG_CORE_DLL)
	{
		StrArray pigCoreTags = ZEROED;
		AddStr(&pigCoreTags, StrLit("PigCore"));
		AddStr(&pigCoreTags, StrLit("LangC"));
		AddStr(&pigCoreTags, StrLit("Library"));
		
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit(".."), &isMsvcInitialized);
			PrintLine("\n[Building %s for Windows...]", FILENAME_PIG_CORE_DLL);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/dll/dll_main.c");
			AddArgNt(&cmd, CL_BINARY_FILE, FILENAME_PIG_CORE_DLL);
			AddArgList(&cmd, &pigCoreCompilerFlags);
			AddArg(&cmd, CL_LINK);
			AddArg(&cmd, LINK_BUILD_DLL);
			AddArgList(&cmd, &pigCoreLinkerFlags);
			AddArgList(&cmd, &thingsToLink);
			
			StrArray tags = ZEROED;
			AddStrArray(&tags, &pigCoreTags);
			AddStr(&tags, StrLit("cl"));
			AddStr(&tags, StrLit("Windows"));
			AddStrArray(&tags, &buildConfigTags);
			
			RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_MSVC_CL), &tags, &cmd, StrLit("Failed to build " FILENAME_PIG_CORE_DLL "!"));
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
			AddArgList(&cmd, &pigCoreCompilerFlags);
			AddArgList(&cmd, &pigCoreLinkerFlags);
			AddArgList(&cmd, &thingsToLink);
			
			StrArray tags = ZEROED;
			AddStrArray(&tags, &pigCoreTags);
			AddStr(&tags, StrLit("clang"));
			AddStr(&tags, StrLit("Linux"));
			AddStr(&tags, StrLit("LinuxOrOsx"));
			AddStrArray(&tags, &buildConfigTags);
			
			#if BUILDING_ON_LINUX
			Str8 clangExe = StrLit(EXE_CLANG);
			#else
			Str8 clangExe = StrLit(EXE_WSL_CLANG);
			mkdir(FOLDERNAME_LINUX, FOLDER_PERMISSIONS);
			chdir(FOLDERNAME_LINUX);
			cmd.rootDirPath = StrLit("../..");
			#endif
			
			RunCliProgramTagArrayAndExitOnFailure(clangExe, &tags, &cmd, StrLit("Failed to build " FILENAME_PIG_CORE_SO "!"));
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
	if (INSTALL_TESTS_APK && !BUILD_TESTS && !DoesFileExist(StrLit(FOLDERNAME_ANDROID "/" FILENAME_TESTS_APK))) { PrintLine("Building %s because it's missing", FILENAME_TESTS_APK); BUILD_TESTS = true; BUILD_ANDROID = true; }
	if (BUILD_TESTS)
	{
		StrArray testsTags = ZEROED;
		AddStr(&testsTags, StrLit("PigCoreTests"));
		AddStr(&testsTags, StrLit("PigCore")); //We add this because we are compiling PigCore implementation directly into tests.exe (not linking as a .dll)
		AddStr(&testsTags, BUILDING_ON_OSX ? StrLit("LangObjectiveC") : StrLit("LangC"));
		AddStr(&testsTags, StrLit("Program"));
		
		// +==============================+
		// |      Windows tests.exe       |
		// +==============================+
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit(".."), &isMsvcInitialized);
			PrintLine("\n[Building %s for Windows...]", FILENAME_TESTS_EXE);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/tests/tests_main.c");
			AddArgNt(&cmd, CL_BINARY_FILE, FILENAME_TESTS_EXE);
			AddArgList(&cmd, &pigCoreCompilerFlags);
			AddArg(&cmd, CL_LINK);
			AddArgList(&cmd, &pigCoreLinkerFlags);
			AddArgList(&cmd, &thingsToLink);
			
			StrArray tags = ZEROED;
			AddStrArray(&tags, &testsTags);
			AddStr(&tags, StrLit("cl"));
			AddStr(&tags, StrLit("Windows"));
			AddStrArray(&tags, &buildConfigTags);
			
			RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_MSVC_CL), &tags, &cmd, StrLit("Failed to build " FILENAME_TESTS_EXE "!"));
			AssertFileExist(StrLit(FILENAME_TESTS_EXE), true);
			PrintLine("[Built %s for Windows!]", FILENAME_TESTS_EXE);
		}
		
		// +==============================+
		// |         Linux tests          |
		// +==============================+
		if (BUILD_LINUX)
		{
			PrintLine("\n[Building %s for Linux...]", FILENAME_TESTS);
			
			CliArgList cmd = ZEROED;
			cmd.pathSepChar = '/';
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/tests/tests_main.c");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, FILENAME_TESTS);
			AddArgNt(&cmd, CLANG_RPATH_DIR, ".");
			AddArgList(&cmd, &pigCoreCompilerFlags);
			AddArgList(&cmd, &pigCoreLinkerFlags);
			AddArgList(&cmd, &thingsToLink);
			
			StrArray tags = ZEROED;
			AddStrArray(&tags, &testsTags);
			AddStr(&tags, StrLit("clang"));
			AddStr(&tags, StrLit("Linux"));
			AddStr(&tags, StrLit("LinuxOrOsx"));
			AddStrArray(&tags, &buildConfigTags);
			
			#if BUILDING_ON_LINUX
			Str8 clangExe = StrLit(EXE_CLANG);
			#else
			Str8 clangExe = StrLit(EXE_WSL_CLANG);
			mkdir(FOLDERNAME_LINUX, FOLDER_PERMISSIONS);
			chdir(FOLDERNAME_LINUX);
			cmd.rootDirPath = StrLit("../..");
			#endif
			
			RunCliProgramTagArrayAndExitOnFailure(clangExe, &tags, &cmd, StrLit("Failed to build " FILENAME_TESTS "!"));
			AssertFileExist(StrLit(FILENAME_TESTS), true);
			PrintLine("[Built %s for Linux!]", FILENAME_TESTS);
			
			#if !BUILDING_ON_LINUX
			chdir("..");
			#endif
		}
		
		// +==============================+
		// |          OSX tests           |
		// +==============================+
		if (BUILD_OSX)
		{
			PrintLine("\n[Building %s for OSX...]", FILENAME_TESTS);
			
			CliArgList cmd = ZEROED;
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/tests/tests_main.m");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, FILENAME_TESTS);
			AddArgList(&cmd, &pigCoreCompilerFlags);
			AddArgList(&cmd, &pigCoreLinkerFlags);
			AddArgList(&cmd, &thingsToLink);
			
			StrArray tags = ZEROED;
			AddStrArray(&tags, &testsTags);
			AddStr(&tags, StrLit("clang"));
			AddStr(&tags, StrLit("OSX"));
			AddStr(&tags, StrLit("LinuxOrOsx"));
			AddStrArray(&tags, &buildConfigTags);
			
			RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_CLANG), &tags, &cmd, StrLit("Failed to build " FILENAME_TESTS "!"));
			AssertFileExist(StrLit(FILENAME_TESTS), true);
			PrintLine("[Built %s for OSX!]", FILENAME_TESTS);
		}
		
		// +==============================+
		// |          Web tests           |
		// +==============================+
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
			AddArgList(&cmd, &pigCoreCompilerFlags);
			AddArgList(&cmd, &pigCoreLinkerFlags);
			AddArgList(&cmd, &thingsToLink);
			
			StrArray tags = ZEROED;
			AddStrArray(&tags, &testsTags);
			if (USE_EMSCRIPTEN) { AddStr(&tags, StrLit("emcc")); }
			AddStr(&tags, StrLit("clang")); //basically all compiler flags that are compatible with clang are compatible with emcc
			AddStr(&tags, StrLit("Web"));
			AddStr(&tags, StrLit("Wasm"));
			AddStrArray(&tags, &buildConfigTags);
			
			RunCliProgramTagArrayAndExitOnFailure(USE_EMSCRIPTEN ? StrLit(EXE_EMSCRIPTEN_COMPILER) : StrLit(EXE_CLANG), &tags, &cmd, StrLit("Failed to build " FILENAME_APP_WASM "!"));
			if (USE_EMSCRIPTEN)
			{
				AssertFileExist(StrLit(FILENAME_INDEX_HTML), true);
				AssertFileExist(StrLit(FILENAME_INDEX_WASM), true);
				AssertFileExist(StrLit("index.js"), true);
			}
			else
			{
				AssertFileExist(MakeStr8Nt(FILENAME_APP_WASM), true);
			}
			PrintLine("[Built %s for Web!]", FILENAME_APP_WASM);
			
			if (CONVERT_WASM_TO_WAT)
			{
				CliArgList convertCmd = ZEROED;
				AddArgNt(&convertCmd, CLI_QUOTED_ARG, USE_EMSCRIPTEN ? FILENAME_INDEX_WASM : FILENAME_APP_WASM);
				AddArgNt(&convertCmd, CLI_PIPE_OUTPUT_TO_FILE, USE_EMSCRIPTEN ? FILENAME_INDEX_WAT : FILENAME_APP_WAT);
				
				int convertStatusCode = RunCliProgram(StrLit("wasm2wat"), "", &convertCmd);
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
				
				CopyFileToPath(StrLit("../../wasm/wasm_app_style.css"), StrLit("main.css"), true);
				CopyFileToPath(StrLit("../../wasm/wasm_app_index.html"), StrLit("index.html"), true);
			}
			
			chdir("..");
		}
		
		// +==============================+
		// |        Android tests         |
		// +==============================+
		if (BUILD_ANDROID)
		{
			PrintLine("\n[Building %s for Android...]", BUILD_ANDROID_APK ? FILENAME_TESTS_APK : FILENAME_TESTS_SO);
			mkdir(FOLDERNAME_ANDROID, FOLDER_PERMISSIONS);
			chdir(FOLDERNAME_ANDROID);
			
			Str8 clangExe = JoinStrings2(androidNdkToolchainDir, StrLit("\\bin\\clang.exe"), false);
			FixPathSlashes(clangExe, PATH_SEP_CHAR);
			Str8 javacExe = StrLit("javac.exe");
			Str8 d8Exe = JoinStrings2(androidSdkBuildToolsDir, StrLit("/d8.bat"), false);
			FixPathSlashes(d8Exe, PATH_SEP_CHAR);
			Str8 aaptExe = JoinStrings2(androidSdkBuildToolsDir, StrLit("/aapt2.exe"), false);
			FixPathSlashes(aaptExe, PATH_SEP_CHAR);
			Str8 apksignerExe = JoinStrings2(androidSdkBuildToolsDir, StrLit("/apksigner.bat"), false);
			FixPathSlashes(apksignerExe, PATH_SEP_CHAR);
			Str8 zipalignExe = JoinStrings2(androidSdkBuildToolsDir, StrLit("/zipalign"), false);
			FixPathSlashes(zipalignExe, PATH_SEP_CHAR);
			Str8 androidJarPath = JoinStrings2(androidSdkPlatformDir, StrLit("/android.jar"), false);
			
			CliArgList cmdBase = ZEROED;
			AddArgNt(&cmdBase, CLI_QUOTED_ARG, "[ROOT]/tests/tests_main.c");
			AddArg(&cmdBase, CLANG_BUILD_SHARED_LIB);
			AddArgNt(&cmdBase, CLANG_OUTPUT_FILE, DUMP_PREPROCESSOR ? "tests_android_PREPROCESSED.c" : FILENAME_TESTS_SO);
			AddArgNt(&cmdBase, CLANG_LIB_SO_NAME, FILENAME_TESTS_SO);
			
			// MyRemoveDirectory(StrLit("lib"), true);
			mkdir("lib", FOLDER_PERMISSIONS);
			chdir("lib");
			for (u64 archIndex = 1; archIndex < AndroidTargetArchitechture_Count; archIndex++)
			{
				AndroidTargetArchitechture architecture = (AndroidTargetArchitechture)archIndex;
				mkdir(GetAndroidTargetArchitechtureFolderName(architecture), FOLDER_PERMISSIONS);
				chdir(GetAndroidTargetArchitechtureFolderName(architecture));
				PrintLine("Building for %s...", GetAndroidTargetArchitechtureFolderName(architecture));
				Str8 architectureStr = MakeStr8Nt(GetAndroidTargetArchitechtureTargetStr(architecture));
				
				CliArgList cmd = ZEROED;
				cmd.pathSepChar = '/';
				cmd.rootDirPath = StrLit("../../../..");
				AddArgList(&cmd, &cmdBase);
				AddArgStr(&cmd, CLANG_TARGET_ARCHITECTURE, architectureStr);
				Str8 sysrootRelativePath = JoinStrings3(StrLit("/sysroot/usr/lib/"), architectureStr, StrLit("/35/"), false);
				AddArgStr(&cmd, CLANG_LIBRARY_DIR, JoinStrings2(androidNdkToolchainDir, sysrootRelativePath, false));
				if (BUILD_WITH_SOKOL_GFX) { AddArgList(&cmd, &clang_AndroidShaderObjects[archIndex]); } //TODO: Remove me!
				AddArgList(&cmd, &pigCoreCompilerFlags);
				AddArgList(&cmd, &pigCoreLinkerFlags);
				AddArgList(&cmd, &thingsToLink);
				
				StrArray tags = ZEROED;
				AddStrArray(&tags, &testsTags);
				AddStr(&tags, StrLit("clang"));
				AddStr(&tags, StrLit("Android"));
				AddStr(&tags, architectureStr);
				AddStrArray(&tags, &buildConfigTags);
				
				RunCliProgramTagArrayAndExitOnFailure(clangExe, &tags, &cmd, StrLit("Failed to build " FILENAME_TESTS_SO "!"));
				if (DUMP_PREPROCESSOR) { chdir(".."); continue; }
				AssertFileExist(StrLit(FILENAME_TESTS_SO), true);
				
				chdir("..");
			}
			chdir("..");
			
			if (BUILD_ANDROID_APK && !DUMP_PREPROCESSOR)
			{
				if (!DoesFileExist(StrLit(FILENAME_CLASSES_DEX)))
				{
					WriteLine("Compiling " FILENAME_DUMMY_JAVA "...");
					
					if (!DoesFileExist(StrLit(FILENAME_DUMMY_JAVA)))
					{
						const char* dummyClassCode = "public class Dummy { }\n";
						CreateAndWriteFile(StrLit(FILENAME_DUMMY_JAVA), MakeStr8Nt(dummyClassCode), true);
					}
					
					CliArgList javacCmd = ZEROED;
					javacCmd.pathSepChar = '/';
					javacCmd.rootDirPath = StrLit("../..");
					AddArgNt(&javacCmd, "-d \"[VAL]\"", ".");
					AddArgStr(&javacCmd, "-classpath \"[VAL]\"", androidJarPath);
					AddArgNt(&javacCmd, CLI_QUOTED_ARG, FILENAME_DUMMY_JAVA);
					RunCliProgramAndExitOnFailure(javacExe, "", &javacCmd, StrLit("Failed to compile " FILENAME_DUMMY_JAVA "!"));
					AssertFileExist(StrLit(FILENAME_DUMMY_CLASS), true);
					
					CliArgList d8Cmd = ZEROED;
					d8Cmd.pathSepChar = '/';
					d8Cmd.rootDirPath = StrLit("../..");
					AddArgStr(&d8Cmd, "--lib \"[VAL]\"", androidJarPath);
					AddArgNt(&d8Cmd, "--output \"[VAL]\"", "./");
					AddArgNt(&d8Cmd, CLI_QUOTED_ARG, FILENAME_DUMMY_CLASS);
					RunCliProgramAndExitOnFailure(d8Exe, "", &d8Cmd, StrLit("Failed to convert Dummy.class to classes.dex!"));
					AssertFileExist(StrLit(FILENAME_CLASSES_DEX), true);
				}
				
				PrintLine("Compiling %s...", FILENAME_ANDROID_RESOURCES_ZIP);
				CliArgList compileResCmd = ZEROED;
				compileResCmd.pathSepChar = '/';
				compileResCmd.rootDirPath = StrLit("../..");
				AddArg(&compileResCmd, "compile");
				AddArgNt(&compileResCmd, "--dir \"[VAL]\"", "[ROOT]/tests/android/res");
				AddArgNt(&compileResCmd, "-o \"[VAL]\"", FILENAME_ANDROID_RESOURCES_ZIP);
				RunCliProgramAndExitOnFailure(aaptExe, "", &compileResCmd, StrLit("Failed to compile " FILENAME_ANDROID_RESOURCES_ZIP "!"));
				AssertFileExist(StrLit(FILENAME_ANDROID_RESOURCES_ZIP), true);
				
				RemoveFile(StrLit(FILENAME_TESTS_APK));
				PrintLine("Linking %s...", FILENAME_TESTS_APK);
				CliArgList linkApkCmd = ZEROED;
				linkApkCmd.pathSepChar = '/';
				linkApkCmd.rootDirPath = StrLit("../..");
				AddArg(&linkApkCmd, "link");
				AddArgNt(&linkApkCmd, "-o \"[VAL]\"", FILENAME_TESTS_APK);
				AddArgStr(&linkApkCmd, "-I \"[VAL]\"", androidJarPath);
				AddArgNt(&linkApkCmd, "-0 [VAL]", "resources.arsc");
				AddArgNt(&linkApkCmd, "--manifest \"[VAL]\"", "[ROOT]/tests/android/AndroidManifest.xml");
				AddArgNt(&linkApkCmd, CLI_QUOTED_ARG, FILENAME_ANDROID_RESOURCES_ZIP);
				RunCliProgramAndExitOnFailure(aaptExe, "", &linkApkCmd, StrLit("Failed to link " FILENAME_TESTS_APK "!"));
				AssertFileExist(StrLit(FILENAME_TESTS_APK), true);
				
				//NOTE: In order to insert our .so files into the apk, we need to unpack it into a folder, add the .so files manually, and then repack it
				{
					PrintLine("Inserting %s files (and " FILENAME_CLASSES_DEX ") into apk...", FILENAME_TESTS_SO);
					MyRemoveDirectory(StrLit("apk_temp"), true);
					mkdir("apk_temp", FOLDER_PERMISSIONS);
					chdir("apk_temp");
					
					CliArgList unpackApkCmd = ZEROED;
					AddArg(&unpackApkCmd, "xf");
					AddArg(&unpackApkCmd, "../" FILENAME_TESTS_APK);
					RunCliProgramAndExitOnFailure(StrLit("jar"), "", &unpackApkCmd, StrLit("Failed to unpack " FILENAME_TESTS_APK "!"));
					
					CopyFileToFolder(StrLit("../" FILENAME_CLASSES_DEX), StrLit("./"), true);
					
					mkdir("lib", FOLDER_PERMISSIONS);
					for (u64 archIndex = 1; archIndex < AndroidTargetArchitechture_Count; archIndex++)
					{
						AndroidTargetArchitechture architecture = (AndroidTargetArchitechture)archIndex;
						Str8 apkFolder = JoinStrings2(StrLit("lib/"), MakeStr8Nt(GetAndroidTargetArchitechtureFolderName(architecture)), true);
						Str8 buildFolder = JoinStrings2(StrLit("../lib/"), MakeStr8Nt(GetAndroidTargetArchitechtureFolderName(architecture)), true);
						mkdir(apkFolder.chars, FOLDER_PERMISSIONS);
						CopyFileToFolder(JoinStrings2(buildFolder, StrLit("/" FILENAME_TESTS_SO), false), apkFolder, true);
					}
					
					CliArgList repackApkCmd = ZEROED;
					AddArg(&repackApkCmd, "cf0");
					AddArg(&repackApkCmd, "../" FILENAME_TESTS_APK);
					AddArg(&repackApkCmd, "*");
					RunCliProgramAndExitOnFailure(StrLit("jar"), "", &repackApkCmd, StrLit("Failed to repack " FILENAME_TESTS_APK "!"));
					
					chdir("..");
					MyRemoveDirectory(StrLit("apk_temp"), true);
				}
				
				WriteLine("Performing ZIP alignment...");
				Str8 tempAlignedApkName = StrLit("tests_aligned.apk");
				RemoveFile(tempAlignedApkName);
				CliArgList alignApkCmd = ZEROED;
				AddArg(&alignApkCmd, "-v");
				AddArg(&alignApkCmd, "4");
				AddArgNt(&alignApkCmd, CLI_QUOTED_ARG, FILENAME_TESTS_APK); //input
				AddArgStr(&alignApkCmd, CLI_QUOTED_ARG, tempAlignedApkName); //output
				RunCliProgramAndExitOnFailure(zipalignExe, "", &alignApkCmd, StrLit("Failed to ZIP align " FILENAME_TESTS_APK "!"));
				AssertFileExist(tempAlignedApkName, true);
				CopyFileToPath(tempAlignedApkName, StrLit(FILENAME_TESTS_APK), true);
				RemoveFile(tempAlignedApkName);
				
				PrintLine("Signing %s with %.*s...", FILENAME_TESTS_APK, StrPrint(ANDROID_SIGNING_KEY_PATH));
				CliArgList signApkCmd = ZEROED;
				signApkCmd.pathSepChar = '/';
				signApkCmd.rootDirPath = StrLit("../..");
				AddArg(&signApkCmd, "sign");
				AddArgStr(&signApkCmd, "--ks \"[VAL]\"", ANDROID_SIGNING_KEY_PATH);
				if (ANDROID_SIGNING_PASSWORD.length > 0) { AddArgStr(&signApkCmd, "--ks-pass pass:[VAL]", ANDROID_SIGNING_PASSWORD); }
				else if (ANDROID_SIGNING_PASS_PATH.length > 0) { AddArgStr(&signApkCmd, "--ks-pass file:[VAL]", ANDROID_SIGNING_PASS_PATH); }
				else { WriteLine_E("You must provide either a ANDROID_SIGNING_PASSWORD or ANDROID_SIGNING_PASS_PATH in order to sign an Android .apk!"); exit(4); }
				AddArgNt(&signApkCmd, CLI_QUOTED_ARG, FILENAME_TESTS_APK);
				RunCliProgramAndExitOnFailure(apksignerExe, "", &signApkCmd, StrLit("Failed to sign " FILENAME_TESTS_APK "!"));
			}
			
			PrintLine("[Built %s for Android!]", BUILD_ANDROID_APK ? FILENAME_TESTS_APK : FILENAME_TESTS_SO);
			chdir("..");
		}
		
		// +==============================+
		// |          Orca tests          |
		// +==============================+
		if (BUILD_ORCA)
		{
			PrintLine("\n[Building %s for Orca...]", FILENAME_MODULE_WASM);
			
			mkdir(FOLDERNAME_ORCA, FOLDER_PERMISSIONS);
			chdir(FOLDERNAME_ORCA);
			
			CliArgList cmd = ZEROED;
			cmd.rootDirPath = StrLit("../..");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, FILENAME_MODULE_WASM);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/tests/tests_main.c");
			AddArgList(&cmd, &pigCoreCompilerFlags);
			AddArgList(&cmd, &pigCoreLinkerFlags);
			AddArgList(&cmd, &thingsToLink);
			
			StrArray tags = ZEROED;
			AddStrArray(&tags, &testsTags);
			AddStr(&tags, StrLit("clang"));
			AddStr(&tags, StrLit("Orca"));
			AddStr(&tags, StrLit("Wasm"));
			AddStrArray(&tags, &buildConfigTags);
			
			RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_CLANG), &tags, &cmd, StrLit("Failed to build " FILENAME_MODULE_WASM "!"));
			AssertFileExist(StrLit(FILENAME_MODULE_WASM), true);
			PrintLine("[Built %s for Orca!]", FILENAME_MODULE_WASM);
			
			CliArgList bundleCmd = ZEROED;
			AddArg(&bundleCmd, "bundle");
			AddArgNt(&bundleCmd, "--name [VAL]", "tests");
			AddArg(&bundleCmd, FILENAME_MODULE_WASM);
			RunCliProgramAndExitOnFailure(StrLit("orca"), "", &bundleCmd, StrLit("Failed to bundle " FILENAME_MODULE_WASM "!"));
			PrintLine("[Bundled %s into \"tests\" app!]", FILENAME_MODULE_WASM);
			
			chdir("..");
		}
		
		// +==============================+
		// |    Playdate Device tests     |
		// +==============================+
		if (BUILD_PLAYDATE_DEVICE)
		{
			PrintLine("\n[Building %s for Playdate...]", FILENAME_PDEX_ELF);
			
			CliArgList compileCmd = ZEROED;
			AddArg(&compileCmd, GCC_COMPILE);
			AddArgNt(&compileCmd, CLI_QUOTED_ARG, "[ROOT]/tests/tests_main.c");
			AddArgNt(&compileCmd, GCC_OUTPUT_FILE, FILENAME_TESTS_OBJ);
			AddArgList(&compileCmd, &pigCoreCompilerFlags);
			
			StrArray compileTags = ZEROED;
			AddStrArray(&compileTags, &testsTags);
			AddStr(&compileTags, StrLit(EXE_ARM_GCC));
			AddStr(&compileTags, StrLit("Playdate"));
			AddStr(&compileTags, StrLit("Device"));
			AddStrArray(&compileTags, &buildConfigTags);
			
			RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_ARM_GCC), &compileTags, &compileCmd, StrLit("Failed to build " FILENAME_TESTS_OBJ "!"));
			AssertFileExist(StrLit(FILENAME_TESTS_OBJ), true);
			
			CliArgList linkCmd = ZEROED;
			AddArgNt(&linkCmd, CLI_QUOTED_ARG, FILENAME_TESTS_OBJ);
			AddArgNt(&linkCmd, GCC_OUTPUT_FILE, FILENAME_PDEX_ELF);
			AddArgNt(&linkCmd, GCC_MAP_FILE, "tests.map");
			AddArgList(&linkCmd, &pigCoreLinkerFlags);
			AddArgList(&linkCmd, &thingsToLink);
			
			StrArray linkTags = ZEROED;
			AddStrArray(&linkTags, &testsTags);
			AddStr(&linkTags, StrLit(EXE_ARM_GCC));
			AddStr(&linkTags, StrLit("Playdate"));
			AddStr(&linkTags, StrLit("Device"));
			AddStrArray(&linkTags, &buildConfigTags);
			
			RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_ARM_GCC), &linkTags, &linkCmd, StrLit("Failed to build " FILENAME_PDEX_ELF "!"));
			AssertFileExist(StrLit(FILENAME_PDEX_ELF), true);
			PrintLine("[Built %s for Playdate!]", FILENAME_PDEX_ELF);
			
			mkdir("playdate_data", FOLDER_PERMISSIONS);
			CopyFileToFolder(StrLit(FILENAME_PDEX_ELF), StrLit("playdate_data"), true);
		}
		
		// +==============================+
		// |   Playdate Simulator tests   |
		// +==============================+
		if (BUILD_PLAYDATE_SIMULATOR)
		{
			InitializeMsvcIf(StrLit(".."), &isMsvcInitialized);
			PrintLine("\n[Building %s for Playdate Simulator...]", FILENAME_PDEX_DLL);
			
			CliArgList compileCmd = ZEROED;
			AddArg(&compileCmd, CL_COMPILE);
			AddArgNt(&compileCmd, CLI_QUOTED_ARG, "[ROOT]/tests/tests_main.c");
			AddArgNt(&compileCmd, CL_OBJ_FILE, FILENAME_TESTS_OBJ);
			AddArgList(&compileCmd, &pigCoreCompilerFlags);
			
			StrArray compileTags = ZEROED;
			AddStrArray(&compileTags, &testsTags);
			AddStr(&compileTags, StrLit("cl"));
			AddStr(&compileTags, StrLit("Playdate"));
			AddStr(&compileTags, StrLit("Simulator"));
			AddStrArray(&compileTags, &buildConfigTags);
			
			RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_MSVC_CL), &compileTags, &compileCmd, StrLit("Failed to build " FILENAME_TESTS_OBJ "!"));
			AssertFileExist(StrLit(FILENAME_TESTS_OBJ), true);
			
			CliArgList linkCmd = ZEROED;
			AddArg(&linkCmd, LINK_BUILD_DLL);
			AddArgNt(&linkCmd, CLI_QUOTED_ARG, FILENAME_TESTS_OBJ);
			AddArgNt(&linkCmd, LINK_OUTPUT_FILE, FILENAME_PDEX_DLL);
			AddArgNt(&linkCmd, LINK_IMPORT_LIBRARY_FILE, "tests.lib"); //TODO: Do we actually need to generate this?
			AddArgNt(&linkCmd, LINK_DEBUG_INFO_FILE, "tests.pdb");
			AddArgList(&linkCmd, &pigCoreLinkerFlags);
			AddArgList(&linkCmd, &thingsToLink);
			
			StrArray linkTags = ZEROED;
			AddStrArray(&linkTags, &testsTags);
			AddStr(&linkTags, StrLit(EXE_MSVC_LINK));
			AddStr(&linkTags, StrLit("Playdate"));
			AddStr(&linkTags, StrLit("Simulator"));
			AddStrArray(&linkTags, &buildConfigTags);
			
			RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_MSVC_LINK), &linkTags, &linkCmd, StrLit("Failed to build " FILENAME_PDEX_DLL "!"));
			AssertFileExist(StrLit(FILENAME_PDEX_DLL), true);
			PrintLine("[Built %s for Playdate Simulator!]", FILENAME_PDEX_DLL);
			
			mkdir("playdate_data", FOLDER_PERMISSIONS);
			CopyFileToFolder(StrLit(FILENAME_PDEX_DLL), StrLit("playdate_data"), true);
		}
		
		// +==============================+
		// |    Playdate Package tests    |
		// +==============================+
		if (BUILD_PLAYDATE_DEVICE || BUILD_PLAYDATE_SIMULATOR)
		{
			CopyFileToFolder(StrLit("../pdxinfo"), StrLit("playdate_data"), true);
			
			CliArgList cmd = ZEROED;
			AddArgList(&cmd, &pigCoreCompilerFlags);
			AddArgList(&cmd, &pigCoreLinkerFlags);
			AddArgList(&cmd, &thingsToLink);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "playdate_data");
			AddArgNt(&cmd, CLI_QUOTED_ARG, FILENAME_TESTS_PDX);
			
			StrArray tags = ZEROED;
			AddStrArray(&tags, &testsTags);
			AddStr(&tags, StrLit(EXE_PDC));
			AddStr(&tags, StrLit("Playdate"));
			AddStrArray(&tags, &buildConfigTags);
			
			RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_PDC), &tags, &cmd, StrLit("Failed to package " FILENAME_TESTS_PDX "!"));
			AssertFileExist(StrLit(FILENAME_TESTS_PDX), true); //TODO: Is this going to work on a folder?
			PrintLine("[Packaged %s for Playdate!]", FILENAME_TESTS_PDX);
		}
	}
	
	// +--------------------------------------------------------------+
	// |                Generate compile_commands.json                |
	// +--------------------------------------------------------------+
	if (GENERATE_COMPILE_COMMANDS_FOR_LSP)
	{
		//TODO: Fill out a compile_commands.json file with all the arguments that we pass to clang to compile tests
		//Either:
		//		TODO: Create a python virtual environment in build/venv if it doesn't already exist. Install compdb inside that environment with `./venv/bin/pip install compdb`
		//		TODO: Run ./venv/bin/compdb -p . list > full_compile_commands.json
		//		TODO: Rename compile_commands.json to base_compile_commands.json and rename full_compile_commands.json to compile_commands.json
		//Or:
		//		TODO: Fill out a complete compile_commands.json that has info for every single header file so that clangd can provide us completions in header files properly
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
		RunCliProgramAndExitOnFailure(StrLit(EXEC_PROGRAM_IN_FOLDER_PREFIX RUNNABLE_FILENAME_TESTS), "", &cmd, StrLit(RUNNABLE_FILENAME_TESTS " Exited With Error!"));
	}
	
	if (INSTALL_TESTS_APK)
	{
		PrintLine("\n[Installing %s on AVD...]", FILENAME_TESTS_APK);
		Str8 adbExe = JoinStrings2(androidSdkDir, StrLit("/platform-tools/adb.exe"), false);
		
		CliArgList installCmd = ZEROED;
		installCmd.pathSepChar = '/';
		AddArgNt(&installCmd, "install \"[VAL]\"", FOLDERNAME_ANDROID "/" FILENAME_TESTS_APK);
		RunCliProgramAndExitOnFailure(adbExe, "", &installCmd, StrLit("abd.exe install exited With Error!"));
		
		PrintLine_E("Launching \"%.*s\"...", StrPrint(ANDROID_ACTIVITY_PATH));
		CliArgList launchCmd = ZEROED;
		launchCmd.pathSepChar = '/';
		AddArg(&launchCmd, "shell");
		AddArg(&launchCmd, "am");
		AddArg(&launchCmd, "start");
		AddArgStr(&launchCmd, "-n \"[VAL]\"", ANDROID_ACTIVITY_PATH);
		RunCliProgramAndExitOnFailure(adbExe, "", &launchCmd, StrLit("abd.exe shell exited With Error!"));
	}
	
	PrintLine("\n[%s Finished Successfully]", BUILD_SCRIPT_EXE_NAME);
	return 0;
}
