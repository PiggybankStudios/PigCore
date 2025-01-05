/*
File:   misc_dbg_level.h
Author: Taylor Robbins
Date:   01\05\2025
*/

#ifndef _MISC_DBG_LEVEL_H
#define _MISC_DBG_LEVEL_H

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
const char* GetDbgLevelStr(DbgLevel enumValue)
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
		default: return "Unknown";
	}
}

#endif //  _MISC_DBG_LEVEL_H
