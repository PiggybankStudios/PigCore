/*
File:   ui_clay_tooltip_registry.h
Author: Taylor Robbins
Date:   12\04\2025
Description:
	** The TooltipRegistry holds a list of all potential tooltips that are currently availble for the user to hover over
	** This file is separate from ui_clay_tooltip.h because a TooltipRegistar pointer is included in UiWidgetContext so any widget can register tooltips if it has the context
	** Much of the logic for tooltips happens in ui_clay_tooltip.h but the state is defined here which is a little unfortunate
	** Tooltips can be attached to a specific Clay UI element by ID or they can be attached to a rectangle on screen
	** Tooltips show up after the mouse is hovering over the target element\rectangle and TOOLTIP_HOVER_DELAY has elapsed since the mouse moved
	** When a tooltip opens, it needs to position itself somewhere that is not under the mouse and is entirely inside the window, so the exact location of the tooltip is somewhat hard to predict
	** Tooltips should be displayed on top of nearly all UI elements, even things like ui_clay_notifications.h since nearly all UI can have tooltips attached and the tooltip is more closely tied to the mouse itself rather than the UI layers for any particular application
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

//TODO: We maye eventually want to support tooltip behavior in applications that don't use Clay
#if BUILD_WITH_CLAY && BUILD_WITH_SOKOL_GFX

#define TOOLTIP_ID_INVALID   0

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
	
	u64 hoverTooltipId;
	u64 hoverTooltipChangeTime;
	u64 lastMouseMoveTime;
	
	u64 openTooltipId;
	v2 openTooltipTargetPos;
	u64 openTooltipTime;
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
	PIG_CORE_INLINE void UpdateTooltipActive(TooltipRegistry* registry, u64 tooltipId, bool isActive);
	PIG_CORE_INLINE void UpdateTooltipFont(TooltipRegistry* registry, u64 tooltipId, PigFont* font, r32 fontSize, u8 fontStyle);
	PIG_CORE_INLINE void UpdateTooltipDisplayStr(TooltipRegistry* registry, u64 tooltipId, Str8 displayStr);
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
	registryOut->hoverTooltipId = TOOLTIP_ID_INVALID;
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
	newTooltip->active = true;
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

PEXPI void UpdateTooltipActive(TooltipRegistry* registry, u64 tooltipId, bool isActive)
{
	NotNull(registry);
	NotNull(registry->arena);
	RegisteredTooltip* tooltip = TryFindRegisteredTooltip(registry, tooltipId);
	if (tooltip == nullptr) { return; }
	tooltip->active = isActive;
}
PEXPI void UpdateTooltipFont(TooltipRegistry* registry, u64 tooltipId, PigFont* font, r32 fontSize, u8 fontStyle)
{
	NotNull(registry);
	NotNull(registry->arena);
	RegisteredTooltip* tooltip = TryFindRegisteredTooltip(registry, tooltipId);
	if (tooltip == nullptr) { return; }
	tooltip->font = font;
	tooltip->fontSize = fontSize;
	tooltip->fontStyle = fontStyle;
}
PEXPI void UpdateTooltipDisplayStr(TooltipRegistry* registry, u64 tooltipId, Str8 displayStr)
{
	NotNull(registry);
	NotNull(registry->arena);
	RegisteredTooltip* tooltip = TryFindRegisteredTooltip(registry, tooltipId);
	if (tooltip == nullptr) { return; }
	if (StrExactEquals(tooltip->displayStr, displayStr)) { return; }
	FreeStr8(registry->arena, &tooltip->displayStr);
	tooltip->displayStr = AllocStr8(registry->arena, displayStr);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_CLAY && BUILD_WITH_SOKOL_GFX

#endif //  _UI_CLAY_TOOLTIP_REGISTRY_H
