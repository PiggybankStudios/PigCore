/*
File:   parse_all.h
Author: Taylor Robbins
Date:   03\31\2025
*/

#ifndef _PARSE_ALL_H
#define _PARSE_ALL_H

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"

#include "base/base_typedefs.h" //needed by parse_c_tokenizer.h, and parse_metadesk.h
#include "base/base_macros.h" //needed by parse_c_tokenizer.h
#include "base/base_assert.h" //needed by parse_c_tokenizer.h, and parse_metadesk.h
#include "base/base_char.h" //needed by parse_c_tokenizer.h
#include "base/base_unicode.h" //needed by parse_c_tokenizer.h
#include "std/std_memset.h" //needed by parse_c_tokenizer.h
#include "mem/mem_arena.h" //needed by parse_c_tokenizer.h, and parse_metadesk.h
#include "struct/struct_string.h" //needed by parse_c_tokenizer.h, and parse_metadesk.h
#include "struct/struct_var_array.h" //needed by parse_c_tokenizer.h

#include "parse/parse_c_tokenizer.h"

#include "os/os_virtual_mem.h" //needed by parse_metadesk.h
#include "os/os_file.h" //needed by parse_metadesk.h

#include "parse/parse_metadesk_include.h" //needed by parse_metadesk.h

#include "parse/parse_metadesk.h"

#endif //  _PARSE_ALL_H
