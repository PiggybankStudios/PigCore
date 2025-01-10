/*
File:   wasm_std_common.h
Author: Taylor Robbins
Date:   01\10\2025
Description:
	** Included FIRST in all other header files.
	** Does some basic checks, defines some fundamental macros and includes the js_api header
	** that defines which functions the std library needs to call from javascript
*/

#ifndef _WASM_STD_COMMON_H
#define _WASM_STD_COMMON_H

#ifdef __cplusplus
#define LANGUAGE_C   0
#define LANGUAGE_CPP 1
#else
#define LANGUAGE_C   1
#define LANGUAGE_CPP 0
#endif

#ifndef __wasm32__
#error This standard library implementation only works for 32-bit WebAssembly!
#endif

#ifndef __clang__
#error This standard library implementation only works with the clang compiler!
#endif

#if !defined(__clang_major__) || !defined(__clang_minor__) || !defined(__clang_patchlevel__)
#error Missing defines for __clang_major__, __clang_minor__, and __clang_patchlevel__!
#endif

#if __clang_major__ > 18 || (__clang_major__ == 18 && __clang_minor__ > 1) || (__clang_major__ == 18 && __clang_minor__ == 1 && __clang_patchlevel__ > 8)
#warning WARNING: This standard library implementation has not been tested with versions of clang past 18.1.8
#endif

#if __clang_major__ < 13 || __clang_patchlevel__ < 1
#error This standard library implementation does not support versions of clang before 13.0.1!
#endif

#ifndef __BYTE_ORDER__
#error Missing __BYTE_ORDER__ define!
#elif __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error This standard library implementation assumes little-endian byte order
#endif

#define WASM_MEMORY_PAGE_SIZE      (64*1024UL) //64kB or 65,536b
#define WASM_MEMORY_MAX_NUM_PAGES  (64*1024UL) //65,536 pages * 64 kB/page = 4GB
#define WASM_MEMORY_MAX_SIZE       (WASM_MEMORY_MAX_NUM_PAGES * WASM_MEMORY_PAGE_SIZE)
#define WASM_PROTECTED_SIZE        1024       //1kB at start of wasm memory should be unused and should never be written to

#if LANGUAGE_CPP
#define CONDITIONAL_EXTERN_C extern "C"
#define CONDITIONAL_EXTERN_C_START extern "C" {
#define CONDITIONAL_EXTERN_C_END   }
#else
#define CONDITIONAL_EXTERN_C       //nothing
#define CONDITIONAL_EXTERN_C_START //nothing
#define CONDITIONAL_EXTERN_C_END   //nothing
#endif

//TODO: __attribute__((import_module("env"), import_name(#functionName)))
#define WASM_IMPORTED_FUNC CONDITIONAL_EXTERN_C
#define WASM_EXPORTED_FUNC(returnType, functionName, ...) CONDITIONAL_EXTERN_C returnType __attribute__((export_name(#functionName))) functionName(__VA_ARGS__)

#if LANGUAGE_C
#define nullptr ((void*)0)
#endif

#if (LANGUAGE_C && __has_c_attribute(fallthrough))
#define STD_FALLTHROUGH [[fallthrough]];
#else
#define STD_FALLTHROUGH
#endif

#include <internal/wasm_std_js_api.h>

#endif //  _WASM_STD_COMMON_H
