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
#include "file_fmt/file_fmt_sprite_sheet.h"
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

#define PIG_UI_DEFAULT_FLOATING_ELEM_DEPTH_OFFSET -0.0001f
#define PIG_UI_ROOT_ID_STR "root"
#define PIG_UI_ROOT_DEPTH 1.0f

#define UI_DEPTH_DEFAULT  0.0f
#define UI_DEPTH_ZERO     0.0001f

#define PigUiDefaultColor_Value TransparentBlack_Value
#define PigUiDefaultColor       TransparentBlack

#define UI_SCROLL_LAG_DEFAULT_DIVISOR 10.0f
#define UI_SCROLL_LAG_DEFAULT 0.0f
#define UI_SCROLL_LAG_NONE    -1.0f
#define UI_SCROLL_GOTO_SNAP_DISTANCE 0.5f //px

#define UiSide_TopLeft      Dir2Ex_TopLeft
#define UiSide_TopCenter    Dir2Ex_Up
#define UiSide_TopRight     Dir2Ex_TopRight
#define UiSide_CenterLeft   Dir2Ex_Left
#define UiSide_Center       Dir2Ex_None
#define UiSide_CenterRight  Dir2Ex_Right
#define UiSide_BottomLeft   Dir2Ex_BottomLeft
#define UiSide_BottomCenter Dir2Ex_Down
#define UiSide_BottomRight  Dir2Ex_BottomRight

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
#define UiId_Root_U64   0x6D216EB70FEE0D58ull
#define UiId_Root       MakeUiId(UiId_Root_U64, StrLit(PIG_UI_ROOT_ID_STR), PIG_UI_ID_INDEX_NONE)
#define UiId_Root_Const MakeUiId_Const(UiId_Root_U64, StrLit_Const(PIG_UI_ROOT_ID_STR), PIG_UI_ID_INDEX_NONE)

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
	UiSizingType_TextClip,
	UiSizingType_TextWrap,
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
		case UiSizingType_TextClip:     return "TextClip";
		case UiSizingType_TextWrap:     return "TextWrap";
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
	plex { UiSizingAxis horizontal, vertical; };
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
#define UI_TEXT_WRAP(minWidth)          { .x={.type=UiSizingType_TextWrap, .value=(minWidth)}, .y={.type=UiSizingType_TextWrap} }
#define UI_TEXT_CLIP(minWidth)          { .x={.type=UiSizingType_TextClip, .value=(minWidth)}, .y={.type=UiSizingType_TextClip} }
#define UI_TEXT_FULL()                  UI_TEXT_CLIP(-1.0f)

