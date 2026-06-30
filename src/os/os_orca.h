/*
File:   os_orca.h
Author: Taylor Robbins
Date:   04\20\2025
Description:
	** Contains aliases for all the functions that take\return Orca types that
	** take\return PigCore types instead. These functions have the same name as
	** their Orca counterparts but prefixed with "poc_" instead of "oc_"
	** NOTE: This file has a lot of cross files associated with it!
*/

#ifndef _OS_ORCA_H
#define _OS_ORCA_H

#include "base/base_compiler_check.h"
#include "std/std_includes.h"
#include "struct/struct_vectors.h"
#include "struct/struct_rectangles.h"
#include "struct/struct_color.h"
#include "struct/struct_string.h"

#if TARGET_IS_ORCA

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void poc_window_set_title(Str8 title);
	PIG_CORE_INLINE void poc_window_set_size(v2 size);
	PIG_CORE_INLINE void poc_clipboard_set_string(Str8 string);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

// +==============================+
// |        app.h Aliases         |
// +==============================+
PEXPI void poc_window_set_title(Str8 title) { oc_window_set_title(ToOcStr8(title)); }
PEXPI void poc_window_set_size(v2 size) { oc_window_set_size(ToOcVec2(size)); }
// void ORCA_IMPORT(oc_request_quit)(void);
// oc_key_code ORCA_IMPORT(oc_scancode_to_keycode)(oc_scan_code scanCode);
PEXPI void poc_clipboard_set_string(Str8 string) { oc_clipboard_set_string(ToOcStr8(string)); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //TARGET_IS_ORCA

#endif //  _OS_ORCA_H
