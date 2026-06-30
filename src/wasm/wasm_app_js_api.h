/*
File:   wasm_app_js_api.h
Author: Taylor Robbins
Date:   01\10\2025
Description:
	** Contains the C-side declaration of the main application API between the
	** WASM module and the Javascript layer.
*/

#ifndef _WASM_APP_JS_API_H
#define _WASM_APP_JS_API_H

//TODO: Get rid of these functions altogether at some point. They are just here for testing purposes when things like MyPrint are misbehaving
// WASM_IMPORTED_FUNC void jsPrintInteger(const char* labelStrPntr, u64 number);
// WASM_IMPORTED_FUNC void jsPrintFloat(const char* labelStrPntr, double number);
// WASM_IMPORTED_FUNC void jsPrintString(const char* labelStrPntr, const char* strPntr);
// WASM_IMPORTED_FUNC void jsPrintStringLength(const char* labelStrPntr, const char* strPntr, u32 strLength);

#endif //  _WASM_APP_JS_API_H
