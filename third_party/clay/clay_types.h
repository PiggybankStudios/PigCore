/*
File:   clay_types.h
Author: Taylor Robbins
Date:   03\31\2025
Description:
	** Contains all the types, enums, and function pointer types for clay code
	** Was a piece of original clay.h (zlib LICENSE attached below)
*/

#ifndef _CLAY_TYPES_H
#define _CLAY_TYPES_H

MAYBE_START_EXTERN_C

// +==============================+
// |       Utility Structs        |
// +==============================+
// Primarily created via the CLAY_ID(), CLAY_IDI(), CLAY_ID_LOCAL() and CLAY_IDI_LOCAL() macros.
// Represents a hashed string ID used for identifying and finding specific clay UI elements, required
// by functions such as Clay_PointerOver() and Clay_GetElementData().
typedef struct Clay_ElementId Clay_ElementId;
struct Clay_ElementId
{
	u32 id; // The resulting hash generated from the other fields.
	u32 offset; // A numerical offset applied after computing the hash from stringId.
	u32 baseId; // A base hash value to start from, for example the parent element ID is used when calculating CLAY_ID_LOCAL().
	Str8 stringId; // The string id to hash.
};

// Controls the "radius", or corner rounding of elements, including rectangles, borders and images.
// The rounding is determined by drawing a circle inset into the element corner by (radius, radius) pixels.
typedef struct Clay_CornerRadius Clay_CornerRadius;
struct Clay_CornerRadius
{
	r32 topLeft;
	r32 topRight;
	r32 bottomLeft;
	r32 bottomRight;
};

// +==============================+
// |       Element Configs        |
// +==============================+
// Controls the direction in which child elements will be automatically laid out.
typedef CLAY_PACKED_ENUM Clay_LayoutDirection Clay_LayoutDirection;
CLAY_PACKED_ENUM Clay_LayoutDirection
{
	// (Default) Lays out child elements from left to right with increasing x.
	CLAY_LEFT_TO_RIGHT,
	// Lays out child elements from top to bottom with increasing y.
	CLAY_TOP_TO_BOTTOM,
};

// Controls the alignment along the x axis (horizontal) of child elements.
typedef CLAY_PACKED_ENUM Clay_LayoutAlignmentX Clay_LayoutAlignmentX;
CLAY_PACKED_ENUM Clay_LayoutAlignmentX
{
	// (Default) Aligns child elements to the left hand side of this element, offset by padding.Width.left
	CLAY_ALIGN_X_LEFT,
	// Aligns child elements to the right hand side of this element, offset by padding.Width.right
	CLAY_ALIGN_X_RIGHT,
	// Aligns child elements horizontally to the center of this element
	CLAY_ALIGN_X_CENTER,
};

// Controls the alignment along the y axis (vertical) of child elements.
typedef CLAY_PACKED_ENUM Clay_LayoutAlignmentY Clay_LayoutAlignmentY;
CLAY_PACKED_ENUM Clay_LayoutAlignmentY
{
	// (Default) Aligns child elements to the top of this element, offset by padding.Width.top
	CLAY_ALIGN_Y_TOP,
	// Aligns child elements to the bottom of this element, offset by padding.Width.bottom
	CLAY_ALIGN_Y_BOTTOM,
	// Aligns child elements vertiically to the center of this element
	CLAY_ALIGN_Y_CENTER,
};

// Controls how the element takes up space inside its parent container.
typedef CLAY_PACKED_ENUM Clay__SizingType Clay__SizingType;
CLAY_PACKED_ENUM Clay__SizingType
{
	// (default) Wraps tightly to the size of the element's contents.
	CLAY__SIZING_TYPE_FIT,
	// Expands along this axis to fill available space in the parent element, sharing it with other GROW elements.
	CLAY__SIZING_TYPE_GROW,
	// Expects 0-1 range. Clamps the axis size to a percent of the parent container's axis size minus padding and child gaps.
	CLAY__SIZING_TYPE_PERCENT,
	// Clamps the axis size to an exact size in pixels.
	CLAY__SIZING_TYPE_FIXED,
};

// Controls how child elements are aligned on each axis.
typedef struct Clay_ChildAlignment Clay_ChildAlignment;
struct Clay_ChildAlignment
{
	Clay_LayoutAlignmentX x; // Controls alignment of children along the x axis.
	Clay_LayoutAlignmentY y; // Controls alignment of children along the y axis.
};

// Controls the minimum and maximum size in pixels that this element is allowed to grow or shrink to,
// overriding sizing types such as FIT or GROW.
typedef struct Clay_SizingMinMax Clay_SizingMinMax;
struct Clay_SizingMinMax
{
	r32 min; // The smallest final size of the element on this axis will be this value in pixels.
	r32 max; // The largest final size of the element on this axis will be this value in pixels.
};

// Controls the sizing of this element along one axis inside its parent container.
typedef struct Clay_SizingAxis Clay_SizingAxis;
struct Clay_SizingAxis
{
	union {
		Clay_SizingMinMax minMax; // Controls the minimum and maximum size in pixels that this element is allowed to grow or shrink to, overriding sizing types such as FIT or GROW.
		r32 percent; // Expects 0-1 range. Clamps the axis size to a percent of the parent container's axis size minus padding and child gaps.
	} size;
	Clay__SizingType type; // Controls how the element takes up space inside its parent container.
};

// Controls the sizing of this element along one axis inside its parent container.
typedef struct Clay_Sizing Clay_Sizing;
struct Clay_Sizing
{
	Clay_SizingAxis width; // Controls the width sizing of the element, along the x axis.
	Clay_SizingAxis height;  // Controls the height sizing of the element, along the y axis.
};

