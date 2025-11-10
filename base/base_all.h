/*
File:   base_all.h
Author: Taylor Robbins
Date:   01\04\2025
Description:
	** Includes all files in this folder, meant as a quick shorthand for applications that just want to #include every "base_" file
*/

#ifndef _BASE_ALL_H
#define _BASE_ALL_H

//NOTE: One or both of these files are #included in almost ever other file in PigCore. This is because stuff like PIG_CORE_IMPLEMENTATION, PEXP or
//      IS_LANGUAGE_C are defined in these files and most files depend on those defines.
//      base_compiler_check.h also includes build_config.h because options in there could affect how we choose core things like PIG_CORE_IMPLEMENTATION
#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"

#include "base/base_assert.h"
#include "base/base_char.h"
#include "base/base_dbg_level.h"
#include "base/base_debug_output.h"
#include "base/base_macros.h"
#include "base/base_math.h"
#include "base/base_notifications.h"
#include "base/base_plex_is_struct.h"
#include "base/base_typedefs.h"
#include "base/base_unicode.h"
#include "os/os_threading.h"

// #include "base/base_debug_output_impl.h" //TODO: Re-enable me once we get this working on Playdate
#include "base/base_notifications_impl.h"

#endif //  _BASE_ALL_H
