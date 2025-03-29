// VERSION: 0.12
// WARNING: Modifications have been made to this version that resides in PigCore.
//  Please refer to the official Clay repostiory for an unmodified version of this file!
//  https://github.com/nicbarker/clay

/*
	NOTE: In order to use this library you must define
	the following macro in exactly one file, _before_ including clay.h:

	#define CLAY_IMPLEMENTATION
	#include "clay.h"

	See the examples folder for details.
*/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// SIMD includes on supported platforms
#if !defined(CLAY_DISABLE_SIMD) && (defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64))
#include <emmintrin.h>
#elif !defined(CLAY_DISABLE_SIMD) && defined(__aarch64__)
#include <arm_neon.h>
#endif

// -----------------------------------------
// HEADER DECLARATIONS ---------------------
// -----------------------------------------

#ifndef CLAY_HEADER
#define CLAY_HEADER

#if !( \
	(defined(__cplusplus) && __cplusplus >= 202002L) || \
	(defined(__STDC__) && __STDC__ == 1 && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || \
	defined(_MSC_VER) \
)
#error "Clay requires C99, C++20, or MSVC"
#endif

#ifdef CLAY_WASM
#define CLAY_WASM_EXPORT(name) __attribute__((export_name(name)))
#else
#define CLAY_WASM_EXPORT(null)
#endif

#ifndef CLAY_DECOR
#define CLAY_DECOR //nothing
#endif


//NOTE: These types MUST be able to be assigned to ZEROED, and a Clay__MemCmp with a zeroed version should be equivalent with checking if it's "empty"
#ifndef CLAY_ELEMENT_USERDATA_TYPE
#define CLAY_ELEMENT_USERDATA_TYPE void*
#endif
#ifndef CLAY_TEXT_USERDATA_TYPE
#define CLAY_TEXT_USERDATA_TYPE void*
#endif
#ifndef CLAY_IMAGEDATA_TYPE
#define CLAY_IMAGEDATA_TYPE void*
#endif

#ifndef CLAY_ERROR_USERDATA_TYPE
#define CLAY_ERROR_USERDATA_TYPE void*
#endif
#ifndef CLAY_ONHOVER_USERDATA_TYPE
#define CLAY_ONHOVER_USERDATA_TYPE void*
#endif
#ifndef CLAY_QUERYSCROLL_USERDATA_TYPE
#define CLAY_QUERYSCROLL_USERDATA_TYPE void*
#endif
#ifndef CLAY_MEASURE_USERDATA_TYPE
#define CLAY_MEASURE_USERDATA_TYPE void*
#endif

// Public Macro API ------------------------

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

#define CLAY__STRING_LENGTH(s) ((sizeof(s) / sizeof((s)[0])) - sizeof((s)[0]))

#define CLAY__ENSURE_STRING_LITERAL(x) ("" x "")

// Note: If an error led you here, it's because CLAY_STRING can only be used with string literals, i.e. CLAY_STRING("SomeString") and not CLAY_STRING(yourString)
#define CLAY_STRING(stringLiteral) NewStr8(CLAY__STRING_LENGTH(CLAY__ENSURE_STRING_LITERAL(stringLiteral)), (stringLiteral))

#define CLAY_STRING_CONST(string) { .length = CLAY__STRING_LENGTH(CLAY__ENSURE_STRING_LITERAL(string)), .chars = (string) }

static uint8_t CLAY__ELEMENT_DEFINITION_LATCH;

// Publicly visible layout element macros -----------------------------------------------------

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

#if LANGUAGE_IS_CPP
	#define CLAY_PACKED_ENUM enum : uint8_t
#else //!LANGUAGE_IS_CPP
	#if COMPILER_IS_MSVC
	#define CLAY_PACKED_ENUM __pragma(pack(push, 1)) enum __pragma(pack(pop))
	#else
	#define CLAY_PACKED_ENUM enum __attribute__((__packed__))
	#endif
#endif // LANGUAGE_IS_CPP

MAYBE_START_EXTERN_C

// Utility Structs -------------------------

typedef struct Clay_Context Clay_Context;

// Primarily created via the CLAY_ID(), CLAY_IDI(), CLAY_ID_LOCAL() and CLAY_IDI_LOCAL() macros.
// Represents a hashed string ID used for identifying and finding specific clay UI elements, required
// by functions such as Clay_PointerOver() and Clay_GetElementData().
typedef struct Clay_ElementId Clay_ElementId;
struct Clay_ElementId
{
	uint32_t id; // The resulting hash generated from the other fields.
	uint32_t offset; // A numerical offset applied after computing the hash from stringId.
	uint32_t baseId; // A base hash value to start from, for example the parent element ID is used when calculating CLAY_ID_LOCAL().
	Str8 stringId; // The string id to hash.
};

// Controls the "radius", or corner rounding of elements, including rectangles, borders and images.
// The rounding is determined by drawing a circle inset into the element corner by (radius, radius) pixels.
typedef struct Clay_CornerRadius Clay_CornerRadius;
struct Clay_CornerRadius
{
	float topLeft;
	float topRight;
	float bottomLeft;
	float bottomRight;
};

// Element Configs ---------------------------

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
	float min; // The smallest final size of the element on this axis will be this value in pixels.
	float max; // The largest final size of the element on this axis will be this value in pixels.
};

// Controls the sizing of this element along one axis inside its parent container.
typedef struct Clay_SizingAxis Clay_SizingAxis;
struct Clay_SizingAxis
{
	union {
		Clay_SizingMinMax minMax; // Controls the minimum and maximum size in pixels that this element is allowed to grow or shrink to, overriding sizing types such as FIT or GROW.
		float percent; // Expects 0-1 range. Clamps the axis size to a percent of the parent container's axis size minus padding and child gaps.
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
	uint16_t left;
	uint16_t right;
	uint16_t top;
	uint16_t bottom;
};

CLAY__WRAPPER_STRUCT(Clay_Padding);

// Controls various settings that affect the size and position of an element, as well as the sizes and positions
// of any child elements.
typedef struct Clay_LayoutConfig Clay_LayoutConfig;
struct Clay_LayoutConfig
{
	Clay_Sizing sizing; // Controls the sizing of this element inside it's parent container, including FIT, GROW, PERCENT and FIXED sizing.
	Clay_Padding padding; // Controls "padding" in pixels, which is a gap between the bounding box of this element and where its children will be placed.
	uint16_t childGap; // Controls the gap in pixels between child elements along the layout axis (horizontal gap for LEFT_TO_RIGHT, vertical gap for TOP_TO_BOTTOM).
	Clay_ChildAlignment childAlignment; // Controls how child elements are aligned on each axis.
	Clay_LayoutDirection layoutDirection; // Controls the direction in which child elements will be automatically laid out.
};

CLAY__WRAPPER_STRUCT(Clay_LayoutConfig);

extern Clay_LayoutConfig CLAY_LAYOUT_DEFAULT;

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
	uint16_t fontId;
	// Controls the size of the font. Handled by the function provided to Clay_MeasureText.
	uint16_t fontSize;
	// Controls extra horizontal spacing between characters. Handled by the function provided to Clay_MeasureText.
	uint16_t letterSpacing;
	// Controls additional vertical space between wrapped lines of text.
	uint16_t lineHeight;
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

// Image --------------------------------

// Controls various settings related to image elements.
typedef struct Clay_ImageElementConfig Clay_ImageElementConfig;
struct Clay_ImageElementConfig
{
	CLAY_IMAGEDATA_TYPE imageData; // A transparent pointer used to pass image data through to the renderer.
	v2 sourceDimensions; // The original dimensions of the source image, used to control aspect ratio.
};

CLAY__WRAPPER_STRUCT(Clay_ImageElementConfig);

// Floating -----------------------------

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

// Controls various settings related to "floating" elements, which are elements that "float" above other elements, potentially overlapping their boundaries,
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
	uint32_t parentId;
	// Controls the z index of this floating element and all its children. Floating elements are sorted in ascending z order before output.
	// zIndex is also passed to the renderer for all elements contained within this floating element.
	int16_t zIndex;
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

// Custom -----------------------------

// Controls various settings related to custom elements.
typedef struct Clay_CustomElementConfig Clay_CustomElementConfig;
struct Clay_CustomElementConfig
{
	// A transparent pointer through which you can pass custom data to the renderer.
	// Generates CUSTOM render commands.
	void* customData;
};

CLAY__WRAPPER_STRUCT(Clay_CustomElementConfig);

// Scroll -----------------------------

// Controls the axis on which an element switches to "scrolling", which clips the contents and allows scrolling in that direction.
typedef struct Clay_ScrollElementConfig Clay_ScrollElementConfig;
struct Clay_ScrollElementConfig
{
	bool horizontal; // Clip overflowing elements on the X axis and allow scrolling left and right.
	bool vertical; // Clip overflowing elements on the YU axis and allow scrolling up and down.
	float scrollLag;
};

CLAY__WRAPPER_STRUCT(Clay_ScrollElementConfig);

// Border -----------------------------

// Controls the widths of individual element borders.
typedef struct Clay_BorderWidth Clay_BorderWidth;
struct Clay_BorderWidth
{
	uint16_t left;
	uint16_t right;
	uint16_t top;
	uint16_t bottom;
	// Creates borders between each child element, depending on the .layoutDirection.
	// e.g. for LEFT_TO_RIGHT, borders will be vertical lines, and for TOP_TO_BOTTOM borders will be horizontal lines.
	// .betweenChildren borders will result in individual RECTANGLE render commands being generated.
	uint16_t betweenChildren;
};

// Controls settings related to element borders.
typedef struct Clay_BorderElementConfig Clay_BorderElementConfig;
struct Clay_BorderElementConfig
{
	Color32 color; // Controls the color of all borders with width > 0. Conventionally represented as 0-255, but interpretation is up to the renderer.
	Clay_BorderWidth width; // Controls the widths of individual borders. At least one of these should be > 0 for a BORDER render command to be generated.
};

CLAY__WRAPPER_STRUCT(Clay_BorderElementConfig);

// Render Command Data -----------------------------

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
	uint16_t fontId;
	uint16_t fontSize;
	// Specifies the extra whitespace gap in pixels between each character.
	uint16_t letterSpacing;
	// The height of the bounding box for this line of text.
	uint16_t lineHeight;
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
	float scrollLag;
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

// Miscellaneous Structs & Enums ---------------------------------

// Data representing the current internal state of a scrolling element.
typedef struct Clay_ScrollContainerData Clay_ScrollContainerData;
struct Clay_ScrollContainerData
{
	// Note: This is a pointer to the real internal scroll position, mutating it may cause a change in final layout.
	// Intended for use with external functionality that modifies scroll position, such as scroll bars or auto scrolling.
	v2 *scrollTarget;
	v2 *scrollPosition;
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
	uint32_t id;
	// The z order required for drawing this command correctly.
	// Note: the render command array is already sorted in ascending order, and will produce correct results if drawn in naive order.
	// This field is intended for use in batching renderers for improved performance.
	int16_t zIndex;
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

// A sized array of render commands.
typedef struct Clay_RenderCommandArray Clay_RenderCommandArray;
struct Clay_RenderCommandArray
{
	// The underlying max capacity of the array, not necessarily all initialized.
	int32_t allocLength;
	// The number of initialized elements in this array. Used for loops and iteration.
	int32_t length;
	// A pointer to the first element in the internal array.
	Clay_RenderCommand* items;
};
typedef struct Clay_RenderCommandArraySlice Clay_RenderCommandArraySlice;
struct Clay_RenderCommandArraySlice
{
	i32 length;
	Clay_RenderCommand* items;
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

#define CLAY_MEASURE_TEXT_DEF(functionName) v2 functionName(Str8 text, Clay_TextElementConfig *config, CLAY_MEASURE_USERDATA_TYPE userData)
typedef CLAY_MEASURE_TEXT_DEF(ClayMeasureText_f);

// Function Forward Declarations ---------------------------------

// Public API functions ------------------------------------------

// Returns the size, in bytes, of the minimum amount of memory Clay requires to operate at its current settings.
CLAY_DECOR uint32_t Clay_MinMemorySize(void);
// Creates an arena for clay to use for its internal allocations, given a certain capacity in bytes and a pointer to an allocation of at least that size.
// Intended to be used with Clay_MinMemorySize in the following way:
// uint32_t minMemoryRequired = Clay_MinMemorySize();
// Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(minMemoryRequired, malloc(minMemoryRequired));
CLAY_DECOR Clay_Arena Clay_CreateArenaWithCapacityAndMemory(uint32_t capacity, void *memory);
// Sets the state of the "pointer" (i.e. the mouse or touch) in Clay's internal data. Used for detecting and responding to mouse events in the debug view,
// as well as for Clay_Hovered() and scroll element handling.
CLAY_DECOR void Clay_SetPointerState(v2 position, bool pointerDown);
// Initialize Clay's internal arena and setup required data before layout can begin. Only needs to be called once.
// - arena can be created using Clay_CreateArenaWithCapacityAndMemory()
// - layoutDimensions are the initial bounding dimensions of the layout (i.e. the screen width and height for a full screen layout)
// - errorHandler is used by Clay to inform you if something has gone wrong in configuration or layout.
CLAY_DECOR Clay_Context* Clay_Initialize(Clay_Arena arena, v2 layoutDimensions, Clay_ErrorHandler errorHandler);
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
CLAY_DECOR bool Clay_UpdateScrollContainers(bool enableDragScrolling, v2 scrollDelta, float deltaTime);
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
CLAY_DECOR Clay_ElementId Clay_GetElementIdWithIndex(Str8 idString, uint32_t index);
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
CLAY_DECOR Clay_ScrollContainerData Clay_GetScrollContainerData(Clay_ElementId id);
// Binds a callback function that Clay will call to determine the dimensions of a given string slice.
// - measureTextFunction is a user provided function that adheres to the interface v2 (Str8 text, Clay_TextElementConfig *config, CLAY_MEASURE_USERDATA_TYPE userData);
// - userData is a pointer that will be transparently passed through when the measureTextFunction is called.
CLAY_DECOR void Clay_SetMeasureTextFunction(ClayMeasureText_f* measureTextFunction, CLAY_MEASURE_USERDATA_TYPE userData);
// Experimental - Used in cases where Clay needs to integrate with a system that manages its own scrolling containers externally.
// Please reach out if you plan to use this function, as it may be subject to change.
CLAY_DECOR void Clay_SetQueryScrollOffsetFunction(v2 (*queryScrollOffsetFunction)(uint32_t elementId, CLAY_QUERYSCROLL_USERDATA_TYPE userData), CLAY_QUERYSCROLL_USERDATA_TYPE userData);
// A bounds-checked "get" function for the Clay_RenderCommandArray returned from Clay_EndLayout().
CLAY_DECOR Clay_RenderCommand * Clay_RenderCommandArray_Get(Clay_RenderCommandArray* array, int32_t index);
// Enables and disables Clay's internal debug tools.
// This state is retained and does not need to be set each frame.
CLAY_DECOR void Clay_SetDebugModeEnabled(bool enabled);
// Returns true if Clay's internal debug tools are currently enabled.
CLAY_DECOR bool Clay_IsDebugModeEnabled(void);
// Enables and disables visibility culling. By default, Clay will not generate render commands for elements whose bounding box is entirely outside the screen.
CLAY_DECOR void Clay_SetCullingEnabled(bool enabled);
// Returns the maximum number of UI elements supported by Clay's current configuration.
CLAY_DECOR int32_t Clay_GetMaxElementCount(void);
// Modifies the maximum number of UI elements supported by Clay's current configuration.
// This may require reallocating additional memory, and re-calling Clay_Initialize();
CLAY_DECOR void Clay_SetMaxElementCount(int32_t maxElementCount);
// Returns the maximum number of measured "words" (whitespace seperated runs of characters) that Clay can store in its internal text measurement cache.
CLAY_DECOR int32_t Clay_GetMaxMeasureTextCacheWordCount(void);
// Modifies the maximum number of measured "words" (whitespace seperated runs of characters) that Clay can store in its internal text measurement cache.
// This may require reallocating additional memory, and re-calling Clay_Initialize();
CLAY_DECOR void Clay_SetMaxMeasureTextCacheWordCount(int32_t maxMeasureTextCacheWordCount);
// Resets Clay's internal text measurement cache, useful if memory to represent strings is being re-used.
// Similar behaviour can be achieved on an individual text element level by using Clay_TextElementConfig.hashStringContents
CLAY_DECOR void Clay_ResetMeasureTextCache(void);

// Internal API functions required by macros ----------------------

CLAY_DECOR void Clay__OpenElement(void);
CLAY_DECOR void Clay__ConfigureOpenElement(const Clay_ElementDeclaration config);
CLAY_DECOR void Clay__CloseElement(void);
CLAY_DECOR Clay_ElementId Clay__HashString(Str8 key, uint32_t offset, uint32_t seed);
CLAY_DECOR void Clay__OpenTextElement(Str8 text, Clay_TextElementConfig *textConfig);
CLAY_DECOR Clay_TextElementConfig *Clay__StoreTextElementConfig(Clay_TextElementConfig config);
CLAY_DECOR uint32_t Clay__GetParentElementId(void);

extern Color32 Clay__debugViewHighlightColor;
extern uint32_t Clay__debugViewWidth;

MAYBE_END_EXTERN_C

#endif // CLAY_HEADER

// -----------------------------------------
// IMPLEMENTATION --------------------------
// -----------------------------------------
#ifdef CLAY_IMPLEMENTATION
#undef CLAY_IMPLEMENTATION

#ifndef CLAY__NULL
#define CLAY__NULL 0
#endif

#ifndef CLAY__MAXFLOAT
#define CLAY__MAXFLOAT 3.40282346638528859812e+38F
#endif

Clay_LayoutConfig CLAY_LAYOUT_DEFAULT = ZEROED;

Color32 Clay__Color_DEFAULT = ZEROED;
Clay_CornerRadius Clay__CornerRadius_DEFAULT = ZEROED;
Clay_BorderWidth Clay__BorderWidth_DEFAULT = ZEROED;

Clay_Context *Clay__currentContext;
int32_t Clay__defaultMaxElementCount = 8192;
int32_t Clay__defaultMaxMeasureTextWordCacheCount = 16384;

void Clay__ErrorHandlerFunctionDefault(Clay_ErrorData errorText)
{
	(void) errorText;
}

typedef struct Clay_BooleanWarnings Clay_BooleanWarnings;
struct Clay_BooleanWarnings
{
	bool maxElementsExceeded;
	bool maxRenderCommandsExceeded;
	bool maxTextMeasureCacheExceeded;
	bool textMeasurementFunctionNotSet;
};

typedef struct Clay__Warning Clay__Warning;
struct Clay__Warning
{
	Str8 baseMessage;
	Str8 dynamicMessage;
};

Clay__Warning CLAY__WARNING_DEFAULT = ZEROED;

typedef struct Clay__WarningArray Clay__WarningArray;
struct Clay__WarningArray
{
	int32_t allocLength;
	int32_t length;
	Clay__Warning* items;
};

typedef struct Clay_SharedElementConfig Clay_SharedElementConfig;
struct Clay_SharedElementConfig
{
	Color32 backgroundColor;
	Clay_CornerRadius cornerRadius;
	CLAY_ELEMENT_USERDATA_TYPE userData;
};

CLAY__WRAPPER_STRUCT(Clay_SharedElementConfig);

Clay__WarningArray Clay__WarningArray_Allocate_Arena(int32_t initialCapacity, Clay_Arena *arena);
Clay__Warning *Clay__WarningArray_Add(Clay__WarningArray *array, Clay__Warning item);
bool Clay__Array_RangeCheck(int32_t index, int32_t length);
bool Clay__Array_AddCapacityCheck(int32_t length, int32_t allocLength);

DEFINE_TYPED_ARRAY(bool, Clay__boolArray)
DEFINE_TYPED_ARRAY(int32_t, Clay__int32_tArray)
DEFINE_TYPED_ARRAY(char, Clay__charArray)
DEFINE_TYPED_ARRAY(Clay_ElementId, Clay__ElementIdArray)
DEFINE_TYPED_ARRAY(Clay_LayoutConfig, Clay__LayoutConfigArray)
DEFINE_TYPED_ARRAY(Clay_TextElementConfig, Clay__TextElementConfigArray)
DEFINE_TYPED_ARRAY(Clay_ImageElementConfig, Clay__ImageElementConfigArray)
DEFINE_TYPED_ARRAY(Clay_FloatingElementConfig, Clay__FloatingElementConfigArray)
DEFINE_TYPED_ARRAY(Clay_CustomElementConfig, Clay__CustomElementConfigArray)
DEFINE_TYPED_ARRAY(Clay_ScrollElementConfig, Clay__ScrollElementConfigArray)
DEFINE_TYPED_ARRAY(Clay_BorderElementConfig, Clay__BorderElementConfigArray)
DEFINE_TYPED_ARRAY(Str8, Clay__Str8Array)
DEFINE_TYPED_ARRAY(Clay_SharedElementConfig, Clay__SharedElementConfigArray)
DEFINE_TYPED_ARRAY_FUNCTIONS(Clay_RenderCommand, Clay_RenderCommandArray)

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
	Clay_TextElementConfig *textElementConfig;
	Clay_ImageElementConfig *imageElementConfig;
	Clay_FloatingElementConfig *floatingElementConfig;
	Clay_CustomElementConfig *customElementConfig;
	Clay_ScrollElementConfig *scrollElementConfig;
	Clay_BorderElementConfig *borderElementConfig;
	Clay_SharedElementConfig *sharedElementConfig;
};

typedef struct Clay_ElementConfig Clay_ElementConfig;
struct Clay_ElementConfig
{
	Clay__ElementConfigType type;
	Clay_ElementConfigUnion config;
};

DEFINE_TYPED_ARRAY(Clay_ElementConfig, Clay__ElementConfigArray)

typedef struct Clay__WrappedTextLine Clay__WrappedTextLine;
struct Clay__WrappedTextLine
{
	v2 dimensions;
	Str8 line;
};

DEFINE_TYPED_ARRAY(Clay__WrappedTextLine, Clay__WrappedTextLineArray)

typedef struct Clay__TextElementData Clay__TextElementData;
struct Clay__TextElementData
{
	Str8 text;
	v2 preferredDimensions;
	int32_t elementIndex;
	Clay__WrappedTextLineArraySlice wrappedLines;
};

DEFINE_TYPED_ARRAY(Clay__TextElementData, Clay__TextElementDataArray)

typedef struct Clay__LayoutElementChildren Clay__LayoutElementChildren;
struct Clay__LayoutElementChildren
{
	int32_t *elements;
	uint16_t length;
};

typedef struct Clay_LayoutElement Clay_LayoutElement;
struct Clay_LayoutElement
{
	union {
		Clay__LayoutElementChildren children;
		Clay__TextElementData *textElementData;
	} childrenOrTextContent;
	v2 dimensions;
	v2 minDimensions;
	Clay_LayoutConfig *layoutConfig;
	Clay__ElementConfigArraySlice elementConfigs;
	uint32_t id;
};

DEFINE_TYPED_ARRAY(Clay_LayoutElement, Clay_LayoutElementArray)

typedef struct Clay__ScrollContainerDataInternal Clay__ScrollContainerDataInternal;
struct Clay__ScrollContainerDataInternal
{
	Clay_LayoutElement *layoutElement;
	rec boundingBox;
	v2 contentSize;
	v2 scrollOrigin;
	v2 pointerOrigin;
	v2 scrollMomentum;
	v2 scrollTarget;
	v2 scrollPosition;
	v2 previousDelta;
	float momentumTime;
	float scrollLag;
	uint32_t elementId;
	bool openThisFrame;
	bool pointerScrollActive;
};

DEFINE_TYPED_ARRAY(Clay__ScrollContainerDataInternal, Clay__ScrollContainerDataInternalArray)

typedef struct Clay__DebugElementData Clay__DebugElementData;
struct Clay__DebugElementData
{
	bool collision;
	bool collapsed;
};

DEFINE_TYPED_ARRAY(Clay__DebugElementData, Clay__DebugElementDataArray)

typedef struct Clay_LayoutElementHashMapItem Clay_LayoutElementHashMapItem;
struct Clay_LayoutElementHashMapItem // todo get this struct into a single cache line
{
	rec boundingBox;
	Clay_ElementId elementId;
	Clay_LayoutElement* layoutElement;
	void (*onHoverFunction)(Clay_ElementId elementId, Clay_PointerData pointerInfo, CLAY_ONHOVER_USERDATA_TYPE userData);
	CLAY_ONHOVER_USERDATA_TYPE hoverFunctionUserData;
	int32_t nextIndex;
	uint32_t generation;
	uint32_t idAlias;
	Clay__DebugElementData *debugData;
};

DEFINE_TYPED_ARRAY(Clay_LayoutElementHashMapItem, Clay__LayoutElementHashMapItemArray)

typedef struct Clay__MeasuredWord Clay__MeasuredWord;
struct Clay__MeasuredWord
{
	int32_t startOffset;
	int32_t length;
	float width;
	int32_t next;
};

DEFINE_TYPED_ARRAY(Clay__MeasuredWord, Clay__MeasuredWordArray)

typedef struct Clay__MeasureTextCacheItem Clay__MeasureTextCacheItem;
struct Clay__MeasureTextCacheItem
{
	v2 unwrappedDimensions;
	int32_t measuredWordsStartIndex;
	bool containsNewlines;
	// Hash map data
	uint32_t id;
	int32_t nextIndex;
	uint32_t generation;
};

DEFINE_TYPED_ARRAY(Clay__MeasureTextCacheItem, Clay__MeasureTextCacheItemArray)

typedef struct Clay__LayoutElementTreeNode Clay__LayoutElementTreeNode;
struct Clay__LayoutElementTreeNode
{
	Clay_LayoutElement *layoutElement;
	v2 position;
	v2 nextChildOffset;
};

DEFINE_TYPED_ARRAY(Clay__LayoutElementTreeNode, Clay__LayoutElementTreeNodeArray)

typedef struct Clay__LayoutElementTreeRoot Clay__LayoutElementTreeRoot;
struct Clay__LayoutElementTreeRoot
{
	int32_t layoutElementIndex;
	uint32_t parentId; // This can be zero in the case of the root layout tree
	uint32_t clipElementId; // This can be zero if there is no clip element
	int16_t zIndex;
	v2 pointerOffset; // Only used when scroll containers are managed externally
};

DEFINE_TYPED_ARRAY(Clay__LayoutElementTreeRoot, Clay__LayoutElementTreeRootArray)

struct Clay_Context {
	int32_t maxElementCount;
	int32_t maxMeasureTextCacheWordCount;
	bool warningsEnabled;
	Clay_ErrorHandler errorHandler;
	Clay_BooleanWarnings booleanWarnings;
	Clay__WarningArray warnings;

