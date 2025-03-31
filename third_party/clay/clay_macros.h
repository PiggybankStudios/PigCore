/*
File:   clay_macros.h
Author: Taylor Robbins
Date:   03\31\2025
Description:
	** Holds all the macros (both public and private) for clay code
	** Was a piece of original clay.h (zlib LICENSE attached below)
*/

#ifndef _CLAY_MACROS_H
#define _CLAY_MACROS_H

// +--------------------------------------------------------------+
// |                       Public Macro API                       |
// +--------------------------------------------------------------+

#define CLAY_TEXT_CONFIG(...) Clay__StoreTextElementConfig(CLAY__CONFIG_WRAPPER(Clay_TextElementConfig, __VA_ARGS__))

#define CLAY_BORDER_OUTSIDE(widthValue) {widthValue, widthValue, widthValue, widthValue, 0}
#define CLAY_BORDER_ALL(widthValue) {widthValue, widthValue, widthValue, widthValue, widthValue}

#define CLAY_CORNER_RADIUS(radius) (NEW_STRUCT(Clay_CornerRadius) { radius, radius, radius, radius })

#define CLAY_PADDING_ALL(padding) CLAY__CONFIG_WRAPPER(Clay_Padding, { padding, padding, padding, padding })

#define CLAY_SIZING_FIT(...) (NEW_STRUCT(Clay_SizingAxis) { .size = { .minMax = { __VA_ARGS__ } }, .type = CLAY__SIZING_TYPE_FIT })
#define CLAY_SIZING_GROW(...) (NEW_STRUCT(Clay_SizingAxis) { .size = { .minMax = { __VA_ARGS__ } }, .type = CLAY__SIZING_TYPE_GROW })
#define CLAY_SIZING_FIXED(fixedSize) (NEW_STRUCT(Clay_SizingAxis) { .size = { .minMax = { fixedSize, fixedSize } }, .type = CLAY__SIZING_TYPE_FIXED })
#define CLAY_SIZING_PERCENT(percentOfParent) (NEW_STRUCT(Clay_SizingAxis) { .size = { .percent = (percentOfParent) }, .type = CLAY__SIZING_TYPE_PERCENT })

#define CLAY_ID(label) CLAY_IDI(label, 0)
#define CLAY_IDI(label, index) Clay__HashString(CLAY_STRING(label), index, 0)
#define CLAY_ID_LOCAL(label) CLAY_IDI_LOCAL(label, 0)
#define CLAY_IDI_LOCAL(label, index) Clay__HashString(CLAY_STRING(label), index, Clay__GetParentElementId())

// Note: If an error led you here, it's because CLAY_STRING can only be used with string literals, i.e. CLAY_STRING("SomeString") and not CLAY_STRING(yourString)
#define CLAY_STRING(stringLiteral) NewStr8(StrLitLength(CheckStrLit(stringLiteral)), (stringLiteral))
#define CLAY_STRING_CONST(string) { .length = StrLitLength(CheckStrLit(string)), .chars = (string) }

/* This macro looks scary on the surface, but is actually quite simple.
  It turns a macro call like this:

  CLAY({
	.id = CLAY_ID("Container"),
	.backgroundColor = { 255, 200, 200, 255 }
  }) {
	  ...children declared here
  }

  Into calls like this:

  Clay_OpenElement();
  Clay_ConfigureOpenElement((Clay_ElementDeclaration) {
	.id = CLAY_ID("Container"),
	.backgroundColor = { 255, 200, 200, 255 }
  });
  ...children declared here
  Clay_CloseElement();

  The for loop will only ever run a single iteration, putting Clay__CloseElement() in the increment of the loop
  means that it will run after the body - where the children are declared. It just exists to make sure you don't forget
  to call Clay_CloseElement().
*/
static u8 CLAY__ELEMENT_DEFINITION_LATCH;
#define CLAY(...)                                                                                                                                           \
	for (                                                                                                                                                   \
		CLAY__ELEMENT_DEFINITION_LATCH = (Clay__OpenElement(), Clay__ConfigureOpenElement(CLAY__CONFIG_WRAPPER(Clay_ElementDeclaration, __VA_ARGS__)), 0);  \
		CLAY__ELEMENT_DEFINITION_LATCH < 1;                                                                                                                 \
		CLAY__ELEMENT_DEFINITION_LATCH=1, Clay__CloseElement()                                                                                              \
	)

// These macros exist to allow the CLAY() macro to be called both with an inline struct definition, such as
// CLAY({ .id = something... });
// As well as by passing a predefined declaration struct
// Clay_ElementDeclaration declarationStruct = ...
// CLAY(declarationStruct);
#define CLAY__WRAPPER_TYPE(type) Clay__##type##Wrapper
#define CLAY__WRAPPER_STRUCT(type) typedef struct { type wrapped; } CLAY__WRAPPER_TYPE(type)
#define CLAY__CONFIG_WRAPPER(type, ...) (NEW_STRUCT(CLAY__WRAPPER_TYPE(type)) { __VA_ARGS__ }).wrapped

#define CLAY_TEXT(text, textConfig) Clay__OpenTextElement(text, textConfig)

#endif //  _CLAY_MACROS_H

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
