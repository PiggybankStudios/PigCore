/*
File:   tools_pig_build_helpers.h
Author: Taylor Robbins
Date:   06\19\2025
*/

#ifndef _TOOLS_PIG_BUILD_HELPERS_H
#define _TOOLS_PIG_BUILD_HELPERS_H

static inline bool ExtractBoolDefine(Str8 buildConfigContents, Str8 defineName)
{
	Str8 defineValueStr = ZEROED;
	if (!TryExtractDefineFrom(buildConfigContents, defineName, &defineValueStr))
	{
		PrintLine_E("Couldn't find #define %.*s in build_config.h!", defineName.length, defineName.chars);
		exit(4);
	}
	bool result = false;
	if (!TryParseBoolArg(defineValueStr, &result))
	{
		PrintLine_E("#define %.*s has a non-bool value: \"%.*s\"", defineName.length, defineName.chars, defineValueStr.length, defineValueStr.chars);
		exit(4);
	}
	return result;
}

// +--------------------------------------------------------------+
// |                  Shader Header File Parsing                  |
// +--------------------------------------------------------------+

//NOTE: The macros and functions below act sort of like regular expressions.
//      For example, CONSUME_WHITESPACE is similar to \s* in RE syntax.

#define CONSUME_WHITESPACE(linePntr) do                                \
{                                                                      \
	while ((linePntr)->length > 0 &&                                   \
		((linePntr)->chars[0] == ' ' || (linePntr)->chars[0] == '\t')) \
	{                                                                  \
		(linePntr)->chars++;                                           \
		(linePntr)->length--;                                          \
	}                                                                  \
} while(0)

#define CONSUME_STR(linePntr, expectedStr) do                              \
{                                                                          \
	if (!StrExactStartsWith(*(linePntr), (expectedStr))) { return false; } \
	*(linePntr) = StrSliceFrom(*(linePntr), (expectedStr).length);         \
} while(0)

#define CONSUME_NT_STR(linePntr, expectedStrNt) do \
{                                                  \
	Str8 expectedStr = StrLit(expectedStrNt);      \
	CONSUME_STR((linePntr), expectedStr);          \
} while(0)

#define CONSUME_UNTIL(linePntr, expectedStr) do                        \
{                                                                      \
	while((linePntr)->length > 0)                                      \
	{                                                                  \
		if (StrExactStartsWith(*(linePntr), (expectedStr))) { break; } \
		(linePntr)->chars++;                                           \
		(linePntr)->length--;                                          \
	}                                                                  \
} while(0)

#define CONSUME_UNTIL_CHARS(linePntr, expectedCharsStr) do                                                   \
{                                                                                                            \
	while((linePntr)->length > 0)                                                                            \
	{                                                                                                        \
		bool isInExpectedStr = false;                                                                        \
		for (uxx cIndex = 0; cIndex < (expectedCharsStr).length; cIndex++)                                   \
		{                                                                                                    \
			if ((linePntr)->chars[0] == (expectedCharsStr).chars[cIndex]) { isInExpectedStr = true; break; } \
		}                                                                                                    \
		if (isInExpectedStr) { break; }                                                                      \
		(linePntr)->chars++;                                                                                 \
		(linePntr)->length--;                                                                                \
	}                                                                                                        \
} while(0)

#define CONSUME_UNTIL_NOT_CHARS(linePntr, expectedCharsStr) do                                               \
{                                                                                                            \
	while((linePntr)->length > 0)                                                                            \
	{                                                                                                        \
		bool isInExpectedStr = false;                                                                        \
		for (uxx cIndex = 0; cIndex < (expectedCharsStr).length; cIndex++)                                   \
		{                                                                                                    \
			if ((linePntr)->chars[0] == (expectedCharsStr).chars[cIndex]) { isInExpectedStr = true; break; } \
		}                                                                                                    \
		if (!isInExpectedStr) { break; }                                                                     \
		(linePntr)->chars++;                                                                                 \
		(linePntr)->length--;                                                                                \
	}                                                                                                        \
} while(0)

#define UPPERCASE_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWERCASE_CHARS "abcdefghijklmnopqrstuvwxyz"
#define NUMBER_CHARS "0123456789"
#define IDENTIFIER_CHARS "_" NUMBER_CHARS UPPERCASE_CHARS LOWERCASE_CHARS

static inline bool IsShaderHeaderLine_Name(Str8 line, Str8* nameOut)
{
	//Matches something like:
	//	Shader program: 'main2d':
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "Shader program:");
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "\'");
	Str8 nameStr = line;
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(IDENTIFIER_CHARS));
	if (line.chars == nameStr.chars) { return false; }
	nameStr.length = (uxx)(line.chars - nameStr.chars);
	CONSUME_NT_STR(&line, "\':");
	CONSUME_WHITESPACE(&line);
	if (line.length > 0) { return false; }
	if (nameOut != nullptr) { *nameOut = nameStr; }
	return true;
}

