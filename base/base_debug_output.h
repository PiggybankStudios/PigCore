/*
File:   base_debug_output.h
Author: Taylor Robbins
Date:   01\18\2025
Description:
	** This file declares the method and macros by which all debug output from the
	** program should be routed through. These debug output messages can be routed 
	** to different places based on the platform we are compiling for and the #defines
	** options that are declared. We also have a callback global function pointer
	** so that debug output can be passed all the way up to the "application" level
	** to be handled in whatever way it wants. This callback allows game applications
	** to have a builtin terminal-like debug window that displays the debug output
	** of the application inside the graphical window. The application needs to store
	** all this debug output in a relatively complex data structure, which the
	** implementation of that data structure probably depends on things that depend
	** on the ability to do debug output, so this indirect callback global allows
	** for a very low level routing function to call "all the way up" to a level
	** where all the power of a regular application is available to handle it.
	** NOTE: Since the implementation of the routing functions depends on a few
	** things like mem_arena.h that want to be able to do debug output, this file
	** is split into a regular header (this one) and an implementation (base_debug_output_impl.h).
	** That way mem_arena.h can #include this file without a circular dependency.
	** This does require the application to include base_debug_output_impl.h if it
	** #includes any file that #includes this one.
*/

#ifndef _BASE_DEBUG_OUTPUT_H
#define _BASE_DEBUG_OUTPUT_H

#include "base/base_typedefs.h"
#include "base/base_dbg_level.h"

#ifndef DEBUG_OUTPUT_PRINT_LEVEL_PREFIX
#define DEBUG_OUTPUT_PRINT_LEVEL_PREFIX 0
#endif
#ifndef DEBUG_OUTPUT_LINE_BUFFER_SIZE
#define DEBUG_OUTPUT_LINE_BUFFER_SIZE 512
#endif
#ifndef DEBUG_OUTPUT_ERRORS_ON_LINE_OVERFLOW
#define DEBUG_OUTPUT_ERRORS_ON_LINE_OVERFLOW 1
#endif

#ifndef DEBUG_OUTPUT_TO_WIN32_OUTPUTDEBUGSTRING
#define DEBUG_OUTPUT_TO_WIN32_OUTPUTDEBUGSTRING 1
#endif

#ifndef DEBUG_OUTPUT_ERRORS_ON_FORMAT_FAILURE
#define DEBUG_OUTPUT_ERRORS_ON_FORMAT_FAILURE 1
#endif

#ifndef DEBUG_OUTPUT_CALLBACK_GLOBAL
#define DEBUG_OUTPUT_CALLBACK_GLOBAL 1
#endif
//NOTE: If this is true, then DebugOutputCallback will never be called with partial lines (i.e. newLine will always be true)
//      and each line is guaranteed to contain no \n characters (i.e. strings with \n characters will result in multiple callbacks)
#ifndef DEBUG_OUTPUT_CALLBACK_ONLY_ON_FINISHED_LINE
#define DEBUG_OUTPUT_CALLBACK_ONLY_ON_FINISHED_LINE 1
#endif

#ifndef ENABLE_DEBUG_OUTPUT_LEVEL_DEBUG
#define ENABLE_DEBUG_OUTPUT_LEVEL_DEBUG    1
#endif
#ifndef ENABLE_DEBUG_OUTPUT_LEVEL_REGULAR
#define ENABLE_DEBUG_OUTPUT_LEVEL_REGULAR  1
#endif
#ifndef ENABLE_DEBUG_OUTPUT_LEVEL_INFO
#define ENABLE_DEBUG_OUTPUT_LEVEL_INFO     1
#endif
#ifndef ENABLE_DEBUG_OUTPUT_LEVEL_NOTIFY
#define ENABLE_DEBUG_OUTPUT_LEVEL_NOTIFY   1
#endif
#ifndef ENABLE_DEBUG_OUTPUT_LEVEL_OTHER
#define ENABLE_DEBUG_OUTPUT_LEVEL_OTHER    1
#endif
#ifndef ENABLE_DEBUG_OUTPUT_LEVEL_WARNING
#define ENABLE_DEBUG_OUTPUT_LEVEL_WARNING  1
#endif
#ifndef ENABLE_DEBUG_OUTPUT_LEVEL_ERROR
#define ENABLE_DEBUG_OUTPUT_LEVEL_ERROR    1
#endif

