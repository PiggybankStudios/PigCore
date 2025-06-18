/*
File:   tools_msvc_flags.h
Author: Taylor Robbins
Date:   06\16\2025
Description:
	** Contains defines for all the MSVC compiler\linker flags that we may use in pig_build.exe
*/

#ifndef _TOOLS_MSVC_FLAGS_H
#define _TOOLS_MSVC_FLAGS_H

// +==============================+
// |           CL Flags           |
// +==============================+
#define CL_COMPILE                      "/c"
#define CL_DEFINE                       "/D\"[VAL]\""
#define CL_OBJ_NAME                     "/Fo\"[VAL]\""
#define CL_BINARY_NAME                  "/Fe\"[VAL]\""
#define CL_LANG_VERSION                 "/std:[VAL]" //clatest, c++20
#define CL_EXPERIMENTAL                 "/experimental:[VAL]" //c11atomics
#define CL_DISABLE_WARNING              "/wd[VAL]"
#define CL_ENABLE_WARNING               "/we[VAL]" //5262
#define CL_FULL_FILE_PATHS              "/FC"
#define CL_WARNING_LEVEL                "/W[VAL]" //4, 3
#define CL_NO_LOGO                      "/nologo"
#define CL_WARNINGS_AS_ERRORS           "/WX"
#define CL_INCLUDE_DIR                  "/I\"[VAL]\""
#define CL_DEBUG_INFO                   "/Zi"
#define CL_STD_LIB_STATIC               "/MT"
#define CL_STD_LIB_DYNAMIC              "/MD"
#define CL_STD_LIB_STATIC_DBG           "/MTd"
#define CL_STD_LIB_DYNAMIC_DBG          "/MDd"
#define CL_OPTIMIZATION_LEVEL           "/O[VAL]" //d, b0, t, y, 2
#define CL_PRECOMPILE_ONLY              "/P"
#define CL_PRECOMPILE_PRESERVE_COMMENTS "/C"
#define CL_ENABLE_FUZZER                "/fsanitize=fuzzer"
#define CL_LINK                         "/link"

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
#define LINK_BUILD_DLL        "/DLL"

//TODO: Make #defines for these
#define LINK_NOT_INCREMENTAL  "-incremental:no"
#define LINK_LIBRARY_DIR      "/LIBPATH:\"[VAL]\""
// /GS /Gm- /RTC1 /Gd /WX-
// /errorReport:prompt /Zc:forScope /Zc:inline /Zc:wchar_t /fp:precise
// /MANIFEST /NXCOMPAT /DYNAMICBASE /DEBUG /MACHINE:X64 /INCREMENTAL /SUBSYSTEM:CONSOLE /ERRORREPORT:PROMPT /NOLOGO /TLBID:1
// /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /ManifestFile:"tests.intermediate.manifest" /LTCGOUT:"tests.iobj" /ILK:"tests.ilk"

#endif //  _TOOLS_MSVC_FLAGS_H