	Clay_PointerData pointerInfo;
	v2 layoutDimensions;
	Clay_ElementId dynamicElementIndexBaseHash;
	uint32_t dynamicElementIndex;
	bool debugModeEnabled;
	bool disableCulling;
	bool externalScrollHandlingEnabled;
	uint32_t debugSelectedElementId;
	uint32_t generation;
	uintptr_t arenaResetOffset;
	CLAY_MEASURE_USERDATA_TYPE measureTextUserData;
	CLAY_QUERYSCROLL_USERDATA_TYPE queryScrollOffsetUserData;
	Clay_Arena internalArena;
	// Layout Elements / Render Commands
	Clay_LayoutElementArray layoutElements;
	Clay_RenderCommandArray renderCommands;
	Clay__int32_tArray openLayoutElementStack;
	Clay__int32_tArray layoutElementChildren;
	Clay__int32_tArray layoutElementChildrenBuffer;
	Clay__TextElementDataArray textElementData;
	Clay__int32_tArray imageElementPointers;
	Clay__int32_tArray reusableElementIndexBuffer;
	Clay__int32_tArray layoutElementClipElementIds;
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
	Clay__Str8Array layoutElementIdStrings;
	Clay__WrappedTextLineArray wrappedTextLines;
	Clay__LayoutElementTreeNodeArray layoutElementTreeNodeArray1;
	Clay__LayoutElementTreeRootArray layoutElementTreeRoots;
	Clay__LayoutElementHashMapItemArray layoutElementsHashMapInternal;
	Clay__int32_tArray layoutElementsHashMap;
	Clay__MeasureTextCacheItemArray measureTextHashMapInternal;
	Clay__int32_tArray measureTextHashMapInternalFreeList;
	Clay__int32_tArray measureTextHashMap;
	Clay__MeasuredWordArray measuredWords;
	Clay__int32_tArray measuredWordsFreeList;
	Clay__int32_tArray openClipElementStack;
	Clay__ElementIdArray pointerOverIds;
	Clay__ScrollContainerDataInternalArray scrollContainerDatas;
	Clay__boolArray treeNodeVisited;
	Clay__charArray dynamicStringData;
	Clay__DebugElementDataArray debugElementData;
};

Clay_Context* Clay__Context_Allocate_Arena(Clay_Arena *arena)
{
	size_t totalSizeBytes = sizeof(Clay_Context);
	uintptr_t memoryAddress = (uintptr_t)arena->memory;
	// Make sure the memory address passed in for clay to use is cache line aligned
	uintptr_t nextAllocOffset = (memoryAddress % 64);
	if (nextAllocOffset + totalSizeBytes > arena->capacity)
	{
		return NULL;
	}
	arena->nextAllocation = nextAllocOffset + totalSizeBytes;
	return (Clay_Context*)(memoryAddress + nextAllocOffset);
}

Str8 Clay__WriteStringToCharBuffer(Clay__charArray *buffer, Str8 string)
{
	for (int32_t i = 0; i < string.length; i++)
	{
		buffer->items[buffer->length + i] = string.chars[i];
	}
	buffer->length += string.length;
	return NewStr8(string.length, buffer->items + buffer->length - string.length);
}

#ifdef CLAY_WASM
	__attribute__((import_module("clay"), import_name("measureTextFunction"))) ClayMeasureText_f* Clay__MeasureText;
	__attribute__((import_module("clay"), import_name("queryScrollOffsetFunction"))) v2 Clay__QueryScrollOffset(uint32_t elementId, CLAY_QUERYSCROLL_USERDATA_TYPE userData);
#else
	ClayMeasureText_f* Clay__MeasureText;
	v2 (*Clay__QueryScrollOffset)(uint32_t elementId, CLAY_QUERYSCROLL_USERDATA_TYPE userData);
#endif

Clay_LayoutElement* Clay__GetOpenLayoutElement(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	return Clay_LayoutElementArray_Get(&context->layoutElements, Clay__int32_tArray_GetValue(&context->openLayoutElementStack, context->openLayoutElementStack.length - 1));
}
CLAY_DECOR uint32_t Clay__GetParentElementId(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	return Clay_LayoutElementArray_Get(&context->layoutElements, Clay__int32_tArray_GetValue(&context->openLayoutElementStack, context->openLayoutElementStack.length - 2))->id;
}

Clay_LayoutConfig * Clay__StoreLayoutConfig(Clay_LayoutConfig config) {  return Clay_GetCurrentContext()->booleanWarnings.maxElementsExceeded ? &CLAY_LAYOUT_DEFAULT : Clay__LayoutConfigArray_Add(&Clay_GetCurrentContext()->layoutConfigs, config); }
CLAY_DECOR Clay_TextElementConfig * Clay__StoreTextElementConfig(Clay_TextElementConfig config) {  return Clay_GetCurrentContext()->booleanWarnings.maxElementsExceeded ? &Clay_TextElementConfig_DEFAULT : Clay__TextElementConfigArray_Add(&Clay_GetCurrentContext()->textElementConfigs, config); }
Clay_ImageElementConfig * Clay__StoreImageElementConfig(Clay_ImageElementConfig config) {  return Clay_GetCurrentContext()->booleanWarnings.maxElementsExceeded ? &Clay_ImageElementConfig_DEFAULT : Clay__ImageElementConfigArray_Add(&Clay_GetCurrentContext()->imageElementConfigs, config); }
Clay_FloatingElementConfig * Clay__StoreFloatingElementConfig(Clay_FloatingElementConfig config) {  return Clay_GetCurrentContext()->booleanWarnings.maxElementsExceeded ? &Clay_FloatingElementConfig_DEFAULT : Clay__FloatingElementConfigArray_Add(&Clay_GetCurrentContext()->floatingElementConfigs, config); }
Clay_CustomElementConfig * Clay__StoreCustomElementConfig(Clay_CustomElementConfig config) {  return Clay_GetCurrentContext()->booleanWarnings.maxElementsExceeded ? &Clay_CustomElementConfig_DEFAULT : Clay__CustomElementConfigArray_Add(&Clay_GetCurrentContext()->customElementConfigs, config); }
Clay_ScrollElementConfig * Clay__StoreScrollElementConfig(Clay_ScrollElementConfig config) {  return Clay_GetCurrentContext()->booleanWarnings.maxElementsExceeded ? &Clay_ScrollElementConfig_DEFAULT : Clay__ScrollElementConfigArray_Add(&Clay_GetCurrentContext()->scrollElementConfigs, config); }
Clay_BorderElementConfig * Clay__StoreBorderElementConfig(Clay_BorderElementConfig config) {  return Clay_GetCurrentContext()->booleanWarnings.maxElementsExceeded ? &Clay_BorderElementConfig_DEFAULT : Clay__BorderElementConfigArray_Add(&Clay_GetCurrentContext()->borderElementConfigs, config); }
Clay_SharedElementConfig * Clay__StoreSharedElementConfig(Clay_SharedElementConfig config) {  return Clay_GetCurrentContext()->booleanWarnings.maxElementsExceeded ? &Clay_SharedElementConfig_DEFAULT : Clay__SharedElementConfigArray_Add(&Clay_GetCurrentContext()->sharedElementConfigs, config); }

Clay_ElementConfig Clay__AttachElementConfig(Clay_ElementConfigUnion config, Clay__ElementConfigType type)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->booleanWarnings.maxElementsExceeded) { return NEW_STRUCT(Clay_ElementConfig) ZEROED; }
	Clay_LayoutElement *openLayoutElement = Clay__GetOpenLayoutElement();
	openLayoutElement->elementConfigs.length++;
	return *Clay__ElementConfigArray_Add(&context->elementConfigs, NEW_STRUCT(Clay_ElementConfig) { .type = type, .config = config });
}

Clay_ElementConfigUnion Clay__FindElementConfigWithType(Clay_LayoutElement *element, Clay__ElementConfigType type)
{
	for (int32_t i = 0; i < element->elementConfigs.length; i++)
	{
		Clay_ElementConfig *config = Clay__ElementConfigArraySlice_Get(&element->elementConfigs, i);
		if (config->type == type) { return config->config; }
	}
	return NEW_STRUCT(Clay_ElementConfigUnion) { NULL };
}

Clay_ElementId Clay__HashNumber(const uint32_t offset, const uint32_t seed)
{
	uint32_t hash = seed;
	hash += (offset + 48);
	hash += (hash << 10);
	hash ^= (hash >> 6);

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return NEW_STRUCT(Clay_ElementId) { .id = hash + 1, .offset = offset, .baseId = seed, .stringId = Str8_Empty }; // Reserve the hash result of zero as "null id"
}

CLAY_DECOR Clay_ElementId Clay__HashString(Str8 key, const uint32_t offset, const uint32_t seed)
{
	uint32_t hash = 0;
	uint32_t base = seed;

	for (int32_t i = 0; i < key.length; i++) {
		base += key.chars[i];
		base += (base << 10);
		base ^= (base >> 6);
	}
	hash = base;
	hash += offset;
	hash += (hash << 10);
	hash ^= (hash >> 6);

	hash += (hash << 3);
	base += (base << 3);
	hash ^= (hash >> 11);
	base ^= (base >> 11);
	hash += (hash << 15);
	base += (base << 15);
	return NEW_STRUCT(Clay_ElementId) { .id = hash + 1, .offset = offset, .baseId = base + 1, .stringId = key }; // Reserve the hash result of zero as "null id"
}

uint32_t Clay__HashTextWithConfig(Str8 *text, Clay_TextElementConfig *config)
{
	uint32_t hash = 0;
	uintptr_t pointerAsNumber = (uintptr_t)text->chars;

	if (config->hashStringContents)
	{
		uint32_t maxLengthToHash = MinI32(text->length, 256);
		for (uint32_t i = 0; i < maxLengthToHash; i++)
		{
			hash += text->chars[i];
			hash += (hash << 10);
			hash ^= (hash >> 6);
		}
	}
	else
	{
		hash += pointerAsNumber;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += text->length;
	hash += (hash << 10);
	hash ^= (hash >> 6);

	hash += config->fontId;
	hash += (hash << 10);
	hash ^= (hash >> 6);

	hash += config->fontSize;
	hash += (hash << 10);
	hash ^= (hash >> 6);

	hash += config->lineHeight;
	hash += (hash << 10);
	hash ^= (hash >> 6);

	hash += config->letterSpacing;
	hash += (hash << 10);
	hash ^= (hash >> 6);

	hash += config->wrapMode;
	hash += (hash << 10);
	hash ^= (hash >> 6);
	
	//TODO: We may want to hash the userData pointer here?

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash + 1; // Reserve the hash result of zero as "null id"
}

Clay__MeasuredWord *Clay__AddMeasuredWord(Clay__MeasuredWord word, Clay__MeasuredWord *previousWord)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->measuredWordsFreeList.length > 0)
	{
		uint32_t newItemIndex = Clay__int32_tArray_GetValue(&context->measuredWordsFreeList, (int)context->measuredWordsFreeList.length - 1);
		context->measuredWordsFreeList.length--;
		Clay__MeasuredWordArray_Set(&context->measuredWords, (int)newItemIndex, word);
		previousWord->next = (int32_t)newItemIndex;
		return Clay__MeasuredWordArray_Get(&context->measuredWords, (int)newItemIndex);
	}
	else
	{
		previousWord->next = (int32_t)context->measuredWords.length;
		return Clay__MeasuredWordArray_Add(&context->measuredWords, word);
	}
}

Clay__MeasureTextCacheItem *Clay__MeasureTextCached(Str8 *text, Clay_TextElementConfig *config)
{
	Clay_Context* context = Clay_GetCurrentContext();
	#ifndef CLAY_WASM
	if (!Clay__MeasureText)
	{
		if (!context->booleanWarnings.textMeasurementFunctionNotSet)
		{
			context->booleanWarnings.textMeasurementFunctionNotSet = true;
			context->errorHandler.errorHandlerFunction(NEW_STRUCT(Clay_ErrorData) {
				.errorType = CLAY_ERROR_TYPE_TEXT_MEASUREMENT_FUNCTION_NOT_PROVIDED,
				.errorText = CLAY_STRING("Clay's internal MeasureText function is null. You may have forgotten to call Clay_SetMeasureTextFunction(), or passed a NULL function pointer by mistake."),
				.userData = context->errorHandler.userData
			});
		}
		return &Clay__MeasureTextCacheItem_DEFAULT;
	}
	#endif
	uint32_t id = Clay__HashTextWithConfig(text, config);
	uint32_t hashBucket = id % (context->maxMeasureTextCacheWordCount / 32);
	int32_t elementIndexPrevious = 0;
	int32_t elementIndex = context->measureTextHashMap.items[hashBucket];
	while (elementIndex != 0)
	{
		Clay__MeasureTextCacheItem *hashEntry = Clay__MeasureTextCacheItemArray_Get(&context->measureTextHashMapInternal, elementIndex);
		if (hashEntry->id == id)
		{
			hashEntry->generation = context->generation;
			return hashEntry;
		}
		// This element hasn't been seen in a few frames, delete the hash map item
		if (context->generation - hashEntry->generation > 2)
		{
			// Add all the measured words that were included in this measurement to the freelist
			int32_t nextWordIndex = hashEntry->measuredWordsStartIndex;
			while (nextWordIndex != -1)
			{
				Clay__MeasuredWord *measuredWord = Clay__MeasuredWordArray_Get(&context->measuredWords, nextWordIndex);
				Clay__int32_tArray_Add(&context->measuredWordsFreeList, nextWordIndex);
				nextWordIndex = measuredWord->next;
			}

			int32_t nextIndex = hashEntry->nextIndex;
			Clay__MeasureTextCacheItemArray_Set(&context->measureTextHashMapInternal, elementIndex, NEW_STRUCT(Clay__MeasureTextCacheItem) { .measuredWordsStartIndex = -1 });
			Clay__int32_tArray_Add(&context->measureTextHashMapInternalFreeList, elementIndex);
			if (elementIndexPrevious == 0)
			{
				context->measureTextHashMap.items[hashBucket] = nextIndex;
			}
			else
			{
				Clay__MeasureTextCacheItem *previousHashEntry = Clay__MeasureTextCacheItemArray_Get(&context->measureTextHashMapInternal, elementIndexPrevious);
				previousHashEntry->nextIndex = nextIndex;
			}
			elementIndex = nextIndex;
		}
		else
		{
			elementIndexPrevious = elementIndex;
			elementIndex = hashEntry->nextIndex;
		}
	}

	int32_t newItemIndex = 0;
	Clay__MeasureTextCacheItem newCacheItem = { .measuredWordsStartIndex = -1, .id = id, .generation = context->generation };
	Clay__MeasureTextCacheItem *measured = NULL;
	if (context->measureTextHashMapInternalFreeList.length > 0)
	{
		newItemIndex = Clay__int32_tArray_GetValue(&context->measureTextHashMapInternalFreeList, context->measureTextHashMapInternalFreeList.length - 1);
		context->measureTextHashMapInternalFreeList.length--;
		Clay__MeasureTextCacheItemArray_Set(&context->measureTextHashMapInternal, newItemIndex, newCacheItem);
		measured = Clay__MeasureTextCacheItemArray_Get(&context->measureTextHashMapInternal, newItemIndex);
	}
	else
	{
		if (context->measureTextHashMapInternal.length == context->measureTextHashMapInternal.allocLength - 1)
		{
			if (context->booleanWarnings.maxTextMeasureCacheExceeded)
			{
				context->errorHandler.errorHandlerFunction(NEW_STRUCT(Clay_ErrorData) {
					.errorType = CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED,
					.errorText = CLAY_STRING("Clay ran out of capacity while attempting to measure text elements. Try using Clay_SetMaxElementCount() with a higher value."),
					.userData = context->errorHandler.userData
				});
				context->booleanWarnings.maxTextMeasureCacheExceeded = true;
			}
			return &Clay__MeasureTextCacheItem_DEFAULT;
		}
		measured = Clay__MeasureTextCacheItemArray_Add(&context->measureTextHashMapInternal, newCacheItem);
		newItemIndex = context->measureTextHashMapInternal.length - 1;
	}

	int32_t start = 0;
	int32_t end = 0;
	float lineWidth = 0;
	float measuredWidth = 0;
	float measuredHeight = 0;
	float spaceWidth = Clay__MeasureText(Str8_Space, config, context->measureTextUserData).Width;
	Clay__MeasuredWord tempWord = { .next = -1 };
	Clay__MeasuredWord *previousWord = &tempWord;
	while (end < text->length)
	{
		if (context->measuredWords.length == context->measuredWords.allocLength - 1)
		{
			if (!context->booleanWarnings.maxTextMeasureCacheExceeded)
			{
				context->errorHandler.errorHandlerFunction(NEW_STRUCT(Clay_ErrorData) {
					.errorType = CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED,
					.errorText = CLAY_STRING("Clay has run out of space in it's internal text measurement cache. Try using Clay_SetMaxMeasureTextCacheWordCount() (default 16384, with 1 unit storing 1 measured word)."),
					.userData = context->errorHandler.userData
				});
				context->booleanWarnings.maxTextMeasureCacheExceeded = true;
			}
			return &Clay__MeasureTextCacheItem_DEFAULT;
		}
		char current = text->chars[end];
		if (current == ' ' || current == '\n')
		{
			int32_t length = end - start;
			v2 dimensions = Clay__MeasureText(StrSliceLength(*text, start, length), config, context->measureTextUserData);
			measuredHeight = MaxR32(measuredHeight, dimensions.Height);
			if (current == ' ')
			{
				dimensions.Width += spaceWidth;
				previousWord = Clay__AddMeasuredWord(NEW_STRUCT(Clay__MeasuredWord) { .startOffset = start, .length = length + 1, .width = dimensions.Width, .next = -1 }, previousWord);
				lineWidth += dimensions.Width;
			}
			if (current == '\n')
			{
				if (length > 0)
				{
					previousWord = Clay__AddMeasuredWord(NEW_STRUCT(Clay__MeasuredWord) { .startOffset = start, .length = length, .width = dimensions.Width, .next = -1 }, previousWord);
				}
				previousWord = Clay__AddMeasuredWord(NEW_STRUCT(Clay__MeasuredWord) { .startOffset = end + 1, .length = 0, .width = 0, .next = -1 }, previousWord);
				lineWidth += dimensions.Width;
				measuredWidth = MaxR32(lineWidth, measuredWidth);
				measured->containsNewlines = true;
				lineWidth = 0;
			}
			start = end + 1;
		}
		end++;
	}
	if (end - start > 0)
	{
		v2 dimensions = Clay__MeasureText(StrSlice(*text, start, end), config, context->measureTextUserData);
		Clay__AddMeasuredWord(NEW_STRUCT(Clay__MeasuredWord) { .startOffset = start, .length = end - start, .width = dimensions.Width, .next = -1 }, previousWord);
		lineWidth += dimensions.Width;
		measuredHeight = MaxR32(measuredHeight, dimensions.Height);
	}
	measuredWidth = MaxR32(lineWidth, measuredWidth);

	measured->measuredWordsStartIndex = tempWord.next;
	measured->unwrappedDimensions.Width = measuredWidth;
	measured->unwrappedDimensions.Height = measuredHeight;

	if (elementIndexPrevious != 0)
	{
		Clay__MeasureTextCacheItemArray_Get(&context->measureTextHashMapInternal, elementIndexPrevious)->nextIndex = newItemIndex;
	}
	else
	{
		context->measureTextHashMap.items[hashBucket] = newItemIndex;
	}
	return measured;
}

bool Clay__PointIsInsideRect(v2 point, rec rect)
{
	return point.X >= rect.X && point.X <= rect.X + rect.Width && point.Y >= rect.Y && point.Y <= rect.Y + rect.Height;
}

Clay_LayoutElementHashMapItem* Clay__AddHashMapItem(Clay_ElementId elementId, Clay_LayoutElement* layoutElement, uint32_t idAlias)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->layoutElementsHashMapInternal.length == context->layoutElementsHashMapInternal.allocLength - 1) { return NULL; }
	Clay_LayoutElementHashMapItem item = { .elementId = elementId, .layoutElement = layoutElement, .nextIndex = -1, .generation = context->generation + 1, .idAlias = idAlias };
	uint32_t hashBucket = elementId.id % context->layoutElementsHashMap.allocLength;
	int32_t hashItemPrevious = -1;
	int32_t hashItemIndex = context->layoutElementsHashMap.items[hashBucket];
	while (hashItemIndex != -1) // Just replace collision, not a big deal - leave it up to the end user
	{
		Clay_LayoutElementHashMapItem *hashItem = Clay__LayoutElementHashMapItemArray_Get(&context->layoutElementsHashMapInternal, hashItemIndex);
		if (hashItem->elementId.id == elementId.id) // Collision - resolve based on generation
		{
			item.nextIndex = hashItem->nextIndex;
			if (hashItem->generation <= context->generation) // First collision - assume this is the "same" element
			{
				hashItem->elementId = elementId; // Make sure to copy this across. If the stringId reference has changed, we should update the hash item to use the new one.
				hashItem->generation = context->generation + 1;
				hashItem->layoutElement = layoutElement;
				hashItem->debugData->collision = false;
			}
			else // Multiple collisions this frame - two elements have the same ID
			{
				context->errorHandler.errorHandlerFunction(NEW_STRUCT(Clay_ErrorData) {
					.errorType = CLAY_ERROR_TYPE_DUPLICATE_ID,
					.errorText = CLAY_STRING("An element with this ID was already previously declared during this layout."),
					.userData = context->errorHandler.userData
				});
				if (context->debugModeEnabled) { hashItem->debugData->collision = true; }
			}
			return hashItem;
		}
		hashItemPrevious = hashItemIndex;
		hashItemIndex = hashItem->nextIndex;
	}
	Clay_LayoutElementHashMapItem *hashItem = Clay__LayoutElementHashMapItemArray_Add(&context->layoutElementsHashMapInternal, item);
	hashItem->debugData = Clay__DebugElementDataArray_Add(&context->debugElementData, NEW_STRUCT(Clay__DebugElementData) ZEROED);
	if (hashItemPrevious != -1)
	{
		Clay__LayoutElementHashMapItemArray_Get(&context->layoutElementsHashMapInternal, hashItemPrevious)->nextIndex = (int32_t)context->layoutElementsHashMapInternal.length - 1;
	}
	else
	{
		context->layoutElementsHashMap.items[hashBucket] = (int32_t)context->layoutElementsHashMapInternal.length - 1;
	}
	return hashItem;
}

Clay_LayoutElementHashMapItem *Clay__GetHashMapItem(uint32_t id)
{
	Clay_Context* context = Clay_GetCurrentContext();
	uint32_t hashBucket = id % context->layoutElementsHashMap.allocLength;
	int32_t elementIndex = context->layoutElementsHashMap.items[hashBucket];
	while (elementIndex != -1)
	{
		Clay_LayoutElementHashMapItem *hashEntry = Clay__LayoutElementHashMapItemArray_Get(&context->layoutElementsHashMapInternal, elementIndex);
		if (hashEntry->elementId.id == id) { return hashEntry; }
		elementIndex = hashEntry->nextIndex;
	}
	return &Clay_LayoutElementHashMapItem_DEFAULT;
}

Clay_ElementId Clay__GenerateIdForAnonymousElement(Clay_LayoutElement *openLayoutElement)
{
	Clay_Context* context = Clay_GetCurrentContext();
	Clay_LayoutElement *parentElement = Clay_LayoutElementArray_Get(&context->layoutElements, Clay__int32_tArray_GetValue(&context->openLayoutElementStack, context->openLayoutElementStack.length - 2));
	Clay_ElementId elementId = Clay__HashNumber(parentElement->childrenOrTextContent.children.length, parentElement->id);
	openLayoutElement->id = elementId.id;
	Clay__AddHashMapItem(elementId, openLayoutElement, 0);
	Clay__Str8Array_Add(&context->layoutElementIdStrings, elementId.stringId);
	return elementId;
}

bool Clay__ElementHasConfig(Clay_LayoutElement *layoutElement, Clay__ElementConfigType type)
{
	for (int32_t i = 0; i < layoutElement->elementConfigs.length; i++)
	{
		if (Clay__ElementConfigArraySlice_Get(&layoutElement->elementConfigs, i)->type == type) { return true; }
	}
	return false;
}

void Clay__UpdateAspectRatioBox(Clay_LayoutElement *layoutElement)
{
	for (int32_t j = 0; j < layoutElement->elementConfigs.length; j++)
	{
		Clay_ElementConfig *config = Clay__ElementConfigArraySlice_Get(&layoutElement->elementConfigs, j);
		if (config->type == CLAY__ELEMENT_CONFIG_TYPE_IMAGE)
		{
			Clay_ImageElementConfig *imageConfig = config->config.imageElementConfig;
			if (imageConfig->sourceDimensions.Width == 0 || imageConfig->sourceDimensions.Height == 0) { break; }
			float aspect = imageConfig->sourceDimensions.Width / imageConfig->sourceDimensions.Height;
			if (layoutElement->dimensions.Width == 0 && layoutElement->dimensions.Height != 0)
			{
				layoutElement->dimensions.Width = layoutElement->dimensions.Height * aspect;
			}
			else if (layoutElement->dimensions.Width != 0 && layoutElement->dimensions.Height == 0)
			{
				layoutElement->dimensions.Height = layoutElement->dimensions.Height * (1 / aspect);
			}
			break;
		}
	}
}

