/*
File:   struct_all.h
Author: Taylor Robbins
Date:   01\04\2025
Description:
	** Includes all files in this folder, meant as a quick shorthand for applications that just want everything from this folder available
	** This file also serves as a reference for the order of dependencies between files in this folder.
	** This file also includes files from other folders like mem and std because they are required for some of the files in struct
*/

#ifndef _STRUCT_ALL_H
#define _STRUCT_ALL_H

#include "base/base_typedefs.h" //Needed by struct_var_array.h
#include "base/base_macros.h" //Needed by struct_var_array.h
#include "base/base_assert.h" //Needed by struct_var_array.h
#include "mem/mem_arena.h" //Needed by struct_var_array.h
#include "std/std_memset.h" //Needed by struct_var_array.h
#include "struct/struct_var_array.h"

#endif //  _STRUCT_ALL_H
