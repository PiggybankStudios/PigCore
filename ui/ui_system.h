/*
File:   ui_layout.h
Author: Taylor Robbins
Date:   02\06\2026
Description:
	** This file serves as the backbone of our Immediate Mode style UI system.
	** This file deals primarily with UI Element registration through the UIELEM macro
	** as well as defining the UiContext where this element tree lives but
	** also houses many other globals that the UI system uses (like keyboard and mouse input).
	** Ultimately this system produces a UiRenderList at the end of each frame
	** the rendering of this list is done in ui_renderer.h.
	** This system is called the "Pig UI" system when differentiating it from things like Clay or Dear ImGui
	** but inside this repository we mostly refer to it as just the "UI System".
	** You must enable BUILD_WITH_PIG_UI for this code to be enabled. This is a temporary measure while we have
	** both Clay and Dear ImGui in the repository. Eventually if we remove one or both of these external
	** libraries, we'll probably make this UI system enabled by default.
*/

#ifndef _UI_SYSTEM_H
#define _UI_SYSTEM_H

#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "base/base_debug_output.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "struct/struct_rectangles.h"
#include "struct/struct_vectors.h"
#include "struct/struct_string.h"
#include "struct/struct_var_array.h"
#include "os/os_threading.h"
#include "input/input_keyboard.h"
#include "input/input_mouse.h"
#include "input/input_touch.h"
#include "gfx/gfx_texture.h"
#include "gfx/gfx_font.h"
#include "misc/misc_hash.h"
#include "misc/misc_sorting.h"
#include "ui/ui_system_core.h"

#if BUILD_WITH_PIG_UI

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
//TODO: Change __declspec stuff to work when compiling with Clang!
//TODO: Should this be thread local?
#if TARGET_IS_WINDOWS
	#if !PIG_CORE_IMPLEMENTATION
	extern __declspec(dllimport) UiContext* UiCtx;
	#else
	__declspec(dllexport) UiContext* UiCtx = nullptr;
	#endif
#else
	#if !PIG_CORE_IMPLEMENTATION
	extern UiContext* UiCtx;
	#else
	UiContext* UiCtx = nullptr;
	#endif
#endif

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	//CalcUiId pre-declared in ui_system_core.h
	//PrintUiId pre-declared in ui_system_core.h
	bool IsUiElemConfigFieldDefault(const UiElemConfig* configPntr, UiElemConfigField field);
	void SetUiElemConfigField(UiElemConfig* configToPntr, const UiElemConfig* configFromPntr, UiElemConfigField field);
	PIG_CORE_INLINE void SetUiElemConfigFields(UiElemConfig* configToPntr, const UiElemConfig* configFromPntr, u64 fieldBits);
	PIG_CORE_INLINE void SetUiElemConfigFieldsIfDefault(UiElemConfig* configToPntr, const UiElemConfig* configFromPntr, u64 fieldBits);
	void InitUiContext(Arena* arena, UiContext* contextOut);
	PIG_CORE_INLINE UiElement* GetCurrentUiElement();
	PIG_CORE_INLINE UiElement* GetUiElementParent(UiElement* element, uxx ancestorIndex);
	PIG_CORE_INLINE UiElement* GetUiElementChild(UiElement* element, uxx childIndex);
	PIG_CORE_INLINE UiElement* GetUiElementByIdInArray(VarArray* arrayPntr, UiId id, bool matchLocalIds);
	PIG_CORE_INLINE UiElement* GetUiElementByIdInPrevFrame(UiId id, bool matchLocalIds);
	PIG_CORE_INLINE UiElement* GetUiElementById(UiId id, bool matchLocalIds);
	PIG_CORE_INLINE UiElement* GetUiElementAttachParent(UiElement* element);
	PIG_CORE_INLINE bool IsUiElementHoveredStrict(UiId id);
	PIG_CORE_INLINE bool IsUiElementHovered(UiId id);
	PIG_CORE_INLINE bool IsCurrentUiElementHoveredStrict();
	PIG_CORE_INLINE bool IsCurrentUiElementHovered();
	PIG_CORE_INLINE bool DidMouseStartClickInsideUiElement(UiId id, MouseBtn mouseBtn);
	PIG_CORE_INLINE bool IsUiElementBeingClicked(UiId id, MouseBtn mouseBtn);
	PIG_CORE_INLINE bool WasUiElementClicked(UiId id, MouseBtn mouseBtn);
	PIG_CORE_INLINE bool DidUiElementClickStart(UiId id, MouseBtn mouseBtn);
	PIG_CORE_INLINE bool IsCurrentUiElementBeingClicked(MouseBtn mouseBtn);
	PIG_CORE_INLINE bool WasCurrentUiElementClicked(MouseBtn mouseBtn);
	PIG_CORE_INLINE bool DidCurrentUiElementClickStart(MouseBtn mouseBtn);
	PIG_CORE_INLINE UiElement* CloseUiElement();
	PIG_CORE_INLINE UiElement* OpenUiElement(UiElemConfig config);
	PIG_CORE_INLINE bool OpenUiElementConditional(UiElemConfig config);
	void StartUiFrame(UiContext* context, v2 screenSize, r32 scale, u64 programTime, KeyboardState* keyboard, MouseState* mouse, TouchscreenState* touchscreen);
	PIG_CORE_INLINE Str8 GetUiElementQualifiedName(Arena* arena, UiElement* element);
	UiRenderList* GetUiRenderList();
	void EndUiFrame();
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
//NOTE: In order to get the UIELEM macro to work properly, we need to surround the __VA_ARGS__ in an extra set of curly brackets
//      We can make this valid by defining a Wrapper struct that the curly brackets construct and then just access the config element inside
typedef plex UiElemConfigWrapper UiElemConfigWrapper;
plex UiElemConfigWrapper { UiElemConfig config; };

//TODO: Write a description of these macros and their need
//NOTE: The preprocessor is going to treat the commas inside the designated initializer list as argument separators.
//      For this reasons we do a variadic argument macro and use __VA_ARGS__ as a way to pass all of the designated initializer through
#define UICONFIG(...) (NEW_STRUCT(UiElemConfigWrapper){ __VA_ARGS__ }.config)
#define UIELEM(...) DeferIfBlockCondEnd(OpenUiElementConditional(NEW_STRUCT(UiElemConfigWrapper){ __VA_ARGS__ }.config), CloseUiElement())
// A "leaf" element has no child elements, and therefore doesn't need a scope following the macro
#define UIELEM_LEAF(...) do { if (OpenUiElementConditional(NEW_STRUCT(UiElemConfigWrapper){ __VA_ARGS__ }.config)) { CloseUiElement(); } } while(0)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

#if (TARGET_HAS_THREADING && DEBUG_BUILD)
#define AssertUiThreadIsSame() AssertMsg(UiCtx == nullptr || UiCtx->threadId == OsGetCurrentThreadId(), "Calling a UI function from a different thread than the UI frame was started on!")
#else
#define AssertUiThreadIsSame() //nothing
#endif

PEXPI UiId CalcUiId(UiId baseId, Str8 idString, uxx index)
{
	UiId result = ZEROED;
	#if TARGET_IS_64BIT
	result.id = FnvHashU64Ex(idString.chars, idString.length, baseId.id);
	if (index != PIG_UI_ID_INDEX_NONE) { result.id = FnvHashU64Ex(&index, sizeof(index), result.id); }
	#else //TARGET_IS_32BIT
	result.id = FnvHashU32Ex(idString.chars, idString.length, baseId.id);
	if (index != PIG_UI_ID_INDEX_NONE) { result.id = FnvHashU32Ex(&index, sizeof(index), result.id); }
	#endif //TARGET_IS_64BIT
	result.str = idString;
	// Whenever we make IDs DURING the UI frame, we allocate the string source from the frameArena to ensure it lives through the UI rendering phase (where we may have debug display of ID strings, or we may print out IDs to stdout)
	if (UiCtx != nullptr && UiCtx->frameArena != nullptr && !IsEmptyStr(result.str) && !IsPntrFromArena(UiCtx->frameArena, result.str.chars))
	{
		result.str = AllocStr8(UiCtx->frameArena, result.str);
	}
	result.index = index;
	return result;
}
PEXPI UiId PrintUiId(UiId baseId, uxx index, const char* formatString, ...)
{
	uxx scratchMark = 0;
	Arena* arena = (UiCtx != nullptr && UiCtx->frameArena != nullptr) ? UiCtx->frameArena : GetScratch(&scratchMark);
	PrintInArenaVa(arena, idStr, idStrLength, formatString);
	AssertMsg(idStrLength >= 0, "Format failed for UI Id being created through PrintUiId (or UiIdPrint/UiIdPrintIndex)");
	UiId result = CalcUiId(baseId, MakeStr8Nt(idStr), index);
	if (UiCtx == nullptr || UiCtx->frameArena == nullptr) { ArenaResetToMark(arena, scratchMark); }
	return result;
}

