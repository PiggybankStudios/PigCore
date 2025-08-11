/*
File:   misc_web.h
Author: Taylor Robbins
Date:   08\08\2025
Description:
	** Contains various helper functions and types related to web tech (primarily HTTP, TLS, IP Addresses, etc.)
*/

#ifndef _MISC_WEB_H
#define _MISC_WEB_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "base/base_macros.h"
#include "struct/struct_string.h"
#include "struct/struct_ranges.h"
#include "mem/mem_arena.h"
#include "misc/misc_two_pass.h"
#include "struct/struct_string_error_list.h"

typedef plex UriParts UriParts;
plex UriParts
{
	Str8 protocol; //aka scheme, ex. "https"
	Str8 hostname; //aka authority ex. "www.website.com:1000"
	Str8 path; //ex. "/blog/post198"
	Str8 parameters; //aka query, ex. "key=value&q=value+with+spaces"
	Str8 anchor; //aka fragment, ex. "section-1.1"
};

typedef enum HttpVerb HttpVerb;
enum HttpVerb
{
	HttpVerb_None = 0,
	HttpVerb_POST,
	HttpVerb_GET,
	HttpVerb_DELETE,
	//TODO: Should we support HEAD, PUT, CONNECT, OPTIONS, and TRACE? Or others registered at https://www.iana.org/assignments/http-methods/http-methods.xhtml
	HttpVerb_Count,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetHttpVerbStr(HttpVerb enumValue);
#else
PEXP const char* GetHttpVerbStr(HttpVerb enumValue)
{
	switch (enumValue)
	{
		case HttpVerb_None:   return "None";
		case HttpVerb_POST:   return "POST";
		case HttpVerb_GET:    return "GET";
		case HttpVerb_DELETE: return "DELETE";
		default: return UNKNOWN_STR;
	}
}
#endif

//See: https://www.iana.org/assignments/media-types/media-types.xhtml
typedef enum MimeType MimeType;
enum MimeType
{
	MimeType_None = 0,
	MimeType_FormUrlEncoded, //application/x-www-form-urlencoded
	MimeType_Count,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetMimeTypeStr(MimeType enumValue);
const char* GetMimeTypeOfficialName(MimeType enumValue);
#else
PEXP const char* GetMimeTypeStr(MimeType enumValue)
{
	switch (enumValue)
	{
		case MimeType_None:           return "None";
		case MimeType_FormUrlEncoded: return "FormUrlEncoded";
		default: return UNKNOWN_STR;
	}
}
PEXP const char* GetMimeTypeOfficialName(MimeType enumValue)
{
	switch (enumValue)
	{
		case MimeType_FormUrlEncoded: return "application/x-www-form-urlencoded"; //https://www.iana.org/assignments/media-types/application/x-www-form-urlencoded
		default: return "";
	}
}
#endif

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	UriParts GetUriParts(Str8 uriStr);
	PIG_CORE_INLINE Str8 GetUriProtocolPart(Str8 uriStr);
	PIG_CORE_INLINE Str8 GetUriHostnamePart(Str8 uriStr);
	PIG_CORE_INLINE Str8 GetUriPathPart(Str8 uriStr);
	PIG_CORE_INLINE Str8 GetUriParametersPart(Str8 uriStr);
	PIG_CORE_INLINE Str8 GetUriAnchorPart(Str8 uriStr);
	uxx GetUriErrors(Str8 uriStr, StrErrorList* list);
	Str8 EncodeHttpHeaders(Arena* arena, uxx numHeaders, const Str8Pair* headers, bool addNullTerm);
	Str8 EscapeStr_FormUrlEncoding(Arena* arena, Str8 str, bool addNullTerm);
	Str8 EncodeHttpKeyValuePairContent(Arena* arena, uxx numItems, const Str8Pair* contentItems, MimeType encoding, bool addNullTerm);
#endif

// +--------------------------------------------------------------+
// |                           Defines                            |
// +--------------------------------------------------------------+
#define HTTP_PORT   80
#define HTTPS_PORT  443

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP UriParts GetUriParts(Str8 uriStr)
{
	NotNullStr(uriStr);
	bool foundProtocolColon = false;
	bool foundPathSlash = false;
	bool foundParametersQuestion = false;
	bool foundAnchorPound = false;
	RangeUXX protocolRange = RangeUXX_Zero_Const;
	RangeUXX hostnameRange = NewRangeUXX(0, uriStr.length);
	RangeUXX pathRange = NewRangeUXX(uriStr.length, uriStr.length);
	RangeUXX parametersRange = NewRangeUXX(uriStr.length, uriStr.length);
	RangeUXX anchorRange = NewRangeUXX(uriStr.length, uriStr.length);
	
	for (uxx cIndex = 0; cIndex < uriStr.length; cIndex++)
	{
		char character = uriStr.chars[cIndex];
		if (!foundAnchorPound)
		{
			if (character == '#')
			{
				foundAnchorPound = true;
				anchorRange = NewRangeUXX(cIndex+1, uriStr.length);
				protocolRange = ClampBelowRangeUXX(protocolRange, cIndex);
				hostnameRange = ClampBelowRangeUXX(hostnameRange, cIndex);
				pathRange = ClampBelowRangeUXX(pathRange, cIndex);
				parametersRange = ClampBelowRangeUXX(parametersRange, cIndex);
			}
			else if (!foundParametersQuestion)
			{
				if (character == '?')
				{
					foundParametersQuestion = true;
					parametersRange = NewRangeUXX(cIndex+1, uriStr.length);
					protocolRange = ClampBelowRangeUXX(protocolRange, cIndex);
					hostnameRange = ClampBelowRangeUXX(hostnameRange, cIndex);
					pathRange = ClampBelowRangeUXX(pathRange, cIndex);
				}
				else if (!foundPathSlash)
				{
					if (character == '/')
					{
						foundPathSlash = true;
						pathRange = NewRangeUXX(cIndex, uriStr.length);
						protocolRange = ClampBelowRangeUXX(protocolRange, cIndex);
						hostnameRange = ClampBelowRangeUXX(hostnameRange, cIndex);
					}
					else if (!foundProtocolColon)
					{
						if (character == ':')
						{
							foundProtocolColon = true;
							protocolRange = NewRangeUXX(0, cIndex);
							if (cIndex+2 < uriStr.length && uriStr.chars[cIndex+1] == '/' && uriStr.chars[cIndex+2] == '/')
							{
								hostnameRange = ClampAboveRangeUXX(hostnameRange, cIndex+3);
								cIndex += 2;
							}
							else
							{
								hostnameRange = ClampAboveRangeUXX(hostnameRange, cIndex+1);
							}
						}
					}
				}
			}
		}
	}
	
	UriParts result;
	result.protocol = StrSlice(uriStr, protocolRange.min, protocolRange.max);
	result.hostname = StrSlice(uriStr, hostnameRange.min, hostnameRange.max);
	result.path = StrSlice(uriStr, pathRange.min, pathRange.max);
	result.parameters = StrSlice(uriStr, parametersRange.min, parametersRange.max);
	result.anchor = StrSlice(uriStr, anchorRange.min, anchorRange.max);
	return result;
}

PEXPI Str8 GetUriProtocolPart(Str8 uriStr) { return GetUriParts(uriStr).protocol; }
PEXPI Str8 GetUriHostnamePart(Str8 uriStr) { return GetUriParts(uriStr).hostname; }
PEXPI Str8 GetUriPathPart(Str8 uriStr) { return GetUriParts(uriStr).path; }
PEXPI Str8 GetUriParametersPart(Str8 uriStr) { return GetUriParts(uriStr).parameters; }
PEXPI Str8 GetUriAnchorPart(Str8 uriStr) { return GetUriParts(uriStr).anchor; }

//TODO: This function is sort of a haphazard collection of possible error scenarios that I could think of. They're not directly based on things in specifications like RFC 3986
PEXP uxx GetUriErrors(Str8 uriStr, StrErrorList* list)
{
	NotNull(list);
	NotNull(list->arena);
	
	if (uriStr.length == 0)
	{
		AddStrError(list, RangeUXX_Zero, StrLit("Uri cannot be empty"));
	}
	
	UriParts parts = GetUriParts(uriStr);
	RangeUXX hostnameRange = SliceToRangeUXX(uriStr, parts.hostname);
	
	if (parts.hostname.length == 0)
	{
		AddStrError(list, hostnameRange, StrLit("Missing hostname (\"www.website.com\" part)"));
	}
	if (parts.hostname.length >= 1)
	{
		char firstChar = uriStr.chars[hostnameRange.min];
		char lastChar = uriStr.chars[hostnameRange.max-1];
		if (!IsCharAlphaNumeric(firstChar))
		{
			AddStrErrorPrint(list, NewRangeUXX(hostnameRange.min, hostnameRange.min+1), "Hostname cannot start with '%c'", firstChar);
		}
		if (!IsCharAlphaNumeric(lastChar))
		{
			AddStrErrorPrint(list, NewRangeUXX(hostnameRange.max-1, hostnameRange.max), "Hostname cannot end with '%c'", lastChar);
		}
		uxx numColonsFound = 0;
		for (uxx cIndex = hostnameRange.min; cIndex < hostnameRange.max; cIndex++)
		{
			if (uriStr.chars[cIndex] == ':')
			{
				if (numColonsFound == 0)
				{
					RangeUXX portRange = NewRangeUXX(cIndex+1, hostnameRange.max);
					Str8 portStr = StrSliceRange(uriStr, portRange);
					Result parseResult = Result_None;
					if (!TryParseU16Ex(portStr, nullptr, &parseResult, false, false, true))
					{
						AddStrErrorPrint(list, portRange, "Invalid port \"%.*s\" (%s)", StrPrint(portStr), GetResultStr(parseResult));
					}
				}
				else
				{
					AddStrError(list, NewRangeUXX(cIndex, cIndex+1), StrLit("Multiple ':' characters not allowed in hostname"));
				}
				numColonsFound++;
			}
		}
	}
	
	u32 prevCodepoint = 0;
	for (uxx cIndex = 0; cIndex < uriStr.length; cIndex++)
	{
		u32 codepoint = 0;
		u8 codepointSize = GetCodepointForUtf8Str(uriStr, cIndex, &codepoint);
		
		if (!IsCharAlphaNumeric(codepoint) &&
			codepoint != ':' && codepoint != '/' && codepoint != '?' && codepoint != '#' && codepoint != '[' && codepoint != ']' &&
			codepoint != '@' && codepoint != '!' && codepoint != '$' && codepoint != '&' && codepoint != '\'' && codepoint != '(' &&
			codepoint != ')' && codepoint != '*' && codepoint != '+' && codepoint != ',' && codepoint != ';' && codepoint != '=' &&
			codepoint != '-' && codepoint != '.' && codepoint != '_' && codepoint != '~')
		{
			AddStrErrorPrint(list, NewRangeUXX(cIndex, cIndex + codepointSize), "Invalid character: \'%.*s\'", codepointSize, &uriStr.chars[cIndex]);
		}
		if (codepoint == '.' && prevCodepoint == '.' && cIndex >= hostnameRange.min && cIndex < hostnameRange.max)
		{
			AddStrError(list, NewRangeUXX(cIndex-1, cIndex+1), StrLit("Two '.' in a row is not allowed in hostname"));
		}
		
		if (codepointSize > 1) { cIndex += codepointSize-1; }
		prevCodepoint = codepoint;
	}
	
	return list->numErrors;
}

PEXP Str8 EncodeHttpHeaders(Arena* arena, uxx numHeaders, const Str8Pair* headers, bool addNullTerm)
{
	Assert(numHeaders == 0 || headers != nullptr);
	TwoPassStr8Loop(result, arena, addNullTerm)
	{
		for (uxx hIndex = 0; hIndex < numHeaders; hIndex++)
		{
			//TODO: Should we escape characters in the key and value strings? Esp. ':', '\n' and '\r'?
			TwoPassStr(&result, headers[hIndex].key);
			TwoPassChar(&result, ':');
			TwoPassStr(&result, headers[hIndex].value);
			if (hIndex+1 < numHeaders) { TwoPassStrNt(&result, "\r\n"); }
		}
		TwoPassStr8LoopEnd(&result);
	}
	return result.str;
}

PEXPI Str8 EscapeStr_FormUrlEncoding(Arena* arena, Str8 str, bool addNullTerm)
{
	TwoPassStr8Loop(result, arena, addNullTerm)
	{
		for (uxx cIndex = 0; cIndex < str.length; cIndex++)
		{
			u32 codepoint = 0;
			u8 codepointSize = GetCodepointForUtf8Str(str, cIndex, &codepoint);
			if (IsCharAlphaNumeric(codepoint) ||
				codepoint == '-' || codepoint == '.' || codepoint == '_' || codepoint == '~') //these are all unreserved characters according to RFC 3986 section 2.3
			{
				TwoPassBytes(&result, codepointSize, &str.chars[cIndex]);
			}
			else if (codepoint == ' ') { TwoPassChar(&result, '+'); } //this is allowed in media type application/x-www-form-urlencoded
			else
			{
				for (u8 byteIndex = 0; byteIndex < codepointSize; byteIndex++)
				{
					TwoPassChar(&result, '%');
					TwoPassPrint(&result, "%02X", str.chars[cIndex + byteIndex]);
				}
			}
			if (codepointSize > 1) { cIndex += codepointSize-1; }
		}
		TwoPassStr8LoopEnd(&result);
	}
	return result.str;
}

PEXP Str8 EncodeHttpKeyValuePairContent(Arena* arena, uxx numItems, const Str8Pair* contentItems, MimeType encoding, bool addNullTerm)
{
	Str8 result = Str8_Empty;
	
	switch (encoding)
	{
		case MimeType_FormUrlEncoded:
		{
			TwoPassStr8Loop(twoPass, arena, addNullTerm)
			{
				for (uxx iIndex = 0; iIndex < numItems; iIndex++)
				{
					if (!IsEmptyStr(contentItems[iIndex].key))
					{
						if (twoPass.index > 0) { TwoPassChar(&twoPass, '&'); }
						
						CreateTwoPassInnerArena(&twoPass, keyArena);
						twoPass.index += EscapeStr_FormUrlEncoding(keyArena, contentItems[iIndex].key, false).length;
						
						if (contentItems[iIndex].value.length > 0)
						{
							TwoPassChar(&twoPass, '=');
							CreateTwoPassInnerArena(&twoPass, valueArena);
							twoPass.index += EscapeStr_FormUrlEncoding(valueArena, contentItems[iIndex].value, false).length;
						}
					}
				}
				TwoPassStr8LoopEnd(&twoPass);
			}
			result = twoPass.str;
		} break;
		
		default: { AssertMsg(false, "EncodeHttpKeyValuePairContent does not have an implementation for the requested encoding!"); } break;
	}
	
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _MISC_WEB_H
