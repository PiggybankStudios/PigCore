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
	let labelStr = wasmPntrToJsString(labelStrPntr);
	console.log(labelStr + ": " + number + " (0x" + number.toString(16) + ")");
}

function jsPrintFloat(labelStrPntr, number)
{
	let labelStr = wasmPntrToJsString(labelStrPntr);
	console.log(labelStr + ": " + number);
}

function jsPrintString(labelStrPntr, strPntr)
{
	let labelStr = wasmPntrToJsString(labelStrPntr);
	let str = wasmPntrToJsString(strPntr);
	console.log(labelStr + ": \"" + str + "\"");
}

function jsPrintStringLength(labelStrPntr, strPntr, strLength)
{
	let labelStr = wasmPntrToJsString(labelStrPntr);
	let str = wasmPntrAndLengthToJsString(strPntr, strLength);
	console.log(labelStr + ": \"" + str + "\"");
}

jsAppApiFuncs =
{
	jsPrintInteger: jsPrintInteger,
	jsPrintFloat: jsPrintFloat,
	jsPrintString: jsPrintString,
	jsPrintStringLength: jsPrintStringLength,
};

// ======================== End of wasm_app_js_api.js ========================