// Controls "padding" in pixels, which is a gap between the bounding box of this element and where its children
// will be placed.
typedef struct Clay_Padding Clay_Padding;
struct Clay_Padding
{
	u16 left;
	u16 right;
	u16 top;
	u16 bottom;
};

CLAY__WRAPPER_STRUCT(Clay_Padding);

// Controls various settings that affect the size and position of an element, as well as the sizes and positions
// of any child elements.
typedef struct Clay_LayoutConfig Clay_LayoutConfig;
struct Clay_LayoutConfig
{
	Clay_Sizing sizing; // Controls the sizing of this element inside it's parent container, including FIT, GROW, PERCENT and FIXED sizing.
	Clay_Padding padding; // Controls "padding" in pixels, which is a gap between the bounding box of this element and where its children will be placed.
	u16 childGap; // Controls the gap in pixels between child elements along the layout axis (horizontal gap for LEFT_TO_RIGHT, vertical gap for TOP_TO_BOTTOM).
	Clay_ChildAlignment childAlignment; // Controls how child elements are aligned on each axis.
	Clay_LayoutDirection layoutDirection; // Controls the direction in which child elements will be automatically laid out.
};

CLAY__WRAPPER_STRUCT(Clay_LayoutConfig);

// Controls how text "wraps", that is how it is broken into multiple lines when there is insufficient horizontal space.
typedef CLAY_PACKED_ENUM Clay_TextElementConfigWrapMode Clay_TextElementConfigWrapMode;
CLAY_PACKED_ENUM Clay_TextElementConfigWrapMode
{
	// (default) breaks on whitespace characters.
	CLAY_TEXT_WRAP_WORDS,
	// Don't break on space characters, only on newlines.
	CLAY_TEXT_WRAP_NEWLINES,
	// Disable text wrapping entirely.
	CLAY_TEXT_WRAP_NONE,
};

// Controls how wrapped lines of text are horizontally aligned within the outer text bounding box.
typedef CLAY_PACKED_ENUM Clay_TextAlignment Clay_TextAlignment;
CLAY_PACKED_ENUM Clay_TextAlignment
{
	// (default) Horizontally aligns wrapped lines of text to the left hand side of their bounding box.
	CLAY_TEXT_ALIGN_LEFT,
	// Horizontally aligns wrapped lines of text to the center of their bounding box.
	CLAY_TEXT_ALIGN_CENTER,
	// Horizontally aligns wrapped lines of text to the right hand side of their bounding box.
	CLAY_TEXT_ALIGN_RIGHT,
	// Acts like ALIGN_LEFT, except that the boundingBox passed to the TEXT render command may be smaller than the measured text size. The renderer must then decide how to shorten the text to make it fit
	CLAY_TEXT_ALIGN_SHRINK,
};

// Controls various functionality related to text elements.
typedef struct Clay_TextElementConfig Clay_TextElementConfig;
struct Clay_TextElementConfig
{
	// The RGBA color of the font to render, conventionally specified as 0-255.
	Color32 textColor;
	// An integer transparently passed to Clay_MeasureText to identify the font to use.
	// The debug view will pass fontId = 0 for its internal text.
	u16 fontId;
	// Controls the size of the font. Handled by the function provided to Clay_MeasureText.
	u16 fontSize;
	// Controls extra horizontal spacing between characters. Handled by the function provided to Clay_MeasureText.
	u16 letterSpacing;
	// Controls additional vertical space between wrapped lines of text.
	u16 lineHeight;
	// Controls how text "wraps", that is how it is broken into multiple lines when there is insufficient horizontal space.
	// CLAY_TEXT_WRAP_WORDS (default) breaks on whitespace characters.
	// CLAY_TEXT_WRAP_NEWLINES doesn't break on space characters, only on newlines.
	// CLAY_TEXT_WRAP_NONE disables wrapping entirely.
	Clay_TextElementConfigWrapMode wrapMode;
	// Controls how wrapped lines of text are horizontally aligned within the outer text bounding box.
	// CLAY_TEXT_ALIGN_LEFT (default) - Horizontally aligns wrapped lines of text to the left hand side of their bounding box.
	// CLAY_TEXT_ALIGN_CENTER - Horizontally aligns wrapped lines of text to the center of their bounding box.
	// CLAY_TEXT_ALIGN_RIGHT - Horizontally aligns wrapped lines of text to the right hand side of their bounding box.
	Clay_TextAlignment textAlignment;
	// When set to true, clay will hash the entire text contents of this string as an identifier for its internal
	// text measurement cache, rather than just the pointer and length. This will incur significant performance cost for
	// long bodies of text.
	bool hashStringContents;
	// A pointer transparently passed through from the original element declaration.
	CLAY_TEXT_USERDATA_TYPE userData;
};

CLAY__WRAPPER_STRUCT(Clay_TextElementConfig);

// +==============================+
// |            Image             |
// +==============================+
// Controls various settings related to image elements.
typedef struct Clay_ImageElementConfig Clay_ImageElementConfig;
struct Clay_ImageElementConfig
{
	CLAY_IMAGEDATA_TYPE imageData; // A transparent pointer used to pass image data through to the renderer.
	v2 sourceDimensions; // The original dimensions of the source image, used to control aspect ratio.
};

CLAY__WRAPPER_STRUCT(Clay_ImageElementConfig);