CLAY_DECOR void Clay__CloseElement(void) {
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->booleanWarnings.maxElementsExceeded) { return; }
	Clay_LayoutElement *openLayoutElement = Clay__GetOpenLayoutElement();
	Clay_LayoutConfig *layoutConfig = openLayoutElement->layoutConfig;
	bool elementHasScrollHorizontal = false;
	bool elementHasScrollVertical = false;
	for (int32_t i = 0; i < openLayoutElement->elementConfigs.length; i++)
	{
		Clay_ElementConfig *config = Clay__ElementConfigArraySlice_Get(&openLayoutElement->elementConfigs, i);
		if (config->type == CLAY__ELEMENT_CONFIG_TYPE_SCROLL)
		{
			elementHasScrollHorizontal = config->config.scrollElementConfig->horizontal;
			elementHasScrollVertical = config->config.scrollElementConfig->vertical;
			context->openClipElementStack.length--;
			break;
		}
	}

	// Attach children to the current open element
	openLayoutElement->childrenOrTextContent.children.elements = &context->layoutElementChildren.items[context->layoutElementChildren.length];
	if (layoutConfig->layoutDirection == CLAY_LEFT_TO_RIGHT)
	{
		openLayoutElement->dimensions.Width = (float)(layoutConfig->padding.left + layoutConfig->padding.right);
		for (int32_t i = 0; i < openLayoutElement->childrenOrTextContent.children.length; i++)
		{
			int32_t childIndex = Clay__int32_tArray_GetValue(&context->layoutElementChildrenBuffer, (int)context->layoutElementChildrenBuffer.length - openLayoutElement->childrenOrTextContent.children.length + i);
			Clay_LayoutElement *child = Clay_LayoutElementArray_Get(&context->layoutElements, childIndex);
			openLayoutElement->dimensions.Width += child->dimensions.Width;
			openLayoutElement->dimensions.Height = MaxR32(openLayoutElement->dimensions.Height, child->dimensions.Height + layoutConfig->padding.top + layoutConfig->padding.bottom);
			// Minimum size of child elements doesn't matter to scroll containers as they can shrink and hide their contents
			if (!elementHasScrollHorizontal)
			{
				openLayoutElement->minDimensions.Width += child->minDimensions.Width;
			}
			if (!elementHasScrollVertical)
			{
				openLayoutElement->minDimensions.Height = MaxR32(openLayoutElement->minDimensions.Height, child->minDimensions.Height + layoutConfig->padding.top + layoutConfig->padding.bottom);
			}
			Clay__int32_tArray_Add(&context->layoutElementChildren, childIndex);
		}
		float childGap = (float)(MaxU16(openLayoutElement->childrenOrTextContent.children.length - 1, 0) * layoutConfig->childGap);
		openLayoutElement->dimensions.Width += childGap; // TODO this is technically a bug with childgap and scroll containers
		openLayoutElement->minDimensions.Width += childGap;
	}
	else if (layoutConfig->layoutDirection == CLAY_TOP_TO_BOTTOM)
	{
		openLayoutElement->dimensions.Height = (float)(layoutConfig->padding.top + layoutConfig->padding.bottom);
		for (int32_t i = 0; i < openLayoutElement->childrenOrTextContent.children.length; i++)
		{
			int32_t childIndex = Clay__int32_tArray_GetValue(&context->layoutElementChildrenBuffer, (int)context->layoutElementChildrenBuffer.length - openLayoutElement->childrenOrTextContent.children.length + i);
			Clay_LayoutElement *child = Clay_LayoutElementArray_Get(&context->layoutElements, childIndex);
			openLayoutElement->dimensions.Height += child->dimensions.Height;
			openLayoutElement->dimensions.Width = MaxR32(openLayoutElement->dimensions.Width, child->dimensions.Width + layoutConfig->padding.left + layoutConfig->padding.right);
			// Minimum size of child elements doesn't matter to scroll containers as they can shrink and hide their contents
			if (!elementHasScrollVertical)
			{
				openLayoutElement->minDimensions.Height += child->minDimensions.Height;
			}
			if (!elementHasScrollHorizontal)
			{
				openLayoutElement->minDimensions.Width = MaxR32(openLayoutElement->minDimensions.Width, child->minDimensions.Width + layoutConfig->padding.left + layoutConfig->padding.right);
			}
			Clay__int32_tArray_Add(&context->layoutElementChildren, childIndex);
		}
		float childGap = (float)(MaxU16(openLayoutElement->childrenOrTextContent.children.length - 1, 0) * layoutConfig->childGap);
		openLayoutElement->dimensions.Height += childGap; // TODO this is technically a bug with childgap and scroll containers
		openLayoutElement->minDimensions.Height += childGap;
	}

	context->layoutElementChildrenBuffer.length -= openLayoutElement->childrenOrTextContent.children.length;

	// Clamp element min and max width to the values configured in the layout
	if (layoutConfig->sizing.width.type != CLAY__SIZING_TYPE_PERCENT)
	{
		if (layoutConfig->sizing.width.size.minMax.max <= 0) // Set the max size if the user didn't specify, makes calculations easier
		{
			layoutConfig->sizing.width.size.minMax.max = CLAY__MAXFLOAT;
		}
		openLayoutElement->dimensions.Width = MinR32(MaxR32(openLayoutElement->dimensions.Width, layoutConfig->sizing.width.size.minMax.min), layoutConfig->sizing.width.size.minMax.max);
		openLayoutElement->minDimensions.Width = MinR32(MaxR32(openLayoutElement->minDimensions.Width, layoutConfig->sizing.width.size.minMax.min), layoutConfig->sizing.width.size.minMax.max);
	}
	else
	{
		openLayoutElement->dimensions.Width = 0;
	}

	// Clamp element min and max height to the values configured in the layout
	if (layoutConfig->sizing.height.type != CLAY__SIZING_TYPE_PERCENT)
	{
		if (layoutConfig->sizing.height.size.minMax.max <= 0) // Set the max size if the user didn't specify, makes calculations easier
		{
			layoutConfig->sizing.height.size.minMax.max = CLAY__MAXFLOAT;
		}
		openLayoutElement->dimensions.Height = MinR32(MaxR32(openLayoutElement->dimensions.Height, layoutConfig->sizing.height.size.minMax.min), layoutConfig->sizing.height.size.minMax.max);
		openLayoutElement->minDimensions.Height = MinR32(MaxR32(openLayoutElement->minDimensions.Height, layoutConfig->sizing.height.size.minMax.min), layoutConfig->sizing.height.size.minMax.max);
	}
	else
	{
		openLayoutElement->dimensions.Height = 0;
	}

	Clay__UpdateAspectRatioBox(openLayoutElement);

	bool elementIsFloating = Clay__ElementHasConfig(openLayoutElement, CLAY__ELEMENT_CONFIG_TYPE_FLOATING);

	// Close the currently open element
	int32_t closingElementIndex = Clay__int32_tArray_RemoveSwapback(&context->openLayoutElementStack, (int)context->openLayoutElementStack.length - 1);
	openLayoutElement = Clay__GetOpenLayoutElement();

	if (!elementIsFloating && context->openLayoutElementStack.length > 1)
	{
		openLayoutElement->childrenOrTextContent.children.length++;
		Clay__int32_tArray_Add(&context->layoutElementChildrenBuffer, closingElementIndex);
	}
}

bool Clay__MemCmp(const char *s1, const char *s2, int32_t length);
#if !defined(CLAY_DISABLE_SIMD) && (defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64))
	bool Clay__MemCmp(const char *s1, const char *s2, int32_t length)
	{
		while (length >= 16)
		{
			__m128i v1 = _mm_loadu_si128((const __m128i *)s1);
			__m128i v2 = _mm_loadu_si128((const __m128i *)s2);
			
			if (_mm_movemask_epi8(_mm_cmpeq_epi8(v1, v2)) != 0xFFFF) { return false; } // If any byte differs
			
			s1 += 16;
			s2 += 16;
			length -= 16;
		}
		
		// Handle remaining bytes
		while (length--)
		{
			if (*s1 != *s2) { return false; }
			s1++;
			s2++;
		}
		
		return true;
	}
#elif !defined(CLAY_DISABLE_SIMD) && defined(__aarch64__)
	bool Clay__MemCmp(const char *s1, const char *s2, int32_t length)
	{
		while (length >= 16)
		{
			uint8x16_t v1 = vld1q_u8((const uint8_t *)s1);
			uint8x16_t v2 = vld1q_u8((const uint8_t *)s2);

			// Compare vectors
			if (vminvq_u32(vceqq_u8(v1, v2)) != 0xFFFFFFFF) { return false; } // If there's a difference

			s1 += 16;
			s2 += 16;
			length -= 16;
		}

		// Handle remaining bytes
		while (length--)
		{
			if (*s1 != *s2) { return false; }
			s1++;
			s2++;
		}

		return true;
	}
#else
	bool Clay__MemCmp(const char *s1, const char *s2, int32_t length)
	{
		for (int32_t i = 0; i < length; i++)
		{
			if (s1[i] != s2[i]) { return false; }
		}
		return true;
	}
#endif

CLAY_DECOR void Clay__OpenElement(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->layoutElements.length == context->layoutElements.allocLength - 1 || context->booleanWarnings.maxElementsExceeded)
	{
		context->booleanWarnings.maxElementsExceeded = true;
		return;
	}
	Clay_LayoutElement layoutElement = ZEROED;
	Clay_LayoutElementArray_Add(&context->layoutElements, layoutElement);
	Clay__int32_tArray_Add(&context->openLayoutElementStack, context->layoutElements.length - 1);
	if (context->openClipElementStack.length > 0)
	{
		Clay__int32_tArray_Set(&context->layoutElementClipElementIds, context->layoutElements.length - 1, Clay__int32_tArray_GetValue(&context->openClipElementStack, (int)context->openClipElementStack.length - 1));
	}
	else
	{
		Clay__int32_tArray_Set(&context->layoutElementClipElementIds, context->layoutElements.length - 1, 0);
	}
}

CLAY_DECOR void Clay__OpenTextElement(Str8 text, Clay_TextElementConfig *textConfig)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->layoutElements.length == context->layoutElements.allocLength - 1 || context->booleanWarnings.maxElementsExceeded)
	{
		context->booleanWarnings.maxElementsExceeded = true;
		return;
	}
	Clay_LayoutElement *parentElement = Clay__GetOpenLayoutElement();

	Clay_LayoutElement layoutElement = ZEROED;
	Clay_LayoutElement *textElement = Clay_LayoutElementArray_Add(&context->layoutElements, layoutElement);
	if (context->openClipElementStack.length > 0)
	{
		Clay__int32_tArray_Set(&context->layoutElementClipElementIds, context->layoutElements.length - 1, Clay__int32_tArray_GetValue(&context->openClipElementStack, (int)context->openClipElementStack.length - 1));
	}
	else
	{
		Clay__int32_tArray_Set(&context->layoutElementClipElementIds, context->layoutElements.length - 1, 0);
	}

	Clay__int32_tArray_Add(&context->layoutElementChildrenBuffer, context->layoutElements.length - 1);
	Clay__MeasureTextCacheItem *textMeasured = Clay__MeasureTextCached(&text, textConfig);
	Clay_ElementId elementId = Clay__HashNumber(parentElement->childrenOrTextContent.children.length, parentElement->id);
	textElement->id = elementId.id;
	Clay__AddHashMapItem(elementId, textElement, 0);
	Clay__Str8Array_Add(&context->layoutElementIdStrings, elementId.stringId);
	v2 textDimensions = NewV2(
		textMeasured->unwrappedDimensions.Width,
		textConfig->lineHeight > 0 ? (float)textConfig->lineHeight : textMeasured->unwrappedDimensions.Height
	);
	textElement->dimensions = textDimensions;
	textElement->minDimensions = NewV2(textMeasured->unwrappedDimensions.Height, textDimensions.Height); // TODO not sure this is the best way to decide min width for text
	textElement->childrenOrTextContent.textElementData = Clay__TextElementDataArray_Add(&context->textElementData, NEW_STRUCT(Clay__TextElementData) { .text = text, .preferredDimensions = textMeasured->unwrappedDimensions, .elementIndex = context->layoutElements.length - 1 });
	textElement->elementConfigs = NEW_STRUCT(Clay__ElementConfigArraySlice) {
		.length = 1,
		.items = Clay__ElementConfigArray_Add(&context->elementConfigs, NEW_STRUCT(Clay_ElementConfig) { .type = CLAY__ELEMENT_CONFIG_TYPE_TEXT, .config = { .textElementConfig = textConfig }})
	};
	textElement->layoutConfig = &CLAY_LAYOUT_DEFAULT;
	parentElement->childrenOrTextContent.children.length++;
}

Clay_ElementId Clay__AttachId(Clay_ElementId elementId)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->booleanWarnings.maxElementsExceeded)
	{
		return Clay_ElementId_DEFAULT;
	}
	Clay_LayoutElement *openLayoutElement = Clay__GetOpenLayoutElement();
	uint32_t idAlias = openLayoutElement->id;
	openLayoutElement->id = elementId.id;
	Clay__AddHashMapItem(elementId, openLayoutElement, idAlias);
	Clay__Str8Array_Add(&context->layoutElementIdStrings, elementId.stringId);
	return elementId;
}

CLAY_DECOR void Clay__ConfigureOpenElement(const Clay_ElementDeclaration declaration)
{
	Clay_Context* context = Clay_GetCurrentContext();
	Clay_LayoutElement *openLayoutElement = Clay__GetOpenLayoutElement();
	openLayoutElement->layoutConfig = Clay__StoreLayoutConfig(declaration.layout);
	if ((declaration.layout.sizing.width.type == CLAY__SIZING_TYPE_PERCENT && declaration.layout.sizing.width.size.percent > 1) ||
		(declaration.layout.sizing.height.type == CLAY__SIZING_TYPE_PERCENT && declaration.layout.sizing.height.size.percent > 1))
	{
		context->errorHandler.errorHandlerFunction(NEW_STRUCT(Clay_ErrorData) {
			.errorType = CLAY_ERROR_TYPE_PERCENTAGE_OVER_1,
			.errorText = CLAY_STRING("An element was configured with CLAY_SIZING_PERCENT, but the provided percentage value was over 1.0. Clay expects a value between 0 and 1, i.e. 20% is 0.2."),
			.userData = context->errorHandler.userData
		});
	}

	Clay_ElementId openLayoutElementId = declaration.id;

	openLayoutElement->elementConfigs.items = &context->elementConfigs.items[context->elementConfigs.length];
	Clay_SharedElementConfig *sharedConfig = NULL;
	if (declaration.backgroundColor.a > 0)
	{
		sharedConfig = Clay__StoreSharedElementConfig(NEW_STRUCT(Clay_SharedElementConfig) { .backgroundColor = declaration.backgroundColor });
		Clay__AttachElementConfig(NEW_STRUCT(Clay_ElementConfigUnion) { .sharedElementConfig = sharedConfig }, CLAY__ELEMENT_CONFIG_TYPE_SHARED);
	}
	if (!Clay__MemCmp((char *)(&declaration.cornerRadius), (char *)(&Clay__CornerRadius_DEFAULT), sizeof(Clay_CornerRadius)))
	{
		if (sharedConfig)
		{
			sharedConfig->cornerRadius = declaration.cornerRadius;
		}
		else
		{
			sharedConfig = Clay__StoreSharedElementConfig(NEW_STRUCT(Clay_SharedElementConfig) { .cornerRadius = declaration.cornerRadius });
			Clay__AttachElementConfig(NEW_STRUCT(Clay_ElementConfigUnion) { .sharedElementConfig = sharedConfig }, CLAY__ELEMENT_CONFIG_TYPE_SHARED);
		}
	}
	CLAY_ELEMENT_USERDATA_TYPE zeroUserDataType = ZEROED;
	if (!Clay__MemCmp((char *)(&declaration.userData), (char *)(&zeroUserDataType), sizeof(CLAY_ELEMENT_USERDATA_TYPE)))
	{
		if (sharedConfig)
		{
			sharedConfig->userData = declaration.userData;
		}
		else
		{
			sharedConfig = Clay__StoreSharedElementConfig(NEW_STRUCT(Clay_SharedElementConfig) { .userData = declaration.userData });
			Clay__AttachElementConfig(NEW_STRUCT(Clay_ElementConfigUnion) { .sharedElementConfig = sharedConfig }, CLAY__ELEMENT_CONFIG_TYPE_SHARED);
		}
	}
	CLAY_IMAGEDATA_TYPE zeroImageDataType = ZEROED;
	if (!Clay__MemCmp((char *)(&declaration.image.imageData), (char *)(&zeroImageDataType), sizeof(CLAY_IMAGEDATA_TYPE)))
	{
		Clay__AttachElementConfig(NEW_STRUCT(Clay_ElementConfigUnion) { .imageElementConfig = Clay__StoreImageElementConfig(declaration.image) }, CLAY__ELEMENT_CONFIG_TYPE_IMAGE);
		Clay__int32_tArray_Add(&context->imageElementPointers, context->layoutElements.length - 1);
	}
	if (declaration.floating.attachTo != CLAY_ATTACH_TO_NONE)
	{
		Clay_FloatingElementConfig floatingConfig = declaration.floating;
		// This looks dodgy but because of the auto generated root element the depth of the tree will always be at least 2 here
		Clay_LayoutElement *hierarchicalParent = Clay_LayoutElementArray_Get(&context->layoutElements, Clay__int32_tArray_GetValue(&context->openLayoutElementStack, context->openLayoutElementStack.length - 2));
		if (hierarchicalParent)
		{
			uint32_t clipElementId = 0;
			if (declaration.floating.attachTo == CLAY_ATTACH_TO_PARENT)
			{
				// Attach to the element's direct hierarchical parent
				floatingConfig.parentId = hierarchicalParent->id;
				if (context->openClipElementStack.length > 0)
				{
					clipElementId = Clay__int32_tArray_GetValue(&context->openClipElementStack, (int)context->openClipElementStack.length - 1);
				}
			}
			else if (declaration.floating.attachTo == CLAY_ATTACH_TO_ELEMENT_WITH_ID)
			{
				Clay_LayoutElementHashMapItem *parentItem = Clay__GetHashMapItem(floatingConfig.parentId);
				if (!parentItem)
				{
					context->errorHandler.errorHandlerFunction(NEW_STRUCT(Clay_ErrorData) {
						.errorType = CLAY_ERROR_TYPE_FLOATING_CONTAINER_PARENT_NOT_FOUND,
						.errorText = CLAY_STRING("A floating element was declared with a parentId, but no element with that ID was found."),
						.userData = context->errorHandler.userData
					});
				}
				else
				{
					clipElementId = Clay__int32_tArray_GetValue(&context->layoutElementClipElementIds, parentItem->layoutElement - context->layoutElements.items);
				}
			}
			else if (declaration.floating.attachTo == CLAY_ATTACH_TO_ROOT)
			{
				floatingConfig.parentId = Clay__HashString(CLAY_STRING("Clay__RootContainer"), 0, 0).id;
			}
			if (!openLayoutElementId.id)
			{
				openLayoutElementId = Clay__HashString(CLAY_STRING("Clay__FloatingContainer"), context->layoutElementTreeRoots.length, 0);
			}
			Clay__LayoutElementTreeRootArray_Add(&context->layoutElementTreeRoots, NEW_STRUCT(Clay__LayoutElementTreeRoot) {
				.layoutElementIndex = Clay__int32_tArray_GetValue(&context->openLayoutElementStack, context->openLayoutElementStack.length - 1),
				.parentId = floatingConfig.parentId,
				.clipElementId = clipElementId,
				.zIndex = floatingConfig.zIndex,
			});
			Clay__AttachElementConfig(NEW_STRUCT(Clay_ElementConfigUnion) { .floatingElementConfig = Clay__StoreFloatingElementConfig(declaration.floating) }, CLAY__ELEMENT_CONFIG_TYPE_FLOATING);
		}
	}
	if (declaration.custom.customData)
	{
		Clay__AttachElementConfig(NEW_STRUCT(Clay_ElementConfigUnion) { .customElementConfig = Clay__StoreCustomElementConfig(declaration.custom) }, CLAY__ELEMENT_CONFIG_TYPE_CUSTOM);
	}

	if (openLayoutElementId.id != 0)
	{
		Clay__AttachId(openLayoutElementId);
	}
	else if (openLayoutElement->id == 0)
	{
		openLayoutElementId = Clay__GenerateIdForAnonymousElement(openLayoutElement);
	}

	if (declaration.scroll.horizontal | declaration.scroll.vertical)
	{
		Clay__AttachElementConfig(NEW_STRUCT(Clay_ElementConfigUnion) { .scrollElementConfig = Clay__StoreScrollElementConfig(declaration.scroll) }, CLAY__ELEMENT_CONFIG_TYPE_SCROLL);
		Clay__int32_tArray_Add(&context->openClipElementStack, (int)openLayoutElement->id);
		// Retrieve or create cached data to track scroll position across frames
		Clay__ScrollContainerDataInternal *scrollOffset = CLAY__NULL;
		for (int32_t i = 0; i < context->scrollContainerDatas.length; i++)
		{
			Clay__ScrollContainerDataInternal *mapping = Clay__ScrollContainerDataInternalArray_Get(&context->scrollContainerDatas, i);
			if (openLayoutElement->id == mapping->elementId)
			{
				scrollOffset = mapping;
				scrollOffset->layoutElement = openLayoutElement;
				scrollOffset->openThisFrame = true;
				scrollOffset->scrollLag = declaration.scroll.scrollLag;
			}
		}
		if (!scrollOffset)
		{
			scrollOffset = Clay__ScrollContainerDataInternalArray_Add(&context->scrollContainerDatas, NEW_STRUCT(Clay__ScrollContainerDataInternal){.layoutElement = openLayoutElement, .scrollOrigin = {-1,-1}, .elementId = openLayoutElement->id, .scrollLag = declaration.scroll.scrollLag, .openThisFrame = true});
		}
		if (context->externalScrollHandlingEnabled)
		{
			scrollOffset->scrollTarget = Clay__QueryScrollOffset(scrollOffset->elementId, context->queryScrollOffsetUserData);
			scrollOffset->scrollPosition = scrollOffset->scrollTarget;
		}
	}
	if (!Clay__MemCmp((char *)(&declaration.border.width), (char *)(&Clay__BorderWidth_DEFAULT), sizeof(Clay_BorderWidth)))
	{
		Clay__AttachElementConfig(NEW_STRUCT(Clay_ElementConfigUnion) { .borderElementConfig = Clay__StoreBorderElementConfig(declaration.border) }, CLAY__ELEMENT_CONFIG_TYPE_BORDER);
	}
}

void Clay__InitializeEphemeralMemory(Clay_Context* context)
{
	int32_t maxElementCount = context->maxElementCount;
	// Ephemeral Memory - reset every frame
	Clay_Arena *arena = &context->internalArena;
	arena->nextAllocation = context->arenaResetOffset;

	context->layoutElementChildrenBuffer = Clay__int32_tArray_Allocate_Arena(maxElementCount, arena);
	context->layoutElements = Clay_LayoutElementArray_Allocate_Arena(maxElementCount, arena);
	context->warnings = Clay__WarningArray_Allocate_Arena(100, arena);

	context->layoutConfigs = Clay__LayoutConfigArray_Allocate_Arena(maxElementCount, arena);
	context->elementConfigs = Clay__ElementConfigArray_Allocate_Arena(maxElementCount, arena);
	context->textElementConfigs = Clay__TextElementConfigArray_Allocate_Arena(maxElementCount, arena);
	context->imageElementConfigs = Clay__ImageElementConfigArray_Allocate_Arena(maxElementCount, arena);
	context->floatingElementConfigs = Clay__FloatingElementConfigArray_Allocate_Arena(maxElementCount, arena);
	context->scrollElementConfigs = Clay__ScrollElementConfigArray_Allocate_Arena(maxElementCount, arena);
	context->customElementConfigs = Clay__CustomElementConfigArray_Allocate_Arena(maxElementCount, arena);
	context->borderElementConfigs = Clay__BorderElementConfigArray_Allocate_Arena(maxElementCount, arena);
	context->sharedElementConfigs = Clay__SharedElementConfigArray_Allocate_Arena(maxElementCount, arena);

	context->layoutElementIdStrings = Clay__Str8Array_Allocate_Arena(maxElementCount, arena);
	context->wrappedTextLines = Clay__WrappedTextLineArray_Allocate_Arena(maxElementCount, arena);
	context->layoutElementTreeNodeArray1 = Clay__LayoutElementTreeNodeArray_Allocate_Arena(maxElementCount, arena);
	context->layoutElementTreeRoots = Clay__LayoutElementTreeRootArray_Allocate_Arena(maxElementCount, arena);
	context->layoutElementChildren = Clay__int32_tArray_Allocate_Arena(maxElementCount, arena);
	context->openLayoutElementStack = Clay__int32_tArray_Allocate_Arena(maxElementCount, arena);
	context->textElementData = Clay__TextElementDataArray_Allocate_Arena(maxElementCount, arena);
	context->imageElementPointers = Clay__int32_tArray_Allocate_Arena(maxElementCount, arena);
	context->renderCommands = Clay_RenderCommandArray_Allocate_Arena(maxElementCount, arena);
	context->treeNodeVisited = Clay__boolArray_Allocate_Arena(maxElementCount, arena);
	context->treeNodeVisited.length = context->treeNodeVisited.allocLength; // This array is accessed directly rather than behaving as a list
	context->openClipElementStack = Clay__int32_tArray_Allocate_Arena(maxElementCount, arena);
	context->reusableElementIndexBuffer = Clay__int32_tArray_Allocate_Arena(maxElementCount, arena);
	context->layoutElementClipElementIds = Clay__int32_tArray_Allocate_Arena(maxElementCount, arena);
	context->dynamicStringData = Clay__charArray_Allocate_Arena(maxElementCount, arena);
}

