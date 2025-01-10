/*
File:   wasm_std_memory.c
Author: Taylor Robbins
Date:   01\10\2025
Description: 
	** None
*/

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
uint32_t WasmMemoryHeapStartAddress = 0;
uint32_t WasmMemoryHeapCurrentAddress = 0;
uint32_t WasmMemoryNumPagesAllocated = 0;

// +--------------------------------------------------------------+
// |                          Heap Base                           |
// +--------------------------------------------------------------+
extern unsigned char __heap_base;

//TODO: What does this attribute do?
__attribute__ ((visibility("default")))
void* GetHeapBasePntr()
{
	return &__heap_base;
}
// +==============================+
// |      GetHeapBaseAddress      |
// +==============================+
// uint32_t GetHeapBaseAddress()
WASM_EXPORTED_FUNC(uint32_t, GetHeapBaseAddress)
{
	void* basePntr = GetHeapBasePntr();
	return (uint32_t)basePntr;
}

// +--------------------------------------------------------------+
// |                        Main Functions                        |
// +--------------------------------------------------------------+
void InitWasmMemory(uint32_t numInitialPages)
{
	WasmMemoryNumPagesAllocated = numInitialPages;
	WasmMemoryHeapStartAddress = GetHeapBaseAddress();
	WasmMemoryHeapCurrentAddress = WasmMemoryHeapStartAddress;
}

void* WasmMemoryAllocate(uint32_t numBytes)
{
	uint32_t numBytesNeeded = (WasmMemoryHeapCurrentAddress + numBytes);
	uint32_t numPagesNeeded = numBytesNeeded / WASM_MEMORY_PAGE_SIZE;
	if ((numBytesNeeded % WASM_MEMORY_PAGE_SIZE) != 0) { numPagesNeeded++; }
	if (numPagesNeeded > WASM_MEMORY_MAX_NUM_PAGES)
	{
		jsStdAbort("The WebAssembly module has run out of memory! WebAssembly only allows for 2GB of memory to be allocated per module", -1);
		return nullptr;
	}
	else if (WasmMemoryNumPagesAllocated < numPagesNeeded)
	{
		jsStdGrowMemory(numPagesNeeded - WasmMemoryNumPagesAllocated);
		WasmMemoryNumPagesAllocated = numPagesNeeded;
	}
	
	void* result = (void*)(WasmMemoryHeapCurrentAddress);
	WasmMemoryHeapCurrentAddress += numBytes;
	
	return result;
}