// +==============================+
// |           Floating           |
// +==============================+
// Controls where a floating element is offset relative to its parent element.
// Note: see https://github.com/user-attachments/assets/b8c6dfaa-c1b1-41a4-be55-013473e4a6ce for a visual explanation.
typedef CLAY_PACKED_ENUM Clay_FloatingAttachPointType Clay_FloatingAttachPointType;
CLAY_PACKED_ENUM Clay_FloatingAttachPointType
{
	CLAY_ATTACH_POINT_LEFT_TOP,
	CLAY_ATTACH_POINT_LEFT_CENTER,
	CLAY_ATTACH_POINT_LEFT_BOTTOM,
	CLAY_ATTACH_POINT_CENTER_TOP,
	CLAY_ATTACH_POINT_CENTER_CENTER,
	CLAY_ATTACH_POINT_CENTER_BOTTOM,
	CLAY_ATTACH_POINT_RIGHT_TOP,
	CLAY_ATTACH_POINT_RIGHT_CENTER,
	CLAY_ATTACH_POINT_RIGHT_BOTTOM,
};

// Controls where a floating element is offset relative to its parent element.
typedef struct Clay_FloatingAttachPoints Clay_FloatingAttachPoints;
struct Clay_FloatingAttachPoints
{
	Clay_FloatingAttachPointType element; // Controls the origin point on a floating element that attaches to its parent.
	Clay_FloatingAttachPointType parent; // Controls the origin point on the parent element that the floating element attaches to.
};

// Controls how mouse pointer events like hover and click are captured or passed through to elements underneath a floating element.
typedef CLAY_PACKED_ENUM Clay_PointerCaptureMode Clay_PointerCaptureMode;
CLAY_PACKED_ENUM Clay_PointerCaptureMode
{
	// (default) "Capture" the pointer event and don't allow events like hover and click to pass through to elements underneath.
	CLAY_POINTER_CAPTURE_MODE_CAPTURE,
	//    CLAY_POINTER_CAPTURE_MODE_PARENT, TODO pass pointer through to attached parent

	// Transparently pass through pointer events like hover and click to elements underneath the floating element.
	CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
};

// Controls which element a floating element is "attached" to (i.e. relative offset from).
typedef CLAY_PACKED_ENUM Clay_FloatingAttachToElement Clay_FloatingAttachToElement;
CLAY_PACKED_ENUM Clay_FloatingAttachToElement
{
	// (default) Disables floating for this element.
	CLAY_ATTACH_TO_NONE,
	// Attaches this floating element to its parent, positioned based on the .attachPoints and .offset fields.
	CLAY_ATTACH_TO_PARENT,
	// Attaches this floating element to an element with a specific ID, specified with the .parentId field. positioned based on the .attachPoints and .offset fields.
	CLAY_ATTACH_TO_ELEMENT_WITH_ID,
	// Attaches this floating element to the root of the layout, which combined with the .offset field provides functionality similar to "absolute positioning".
	CLAY_ATTACH_TO_ROOT,
};

// Controls various settings related to "floating" elements, which are elements that "r32" above other elements, potentially overlapping their boundaries,
// and not affecting the layout of sibling or parent elements.
typedef struct Clay_FloatingElementConfig Clay_FloatingElementConfig;
struct Clay_FloatingElementConfig
{
	// Offsets this floating element by the provided x,y coordinates from its attachPoints.
	v2 offset;
	// Expands the boundaries of the outer floating element without affecting its children.
	v2 expand;
	// When used in conjunction with .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID, attaches this floating element to the element in the hierarchy with the provided ID.
	// Hint: attach the ID to the other element with .id = CLAY_ID("yourId"), and specify the id the same way, with .parentId = CLAY_ID("yourId").id
	u32 parentId;
	// Controls the z index of this floating element and all its children. Floating elements are sorted in ascending z order before output.
	// zIndex is also passed to the renderer for all elements contained within this floating element.
	i16 zIndex;
	// Controls how mouse pointer events like hover and click are captured or passed through to elements underneath / behind a floating element.
	// Enum is of the form CLAY_ATTACH_POINT_foo_bar. See Clay_FloatingAttachPoints for more details.
	// Note: see <img src="https://github.com/user-attachments/assets/b8c6dfaa-c1b1-41a4-be55-013473e4a6ce />
	// and <img src="https://github.com/user-attachments/assets/ebe75e0d-1904-46b0-982d-418f929d1516 /> for a visual explanation.
	Clay_FloatingAttachPoints attachPoints;
	// Controls how mouse pointer events like hover and click are captured or passed through to elements underneath a floating element.
	// CLAY_POINTER_CAPTURE_MODE_CAPTURE (default) - "Capture" the pointer event and don't allow events like hover and click to pass through to elements underneath.
	// CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH - Transparently pass through pointer events like hover and click to elements underneath the floating element.
	Clay_PointerCaptureMode pointerCaptureMode;
	// Controls which element a floating element is "attached" to (i.e. relative offset from).
	// CLAY_ATTACH_TO_NONE (default) - Disables floating for this element.
	// CLAY_ATTACH_TO_PARENT - Attaches this floating element to its parent, positioned based on the .attachPoints and .offset fields.
	// CLAY_ATTACH_TO_ELEMENT_WITH_ID - Attaches this floating element to an element with a specific ID, specified with the .parentId field. positioned based on the .attachPoints and .offset fields.
	// CLAY_ATTACH_TO_ROOT - Attaches this floating element to the root of the layout, which combined with the .offset field provides functionality similar to "absolute positioning".
	Clay_FloatingAttachToElement attachTo;
};

CLAY__WRAPPER_STRUCT(Clay_FloatingElementConfig);