void Clay__InitializePersistentMemory(Clay_Context* context)
{
	// Persistent memory - initialized once and not reset
	int32_t maxElementCount = context->maxElementCount;
	int32_t maxMeasureTextCacheWordCount = context->maxMeasureTextCacheWordCount;
	Clay_Arena *arena = &context->internalArena;

	context->scrollContainerDatas = Clay__ScrollContainerDataInternalArray_Allocate_Arena(10, arena);
	context->layoutElementsHashMapInternal = Clay__LayoutElementHashMapItemArray_Allocate_Arena(maxElementCount, arena);
	context->layoutElementsHashMap = Clay__int32_tArray_Allocate_Arena(maxElementCount, arena);
	context->measureTextHashMapInternal = Clay__MeasureTextCacheItemArray_Allocate_Arena(maxElementCount, arena);
	context->measureTextHashMapInternalFreeList = Clay__int32_tArray_Allocate_Arena(maxElementCount, arena);
	context->measuredWordsFreeList = Clay__int32_tArray_Allocate_Arena(maxMeasureTextCacheWordCount, arena);
	context->measureTextHashMap = Clay__int32_tArray_Allocate_Arena(maxElementCount, arena);
	context->measuredWords = Clay__MeasuredWordArray_Allocate_Arena(maxMeasureTextCacheWordCount, arena);
	context->pointerOverIds = Clay__ElementIdArray_Allocate_Arena(maxElementCount, arena);
	context->debugElementData = Clay__DebugElementDataArray_Allocate_Arena(maxElementCount, arena);
	context->arenaResetOffset = arena->nextAllocation;
}

void Clay__CompressChildrenAlongAxis(bool xAxis, float totalSizeToDistribute, Clay__int32_tArray resizableContainerBuffer)
{
	Clay_Context* context = Clay_GetCurrentContext();
	Clay__int32_tArray largestContainers = context->openClipElementStack;

	while (totalSizeToDistribute > 0.1)
	{
		largestContainers.length = 0;
		float largestSize = 0;
		float targetSize = 0;
		for (int32_t i = 0; i < resizableContainerBuffer.length; ++i)
		{
			Clay_LayoutElement *childElement = Clay_LayoutElementArray_Get(&context->layoutElements, Clay__int32_tArray_GetValue(&resizableContainerBuffer, i));
			float childSize = xAxis ? childElement->dimensions.Width : childElement->dimensions.Height;
			if ((childSize - largestSize) < 0.1 && (childSize - largestSize) > -0.1)
			{
				Clay__int32_tArray_Add(&largestContainers, Clay__int32_tArray_GetValue(&resizableContainerBuffer, i));
			}
			else if (childSize > largestSize)
			{
				targetSize = largestSize;
				largestSize = childSize;
				largestContainers.length = 0;
				Clay__int32_tArray_Add(&largestContainers, Clay__int32_tArray_GetValue(&resizableContainerBuffer, i));
			}
			else if (childSize > targetSize)
			{
				targetSize = childSize;
			}
		}

		if (largestContainers.length == 0) { return; }

		targetSize = MaxR32(targetSize, (largestSize * largestContainers.length) - totalSizeToDistribute) / largestContainers.length;

		for (int32_t childOffset = 0; childOffset < largestContainers.length; childOffset++)
		{
			int32_t childIndex = Clay__int32_tArray_GetValue(&largestContainers, childOffset);
			Clay_LayoutElement *childElement = Clay_LayoutElementArray_Get(&context->layoutElements, childIndex);
			float *childSize = xAxis ? &childElement->dimensions.Width : &childElement->dimensions.Height;
			float childMinSize = xAxis ? childElement->minDimensions.Width : childElement->minDimensions.Height;
			float oldChildSize = *childSize;
			*childSize = MaxR32(childMinSize, targetSize);
			totalSizeToDistribute -= (oldChildSize - *childSize);
			if (*childSize == childMinSize)
			{
				for (int32_t i = 0; i < resizableContainerBuffer.length; i++)
				{
					if (Clay__int32_tArray_GetValue(&resizableContainerBuffer, i) == childIndex)
					{
						Clay__int32_tArray_RemoveSwapback(&resizableContainerBuffer, i);
						break;
					}
				}
			}
		}
	}
}

void Clay__SizeContainersAlongAxis(bool xAxis)
{
	Clay_Context* context = Clay_GetCurrentContext();
	Clay__int32_tArray bfsBuffer = context->layoutElementChildrenBuffer;
	Clay__int32_tArray resizableContainerBuffer = context->openLayoutElementStack;
	for (int32_t rootIndex = 0; rootIndex < context->layoutElementTreeRoots.length; ++rootIndex)
	{
		bfsBuffer.length = 0;
		Clay__LayoutElementTreeRoot *root = Clay__LayoutElementTreeRootArray_Get(&context->layoutElementTreeRoots, rootIndex);
		Clay_LayoutElement *rootElement = Clay_LayoutElementArray_Get(&context->layoutElements, (int)root->layoutElementIndex);
		Clay__int32_tArray_Add(&bfsBuffer, (int32_t)root->layoutElementIndex);

		// Size floating containers to their parents
		if (Clay__ElementHasConfig(rootElement, CLAY__ELEMENT_CONFIG_TYPE_FLOATING))
		{
			Clay_FloatingElementConfig *floatingElementConfig = Clay__FindElementConfigWithType(rootElement, CLAY__ELEMENT_CONFIG_TYPE_FLOATING).floatingElementConfig;
			Clay_LayoutElementHashMapItem *parentItem = Clay__GetHashMapItem(floatingElementConfig->parentId);
			if (parentItem && parentItem != &Clay_LayoutElementHashMapItem_DEFAULT)
			{
				Clay_LayoutElement *parentLayoutElement = parentItem->layoutElement;
				if (rootElement->layoutConfig->sizing.width.type == CLAY__SIZING_TYPE_GROW)
				{
					rootElement->dimensions.Width = parentLayoutElement->dimensions.Width;
				}
				if (rootElement->layoutConfig->sizing.height.type == CLAY__SIZING_TYPE_GROW)
				{
					rootElement->dimensions.Height = parentLayoutElement->dimensions.Height;
				}
			}
		}

		rootElement->dimensions.Width = MinR32(MaxR32(rootElement->dimensions.Width, rootElement->layoutConfig->sizing.width.size.minMax.min), rootElement->layoutConfig->sizing.width.size.minMax.max);
		rootElement->dimensions.Height = MinR32(MaxR32(rootElement->dimensions.Height, rootElement->layoutConfig->sizing.height.size.minMax.min), rootElement->layoutConfig->sizing.height.size.minMax.max);

		for (int32_t i = 0; i < bfsBuffer.length; ++i)
		{
			int32_t parentIndex = Clay__int32_tArray_GetValue(&bfsBuffer, i);
			Clay_LayoutElement *parent = Clay_LayoutElementArray_Get(&context->layoutElements, parentIndex);
			Clay_LayoutConfig *parentStyleConfig = parent->layoutConfig;
			int32_t growContainerCount = 0;
			float parentSize = xAxis ? parent->dimensions.Width : parent->dimensions.Height;
			float parentPadding = (float)(xAxis ? (parent->layoutConfig->padding.left + parent->layoutConfig->padding.right) : (parent->layoutConfig->padding.top + parent->layoutConfig->padding.bottom));
			float innerContentSize = 0, growContainerContentSize = 0, totalPaddingAndChildGaps = parentPadding;
			bool sizingAlongAxis = (xAxis && parentStyleConfig->layoutDirection == CLAY_LEFT_TO_RIGHT) || (!xAxis && parentStyleConfig->layoutDirection == CLAY_TOP_TO_BOTTOM);
			resizableContainerBuffer.length = 0;
			float parentChildGap = parentStyleConfig->childGap;

			for (int32_t childOffset = 0; childOffset < parent->childrenOrTextContent.children.length; childOffset++)
			{
				int32_t childElementIndex = parent->childrenOrTextContent.children.elements[childOffset];
				Clay_LayoutElement *childElement = Clay_LayoutElementArray_Get(&context->layoutElements, childElementIndex);
				Clay_SizingAxis childSizing = xAxis ? childElement->layoutConfig->sizing.width : childElement->layoutConfig->sizing.height;
				float childSize = xAxis ? childElement->dimensions.Width : childElement->dimensions.Height;

				if (!Clay__ElementHasConfig(childElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT) && childElement->childrenOrTextContent.children.length > 0)
				{
					Clay__int32_tArray_Add(&bfsBuffer, childElementIndex);
				}

				if (childSizing.type != CLAY__SIZING_TYPE_PERCENT
					&& childSizing.type != CLAY__SIZING_TYPE_FIXED
					&& (!Clay__ElementHasConfig(childElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT) ||
						(Clay__FindElementConfigWithType(childElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT).textElementConfig->wrapMode == CLAY_TEXT_WRAP_WORDS) ||
						(Clay__FindElementConfigWithType(childElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT).textElementConfig->textAlignment == CLAY_TEXT_ALIGN_SHRINK)) // todo too many loops
					&& (xAxis || !Clay__ElementHasConfig(childElement, CLAY__ELEMENT_CONFIG_TYPE_IMAGE)))
				{
					Clay__int32_tArray_Add(&resizableContainerBuffer, childElementIndex);
				}

				if (sizingAlongAxis)
				{
					innerContentSize += (childSizing.type == CLAY__SIZING_TYPE_PERCENT ? 0 : childSize);
					if (childSizing.type == CLAY__SIZING_TYPE_GROW)
					{
						growContainerContentSize += childSize;
						growContainerCount++;
					}
					if (childOffset > 0)
					{
						innerContentSize += parentChildGap; // For children after index 0, the childAxisOffset is the gap from the previous child
						totalPaddingAndChildGaps += parentChildGap;
					}
				}
				else
				{
					innerContentSize = MaxR32(childSize, innerContentSize);
				}
			}

			// Expand percentage containers to size
			for (int32_t childOffset = 0; childOffset < parent->childrenOrTextContent.children.length; childOffset++)
			{
				int32_t childElementIndex = parent->childrenOrTextContent.children.elements[childOffset];
				Clay_LayoutElement *childElement = Clay_LayoutElementArray_Get(&context->layoutElements, childElementIndex);
				Clay_SizingAxis childSizing = xAxis ? childElement->layoutConfig->sizing.width : childElement->layoutConfig->sizing.height;
				float *childSize = xAxis ? &childElement->dimensions.Width : &childElement->dimensions.Height;
				if (childSizing.type == CLAY__SIZING_TYPE_PERCENT)
				{
					*childSize = (parentSize - totalPaddingAndChildGaps) * childSizing.size.percent;
					if (sizingAlongAxis) { innerContentSize += *childSize; }
					Clay__UpdateAspectRatioBox(childElement);
				}
			}

			if (sizingAlongAxis)
			{
				float sizeToDistribute = parentSize - parentPadding - innerContentSize;
				// The content is too large, compress the children as much as possible
				if (sizeToDistribute < 0)
				{
					// If the parent can scroll in the axis direction in this direction, don't compress children, just leave them alone
					Clay_ScrollElementConfig *scrollElementConfig = Clay__FindElementConfigWithType(parent, CLAY__ELEMENT_CONFIG_TYPE_SCROLL).scrollElementConfig;
					if (scrollElementConfig)
					{
						if (((xAxis && scrollElementConfig->horizontal) || (!xAxis && scrollElementConfig->vertical))) { continue; }
					}
					// Scrolling containers preferentially compress before others
					Clay__CompressChildrenAlongAxis(xAxis, -sizeToDistribute, resizableContainerBuffer);
				// The content is too small, allow SIZING_GROW containers to expand
				}
				else if (sizeToDistribute > 0 && growContainerCount > 0)
				{
					float targetSize = (sizeToDistribute + growContainerContentSize) / (float)growContainerCount;
					for (int32_t childOffset = 0; childOffset < resizableContainerBuffer.length; childOffset++)
					{
						Clay_LayoutElement *childElement = Clay_LayoutElementArray_Get(&context->layoutElements, Clay__int32_tArray_GetValue(&resizableContainerBuffer, childOffset));
						Clay_SizingAxis childSizing = xAxis ? childElement->layoutConfig->sizing.width : childElement->layoutConfig->sizing.height;
						if (childSizing.type == CLAY__SIZING_TYPE_GROW)
						{
							float *childSize = xAxis ? &childElement->dimensions.Width : &childElement->dimensions.Height;
							float *minSize = xAxis ? &childElement->minDimensions.Width : &childElement->minDimensions.Height;
							if (targetSize < *minSize)
							{
								growContainerContentSize -= *minSize;
								Clay__int32_tArray_RemoveSwapback(&resizableContainerBuffer, childOffset);
								growContainerCount--;
								targetSize = (sizeToDistribute + growContainerContentSize) / (float)growContainerCount;
								childOffset = -1;
								continue;
							}
							*childSize = targetSize;
						}
					}
				}
			// Sizing along the non layout axis ("off axis")
			}
			else
			{
				for (int32_t childOffset = 0; childOffset < resizableContainerBuffer.length; childOffset++)
				{
					Clay_LayoutElement *childElement = Clay_LayoutElementArray_Get(&context->layoutElements, Clay__int32_tArray_GetValue(&resizableContainerBuffer, childOffset));
					Clay_SizingAxis childSizing = xAxis ? childElement->layoutConfig->sizing.width : childElement->layoutConfig->sizing.height;
					float *childSize = xAxis ? &childElement->dimensions.Width : &childElement->dimensions.Height;

					if (!xAxis && Clay__ElementHasConfig(childElement, CLAY__ELEMENT_CONFIG_TYPE_IMAGE)) { continue; } // Currently we don't support resizing aspect ratio images on the Y axis because it would break the ratio

					// If we're laying out the children of a scroll panel, grow containers expand to the height of the inner content, not the outer container
					float maxSize = parentSize - parentPadding;
					if (Clay__ElementHasConfig(parent, CLAY__ELEMENT_CONFIG_TYPE_SCROLL))
					{
						Clay_ScrollElementConfig *scrollElementConfig = Clay__FindElementConfigWithType(parent, CLAY__ELEMENT_CONFIG_TYPE_SCROLL).scrollElementConfig;
						if (((xAxis && scrollElementConfig->horizontal) || (!xAxis && scrollElementConfig->vertical)))
						{
							maxSize = MaxR32(maxSize, innerContentSize);
						}
					}
					if (childSizing.type == CLAY__SIZING_TYPE_FIT)
					{
						*childSize = MaxR32(childSizing.size.minMax.min, MinR32(*childSize, maxSize));
					}
					else if (childSizing.type == CLAY__SIZING_TYPE_GROW)
					{
						*childSize = MinR32(maxSize, childSizing.size.minMax.max);
					}
				}
			}
		}
	}
}

Str8 Clay__IntToString(int32_t integer)
{
	if (integer == 0) { return StrLit("0"); }
	Clay_Context* context = Clay_GetCurrentContext();
	char *chars = (char *)(context->dynamicStringData.items + context->dynamicStringData.length);
	int32_t length = 0;
	int32_t sign = integer;

	if (integer < 0) { integer = -integer; }
	while (integer > 0)
	{
		chars[length++] = (char)(integer % 10 + '0');
		integer /= 10;
	}

	if (sign < 0) { chars[length++] = '-'; }

	// Reverse the string to get the correct order
	for (int32_t j = 0, k = length - 1; j < k; j++, k--)
	{
		char temp = chars[j];
		chars[j] = chars[k];
		chars[k] = temp;
	}
	context->dynamicStringData.length += length;
	return NewStr8(length, chars);
}

void Clay__AddRenderCommand(Clay_RenderCommand renderCommand)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->renderCommands.length < context->renderCommands.allocLength - 1)
	{
		Clay_RenderCommandArray_Add(&context->renderCommands, renderCommand);
	}
	else
	{
		if (!context->booleanWarnings.maxRenderCommandsExceeded)
		{
			context->booleanWarnings.maxRenderCommandsExceeded = true;
			context->errorHandler.errorHandlerFunction(NEW_STRUCT(Clay_ErrorData) {
				.errorType = CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED,
				.errorText = CLAY_STRING("Clay ran out of capacity while attempting to create render commands. This is usually caused by a large amount of wrapping text elements while close to the max element capacity. Try using Clay_SetMaxElementCount() with a higher value."),
				.userData = context->errorHandler.userData
			});
		}
	}
}

bool Clay__ElementIsOffscreen(rec *boundingBox)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->disableCulling) { return false; }

	return (
		(boundingBox->X > (float)context->layoutDimensions.Width) ||
		(boundingBox->Y > (float)context->layoutDimensions.Height) ||
		(boundingBox->X + boundingBox->Width < 0) ||
		(boundingBox->Y + boundingBox->Height < 0));
}

