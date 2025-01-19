/*
File:   base_all.h
Author: Taylor Robbins
Date:   01\04\2025
Description:
	** Includes all files in this folder, meant as a quick shorthand for applications that just want everything from this folder available
	** This file also serves as a reference for the order of dependencies between files in this folder.
	** This file also includes files from other folders like std because they are required for some of the files in base
*/

#ifndef _BASE_ALL_H
#define _BASE_ALL_H

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_dbg_level.h"
#include "base/base_debug_output.h"
#include "base/base_math.h"

#include "std/std_includes.h" //required by base_assert.h

#include "base/base_assert.h"
#include "base/base_char.h"
#include "base/base_unicode.h"

#include "base/base_debug_output_impl.h"

#endif //  _BASE_ALL_H
