/*
File:   cross_gfx_system_global_and_helpers.h
Author: Taylor Robbins
Date:   11\14\2025
*/

#ifndef _CROSS_GFX_SYSTEM_GLOBAL_AND_HELPERS_H
#define _CROSS_GFX_SYSTEM_GLOBAL_AND_HELPERS_H

//NOTE: Intentionally no includes here

#if BUILD_WITH_SOKOL_GFX

#define DrawTextEmbossedEx(text, position, color, embossColor, embossSides, embossOffset) GfxSystem_DrawTextEmbossedEx(&gfx, (text), (position), (color), (embossColor), (embossSides), (embossOffset))
#define DrawTextEmbossed(text, position, color)                                           GfxSystem_DrawTextEmbossed(&gfx, (text), (position), (color))

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _CROSS_GFX_SYSTEM_GLOBAL_AND_HELPERS_H
