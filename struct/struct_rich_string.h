/*
File:   struct_rich_string.h
Author: Taylor Robbins
Date:   03\16\2025
Description:
	** A "rich" string is really just an array of Str8 pieces with
	** font styling options for each piece. This allows us to have
	** a section of bold characters embedded in a string of non-bold
	** characters. Or we can have different colors for different
	** portions of the text.
*/

#ifndef _STRUCT_RICH_STRING_H
#define _STRUCT_RICH_STRING_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "std/std_memset.h"
#include "struct/struct_string.h"
#include "struct/struct_color.h"
#include "mem/mem_arena.h"

typedef enum FontStyleFlag FontStyleFlag;
enum FontStyleFlag
{
	FontStyleFlag_None          = 0x00,
	FontStyleFlag_Bold          = 0x01,
	FontStyleFlag_Italic        = 0x02,
	FontStyleFlag_Inverted      = 0x04,
	FontStyleFlag_Underline     = 0x08,
	FontStyleFlag_Strikethrough = 0x10,
	FontStyleFlag_Outline       = 0x20,
	FontStyleFlag_Highlighted   = 0x40,
	FontStyleFlag_All           = 0x7F,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetFontStyleFlagStr(FontStyleFlag enumValue);
#else
const char* GetFontStyleFlagStr(FontStyleFlag enumValue)
{
	switch (enumValue)
	{
		case FontStyleFlag_Bold:          return "Bold";
		case FontStyleFlag_Italic:        return "Italic";
		case FontStyleFlag_Inverted:      return "Inverted";
		case FontStyleFlag_Underline:     return "Underline";
		case FontStyleFlag_Strikethrough: return "Strikethrough";
		case FontStyleFlag_Outline:       return "Outline";
		case FontStyleFlag_Highlighted:   return "Highlighted";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef enum RichStrStyleChangeType RichStrStyleChangeType;
enum RichStrStyleChangeType
{
	RichStrStyleChangeType_None = 0,
	RichStrStyleChangeType_FontSize,
	RichStrStyleChangeType_FontStyle,
	RichStrStyleChangeType_Color,
	RichStrStyleChangeType_Count,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetRichStrStyleChangeTypeStr(RichStrStyleChangeType enumValue);
#else
const char* GetRichStrStyleChangeTypeStr(RichStrStyleChangeType enumValue)
{
	switch (enumValue)
	{
		case RichStrStyleChangeType_None:      return "None";
		case RichStrStyleChangeType_FontSize:  return "FontSize";
		case RichStrStyleChangeType_FontStyle: return "FontStyle";
		case RichStrStyleChangeType_Color:     return "Color";
		case RichStrStyleChangeType_Count:     return "Count";
		default: return "Unknown";
	}
}
#endif

#define RICH_STYLE_DEFAULT_COLOR_VALUE  0x00FFFFFF
#define RICH_STYLE_DEFAULT_COLOR  NewColorU32(RICH_STYLE_DEFAULT_COLOR_VALUE)

typedef struct RichStrStyle RichStrStyle;
struct RichStrStyle
{
	r32 fontSize;
	u8 fontStyle;
	Color32 color;
};

// Each piece has a set of changes to the style, not a style directly
typedef struct RichStrStyleChange RichStrStyleChange;
struct RichStrStyleChange
{
	RichStrStyleChangeType type;
	r32 fontSize;  // (0.0f means default size)
	u8 enableStyleFlags;
	u8 disableStyleFlags;
	u8 defaultStyleFlags;
	Color32 color; // (TransparentWhite means default color)
};

typedef struct RichStrPiece RichStrPiece;
struct RichStrPiece
{
	RichStrStyleChange styleChange;
	Str8 str;
};

//NOTE: When a RichStr has only one piece we store it directly in RichStr structure in fullPiece,
// otherwise the pieces are allocated separately but the str in each piece is just a slice of fullPiece.str
typedef struct RichStr RichStr;
struct RichStr
{
	RichStrPiece fullPiece;
	uxx numPieces;
	RichStrPiece* pieces;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE RichStrStyle NewRichStrStyle(r32 fontSize, u8 fontStyle, Color32 color);
	PIG_CORE_INLINE RichStrStyleChange NewRichStrStyleChange(RichStrStyleChangeType type, r32 fontSize, u8 enableStyleFlags, u8 disableStyleFlags, u8 defaultStyleFlags, Color32 color);
	PIG_CORE_INLINE RichStrStyleChange NewRichStrStyleChangeSize(r32 fontSize);
	PIG_CORE_INLINE RichStrStyleChange NewRichStrStyleChangeEnableFlags(u8 enableStyleFlags);
	PIG_CORE_INLINE RichStrStyleChange NewRichStrStyleChangeDisableFlags(u8 disableStyleFlags);
	PIG_CORE_INLINE RichStrStyleChange NewRichStrStyleChangeDefaultFlags(u8 defaultStyleFlags);
	PIG_CORE_INLINE RichStrStyleChange NewRichStrStyleChangeColor(Color32 color);
	PIG_CORE_INLINE RichStr ToRichStrEx(Str8 string, RichStrStyleChange styleChange);
	PIG_CORE_INLINE RichStr ToRichStr(Str8 string);
	PIG_CORE_INLINE void FreeRichStr(Arena* arena, RichStr* richStrPntr);
	PIG_CORE_INLINE RichStr NewRichStr(Arena* arena, uxx numPieces, const RichStrPiece* pieces);
	PIG_CORE_INLINE RichStr AllocRichStr(Arena* arena, RichStr richStr);
	PIG_CORE_INLINE RichStrPiece* GetRichStrPiece(RichStr* richStr, uxx pieceIndex);
	RichStr RichStrSlice(Arena* arena, RichStr baseString, uxx startIndex, uxx endIndex);
	PIG_CORE_INLINE RichStr RichStrSliceLength(Arena* arena, RichStr baseString, uxx startIndex, uxx length);
	PIG_CORE_INLINE RichStr RichStrSliceFrom(Arena* arena, RichStr baseString, uxx startIndex);
	PIG_CORE_INLINE void ApplyRichStyleChange(RichStrStyle* style, RichStrStyleChange change, r32 defaultFontSize, u8 defaultFontStyle, Color32 defaultColor);
#endif

#define RichStrStyleChange_None NewRichStrStyleChange(RichStrStyleChangeType_None, 0.0f, 0x00, 0x00, 0x00, RICH_STYLE_DEFAULT_COLOR)
#define RichStrStyleChange_None_Const (RichStrStyleChange){ .type=RichStrStyleChangeType_None, .fontSize=0.0f, .enableStyleFlags=0x00, .disableStyleFlags=0x00, .defaultStyleFlags=0x00, .color={.valueU32=RICH_STYLE_DEFAULT_COLOR_VALUE} }

#define RichStr_Empty NewRichStr(nullptr, 0, nullptr)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI RichStrStyle NewRichStrStyle(r32 fontSize, u8 fontStyle, Color32 color)
{
	RichStrStyle result = ZEROED;
	result.fontSize = fontSize;
	result.fontStyle = fontStyle;
	result.color = color;
	return result;
}
PEXPI RichStrStyleChange NewRichStrStyleChange(RichStrStyleChangeType type, r32 fontSize, u8 enableStyleFlags, u8 disableStyleFlags, u8 defaultStyleFlags, Color32 color)
{
	RichStrStyleChange result = ZEROED;
	result.type = type;
	result.fontSize = fontSize;
	result.enableStyleFlags = enableStyleFlags;
	result.disableStyleFlags = disableStyleFlags;
	result.defaultStyleFlags = defaultStyleFlags;
	result.color = color;
	return result;
}
PEXPI RichStrStyleChange NewRichStrStyleChangeSize(r32 fontSize)
{
	return NewRichStrStyleChange(RichStrStyleChangeType_FontSize, fontSize, 0x00, 0x00, 0x00, RICH_STYLE_DEFAULT_COLOR);
}
PEXPI RichStrStyleChange NewRichStrStyleChangeEnableFlags(u8 enableStyleFlags)
{
	return NewRichStrStyleChange(RichStrStyleChangeType_FontStyle, 0.0f, enableStyleFlags, 0x00, 0x00, RICH_STYLE_DEFAULT_COLOR);
}
PEXPI RichStrStyleChange NewRichStrStyleChangeDisableFlags(u8 disableStyleFlags)
{
	return NewRichStrStyleChange(RichStrStyleChangeType_FontStyle, 0.0f, 0x00, disableStyleFlags, 0x00, RICH_STYLE_DEFAULT_COLOR);
}
PEXPI RichStrStyleChange NewRichStrStyleChangeDefaultFlags(u8 defaultStyleFlags)
{
	return NewRichStrStyleChange(RichStrStyleChangeType_FontStyle, 0.0f, 0x00, 0x00, defaultStyleFlags, RICH_STYLE_DEFAULT_COLOR);
}
PEXPI RichStrStyleChange NewRichStrStyleChangeColor(Color32 color)
{
	return NewRichStrStyleChange(RichStrStyleChangeType_Color, 0.0f, 0x00, 0x00, 0x00, color);
}

PEXPI RichStr ToRichStrEx(Str8 string, RichStrStyleChange styleChange)
{
	RichStr result = ZEROED;
	result.fullPiece.styleChange = styleChange;
	result.fullPiece.str = string;
	result.numPieces = IsEmptyStr(string) ? 0 : 1;
	return result;
}
PEXPI RichStr ToRichStr(Str8 string)
{
	return ToRichStrEx(string, RichStrStyleChange_None_Const);
}

PEXPI void FreeRichStr(Arena* arena, RichStr* richStrPntr)
{
	NotNull(richStrPntr);
	if (richStrPntr->fullPiece.str.chars != nullptr)
	{
		NotNull(arena);
		FreeStr8(arena, &richStrPntr->fullPiece.str);
		if (richStrPntr->numPieces > 1)
		{
			FreeArray(RichStrPiece, arena, richStrPntr->numPieces, richStrPntr->pieces);
		}
		else { Assert(richStrPntr->pieces == nullptr); }
	}
	else { Assert(richStrPntr->pieces == nullptr); }
	ClearPointer(richStrPntr);
}

PEXPI RichStr NewRichStr(Arena* arena, uxx numPieces, const RichStrPiece* pieces)
{
	if (numPieces == 0) { return ToRichStrEx(Str8_Empty_Const, RichStrStyleChange_None_Const); }
	NotNull(arena);
	NotNull(pieces);
	if (numPieces == 1) { return ToRichStrEx(AllocStr8(arena, pieces[0].str), pieces[0].styleChange); }
	
	uxx bIndex = 0;
	RichStr result = ZEROED;
	
	result.fullPiece.str.length = 0;
	for (uxx pIndex = 0; pIndex < numPieces; pIndex++)
	{
		const RichStrPiece* inPiece = &pieces[pIndex];
		NotNullStr(inPiece->str);
		result.fullPiece.str.length += inPiece->str.length;
	}
	
	if (result.fullPiece.str.length > 0)
	{
		result.fullPiece.str.chars = (char*)AllocMem(arena, result.fullPiece.str.length);
		NotNull(result.fullPiece.str.chars);
		bIndex = 0;
		for (uxx pIndex = 0; pIndex < numPieces; pIndex++)
		{
			const RichStrPiece* inPiece = &pieces[pIndex];
			if (inPiece->str.length > 0)
			{
				MyMemCopy(&result.fullPiece.str.chars[bIndex], inPiece->str.chars, inPiece->str.length);
				bIndex += inPiece->str.length;
			}
		}
		DebugAssert(bIndex == result.fullPiece.str.length);
	}
	
	result.pieces = AllocArray(RichStrPiece, arena, numPieces);
	NotNull(result.pieces);
	
	bIndex = 0;
	for (uxx pIndex = 0; pIndex < numPieces; pIndex++)
	{
		const RichStrPiece* inPiece = &pieces[pIndex];
		RichStrPiece* newPiece = &result.pieces[pIndex];
		newPiece->styleChange = inPiece->styleChange;
		newPiece->str = NewStr8(inPiece->str.length, &result.fullPiece.str.chars[bIndex]);
		bIndex += inPiece->str.length;
	}
	
	return result;
}

PEXPI RichStrPiece* GetRichStrPiece(RichStr* richStr, uxx pieceIndex)
{
	NotNull(richStr);
	if (pieceIndex >= richStr->numPieces) { return nullptr; }
	if (richStr->numPieces > 1)
	{
		NotNull(richStr->pieces);
		return &richStr->pieces[pieceIndex];
	}
	else { return &richStr->fullPiece; }
}

PEXPI RichStr AllocRichStr(Arena* arena, RichStr richStr)
{
	return NewRichStr(arena, richStr.numPieces, (richStr.numPieces > 1) ? richStr.pieces : &richStr.fullPiece);
}

PEXP RichStr RichStrSlice(Arena* arena, RichStr baseString, uxx startIndex, uxx endIndex)
{
	NotNull(arena);
	Assert(endIndex >= startIndex);
	Assert(startIndex <= baseString.fullPiece.str.length);
	Assert(endIndex <= baseString.fullPiece.str.length);
	if (baseString.numPieces == 0) { return AllocRichStr(arena, baseString); }
	else if (baseString.numPieces == 1)
	{
		RichStrPiece slicePiece = ZEROED;
		slicePiece.styleChange = baseString.pieces[0].styleChange;
		slicePiece.str = StrSlice(baseString.fullPiece.str, startIndex, endIndex);
		return NewRichStr(arena, 1, &slicePiece);
	}
	else
	{
		NotNull(baseString.pieces);
		
		uxx firstPieceIndex = baseString.numPieces;
		uxx firstPieceByteOffset = 0;
		if (startIndex == baseString.fullPiece.str.length) { firstPieceIndex = baseString.numPieces-1; firstPieceByteOffset = GetRichStrPiece(&baseString, baseString.numPieces-1)->str.length; }
		else if (startIndex == baseString.fullPiece.str.length) { firstPieceIndex = 0; firstPieceByteOffset = 0; }
		else
		{
			uxx bIndex = 0;
			for (uxx pIndex = 0; pIndex < baseString.numPieces; pIndex++)
			{
				const RichStrPiece* piece = &baseString.pieces[pIndex];
				if (bIndex + piece->str.length > startIndex)
				{
					firstPieceIndex = pIndex;
					firstPieceByteOffset = startIndex - bIndex;
					break;
				}
				bIndex += piece->str.length;
			}
		}
		AssertMsg(firstPieceIndex < baseString.numPieces, "Pieces did not add up to full string in RichStrSlice!");
		
		RichStr result = ZEROED;
		result.numPieces = baseString.numPieces - firstPieceIndex;
		result.pieces = AllocArray(RichStrPiece, arena, result.numPieces);
		NotNull(result.pieces);
		result.fullPiece = baseString.fullPiece;
		result.fullPiece.str = AllocStr8(arena, StrSlice(baseString.fullPiece.str, startIndex, endIndex));
		
		uxx numBytesInPieces = 0;
		for (uxx pIndex = 0; pIndex < result.numPieces; pIndex++)
		{
			const RichStrPiece* basePiece = &baseString.pieces[firstPieceIndex + pIndex];
			RichStrPiece* piece = &result.pieces[pIndex];
			MyMemCopy(piece, basePiece, sizeof(RichStrPiece));
			if (pIndex == 0)
			{
				piece->str = StrSliceFrom(piece->str, firstPieceByteOffset);
			}
			if (numBytesInPieces + piece->str.length > result.fullPiece.str.length)
			{
				piece->str = StrSlice(piece->str, 0, result.fullPiece.str.length - numBytesInPieces);
			}
			piece->str = AllocStr8(arena, piece->str);
			numBytesInPieces += piece->str.length;
			Assert(numBytesInPieces <= result.fullPiece.str.length);
		}
		Assert(numBytesInPieces == result.fullPiece.str.length);
		
		return result;
	}
}
PEXPI RichStr RichStrSliceLength(Arena* arena, RichStr baseString, uxx startIndex, uxx length)
{
	Assert(startIndex <= baseString.fullPiece.str.length);
	Assert(startIndex + length <= baseString.fullPiece.str.length);
	return RichStrSlice(arena, baseString, startIndex, startIndex + length);
}
PEXPI RichStr RichStrSliceFrom(Arena* arena, RichStr baseString, uxx startIndex)
{
	Assert(startIndex <= baseString.fullPiece.str.length);
	return RichStrSlice(arena, baseString, startIndex, baseString.fullPiece.str.length);
}

PEXPI void ApplyRichStyleChange(RichStrStyle* style, RichStrStyleChange styleChange, r32 defaultFontSize, u8 defaultFontStyle, Color32 defaultColor)
{
	switch (styleChange.type)
	{
		case RichStrStyleChangeType_FontSize: style->fontSize = (styleChange.fontSize != 0.0f) ? styleChange.fontSize : defaultFontSize; break;
		case RichStrStyleChangeType_FontStyle:
		{
			FlagSet(style->fontStyle, styleChange.enableStyleFlags);
			FlagUnset(style->fontStyle, styleChange.disableStyleFlags);
			style->fontStyle = (style->fontStyle & (~styleChange.defaultStyleFlags)) | (defaultFontStyle & styleChange.defaultStyleFlags);
		} break;
		case RichStrStyleChangeType_Color: style->color = (styleChange.color.valueU32 != RICH_STYLE_DEFAULT_COLOR_VALUE) ? styleChange.color : defaultColor; break;
	}
}

typedef struct RichStrParseState RichStrParseState;
struct RichStrParseState
{
	RichStr result;
	RichStrStyleChange nextStyleChange;
	Str8 encodedString;
	uxx pieceStartIndex;
	uxx fullStrByteIndex;
	u8 utf8ByteSize;
	uxx pieceIndex;
	u8 enabledFlags;
};

static void TwoPassRichStrPiece(RichStrParseState* state, u8 pass, uxx bIndex)
{
	Str8 pieceStr = StrSlice(state->encodedString, state->pieceStartIndex, bIndex);
	if (pass == 1)
	{
		DebugAssert(state->pieceIndex < state->result.numPieces);
		RichStrPiece* piece = GetRichStrPiece(&state->result, state->pieceIndex);
		piece->str = StrSliceLength(state->result.fullPiece.str, state->fullStrByteIndex, pieceStr.length);
		MyMemCopy(piece->str.chars, pieceStr.chars, pieceStr.length);
		piece->styleChange = state->nextStyleChange;
	}
	state->fullStrByteIndex += pieceStr.length;
	state->pieceStartIndex = bIndex + state->utf8ByteSize;
	state->pieceIndex++;
}

//TODO: If the string starts out with a style change sequence then the first piece shouldn't be an empty string with no style changes
PEXP RichStr DecodeStrToRichStr(Arena* arena, Str8 encodedString)
{
	RichStrParseState state = ZEROED;
	state.encodedString = encodedString;
	for (u8 pass = 0; pass < 2; pass++)
	{
		state.nextStyleChange = RichStrStyleChange_None_Const;
		state.pieceStartIndex = 0;
		state.fullStrByteIndex = 0;
		state.pieceIndex = 0;
		state.enabledFlags = 0x00;
		for (uxx bIndex = 0; bIndex < encodedString.length; bIndex++)
		{
			u32 codepoint = 0;
			state.utf8ByteSize = GetCodepointForUtf8Str(encodedString, bIndex, &codepoint);
			if (state.utf8ByteSize == 0)
			{
				//TODO: Should we somehow let the calling code know we have invalid UTF-8?
				state.utf8ByteSize = 1;
				codepoint = CharToU32(encodedString.chars[bIndex]);
			}
			
			if (codepoint == '\b')
			{
				TwoPassRichStrPiece(&state, pass, bIndex);
				if (IsFlagSet(state.enabledFlags, FontStyleFlag_Bold))
				{
					state.nextStyleChange = NewRichStrStyleChangeDefaultFlags(FontStyleFlag_Bold);
					FlagUnset(state.enabledFlags, FontStyleFlag_Bold);
				}
				else
				{
					state.nextStyleChange = NewRichStrStyleChangeEnableFlags(FontStyleFlag_Bold);
					FlagSet(state.enabledFlags, FontStyleFlag_Bold);
				}
			}
			else if (codepoint == '\a')
			{
				TwoPassRichStrPiece(&state, pass, bIndex);
				if (IsFlagSet(state.enabledFlags, FontStyleFlag_Italic))
				{
					state.nextStyleChange = NewRichStrStyleChangeDefaultFlags(FontStyleFlag_Italic);
					FlagUnset(state.enabledFlags, FontStyleFlag_Italic);
				}
				else
				{
					state.nextStyleChange = NewRichStrStyleChangeEnableFlags(FontStyleFlag_Italic);
					FlagSet(state.enabledFlags, FontStyleFlag_Italic);
				}
			}
			
			if (state.utf8ByteSize > 1) { bIndex += state.utf8ByteSize-1; }
		}
		if (state.pieceStartIndex < encodedString.length)
		{
			TwoPassRichStrPiece(&state, pass, encodedString.length);
		}
		
		if (pass == 0)
		{
			if (state.pieceIndex <= 1)
			{
				state.result = AllocRichStr(arena, ToRichStr(encodedString));
				break;
			}
			Assert(state.fullStrByteIndex > 0);
			state.result.fullPiece.str.length = state.fullStrByteIndex;
			state.result.fullPiece.str.chars = (char*)AllocMem(arena, state.result.fullPiece.str.length);
			NotNull(state.result.fullPiece.str.chars);
			state.result.numPieces = state.pieceIndex;
			state.result.pieces = AllocArray(RichStrPiece, arena, state.result.numPieces);
			NotNull(state.result.pieces);
		}
		else
		{
			DebugAssert(state.result.fullPiece.str.length == state.fullStrByteIndex);
			DebugAssert(state.result.numPieces == state.pieceIndex);
		}
	}
	return state.result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_RICH_STRING_H
