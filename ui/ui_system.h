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

#if BUILD_WITH_PIG_UI

#define PIG_UI_INDEX_INVALID UINTXX_MAX
#define PIG_UI_ID_INDEX_NONE UINTXX_MAX
#define PIG_UI_DEFAULT_ID_STR "elem"

typedef plex UiId UiId;
plex UiId
{
	uxx id;
	//We copy the string/index the Id was sourced from for 2 reasons
	// 1. In OpenUiElement we need to calculate the "real" ID which is a mix of parent element's ID and the new element's string+index
	// 2. For debug purposes we copy this into the frameArena and keep it in the config for the element so we can print out or display the ID's
	Str8 str;
	uxx index;
};

typedef enum UiLayoutDir UiLayoutDir;
enum UiLayoutDir
{
	UiLayoutDir_TopDown = 0,
	UiLayoutDir_BottomUp,
	UiLayoutDir_LeftToRight,
	UiLayoutDir_RightToLeft,
	UiLayoutDir_Count,
	UiLayoutDir_Default = UiLayoutDir_TopDown,
};
#if !PIG_CORE_IMPLEMENTATION
PIG_CORE_INLINE const char* GetUiLayoutDirStr(UiLayoutDir enumValue);
#else
PEXPI const char* GetUiLayoutDirStr(UiLayoutDir enumValue)
{
	switch (enumValue)
	{
		case UiLayoutDir_TopDown:     return "TopDown(Default)";
		case UiLayoutDir_BottomUp:    return "BottomUp";
		case UiLayoutDir_LeftToRight: return "LeftToRight";
		case UiLayoutDir_RightToLeft: return "RightToLeft";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef enum UiSizingType UiSizingType;
enum UiSizingType
{
	UiSizingType_Expand = 0,
	UiSizingType_FixedPx,
	UiSizingType_FixedPercent,
	UiSizingType_Fit,
	UiSizingType_Count,
	UiSizingType_Default = UiSizingType_Expand,
};
#if !PIG_CORE_IMPLEMENTATION
PIG_CORE_INLINE const char* GetUiSizingTypeStr(UiSizingType enumValue);
#else
PEXPI const char* GetUiSizingTypeStr(UiSizingType enumValue)
{
	switch (enumValue)
	{
		case UiSizingType_Expand:       return "Expand(Default)";
		case UiSizingType_FixedPx:      return "FixedPx";
		case UiSizingType_FixedPercent: return "FixedPercent";
		case UiSizingType_Fit:          return "Fit";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef plex UiSizingAxis UiSizingAxis;
plex UiSizingAxis
{
	UiSizingType type;
	r32 value;
};

typedef car UiSizing UiSizing;
car UiSizing
{
	UiSizingAxis axis[2];
	plex { UiSizingAxis x, y; };
	plex { UiSizingAxis horiztonal, vertical; };
};

// +==============================+
// |        Element Config        |
// +==============================+
typedef plex UiElemConfig UiElemConfig;
plex UiElemConfig
{
	UiId id;
	bool globalId;
	UiLayoutDir direction;
	UiSizing sizing;
	Color32 color;
	v4r margins; //space between our bounds and our childrens' bounds
	v4r padding; //space between allocated area and our bounds
	r32 childPadding; //space in-between each child, along the layout direction
	v4r borderThickness;
	Color32 borderColor;
};

typedef plex UiElement UiElement;
plex UiElement
{
	UiElemConfig config;
	UiId id; //This is the "real" ID, the one in config actually gets recalculated in OpenUiElement based on the parent element's ID
	
	uxx depth; //How many parents this element has
	uxx elementIndex; //What's this element's index in the UiContext->elements VarArray
	uxx siblingIndex; //which index child is this element amongst it's siblings
	uxx parentIndex; //What is the index of the parent element in the UiContext>elements VarArray
	UiId parentId;
	bool isOpen; //are we currently adding children to this element
	bool runChildCode;
	uxx numChildren; //How many direct children does this element have
	uxx numDescendents; //How many elements after this one are a descendent of this element (children, grandchildren, etc. Useful for knowing how many elements to skip over if we don't want to walk the tree of elements below this one)
	
	v2 minimumSize;
	v2 preferredSize;
	rec layoutRec;
};

typedef enum UiRenderCmdType UiRenderCmdType;
enum UiRenderCmdType
{
	UiRenderCmdType_None = 0,
	UiRenderCmdType_Rectangle,
	UiRenderCmdType_Text,
	UiRenderCmdType_Scissor,
	UiRenderCmdType_Count,
};
#if !PIG_CORE_IMPLEMENTATION
PIG_CORE_INLINE const char* GetUiRenderCmdTypeStr(UiRenderCmdType enumValue);
#else
PEXPI const char* GetUiRenderCmdTypeStr(UiRenderCmdType enumValue)
{
	switch (enumValue)
	{
		case UiRenderCmdType_None:      return "None";
		case UiRenderCmdType_Rectangle: return "Rectangle";
		case UiRenderCmdType_Text:      return "Text";
		case UiRenderCmdType_Scissor:   return "Scissor";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef plex UiRenderCmd UiRenderCmd;
plex UiRenderCmd
{
	UiRenderCmdType type;
	car
	{
		// +==============================+
		// |  UiRenderCmdType_Rectangle   |
		// +==============================+
		plex
		{
			rec rectangle;
			Color32 color;
			v4r cornerRadius;
			v4r borderThickness;
			Color32 borderColor;
			Texture* texture;
		} rectangle;
		
		// +==============================+
		// |     UiRenderCmdType_Text     |
		// +==============================+
		plex
		{
			v2 position;
			PigFont* font;
			Str8 text;
			Color32 color;
		} text;
		
		// +==============================+
		// |   UiRenderCmdType_Scissor    |
		// +==============================+
		plex
		{
			bool start;
			rec rectangle;
		} scissor;
	};
};

typedef plex UiRenderList UiRenderList;
plex UiRenderList
{
	Arena* arena;
	plex UiContext* context;
	VarArray commands; //UiRenderCmd
};

typedef plex UiContext UiContext;
plex UiContext
{
	Arena* arena;
	
	Arena* frameArena;
	uxx frameArenaMark;
	#if (TARGET_HAS_THREADING && DEBUG_BUILD)
	ThreadId threadId;
	#endif
	v2 screenSize; //TODO: Remove me
	r32 scale;
	u64 programTime;
	KeyboardState* keyboard;
	MouseState* mouse;
	TouchscreenState* touchscreen;
	
	uxx currentElementIndex;
	uxx numTopLevelElements; //TODO: Remove me
	VarArray elements; //UiElement
	
	UiRenderList renderList; //allocated from frameArena
};

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
	PIG_CORE_INLINE UiId CalcUiId(UiId baseId, Str8 idString, uxx index);
	PIG_CORE_INLINE UiId PrintUiId(UiId baseId, uxx index, const char* formatString, ...);
	void InitUiContext(Arena* arena, UiContext* contextOut);
	PIG_CORE_INLINE UiElement* GetUiElementParent(UiElement* element, uxx ancestorIndex);
	PIG_CORE_INLINE UiElement* GetUiElementChild(UiElement* element, uxx childIndex);
	PIG_CORE_INLINE UiElement* OpenUiElement(UiElemConfig config);
	PIG_CORE_INLINE bool OpenUiElementConditional(UiElemConfig config);
	void StartUiFrame(UiContext* context, v2 screenSize, r32 scale, u64 programTime, KeyboardState* keyboard, MouseState* mouse, TouchscreenState* touchscreen);
	PIG_CORE_INLINE UiElement* CloseUiElement();
	PIG_CORE_INLINE Str8 GetUiElementQualifiedName(Arena* arena, UiElement* element);
	UiRenderList* GetUiRenderList();
	void EndUiFrame();
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define MakeUiId_Const(idValue, strValue, indexValue) { .id=(idValue), .str=(strValue), .index=(indexValue) }
#define MakeUiId(id, str, index)                      NEW_STRUCT(UiId)MakeUiId_Const((id), (str), (index))

#define UiId_None       MakeUiId(0, Str8_Empty, 0)
#define UiId_None_Const MakeUiId_Const(0, Str8_Empty_Const, 0)

//This is the most convenient way to give a UiId from a string literal, like UiIdNt("Element1")
#define UiIdLit(idStrLit)             CalcUiId(UiId_None, StrLit(idStrLit),    PIG_UI_ID_INDEX_NONE)
#define UiIdNt(idStrNt)               CalcUiId(UiId_None, MakeStr8Nt(idStrNt), PIG_UI_ID_INDEX_NONE)
#define UiIdStr(idStr)                CalcUiId(UiId_None, (idStr),             PIG_UI_ID_INDEX_NONE)
#define UiIdLitIndex(idStrLit, index) CalcUiId(UiId_None, StrLit(idStrLit),    (index))
#define UiIdNtIndex(idStrNt, index)   CalcUiId(UiId_None, MakeStr8Nt(idStrNt), (index))
#define UiIdStrIndex(idStr, index)    CalcUiId(UiId_None, (idStr),             (index))

#define UiIdPrint(formatString, ...)              PrintUiId(UiId_None, PIG_UI_ID_INDEX_NONE, (formatString),     ##__VA_ARGS__)
#define UiIdPrintIndex(index, formatString, ...)  PrintUiId(UiId_None, (index),              (formatString),     ##__VA_ARGS__)

//NOTE: In order to get the UIELEM macro to work properly, we need to surround the __VA_ARGS__ in an extra set of curly brackets
//      We can make this valid by defining a Wrapper struct that the curly brackets construct and then just access the config element inside
typedef plex UiElemConfigWrapper UiElemConfigWrapper;
plex UiElemConfigWrapper { UiElemConfig config; };

//TODO: Write a description of this macro and it's need
//NOTE: The preprocessor is going to treat the commas inside the designated initializer list as argument separators.
//      For this reasons we do a variadic argument macro and use __VA_ARGS__ as a way to pass all of the designated initializer through
#define UIELEM(...) DeferIfBlock(OpenUiElementConditional(NEW_STRUCT(UiElemConfigWrapper){ __VA_ARGS__ }.config), CloseUiElement())

#define UI_FIXED(numPx)                 { .type=UiSizingType_FixedPx,      .value=(numPx)   }
#define UI_PERCENT(percent)             { .type=UiSizingType_FixedPercent, .value=(percent) }
#define UI_FIT()                        { .type=UiSizingType_Fit,          .value=0         }
#define UI_EXPAND()                     { .type=UiSizingType_Expand,       .value=0         }
#define UI_EXPAND_MIN(minPx)            { .type=UiSizingType_Expand,       .value=(minPx)   }
#define UI_FIXED2(numPxX, numPxY)       { .x=UI_FIXED(numPxX),     .y=UI_FIXED(numPxY)     }
#define UI_PERCENT2(percentX, percentY) { .x=UI_PERCENT(percentX), .y=UI_PERCENT(percentY) }
#define UI_FIT2()                       { .x=UI_FIT(),             .y=UI_FIT()             }
#define UI_EXPAND2()                    { .x=UI_EXPAND(),          .y=UI_EXPAND()          }
#define UI_EXPAND_MIN2(minPxX, minPxY)  { .x=UI_EXPAND(minPxX),    .y=UI_EXPAND(minPxY)    }

#define IsUiDirHorizontal(direction) ( (direction) == UiLayoutDir_RightToLeft || (direction) == UiLayoutDir_LeftToRight )
#define IsUiDirVertical(direction)   ( (direction) == UiLayoutDir_TopDown     || (direction) == UiLayoutDir_BottomUp    )

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
	if (index != PIG_UI_ID_INDEX_NONE)
	{
		result.id = FnvHashU64Ex(&index, sizeof(index), result.id);
		// PrintLine_D("FNV(\"%.*s\"[%llu]) = 0x%02X%02X%02X%02X%02X%02X%02X%02X",
		// 	StrPrint(idString),
		// 	index,
		// 	((u8*)&result.id)[0],
		// 	((u8*)&result.id)[1],
		// 	((u8*)&result.id)[2],
		// 	((u8*)&result.id)[3],
		// 	((u8*)&result.id)[4],
		// 	((u8*)&result.id)[5],
		// 	((u8*)&result.id)[6],
		// 	((u8*)&result.id)[7]
		// );
	}
	else
	{
		// PrintLine_D("FNV(\"%.*s\") = 0x%02X%02X%02X%02X%02X%02X%02X%02X",
		// 	StrPrint(idString),
		// 	((u8*)&result.id)[0],
		// 	((u8*)&result.id)[1],
		// 	((u8*)&result.id)[2],
		// 	((u8*)&result.id)[3],
		// 	((u8*)&result.id)[4],
		// 	((u8*)&result.id)[5],
		// 	((u8*)&result.id)[6],
		// 	((u8*)&result.id)[7]
		// );
	}
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

PEXP void InitUiContext(Arena* arena, UiContext* contextOut)
{
	NotNull(arena);
	NotNull(contextOut);
	ClearPointer(contextOut);
	contextOut->arena = arena;
	InitVarArray(UiElement, &contextOut->elements, arena);
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

//NOTE: This pointer becomes potentially invalid once OpenUiElement is called again, VarArrayAdd semantics
PEXPI UiElement* OpenUiElement(UiElemConfig config)
{
	NotNull(UiCtx);
	AssertUiThreadIsSame();
	
	UiElement* newElement = VarArrayAdd(UiElement, &UiCtx->elements);
	NotNull(newElement);
	ClearPointer(newElement);
	newElement->elementIndex = (UiCtx->elements.length - 1);
	newElement->parentIndex = UiCtx->currentElementIndex;
	UiElement* parentElement = GetUiElementParent(newElement, 0);
	if (parentElement != nullptr)
	{
		DebugAssert(parentElement->isOpen);
		if (IsEmptyStr(config.id.str)) { config.id = UiIdLitIndex(PIG_UI_DEFAULT_ID_STR, parentElement->numChildren); }
		newElement->id = CalcUiId(parentElement->id, config.id.str, config.id.index);
		newElement->parentId = parentElement->config.id;
		newElement->depth = (parentElement->depth + 1);
		newElement->siblingIndex = parentElement->numChildren;
		parentElement->numChildren++;
		while (parentElement != nullptr)
		{
			parentElement->numDescendents++;
			parentElement = VarArrayGetSoft(UiElement, &UiCtx->elements, parentElement->parentIndex);
		}
	}
	else
	{
		if (IsEmptyStr(config.id.str)) { config.id = UiIdLitIndex(PIG_UI_DEFAULT_ID_STR, UiCtx->numTopLevelElements); }
		newElement->id = config.id; //Top-level IDs are not modified by any base ID
		newElement->depth = 0;
		newElement->siblingIndex = UiCtx->numTopLevelElements;
		UiCtx->numTopLevelElements++;
	}
	UiCtx->currentElementIndex = newElement->elementIndex;
	MyMemCopy(&newElement->config, &config, sizeof(UiElemConfig));
	newElement->isOpen = true;
	
	//TODO: How do we decie runChildCode for things like buttons?
	newElement->runChildCode = true;
	
	// if (newElement->parentIndex == 0) { MyBreak(); }
	return newElement;
}
PEXPI bool OpenUiElementConditional(UiElemConfig config) { return OpenUiElement(config)->runChildCode; }

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
	context->scale = scale;
	context->programTime = programTime;
	context->keyboard = keyboard;
	context->mouse = mouse;
	context->touchscreen = touchscreen;
	
	//TODO: Copy the array from last frame somewhere?
	VarArrayClear(&context->elements);
	context->numTopLevelElements = 0;
	context->currentElementIndex = PIG_UI_INDEX_INVALID;
	
	Assert(UiCtx == nullptr);
	UiCtx = context;
	
	UiElement* rootElement = OpenUiElement(NEW_STRUCT(UiElemConfig){
		.id = UiIdLit("root"),
		.sizing = UI_FIXED2(screenSize.Width, screenSize.Height),
		.color = backgroundColor,
	});
	UNUSED(rootElement);
}

static void CalculateUiElemSizeOnAxisOnClose(UiElement* element, UiElement* parent, bool xAxis)
{
	DebugNotNull(element);
	DebugAssert(parent != nullptr || element->elementIndex == 0); //parent can be null, only for the root element
	bool isThisLayoutDir = (IsUiDirHorizontal(element->config.direction) == xAxis);
	r32 layoutAxisChildPadding = isThisLayoutDir ? ((r32)(element->numChildren > 1 ? element->numChildren-1 : 0) * element->config.childPadding) : 0.0f;
	r32 elemMargins = (xAxis ? (element->config.margins.Left + element->config.margins.Right) : (element->config.margins.Top + element->config.margins.Bottom));
	
	r32* minimumSizePntr = (xAxis ? &element->minimumSize.Width : &element->minimumSize.Height);
	r32* preferredSizePntr = (xAxis ? &element->preferredSize.Width : &element->preferredSize.Height);
	
	UiSizingType sizingType = (xAxis ? element->config.sizing.x.type : element->config.sizing.y.type);
	r32 sizingValue = (xAxis ? element->config.sizing.x.value : element->config.sizing.y.value);
	if (sizingType == UiSizingType_FixedPx)
	{
		*minimumSizePntr = sizingValue;
		*preferredSizePntr = sizingValue;
	}
	//TODO: Handle UiSizingType_FixedPercent
	else if (sizingType == UiSizingType_Fit)
	{
		*minimumSizePntr += elemMargins + layoutAxisChildPadding;
		if (!IsInfiniteOrNanR32(*preferredSizePntr)) { *preferredSizePntr += elemMargins + layoutAxisChildPadding; }
	}
	else if (sizingType == UiSizingType_Expand)
	{
		*minimumSizePntr += sizingValue + elemMargins + layoutAxisChildPadding;
		*preferredSizePntr = INFINITY;
	}
	else { DebugAssert(false); }
	
	if (parent != nullptr)
	{
		bool isParentLayoutDir = (IsUiDirHorizontal(parent->config.direction) == xAxis);
		r32 paddingLrOrTb = (xAxis ? (element->config.padding.Left + element->config.padding.Right) : (element->config.padding.Top + element->config.padding.Bottom));
		r32* parentMinimumSizePntr = (xAxis ? &parent->minimumSize.Width : &parent->minimumSize.Height);
		r32* parentPreferredSizePntr = (xAxis ? &parent->preferredSize.Width : &parent->preferredSize.Height);
		
		if (isParentLayoutDir) { *parentMinimumSizePntr += *minimumSizePntr + paddingLrOrTb; }
		else { *parentMinimumSizePntr = MaxR32(*parentMinimumSizePntr, *minimumSizePntr + paddingLrOrTb); }
		
		if (!IsInfiniteOrNanR32(*parentPreferredSizePntr))
		{
			if (IsInfiniteOrNanR32(*preferredSizePntr)) { *parentPreferredSizePntr = INFINITY; }
			else if (isParentLayoutDir) { *parentPreferredSizePntr += *preferredSizePntr + paddingLrOrTb; }
			else { *parentPreferredSizePntr = MaxR32(*parentPreferredSizePntr, *preferredSizePntr + paddingLrOrTb); }
		}
	}
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
	
	// We take the opportunity in CloseUiElement (which is a Reverse Breadth-First Visit on the tree) to do some basic sizing calculations
	UiElement* parent = GetUiElementParent(element, 0);
	CalculateUiElemSizeOnAxisOnClose(element, parent, true);
	CalculateUiElemSizeOnAxisOnClose(element, parent, false);
	
	return element;
}

PEXPI Str8 GetUiElementQualifiedName(Arena* arena, UiElement* element)
{
	DebugNotNull(element);
	TwoPassStr8Loop(result, arena, false)
	{
		for (uxx depth = 0; depth < element->depth; depth++)
		{
			UiElement* parent = GetUiElementParent(element, (element->depth-1) - depth);
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

static void DistributeSpaceToUiElemChildrenOnAxis(UiElement* element, bool xAxis, bool printDebug)
{
	bool isLayoutDir = (IsUiDirHorizontal(element->config.direction) == xAxis);
	r32 layoutAxisChildPadding = isLayoutDir ? ((r32)(element->numChildren > 1 ? element->numChildren-1 : 0) * element->config.childPadding) : 0.0f;
	r32 elemMargins = (xAxis ? (element->config.margins.Left + element->config.margins.Right) : (element->config.margins.Top + element->config.margins.Bottom));
	
	r32 minimumSize = (xAxis ? element->minimumSize.Width : element->minimumSize.Height);
	r32 preferredSize = (xAxis ? element->preferredSize.Width : element->preferredSize.Height);
	r32* sizePntr = (xAxis ? &element->layoutRec.Width : &element->layoutRec.Height);
	r32 innerSize = *sizePntr - elemMargins;
	
	// Copy children's minimumSize into layoutRec.Size and track how many want to be bigger and what their total minimum size is
	uxx numGrowableChildren = 0;
	r32 childrenMinimumTotal = (xAxis ? (element->config.padding.Left + element->config.padding.Right) : (element->config.padding.Top + element->config.padding.Bottom));
	for (uxx cIndex = 0; cIndex < element->numChildren; cIndex++)
	{
		UiElement* child = GetUiElementChild(element, cIndex);
		r32 childMinimumSize = (xAxis ? child->minimumSize.Width : child->minimumSize.Height);
		r32 childPreferredSize = (xAxis ? child->preferredSize.Width : child->preferredSize.Height);
		r32 childPaddingLrOrTb = (xAxis ? (child->config.padding.Left + child->config.padding.Right) : (child->config.padding.Top + child->config.padding.Bottom));
		r32* childSizePntr = (xAxis ? &child->layoutRec.Width : &child->layoutRec.Height);
		if (IsInfiniteOrNanR32(childPreferredSize) || childPreferredSize > childMinimumSize) { numGrowableChildren++; }
		*childSizePntr = childMinimumSize;
		childrenMinimumTotal += childMinimumSize + childPaddingLrOrTb + ((cIndex > 0) ? element->config.childPadding : 0.0f);
	}
	
	//TODO: We should probably be rounding to whole pixel values, and figuring out how to distribute remainders amongst n children
	r32 spaceToDistribute = (*sizePntr - childrenMinimumTotal);
	if (printDebug)
	{
		Str8 fullName = GetUiElementQualifiedName(UiCtx->frameArena, element);
		PrintLine_D("%.*s has %gpx along %s to %s %llu/%llu growable child%s",
			StrPrint(fullName),
			spaceToDistribute,
			xAxis ? "X" : "Y",
			isLayoutDir ? "distribute amongst" : "give to",
			numGrowableChildren, element->numChildren, PluralEx(numGrowableChildren, "", "ren")
		);
	}
	if (spaceToDistribute > 0)
	{
		for (uxx cIndex = 0; cIndex < element->numChildren; cIndex++)
		{
			UiElement* child = GetUiElementChild(element, cIndex);
			r32 childMinimumSize = (xAxis ? child->minimumSize.Width : child->minimumSize.Height);
			r32 childPreferredSize = (xAxis ? child->preferredSize.Width : child->preferredSize.Height);
			r32 childPaddingLrOrTb = (xAxis ? (child->config.padding.Left + child->config.padding.Right) : (child->config.padding.Top + child->config.padding.Bottom));
			r32* childSizePntr = (xAxis ? &child->layoutRec.Width : &child->layoutRec.Height);
			if (IsInfiniteOrNanR32(childPreferredSize) || childPreferredSize > childMinimumSize)
			{
				if (isLayoutDir)
				{
					*childSizePntr += (spaceToDistribute / (r32)numGrowableChildren);
				}
				else
				{
					*childSizePntr = MinR32(innerSize - childPaddingLrOrTb, childPreferredSize);
				}
			}
		}
	}
	else if (spaceToDistribute < 0)
	{
		//TODO: Implement me!
	}
}

static void UiSystemDoLayout()
{
	#if DEBUG_BUILD
	bool printDebug = IsKeyboardKeyPressed(UiCtx->keyboard, nullptr, Key_T, false);
	if (printDebug)
	{
		VarArrayLoop(&UiCtx->elements, eIndex)
		{
			VarArrayLoopGet(UiElement, element, &UiCtx->elements, eIndex);
			Str8 fullName = GetUiElementQualifiedName(UiCtx->frameArena, element);
			PrintLine_D("0x%016llX %.*s: element=%llu depth=%llu sibling=%llu parent=%llu children=%llu descendents=%llu - sizing[X=%s, Y=%s] minimum(%g,%g) preferred(%g,%g)",
				element->id.id,
				StrPrint(fullName),
				element->elementIndex,
				element->depth,
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
	
	// The root element is the only element that decides it's own final position/size
	// Otherwise we decide the size of children elements when visiting the parent
	UiElement* rootElement = VarArrayGetFirst(UiElement, &UiCtx->elements);
	rootElement->layoutRec.TopLeft = V2_Zero;
	rootElement->layoutRec.Size = rootElement->preferredSize;
	
	// +======================================+
	// | Distribute Sizes to Children via BFS |
	// +======================================+
	VarArray bfsIndices; //bfs = Breadth-First Search
	InitVarArrayWithInitial(uxx, &bfsIndices, UiCtx->frameArena, UiCtx->elements.length); //TODO: This is a hard upper bound, and quite an overestimate in most cases
	VarArrayAddValue(uxx, &bfsIndices, 0); //Add the root element, always at index 0
	while (bfsIndices.length > 0)
	{
		uxx eIndex = VarArrayGetFirstValue(uxx, &bfsIndices);
		VarArrayRemoveFirst(uxx, &bfsIndices);
		UiElement* element = VarArrayGetHard(UiElement, &UiCtx->elements, eIndex);
		
		if (printDebug)
		{
			Str8 fullName = GetUiElementQualifiedName(UiCtx->frameArena, element);
			PrintLine_D("0x%016llX %.*s Sizing %llu child%s",
				element->id.id,
				StrPrint(fullName),
				element->numChildren, PluralEx(element->numChildren, "", "ren")
			);
		}
		DistributeSpaceToUiElemChildrenOnAxis(element, true, printDebug);
		DistributeSpaceToUiElemChildrenOnAxis(element, false, printDebug);
		
		//Add all children to bfsIndices
		for (uxx cIndex = 0; cIndex < element->numChildren; cIndex++)
		{
			UiElement* child = GetUiElementChild(element, cIndex);
			if (child->numChildren > 0)
			{
				VarArrayAddValue(uxx, &bfsIndices, child->elementIndex);
			}
		}
		// uxx childIndex = eIndex+1;
		// UiElement* child = element+1;
		// for (uxx cIndex = 0; cIndex < element->numChildren; cIndex++)
		// {
		// 	if (child->numChildren > 0) { VarArrayAddValue(uxx, &bfsIndices, childIndex); }
		// 	childIndex += 1 + child->numDescendents;
		// }
	}
	
	// +============================================+
	// | Position Children According to UiLayoutDir |
	// +============================================+
	VarArrayLoop(&UiCtx->elements, eIndex)
	{
		VarArrayLoopGet(UiElement, element, &UiCtx->elements, eIndex);
		
		v2 layoutPos = V2_Zero_Const;
		if (element->config.direction == UiLayoutDir_LeftToRight || element->config.direction == UiLayoutDir_TopDown)
		{
			layoutPos = AddV2(element->layoutRec.TopLeft, element->config.margins.XY); //XY is alias for (Left,Top)
		}
		else if (element->config.direction == UiLayoutDir_RightToLeft)
		{
			layoutPos = AddV2(element->layoutRec.TopLeft, MakeV2(element->layoutRec.Width - element->config.margins.Right, element->config.margins.Top));
		}
		else if (element->config.direction == UiLayoutDir_BottomUp)
		{
			layoutPos = AddV2(element->layoutRec.TopLeft, MakeV2(element->config.margins.Left, element->layoutRec.Height - element->config.margins.Bottom));
		}
		else { DebugAssert(false); }
		
		for (uxx cIndex = 0; cIndex < element->numChildren; cIndex++)
		{
			UiElement* child = GetUiElementChild(element, cIndex);
			r32 childPadding = ((cIndex > 0) ? element->config.childPadding : 0.0f);
			if (element->config.direction == UiLayoutDir_LeftToRight) { layoutPos.X += child->config.padding.Left; }
			if (element->config.direction == UiLayoutDir_TopDown) { layoutPos.Y += child->config.padding.Top; }
			if (element->config.direction == UiLayoutDir_RightToLeft) { layoutPos.X -= child->layoutRec.Width + childPadding + child->config.padding.Right; }
			if (element->config.direction == UiLayoutDir_BottomUp) { layoutPos.Y -= child->layoutRec.Height + childPadding + child->config.padding.Bottom; }
			
			child->layoutRec.TopLeft = layoutPos;
			if (IsUiDirHorizontal(element->config.direction)) { child->layoutRec.Y += child->config.padding.Top; }
			else { child->layoutRec.X += child->config.padding.Left; }
			
			if (element->config.direction == UiLayoutDir_LeftToRight) { layoutPos.X += child->layoutRec.Width + childPadding + child->config.padding.Right; }
			if (element->config.direction == UiLayoutDir_TopDown) { layoutPos.Y += child->layoutRec.Height + childPadding + child->config.padding.Bottom; }
			if (element->config.direction == UiLayoutDir_RightToLeft) { layoutPos.X -= child->config.padding.Left; }
			if (element->config.direction == UiLayoutDir_BottomUp) { layoutPos.Y -= child->config.padding.Top; }
			
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
		}
	}
	
	#if 0
	// +==============================+
	// |          UI Layout           |
	// +==============================+
	#if DEBUG_BUILD
	if (printDebug) { PrintLine_N("LAYOUT TIME! %gx%g", UiCtx->screenSize.Width, UiCtx->screenSize.Height); }
	#endif
	VarArrayLoop(&UiCtx->elements, eIndex)
	{
		VarArrayLoopGet(UiElement, element, &UiCtx->elements, eIndex);
		UiElement* parent = VarArrayGetSoft(UiElement, &UiCtx->elements, element->parentIndex);
		rec parentRec = (parent != nullptr) ? parent->layoutRec : MakeRecV(V2_Zero, UiCtx->screenSize);
		v4r parentMargins = (parent != nullptr) ? parent->config.margins : V4r_Zero;
		parentRec = InflateRecEx(parentRec, -parentMargins.Left, -parentMargins.Right, -parentMargins.Top, -parentMargins.Bottom);//TODO: Change this to "DelateRecEx" once we make that alias
		UiLayoutDir direction = (parent != nullptr) ? parent->config.direction : UiLayoutDir_Default;
		uxx parentChildCount = (parent != nullptr) ? parent->numChildren : UiCtx->numTopLevelElements;
		if (direction == UiLayoutDir_TopDown || direction == UiLayoutDir_BottomUp)
		{
			element->layoutRec.X = parentRec.X;
			element->layoutRec.Width = parentRec.Width;
			element->layoutRec.Height = parentRec.Height / (r32)parentChildCount;
			if (direction == UiLayoutDir_TopDown)
			{
				element->layoutRec.Y = parentRec.Y + (element->layoutRec.Height * (r32)element->siblingIndex);
			}
			else //(direction == UiLayoutDir_BottomUp)
			{
				 element->layoutRec.Y = parentRec.Y + parentRec.Height - (element->layoutRec.Height * (r32)(element->siblingIndex+1));
			}
			#if DEBUG_BUILD
			if (printDebug)
			{
				Str8 fullName = GetUiElementQualifiedName(UiCtx->frameArena, element);
				PrintLine_D("%.*s: vertical, sibling=%llu/%llu, Height=%g/%g Y=%g/[%g,%g]",
					StrPrint(fullName),
					element->siblingIndex+1,
					parentChildCount,
					element->layoutRec.Height,
					parentRec.Height,
					element->layoutRec.Y,
					parentRec.Y,
					parentRec.Y + parentRec.Height
				);
			}
			#endif
		}
		else if (direction == UiLayoutDir_LeftToRight || direction == UiLayoutDir_RightToLeft)
		{
			element->layoutRec.Y = parentRec.Y;
			element->layoutRec.Height = parentRec.Height;
			element->layoutRec.Width = parentRec.Width / (r32)parentChildCount;
			if (direction == UiLayoutDir_LeftToRight)
			{
				element->layoutRec.X = parentRec.X + (element->layoutRec.Width * (r32)element->siblingIndex);
			}
			else //(direction == UiLayoutDir_RightToLeft)
			{
				 element->layoutRec.X = parentRec.X + parentRec.Width - (element->layoutRec.Width * (r32)(element->siblingIndex+1));
			}
			#if DEBUG_BUILD
			if (printDebug)
			{
				Str8 fullName = GetUiElementQualifiedName(UiCtx->frameArena, element);
				PrintLine_D("%.*s: horizontal, sibling=%llu/%llu, Width=%g/%g X=%g/[%g,%g]",
					StrPrint(fullName),
					element->siblingIndex+1,
					parentChildCount,
					element->layoutRec.Width,
					parentRec.Width,
					element->layoutRec.X,
					parentRec.X,
					parentRec.X + parentRec.Width
				);
			}
			#endif
		}
		else
		{
			PrintLine_E("Invalid direction %llu in UI element \"%.*s\"", (uxx)direction, StrPrint(element->config.id.str)); //TODO: Change this debug output once we have things more solidified. Printing out the config.id.str is probably not enough!
			DebugAssertMsg(false, "Invalid direction in UI element config!");
			element->layoutRec = parentRec;
		}
		
		// Deflate layoutRec by padding TODO: Turn this sort of "safe" deflation into a function!
		if (element->config.padding.Left + element->config.padding.Right < element->layoutRec.Width) { element->layoutRec.X += element->config.padding.Left; }
		else { element->layoutRec.X = element->layoutRec.X + (element->layoutRec.Width * (element->config.padding.Left / (element->config.padding.Left + element->config.padding.Right))); }
		if (element->config.padding.Top + element->config.padding.Bottom < element->layoutRec.Height) { element->layoutRec.Y += element->config.padding.Top; }
		else { element->layoutRec.Y = element->layoutRec.Y + (element->layoutRec.Height * (element->config.padding.Top / (element->config.padding.Top + element->config.padding.Bottom))); }
		element->layoutRec.Width = MaxR32(0, element->layoutRec.Width - element->config.padding.Left - element->config.padding.Right);
		element->layoutRec.Height = MaxR32(0, element->layoutRec.Height - element->config.padding.Left - element->config.padding.Right);
	}
	#endif
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
	UiCtx->renderList.context = UiCtx;
	UiCtx->renderList.arena = UiCtx->frameArena;
	InitVarArrayWithInitial(UiRenderCmd, &UiCtx->renderList.commands, UiCtx->frameArena, UiCtx->elements.length); //Lower estimate (TODO: Should we decide on a better upper estimate?)
	VarArrayLoop(&UiCtx->elements, eIndex)
	{
		VarArrayLoopGet(UiElement, element, &UiCtx->elements, eIndex);
		if (element->config.color.a != 0 ||
			(element->config.borderColor.a != 0 && (element->config.borderThickness.Left != 0 || element->config.borderThickness.Top != 0 || element->config.borderThickness.Right != 0 || element->config.borderThickness.Bottom != 0)))
		{
			UiRenderCmd* newCmd = VarArrayAdd(UiRenderCmd, &UiCtx->renderList.commands);
			DebugNotNull(newCmd);
			ClearPointer(newCmd);
			newCmd->type = UiRenderCmdType_Rectangle;
			newCmd->rectangle.rectangle = element->layoutRec;
			newCmd->rectangle.color = element->config.color;
			newCmd->rectangle.borderThickness = element->config.borderThickness;
			newCmd->rectangle.borderColor = element->config.borderColor;
			newCmd->rectangle.cornerRadius = V4r_Zero; //TODO: Implement this!
			newCmd->rectangle.texture = nullptr; //TODO: Implement this!
		}
	}
	
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
