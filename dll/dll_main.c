/*
File:   dll_main.c
Author: Taylor Robbins
Date:   01\19\2025
Description: 
	** This main file is used when compiling PigCore as a dynamic library (.dll)
	** It will include all files that can be used from PigCore as a library except
	** for things explicitly excluded by #defines
*/

#define PIG_CORE_BUILDING_AS_DLL 1

#include "build_config.h"

#ifndef PIG_CORE_DLL_INCLUDE_BASE
#define PIG_CORE_DLL_INCLUDE_BASE 1
#endif
#ifndef PIG_CORE_DLL_INCLUDE_STD
#define PIG_CORE_DLL_INCLUDE_STD 1
#endif
#ifndef PIG_CORE_DLL_INCLUDE_OS
#define PIG_CORE_DLL_INCLUDE_OS 1
#endif
#ifndef PIG_CORE_DLL_INCLUDE_MISC
#define PIG_CORE_DLL_INCLUDE_MISC 1
#endif
#ifndef PIG_CORE_DLL_INCLUDE_MEM
#define PIG_CORE_DLL_INCLUDE_MEM 1
#endif
#ifndef PIG_CORE_DLL_INCLUDE_STRUCT
#define PIG_CORE_DLL_INCLUDE_STRUCT 1
#endif
#ifndef PIG_CORE_DLL_INCLUDE_INPUT
#define PIG_CORE_DLL_INCLUDE_INPUT 1
#endif
#ifndef PIG_CORE_DLL_INCLUDE_FILE_FMT
#define PIG_CORE_DLL_INCLUDE_FILE_FMT 1
#endif
#ifndef PIG_CORE_DLL_INCLUDE_UI
#define PIG_CORE_DLL_INCLUDE_UI 1
#endif
#ifndef PIG_CORE_DLL_INCLUDE_GFX
#define PIG_CORE_DLL_INCLUDE_GFX 1
#endif
#ifndef PIG_CORE_DLL_INCLUDE_GFX_SYSTEM_GLOBAL
#define PIG_CORE_DLL_INCLUDE_GFX_SYSTEM_GLOBAL 0
#endif
#ifndef PIG_CORE_DLL_INCLUDE_PHYS
#define PIG_CORE_DLL_INCLUDE_PHYS 1
#endif
#ifndef PIG_CORE_DLL_INCLUDE_PARSE
#define PIG_CORE_DLL_INCLUDE_PARSE 1
#endif

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"

#if BUILD_WITH_RAYLIB
#include "third_party/raylib/raylib.h"
#endif

#if PIG_CORE_DLL_INCLUDE_BASE
#include "base/base_all.h"
#endif

#include "base/base_debug_output_impl.h"

#if PIG_CORE_DLL_INCLUDE_BASE
#include "base/base_all.h"
#endif

#if PIG_CORE_DLL_INCLUDE_BASE
#include "base/base_all.h"
#endif

#if PIG_CORE_DLL_INCLUDE_STD
#include "std/std_all.h"
#endif

#if PIG_CORE_DLL_INCLUDE_OS
#include "os/os_all.h"
#endif

#if PIG_CORE_DLL_INCLUDE_MISC
#include "misc/misc_all.h"
#endif

#if PIG_CORE_DLL_INCLUDE_MEM
#include "mem/mem_all.h"
#endif

#if PIG_CORE_DLL_INCLUDE_STRUCT
#include "struct/struct_all.h"
#endif

#if PIG_CORE_DLL_INCLUDE_INPUT
#include "input/input_all.h"
#endif

#if PIG_CORE_DLL_INCLUDE_FILE_FMT
#include "file_fmt/file_fmt_all.h"
#endif

#if PIG_CORE_DLL_INCLUDE_UI
#include "ui/ui_all.h"
#endif

#if PIG_CORE_DLL_INCLUDE_GFX
#include "gfx/gfx_all.h"
#endif

#if PIG_CORE_DLL_INCLUDE_GFX_SYSTEM_GLOBAL
#include "gfx/gfx_system_global.h"
#endif

#if PIG_CORE_DLL_INCLUDE_PHYS
#include "phys/phys_all.h"
#endif

#if PIG_CORE_DLL_INCLUDE_PARSE
#include "parse/parse_all.h"
#endif

//TODO: Do we want a DllMain for any reason?
