/*
File:   misc_simple_parsers.h
Author: Taylor Robbins
Date:   02\01\2025
Description:
	** Contains a few very simple "parsers" (aka tokenizers). For example LineParser
	** simply splits an input string into lines, handling '\r\n' and '\n' line types.
*/

#ifndef _MISC_SIMPLE_PARSERS_H
#define _MISC_SIMPLE_PARSERS_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "struct/struct_string.h"

//TODO: Should we add a Csv parser?

typedef plex LineParser LineParser;
plex LineParser
{
	uxx byteIndex;
	uxx lineBeginByteIndex;
	uxx lineIndex; //This is not zero based! It's more like a line number you'd see in the gutter of a text editor! It also contains the total number of lines in the input after the iteration has finished
	Str8 inputStr;
	//TODO: Should we add support for Streams again?
};
#define MakeLineParserEx(byteIndexValue, lineBeginByteIndexValue, lineIndexValue, inputStrValue) NEW_STRUCT(LineParser){ .byteIndex=(byteIndexValue), .lineBeginByteIndex=(lineBeginByteIndexValue), .lineIndex=(lineIndexValue), .inputStr=(inputStrValue) }
#define MakeLineParser(inputStr) MakeLineParserEx(0, 0, 0, (inputStr))

typedef enum ParsingTokenType ParsingTokenType;
enum ParsingTokenType
{
	ParsingTokenType_None = 0,
	ParsingTokenType_FilePrefix, //lines starting with #
	ParsingTokenType_Directive, //lines starting with @
	ParsingTokenType_KeyValuePair, //lines with key: value (where leading/trailing whitespace is stripped from key and value)
	ParsingTokenType_Comment, //anything after a // on any line
	ParsingTokenType_Count,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetParsingTokenTypeStr(ParsingTokenType enumValue);
#else
const char* GetParsingTokenTypeStr(ParsingTokenType enumValue)
{
	switch (enumValue)
	{
		case ParsingTokenType_None:         return "None";
		case ParsingTokenType_FilePrefix:   return "FilePrefix";
		case ParsingTokenType_Directive:    return "Directive";
		case ParsingTokenType_KeyValuePair: return "KeyValuePair";
		case ParsingTokenType_Comment:      return "Comment";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef plex ParsingToken ParsingToken;
plex ParsingToken
{
	ParsingTokenType type;
	Str8 str;
	Str8 key;
	Str8 value;
};
#define MakeParsingToken(typeValue, strValue, keyStr, valueStr) NEW_STRUCT(ParsingToken){ .type=(typeValue), .str=(strVale) .key=(keyStr), .value=(valueStr) }

typedef plex TextParser TextParser;
plex TextParser
{
	LineParser lineParser;
	Str8 currentLine;
	uxx byteIndex;
	bool noComments;
};
#define MakeTextParserEx(lineParserValue, currentLineValue, byteIndexValue, noCommentsValue) NEW_STRUCT(TextParser){ .lineParser=(lineParserValue), .currentLine=(currentLineValue), .byteIndex=(byteIndexValue), .noComments=(noCommentsValue) }
#define MakeTextParser(inputStr) MakeTextParserEx(MakeLineParser(inputStr), Str8_Empty, 0, false)

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	bool LineParserGetLine(LineParser* parser, Str8* lineOut);
	PIG_CORE_INLINE bool LineParserIsFinished(const LineParser* parser);
	bool TextParserGetToken(TextParser* parser, ParsingToken* tokenOut);
#endif


// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

/*
Usage Example:
LineParser lineParser = MakeLineParser(fileContents);
Str8 line = Str8_Empty;
while (LineParserGetLine(&lineParser, &line))
{
	// Do whatever you want with line
	// Also check lineParser.lineIndex for a line number (starts at 1 not 0)
	// Also check lineParser.lineBeginByteIndex if you need to know the index in the fileContents at which the line starts (rather than doing pointer arithmetic)
}
*/
PEXP bool LineParserGetLine(LineParser* parser, Str8* lineOut)
{
	NotNull(parser);
	NotNullStr(parser->inputStr);
	if (parser->byteIndex >= parser->inputStr.length) { return false; }
	parser->lineIndex++;
	parser->lineBeginByteIndex = parser->byteIndex;
	
	uxx endOfLineByteSize = 0;
	uxx startIndex = parser->byteIndex;
	while (parser->byteIndex < parser->inputStr.length)
	{
		char nextChar = parser->inputStr.chars[parser->byteIndex];
		char nextNextChar = parser->inputStr.chars[parser->byteIndex+1];
		//TODO: Should we handle \n\r sequence? Windows is \r\n and I don't know of any space where \n\r is considered a valid single new-line
		if (nextChar != nextNextChar &&
			(nextChar     == '\n' || nextChar     == '\r') &&
			(nextNextChar == '\n' || nextNextChar == '\r'))
		{
			endOfLineByteSize = 2;
			break;
		}
		else if (nextChar == '\n' || nextChar == '\r')
		{
			endOfLineByteSize = 1;
			break;
		}
		else
		{
			parser->byteIndex++;
		}
	}
	
	Str8 line = MakeStr8(parser->byteIndex - startIndex, &parser->inputStr.chars[startIndex]);
	parser->byteIndex += endOfLineByteSize;
	SetOptionalOutPntr(lineOut, line);
	return true;
}

PEXPI bool LineParserIsFinished(const LineParser* parser)
{
	return (parser->byteIndex >= parser->inputStr.length);
}

PEXP bool TextParserGetToken(TextParser* parser, ParsingToken* tokenOut)
{
	NotNull(parser);
	NotNull(tokenOut);
	
	while (!LineParserIsFinished(&parser->lineParser))
	{
		if (parser->byteIndex >= parser->currentLine.length)
		{
			bool gotLine = LineParserGetLine(&parser->lineParser, &parser->currentLine);
			if (!gotLine) { return false; }
			parser->byteIndex = 0;
		}
		
		Str8 line = StrSliceFrom(parser->currentLine, parser->byteIndex);
		uxx preTrimLineLength = line.length;
		line = TrimLeadingWhitespace(line);
		uxx numTrimmedWhitespaceChars = preTrimLineLength - line.length;
		if (line.length == 0)
		{
			parser->byteIndex = parser->currentLine.length;
			continue;
		}
		
		//TODO: This doesn't handle if a // shows up inside something like a string where it shouldn't be treated as a comment
		uxx commentStartIndex = 0;
		bool lineContainsComment = false;
		if (!parser->noComments) { lineContainsComment = StrTryExactFind(line, StrLit("//"), &commentStartIndex); }
		
		if (lineContainsComment && commentStartIndex == 0)
		{
			ClearPointer(tokenOut);
			tokenOut->type = ParsingTokenType_Comment;
			tokenOut->str = line;
			tokenOut->value = StrSlice(tokenOut->str, 2, tokenOut->str.length);
			parser->byteIndex = parser->currentLine.length;
			return true;
		}
		else if (lineContainsComment)
		{
			line = StrSlice(line, 0, commentStartIndex);
		}
		
		uxx colonIndex = 0;
		bool lineContainsColon = StrTryExactFind(line, StrLit(":"), &colonIndex);
		
		if (StrStartsWith(line, StrLit("#"), false))
		{
			ClearPointer(tokenOut);
			tokenOut->type = ParsingTokenType_FilePrefix;
			tokenOut->str = line;
			tokenOut->value = TrimWhitespace(StrSliceFrom(line, 1));
			parser->byteIndex += numTrimmedWhitespaceChars + line.length;
			return true;
		}
		else if (StrStartsWith(line, StrLit("@"), false))
		{
			ClearPointer(tokenOut);
			tokenOut->type = ParsingTokenType_Directive;
			tokenOut->str = line;
			tokenOut->value = TrimWhitespace(StrSliceFrom(line, 1));
			parser->byteIndex += numTrimmedWhitespaceChars + line.length;
			return true;
		}
		else if (lineContainsColon)
		{
			ClearPointer(tokenOut);
			tokenOut->type = ParsingTokenType_KeyValuePair;
			tokenOut->str = line;
			tokenOut->key = StrSlice(line, 0, colonIndex);
			tokenOut->value = StrSlice(line, colonIndex+1, line.length);
			tokenOut->key = TrimWhitespace(tokenOut->key);
			tokenOut->value = TrimWhitespace(tokenOut->value);
			parser->byteIndex += numTrimmedWhitespaceChars + line.length;
			return true;
		}
		
		ClearPointer(tokenOut);
		tokenOut->type = ParsingTokenType_None;
		tokenOut->str = line;
		parser->byteIndex += numTrimmedWhitespaceChars + line.length;
		return true;
	}
	
	// WriteLine_W("Line Parser is finished");
	return false;
}

//TODO: Should we bring the XML Parser back?

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _MISC_SIMPLE_PARSERS_H
