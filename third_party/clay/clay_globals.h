/*
File:   clay_globals.h
Author: Taylor Robbins
Date:   03\31\2025
Description:
	** Contains all the extern globals, and also static variables that clay uses to track state
	** Was a piece of original clay.h (zlib LICENSE attached below)
*/

#ifndef _CLAY_GLOBALS_H
#define _CLAY_GLOBALS_H

MAYBE_START_EXTERN_C

typedef struct Clay_Context Clay_Context;

// +--------------------------------------------------------------+
// |               Public Globals (Used by Macros)                |
// +--------------------------------------------------------------+
#ifndef CLAY_IMPLEMENTATION
extern Clay_LayoutConfig CLAY_LAYOUT_DEFAULT;
extern Color32 Clay__debugViewHighlightColor;
extern u32 Clay__debugViewWidth;
#else
Clay_LayoutConfig CLAY_LAYOUT_DEFAULT = ZEROED;
u32 Clay__debugViewWidth = 400;
Color32 Clay__debugViewHighlightColor = {.valueU32=0x64A8421C}; //(168, 66, 28, 100)
#endif

// +--------------------------------------------------------------+
// |                       Private Globals                        |
// +--------------------------------------------------------------+
#ifdef CLAY_IMPLEMENTATION

Color32 Clay__Color_DEFAULT = ZEROED;
Clay_CornerRadius Clay__CornerRadius_DEFAULT = ZEROED;
Clay_BorderWidth Clay__BorderWidth_DEFAULT = ZEROED;

Clay_Context* Clay__currentContext;

i32 Clay__defaultMaxElementCount = 8192;
i32 Clay__defaultMaxMeasureTextWordCacheCount = 16384;

Clay__Warning CLAY__WARNING_DEFAULT = ZEROED;

void Clay__ErrorHandlerFunctionDefault(Clay_ErrorData errorText)
{
	UNUSED(errorText);
}

#ifdef CLAY_WASM
	__attribute__((import_module("clay"), import_name("measureTextFunction"))) ClayMeasureText_f* Clay__MeasureText;
	__attribute__((import_module("clay"), import_name("hashTextUserData"))) ClayHashTextUserData_f* Clay__HashTextUserData;
	__attribute__((import_module("clay"), import_name("queryScrollOffsetFunction"))) v2 Clay__QueryScrollOffset(u32 elementId, CLAY_QUERYSCROLL_USERDATA_TYPE userData);
#else
	ClayMeasureText_f* Clay__MeasureText;
	ClayHashTextUserData_f* Clay__HashTextUserData;
	v2 (*Clay__QueryScrollOffset)(u32 elementId, CLAY_QUERYSCROLL_USERDATA_TYPE userData);
#endif

// +==============================+
// |            Debug             |
// +==============================+
Color32 CLAY__DEBUGVIEW_COLOR_1 = {.valueU32=0x373A3834}; //(58, 56, 52, 55)
Color32 CLAY__DEBUGVIEW_COLOR_2 = {.valueU32=0xFF3E3C3A}; //(62, 60, 58, 255)
Color32 CLAY__DEBUGVIEW_COLOR_3 = {.valueU32=0xFF8D8587}; //(141, 133, 135, 255)
Color32 CLAY__DEBUGVIEW_COLOR_4 = {.valueU32=0xFFEEE2E7}; //(238, 226, 231, 255)
Color32 CLAY__DEBUGVIEW_COLOR_SELECTED_ROW = {.valueU32=0xFF66504E}; //(102, 80, 78, 255)
const i32 CLAY__DEBUGVIEW_ROW_HEIGHT = 30;
const i32 CLAY__DEBUGVIEW_OUTER_PADDING = 10;
const i32 CLAY__DEBUGVIEW_INDENT_WIDTH = 16;
Clay_TextElementConfig Clay__DebugView_TextNameConfig = {.textColor = {.valueU32=0xFFEEE2E7}, .fontSize = 16, .wrapMode = CLAY_TEXT_WRAP_NONE }; //textColor=(238, 226, 231, 255)
Clay_LayoutConfig Clay__DebugView_ScrollViewItemLayoutConfig = ZEROED;

#endif //PIG_CORE_IMPLEMENTATION

MAYBE_END_EXTERN_C

#endif //  _CLAY_GLOBALS_H

/*
LICENSE
zlib/libpng license

Copyright (c) 2024 Nic Barker

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the
use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software in a
	product, an acknowledgment in the product documentation would be
	appreciated but is not required.

	2. Altered source versions must be plainly marked as such, and must not
	be misrepresented as being the original software.

	3. This notice may not be removed or altered from any source
	distribution.
*/
