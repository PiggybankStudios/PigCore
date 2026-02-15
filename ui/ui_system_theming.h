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
	PIG_CORE_INLINE void PopUiThemer(UiThemerRegistry* registry, uxx themerId);
	PIG_CORE_INLINE uxx PushUiThemer(UiThemerRegistry* registry, UiThemerCallback_f* callback, void* userPntr);
	PIG_CORE_INLINE uxx PushBasicUiThemerFields(UiThemerRegistry* registry, u64 fields, UiElemConfig config);
	PIG_CORE_INLINE uxx PushBasicUiThemerConfig(UiThemerRegistry* registry, UiElemConfig config);
	PIG_CORE_INLINE void SetUiThemerActive(UiThemerRegistry* registry, uxx themerId, bool active);
	PIG_CORE_INLINE void EnableUiThemer(UiThemerRegistry* registry, uxx themerId);
	PIG_CORE_INLINE void DisableUiThemer(UiThemerRegistry* registry, uxx themerId);
	//UiThemerRegistryStartFrame pre-declared in ui_system_core.h
	//RunUiThemerCallbacks pre-declared in ui_system_core.h
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
// This macro is very powerful, it allows you to set a specific UiElemConfig field for all elements in the tree while the themer is active by doing something like:
//     uxx redSetterId = PushUiFields(UICONFIG({ .color = MonokaiRed }));
//     ...
//     PopUiFields(redSetterId);
#define PushUiFields(...) PushBasicUiThemerConfig(((UiCtx != nullptr) ? &UiCtx->themers : nullptr), NEW_STRUCT(UiElemConfigWrapper){ __VA_ARGS__ }.config)
#define PopUiFields(themerId) PopUiThemer(((UiCtx != nullptr) ? &UiCtx->themers : nullptr), (themerId))

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

// +==============================+
// |    BasicUiThemerCallback     |
// +==============================+
// bool UI_THEMER_CALLBACK_DEF(plex UiContext* context, UiElement* element, void* userPntr)
static UI_THEMER_CALLBACK_DEF(BasicUiThemerCallback)
{
	UNUSED(context);
	NotNull(userPntr);
	BasicUiThemerOptions* options = (BasicUiThemerOptions*)userPntr;
	SetUiElemConfigFieldsIfDefault(&element->config, &options->config, options->fields);
	return true;
}

PEXPI void FreeUiThemerRegistry(UiThemerRegistry* registry)
{
	NotNull(registry);
	if (registry->arena != nullptr)
	{
		FreeVarArray(&registry->themers);
		FreeVarArray(&registry->basicOptions);
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
	InitVarArray(BasicUiThemerOptions, &registryOut->basicOptions, arena);
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

PEXPI void PopUiThemer(UiThemerRegistry* registry, uxx themerId)
{
	NotNull(registry);
	NotNull(registry->arena);
	Assert(themerId != UiThemerId_Invalid);
	VarArrayLoop(&registry->themers, tIndex)
	{
		VarArrayLoopGet(UiThemer, themer, &registry->themers, tIndex);
		if (themer->id == themerId)
		{
			VarArrayRemoveAt(UiThemer, &registry->themers, tIndex);
			return;
		}
	}
	AssertMsg(false, "Couldn't find themer by ID to pop!");
}

PEXPI uxx PushUiThemer(UiThemerRegistry* registry, UiThemerCallback_f* callback, void* userPntr)
{
	NotNull(registry);
	NotNull(registry->arena);
	UiThemer* newThemer = VarArrayAdd(UiThemer, &registry->themers);
	NotNull(newThemer);
	ClearPointer(newThemer);
	newThemer->id = registry->nextThemerId;
	registry->nextThemerId++;
	newThemer->callback = callback;
	newThemer->userPntr = userPntr;
	newThemer->isActive = true;
	return newThemer->id;
}

PEXPI uxx PushBasicUiThemerFields(UiThemerRegistry* registry, u64 fields, UiElemConfig config)
{
	NotNull(registry);
	NotNull(registry->arena);
	BasicUiThemerOptions* options = VarArrayAdd(BasicUiThemerOptions, &registry->basicOptions);
	NotNull(options);
	ClearPointer(options);
	options->fields = fields;
	MyMemCopy(&options->config, &config, sizeof(UiElemConfig));
	return PushUiThemer(registry, BasicUiThemerCallback, options);
}
PEXPI uxx PushBasicUiThemerConfig(UiThemerRegistry* registry, UiElemConfig config)
{
	u64 fields = UiElemConfigField_None;
	for (uxx bIndex = 0; bIndex < UiElemConfigField_Count; bIndex++)
	{
		UiElemConfigField fieldBit = (UiElemConfigField)(1ull << bIndex);
		if (!IsUiElemConfigFieldDefault(&config, fieldBit)) { FlagSet(fields, fieldBit); }
	}
	return PushBasicUiThemerFields(registry, fields, config);
}

PEXPI void SetUiThemerActive(UiThemerRegistry* registry, uxx themerId, bool active)
{
	UiThemer* themer = TryGetUiThemerById(registry, themerId);
	NotNull(themer);
	themer->isActive = active;
}
PEXPI void EnableUiThemer(UiThemerRegistry* registry, uxx themerId) { SetUiThemerActive(registry, themerId, true); }
PEXPI void DisableUiThemer(UiThemerRegistry* registry, uxx themerId) { SetUiThemerActive(registry, themerId, false); }

PEXP void UiThemerRegistryStartFrame(UiThemerRegistry* registry)
{
	NotNull(registry);
	NotNull(registry->arena);
	VarArrayClear(&registry->themers);
	VarArrayClear(&registry->basicOptions);
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
			bool callbackResult = themer->callback(context, element, themer->userPntr);
			if (!callbackResult) { allThemersAcceptedElement = false; break; }
		}
	}
	return allThemersAcceptedElement;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_PIG_UI

#endif //  _UI_SYSTEM_THEMING_H
