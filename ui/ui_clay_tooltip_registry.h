/*
File:   ui_clay_tooltip_registry.h
Author: Taylor Robbins
Date:   12\04\2025
Description:
	** The TooltipRegistry holds a list of all potential tooltips that are currently availble for the user to hover over
	** This file is separate from ui_clay_tooltip.h because a TooltipRegistar pointer is included in UiWidgetContext so any widget can register tooltips if it has the context
*/

#ifndef _UI_CLAY_TOOLTIP_REGISTRY_H
#define _UI_CLAY_TOOLTIP_REGISTRY_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
#include "struct/struct_string.h"
#include "struct/struct_var_array.h"
#include "ui/ui_clay.h"
#include "gfx/gfx_font.h"

#if BUILD_WITH_CLAY && BUILD_WITH_SOKOL_GFX

#define TOOLTIP_ID_INVALID  0

typedef plex RegisteredTooltip RegisteredTooltip;
plex RegisteredTooltip
{
	u64 id;
	bool active;
	Str8 targetClayIdStr;
	rec targetRec;
	
	Str8 displayStr;
	PigFont* font;
	r32 fontSize;
	u8 fontStyle;
};

typedef plex TooltipRegistry TooltipRegistry;
plex TooltipRegistry
{
	Arena* arena;
	u64 nextTooltipId;
	VarArray tooltips; //RegisteredTooltip
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeRegisteredTooltip(TooltipRegistry* registry, RegisteredTooltip* tooltip);
	PIG_CORE_INLINE void FreeTooltipRegistry(TooltipRegistry* registry);
	PIG_CORE_INLINE void InitTooltipRegistry(Arena* arena, TooltipRegistry* registryOut);
	PIG_CORE_INLINE void UnregisterTooltip(TooltipRegistry* registry, u64 id);
	PIG_CORE_INLINE RegisteredTooltip* TryFindRegisteredTooltip(TooltipRegistry* registry, u64 id);
	PIG_CORE_INLINE RegisteredTooltip* RegisterTooltipGetPntr(TooltipRegistry* registry, Str8 targetClayIdStr, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle);
	PIG_CORE_INLINE u64 RegisterTooltip(TooltipRegistry* registry, Str8 targetClayIdStr, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void FreeRegisteredTooltip(TooltipRegistry* registry, RegisteredTooltip* tooltip)
{
	NotNull(registry);
	NotNull(tooltip);
	if (!IsEmptyStr(tooltip->targetClayIdStr)) { FreeStr8(registry->arena, &tooltip->targetClayIdStr); }
	FreeStr8(registry->arena, &tooltip->displayStr);
	ClearPointer(tooltip);
}

PEXPI void FreeTooltipRegistry(TooltipRegistry* registry)
{
	NotNull(registry);
	if (registry->arena != nullptr)
	{
		VarArrayLoop(&registry->tooltips, tIndex)
		{
			VarArrayLoopGet(RegisteredTooltip, tooltip, &registry->tooltips, tIndex);
			FreeRegisteredTooltip(registry, tooltip);
		}
		FreeVarArray(&registry->tooltips);
	}
	ClearPointer(registry);
}

PEXPI void InitTooltipRegistry(Arena* arena, TooltipRegistry* registryOut)
{
	NotNull(arena);
	NotNull(registryOut);
	ClearPointer(registryOut);
	registryOut->arena = arena;
	registryOut->nextTooltipId = 1;
	InitVarArray(RegisteredTooltip, &registryOut->tooltips, arena);
}

PEXPI RegisteredTooltip* TryFindRegisteredTooltip(TooltipRegistry* registry, u64 id)
{
	NotNull(registry);
	NotNull(registry->arena);
	if (id == TOOLTIP_ID_INVALID) { return nullptr; }
	VarArrayLoop(&registry->tooltips, tIndex)
	{
		VarArrayLoopGet(RegisteredTooltip, tooltip, &registry->tooltips, tIndex);
		if (tooltip->id == id) { return tooltip; }
	}
	return nullptr;
}

PEXPI void UnregisterTooltip(TooltipRegistry* registry, u64 id)
{
	NotNull(registry);
	NotNull(registry->arena);
	if (id == TOOLTIP_ID_INVALID) { return; }
	VarArrayLoop(&registry->tooltips, tIndex)
	{
		VarArrayLoopGet(RegisteredTooltip, tooltip, &registry->tooltips, tIndex);
		if (tooltip->id == id)
		{
			FreeRegisteredTooltip(registry, tooltip);
			VarArrayRemoveAt(RegisteredTooltip, &registry->tooltips, tIndex);
			break;
		}
	}
}

PEXPI RegisteredTooltip* RegisterTooltipGetPntr(TooltipRegistry* registry, Str8 targetClayIdStr, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle)
{
	NotNull(registry);
	NotNull(registry->arena);
	NotNullStr(displayStr);
	RegisteredTooltip* newTooltip = VarArrayAdd(RegisteredTooltip, &registry->tooltips);
	if (newTooltip == nullptr) { return nullptr; }
	ClearPointer(newTooltip);
	newTooltip->id = registry->nextTooltipId;
	registry->nextTooltipId++;
	newTooltip->targetClayIdStr = IsEmptyStr(targetClayIdStr) ? targetClayIdStr : AllocStr8(registry->arena, targetClayIdStr);
	newTooltip->targetRec = targetRec;
	newTooltip->displayStr = AllocStr8(registry->arena, displayStr);
	newTooltip->font = font;
	newTooltip->fontSize = fontSize;
	newTooltip->fontStyle = fontStyle;
	return newTooltip;
}
PEXPI u64 RegisterTooltip(TooltipRegistry* registry, Str8 targetClayIdStr, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle)
{
	RegisteredTooltip* tooltip = RegisterTooltipGetPntr(registry, targetClayIdStr, targetRec, displayStr, font, fontSize, fontStyle);
	return (tooltip != nullptr) ? tooltip->id : TOOLTIP_ID_INVALID;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_CLAY && BUILD_WITH_SOKOL_GFX

#endif //  _UI_CLAY_TOOLTIP_REGISTRY_H