PEXP bool IsUiElemConfigFieldDefault(const UiElemConfig* configPntr, UiElemConfigField field)
{
	switch (field)
	{
		case UiElemConfigField_Id: return (configPntr->id.id == 0);
		case UiElemConfigField_GlobalId:              return (configPntr->globalId == false);
		case UiElemConfigField_Direction:             return (configPntr->direction == UiLayoutDir_Default);
		case UiElemConfigField_SizingTypeX:           return (configPntr->sizing.x.type == UiSizingType_Default);
		case UiElemConfigField_SizingValueX:          return (configPntr->sizing.x.value == 0.0f);
		case UiElemConfigField_SizingTypeY:           return (configPntr->sizing.y.type == UiSizingType_Default);
		case UiElemConfigField_SizingValueY:          return (configPntr->sizing.y.value == 0.0f);
		case UiElemConfigField_Depth:                 return (configPntr->depth == UI_DEPTH_DEFAULT);
		case UiElemConfigField_Color:                 return (configPntr->color.valueU32 == PigUiDefaultColor_Value);
		case UiElemConfigField_Texture:               return (configPntr->texture == nullptr);
		case UiElemConfigField_DontSizeToTexture:     return (configPntr->dontSizeToTexture == false);
		case UiElemConfigField_InnerPaddingLeft:      return (configPntr->padding.inner.Left   == 0.0f);
		case UiElemConfigField_InnerPaddingTop:       return (configPntr->padding.inner.Top    == 0.0f);
		case UiElemConfigField_InnerPaddingRight:     return (configPntr->padding.inner.Right  == 0.0f);
		case UiElemConfigField_InnerPaddingBottom:    return (configPntr->padding.inner.Bottom == 0.0f);
		case UiElemConfigField_OuterPaddingLeft:      return (configPntr->padding.outer.Left   == 0.0f);
		case UiElemConfigField_OuterPaddingTop:       return (configPntr->padding.outer.Top    == 0.0f);
		case UiElemConfigField_OuterPaddingRight:     return (configPntr->padding.outer.Right  == 0.0f);
		case UiElemConfigField_OuterPaddingBottom:    return (configPntr->padding.outer.Bottom == 0.0f);
		case UiElemConfigField_ChildPadding:          return (configPntr->padding.child == 0.0f);
		case UiElemConfigField_BorderThicknessLeft:   return (configPntr->borderThickness.Left   == 0.0f);
		case UiElemConfigField_BorderThicknessTop:    return (configPntr->borderThickness.Top    == 0.0f);
		case UiElemConfigField_BorderThicknessRight:  return (configPntr->borderThickness.Right  == 0.0f);
		case UiElemConfigField_BorderThicknessBottom: return (configPntr->borderThickness.Bottom == 0.0f);
		case UiElemConfigField_BorderColor:           return (configPntr->borderColor.valueU32 == PigUiDefaultColor_Value);
		case UiElemConfigField_BorderDepth:           return (configPntr->borderDepth == 0.0f);
		case UiElemConfigField_FloatingType:          return (configPntr->floating.type == UiFloatingType_Default);
		case UiElemConfigField_FloatingOffsetX:       return (configPntr->floating.offset.X == 0.0f);
		case UiElemConfigField_FloatingOffsetY:       return (configPntr->floating.offset.Y == 0.0f);
		case UiElemConfigField_FloatingAttachId:      return (configPntr->floating.attachId.id == 0);
		case UiElemConfigField_FloatingParentSide:    return (configPntr->floating.parentSide == Dir2Ex_None);
		case UiElemConfigField_FloatingElemSide:      return (configPntr->floating.elemSide == Dir2Ex_None);
		case UiElemConfigField_Condition:             return (configPntr->condition == UiConditionType_None);
		case UiElemConfigField_MousePassthrough:      return (configPntr->mousePassthrough == false);
		case UiElemConfigField_StrictHover:           return (configPntr->strictHover == false);
		case UiElemConfigField_RendererParams:        return true; //TODO: What do we want to do for these?
		case UiElemConfigField_ThemerParams:          return true; //TODO: What do we want to do for these?
		default: DebugAssert(false); return true;
	}
}
PEXP void SetUiElemConfigField(UiElemConfig* configToPntr, const UiElemConfig* configFromPntr, UiElemConfigField field)
{
	switch (field)
	{
		case UiElemConfigField_Id:                    configToPntr->id = configFromPntr->id; break;
		case UiElemConfigField_GlobalId:              configToPntr->globalId = configFromPntr->globalId; break;
		case UiElemConfigField_Direction:             configToPntr->direction = configFromPntr->direction; break;
		case UiElemConfigField_SizingTypeX:           configToPntr->sizing.x.type = configFromPntr->sizing.x.type; break;
		case UiElemConfigField_SizingValueX:          configToPntr->sizing.x.value = configFromPntr->sizing.x.value; break;
		case UiElemConfigField_SizingTypeY:           configToPntr->sizing.y.type = configFromPntr->sizing.y.type; break;
		case UiElemConfigField_SizingValueY:          configToPntr->sizing.y.value = configFromPntr->sizing.y.value; break;
		case UiElemConfigField_Depth:                 configToPntr->depth = configFromPntr->depth; break;
		case UiElemConfigField_Color:                 configToPntr->color = configFromPntr->color; break;
		case UiElemConfigField_Texture:               configToPntr->texture = configFromPntr->texture; break;
		case UiElemConfigField_DontSizeToTexture:     configToPntr->dontSizeToTexture = configFromPntr->dontSizeToTexture; break;
		case UiElemConfigField_InnerPaddingLeft:      configToPntr->padding.inner.Left   = configFromPntr->padding.inner.Left; break;
		case UiElemConfigField_InnerPaddingTop:       configToPntr->padding.inner.Top    = configFromPntr->padding.inner.Top; break;
		case UiElemConfigField_InnerPaddingRight:     configToPntr->padding.inner.Right  = configFromPntr->padding.inner.Right; break;
		case UiElemConfigField_InnerPaddingBottom:    configToPntr->padding.inner.Bottom = configFromPntr->padding.inner.Bottom; break;
		case UiElemConfigField_OuterPaddingLeft:      configToPntr->padding.outer.Left   = configFromPntr->padding.outer.Left; break;
		case UiElemConfigField_OuterPaddingTop:       configToPntr->padding.outer.Top    = configFromPntr->padding.outer.Top; break;
		case UiElemConfigField_OuterPaddingRight:     configToPntr->padding.outer.Right  = configFromPntr->padding.outer.Right; break;
		case UiElemConfigField_OuterPaddingBottom:    configToPntr->padding.outer.Bottom = configFromPntr->padding.outer.Bottom; break;
		case UiElemConfigField_ChildPadding:          configToPntr->padding.child = configFromPntr->padding.child; break;
		case UiElemConfigField_BorderThicknessLeft:   configToPntr->borderThickness.Left   = configFromPntr->borderThickness.Left; break;
		case UiElemConfigField_BorderThicknessTop:    configToPntr->borderThickness.Top    = configFromPntr->borderThickness.Top; break;
		case UiElemConfigField_BorderThicknessRight:  configToPntr->borderThickness.Right  = configFromPntr->borderThickness.Right; break;
		case UiElemConfigField_BorderThicknessBottom: configToPntr->borderThickness.Bottom = configFromPntr->borderThickness.Bottom; break;
		case UiElemConfigField_BorderColor:           configToPntr->borderColor = configFromPntr->borderColor; break;
		case UiElemConfigField_BorderDepth:           configToPntr->borderDepth = configFromPntr->borderDepth; break;
		case UiElemConfigField_FloatingType:          configToPntr->floating.type = configFromPntr->floating.type; break;
		case UiElemConfigField_FloatingOffsetX:       configToPntr->floating.offset.X = configFromPntr->floating.offset.X; break;
		case UiElemConfigField_FloatingOffsetY:       configToPntr->floating.offset.Y = configFromPntr->floating.offset.Y; break;
		case UiElemConfigField_FloatingAttachId:      configToPntr->floating.attachId = configFromPntr->floating.attachId; break;
		case UiElemConfigField_FloatingParentSide:    configToPntr->floating.parentSide = configFromPntr->floating.parentSide; break;
		case UiElemConfigField_FloatingElemSide:      configToPntr->floating.elemSide = configFromPntr->floating.elemSide; break;
		case UiElemConfigField_Condition:             configToPntr->condition = configFromPntr->condition; break;
		case UiElemConfigField_MousePassthrough:      configToPntr->mousePassthrough = configFromPntr->mousePassthrough; break;
		case UiElemConfigField_StrictHover:           configToPntr->strictHover = configFromPntr->strictHover; break;
		case UiElemConfigField_RendererParams:        MyMemCopy(&configToPntr->renderer, &configFromPntr->renderer, sizeof(UiRendererParameters)); break;
		case UiElemConfigField_ThemerParams:          MyMemCopy(&configToPntr->themer, &configFromPntr->themer, sizeof(UiThemerParameters)); break;
		default: DebugAssert(false); break;
	}
}
PEXPI void SetUiElemConfigFields(UiElemConfig* configToPntr, const UiElemConfig* configFromPntr, u64 fieldBits)
{
	for (uxx fIndex = 0; fIndex < UiElemConfigField_Count; fIndex++)
	{
		UiElemConfigField fieldBit = (UiElemConfigField)(1ull << fIndex);
		if (IsFlagSet(fieldBits, fieldBit))
		{
			SetUiElemConfigField(configToPntr, configFromPntr, fieldBit);
		}
	}
}
PEXPI void SetUiElemConfigFieldsIfDefault(UiElemConfig* configToPntr, const UiElemConfig* configFromPntr, u64 fieldBits)
{
	for (uxx fIndex = 0; fIndex < UiElemConfigField_Count; fIndex++)
	{
		UiElemConfigField fieldBit = (UiElemConfigField)(1ull << fIndex);
		if (IsFlagSet(fieldBits, fieldBit))
		{
			if (IsUiElemConfigFieldDefault(configToPntr, fieldBit)) { SetUiElemConfigField(configToPntr, configFromPntr, fieldBit); }
		}
	}
}

//TODO: Add FreeUiContext? We don't usually need it

PEXP void InitUiContext(Arena* arena, UiContext* contextOut)
{
	NotNull(arena);
	NotNull(contextOut);
	ClearPointer(contextOut);
	contextOut->arena = arena;
	InitVarArray(UiElement, &contextOut->elements, arena);
	InitVarArray(UiElement, &contextOut->prevElements, arena);
	InitUiThemerRegistry(arena, &contextOut->themers);
}

PEXPI UiElement* GetCurrentUiElement()
{
	DebugAssert(UiCtx != nullptr);
	DebugAssert(UiCtx->currentElementIndex < UiCtx->elements.length);
	return VarArrayGetHard(UiElement, &UiCtx->elements, UiCtx->currentElementIndex);
}

PEXPI UiElement* GetUiElementParent(UiElement* element, uxx ancestorIndex)
{
	DebugNotNull(element);
	UiElement* result = element;
	for (uxx aIndex = 0; aIndex <= ancestorIndex; aIndex++)
	{
		if (result->parentIndex == PIG_UI_INDEX_INVALID) { return nullptr; }
		DebugAssert(result->parentIndex < result->elementIndex);
		result -= (result->elementIndex - result->parentIndex);
	}
	return result;
}
PEXPI UiElement* GetUiElementChild(UiElement* element, uxx childIndex)
{
	DebugNotNull(element);
	if (childIndex >= element->numChildren) { return nullptr; }
	UiElement* result = element + 1;
	for (uxx cIndex = 0; cIndex < childIndex; cIndex++)
	{
		result += 1 + result->numDescendents;
	}
	return result;
}

PEXPI UiElement* GetUiElementByIdInArray(VarArray* arrayPntr, UiId id, bool matchLocalIds)
{
	VarArrayLoop(arrayPntr, eIndex)
	{
		VarArrayLoopGet(UiElement, element, arrayPntr, eIndex);
		if (element->id.id == id.id) { return element; }
		else if (matchLocalIds && element->config.id.id == id.id) { return element; }
	}
	return nullptr;
}
PEXPI UiElement* GetUiElementByIdInPrevFrame(UiId id, bool matchLocalIds) { NotNull(UiCtx); return GetUiElementByIdInArray(&UiCtx->prevElements, id, matchLocalIds); }
PEXPI UiElement* GetUiElementById(UiId id, bool matchLocalIds) { NotNull(UiCtx); return GetUiElementByIdInArray(&UiCtx->elements, id, matchLocalIds); }

PEXPI UiElement* GetUiElementAttachParent(UiElement* element)
{
	if (element->config.floating.type == UiFloatingType_Parent) { return GetUiElementParent(element, 0); }
	else if (element->config.floating.type == UiFloatingType_Root) { return VarArrayGetFirst(UiElement, &UiCtx->elements); }
	else if (element->config.floating.type == UiFloatingType_Id)
	{
		UiElement* result = GetUiElementById(element->config.floating.attachId, true);
		AssertMsg(result != nullptr, "Floating element tried to attach to a UI Element that doesn't exist yet. Floating elements must be declared AFTER the element they are attaching to");
		return result;
	}
	else { DebugAssert(false); return nullptr; }
}