void Clay__CalculateFinalLayout(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	// Calculate sizing along the X axis
	Clay__SizeContainersAlongAxis(true);

	// Wrap text
	for (int32_t textElementIndex = 0; textElementIndex < context->textElementData.length; ++textElementIndex)
	{
		Clay__TextElementData *textElementData = Clay__TextElementDataArray_Get(&context->textElementData, textElementIndex);
		textElementData->wrappedLines = NEW_STRUCT(Clay__WrappedTextLineArraySlice) { .length = 0, .items = &context->wrappedTextLines.items[context->wrappedTextLines.length] };
		Clay_LayoutElement *containerElement = Clay_LayoutElementArray_Get(&context->layoutElements, (int)textElementData->elementIndex);
		Clay_TextElementConfig *textConfig = Clay__FindElementConfigWithType(containerElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT).textElementConfig;
		Clay__MeasureTextCacheItem *measureTextCacheItem = Clay__MeasureTextCached(&textElementData->text, textConfig);
		bool considerNewLines = (textConfig->wrapMode == CLAY_TEXT_WRAP_NEWLINES || textConfig->wrapMode == CLAY_TEXT_WRAP_WORDS);
		bool considerMaxWidth = (textConfig->wrapMode == CLAY_TEXT_WRAP_WORDS && textConfig->textAlignment != CLAY_TEXT_ALIGN_SHRINK);
		float lineWidth = 0;
		float lineHeight = textConfig->lineHeight > 0 ? (float)textConfig->lineHeight : textElementData->preferredDimensions.Height;
		int32_t lineLengthChars = 0;
		int32_t lineStartOffset = 0;
		if (!measureTextCacheItem->containsNewlines && textElementData->preferredDimensions.Width <= containerElement->dimensions.Width)
		{
			Clay__WrappedTextLineArray_Add(&context->wrappedTextLines, NEW_STRUCT(Clay__WrappedTextLine) { containerElement->dimensions,  textElementData->text });
			textElementData->wrappedLines.length++;
			continue;
		}
		float spaceWidth = Clay__MeasureText(Str8_Space, textConfig, context->measureTextUserData).Width;
		int32_t wordIndex = measureTextCacheItem->measuredWordsStartIndex;
		while (wordIndex != -1)
		{
			if (context->wrappedTextLines.length > context->wrappedTextLines.allocLength - 1) { break; }
			Clay__MeasuredWord *measuredWord = Clay__MeasuredWordArray_Get(&context->measuredWords, wordIndex);
			// Only word on the line is too large, just render it anyway
			if (lineLengthChars == 0 && lineWidth + measuredWord->width > containerElement->dimensions.Width && considerMaxWidth)
			{
				Clay__WrappedTextLineArray_Add(&context->wrappedTextLines, NEW_STRUCT(Clay__WrappedTextLine) { { measuredWord->width, lineHeight }, { .length = measuredWord->length, .chars = &textElementData->text.chars[measuredWord->startOffset] } });
				textElementData->wrappedLines.length++;
				wordIndex = measuredWord->next;
				lineStartOffset = measuredWord->startOffset + measuredWord->length;
			}
			// measuredWord->length == 0 means a newline character
			else if ((measuredWord->length == 0 && considerNewLines) || (lineWidth + measuredWord->width > containerElement->dimensions.Width && considerMaxWidth))
			{
				// Wrapped text lines list has overflowed, just render out the line
				bool finalCharIsSpace = textElementData->text.chars[lineStartOffset + lineLengthChars - 1] == ' ';
				Clay__WrappedTextLineArray_Add(&context->wrappedTextLines, NEW_STRUCT(Clay__WrappedTextLine) { { lineWidth + (finalCharIsSpace ? -spaceWidth : 0), lineHeight }, { .length = lineLengthChars + (finalCharIsSpace ? -1 : 0), .chars = &textElementData->text.chars[lineStartOffset] } });
				textElementData->wrappedLines.length++;
				if (lineLengthChars == 0 || measuredWord->length == 0) { wordIndex = measuredWord->next; }
				lineWidth = 0;
				lineLengthChars = 0;
				lineStartOffset = measuredWord->startOffset;
			}
			else
			{
				lineWidth += measuredWord->width;
				lineLengthChars += measuredWord->length;
				wordIndex = measuredWord->next;
			}
		}
		if (lineLengthChars > 0)
		{
			Clay__WrappedTextLineArray_Add(&context->wrappedTextLines, NEW_STRUCT(Clay__WrappedTextLine) { { lineWidth, lineHeight }, {.length = lineLengthChars, .chars = &textElementData->text.chars[lineStartOffset] } });
			textElementData->wrappedLines.length++;
		}
		containerElement->dimensions.Height = lineHeight * (float)textElementData->wrappedLines.length;
	}

	// Scale vertical image heights according to aspect ratio
	for (int32_t i = 0; i < context->imageElementPointers.length; ++i)
	{
		Clay_LayoutElement* imageElement = Clay_LayoutElementArray_Get(&context->layoutElements, Clay__int32_tArray_GetValue(&context->imageElementPointers, i));
		Clay_ImageElementConfig *config = Clay__FindElementConfigWithType(imageElement, CLAY__ELEMENT_CONFIG_TYPE_IMAGE).imageElementConfig;
		imageElement->dimensions.Height = (config->sourceDimensions.Height / MaxR32(config->sourceDimensions.Width, 1)) * imageElement->dimensions.Width;
	}

	// Propagate effect of text wrapping, image aspect scaling etc. on height of parents
	Clay__LayoutElementTreeNodeArray dfsBuffer = context->layoutElementTreeNodeArray1;
	dfsBuffer.length = 0;
	for (int32_t i = 0; i < context->layoutElementTreeRoots.length; ++i)
	{
		Clay__LayoutElementTreeRoot *root = Clay__LayoutElementTreeRootArray_Get(&context->layoutElementTreeRoots, i);
		context->treeNodeVisited.items[dfsBuffer.length] = false;
		Clay__LayoutElementTreeNodeArray_Add(&dfsBuffer, NEW_STRUCT(Clay__LayoutElementTreeNode) { .layoutElement = Clay_LayoutElementArray_Get(&context->layoutElements, (int)root->layoutElementIndex) });
	}
	while (dfsBuffer.length > 0)
	{
		Clay__LayoutElementTreeNode *currentElementTreeNode = Clay__LayoutElementTreeNodeArray_Get(&dfsBuffer, (int)dfsBuffer.length - 1);
		Clay_LayoutElement *currentElement = currentElementTreeNode->layoutElement;
		if (!context->treeNodeVisited.items[dfsBuffer.length - 1])
		{
			context->treeNodeVisited.items[dfsBuffer.length - 1] = true;
			// If the element has no children or is the container for a text element, don't bother inspecting it
			if (Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT) || currentElement->childrenOrTextContent.children.length == 0)
			{
				dfsBuffer.length--;
				continue;
			}
			// Add the children to the DFS buffer (needs to be pushed in reverse so that stack traversal is in correct layout order)
			for (int32_t i = 0; i < currentElement->childrenOrTextContent.children.length; i++)
			{
				context->treeNodeVisited.items[dfsBuffer.length] = false;
				Clay__LayoutElementTreeNodeArray_Add(&dfsBuffer, NEW_STRUCT(Clay__LayoutElementTreeNode) { .layoutElement = Clay_LayoutElementArray_Get(&context->layoutElements, currentElement->childrenOrTextContent.children.elements[i]) });
			}
			continue;
		}
		dfsBuffer.length--;

		// DFS node has been visited, this is on the way back up to the root
		Clay_LayoutConfig *layoutConfig = currentElement->layoutConfig;
		if (layoutConfig->layoutDirection == CLAY_LEFT_TO_RIGHT)
		{
			// Resize any parent containers that have grown in height along their non layout axis
			for (int32_t j = 0; j < currentElement->childrenOrTextContent.children.length; ++j)
			{
				Clay_LayoutElement *childElement = Clay_LayoutElementArray_Get(&context->layoutElements, currentElement->childrenOrTextContent.children.elements[j]);
				float childHeightWithPadding = MaxR32(childElement->dimensions.Height + layoutConfig->padding.top + layoutConfig->padding.bottom, currentElement->dimensions.Height);
				currentElement->dimensions.Height = MinR32(MaxR32(childHeightWithPadding, layoutConfig->sizing.height.size.minMax.min), layoutConfig->sizing.height.size.minMax.max);
			}
		}
		else if (layoutConfig->layoutDirection == CLAY_TOP_TO_BOTTOM)
		{
			// Resizing along the layout axis
			float contentHeight = (float)(layoutConfig->padding.top + layoutConfig->padding.bottom);
			for (int32_t j = 0; j < currentElement->childrenOrTextContent.children.length; ++j)
			{
				Clay_LayoutElement *childElement = Clay_LayoutElementArray_Get(&context->layoutElements, currentElement->childrenOrTextContent.children.elements[j]);
				contentHeight += childElement->dimensions.Height;
			}
			contentHeight += (float)(MaxU16(currentElement->childrenOrTextContent.children.length - 1, 0) * layoutConfig->childGap);
			currentElement->dimensions.Height = MinR32(MaxR32(contentHeight, layoutConfig->sizing.height.size.minMax.min), layoutConfig->sizing.height.size.minMax.max);
		}
	}

	// Calculate sizing along the Y axis
	Clay__SizeContainersAlongAxis(false);

	// Sort tree roots by z-index
	int32_t sortMax = context->layoutElementTreeRoots.length - 1;
	while (sortMax > 0)
	{ // todo dumb bubble sort
		for (int32_t i = 0; i < sortMax; ++i)
		{
			Clay__LayoutElementTreeRoot current = *Clay__LayoutElementTreeRootArray_Get(&context->layoutElementTreeRoots, i);
			Clay__LayoutElementTreeRoot next = *Clay__LayoutElementTreeRootArray_Get(&context->layoutElementTreeRoots, i + 1);
			if (next.zIndex < current.zIndex)
			{
				Clay__LayoutElementTreeRootArray_Set(&context->layoutElementTreeRoots, i, next);
				Clay__LayoutElementTreeRootArray_Set(&context->layoutElementTreeRoots, i + 1, current);
			}
		}
		sortMax--;
	}

	// Calculate final positions and generate render commands
	context->renderCommands.length = 0;
	dfsBuffer.length = 0;
	for (int32_t rootIndex = 0; rootIndex < context->layoutElementTreeRoots.length; ++rootIndex)
	{
		dfsBuffer.length = 0;
		Clay__LayoutElementTreeRoot *root = Clay__LayoutElementTreeRootArray_Get(&context->layoutElementTreeRoots, rootIndex);
		Clay_LayoutElement *rootElement = Clay_LayoutElementArray_Get(&context->layoutElements, (int)root->layoutElementIndex);
		v2 rootPosition = ZEROED;
		Clay_LayoutElementHashMapItem *parentHashMapItem = Clay__GetHashMapItem(root->parentId);
		// Position root floating containers
		if (Clay__ElementHasConfig(rootElement, CLAY__ELEMENT_CONFIG_TYPE_FLOATING) && parentHashMapItem)
		{
			Clay_FloatingElementConfig *config = Clay__FindElementConfigWithType(rootElement, CLAY__ELEMENT_CONFIG_TYPE_FLOATING).floatingElementConfig;
			v2 rootDimensions = rootElement->dimensions;
			rec parentBoundingBox = parentHashMapItem->boundingBox;
			// Set X position
			v2 targetAttachPosition = ZEROED;
			switch (config->attachPoints.parent)
			{
				case CLAY_ATTACH_POINT_LEFT_TOP:
				case CLAY_ATTACH_POINT_LEFT_CENTER:
				case CLAY_ATTACH_POINT_LEFT_BOTTOM: targetAttachPosition.X = parentBoundingBox.X; break;
				case CLAY_ATTACH_POINT_CENTER_TOP:
				case CLAY_ATTACH_POINT_CENTER_CENTER:
				case CLAY_ATTACH_POINT_CENTER_BOTTOM: targetAttachPosition.X = parentBoundingBox.X + (parentBoundingBox.Width / 2); break;
				case CLAY_ATTACH_POINT_RIGHT_TOP:
				case CLAY_ATTACH_POINT_RIGHT_CENTER:
				case CLAY_ATTACH_POINT_RIGHT_BOTTOM: targetAttachPosition.X = parentBoundingBox.X + parentBoundingBox.Width; break;
			}
			switch (config->attachPoints.element)
			{
				case CLAY_ATTACH_POINT_LEFT_TOP:
				case CLAY_ATTACH_POINT_LEFT_CENTER:
				case CLAY_ATTACH_POINT_LEFT_BOTTOM: break;
				case CLAY_ATTACH_POINT_CENTER_TOP:
				case CLAY_ATTACH_POINT_CENTER_CENTER:
				case CLAY_ATTACH_POINT_CENTER_BOTTOM: targetAttachPosition.X -= (rootDimensions.Width / 2); break;
				case CLAY_ATTACH_POINT_RIGHT_TOP:
				case CLAY_ATTACH_POINT_RIGHT_CENTER:
				case CLAY_ATTACH_POINT_RIGHT_BOTTOM: targetAttachPosition.X -= rootDimensions.Width; break;
			}
			switch (config->attachPoints.parent) // I know I could merge the x and y switch statements, but this is easier to read
			{
				case CLAY_ATTACH_POINT_LEFT_TOP:
				case CLAY_ATTACH_POINT_RIGHT_TOP:
				case CLAY_ATTACH_POINT_CENTER_TOP: targetAttachPosition.Y = parentBoundingBox.Y; break;
				case CLAY_ATTACH_POINT_LEFT_CENTER:
				case CLAY_ATTACH_POINT_CENTER_CENTER:
				case CLAY_ATTACH_POINT_RIGHT_CENTER: targetAttachPosition.Y = parentBoundingBox.Y + (parentBoundingBox.Height / 2); break;
				case CLAY_ATTACH_POINT_LEFT_BOTTOM:
				case CLAY_ATTACH_POINT_CENTER_BOTTOM:
				case CLAY_ATTACH_POINT_RIGHT_BOTTOM: targetAttachPosition.Y = parentBoundingBox.Y + parentBoundingBox.Height; break;
			}
			switch (config->attachPoints.element)
			{
				case CLAY_ATTACH_POINT_LEFT_TOP:
				case CLAY_ATTACH_POINT_RIGHT_TOP:
				case CLAY_ATTACH_POINT_CENTER_TOP: break;
				case CLAY_ATTACH_POINT_LEFT_CENTER:
				case CLAY_ATTACH_POINT_CENTER_CENTER:
				case CLAY_ATTACH_POINT_RIGHT_CENTER: targetAttachPosition.Y -= (rootDimensions.Height / 2); break;
				case CLAY_ATTACH_POINT_LEFT_BOTTOM:
				case CLAY_ATTACH_POINT_CENTER_BOTTOM:
				case CLAY_ATTACH_POINT_RIGHT_BOTTOM: targetAttachPosition.Y -= rootDimensions.Height; break;
			}
			targetAttachPosition.X += config->offset.X;
			targetAttachPosition.Y += config->offset.Y;
			rootPosition = targetAttachPosition;
		}
		if (root->clipElementId)
		{
			Clay_LayoutElementHashMapItem *clipHashMapItem = Clay__GetHashMapItem(root->clipElementId);
			if (clipHashMapItem)
			{
				// Floating elements that are attached to scrolling contents won't be correctly positioned if external scroll handling is enabled, fix here
				if (context->externalScrollHandlingEnabled)
				{
					Clay_ScrollElementConfig *scrollConfig = Clay__FindElementConfigWithType(clipHashMapItem->layoutElement, CLAY__ELEMENT_CONFIG_TYPE_SCROLL).scrollElementConfig;
					for (int32_t i = 0; i < context->scrollContainerDatas.length; i++)
					{
						Clay__ScrollContainerDataInternal *mapping = Clay__ScrollContainerDataInternalArray_Get(&context->scrollContainerDatas, i);
						if (mapping->layoutElement == clipHashMapItem->layoutElement)
						{
							root->pointerOffset = mapping->scrollPosition;
							if (scrollConfig->horizontal) { rootPosition.X += mapping->scrollPosition.X; }
							if (scrollConfig->vertical) { rootPosition.Y += mapping->scrollPosition.X; }
							break;
						}
					}
				}
				Clay__AddRenderCommand(NEW_STRUCT(Clay_RenderCommand) {
					.boundingBox = clipHashMapItem->boundingBox,
					.userData = ZEROED,
					.id = Clay__HashNumber(rootElement->id, rootElement->childrenOrTextContent.children.length + 10).id, // TODO need a better strategy for managing derived ids
					.zIndex = root->zIndex,
					.commandType = CLAY_RENDER_COMMAND_TYPE_SCISSOR_START,
				});
			}
		}
		Clay__LayoutElementTreeNodeArray_Add(&dfsBuffer, NEW_STRUCT(Clay__LayoutElementTreeNode) { .layoutElement = rootElement, .position = rootPosition, .nextChildOffset = { .X = (float)rootElement->layoutConfig->padding.left, .Y = (float)rootElement->layoutConfig->padding.top } });

		context->treeNodeVisited.items[0] = false;
		while (dfsBuffer.length > 0)
		{
			Clay__LayoutElementTreeNode *currentElementTreeNode = Clay__LayoutElementTreeNodeArray_Get(&dfsBuffer, (int)dfsBuffer.length - 1);
			Clay_LayoutElement *currentElement = currentElementTreeNode->layoutElement;
			Clay_LayoutConfig *layoutConfig = currentElement->layoutConfig;
			v2 scrollOffset = ZEROED;

			// This will only be run a single time for each element in downwards DFS order
			if (!context->treeNodeVisited.items[dfsBuffer.length - 1])
			{
				context->treeNodeVisited.items[dfsBuffer.length - 1] = true;

				rec currentElementBoundingBox = { currentElementTreeNode->position.X, currentElementTreeNode->position.Y, currentElement->dimensions.Width, currentElement->dimensions.Height };
				if (Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_FLOATING))
				{
					Clay_FloatingElementConfig *floatingElementConfig = Clay__FindElementConfigWithType(currentElement, CLAY__ELEMENT_CONFIG_TYPE_FLOATING).floatingElementConfig;
					v2 expand = floatingElementConfig->expand;
					currentElementBoundingBox.X -= expand.Width;
					currentElementBoundingBox.Width += expand.Width * 2;
					currentElementBoundingBox.Y -= expand.Height;
					currentElementBoundingBox.Height += expand.Height * 2;
				}

				Clay__ScrollContainerDataInternal *scrollContainerData = CLAY__NULL;
				// Apply scroll offsets to container
				if (Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_SCROLL))
				{
					Clay_ScrollElementConfig *scrollConfig = Clay__FindElementConfigWithType(currentElement, CLAY__ELEMENT_CONFIG_TYPE_SCROLL).scrollElementConfig;

					// This linear scan could theoretically be slow under very strange conditions, but I can't imagine a real UI with more than a few 10's of scroll containers
					for (int32_t i = 0; i < context->scrollContainerDatas.length; i++)
					{
						Clay__ScrollContainerDataInternal *mapping = Clay__ScrollContainerDataInternalArray_Get(&context->scrollContainerDatas, i);
						if (mapping->layoutElement == currentElement)
						{
							scrollContainerData = mapping;
							mapping->boundingBox = currentElementBoundingBox;
							if (scrollConfig->horizontal) { scrollOffset.X = mapping->scrollPosition.X; }
							if (scrollConfig->vertical) { scrollOffset.Y = mapping->scrollPosition.Y; }
							if (context->externalScrollHandlingEnabled) { scrollOffset = V2_Zero; }
							break;
						}
					}
				}

				Clay_LayoutElementHashMapItem *hashMapItem = Clay__GetHashMapItem(currentElement->id);
				if (hashMapItem)
				{
					hashMapItem->boundingBox = currentElementBoundingBox;
					if (hashMapItem->idAlias)
					{
						Clay_LayoutElementHashMapItem *hashMapItemAlias = Clay__GetHashMapItem(hashMapItem->idAlias);
						if (hashMapItemAlias)
						{
							hashMapItemAlias->boundingBox = currentElementBoundingBox;
						}
					}
				}

				int32_t sortedConfigIndexes[20];
				for (int32_t elementConfigIndex = 0; elementConfigIndex < currentElement->elementConfigs.length; ++elementConfigIndex)
				{
					sortedConfigIndexes[elementConfigIndex] = elementConfigIndex;
				}
				sortMax = currentElement->elementConfigs.length - 1;
				while (sortMax > 0) // todo dumb bubble sort
				{
					for (int32_t i = 0; i < sortMax; ++i)
					{
						int32_t current = sortedConfigIndexes[i];
						int32_t next = sortedConfigIndexes[i + 1];
						Clay__ElementConfigType currentType = Clay__ElementConfigArraySlice_Get(&currentElement->elementConfigs, current)->type;
						Clay__ElementConfigType nextType = Clay__ElementConfigArraySlice_Get(&currentElement->elementConfigs, next)->type;
						if (nextType == CLAY__ELEMENT_CONFIG_TYPE_SCROLL || currentType == CLAY__ELEMENT_CONFIG_TYPE_BORDER)
						{
							sortedConfigIndexes[i] = next;
							sortedConfigIndexes[i + 1] = current;
						}
					}
					sortMax--;
				}

				bool emitRectangle = false;
				// Create the render commands for this element
				Clay_SharedElementConfig *sharedConfig = Clay__FindElementConfigWithType(currentElement, CLAY__ELEMENT_CONFIG_TYPE_SHARED).sharedElementConfig;
				if (sharedConfig && sharedConfig->backgroundColor.a > 0) { emitRectangle = true; }
				else if (!sharedConfig)
				{
					emitRectangle = false;
					sharedConfig = &Clay_SharedElementConfig_DEFAULT;
				}
				for (int32_t elementConfigIndex = 0; elementConfigIndex < currentElement->elementConfigs.length; ++elementConfigIndex)
				{
					Clay_ElementConfig *elementConfig = Clay__ElementConfigArraySlice_Get(&currentElement->elementConfigs, sortedConfigIndexes[elementConfigIndex]);
					Clay_RenderCommand renderCommand = {
						.boundingBox = currentElementBoundingBox,
						.userData = sharedConfig->userData,
						.id = currentElement->id,
					};

					bool offscreen = Clay__ElementIsOffscreen(&currentElementBoundingBox);
					// Culling - Don't bother to generate render commands for rectangles entirely outside the screen - this won't stop their children from being rendered if they overflow
					bool shouldRender = !offscreen;
					switch (elementConfig->type)
					{
						case CLAY__ELEMENT_CONFIG_TYPE_FLOATING:
						case CLAY__ELEMENT_CONFIG_TYPE_SHARED:
						case CLAY__ELEMENT_CONFIG_TYPE_BORDER:
						{
							shouldRender = false;
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_SCROLL:
						{
							renderCommand.commandType = CLAY_RENDER_COMMAND_TYPE_SCISSOR_START;
							renderCommand.renderData = NEW_STRUCT(Clay_RenderData) {
								.scroll = {
									.horizontal = elementConfig->config.scrollElementConfig->horizontal,
									.vertical = elementConfig->config.scrollElementConfig->vertical,
									.scrollLag = elementConfig->config.scrollElementConfig->scrollLag,
								}
							};
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_IMAGE:
						{
							renderCommand.commandType = CLAY_RENDER_COMMAND_TYPE_IMAGE;
							renderCommand.renderData = NEW_STRUCT(Clay_RenderData) {
								.image = {
									.backgroundColor = sharedConfig->backgroundColor,
									.cornerRadius = sharedConfig->cornerRadius,
									.sourceDimensions = elementConfig->config.imageElementConfig->sourceDimensions,
									.imageData = elementConfig->config.imageElementConfig->imageData,
							   }
							};
							emitRectangle = false;
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_TEXT:
						{
							if (!shouldRender) { break; }
							shouldRender = false;
							Clay_ElementConfigUnion configUnion = elementConfig->config;
							Clay_TextElementConfig *textElementConfig = configUnion.textElementConfig;
							float naturalLineHeight = currentElement->childrenOrTextContent.textElementData->preferredDimensions.Height;
							float finalLineHeight = textElementConfig->lineHeight > 0 ? (float)textElementConfig->lineHeight : naturalLineHeight;
							float lineHeightOffset = (finalLineHeight - naturalLineHeight) / 2;
							float yPosition = lineHeightOffset;
							for (int32_t lineIndex = 0; lineIndex < currentElement->childrenOrTextContent.textElementData->wrappedLines.length; ++lineIndex)
							{
								Clay__WrappedTextLine *wrappedLine = Clay__WrappedTextLineArraySlice_Get(&currentElement->childrenOrTextContent.textElementData->wrappedLines, lineIndex);
								if (wrappedLine->line.length == 0)
								{
									yPosition += finalLineHeight;
									continue;
								}
								float offset = (currentElementBoundingBox.Width - wrappedLine->dimensions.Width);
								if (textElementConfig->textAlignment == CLAY_TEXT_ALIGN_LEFT || textElementConfig->textAlignment == CLAY_TEXT_ALIGN_SHRINK)
								{
									offset = 0;
								}
								if (textElementConfig->textAlignment == CLAY_TEXT_ALIGN_CENTER)
								{
									offset /= 2;
								}
								rec boundingBox = {
									currentElementBoundingBox.X + offset,
									currentElementBoundingBox.Y + yPosition,
									wrappedLine->dimensions.Width,
									wrappedLine->dimensions.Height
								};
								if (textElementConfig->textAlignment == CLAY_TEXT_ALIGN_SHRINK && boundingBox.Width > currentElementBoundingBox.Width)
								{
									boundingBox.Width = currentElementBoundingBox.Width;
								}
								Clay__AddRenderCommand(NEW_STRUCT(Clay_RenderCommand) {
									.boundingBox = boundingBox,
									.renderData = { .text = {
										.stringContents = wrappedLine->line,
										.textColor = textElementConfig->textColor,
										.fontId = textElementConfig->fontId,
										.fontSize = textElementConfig->fontSize,
										.letterSpacing = textElementConfig->letterSpacing,
										.lineHeight = textElementConfig->lineHeight,
										.userData = textElementConfig->userData,
									}},
									.userData = sharedConfig->userData,
									.id = Clay__HashNumber(lineIndex, currentElement->id).id,
									.zIndex = root->zIndex,
									.commandType = CLAY_RENDER_COMMAND_TYPE_TEXT,
								});
								yPosition += finalLineHeight;

								if (!context->disableCulling && (currentElementBoundingBox.Y + yPosition > context->layoutDimensions.Height)) { break; }
							}
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_CUSTOM:
						{
							renderCommand.commandType = CLAY_RENDER_COMMAND_TYPE_CUSTOM;
							renderCommand.renderData = NEW_STRUCT(Clay_RenderData) {
								.custom = {
									.backgroundColor = sharedConfig->backgroundColor,
									.cornerRadius = sharedConfig->cornerRadius,
									.customData = elementConfig->config.customElementConfig->customData,
								}
							};
							emitRectangle = false;
							break;
						}
						default: break;
					}
					if (shouldRender)
					{
						Clay__AddRenderCommand(renderCommand);
					}
					if (offscreen)
					{
						// NOTE: You may be tempted to try an early return / continue if an element is off screen. Why bother calculating layout for its children, right?
						// Unfortunately, a FLOATING_CONTAINER may be defined that attaches to a child or grandchild of this element, which is large enough to still
						// be on screen, even if this element isn't. That depends on this element and it's children being laid out correctly (even if they are entirely off screen)
					}
				}

				if (emitRectangle)
				{
					Clay__AddRenderCommand(NEW_STRUCT(Clay_RenderCommand) {
						.boundingBox = currentElementBoundingBox,
						.renderData = { .rectangle = {
								.backgroundColor = sharedConfig->backgroundColor,
								.cornerRadius = sharedConfig->cornerRadius,
						}},
						.userData = sharedConfig->userData,
						.id = currentElement->id,
						.zIndex = root->zIndex,
						.commandType = CLAY_RENDER_COMMAND_TYPE_RECTANGLE,
					});
				}

				// Setup initial on-axis alignment
				if (!Clay__ElementHasConfig(currentElementTreeNode->layoutElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT))
				{
					v2 contentSize = {0,0};
					if (layoutConfig->layoutDirection == CLAY_LEFT_TO_RIGHT)
					{
						for (int32_t i = 0; i < currentElement->childrenOrTextContent.children.length; ++i)
						{
							Clay_LayoutElement *childElement = Clay_LayoutElementArray_Get(&context->layoutElements, currentElement->childrenOrTextContent.children.elements[i]);
							contentSize.Width += childElement->dimensions.Width;
							contentSize.Height = MaxR32(contentSize.Height, childElement->dimensions.Height);
						}
						contentSize.Width += (float)(MaxU16(currentElement->childrenOrTextContent.children.length - 1, 0) * layoutConfig->childGap);
						float extraSpace = currentElement->dimensions.Width - (float)(layoutConfig->padding.left + layoutConfig->padding.right) - contentSize.Width;
						switch (layoutConfig->childAlignment.x)
						{
							case CLAY_ALIGN_X_LEFT: extraSpace = 0; break;
							case CLAY_ALIGN_X_CENTER: extraSpace /= 2; break;
							default: break;
						}
						currentElementTreeNode->nextChildOffset.X += extraSpace;
					}
					else
					{
						for (int32_t i = 0; i < currentElement->childrenOrTextContent.children.length; ++i)
						{
							Clay_LayoutElement *childElement = Clay_LayoutElementArray_Get(&context->layoutElements, currentElement->childrenOrTextContent.children.elements[i]);
							contentSize.Width = MaxR32(contentSize.Width, childElement->dimensions.Width);
							contentSize.Height += childElement->dimensions.Height;
						}
						contentSize.Height += (float)(MaxU16(currentElement->childrenOrTextContent.children.length - 1, 0) * layoutConfig->childGap);
						float extraSpace = currentElement->dimensions.Height - (float)(layoutConfig->padding.top + layoutConfig->padding.bottom) - contentSize.Height;
						switch (layoutConfig->childAlignment.y)
						{
							case CLAY_ALIGN_Y_TOP: extraSpace = 0; break;
							case CLAY_ALIGN_Y_CENTER: extraSpace /= 2; break;
							default: break;
						}
						currentElementTreeNode->nextChildOffset.Y += extraSpace;
					}

					if (scrollContainerData)
					{
						scrollContainerData->contentSize = NewV2(contentSize.Width + (float)(layoutConfig->padding.left + layoutConfig->padding.right), contentSize.Height + (float)(layoutConfig->padding.top + layoutConfig->padding.bottom));
					}
				}
			}
			else
			{
				// DFS is returning upwards backwards
				bool closeScrollElement = false;
				Clay_ScrollElementConfig *scrollConfig = Clay__FindElementConfigWithType(currentElement, CLAY__ELEMENT_CONFIG_TYPE_SCROLL).scrollElementConfig;
				if (scrollConfig) {
					closeScrollElement = true;
					for (int32_t i = 0; i < context->scrollContainerDatas.length; i++)
					{
						Clay__ScrollContainerDataInternal *mapping = Clay__ScrollContainerDataInternalArray_Get(&context->scrollContainerDatas, i);
						if (mapping->layoutElement == currentElement)
						{
							if (scrollConfig->horizontal) { scrollOffset.X = mapping->scrollPosition.X; }
							if (scrollConfig->vertical) { scrollOffset.Y = mapping->scrollPosition.Y; }
							if (context->externalScrollHandlingEnabled) { scrollOffset = V2_Zero; }
							break;
						}
					}
				}

				if (Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_BORDER))
				{
					Clay_LayoutElementHashMapItem *currentElementData = Clay__GetHashMapItem(currentElement->id);
					rec currentElementBoundingBox = currentElementData->boundingBox;

					// Culling - Don't bother to generate render commands for rectangles entirely outside the screen - this won't stop their children from being rendered if they overflow
					if (!Clay__ElementIsOffscreen(&currentElementBoundingBox))
					{
						Clay_SharedElementConfig *sharedConfig = Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_SHARED) ? Clay__FindElementConfigWithType(currentElement, CLAY__ELEMENT_CONFIG_TYPE_SHARED).sharedElementConfig : &Clay_SharedElementConfig_DEFAULT;
						Clay_BorderElementConfig *borderConfig = Clay__FindElementConfigWithType(currentElement, CLAY__ELEMENT_CONFIG_TYPE_BORDER).borderElementConfig;
						Clay_RenderCommand renderCommand = {
								.boundingBox = currentElementBoundingBox,
								.renderData = { .border = {
									.color = borderConfig->color,
									.cornerRadius = sharedConfig->cornerRadius,
									.width = borderConfig->width
								}},
								.userData = sharedConfig->userData,
								.id = Clay__HashNumber(currentElement->id, currentElement->childrenOrTextContent.children.length).id,
								.commandType = CLAY_RENDER_COMMAND_TYPE_BORDER,
						};
						Clay__AddRenderCommand(renderCommand);
						if (borderConfig->width.betweenChildren > 0 && borderConfig->color.a > 0)
						{
							float halfGap = layoutConfig->childGap / 2;
							v2 borderOffset = { (float)layoutConfig->padding.left - halfGap, (float)layoutConfig->padding.top - halfGap };
							if (layoutConfig->layoutDirection == CLAY_LEFT_TO_RIGHT)
							{
								for (int32_t i = 0; i < currentElement->childrenOrTextContent.children.length; ++i)
								{
									Clay_LayoutElement *childElement = Clay_LayoutElementArray_Get(&context->layoutElements, currentElement->childrenOrTextContent.children.elements[i]);
									if (i > 0)
									{
										Clay__AddRenderCommand(NEW_STRUCT(Clay_RenderCommand) {
											.boundingBox = { currentElementBoundingBox.X + borderOffset.X + scrollOffset.X, currentElementBoundingBox.Y + scrollOffset.Y, (float)borderConfig->width.betweenChildren, currentElement->dimensions.Height },
											.renderData = { .rectangle = {
												.backgroundColor = borderConfig->color,
											} },
											.userData = sharedConfig->userData,
											.id = Clay__HashNumber(currentElement->id, currentElement->childrenOrTextContent.children.length + 1 + i).id,
											.commandType = CLAY_RENDER_COMMAND_TYPE_RECTANGLE,
										});
									}
									borderOffset.X += (childElement->dimensions.Width + (float)layoutConfig->childGap);
								}
							}
							else
							{
								for (int32_t i = 0; i < currentElement->childrenOrTextContent.children.length; ++i)
								{
									Clay_LayoutElement *childElement = Clay_LayoutElementArray_Get(&context->layoutElements, currentElement->childrenOrTextContent.children.elements[i]);
									if (i > 0)
									{
										Clay__AddRenderCommand(NEW_STRUCT(Clay_RenderCommand) {
											.boundingBox = { currentElementBoundingBox.X + scrollOffset.X, currentElementBoundingBox.Y + borderOffset.Y + scrollOffset.Y, currentElement->dimensions.Width, (float)borderConfig->width.betweenChildren },
											.renderData = { .rectangle = {
													.backgroundColor = borderConfig->color,
											} },
											.userData = sharedConfig->userData,
											.id = Clay__HashNumber(currentElement->id, currentElement->childrenOrTextContent.children.length + 1 + i).id,
											.commandType = CLAY_RENDER_COMMAND_TYPE_RECTANGLE,
										});
									}
									borderOffset.Y += (childElement->dimensions.Height + (float)layoutConfig->childGap);
								}
							}
						}
					}
				}
				// This exists because the scissor needs to end _after_ borders between elements
				if (closeScrollElement)
				{
					Clay__AddRenderCommand(NEW_STRUCT(Clay_RenderCommand) {
						.id = Clay__HashNumber(currentElement->id, rootElement->childrenOrTextContent.children.length + 11).id,
						.commandType = CLAY_RENDER_COMMAND_TYPE_SCISSOR_END,
					});
				}

				dfsBuffer.length--;
				continue;
			}

			// Add children to the DFS buffer
			if (!Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT))
			{
				dfsBuffer.length += currentElement->childrenOrTextContent.children.length;
				for (int32_t i = 0; i < currentElement->childrenOrTextContent.children.length; ++i)
				{
					Clay_LayoutElement *childElement = Clay_LayoutElementArray_Get(&context->layoutElements, currentElement->childrenOrTextContent.children.elements[i]);
					// Alignment along non layout axis
					if (layoutConfig->layoutDirection == CLAY_LEFT_TO_RIGHT)
					{
						currentElementTreeNode->nextChildOffset.Y = currentElement->layoutConfig->padding.top;
						float whiteSpaceAroundChild = currentElement->dimensions.Height - (float)(layoutConfig->padding.top + layoutConfig->padding.bottom) - childElement->dimensions.Height;
						switch (layoutConfig->childAlignment.y)
						{
							case CLAY_ALIGN_Y_TOP: break;
							case CLAY_ALIGN_Y_CENTER: currentElementTreeNode->nextChildOffset.Y += whiteSpaceAroundChild / 2; break;
							case CLAY_ALIGN_Y_BOTTOM: currentElementTreeNode->nextChildOffset.Y += whiteSpaceAroundChild; break;
						}
					}
					else
					{
						currentElementTreeNode->nextChildOffset.X = currentElement->layoutConfig->padding.left;
						float whiteSpaceAroundChild = currentElement->dimensions.Width - (float)(layoutConfig->padding.left + layoutConfig->padding.right) - childElement->dimensions.Width;
						switch (layoutConfig->childAlignment.x)
						{
							case CLAY_ALIGN_X_LEFT: break;
							case CLAY_ALIGN_X_CENTER: currentElementTreeNode->nextChildOffset.X += whiteSpaceAroundChild / 2; break;
							case CLAY_ALIGN_X_RIGHT: currentElementTreeNode->nextChildOffset.X += whiteSpaceAroundChild; break;
						}
					}

					v2 childPosition = {
						currentElementTreeNode->position.X + currentElementTreeNode->nextChildOffset.X + scrollOffset.X,
						currentElementTreeNode->position.Y + currentElementTreeNode->nextChildOffset.Y + scrollOffset.Y,
					};

					// DFS buffer elements need to be added in reverse because stack traversal happens backwards
					uint32_t newNodeIndex = dfsBuffer.length - 1 - i;
					dfsBuffer.items[newNodeIndex] = NEW_STRUCT(Clay__LayoutElementTreeNode) {
						.layoutElement = childElement,
						.position = childPosition,
						.nextChildOffset = { .X = (float)childElement->layoutConfig->padding.left, .Y = (float)childElement->layoutConfig->padding.top },
					};
					context->treeNodeVisited.items[newNodeIndex] = false;

					// Update parent offsets
					if (layoutConfig->layoutDirection == CLAY_LEFT_TO_RIGHT)
					{
						currentElementTreeNode->nextChildOffset.X += childElement->dimensions.Width + (float)layoutConfig->childGap;
					}
					else
					{
						currentElementTreeNode->nextChildOffset.Y += childElement->dimensions.Height + (float)layoutConfig->childGap;
					}
				}
			}
		}

		if (root->clipElementId)
		{
			Clay__AddRenderCommand(NEW_STRUCT(Clay_RenderCommand) { .id = Clay__HashNumber(rootElement->id, rootElement->childrenOrTextContent.children.length + 11).id, .commandType = CLAY_RENDER_COMMAND_TYPE_SCISSOR_END });
		}
	}
}