static inline bool IsShaderHeaderLine_Attribute(Str8 shaderName, Str8 line, Str8* nameOut)
{
	//Matches something like:
	//	#define ATTR_main2d_position (0)
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "#define ATTR_");
	CONSUME_STR(&line, shaderName);
	CONSUME_NT_STR(&line, "_");
	Str8 nameStr = line;
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(IDENTIFIER_CHARS));
	if (line.chars == nameStr.chars) { return false; }
	nameStr.length = (uxx)(line.chars - nameStr.chars);
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "(");
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(NUMBER_CHARS));
	CONSUME_NT_STR(&line, ")");
	CONSUME_WHITESPACE(&line);
	if (line.length > 0) { return false; }
	if (nameOut != nullptr) { *nameOut = nameStr; }
	return true;
}

static inline bool IsShaderHeaderLine_Image(Str8 shaderName, Str8 line, Str8* nameOut)
{
	//Matches something like:
	//	#define IMG_main2d_texture0 (0)
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "#define IMG_");
	CONSUME_STR(&line, shaderName);
	CONSUME_NT_STR(&line, "_");
	Str8 nameStr = line;
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(IDENTIFIER_CHARS));
	if (line.chars == nameStr.chars) { return false; }
	nameStr.length = (uxx)(line.chars - nameStr.chars);
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "(");
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(NUMBER_CHARS));
	CONSUME_NT_STR(&line, ")");
	CONSUME_WHITESPACE(&line);
	if (line.length > 0) { return false; }
	if (nameOut != nullptr) { *nameOut = nameStr; }
	return true;
}

static inline bool IsShaderHeaderLine_Sampler(Str8 shaderName, Str8 line, Str8* nameOut)
{
	//Matches something like:
	//	#define SMP_main2d_sampler0 (0)
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "#define SMP_");
	CONSUME_STR(&line, shaderName);
	CONSUME_NT_STR(&line, "_");
	Str8 nameStr = line;
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(IDENTIFIER_CHARS));
	if (line.chars == nameStr.chars) { return false; }
	nameStr.length = (uxx)(line.chars - nameStr.chars);
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "(");
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(NUMBER_CHARS));
	CONSUME_NT_STR(&line, ")");
	CONSUME_WHITESPACE(&line);
	if (line.length > 0) { return false; }
	if (nameOut != nullptr) { *nameOut = nameStr; }
	return true;
}

static inline bool IsShaderHeaderLine_UniformStruct(Str8 shaderName, Str8 line, Str8* nameOut)
{
	//Matches something like:
	//	SOKOL_SHDC_ALIGN(16) typedef struct main2d_VertParams_t {
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "SOKOL_SHDC_ALIGN(");
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(NUMBER_CHARS));
	CONSUME_NT_STR(&line, ")");
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "typedef");
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "struct");
	CONSUME_WHITESPACE(&line);
	CONSUME_STR(&line, shaderName);
	CONSUME_NT_STR(&line, "_");
	Str8 nameStr = line;
	CONSUME_UNTIL(&line, StrLit("_t"));
	if (line.chars == nameStr.chars) { return false; }
	nameStr.length = (uxx)(line.chars - nameStr.chars);
	CONSUME_NT_STR(&line, "_t");
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "{");
	CONSUME_WHITESPACE(&line);
	if (line.length > 0) { return false; }
	if (nameOut != nullptr) { *nameOut = nameStr; }
	return true;
}
static inline bool IsShaderHeaderLine_UniformStructEnd(Str8 shaderName, Str8 uniformBlockName, Str8 line)
{
	//Matches something like:
	//	} main2d_VertParams_t;
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "}");
	CONSUME_WHITESPACE(&line);
	CONSUME_STR(&line, shaderName);
	CONSUME_NT_STR(&line, "_");
	CONSUME_STR(&line, uniformBlockName);
	CONSUME_NT_STR(&line, "_t;");
	CONSUME_WHITESPACE(&line);
	if (line.length > 0) { return false; }
	return true;
}
static inline bool IsShaderHeaderLine_UniformMember(Str8 line, Str8* typeOut, Str8* nameOut)
{
	//Matches something like:
	//	mat4 world;
	CONSUME_WHITESPACE(&line);
	Str8 typeStr = line;
	CONSUME_UNTIL_CHARS(&line, StrLit(" \t"));
	if (line.chars == typeStr.chars) { return false; }
	typeStr.length = (uxx)(line.chars - typeStr.chars);
	CONSUME_WHITESPACE(&line);
	Str8 nameStr = line;
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(IDENTIFIER_CHARS));
	if (line.chars == nameStr.chars) { return false; }
	nameStr.length = (uxx)(line.chars - nameStr.chars);
	CONSUME_NT_STR(&line, ";");
	CONSUME_WHITESPACE(&line);
	if (line.length > 0) { return false; }
	if (typeOut != nullptr) { *typeOut = typeStr; }
	if (nameOut != nullptr) { *nameOut = nameStr; }
	return true;
}

#endif //  _TOOLS_PIG_BUILD_HELPERS_H
