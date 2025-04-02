/*
File:   piggen_main.c
Author: Taylor Robbins
Date:   12\31\2024
Description: 
	** Holds the main entry point for piggen.exe which is a tool that scrapes C(++)
	** files, searching for special syntax, in order to generate code and modify
	** the source file(s) to #include the generated code
*/

#include "build_config.h"
#if !BUILD_FOR_PIGGEN
#error The wrong build_config.h was found!
#endif

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "std/std_includes.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "misc/misc_result.h"
#include "os/os_file.h"
#include "os/os_virtual_mem.h"
#include "std/std_memset.h"
#include "std/std_malloc.h"
#include "std/std_basic_math.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "base/base_unicode.h"
#include "struct/struct_string.h"
#include "os/os_path.h"
#include "misc/misc_parsing.h"
#include "os/os_program_args.h"

#include "base/base_debug_output_impl.h"

int main()
{
	InitScratchArenasVirtual(Gigabytes(4));
	ScratchBegin(scratch);
	ScratchBegin1(scratch2, scratch);
	ScratchBegin2(scratch3, scratch, scratch2);
	
	Print_N("Running piggen...");
	// getchar(); //wait for user to press ENTER
	PrintLine_N("DONE!");
	
	ScratchEnd(scratch);
	ScratchEnd(scratch2);
	ScratchEnd(scratch3);
	return 0;
}
