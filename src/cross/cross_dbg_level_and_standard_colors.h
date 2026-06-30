/*
File:   cross_dbg_level_and_standard_colors.h
Author: Taylor Robbins
Date:   01\18\2025
*/

#ifndef _CROSS_DBG_LEVEL_AND_STANDARD_COLORS_H
#define _CROSS_DBG_LEVEL_AND_STANDARD_COLORS_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Color32 GetDbgLevelTextColor(DbgLevel dbgLevel);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI Color32 GetDbgLevelTextColor(DbgLevel dbgLevel)
{
	switch (dbgLevel)
	{
		case DbgLevel_Debug:   return MonokaiGray1;     break;
		case DbgLevel_Regular: return MonokaiWhite;     break;
		case DbgLevel_Info:    return MonokaiGreen;     break;
		case DbgLevel_Notify:  return MonokaiPurple;    break;
		case DbgLevel_Other:   return MonokaiLightBlue; break;
		case DbgLevel_Warning: return MonokaiOrange;    break;
		case DbgLevel_Error:   return MonokaiMagenta;   break;
		default: return MonokaiWhite; break;
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_DBG_LEVEL_AND_STANDARD_COLORS_H
