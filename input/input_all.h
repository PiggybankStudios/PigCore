/*
File:   input_all.h
Author: Taylor Robbins
Date:   02\03\2025
*/

#ifndef _INPUT_ALL_H
#define _INPUT_ALL_H

#include "base/base_defines_check.h" //required by all the other files
#include "base/base_macros.h" //required by all other files

#include "input/input_keys.h"
#include "input/input_mouse_btns.h"
#include "input/input_controller_btns.h"
#include "input/input_touch.h"

#include "base/base_typedefs.h" //required by input_btn_state.h
#include "base/base_assert.h" //required by input_btn_state.h
#include "std/std_memset.h" //required by input_btn_state.h

#include "input/input_btn_state.h"

#include "misc/misc_sokol_app_include.h" //required by input_sokol.h

#include "input/input_sokol.h"

#endif //  _INPUT_ALL_H
