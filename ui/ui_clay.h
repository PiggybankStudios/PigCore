/*
File:   ui_clay.h
Author: Taylor Robbins
Date:   02\16\2025
Description:
	** Holds header or implementation #include for clay.h and some extensions to it's API
	** that use our own types or paradigms
*/

#ifndef _UI_CLAY_H
#define _UI_CLAY_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "base/base_debug_output.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
#include "struct/struct_vectors.h"
#include "struct/struct_rectangles.h"
#include "struct/struct_string.h"
#include "struct/struct_color.h"

#if BUILD_WITH_CLAY

typedef plex ClayElementUserData ClayElementUserData;
plex ClayElementUserData
{
	bool outsideBorder;
};

typedef enum TextContraction TextContraction;
enum TextContraction
{
	TextContraction_ClipRight = 0,
	TextContraction_ClipLeft,
	TextContraction_EllipseRight,
	TextContraction_EllipseMiddle,
	TextContraction_EllipseLeft,
	TextContraction_EllipseFilePath,
	TextContraction_Count,
};
const char* GetTextContractionStr(TextContraction enumValue)
{
	switch (enumValue)
	{
		case TextContraction_ClipRight:       return "ClipRight";
		case TextContraction_ClipLeft:        return "ClipLeft";
		case TextContraction_EllipseRight:    return "EllipseRight";
		case TextContraction_EllipseMiddle:   return "EllipseMiddle";
		case TextContraction_EllipseLeft:     return "EllipseLeft";
		case TextContraction_EllipseFilePath: return "EllipseFilePath";
		default: return UNKNOWN_STR;
	}
}

typedef plex ClayTextUserData ClayTextUserData;
plex ClayTextUserData
{
	TextContraction contraction;
	plex FontFlow* flowTarget;
	Color32 backgroundColor; //used for text that has selection
	bool richText;
	r32 wrapWidth; //Using this wrapWidth bypasses Clay's word wrapping behavior so it should only be used when the parent container has already been vertically sized based off previous MeasureText call with wrapWidth set
};

#define CLAY_ELEMENT_USERDATA_TYPE ClayElementUserData
#define CLAY_TEXT_USERDATA_TYPE ClayTextUserData
#define CLAY_IMAGEDATA_TYPE plex Texture*

#if PIG_CORE_IMPLEMENTATION
#define CLAY_IMPLEMENTATION
#define CLAY_DECOR PEXP
#else
#define CLAY_DECOR //nothing
#endif
#if COMPILER_IS_MSVC
#pragma warning(push)
#pragma warning(disable: 4244) //conversion from 'int' to 'float', possible loss of data
#endif
#include "third_party/clay/clay.h"
#if COMPILER_IS_MSVC
#pragma warning(pop)
#endif

//TODO: We may want to make sure that ClayUI is always accessed from the same thread it was created on!
typedef plex ClayUI ClayUI;
plex ClayUI
{
	Arena* arena;
	ClayMeasureText_f* measureTextFunc;
	Clay_Context* context;
};

typedef Clay_ElementId ClayId;

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE ClayId ToClayIdEx(Str8 idStr, uxx index);
	PIG_CORE_INLINE ClayId ToClayId(Str8 idStr);
	PIG_CORE_INLINE ClayId ToClayIdNt(const char* idNullTermString);
	void SetClayContext(ClayUI* clay);
	void InitClayUI(Arena* arena, v2 windowSize, ClayMeasureText_f* measureTextFunc, void* measureUserData, ClayUI* clayOut);
	PIG_CORE_INLINE bool UpdateClayScrolling(ClayUI* clay, r32 elapsedMs, bool isMouseOverOther, v2 mouseScrollDelta, bool allowTouchScrolling);
	PIG_CORE_INLINE void BeginClayUIRender(ClayUI* clay, v2 windowSize, bool isMouseOverOther, v2 mousePos, bool isMouseDown);
	PIG_CORE_INLINE Clay_RenderCommandArray EndClayUIRender(ClayUI* clay);
	PIG_CORE_INLINE rec GetClayElementDrawRec(ClayId elementId);
	PIG_CORE_INLINE rec GetClayElementDrawRecStr(Str8 elementIdStr);
	PIG_CORE_INLINE rec GetClayElementDrawRecNt(const char* elementIdStrNt);
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
// These macros are really shorthand for multiplying by app->uiScale and dealing with how to round or clamp to good values when uiScale is very big/small
#define UISCALE_R32(scale, pixels) RoundR32((r32)(pixels) * (scale))
#define UISCALE_U16(scale, pixels) (u16)RoundR32i((r32)(pixels) * (scale))
#define UISCALE_BORDER(scale, pixels) MaxU16(((pixels) > 0) ? (u16)1 : (u16)0, (u16)RoundR32i((r32)(pixels) * (scale)))

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

