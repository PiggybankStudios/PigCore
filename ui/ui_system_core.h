/*
File:   ui_system_core.h
Author: Taylor Robbins
Date:   02\11\2026
Description:
	** Most of the types, enums, constants, and forward declarations of some functions for the UI System go here.
	** This allows us to bypass a dependency problem between ui_system.h and other files like ui_system_theming.h.
	** The various ui_system_[x].h files are separated mostly for organizational purposes, so they depend on each
	** other and cannot be separated, but they live in separate files to keep things organized
*/

#ifndef _UI_SYSTEM_CORE_H
#define _UI_SYSTEM_CORE_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "mem/mem_arena.h"
#include "struct/struct_rectangles.h"
#include "struct/struct_vectors.h"
#include "struct/struct_string.h"
#include "struct/struct_var_array.h"
#include "struct/struct_directions.h"
#include "os/os_threading.h"
#include "input/input_keyboard.h"
#include "input/input_mouse.h"
#include "input/input_touch.h"
#include "gfx/gfx_texture.h"
#include "gfx/gfx_font.h"

#if BUILD_WITH_PIG_UI

#define PIG_UI_INDEX_INVALID UINTXX_MAX
#define PIG_UI_ID_INDEX_NONE UINTXX_MAX
#define PIG_UI_DEFAULT_ID_STR "elem"

#define PigUiDefaultColor_Value TransparentBlack_Value
#define PigUiDefaultColor       TransparentBlack

// Application Defined Types
//NOTE: These #defines can be set to change the UiElemConfig structure to the applications needs.
//      If an application-specific renderer, themer, etc. needs more parameters for each element
//      to dictate it's behavior then the application can makes it own type, set this #define,
//      fill out the parameters in the UiElemConfig, and then use that parameters in the renderer, themer, etc.
typedef plex UiRendererParameters_Stub UiRendererParameters_Stub;
plex UiRendererParameters_Stub { int stub; };
#ifndef UiRendererParameters
#define UiRendererParameters UiRendererParameters_Stub
#endif
typedef plex UiThemerParameters_Stub UiThemerParameters_Stub;
plex UiThemerParameters_Stub { int stub; };
#ifndef UiThemerParameters
#define UiThemerParameters UiThemerParameters_Stub
#endif

struct UiContext; //forward declared so we don't get clang's visibility warning for the typedef of UiThemerCallback_f below

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

#define IsUiDirHorizontal(direction) ( (direction) == UiLayoutDir_RightToLeft || (direction) == UiLayoutDir_LeftToRight )
#define IsUiDirVertical(direction)   ( (direction) == UiLayoutDir_TopDown     || (direction) == UiLayoutDir_BottomUp    )

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

