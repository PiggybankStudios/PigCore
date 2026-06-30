/*
File:   gfx_helpers.h
Author: Taylor Robbins
Date:   11\14\2025
Description:
	** These are all functions that help do common effects and drawing
	** patterns through the GfxSystem API but don't belong in the main API
*/

#ifndef _GFX_HELPERS_H
#define _GFX_HELPERS_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "struct/struct_vectors.h"
#include "struct/struct_directions.h"
#include "struct/struct_color.h"
#include "struct/struct_string.h"
#include "gfx/gfx_font.h"
#include "gfx/gfx_font_flow.h"
#include "gfx/gfx_texture.h"
#include "gfx/gfx_system.h"

#if BUILD_WITH_SOKOL_GFX

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void GfxSystem_DrawTextEmbossedEx(GfxSystem* system, Str8 text, v2 position, Color32 color, Color32 embossColor, u8 embossSides, r32 embossOffset);
	PIG_CORE_INLINE void GfxSystem_DrawTextEmbossed(GfxSystem* system, Str8 text, v2 position, Color32 color);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

// NOTE: "Embossing" is just a crude technique for getting an outline-like effect without shaders or offscreen rendering
//       We simply render the same text multiple times, the first time we offset it by some small amount and make it a dark color
PEXP void GfxSystem_DrawTextEmbossedEx(GfxSystem* system, Str8 text, v2 position, Color32 color, Color32 embossColor, u8 embossSides, r32 embossOffset)
{
	if (IsFlagSet(embossSides, Dir2_Right)) { GfxSystem_DrawText(system, text, AddV2(position, MakeV2( embossOffset, 0)), embossColor); }
	if (IsFlagSet(embossSides, Dir2_Left))  { GfxSystem_DrawText(system, text, AddV2(position, MakeV2(-embossOffset, 0)), embossColor); }
	if (IsFlagSet(embossSides, Dir2_Down))  { GfxSystem_DrawText(system, text, AddV2(position, MakeV2(0,  embossOffset)), embossColor); }
	if (IsFlagSet(embossSides, Dir2_Up))    { GfxSystem_DrawText(system, text, AddV2(position, MakeV2(0, -embossOffset)), embossColor); }
	GfxSystem_DrawText(system, text, position, color);
}
PEXPI void GfxSystem_DrawTextEmbossed(GfxSystem* system, Str8 text, v2 position, Color32 color)
{
	GfxSystem_DrawTextEmbossedEx(system, text, position, color, Black, Dir2_All, 1.0f);
}

//TODO: Emboss functions for:
//    GfxSystem_DrawWrappedRichTextWithFont
//    GfxSystem_DrawRichTextWithFont
//    GfxSystem_DrawTextWithFont
//    GfxSystem_DrawTextAtSize
//    GfxSystem_DrawTextBold
//    GfxSystem_DrawTextItalic
//    GfxSystem_DrawWrappedText

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_SOKOL_GFX

#endif //  _GFX_HELPERS_H

#if defined(_GFX_SYSTEM_GLOBAL_H) && defined(_GFX_HELPERS_H)
#include "cross/cross_gfx_system_global_and_helpers.h"
#endif