// +==============================+
// |            Custom            |
// +==============================+
// Controls various settings related to custom elements.
typedef struct Clay_CustomElementConfig Clay_CustomElementConfig;
struct Clay_CustomElementConfig
{
	// A transparent pointer through which you can pass custom data to the renderer.
	// Generates CUSTOM render commands.
	void* customData;
};

CLAY__WRAPPER_STRUCT(Clay_CustomElementConfig);

// +==============================+
// |            Scroll            |
// +==============================+
// Controls the axis on which an element switches to "scrolling", which clips the contents and allows scrolling in that direction.
typedef struct Clay_ScrollElementConfig Clay_ScrollElementConfig;
struct Clay_ScrollElementConfig
{
	bool horizontal; // Clip overflowing elements on the X axis and allow scrolling left and right.
	bool vertical; // Clip overflowing elements on the YU axis and allow scrolling up and down.
	r32 scrollLag;
};

CLAY__WRAPPER_STRUCT(Clay_ScrollElementConfig);

// +==============================+
// |            Border            |
// +==============================+
// Controls the widths of individual element borders.
typedef struct Clay_BorderWidth Clay_BorderWidth;
struct Clay_BorderWidth
{
	u16 left;
	u16 right;
	u16 top;
	u16 bottom;
	// Creates borders between each child element, depending on the .layoutDirection.
	// e.g. for LEFT_TO_RIGHT, borders will be vertical lines, and for TOP_TO_BOTTOM borders will be horizontal lines.
	// .betweenChildren borders will result in individual RECTANGLE render commands being generated.
	u16 betweenChildren;
};

// Controls settings related to element borders.
typedef struct Clay_BorderElementConfig Clay_BorderElementConfig;
struct Clay_BorderElementConfig
{
	Color32 color; // Controls the color of all borders with width > 0. Conventionally represented as 0-255, but interpretation is up to the renderer.
	Clay_BorderWidth width; // Controls the widths of individual borders. At least one of these should be > 0 for a BORDER render command to be generated.
};

CLAY__WRAPPER_STRUCT(Clay_BorderElementConfig);

// +==============================+
// |     Render Command Data      |
// +==============================+
// Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_TEXT
typedef struct Clay_TextRenderData Clay_TextRenderData;
struct Clay_TextRenderData
{
	// A string slice containing the text to be rendered.
	// Note: this is not guaranteed to be null terminated.
	Str8 stringContents;
	// Conventionally represented as 0-255 for each channel, but interpretation is up to the renderer.
	Color32 textColor;
	// An integer representing the font to use to render this text, transparently passed through from the text declaration.
	u16 fontId;
	u16 fontSize;
	// Specifies the extra whitespace gap in pixels between each character.
	u16 letterSpacing;
	// The height of the bounding box for this line of text.
	u16 lineHeight;
	// A pointer transparently passed through from the original text config.
	CLAY_TEXT_USERDATA_TYPE userData;
};

// Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_RECTANGLE
typedef struct Clay_RectangleRenderData Clay_RectangleRenderData;
struct Clay_RectangleRenderData
{
	// The solid background color to fill this rectangle with. Conventionally represented as 0-255 for each channel, but interpretation is up to the renderer.
	Color32 backgroundColor;
	// Controls the "radius", or corner rounding of elements, including rectangles, borders and images.
	// The rounding is determined by drawing a circle inset into the element corner by (radius, radius) pixels.
	Clay_CornerRadius cornerRadius;
};

// Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_IMAGE
typedef struct Clay_ImageRenderData Clay_ImageRenderData;
struct Clay_ImageRenderData
{
	// The tint color for this image. Note that the default value is 0,0,0,0 and should likely be interpreted
	// as "untinted".
	// Conventionally represented as 0-255 for each channel, but interpretation is up to the renderer.
	Color32 backgroundColor;
	// Controls the "radius", or corner rounding of this image.
	// The rounding is determined by drawing a circle inset into the element corner by (radius, radius) pixels.
	Clay_CornerRadius cornerRadius;
	// The original dimensions of the source image, used to control aspect ratio.
	v2 sourceDimensions;
	// A pointer transparently passed through from the original element definition, typically used to represent image data.
	CLAY_IMAGEDATA_TYPE imageData;
};

// Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_CUSTOM
typedef struct Clay_CustomRenderData Clay_CustomRenderData;
struct Clay_CustomRenderData
{
	// Passed through from .backgroundColor in the original element declaration.
	// Conventionally represented as 0-255 for each channel, but interpretation is up to the renderer.
	Color32 backgroundColor;
	// Controls the "radius", or corner rounding of this custom element.
	// The rounding is determined by drawing a circle inset into the element corner by (radius, radius) pixels.
	Clay_CornerRadius cornerRadius;
	// A pointer transparently passed through from the original element definition.
	void* customData;
};

// Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_SCISSOR_START || commandType == CLAY_RENDER_COMMAND_TYPE_SCISSOR_END
typedef struct Clay_ScrollRenderData Clay_ScrollRenderData;
struct Clay_ScrollRenderData
{
	bool horizontal;
	bool vertical;
	r32 scrollLag;
};

// Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_BORDER
typedef struct Clay_BorderRenderData Clay_BorderRenderData;
struct Clay_BorderRenderData
{
	// Controls a shared color for all this element's borders.
	// Conventionally represented as 0-255 for each channel, but interpretation is up to the renderer.
	Color32 color;
	// Specifies the "radius", or corner rounding of this border element.
	// The rounding is determined by drawing a circle inset into the element corner by (radius, radius) pixels.
	Clay_CornerRadius cornerRadius;
	// Controls individual border side widths.
	Clay_BorderWidth width;
};

