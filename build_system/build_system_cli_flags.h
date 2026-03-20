/*
File:   build_system_cli_flags.h
Author: Taylor Robbins
Date:   06\16\2025
Description:
	** Contains a bunch of #defines that help us make flags for various CLI programs more readable in the build_script.c
	** Supported CLI Tools: msvc cl\link\rc, clang, gcc, emcc, shdc, pdc
*/

#ifndef _BUILD_SYSTEM_CLI_FLAGS_H
#define _BUILD_SYSTEM_CLI_FLAGS_H

// +--------------------------------------------------------------+
// |                   MSVC Compiler CLI Flags                    |
// +--------------------------------------------------------------+
#define EXE_MSVC_CL    "cl"
#define EXE_MSVC_LINK  "LINK"
#define EXE_MSVC_RC    "rc"

// +==============================+
// |        MSVC CL Flags         |
// +==============================+
#define CL_COMPILE                          "/c"
#define CL_DEFINE                           "/D\"[VAL]\""
#define CL_OBJ_FILE                         "/Fo\"[VAL]\""
#define CL_BINARY_FILE                      "/Fe\"[VAL]\""
#define CL_PDB_FILE                         "/Fd\"[VAL]\""
#define CL_GENERATE_ASSEMB_LISTING          "/FA[VAL]" //(assembly always included) c = include machine code, s = include source code, u = encode as UTF-8
#define CL_ASSEMB_LISTING_FILE              "/Fa\"[VAL]\""
#define CL_LANG_VERSION                     "/std:[VAL]" //clatest, c++20
#define CL_EXPERIMENTAL                     "/experimental:[VAL]" //c11atomics
#define CL_DISABLE_WARNING                  "/wd[VAL]"
#define CL_ENABLE_WARNING                   "/we[VAL]" //5262
#define CL_FULL_FILE_PATHS                  "/FC"
#define CL_WARNING_LEVEL                    "/W[VAL]" //4, 3
#define CL_NO_LOGO                          "/nologo" //Suppress the startup banner (the message about copyright and version info)
#define CL_WARNINGS_AS_ERRORS               "/WX"
#define CL_NO_WARNINGS_AS_ERRORS            "/WX-"
#define CL_INCLUDE_DIR                      "/I\"[VAL]\"" //Adds an include directory to search in when resolving #includes
#define CL_DEBUG_INFO                       "/Zi"
#define CL_STD_LIB_STATIC                   "/MT"
#define CL_STD_LIB_DYNAMIC                  "/MD"
#define CL_STD_LIB_STATIC_DBG               "/MTd"
#define CL_STD_LIB_DYNAMIC_DBG              "/MDd"
#define CL_OPTIMIZATION_LEVEL               "/O[VAL]" //t, y
#define CL_INLINE_EXPANSION_LEVEL           "/Ob[VAL]" //0, 1, 2, 3
#define CL_PRECOMPILE_ONLY                  "/P"
#define CL_PRECOMPILE_PRESERVE_COMMENTS     "/C"
#define CL_ENABLE_FUZZER                    "/fsanitize=fuzzer"
#define CL_ENABLE_ADDRESS_SANATIZER         "/fsanitize=address"
#define CL_ENABLE_BUFFER_SECURITY_CHECK     "/GS" //Detects some buffer overruns that overwrite a function's return address, exception handler address, or certain types of parameters
#define CL_DISABLE_MINIMAL_REBUILD          "/Gm-" //Disables minimal rebuild (a deprecated feature), which determines whether C++ source files that include changed C++ class definitions (stored in header (.h) files) need to be recompiled.
#define CL_ENABLE_RUNTIME_CHECKS            "/RTC[VAL]" //1 = Enable fast runtime checks (Equivalent to `/RTCsu`)
#define CL_CALLING_CONVENTION               "/G[VAL]" //Determines the order in which function arguments are pushed onto the stack, whether the caller function or called function removes the arguments from the stack at the end of the call, and the name-decorating convention that the compiler uses to identify individual functions
#define CL_ENABLE_LANG_CONFORMANCE_OPTION   "/Zc:[VAL]"
#define CL_DISABLE_LANG_CONFORMANCE_OPTION  "/Zc:[VAL]-"
#define CL_INTERNAL_COMPILER_ERROR_BEHAVIOR "/errorReport:[VAL]"
#define CL_FLOATING_POINT_MODEL             "/fp:[VAL]"
#define CL_CONFIGURE_EXCEPTION_HANDLING     "/EH[VAL]"
#define CL_LINK                             "/link"

