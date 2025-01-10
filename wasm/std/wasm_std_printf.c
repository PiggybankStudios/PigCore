/*
File:   wasm_std_printf.c
Author: Taylor Robbins
Date:   01\10\2025
*/

#define STB_SPRINTF_IMPLEMENTATION
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#include "third_party/stb/stb_sprintf.h"
#pragma clang diagnostic pop

//TODO: Should we increase this buffer size? Or make it an option for the user to change? Or dynamically allocate from the stack? Or something else?
#define STD_PRINTF_BUFFER_SIZE 1024 //chars
char stdGlobalPrintBuffer[STD_PRINTF_BUFFER_SIZE];

int vsnprintf(char* bufferPntr, size_t bufferSize, const char* formatStr, va_list args)
{
	assert(bufferSize <= INT_MAX);
	return stbsp_vsnprintf(bufferPntr, bufferSize, formatStr, args);
}
int vsprintf(char* restrict bufferPntr, const char* restrict formatStr, va_list args)
{
	return vsnprintf(bufferPntr, INT_MAX, formatStr, args);
}
int vprintf(const char* restrict formatStr, va_list args)
{
	int result = stbsp_vsnprintf(stdGlobalPrintBuffer, STD_PRINTF_BUFFER_SIZE, formatStr, args);
	if (result > 0 && result < STD_PRINTF_BUFFER_SIZE)
	{
		stdGlobalPrintBuffer[result] = '\0'; //ensure null-termination
		jsStdPrint(&stdGlobalPrintBuffer[0], result);
	}
	return result;
}

int snprintf(char* restrict bufferPntr, size_t bufferSize, const char* restrict formatStr, ...)
{
	int result;
	va_list args;
	va_start(args, formatStr);
	result = vsnprintf(bufferPntr, bufferSize, formatStr, args);
	va_end(args);
	return result;
}
int sprintf(char* restrict bufferPntr, const char* restrict formatStr, ...)
{
	int result;
	va_list args;
	va_start(args, formatStr);
	result = vsprintf(bufferPntr, formatStr, args);
	va_end(args);
	return result;
}
int printf(const char* restrict formatStr, ...)
{
	int result;
	va_list args;
	va_start(args, formatStr);
	result = vprintf(formatStr, args);
	va_end(args);
	return result;
}