// A struct union containing data specific to this command's .commandType
typedef union Clay_RenderData Clay_RenderData;
union Clay_RenderData
{
	// Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_RECTANGLE
	Clay_RectangleRenderData rectangle;
	// Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_TEXT
	Clay_TextRenderData text;
	// Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_IMAGE
	Clay_ImageRenderData image;
	// Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_CUSTOM
	Clay_CustomRenderData custom;
	// Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_BORDER
	Clay_BorderRenderData border;
	// Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_SCROLL
	Clay_ScrollRenderData scroll;
};

// Used by renderers to determine specific handling for each render command.
typedef CLAY_PACKED_ENUM Clay_RenderCommandType Clay_RenderCommandType;
CLAY_PACKED_ENUM Clay_RenderCommandType
{
	// This command type should be skipped.
	CLAY_RENDER_COMMAND_TYPE_NONE,
	// The renderer should draw a solid color rectangle.
	CLAY_RENDER_COMMAND_TYPE_RECTANGLE,
	// The renderer should draw a colored border inset into the bounding box.
	CLAY_RENDER_COMMAND_TYPE_BORDER,
	// The renderer should draw text.
	CLAY_RENDER_COMMAND_TYPE_TEXT,
	// The renderer should draw an image.
	CLAY_RENDER_COMMAND_TYPE_IMAGE,
	// The renderer should begin clipping all future draw commands, only rendering content that falls within the provided boundingBox.
	CLAY_RENDER_COMMAND_TYPE_SCISSOR_START,
	// The renderer should finish any previously active clipping, and begin rendering elements in full again.
	CLAY_RENDER_COMMAND_TYPE_SCISSOR_END,
	// The renderer should provide a custom implementation for handling this render command based on its .customData
	CLAY_RENDER_COMMAND_TYPE_CUSTOM,
};

typedef struct Clay_RenderCommand Clay_RenderCommand;
struct Clay_RenderCommand
{
	// A rectangular box that fully encloses this UI element, with the position relative to the root of the layout.
	rec boundingBox;
	// A struct union containing data specific to this command's commandType.
	Clay_RenderData renderData;
	// A pointer transparently passed through from the original element declaration.
	CLAY_ELEMENT_USERDATA_TYPE userData;
	// The id of this element, transparently passed through from the original element declaration.
	u32 id;
	// The z order required for drawing this command correctly.
	// Note: the render command array is already sorted in ascending order, and will produce correct results if drawn in naive order.
	// This field is intended for use in batching renderers for improved performance.
	i16 zIndex;
	// Specifies how to handle rendering of this command.
	// CLAY_RENDER_COMMAND_TYPE_RECTANGLE - The renderer should draw a solid color rectangle.
	// CLAY_RENDER_COMMAND_TYPE_BORDER - The renderer should draw a colored border inset into the bounding box.
	// CLAY_RENDER_COMMAND_TYPE_TEXT - The renderer should draw text.
	// CLAY_RENDER_COMMAND_TYPE_IMAGE - The renderer should draw an image.
	// CLAY_RENDER_COMMAND_TYPE_SCISSOR_START - The renderer should begin clipping all future draw commands, only rendering content that falls within the provided boundingBox.
	// CLAY_RENDER_COMMAND_TYPE_SCISSOR_END - The renderer should finish any previously active clipping, and begin rendering elements in full again.
	// CLAY_RENDER_COMMAND_TYPE_CUSTOM - The renderer should provide a custom implementation for handling this render command based on its .customData
	Clay_RenderCommandType commandType;
};

// +===============================+
// | Miscellaneous Structs & Enums |
// +===============================+
// Data representing the current internal state of a scrolling element.
typedef struct Clay_ScrollContainerData Clay_ScrollContainerData;
struct Clay_ScrollContainerData
{
	// Note: This is a pointer to the real internal scroll position, mutating it may cause a change in final layout.
	// Intended for use with external functionality that modifies scroll position, such as scroll bars or auto scrolling.
	v2* scrollTarget;
	v2* scrollPosition;
	// The bounding box of the scroll element.
	v2 scrollContainerDimensions;
	// The outer dimensions of the inner scroll container content, including the padding of the parent scroll container.
	v2 contentDimensions;
	// The config that was originally passed to the scroll element.
	Clay_ScrollElementConfig config;
	// Indicates whether an actual scroll container matched the provided ID or if the default struct was returned.
	bool found;
};

// Bounding box and other data for a specific UI element.
typedef struct Clay_ElementData Clay_ElementData;
struct Clay_ElementData
{
	// The rectangle that encloses this UI element, with the position relative to the root of the layout.
	rec boundingBox;
	// Indicates whether an actual Element matched the provided ID or if the default struct was returned.
	bool found;
};

// Represents the current state of interaction with clay this frame.
typedef CLAY_PACKED_ENUM Clay_PointerDataInteractionState Clay_PointerDataInteractionState;
CLAY_PACKED_ENUM Clay_PointerDataInteractionState
{
	// A left mouse click, or touch occurred this frame.
	CLAY_POINTER_DATA_PRESSED_THIS_FRAME,
	// The left mouse button click or touch happened at some point in the past, and is still currently held down this frame.
	CLAY_POINTER_DATA_PRESSED,
	// The left mouse button click or touch was released this frame.
	CLAY_POINTER_DATA_RELEASED_THIS_FRAME,
	// The left mouse button click or touch is not currently down / was released at some point in the past.
	CLAY_POINTER_DATA_RELEASED,
};