typedef enum UiAlign UiAlign;
enum UiAlign
{
	UiAlign_Center = 0,
	UiAlign_Left,
	UiAlign_Right,
	UiAlign_Count,
	UiAlign_Default = UiAlign_Center,
	UiAlign_Top = UiAlign_Left,
	UiAlign_Bottom = UiAlign_Right,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetUiAlignStr(UiAlign enumValue);
#else
PEXP const char* GetUiAlignStr(UiAlign enumValue)
{
	switch (enumValue)
	{
		case UiAlign_Center: return "Center(Default)";
		case UiAlign_Left:   return "Left(Top)";
		case UiAlign_Right:  return "Right(Bottom)";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef car UiAlignment UiAlignment;
car UiAlignment
{
	UiAlign axis[2];
	plex { UiAlign x, y; };
	plex { UiAlign horizontal, vertical; };
};
#define UI_ALIGN_CENTER()         { .x=UiAlign_Center, .y=UiAlign_Center }
#define UI_ALIGN_LEFT_CENTER()    { .x=UiAlign_Left,   .y=UiAlign_Center }
#define UI_ALIGN_RIGHT_CENTER()   { .x=UiAlign_Right,  .y=UiAlign_Center }
#define UI_ALIGN_TOP_CENTER()     { .x=UiAlign_Center, .y=UiAlign_Top    }
#define UI_ALIGN_BOTTOM_CENTER()  { .x=UiAlign_Center, .y=UiAlign_Bottom }
#define UI_ALIGN_TOP_LEFT()       { .x=UiAlign_Left,   .y=UiAlign_Top    }
#define UI_ALIGN_TOP_RIGHT()      { .x=UiAlign_Right,  .y=UiAlign_Top    }
#define UI_ALIGN_BOTTOM_LEFT()    { .x=UiAlign_Left,   .y=UiAlign_Bottom }
#define UI_ALIGN_BOTTOM_RIGHT()   { .x=UiAlign_Right,  .y=UiAlign_Bottom }

typedef plex UiScrollingAxis UiScrollingAxis;
plex UiScrollingAxis
{
	bool enabled;
	r32 lag; //divisor
};

typedef car UiScrolling UiScrolling;
car UiScrolling
{
	UiScrollingAxis axis[2];
	plex { UiScrollingAxis x; UiScrollingAxis y; };
	plex { UiScrollingAxis horizontal; UiScrollingAxis vertical; };
};
#define UI_SCROLL_HORIZONTAL()               { .x={ .enabled=true,  .lag=UI_SCROLL_LAG_DEFAULT }, .y={ .enabled=false, .lag=0.0f                  } }
#define UI_SCROLL_VERTICAL()                 { .x={ .enabled=false, .lag=0.0f                  }, .y={ .enabled=true,  .lag=UI_SCROLL_LAG_DEFAULT } }
#define UI_SCROLL_BOTH()                     { .x={ .enabled=true,  .lag=UI_SCROLL_LAG_DEFAULT }, .y={ .enabled=true,  .lag=UI_SCROLL_LAG_DEFAULT } }
#define UI_SCROLL_HORIZONTAL_LAG(lagDivisor) { .x={ .enabled=true,  .lag=(lagDivisor)          }, .y={ .enabled=false, .lag=0.0f                  } }
#define UI_SCROLL_VERTICAL_LAG(lagDivisor)   { .x={ .enabled=false, .lag=0.0f                  }, .y={ .enabled=true,  .lag=(lagDivisor)          } }
#define UI_SCROLL_BOTH_LAG(lagDivisor)       { .x={ .enabled=true,  .lag=(lagDivisor)          }, .y={ .enabled=true,  .lag=(lagDivisor)          } }
#define UI_SCROLL_HORIZONTAL_NO_LAG()        { .x={ .enabled=true,  .lag=UI_SCROLL_LAG_NONE    }, .y={ .enabled=false, .lag=0.0f                  } }
#define UI_SCROLL_VERTICAL_NO_LAG()          { .x={ .enabled=false, .lag=0.0f                  }, .y={ .enabled=true,  .lag=UI_SCROLL_LAG_NONE    } }
#define UI_SCROLL_BOTH_NO_LAG()              { .x={ .enabled=true,  .lag=UI_SCROLL_LAG_NONE    }, .y={ .enabled=true,  .lag=UI_SCROLL_LAG_NONE    } }

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
		default: return UNKNOWN_STR;
	}
}
#endif

typedef plex UiFloatingConfig UiFloatingConfig;
plex UiFloatingConfig
{
	UiFloatingType type;
	v2 offset; // This is not multiplied by UI scale (unlike other UiElemConfig sizing/padding members)
	UiId attachId;
	Dir2Ex parentSide;
	Dir2Ex elemSide;
};

typedef plex UiPadding UiPadding;
plex UiPadding
{
	v4r inner; //space between our bounds and our childrens' bounds
	v4r outer; //space between allocated area and our bounds
	r32 child; //space in-between each child, along the layout direction
};

typedef enum UiConditionType UiConditionType;
enum UiConditionType
{
	UiConditionType_None = 0,
	UiConditionType_MouseHover,
	UiConditionType_MouseLeftClicked,
	UiConditionType_MouseLeftClickStart,
	UiConditionType_MouseRightClicked,
	UiConditionType_MouseRightClickStart,
	UiConditionType_MouseMiddleClicked,
	UiConditionType_MouseMiddleClickStart,
	UiConditionType_Count,
	UiConditionType_Default = UiConditionType_None,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetUiConditionTypeStr(UiConditionType enumValue);
#else
PEXP const char* GetUiConditionTypeStr(UiConditionType enumValue)
{
	switch (enumValue)
	{
		case UiConditionType_None:                  return "None(Default)";
		case UiConditionType_MouseHover:            return "MouseHover";
		case UiConditionType_MouseLeftClicked:      return "MouseLeftClicked";
		case UiConditionType_MouseLeftClickStart:   return "MouseLeftClickStart";
		case UiConditionType_MouseRightClicked:     return "MouseRightClicked";
		case UiConditionType_MouseRightClickStart:  return "MouseRightClickStart";
		case UiConditionType_MouseMiddleClicked:    return "MouseMiddleClicked";
		case UiConditionType_MouseMiddleClickStart: return "MouseMiddleClickStart";
		default: return UNKNOWN_STR;
	}
}
#endif

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
	bool dontSizeToImage;
	UiAlignment alignment;
	UiScrolling scrolling;
	bool clipChildren; //this is assumed true if scrolling is enable, in which case this has no effect
	r32 depth;
	Color32 color;
	Color32 colorRecursive; //This color is multiplied through all children
	Texture* texture;
	bool repeatingTexture;
	rec textureSourceRec;
	SpriteSheet* spriteSheet;
	v2i sheetCell;
	UiPadding padding;
	v4r borderThickness;
	Color32 borderColor;
	r32 borderDepth; //0.0f will result in borderDepth inheriting value of depth
	UiFloatingConfig floating;
	UiConditionType condition;
	bool mousePassthrough;
	bool strictHover; //this element is not considered hovered if any of it's child elements is hovered over
	Str8 text;
	RichStr richText;
	Color32 textColor;
	r32 textWrapWidth;
	PigFont* font;
	r32 fontSize;
	u8 fontStyle;
	
