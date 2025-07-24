/*
File:   clay_api.h
Author: Taylor Robbins
Date:   03\31\2025
Description:
	** Contains all the types, enums, and function pointer types for clay code
	** Was a piece of original clay.h (zlib LICENSE attached below)
*/

#ifndef _CLAY_API_H
#define _CLAY_API_H

MAYBE_START_EXTERN_C

// Sets the state of the "pointer" (i.e. the mouse or touch) in Clay's internal data. Used for detecting and responding to mouse events in the debug view,
// as well as for Clay_Hovered() and scroll element handling.
CLAY_DECOR void Clay_SetPointerState(v2 position, bool pointerDown);
// Initialize Clay's internal arena and setup required data before layout can begin. Only needs to be called once.
// - arena can be created using Clay_CreateArenaWithCapacityAndMemory()
// - layoutDimensions are the initial bounding dimensions of the layout (i.e. the screen width and height for a full screen layout)
// - errorHandler is used by Clay to inform you if something has gone wrong in configuration or layout.
CLAY_DECOR Clay_Context* Clay_Initialize(Arena* arena, v2 layoutDimensions, Clay_ErrorHandler errorHandler);
// Returns the Context that clay is currently using. Used when using multiple instances of clay simultaneously.
CLAY_DECOR Clay_Context* Clay_GetCurrentContext(void);
// Sets the context that clay will use to compute the layout.
// Used to restore a context saved from Clay_GetCurrentContext when using multiple instances of clay simultaneously.
CLAY_DECOR void Clay_SetCurrentContext(Clay_Context* context);
// Updates the state of Clay's internal scroll data, updating scroll content positions if scrollDelta is non zero, and progressing momentum scrolling.
// - enableDragScrolling when set to true will enable mobile device like "touch drag" scroll of scroll containers, including momentum scrolling after the touch has ended.
// - scrollDelta is the amount to scroll this frame on each axis in pixels.
// - deltaTime is the time in seconds since the last "frame" (scroll update)
// Returns true if a container is currently momentum scrolling (i.e. a touch sent it moving for some amount of time) or scrolling with lag (if scrollLag != 0). Useful for keeping the application from "going to sleep" while the scrolling happens
CLAY_DECOR bool Clay_UpdateScrollContainers(bool enableDragScrolling, v2 scrollDelta, r32 deltaTime);
// Updates the layout dimensions in response to the window or outer container being resized.
CLAY_DECOR void Clay_SetLayoutDimensions(v2 dimensions);
// Called before starting any layout declarations.
CLAY_DECOR void Clay_BeginLayout(void);
// Called when all layout declarations are finished.
// Computes the layout and generates and returns the array of render commands to draw.
CLAY_DECOR Clay_RenderCommandArray Clay_EndLayout(void);
// Calculates a hash ID from the given idString.
// Generally only used for dynamic strings when CLAY_ID("stringLiteral") can't be used.
CLAY_DECOR Clay_ElementId Clay_GetElementId(Str8 idString);
// Calculates a hash ID from the given idString and index.
// - index is used to avoid constructing dynamic ID strings in loops.
// Generally only used for dynamic strings when CLAY_IDI("stringLiteral", index) can't be used.
CLAY_DECOR Clay_ElementId Clay_GetElementIdWithIndex(Str8 idString, u32 index);
// Returns layout data such as the final calculated bounding box for an element with a given ID.
// The returned Clay_ElementData contains a `found` bool that will be true if an element with the provided ID was found.
// This ID can be calculated either with CLAY_ID() for string literal IDs, or Clay_GetElementId for dynamic strings.
CLAY_DECOR Clay_ElementData Clay_GetElementData(Clay_ElementId id);
// Returns true if the pointer position provided by Clay_SetPointerState is within the current element's bounding box.
// Works during element declaration, e.g. CLAY({ .backgroundColor = Clay_Hovered() ? BLUE : RED });
CLAY_DECOR bool Clay_Hovered(void);
// Bind a callback that will be called when the pointer position provided by Clay_SetPointerState is within the current element's bounding box.
// - onHoverFunction is a function pointer to a user defined function.
// - userData is a pointer that will be transparently passed through when the onHoverFunction is called.
CLAY_DECOR void Clay_OnHover(void (*onHoverFunction)(Clay_ElementId elementId, Clay_PointerData pointerData, CLAY_ONHOVER_USERDATA_TYPE userData), CLAY_ONHOVER_USERDATA_TYPE userData);
// An imperative function that returns true if the pointer position provided by Clay_SetPointerState is within the element with the provided ID's bounding box.
// This ID can be calculated either with CLAY_ID() for string literal IDs, or Clay_GetElementId for dynamic strings.
CLAY_DECOR bool Clay_PointerOver(Clay_ElementId elementId);
// Returns data representing the state of the scrolling element with the provided ID.
// The returned Clay_ScrollContainerData contains a `found` bool that will be true if a scroll element was found with the provided ID.
// An imperative function that returns true if the pointer position provided by Clay_SetPointerState is within the element with the provided ID's bounding box.
// This ID can be calculated either with CLAY_ID() for string literal IDs, or Clay_GetElementId for dynamic strings.
// NOTE: If getConfig is true then Clay_GetScrollContainerData must be called AFTER the container has been declared this frame!
CLAY_DECOR Clay_ScrollContainerData Clay_GetScrollContainerData(Clay_ElementId id, bool getConfig);
// Binds a callback function that Clay will call to determine the dimensions of a given string slice.
// - measureTextFunction is a user provided function that adheres to the interface v2 (Str8 text, Clay_TextElementConfig* config, CLAY_MEASURE_USERDATA_TYPE userData);
// - userData is a pointer that will be transparently passed through when the measureTextFunction is called.
CLAY_DECOR void Clay_SetMeasureTextFunction(ClayMeasureText_f* measureTextFunction, CLAY_MEASURE_USERDATA_TYPE userData);
// Experimental - Used in cases where Clay needs to integrate with a system that manages its own scrolling containers externally.
// Please reach out if you plan to use this function, as it may be subject to change.
CLAY_DECOR void Clay_SetQueryScrollOffsetFunction(v2 (*queryScrollOffsetFunction)(u32 elementId, CLAY_QUERYSCROLL_USERDATA_TYPE userData), CLAY_QUERYSCROLL_USERDATA_TYPE userData);
// Enables and disables Clay's internal debug tools.
// This state is retained and does not need to be set each frame.
CLAY_DECOR void Clay_SetDebugModeEnabled(bool enabled);
// Returns true if Clay's internal debug tools are currently enabled.
CLAY_DECOR bool Clay_IsDebugModeEnabled(void);
// Enables and disables visibility culling. By default, Clay will not generate render commands for elements whose bounding box is entirely outside the screen.
CLAY_DECOR void Clay_SetCullingEnabled(bool enabled);
// Returns the maximum number of UI elements supported by Clay's current configuration.
CLAY_DECOR i32 Clay_GetMaxElementCount(void);
// Modifies the maximum number of UI elements supported by Clay's current configuration.
// This may require reallocating additional memory, and re-calling Clay_Initialize();
CLAY_DECOR void Clay_SetMaxElementCount(i32 maxElementCount);
// Returns the maximum number of measured "words" (whitespace seperated runs of characters) that Clay can store in its internal text measurement cache.
CLAY_DECOR i32 Clay_GetMaxMeasureTextCacheWordCount(void);
// Modifies the maximum number of measured "words" (whitespace seperated runs of characters) that Clay can store in its internal text measurement cache.
// This may require reallocating additional memory, and re-calling Clay_Initialize();
CLAY_DECOR void Clay_SetMaxMeasureTextCacheWordCount(i32 maxMeasureTextCacheWordCount);
// Resets Clay's internal text measurement cache, useful if memory to represent strings is being re-used.
// Similar behaviour can be achieved on an individual text element level by using Clay_TextElementConfig.hashStringContents
CLAY_DECOR void Clay_ResetMeasureTextCache(void);

// +============================================+
// | Internal API functions required by macros  |
// +============================================+

CLAY_DECOR void Clay__OpenElement(void);
CLAY_DECOR void Clay__ConfigureOpenElement(const Clay_ElementDeclaration config);
CLAY_DECOR void Clay__CloseElement(void);
CLAY_DECOR Clay_ElementId Clay__HashString(Str8 key, u32 offset, u32 seed);
CLAY_DECOR void Clay__OpenTextElement(Str8 text, Clay_TextElementConfig* textConfig);
CLAY_DECOR Clay_TextElementConfig* Clay__StoreTextElementConfig(Clay_TextElementConfig config);
CLAY_DECOR u32 Clay__GetParentElementId(void);

MAYBE_END_EXTERN_C

#endif //  _CLAY_API_H

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