#define DEBUG_OUTPUT_HANDLER_DEF(functionName) void functionName(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel level, bool newLine, const char* message)
typedef DEBUG_OUTPUT_HANDLER_DEF(DebugOutput_f);
#define DEBUG_PRINT_HANDLER_DEF(functionName) void functionName(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel level, bool newLine, const char* formatString, ...)
typedef DEBUG_PRINT_HANDLER_DEF(DebugPrint_f);

#if DEBUG_OUTPUT_CALLBACK_GLOBAL
extern DebugOutput_f* DebugOutputCallback;
#endif //DEBUG_OUTPUT_CALLBACK_GLOBAL

#if PIG_CORE_IMPLEMENTATION
PEXP DEBUG_OUTPUT_HANDLER_DEF(DebugOutputRouter);
PEXP DEBUG_PRINT_HANDLER_DEF(DebugPrintRouter);
#else
DEBUG_OUTPUT_HANDLER_DEF(DebugOutputRouter);
DEBUG_PRINT_HANDLER_DEF(DebugPrintRouter);
#endif

//NOTE: Calling the level specific macros below is preferrable because they can be compile-time removed if one of the ENABLE defines above is false
//      Using the ...At variants here will still guard against output of the debug string at runtime if the specific level is disabled but
//      the function call will still exist, and all the parameters for the format string will still be evaluated and passed to the function
//      so it's a little less efficient than using the level specific macro. These ...At variants should be used when the level at which the
//      output is being done is dependent upon some runtime value. For example outputting a warning or error based on some logic could do:
//      PrintLineAt(isError ? DbgLevel_Error : DbgLevel_Warning, "%s Occurred: %s", isError ? "ERROR" : "WARNING", message);
#define WriteAt(level, message)               DebugOutputRouter(__FILE__, __LINE__, __func__, (level), false, (message))
#define WriteLineAt(level, message)           DebugOutputRouter(__FILE__, __LINE__, __func__, (level), true,  (message))
#define PrintAt(level, formatString, ...)     DebugPrintRouter (__FILE__, __LINE__, __func__, (level), false, (formatString), ##__VA_ARGS__)
#define PrintLineAt(level, formatString, ...) DebugPrintRouter (__FILE__, __LINE__, __func__, (level), true,  (formatString), ##__VA_ARGS__)

#if ENABLE_DEBUG_OUTPUT_LEVEL_DEBUG
#define Write_D(message)               DebugOutputRouter(__FILE__, __LINE__, __func__, DbgLevel_Debug, false, (message))
#define WriteLine_D(message)           DebugOutputRouter(__FILE__, __LINE__, __func__, DbgLevel_Debug, true,  (message))
#define Print_D(formatString, ...)     DebugPrintRouter (__FILE__, __LINE__, __func__, DbgLevel_Debug, false, (formatString), ##__VA_ARGS__)
#define PrintLine_D(formatString, ...) DebugPrintRouter (__FILE__, __LINE__, __func__, DbgLevel_Debug, true,  (formatString), ##__VA_ARGS__)
#else
#define Write_D(message)               //nothing
#define WriteLine_D(message)           //nothing
#define Print_D(formatString, ...)     //nothing
#define PrintLine_D(formatString, ...) //nothing
#endif

#if ENABLE_DEBUG_OUTPUT_LEVEL_REGULAR
#define Write_R(message)               DebugOutputRouter(__FILE__, __LINE__, __func__, DbgLevel_Regular, false, (message))
#define WriteLine_R(message)           DebugOutputRouter(__FILE__, __LINE__, __func__, DbgLevel_Regular, true,  (message))
#define Print_R(formatString, ...)     DebugPrintRouter (__FILE__, __LINE__, __func__, DbgLevel_Regular, false, (formatString), ##__VA_ARGS__)
#define PrintLine_R(formatString, ...) DebugPrintRouter (__FILE__, __LINE__, __func__, DbgLevel_Regular, true,  (formatString), ##__VA_ARGS__)
#else
#define Write_R(message)               //nothing
#define WriteLine_R(message)           //nothing
#define Print_R(formatString, ...)     //nothing
#define PrintLine_R(formatString, ...) //nothing
#endif

