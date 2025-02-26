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

#if BUILD_WITH_CLAY

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

#define CLAY_UI_MEASURE_TEXT_DEF(functionName) Clay_Dimensions functionName(Clay_StringSlice text, Clay_TextElementConfig* config, void* userData)
typedef CLAY_UI_MEASURE_TEXT_DEF(ClayUiMeasureText_f);

//TODO: We may want to make sure that ClayUI is always accessed from the same thread it was created on!
typedef struct ClayUI ClayUI;
struct ClayUI
{
	Arena* arena;
	Clay_Arena clayArena;
	ClayUiMeasureText_f* measureTextFunc;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE v2 ToV2FromClay(Clay_Vector2 clayVec);
	PIG_CORE_INLINE Clay_Vector2 ToClayVector2(v2 vector);
	PIG_CORE_INLINE v2 ToV2iFromClay(Clay_Dimensions clayDimensions);
	PIG_CORE_INLINE Clay_Dimensions ToClayDimensions(v2 vector);
	void InitClayUI(Arena* arena, v2 windowSize, ClayUiMeasureText_f* measureTextFunc, void* measureUserData, ClayUI* clayOut);
	PIG_CORE_INLINE void BeginClayUIRender(ClayUI* clay, v2 windowSize, r32 elapsedMs, bool isMouseOverOther, v2 mousePos, bool isMouseDown, v2 mouseScrollDelta);
	PIG_CORE_INLINE Clay_RenderCommandArray EndClayUIRender(ClayUI* clay);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

static void ClayErrorCallback(Clay_ErrorData errorData)
{
	PrintLine_E("Clay Error: %s", errorData.errorText.chars);
	//TODO: Implement me better!
}

PEXPI v2 ToV2FromClay(Clay_Vector2 clayVec) { return NewV2(clayVec.x, clayVec.y); }
PEXPI Clay_Vector2 ToClayVector2(v2 vector) { return (Clay_Vector2){ .x = vector.X, .y = vector.Y }; }

PEXPI v2 ToV2iFromClay(Clay_Dimensions clayDimensions) { return NewV2(clayDimensions.width, clayDimensions.height); }
PEXPI Clay_Dimensions ToClayDimensions(v2 vector) { return (Clay_Dimensions){ .width = vector.Width, .height = vector.Height }; }

PEXP void InitClayUI(Arena* arena, v2 windowSize, ClayUiMeasureText_f* measureTextFunc, void* measureUserData, ClayUI* clayOut)
{
	NotNull(measureTextFunc);
	NotNull(clayOut);
	ClearPointer(clayOut);
	clayOut->arena = arena;
	clayOut->measureTextFunc = measureTextFunc;
	
	u32 minMemory = Clay_MinMemorySize();
	Assert(minMemory > 0);
	u8* clayMemory = (u8*)AllocMem(arena, minMemory);
	NotNull(clayMemory);
	clayOut->clayArena = Clay_CreateArenaWithCapacityAndMemory(minMemory, clayMemory);
	
	Clay_Initialize(clayOut->clayArena, ToClayDimensions(windowSize), (Clay_ErrorHandler){ ClayErrorCallback });
	
	Clay_SetMeasureTextFunction(measureTextFunc, measureUserData);
}

PEXPI void BeginClayUIRender(ClayUI* clay, v2 windowSize, r32 elapsedMs, bool isMouseOverOther, v2 mousePos, bool isMouseDown, v2 mouseScrollDelta)
{
	NotNull(clay);
	NotNull(clay->arena);
	Clay_SetLayoutDimensions(ToClayDimensions(windowSize));
	if (isMouseOverOther)
	{
		Clay_SetPointerState(ToClayVector2(NewV2(-1, -1)), false);
	}
	else
	{
		Clay_SetPointerState(ToClayVector2(mousePos), isMouseDown);
		Clay_UpdateScrollContainers(false, ToClayVector2(mouseScrollDelta), elapsedMs / 1000.0f);
	}
	Clay_BeginLayout();
}

PEXPI Clay_RenderCommandArray EndClayUIRender(ClayUI* clay)
{
	UNUSED(clay);
	return Clay_EndLayout();
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_CLAY

#endif //  _UI_CLAY_H

#if defined(_STRUCT_RECTANGLES_H) && defined(_UI_CLAY_H)
#include "cross/cross_rectangles_and_clay.h"
#endif

#if defined(_STRUCT_COLOR_H) && defined(_UI_CLAY_H)
#include "cross/cross_color_and_clay.h"
#endif

#if defined(_STRUCT_STRING_H) && defined(_UI_CLAY_H)
#include "cross/cross_string_and_clay.h"
#endif

#if defined(_STRUCT_STRING_H) && defined(_MEM_SCRATCH_H) && defined(_UI_CLAY_H)
#include "cross/cross_string_scratch_and_clay.h"
#endif
