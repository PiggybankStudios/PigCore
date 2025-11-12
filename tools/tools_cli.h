/*
File:   tools_cli.h
Author: Taylor Robbins
Date:   06\16\2025
Description:
	** Holds various things that help us build up arguments for a CLI program in a readable way,
	** and then join them together and run an external CLI tool with those arguments.
*/

#ifndef _TOOLS_CLI_H
#define _TOOLS_CLI_H

// +--------------------------------------------------------------+
// |                   Composing Argument Lists                   |
// +--------------------------------------------------------------+
// We have this string inside a bunch of #defines in places like tools_msvc_flags.h
// This allows us to replace that part of the argument string with an actual value, adding escaping if the argument is in quotes
#define CLI_VAL_STR      "[VAL]"
#define CLI_UNQUOTED_ARG "[VAL]"
#define CLI_QUOTED_ARG   "\"[VAL]\""
#if BUILDING_ON_WINDOWS
#define CLI_PIPE_OUTPUT_TO_FILE "> \"[VAL]\""
#else
#define CLI_PIPE_OUTPUT_TO_FILE "| \"[VAL]\""
#endif
#define CLI_ROOT_DIR "[ROOT]"

//When running a program on Linux/OSX/etc. we have to specify we want to run a program out of the current working directory with "./"
#if BUILDING_ON_WINDOWS
#define EXEC_PROGRAM_IN_FOLDER_PREFIX ""
#else
#define EXEC_PROGRAM_IN_FOLDER_PREFIX "./"
#endif

typedef plex CliArg CliArg;
plex CliArg
{
	Str8 format;
	Str8 value;
};

#define CLI_MAX_ARGS 256
typedef plex CliArgList CliArgList;
plex CliArgList
{
	Str8 rootDirPath;
	char pathSepChar;
	uxx numArgs;
	CliArg args[CLI_MAX_ARGS];
};

Str8 FormatArg(const CliArg* arg, Str8 rootDirPath, char pathSepChar)
{
	Str8 valTargetStr = StrLit(CLI_VAL_STR);
	Str8 formatStr = arg->format;
	Str8 valueStr = StrReplace(arg->value, StrLit(CLI_ROOT_DIR), rootDirPath, false);
	FixPathSlashes(valueStr, pathSepChar);
	
	uxx insertValIndex = formatStr.length;
	for (uxx cIndex = 0; cIndex + valTargetStr.length <= formatStr.length; cIndex++)
	{
		if (StrExactEquals(StrSlice(formatStr, cIndex, cIndex+valTargetStr.length), valTargetStr))
		{
			insertValIndex = cIndex;
			if (cIndex > 0 && cIndex + valTargetStr.length < formatStr.length &&
				formatStr.chars[cIndex-1] == '\"' && formatStr.chars[cIndex + valTargetStr.length] == '\"')
			{
				Str8 escapedString = EscapeString(valueStr, false);
				free(valueStr.chars);
				valueStr = escapedString;
			}
			break;
		}
	}
	if (valueStr.length > 0 && insertValIndex >= formatStr.length)
	{
		PrintLine_E("Tried to fill value in CLI argument that doesn't take a value! %.*s", formatStr.length, formatStr.chars);
		exit(4);
	}
	if (valueStr.length == 0 && insertValIndex < formatStr.length)
	{
		PrintLine_E("Missing value in CLI argument that takes a value! %.*s - %.*s - %.*s", formatStr.length, formatStr.chars, valueStr.length, valueStr.chars, arg->value.length, arg->value.chars);
		// PrintLine_E("There are %u arguments in this list:", list->numArgs);
		// for (uxx aIndex = 0; aIndex < list->numArgs; aIndex++) { PrintLine_E("\t[%u] \"%.*s\"", aIndex, list->args[aIndex].length, list->args[aIndex].chars); }
		exit(4);
	}
	
	Str8 result = CopyStr8(formatStr, false);
	if (insertValIndex < formatStr.length)
	{
		Str8 cliLeftPart = StrSlice(formatStr, 0, insertValIndex);
		Str8 cliRightPart = StrSliceFrom(formatStr, insertValIndex + valTargetStr.length);
		Str8 joinedStr = JoinStrings3(cliLeftPart, valueStr, cliRightPart, true);
		free(result.chars);
		result = joinedStr;
	}
	free(valueStr.chars);
	
	return result;
}

void AddArgStr(CliArgList* list, const char* formatStrNt, Str8 valueStr)
{
	if (list->numArgs >= CLI_MAX_ARGS) { WriteLine_E("Too many CLI arguments!"); exit(4); }
	list->args[list->numArgs].format = CopyStr8(MakeStr8Nt(formatStrNt), false);
	list->args[list->numArgs].value = CopyStr8(valueStr, false);
	list->numArgs++;
}
void AddArgNt(CliArgList* list, const char* formatStrNt, const char* valueStr)
{
	AddArgStr(list, formatStrNt, MakeStr8Nt(valueStr));
}
void AddArgInt(CliArgList* list, const char* formatStrNt, int32_t valueInt)
{
	char printBuffer[12];
	int printResult = snprintf(&printBuffer[0], 12, "%d", valueInt);
	printBuffer[printResult] = '\0';
	AddArgStr(list, formatStrNt, MakeStr8((uxx)printResult, &printBuffer[0]));
}
void AddArg(CliArgList* list, const char* formatStrNt)
{
	AddArgStr(list, formatStrNt, MakeStr8(0, nullptr));
}

