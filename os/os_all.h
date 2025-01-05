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

#include "base/base_compiler_check.h" //required by os_virtual_mem.h
#include "base/base_typedefs.h" //required by os_virtual_mem.h
#include "base/base_macros.h" //required by os_virtual_mem.h
#include "std/std_includes.h" //required by os_virtual_mem.h
#include "os/os_virtual_mem.h"

#endif //  _OS_ALL_H
