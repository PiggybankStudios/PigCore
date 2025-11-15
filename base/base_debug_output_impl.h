/*
File:   base_debug_output_impl.h
Author: Taylor Robbins
Date:   01\18\2025
Description:
	** Contains the implementation for routing functions declared in base_debug_output.h
	** This file is not automatically included as a dependency (except for in base_all.h)
	** so it must be manually included if you #include any file that depends on base_debug_output.h
	** See base_debug_output.h description for a longer explanation
*/

#ifndef _BASE_DEBUG_OUTPUT_IMPL_H
#define _BASE_DEBUG_OUTPUT_IMPL_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_dbg_level.h"
#include "base/base_debug_output.h"
#include "mem/mem_scratch.h"
#include "std/std_memset.h"
#include "std/std_printf.h"
#include "misc/misc_printing.h"
#include "os/os_threading.h"

//TODO: Check if the scratch arenas have been initialized on the current thread before using them for print formatting. Maybe use a small thread_local buffer instead when that happens?

//TODO: We could potentially change the function signature of these functions to take Str8 (instead of const char* with implicit reliance on null-terminating character)
//      This could allow places to output strings without needing a null-terminating character and without a memory arena to allocate into to get a null-terminated string copy

//TODO: Can we add support for coloring text in terminals that support text coloring?

//TODO: Do we want to support debug_output WITHOUT mem_arena.h reliance? And/or without mem_scratch.h reliance?
//      My gut currently says that we want to have at least SOME amount of scratch arena support on all platforms
//      that we want to ship on, so maybe this reliance is totally fine.

//TODO: Since DebugPrintRouter uses scratch memory, we could get rid of the staticly sized DebugOutputLineBuffer
//      And just grow a buffer in the scratch arena as a line is built. This might require us to use a data structure
//      like VarArray which could expose us to more re-entrancy (if VarArray does debug output) but maybe it's worth it?
//      Or maybe we just handle the memory without a data structure?

#if PIG_CORE_IMPLEMENTATION

DebugOutput_f* DebugOutputCallback = nullptr;

THREAD_LOCAL bool DebugOutputIsOnNewLine = true;
#if DEBUG_OUTPUT_CALLBACK_ONLY_ON_FINISHED_LINE
THREAD_LOCAL uxx DebugOutputLineLength = 0;
THREAD_LOCAL char DebugOutputLineBuffer[DEBUG_OUTPUT_LINE_BUFFER_SIZE] = ZEROED;
THREAD_LOCAL bool DebugOutputLineOverflowOccurred = false;
#endif //DEBUG_OUTPUT_CALLBACK_ONLY_ON_FINISHED_LINE