#pragma region DebugTools
Color32 CLAY__DEBUGVIEW_COLOR_1 = {.valueU32=0x373A3834}; //(58, 56, 52, 55)
Color32 CLAY__DEBUGVIEW_COLOR_2 = {.valueU32=0xFF3E3C3A}; //(62, 60, 58, 255)
Color32 CLAY__DEBUGVIEW_COLOR_3 = {.valueU32=0xFF8D8587}; //(141, 133, 135, 255)
Color32 CLAY__DEBUGVIEW_COLOR_4 = {.valueU32=0xFFEEE2E7}; //(238, 226, 231, 255)
Color32 CLAY__DEBUGVIEW_COLOR_SELECTED_ROW = {.valueU32=0xFF66504E}; //(102, 80, 78, 255)
const int32_t CLAY__DEBUGVIEW_ROW_HEIGHT = 30;
const int32_t CLAY__DEBUGVIEW_OUTER_PADDING = 10;
const int32_t CLAY__DEBUGVIEW_INDENT_WIDTH = 16;
Clay_TextElementConfig Clay__DebugView_TextNameConfig = {.textColor = {.valueU32=0xFFEEE2E7}, .fontSize = 16, .wrapMode = CLAY_TEXT_WRAP_NONE }; //textColor=(238, 226, 231, 255)
Clay_LayoutConfig Clay__DebugView_ScrollViewItemLayoutConfig = ZEROED;

typedef struct Clay__DebugElementConfigTypeLabelConfig Clay__DebugElementConfigTypeLabelConfig;
struct Clay__DebugElementConfigTypeLabelConfig
{
	Str8 label;
	Color32 color;
};

Clay__DebugElementConfigTypeLabelConfig Clay__DebugGetElementConfigTypeLabel(Clay__ElementConfigType type)
{
	switch (type)
	{
		case CLAY__ELEMENT_CONFIG_TYPE_SHARED:   return NEW_STRUCT(Clay__DebugElementConfigTypeLabelConfig) { CLAY_STRING("Shared"),   {.valueU32=0xFFF38630} }; //(243, 134, 48, 255)
		case CLAY__ELEMENT_CONFIG_TYPE_TEXT:     return NEW_STRUCT(Clay__DebugElementConfigTypeLabelConfig) { CLAY_STRING("Text"),     {.valueU32=0xFF69D2E7} }; //(105, 210, 231, 255)
		case CLAY__ELEMENT_CONFIG_TYPE_IMAGE:    return NEW_STRUCT(Clay__DebugElementConfigTypeLabelConfig) { CLAY_STRING("Image"),    {.valueU32=0xFF79BD9A} }; //(121, 189, 154, 255)
		case CLAY__ELEMENT_CONFIG_TYPE_FLOATING: return NEW_STRUCT(Clay__DebugElementConfigTypeLabelConfig) { CLAY_STRING("Floating"), {.valueU32=0xFFFA6900} }; //(250, 105, 0, 255)
		case CLAY__ELEMENT_CONFIG_TYPE_SCROLL:   return NEW_STRUCT(Clay__DebugElementConfigTypeLabelConfig) { CLAY_STRING("Scroll"),   {.valueU32=0xFFF2C45A} }; //(242, 196, 90, 255)
		case CLAY__ELEMENT_CONFIG_TYPE_BORDER:   return NEW_STRUCT(Clay__DebugElementConfigTypeLabelConfig) { CLAY_STRING("Border"),   {.valueU32=0xFF6C5B7B} }; //(108, 91, 123, 255)
		case CLAY__ELEMENT_CONFIG_TYPE_CUSTOM:   return NEW_STRUCT(Clay__DebugElementConfigTypeLabelConfig) { CLAY_STRING("Custom"),   {.valueU32=0xFF0B486B} }; //(11, 72, 107, 255)
		default: break;
	}
	return NEW_STRUCT(Clay__DebugElementConfigTypeLabelConfig) { CLAY_STRING("Error"), {.valueU32=0xFF000000} };
}

typedef struct Clay__RenderDebugLayoutData Clay__RenderDebugLayoutData;
struct Clay__RenderDebugLayoutData
{
	int32_t rowCount;
	int32_t selectedElementRowIndex;
};

// Returns row count
Clay__RenderDebugLayoutData Clay__RenderDebugLayoutElementsList(int32_t initialRootsLength, int32_t highlightedRowIndex)
{
	Clay_Context* context = Clay_GetCurrentContext();
	Clay__int32_tArray dfsBuffer = context->reusableElementIndexBuffer;
	Clay__DebugView_ScrollViewItemLayoutConfig = NEW_STRUCT(Clay_LayoutConfig) { .sizing = { .height = CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT) }, .childGap = 6, .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }};
	Clay__RenderDebugLayoutData layoutData = ZEROED;

	uint32_t highlightedElementId = 0;

	for (int32_t rootIndex = 0; rootIndex < initialRootsLength; ++rootIndex)
	{
		dfsBuffer.length = 0;
		Clay__LayoutElementTreeRoot *root = Clay__LayoutElementTreeRootArray_Get(&context->layoutElementTreeRoots, rootIndex);
		Clay__int32_tArray_Add(&dfsBuffer, (int32_t)root->layoutElementIndex);
		context->treeNodeVisited.items[0] = false;
		if (rootIndex > 0)
		{
			CLAY({ .id = CLAY_IDI("Clay__DebugView_EmptyRowOuter", rootIndex), .layout = { .sizing = {.width = CLAY_SIZING_GROW(0)}, .padding = {CLAY__DEBUGVIEW_INDENT_WIDTH / 2, 0, 0, 0} } })
			{
				CLAY({ .id = CLAY_IDI("Clay__DebugView_EmptyRow", rootIndex), .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)CLAY__DEBUGVIEW_ROW_HEIGHT) }}, .border = { .color = CLAY__DEBUGVIEW_COLOR_3, .width = { .top = 1 } } }) {}
			}
			layoutData.rowCount++;
		}
		while (dfsBuffer.length > 0)
		{
			int32_t currentElementIndex = Clay__int32_tArray_GetValue(&dfsBuffer, (int)dfsBuffer.length - 1);
			Clay_LayoutElement *currentElement = Clay_LayoutElementArray_Get(&context->layoutElements, (int)currentElementIndex);
			if (context->treeNodeVisited.items[dfsBuffer.length - 1])
			{
				if (!Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT) && currentElement->childrenOrTextContent.children.length > 0)
				{
					Clay__CloseElement();
					Clay__CloseElement();
					Clay__CloseElement();
				}
				dfsBuffer.length--;
				continue;
			}

			if (highlightedRowIndex == layoutData.rowCount)
			{
				if (context->pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
				{
					context->debugSelectedElementId = currentElement->id;
				}
				highlightedElementId = currentElement->id;
			}

			context->treeNodeVisited.items[dfsBuffer.length - 1] = true;
			Clay_LayoutElementHashMapItem *currentElementData = Clay__GetHashMapItem(currentElement->id);
			bool offscreen = Clay__ElementIsOffscreen(&currentElementData->boundingBox);
			if (context->debugSelectedElementId == currentElement->id)
			{
				layoutData.selectedElementRowIndex = layoutData.rowCount;
			}
			CLAY({ .id = CLAY_IDI("Clay__DebugView_ElementOuter", currentElement->id), .layout = Clay__DebugView_ScrollViewItemLayoutConfig })
			{
				// Collapse icon / button
				if (!(Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT) || currentElement->childrenOrTextContent.children.length == 0))
				{
					CLAY({
						.id = CLAY_IDI("Clay__DebugView_CollapseElement", currentElement->id),
						.layout = { .sizing = {CLAY_SIZING_FIXED(16), CLAY_SIZING_FIXED(16)}, .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER} },
						.cornerRadius = CLAY_CORNER_RADIUS(4),
						.border = { .color = CLAY__DEBUGVIEW_COLOR_3, .width = {1, 1, 1, 1, 0} },
					})
					{
						CLAY_TEXT((currentElementData && currentElementData->debugData->collapsed) ? CLAY_STRING("+") : CLAY_STRING("-"), CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_4, .fontSize = 16 }));
					}
				}
				else
				{ // Square dot for empty containers
					CLAY({ .layout = { .sizing = {CLAY_SIZING_FIXED(16), CLAY_SIZING_FIXED(16)}, .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER } } })
					{
						CLAY({ .layout = { .sizing = {CLAY_SIZING_FIXED(8), CLAY_SIZING_FIXED(8)} }, .backgroundColor = CLAY__DEBUGVIEW_COLOR_3, .cornerRadius = CLAY_CORNER_RADIUS(2) }) {}
					}
				}
				// Collisions and offscreen info
				if (currentElementData)
				{
					if (currentElementData->debugData->collision)
					{
						CLAY({ .layout = { .padding = { 8, 8, 2, 2 }}, .border = { .color = {.valueU32=0xFFB19308}, .width = {1, 1, 1, 1, 0} } }) //(177, 147, 8, 255)
						{
							CLAY_TEXT(CLAY_STRING("Duplicate ID"), CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_3, .fontSize = 16 }));
						}
					}
					if (offscreen)
					{
						CLAY({ .layout = { .padding = { 8, 8, 2, 2 } }, .border = {  .color = CLAY__DEBUGVIEW_COLOR_3, .width = { 1, 1, 1, 1, 0} } })
						{
							CLAY_TEXT(CLAY_STRING("Offscreen"), CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_3, .fontSize = 16 }));
						}
					}
				}
				Str8 idString = context->layoutElementIdStrings.items[currentElementIndex];
				if (idString.length > 0)
				{
					CLAY_TEXT(idString, offscreen ? CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_3, .fontSize = 16 }) : &Clay__DebugView_TextNameConfig);
				}
				for (int32_t elementConfigIndex = 0; elementConfigIndex < currentElement->elementConfigs.length; ++elementConfigIndex)
				{
					Clay_ElementConfig *elementConfig = Clay__ElementConfigArraySlice_Get(&currentElement->elementConfigs, elementConfigIndex);
					if (elementConfig->type == CLAY__ELEMENT_CONFIG_TYPE_SHARED)
					{
						Color32 labelColor = {.valueU32=0x5AF38630}; //(243, 134, 48, 90)
						labelColor.a = 90;
						Color32 backgroundColor = elementConfig->config.sharedElementConfig->backgroundColor;
						Clay_CornerRadius radius = elementConfig->config.sharedElementConfig->cornerRadius;
						if (backgroundColor.a > 0)
						{
							CLAY({ .layout = { .padding = { 8, 8, 2, 2 } }, .backgroundColor = labelColor, .cornerRadius = CLAY_CORNER_RADIUS(4), .border = { .color = labelColor, .width = { 1, 1, 1, 1, 0} } })
							{
								CLAY_TEXT(CLAY_STRING("Color"), CLAY_TEXT_CONFIG({ .textColor = offscreen ? CLAY__DEBUGVIEW_COLOR_3 : CLAY__DEBUGVIEW_COLOR_4, .fontSize = 16 }));
							}
						}
						if (radius.bottomLeft > 0)
						{
							CLAY({ .layout = { .padding = { 8, 8, 2, 2 } }, .backgroundColor = labelColor, .cornerRadius = CLAY_CORNER_RADIUS(4), .border = { .color = labelColor, .width = { 1, 1, 1, 1, 0 } } })
							{
								CLAY_TEXT(CLAY_STRING("Radius"), CLAY_TEXT_CONFIG({ .textColor = offscreen ? CLAY__DEBUGVIEW_COLOR_3 : CLAY__DEBUGVIEW_COLOR_4, .fontSize = 16 }));
							}
						}
						continue;
					}
					Clay__DebugElementConfigTypeLabelConfig config = Clay__DebugGetElementConfigTypeLabel(elementConfig->type);
					Color32 backgroundColor = config.color;
					backgroundColor.a = 90;
					CLAY({ .layout = { .padding = { 8, 8, 2, 2 } }, .backgroundColor = backgroundColor, .cornerRadius = CLAY_CORNER_RADIUS(4), .border = { .color = config.color, .width = { 1, 1, 1, 1, 0 } } })
					{
						CLAY_TEXT(config.label, CLAY_TEXT_CONFIG({ .textColor = offscreen ? CLAY__DEBUGVIEW_COLOR_3 : CLAY__DEBUGVIEW_COLOR_4, .fontSize = 16 }));
					}
				}
			}

			// Render the text contents below the element as a non-interactive row
			if (Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT))
			{
				layoutData.rowCount++;
				Clay__TextElementData *textElementData = currentElement->childrenOrTextContent.textElementData;
				Clay_TextElementConfig *rawTextConfig = offscreen ? CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_3, .fontSize = 16 }) : &Clay__DebugView_TextNameConfig;
				CLAY({ .layout = { .sizing = { .height = CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT)}, .childAlignment = { .y = CLAY_ALIGN_Y_CENTER } } })
				{
					CLAY({ .layout = { .sizing = {.width = CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_INDENT_WIDTH + 16) } } }) {}
					CLAY_TEXT(CLAY_STRING("\""), rawTextConfig);
					CLAY_TEXT(textElementData->text.length > 40 ? (NEW_STRUCT(Str8) { .length = 40, .chars = textElementData->text.chars }) : textElementData->text, rawTextConfig);
					if (textElementData->text.length > 40) { CLAY_TEXT(CLAY_STRING("..."), rawTextConfig); }
					CLAY_TEXT(CLAY_STRING("\""), rawTextConfig);
				}
			}
			else if (currentElement->childrenOrTextContent.children.length > 0)
			{
				Clay__OpenElement();
				Clay__ConfigureOpenElement(NEW_STRUCT(Clay_ElementDeclaration) { .layout = { .padding = { .left = 8 } } });
				Clay__OpenElement();
				Clay__ConfigureOpenElement(NEW_STRUCT(Clay_ElementDeclaration) { .layout = { .padding = { .left = CLAY__DEBUGVIEW_INDENT_WIDTH }}, .border = { .color = CLAY__DEBUGVIEW_COLOR_3, .width = { .left = 1 } }});
				Clay__OpenElement();
				Clay__ConfigureOpenElement(NEW_STRUCT(Clay_ElementDeclaration) { .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM } });
			}

			layoutData.rowCount++;
			if (!(Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT) || (currentElementData && currentElementData->debugData->collapsed)))
			{
				for (int32_t i = currentElement->childrenOrTextContent.children.length - 1; i >= 0; --i)
				{
					Clay__int32_tArray_Add(&dfsBuffer, currentElement->childrenOrTextContent.children.elements[i]);
					context->treeNodeVisited.items[dfsBuffer.length - 1] = false; // TODO needs to be ranged checked
				}
			}
		}
	}

	if (context->pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
	{
		Clay_ElementId collapseButtonId = Clay__HashString(CLAY_STRING("Clay__DebugView_CollapseElement"), 0, 0);
		for (int32_t i = (int)context->pointerOverIds.length - 1; i >= 0; i--)
		{
			Clay_ElementId *elementId = Clay__ElementIdArray_Get(&context->pointerOverIds, i);
			if (elementId->baseId == collapseButtonId.baseId)
			{
				Clay_LayoutElementHashMapItem *highlightedItem = Clay__GetHashMapItem(elementId->offset);
				highlightedItem->debugData->collapsed = !highlightedItem->debugData->collapsed;
				break;
			}
		}
	}

	if (highlightedElementId)
	{
		CLAY({ .id = CLAY_ID("Clay__DebugView_ElementHighlight"), .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)} }, .floating = { .parentId = highlightedElementId, .zIndex = 32767, .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH, .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID } })
		{
			CLAY({ .id = CLAY_ID("Clay__DebugView_ElementHighlightRectangle"), .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)} }, .backgroundColor = Clay__debugViewHighlightColor }) {}
		}
	}
	return layoutData;
}

void Clay__RenderDebugLayoutSizing(Clay_SizingAxis sizing, Clay_TextElementConfig *infoTextConfig)
{
	Str8 sizingLabel = CLAY_STRING("GROW");
	if (sizing.type == CLAY__SIZING_TYPE_FIT) { sizingLabel = CLAY_STRING("FIT"); }
	else if (sizing.type == CLAY__SIZING_TYPE_PERCENT) { sizingLabel = CLAY_STRING("PERCENT"); }
	CLAY_TEXT(sizingLabel, infoTextConfig);
	if (sizing.type == CLAY__SIZING_TYPE_GROW || sizing.type == CLAY__SIZING_TYPE_FIT)
	{
		CLAY_TEXT(CLAY_STRING("("), infoTextConfig);
		if (sizing.size.minMax.min != 0)
		{
			CLAY_TEXT(CLAY_STRING("min: "), infoTextConfig);
			CLAY_TEXT(Clay__IntToString(sizing.size.minMax.min), infoTextConfig);
			if (sizing.size.minMax.max != CLAY__MAXFLOAT) { CLAY_TEXT(CLAY_STRING(", "), infoTextConfig); }
		}
		if (sizing.size.minMax.max != CLAY__MAXFLOAT)
		{
			CLAY_TEXT(CLAY_STRING("max: "), infoTextConfig);
			CLAY_TEXT(Clay__IntToString(sizing.size.minMax.max), infoTextConfig);
		}
		CLAY_TEXT(CLAY_STRING(")"), infoTextConfig);
	}
}

void Clay__RenderDebugViewElementConfigHeader(Str8 elementId, Clay__ElementConfigType type)
{
	Clay__DebugElementConfigTypeLabelConfig config = Clay__DebugGetElementConfigTypeLabel(type);
	Color32 backgroundColor = config.color;
	backgroundColor.a = 90;
	CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) }, .padding = CLAY_PADDING_ALL(CLAY__DEBUGVIEW_OUTER_PADDING), .childAlignment = { .y = CLAY_ALIGN_Y_CENTER } } })
	{
		CLAY({ .layout = { .padding = { 8, 8, 2, 2 } }, .backgroundColor = backgroundColor, .cornerRadius = CLAY_CORNER_RADIUS(4), .border = { .color = config.color, .width = { 1, 1, 1, 1, 0 } } })
		{
			CLAY_TEXT(config.label, CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_4, .fontSize = 16 }));
		}
		CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) } } }) {}
		CLAY_TEXT(elementId, CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_3, .fontSize = 16, .wrapMode = CLAY_TEXT_WRAP_NONE }));
	}
}

void Clay__RenderDebugViewColor(Color32 color, Clay_TextElementConfig *textConfig)
{
	CLAY({ .layout = { .childAlignment = {.y = CLAY_ALIGN_Y_CENTER} } })
	{
		CLAY_TEXT(CLAY_STRING("{ r: "), textConfig);
		CLAY_TEXT(Clay__IntToString(color.r), textConfig);
		CLAY_TEXT(CLAY_STRING(", g: "), textConfig);
		CLAY_TEXT(Clay__IntToString(color.g), textConfig);
		CLAY_TEXT(CLAY_STRING(", b: "), textConfig);
		CLAY_TEXT(Clay__IntToString(color.b), textConfig);
		CLAY_TEXT(CLAY_STRING(", a: "), textConfig);
		CLAY_TEXT(Clay__IntToString(color.a), textConfig);
		CLAY_TEXT(CLAY_STRING(" }"), textConfig);
		CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_FIXED(10) } } }) {}
		CLAY({ .layout = { .sizing = { CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT - 8), CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT - 8)} }, .backgroundColor = color, .cornerRadius = CLAY_CORNER_RADIUS(4), .border = { .color = CLAY__DEBUGVIEW_COLOR_4, .width = { 1, 1, 1, 1, 0 } } }) {}
	}
}

void Clay__RenderDebugViewCornerRadius(Clay_CornerRadius cornerRadius, Clay_TextElementConfig *textConfig)
{
	CLAY({ .layout = { .childAlignment = {.y = CLAY_ALIGN_Y_CENTER} } })
	{
		CLAY_TEXT(CLAY_STRING("{ topLeft: "), textConfig);
		CLAY_TEXT(Clay__IntToString(cornerRadius.topLeft), textConfig);
		CLAY_TEXT(CLAY_STRING(", topRight: "), textConfig);
		CLAY_TEXT(Clay__IntToString(cornerRadius.topRight), textConfig);
		CLAY_TEXT(CLAY_STRING(", bottomLeft: "), textConfig);
		CLAY_TEXT(Clay__IntToString(cornerRadius.bottomLeft), textConfig);
		CLAY_TEXT(CLAY_STRING(", bottomRight: "), textConfig);
		CLAY_TEXT(Clay__IntToString(cornerRadius.bottomRight), textConfig);
		CLAY_TEXT(CLAY_STRING(" }"), textConfig);
	}
}

void HandleDebugViewCloseButtonInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, CLAY_ONHOVER_USERDATA_TYPE userData)
{
	Clay_Context* context = Clay_GetCurrentContext();
	(void) elementId; (void) pointerInfo; (void) userData;
	if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) { context->debugModeEnabled = false; }
}