#if ENABLE_DEBUG_OUTPUT_LEVEL_INFO
#define Write_I(message)               DebugOutputRouter(__FILE__, __LINE__, __func__, DbgLevel_Info, false, (message))
#define WriteLine_I(message)           DebugOutputRouter(__FILE__, __LINE__, __func__, DbgLevel_Info, true,  (message))
#define Print_I(formatString, ...)     DebugPrintRouter (__FILE__, __LINE__, __func__, DbgLevel_Info, false, (formatString), ##__VA_ARGS__)
#define PrintLine_I(formatString, ...) DebugPrintRouter (__FILE__, __LINE__, __func__, DbgLevel_Info, true,  (formatString), ##__VA_ARGS__)
#else
#define Write_I(message)               //nothing
#define WriteLine_I(message)           //nothing
#define Print_I(formatString, ...)     //nothing
#define PrintLine_I(formatString, ...) //nothing
#endif

#if ENABLE_DEBUG_OUTPUT_LEVEL_NOTIFY
#define Write_N(message)               DebugOutputRouter(__FILE__, __LINE__, __func__, DbgLevel_Notify, false, (message))
#define WriteLine_N(message)           DebugOutputRouter(__FILE__, __LINE__, __func__, DbgLevel_Notify, true,  (message))
#define Print_N(formatString, ...)     DebugPrintRouter (__FILE__, __LINE__, __func__, DbgLevel_Notify, false, (formatString), ##__VA_ARGS__)
#define PrintLine_N(formatString, ...) DebugPrintRouter (__FILE__, __LINE__, __func__, DbgLevel_Notify, true,  (formatString), ##__VA_ARGS__)
#else
#define Write_N(message)               //nothing
#define WriteLine_N(message)           //nothing
#define Print_N(formatString, ...)     //nothing
#define PrintLine_N(formatString, ...) //nothing
#endif

#if ENABLE_DEBUG_OUTPUT_LEVEL_OTHER
#define Write_O(message)               DebugOutputRouter(__FILE__, __LINE__, __func__, DbgLevel_Other, false, (message))
#define WriteLine_O(message)           DebugOutputRouter(__FILE__, __LINE__, __func__, DbgLevel_Other, true,  (message))
#define Print_O(formatString, ...)     DebugPrintRouter (__FILE__, __LINE__, __func__, DbgLevel_Other, false, (formatString), ##__VA_ARGS__)
#define PrintLine_O(formatString, ...) DebugPrintRouter (__FILE__, __LINE__, __func__, DbgLevel_Other, true,  (formatString), ##__VA_ARGS__)
#else
#define Write_O(message)               //nothing
#define WriteLine_O(message)           //nothing
#define Print_O(formatString, ...)     //nothing
#define PrintLine_O(formatString, ...) //nothing
#endif

#if ENABLE_DEBUG_OUTPUT_LEVEL_WARNING
#define Write_W(message)               DebugOutputRouter(__FILE__, __LINE__, __func__, DbgLevel_Warning, false, (message))
#define WriteLine_W(message)           DebugOutputRouter(__FILE__, __LINE__, __func__, DbgLevel_Warning, true,  (message))
#define Print_W(formatString, ...)     DebugPrintRouter (__FILE__, __LINE__, __func__, DbgLevel_Warning, false, (formatString), ##__VA_ARGS__)
#define PrintLine_W(formatString, ...) DebugPrintRouter (__FILE__, __LINE__, __func__, DbgLevel_Warning, true,  (formatString), ##__VA_ARGS__)
#else
#define Write_W(message)               //nothing
#define WriteLine_W(message)           //nothing
#define Print_W(formatString, ...)     //nothing
#define PrintLine_W(formatString, ...) //nothing
#endif

#if ENABLE_DEBUG_OUTPUT_LEVEL_ERROR
#define Write_E(message)               DebugOutputRouter(__FILE__, __LINE__, __func__, DbgLevel_Error, false, (message))
#define WriteLine_E(message)           DebugOutputRouter(__FILE__, __LINE__, __func__, DbgLevel_Error, true,  (message))
#define Print_E(formatString, ...)     DebugPrintRouter (__FILE__, __LINE__, __func__, DbgLevel_Error, false, (formatString), ##__VA_ARGS__)
#define PrintLine_E(formatString, ...) DebugPrintRouter (__FILE__, __LINE__, __func__, DbgLevel_Error, true,  (formatString), ##__VA_ARGS__)
#else
#define Write_E(message)               //nothing
#define WriteLine_E(message)           //nothing
#define Print_E(formatString, ...)     //nothing
#define PrintLine_E(formatString, ...) //nothing
#endif

#endif //  _BASE_DEBUG_OUTPUT_H
