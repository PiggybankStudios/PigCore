/*
File:   ui_system_theming.h
Author: Taylor Robbins
Date:   02\11\2026
Description:
	** This file contains types and functions that help us apply consistent theming
	** across all UI elements that are being created\drawn through the UI System.
	** One of the downsides of doing the UIELEM macro with designated initializer syntax
	** is it encourages the entire definition for a component to be in one spot
	** This often leads to repeating ourselves (referencing the same global padding value
	** inside every initializer, setting the default background color for every element, etc.)
	** The "theming" system allows us to register things that apply changes to the UiElemConfig
	** after it has been passed to OpenUiElement (through UIELEM macro). This allows us to
	** find a balance between brevity when declaring each element and specificity when customizing
	** all elements in the app, or all elements in a particular panel\area\etc.
*/

#ifndef _UI_SYSTEM_THEMING_H
#define _UI_SYSTEM_THEMING_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "mem/mem_arena.h"
#include "struct/struct_var_array.h"
#include "ui/ui_system_core.h"
#include "ui/ui_system.h"

#if BUILD_WITH_PIG_UI

#define UiThemerId_Invalid   0

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	//FreeUiThemerRegistry pre-declared in ui_system_core.h
	//InitUiThemerRegistry pre-declared in ui_system_core.h
	PIG_CORE_INLINE UiThemer* TryGetUiThemerById(UiThemerRegistry* registry, uxx themerId);
	PIG_CORE_INLINE uxx RegisterUiThemer(UiThemerRegistry* registry, UiThemerCallback_f* callback, void* contextPntr);
	PIG_CORE_INLINE void SetUiThemerActive(UiThemerRegistry* registry, uxx themerId, bool active);
	//UiThemerRegistryStartFrame pre-declared in ui_system_core.h
	//RunUiThemerCallbacks pre-declared in ui_system_core.h
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void FreeUiThemerRegistry(UiThemerRegistry* registry)
{
	NotNull(registry);
	if (registry->arena != nullptr)
	{
		FreeVarArray(&registry->themers);
	}
	ClearPointer(registry);
}

PEXP void InitUiThemerRegistry(Arena* arena, UiThemerRegistry* registryOut)
{
	NotNull(arena);
	NotNull(registryOut);
	ClearPointer(registryOut);
	registryOut->arena = arena;
	registryOut->nextThemerId = 1;
	InitVarArray(UiThemer, &registryOut->themers, arena);
}

PEXPI UiThemer* TryGetUiThemerById(UiThemerRegistry* registry, uxx themerId)
{
	NotNull(registry);
	NotNull(registry->arena);
	if (themerId == UiThemerId_Invalid) { return nullptr; }
	VarArrayLoop(&registry->themers, tIndex)
	{
		VarArrayLoopGet(UiThemer, themer, &registry->themers, tIndex);
		if (themer->id == themerId) { return themer; }
	}
	return nullptr;
}

PEXPI uxx RegisterUiThemer(UiThemerRegistry* registry, UiThemerCallback_f* callback, void* contextPntr)
{
	NotNull(registry);
	NotNull(registry->arena);
	UiThemer* newThemer = VarArrayAdd(UiThemer, &registry->themers);
	NotNull(newThemer);
	ClearPointer(newThemer);
	newThemer->id = registry->nextThemerId;
	registry->nextThemerId++;
	newThemer->callback = callback;
	newThemer->contextPntr = contextPntr;
	newThemer->isActive = true;
	return newThemer->id;
}

PEXPI void SetUiThemerActive(UiThemerRegistry* registry, uxx themerId, bool active)
{
	UiThemer* themer = TryGetUiThemerById(registry, themerId);
	NotNull(themer);
	themer->isActive = active;
}

PEXP void UiThemerRegistryStartFrame(UiThemerRegistry* registry)
{
	NotNull(registry);
	NotNull(registry->arena);
	VarArrayClear(&registry->themers);
	registry->nextThemerId = 1;
}

PEXP bool RunUiThemerCallbacks(UiThemerRegistry* registry, UiContext* context, UiElement* element)
{
	NotNull(registry);
	NotNull(registry->arena);
	bool allThemersAcceptedElement = true;
	VarArrayLoop(&registry->themers, tIndex)
	{
		VarArrayLoopGet(UiThemer, themer, &registry->themers, tIndex);
		if (themer->isActive)
		{
			bool callbackResult = themer->callback(context, element, themer->contextPntr);
			if (!callbackResult) { allThemersAcceptedElement = false; break; }
		}
	}
	return allThemersAcceptedElement;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_PIG_UI

#endif //  _UI_SYSTEM_THEMING_H
