/*
File:   std_all.h
Author: Taylor Robbins
Date:   01\04\2025
Description:
	** Includes all files in this folder, meant as a quick shorthand for applications that just want everything from this folder available
	** This file also serves as a reference for the order of dependencies between files in this folder.
	** This file also includes files from other folders like base because they are required for some of the files in std
*/

#ifndef _STD_ALL_H
#define _STD_ALL_H

#include "base/base_compiler_check.h" //required by all the other files

#include "std/std_includes.h"

#include "base/base_macros.h" //required by std_trig.h and std_basic_math.h

#include "std/std_trig.h"

#include "base/base_compiler_check.h" //required by the rest of the files
#include "base/base_typedefs.h" //required by the rest of the files

#include "std/std_malloc.h"
#include "std/std_memset.h"
#include "std/std_printf.h"
#include "std/std_basic_math.h"
#include "std/std_angles.h"

#include "base/base_math.h" //required by std_math_ex.h

#include "std/std_math_ex.h"

#endif //  _STD_ALL_H
