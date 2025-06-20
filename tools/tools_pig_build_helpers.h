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

#define FILENAME_MSVC_ENVIRONMENT "environment.txt"

static inline void InitializeMsvcIf(bool* isMsvcInitialized)
{
	if (*isMsvcInitialized == false)
	{
		PrintLine("Initializing MSVC Compiler...");
		
		CliArgList cmd = ZEROED;
		AddArgNt(&cmd, CLI_QUOTED_ARG, FILENAME_MSVC_ENVIRONMENT);
		
		int statusCode = RunCliProgram(StrLit("..\\init_msvc.bat"), &cmd); //this batch file runs VsDevCmd.bat and then dumps it's environment variables to environment.txt. We can then open and parse that file and change our environment to match what VsDevCmd.bat changed
		if (statusCode != 0)
		{
			PrintLine_E("Failed to initialize MSVC compiler! Status Code: %d", statusCode);
			exit(statusCode);
		}
		
		Str8 environmentFileContents = ZEROED;
		if (!TryReadFile(StrLit(FILENAME_MSVC_ENVIRONMENT), &environmentFileContents))
		{
			PrintLine_E("init_msvc.bat did not create \"%s\"! Or we can't open it for some reason", FILENAME_MSVC_ENVIRONMENT);
			exit(4);
		}
		ParseAndApplyEnvironmentVariables(environmentFileContents);
		free(environmentFileContents.chars);
		*isMsvcInitialized = true;
	}
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

void ScrapeShaderHeaderFileAndAddExtraInfo(Str8 headerPath, Str8 shaderPath)
{
	Str8 headerFileContents = ZEROED;
	if (!TryReadFile(headerPath, &headerFileContents)) { PrintLine_E("Failed to open %.*s for reading after creation!", headerPath.length, headerPath.chars); exit(4); }
	
	Str8 shaderName = ZEROED;
	StrArray shaderAttributes = ZEROED;
	StrArray shaderImages = ZEROED;
	StrArray shaderSamplers = ZEROED;
	StrArray shaderUniformBlocks = ZEROED;
	StrArray shaderUniforms = ZEROED;
	StrArray shaderUniformsBlockNames = ZEROED;
	
	bool insideUniformBlock = false;
	Str8 uniformBlockName = ZEROED;
	Str8 line = ZEROED;
	LineParser lineParser = NewLineParser(headerFileContents);
	while (LineParserGetLine(&lineParser, &line))
	{
		if (shaderName.length == 0)
		{
			if (IsShaderHeaderLine_Name(line, &shaderName))
			{
				assert(shaderName.length > 0);
				// PrintLine("Shader name: \"%.*s\"", shaderName.length, shaderName.chars);
			}
		}
		else if (insideUniformBlock)
		{
			Str8 uniformType = ZEROED;
			Str8 uniformName = ZEROED;
			if (IsShaderHeaderLine_UniformStructEnd(shaderName, uniformBlockName, line))
			{
				insideUniformBlock = false;
			}
			else if (IsShaderHeaderLine_UniformMember(line, &uniformType, &uniformName))
			{
				// PrintLine("Found uniform \"%.*s\" \"%.*s\"", uniformType.length, uniformType.chars, uniformName.length, uniformName.chars);
				AddStr(&shaderUniforms, uniformName);
				AddStr(&shaderUniformsBlockNames, uniformBlockName);
			}
		}
		else
		{
			Str8 name = ZEROED;
			if (IsShaderHeaderLine_Attribute(shaderName, line, &name))
			{
				// PrintLine("Found attribute \"%.*s\"", name.length, name.chars);
				AddStr(&shaderAttributes, name);
			}
			else if (IsShaderHeaderLine_Image(shaderName, line, &name))
			{
				// PrintLine("Found image \"%.*s\"", name.length, name.chars);
				AddStr(&shaderImages, name);
			}
			else if (IsShaderHeaderLine_Sampler(shaderName, line, &name))
			{
				// PrintLine("Found sampler \"%.*s\"", name.length, name.chars);
				AddStr(&shaderSamplers, name);
			}
			else if (IsShaderHeaderLine_UniformStruct(shaderName, line, &name))
			{
				// PrintLine("Found uniform block \"%.*s\"", name.length, name.chars);
				uniformBlockName = name;
				insideUniformBlock = true;
			}
		}
	}
	
	assert(shaderName.length > 0);
	
	Str8 shaderFullPath = GetFullPath(shaderPath, '/');
	Str8 escapedFullShaderPath = EscapeString(shaderFullPath, false);
	AppendToFile(headerPath, StrLit(
		"\n\n//NOTE: These lines were added by pig_build.exe\n"
		"//NOTE: Because an empty array is invalid in C, we always add at least one dummy entry to these definition #defines while the corresponding COUNT #define will remain 0\n"
		"#ifndef NO_ENTRIES_STR\n"
		"#define NO_ENTRIES_STR \"no_entries\"\n"
		"#endif\n"),
		true
	);
	AppendPrintToFile(headerPath,
		"#define %.*s_SHADER_FILE_PATH \"%.*s\"\n",
		shaderName.length, shaderName.chars,
		escapedFullShaderPath.length, escapedFullShaderPath.chars
	);
	
	//Attributes
	{
		AppendPrintToFile(headerPath,
			"#define %.*s_SHADER_ATTR_COUNT %u\n"
			"#define %.*s_SHADER_ATTR_DEFS { \\\n",
			shaderName.length, shaderName.chars,
			shaderAttributes.length,
			shaderName.length, shaderName.chars
		);
		free(shaderFullPath.chars);
		free(escapedFullShaderPath.chars);
		for (uxx attributeIndex = 0; attributeIndex < shaderAttributes.length; attributeIndex++)
		{
			Str8 attributeName = shaderAttributes.strings[attributeIndex];
			AppendPrintToFile(headerPath,
				"\t{ .name=\"%.*s\", .index=ATTR_%.*s_%.*s }, \\\n",
				attributeName.length, attributeName.chars,
				shaderName.length, shaderName.chars,
				attributeName.length, attributeName.chars
			);
		}
		if (shaderAttributes.length == 0) { AppendToFile(headerPath, StrLit("\t{ .name=NO_ENTRIES_STR, .index=0 } \\\n"), true); }
		AppendToFile(headerPath, StrLit("} // These should match ShaderAttributeDef struct found in gfx_shader.h\n"), true);
	}
	
	//Images
	{
		AppendPrintToFile(headerPath,
			"#define %.*s_SHADER_IMAGE_COUNT %u\n"
			"#define %.*s_SHADER_IMAGE_DEFS { \\\n",
			shaderName.length, shaderName.chars,
			shaderImages.length,
			shaderName.length, shaderName.chars
		);
		for (uxx imageIndex = 0; imageIndex < shaderImages.length; imageIndex++)
		{
			Str8 imageName = shaderImages.strings[imageIndex];
			AppendPrintToFile(headerPath,
				"\t{ .name=\"%.*s_%.*s\", .index=IMG_%.*s_%.*s }, \\\n",
				shaderName.length, shaderName.chars,
				imageName.length, imageName.chars,
				shaderName.length, shaderName.chars,
				imageName.length, imageName.chars
			);
		}
		if (shaderImages.length == 0) { AppendToFile(headerPath, StrLit("\t{ .name=NO_ENTRIES_STR, .index=0 } \\\n"), true); }
		AppendToFile(headerPath, StrLit("} // These should match ShaderImageDef struct found in gfx_shader.h\n"), true);
	}
	
	//Samplers
	{
		AppendPrintToFile(headerPath,
			"#define %.*s_SHADER_SAMPLER_COUNT %u\n"
			"#define %.*s_SHADER_SAMPLER_DEFS { \\\n",
			shaderName.length, shaderName.chars,
			shaderSamplers.length,
			shaderName.length, shaderName.chars
		);
		for (uxx samplerIndex = 0; samplerIndex < shaderSamplers.length; samplerIndex++)
		{
			Str8 samplerName = shaderSamplers.strings[samplerIndex];
			AppendPrintToFile(headerPath,
				"\t{ .name=\"%.*s_%.*s\", .index=SMP_%.*s_%.*s }, \\\n",
				shaderName.length, shaderName.chars,
				samplerName.length, samplerName.chars,
				shaderName.length, shaderName.chars,
				samplerName.length, samplerName.chars
			);
		}
		if (shaderSamplers.length == 0) { AppendToFile(headerPath, StrLit("\t{ .name=NO_ENTRIES_STR, .index=0 } \\\n"), true); }
		AppendToFile(headerPath, StrLit("} // These should match ShaderSamplerDef struct found in gfx_shader.h\n"), true);
	}
	
	//Uniforms
	{
		AppendPrintToFile(headerPath,
			"#define %.*s_SHADER_UNIFORM_COUNT %u\n"
			"#define %.*s_SHADER_UNIFORM_DEFS { \\\n",
			shaderName.length, shaderName.chars,
			shaderUniforms.length,
			shaderName.length, shaderName.chars
		);
		for (uxx uniformIndex = 0; uniformIndex < shaderUniforms.length; uniformIndex++)
		{
			Str8 uniformName = shaderUniforms.strings[uniformIndex];
			Str8 uniformBlockName = shaderUniformsBlockNames.strings[uniformIndex];
			AppendPrintToFile(headerPath,
				"\t{ .name=\"%.*s\", "
				".blockIndex=UB_%.*s_%.*s, "
				".offset=STRUCT_VAR_OFFSET(%.*s_%.*s_t, %.*s), "
				".size=STRUCT_VAR_SIZE(%.*s_%.*s_t, %.*s) }, \\\n",
				uniformName.length, uniformName.chars,
				shaderName.length, shaderName.chars,
				uniformBlockName.length, uniformBlockName.chars,
				shaderName.length, shaderName.chars,
				uniformBlockName.length, uniformBlockName.chars,
				uniformName.length, uniformName.chars,
				shaderName.length, shaderName.chars,
				uniformBlockName.length, uniformBlockName.chars,
				uniformName.length, uniformName.chars
			);
		}
		if (shaderUniforms.length == 0) { AppendToFile(headerPath, StrLit("\t{ .name=NO_ENTRIES_STR, .blockIndex=0, .offset=0 } \\\n"), true); }
		AppendToFile(headerPath, StrLit("} // These should match ShaderUniformDef struct found in gfx_shader.h\n"), true);
	}
	
	free(headerFileContents.chars);
}

#endif //  _TOOLS_PIG_BUILD_HELPERS_H
