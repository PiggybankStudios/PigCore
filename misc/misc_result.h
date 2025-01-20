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

#include "base/base_macros.h"

typedef enum Result Result;
enum Result
{
	Result_None = 0,
	Result_Success,
	Result_Failure,
	Result_Unknown, //TODO: This name conflicts with UNKNOWN_STR macro thats returned from the GetResultStr function!
	Result_Finished,
	
	Result_EmptyPath,
	Result_ExePathTooLong,
	Result_FailedToAllocateMemory,
	Result_UnsupportedPlatform,
	Result_ParsingFailure,
	
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
		case Result_EmptyPath: return "EmptyPath";
		case Result_ExePathTooLong: return "ExePathTooLong";
		case Result_FailedToAllocateMemory: return "FailedToAllocateMemory";
		case Result_UnsupportedPlatform: return "UnsupportedPlatform";
		case Result_ParsingFailure: return "ParsingFailure";
		default: return UNKNOWN_STR;
	}
}
#endif

#endif //  _MISC_RESULT_H
