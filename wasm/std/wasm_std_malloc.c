/*
File:   wasm_std_malloc.c
Author: Taylor Robbins
Date:   01\10\2025
Description: 
	** Holds the implementations for functions like malloc(), realloc(), and free()
*/

//TODO: We should probably use something like dlmalloc (or our own allocator) to
//      manage the growable memory block that is provided to us as a WASM module.
//      For now we simply assert that freeing is not possible, and malloc always
//      just grows the memory. The application is expected to only call malloc
//      and manage the memory itself. The problem is that malloc doesn't obviously
//      guarantee that the returned pointer is going to be right after the previous
//      allocated chunk, so the guarantees of a growing block of memory are a little
//      clunky to rely upon. And of course, if anyone calls free() we assert.

void* malloc(size_t numBytes)
{
	return WasmMemoryAllocate(numBytes);
}

void* calloc(size_t numElements, size_t elemSize)
{
	UNUSED(numElements);
	UNUSED(elemSize);
	assert_msg(false, "callocing memory is not allowed in WebAssembly!");
	return nullptr;
}

void* realloc(void* prevAllocPntr, size_t newSize)
{
	UNUSED(prevAllocPntr);
	UNUSED(newSize);
	assert_msg(false, "reallocing memory is not allowed in WebAssembly!");
	return nullptr;
}

void free(void* allocPntr)
{
	UNUSED(allocPntr);
	assert_msg(false, "freeing memory is not allowed in WebAssembly!");
}

void* aligned_alloc(size_t numBytes, size_t alignmentSize)
{
	UNUSED(numBytes);
	UNUSED(alignmentSize);
	assert_msg(false, "aligned_allocing memory is not allowed in WebAssembly!");
	return nullptr;
}

void* alloca(size_t numBytes)
{
	return __builtin_alloca(numBytes);
}
