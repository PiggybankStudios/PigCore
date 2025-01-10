/*
File:   wasm_std_js_api.h
Author: Taylor Robbins
Date:   01\10\2025
Description:
	** Declares imports for all the functions we need to be provided by javascript (in wasm_std_lib.js)
*/

#ifndef _WASM_STD_JS_API_H
#define _WASM_STD_JS_API_H

WASM_IMPORTED_FUNC void jsStdPrint(const char* messageStrPntr, int messageLength);
WASM_IMPORTED_FUNC _Noreturn void jsStdAbort(const char* messageStrPntr, int exitCode);
WASM_IMPORTED_FUNC _Noreturn void jsStdAssertFailure(const char* filePathPntr, int fileLineNum, const char* funcNamePntr, const char* conditionStrPntr, const char* messageStrPntr);
WASM_IMPORTED_FUNC void jsStdDebugBreak();
WASM_IMPORTED_FUNC void jsStdGrowMemory(unsigned int numPages);
WASM_IMPORTED_FUNC unsigned int jsStdGetHeapSize();

#endif //  _WASM_STD_JS_API_H