PEXPI bool IsUiElementHoveredStrict(UiId id) { NotNull(UiCtx); return (UiCtx->mouseHoveredId.id == id.id || UiCtx->mouseHoveredLocalId.id == id.id); }
PEXPI bool IsUiElementHovered(UiId id)
{
	NotNull(UiCtx);
	if (UiCtx->mouseHoveredId.id == id.id) { return true; }
	if (UiCtx->mouseHoveredLocalId.id == id.id) { return true; }
	if (UiCtx->mouseHoveredId.id != 0)
	{
		UiElement* hoveredElement = GetUiElementByIdInPrevFrame(UiCtx->mouseHoveredId, false);
		if (hoveredElement != nullptr && hoveredElement->config.floating.type == UiFloatingType_None)
		{
			UiElement* parent = GetUiElementParent(hoveredElement, 0);
			while (parent != nullptr)
			{
				if (parent->id.id == id.id || parent->config.id.id == id.id) { return true; }
				if (parent->config.floating.type != UiFloatingType_None) { break; }
				parent = GetUiElementParent(parent, 0);
			}
		}
		else { WriteLine_W("Couldn't find hovered element in prev frame hierarchy!"); }
	}
	return false;
}
PEXPI bool IsCurrentUiElementHoveredStrict()
{
	UiElement* currentUiElement = GetCurrentUiElement();
	NotNull(currentUiElement);
	return IsUiElementHoveredStrict(currentUiElement->id);
}
PEXPI bool IsCurrentUiElementHovered()
{
	UiElement* currentUiElement = GetCurrentUiElement();
	NotNull(currentUiElement);
	return IsUiElementHovered(currentUiElement->id);
}

PEXPI bool DidMouseStartClickInsideUiElement(UiId id, MouseBtn mouseBtn)
{
	NotNull(UiCtx);
	Assert(mouseBtn > MouseBtn_None && mouseBtn < MouseBtn_Count);
	if (UiCtx->mouse == nullptr) { return false; }
	if (UiCtx->clickStartHoveredId[mouseBtn].id == 0) { return false; }
	if (UiCtx->clickStartHoveredId[mouseBtn].id == id.id) { return true; }
	if (UiCtx->clickStartHoveredLocalId[mouseBtn].id == id.id) { return true; }
	UiElement* clickedElement = GetUiElementByIdInPrevFrame(UiCtx->clickStartHoveredId[mouseBtn], false);
	if (clickedElement != nullptr && clickedElement->config.floating.type == UiFloatingType_None)
	{
		UiElement* parent = GetUiElementParent(clickedElement, 0);
		while (parent != nullptr)
		{
			if (parent->id.id == id.id || parent->config.id.id == id.id) { return true; }
			if (parent->config.floating.type != UiFloatingType_None) { break; }
			parent = GetUiElementParent(parent, 0);
		}
	}
	return false;
}
PEXPI bool IsUiElementBeingClicked(UiId id, MouseBtn mouseBtn)
{
	NotNull(UiCtx);
	Assert(mouseBtn > MouseBtn_None && mouseBtn < MouseBtn_Count);
	if (UiCtx->mouse == nullptr) { return false; }
	if (!IsMouseBtnDown(UiCtx->mouse, nullptr, mouseBtn)) { return false; }
	if (!IsUiElementHovered(id)) { return false; }
	if (DidMouseStartClickInsideUiElement(id, mouseBtn)) { return true; }
	return false;
}
PEXPI bool WasUiElementClicked(UiId id, MouseBtn mouseBtn)
{
	NotNull(UiCtx);
	Assert(mouseBtn > MouseBtn_None && mouseBtn < MouseBtn_Count);
	if (UiCtx->mouse == nullptr) { return false; }
	if (!IsMouseBtnReleased(UiCtx->mouse, nullptr, mouseBtn)) { return false; }
	if (!IsUiElementHovered(id)) { return false; }
	if (DidMouseStartClickInsideUiElement(id, mouseBtn)) { return true; }
	return false;
}
PEXPI bool DidUiElementClickStart(UiId id, MouseBtn mouseBtn)
{
	NotNull(UiCtx);
	Assert(mouseBtn > MouseBtn_None && mouseBtn < MouseBtn_Count);
	if (UiCtx->mouse == nullptr) { return false; }
	if (!IsMouseBtnPressed(UiCtx->mouse, nullptr, mouseBtn)) { return false; }
	if (IsUiElementHovered(id)) { return true; }
	return false;
}
PEXPI bool IsCurrentUiElementBeingClicked(MouseBtn mouseBtn)
{
	UiElement* currentUiElement = GetCurrentUiElement();
	NotNull(currentUiElement);
	return IsUiElementBeingClicked(currentUiElement->id, mouseBtn);
}
PEXPI bool WasCurrentUiElementClicked(MouseBtn mouseBtn)
{
	UiElement* currentUiElement = GetCurrentUiElement();
	NotNull(currentUiElement);
	return WasUiElementClicked(currentUiElement->id, mouseBtn);
}
PEXPI bool DidCurrentUiElementClickStart(MouseBtn mouseBtn)
{
	UiElement* currentUiElement = GetCurrentUiElement();
	NotNull(currentUiElement);
	return DidUiElementClickStart(currentUiElement->id, mouseBtn);
}

PEXPI UiElement* CloseUiElement()
{
	NotNull(UiCtx);
	AssertUiThreadIsSame();
	DebugAssertMsg(UiCtx->currentElementIndex < UiCtx->elements.length, "Tried to close UI element when none was open! UI hierarchy is potentially invalid!");
	if (UiCtx->currentElementIndex >= UiCtx->elements.length) { return nullptr; }
	UiElement* element = VarArrayGetHard(UiElement, &UiCtx->elements, UiCtx->currentElementIndex);
	DebugAssert(element->isOpen);
	element->isOpen = false;
	UiCtx->currentElementIndex = element->parentIndex;
	
	//Multiply config sizing values by UiCtx->scale
	if (element->config.sizing.x.type == UiSizingType_FixedPx || element->config.sizing.x.type == UiSizingType_Expand)
	{
		element->config.sizing.x.value *= UiCtx->scale;
	}
	if (element->config.sizing.y.type == UiSizingType_FixedPx || element->config.sizing.y.type == UiSizingType_Expand)
	{
		element->config.sizing.y.value *= UiCtx->scale;
	}
	element->config.padding.inner.Left     *= UiCtx->scale;
	element->config.padding.inner.Top      *= UiCtx->scale;
	element->config.padding.inner.Right    *= UiCtx->scale;
	element->config.padding.inner.Bottom   *= UiCtx->scale;
	element->config.padding.outer.Left     *= UiCtx->scale;
	element->config.padding.outer.Top      *= UiCtx->scale;
	element->config.padding.outer.Right    *= UiCtx->scale;
	element->config.padding.outer.Bottom   *= UiCtx->scale;
	element->config.padding.child          *= UiCtx->scale;
	//NOTE: For border thicknesses we want to keep them from going below 1px if the thickness was non-zero at uiScale=1.0
	element->config.borderThickness.Left   *= UiCtx->scale; if (element->config.borderThickness.Left   > 0.0f && element->config.borderThickness.Left   < 1.0f) { element->config.borderThickness.Left   = 1.0f; }
	element->config.borderThickness.Top    *= UiCtx->scale; if (element->config.borderThickness.Top    > 0.0f && element->config.borderThickness.Top    < 1.0f) { element->config.borderThickness.Top    = 1.0f; }
	element->config.borderThickness.Right  *= UiCtx->scale; if (element->config.borderThickness.Right  > 0.0f && element->config.borderThickness.Right  < 1.0f) { element->config.borderThickness.Right  = 1.0f; }
	element->config.borderThickness.Bottom *= UiCtx->scale; if (element->config.borderThickness.Bottom > 0.0f && element->config.borderThickness.Bottom < 1.0f) { element->config.borderThickness.Bottom = 1.0f; }
	
	return element;
}