void AddArgList(CliArgList* dest, const CliArgList* source)
{
	if (dest->numArgs + source->numArgs > CLI_MAX_ARGS) { WriteLine_E("Too many CLI arguments!"); exit(4); }
	for (uxx aIndex = 0; aIndex < source->numArgs; aIndex++)
	{
		dest->args[dest->numArgs].format = CopyStr8(source->args[aIndex].format, false);
		dest->args[dest->numArgs].value = CopyStr8(source->args[aIndex].value, false);
		dest->numArgs++;
	}
}

Str8 JoinCliArgsList(Str8 prefix, const CliArgList* list, bool addNullTerm)
{
	char pathSepChar = list->pathSepChar;
	if (pathSepChar == '\0') { pathSepChar = PATH_SEP_CHAR; }
	Str8 rootDirPath = ZEROED;
	if (list->rootDirPath.length == 0) { rootDirPath = CopyStr8(StrLit(".."), false); }
	else { rootDirPath = CopyStr8(list->rootDirPath, false); }
	FixPathSlashes(rootDirPath, pathSepChar);
	
	Str8* formattedStrings = (list->numArgs > 0) ? (Str8*)malloc(sizeof(Str8) * list->numArgs) : nullptr;
	uxx totalLength = prefix.length;
	for (uxx aIndex = 0; aIndex < list->numArgs; aIndex++)
	{
		formattedStrings[aIndex] = FormatArg(&list->args[aIndex], rootDirPath, pathSepChar);
		if (formattedStrings[aIndex].length > 0)
		{
			if (totalLength > 0) { totalLength++; } //+1 for space between arguments
			totalLength += formattedStrings[aIndex].length;
		}
	}
	free(rootDirPath.chars);
	
	Str8 result;
	result.length = totalLength;
	result.pntr = malloc(result.length + (addNullTerm ? 1 : 0));
	
	uxx writeIndex = 0;
	memcpy(&result.chars[writeIndex], &prefix.chars[0], prefix.length); writeIndex += prefix.length;
	
	for (uxx aIndex = 0; aIndex < list->numArgs; aIndex++)
	{
		if (formattedStrings[aIndex].length > 0)
		{
			if (writeIndex > 0)
			{
				result.chars[writeIndex] = ' ';
				writeIndex++;
			}
			
			memcpy(&result.chars[writeIndex], formattedStrings[aIndex].chars, formattedStrings[aIndex].length);
			writeIndex += formattedStrings[aIndex].length;
		}
	}
	
	if (addNullTerm) { result.chars[writeIndex] = '\0'; }
	return result;
}

// +--------------------------------------------------------------+
// |                      Running CLI Tools                       |
// +--------------------------------------------------------------+

#include <stdlib.h>

int RunCliProgram(Str8 programName, const CliArgList* args)
{
	Str8 joinedArgs = JoinCliArgsList(programName, args, true);
	// PrintLine(">> %s", joinedArgs.chars);
	fflush(stdout);
	fflush(stderr);
	int resultCode = system(joinedArgs.chars);
	free(joinedArgs.chars);
	return resultCode;
}
void RunCliProgramAndExitOnFailure(Str8 programName, const CliArgList* args, Str8 errorMessage)
{
	int statusCode = RunCliProgram(programName, args);
	if (statusCode != 0)
	{
		Str8 programNamePart = GetFileNamePart(programName, true);
		PrintLine_E("%.*s\n%.*s Status Code: %d",
			errorMessage.length, errorMessage.chars,
			programNamePart.length, programNamePart.chars,
			statusCode
		);
		exit(statusCode);
	}
}

void ParseAndApplyEnvironmentVariables(Str8 environmentVars)
{
	uxx lineIndex = 0;
	uxx lineStart = 0;
	uxx equalsIndex = 0;
	for (uxx cIndex = 0; cIndex < environmentVars.length; cIndex++)
	{
		char character = environmentVars.chars[cIndex];
		char nextChar = (cIndex+1 < environmentVars.length) ? environmentVars.chars[cIndex+1] : '\0';
		if (character == '\n' || (character == '\r' && nextChar == '\n'))
		{
			Str8 line = MakeStr8(cIndex - lineStart, &environmentVars.chars[lineStart]);
			
			if (equalsIndex >= lineStart)
			{
				Str8 varName = StrSlice(line, 0, equalsIndex-lineStart);
				Str8 varValue = StrSliceFrom(line, (equalsIndex-lineStart)+1);
				
				// PrintLine("set %.*s=%.*s", varName.length, varName.chars, varValue.length, varValue.chars);
				varName = CopyStr8(varName, true);
				varValue = CopyStr8(varValue, true);
				#if BUILDING_ON_WINDOWS
				_putenv_s(varName.chars, varValue.chars);
				#else
				Str8 varEqualsValueStr = JoinStrings3(varName, StrLit("="), varValue, true);
				putenv(varEqualsValueStr.chars);
				#endif
				free(varName.chars);
				free(varValue.chars);
			}
			else if (line.length > 0)
			{
				PrintLine_E("WARNING: No \'=\' character found in line %u of environment file. Ignoring line: \"%.*s\"", lineIndex+1, line.length, line.chars);
			}
			
			if (character == '\r' && nextChar == '\n') { cIndex++; }
			lineStart = cIndex + 1;
			lineIndex++;
		}
		if (character == '=') { equalsIndex = cIndex; }
	}
}

bool WasMsvcDevBatchRun()
{
	const char* versionEnvVarValue = getenv("VSCMD_VER");
    return (versionEnvVarValue != nullptr);
}
bool WasEmsdkEnvBatchRun()
{
	const char* sdkEnvVarValue = getenv("EMSDK");
    return (sdkEnvVarValue != nullptr);
}

#endif //  _TOOLS_CLI_H