#define CL_WARNING_ENUMERATION_MUST_HAVE_UNDERLYING_TYPE 4471 //a forward declaration of an unscoped enumeration must have an underlying type
#define CL_WARNING_BITWISE_OP_BETWEEN_ENUMS              5054 //operator '|': deprecated between enumerations of different types
#define CL_WARNING_LOGICAL_OP_ON_ADDRESS_OF_STR_CONST    4130 //Logical operation on address of string constant [W4] TODO: Should we re-enable this one? Don't know any scenarios where I want to do this
#define CL_WARNING_NAMELESS_STRUCT_OR_UNION              4201 //Nonstandard extension used: nameless struct/union [W4] TODO: Should we re-enable this restriction for ANSI compatibility?
#define CL_WARNING_STRUCT_WAS_PADDED                     4324 //Structure was padded due to __declspec[align[]] [W4]
#define CL_WARNING_DECLARATION_HIDES_CLASS_MEMBER        4458 //Declaration of 'identifier' hides class member [W4]
#define CL_WARNING_UNREFERENCED_FUNC_REMOVED             4505 //Unreferenced local function has been removed [W4]
#define CL_WARNING_USAGE_OF_DEPRECATED                   4996 //Usage of deprecated function, class member, variable, or typedef [W3]
#define CL_WARNING_ASSIGNMENT_WITHIN_CONDITIONAL_EXPR    4706 //assignment within conditional expression [W?]
#define CL_WARNING_NAMED_TYPEDEF_IN_PARENTHESES          4115 //named type definition in parentheses
#define CL_WARNING_SWITCH_ONLY_DEFAULT                   4065 //Switch statement contains 'default' but no 'case' labels
#define CL_WARNING_UNREFERENCED_FUNC_PARAMETER           4100 //Unreferenced formal parameter [W4]
#define CL_WARNING_UNREFERENCED_LCOAL_VARIABLE           4101 //Unreferenced local variable [W3]
#define CL_WARNING_CONDITIONAL_EXPR_IS_CONSTANT          4127 //Conditional expression is constant [W4]
#define CL_WARNING_LOCAL_VAR_INIT_BUT_NOT_REFERENCED     4189 //Local variable is initialized but not referenced [W4]
#define CL_WARNING_UNREACHABLE_CODE_DETECTED             4702 //Unreachable code [W4]
#define CL_WARNING_SWITCH_FALLTHROUGH                    5262 //Enable the [[fallthrough]] missing warning