//NOTE: This pointer becomes potentially invalid once OpenUiElement is called again, VarArrayAdd semantics
PEXPI UiElement* OpenUiElement(UiElemConfig config)
{
	NotNull(UiCtx);
	// DebugAssert(UiCtx->currentElementIndex < UiCtx->elements.length);//TODO: Enable this once we figure out how to allow this ONLY when the root element is getting opened in StartUiFrame
	AssertUiThreadIsSame();
	
	//NOTE: Everything that happens in the first part of this function must be reversable if !allThemersAcceptedElement
	
	UiElement* newElement = VarArrayAdd(UiElement, &UiCtx->elements);
	NotNull(newElement);
	ClearPointer(newElement);
	newElement->elementIndex = (UiCtx->elements.length - 1);
	newElement->parentIndex = UiCtx->currentElementIndex;
	MyMemCopy(&newElement->config, &config, sizeof(UiElemConfig));
	UiElement* parentElement = GetUiElementParent(newElement, 0);
	if (parentElement != nullptr)
	{
		DebugAssert(parentElement->isOpen);
		if (IsEmptyStr(newElement->config.id.str)) { newElement->config.id = UiIdLitIndex(PIG_UI_DEFAULT_ID_STR, parentElement->numChildren); }
		if (newElement->config.globalId) { newElement->id = newElement->config.id; }
		else { newElement->id = CalcUiId(parentElement->id, newElement->config.id.str, newElement->config.id.index); }
		newElement->parentId = parentElement->config.id;
		newElement->treeDepth = (parentElement->treeDepth + 1);
		newElement->siblingIndex = parentElement->numChildren;
		parentElement->numChildren++;
		UiElement* ancestor = parentElement;
		while (ancestor != nullptr)
		{
			ancestor->numDescendents++;
			ancestor = VarArrayGetSoft(UiElement, &UiCtx->elements, ancestor->parentIndex);
		}
	}
	else
	{
		if (IsEmptyStr(newElement->config.id.str)) { newElement->config.id = UiIdLitIndex(PIG_UI_DEFAULT_ID_STR, UiCtx->numTopLevelElements); }
		newElement->id = newElement->config.id; //Top-level IDs are not modified by any base ID
		newElement->treeDepth = 0;
		newElement->siblingIndex = UiCtx->numTopLevelElements;
		UiCtx->numTopLevelElements++;
	}
	
	uxx oldCurrentElementIndex = UiCtx->currentElementIndex;
	UiCtx->currentElementIndex = newElement->elementIndex;
	newElement->isOpen = true;
	
	bool allThemersAcceptedElement = RunUiThemerCallbacks(&UiCtx->themers, UiCtx, newElement);
	if (!allThemersAcceptedElement)
	{
		if (parentElement != nullptr)
		{
			parentElement->numChildren--;
			UiElement* ancestor = parentElement;
			while (ancestor != nullptr)
			{
				ancestor->numDescendents--;
				ancestor = VarArrayGetSoft(UiElement, &UiCtx->elements, ancestor->parentIndex);
			}
		}
		else { UiCtx->numTopLevelElements--; }
		VarArrayRemoveLast(UiElement, &UiCtx->elements);
		UiCtx->currentElementIndex = oldCurrentElementIndex;
		return nullptr;
	}
	
	// If you don't specify depth then we inherit depth from the parent
	if (newElement->config.depth == UI_DEPTH_DEFAULT && parentElement != nullptr)
	{
		newElement->config.depth = parentElement->config.depth;
		if (newElement->config.floating.type != UiFloatingType_None) { newElement->config.depth += PIG_UI_DEFAULT_FLOATING_ELEM_DEPTH_OFFSET; }
	}
	// If you have default sizing parameters and you attached a texture then we copy in the texture dimensions as UiSizingType_Fit
	if (newElement->config.texture != nullptr && !newElement->config.repeatingTexture && !newElement->config.dontSizeToTexture &&
		newElement->config.sizing.x.type == UiSizingType_Default && newElement->config.sizing.y.type == UiSizingType_Default &&
		newElement->config.sizing.x.value == 0.0f && newElement->config.sizing.y.value == 0.0f)
	{
		newElement->config.sizing = NEW_STRUCT(UiSizing)UI_FIXED2(newElement->config.texture->Width, newElement->config.texture->Height);
	}
	
	if (newElement->config.floating.type == UiFloatingType_None && parentElement != nullptr)
	{
		parentElement->numNonFloatingChildren++;
	}
	
	if (newElement->config.floating.type != UiFloatingType_None)
	{
		UiElement* attachParent = GetUiElementAttachParent(newElement);
		AssertMsg(attachParent != nullptr, "Could not find floating element attach parent. NOTE: Floating elements must be defined AFTER the element they attach to!");
		newElement->floatDepth = attachParent->floatDepth + 1;
	}
	else
	{
		newElement->floatDepth = (parentElement != nullptr) ? parentElement->floatDepth : 0;
	}
	
	if (newElement->config.colorRecursive.valueU32 != PigUiDefaultColor_Value ||
		(parentElement != nullptr && parentElement->config.colorRecursive.valueU32 != PigUiDefaultColor_Value))
	{
		Color32 parentColorRecursive = (parentElement != nullptr) ? UiConfigColorToActualColor(parentElement->config.colorRecursive) : White;
		newElement->config.colorRecursive = ColorMultSimple(UiConfigColorToActualColor(newElement->config.colorRecursive), parentColorRecursive);
	}
	if (!IsEmptyRichStr(newElement->config.richText) && !IsPntrFromArena(UiCtx->frameArena, newElement->config.richText.fullPiece.str.chars))
	{
		newElement->config.richText = AllocRichStr(UiCtx->frameArena, newElement->config.richText);
	}
	if (!IsEmptyStr(newElement->config.text) && !IsPntrFromArena(UiCtx->frameArena, newElement->config.text.chars))
	{
		newElement->config.text = AllocStr8(UiCtx->frameArena, newElement->config.text);
	}
	
	newElement->runChildCode = true;
	if (newElement->config.condition != UiConditionType_None)
	{
		newElement->runChildCode = false;
		UiElement* prevFrameElement = GetUiElementByIdInPrevFrame(newElement->id, false);
		if (prevFrameElement != nullptr)
		{
			bool isHovered = (newElement->config.strictHover ? IsUiElementHoveredStrict(newElement->id) : IsUiElementHovered(newElement->id));
			
			switch (newElement->config.condition)
			{
				case UiConditionType_MouseHover:
				{
					newElement->runChildCode = isHovered;
				} break;
				case UiConditionType_MouseLeftClicked:
				{
					if (UiCtx->mouse != nullptr && isHovered)
					{
						newElement->runChildCode = IsMouseBtnReleased(UiCtx->mouse, nullptr, MouseBtn_Left);
					}
				} break;
				case UiConditionType_MouseLeftClickStart:
				{
					if (UiCtx->mouse != nullptr && isHovered)
					{
						newElement->runChildCode = IsMouseBtnPressed(UiCtx->mouse, nullptr, MouseBtn_Left);
					}
				} break;
				case UiConditionType_MouseRightClicked:
				{
					if (UiCtx->mouse != nullptr && isHovered)
					{
						newElement->runChildCode = IsMouseBtnReleased(UiCtx->mouse, nullptr, MouseBtn_Right);
					}
				} break;
				case UiConditionType_MouseRightClickStart:
				{
					if (UiCtx->mouse != nullptr && isHovered)
					{
						newElement->runChildCode = IsMouseBtnPressed(UiCtx->mouse, nullptr, MouseBtn_Right);
					}
				} break;
				case UiConditionType_MouseMiddleClicked:
				{
					if (UiCtx->mouse != nullptr && isHovered)
					{
						newElement->runChildCode = IsMouseBtnReleased(UiCtx->mouse, nullptr, MouseBtn_Middle);
					}
				} break;
				case UiConditionType_MouseMiddleClickStart:
				{
					if (UiCtx->mouse != nullptr && isHovered)
					{
						newElement->runChildCode = IsMouseBtnPressed(UiCtx->mouse, nullptr, MouseBtn_Middle);
					}
				} break;
				
				default: DebugAssertMsg(false, "Unimplemented condition!"); break;
			};
		}
	}
	
	if (!newElement->runChildCode) { CloseUiElement(); }
	return newElement;
}
PEXPI bool OpenUiElementConditional(UiElemConfig config)
{
	UiElement* elementPntr = OpenUiElement(config);
	return (elementPntr != nullptr && elementPntr->runChildCode && elementPntr->isOpen);
}

PEXP void StartUiFrame(UiContext* context, v2 screenSize, Color32 backgroundColor, r32 scale, u64 programTime, KeyboardState* keyboard, MouseState* mouse, TouchscreenState* touchscreen)
{
	DebugNotNull(context);
	DebugNotNull(context->arena);
	// NOTE: Normally scratch arenas are reset to their previous location when ScratchEnd is called.
	//       However, since we are sneakily using one of these arenas to guarantee allocations live
	//       until the end of the UI render we need to prevent this resetting behavior.
	//       In order to not cause too much memory bloat we use the last scratch arena, which often
	//       doesn't get used much anyways
	context->frameArena = &scratchArenasArray[NUM_SCRATCH_ARENAS_PER_THREAD-1];
	DebugAssert(context->frameArena->type == ArenaType_StackVirtual || context->frameArena->type == ArenaType_StackPaged);
	DebugAssert(context->arena != context->frameArena);
	FlagSet(context->frameArena->flags, ArenaFlag_DontPop);
	context->frameArenaMark = ArenaGetMark(context->frameArena);
	
	#if (TARGET_HAS_THREADING && DEBUG_BUILD)
	context->threadId = OsGetCurrentThreadId();
	#endif
	
	context->screenSize = screenSize;
	context->scale = (scale != 0.0f) ? scale : 1.0f;
	context->programTime = programTime;
	context->keyboard = keyboard;
	context->mouse = mouse;
	context->touchscreen = touchscreen;
	
	if (context->hasDoneOneLayout)
	{
		VarArrayExpand(&context->prevElements, context->elements.length);
		VarArrayClear(&context->prevElements);
		UiElement* prevElemsRange = VarArrayAddMulti(UiElement, &context->prevElements, context->elements.length);
		UiElement* elemsRange = VarArrayGetFirst(UiElement, &context->elements);
		MyMemCopy(prevElemsRange, elemsRange, sizeof(UiElement) * context->elements.length);
	}
	VarArrayClear(&context->elements);
	context->numTopLevelElements = 0;
	context->currentElementIndex = PIG_UI_INDEX_INVALID;
	
	UiThemerRegistryStartFrame(&context->themers);
	
	Assert(UiCtx == nullptr);
	UiCtx = context;
	
	UiElement* rootElement = OpenUiElement(NEW_STRUCT(UiElemConfig){
		.id = UiId_Root,
		.sizing = UI_FIXED2(screenSize.Width / context->scale, screenSize.Height / context->scale),
		.color = backgroundColor,
		.depth = PIG_UI_ROOT_DEPTH,
	});
	UNUSED(rootElement);
}

PEXPI Str8 GetUiElementQualifiedName(Arena* arena, UiElement* element)
{
	DebugNotNull(element);
	TwoPassStr8Loop(result, arena, false)
	{
		for (uxx depth = 0; depth < element->treeDepth; depth++)
		{
			UiElement* parent = GetUiElementParent(element, (element->treeDepth-1) - depth);
			DebugNotNull(parent);
			TwoPassStr(&result, parent->config.id.str);
			if (parent->config.id.index != PIG_UI_ID_INDEX_NONE) { TwoPassPrint(&result, "[%llu]", parent->config.id.index); }
			TwoPassChar(&result, '.');
		}
		TwoPassStr(&result, element->config.id.str);
		if (element->config.id.index != PIG_UI_ID_INDEX_NONE) { TwoPassPrint(&result, "[%llu]", element->config.id.index); }
		TwoPassStr8LoopEnd(&result);
	}
	return result.str;
}

// +==============================+
// |     UI Layout Algorithm      |
// +==============================+
static void CalcUiElementMinimumAndPreferredOnAxis(UiElement* element, UiElement* parent, bool xAxis)
{
	DebugNotNull(element);
	DebugAssert(parent != nullptr || element->elementIndex == 0); //parent can be null, only for the root element
	bool isThisLayoutDir = (IsUiDirHorizontal(element->config.direction) == xAxis);
	r32 layoutAxisChildPadding = isThisLayoutDir ? ((r32)(element->numNonFloatingChildren > 1 ? element->numNonFloatingChildren-1 : 0) * element->config.padding.child) : 0.0f;
	r32 elemInnerPaddingLrOrTb = (xAxis ? (element->config.padding.inner.Left + element->config.padding.inner.Right) : (element->config.padding.inner.Top + element->config.padding.inner.Bottom));
	
	r32* minimumSizePntr = (xAxis ? &element->minimumSize.Width : &element->minimumSize.Height);
	r32* preferredSizePntr = (xAxis ? &element->preferredSize.Width : &element->preferredSize.Height);
	
	UiSizingType sizingType = (xAxis ? element->config.sizing.x.type : element->config.sizing.y.type);
	r32 sizingValue = (xAxis ? element->config.sizing.x.value : element->config.sizing.y.value);
	if (sizingType == UiSizingType_FixedPx)
	{
		*minimumSizePntr = sizingValue;
		*preferredSizePntr = sizingValue;
	}
	else if (sizingType == UiSizingType_FixedPercent)
	{
		// NOTE: Nothing special happens here for percentage sizing. We handle this primarily in DistributeSpaceToUiElemChildrenOnAxis
		*minimumSizePntr = elemInnerPaddingLrOrTb + layoutAxisChildPadding;
		*preferredSizePntr = elemInnerPaddingLrOrTb + layoutAxisChildPadding;
	}
	else if (sizingType == UiSizingType_Fit)
	{
		*minimumSizePntr += elemInnerPaddingLrOrTb + layoutAxisChildPadding;
		if (!IsInfiniteOrNanR32(*preferredSizePntr)) { *preferredSizePntr += elemInnerPaddingLrOrTb + layoutAxisChildPadding; }
	}
	else if (sizingType == UiSizingType_Expand)
	{
		*minimumSizePntr += elemInnerPaddingLrOrTb + layoutAxisChildPadding;
		if (*minimumSizePntr < sizingValue) { *minimumSizePntr = sizingValue; }
		*preferredSizePntr = INFINITY;
	}
	else if (sizingType == UiSizingType_TextClip || sizingType == UiSizingType_TextWrap)
	{
		AssertMsg(element->config.font != nullptr, "Text elements must have a font set!");
		AssertMsg(element->numChildren == 0, "Text elements cannot have children!");
		TextMeasure measure = ZEROED;
		r32 wrapWidth = element->config.textWrapWidth;
		if (!xAxis && sizingType == UiSizingType_TextWrap)
		{
			wrapWidth = element->layoutRec.Width; //The layoutRec.Width is already decided because of the full X-axis sizing pass before Y-axis in UiSystemDoLayout
			if (element->config.textWrapWidth != 0.0f && wrapWidth > element->config.textWrapWidth) { wrapWidth = element->config.textWrapWidth; }
		}
		r32 fontSize = ((element->config.fontSize != 0.0f) ? element->config.fontSize : GetDefaultFontSize(element->config.font));
		if (!IsEmptyRichStr(element->config.richText))
		{
			measure = MeasureRichTextEx(element->config.font, fontSize, element->config.fontStyle, true, wrapWidth, element->config.richText);
		}
		else
		{
			measure = MeasureTextEx(element->config.font, fontSize, element->config.fontStyle, true, wrapWidth, element->config.text);
		}
		r32 minWidthOrWrapWidth = MaxR32(sizingValue, element->config.textWrapWidth);
		r32 measureHeightOrLineHeight = MaxR32(measure.visualRec.Height, GetFontLineHeight(element->config.font, fontSize, element->config.fontStyle));
		*minimumSizePntr = (xAxis ? ((minWidthOrWrapWidth >= 0.0f) ? minWidthOrWrapWidth : measure.visualRec.Width) : measureHeightOrLineHeight);
		*preferredSizePntr = (xAxis ? measure.visualRec.Width : measureHeightOrLineHeight);
	}
	else { DebugAssert(false); }
}

