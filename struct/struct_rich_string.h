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
	
	** NOTE: Upon revisiting RichStr when implementing EncodeRichStr I found the
	** design of this system somewhat confusing. Either we should document the
	** reasons for the design choices and tradeoffs or we should consider rewriting
	** this system to make it simpler.
*/

#ifndef _STRUCT_RICH_STRING_H
#define _STRUCT_RICH_STRING_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_unicode.h"
#include "std/std_memset.h"
#include "struct/struct_string.h"
#include "struct/struct_color.h"
#include "misc/misc_standard_colors.h"
#include "mem/mem_arena.h"
#include "misc/misc_parsing.h"
#include "misc/misc_two_pass.h"

typedef enum FontStyleFlag FontStyleFlag;
enum FontStyleFlag
{
	FontStyleFlag_None           = 0x00,
	FontStyleFlag_Bold           = 0x01,
	FontStyleFlag_Italic         = 0x02,
	FontStyleFlag_Inverted       = 0x04,
	FontStyleFlag_Underline      = 0x08,
	FontStyleFlag_Strikethrough  = 0x10,
	FontStyleFlag_Outline        = 0x20,
	FontStyleFlag_Highlighted    = 0x40,
	FontStyleFlag_ColoredGlyphs  = 0x80,
	FontStyleFlag_All            = 0xFF,
	FontStyleFlag_FontFileFlags  = FontStyleFlag_Bold|FontStyleFlag_Italic|FontStyleFlag_ColoredGlyphs, //These are flags that affect which font file we pull from for rasterization
	FontStyleFlag_FontAtlasFlags = FontStyleFlag_Bold|FontStyleFlag_Italic|FontStyleFlag_Inverted, //These are flags that affect which font atlas we use
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
		case FontStyleFlag_ColoredGlyphs: return "ColoredGlyphs";
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
	RichStrStyleChangeType_ColorAndAlpha,
	RichStrStyleChangeType_Alpha,
	RichStrStyleChangeType_Count,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetRichStrStyleChangeTypeStr(RichStrStyleChangeType enumValue);
#else
const char* GetRichStrStyleChangeTypeStr(RichStrStyleChangeType enumValue)
{
	switch (enumValue)
	{
		case RichStrStyleChangeType_None:          return "None";
		case RichStrStyleChangeType_FontSize:      return "FontSize";
		case RichStrStyleChangeType_FontStyle:     return "FontStyle";
		case RichStrStyleChangeType_Color:         return "Color";
		case RichStrStyleChangeType_ColorAndAlpha: return "ColorAndAlpha";
		case RichStrStyleChangeType_Alpha:         return "Alpha";
		case RichStrStyleChangeType_Count:         return "Count";
		default: return "Unknown";
	}
}
#endif

#define RICH_STYLE_DEFAULT_COLOR_VALUE  0x00FFFFFF
#define RICH_STYLE_DEFAULT_COLOR  NewColorU32(RICH_STYLE_DEFAULT_COLOR_VALUE)

typedef plex RichStrStyle RichStrStyle;
plex RichStrStyle
{
	r32 fontSize;
	u8 fontStyle;
	Color32 color;
};

// Each piece has a set of changes to the style, not a style directly
typedef plex RichStrStyleChange RichStrStyleChange;
plex RichStrStyleChange
{
	RichStrStyleChangeType type;
	car
	{
		r32 fontSize;  // (0.0f means default size)
		plex
		{
			u8 enableStyleFlags;
			u8 disableStyleFlags;
			u8 defaultStyleFlags;
		};
		Color32 color; // (TransparentWhite means default color)
		r32 alpha; // (-1.0f means default alpha)
	};
};

typedef plex RichStrPiece RichStrPiece;
plex RichStrPiece
{
	RichStrStyleChange styleChange;
	Str8 str;
};

//NOTE: When a RichStr has only one piece we store it directly in RichStr structure in fullPiece,
// otherwise the pieces are allocated separately but the str in each piece is just a slice of fullPiece.str
typedef plex RichStr RichStr;
plex RichStr
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
	PIG_CORE_INLINE RichStrStyleChange NewRichStrStyleChange(RichStrStyleChangeType type, r32 fontSize, u8 enableStyleFlags, u8 disableStyleFlags, u8 defaultStyleFlags, Color32 color, r32 alpha);
	PIG_CORE_INLINE RichStrStyleChange NewRichStrStyleChangeSize(r32 fontSize);
	PIG_CORE_INLINE RichStrStyleChange NewRichStrStyleChangeEnableFlags(u8 enableStyleFlags);
	PIG_CORE_INLINE RichStrStyleChange NewRichStrStyleChangeDisableFlags(u8 disableStyleFlags);
	PIG_CORE_INLINE RichStrStyleChange NewRichStrStyleChangeDefaultFlags(u8 defaultStyleFlags);
	PIG_CORE_INLINE RichStrStyleChange NewRichStrStyleChangeColor(Color32 color, bool includeAlpha);
	PIG_CORE_INLINE RichStrStyleChange NewRichStrStyleChangeAlpha(r32 alpha);
	PIG_CORE_INLINE RichStrStyleChange NewRichStrStyleChangeAlphaU8(u8 alpha);
	PIG_CORE_INLINE RichStrStyleChange OppositeRichStrStyleChange(RichStrStyleChange change);
	PIG_CORE_INLINE RichStr ToRichStrEx(Str8 string, RichStrStyleChange styleChange);
	PIG_CORE_INLINE RichStr ToRichStr(Str8 string);
	PIG_CORE_INLINE void FreeRichStr(Arena* arena, RichStr* richStrPntr);
	PIG_CORE_INLINE RichStr NewRichStr(Arena* arena, uxx numPieces, const RichStrPiece* pieces);
	PIG_CORE_INLINE RichStrPiece* GetRichStrPiece(RichStr* richStr, uxx pieceIndex);
	PIG_CORE_INLINE RichStrPiece* GetRichStrPieceForByteIndex(RichStr* richStr, uxx byteIndex, uxx* pieceIndexOut);
	PIG_CORE_INLINE RichStr AllocRichStr(Arena* arena, RichStr richStr);
	RichStr ToRichStrWithHighlight(Arena* arena, Str8 string, uxx highlightStartIndex, uxx highlightEndIndex);
	RichStr RichStrSlice(Arena* arena, RichStr baseString, uxx startIndex, uxx endIndex);
	PIG_CORE_INLINE RichStr RichStrSliceLength(Arena* arena, RichStr baseString, uxx startIndex, uxx length);
	PIG_CORE_INLINE RichStr RichStrSliceFrom(Arena* arena, RichStr baseString, uxx startIndex);
	PIG_CORE_INLINE bool IsFontStyleFlagChangingInRichStrStyleChange(const RichStrStyle* style, u8 defaultFontStyle, RichStrStyleChange styleChange, u8 fontStyleFlag);
	PIG_CORE_INLINE void ApplyRichStyleChange(RichStrStyle* style, RichStrStyleChange change, r32 defaultFontSize, u8 defaultFontStyle, Color32 defaultColor);
	RichStr DecodeStrToRichStr(Arena* arena, Str8 encodedString);
	Str8 EncodeRichStr(Arena* arena, RichStr richStr, bool useBackspaceAndBellChars, bool addNullTerm);
#endif

#define RichStrStyleChange_None NewRichStrStyleChange(RichStrStyleChangeType_None, 0.0f, 0x00, 0x00, 0x00, RICH_STYLE_DEFAULT_COLOR, 0.0f)
#define RichStrStyleChange_None_Const (RichStrStyleChange){ .type=RichStrStyleChangeType_None }

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
PEXPI RichStrStyleChange NewRichStrStyleChange(RichStrStyleChangeType type, r32 fontSize, u8 enableStyleFlags, u8 disableStyleFlags, u8 defaultStyleFlags, Color32 color, r32 alpha)
{
	RichStrStyleChange result = ZEROED;
	result.type = type;
	if (type == RichStrStyleChangeType_FontSize) { result.fontSize = fontSize; }
	if (type == RichStrStyleChangeType_FontStyle)
	{
		result.enableStyleFlags = enableStyleFlags;
		result.disableStyleFlags = disableStyleFlags;
		result.defaultStyleFlags = defaultStyleFlags;
	}
	if (type == RichStrStyleChangeType_Color || type == RichStrStyleChangeType_ColorAndAlpha) { result.color = color; }
	if (type == RichStrStyleChangeType_Alpha) { result.alpha = alpha; }
	return result;
}
PEXPI RichStrStyleChange NewRichStrStyleChangeSize(r32 fontSize)
{
	return NewRichStrStyleChange(RichStrStyleChangeType_FontSize, fontSize, 0x00, 0x00, 0x00, RICH_STYLE_DEFAULT_COLOR, 0.0f);
}
PEXPI RichStrStyleChange NewRichStrStyleChangeEnableFlags(u8 enableStyleFlags)
{
	return NewRichStrStyleChange(RichStrStyleChangeType_FontStyle, 0.0f, enableStyleFlags, 0x00, 0x00, RICH_STYLE_DEFAULT_COLOR, 0.0f);
}
PEXPI RichStrStyleChange NewRichStrStyleChangeDisableFlags(u8 disableStyleFlags)
{
	return NewRichStrStyleChange(RichStrStyleChangeType_FontStyle, 0.0f, 0x00, disableStyleFlags, 0x00, RICH_STYLE_DEFAULT_COLOR, 0.0f);
}
PEXPI RichStrStyleChange NewRichStrStyleChangeDefaultFlags(u8 defaultStyleFlags)
{
	return NewRichStrStyleChange(RichStrStyleChangeType_FontStyle, 0.0f, 0x00, 0x00, defaultStyleFlags, RICH_STYLE_DEFAULT_COLOR, 0.0f);
}
PEXPI RichStrStyleChange NewRichStrStyleChangeColor(Color32 color, bool includeAlpha)
{
	return NewRichStrStyleChange(includeAlpha ? RichStrStyleChangeType_ColorAndAlpha : RichStrStyleChangeType_Color, 0.0f, 0x00, 0x00, 0x00, color, 0.0f);
}
PEXPI RichStrStyleChange NewRichStrStyleChangeAlpha(r32 alpha)
{
	return NewRichStrStyleChange(RichStrStyleChangeType_Alpha, 0.0f, 0x00, 0x00, 0x00, RICH_STYLE_DEFAULT_COLOR, alpha);
}
PEXPI RichStrStyleChange NewRichStrStyleChangeAlphaU8(u8 alpha)
{
	return NewRichStrStyleChange(RichStrStyleChangeType_Alpha, 0.0f, 0x00, 0x00, 0x00, RICH_STYLE_DEFAULT_COLOR, (r32)alpha / 255.0f);
}

PEXPI RichStrStyleChange OppositeRichStrStyleChange(RichStrStyleChange change)
{
	RichStrStyleChange result = RichStrStyleChange_None_Const;
	result.type = change.type;
	if (change.type == RichStrStyleChangeType_FontSize)
	{
		result.fontSize = 0.0f;
	}
	else if (change.type == RichStrStyleChangeType_FontStyle)
	{
		if (change.enableStyleFlags != 0x00)
		{
			result.disableStyleFlags = change.enableStyleFlags;
		}
		else if (change.disableStyleFlags != 0x00)
		{
			result.enableStyleFlags = change.disableStyleFlags;
		}
		else { result.defaultStyleFlags = change.defaultStyleFlags; }
	}
	else if (change.type == RichStrStyleChangeType_Color)
	{
		result.color = RICH_STYLE_DEFAULT_COLOR;
	}
	else if (change.type == RichStrStyleChangeType_ColorAndAlpha)
	{
		result.color = RICH_STYLE_DEFAULT_COLOR;
	}
	else if (change.type == RichStrStyleChangeType_Alpha)
	{
		result.alpha = -1.0f;
	}
	return result;
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
	
	result.numPieces = numPieces;
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

PEXPI RichStrPiece* GetRichStrPieceForByteIndex(RichStr* richStr, uxx byteIndex, uxx* pieceIndexOut)
{
	NotNull(richStr);
	if (richStr->numPieces > 1)
	{
		NotNull(richStr->pieces);
		for (uxx pIndex = 0; pIndex < richStr->numPieces; pIndex++)
		{
			if (byteIndex < richStr->pieces[pIndex].str.length)
			{
				SetOptionalOutPntr(pieceIndexOut, byteIndex);
				return &richStr->pieces[pIndex];
			}
			byteIndex -= richStr->pieces[pIndex].str.length;
		}
		SetOptionalOutPntr(pieceIndexOut, richStr->pieces[richStr->numPieces-1].str.length + byteIndex);
		return &richStr->pieces[richStr->numPieces-1];
	}
	else
	{
		SetOptionalOutPntr(pieceIndexOut, byteIndex);
		return &richStr->fullPiece;
	}
}

PEXPI RichStr AllocRichStr(Arena* arena, RichStr richStr)
{
	return NewRichStr(arena, richStr.numPieces, (richStr.numPieces > 1) ? richStr.pieces : &richStr.fullPiece);
}

PEXP RichStr ToRichStrWithHighlight(Arena* arena, Str8 string, uxx highlightStartIndex, uxx highlightEndIndex)
{
	Assert(highlightEndIndex >= highlightStartIndex);
	if (highlightEndIndex == highlightStartIndex)
	{
		RichStr result = ToRichStr(string);
		result = AllocRichStr(arena, result);
		return result;
	}
	
	RichStrPiece pieces[3];
	uxx pIndex = 0;
	
	if (highlightStartIndex > 0)
	{
		pieces[pIndex].str = StrSlice(string, 0, highlightStartIndex);
		pieces[pIndex].styleChange = RichStrStyleChange_None;
		pIndex++;
	}
	
	pieces[pIndex].str = StrSlice(string, highlightStartIndex, highlightEndIndex);
	pieces[pIndex].styleChange = NewRichStrStyleChangeEnableFlags(FontStyleFlag_Highlighted);
	pIndex++;
	
	if (highlightEndIndex < string.length)
	{
		pieces[pIndex].str = StrSliceFrom(string, highlightEndIndex);
		pieces[pIndex].styleChange = NewRichStrStyleChangeDefaultFlags(FontStyleFlag_Highlighted);
		pIndex++;
	}
	
	return NewRichStr(arena, pIndex, &pieces[0]);
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

PEXPI bool IsFontStyleFlagChangingInRichStrStyleChange(const RichStrStyle* style, u8 defaultFontStyle, RichStrStyleChange styleChange, u8 fontStyleFlag)
{
	NotNull(style);
	if (fontStyleFlag == 0) { return false; }
	if (styleChange.type != RichStrStyleChangeType_FontStyle) { return false; }
	if (IsFlagSet(style->fontStyle, fontStyleFlag) && IsFlagSet(styleChange.disableStyleFlags, fontStyleFlag)) { return true; }
	if (!IsFlagSet(style->fontStyle, fontStyleFlag) && IsFlagSet(styleChange.enableStyleFlags, fontStyleFlag)) { return true; }
	if (IsFlagSet(styleChange.defaultStyleFlags, fontStyleFlag) && IsFlagSet(defaultFontStyle, fontStyleFlag) != IsFlagSet(style->fontStyle, fontStyleFlag)) { return true; }
	return false;
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
		case RichStrStyleChangeType_Color: { u8 oldAlpha = style->color.a; style->color = (styleChange.color.valueU32 != RICH_STYLE_DEFAULT_COLOR_VALUE) ? styleChange.color : defaultColor; style->color.a = oldAlpha; } break;
		case RichStrStyleChangeType_ColorAndAlpha: style->color = (styleChange.color.valueU32 != RICH_STYLE_DEFAULT_COLOR_VALUE) ? styleChange.color : defaultColor; break;
		case RichStrStyleChangeType_Alpha: style->color.a = (styleChange.alpha >= 0) ? (u8)RoundR32i(styleChange.alpha * 255) : defaultColor.a; break;
	}
}

typedef plex RichStrParseState RichStrParseState;
plex RichStrParseState
{
	RichStr result;
	RichStrStyleChange nextStyleChange;
	Str8 encodedString;
	uxx pieceStartIndex;
	uxx fullStrByteIndex;
	u8 utf8ByteSize;
	uxx pieceIndex;
	u8 enabledFlags;
	bool prevCharWasEscape;
};

static void TwoPassRichStrPiece(RichStrParseState* state, u8 pass, uxx bIndex)
{
	Str8 pieceStr = StrSlice(state->encodedString, state->pieceStartIndex, bIndex);
	uxx numEscapeSequences = 0;
	for (uxx cIndex = 0; cIndex < pieceStr.length; cIndex++)
	{
		if (pieceStr.chars[cIndex] == '\\' && cIndex+1 < pieceStr.length &&
			(pieceStr.chars[cIndex+1] == '\\' || pieceStr.chars[cIndex+1] == '[' || pieceStr.chars[cIndex+1] == ']'))
		{
			numEscapeSequences++;
			cIndex++;
		}
	}
	if (pass == 1)
	{
		DebugAssert(state->pieceIndex < state->result.numPieces);
		RichStrPiece* piece = GetRichStrPiece(&state->result, state->pieceIndex);
		piece->str = StrSliceLength(state->result.fullPiece.str, state->fullStrByteIndex, pieceStr.length - numEscapeSequences);
		uxx writeIndex = 0;
		for (uxx cIndex = 0; cIndex < pieceStr.length; cIndex++)
		{
			if (pieceStr.chars[cIndex] == '\\' && cIndex+1 < pieceStr.length &&
				(pieceStr.chars[cIndex+1] == '\\' || pieceStr.chars[cIndex+1] == '[' || pieceStr.chars[cIndex+1] == ']'))
			{
				continue; //skip writing the backslashes that served as escapes
			}
			Assert(writeIndex < pieceStr.length - numEscapeSequences);
			piece->str.chars[writeIndex] = pieceStr.chars[cIndex];
			writeIndex++;
		}
		Assert(writeIndex == pieceStr.length - numEscapeSequences);
		piece->styleChange = state->nextStyleChange;
	}
	state->fullStrByteIndex += pieceStr.length - numEscapeSequences;
	state->pieceStartIndex = bIndex + state->utf8ByteSize;
	state->pieceIndex++;
}

//TODO: If the string starts out with a style change sequence then the first piece shouldn't be an empty string with no style changes
// Syntax: All style changes are encoded as text between square brackets: [color=F83333] The text inside the pipe walls always contains
//         a word and optionally contains an equals and value. Possible words:
//         bold[=true/false/default]
//         italic[=true/false/default]
//         underline[=true/false/default]
//         strike[=true/false/default]
//         outline[=true/false/default]
//         highlight[=true/false/default]
//         color[=RRGGB/AARRGGBB/default] (no value implies default color)
//         rgb[=RRGGB/default] (alpha channel of color is unchanged, no value implies default color)
//         alpha[=N.N/0xNN] (float and 2-byte hex accepted, no value implies default alpha)
//         size[=N] (no value implies default font size)
//         Bold and Italic use \b (backspace) and \a (bell) escape sequences as special shorthand characters to toggling them on in a C string
//         since we don't use them for anything in our strings normally. Bold and Italic also have regular syntax as well though which are
//         More useful in other contexts, like writing an encoded string into a file that gets loaded and parsed, etc.
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
			else if (codepoint == '[' && !state.prevCharWasEscape)
			{
				RichStrStyleChange styleChange = RichStrStyleChange_None_Const;
				uxx styleChangePartLength = 0;
				
				uxx nextPipeIndex = FindNextCharInStr(encodedString, bIndex + state.utf8ByteSize, StrLit("]"));
				if (nextPipeIndex < encodedString.length)
				{
					Str8 contentsStr = StrSlice(encodedString, bIndex + state.utf8ByteSize, nextPipeIndex);
					uxx equalsIndex = FindNextCharInStr(contentsStr, 0, StrLit("="));
					if (equalsIndex < contentsStr.length)
					{
						Str8 namePart = TrimWhitespace(StrSlice(contentsStr, 0, equalsIndex));
						Str8 valuePart = TrimWhitespace(StrSliceFrom(contentsStr, equalsIndex+1));
						
						if (StrAnyCaseEquals(namePart, StrLit("bold")) ||
							StrAnyCaseEquals(namePart, StrLit("italic")) ||
							StrAnyCaseEquals(namePart, StrLit("underline")) ||
							StrAnyCaseEquals(namePart, StrLit("strike")) ||
							StrAnyCaseEquals(namePart, StrLit("outline")) ||
							StrAnyCaseEquals(namePart, StrLit("highlight")))
						{
							FontStyleFlag styleFlag = FontStyleFlag_None;
							if (StrAnyCaseEquals(namePart, StrLit("bold")))           { styleFlag = FontStyleFlag_Bold;          }
							else if (StrAnyCaseEquals(namePart, StrLit("italic")))    { styleFlag = FontStyleFlag_Italic;        }
							else if (StrAnyCaseEquals(namePart, StrLit("underline"))) { styleFlag = FontStyleFlag_Underline;     }
							else if (StrAnyCaseEquals(namePart, StrLit("strike")))    { styleFlag = FontStyleFlag_Strikethrough; }
							else if (StrAnyCaseEquals(namePart, StrLit("outline")))   { styleFlag = FontStyleFlag_Outline;       }
							else if (StrAnyCaseEquals(namePart, StrLit("highlight"))) { styleFlag = FontStyleFlag_Highlighted;   }
							
							bool valueBool = false;
							if (TryParseBool(valuePart, &valueBool, nullptr))
							{
								styleChangePartLength = (nextPipeIndex + 1) - bIndex;
								styleChange.type = RichStrStyleChangeType_FontStyle;
								if (valueBool) { styleChange.enableStyleFlags = styleFlag; FlagSet(state.enabledFlags, styleFlag); }
								else { styleChange.disableStyleFlags = styleFlag; FlagUnset(state.enabledFlags, styleFlag); }
							}
							else if (StrAnyCaseEquals(valuePart, StrLit("default")))
							{
								styleChangePartLength = (nextPipeIndex + 1) - bIndex;
								styleChange.type = RichStrStyleChangeType_FontStyle;
								styleChange.defaultStyleFlags = styleFlag;
								FlagUnset(state.enabledFlags, styleFlag);
							}
						}
						else if (StrAnyCaseEquals(namePart, StrLit("color")) ||
							StrAnyCaseEquals(namePart, StrLit("rgb")))
						{
							Color32 valueColor = Transparent;
							if (TryParseColor(valuePart, &valueColor, nullptr))
							{
								styleChangePartLength = (nextPipeIndex + 1) - bIndex;
								styleChange.type = StrAnyCaseEquals(namePart, StrLit("rgb")) ? RichStrStyleChangeType_Color : RichStrStyleChangeType_ColorAndAlpha;
								styleChange.color = valueColor;
							}
							else if (StrAnyCaseEquals(valuePart, StrLit("default")))
							{
								styleChangePartLength = (nextPipeIndex + 1) - bIndex;
								styleChange.type = RichStrStyleChangeType_ColorAndAlpha;
								styleChange.color = RICH_STYLE_DEFAULT_COLOR;
							}
						}
						else if (StrAnyCaseEquals(namePart, StrLit("alpha")))
						{
							r32 valueR32 = 0.0f;
							if (TryParseR32(valuePart, &valueR32, nullptr))
							{
								styleChangePartLength = (nextPipeIndex + 1) - bIndex;
								styleChange.type = RichStrStyleChangeType_Alpha;
								styleChange.alpha = valueR32;
							}
						}
						else if (StrAnyCaseEquals(namePart, StrLit("size")))
						{
							r32 valueR32 = 0.0f;
							if (TryParseR32(valuePart, &valueR32, nullptr))
							{
								styleChangePartLength = (nextPipeIndex + 1) - bIndex;
								styleChange.type = RichStrStyleChangeType_FontSize;
								styleChange.fontSize = valueR32;
							}
						}
					}
					else
					{
						Str8 namePart = TrimWhitespace(contentsStr);
						if (StrAnyCaseEquals(namePart, StrLit("bold")) ||
							StrAnyCaseEquals(namePart, StrLit("italic")) ||
							StrAnyCaseEquals(namePart, StrLit("underline")) ||
							StrAnyCaseEquals(namePart, StrLit("strike")) ||
							StrAnyCaseEquals(namePart, StrLit("outline")) ||
							StrAnyCaseEquals(namePart, StrLit("highlight")))
						{
							FontStyleFlag styleFlag = FontStyleFlag_None;
							if (StrAnyCaseEquals(namePart, StrLit("bold")))           { styleFlag = FontStyleFlag_Bold;          }
							else if (StrAnyCaseEquals(namePart, StrLit("italic")))    { styleFlag = FontStyleFlag_Italic;        }
							else if (StrAnyCaseEquals(namePart, StrLit("underline"))) { styleFlag = FontStyleFlag_Underline;     }
							else if (StrAnyCaseEquals(namePart, StrLit("strike")))    { styleFlag = FontStyleFlag_Strikethrough; }
							else if (StrAnyCaseEquals(namePart, StrLit("outline")))   { styleFlag = FontStyleFlag_Outline;       }
							else if (StrAnyCaseEquals(namePart, StrLit("highlight"))) { styleFlag = FontStyleFlag_Highlighted;   }
							
							styleChangePartLength = (nextPipeIndex + 1) - bIndex;
							styleChange.type = RichStrStyleChangeType_FontStyle;
							if (IsFlagSet(state.enabledFlags, styleFlag)) { styleChange.defaultStyleFlags = styleFlag; FlagUnset(state.enabledFlags, styleFlag); }
							else { styleChange.enableStyleFlags = styleFlag; FlagSet(state.enabledFlags, styleFlag); }
						}
						else if (StrAnyCaseEquals(namePart, StrLit("color")) ||
							StrAnyCaseEquals(namePart, StrLit("rgb")))
						{
							styleChangePartLength = (nextPipeIndex + 1) - bIndex;
							styleChange.type = StrAnyCaseEquals(namePart, StrLit("rgb")) ? RichStrStyleChangeType_Color : RichStrStyleChangeType_ColorAndAlpha;
							styleChange.color = RICH_STYLE_DEFAULT_COLOR;
						}
						else if (StrAnyCaseEquals(namePart, StrLit("alpha")))
						{
							styleChangePartLength = (nextPipeIndex + 1) - bIndex;
							styleChange.type = RichStrStyleChangeType_Alpha;
							styleChange.alpha = -1.0f;
						}
						else if (StrAnyCaseEquals(namePart, StrLit("size")))
						{
							styleChangePartLength = (nextPipeIndex + 1) - bIndex;
							styleChange.type = RichStrStyleChangeType_FontSize;
							styleChange.fontSize = 0.0f;
						}
					}
				}
				
				if (styleChange.type != RichStrStyleChangeType_None)
				{
					TwoPassRichStrPiece(&state, pass, bIndex);
					state.pieceStartIndex = bIndex + styleChangePartLength;
					state.nextStyleChange = styleChange;
					bIndex += styleChangePartLength-1;
				}
			}
			
			state.prevCharWasEscape = (codepoint == '\\' && !state.prevCharWasEscape);
			if (state.utf8ByteSize > 1) { bIndex += state.utf8ByteSize-1; }
		}
		if (state.pieceStartIndex < encodedString.length)
		{
			TwoPassRichStrPiece(&state, pass, encodedString.length);
		}
		
