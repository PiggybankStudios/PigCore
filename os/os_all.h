/*
File:   os_all.h
Author: Taylor Robbins
Date:   01\04\2025
Description:
	** Includes all files in this folder, meant as a quick shorthand for applications that just want everything from this folder available
	** This file also serves as a reference for the order of dependencies between files in this folder.
	** This file also includes files from other folders like std and base because they are required for some of the files in os
*/

#ifndef _OS_ALL_H
#define _OS_ALL_H

#include "base/base_compiler_check.h" //required by all the other files
#include "base/base_defines_check.h" //required by all the other files
#include "base/base_typedefs.h" //required by os_error.h, os_virtual_mem.h, os_file.h and os_process_info.h
#include "base/base_macros.h" //required by os_error.h, os_virtual_mem.h, os_path.h and os_file.h
#include "std/std_includes.h" //required by os_error.h, os_virtual_mem.h, os_path.h, os_file.h and os_process_info.h
#include "base/base_assert.h" //required by os_virtual_mem.h, os_path.h and os_file.h

#include "os/os_time.h"
#include "os/os_error.h" //required by os_virtual_mem.h and os_file.h
#include "os/os_virtual_mem.h"

#include "mem/mem_arena.h" //required by os_path.h, os_program_args.h, os_file.h and os_process_info.h
#include "struct/struct_string.h" //required by os_path.h, os_program_args.h, os_file.h and os_orca.h

#include "os/os_path.h"

#include "struct/struct_var_array.h" //required by os_program_args.h
#include "misc/misc_parsing.h" //required by os_program_args.h

#include "os/os_program_args.h"

#include "misc/misc_result.h" //required by os_dll.h, os_file.h, os_file_dialog.h and os_process_info.h
#include "base/base_debug_output.h" //required by os_file.h

#include "os/os_dll.h"
#include "os/os_file.h"
#include "os/os_file_dialog.h"

#include "mem/mem_scratch.h" //required by os_process_info.h

#include "os/os_process_info.h"
#include "os/os_font.h"

#include "struct/struct_vectors.h" //required by os_orca.h
#include "struct/struct_rectangles.h" //required by os_orca.h
#include "struct/struct_color.h" //required by os_orca.h

#include "os/os_orca.h"

#endif //  _OS_ALL_H