// Information on the current state of pointer interactions this frame.
typedef struct Clay_PointerData Clay_PointerData;
struct Clay_PointerData
{
	// The position of the mouse / touch / pointer relative to the root of the layout.
	v2 position;
	// Represents the current state of interaction with clay this frame.
	// CLAY_POINTER_DATA_PRESSED_THIS_FRAME - A left mouse click, or touch occurred this frame.
	// CLAY_POINTER_DATA_PRESSED - The left mouse button click or touch happened at some point in the past, and is still currently held down this frame.
	// CLAY_POINTER_DATA_RELEASED_THIS_FRAME - The left mouse button click or touch was released this frame.
	// CLAY_POINTER_DATA_RELEASED - The left mouse button click or touch is not currently down / was released at some point in the past.
	Clay_PointerDataInteractionState state;
};

typedef struct Clay_ElementDeclaration Clay_ElementDeclaration;
struct Clay_ElementDeclaration
{
	// Primarily created via the CLAY_ID(), CLAY_IDI(), CLAY_ID_LOCAL() and CLAY_IDI_LOCAL() macros.
	// Represents a hashed string ID used for identifying and finding specific clay UI elements, required by functions such as Clay_PointerOver() and Clay_GetElementData().
	Clay_ElementId id;
	// Controls various settings that affect the size and position of an element, as well as the sizes and positions of any child elements.
	Clay_LayoutConfig layout;
	// Controls the background color of the resulting element.
	// By convention specified as 0-255, but interpretation is up to the renderer.
	// If no other config is specified, .backgroundColor will generate a RECTANGLE render command, otherwise it will be passed as a property to IMAGE or CUSTOM render commands.
	Color32 backgroundColor;
	// Controls the "radius", or corner rounding of elements, including rectangles, borders and images.
	Clay_CornerRadius cornerRadius;
	// Controls settings related to image elements.
	Clay_ImageElementConfig image;
	// Controls whether and how an element "floats", which means it layers over the top of other elements in z order, and doesn't affect the position and size of siblings or parent elements.
	// Note: in order to activate floating, .floating.attachTo must be set to something other than the default value.
	Clay_FloatingElementConfig floating;
	// Used to create CUSTOM render commands, usually to render element types not supported by Clay.
	Clay_CustomElementConfig custom;
	// Controls whether an element should clip its contents and allow scrolling rather than expanding to contain them.
	Clay_ScrollElementConfig scroll;
	// Controls settings related to element borders, and will generate BORDER render commands.
	Clay_BorderElementConfig border;
	// A pointer that will be transparently passed through to resulting render commands.
	CLAY_ELEMENT_USERDATA_TYPE userData;
};

CLAY__WRAPPER_STRUCT(Clay_ElementDeclaration);

// +==============================+
// |          Callbacks           |
// +==============================+
// Represents the type of error clay encountered while computing layout.
typedef CLAY_PACKED_ENUM Clay_ErrorType Clay_ErrorType;
CLAY_PACKED_ENUM Clay_ErrorType
{
	// A text measurement function wasn't provided using Clay_SetMeasureTextFunction(), or the provided function was null.
	CLAY_ERROR_TYPE_TEXT_MEASUREMENT_FUNCTION_NOT_PROVIDED,
	// Clay attempted to allocate its internal data structures but ran out of space.
	// The arena passed to Clay_Initialize was created with a capacity smaller than that required by Clay_MinMemorySize().
	CLAY_ERROR_TYPE_ARENA_CAPACITY_EXCEEDED,
	// Clay ran out of capacity in its internal array for storing elements. This limit can be increased with Clay_SetMaxElementCount().
	CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED,
	// Clay ran out of capacity in its internal array for storing elements. This limit can be increased with Clay_SetMaxMeasureTextCacheWordCount().
	CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED,
	// Two elements were declared with exactly the same ID within one layout.
	CLAY_ERROR_TYPE_DUPLICATE_ID,
	// A floating element was declared using CLAY_ATTACH_TO_ELEMENT_ID and either an invalid .parentId was provided or no element with the provided .parentId was found.
	CLAY_ERROR_TYPE_FLOATING_CONTAINER_PARENT_NOT_FOUND,
	// An element was declared that using CLAY_SIZING_PERCENT but the percentage value was over 1. Percentage values are expected to be in the 0-1 range.
	CLAY_ERROR_TYPE_PERCENTAGE_OVER_1,
	// Clay encountered an internal error. It would be wonderful if you could report this so we can fix it!
	CLAY_ERROR_TYPE_INTERNAL_ERROR,
};

// Data to identify the error that clay has encountered.
typedef struct Clay_ErrorData Clay_ErrorData;
struct Clay_ErrorData
{
	// Represents the type of error clay encountered while computing layout.
	// CLAY_ERROR_TYPE_TEXT_MEASUREMENT_FUNCTION_NOT_PROVIDED - A text measurement function wasn't provided using Clay_SetMeasureTextFunction(), or the provided function was null.
	// CLAY_ERROR_TYPE_ARENA_CAPACITY_EXCEEDED - Clay attempted to allocate its internal data structures but ran out of space. The arena passed to Clay_Initialize was created with a capacity smaller than that required by Clay_MinMemorySize().
	// CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED - Clay ran out of capacity in its internal array for storing elements. This limit can be increased with Clay_SetMaxElementCount().
	// CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED - Clay ran out of capacity in its internal array for storing elements. This limit can be increased with Clay_SetMaxMeasureTextCacheWordCount().
	// CLAY_ERROR_TYPE_DUPLICATE_ID - Two elements were declared with exactly the same ID within one layout.
	// CLAY_ERROR_TYPE_FLOATING_CONTAINER_PARENT_NOT_FOUND - A floating element was declared using CLAY_ATTACH_TO_ELEMENT_ID and either an invalid .parentId was provided or no element with the provided .parentId was found.
	// CLAY_ERROR_TYPE_PERCENTAGE_OVER_1 - An element was declared that using CLAY_SIZING_PERCENT but the percentage value was over 1. Percentage values are expected to be in the 0-1 range.
	// CLAY_ERROR_TYPE_INTERNAL_ERROR - Clay encountered an internal error. It would be wonderful if you could report this so we can fix it!
	Clay_ErrorType errorType;
	// A string containing human-readable error text that explains the error in more detail.
	Str8 errorText;
	// A transparent pointer passed through from when the error handler was first provided.
	CLAY_ERROR_USERDATA_TYPE userData;
};

