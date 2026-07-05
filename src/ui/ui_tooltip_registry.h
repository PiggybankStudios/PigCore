/*
File:   ui_tooltip_registry.h
Author: Taylor Robbins
Date:   12\04\2025
Description:
	** The TooltipRegistry holds a list of all potential tooltips that are currently availble for the user to hover over
	** This file is separate from ui_clay_tooltip.h because a TooltipRegistar pointer is included in UiWidgetContext so any widget can register tooltips if it has the context
	** Much of the logic for tooltips happens in ui_clay_tooltip.h but the state is defined here which is a little unfortunate
	** Tooltips can be attached to a specific Clay UI element by ID or they can be attached to a rectangle on screen
	** Tooltips show up after the mouse is hovering over the target element\rectangle and TOOLTIP_HOVER_DELAY has elapsed since the mouse moved
	** When a tooltip opens, it needs to position itself somewhere that is not under the mouse and is entirely inside the window, so the exact location of the tooltip is somewhat hard to predict
	** Tooltips should be displayed on top of nearly all UI elements, even things like ui_notifications.h since nearly all UI can have tooltips attached and the tooltip is more closely tied to the mouse itself rather than the UI layers for any particular application
	** Tooltips can be registered manually through RegisterTooltip\UnregisterTooltip OR they can be registered through CLAY macros which is the preferred method since it's more ergonomic. Simply fill out .tooltip inside the Clay_ElementDeclaration struct
*/

#ifndef _UI_TOOLTIP_REGISTRY_H
#define _UI_TOOLTIP_REGISTRY_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
#include "struct/struct_string.h"
#include "struct/struct_var_array.h"
#include "ui/ui_clay.h"
#include "ui/ui_system_core.h"
#include "gfx/gfx_font.h"

#if (BUILD_WITH_CLAY || BUILD_WITH_PIG_UI) && BUILD_WITH_SOKOL_GFX

#define TOOLTIP_ID_INVALID   0

