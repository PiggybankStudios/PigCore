/*
File:   os_program_args.h
Author: Taylor Robbins
Date:   02\25\2025
*/

#ifndef _OS_PROGRAM_ARGS_H
#define _OS_PROGRAM_ARGS_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "mem/mem_arena.h"
#include "struct/struct_string.h"
#include "struct/struct_var_array.h"
#include "misc/misc_parsing.h"

typedef plex ProgramArg ProgramArg;
plex ProgramArg
{
	Str8 rawString;
	Str8 name;
	Str8 value;
};

typedef plex ProgramArgs ProgramArgs;
plex ProgramArgs
{
	Arena* arena;
	VarArray args; //ProgramArg
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	void FreeProgramArgs(ProgramArgs* args);
	void ParseProgramArgStr(Arena* arena, Str8 rawString, ProgramArg* argOut);
	void ParseProgramArgs(Arena* arena, uxx numArguments, const char** arguments, ProgramArgs* argsOut);
	Str8 GetNamelessProgramArg(const ProgramArgs* args, uxx argIndex);
	bool FindNamedProgramArgBoolEx(const ProgramArgs* args, Str8 name, Str8 otherName, bool defaultValue, uxx skipCount);
	PIG_CORE_INLINE bool FindNamedProgramArgBool(const ProgramArgs* args, Str8 name, bool defaultValue);
	Str8 FindNamedProgramArgStrEx(const ProgramArgs* args, Str8 name, Str8 otherName, Str8 defaultValue, uxx skipCount);
	PIG_CORE_INLINE Str8 FindNamedProgramArgStr(const ProgramArgs* args, Str8 name, Str8 otherName, Str8 defaultValue);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP void FreeProgramArgs(ProgramArgs* args)
{
	NotNull(args);
	if (args->arena != nullptr)
	{
		VarArrayLoop(&args->args, aIndex)
		{
			VarArrayLoopGet(ProgramArg, arg, &args->args, aIndex);
			FreeStr8(args->arena, &arg->rawString);
			FreeStr8(args->arena, &arg->name);
			FreeStr8(args->arena, &arg->value);
		}
		FreeVarArray(&args->args);
	}
	ClearPointer(args);
}

PEXP void ParseProgramArgStr(Arena* arena, Str8 rawString, ProgramArg* argOut)
{
	NotNull(arena);
	NotNullStr(rawString);
	NotNull(argOut);
	ClearPointer(argOut);
	argOut->rawString = AllocStr8(arena, rawString);
	rawString = TrimWhitespace(rawString);
	bool isNamedArg = false;
	if (StrExactStartsWith(rawString, StrLit("-"))) { isNamedArg = true; rawString = StrSliceFrom(rawString, 1); }
	if (StrExactStartsWith(rawString, StrLit("-"))) { isNamedArg = true; rawString = StrSliceFrom(rawString, 1); }
	if (isNamedArg)
	{
		uxx equalsIndex = StrExactFind(rawString, StrLit("="));
		if (equalsIndex < rawString.length)
		{
			Str8 namePart = TrimWhitespace(StrSlice(rawString, 0, equalsIndex));
			Str8 valuePart = TrimWhitespace(StrSliceFrom(rawString, equalsIndex+1));
			argOut->name = AllocStr8(arena, namePart);
			if (StrExactStartsWith(valuePart, StrLit("\"")) && StrExactEndsWith(valuePart, StrLit("\"")))
			{
				valuePart = StrSlice(valuePart, 1, valuePart.length-1);
				//TODO: We need to remove escape sequences and replace them with their literal character values!
				argOut->value = AllocStr8(arena, valuePart);
			}
			else
			{
				argOut->value = AllocStr8(arena, valuePart);
			}
		}
		else
		{
			argOut->name = AllocStr8(arena, rawString);
		}
	}
	else
	{
		if (StrExactStartsWith(rawString, StrLit("\"")) && StrExactEndsWith(rawString, StrLit("\"")))
		{
			rawString = StrSlice(rawString, 1, rawString.length-1);
			//TODO: We need to remove escape sequences and replace them with their literal character values!
			argOut->value = AllocStr8(arena, rawString);
		}
		else
		{
			argOut->value = AllocStr8(arena, rawString);
		}
	}
}

PEXP void ParseProgramArgs(Arena* arena, uxx numArguments, const char** arguments, ProgramArgs* argsOut)
{
	// #define InitVarArrayWithInitial(type, arrayPntr, arena, initialCountNeeded)
	NotNull(arena);
	NotNull(argsOut);
	Assert(numArguments == 0 || arguments != nullptr);
	ClearPointer(argsOut);
	argsOut->arena = arena;
	InitVarArrayWithInitial(ProgramArg, &argsOut->args, arena, numArguments);
	for (uxx aIndex = 0; aIndex < numArguments; aIndex++)
	{
		ProgramArg* newArg = VarArrayAdd(ProgramArg, &argsOut->args);
		NotNull(newArg);
		ParseProgramArgStr(arena, MakeStr8Nt(arguments[aIndex]), newArg);
	}
}

PEXP Str8 GetNamelessProgramArg(const ProgramArgs* args, uxx argIndex)
{
	uxx foundIndex = 0;
	VarArrayLoop(&args->args, aIndex)
	{
		VarArrayLoopGet(ProgramArg, arg, &args->args, aIndex);
		if (IsEmptyStr(arg->name))
		{
			if (foundIndex >= argIndex) { return arg->value; }
			foundIndex++;
		}
	}
	return Str8_Empty;
}

PEXP bool FindNamedProgramArgBoolEx(const ProgramArgs* args, Str8 name, Str8 otherName, bool defaultValue, uxx skipCount)
{
	uxx foundIndex = 0;
	VarArrayLoop(&args->args, aIndex)
	{
		VarArrayLoopGet(ProgramArg, arg, &args->args, aIndex);
		if (StrAnyCaseEquals(arg->name, name) || (!IsEmptyStr(otherName) && StrAnyCaseEquals(arg->name, otherName)))
		{
			if (foundIndex >= skipCount)
			{
				if (!IsEmptyStr(arg->value))
				{
					bool parsedValue = defaultValue;
					if (TryParseBool(arg->value, &parsedValue, nullptr))
					{
						return parsedValue;
					}
					else
					{
						PrintLine_W("Unable to parse argument value as bool: \"%.*s\"", StrPrint(arg->rawString));
						return defaultValue;
					}
				}
				else { return true; }
			}
			foundIndex++;
		}
	}
	return defaultValue;
}
PEXPI bool FindNamedProgramArgBool(const ProgramArgs* args, Str8 name, bool defaultValue)
{
	return FindNamedProgramArgBoolEx(args, name, Str8_Empty, defaultValue, 0);
}

PEXP Str8 FindNamedProgramArgStrEx(const ProgramArgs* args, Str8 name, Str8 otherName, Str8 defaultValue, uxx skipCount)
{
	uxx foundIndex = 0;
	VarArrayLoop(&args->args, aIndex)
	{
		VarArrayLoopGet(ProgramArg, arg, &args->args, aIndex);
		if (StrAnyCaseEquals(arg->name, name) || (!IsEmptyStr(otherName) && StrAnyCaseEquals(arg->name, otherName)))
		{
			if (foundIndex >= skipCount)
			{
				return arg->value;
			}
			foundIndex++;
		}
	}
	return defaultValue;
}
PEXPI Str8 FindNamedProgramArgStr(const ProgramArgs* args, Str8 name, Str8 otherName, Str8 defaultValue)
{
	return FindNamedProgramArgStrEx(args, name, otherName, defaultValue, 0);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _OS_PROGRAM_ARGS_H