void Clay__RenderDebugView(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	Clay_ElementId closeButtonId = Clay__HashString(CLAY_STRING("Clay__DebugViewTopHeaderCloseButtonOuter"), 0, 0);
	if (context->pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
	{
		for (int32_t i = 0; i < context->pointerOverIds.length; ++i)
		{
			Clay_ElementId *elementId = Clay__ElementIdArray_Get(&context->pointerOverIds, i);
			if (elementId->id == closeButtonId.id)
			{
				context->debugModeEnabled = false;
				return;
			}
		}
	}

	uint32_t initialRootsLength = context->layoutElementTreeRoots.length;
	uint32_t initialElementsLength = context->layoutElements.length;
	Clay_TextElementConfig *infoTextConfig = CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_4, .fontSize = 16, .wrapMode = CLAY_TEXT_WRAP_NONE });
	Clay_TextElementConfig *infoTitleConfig = CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_3, .fontSize = 16, .wrapMode = CLAY_TEXT_WRAP_NONE });
	Clay_ElementId scrollId = Clay__HashString(CLAY_STRING("Clay__DebugViewOuterScrollPane"), 0, 0);
	float scrollYOffset = 0;
	bool pointerInDebugView = context->pointerInfo.position.Y < context->layoutDimensions.Height - 300;
	for (int32_t i = 0; i < context->scrollContainerDatas.length; ++i)
	{
		Clay__ScrollContainerDataInternal *scrollContainerData = Clay__ScrollContainerDataInternalArray_Get(&context->scrollContainerDatas, i);
		if (scrollContainerData->elementId == scrollId.id)
		{
			if (!context->externalScrollHandlingEnabled)
			{
				scrollYOffset = scrollContainerData->scrollPosition.Y;
			}
			else
			{
				pointerInDebugView = context->pointerInfo.position.Y + scrollContainerData->scrollPosition.Y < context->layoutDimensions.Height - 300;
			}
			break;
		}
	}
	int32_t highlightedRow = pointerInDebugView
			? (int32_t)((context->pointerInfo.position.Y - scrollYOffset) / (float)CLAY__DEBUGVIEW_ROW_HEIGHT) - 1
			: -1;
	if (context->pointerInfo.position.X < context->layoutDimensions.Width - (float)Clay__debugViewWidth)
	{
		highlightedRow = -1;
	}
	Clay__RenderDebugLayoutData layoutData = ZEROED;
	CLAY({ .id = CLAY_ID("Clay__DebugView"),
		 .layout = { .sizing = { CLAY_SIZING_FIXED((float)Clay__debugViewWidth) , CLAY_SIZING_FIXED(context->layoutDimensions.Height) }, .layoutDirection = CLAY_TOP_TO_BOTTOM },
		.floating = { .zIndex = 32765, .attachPoints = { .element = CLAY_ATTACH_POINT_LEFT_CENTER, .parent = CLAY_ATTACH_POINT_RIGHT_CENTER }, .attachTo = CLAY_ATTACH_TO_ROOT },
		.border = { .color = CLAY__DEBUGVIEW_COLOR_3, .width = { .bottom = 1 } }
	})
	{
		CLAY({ .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT)}, .padding = {CLAY__DEBUGVIEW_OUTER_PADDING, CLAY__DEBUGVIEW_OUTER_PADDING, 0, 0 }, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER} }, .backgroundColor = CLAY__DEBUGVIEW_COLOR_2 })
		{
			CLAY_TEXT(CLAY_STRING("Clay Debug Tools"), infoTextConfig);
			CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) } } }) {}
			// Close button
			CLAY({
				.layout = { .sizing = {CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT - 10), CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT - 10)}, .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER} },
				.backgroundColor = {.valueU32=0x50D95B43}, //(217, 91, 67, 80)
				.cornerRadius = CLAY_CORNER_RADIUS(4),
				.border = { .color = {.valueU32=0xFFD95B43}, .width = { 1, 1, 1, 1, 0 } }, //(217, 91, 67, 255)
			})
			{
				CLAY_ONHOVER_USERDATA_TYPE zeroUserDataType = ZEROED;
				Clay_OnHover(HandleDebugViewCloseButtonInteraction, zeroUserDataType);
				CLAY_TEXT(CLAY_STRING("x"), CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_4, .fontSize = 16 }));
			}
		}
		CLAY({ .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(1)} }, .backgroundColor = CLAY__DEBUGVIEW_COLOR_3 } ) {}
		CLAY({ .id = scrollId, .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)} }, .scroll = { .horizontal = true, .vertical = true } })
		{
			CLAY({ .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }, .backgroundColor = ((initialElementsLength + initialRootsLength) & 1) == 0 ? CLAY__DEBUGVIEW_COLOR_2 : CLAY__DEBUGVIEW_COLOR_1 })
			{
				Clay_ElementId panelContentsId = Clay__HashString(CLAY_STRING("Clay__DebugViewPaneOuter"), 0, 0);
				// Element list
				CLAY({ .id = panelContentsId, .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)} }, .floating = { .zIndex = 32766, .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH, .attachTo = CLAY_ATTACH_TO_PARENT } })
				{
					CLAY({ .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}, .padding = { CLAY__DEBUGVIEW_OUTER_PADDING, CLAY__DEBUGVIEW_OUTER_PADDING, 0, 0 }, .layoutDirection = CLAY_TOP_TO_BOTTOM } })
					{
						layoutData = Clay__RenderDebugLayoutElementsList((int32_t)initialRootsLength, highlightedRow);
					}
				}
				float contentWidth = Clay__GetHashMapItem(panelContentsId.id)->layoutElement->dimensions.Width;
				CLAY({ .layout = { .sizing = {.width = CLAY_SIZING_FIXED(contentWidth) }, .layoutDirection = CLAY_TOP_TO_BOTTOM } }) {}
				for (int32_t i = 0; i < layoutData.rowCount; i++)
				{
					Color32 rowColor = (i & 1) == 0 ? CLAY__DEBUGVIEW_COLOR_2 : CLAY__DEBUGVIEW_COLOR_1;
					if (i == layoutData.selectedElementRowIndex)
					{
						rowColor = CLAY__DEBUGVIEW_COLOR_SELECTED_ROW;
					}
					if (i == highlightedRow)
					{
						rowColor.r *= 1.25f;
						rowColor.g *= 1.25f;
						rowColor.b *= 1.25f;
					}
					CLAY({ .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }, .backgroundColor = rowColor } ) {}
				}
			}
		}
		CLAY({ .layout = { .sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(1)} }, .backgroundColor = CLAY__DEBUGVIEW_COLOR_3 }) {}
		if (context->debugSelectedElementId != 0)
		{
			Clay_LayoutElementHashMapItem *selectedItem = Clay__GetHashMapItem(context->debugSelectedElementId);
			CLAY({
				.layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(300)}, .layoutDirection = CLAY_TOP_TO_BOTTOM },
				.backgroundColor = CLAY__DEBUGVIEW_COLOR_2 ,
				.scroll = { .vertical = true },
				.border = { .color = CLAY__DEBUGVIEW_COLOR_3, .width = { .betweenChildren = 1 } }
			})
			{
				CLAY({ .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT + 8)}, .padding = {CLAY__DEBUGVIEW_OUTER_PADDING, CLAY__DEBUGVIEW_OUTER_PADDING, 0, 0 }, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER} } })
				{
					CLAY_TEXT(CLAY_STRING("Layout Config"), infoTextConfig);
					CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) } } }) {}
					if (selectedItem->elementId.stringId.length != 0)
					{
						CLAY_TEXT(selectedItem->elementId.stringId, infoTitleConfig);
						if (selectedItem->elementId.offset != 0)
						{
							CLAY_TEXT(CLAY_STRING(" ("), infoTitleConfig);
							CLAY_TEXT(Clay__IntToString(selectedItem->elementId.offset), infoTitleConfig);
							CLAY_TEXT(CLAY_STRING(")"), infoTitleConfig);
						}
					}
				}
				Clay_Padding attributeConfigPadding = {CLAY__DEBUGVIEW_OUTER_PADDING, CLAY__DEBUGVIEW_OUTER_PADDING, 8, 8};
				// Clay_LayoutConfig debug info
				CLAY({ .layout = { .padding = attributeConfigPadding, .childGap = 8, .layoutDirection = CLAY_TOP_TO_BOTTOM } })
				{
					// .boundingBox
					CLAY_TEXT(CLAY_STRING("Bounding Box"), infoTitleConfig);
					CLAY({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT } })
					{
						CLAY_TEXT(CLAY_STRING("{ x: "), infoTextConfig);
						CLAY_TEXT(Clay__IntToString(selectedItem->boundingBox.X), infoTextConfig);
						CLAY_TEXT(CLAY_STRING(", y: "), infoTextConfig);
						CLAY_TEXT(Clay__IntToString(selectedItem->boundingBox.Y), infoTextConfig);
						CLAY_TEXT(CLAY_STRING(", width: "), infoTextConfig);
						CLAY_TEXT(Clay__IntToString(selectedItem->boundingBox.Width), infoTextConfig);
						CLAY_TEXT(CLAY_STRING(", height: "), infoTextConfig);
						CLAY_TEXT(Clay__IntToString(selectedItem->boundingBox.Height), infoTextConfig);
						CLAY_TEXT(CLAY_STRING(" }"), infoTextConfig);
					}
					// .layoutDirection
					CLAY_TEXT(CLAY_STRING("Layout Direction"), infoTitleConfig);
					Clay_LayoutConfig *layoutConfig = selectedItem->layoutElement->layoutConfig;
					CLAY_TEXT(layoutConfig->layoutDirection == CLAY_TOP_TO_BOTTOM ? CLAY_STRING("TOP_TO_BOTTOM") : CLAY_STRING("LEFT_TO_RIGHT"), infoTextConfig);
					// .sizing
					CLAY_TEXT(CLAY_STRING("Sizing"), infoTitleConfig);
					CLAY({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT } })
					{
						CLAY_TEXT(CLAY_STRING("width: "), infoTextConfig);
						Clay__RenderDebugLayoutSizing(layoutConfig->sizing.width, infoTextConfig);
					}
					CLAY({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT } })
					{
						CLAY_TEXT(CLAY_STRING("height: "), infoTextConfig);
						Clay__RenderDebugLayoutSizing(layoutConfig->sizing.height, infoTextConfig);
					}
					// .padding
					CLAY_TEXT(CLAY_STRING("Padding"), infoTitleConfig);
					CLAY({ .id = CLAY_ID("Clay__DebugViewElementInfoPadding") })
					{
						CLAY_TEXT(CLAY_STRING("{ left: "), infoTextConfig);
						CLAY_TEXT(Clay__IntToString(layoutConfig->padding.left), infoTextConfig);
						CLAY_TEXT(CLAY_STRING(", right: "), infoTextConfig);
						CLAY_TEXT(Clay__IntToString(layoutConfig->padding.right), infoTextConfig);
						CLAY_TEXT(CLAY_STRING(", top: "), infoTextConfig);
						CLAY_TEXT(Clay__IntToString(layoutConfig->padding.top), infoTextConfig);
						CLAY_TEXT(CLAY_STRING(", bottom: "), infoTextConfig);
						CLAY_TEXT(Clay__IntToString(layoutConfig->padding.bottom), infoTextConfig);
						CLAY_TEXT(CLAY_STRING(" }"), infoTextConfig);
					}
					// .childGap
					CLAY_TEXT(CLAY_STRING("Child Gap"), infoTitleConfig);
					CLAY_TEXT(Clay__IntToString(layoutConfig->childGap), infoTextConfig);
					// .childAlignment
					CLAY_TEXT(CLAY_STRING("Child Alignment"), infoTitleConfig);
					CLAY({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT } })
					{
						CLAY_TEXT(CLAY_STRING("{ x: "), infoTextConfig);
						Str8 alignX = CLAY_STRING("LEFT");
						if (layoutConfig->childAlignment.x == CLAY_ALIGN_X_CENTER) { alignX = CLAY_STRING("CENTER"); }
						else if (layoutConfig->childAlignment.x == CLAY_ALIGN_X_RIGHT) { alignX = CLAY_STRING("RIGHT"); }
						CLAY_TEXT(alignX, infoTextConfig);
						CLAY_TEXT(CLAY_STRING(", y: "), infoTextConfig);
						Str8 alignY = CLAY_STRING("TOP");
						if (layoutConfig->childAlignment.y == CLAY_ALIGN_Y_CENTER) { alignY = CLAY_STRING("CENTER"); }
						else if (layoutConfig->childAlignment.y == CLAY_ALIGN_Y_BOTTOM) { alignY = CLAY_STRING("BOTTOM"); }
						CLAY_TEXT(alignY, infoTextConfig);
						CLAY_TEXT(CLAY_STRING(" }"), infoTextConfig);
					}
				}
				for (int32_t elementConfigIndex = 0; elementConfigIndex < selectedItem->layoutElement->elementConfigs.length; ++elementConfigIndex)
				{
					Clay_ElementConfig *elementConfig = Clay__ElementConfigArraySlice_Get(&selectedItem->layoutElement->elementConfigs, elementConfigIndex);
					Clay__RenderDebugViewElementConfigHeader(selectedItem->elementId.stringId, elementConfig->type);
					switch (elementConfig->type)
					{
						case CLAY__ELEMENT_CONFIG_TYPE_SHARED:
						{
							Clay_SharedElementConfig *sharedConfig = elementConfig->config.sharedElementConfig;
							CLAY({ .layout = { .padding = attributeConfigPadding, .childGap = 8, .layoutDirection = CLAY_TOP_TO_BOTTOM }})
							{
								// .backgroundColor
								CLAY_TEXT(CLAY_STRING("Background Color"), infoTitleConfig);
								Clay__RenderDebugViewColor(sharedConfig->backgroundColor, infoTextConfig);
								// .cornerRadius
								CLAY_TEXT(CLAY_STRING("Corner Radius"), infoTitleConfig);
								Clay__RenderDebugViewCornerRadius(sharedConfig->cornerRadius, infoTextConfig);
							}
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_TEXT:
						{
							Clay_TextElementConfig *textConfig = elementConfig->config.textElementConfig;
							CLAY({ .layout = { .padding = attributeConfigPadding, .childGap = 8, .layoutDirection = CLAY_TOP_TO_BOTTOM } })
							{
								// .fontSize
								CLAY_TEXT(CLAY_STRING("Font Size"), infoTitleConfig);
								CLAY_TEXT(Clay__IntToString(textConfig->fontSize), infoTextConfig);
								// .fontId
								CLAY_TEXT(CLAY_STRING("Font ID"), infoTitleConfig);
								CLAY_TEXT(Clay__IntToString(textConfig->fontId), infoTextConfig);
								// .lineHeight
								CLAY_TEXT(CLAY_STRING("Line Height"), infoTitleConfig);
								CLAY_TEXT(textConfig->lineHeight == 0 ? CLAY_STRING("auto") : Clay__IntToString(textConfig->lineHeight), infoTextConfig);
								// .letterSpacing
								CLAY_TEXT(CLAY_STRING("Letter Spacing"), infoTitleConfig);
								CLAY_TEXT(Clay__IntToString(textConfig->letterSpacing), infoTextConfig);
								// .wrapMode
								CLAY_TEXT(CLAY_STRING("Wrap Mode"), infoTitleConfig);
								Str8 wrapMode = CLAY_STRING("WORDS");
								if (textConfig->wrapMode == CLAY_TEXT_WRAP_NONE) { wrapMode = CLAY_STRING("NONE"); }
								else if (textConfig->wrapMode == CLAY_TEXT_WRAP_NEWLINES) { wrapMode = CLAY_STRING("NEWLINES"); }
								CLAY_TEXT(wrapMode, infoTextConfig);
								// .textAlignment
								CLAY_TEXT(CLAY_STRING("Text Alignment"), infoTitleConfig);
								Str8 textAlignment = CLAY_STRING("LEFT");
								if (textConfig->textAlignment == CLAY_TEXT_ALIGN_CENTER) { textAlignment = CLAY_STRING("CENTER"); }
								else if (textConfig->textAlignment == CLAY_TEXT_ALIGN_RIGHT) { textAlignment = CLAY_STRING("RIGHT"); }
								CLAY_TEXT(textAlignment, infoTextConfig);
								// .textColor
								CLAY_TEXT(CLAY_STRING("Text Color"), infoTitleConfig);
								Clay__RenderDebugViewColor(textConfig->textColor, infoTextConfig);
							}
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_IMAGE:
						{
							Clay_ImageElementConfig *imageConfig = elementConfig->config.imageElementConfig;
							CLAY({ .id = CLAY_ID("Clay__DebugViewElementInfoImageBody"), .layout = { .padding = attributeConfigPadding, .childGap = 8, .layoutDirection = CLAY_TOP_TO_BOTTOM } })
							{
								// .sourceDimensions
								CLAY_TEXT(CLAY_STRING("Source Dimensions"), infoTitleConfig);
								CLAY({ .id = CLAY_ID("Clay__DebugViewElementInfoImageDimensions") })
								{
									CLAY_TEXT(CLAY_STRING("{ width: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(imageConfig->sourceDimensions.Width), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(", height: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(imageConfig->sourceDimensions.Height), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(" }"), infoTextConfig);
								}
								// Image Preview
								CLAY_TEXT(CLAY_STRING("Preview"), infoTitleConfig);
								CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0, imageConfig->sourceDimensions.Width) }}, .image = *imageConfig }) {}
							}
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_SCROLL:
						{
							Clay_ScrollElementConfig *scrollConfig = elementConfig->config.scrollElementConfig;
							CLAY({ .layout = { .padding = attributeConfigPadding, .childGap = 8, .layoutDirection = CLAY_TOP_TO_BOTTOM } })
							{
								// .vertical
								CLAY_TEXT(CLAY_STRING("Vertical"), infoTitleConfig);
								CLAY_TEXT(scrollConfig->vertical ? CLAY_STRING("true") : CLAY_STRING("false") , infoTextConfig);
								// .horizontal
								CLAY_TEXT(CLAY_STRING("Horizontal"), infoTitleConfig);
								CLAY_TEXT(scrollConfig->horizontal ? CLAY_STRING("true") : CLAY_STRING("false") , infoTextConfig);
							}
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_FLOATING:
						{
							Clay_FloatingElementConfig *floatingConfig = elementConfig->config.floatingElementConfig;
							CLAY({ .layout = { .padding = attributeConfigPadding, .childGap = 8, .layoutDirection = CLAY_TOP_TO_BOTTOM } })
							{
								// .offset
								CLAY_TEXT(CLAY_STRING("Offset"), infoTitleConfig);
								CLAY({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT } })
								{
									CLAY_TEXT(CLAY_STRING("{ x: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(floatingConfig->offset.X), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(", y: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(floatingConfig->offset.Y), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(" }"), infoTextConfig);
								}
								// .expand
								CLAY_TEXT(CLAY_STRING("Expand"), infoTitleConfig);
								CLAY({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT } })
								{
									CLAY_TEXT(CLAY_STRING("{ width: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(floatingConfig->expand.Width), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(", height: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(floatingConfig->expand.Height), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(" }"), infoTextConfig);
								}
								// .zIndex
								CLAY_TEXT(CLAY_STRING("z-index"), infoTitleConfig);
								CLAY_TEXT(Clay__IntToString(floatingConfig->zIndex), infoTextConfig);
								// .parentId
								CLAY_TEXT(CLAY_STRING("Parent"), infoTitleConfig);
								Clay_LayoutElementHashMapItem *hashItem = Clay__GetHashMapItem(floatingConfig->parentId);
								CLAY_TEXT(hashItem->elementId.stringId, infoTextConfig);
							}
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_BORDER:
						{
							Clay_BorderElementConfig *borderConfig = elementConfig->config.borderElementConfig;
							CLAY({ .id = CLAY_ID("Clay__DebugViewElementInfoBorderBody"), .layout = { .padding = attributeConfigPadding, .childGap = 8, .layoutDirection = CLAY_TOP_TO_BOTTOM } })
							{
								CLAY_TEXT(CLAY_STRING("Border Widths"), infoTitleConfig);
								CLAY({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT } })
								{
									CLAY_TEXT(CLAY_STRING("{ left: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(borderConfig->width.left), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(", right: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(borderConfig->width.right), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(", top: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(borderConfig->width.top), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(", bottom: "), infoTextConfig);
									CLAY_TEXT(Clay__IntToString(borderConfig->width.bottom), infoTextConfig);
									CLAY_TEXT(CLAY_STRING(" }"), infoTextConfig);
								}
								// .textColor
								CLAY_TEXT(CLAY_STRING("Border Color"), infoTitleConfig);
								Clay__RenderDebugViewColor(borderConfig->color, infoTextConfig);
							}
							break;
						}
						case CLAY__ELEMENT_CONFIG_TYPE_CUSTOM:
						default: break;
					}
				}
			}
		}
		else
		{
			CLAY({ .id = CLAY_ID("Clay__DebugViewWarningsScrollPane"), .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(300)}, .childGap = 6, .layoutDirection = CLAY_TOP_TO_BOTTOM }, .backgroundColor = CLAY__DEBUGVIEW_COLOR_2, .scroll = { .horizontal = true, .vertical = true } })
			{
				Clay_TextElementConfig *warningConfig = CLAY_TEXT_CONFIG({ .textColor = CLAY__DEBUGVIEW_COLOR_4, .fontSize = 16, .wrapMode = CLAY_TEXT_WRAP_NONE });
				CLAY({ .id = CLAY_ID("Clay__DebugViewWarningItemHeader"), .layout = { .sizing = {.height = CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT)}, .padding = {CLAY__DEBUGVIEW_OUTER_PADDING, CLAY__DEBUGVIEW_OUTER_PADDING, 0, 0 }, .childGap = 8, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER} } })
				{
					CLAY_TEXT(CLAY_STRING("Warnings"), warningConfig);
				}
				CLAY({ .id = CLAY_ID("Clay__DebugViewWarningsTopBorder"), .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(1)} }, .backgroundColor = {.valueU32=0xFFC8C8C8} }) {} //(200, 200, 200, 255)
				int32_t previousWarningsLength = context->warnings.length;
				for (int32_t i = 0; i < previousWarningsLength; i++)
				{
					Clay__Warning warning = context->warnings.items[i];
					CLAY({ .id = CLAY_IDI("Clay__DebugViewWarningItem", i), .layout = { .sizing = {.height = CLAY_SIZING_FIXED(CLAY__DEBUGVIEW_ROW_HEIGHT)}, .padding = {CLAY__DEBUGVIEW_OUTER_PADDING, CLAY__DEBUGVIEW_OUTER_PADDING, 0, 0 }, .childGap = 8, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER} } })
					{
						CLAY_TEXT(warning.baseMessage, warningConfig);
						if (warning.dynamicMessage.length > 0)
						{
							CLAY_TEXT(warning.dynamicMessage, warningConfig);
						}
					}
				}
			}
		}
	}
}
#pragma endregion

uint32_t Clay__debugViewWidth = 400;
Color32 Clay__debugViewHighlightColor = {.valueU32=0x64A8421C}; //(168, 66, 28, 100)

Clay__WarningArray Clay__WarningArray_Allocate_Arena(int32_t initialCapacity, Clay_Arena* arena)
{
	size_t totalSizeBytes = initialCapacity * sizeof(Str8);
	Clay__WarningArray array = { .allocLength = initialCapacity, .length = 0 };
	uintptr_t nextAllocOffset = arena->nextAllocation + (64 - (arena->nextAllocation % 64));
	if (nextAllocOffset + totalSizeBytes <= arena->capacity)
	{
		array.items = (Clay__Warning*)((uintptr_t)arena->memory + (uintptr_t)nextAllocOffset);
		arena->nextAllocation = nextAllocOffset + totalSizeBytes;
	}
	else
	{
		Clay__currentContext->errorHandler.errorHandlerFunction(NEW_STRUCT(Clay_ErrorData) {
			.errorType = CLAY_ERROR_TYPE_ARENA_CAPACITY_EXCEEDED,
			.errorText = CLAY_STRING("Clay attempted to allocate memory in its arena, but ran out of capacity. Try increasing the capacity of the arena passed to Clay_Initialize()"),
			.userData = Clay__currentContext->errorHandler.userData
		});
	}
	return array;
}

Clay__Warning *Clay__WarningArray_Add(Clay__WarningArray *array, Clay__Warning item)
{
	if (array->length < array->allocLength)
	{
		array->items[array->length++] = item;
		return &array->items[array->length - 1];
	}
	return &CLAY__WARNING_DEFAULT;
}

bool Clay__Array_RangeCheck(int32_t index, int32_t length)
{
	if (index < length && index >= 0) { return true; }
	Clay_Context* context = Clay_GetCurrentContext();
	context->errorHandler.errorHandlerFunction(NEW_STRUCT(Clay_ErrorData) {
		.errorType = CLAY_ERROR_TYPE_INTERNAL_ERROR,
		.errorText = CLAY_STRING("Clay attempted to make an out of bounds array access. This is an internal error and is likely a bug."),
		.userData = context->errorHandler.userData
	});
	return false;
}

bool Clay__Array_AddCapacityCheck(int32_t length, int32_t allocLength)
{
	if (length < allocLength) { return true; }
	Clay_Context* context = Clay_GetCurrentContext();
	context->errorHandler.errorHandlerFunction(NEW_STRUCT(Clay_ErrorData) {
		.errorType = CLAY_ERROR_TYPE_INTERNAL_ERROR,
		.errorText = CLAY_STRING("Clay attempted to make an out of bounds array access. This is an internal error and is likely a bug."),
		.userData = context->errorHandler.userData
	});
	return false;
}

// PUBLIC API FROM HERE ---------------------------------------

CLAY_WASM_EXPORT("Clay_MinMemorySize")
CLAY_DECOR uint32_t Clay_MinMemorySize(void)
{
	Clay_Context fakeContext = {
		.maxElementCount = Clay__defaultMaxElementCount,
		.maxMeasureTextCacheWordCount = Clay__defaultMaxMeasureTextWordCacheCount,
		.internalArena = {
			.capacity = SIZE_MAX,
			.memory = NULL,
		}
	};
	Clay_Context* currentContext = Clay_GetCurrentContext();
	if (currentContext)
	{
		fakeContext.maxElementCount = currentContext->maxElementCount;
		fakeContext.maxMeasureTextCacheWordCount = currentContext->maxElementCount;
	}
	// Reserve space in the arena for the context, important for calculating min memory size correctly
	Clay__Context_Allocate_Arena(&fakeContext.internalArena);
	Clay__InitializePersistentMemory(&fakeContext);
	Clay__InitializeEphemeralMemory(&fakeContext);
	return fakeContext.internalArena.nextAllocation + 128;
}

CLAY_WASM_EXPORT("Clay_CreateArenaWithCapacityAndMemory")
CLAY_DECOR Clay_Arena Clay_CreateArenaWithCapacityAndMemory(uint32_t capacity, void *memory)
{
	Clay_Arena arena = {
		.capacity = capacity,
		.memory = (char *)memory
	};
	return arena;
}

#ifndef CLAY_WASM
CLAY_DECOR void Clay_SetMeasureTextFunction(ClayMeasureText_f* measureTextFunction, CLAY_MEASURE_USERDATA_TYPE userData)
{
	Clay_Context* context = Clay_GetCurrentContext();
	Clay__MeasureText = measureTextFunction;
	context->measureTextUserData = userData;
}
CLAY_DECOR void Clay_SetQueryScrollOffsetFunction(v2 (*queryScrollOffsetFunction)(uint32_t elementId, CLAY_QUERYSCROLL_USERDATA_TYPE userData), CLAY_QUERYSCROLL_USERDATA_TYPE userData)
{
	Clay_Context* context = Clay_GetCurrentContext();
	Clay__QueryScrollOffset = queryScrollOffsetFunction;
	context->queryScrollOffsetUserData = userData;
}
#endif

CLAY_WASM_EXPORT("Clay_SetLayoutDimensions")
CLAY_DECOR void Clay_SetLayoutDimensions(v2 dimensions)
{
	Clay_GetCurrentContext()->layoutDimensions = dimensions;
}

