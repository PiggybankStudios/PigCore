// ======================= Start of wasm_globals.js =======================
/*
File:   wasm_globals.js
Author: Taylor Robbins
Date:   01\10\2025
Description:
	** This file is included first and defines some global variables and constants
	** that all the files can use when tracking state of the application
*/

const WASM_MEMORY_PAGE_SIZE = (64 * 1024); //64kB or 65,536b
const WASM_MEMORY_MAX_NUM_PAGES = (64 * 1024) //65,536 pages * 64 kB/page = 4GB
const WASM_MEMORY_MAX_SIZE = (WASM_MEMORY_MAX_NUM_PAGES * WASM_MEMORY_PAGE_SIZE)

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
var appGlobals =
{
	heapBase: 0,
	canvas: null,
	glContext: null,
	wasmMemory: null,
	wasmModule: null,
};

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
async function loadWasmModule(filePath, environment)
{
	// console.log("Loading " + filePath + "...");
	let result = null;
	try
	{
		const fetchPromise = fetch(filePath);
		const wasmModule = await WebAssembly.instantiateStreaming(
			fetchPromise,
			{ env: environment }
		);
		result = wasmModule.instance;
		// console.log("Loaded module exports:", result.exports);
	}
	catch (exception)
	{
		console.error("Failed to load WASM module from \"" + filePath + "\":", exception);
	}
	return result;
}

function wasmPntrToJsString(memory, ptr)
{
	const codes = [];
	const buf = new Uint8Array(memory.buffer);
	
	let cIndex = 0;
	while (true)
	{
		const char = buf[ptr + cIndex];
		if (!char) { break; }
		codes.push(char);
		cIndex++;
	}
	
	//TODO: Can we do something else? If we do our own UTF-8 parsing maybe?
	return String.fromCharCode(...codes);
}

// ======================== End of wasm_globals.js ========================