/*
File:   base_dbg_level.h
Author: Taylor Robbins
Date:   01\05\2025
Description:
	** This file simply holds an enum with values representing various "levels" of
	** debug output that we might do. Debug output being strings that get routed to
	** stdout, or a file, or other places depending on the operating system.
	** The "levels" serve two purposes, they usually indicate severity or importance
	** of a particular debug output line and therefore can be a mechanism by which
	** we disable less important debug output lines while keeping more important
	** ones disabled (see base_debug_otuput.h for how to disable each level)
	** The second purpose is to give the line a particular color when being displayed
	** in a context where color is something we can choose (Usually in a game we'll
	** have a built-in debug console that we render ourselves so we can render color).
	** This coloring ends up being the more important reason for debug levels since
	** it serves the primary iteration workflow in day-to-day app\game development
	** (we don't usually need to turn off debug levels, but seeing an error show up
	** as a red line in the debug console makes debugging WAY easier).
	** The color choice is also the main distinction behind the levels that might be
	** ambiguous: Regular (White) | Info (Green) | Notify (Purple) | Other (Blue).
*/

#ifndef _BASE_DBG_LEVEL_H
#define _BASE_DBG_LEVEL_H

typedef enum DbgLevel DbgLevel;
enum DbgLevel
{
	DbgLevel_None = 0,
	DbgLevel_Debug,
	DbgLevel_Regular,
	DbgLevel_Info,
	DbgLevel_Notify,
	DbgLevel_Other,
	DbgLevel_Warning,
	DbgLevel_Error,
	DbgLevel_Count,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetDbgLevelStr(DbgLevel enumValue);
#else
PEXP const char* GetDbgLevelStr(DbgLevel enumValue)
{
	switch (enumValue)
	{
		case DbgLevel_None:    return "None";
		case DbgLevel_Debug:   return "Debug";
		case DbgLevel_Regular: return "Regular";
		case DbgLevel_Info:    return "Info";
		case DbgLevel_Notify:  return "Notify";
		case DbgLevel_Other:   return "Other";
		case DbgLevel_Warning: return "Warning";
		case DbgLevel_Error:   return "Error";
		default: return UNKNOWN_STR;
	}
}
#endif //PIG_CORE_IMPLEMENTATION

#endif //  _BASE_DBG_LEVEL_H

#if defined(_BASE_DBG_LEVEL_H) && defined(_MISC_STANDARD_COLORS_H)
#include "cross/cross_dbg_level_and_standard_colors.h"
#endif
