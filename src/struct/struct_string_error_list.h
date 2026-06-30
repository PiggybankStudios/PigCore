/*
File:   struct_string_error_list.h
Author: Taylor Robbins
Date:   08\11\2025
Description:
	** This is sort of a weird data structure, and somewhat specialized.
	** We wanted a way to report syntax errors when typing into a textbox with restrictions
	** Ideally each error comes with an explanation string for what the problem is and
	** a range into the textbox string to indicate where the problem is. But from testing
	** it seemed like we wanted to deduplicate the error strings so that the same error
	** repeated multiple times in the textbox only shows up as a single message in the
	** error string display. However, we don't deduplicat the ranges because we want
	** each instance of the error to be highlighted in the textbox even if the message
	** is a duplicate of a previous error.
*/

#ifndef _STRUCT_STRING_ERROR_LIST_H
#define _STRUCT_STRING_ERROR_LIST_H

typedef plex StrError StrError;
plex StrError
{
	RangeUXX range;
	Str8 error;
	uxx duplicateIndex;
};

typedef plex StrErrorList StrErrorList;
plex StrErrorList
{
	Arena* arena;
	uxx numErrors;
	uxx maxNumErrors;
	StrError* errors;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeStrErrorList(StrErrorList* list);
	StrErrorList NewStrErrorList(Arena* arena, uxx maxNumErrors);
	void AddStrError(StrErrorList* list, RangeUXX range, Str8 errorStr);
	void AddStrErrorPrint(StrErrorList* list, RangeUXX range, const char* formatString, ...);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void FreeStrErrorList(StrErrorList* list)
{
	NotNull(list);
	if (list->arena != nullptr)
	{
		if (list->errors != nullptr) { FreeArray(StrError, list->arena, list->maxNumErrors, list->errors); }
	}
	ClearPointer(list);
}

PEXP StrErrorList NewStrErrorList(Arena* arena, uxx maxNumErrors)
{
	StrErrorList result = ZEROED;
	result.arena = arena;
	result.numErrors = 0;
	result.maxNumErrors = maxNumErrors;
	result.errors = AllocArray(StrError, arena, maxNumErrors);
	NotNull(result.errors);
	return result;
}

PEXP void AddStrError(StrErrorList* list, RangeUXX range, Str8 errorStr)
{
	NotNull(list);
	NotNull(list->arena);
	if (list->numErrors >= list->maxNumErrors) { return; }
	uxx duplicateIndex = UINTXX_MAX;
	for (uxx eIndex = 0; eIndex < list->numErrors; eIndex++)
	{
		if (list->errors[eIndex].duplicateIndex == UINTXX_MAX &&
			StrExactEquals(list->errors[eIndex].error, errorStr))
		{
			duplicateIndex = eIndex;
			break;
		}
	}
	
	list->errors[list->numErrors].error = AllocStr8(list->arena, errorStr);
	list->errors[list->numErrors].range = range;
	list->errors[list->numErrors].duplicateIndex = duplicateIndex;
	list->numErrors++;
}

PEXP void AddStrErrorPrint(StrErrorList* list, RangeUXX range, const char* formatString, ...)
{
	NotNull(list);
	NotNull(list->arena);
	if (list->numErrors >= list->maxNumErrors) { return; }
	ScratchBegin1(scratch, list->arena);
	
	Str8 errorStr = Str8_Empty;
	va_list args;
	va_start(args, formatString);
	int printResult1 = MyVaListPrintf(nullptr, 0, formatString, args); //Measure first
	Assert(printResult1 >= 0);
	va_end(args);
	errorStr.length = (uxx)printResult1;
	errorStr.chars = (char*)AllocMem(scratch, errorStr.length+1); //Allocate
	NotNull(errorStr.chars);
	va_start(args, formatString);
	MyVaListPrintf(errorStr.chars, (size_t)(errorStr.length+1), formatString, args); //Real printf
	va_end(args);
	errorStr.chars[errorStr.length] = '\0';
	
	AddStrError(list, range, errorStr);
	ScratchEnd(scratch);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_STRING_ERROR_LIST_H
