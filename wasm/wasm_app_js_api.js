// ======================= Start of wasm_app_js_api.js =======================
/*
File:   wasm_app_js_api.js
Author: Taylor Robbins
Date:   01\10\2025
Description:
	** TODO:
*/

function jsPrintInteger(labelStrPntr, number)
{
	let labelStr = wasmPntrToJsString(appGlobals.wasmMemory, labelStrPntr);
	console.log(labelStr.length + ": " + number + " (0x" + number.toString(16) + ")");
}

function jsPrintFloat(labelStrPntr, number)
{
	let labelStr = wasmPntrToJsString(appGlobals.wasmMemory, labelStrPntr);
	console.log(labelStr + ": " + number);
}

function jsPrintString(labelStrPntr, strPntr)
{
	let labelStr = wasmPntrToJsString(appGlobals.wasmMemory, labelStrPntr);
	let str = wasmPntrToJsString(appGlobals.wasmMemory, strPntr);
	console.log(labelStr + ": \"" + str + "\"");
}

jsAppApiFuncs =
{
	jsPrintInteger: jsPrintInteger,
	jsPrintFloat: jsPrintFloat,
	jsPrintString: jsPrintString,
};

// ======================== End of wasm_app_js_api.js ========================