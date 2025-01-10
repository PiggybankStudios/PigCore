/*
File:   stdarg.h
Author: Taylor Robbins
Date:   01\10\2025
*/

#ifndef _STDARG_H
#define _STDARG_H

#include <internal/wasm_std_common.h>

CONDITIONAL_EXTERN_C_START

typedef __builtin_va_list va_list;
typedef __builtin_va_list __isoc_va_list;

#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)
#define va_copy(d,s)    __builtin_va_copy(d,s)

CONDITIONAL_EXTERN_C_END

#endif //  _STDARG_H
