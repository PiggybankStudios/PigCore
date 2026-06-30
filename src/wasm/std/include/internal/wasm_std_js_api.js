// ======================= Start of wasm_std_js_api.js =======================
/*
File:   wasm_std_js_api.js
Author: Taylor Robbins
Date:   01\10\2025
Description:
	** Contains a bunch of client-side code that is required for our custom Wasm StdLib to function properly
	** This includes a bunch of functions that need to be imported to the wasm module for it to be able to
	** communicate with the browser for basic functionality (like stdout, memory allocation, etc.)
	** The application should combine this javascript file with their own file(s) to create a "combined.js"
	** file which will be included by index.html
*/

function jsStdPrint(messageStrPntr, messageLength)
{
	let messageStr = wasmPntrToJsString(messageStrPntr);
	console.log(messageStr);
}

function jsStdAbort(messageStrPntr, exitCode)
{
	let messageStr = wasmPntrToJsString(messageStrPntr);
	let exitStr = "Abort [" + exitCode + "]: " + messageStr;
	console.error(exitStr);
	throw new Error(exitStr);
}

function jsStdAssertFailure(filePathPntr, fileLineNum, funcNamePntr, conditionStrPntr, messageStrPntr)
{
	let filePath = wasmPntrToJsString(filePathPntr);
	let funcName = wasmPntrToJsString(funcNamePntr);
	let conditionStr = wasmPntrToJsString(conditionStrPntr);
	let outputMessage = "";
	if (messageStrPntr != 0)
	{
		let messageStr = wasmPntrToJsString(messageStrPntr);
		outputMessage = "Assertion failed, " + messageStr + " (" + conditionStr + ") is not true! In " + filePath + ":" + fileLineNum + " " + funcName + "(...)";
	}
	else
	{
		outputMessage = "Assertion failed! (" + conditionStr + ") is not true! In " + filePath + ":" + fileLineNum + " " + funcName + "(...)";
	}
	console.error(outputMessage);
	throw new Error(outputMessage);
}

function jsStdDebugBreak()
{
	//TODO: This is not a proper solution, really. Can we somehow notify the debugger in Firefox/Chrome/Safari/etc.?
	alert("A debug breakpoint has been hit!");
}

//TODO:
// __builtin_wasm_memory_size(0); // the number of 64Kb pages we have
// __builtin_wasm_memory_grow(0, blocks); // increases amount of pages
// __builtin_huge_valf(); // similar to Infinity in JS
function jsStdGrowMemory(numPages)
{
	let currentPageCount = appGlobals.wasmModule.exports.memory.buffer.byteLength / WASM_MEMORY_PAGE_SIZE;
	// console.log("Memory growing by " + numPages + " pages (" + currentPageCount + " -> " + (currentPageCount + numPages) + ")");
	appGlobals.wasmModule.exports.memory.grow(numPages);
	appGlobals.memDataView = new DataView(new Uint8Array(appGlobals.wasmModule.exports.memory.buffer).buffer);
}

jsStdApiFuncs =
{
	jsStdPrint: jsStdPrint,
	jsStdAbort: jsStdAbort,
	jsStdAssertFailure: jsStdAssertFailure,
	jsStdDebugBreak: jsStdDebugBreak,
	jsStdGrowMemory: jsStdGrowMemory,
};

// ======================== End of wasm_std_js_api.js ========================