CLAY_WASM_EXPORT("Clay_SetPointerState")
CLAY_DECOR void Clay_SetPointerState(v2 position, bool isPointerDown)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->booleanWarnings.maxElementsExceeded) { return; }
	context->pointerInfo.position = position;
	context->pointerOverIds.length = 0;
	Clay__int32_tArray dfsBuffer = context->layoutElementChildrenBuffer;
	for (int32_t rootIndex = context->layoutElementTreeRoots.length - 1; rootIndex >= 0; --rootIndex)
	{
		dfsBuffer.length = 0;
		Clay__LayoutElementTreeRoot *root = Clay__LayoutElementTreeRootArray_Get(&context->layoutElementTreeRoots, rootIndex);
		Clay__int32_tArray_Add(&dfsBuffer, (int32_t)root->layoutElementIndex);
		context->treeNodeVisited.items[0] = false;
		bool found = false;
		while (dfsBuffer.length > 0)
		{
			if (context->treeNodeVisited.items[dfsBuffer.length - 1])
			{
				dfsBuffer.length--;
				continue;
			}
			context->treeNodeVisited.items[dfsBuffer.length - 1] = true;
			Clay_LayoutElement *currentElement = Clay_LayoutElementArray_Get(&context->layoutElements, Clay__int32_tArray_GetValue(&dfsBuffer, (int)dfsBuffer.length - 1));
			Clay_LayoutElementHashMapItem *mapItem = Clay__GetHashMapItem(currentElement->id); // TODO think of a way around this, maybe the fact that it's essentially a binary tree limits the cost, but the worst case is not great
			rec elementBox = mapItem->boundingBox;
			elementBox.X -= root->pointerOffset.X;
			elementBox.Y -= root->pointerOffset.Y;
			if (mapItem)
			{
				if ((Clay__PointIsInsideRect(position, elementBox)))
				{
					if (mapItem->onHoverFunction)
					{
						mapItem->onHoverFunction(mapItem->elementId, context->pointerInfo, mapItem->hoverFunctionUserData);
					}
					Clay__ElementIdArray_Add(&context->pointerOverIds, mapItem->elementId);
					found = true;

					if (mapItem->idAlias != 0)
					{
						Clay__ElementIdArray_Add(&context->pointerOverIds, NEW_STRUCT(Clay_ElementId) { .id = mapItem->idAlias });
					}
				}
				if (Clay__ElementHasConfig(currentElement, CLAY__ELEMENT_CONFIG_TYPE_TEXT))
				{
					dfsBuffer.length--;
					continue;
				}
				for (int32_t i = currentElement->childrenOrTextContent.children.length - 1; i >= 0; --i)
				{
					Clay__int32_tArray_Add(&dfsBuffer, currentElement->childrenOrTextContent.children.elements[i]);
					context->treeNodeVisited.items[dfsBuffer.length - 1] = false; // TODO needs to be ranged checked
				}
			}
			else
			{
				dfsBuffer.length--;
			}
		}

		Clay_LayoutElement *rootElement = Clay_LayoutElementArray_Get(&context->layoutElements, root->layoutElementIndex);
		if (found && Clay__ElementHasConfig(rootElement, CLAY__ELEMENT_CONFIG_TYPE_FLOATING) &&
			Clay__FindElementConfigWithType(rootElement, CLAY__ELEMENT_CONFIG_TYPE_FLOATING).floatingElementConfig->pointerCaptureMode == CLAY_POINTER_CAPTURE_MODE_CAPTURE)
		{
			break;
		}
	}

	if (isPointerDown)
	{
		if (context->pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
		{
			context->pointerInfo.state = CLAY_POINTER_DATA_PRESSED;
		}
		else if (context->pointerInfo.state != CLAY_POINTER_DATA_PRESSED)
		{
			context->pointerInfo.state = CLAY_POINTER_DATA_PRESSED_THIS_FRAME;
		}
	} 
	else
	{
		if (context->pointerInfo.state == CLAY_POINTER_DATA_RELEASED_THIS_FRAME)
		{
			context->pointerInfo.state = CLAY_POINTER_DATA_RELEASED;
		}
		else if (context->pointerInfo.state != CLAY_POINTER_DATA_RELEASED)
		{
			context->pointerInfo.state = CLAY_POINTER_DATA_RELEASED_THIS_FRAME;
		}
	}
}

CLAY_WASM_EXPORT("Clay_Initialize")
CLAY_DECOR Clay_Context* Clay_Initialize(Clay_Arena arena, v2 layoutDimensions, Clay_ErrorHandler errorHandler)
{
	Clay_Context *context = Clay__Context_Allocate_Arena(&arena);
	if (context == NULL) { return NULL; }
	// DEFAULTS
	Clay_Context *oldContext = Clay_GetCurrentContext();
	*context = NEW_STRUCT(Clay_Context) {
		.maxElementCount = oldContext ? oldContext->maxElementCount : Clay__defaultMaxElementCount,
		.maxMeasureTextCacheWordCount = oldContext ? oldContext->maxMeasureTextCacheWordCount : Clay__defaultMaxMeasureTextWordCacheCount,
		.errorHandler = errorHandler.errorHandlerFunction ? errorHandler : NEW_STRUCT(Clay_ErrorHandler) { Clay__ErrorHandlerFunctionDefault, 0 },
		.layoutDimensions = layoutDimensions,
		.internalArena = arena,
	};
	Clay_SetCurrentContext(context);
	Clay__InitializePersistentMemory(context);
	Clay__InitializeEphemeralMemory(context);
	for (int32_t i = 0; i < context->layoutElementsHashMap.allocLength; ++i)
	{
		context->layoutElementsHashMap.items[i] = -1;
	}
	for (int32_t i = 0; i < context->measureTextHashMap.allocLength; ++i)
	{
		context->measureTextHashMap.items[i] = 0;
	}
	context->measureTextHashMapInternal.length = 1; // Reserve the 0 value to mean "no next element"
	context->layoutDimensions = layoutDimensions;
	return context;
}

CLAY_WASM_EXPORT("Clay_GetCurrentContext")
CLAY_DECOR Clay_Context* Clay_GetCurrentContext(void)
{
	return Clay__currentContext;
}

CLAY_WASM_EXPORT("Clay_SetCurrentContext")
CLAY_DECOR void Clay_SetCurrentContext(Clay_Context* context)
{
	Clay__currentContext = context;
}

CLAY_WASM_EXPORT("Clay_UpdateScrollContainers")
CLAY_DECOR bool Clay_UpdateScrollContainers(bool enableDragScrolling, v2 scrollDelta, float deltaTime)
{
	Clay_Context* context = Clay_GetCurrentContext();
	bool isAutoScrollingOccurring = false;
	bool isPointerActive = enableDragScrolling && (context->pointerInfo.state == CLAY_POINTER_DATA_PRESSED || context->pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME);
	// Don't apply scroll events to ancestors of the inner element
	int32_t highestPriorityElementIndex = -1;
	Clay__ScrollContainerDataInternal *highestPriorityScrollData = CLAY__NULL;
	for (int32_t i = 0; i < context->scrollContainerDatas.length; i++)
	{
		Clay__ScrollContainerDataInternal *scrollData = Clay__ScrollContainerDataInternalArray_Get(&context->scrollContainerDatas, i);
		if (!scrollData->openThisFrame)
		{
			Clay__ScrollContainerDataInternalArray_RemoveSwapback(&context->scrollContainerDatas, i);
			continue;
		}
		scrollData->openThisFrame = false;
		Clay_LayoutElementHashMapItem *hashMapItem = Clay__GetHashMapItem(scrollData->elementId);
		// Element isn't rendered this frame but scroll offset has been retained
		if (!hashMapItem)
		{
			Clay__ScrollContainerDataInternalArray_RemoveSwapback(&context->scrollContainerDatas, i);
			continue;
		}

		// Touch / click is released
		if (!isPointerActive && scrollData->pointerScrollActive)
		{
			float xDiff = scrollData->scrollTarget.X - scrollData->scrollOrigin.X;
			if (xDiff < -10 || xDiff > 10)
			{
				scrollData->scrollMomentum.X = (scrollData->scrollTarget.X - scrollData->scrollOrigin.X) / (scrollData->momentumTime * 25);
			}
			float yDiff = scrollData->scrollTarget.Y - scrollData->scrollOrigin.Y;
			if (yDiff < -10 || yDiff > 10)
			{
				scrollData->scrollMomentum.Y = (scrollData->scrollTarget.Y - scrollData->scrollOrigin.Y) / (scrollData->momentumTime * 25);
			}
			scrollData->pointerScrollActive = false;

			scrollData->pointerOrigin = V2_Zero;
			scrollData->scrollOrigin = V2_Zero;
			scrollData->momentumTime = 0;
		}
		
		bool scrollMomentumOccurring = (scrollData->scrollMomentum.X != 0 || scrollData->scrollMomentum.Y != 0);
		if (scrollMomentumOccurring) { isAutoScrollingOccurring = true; }

		// Apply existing momentum
		scrollData->scrollTarget.X += scrollData->scrollMomentum.X;
		scrollData->scrollMomentum.X *= 0.95f;
		bool scrollOccurred = scrollDelta.X != 0 || scrollDelta.Y != 0;
		if ((scrollData->scrollMomentum.X > -0.1f && scrollData->scrollMomentum.X < 0.1f) || scrollOccurred)
		{
			scrollData->scrollMomentum.X = 0;
		}
		scrollData->scrollTarget.X = MinR32(MaxR32(scrollData->scrollTarget.X, -(MaxR32(scrollData->contentSize.Width - scrollData->layoutElement->dimensions.Width, 0))), 0);

		scrollData->scrollTarget.Y += scrollData->scrollMomentum.Y;
		scrollData->scrollMomentum.Y *= 0.95f;
		if ((scrollData->scrollMomentum.Y > -0.1f && scrollData->scrollMomentum.Y < 0.1f) || scrollOccurred)
		{
			scrollData->scrollMomentum.Y = 0;
		}
		scrollData->scrollTarget.Y = MinR32(MaxR32(scrollData->scrollTarget.Y, -(MaxR32(scrollData->contentSize.Height - scrollData->layoutElement->dimensions.Height, 0))), 0);
		
		// Update scrollPosition to scrollTarget with scrollLag taken into account
		if (scrollData->scrollLag == 0 || scrollMomentumOccurring || isPointerActive)
		{
			scrollData->scrollPosition = scrollData->scrollTarget;
		}
		else
		{
			v2 targetDelta = NewV2(
				scrollData->scrollTarget.X - scrollData->scrollPosition.X,
				scrollData->scrollTarget.Y - scrollData->scrollPosition.Y
			);
			r32 targetDistanceSquared = (targetDelta.X * targetDelta.X) + (targetDelta.Y * targetDelta.Y);
			if (targetDistanceSquared >= 1.0f)
			{
				//TODO: We should do the proper framerate independent calculation here!
				scrollData->scrollPosition.X += targetDelta.X / scrollData->scrollLag;
				scrollData->scrollPosition.Y += targetDelta.Y / scrollData->scrollLag;
				isAutoScrollingOccurring = true;
			}
			else
			{
				scrollData->scrollPosition = scrollData->scrollTarget;
			}
		}
		
		for (int32_t j = 0; j < context->pointerOverIds.length; ++j) // TODO n & m are small here but this being n*m gives me the creeps
		{
			if (scrollData->layoutElement->id == Clay__ElementIdArray_Get(&context->pointerOverIds, j)->id)
			{
				highestPriorityElementIndex = j;
				highestPriorityScrollData = scrollData;
			}
		}
	}

	if (highestPriorityElementIndex > -1 && highestPriorityScrollData)
	{
		Clay_LayoutElement *scrollElement = highestPriorityScrollData->layoutElement;
		Clay_ScrollElementConfig *scrollConfig = Clay__FindElementConfigWithType(scrollElement, CLAY__ELEMENT_CONFIG_TYPE_SCROLL).scrollElementConfig;
		highestPriorityScrollData->scrollLag = scrollConfig->scrollLag;
		bool canScrollVertically = scrollConfig->vertical && highestPriorityScrollData->contentSize.Height > scrollElement->dimensions.Height;
		bool canScrollHorizontally = scrollConfig->horizontal && highestPriorityScrollData->contentSize.Width > scrollElement->dimensions.Width;
		// Handle wheel scroll
		if (canScrollVertically)
		{
			highestPriorityScrollData->scrollTarget.Y = highestPriorityScrollData->scrollTarget.Y + scrollDelta.Y * 10;
		}
		if (canScrollHorizontally)
		{
			highestPriorityScrollData->scrollTarget.X = highestPriorityScrollData->scrollTarget.X + scrollDelta.X * 10;
		}
		// Handle click / touch scroll
		if (isPointerActive)
		{
			highestPriorityScrollData->scrollMomentum = V2_Zero;
			if (!highestPriorityScrollData->pointerScrollActive)
			{
				highestPriorityScrollData->pointerOrigin = context->pointerInfo.position;
				highestPriorityScrollData->scrollOrigin = highestPriorityScrollData->scrollTarget;
				highestPriorityScrollData->pointerScrollActive = true;
			}
			else
			{
				float scrollDeltaX = 0, scrollDeltaY = 0;
				if (canScrollHorizontally)
				{
					float oldXScrollPosition = highestPriorityScrollData->scrollTarget.X;
					highestPriorityScrollData->scrollTarget.X = highestPriorityScrollData->scrollOrigin.X + (context->pointerInfo.position.X - highestPriorityScrollData->pointerOrigin.X);
					highestPriorityScrollData->scrollTarget.X = MaxR32(MinR32(highestPriorityScrollData->scrollTarget.X, 0), -(highestPriorityScrollData->contentSize.Width - highestPriorityScrollData->boundingBox.Width));
					scrollDeltaX = highestPriorityScrollData->scrollTarget.X - oldXScrollPosition;
				}
				if (canScrollVertically)
				{
					float oldYScrollPosition = highestPriorityScrollData->scrollTarget.Y;
					highestPriorityScrollData->scrollTarget.Y = highestPriorityScrollData->scrollOrigin.Y + (context->pointerInfo.position.Y - highestPriorityScrollData->pointerOrigin.Y);
					highestPriorityScrollData->scrollTarget.Y = MaxR32(MinR32(highestPriorityScrollData->scrollTarget.Y, 0), -(highestPriorityScrollData->contentSize.Height - highestPriorityScrollData->boundingBox.Height));
					scrollDeltaY = highestPriorityScrollData->scrollTarget.Y - oldYScrollPosition;
				}
				if (scrollDeltaX > -0.1f && scrollDeltaX < 0.1f && scrollDeltaY > -0.1f && scrollDeltaY < 0.1f && highestPriorityScrollData->momentumTime > 0.15f)
				{
					highestPriorityScrollData->momentumTime = 0;
					highestPriorityScrollData->pointerOrigin = context->pointerInfo.position;
					highestPriorityScrollData->scrollOrigin = highestPriorityScrollData->scrollTarget;
				}
				else
				{
					 highestPriorityScrollData->momentumTime += deltaTime;
				}
			}
		}
		// Clamp any changes to scroll position to the maximum size of the contents
		if (canScrollVertically)
		{
			highestPriorityScrollData->scrollPosition.Y = MaxR32(MinR32(highestPriorityScrollData->scrollPosition.Y, 0), -(highestPriorityScrollData->contentSize.Height - scrollElement->dimensions.Height));
			highestPriorityScrollData->scrollTarget.Y = MaxR32(MinR32(highestPriorityScrollData->scrollTarget.Y, 0), -(highestPriorityScrollData->contentSize.Height - scrollElement->dimensions.Height));
		}
		if (canScrollHorizontally)
		{
			highestPriorityScrollData->scrollPosition.X = MaxR32(MinR32(highestPriorityScrollData->scrollPosition.X, 0), -(highestPriorityScrollData->contentSize.Width - scrollElement->dimensions.Width));
			highestPriorityScrollData->scrollTarget.X = MaxR32(MinR32(highestPriorityScrollData->scrollTarget.X, 0), -(highestPriorityScrollData->contentSize.Width - scrollElement->dimensions.Width));
		}
		
		//If no scrollLag, or currently scrolling with touch, immediately move scrollPosition to scrollTarget rather than waiting for next frame
		if (highestPriorityScrollData->scrollLag == 0 || isPointerActive)
		{
			highestPriorityScrollData->scrollPosition = highestPriorityScrollData->scrollTarget;
		}
	}
	return isAutoScrollingOccurring;
}

CLAY_WASM_EXPORT("Clay_BeginLayout")
CLAY_DECOR void Clay_BeginLayout(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	Clay__InitializeEphemeralMemory(context);
	context->generation++;
	context->dynamicElementIndex = 0;
	// Set up the root container that covers the entire window
	v2 rootDimensions = NewV2(context->layoutDimensions.Width, context->layoutDimensions.Height);
	if (context->debugModeEnabled) { rootDimensions.Width -= (float)Clay__debugViewWidth; }
	context->booleanWarnings = NEW_STRUCT(Clay_BooleanWarnings) ZEROED;
	Clay__OpenElement();
	Clay__ConfigureOpenElement(NEW_STRUCT(Clay_ElementDeclaration) {
		.id = CLAY_ID("Clay__RootContainer"),
		.layout = { .sizing = {CLAY_SIZING_FIXED((rootDimensions.Width)), CLAY_SIZING_FIXED(rootDimensions.Height)} }
	});
	Clay__int32_tArray_Add(&context->openLayoutElementStack, 0);
	Clay__LayoutElementTreeRootArray_Add(&context->layoutElementTreeRoots, NEW_STRUCT(Clay__LayoutElementTreeRoot) { .layoutElementIndex = 0 });
}

CLAY_WASM_EXPORT("Clay_EndLayout")
CLAY_DECOR Clay_RenderCommandArray Clay_EndLayout(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	Clay__CloseElement();
	bool elementsExceededBeforeDebugView = context->booleanWarnings.maxElementsExceeded;
	if (context->debugModeEnabled && !elementsExceededBeforeDebugView)
	{
		context->warningsEnabled = false;
		Clay__RenderDebugView();
		context->warningsEnabled = true;
	}
	if (context->booleanWarnings.maxElementsExceeded)
	{
		Str8 message;
		if (!elementsExceededBeforeDebugView)
		{
			message = StrLit("Clay Error: Layout elements exceeded Clay__maxElementCount after adding the debug-view to the layout.");
		}
		else
		{
			message = StrLit("Clay Error: Layout elements exceeded Clay__maxElementCount");
		}
		Clay__AddRenderCommand(NEW_STRUCT(Clay_RenderCommand ) {
			.boundingBox = { context->layoutDimensions.Width / 2 - 59 * 4, context->layoutDimensions.Height / 2, 0, 0 },
			.renderData = { .text = { .stringContents = message, .textColor = {.valueU32=0xFFFF0000}, .fontSize = 16 } }, //(255, 0, 0, 255)
			.commandType = CLAY_RENDER_COMMAND_TYPE_TEXT
		});
	}
	else
	{
		Clay__CalculateFinalLayout();
	}
	return context->renderCommands;
}

CLAY_WASM_EXPORT("Clay_GetElementId")
CLAY_DECOR Clay_ElementId Clay_GetElementId(Str8 idString)
{
	return Clay__HashString(idString, 0, 0);
}

CLAY_WASM_EXPORT("Clay_GetElementIdWithIndex")
CLAY_DECOR Clay_ElementId Clay_GetElementIdWithIndex(Str8 idString, uint32_t index)
{
	return Clay__HashString(idString, index, 0);
}

CLAY_DECOR bool Clay_Hovered(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->booleanWarnings.maxElementsExceeded) { return false; }
	Clay_LayoutElement *openLayoutElement = Clay__GetOpenLayoutElement();
	// If the element has no id attached at this point, we need to generate one
	if (openLayoutElement->id == 0)
	{
		Clay__GenerateIdForAnonymousElement(openLayoutElement);
	}
	for (int32_t i = 0; i < context->pointerOverIds.length; ++i)
	{
		if (Clay__ElementIdArray_Get(&context->pointerOverIds, i)->id == openLayoutElement->id)
		{
			return true;
		}
	}
	return false;
}

CLAY_DECOR void Clay_OnHover(void (*onHoverFunction)(Clay_ElementId elementId, Clay_PointerData pointerInfo, CLAY_ONHOVER_USERDATA_TYPE userData), CLAY_ONHOVER_USERDATA_TYPE userData)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context->booleanWarnings.maxElementsExceeded) { return; }
	Clay_LayoutElement *openLayoutElement = Clay__GetOpenLayoutElement();
	if (openLayoutElement->id == 0) { Clay__GenerateIdForAnonymousElement(openLayoutElement); }
	Clay_LayoutElementHashMapItem *hashMapItem = Clay__GetHashMapItem(openLayoutElement->id);
	hashMapItem->onHoverFunction = onHoverFunction;
	hashMapItem->hoverFunctionUserData = userData;
}

CLAY_WASM_EXPORT("Clay_PointerOver")
CLAY_DECOR bool Clay_PointerOver(Clay_ElementId elementId) // TODO return priority for separating multiple results
{
	Clay_Context* context = Clay_GetCurrentContext();
	for (int32_t i = 0; i < context->pointerOverIds.length; ++i)
	{
		if (Clay__ElementIdArray_Get(&context->pointerOverIds, i)->id == elementId.id)
		{
			return true;
		}
	}
	return false;
}

CLAY_WASM_EXPORT("Clay_GetScrollContainerData")
CLAY_DECOR Clay_ScrollContainerData Clay_GetScrollContainerData(Clay_ElementId id)
{
	Clay_Context* context = Clay_GetCurrentContext();
	for (int32_t i = 0; i < context->scrollContainerDatas.length; ++i)
	{
		Clay__ScrollContainerDataInternal *scrollContainerData = Clay__ScrollContainerDataInternalArray_Get(&context->scrollContainerDatas, i);
		if (scrollContainerData->elementId == id.id)
		{
			Clay_ScrollElementConfig* scrollConfig = Clay__FindElementConfigWithType(scrollContainerData->layoutElement, CLAY__ELEMENT_CONFIG_TYPE_SCROLL).scrollElementConfig;
			if (scrollConfig != nullptr)
			{
				return NEW_STRUCT(Clay_ScrollContainerData) {
					.scrollTarget = &scrollContainerData->scrollTarget,
					.scrollPosition = &scrollContainerData->scrollPosition,
					.scrollContainerDimensions = { scrollContainerData->boundingBox.Width, scrollContainerData->boundingBox.Height },
					.contentDimensions = scrollContainerData->contentSize,
					.config = *scrollConfig,
					.found = true
				};
			}
		}
	}
	return NEW_STRUCT(Clay_ScrollContainerData) ZEROED;
}

CLAY_WASM_EXPORT("Clay_GetElementData")
CLAY_DECOR Clay_ElementData Clay_GetElementData(Clay_ElementId id)
{
	Clay_LayoutElementHashMapItem * item = Clay__GetHashMapItem(id.id);
	if(item == &Clay_LayoutElementHashMapItem_DEFAULT)
	{
		return NEW_STRUCT(Clay_ElementData) ZEROED;
	}

	return NEW_STRUCT(Clay_ElementData){
		.boundingBox = item->boundingBox,
		.found = true
	};
}

CLAY_WASM_EXPORT("Clay_SetDebugModeEnabled")
CLAY_DECOR void Clay_SetDebugModeEnabled(bool enabled)
{
	Clay_Context* context = Clay_GetCurrentContext();
	context->debugModeEnabled = enabled;
}

CLAY_WASM_EXPORT("Clay_IsDebugModeEnabled")
CLAY_DECOR bool Clay_IsDebugModeEnabled(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	return context->debugModeEnabled;
}

CLAY_WASM_EXPORT("Clay_SetCullingEnabled")
CLAY_DECOR void Clay_SetCullingEnabled(bool enabled)
{
	Clay_Context* context = Clay_GetCurrentContext();
	context->disableCulling = !enabled;
}

CLAY_WASM_EXPORT("Clay_SetExternalScrollHandlingEnabled")
void Clay_SetExternalScrollHandlingEnabled(bool enabled)
{
	Clay_Context* context = Clay_GetCurrentContext();
	context->externalScrollHandlingEnabled = enabled;
}

CLAY_WASM_EXPORT("Clay_GetMaxElementCount")
CLAY_DECOR int32_t Clay_GetMaxElementCount(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	return context->maxElementCount;
}

CLAY_WASM_EXPORT("Clay_SetMaxElementCount")
CLAY_DECOR void Clay_SetMaxElementCount(int32_t maxElementCount)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context)
	{
		context->maxElementCount = maxElementCount;
	}
	else
	{
		Clay__defaultMaxElementCount = maxElementCount; // TODO: Fix this
		Clay__defaultMaxMeasureTextWordCacheCount = maxElementCount * 2;
	}
}

CLAY_WASM_EXPORT("Clay_GetMaxMeasureTextCacheWordCount")
CLAY_DECOR int32_t Clay_GetMaxMeasureTextCacheWordCount(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	return context->maxMeasureTextCacheWordCount;
}

CLAY_WASM_EXPORT("Clay_SetMaxMeasureTextCacheWordCount")
CLAY_DECOR void Clay_SetMaxMeasureTextCacheWordCount(int32_t maxMeasureTextCacheWordCount)
{
	Clay_Context* context = Clay_GetCurrentContext();
	if (context)
	{
		Clay__currentContext->maxMeasureTextCacheWordCount = maxMeasureTextCacheWordCount;
	}
	else
	{
		Clay__defaultMaxMeasureTextWordCacheCount = maxMeasureTextCacheWordCount; // TODO: Fix this
	}
}

CLAY_WASM_EXPORT("Clay_ResetMeasureTextCache")
CLAY_DECOR void Clay_ResetMeasureTextCache(void)
{
	Clay_Context* context = Clay_GetCurrentContext();
	context->measureTextHashMapInternal.length = 0;
	context->measureTextHashMapInternalFreeList.length = 0;
	context->measureTextHashMap.length = 0;
	context->measuredWords.length = 0;
	context->measuredWordsFreeList.length = 0;
	
	for (int32_t i = 0; i < context->measureTextHashMap.allocLength; ++i)
	{
		context->measureTextHashMap.items[i] = 0;
	}
	context->measureTextHashMapInternal.length = 1; // Reserve the 0 value to mean "no next element"
}

#endif // CLAY_IMPLEMENTATION

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