typedef enum UiFloatingType UiFloatingType;
enum UiFloatingType
{
	UiFloatingType_None = 0,
	UiFloatingType_Root,
	UiFloatingType_Parent,
	UiFloatingType_Id,
	UiFloatingType_Count,
	UiFloatingType_Default = UiFloatingType_None,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetUiFloatingTypeStr(UiFloatingType enumValue);
#else
PEXP const char* GetUiFloatingTypeStr(UiFloatingType enumValue)
{
	switch (enumValue)
	{
		case UiFloatingType_None:   return "None";
		case UiFloatingType_Root:   return "Root";
		case UiFloatingType_Parent: return "Parent";
		case UiFloatingType_Id:     return "Id";
		case UiFloatingType_Count:  return "Count";
		default: return "Unknown";
	}
}
#endif

typedef plex UiFloatingConfig UiFloatingConfig;
plex UiFloatingConfig
{
	UiFloatingType type;
	v2 offset;
	Dir2Ex parentAttach;
	Dir2Ex elemAttach;
};

// +==============================+
// |        Element Config        |
// +==============================+
//NOTE: Changes to this structure should be reflect in the UiElemConfigField enum
typedef plex UiElemConfig UiElemConfig;
plex UiElemConfig
{
	UiId id;
	bool globalId; //keeps the ID in the UiElement from being based on the parents' IDs
	UiLayoutDir direction;
	UiSizing sizing;
	Color32 color;
	Texture* texture;
	bool dontSizeToTexture;
	v4r margins; //space between our bounds and our childrens' bounds
	v4r padding; //space between allocated area and our bounds
	r32 childPadding; //space in-between each child, along the layout direction
	v4r borderThickness;
	Color32 borderColor;
	UiFloatingConfig floating;
	
	//These types can contain different things for each application, see the description near the top of the file
	UiRendererParameters renderer;
	UiThemerParameters themer;
};

//When configuring an element we often use the 0 value as a "default". So a color of "transparent black" actually means the default color, which is fully opaque white
#define UiConfigColorToActualColor(color) (((color).valueU32 != PigUiDefaultColor_Value) ? (color) : White)

// This is a bitwise enum where each bit represents a single "field" in the UiElemConfig structure above
// Some values are aliases for combinations of other values, like Margins is MarginLeft|MarginTop|MarginRight|MarginBottom
// This is useful when we want to explicitly list which fields are overridden, esp. for things like UiThemers
// NOTE: This is currently a 32-bit bitfield, but it may grow soon since 29/32 bits are used.
typedef enum UiElemConfigField UiElemConfigField;
enum UiElemConfigField
{
	UiElemConfigField_None                  = 0x00000000ull,
	UiElemConfigField_Id                    = ((1ull) << 0),
	UiElemConfigField_GlobalId              = ((1ull) << 1),
	UiElemConfigField_Direction             = ((1ull) << 2),
	UiElemConfigField_SizingTypeX           = ((1ull) << 3),
	UiElemConfigField_SizingValueX          = ((1ull) << 4),
	UiElemConfigField_SizingTypeY           = ((1ull) << 5),
	UiElemConfigField_SizingValueY          = ((1ull) << 6),
	UiElemConfigField_Color                 = ((1ull) << 7),
	UiElemConfigField_Texture               = ((1ull) << 8),
	UiElemConfigField_DontSizeToTexture     = ((1ull) << 9),
	UiElemConfigField_MarginLeft            = ((1ull) << 10),
	UiElemConfigField_MarginTop             = ((1ull) << 11),
	UiElemConfigField_MarginRight           = ((1ull) << 12),
	UiElemConfigField_MarginBottom          = ((1ull) << 13),
	UiElemConfigField_PaddingLeft           = ((1ull) << 14),
	UiElemConfigField_PaddingTop            = ((1ull) << 15),
	UiElemConfigField_PaddingRight          = ((1ull) << 16),
	UiElemConfigField_PaddingBottom         = ((1ull) << 17),
	UiElemConfigField_ChildPadding          = ((1ull) << 18),
	UiElemConfigField_BorderThicknessLeft   = ((1ull) << 19),
	UiElemConfigField_BorderThicknessTop    = ((1ull) << 20),
	UiElemConfigField_BorderThicknessRight  = ((1ull) << 21),
	UiElemConfigField_BorderThicknessBottom = ((1ull) << 22),
	UiElemConfigField_BorderColor           = ((1ull) << 23),
	UiElemConfigField_FloatingType          = ((1ull) << 24),
	UiElemConfigField_FloatingOffsetX       = ((1ull) << 25),
	UiElemConfigField_FloatingOffsetY       = ((1ull) << 26),
	UiElemConfigField_FloatingParentAttach  = ((1ull) << 27),
	UiElemConfigField_FloatingElemAttach    = ((1ull) << 28),
	UiElemConfigField_RendererParams        = ((1ull) << 29), // NOTE: Fields inside UiRendererParameters struct are not represented individually
	UiElemConfigField_ThemerParams          = ((1ull) << 30), // NOTE: Fields inside UiThemerParameters struct are not represented individually
	UiElemConfigField_Count                 = 31,
	UiElemConfigField_All                   = (((1ull) << UiElemConfigField_Count)-1),
	UiElemConfigField_Sizing                = (UiElemConfigField_SizingTypeX|UiElemConfigField_SizingTypeY|UiElemConfigField_SizingValueX|UiElemConfigField_SizingValueY),
	UiElemConfigField_SizingX               = (UiElemConfigField_SizingTypeX|UiElemConfigField_SizingValueX),
	UiElemConfigField_SizingY               = (UiElemConfigField_SizingTypeY|UiElemConfigField_SizingValueY),
	UiElemConfigField_Margins               = (UiElemConfigField_MarginLeft|UiElemConfigField_MarginTop|UiElemConfigField_MarginRight|UiElemConfigField_MarginBottom),
	UiElemConfigField_Padding               = (UiElemConfigField_PaddingLeft|UiElemConfigField_PaddingTop|UiElemConfigField_PaddingRight|UiElemConfigField_PaddingBottom),
	UiElemConfigField_BorderThickness       = (UiElemConfigField_BorderThicknessLeft|UiElemConfigField_BorderThicknessTop|UiElemConfigField_BorderThicknessRight|UiElemConfigField_BorderThicknessBottom),
	UiElemConfigField_Floating              = (UiElemConfigField_FloatingType|UiElemConfigField_FloatingOffsetX|UiElemConfigField_FloatingOffsetY|UiElemConfigField_FloatingParentAttach|UiElemConfigField_FloatingElemAttach),
	UiElemConfigField_FloatingOffset        = (UiElemConfigField_FloatingOffsetX|UiElemConfigField_FloatingOffsetY),
};

#if !PIG_CORE_IMPLEMENTATION
const char* GetUiElemConfigFieldStr(UiElemConfigField enumValue);
#else 
PEXP const char* GetUiElemConfigFieldStr(UiElemConfigField enumValue)
{
	switch (enumValue)
	{
		case UiElemConfigField_None:                  return "None";
		case UiElemConfigField_Id:                    return "Id";
		case UiElemConfigField_GlobalId:              return "GlobalId";
		case UiElemConfigField_Direction:             return "Direction";
		case UiElemConfigField_SizingTypeX:           return "SizingTypeX";
		case UiElemConfigField_SizingTypeY:           return "SizingTypeY";
		case UiElemConfigField_SizingValueX:          return "SizingValueX";
		case UiElemConfigField_SizingValueY:          return "SizingValueY";
		case UiElemConfigField_Color:                 return "Color";
		case UiElemConfigField_Texture:               return "Texture";
		case UiElemConfigField_DontSizeToTexture:     return "DontSizeToTexture";
		case UiElemConfigField_MarginLeft:            return "MarginLeft";
		case UiElemConfigField_MarginTop:             return "MarginTop";
		case UiElemConfigField_MarginRight:           return "MarginRight";
		case UiElemConfigField_MarginBottom:          return "MarginBottom";
		case UiElemConfigField_PaddingLeft:           return "PaddingLeft";
		case UiElemConfigField_PaddingTop:            return "PaddingTop";
		case UiElemConfigField_PaddingRight:          return "PaddingRight";
		case UiElemConfigField_PaddingBottom:         return "PaddingBottom";
		case UiElemConfigField_ChildPadding:          return "ChildPadding";
		case UiElemConfigField_BorderThicknessLeft:   return "BorderThicknessLeft";
		case UiElemConfigField_BorderThicknessTop:    return "BorderThicknessTop";
		case UiElemConfigField_BorderThicknessRight:  return "BorderThicknessRight";
		case UiElemConfigField_BorderThicknessBottom: return "BorderThicknessBottom";
		case UiElemConfigField_BorderColor:           return "BorderColor";
		case UiElemConfigField_FloatingType:          return "FloatingType";
		case UiElemConfigField_FloatingOffsetX:       return "FloatingOffsetX";
		case UiElemConfigField_FloatingOffsetY:       return "FloatingOffsetY";
		case UiElemConfigField_FloatingParentAttach:  return "FloatingParentAttach";
		case UiElemConfigField_FloatingElemAttach:    return "FloatingElemAttach";
		case UiElemConfigField_RendererParams:        return "RendererParams";
		case UiElemConfigField_ThemerParams:          return "ThemerParams";
		case UiElemConfigField_All:                   return "All";
		default: return UNKNOWN_STR;
	}
}
#endif

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

//NOTE: Returning false makes the element disappear and it's child scope not run
#define UI_THEMER_CALLBACK_DEF(functionName) bool functionName(struct UiContext* context, UiElement* element, void* userPntr)
typedef UI_THEMER_CALLBACK_DEF(UiThemerCallback_f);

typedef plex UiThemer UiThemer;
plex UiThemer
{
	uxx id;
	bool isActive;
	void* userPntr;
	UiThemerCallback_f* callback;
};

typedef plex BasicUiThemerOptions BasicUiThemerOptions;
plex BasicUiThemerOptions
{
	u32 fields;
	UiElemConfig config;
};

typedef plex UiThemerRegistry UiThemerRegistry;
plex UiThemerRegistry
{
	Arena* arena;
	uxx nextThemerId;
	VarArray themers;
	VarArray basicOptions; //BasicUiThemerOptions
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
	UiThemerRegistry themers;
	
	uxx currentElementIndex;
	uxx numTopLevelElements; //TODO: Remove me
	VarArray elements; //UiElement
	
	UiRenderList renderList; //allocated from frameArena
};

// +--------------------------------------------------------------+
// |                Forward Function Declarations                 |
// +--------------------------------------------------------------+
// ui_system.h:
PIG_CORE_INLINE UiId CalcUiId(UiId baseId, Str8 idString, uxx index);
PIG_CORE_INLINE UiId PrintUiId(UiId baseId, uxx index, const char* formatString, ...);
// ui_system_theming.h:
PIG_CORE_INLINE void FreeUiThemerRegistry(UiThemerRegistry* registry);
void InitUiThemerRegistry(Arena* arena, UiThemerRegistry* registryOut);
void UiThemerRegistryStartFrame(UiThemerRegistry* registry);
bool RunUiThemerCallbacks(UiThemerRegistry* registry, UiContext* context, UiElement* element);

#endif //BUILD_WITH_PIG_UI

#endif //  _UI_SYSTEM_CORE_H
