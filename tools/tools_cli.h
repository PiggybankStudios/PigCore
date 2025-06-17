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
#define CLI_VAL_STR "[VAL]"

#define CLI_MAX_ARGS 256
typedef struct CliArgList CliArgList;
struct CliArgList
{
	uxx numArgs;
	Str8 args[CLI_MAX_ARGS];
};

void AddArgStr(CliArgList* list, const char* cliStrNt, Str8 valueStr)
{
	if (list->numArgs >= CLI_MAX_ARGS) { WriteLine_E("Too many CLI arguments!"); exit(4); }
	
	Str8 cliStr = NewStr8Nt(cliStrNt);
	Str8 valTargetStr = StrLit(CLI_VAL_STR);
	
	uxx insertValIndex = cliStr.length;
	for (uxx cIndex = 0; cIndex + valTargetStr.length <= cliStr.length; cIndex++)
	{
		if (StrExactEquals(StrSlice(cliStr, cIndex, cIndex+valTargetStr.length), valTargetStr))
		{
			insertValIndex = cIndex;
			if (cIndex > 0 && cIndex + valTargetStr.length < cliStr.length &&
				cliStr.chars[cIndex-1] == '\"' && cliStr.chars[cIndex + valTargetStr.length] == '\"')
			{
				valueStr = EscapeString(valueStr, false);
			}
			break;
		}
	}
	if (valueStr.length > 0 && insertValIndex == cliStr.length)
	{
		PrintLine_E("Tried to fill value in CLI argument that doesn't take a value! %s", cliStrNt);
		exit(4);
	}
	if (valueStr.length == 0 && insertValIndex < cliStr.length)
	{
		PrintLine_E("Missing value in CLI argument that takes a value! %s", cliStrNt);
		exit(4);
	}
	
	if (insertValIndex < cliStr.length)
	{
		Str8 cliLeftPart = StrSlice(cliStr, 0, insertValIndex);
		Str8 cliRightPart = StrSliceFrom(cliStr, insertValIndex + valTargetStr.length);
		cliStr = JoinStrings3(cliLeftPart, valueStr, cliRightPart, true);
	}
	
	list->args[list->numArgs] = cliStr;
	list->numArgs++;
}
void AddArgNt(CliArgList* list, const char* cliStr, const char* valueStr)
{
	AddArgStr(list, cliStr, NewStr8Nt(valueStr));
}
void AddArgInt(CliArgList* list, const char* cliStr, int32_t valueInt)
{
	char printBuffer[12];
	int printResult = snprintf(&printBuffer[0], 12, "%d", valueInt);
	printBuffer[printResult] = '\0';
	AddArgStr(list, cliStr, NewStr8((uxx)printResult, &printBuffer[0]));
}
void AddArg(CliArgList* list, const char* cliStr)
{
	AddArgStr(list, cliStr, NewStr8(0, nullptr));
}

void AddArgList(CliArgList* dest, const CliArgList* source)
{
	if (dest->numArgs + source->numArgs > CLI_MAX_ARGS) { WriteLine_E("Too many CLI arguments!"); exit(4); }
	
	for (uxx aIndex = 0; aIndex < source->numArgs; aIndex++)
	{
		dest->args[dest->numArgs] = CopyStr8(source->args[aIndex], false);
		dest->numArgs++;
	}
}

Str8 JoinCliArgsList(Str8 prefix, const CliArgList* list, bool addNullTerm)
{
	uxx totalLength = prefix.length;
	for (uxx aIndex = 0; aIndex < list->numArgs; aIndex++)
	{
		if (list->args[aIndex].length > 0)
		{
			if (totalLength > 0) { totalLength++; } //+1 for space between arguments
			totalLength += list->args[aIndex].length;
		}
	}
	
	Str8 result;
	result.length = totalLength;
	result.pntr = malloc(result.length + (addNullTerm ? 1 : 0));
	
	uxx writeIndex = 0;
	memcpy(&result.chars[writeIndex], &prefix.chars[0], prefix.length); writeIndex += prefix.length;
	
	for (uxx aIndex = 0; aIndex < list->numArgs; aIndex++)
	{
		if (list->args[aIndex].length > 0)
		{
			if (writeIndex > 0)
			{
				result.chars[writeIndex] = ' ';
				writeIndex++;
			}
			
			memcpy(&result.chars[writeIndex], list->args[aIndex].chars, list->args[aIndex].length);
			writeIndex += list->args[aIndex].length;
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
	return resultCode;
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
			Str8 line = NewStr8(cIndex - lineStart, &environmentVars.chars[lineStart]);
			
			if (equalsIndex >= lineStart)
			{
				Str8 varName = StrSlice(line, 0, equalsIndex-lineStart);
				Str8 varValue = StrSliceFrom(line, (equalsIndex-lineStart)+1);
				
				// PrintLine("set %.*s=%.*s", varName.length, varName.chars, varValue.length, varValue.chars);
				varName = CopyStr8(varName, true);
				varValue = CopyStr8(varValue, true);
				_putenv_s(varName.chars, varValue.chars);
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

#endif //  _TOOLS_CLI_H