	//These types can contain different things for each application, see the description near the top of the file
	UiRendererParameters renderer;
	UiThemerParameters themer;
};

//When configuring an element we often use the 0 value as a "default". So a color of "transparent black" actually means the default color, which is fully opaque white
#define UiConfigColorToActualColor(color) (((color).valueU32 != PigUiDefaultColor_Value) ? (color) : White)
#define UiConfigTextColorToActualColor(color) (((color).valueU32 != PigUiDefaultColor_Value) ? (color) : Black)

// This is a bitwise enum where each bit represents a single "field" in the UiElemConfig structure above
// Some values are aliases for combinations of other values, like Margins is MarginLeft|MarginTop|MarginRight|MarginBottom
// This is useful when we want to explicitly list which fields are overridden, esp. for things like UiThemers
typedef u64 UiElemConfigField;
#define UiElemConfigField_None                  0x00000000ull
#define UiElemConfigField_Id                    ((1ull) << 0ull)
#define UiElemConfigField_GlobalId              ((1ull) << 1ull)
#define UiElemConfigField_Direction             ((1ull) << 2ull)
#define UiElemConfigField_SizingTypeX           ((1ull) << 3ull)
#define UiElemConfigField_SizingValueX          ((1ull) << 4ull)
#define UiElemConfigField_SizingTypeY           ((1ull) << 5ull)
#define UiElemConfigField_SizingValueY          ((1ull) << 6ull)
#define UiElemConfigField_DontSizeToImage       ((1ull) << 7ull)
#define UiElemConfigField_AlignmentX            ((1ull) << 8ull)
#define UiElemConfigField_AlignmentY            ((1ull) << 9ull)
#define UiElemConfigField_ScrollingX            ((1ull) << 10ull)
#define UiElemConfigField_ScrollingLagX         ((1ull) << 11ull)
#define UiElemConfigField_ScrollingY            ((1ull) << 12ull)
#define UiElemConfigField_ScrollingLagY         ((1ull) << 13ull)
#define UiElemConfigField_ClipChildren          ((1ull) << 14ull)
#define UiElemConfigField_Depth                 ((1ull) << 15ull)
#define UiElemConfigField_Color                 ((1ull) << 16ull)
#define UiElemConfigField_ColorRecursive        ((1ull) << 17ull)
#define UiElemConfigField_Texture               ((1ull) << 18ull)
#define UiElemConfigField_RepeatingTexture      ((1ull) << 19ull)
#define UiElemConfigField_TextureSourceRec      ((1ull) << 20ull)
#define UiElemConfigField_SpriteSheet           ((1ull) << 21ull)
#define UiElemConfigField_SheetCell             ((1ull) << 22ull)
#define UiElemConfigField_InnerPaddingLeft      ((1ull) << 23ull)
#define UiElemConfigField_InnerPaddingTop       ((1ull) << 24ull)
#define UiElemConfigField_InnerPaddingRight     ((1ull) << 25ull)
#define UiElemConfigField_InnerPaddingBottom    ((1ull) << 26ull)
#define UiElemConfigField_OuterPaddingLeft      ((1ull) << 27ull)
#define UiElemConfigField_OuterPaddingTop       ((1ull) << 28ull)
#define UiElemConfigField_OuterPaddingRight     ((1ull) << 29ull)
#define UiElemConfigField_OuterPaddingBottom    ((1ull) << 30ull)
#define UiElemConfigField_ChildPadding          ((1ull) << 31ull)
#define UiElemConfigField_BorderThicknessLeft   ((1ull) << 32ull)
#define UiElemConfigField_BorderThicknessTop    ((1ull) << 33ull)
#define UiElemConfigField_BorderThicknessRight  ((1ull) << 34ull)
#define UiElemConfigField_BorderThicknessBottom ((1ull) << 35ull)
#define UiElemConfigField_BorderColor           ((1ull) << 36ull)
#define UiElemConfigField_BorderDepth           ((1ull) << 37ull)
#define UiElemConfigField_FloatingType          ((1ull) << 38ull)
#define UiElemConfigField_FloatingOffsetX       ((1ull) << 39ull)
#define UiElemConfigField_FloatingOffsetY       ((1ull) << 40ull)
#define UiElemConfigField_FloatingAttachId      ((1ull) << 41ull)
#define UiElemConfigField_FloatingParentSide    ((1ull) << 42ull)
#define UiElemConfigField_FloatingElemSide      ((1ull) << 43ull)
#define UiElemConfigField_Condition             ((1ull) << 44ull)
#define UiElemConfigField_MousePassthrough      ((1ull) << 45ull)
#define UiElemConfigField_StrictHover           ((1ull) << 46ull)
#define UiElemConfigField_Text                  ((1ull) << 47ull)
#define UiElemConfigField_RichText              ((1ull) << 48ull)
#define UiElemConfigField_TextColor             ((1ull) << 49ull)
#define UiElemConfigField_TextWrapWidth         ((1ull) << 50ull)
#define UiElemConfigField_Font                  ((1ull) << 51ull)
#define UiElemConfigField_FontSize              ((1ull) << 52ull)
#define UiElemConfigField_FontStyle             ((1ull) << 53ull)
#define UiElemConfigField_RendererParams        ((1ull) << 54ull) // NOTE: Fields inside UiRendererParameters struct are not represented individually
#define UiElemConfigField_ThemerParams          ((1ull) << 55ull) // NOTE: Fields inside UiThemerParameters struct are not represented individually
#define UiElemConfigField_Count                 56
#define UiElemConfigField_All                   (((1ull) << UiElemConfigField_Count)-1)
#define UiElemConfigField_Sizing                (UiElemConfigField_SizingTypeX|UiElemConfigField_SizingTypeY|UiElemConfigField_SizingValueX|UiElemConfigField_SizingValueY)
#define UiElemConfigField_SizingX               (UiElemConfigField_SizingTypeX|UiElemConfigField_SizingValueX)
#define UiElemConfigField_SizingY               (UiElemConfigField_SizingTypeY|UiElemConfigField_SizingValueY)
#define UiElemConfigField_Alignment             (UiElemConfigField_AlignmentX|UiElemConfigField_AlignmentY)
#define UiElemConfigField_Scrolling             (UiElemConfigField_ScrollingX|UiElemConfigField_ScrollingY)
#define UiElemConfigField_ScrollingLag          (UiElemConfigField_ScrollingLagX|UiElemConfigField_ScrollingLagY)
#define UiElemConfigField_InnerPadding          (UiElemConfigField_InnerPaddingLeft|UiElemConfigField_InnerPaddingTop|UiElemConfigField_InnerPaddingRight|UiElemConfigField_InnerPaddingBottom)
#define UiElemConfigField_OuterPadding          (UiElemConfigField_OuterPaddingLeft|UiElemConfigField_OuterPaddingTop|UiElemConfigField_OuterPaddingRight|UiElemConfigField_OuterPaddingBottom)
#define UiElemConfigField_Padding               (UiElemConfigField_InnerPadding|UiElemConfigField_OuterPadding|UiElemConfigField_ChildPadding)
#define UiElemConfigField_BorderThickness       (UiElemConfigField_BorderThicknessLeft|UiElemConfigField_BorderThicknessTop|UiElemConfigField_BorderThicknessRight|UiElemConfigField_BorderThicknessBottom)
#define UiElemConfigField_Floating              (UiElemConfigField_FloatingType|UiElemConfigField_FloatingOffsetX|UiElemConfigField_FloatingOffsetY|UiElemConfigField_FloatingAttachId|UiElemConfigField_FloatingParentSide|UiElemConfigField_FloatingElemSide)
#define UiElemConfigField_FloatingOffset        (UiElemConfigField_FloatingOffsetX|UiElemConfigField_FloatingOffsetY)

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
		case UiElemConfigField_SizingValueX:          return "SizingValueX";
		case UiElemConfigField_SizingTypeY:           return "SizingTypeY";
		case UiElemConfigField_SizingValueY:          return "SizingValueY";
		case UiElemConfigField_DontSizeToImage:       return "DontSizeToImage";
		case UiElemConfigField_AlignmentX:            return "AlignmentX";
		case UiElemConfigField_AlignmentY:            return "AlignmentY";
		case UiElemConfigField_ScrollingX:            return "ScrollingX";
		case UiElemConfigField_ScrollingLagX:         return "ScrollingLagX";
		case UiElemConfigField_ScrollingY:            return "ScrollingY";
		case UiElemConfigField_ScrollingLagY:         return "ScrollingLagY";
		case UiElemConfigField_ClipChildren:          return "ClipChildren";
		case UiElemConfigField_Depth:                 return "Depth";
		case UiElemConfigField_Color:                 return "Color";
		case UiElemConfigField_ColorRecursive:        return "ColorRecursive";
		case UiElemConfigField_Texture:               return "Texture";
		case UiElemConfigField_RepeatingTexture:      return "RepeatingTexture";
		case UiElemConfigField_TextureSourceRec:      return "TextureSourceRec";
		case UiElemConfigField_SpriteSheet:           return "SpriteSheet";
		case UiElemConfigField_SheetCell:             return "SheetCell";
		case UiElemConfigField_InnerPaddingLeft:      return "InnerPaddingLeft";
		case UiElemConfigField_InnerPaddingTop:       return "InnerPaddingTop";
		case UiElemConfigField_InnerPaddingRight:     return "InnerPaddingRight";
		case UiElemConfigField_InnerPaddingBottom:    return "InnerPaddingBottom";
		case UiElemConfigField_OuterPaddingLeft:      return "OuterPaddingLeft";
		case UiElemConfigField_OuterPaddingTop:       return "OuterPaddingTop";
		case UiElemConfigField_OuterPaddingRight:     return "OuterPaddingRight";
		case UiElemConfigField_OuterPaddingBottom:    return "OuterPaddingBottom";
		case UiElemConfigField_ChildPadding:          return "ChildPadding";
		case UiElemConfigField_BorderThicknessLeft:   return "BorderThicknessLeft";
		case UiElemConfigField_BorderThicknessTop:    return "BorderThicknessTop";
		case UiElemConfigField_BorderThicknessRight:  return "BorderThicknessRight";
		case UiElemConfigField_BorderThicknessBottom: return "BorderThicknessBottom";
		case UiElemConfigField_BorderColor:           return "BorderColor";
		case UiElemConfigField_BorderDepth:           return "BorderDepth";
		case UiElemConfigField_FloatingType:          return "FloatingType";
		case UiElemConfigField_FloatingOffsetX:       return "FloatingOffsetX";
		case UiElemConfigField_FloatingOffsetY:       return "FloatingOffsetY";
		case UiElemConfigField_FloatingAttachId:      return "FloatingAttachId";
		case UiElemConfigField_FloatingParentSide:    return "FloatingParentSide";
		case UiElemConfigField_FloatingElemSide:      return "FloatingElemSide";
		case UiElemConfigField_Condition:             return "Condition";
		case UiElemConfigField_MousePassthrough:      return "MousePassthrough";
		case UiElemConfigField_StrictHover:           return "StrictHover";
		case UiElemConfigField_Text:                  return "Text";
		case UiElemConfigField_RichText:              return "RichText";
		case UiElemConfigField_TextColor:             return "TextColor";
		case UiElemConfigField_TextWrapWidth:         return "TextWrapWidth";
		case UiElemConfigField_Font:                  return "Font";
		case UiElemConfigField_FontSize:              return "FontSize";
		case UiElemConfigField_FontStyle:             return "FontStyle";
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
	