// +==============================+
// |       MSVC LINK Flags        |
// +==============================+
#define LINK_BUILD_DLL                 "/DLL" //Makes a .dll file as the binary output (instead of .exe)
#define LINK_NO_LOGO                   "/NOLOGO" //Suppress the startup banner (the message about copyright and version info)
#define LINK_DEBUG_INFO                "/DEBUG"
#define LINK_OUTPUT_FILE               "/OUT:\"[VAL]\""
#define LINK_LIBRARY_DIR               "/LIBPATH:\"[VAL]\""
#define LINK_CREATE_ASSEMBLY_MANIFEST  "/MANIFEST" //Creates a side-by-side manifest file (XML) See https://learn.microsoft.com/en-us/windows/win32/SbsCs/manifest-files-reference
#define LINK_ASSEMBLY_MANIFEST_FILE    "/MANIFESTFILE:\"[VAL]\"" //Change the default name of the manifest file
#define LINK_DATA_EXEC_COMPAT          "/NXCOMPAT" //Indicates that an executable is compatible with the Windows Data Execution Prevention feature
#define LINK_ENABLE_ASLR               "/DYNAMICBASE" //Enables Address Space Layout Randomization
#define LINK_DISABLE_ASLR              "/DYNAMICBASE:NO" //Disables Address Space Layout Randomization
#define LINK_TARGET_ARCHITECTURE       "/MACHINE:[VAL]" //Specifies the target platform for the program (ARM, ARM64, ARM64EC, EBC, X64, X86)
#define LINK_ENABLE_INCREMENTAL        "/INCREMENTAL" //Link incrementally (default)
#define LINK_DISABLE_INCREMENTAL       "/INCREMENTAL:NO" //Always perform a full link
#define LINK_CONSOLE_APPLICATION       "/SUBSYSTEM:CONSOLE" //The program is meant for the console, the OS should provide a console on startup
#define LINK_WINDOWS_APPLICATION       "/SUBSYSTEM:WINDOWS" //The program is NOT meant for the console, the OS does not need to provide a console on startup
#define LINK_TYPELIB_RESOURCE_ID       "/TLBID:[VAL]" //When compiling a program that uses attributes, the linker will create a type library. By default the resource ID is 1, this option allows you to change that ID if 1 conflicts with one of your own resources
#define LINK_EMBED_UAC_INFO            "/MANIFESTUAC" //Specifies whether User Account Control (UAC) information is embedded in the program manifest
#define LINK_DONT_EMBED_UAC_INFO       "/MANIFESTUAC:NO"
#define LINK_EMBED_UAC_INFO_EX         "/MANIFESTUAC:\"[VAL]\"" //You can enter strings like "level='asInvoker' uiAccess='false'"
#define LINK_MANIFESTFILE              "/ManifestFile:\"[VAL]\"" //tests.intermediate.manifest
#define LINK_LINK_TIME_CODEGEN_FILE    "/LTCGOUT:\"[VAL]\"" //Specifies the filename for link-time code generation
#define LINK_INCREMENTAL_FILE_NAME     "/ILK:\"[VAL]\""
#define LINK_IMPORT_LIBRARY_FILE       "/IMPLIB:\"[VAL]\"" //Specify the name of the import library (the .lib that pairs with a .dll)
#define LINK_DEBUG_INFO_FILE           "/PDB:\"[VAL]\""

// +==============================+
// |        MSVC RC Flags         |
// +==============================+
#define RC_NO_LOGO     "/nologo"
#define RC_OUTPUT_FILE "/Fo\"[VAL]\""



// +--------------------------------------------------------------+
// |                       Clang CLI Flags                        |
// +--------------------------------------------------------------+
#define EXE_CLANG     "clang"
#define EXE_WSL_CLANG "wsl clang-18"