// A wrapper struct around Clay's error handler function.
typedef struct Clay_ErrorHandler Clay_ErrorHandler;
struct Clay_ErrorHandler
{
	// A user provided function to call when Clay encounters an error during layout.
	void (*errorHandlerFunction)(Clay_ErrorData errorText);
	// A pointer that will be transparently passed through to the error handler when it is called.
	CLAY_ERROR_USERDATA_TYPE userData;
};

#define CLAY_MEASURE_TEXT_DEF(functionName) v2 functionName(Str8 text, Clay_TextElementConfig* config, CLAY_MEASURE_USERDATA_TYPE userData)
typedef CLAY_MEASURE_TEXT_DEF(ClayMeasureText_f);

#define CLAY_HASH_TEXT_USERDATA_DEF(functionName) u32 functionName(u32 currentHash, Clay_TextElementConfig* config)
typedef CLAY_HASH_TEXT_USERDATA_DEF(ClayHashTextUserData_f);

typedef struct Clay__Warning Clay__Warning;
struct Clay__Warning
{
	Str8 baseMessage;
	Str8 dynamicMessage;
};

// +--------------------------------------------------------------+
// |                        Private Types                         |
// +--------------------------------------------------------------+
#ifdef CLAY_IMPLEMENTATION

typedef struct Clay_BooleanWarnings Clay_BooleanWarnings;
struct Clay_BooleanWarnings
{
	bool maxElementsExceeded;
	bool maxRenderCommandsExceeded;
	bool maxTextMeasureCacheExceeded;
	bool textMeasurementFunctionNotSet;
};

typedef struct Clay_SharedElementConfig Clay_SharedElementConfig;
struct Clay_SharedElementConfig
{
	Color32 backgroundColor;
	Clay_CornerRadius cornerRadius;
	CLAY_ELEMENT_USERDATA_TYPE userData;
};

CLAY__WRAPPER_STRUCT(Clay_SharedElementConfig);

typedef CLAY_PACKED_ENUM Clay__ElementConfigType Clay__ElementConfigType;
CLAY_PACKED_ENUM Clay__ElementConfigType
{
	CLAY__ELEMENT_CONFIG_TYPE_NONE,
	CLAY__ELEMENT_CONFIG_TYPE_BORDER,
	CLAY__ELEMENT_CONFIG_TYPE_FLOATING,
	CLAY__ELEMENT_CONFIG_TYPE_SCROLL,
	CLAY__ELEMENT_CONFIG_TYPE_IMAGE,
	CLAY__ELEMENT_CONFIG_TYPE_TEXT,
	CLAY__ELEMENT_CONFIG_TYPE_CUSTOM,
	CLAY__ELEMENT_CONFIG_TYPE_SHARED,
};

typedef union Clay_ElementConfigUnion Clay_ElementConfigUnion;
union Clay_ElementConfigUnion
{
	Clay_TextElementConfig* textElementConfig;
	Clay_ImageElementConfig* imageElementConfig;
	Clay_FloatingElementConfig* floatingElementConfig;
	Clay_CustomElementConfig* customElementConfig;
	Clay_ScrollElementConfig* scrollElementConfig;
	Clay_BorderElementConfig* borderElementConfig;
	Clay_SharedElementConfig* sharedElementConfig;
};

typedef struct Clay_ElementConfig Clay_ElementConfig;
struct Clay_ElementConfig
{
	Clay__ElementConfigType type;
	Clay_ElementConfigUnion config;
};

typedef struct Clay__WrappedTextLine Clay__WrappedTextLine;
struct Clay__WrappedTextLine
{
	v2 dimensions;
	Str8 line;
};

typedef struct Clay__TextElementData Clay__TextElementData;
struct Clay__TextElementData
{
	Str8 text;
	v2 preferredDimensions;
	i32 elementIndex;
	Clay__WrappedTextLineArraySlice wrappedLines;
};

typedef struct Clay__LayoutElementChildren Clay__LayoutElementChildren;
struct Clay__LayoutElementChildren
{
	i32* elements;
	u16 length;
};

typedef struct Clay_LayoutElement Clay_LayoutElement;
struct Clay_LayoutElement
{
	union
	{
		Clay__LayoutElementChildren children;
		Clay__TextElementData* textElementData;
	} childrenOrTextContent;
	v2 dimensions;
	v2 minDimensions;
	Clay_LayoutConfig* layoutConfig;
	Clay__ElementConfigArraySlice elementConfigs;
	u32 id;
};

typedef struct Clay__ScrollContainerDataInternal Clay__ScrollContainerDataInternal;
struct Clay__ScrollContainerDataInternal
{
	Clay_LayoutElement* layoutElement;
	rec boundingBox;
	v2 contentSize;
	v2 scrollOrigin;
	v2 pointerOrigin;
	v2 scrollMomentum;
	v2 scrollTarget;
	v2 scrollPosition;
	v2 previousDelta;
	r32 momentumTime;
	r32 scrollLag;
	u32 elementId;
	bool openThisFrame;
	bool pointerScrollActive;
};

