/*
File:   mem_all.h
Author: Taylor Robbins
Date:   01\04\2025
Description:
	** Includes all files in this folder, meant as a quick shorthand for applications that just want everything from this folder available
	** This file also serves as a reference for the order of dependencies between files in this folder.
	** This file also includes files from other folders like std because they are required for some of the files in mem
*/

#ifndef _MEM_ALL_H
#define _MEM_ALL_H

#include "base/base_defines_check.h" //required by all the other files
#include "base/base_typedefs.h" //required by mem_arena.h and mem_scratch.h
#include "base/base_macros.h" //required by mem_arena.h and mem_scratch.h
#include "base/base_assert.h" //required by mem_arena.h and mem_scratch.h
#include "base/base_math.h" //required by mem_arena.h
#include "std/std_malloc.h" //required by mem_arena.h
#include "std/std_memset.h" //required by mem_arena.h
#include "os/os_virtual_mem.h" //required by mem_arena.h

#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"

#endif //  _MEM_ALL_H