static void PropogateMinimumAndPreferredSizeToParentOnAxis(UiElement* element, UiElement* parent, bool xAxis)
{
	DebugNotNull(element);
	DebugAssert(parent != nullptr || element->elementIndex == 0); //parent can be null, only for the root element
	if (parent != nullptr && element->config.floating.type == UiFloatingType_None)
	{
		r32 minimumSize = (xAxis ? element->minimumSize.Width : element->minimumSize.Height);
		r32 preferredSize = (xAxis ? element->preferredSize.Width : element->preferredSize.Height);
		
		bool isParentLayoutDir = (IsUiDirHorizontal(parent->config.direction) == xAxis);
		r32 outerPaddingLrOrTb = (xAxis ? (element->config.padding.outer.Left + element->config.padding.outer.Right) : (element->config.padding.outer.Top + element->config.padding.outer.Bottom));
		r32* parentMinimumSizePntr = (xAxis ? &parent->minimumSize.Width : &parent->minimumSize.Height);
		r32* parentPreferredSizePntr = (xAxis ? &parent->preferredSize.Width : &parent->preferredSize.Height);
		
		if (isParentLayoutDir) { *parentMinimumSizePntr += minimumSize + outerPaddingLrOrTb; }
		else { *parentMinimumSizePntr = MaxR32(*parentMinimumSizePntr, minimumSize + outerPaddingLrOrTb); }
		
		if (!IsInfiniteOrNanR32(*parentPreferredSizePntr))
		{
			if (IsInfiniteOrNanR32(preferredSize)) { *parentPreferredSizePntr = INFINITY; }
			else if (isParentLayoutDir) { *parentPreferredSizePntr += preferredSize + outerPaddingLrOrTb; }
			else { *parentPreferredSizePntr = MaxR32(*parentPreferredSizePntr, preferredSize + outerPaddingLrOrTb); }
		}
	}
}

static void CalculateSizeForFloatingUiElementOnAxis(UiElement* element, bool xAxis)
{
	//TODO: This is a weird subset of the logic that lives in CalculateUiElemSizeOnAxisOnClose and DistributeSpaceToUiElemChildrenOnAxis. Maybe we can merge these better somehow?
	NotNull(element);
	if (element->config.floating.type != UiFloatingType_None)
	{
		r32* minimumSizePntr = (xAxis ? &element->minimumSize.Width : &element->minimumSize.Height);
		r32* preferredSizePntr = (xAxis ? &element->preferredSize.Width : &element->preferredSize.Height);
		
		UiElement* attachParent = GetUiElementAttachParent(element);
		DebugNotNull(attachParent);
		r32 attachParentSize = (xAxis ? attachParent->layoutRec.Width : attachParent->layoutRec.Height);
		
		UiSizingType elemSizingType = (xAxis ? element->config.sizing.x.type : element->config.sizing.y.type);
		r32 elemSizingValue = (xAxis ? element->config.sizing.x.value : element->config.sizing.y.value);
		if (elemSizingType == UiSizingType_FixedPercent)
		{
			*minimumSizePntr = attachParentSize * elemSizingValue; //TODO: This should take outer padding into account
			*preferredSizePntr = *minimumSizePntr;
		}
		else if (elemSizingType == UiSizingType_Expand)
		{
			*minimumSizePntr = attachParentSize;
			if (*minimumSizePntr < elemSizingValue) { *minimumSizePntr = elemSizingValue; }
			*preferredSizePntr = INFINITY;
		}
		
		r32* sizePntr = (xAxis ? &element->layoutRec.Width : &element->layoutRec.Height);
		*sizePntr = IsInfiniteOrNanR32(*preferredSizePntr) ? *minimumSizePntr : *preferredSizePntr;
	}
}

static void DistributeSpaceToUiElemChildrenOnAxis(UiElement* element, bool xAxis, bool printDebug)
{
	#if !DEBUG_BUILD
	UNUSED(printDebug);
	#endif
	r32* sizePntr = (xAxis ? &element->layoutRec.Width : &element->layoutRec.Height);
	bool isLayoutDir = (IsUiDirHorizontal(element->config.direction) == xAxis);
	r32 layoutAxisChildPadding = isLayoutDir ? ((r32)(element->numNonFloatingChildren > 1 ? element->numNonFloatingChildren-1 : 0) * element->config.padding.child) : 0.0f;
	r32 elemInnerPaddingLrOrTb = (xAxis ? (element->config.padding.inner.Left + element->config.padding.inner.Right) : (element->config.padding.inner.Top + element->config.padding.inner.Bottom));
	r32 minimumSize = (xAxis ? element->minimumSize.Width : element->minimumSize.Height);
	r32 preferredSize = (xAxis ? element->preferredSize.Width : element->preferredSize.Height);
	r32 innerSize = *sizePntr - elemInnerPaddingLrOrTb;
	
	// Visit all percentage-based children (including floating) and size them according to our decided size
	for (uxx cIndex = 0; cIndex < element->numChildren; cIndex++)
	{
		UiElement* child = GetUiElementChild(element, cIndex);
		if (child->config.floating.type == UiFloatingType_None)
		{
			UiSizingType childSizingType = (xAxis ? child->config.sizing.x.type : child->config.sizing.y.type);
			if (childSizingType == UiSizingType_FixedPercent)
			{
				r32 childSizingValue = (xAxis ? child->config.sizing.x.value : child->config.sizing.y.value);
				r32* childMinimumSizePntr = (xAxis ? &child->minimumSize.Width : &child->minimumSize.Height);
				r32* childPreferredSizePntr = (xAxis ? &child->preferredSize.Width : &child->preferredSize.Height);
				*childMinimumSizePntr = innerSize * childSizingValue; //TODO: This should take outer padding into account
				*childPreferredSizePntr = *childMinimumSizePntr;
			}
		}
	}
	
	if (isLayoutDir)
	{
		// Copy children's minimumSize into layoutRec.Size and track how many want to be bigger and what their total minimum size is
		uxx numGrowableChildren = 0;
		r32 childrenMinimumTotal = elemInnerPaddingLrOrTb + (element->config.padding.child * (element->numNonFloatingChildren-1));
		for (uxx cIndex = 0; cIndex < element->numChildren; cIndex++)
		{
			UiElement* child = GetUiElementChild(element, cIndex);
			if (child->config.floating.type == UiFloatingType_None)
			{
				r32 childMinimumSize = (xAxis ? child->minimumSize.Width : child->minimumSize.Height);
				r32 childPreferredSize = (xAxis ? child->preferredSize.Width : child->preferredSize.Height);
				r32 childOuterPaddingLrOrTb = (xAxis ? (child->config.padding.outer.Left + child->config.padding.outer.Right) : (child->config.padding.outer.Top + child->config.padding.outer.Bottom));
				r32* childSizePntr = (xAxis ? &child->layoutRec.Width : &child->layoutRec.Height);
				if (IsInfiniteOrNanR32(childPreferredSize) || childPreferredSize > childMinimumSize) { numGrowableChildren++; }
				*childSizePntr = childMinimumSize;
				childrenMinimumTotal += childMinimumSize + childOuterPaddingLrOrTb;
			}
		}
		
		//TODO: We should probably be rounding to whole pixel values, and figuring out how to distribute remainders amongst n children
		r32 spaceToDistribute = (*sizePntr - childrenMinimumTotal);
		#if DEBUG_BUILD
		if (printDebug)
		{
			Str8 fullName = GetUiElementQualifiedName(UiCtx->frameArena, element);
			PrintLine_D("%.*s has %gpx along %s to distribute amongst %llu/%llu growable child%s",
				StrPrint(fullName),
				spaceToDistribute,
				xAxis ? "X" : "Y",
				numGrowableChildren, element->numChildren, PluralEx(numGrowableChildren, "", "ren")
			);
		}
		#endif //DEBUG_BUILD
		
		uxx distRoundIndex = 0;
		while (spaceToDistribute > 0.0f + DEFAULT_R32_TOLERANCE && distRoundIndex < element->numChildren)
		{
			r32 smallestChildSize = INFINITY;
			uxx smallestChildCount = 0;
			r32 secondSmallestChildSize = INFINITY;
			for (uxx cIndex = 0; cIndex < element->numChildren; cIndex++)
			{
				UiElement* child = GetUiElementChild(element, cIndex);
				if (child->config.floating.type == UiFloatingType_None)
				{
					r32 childMinimumSize = (xAxis ? child->minimumSize.Width : child->minimumSize.Height);
					r32 childPreferredSize = (xAxis ? child->preferredSize.Width : child->preferredSize.Height);
					r32 childSize = (xAxis ? child->layoutRec.Width : child->layoutRec.Height);
					// #if DEBUG_BUILD
					// if (printDebug) { PrintLine_D("\t-> Child[%llu] is %g, [%g,%g]", cIndex, childSize, childMinimumSize, childPreferredSize); }
					// #endif
					if (IsInfiniteOrNanR32(childPreferredSize) || (childPreferredSize > childMinimumSize && childSize < childPreferredSize))
					{
						if (IsInfiniteOrNanR32(smallestChildSize) || childSize < smallestChildSize)
						{
							// #if DEBUG_BUILD
							// if (printDebug) { PrintLine_D("\t-> Child[%llu] is new smallest!", cIndex); }
							// #endif
							if (!IsInfiniteOrNanR32(smallestChildSize)) { secondSmallestChildSize = smallestChildSize; }
							if (childPreferredSize < secondSmallestChildSize) { secondSmallestChildSize = childPreferredSize; }
							smallestChildSize = childSize;
							smallestChildCount = 1;
						}
						else if (AreSimilarR32(smallestChildSize, childSize, DEFAULT_R32_TOLERANCE))
						{
							// #if DEBUG_BUILD
							// if (printDebug) { PrintLine_D("\t-> Child[%llu] is same as smallest!", cIndex); }
							// #endif
							if (childPreferredSize < secondSmallestChildSize) { secondSmallestChildSize = childPreferredSize; }
							smallestChildCount++;
						}
						else if (childSize < secondSmallestChildSize)
						{
							// #if DEBUG_BUILD
							// if (printDebug) { PrintLine_D("\t-> Child[%llu] is new second smallest!", cIndex); }
							// #endif
							secondSmallestChildSize = childSize;
						}
					}
					// #if DEBUG_BUILD
					// if (printDebug) { PrintLine_D("\t-> After Child[%llu]: %llux %g (%g)", cIndex, smallestChildCount, smallestChildSize, secondSmallestChildSize); }
					// #endif
				}
			}
			if (smallestChildCount == 0) { break; }
			
			r32 smallestToNextSmallestDiff = (!IsInfiniteOrNanR32(secondSmallestChildSize) && smallestChildCount > 0)
				? (secondSmallestChildSize - smallestChildSize)
				: INFINITY;
			r32 spaceToDistributeThisRound = MinR32(spaceToDistribute, smallestToNextSmallestDiff * (r32)smallestChildCount);
			// #if DEBUG_BUILD
			// if (printDebug) { PrintLine_D("\t[%llu] There are %llu children at %g that can expand to %g. Distributing %g (%g left)...", distRoundIndex, smallestChildCount, smallestChildSize, secondSmallestChildSize, spaceToDistributeThisRound, spaceToDistribute - spaceToDistributeThisRound); }
			// #endif
			if (spaceToDistributeThisRound <= 0.0f) { break; }
			spaceToDistribute -= spaceToDistributeThisRound;
			for (uxx cIndex = 0; cIndex < element->numChildren; cIndex++)
			{
				UiElement* child = GetUiElementChild(element, cIndex);
				if (child->config.floating.type == UiFloatingType_None)
				{
					r32 childMinimumSize = (xAxis ? child->minimumSize.Width : child->minimumSize.Height);
					r32 childPreferredSize = (xAxis ? child->preferredSize.Width : child->preferredSize.Height);
					// r32 childOuterPaddingLrOrTb = (xAxis ? (child->config.padding.outer.Left + child->config.padding.outer.Right) : (child->config.padding.outer.Top + child->config.padding.outer.Bottom));
					r32* childSizePntr = (xAxis ? &child->layoutRec.Width : &child->layoutRec.Height);
					if (AreSimilarR32(*childSizePntr, smallestChildSize, DEFAULT_R32_TOLERANCE) &&
						(IsInfiniteOrNanR32(childPreferredSize) || childPreferredSize > childMinimumSize))
					{
						*childSizePntr += (spaceToDistributeThisRound / (r32)smallestChildCount);
					}
				}
			}
			distRoundIndex++;
		}
		// else if (spaceToDistribute < 0)
		// {
		// 	//TODO: Implement me!
		// }
	}
	else //sizing in the non-layout direction
	{
		for (uxx cIndex = 0; cIndex < element->numChildren; cIndex++)
		{
			UiElement* child = GetUiElementChild(element, cIndex);
			if (child->config.floating.type == UiFloatingType_None)
			{
				r32 childMinimumSize = (xAxis ? child->minimumSize.Width : child->minimumSize.Height);
				r32 childPreferredSize = (xAxis ? child->preferredSize.Width : child->preferredSize.Height);
				r32 childOuterPaddingLrOrTb = (xAxis ? (child->config.padding.outer.Left + child->config.padding.outer.Right) : (child->config.padding.outer.Top + child->config.padding.outer.Bottom));
				r32* childSizePntr = (xAxis ? &child->layoutRec.Width : &child->layoutRec.Height);
				if (IsInfiniteOrNanR32(childPreferredSize) || childPreferredSize > childMinimumSize)
				{
					*childSizePntr = MinR32(innerSize - childOuterPaddingLrOrTb, childPreferredSize);
				}
				else
				{
					*childSizePntr = childMinimumSize;
				}
			}
		}
	}
}

