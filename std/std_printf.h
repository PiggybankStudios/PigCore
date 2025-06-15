/*
File:   std_printf.h
Author: Taylor Robbins
Date:   01\02\2025
Description:
	** Contains routing aliases for printf, snprintf, vsnprintf, htons, htonl, ntohs, ntohl, and atof
*/

#ifndef _STD_PRINTF_H
#define _STD_PRINTF_H

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "std/std_includes.h"

// +--------------------------------------------------------------+
// |                 Playdate stb_printf Routing                  |
// +--------------------------------------------------------------+
#if TARGET_IS_PLAYDATE

#if PIG_CORE_IMPLEMENTATION
#define STB_SPRINTF_IMPLEMENTATION
#endif
#include "third_party/stb/stb_sprintf.h"

//TODO: If we ever compile PigCore as a dll for Playdate then we may need to markup
//      stbsp_sprintf with PEXP or maybe we need our own routing function around
//      the call just so we can mark it up
#ifndef MyBufferPrintf
#define MyBufferPrintf(buffer, bufferSize, formatStr, ...) stbsp_snprintf((buffer), (bufferSize), (formatStr), ##__VA_ARGS__)
#endif
#ifndef MyVaListPrintf
#define MyVaListPrintf(buffer, bufferSize, formatStr, vaList) stbsp_vsnprintf((buffer), (bufferSize), (formatStr), vaList)
#endif

#if !PIG_CORE_IMPLEMENTATION
void PlaydatePrint(bool newLine, const char* formatStr, ...);
#else
PEXP void PlaydatePrint(bool newLine, const char* formatStr, ...)
{
	char printBuffer[256];
	va_list args;
	va_start(args, formatStr);
	int printResult = MyVaListPrintf(printBuffer, 256, formatStr, args);
	if (printResult >= 0)
	{
		if (printResult <= 256-2)
		{
			if (newLine) { printBuffer[printResult] = '\n'; printResult++; }
			printBuffer[printResult] = '\0';
			pd->system->logToConsole(printBuffer);
		}
		else
		{
			pd->system->logToConsole("PRINT BUFFER OVERFLOW");
			pd->system->logToConsole(formatStr);
		}
	}
	va_end(args);
}
#endif

#define MyPrint(formatStr, ...)       PlaydatePrint(true,  (formatStr), ##__VA_ARGS__)
#define MyPrintNoLine(formatStr, ...) PlaydatePrint(false, (formatStr), ##__VA_ARGS__)

// +--------------------------------------------------------------+
// |                    Regular printf Routing                    |
// +--------------------------------------------------------------+
#else

#define MyPrint(formatStr, ...) printf(formatStr "\n", ##__VA_ARGS__)
#define MyPrintNoLine(formatStr, ...) printf((formatStr), ##__VA_ARGS__)
#define MyBufferPrintf(buffer, bufferSize, formatStr, ...) snprintf((buffer), (bufferSize), (formatStr), ##__VA_ARGS__)
#define MyBufferPrintf(buffer, bufferSize, formatStr, ...) snprintf((buffer), (bufferSize), (formatStr), ##__VA_ARGS__)
#define MyVaListPrintf(buffer, bufferSize, formatStr, vaList) vsnprintf((buffer), (bufferSize), (formatStr), vaList)

#endif

// +--------------------------------------------------------------+
// |                  Numeric String Conversions                  |
// +--------------------------------------------------------------+
#define MyHostToNetworkByteOrderU16(integer) htons(integer)
#define MyHostToNetworkByteOrderU32(integer) htonl(integer)
#define MyNetworkToHostByteOrderU16(integer) ntohs(integer)
#define MyNetworkToHostByteOrderU32(integer) ntohl(integer)

#if TARGET_IS_PLAYDATE || USING_CUSTOM_STDLIB
	#if !PIG_CORE_IMPLEMENTATION
	float ratof(char* arr);
	#else
	PEXP float ratof(char* arr)
	{
		float val = 0;
		int afterdot=0;
		float scale=1;
		int neg = 0; 
		
		if (*arr == '-')
		{
			arr++;
			neg = 1;
		}
		while (*arr)
		{
			if (afterdot)
			{
				scale = scale/10;
				val = val + (*arr-'0')*scale;
			}
			else
			{
				if (*arr == '.') { afterdot++; }
				else { val = val * 10.0f + (*arr - '0'); }
			}
			arr++;
		}
		
		return ((neg) ? -val : val);
	}
	#endif
	
	#define MyStrToFloat(nullTermStr) ratof(nullTermStr)
#else
#define MyStrToFloat(nullTermStr) atof(nullTermStr)
#endif

#if USING_CUSTOM_STDLIB
#endif

#endif //  _STD_PRINTF_H

#if defined(_STD_PRINTF_H) && defined(_STRUCT_STRING_BUFFER_H)
#include "cross/cross_printf_and_string_buffer.h"
#endif