#define CLANG_COMPILE                "-c"
#define CLANG_DEFINE                 "-D \"[VAL]\""
#define CLANG_LANG_VERSION           "-std=[VAL]"
#define CLANG_OUTPUT_FILE            "-o \"[VAL]\""
#define CLANG_OPTIMIZATION_LEVEL     "-O[VAL]"
#define CLANG_DEBUG_INFO             "-g[VAL]"
#define CLANG_DEBUG_INFO_DEFAULT     "-g"
#define CLANG_WARNING_LEVEL          "-W[VAL]"
#define CLANG_ENABLE_WARNING         "-W[VAL]"
#define CLANG_DISABLE_WARNING        "-Wno-[VAL]"
#define CLANG_FULL_FILE_PATHS        "-fdiagnostics-absolute-paths" //Print absolute paths in diagnostics TODO: Figure out how to resolve these back to windows paths for Sublime error linking?
#define CLANG_INCLUDE_DIR            "-I \"[VAL]\"" //Adds an include directory to search in when resolving #includes
#define CLANG_LIBRARY_DIR            "-L \"[VAL]\""
#define CLANG_RPATH_DIR              "-rpath \"[VAL]\""
#define CLANG_SYSTEM_LIBRARY         "-l[VAL]"
#define CLANG_FRAMEWORK              "-framework [VAL]"
#define CLANG_PRECOMPILE_ONLY        "-E"
#define CLANG_INCLUDE_MACROS         "-dD" //Only matters when -E is passed
#define CLANG_Q_FLAG                 "-Q[VAL]" //TODO: Name this better!
#define CLANG_BUILD_SHARED_LIB       "-shared"
#define CLANG_fPIC                   "-fPIC" //TODO: Name this better!
#define CLANG_ENABLE_OBJC_ARC        "-fobjc-arc" // Enables Automatic Reference Counting (ARC) feature in Objective-C
#define CLANG_STDLIB_FOLDER          "--sysroot \"[VAL]\""
#define CLANG_M_FLAG                 "-m[VAL]" //TODO: Name this better!
#define CLANG_TARGET_ARCHITECTURE    "--target=[VAL]"
#define CLANG_LINKER_ARG             "-Xlinker [VAL]" //TODO: Name this better!
//NOTE: Arguments prefixed with -Wl, are passed along to the linker
#define CLANG_NO_ENTRYPOINT          "-Wl,--no-entry"
#define CLANG_EXPORT_DYNAMIC         "-Wl,--export-dynamic"
#define CLANG_EXPORT_SYMBOL          "-Wl,--export=[VAL]"
#define CLANG_ALLOW_UNDEFINED        "-Wl,--allow-undefined"
#define CLANG_NO_UNDEFINED           "-Wl,--no-undefined"
#define CLANG_NO_UNDEFINED_VERSION   "-Wl,--no-undefined-version"
#define CLANG_FATAL_WARNINGS         "-Wl,--fatal-warnings"
#define CLANG_BUILD_ID               "-Wl,--build-id=[VAL]" //TODO: Better name? Description?
#define CLANG_MAX_PAGE_SIZE          "-Wl,-z,max-page-size=[VAL]"
#define CLANG_LIB_SO_NAME            "-Wl,-soname,[VAL]"
#define CLANG_NO_STD_LIBRARIES       "--no-standard-libraries"
#define CLANG_NO_STD_INCLUDES        "--no-standard-includes"
#define CLANG_NO_CANONICAL_PREFIXES  "-no-canonical-prefixes" //TODO: Better name? Description?
#define CLANG_RPATH_DIR              "-rpath \"[VAL]\""
#define CLANG_DATA_SECTIONS          "-fdata-sections" //TODO: Better name? Decription?
#define CLANG_FUNCTION_SECTIONS      "-ffunction-sections" //TODO: Better name? Decription?
#define CLANG_UNWIND_TABLES          "-funwind-tables" //TODO: Better name? Decription?
#define CLANG_STACK_PROTECTOR_STRONG "-fstack-protector-strong" //TODO: Better name? Decription?
#define CLANG_NO_STDLIB_CPP          "-nostdlib++"

#define CLANG_WARNING_SWITCH_MISSING_CASES          "switch" //8 enumeration values not handled in switch: 'ArenaType_None', 'ArenaType_Funcs', 'ArenaType_Generic'...
#define CLANG_WARNING_UNUSED_FUNCTION               "unused-function" //unused function 'MeowExpandSeed'
#define CLANG_WARNING_UNUSED_CMD_LINE_ARG           "unused-command-line-argument" //argument unused during compilation: '-L../third_party/_lib_debug'
#define CLANG_WARNING_SHADOWING                     "shadow" //Warn whenever a local variable or type declaration shadows another variable, parameter, type, class member (in C++), or instance variable (in Objective-C) or whenever a built-in function is shadowed
#define CLANG_WARNING_MISSING_FALLTHROUGH_IN_SWITCH "implicit-fallthrough" //Must use [[fallthrough]] on a case label that falls through to the next case
#define CLANG_WARNING_MISSING_FIELD_INITIALIZERS    "missing-field-initializers" //Warn when curly bracket initializers don't contain values for all fields