typedef plex RegisteredTooltip RegisteredTooltip;
plex RegisteredTooltip
{
	u64 id;
	bool active;
	#if BUILD_WITH_CLAY
	Str8 targetUiIdStr;
	uxx targetUiIdIndex;
	ClayId targetContainerClayId;
	#elif BUILD_WITH_PIG_UI
	UiId targetUiId;
	UiId targetContainerId;
	#endif
	rec targetRec; //only used if targetUiIdStr is empty
	bool autoUnregister;
	bool registeredThisFrame;
	
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
	PIG_CORE_INLINE void UpdateTooltipRegistry(TooltipRegistry* registry);
	PIG_CORE_INLINE void UnregisterTooltip(TooltipRegistry* registry, u64 id);
	PIG_CORE_INLINE RegisteredTooltip* TryFindRegisteredTooltip(TooltipRegistry* registry, u64 id);
	#if BUILD_WITH_CLAY
	PIG_CORE_INLINE RegisteredTooltip* TryFindRegisteredTooltipByClayId(TooltipRegistry* registry, ClayId targetClayId);
	#elif BUILD_WITH_PIG_UI
	PIG_CORE_INLINE RegisteredTooltip* TryFindRegisteredTooltipByUiId(TooltipRegistry* registry, UiId targetUiId);
	#endif
	#if BUILD_WITH_CLAY
	PIG_CORE_INLINE RegisteredTooltip* RegisterTooltipGetPntr(TooltipRegistry* registry, bool autoUnregister, Str8 targetUiIdStr, uxx targetUiIdIndex, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle);
	PIG_CORE_INLINE u64 RegisterTooltip(TooltipRegistry* registry, bool autoUnregister, Str8 targetUiIdStr, uxx targetUiIdIndex, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle);
	#elif BUILD_WITH_PIG_UI
	PIG_CORE_INLINE RegisteredTooltip* RegisterTooltipGetPntr(TooltipRegistry* registry, bool autoUnregister, UiId targetUiId, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle);
	PIG_CORE_INLINE u64 RegisterTooltip(TooltipRegistry* registry, bool autoUnregister, UiId targetUiId, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle);
	#endif
	PIG_CORE_INLINE void UpdateTooltipActive(TooltipRegistry* registry, u64 tooltipId, bool isActive);
	PIG_CORE_INLINE void UpdateTooltipFont(TooltipRegistry* registry, u64 tooltipId, PigFont* font, r32 fontSize, u8 fontStyle);
	PIG_CORE_INLINE void UpdateTooltipDisplayStr(TooltipRegistry* registry, u64 tooltipId, Str8 displayStr);
	#if BUILD_WITH_CLAY
	PIG_CORE_INLINE RegisteredTooltip* SoftRegisterTooltipGetPntr(TooltipRegistry* registry, u64 existingTooltipId, Str8 targetUiIdStr, uxx targetUiIdIndex, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle);
	PIG_CORE_INLINE u64 SoftRegisterTooltip(TooltipRegistry* registry, u64 existingTooltipId, Str8 targetUiIdStr, uxx targetUiIdIndex, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle);
	PIG_CORE_INLINE RegisteredTooltip* SoftRegisterTooltipByClayIdGetPntr(TooltipRegistry* registry, ClayId clayId, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle);
	PIG_CORE_INLINE u64 SoftRegisterTooltipByClayId(TooltipRegistry* registry, ClayId clayId, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle);
	#elif BUILD_WITH_PIG_UI
	PIG_CORE_INLINE RegisteredTooltip* SoftRegisterTooltipGetPntr(TooltipRegistry* registry, UiId targetUiId, uxx targetUiIdIndex, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle);
	PIG_CORE_INLINE u64 SoftRegisterTooltip(TooltipRegistry* registry, u64 existingTooltipId, UiId targetUiId, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle);
	PIG_CORE_INLINE RegisteredTooltip* SoftRegisterTooltipByUiIdGetPntr(TooltipRegistry* registry, UiId uiId, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle);
	PIG_CORE_INLINE u64 SoftRegisterTooltipByUiId(TooltipRegistry* registry, UiId uiId, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle);
	#endif
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void FreeRegisteredTooltip(TooltipRegistry* registry, RegisteredTooltip* tooltip)
{
	NotNull(registry);
	NotNull(tooltip);
	#if BUILD_WITH_CLAY
	if (!IsEmptyStr(tooltip->targetUiIdStr)) { FreeStr8(registry->arena, &tooltip->targetUiIdStr); }
	#elif BUILD_WITH_PIG_UI
	if (!IsEmptyStr(tooltip->targetUiId.str)) { FreeStr8(registry->arena, &tooltip->targetUiId.str); }
	#endif
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

PEXPI void UpdateTooltipRegistry(TooltipRegistry* registry)
{
	NotNull(registry);
	NotNull(registry->arena);
	
	VarArrayLoop(&registry->tooltips, tIndex)
	{
		VarArrayLoopGet(RegisteredTooltip, tooltip, &registry->tooltips, tIndex);
		if (tooltip->autoUnregister && !tooltip->registeredThisFrame)
		{
			FreeRegisteredTooltip(registry, tooltip);
			VarArrayRemoveAt(RegisteredTooltip, &registry->tooltips, tIndex);
			tIndex--;
			continue;
		}
		else { tooltip->registeredThisFrame = false; }
	}
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
#if BUILD_WITH_CLAY
PEXPI RegisteredTooltip* TryFindRegisteredTooltipByClayId(TooltipRegistry* registry, ClayId targetClayId)
{
	NotNull(registry);
	NotNull(registry->arena);
	if (targetClayId.id == ClayId_Invalid.id) { return nullptr; }
	VarArrayLoop(&registry->tooltips, tIndex)
	{
		VarArrayLoopGet(RegisteredTooltip, tooltip, &registry->tooltips, tIndex);
		if (!IsEmptyStr(tooltip->targetUiIdStr))
		{
			ClayId tooltipTargetClayId = ToClayIdEx(tooltip->targetUiIdStr, tooltip->targetUiIdIndex);
			if (tooltipTargetClayId.id == targetClayId.id) { return tooltip; }
		}
	}
	return nullptr;
}
#elif BUILD_WITH_PIG_UI
PEXPI RegisteredTooltip* TryFindRegisteredTooltipByUiId(TooltipRegistry* registry, UiId targetUiId)
{
	NotNull(registry);
	NotNull(registry->arena);
	if (targetUiId.id == 0) { return nullptr; }
	VarArrayLoop(&registry->tooltips, tIndex)
	{
		VarArrayLoopGet(RegisteredTooltip, tooltip, &registry->tooltips, tIndex);
		if (tooltip->targetUiId.id == targetUiId.id) { return tooltip; }
	}
	return nullptr;
}
#endif //BUILD_WITH_CLAY/BUILD_WITH_PIG_UI

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

#if BUILD_WITH_CLAY
PEXPI RegisteredTooltip* RegisterTooltipGetPntr(TooltipRegistry* registry, bool autoUnregister, Str8 targetUiIdStr, uxx targetUiIdIndex, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle)
#elif BUILD_WITH_PIG_UI
PEXPI RegisteredTooltip* RegisterTooltipGetPntr(TooltipRegistry* registry, bool autoUnregister, UiId targetUiId, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle)
#endif
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
	newTooltip->autoUnregister = autoUnregister;
	#if BUILD_WITH_CLAY
	newTooltip->targetUiIdStr = IsEmptyStr(targetUiIdStr) ? targetUiIdStr : AllocStr8(registry->arena, targetUiIdStr);
	newTooltip->targetUiIdIndex = targetUiIdIndex;
	#elif BUILD_WITH_PIG_UI
	newTooltip->targetUiId = targetUiId;
	newTooltip->targetUiId.str = IsEmptyStr(targetUiId.str) ? targetUiId.str : AllocStr8(registry->arena, targetUiId.str);
	#endif
	newTooltip->targetRec = targetRec;
	newTooltip->displayStr = AllocStr8(registry->arena, displayStr);
	newTooltip->font = font;
	newTooltip->fontSize = fontSize;
	newTooltip->fontStyle = fontStyle;
	newTooltip->registeredThisFrame = true;
	return newTooltip;
}
#if BUILD_WITH_CLAY
PEXPI u64 RegisterTooltip(TooltipRegistry* registry, bool autoUnregister, Str8 targetUiIdStr, uxx targetUiIdIndex, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle)
#elif BUILD_WITH_PIG_UI
PEXPI u64 RegisterTooltip(TooltipRegistry* registry, bool autoUnregister, UiId targetUiId, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle)
#endif
{
	#if BUILD_WITH_CLAY
	RegisteredTooltip* tooltip = RegisterTooltipGetPntr(registry, autoUnregister, targetUiIdStr, targetUiIdIndex, targetRec, displayStr, font, fontSize, fontStyle);
	#elif BUILD_WITH_PIG_UI
	RegisteredTooltip* tooltip = RegisterTooltipGetPntr(registry, autoUnregister, targetUiId, targetRec, displayStr, font, fontSize, fontStyle);
	#endif
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

// "Soft" means that if the tooltip was already registered then just update any fields that have changed. This implies autoUnregister so there is no need to call UnregisterTooltip with Soft registration
#if BUILD_WITH_CLAY
PEXPI RegisteredTooltip* SoftRegisterTooltipGetPntr(TooltipRegistry* registry, u64 existingTooltipId, Str8 targetUiIdStr, uxx targetUiIdIndex, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle)
{
	RegisteredTooltip* existingTooltip = TryFindRegisteredTooltip(registry, existingTooltipId);
	if (existingTooltip != nullptr)
	{
		if (!StrExactEquals(existingTooltip->targetUiIdStr, targetUiIdStr))
		{
			if (!IsEmptyStr(existingTooltip->targetUiIdStr)) { FreeStr8(registry->arena, &existingTooltip->targetUiIdStr); }
			existingTooltip->targetUiIdStr = IsEmptyStr(targetUiIdStr) ? targetUiIdStr : AllocStr8(registry->arena, targetUiIdStr);
		}
		if (!StrExactEquals(existingTooltip->displayStr, displayStr))
		{
			FreeStr8(registry->arena, &existingTooltip->displayStr);
			existingTooltip->displayStr = AllocStr8(registry->arena, displayStr);
		}
		existingTooltip->targetRec = targetRec;
		existingTooltip->font = font;
		existingTooltip->fontSize = fontSize;
		existingTooltip->fontStyle = fontStyle;
		
		existingTooltip->registeredThisFrame = true;
		return existingTooltip;
	}
	else
	{
		return RegisterTooltipGetPntr(registry, true, targetUiIdStr, targetUiIdIndex, targetRec, displayStr, font, fontSize, fontStyle);
	}
}
#elif BUILD_WITH_PIG_UI
PEXPI RegisteredTooltip* SoftRegisterTooltipGetPntr(TooltipRegistry* registry, u64 existingTooltipId, UiId targetUiId, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle)
{
	RegisteredTooltip* existingTooltip = TryFindRegisteredTooltip(registry, existingTooltipId);
	if (existingTooltip != nullptr)
	{
		existingTooltip->targetUiId.id = targetUiId.id;
		existingTooltip->targetUiId.index = targetUiId.index;
		if (!StrExactEquals(existingTooltip->targetUiId.str, targetUiId.str))
		{
			if (!IsEmptyStr(existingTooltip->targetUiId.str)) { FreeStr8(registry->arena, &existingTooltip->targetUiId.str); }
			existingTooltip->targetUiId.str = IsEmptyStr(targetUiId.str) ? targetUiId.str : AllocStr8(registry->arena, targetUiId.str);
		}
		if (!StrExactEquals(existingTooltip->displayStr, displayStr))
		{
			FreeStr8(registry->arena, &existingTooltip->displayStr);
			existingTooltip->displayStr = AllocStr8(registry->arena, displayStr);
		}
		existingTooltip->targetRec = targetRec;
		existingTooltip->font = font;
		existingTooltip->fontSize = fontSize;
		existingTooltip->fontStyle = fontStyle;
		
		existingTooltip->registeredThisFrame = true;
		return existingTooltip;
	}
	else
	{
		return RegisterTooltipGetPntr(registry, true, targetUiId, targetRec, displayStr, font, fontSize, fontStyle);
	}
}
#endif //BUILD_WITH_CLAY/BUILD_WITH_PIG_UI

#if BUILD_WITH_CLAY
PEXPI u64 SoftRegisterTooltip(TooltipRegistry* registry, u64 existingTooltipId, Str8 targetUiIdStr, uxx targetUiIdIndex, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle)
#elif BUILD_WITH_PIG_UI
PEXPI u64 SoftRegisterTooltip(TooltipRegistry* registry, u64 existingTooltipId, UiId targetUiId, rec targetRec, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle)
#endif
{
	#if BUILD_WITH_CLAY
	RegisteredTooltip* tooltip = SoftRegisterTooltipGetPntr(registry, existingTooltipId, targetUiIdStr, targetUiIdIndex, targetRec, displayStr, font, fontSize, fontStyle);
	#elif BUILD_WITH_PIG_UI
	RegisteredTooltip* tooltip = SoftRegisterTooltipGetPntr(registry, existingTooltipId, targetUiId, targetRec, displayStr, font, fontSize, fontStyle);
	#endif
	return (tooltip != nullptr) ? tooltip->id : TOOLTIP_ID_INVALID;
}

#if BUILD_WITH_CLAY
//NOTE: This function pokes into clayId.stringId so that must be filled properly!
PEXPI RegisteredTooltip* SoftRegisterTooltipByClayIdGetPntr(TooltipRegistry* registry, ClayId clayId, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle)
{
	RegisteredTooltip* existingTooltip = TryFindRegisteredTooltipByClayId(registry, clayId);
	u64 existingTooltipId = (existingTooltip != nullptr) ? existingTooltip->id : TOOLTIP_ID_INVALID;
	return SoftRegisterTooltipGetPntr(registry, existingTooltipId, clayId.stringId, clayId.offset, Rec_Zero, displayStr, font, fontSize, fontStyle);
}
PEXPI u64 SoftRegisterTooltipByClayId(TooltipRegistry* registry, ClayId clayId, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle)
{
	RegisteredTooltip* tooltip = SoftRegisterTooltipByClayIdGetPntr(registry, clayId, displayStr, font, fontSize, fontStyle);
	return (tooltip != nullptr) ? tooltip->id : TOOLTIP_ID_INVALID;
}
#elif BUILD_WITH_PIG_UI
PEXPI RegisteredTooltip* SoftRegisterTooltipByUiIdGetPntr(TooltipRegistry* registry, UiId uiId, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle)
{
	RegisteredTooltip* existingTooltip = TryFindRegisteredTooltipByUiId(registry, uiId);
	u64 existingTooltipId = (existingTooltip != nullptr) ? existingTooltip->id : TOOLTIP_ID_INVALID;
	return SoftRegisterTooltipGetPntr(registry, existingTooltipId, uiId, Rec_Zero, displayStr, font, fontSize, fontStyle);
}
PEXPI u64 SoftRegisterTooltipByUiId(TooltipRegistry* registry, UiId uiId, Str8 displayStr, PigFont* font, r32 fontSize, u8 fontStyle)
{
	RegisteredTooltip* tooltip = SoftRegisterTooltipByUiIdGetPntr(registry, uiId, displayStr, font, fontSize, fontStyle);
	return (tooltip != nullptr) ? tooltip->id : TOOLTIP_ID_INVALID;
}
#endif //BUILD_WITH_CLAY/BUILD_WITH_PIG_UI

#endif //PIG_CORE_IMPLEMENTATION

#endif //(BUILD_WITH_CLAY || BUILD_WITH_PIG_UI) && BUILD_WITH_SOKOL_GFX

#endif //  _UI_TOOLTIP_REGISTRY_H