static void TrackMouseInteractionAfterUiLayout()
{
	UiElement* hoveredElement = nullptr;
	if (UiCtx->mouse != nullptr && UiCtx->mouse->isOverWindow)
	{
		VarArrayLoop(&UiCtx->elements, eIndex)
		{
			VarArrayLoopGet(UiElement, element, &UiCtx->elements, eIndex);
			if (!element->config.mousePassthrough && IsInsideRec(element->layoutRec, UiCtx->mouse->position) && IsInsideRec(element->clipRec, UiCtx->mouse->position))
			{
				if (hoveredElement == nullptr ||
					element->config.depth < hoveredElement->config.depth ||
					(element->config.depth == hoveredElement->config.depth && element->elementIndex >= hoveredElement->elementIndex))
				{
					hoveredElement = element;
				}
			}
		}
	}
	UiCtx->mouseHoveredId = (hoveredElement != nullptr) ? hoveredElement->id : UiId_None;
	UiCtx->mouseHoveredLocalId = (hoveredElement != nullptr) ? hoveredElement->config.id : UiId_None;
	// PrintLine_D("Hovering over \"%.*s\"", StrPrint(UiCtx->mouseHoveredId.str));

	for (uxx bIndex = 1; bIndex < MouseBtn_Count; bIndex++)
	{
		MouseBtn mouseBtn = (MouseBtn)bIndex;
		if (UiCtx->clickStartHoveredId[mouseBtn].id != 0 && UiCtx->mouseHoveredId.id != UiCtx->clickStartHoveredId[mouseBtn].id)
		{
			UiCtx->clickStartHoveredId[mouseBtn] = UiId_None;
			UiCtx->clickStartHoveredLocalId[mouseBtn] = UiId_None;
		}
		if (!IsMouseBtnDown(UiCtx->mouse, nullptr, mouseBtn) && !IsMouseBtnReleased(UiCtx->mouse, nullptr, mouseBtn))
		{
			UiCtx->clickStartHoveredId[mouseBtn] = UiId_None;
			UiCtx->clickStartHoveredLocalId[mouseBtn] = UiId_None;
		}
		else if (IsMouseBtnPressed(UiCtx->mouse, nullptr, mouseBtn))
		{
			UiCtx->clickStartHoveredId[mouseBtn] = UiCtx->mouseHoveredId;
			UiCtx->clickStartHoveredLocalId[mouseBtn] = UiCtx->mouseHoveredLocalId;
		}
	}
}

// +==================================================+
// | UiElementPntr_FloatDepthThenElementIndex_Compare |
// +==================================================+
// i32 UiElementPntr_FloatDepthThenElementIndex_Compare(const void* left, const void* right, void* contextPntr)
COMPARE_FUNC_DEF(UiElementPntr_FloatDepthThenElementIndex_Compare)
{
	UNUSED(contextPntr);
	UiElement* leftElement = *((UiElement**)left);
	UiElement* rightElement = *((UiElement**)right);
	if (leftElement->floatDepth > rightElement->floatDepth) { return 1; }
	else if (leftElement->floatDepth < rightElement->floatDepth) { return -1; }
	else if (leftElement->elementIndex > rightElement->elementIndex) { return 1; }
	else if (leftElement->elementIndex < rightElement->elementIndex) { return -1; }
	else { return 0; }
}

