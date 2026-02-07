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

typedef plex UiId UiId;
plex UiId
{
	uxx id;
	uxx index;
	//We copy the string the Id was sourced from for 2 reasons
	// 1. In OpenUiElement we need to calculate the "real" ID which is a mix of parent element's ID and the new element's string
	// 2. For debug purposes we copy this into the frameArena and keep it in the config for the element so we can print out or display the ID's
	Str8 str;
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
		case UiElemDirection_TopDown:     return "TopDown";
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
	
	uxx elementIndex;
	uxx parentIndex;
	UiId parentId;
	bool isOpen; //are we currently adding children to this element
	bool runChildCode;
	uxx numChildren;
	uxx numDescendents;
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
	
	uxx parentElementIndex;
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
	PIG_CORE_INLINE UiId UiIdFromStrIndexWithBase(UiId baseId, Str8 idString, uxx index);
	PIG_CORE_INLINE UiId UiIdFromStrIndex(Str8 idString, uxx index);
	PIG_CORE_INLINE UiId UiIdFromStr(Str8 idString);
	void InitUiContext(Arena* arena, UiContext* contextOut);
	PIG_CORE_INLINE UiElement* GetUiElementParent(UiElement* element, uxx ancestorIndex);
	PIG_CORE_INLINE UiElement* GetUiElementChild(UiElement* element, uxx childIndex);
	void StartUiFrame(UiContext* context, v2 screenSize, r32 scale, u64 programTime, KeyboardState* keyboard, MouseState* mouse, TouchscreenState* touchscreen);
	PIG_CORE_INLINE UiElement* OpenUiElement(UiElemConfig config);
	PIG_CORE_INLINE bool OpenUiElementConditional(UiElemConfig config);
	PIG_CORE_INLINE void CloseUiElement();
	UiRenderList* GetUiRenderList();
	void EndUiFrame();
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define MakeUiId_Const(idValue, indexValue, strValue) { .id=(idValue), .index=(indexValue), .str=(strValue) }
#define MakeUiId(id, index, str) NEW_STRUCT(UiId)MakeUiId_Const((id), (index), (str))

#define UiId_None       MakeUiId(0, 0, Str8_Empty)
#define UiId_None_Const MakeUiId_Const(0, 0, Str8_Empty_Const)

//This is the most convenient way to give a UiId from a string literal, like UiIdNt("Element1")
#define UiIdLit(idStrLit) UiIdFromStrIndexWithBase(UiId_None, StrLit(idStrLit), PIG_UI_ID_INDEX_NONE)
#define UiIdNt(idStrNt) UiIdFromStrIndexWithBase(UiId_None, MakeStr8Nt(idStrNt), PIG_UI_ID_INDEX_NONE)

//TODO: Write a description of this macro and it's need
#define UIELEM(config) DeferIfBlock(OpenUiElementConditional(NEW_STRUCT(UiElemConfig)config), CloseUiElement())

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

#if (TARGET_HAS_THREADING && DEBUG_BUILD)
#define AssertUiThreadIsSame() AssertMsg(UiCtx == nullptr || UiCtx->threadId == OsGetCurrentThreadId(), "Calling a UI function from a different thread than the UI frame was started on!")
#else
#define AssertUiThreadIsSame() //nothing
#endif

PEXPI UiId UiIdFromStrIndexWithBase(UiId baseId, Str8 idString, uxx index)
{
	UiId result = ZEROED;
	#if TARGET_IS_64BIT
	result.id = FnvHashU64Ex(idString.chars, idString.length, baseId.id);
	#else //TARGET_IS_32BIT
	result.id = FnvHashU32Ex(idString.chars, idString.length, baseId.id);
	#endif //TARGET_IS_64BIT
	result.id += index;
	result.index = index;
	#if DEBUG_BUILD
	result.str = idString; //TODO: Allocate this in the UiCtx->frameArena?
	#endif
	return result;
}
PEXPI UiId UiIdFromStrIndex(Str8 idString, uxx index) { return UiIdFromStrIndexWithBase(UiId_None, idString, index); }
PEXPI UiId UiIdFromStr(Str8 idString) { return UiIdFromStrIndexWithBase(UiId_None, idString, PIG_UI_ID_INDEX_NONE); }

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
	context->parentElementIndex = PIG_UI_INDEX_INVALID;
	
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
	newElement->parentIndex = UiCtx->parentElementIndex;
	MyMemCopy(&newElement->config, &config, sizeof(UiElemConfig));
	UiElement* parentElement = VarArrayGetSoft(UiElement, &UiCtx->elements, UiCtx->parentElementIndex);
	if (parentElement != nullptr)
	{
		DebugAssert(parentElement->isOpen);
		newElement->id = UiIdFromStrIndexWithBase(parentElement->id, config.id.str, config.id.index);
		newElement->parentId = parentElement->config.id;
		parentElement->numChildren++;
		while (parentElement != nullptr)
		{
			parentElement->numDescendents++;
			parentElement = VarArrayGetSoft(UiElement, &UiCtx->elements, parentElement->parentIndex);
		}
	}
	else
	{
		newElement->id = config.id;
	}
	UiCtx->parentElementIndex = (UiCtx->elements.length - 1);
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
	DebugAssertMsg(UiCtx->parentElementIndex < UiCtx->elements.length, "Tried to close UI element when none was open! UI hierarchy is potentially invalid!");
	if (UiCtx->parentElementIndex >= UiCtx->elements.length) { return; }
	UiElement* element = VarArrayGetHard(UiElement, &UiCtx->elements, UiCtx->parentElementIndex);
	DebugAssert(element->isOpen);
	element->isOpen = false;
	UiCtx->parentElementIndex = element->parentIndex;
}

PEXP UiRenderList* GetUiRenderList()
{
	NotNull(UiCtx);
	AssertUiThreadIsSame();
	DebugAssertMsg(UiCtx->parentElementIndex >= UiCtx->elements.length, "Not all UI elements had a CloseUiElement call! UI hierarchy is potentially invalid!");
	//TODO: Empty context->renderList
	//TODO: Fill context->renderList
	
	#if DEBUG_BUILD
	if (IsKeyboardKeyPressed(UiCtx->keyboard, nullptr, Key_T, false))
	{
		VarArrayLoop(&UiCtx->elements, eIndex)
		{
			VarArrayLoopGet(UiElement, element, &UiCtx->elements, eIndex);
			NewStrBuffEx(parentBuffer, 128);
			uxx parentIndex = element->parentIndex;
			while (parentIndex < UiCtx->elements.length)
			{
				UiElement* parent = VarArrayGetHard(UiElement, &UiCtx->elements, parentIndex);
				InsertIntoStrBuff(&parentBuffer, 0, ".");
				InsertIntoStrBuffStr(&parentBuffer, 0, parent->config.id.str);
				DebugAssert(parent->parentIndex != parentIndex);
				parentIndex = parent->parentIndex;
			}
			PrintLine_D("%.*s%.*s", StrPrint(parentBuffer.str), StrPrint(element->config.id.str));
		}
	}
	#endif //DEBUG_BUILD
	
	UiCtx->renderList.context = UiCtx;
	UiCtx->renderList.arena = UiCtx->frameArena;
	return &UiCtx->renderList;
}

PEXP void EndUiFrame()
{
	NotNull(UiCtx);
	AssertUiThreadIsSame();
	
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
