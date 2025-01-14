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
	// wasmMemory: null,
	memDataView: null,
	wasmModule: null,
	textDecoder: null,
};

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
async function loadWasmModule(filePath, environment)
{
	let result = null;
	try
	{
		const fetchPromise = fetch(filePath);
		const wasmModule = await WebAssembly.instantiateStreaming(
			fetchPromise,
			{ env: environment }
		);
		result = wasmModule.instance;
	}
	catch (exception)
	{
		console.error("Failed to load WASM module from \"" + filePath + "\":", exception);
	}
	return result;
}

//TODO: We should do some performance measurements of wasmPntrToJsString vs wasmPntrAndLengthToJsString!
function wasmPntrToJsString(ptr)
{
	let cIndex = ptr;
	while (cIndex < appGlobals.memDataView.byteLength)
	{
		let byteValue = appGlobals.memDataView.getUint8(cIndex, true);
		if (byteValue == 0)
		{
			break;
		}
		cIndex++;
	}
	return appGlobals.textDecoder.decode(
		appGlobals.memDataView.buffer.slice(ptr, cIndex)
	);
}
function wasmPntrAndLengthToJsString(ptr, length)
{
	return appGlobals.textDecoder.decode(
		appGlobals.memDataView.buffer.slice(ptr, ptr + length)
	);
}

// ======================== End of wasm_globals.js ========================