/*
File:   stdbool.h
Author: Taylor Robbins
Date:   01\10\2025
*/

#ifndef _STDBOOL_H
#define _STDBOOL_H

#include <internal/wasm_std_common.h>

#if LANGUAGE_C

#define true 1
#define false 0
#define bool _Bool

#endif

#define __bool_true_false_are_defined 1

#endif //  _STDBOOL_H