		if (pass == 0)
		{
			state.result.fullPiece.str.length = state.fullStrByteIndex;
			if (state.result.fullPiece.str.length == 0) { break; }
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

PEXP Str8 EncodeRichStr(Arena* arena, RichStr richStr, bool useBackspaceAndBellChars, bool addNullTerm)
{
	TwoPassStr8Loop(result, arena, addNullTerm)
	{
		for (uxx pIndex = 0; pIndex < richStr.numPieces; pIndex++)
		{
			RichStrPiece* piece = GetRichStrPiece(&richStr, pIndex);
			if (piece->styleChange.type != RichStrStyleChangeType_None)
			{
				if (piece->styleChange.type == RichStrStyleChangeType_FontStyle)
				{
					if (piece->styleChange.enableStyleFlags != 0x00)
					{
						if (IsFlagSet(piece->styleChange.enableStyleFlags, FontStyleFlag_Bold))          { TwoPassPrint(&result, "[bold=1]"); }
						if (IsFlagSet(piece->styleChange.enableStyleFlags, FontStyleFlag_Italic))        { TwoPassPrint(&result, "[italic=1]"); }
						if (IsFlagSet(piece->styleChange.enableStyleFlags, FontStyleFlag_Underline))     { TwoPassPrint(&result, "[underline=1]"); }
						if (IsFlagSet(piece->styleChange.enableStyleFlags, FontStyleFlag_Strikethrough)) { TwoPassPrint(&result, "[strike=1]"); }
						if (IsFlagSet(piece->styleChange.enableStyleFlags, FontStyleFlag_Outline))       { TwoPassPrint(&result, "[outline=1]"); }
						if (IsFlagSet(piece->styleChange.enableStyleFlags, FontStyleFlag_Highlighted))   { TwoPassPrint(&result, "[highlight=1]"); }
					}
					else if (piece->styleChange.disableStyleFlags != 0x00)
					{
						if (IsFlagSet(piece->styleChange.disableStyleFlags, FontStyleFlag_Bold))          { TwoPassPrint(&result, "[bold=0]"); }
						if (IsFlagSet(piece->styleChange.disableStyleFlags, FontStyleFlag_Italic))        { TwoPassPrint(&result, "[italic=0]"); }
						if (IsFlagSet(piece->styleChange.disableStyleFlags, FontStyleFlag_Underline))     { TwoPassPrint(&result, "[underline=0]"); }
						if (IsFlagSet(piece->styleChange.disableStyleFlags, FontStyleFlag_Strikethrough)) { TwoPassPrint(&result, "[strike=0]"); }
						if (IsFlagSet(piece->styleChange.disableStyleFlags, FontStyleFlag_Outline))       { TwoPassPrint(&result, "[outline=0]"); }
						if (IsFlagSet(piece->styleChange.disableStyleFlags, FontStyleFlag_Highlighted))   { TwoPassPrint(&result, "[highlight=0]"); }
					}
					else if (piece->styleChange.defaultStyleFlags != 0x00)
					{
						if (IsFlagSet(piece->styleChange.defaultStyleFlags, FontStyleFlag_Bold))          { TwoPassPrint(&result, useBackspaceAndBellChars ? "\b" : "[bold]"); }
						if (IsFlagSet(piece->styleChange.defaultStyleFlags, FontStyleFlag_Italic))        { TwoPassPrint(&result, useBackspaceAndBellChars ? "\a" : "[italic]"); }
						if (IsFlagSet(piece->styleChange.defaultStyleFlags, FontStyleFlag_Underline))     { TwoPassPrint(&result, "[underline]"); }
						if (IsFlagSet(piece->styleChange.defaultStyleFlags, FontStyleFlag_Strikethrough)) { TwoPassPrint(&result, "[strike]"); }
						if (IsFlagSet(piece->styleChange.defaultStyleFlags, FontStyleFlag_Outline))       { TwoPassPrint(&result, "[outline]"); }
						if (IsFlagSet(piece->styleChange.defaultStyleFlags, FontStyleFlag_Highlighted))   { TwoPassPrint(&result, "[highlight]"); }
					}
				}
				else if (piece->styleChange.type == RichStrStyleChangeType_FontSize)
				{
					if (piece->styleChange.fontSize == 0.0f)
					{
						TwoPassPrint(&result, "[size]");
					}
					else
					{
						TwoPassPrint(&result, "[size=%.0f]", piece->styleChange.fontSize);
					}
				}
				else if (piece->styleChange.type == RichStrStyleChangeType_Alpha)
				{
					if (piece->styleChange.alpha == -1.0f)
					{
						TwoPassPrint(&result, "[alpha]");
					}
					else
					{
						TwoPassPrint(&result, "[alpha=%g]", piece->styleChange.alpha);
					}
				}
				else if (piece->styleChange.type == RichStrStyleChangeType_ColorAndAlpha)
				{
					if (piece->styleChange.color.valueU32 == RICH_STYLE_DEFAULT_COLOR.valueU32)
					{
						TwoPassPrint(&result, "[color]");
					}
					else if (piece->styleChange.color.a == 0xFF)
					{
						TwoPassPrint(&result, "[color=%02X%02X%02X]", piece->styleChange.color.r, piece->styleChange.color.g, piece->styleChange.color.b);
					}
					else
					{
						TwoPassPrint(&result, "[color=%02X%02X%02X%02X]", piece->styleChange.color.a, piece->styleChange.color.r, piece->styleChange.color.g, piece->styleChange.color.b);
					}
				}
				else if (piece->styleChange.type == RichStrStyleChangeType_Color)
				{
					if (piece->styleChange.color.valueU32 == RICH_STYLE_DEFAULT_COLOR.valueU32)
					{
						TwoPassPrint(&result, "[rgb]");
					}
					else
					{
						TwoPassPrint(&result, "[rgb=%02X%02X%02X]", piece->styleChange.color.r, piece->styleChange.color.g, piece->styleChange.color.b);
					}
				}
				else { DebugAssert(false); }
			}
			
			//Push all characters, add backslashes before '[' to escape them
			for (uxx cIndex = 0; cIndex < piece->str.length; cIndex++)
			{
				if (piece->str.chars[cIndex] == '[' || piece->str.chars[cIndex] == '\\') { TwoPassChar(&result, '\\'); }
				TwoPassChar(&result, piece->str.chars[cIndex]);
			}
		}
		
		TwoPassStr8LoopEnd(&result);
	}
	return result.str;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_RICH_STRING_H

#if defined(_STRUCT_RICH_STRING_H) && defined(_MEM_SCRATCH_H) && defined(_STRUCT_RANGES_H)
#include "cross/cross_rich_string_scratch_and_ranges.h"
#endif