	uxx treeDepth; //How many parents this element has
	uxx floatDepth; //How many of this elements parents are floating
	uxx elementIndex; //What's this element's index in the UiContext->elements VarArray
	uxx siblingIndex; //which index child is this element amongst it's siblings
	uxx parentIndex; //What is the index of the parent element in the UiContext>elements VarArray
	UiId parentId;
	bool isOpen; //are we currently adding children to this element
	bool runChildCode;
	uxx numChildren; //How many direct children does this element have
	uxx numNonFloatingChildren; //How many direct children does this element have that are NOT floating
	uxx numDescendents; //How many elements after this one are a descendent of this element (children, grandchildren, etc. Useful for knowing how many elements to skip over if we don't want to walk the tree of elements below this one)
	
	v2 minimumSize;
	v2 preferredSize;
	rec clipRec;
	rec layoutRec;
	v2 contentSize;
	v2 scroll;
	v2 scrollGoto;
	v2 scrollMax;
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
	bool applyToNonText;
	bool applyToText;
	u64 fields;
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
	UiRenderCmdType_RichText,
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
		case UiRenderCmdType_RichText:  return "RichText";
		case UiRenderCmdType_Scissor:   return "Scissor";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef plex UiRenderCmd UiRenderCmd;
plex UiRenderCmd
{
	UiRenderCmdType type;
	uxx srcElementIndex;
	UiId srcElementId;
	r32 depth;
	Color32 color;
	rec clipRec;
	car
	{
		// +==============================+
		// |  UiRenderCmdType_Rectangle   |
		// +==============================+
		plex
		{
			rec rectangle;
			v4r cornerRadius;
			v4r borderThickness;
			Color32 borderColor;
			Texture* texture;
			rec sourceRec;
		} rectangle;
		
		// +==============================+
		// |     UiRenderCmdType_Text     |
		// +==============================+
		plex
		{
			v2 position;
			PigFont* font;
			r32 fontSize;
			u8 fontStyle;
			r32 wrapWidth;
			Str8 text;
			//TODO: clipRec?
		} text;
		
		// +==============================+
		// |   UiRenderCmdType_RichText   |
		// +==============================+
		plex
		{
			v2 position;
			PigFont* font;
			r32 fontSize;
			u8 fontStyle;
			r32 wrapWidth;
			RichStr text;
			//TODO: clipRec?
		} richText;
		
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
	v2 screenSize;
	r32 scale;
	u64 programTime;
	r32 defaultScrollLagDivisor;
	KeyboardState* keyboard;
	MouseState* mouse;
	TouchscreenState* touchscreen;
	UiThemerRegistry themers;
	
	uxx currentElementIndex;
	uxx numTopLevelElements; //TODO: Remove me
	VarArray elements; //UiElement
	
	VarArray prevElements; //UiElement
	UiId mouseHoveredId;
	UiId mouseHoveredLocalId;
	UiId clickStartHoveredId[MouseBtn_Count];
	UiId clickStartHoveredLocalId[MouseBtn_Count];
	
	bool hasDoneOneLayout;
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