static void ClayErrorCallback(Clay_ErrorData errorData)
{
	PrintLine_E("Clay Error: %s", errorData.errorText.chars);
	//TODO: Implement me better!
}

// +--------------------------------------------------------------+
// |                       Type Conversions                       |
// +--------------------------------------------------------------+
PEXPI ClayId ToClayIdEx(Str8 idStr, uxx index) { Assert(index <= UINT32_MAX); return Clay__HashString(idStr, (uint32_t)index, 0); }
PEXPI ClayId ToClayId(Str8 idStr) { return ToClayIdEx(idStr, 0); }
PEXPI ClayId ToClayIdNt(const char* idNullTermString) { return ToClayId(StrLit(idNullTermString)); }

// +--------------------------------------------------------------+
// |                   Initialize and Begin/End                   |
// +--------------------------------------------------------------+
PEXPI void SetClayContext(ClayUI* clay)
{
	NotNull(clay);
	NotNull(clay->context);
	Clay_SetCurrentContext(clay->context);
}

PEXP void InitClayUI(Arena* arena, v2 windowSize, ClayMeasureText_f* measureTextFunc, void* measureUserData, ClayUI* clayOut)
{
	NotNull(measureTextFunc);
	NotNull(clayOut);
	ClearPointer(clayOut);
	clayOut->arena = arena;
	clayOut->measureTextFunc = measureTextFunc;
	
	clayOut->context = Clay_Initialize(arena, windowSize, (Clay_ErrorHandler){ .errorHandlerFunction=ClayErrorCallback });
	
	Clay_SetMeasureTextFunction(measureTextFunc, measureUserData);
}

PEXPI bool UpdateClayScrolling(ClayUI* clay, r32 elapsedMs, bool isMouseOverOther, v2 mouseScrollDelta, bool allowTouchScrolling)
{
	SetClayContext(clay);
	bool isMomentumScrolling = Clay_UpdateScrollContainers(allowTouchScrolling, (isMouseOverOther ? V2_Zero : mouseScrollDelta), elapsedMs / 1000.0f);
	return isMomentumScrolling;
	
}

PEXPI void BeginClayUIRender(ClayUI* clay, v2 windowSize, bool isMouseOverOther, v2 mousePos, bool isMouseDown)
{
	SetClayContext(clay);
	Clay_SetLayoutDimensions(windowSize);
	if (isMouseOverOther)
	{
		Clay_SetPointerState(NewV2(-1, -1), false);
	}
	else
	{
		Clay_SetPointerState(mousePos, isMouseDown);
	}
	Clay_BeginLayout();
}

PEXPI Clay_RenderCommandArray EndClayUIRender(ClayUI* clay)
{
	SetClayContext(clay);
	return Clay_EndLayout();
}

// +--------------------------------------------------------------+
// |                           Helpers                            |
// +--------------------------------------------------------------+
PEXPI rec GetClayElementDrawRec(ClayId elementId)
{
	Clay_ElementData elementData = Clay_GetElementData(elementId);
	return (elementData.found ? elementData.boundingBox : Rec_Zero);
}
PEXPI rec GetClayElementDrawRecStr(Str8 elementIdStr) { return GetClayElementDrawRec(ToClayId(elementIdStr)); }
PEXPI rec GetClayElementDrawRecNt(const char* elementIdStrNt) { return GetClayElementDrawRec(ToClayIdNt(elementIdStrNt)); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_CLAY

#endif //  _UI_CLAY_H

#if defined(_MEM_SCRATCH_H) && defined(_UI_CLAY_H)
#include "cross/cross_scratch_and_clay.h"
#endif
