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

#define CONSUME_WHITESPACE(linePntr) do                                                             \
{                                                                                                   \
	while ((linePntr)->length > 0 && ((linePntr)->chars[0] == ' ' || (linePntr)->chars[0] == '\t')) \
	{                                                                                               \
		(linePntr)->chars++;                                                                        \
		(linePntr)->length--;                                                                       \
	}                                                                                               \
} while(0)
#define CONSUME_STR(linePntr, expectedStr) do                              \
{                                                                          \
	if (!StrExactStartsWith(*(linePntr), (expectedStr))) { return false; } \
	*(linePntr) = StrSliceFrom(*(linePntr), (expectedStr).length);         \
} while(0)

static inline bool IsShaderHeaderLine_Name(Str8 line, Str8* nameOut)
{
	CONSUME_WHITESPACE(&line);
	CONSUME_STR(&line, StrLit("Shader program: \'"));
	//TODO: Shader program: 'main2d':
	return false;
}
static inline bool IsShaderHeaderLine_Attribute(Str8 shaderName, Str8 line, Str8* nameOut)
{
	//TODO: #define ATTR_main2d_position (0)
	return false;
}
static inline bool IsShaderHeaderLine_UniformStruct(Str8 shaderName, Str8 line, Str8* nameOut)
{
	//TODO: SOKOL_SHDC_ALIGN(16) typedef struct main2d_VertParams_t {
	return false;
}
static inline bool IsShaderHeaderLine_UniformMember(Str8 shaderName, Str8 line, Str8* typeOut, Str8* nameOut)
{
	//TODO: mat4 world;
	return false;
}

#endif //  _TOOLS_PIG_BUILD_HELPERS_H
