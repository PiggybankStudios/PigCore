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

#define MyPrint(formatStr, ...) printf_is_not_supported_on_playdate
#define MyPrintNoLine(formatStr, ...) printf_is_not_supported_on_playdate

//TODO: If we ever compile PigCore as a dll for Playdate then we may need to markup
//      stbsp_sprintf with PEXP or maybe we need our own routing function around
//      the call just so we can mark it up
#ifndef MyBufferPrintf
#define MyBufferPrintf(buffer, bufferSize, formatStr, ...) stbsp_snprintf((buffer), (bufferSize), (formatStr), ##__VA_ARGS__)
#endif
#ifndef MyVaListPrintf
#define MyVaListPrintf(buffer, bufferSize, formatStr, vaList) stbsp_vsnprintf((buffer), (bufferSize), (formatStr), vaList)
#endif

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
#define MyStrToFloat(nullTermStr) atof(nullTermStr)

#endif //  _STD_PRINTF_H

#if defined(_STD_PRINTF_H) && defined(_STRUCT_STRING_BUFFER_H)
#include "cross/cross_printf_and_string_buffer.h"
#endif