// +--------------------------------------------------------------+
// |                        GCC CLI Flags                         |
// +--------------------------------------------------------------+
#define EXE_ARM_GCC "arm-none-eabi-gcc" //we use this when compiling for the Playdate device

#define GCC_COMPILE                  "-c"
#define GCC_PRECOMPILE_ONLY          "-E"
#define GCC_OUTPUT_FILE              "-o \"[VAL]\""
#define GCC_DEFINE                   "-D \"[VAL]\""
#define GCC_INCLUDE_DIR              "-I \"[VAL]\""
#define GCC_DEBUG_INFO_EX            "-g[VAL]" //Produce debug information in the OS's native format
#define GCC_STD_LIB_STATIC           "-MT" //MSVC-style option
#define GCC_STD_LIB_DYNAMIC          "-MD" //MSVC-style option
#define GCC_STD_LIB_STATIC_DBG       "-MTd" //MSVC-style option
#define GCC_STD_LIB_DYNAMIC_DBG      "-MDd" //MSVC-style option
#define GCC_WARNING_LEVEL            "-W[VAL]"
#define GCC_ENABLE_WARNING           "-W[VAL]"
#define GCC_DISABLE_WARNING          "-Wno-[VAL]"
#define GCC_TARGET_THUMB             "-mthumb" //Requests that the compiler targets the T32 (Thumb) instruction set instead of A32 (Arm)
#define GCC_TARGET_CPU               "-mcpu=[VAL]" //Specify a specific CPU to target during code generation
#define GCC_FLOAT_ABI_MODE           "-mfloat-abi=[VAL]" //Whether to use hardware instructions for floating-point operations
#define GCC_TARGET_FPU               "-mfpu=[VAL]" //Specifies the target FPU architecture, that is the floating-point hardware available on the target
#define GCC_ONLY_RELOC_WORD_SIZE     "-mword-relocations" //Only generate absolute relocations on word-sized values (ARM specific option?)
#define GCC_USE_SPEC_FILE            "-specs=[VAL]" //Specify a file to process after the compiler reads in the standard "specs" file. Determines what switches to pass to cc1, cc1plus, as, ld, etc. These are likely located somewhere like [gcc_install_dir]/lib/[file_name].specs
#define GCC_DEPENDENCY_FILE          "-MF \"[VAL]\""
#define GCC_VERBOSE_ASSEMBLY         "-fverbose-asm" //Put extra commentary information in the generated assembly code to make it more readable
#define GCC_GLOBAL_VAR_NO_COMMON     "-fno-common" //Specifies that the compiler places uninitialized global variables in the BSS section of the object files
#define GCC_ALIGN_FUNCS_TO           "-falign-functions=[VAL]" //Align the start of functions to a power of 2 >= the given value
#define GCC_SEP_DATA_SECTIONS        "-fdata-sections" //Place each data into its own section in the output file if the target supports arbitrary sections
#define GCC_SEP_FUNC_SECTIONS        "-ffunction-sections" //Place each function into its own section in the output file if the target supports arbitrary sections
#define GCC_DISABLE_EXCEPTIONS       "-fno-exceptions" //Disable exception handling, which means we don't need to generate code to propagate exceptions up the call stack
#define GCC_OMIT_FRAME_PNTR          "-fomit-frame-pointer" //Omit the frame pointer in functions that don’t need one. This avoids the instructions to save, set up and restore the frame pointer; on many targets it also makes an extra register available
#define GCC_LINKER_SCRIPT            "-T\"[VAL]\"" //Specify the path to a linker script to use (.ld file)
#define GCC_NO_STD_STARTUP           "-nostartfiles" //Do not use the standard system startup files when linking
#define GCC_ENTRYPOINT_NAME          "--entry=[VAL]" //Specify a function name to use as the entrypoint for the application
//NOTE: Arguments prefixed with -Wl, are passed along to the linker
#define GCC_DISABLE_RWX_WARNING      "-Wl,--no-warn-rwx-segments" //Disables an annoying warning about executable+writable sections which comes from choices in the linker script which we don't control
#define GCC_CREF                     "-Wl,--cref" //TODO: Find the documentation for this option and give this define a better name!
#define GCC_GC_SECTIONS              "-Wl,--gc-sections" //TODO: Find the documentation for this option and give this define a better name!
#define GCC_DISABLE_MISMATCH_WARNING "-Wl,--no-warn-mismatch" //Do not warn about mismatching input files? TODO: Find the documentation for this option and give this define a better name!
#define GCC_EMIT_RELOCATIONS         "-Wl,--emit-relocs" //TODO: Find the documentation for this option and give this define a better name!
#define GCC_MAP_FILE                 "-Wl,-Map=\"[VAL]\""



