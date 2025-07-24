/*
File:   tools_msvc_flags.h
Author: Taylor Robbins
Date:   06\16\2025
Description:
	** Contains defines for all the MSVC compiler\linker flags that we may use in our build scripts
*/

#ifndef _TOOLS_MSVC_FLAGS_H
#define _TOOLS_MSVC_FLAGS_H

#define EXE_MSVC_CL    "cl"
#define EXE_MSVC_LINK  "LINK"

// +==============================+
// |           CL Flags           |
// +==============================+
#define CL_COMPILE                          "/c"
#define CL_DEFINE                           "/D\"[VAL]\""
#define CL_OBJ_FILE                         "/Fo\"[VAL]\""
#define CL_BINARY_FILE                      "/Fe\"[VAL]\""
#define CL_PDB_FILE                         "/Fd\"[VAL]\""
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
#define CL_WARNING_SWITCH_ONLY_DEFAULT                   4065 //Switch statement contains 'default' but no 'case' labels
#define CL_WARNING_UNREFERENCED_FUNC_PARAMETER           4100 //Unreferenced formal parameter [W4]
#define CL_WARNING_UNREFERENCED_LCOAL_VARIABLE           4101 //Unreferenced local variable [W3]
#define CL_WARNING_CONDITIONAL_EXPR_IS_CONSTANT          4127 //Conditional expression is constant [W4]
#define CL_WARNING_LOCAL_VAR_INIT_BUT_NOT_REFERENCED     4189 //Local variable is initialized but not referenced [W4]
#define CL_WARNING_UNREACHABLE_CODE_DETECTED             4702 //Unreachable code [W4]
#define CL_WARNING_SWITCH_FALLTHROUGH                    5262 //Enable the [[fallthrough]] missing warning

// +==============================+
// |          LINK Flags          |
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

#endif //  _TOOLS_MSVC_FLAGS_H
