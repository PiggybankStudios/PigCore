/*
File:   misc_all.h
Author: Taylor Robbins
Date:   01\05\2025
*/

#ifndef _MISC_ALL_H
#define _MISC_ALL_H

#include "base/base_defines_check.h" //required by all the other files
#include "base/base_macros.h" //required by misc_result.h and misc_random.h

#include "misc/misc_profiling_tracy_include.h"
#include "misc/misc_result.h"

#include "base/base_typedefs.h" //required by everyone below
#include "base/base_assert.h" //required by everyone below

#include "misc/misc_hash.h"
#include "misc/misc_random.h"

#include "std/std_trig.h" //required by misc_easing.h
#include "std/std_basic_math.h" //required by misc_easing.h and misc_parsing.h

#include "misc/misc_easing.h"

#include "struct/struct_color.h" //required by misc_standard_colors.h

#include "misc/misc_standard_colors.h"

#include "struct/struct_string.h" //required by misc_web.h, misc_printing.h, misc_parsing.h and misc_file_watch.h
#include "struct/struct_ranges.h" //required by misc_web.h

#include "misc/misc_web.h"

#include "std/std_includes.h" //required by misc_printing.h
#include "std/std_printf.h" //required by misc_printing.h and misc_parsing.h
#include "mem/mem_arena.h" //required by misc_printing.h, misc_file_watch.h and misc_sorting.h
#include "os/os_file.h" //required by misc_file_watch.h

#include "misc/misc_simple_parsers.h"
#include "misc/misc_printing.h"
#include "misc/misc_two_pass.h"
#include "misc/misc_file_watch.h"
#include "misc/misc_sorting.h"

#include "struct/struct_vectors.h" //required by misc_noise.h and misc_triangulation.h

#include "misc/misc_noise.h"
#include "misc/misc_triangulation.h"
#include "misc/misc_zip.h"

#include "base/base_char.h" //required by misc_parsing.h
#include "base/base_unicode.h" //required by misc_parsing.h

#include "misc/misc_parsing.h"

#include "mem/mem_scratch.h" //required for misc_regex.h

#include "misc/misc_regex.h"

#endif //  _MISC_ALL_H