typedef struct Clay__DebugElementData Clay__DebugElementData;
struct Clay__DebugElementData
{
	bool collision;
	bool collapsed;
};

typedef struct Clay_LayoutElementHashMapItem Clay_LayoutElementHashMapItem;
struct Clay_LayoutElementHashMapItem // todo get this struct into a single cache line
{
	rec boundingBox;
	Clay_ElementId elementId;
	Clay_LayoutElement* layoutElement;
	void (*onHoverFunction)(Clay_ElementId elementId, Clay_PointerData pointerInfo, CLAY_ONHOVER_USERDATA_TYPE userData);
	CLAY_ONHOVER_USERDATA_TYPE hoverFunctionUserData;
	i32 nextIndex;
	u32 generation;
	u32 idAlias;
	Clay__DebugElementData* debugData;
};

typedef struct Clay__MeasuredWord Clay__MeasuredWord;
struct Clay__MeasuredWord
{
	i32 startOffset;
	i32 length;
	r32 width;
	i32 next;
};

typedef struct Clay__MeasureTextCacheItem Clay__MeasureTextCacheItem;
struct Clay__MeasureTextCacheItem
{
	v2 unwrappedDimensions;
	i32 measuredWordsStartIndex;
	bool containsNewlines;
	// Hash map data
	u32 id;
	i32 nextIndex;
	u32 generation;
};

typedef struct Clay__LayoutElementTreeNode Clay__LayoutElementTreeNode;
struct Clay__LayoutElementTreeNode
{
	Clay_LayoutElement* layoutElement;
	v2 position;
	v2 nextChildOffset;
};

typedef struct Clay__LayoutElementTreeRoot Clay__LayoutElementTreeRoot;
struct Clay__LayoutElementTreeRoot
{
	i32 layoutElementIndex;
	u32 parentId; // This can be zero in the case of the root layout tree
	u32 clipElementId; // This can be zero if there is no clip element
	i16 zIndex;
	v2 pointerOffset; // Only used when scroll containers are managed externally
};

// +==============================+
// |         Clay Context         |
// +==============================+
struct Clay_Context
{
	i32 maxElementCount;
	i32 maxMeasureTextCacheWordCount;
	bool warningsEnabled;
	Clay_ErrorHandler errorHandler;
	Clay_BooleanWarnings booleanWarnings;
	Clay__WarningArray warnings;

	Clay_PointerData pointerInfo;
	v2 layoutDimensions;
	Clay_ElementId dynamicElementIndexBaseHash;
	u32 dynamicElementIndex;
	bool debugModeEnabled;
	bool disableCulling;
	bool externalScrollHandlingEnabled;
	u32 debugSelectedElementId;
	u32 generation;
	CLAY_MEASURE_USERDATA_TYPE measureTextUserData;
	CLAY_QUERYSCROLL_USERDATA_TYPE queryScrollOffsetUserData;
	Arena* internalArena;
	// Layout Elements / Render Commands
	Clay_LayoutElementArray layoutElements;
	Clay_RenderCommandArray renderCommands;
	i32Array openLayoutElementStack;
	i32Array layoutElementChildren;
	i32Array layoutElementChildrenBuffer;
	Clay__TextElementDataArray textElementData;
	i32Array imageElementPointers;
	i32Array reusableElementIndexBuffer;
	i32Array layoutElementClipElementIds;
	// Configs
	Clay__LayoutConfigArray layoutConfigs;
	Clay__ElementConfigArray elementConfigs;
	Clay__TextElementConfigArray textElementConfigs;
	Clay__ImageElementConfigArray imageElementConfigs;
	Clay__FloatingElementConfigArray floatingElementConfigs;
	Clay__ScrollElementConfigArray scrollElementConfigs;
	Clay__CustomElementConfigArray customElementConfigs;
	Clay__BorderElementConfigArray borderElementConfigs;
	Clay__SharedElementConfigArray sharedElementConfigs;
	// Misc Data Structures
	Str8Array layoutElementIdStrings;
	Clay__WrappedTextLineArray wrappedTextLines;
	Clay__LayoutElementTreeNodeArray layoutElementTreeNodeArray1;
	Clay__LayoutElementTreeRootArray layoutElementTreeRoots;
	Clay__LayoutElementHashMapItemArray layoutElementsHashMapInternal;
	i32Array layoutElementsHashMap;
	Clay__MeasureTextCacheItemArray measureTextHashMapInternal;
	i32Array measureTextHashMapInternalFreeList;
	i32Array measureTextHashMap;
	Clay__MeasuredWordArray measuredWords;
	i32Array measuredWordsFreeList;
	i32Array openClipElementStack;
	Clay__ElementIdArray pointerOverIds;
	Clay__ScrollContainerDataInternalArray scrollContainerDatas;
	boolArray treeNodeVisited;
	charArray dynamicStringData;
	Clay__DebugElementDataArray debugElementData;
};

// +==============================+
// |            Debug             |
// +==============================+
typedef struct Clay__DebugElementConfigTypeLabelConfig Clay__DebugElementConfigTypeLabelConfig;
struct Clay__DebugElementConfigTypeLabelConfig
{
	Str8 label;
	Color32 color;
};

typedef struct Clay__RenderDebugLayoutData Clay__RenderDebugLayoutData;
struct Clay__RenderDebugLayoutData
{
	i32 rowCount;
	i32 selectedElementRowIndex;
};

#endif //CLAY_IMPLEMENTATION

MAYBE_END_EXTERN_C

#endif //  _CLAY_TYPES_H

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