static void UiSystemDoLayout()
{
	ScratchBegin2(scratch, UiCtx->arena, UiCtx->frameArena);
	bool printDebug = IsKeyboardKeyPressed(UiCtx->keyboard, nullptr, Key_T, false);
	// The root element decides it's own final position/size
	// Most elements have their size decided by their parent inside DistributeSpaceToUiElemChildrenOnAxis
	// Floating elements are the only other elements that decide their own position/size
	UiElement* rootElement = VarArrayGetFirst(UiElement, &UiCtx->elements);
	rootElement->layoutRec.TopLeft = V2_Zero;
	rootElement->layoutRec.Size = UiCtx->screenSize;
	rootElement->clipRec = rootElement->layoutRec;
	
	VarArray sortedElements = ZEROED; //UiElement*
	InitVarArrayWithInitial(UiElement*, &sortedElements, scratch, UiCtx->elements.length);
	UiElement** sortedElementsRange = VarArrayAddMulti(UiElement*, &sortedElements, UiCtx->elements.length);
	VarArrayLoop(&UiCtx->elements, eIndex)
	{
		VarArrayLoopGet(UiElement, element, &UiCtx->elements, eIndex);
		sortedElementsRange[eIndex] = element;
	}
	QuickSortVarArray(UiElement*, &sortedElements, UiElementPntr_FloatDepthThenElementIndex_Compare, nullptr);
	#if DEBUG_BUILD
	if (printDebug)
	{
		PrintLine_D("Sorted %llu elements:", sortedElements.length);
		VarArrayLoop(&sortedElements, eIndex)
		{
			VarArrayLoopGetValue(UiElement*, element, &sortedElements, eIndex);
			Str8 fullName = GetUiElementQualifiedName(scratch, element);
			PrintLine_D("[%llu] \"%.*s\" elementIndex=%llu treeDepth=%llu floatDepth=%llu",
				eIndex,
				StrPrint(fullName),
				element->elementIndex,
				element->treeDepth,
				element->floatDepth
			);
		}
	}
	#endif
	
	//NOTE: Because text wrapping will affect the minimumSize along the Y-axis of text elements based on the
	//      final size given on the X-axis, we have to do both passes of X-axis sizing BEFORE we do Y-axis sizing
	// +================================+
	// | Size X-axis in Bottom-Up order |
	// +================================+
	VarArrayLoop(&sortedElements, eIndex)
	{
		VarArrayLoopGetValueReverse(UiElement*, element, &sortedElements, eIndex);
		// #if DEBUG_BUILD
		// if (printDebug)
		// {
		// 	Str8 fullName = GetUiElementQualifiedName(scratch, element);
		// 	PrintLine_D("Visiting %.*s X-Axis (Min/Preferred Pass)", StrPrint(fullName));
		// }
		// #endif
		UiElement* parent = GetUiElementParent(element, 0);
		CalcUiElementMinimumAndPreferredOnAxis(element, parent, true);
		PropogateMinimumAndPreferredSizeToParentOnAxis(element, parent, true);
	}
	// +======================================+
	// | Distribute X-axis in Top-Down order  |
	// +======================================+
	VarArrayLoop(&sortedElements, eIndex)
	{
		VarArrayLoopGetValue(UiElement*, element, &sortedElements, eIndex);
		// #if DEBUG_BUILD
		// if (printDebug)
		// {
		// 	Str8 fullName = GetUiElementQualifiedName(scratch, element);
		// 	PrintLine_D("Visiting %.*s X-Axis (Distribute Pass)", StrPrint(fullName));
		// }
		// #endif
		if (element->config.floating.type != UiFloatingType_None) { CalculateSizeForFloatingUiElementOnAxis(element, true); }
		if (element->numChildren > 0)
		{
			#if DEBUG_BUILD
			if (printDebug)
			{
				Str8 fullName = GetUiElementQualifiedName(UiCtx->frameArena, element);
				PrintLine_D("0x%016llX %.*s Sizing X-Axis %llu child%s",
					element->id.id,
					StrPrint(fullName),
					element->numChildren, PluralEx(element->numChildren, "", "ren")
				);
			}
			#endif //DEBUG_BUILD
			DistributeSpaceToUiElemChildrenOnAxis(element, true, printDebug);
		}
	}
	
	// +================================+
	// | Size Y-axis in Bottom-Up order |
	// +================================+
	VarArrayLoop(&sortedElements, eIndex)
	{
		VarArrayLoopGetValueReverse(UiElement*, element, &sortedElements, eIndex);
		// #if DEBUG_BUILD
		// if (printDebug)
		// {
		// 	Str8 fullName = GetUiElementQualifiedName(scratch, element);
		// 	PrintLine_D("Visiting %.*s Y-Axis (Min/Preferred Pass)", StrPrint(fullName));
		// }
		// #endif
		UiElement* parent = GetUiElementParent(element, 0);
		CalcUiElementMinimumAndPreferredOnAxis(element, parent, false);
		PropogateMinimumAndPreferredSizeToParentOnAxis(element, parent, false);
	}
	// +======================================+
	// | Distribute Y-axis in Top-Down order  |
	// +======================================+
	VarArrayLoop(&sortedElements, eIndex)
	{
		VarArrayLoopGetValue(UiElement*, element, &sortedElements, eIndex);
		// #if DEBUG_BUILD
		// if (printDebug)
		// {
		// 	Str8 fullName = GetUiElementQualifiedName(scratch, element);
		// 	PrintLine_D("Visiting %.*s Y-Axis (Distribute Pass)", StrPrint(fullName));
		// }
		// #endif
		if (element->config.floating.type != UiFloatingType_None) { CalculateSizeForFloatingUiElementOnAxis(element, false); }
		if (element->numChildren > 0)
		{
			#if DEBUG_BUILD
			if (printDebug)
			{
				Str8 fullName = GetUiElementQualifiedName(scratch, element);
				PrintLine_D("0x%016llX %.*s Sizing Y-Axis %llu child%s",
					element->id.id,
					StrPrint(fullName),
					element->numChildren, PluralEx(element->numChildren, "", "ren")
				);
			}
			#endif //DEBUG_BUILD
			DistributeSpaceToUiElemChildrenOnAxis(element, false, printDebug);
		}
	}
	
	#if DEBUG_BUILD
	if (printDebug)
	{
		VarArrayLoop(&UiCtx->elements, eIndex)
		{
			VarArrayLoopGet(UiElement, element, &UiCtx->elements, eIndex);
			Str8 fullName = GetUiElementQualifiedName(UiCtx->frameArena, element);
			PrintLine_D("0x%016llX %.*s: element=%llu treeDepth=%llu sibling=%llu parent=%llu children=%llu descendents=%llu - sizing[X=%s, Y=%s] minimum(%g,%g) preferred(%g,%g)",
				element->id.id,
				StrPrint(fullName),
				element->elementIndex,
				element->treeDepth,
				element->siblingIndex,
				element->parentIndex,
				element->numChildren,
				element->numDescendents,
				GetUiSizingTypeStr(element->config.sizing.x.type), GetUiSizingTypeStr(element->config.sizing.y.type),
				element->minimumSize.Width, element->minimumSize.Height,
				element->preferredSize.Width, element->preferredSize.Height
			);
		}
	}
	#endif //DEBUG_BUILD
	
	// +============================================+
	// | Position Children According to UiLayoutDir |
	// +============================================+
	VarArrayLoop(&sortedElements, eIndex)
	{
		VarArrayLoopGetValue(UiElement*, element, &sortedElements, eIndex);
		
		if (element->config.floating.type != UiFloatingType_None)
		{
			UiElement* attachParent = GetUiElementAttachParent(element);
			DebugNotNull(attachParent);
			v2 parentHalfSize = ScaleV2(attachParent->layoutRec.Size, 0.5f);
			v2 elemHalfSize = ScaleV2(element->layoutRec.Size, 0.5f);
			v2 parentAttachPos = AddV2(attachParent->layoutRec.TopLeft, parentHalfSize);
			v2 parentDirMult = ToV2Fromi(V2iFromDir2Ex(element->config.floating.parentSide));
			v2 elemDirMult = ToV2Fromi(V2iFromDir2Ex(element->config.floating.elemSide));
			parentAttachPos = AddV2(parentAttachPos, MulV2(parentHalfSize, parentDirMult));
			parentAttachPos = SubV2(parentAttachPos, elemHalfSize);
			parentAttachPos = SubV2(parentAttachPos, MulV2(elemHalfSize, elemDirMult));
			parentAttachPos = AddV2(parentAttachPos, element->config.floating.offset);
			element->layoutRec.TopLeft = parentAttachPos;
			#if DEBUG_BUILD
			if (printDebug)
			{
				PrintLine_D("Put floating element \"%.*s\" at (%g,%g) from parent \"%.*s\" (%g, %g)+(%g,%g)",
					StrPrint(element->id.str),
					element->layoutRec.X, element->layoutRec.Y,
					StrPrint(attachParent->id.str),
					attachParent->layoutRec.X, attachParent->layoutRec.Y,
					element->config.floating.offset.X, element->config.floating.offset.Y
				);
			}
			#endif
		}
		
		v2 childrenTotalSize = V2_Zero;
		uxx nonFloatingChildIndex = 0;
		for (uxx cIndex = 0; cIndex < element->numChildren; cIndex++)
		{
			UiElement* child = GetUiElementChild(element, cIndex);
			if (child->config.floating.type == UiFloatingType_None)
			{
				v2 childSizeAndPadding = AddV2(child->layoutRec.Size, MakeV2(
					child->config.padding.outer.Left + child->config.padding.outer.Right,
					child->config.padding.outer.Top + child->config.padding.outer.Bottom
				));
				if (IsUiDirHorizontal(element->config.direction))
				{
					if (nonFloatingChildIndex > 0) { childrenTotalSize.Width += element->config.padding.child; }
					childrenTotalSize.Width += childSizeAndPadding.Width;
					childrenTotalSize.Height = MaxR32(childrenTotalSize.Height, childSizeAndPadding.Height);
				}
				else
				{
					if (nonFloatingChildIndex > 0) { childrenTotalSize.Height += element->config.padding.child; }
					childrenTotalSize.Width = MaxR32(childrenTotalSize.Width, childSizeAndPadding.Width);
					childrenTotalSize.Height += childSizeAndPadding.Height;
				}
				nonFloatingChildIndex++;
			}
		}
		v2 innerSize = SubV2(element->layoutRec.Size, MakeV2(
			element->config.padding.inner.Left + element->config.padding.inner.Right,
			element->config.padding.inner.Top + element->config.padding.inner.Bottom
		));
		v2 emptySpaceForAlignment = MakeV2(
			MaxR32(0.0f, innerSize.Width - childrenTotalSize.Width),
			MaxR32(0.0f, innerSize.Height - childrenTotalSize.Height)
		);
		v2 alignmentOffset = V2_Zero;
		if (element->config.alignment.x == UiAlignmentType_Left)   { alignmentOffset.X = emptySpaceForAlignment.X * 0.0f; }
		if (element->config.alignment.x == UiAlignmentType_Center) { alignmentOffset.X = emptySpaceForAlignment.X * 0.5f; }
		if (element->config.alignment.x == UiAlignmentType_Right)  { alignmentOffset.X = emptySpaceForAlignment.X * 1.0f; }
		if (element->config.alignment.y == UiAlignmentType_Top)    { alignmentOffset.Y = emptySpaceForAlignment.Y * 0.0f; }
		if (element->config.alignment.y == UiAlignmentType_Center) { alignmentOffset.Y = emptySpaceForAlignment.Y * 0.5f; }
		if (element->config.alignment.y == UiAlignmentType_Bottom) { alignmentOffset.Y = emptySpaceForAlignment.Y * 1.0f; }
		
		v2 layoutPos = V2_Zero_Const;
		if (element->config.direction == UiLayoutDir_LeftToRight || element->config.direction == UiLayoutDir_TopDown)
		{
			layoutPos = AddV2(element->layoutRec.TopLeft, element->config.padding.inner.XY); //XY is alias for (Left,Top)
			layoutPos = AddV2(layoutPos, alignmentOffset);
		}
		else if (element->config.direction == UiLayoutDir_RightToLeft)
		{
			layoutPos = AddV2(element->layoutRec.TopLeft, MakeV2(element->layoutRec.Width - element->config.padding.inner.Right, element->config.padding.inner.Top));
			//TODO: How do we take alignmentOffset into account?
		}
		else if (element->config.direction == UiLayoutDir_BottomUp)
		{
			layoutPos = AddV2(element->layoutRec.TopLeft, MakeV2(element->config.padding.inner.Left, element->layoutRec.Height - element->config.padding.inner.Bottom));
			//TODO: How do we take alignmentOffset into account?
		}
		else { DebugAssert(false); }
		
		for (uxx cIndex = 0; cIndex < element->numChildren; cIndex++)
		{
			UiElement* child = GetUiElementChild(element, cIndex);
			
			if (child->config.floating.type == UiFloatingType_None)
			{
				r32 childPadding = ((cIndex > 0) ? element->config.padding.child : 0.0f);
				if (element->config.direction == UiLayoutDir_LeftToRight) { layoutPos.X += childPadding + child->config.padding.outer.Left; }
				if (element->config.direction == UiLayoutDir_TopDown) { layoutPos.Y += childPadding + child->config.padding.outer.Top; }
				if (element->config.direction == UiLayoutDir_RightToLeft) { layoutPos.X -= child->layoutRec.Width + childPadding + child->config.padding.outer.Right; }
				if (element->config.direction == UiLayoutDir_BottomUp) { layoutPos.Y -= child->layoutRec.Height + childPadding + child->config.padding.outer.Bottom; }
				
				child->layoutRec.TopLeft = layoutPos;
				if (IsUiDirHorizontal(element->config.direction)) { child->layoutRec.Y += child->config.padding.outer.Top; }
				else { child->layoutRec.X += child->config.padding.outer.Left; }
				
				if (element->config.direction == UiLayoutDir_LeftToRight) { layoutPos.X += child->layoutRec.Width + child->config.padding.outer.Right; }
				if (element->config.direction == UiLayoutDir_TopDown) { layoutPos.Y += child->layoutRec.Height + child->config.padding.outer.Bottom; }
				if (element->config.direction == UiLayoutDir_RightToLeft) { layoutPos.X -= child->config.padding.outer.Left; }
				if (element->config.direction == UiLayoutDir_BottomUp) { layoutPos.Y -= child->config.padding.outer.Top; }
			}
			
			if (element->config.clipChildren) { child->clipRec = OverlapPartRec(element->clipRec, element->layoutRec); }
			else { child->clipRec = element->clipRec; }
			
			#if DEBUG_BUILD
			if (printDebug)
			{
				Str8 fullName = GetUiElementQualifiedName(UiCtx->frameArena, child);
				PrintLine_D("0x%016llX %.*s size=(%g,%g) layoutRec=(%g,%g,%g,%g)",
					child->id.id,
					StrPrint(fullName),
					child->layoutRec.Width, child->layoutRec.Height,
					child->layoutRec.X, child->layoutRec.Y, child->layoutRec.X + child->layoutRec.Width, child->layoutRec.Y + child->layoutRec.Height
				);
			}
			#endif
		}
	}
	
	TrackMouseInteractionAfterUiLayout();
	UiCtx->hasDoneOneLayout = true;
	ScratchEnd(scratch);
}