// +--------------------------------------------------------------+
// |                     Emscripten CLI Flags                     |
// +--------------------------------------------------------------+
#define EXE_EMSCRIPTEN_COMPILER "emcc"
//NOTE: Emscripten's compiler accepts all the same flags that Clang does
#define EMSCRIPTEN_S_FLAG "-s[VAL]" //TODO: Give this a better name!



// +--------------------------------------------------------------+
// |          Sokol Shader Cross-Compiler SHDC CLI Flags          |
// +--------------------------------------------------------------+
#if BUILDING_ON_WINDOWS
#define EXE_SHDC        "third_party/_tools/win32/sokol-shdc.exe"
#define EXE_SHDC_NAME   "sokol-shdc.exe"
#elif BUILDING_ON_LINUX
#define EXE_SHDC        "third_party/_tools/linux/sokol-shdc"
#define EXE_SHDC_NAME   "sokol-shdc"
#elif BUILDING_ON_OSX
#define EXE_SHDC        "third_party/_tools/osx/sokol-shdc"
#define EXE_SHDC_NAME   "sokol-shdc"
#endif

#define SHDC_FORMAT           "--format=[VAL]"
#define SHDC_ERROR_FORMAT     "--errfmt=[VAL]"
#define SHDC_REFLECTION       "--reflection"
#define SHDC_SHADER_LANGUAGES "--slang=[VAL]"
#define SHDC_INPUT            "--input=\"[VAL]\""
#define SHDC_OUTPUT           "--output=\"[VAL]\""



// +--------------------------------------------------------------+
// |               Playdate Packager PDC CLI Flags                |
// +--------------------------------------------------------------+
#define PDC_SDK_PATH      "-sdkpath \"[VAL]\"" //use the SDK at the given path instead of the default
#define PDC_LIBPATH       "--libpath \"[VAL]\"" //add the given path to the list of folders to search when resolving imports
#define PDC_STRIP         "--strip" //strip debug symbols
#define PDC_NO_COMPRESS   "--no-compress" //don't compress output files
#define PDC_MAIN          "--main" //compile lua script at <input> as if it were main.lua
#define PDC_VERBOSE       "--verbose" //verbose mode, gives info about what the compiler is doing
#define PDC_QUIET         "--quiet" //quiet mode, suppresses non-error output
#define PDC_SKIP_UNKNOWN  "--skip-unknown" //skip unrecognized files instead of copying them to the pdx folder
#define PDC_CHECK_FONTS   "--check-fonts" //perform additional validation on font data (may produce false warnings)

#endif //  _BUILD_SYSTEM_CLI_FLAGS_H
