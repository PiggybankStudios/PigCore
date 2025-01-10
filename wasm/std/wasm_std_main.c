/*
File:   wasm_std_main.c
Author: Taylor Robbins
Date:   01\10\2025
Description: 
	** Includes all other .c files in the wasm\std folder
*/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <intrin.h>
#include <stdarg.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>

#include "wasm/std/wasm_std_memory.c"
#include "wasm/std/wasm_std_malloc.c"
#include "wasm/std/wasm_std_debugbreak.c"
#include "wasm/std/wasm_std_printf.c"
#include "wasm/std/wasm_std_misc.c"

// +==============================+
// |          InitStdLib          |
// +==============================+
WASM_EXPORTED_FUNC(void, InitStdLib, uint32_t initialMemPageCount)
{
	InitWasmMemory(initialMemPageCount);
}