// +==============================+
// |      DebugOutputRouter       |
// +==============================+
// void DebugOutputRouter(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel level, bool isNotification, bool newLine, const char* message)
PEXP DEBUG_OUTPUT_HANDLER_DEF(DebugOutputRouter)
{
	#if !DEBUG_OUTPUT_SHOW_NOTIFICATIONS
	if (isNotification) { return; }
	#endif
	
	if ((level == DbgLevel_Debug   && ENABLE_DEBUG_OUTPUT_LEVEL_DEBUG)   ||
		(level == DbgLevel_Regular && ENABLE_DEBUG_OUTPUT_LEVEL_REGULAR) ||
		(level == DbgLevel_Info    && ENABLE_DEBUG_OUTPUT_LEVEL_INFO)    ||
		(level == DbgLevel_Notify  && ENABLE_DEBUG_OUTPUT_LEVEL_NOTIFY)  ||
		(level == DbgLevel_Other   && ENABLE_DEBUG_OUTPUT_LEVEL_OTHER)   ||
		(level == DbgLevel_Warning && ENABLE_DEBUG_OUTPUT_LEVEL_WARNING) ||
		(level == DbgLevel_Error   && ENABLE_DEBUG_OUTPUT_LEVEL_ERROR)   ||
		level == DbgLevel_None || level >= DbgLevel_Count)
	{
		#if DEBUG_OUTPUT_CALLBACK_ONLY_ON_FINISHED_LINE
		bool isLineOverflowOutput = DebugOutputLineOverflowOccurred;
		#endif
		
		#if DEBUG_OUTPUT_PRINT_LEVEL_PREFIX
		if (DebugOutputIsOnNewLine)
		{
			MyPrintNoLine("%s%s: %s%s", isNotification ? "NOTIFICATION: " : "", GetDbgLevelStr(level), message, newLine ? "\n" : "");
		}
		else
		{
			MyPrintNoLine("%s%s%s", isNotification ? "NOTIFICATION: " : "", message, newLine ? "\n" : "");
		}
		#else
		MyPrintNoLine("%s%s%s", isNotification ? "NOTIFICATION: " : "", message, newLine ? "\n" : "");
		#endif
		
		#if TARGET_IS_WINDOWS
		{
			//NOTE: OutputDebugStringA comes from Debugapi.h which is included by Windows.h
			//      Visual Studio and most Windows debuggers do not show things printed to stdout in
			//      the "Output" window when debugging. Sending our debug output to OutputDebugString
			//      ensures that our debug output can be viewed from the "Output" window in Windows debuggers.
			if (isNotification)
			{
				OutputDebugStringA("NOTIFICATION: ");
			}
			#if DEBUG_OUTPUT_PRINT_LEVEL_PREFIX
			if (DebugOutputIsOnNewLine)
			{
				OutputDebugStringA(GetDbgLevelStr(level));
				OutputDebugStringA(": ");
			}
			#endif
			OutputDebugStringA(message);
			if (newLine) { OutputDebugStringA("\n"); }
		}
		#endif //TARGET_IS_WINDOWS
		
		#if TARGET_IS_ANDROID
		android_LogPriority androidPriority = ANDROID_LOG_VERBOSE;
		switch (level)
		{
			case DbgLevel_Debug:   androidPriority = ANDROID_LOG_DEBUG; break;
			case DbgLevel_Regular: androidPriority = ANDROID_LOG_INFO;  break;
			case DbgLevel_Info:    androidPriority = ANDROID_LOG_INFO;  break;
			case DbgLevel_Notify:  androidPriority = ANDROID_LOG_INFO;  break;
			case DbgLevel_Other:   androidPriority = ANDROID_LOG_INFO;  break;
			case DbgLevel_Warning: androidPriority = ANDROID_LOG_WARN;  break;
			case DbgLevel_Error:   androidPriority = ANDROID_LOG_ERROR; break;
		}
		__android_log_print(androidPriority, "pigcore", "%s", message);
		#endif
		
		#if TARGET_IS_ORCA
		{
			//TODO: How do we handle when newLine == false?
			oc_log_level orcaLevel = OC_LOG_LEVEL_INFO;
			if (level == DbgLevel_Error) { orcaLevel = OC_LOG_LEVEL_ERROR; }
			else if (level == DbgLevel_Warning) { orcaLevel = OC_LOG_LEVEL_WARNING; }
			#if DEBUG_OUTPUT_MINIMAL_ORCA_PREFIX
			oc_log_ext(orcaLevel, nullptr, nullptr, 0, message);
			#else
			oc_log_generic(orcaLevel, message);
			#endif
		}
		#endif //TARGET_IS_ORCA
		
		#if DEBUG_OUTPUT_CALLBACK_GLOBAL
		if (DebugOutputCallback != nullptr)
		{
			#if DEBUG_OUTPUT_CALLBACK_ONLY_ON_FINISHED_LINE
			uxx prevNewLineIndex = 0;
			for (uxx bIndex = 0; ; bIndex++) //we end loop AFTER finding the nullterm, see break below
			{
				if (message[bIndex] == '\n' || message[bIndex] == '\0')
				{
					uxx numBytesToCopy = bIndex - prevNewLineIndex;
					if (numBytesToCopy > DEBUG_OUTPUT_LINE_BUFFER_SIZE-1 - DebugOutputLineLength)
					{
						//NOTE: This is where we drop characters if a single line is bigger than DEBUG_OUTPUT_LINE_BUFFER_SIZE-1
						numBytesToCopy = DEBUG_OUTPUT_LINE_BUFFER_SIZE-1 - DebugOutputLineLength;
						DebugOutputLineOverflowOccurred = true;
					}
					if (numBytesToCopy > 0)
					{
						MyMemCopy(&DebugOutputLineBuffer[DebugOutputLineLength], &message[prevNewLineIndex], numBytesToCopy);
						DebugOutputLineLength += numBytesToCopy;
						DebugOutputLineBuffer[DebugOutputLineLength] = '\0';
					}
					if (message[bIndex] == '\n' || newLine)
					{
						DebugOutputCallback(filePath, lineNumber, funcName, level, isNotification, true, &DebugOutputLineBuffer[0]);
						DebugOutputLineLength = 0;
					}
					prevNewLineIndex = bIndex+1;
				}
				if (message[bIndex] == '\0') { break; }
			}
			#else //DEBUG_OUTPUT_CALLBACK_ONLY_ON_FINISHED_LINE
			DebugOutputCallback(filePath, lineNumber, funcName, level, newLine, message);
			#endif
		}
		#endif
		
		#if (DEBUG_OUTPUT_CALLBACK_ONLY_ON_FINISHED_LINE && DEBUG_OUTPUT_ERRORS_ON_LINE_OVERFLOW)
		if (!isLineOverflowOutput && DebugOutputLineOverflowOccurred)
		{
			DebugOutputRouter(filePath, lineNumber, funcName, DbgLevel_Error, isNotification, true, "DEBUG_OUTPUT_LINE_BUFFER_OVERFLOW!");
			DebugOutputLineOverflowOccurred = false;
		}
		#endif
		
		if (newLine) { DebugOutputIsOnNewLine = true; }
		else if (message != nullptr)
		{
			uxx messageLength = (uxx)MyStrLength(message);
			if (message != nullptr && message[messageLength-1] == '\n') { DebugOutputIsOnNewLine = true; }
			else { DebugOutputIsOnNewLine = false; }
		}
		else { DebugOutputIsOnNewLine = false; }
	}
}

