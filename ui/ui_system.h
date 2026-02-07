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

typedef enum UiElemDirection UiElemDirection;
enum UiElemDirection
{
	UiElemDirection_Default = 0, //TopDown
	UiElemDirection_TopDown = 0,
	UiElemDirection_BottomUp,
	UiElemDirection_LeftToRight,
	UiElemDirection_RightToLeft,
	UiElemDirection_Count,
};
const char* GetUiElemDirectionStr(UiElemDirection enumValue)
{
	switch (enumValue)
	{
		case UiElemDirection_TopDown:     return "TopDown(Default)";
		case UiElemDirection_BottomUp:    return "BottomUp";
		case UiElemDirection_LeftToRight: return "LeftToRight";
		case UiElemDirection_RightToLeft: return "RightToLeft";
		default: return UNKNOWN_STR;
	}
}

typedef plex UiElemConfig UiElemConfig;
plex UiElemConfig
{
	UiId id;
	bool globalId;
	UiElemDirection direction;
	Color32 color;
	v4 borderThickness;
	Color32 borderColor;
	//TODO: Bunch of other layout parameters!
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
			v4 cornerRadius;
			v4 borderThickness;
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
	v2 screenSize;
	r32 scale;
	u64 programTime;
	KeyboardState* keyboard;
	MouseState* mouse;
	TouchscreenState* touchscreen;
	
	uxx currentElementIndex;
	uxx numTopLevelElements;
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
	void StartUiFrame(UiContext* context, v2 screenSize, r32 scale, u64 programTime, KeyboardState* keyboard, MouseState* mouse, TouchscreenState* touchscreen);
	PIG_CORE_INLINE UiElement* OpenUiElement(UiElemConfig config);
	PIG_CORE_INLINE bool OpenUiElementConditional(UiElemConfig config);
	PIG_CORE_INLINE void CloseUiElement();
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

//TODO: Write a description of this macro and it's need
#define UIELEM(config) DeferIfBlock(OpenUiElementConditional(/*NEW_STRUCT(UiElemConfig)*/config), CloseUiElement())

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

PEXP void StartUiFrame(UiContext* context, v2 screenSize, r32 scale, u64 programTime, KeyboardState* keyboard, MouseState* mouse, TouchscreenState* touchscreen)
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
	context->currentElementIndex = PIG_UI_INDEX_INVALID;
	context->numTopLevelElements = 0;
	
	//TODO: Reset the elements array? Copy the array from last frame somewhere?
	
	Assert(UiCtx == nullptr);
	UiCtx = context;
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
	UiElement* parentElement = GetUiElementParent(newElement, 0); //VarArrayGetSoft(UiElement, &UiCtx->elements, UiCtx->currentElementIndex);
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

PEXPI void CloseUiElement()
{
	NotNull(UiCtx);
	AssertUiThreadIsSame();
	DebugAssertMsg(UiCtx->currentElementIndex < UiCtx->elements.length, "Tried to close UI element when none was open! UI hierarchy is potentially invalid!");
	if (UiCtx->currentElementIndex >= UiCtx->elements.length) { return; }
	UiElement* element = VarArrayGetHard(UiElement, &UiCtx->elements, UiCtx->currentElementIndex);
	DebugAssert(element->isOpen);
	element->isOpen = false;
	UiCtx->currentElementIndex = element->parentIndex;
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

PEXP UiRenderList* GetUiRenderList()
{
	NotNull(UiCtx);
	AssertUiThreadIsSame();
	DebugAssertMsg(UiCtx->currentElementIndex >= UiCtx->elements.length, "Not all UI elements had a CloseUiElement call! UI hierarchy is potentially invalid!");
	
	#if DEBUG_BUILD
	bool printDebug = IsKeyboardKeyPressed(UiCtx->keyboard, nullptr, Key_T, false);
	if (printDebug)
	{
		VarArrayLoop(&UiCtx->elements, eIndex)
		{
			VarArrayLoopGet(UiElement, element, &UiCtx->elements, eIndex);
			Str8 fullName = GetUiElementQualifiedName(UiCtx->frameArena, element);
			PrintLine_D("0x%016llX %.*s: element=%llu depth=%llu sibling=%llu parent=%llu children=%llu descendents=%llu",
				element->id.id,
				StrPrint(fullName),
				element->elementIndex,
				element->depth,
				element->siblingIndex,
				element->parentIndex,
				element->numChildren,
				element->numDescendents
			);
		}
	}
	#endif //DEBUG_BUILD
	
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
		UiElemDirection direction = (parent != nullptr) ? parent->config.direction : UiElemDirection_Default;
		uxx parentChildCount = (parent != nullptr) ? parent->numChildren : UiCtx->numTopLevelElements;
		if (direction == UiElemDirection_TopDown || direction == UiElemDirection_BottomUp)
		{
			element->layoutRec.X = parentRec.X;
			element->layoutRec.Width = parentRec.Width;
			element->layoutRec.Height = parentRec.Height / (r32)parentChildCount;
			if (direction == UiElemDirection_TopDown)
			{
				element->layoutRec.Y = parentRec.Y + (element->layoutRec.Height * (r32)element->siblingIndex);
			}
			else //(direction == UiElemDirection_BottomUp)
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
		else if (direction == UiElemDirection_LeftToRight || direction == UiElemDirection_RightToLeft)
		{
			element->layoutRec.Y = parentRec.Y;
			element->layoutRec.Height = parentRec.Height;
			element->layoutRec.Width = parentRec.Width / (r32)parentChildCount;
			if (direction == UiElemDirection_LeftToRight)
			{
				element->layoutRec.X = parentRec.X + (element->layoutRec.Width * (r32)element->siblingIndex);
			}
			else //(direction == UiElemDirection_RightToLeft)
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
			(element->config.borderColor.a != 0 && (element->config.borderThickness.X != 0 || element->config.borderThickness.Y != 0 || element->config.borderThickness.Z != 0 || element->config.borderThickness.W != 0)))
		{
			UiRenderCmd* newCmd = VarArrayAdd(UiRenderCmd, &UiCtx->renderList.commands);
			DebugNotNull(newCmd);
			ClearPointer(newCmd);
			newCmd->type = UiRenderCmdType_Rectangle;
			newCmd->rectangle.rectangle = element->layoutRec;
			newCmd->rectangle.color = element->config.color;
			newCmd->rectangle.borderThickness = element->config.borderThickness;
			newCmd->rectangle.borderColor = element->config.borderColor;
			newCmd->rectangle.cornerRadius = V4_Zero; //TODO: Implement this!
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
	
	VarArrayClear(&UiCtx->elements);
	
	UiCtx = nullptr;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_PIG_UI

#endif //  _UI_SYSTEM_H
