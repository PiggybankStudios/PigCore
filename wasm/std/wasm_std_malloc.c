/*
File:   wasm_std_malloc.c
Author: Taylor Robbins
Date:   01\10\2025
Description: 
	** Holds the implementations for functions like malloc(), realloc(), and free()
*/

void* malloc(size_t numBytes)
{
	return WasmMemoryAllocate(numBytes);
}

void* calloc(size_t numElements, size_t elemSize)
{
	assert_msg(false, "callocing memory is not allowed in WebAssembly!");
	return nullptr;
}

void* realloc(void* prevAllocPntr, size_t newSize)
{
	assert_msg(false, "reallocing memory is not allowed in WebAssembly!");
	return nullptr;
}

void free(void* allocPntr)
{
	assert_msg(false, "freeing memory is not allowed in WebAssembly!");
}

void* aligned_alloc(size_t numBytes, size_t alignmentSize)
{
	assert_msg(false, "aligned_allocing memory is not allowed in WebAssembly!");
	return nullptr;
}

void* alloca(size_t numBytes)
{
	return __builtin_alloca(numBytes);
}