// +==============================+
// |       DebugPrintRouter       |
// +==============================+
// void DebugPrintRouter(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel level, bool isNotification, bool newLine, uxx printBufferLength, char* printBuffer, const char* formatString, ...)
PEXP DEBUG_PRINT_HANDLER_DEF(DebugPrintRouter)
{
	#if !DEBUG_OUTPUT_SHOW_NOTIFICATIONS
	if (isNotification) { return; }
	#endif
	
	if ((level == DbgLevel_Debug   && ENABLE_DEBUG_OUTPUT_LEVEL_DEBUG)   ||
		(level == DbgLevel_Regular && ENABLE_DEBUG_OUTPUT_LEVEL_REGULAR) ||
		(level == DbgLevel_Info    && ENABLE_DEBUG_OUTPUT_LEVEL_INFO)    ||
		(level == DbgLevel_Notify  && ENABLE_DEBUG_OUTPUT_LEVEL_NOTIFY)  ||
		(level == DbgLevel_Other   && ENABLE_DEBUG_OUTPUT_LEVEL_OTHER)   ||
		(level == DbgLevel_Warning && ENABLE_DEBUG_OUTPUT_LEVEL_WARNING) ||
		(level == DbgLevel_Error   && ENABLE_DEBUG_OUTPUT_LEVEL_ERROR)   ||
		level == DbgLevel_None || level >= DbgLevel_Count)
	{
		// When doing multi-threaded applications we may decide to use a buffer allocated on the stack rather than reaching for a Scratch arena (the thread may not have initialized any scratch arenas)
		if (printBufferLength > 0 && printBuffer != nullptr)
		{
			va_list args;
			va_start(args, formatString);
			int printResult = MyVaListPrintf(printBuffer, printBufferLength, formatString, args);
			va_end(args);
			if (printResult >= 0)
			{
				printBuffer[printBufferLength-1] = '\0';
				DebugOutputRouter(filePath, lineNumber, funcName, level, isNotification, newLine, printBuffer);
			}
			#if DEBUG_OUTPUT_ERRORS_ON_FORMAT_FAILURE
			else
			{
				DebugOutputRouter(filePath, lineNumber, funcName, DbgLevel_Error, isNotification, false, "\nDEBUG_OUTPUT_BUFFER_PRINT_FAILED!\n\tFORMAT: \"");
				DebugOutputRouter(filePath, lineNumber, funcName, DbgLevel_Error, isNotification, false, formatString);
				DebugOutputRouter(filePath, lineNumber, funcName, DbgLevel_Error, isNotification, true, "\"");
			}
			#endif //DEBUG_OUTPUT_ERRORS_ON_FORMAT_FAILURE
		}
		else
		{
			ScratchBegin(scratch);
			if (scratch != nullptr)
			{
				PrintInArenaVa(scratch, messageStr, messageLength, formatString);
				if (messageLength >= 0 && (messageStr != nullptr || messageLength == 0))
				{
					DebugOutputRouter(filePath, lineNumber, funcName, level, isNotification, newLine, messageStr);
				}
				#if DEBUG_OUTPUT_ERRORS_ON_FORMAT_FAILURE
				else
				{
					DebugOutputRouter(filePath, lineNumber, funcName, DbgLevel_Error, isNotification, false, "\nDEBUG_OUTPUT_SCRATCH_PRINT_FAILED!\n\tFORMAT: \"");
					DebugOutputRouter(filePath, lineNumber, funcName, DbgLevel_Error, isNotification, false, formatString);
					DebugOutputRouter(filePath, lineNumber, funcName, DbgLevel_Error, isNotification, true, "\"");
				}
				#endif //DEBUG_OUTPUT_ERRORS_ON_FORMAT_FAILURE
			}
			else
			{
				DebugOutputRouter(filePath, lineNumber, funcName, DbgLevel_Error, isNotification, false, "\nNO_SCRATCH_FOR_DEBUG_OUTPUT_PRINT!\n\tFORMAT: \"");
				DebugOutputRouter(filePath, lineNumber, funcName, DbgLevel_Error, isNotification, false, formatString);
				DebugOutputRouter(filePath, lineNumber, funcName, DbgLevel_Error, isNotification, true, "\"");
			}
			ScratchEnd(scratch);
		}
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _BASE_DEBUG_OUTPUT_IMPL_H