// +==========================================+
// | UiRenderCmd_DepthAndHierarchySortCompare |
// +==========================================+
// i32 UiRenderCmd_DepthAndHierarchySortCompare(const void* left, const void* right, void* contextPntr)
static COMPARE_FUNC_DEF(UiRenderCmd_DepthAndHierarchySortCompare)
{
	DebugNotNull(left);
	DebugNotNull(right);
	UiRenderCmd* leftCmd = (UiRenderCmd*)left;
	UiRenderCmd* rightCmd = (UiRenderCmd*)right;
	if (leftCmd->depth < rightCmd->depth) { return 1; }
	else if (leftCmd->depth > rightCmd->depth) { return -1; }
	else
	{
		// return 0;
		if (leftCmd->srcElementIndex < rightCmd->srcElementIndex) { return -1; }
		else if (leftCmd->srcElementIndex > rightCmd->srcElementIndex) { return 1; }
		else { return 0; }
	}
}

PEXP UiRenderList* GetUiRenderList()
{
	NotNull(UiCtx);
	AssertUiThreadIsSame();
	
	// Close the root element if it's still open. This also acts as a way for us to know if GetUiRenderList has been called yet this frame
	if (UiCtx->currentElementIndex < UiCtx->elements.length)
	{
		DebugAssertMsg(UiCtx->currentElementIndex == 0, "Not all UI elements had a CloseUiElement call! UI hierarchy is potentially invalid!");
		CloseUiElement();
		UiSystemDoLayout();
	}
	
	// +==============================+
	// |          UI Render           |
	// +==============================+
	rec screenRec = MakeRecV(V2_Zero, UiCtx->screenSize);
	UiCtx->renderList.context = UiCtx;
	UiCtx->renderList.arena = UiCtx->frameArena;
	InitVarArrayWithInitial(UiRenderCmd, &UiCtx->renderList.commands, UiCtx->frameArena, UiCtx->elements.length); //Lower estimate (TODO: Should we decide on a better upper estimate?)
	VarArrayLoop(&UiCtx->elements, eIndex)
	{
		VarArrayLoopGet(UiElement, element, &UiCtx->elements, eIndex);
		if (DoesOverlapRec(element->layoutRec, element->clipRec, true))
		{
			Color32 colorRecursive = UiConfigColorToActualColor(element->config.colorRecursive);
			Color32 colorIfTexture = (element->config.texture != nullptr) ? UiConfigColorToActualColor(element->config.color) : element->config.color;
			Color32 actualColor = ColorMultSimple(colorRecursive, colorIfTexture);
			Color32 actualBorderColor = ColorMultSimple(UiConfigColorToActualColor(element->config.borderColor), colorRecursive);
			Color32 actualTextColor = ColorMultSimple(UiConfigTextColorToActualColor(element->config.textColor), colorRecursive);
			bool isBorderSameDepth = (AreSimilarR32(element->config.borderDepth, element->config.depth, DEFAULT_R32_TOLERANCE) || element->config.borderDepth == 0.0f);
			bool borderHasAlpha = (element->config.borderColor.a != 0 && (element->config.borderThickness.Left != 0 || element->config.borderThickness.Top != 0 || element->config.borderThickness.Right != 0 || element->config.borderThickness.Bottom != 0));
			if (actualColor.a != 0 || (borderHasAlpha && isBorderSameDepth))
			{
				UiRenderCmd* newCmd = VarArrayAdd(UiRenderCmd, &UiCtx->renderList.commands);
				DebugNotNull(newCmd);
				ClearPointer(newCmd);
				newCmd->type = UiRenderCmdType_Rectangle;
				newCmd->depth = element->config.depth;
				newCmd->srcElementIndex = element->elementIndex;
				newCmd->srcElementId = element->id;
				newCmd->clipRec = element->clipRec;
				newCmd->color = actualColor;
				newCmd->rectangle.rectangle = element->layoutRec;
				newCmd->rectangle.texture = element->config.texture;
				if (element->config.texture != nullptr)
				{
					if (element->config.repeatingTexture)
					{
						newCmd->rectangle.sourceRec = MakeRecV(V2_Zero, ScaleV2(element->layoutRec.Size, 1.0f / UiCtx->scale));
					}
					else if (element->config.textureSourceRec.X != 0 || element->config.textureSourceRec.Y != 0 || element->config.textureSourceRec.Width != 0 || element->config.textureSourceRec.Height != 0)
					{
						newCmd->rectangle.sourceRec = element->config.textureSourceRec;
					}
					else 
					{
						newCmd->rectangle.sourceRec = MakeRecV(V2_Zero, ToV2Fromi(element->config.texture->size));
					}
				}
				
				if (isBorderSameDepth)
				{
					newCmd->rectangle.borderThickness = element->config.borderThickness;
					newCmd->rectangle.borderColor = actualBorderColor;
					newCmd->rectangle.cornerRadius = V4r_Zero; //TODO: Implement this!
				}
			}
			if (!isBorderSameDepth && borderHasAlpha)
			{
				UiRenderCmd* newCmd = VarArrayAdd(UiRenderCmd, &UiCtx->renderList.commands);
				DebugNotNull(newCmd);
				ClearPointer(newCmd);
				newCmd->type = UiRenderCmdType_Rectangle;
				newCmd->depth = element->config.borderDepth;
				newCmd->srcElementIndex = element->elementIndex;
				newCmd->srcElementId = element->id;
				newCmd->clipRec = element->clipRec;
				newCmd->color = Transparent;
				newCmd->rectangle.rectangle = element->layoutRec;
				newCmd->rectangle.borderThickness = element->config.borderThickness;
				newCmd->rectangle.borderColor = actualBorderColor;
				newCmd->rectangle.cornerRadius = V4r_Zero; //TODO: Implement this!
			}
			
			bool isWrappingText = (element->config.sizing.x.type == UiSizingType_TextWrap);
			if (!IsEmptyRichStr(element->config.richText))
			{
				rec textClipRec = element->clipRec;
				if (element->config.sizing.x.type == UiSizingType_TextClip) { textClipRec = OverlapPartRec(element->layoutRec, element->clipRec); }
				if (textClipRec.Width > 0 && textClipRec.Height > 0)
				{
					UiRenderCmd* newCmd = VarArrayAdd(UiRenderCmd, &UiCtx->renderList.commands);
					DebugNotNull(newCmd);
					ClearPointer(newCmd);
					newCmd->type = UiRenderCmdType_RichText;
					newCmd->depth = element->config.depth;
					newCmd->srcElementIndex = element->elementIndex;
					newCmd->srcElementId = element->id;
					newCmd->clipRec = textClipRec;
					newCmd->color = actualTextColor;
					newCmd->richText.font = element->config.font;
					newCmd->richText.fontSize = (element->config.fontSize != 0.0f) ? element->config.fontSize : GetDefaultFontSize(newCmd->richText.font);
					newCmd->richText.fontStyle = element->config.fontStyle;
					newCmd->richText.wrapWidth = (isWrappingText ? MaxR32(element->layoutRec.Width, 1.0f) : element->config.textWrapWidth);
					if (isWrappingText && element->config.textWrapWidth != 0.0f && newCmd->richText.wrapWidth > element->config.textWrapWidth) { newCmd->richText.wrapWidth = element->config.textWrapWidth; }
					newCmd->richText.position = AddV2(element->layoutRec.TopLeft, MakeV2(0, GetFontMaxAscend(newCmd->richText.font, newCmd->richText.fontSize, newCmd->richText.fontStyle)));
					newCmd->richText.text = element->config.richText;
				}
			}
			else if (!IsEmptyStr(element->config.text))
			{
				rec textClipRec = element->clipRec;
				if (element->config.sizing.x.type == UiSizingType_TextClip) { textClipRec = OverlapPartRec(element->layoutRec, element->clipRec); }
				if (textClipRec.Width > 0 && textClipRec.Height > 0)
				{
					UiRenderCmd* newCmd = VarArrayAdd(UiRenderCmd, &UiCtx->renderList.commands);
					DebugNotNull(newCmd);
					ClearPointer(newCmd);
					newCmd->type = UiRenderCmdType_Text;
					newCmd->depth = element->config.depth;
					newCmd->srcElementIndex = element->elementIndex;
					newCmd->srcElementId = element->id;
					newCmd->clipRec = textClipRec;
					newCmd->color = actualTextColor;
					newCmd->text.font = element->config.font;
					newCmd->text.fontSize = (element->config.fontSize != 0.0f) ? element->config.fontSize : GetDefaultFontSize(newCmd->text.font);
					newCmd->text.fontStyle = element->config.fontStyle;
					newCmd->richText.wrapWidth = (isWrappingText ? MaxR32(element->layoutRec.Width, 1.0f) : element->config.textWrapWidth);
					if (isWrappingText && element->config.textWrapWidth != 0.0f && newCmd->richText.wrapWidth > element->config.textWrapWidth) { newCmd->richText.wrapWidth = element->config.textWrapWidth; }
					newCmd->text.position = AddV2(element->layoutRec.TopLeft, MakeV2(0, GetFontMaxAscend(newCmd->text.font, newCmd->text.fontSize, newCmd->text.fontStyle)));
					newCmd->text.text = element->config.text;
				}
			}
		}
	}
	
	//NOTE: We used to sort but depth only, but we actually want to maintain ordering amongst same-depth elements so we sort by cmd->elementIndex secondarily
	QuickSortVarArray(UiRenderCmd, &UiCtx->renderList.commands, UiRenderCmd_DepthAndHierarchySortCompare, nullptr);
	
	return &UiCtx->renderList;
}

PEXP void EndUiFrame()
{
	NotNull(UiCtx);
	AssertUiThreadIsSame();
	
	ClearStruct(UiCtx->renderList); //Memset to zero since it was allocated from frameArena and will get freed in the ResetToMark below
	
	FlagUnset(UiCtx->frameArena->flags, ArenaFlag_DontPop);
	ArenaResetToMark(UiCtx->frameArena, UiCtx->frameArenaMark);
	UiCtx->frameArena = nullptr;
	
	UiCtx->keyboard = nullptr;
	UiCtx->mouse = nullptr;
	UiCtx->touchscreen = nullptr;
	
	UiCtx = nullptr;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_PIG_UI

#endif //  _UI_SYSTEM_H
 