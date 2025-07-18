/*
File:   misc_result.h
Author: Taylor Robbins
Date:   01\18\2025
Description:
	** Result is an enumeration that is used for many of our enumeration scenarios where:
	**  1. the full list of enum values is irrelevant (i.e. unapplicable values are not a problem)
	**  2. The integer values of enum entries are unimportant
	**  3. There are a small number of possibilities (or the possibilities are shared with many other enums)
	** With all this in mind, we can make a single enum that serves all these
	** disparate scenarios, so we don't have to create new enums all the time.
	
	** Another benefit of having a single enum for error codes is that we can
	** easily pass the result from some inner error up to higher levels without
	** worrying about mapping the inner error code to one that matches the upper
	** list of possible errors.
	
	** NOTE: This is somewhat similar to "string interning": wikipedia.org/wiki/String_interning
*/

#ifndef _MISC_RESULT_H
#define _MISC_RESULT_H

#include "base/base_defines_check.h"
#include "base/base_macros.h"

typedef enum Result Result;
enum Result
{
	Result_None = 0,
	Result_Success,
	Result_Failure,
	Result_Unknown, //TODO: This name conflicts with UNKNOWN_STR macro thats returned from the GetResultStr function!
	Result_Finished,
	Result_Canceled,
	
	Result_InvalidCharacter,
	Result_InvalidUtf8,
	Result_InvalidType,
	Result_TooShort,
	Result_TooLong,
	Result_EmptyString,
	Result_EmptyPath,
	Result_EmptyFile,
	Result_ExePathTooLong,
	Result_FailedToAllocateMemory,
	Result_UnsupportedPlatform,
	Result_ParsingFailure,
	Result_FileNotFound,
	Result_FailedToReadFile,
	Result_SokolError,
	Result_Underflow,
	Result_Overflow,
	Result_NoNumbers,
	Result_StringOverflow,
	Result_InfinityOrNan,
	Result_FloatParseFailure,
	Result_UnknownString,
	Result_NotEnoughCommas,
	Result_TooManyCommas,
	Result_WrongNumCharacters,
	Result_Nullptr,
	Result_MismatchedParenthesis,
	Result_CharactersAfterParenthesis,
	Result_MissingExponent,
	Result_NoMatch,
	Result_UnexpectedQuantifier,
	Result_UnbalancedBrackets,
	Result_NotEnoughSpace,
	Result_TooManyBranches,
	Result_TooManyBrackets,
	Result_NotFound,
	Result_NoForegroundWindow,
	Result_NoDeviceContext,
	Result_MissingFileHeader,
	Result_InvalidSyntax,
	Result_InvalidOptions,
	Result_Deprecated,
	Result_NoMoreBytes,
	Result_Partial,
	Result_WrongFolder,
	Result_StackOverflow,
	Result_UnexpectedEndElement,
	Result_WrongEndElementType,
	Result_MissingEndElement,
	Result_UnexpectedAttribute,
	Result_InvalidInput,
	Result_Duplicate,
	
	Result_Count,
};

#if !PIG_CORE_IMPLEMENTATION
const char* GetResultStr(Result result);
#else
PEXP const char* GetResultStr(Result result)
{
	switch (result)
	{
		case Result_None: return "None";
		case Result_Success: return "Success";
		case Result_Failure: return "Failure";
		case Result_Unknown: return "Unknown";
		case Result_Finished: return "Finished";
		case Result_Canceled: return "Canceled";
		case Result_InvalidCharacter: return "InvalidCharacter";
		case Result_InvalidUtf8: return "InvalidUtf8";
		case Result_InvalidType: return "InvalidType";
		case Result_TooShort: return "TooShort";
		case Result_TooLong: return "TooLong";
		case Result_EmptyString: return "EmptyString";
		case Result_EmptyPath: return "EmptyPath";
		case Result_EmptyFile: return "EmptyFile";
		case Result_ExePathTooLong: return "ExePathTooLong";
		case Result_FailedToAllocateMemory: return "FailedToAllocateMemory";
		case Result_UnsupportedPlatform: return "UnsupportedPlatform";
		case Result_ParsingFailure: return "ParsingFailure";
		case Result_FileNotFound: return "FileNotFound";
		case Result_FailedToReadFile: return "FailedToReadFile";
		case Result_SokolError: return "SokolError";
		case Result_Underflow: return "Underflow";
		case Result_Overflow: return "Overflow";
		case Result_NoNumbers: return "NoNumbers";
		case Result_StringOverflow: return "StringOverflow";
		case Result_InfinityOrNan: return "InfinityOrNan";
		case Result_FloatParseFailure: return "FloatParseFailure";
		case Result_UnknownString: return "UnknownString";
		case Result_NotEnoughCommas: return "NotEnoughCommas";
		case Result_TooManyCommas: return "TooManyCommas";
		case Result_WrongNumCharacters: return "WrongNumCharacters";
		case Result_Nullptr: return "Nullptr";
		case Result_MismatchedParenthesis: return "MismatchedParenthesis";
		case Result_CharactersAfterParenthesis: return "CharactersAfterParenthesis";
		case Result_MissingExponent: return "MissingExponent";
		case Result_NoMatch: return "NoMatch";
		case Result_UnexpectedQuantifier: return "UnexpectedQuantifier";
		case Result_UnbalancedBrackets: return "UnbalancedBrackets";
		case Result_NotEnoughSpace: return "NotEnoughSpace";
		case Result_TooManyBranches: return "TooManyBranches";
		case Result_TooManyBrackets: return "TooManyBrackets";
		case Result_NotFound: return "NotFound";
		case Result_NoForegroundWindow: return "NoForegroundWindow";
		case Result_NoDeviceContext: return "NoDeviceContext";
		case Result_MissingFileHeader: return "MissingFileHeader";
		case Result_InvalidSyntax: return "InvalidSyntax";
		case Result_InvalidOptions: return "InvalidOptions";
		case Result_Deprecated: return "Deprecated";
		case Result_NoMoreBytes: return "NoMoreBytes";
		case Result_Partial: return "Partial";
		case Result_WrongFolder: return "WrongFolder";
		case Result_StackOverflow: return "StackOverflow";
		case Result_UnexpectedEndElement: return "UnexpectedEndElement";
		case Result_WrongEndElementType: return "WrongEndElementType";
		case Result_MissingEndElement: return "MissingEndElement";
		case Result_UnexpectedAttribute: return "UnexpectedAttribute";
		case Result_InvalidInput: return "InvalidInput";
		case Result_Duplicate: return "Duplicate";
		default: return UNKNOWN_STR;
	}
}
#endif

#endif //  _MISC_RESULT_H
