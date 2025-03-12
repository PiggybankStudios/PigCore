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

typedef struct ClayElementUserData ClayElementUserData;
struct ClayElementUserData
{
	int placeholder; //TODO: Add extensions here!
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

typedef struct ClayTextUserData ClayTextUserData;
struct ClayTextUserData
{
	TextContraction contraction;
};

#define CLAY_ELEMENT_USERDATA_TYPE ClayElementUserData
#define CLAY_TEXT_USERDATA_TYPE ClayTextUserData
#define CLAY_IMAGEDATA_TYPE struct Texture*

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
	Clay_Context* context;
};

typedef Clay_ElementId ClayId;

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE v2 ToV2FromClay(Clay_Vector2 clayVec);
	PIG_CORE_INLINE Clay_Vector2 ToClayVector2(v2 vector);
	PIG_CORE_INLINE v2 ToV2iFromClay(Clay_Dimensions clayDimensions);
	PIG_CORE_INLINE Clay_Dimensions ToClayDimensions(v2 vector);
	PIG_CORE_INLINE rec ToRecFromClay(Clay_BoundingBox clayBoundingBox);
	PIG_CORE_INLINE Clay_BoundingBox ToClayBoundingBox(rec rectangle);
	PIG_CORE_INLINE Str8 ToStrFromClay(Clay_String clayString);
	PIG_CORE_INLINE Clay_String ToClayString(Str8 str);
	PIG_CORE_INLINE ClayId ToClayIdEx(Str8 idStr, uxx index);
	PIG_CORE_INLINE ClayId ToClayId(Str8 idStr);
	PIG_CORE_INLINE ClayId ToClayIdNt(const char* idNullTermString);
	PIG_CORE_INLINE Color32 ToColorFromClay(Clay_Color clayColor);
	PIG_CORE_INLINE Clay_Color ToClayColor(Color32 color);
	void SetClayContext(ClayUI* clay);
	void InitClayUI(Arena* arena, v2 windowSize, ClayUiMeasureText_f* measureTextFunc, void* measureUserData, ClayUI* clayOut);
	PIG_CORE_INLINE bool UpdateClayScrolling(ClayUI* clay, r32 elapsedMs, bool isMouseOverOther, v2 mouseScrollDelta, bool allowTouchScrolling);
	PIG_CORE_INLINE void BeginClayUIRender(ClayUI* clay, v2 windowSize, bool isMouseOverOther, v2 mousePos, bool isMouseDown);
	PIG_CORE_INLINE Clay_RenderCommandArray EndClayUIRender(ClayUI* clay);
	PIG_CORE_INLINE rec GetClayElementDrawRec(ClayId elementId);
	PIG_CORE_INLINE rec GetClayElementDrawRecStr(Str8 elementIdStr);
	PIG_CORE_INLINE rec GetClayElementDrawRecNt(const char* elementIdStrNt);
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

// +--------------------------------------------------------------+
// |                       Type Conversions                       |
// +--------------------------------------------------------------+
PEXPI v2 ToV2FromClay(Clay_Vector2 clayVec) { return NewV2(clayVec.x, clayVec.y); }
PEXPI Clay_Vector2 ToClayVector2(v2 vector) { return (Clay_Vector2){ .x = vector.X, .y = vector.Y }; }

PEXPI v2 ToV2iFromClay(Clay_Dimensions clayDimensions) { return NewV2(clayDimensions.width, clayDimensions.height); }
PEXPI Clay_Dimensions ToClayDimensions(v2 vector) { return (Clay_Dimensions){ .width = vector.Width, .height = vector.Height }; }

PEXPI rec ToRecFromClay(Clay_BoundingBox clayBoundingBox) { return NewRec(clayBoundingBox.x, clayBoundingBox.y, clayBoundingBox.width, clayBoundingBox.height); }
PEXPI Clay_BoundingBox ToClayBoundingBox(rec rectangle) { return (Clay_BoundingBox){ .x = rectangle.X, .y = rectangle.Y, .width = rectangle.Width, .height = rectangle.Height }; }

PEXPI Str8 ToStrFromClay(Clay_String clayString) { return NewStr8((uxx)clayString.length, clayString.chars); }
PEXPI Clay_String ToClayString(Str8 str) { return (Clay_String){ .length = (int32_t)str.length, .chars = str.chars }; }
PEXPI ClayId ToClayIdEx(Str8 idStr, uxx index) { Assert(index <= UINT32_MAX); return Clay__HashString(ToClayString(idStr), (uint32_t)index, 0); }
PEXPI ClayId ToClayId(Str8 idStr) { return ToClayIdEx(idStr, 0); }
PEXPI ClayId ToClayIdNt(const char* idNullTermString) { return ToClayId(StrLit(idNullTermString)); }

PEXPI Color32 ToColorFromClay(Clay_Color clayColor) { return ToColor32FromV4(NewV4(clayColor.r/255.0f, clayColor.g/255.0f, clayColor.b/255.0f, clayColor.a/255.0f)); }
PEXPI Clay_Color ToClayColor(Color32 color) { v4 colorVec = ToV4FromColor32(color); return (Clay_Color){ .r = colorVec.R*255.0f, .g = colorVec.G*255.0f, .b = colorVec.B*255.0f, .a = colorVec.A*255.0f }; }

// +--------------------------------------------------------------+
// |                   Initialize and Begin/End                   |
// +--------------------------------------------------------------+
PEXPI void SetClayContext(ClayUI* clay)
{
	NotNull(clay);
	NotNull(clay->context);
	Clay_SetCurrentContext(clay->context);
}

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
	
	clayOut->context = Clay_Initialize(clayOut->clayArena, ToClayDimensions(windowSize), (Clay_ErrorHandler){ ClayErrorCallback });
	
	Clay_SetMeasureTextFunction(measureTextFunc, measureUserData);
}

PEXPI bool UpdateClayScrolling(ClayUI* clay, r32 elapsedMs, bool isMouseOverOther, v2 mouseScrollDelta, bool allowTouchScrolling)
{
	SetClayContext(clay);
	bool isMomentumScrolling = Clay_UpdateScrollContainers(allowTouchScrolling, ToClayVector2(isMouseOverOther ? V2_Zero : mouseScrollDelta), elapsedMs / 1000.0f);
	return isMomentumScrolling;
	
}

PEXPI void BeginClayUIRender(ClayUI* clay, v2 windowSize, bool isMouseOverOther, v2 mousePos, bool isMouseDown)
{
	SetClayContext(clay);
	Clay_SetLayoutDimensions(ToClayDimensions(windowSize));
	if (isMouseOverOther)
	{
		Clay_SetPointerState(ToClayVector2(NewV2(-1, -1)), false);
	}
	else
	{
		Clay_SetPointerState(ToClayVector2(mousePos), isMouseDown);
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
	return (elementData.found ? ToRecFromClay(elementData.boundingBox) : Rec_Zero);
}
PEXPI rec GetClayElementDrawRecStr(Str8 elementIdStr) { return GetClayElementDrawRec(ToClayId(elementIdStr)); }
PEXPI rec GetClayElementDrawRecNt(const char* elementIdStrNt) { return GetClayElementDrawRec(ToClayIdNt(elementIdStrNt)); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_CLAY

#endif //  _UI_CLAY_H

#if defined(_MEM_SCRATCH_H) && defined(_UI_CLAY_H)
#include "cross/cross_scratch_and_clay.h"
#endif
