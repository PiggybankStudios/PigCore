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
	** the rendering of this list is done in ui_renderer.h
*/

#ifndef _UI_SYSTEM_H
#define _UI_SYSTEM_H

#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
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

#if BUILD_WITH_PIG_UI

typedef plex UiId UiId;
plex UiId
{
	uxx id;
	uxx index;
	#if DEBUG_BUILD
	Str8 str; //for debug purposes we snag a pointer to the string
	#endif
};

typedef enum UiElemDirection UiElemDirection;
enum UiElemDirection
{
	UiElemDirection_Default = 0, //TopDown
	UiElemDirection_TopDown,
	UiElemDirection_BottomUp,
	UiElemDirection_LeftToRight,
	UiElemDirection_RightToLeft,
	UiElemDirection_Count,
};
const char* GetUiElemDirectionStr(UiElemDirection enumValue)
{
	switch (enumValue)
	{
		case UiElemDirection_Default:     return "Default";
		case UiElemDirection_TopDown:     return "TopDown";
		case UiElemDirection_BottomUp:    return "BottomUp";
		case UiElemDirection_LeftToRight: return "LeftToRight";
		case UiElemDirection_RightToLeft: return "RightToLeft";
		case UiElemDirection_Count:       return "Count";
		default: return "Unknown";
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
	UiId parentId;
	bool isOpen; //are we currently adding children to this element
	uxx numChildren;
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
	void InitUiContext(Arena* arena, UiContext* contextOut);
	void StartUiFrame(UiContext* context, v2 screenSize, r32 scale, u64 programTime, KeyboardState* keyboard, MouseState* mouse, TouchscreenState* touchscreen);
	PIG_CORE_INLINE UiElement* OpenUiElement(UiElemConfig config);
	PIG_CORE_INLINE void CloseUiElement();
	UiRenderList* GetUiRenderList();
	void EndUiRender();
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define MakeUiId_Const(idValue, indexValue) { .id=(idValue), .index=(indexValue) }
#define MakeUiId(id, index) NEW_STRUCT(UiId)MakeUiId_Const((id), (index))

#define UiId_None       MakeUiId(0, 0)
#define UiId_None_Const MakeUiId_Const(0, 0)

#define UIELEM() //TODO:

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

#if (TARGET_HAS_THREADING && DEBUG_BUILD)
#define AssertUiThreadIsSame() AssertMsg(UiCtx == nullptr || UiCtx->threadId == OsGetCurrentThreadId(), "Calling a UI function from a different thread than the UI frame was started on!")
#else
#define AssertUiThreadIsSame() //nothing
#endif

PEXP void InitUiContext(Arena* arena, UiContext* contextOut)
{
	NotNull(arena);
	NotNull(contextOut);
	ClearPointer(contextOut);
	contextOut->arena = arena;
	InitVarArray(UiElement, &contextOut->elements, arena);
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
	context->parentElementIndex = UINTXX_MAX;
	
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
	MyMemCopy(&newElement->config, &config, sizeof(UiElemConfig));
	UiElement* parentElement = VarArrayGetSoft(UiElement, &UiCtx->elements, UiCtx->parentElementIndex);
	if (parentElement != nullptr)
	{
		DebugAssert(parentElement->isOpen);
		parentElement->numChildren++;
		newElement->parentId = parentElement->config.id;
	}
	UiCtx->parentElementIndex = (UiCtx->elements.length - 1);
	newElement->isOpen = true;
	
	return newElement;
}

PEXPI void CloseUiElement()
{
	NotNull(UiCtx);
	AssertUiThreadIsSame();
	DebugAssertMsg(UiCtx->parentElementIndex < UiCtx->elements.length, "Tried to close UI element when none was open! UI hierarchy is potentially invalid!");
	if (UiCtx->parentElementIndex >= UiCtx->elements.length) { return; }
	UiElement* element = VarArrayGetHard(UiElement, &UiCtx->elements, UiCtx->parentElementIndex);
	DebugAssert(element->isOpen);
	element->isOpen = false;
	UiCtx->parentElementIndex = UINTXX_MAX;
	//Find the last element with isOpen still set to true (if there is any)
	for (uxx eIndex = UiCtx->elements.length; eIndex > 0; eIndex--)
	{
		UiElement* parentElement = VarArrayGet(UiElement, &UiCtx->elements, eIndex-1);
		if (parentElement->isOpen) { UiCtx->parentElementIndex = eIndex-1; break; }
	}
}

PEXP UiRenderList* GetUiRenderList()
{
	NotNull(UiCtx);
	AssertUiThreadIsSame();
	DebugAssertMsg(UiCtx->parentElementIndex >= UiCtx->elements.length, "Not all UI elements had a CloseUiElement call! UI hierarchy is potentially invalid!");
	//TODO: Empty context->renderList
	//TODO: Fill context->renderList
	UiCtx->renderList.context = UiCtx;
	UiCtx->renderList.arena = UiCtx->frameArena;
	return &UiCtx->renderList;
}

PEXP void EndUiRender()
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
