/*
File:   misc_all.h
Author: Taylor Robbins
Date:   01\05\2025
*/

#ifndef _MISC_ALL_H
#define _MISC_ALL_H

#include "base/base_defines_check.h" //required by all the other files
#include "base/base_macros.h" //required by misc_result.h and misc_random.h

#include "misc/misc_result.h"

#include "base/base_typedefs.h" //required by everyone below
#include "base/base_assert.h" //required by everyone below

#include "misc/misc_hash.h"
#include "misc/misc_random.h"

#include "std/std_trig.h" //required by misc_easing.h
#include "std/std_basic_math.h" //required by misc_easing.h

#include "misc/misc_easing.h"

#include "struct/struct_color.h" //required by misc_standard_colors.h

#include "misc/misc_standard_colors.h"

#include "std/std_includes.h" //required by misc_printing.h
#include "struct/struct_string.h" //required by misc_printing.h
#include "std/std_printf.h" //required by misc_printing.h
#include "mem/mem_arena.h" //required by misc_printing.h

#include "misc/misc_simple_parsers.h"
#include "misc/misc_printing.h"

#include "struct/struct_vectors.h" //required by misc_noise.h

#include "misc/misc_noise.h"

#include "misc/misc_zip.h"

#endif //  _MISC_ALL_H
