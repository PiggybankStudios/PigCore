/*
File:   tools_build_helpers.h
Author: Taylor Robbins
Date:   06\19\2025
*/

#ifndef _TOOLS_BUILD_HELPERS_H
#define _TOOLS_BUILD_HELPERS_H

static inline Str8 ExtractStrDefine(Str8 buildConfigContents, Str8 defineName)
{
	Str8 defineValueStr = ZEROED;
	if (!TryExtractDefineFrom(buildConfigContents, defineName, &defineValueStr))
	{
		PrintLine_E("Couldn't find #define %.*s in build_config.h!", defineName.length, defineName.chars);
		exit(4);
	}
	return defineValueStr;
}
static inline bool ExtractBoolDefine(Str8 buildConfigContents, Str8 defineName)
{
	Str8 defineValueStr = ExtractStrDefine(buildConfigContents, defineName);
	bool result = false;
	if (!TryParseBoolArg(defineValueStr, &result))
	{
		PrintLine_E("#define %.*s has a non-bool value: \"%.*s\"", defineName.length, defineName.chars, defineValueStr.length, defineValueStr.chars);
		exit(4);
	}
	return result;
}

static inline void RunBatchFileAndApplyDumpedEnvironment(Str8 batchFilePath, Str8 environmentFilePath, bool skipRunningIfFileExists)
{
	CliArgList cmd = ZEROED;
	AddArgStr(&cmd, CLI_QUOTED_ARG, environmentFilePath);
	Str8 fixedBatchFilePath = CopyStr8(batchFilePath, false);
	FixPathSlashes(fixedBatchFilePath, PATH_SEP_CHAR);
	
	if (!DoesFileExist(environmentFilePath) || !skipRunningIfFileExists)
	{
		int statusCode = RunCliProgram(fixedBatchFilePath, &cmd); //this batch file runs emsdk_env.bat and then dumps it's environment variables to environment.txt. We can then open and parse that file and change our environment to match what emsdk_env.bat changed
		if (statusCode != 0)
		{
			PrintLine_E("%.*s failed! Status Code: %d", fixedBatchFilePath.length, fixedBatchFilePath.chars, statusCode);
			exit(statusCode);
		}
	}
	
	Str8 environmentFileContents = ZEROED;
	if (!TryReadFile(environmentFilePath, &environmentFileContents))
	{
		PrintLine_E("%.*s did not create \"%.*s\"! Or we can't open it for some reason", batchFilePath.length, batchFilePath.chars, environmentFilePath.length, environmentFilePath.chars);
		exit(4);
	}
	
	ParseAndApplyEnvironmentVariables(environmentFileContents);
	
	free(fixedBatchFilePath.chars);
	free(environmentFileContents.chars);
}

static inline void InitializeMsvcIf(Str8 pigCoreFolder, bool* isMsvcInitialized)
{
	if (*isMsvcInitialized == false)
	{
		PrintLine("Initializing MSVC Compiler...");
		Str8 batchPath = JoinStrings2(pigCoreFolder, StrLit("/init_msvc.bat"), false);
		RunBatchFileAndApplyDumpedEnvironment(batchPath, StrLit("msvc_environment.txt"), true);
		*isMsvcInitialized = true;
	}
}

static inline void InitializeEmsdkIf(Str8 pigCoreFolder, bool* isEmsdkInitialized)
{
	if (*isEmsdkInitialized == false)
	{
		PrintLine("Initializing Emscripten SDK...");
		Str8 batchPath = JoinStrings2(pigCoreFolder, StrLit("/init_emsdk.bat"), false);
		RunBatchFileAndApplyDumpedEnvironment(batchPath, StrLit("emsdk_environment.txt"), false);
		*isEmsdkInitialized = true;
	}
}

static inline void ConcatAllFilesIntoSingleFile(const StrArray* pathArray, Str8 outputFilePath)
{
	//TODO: We really should handle new-line differences between Windows and Linux/etc. a little smarter here
	//      Just because we are building on Windows doesn't mean all these .js files are using Windows style line-endings
	
	StrArray allFilesContents = ZEROED;
	uxx totalLength = 0;
	for (uxx fIndex = 0; fIndex < pathArray->length; fIndex++)
	{
		Str8 inputPath = pathArray->strings[fIndex];
		Str8 inputFileContents = ZEROED;
		if (!TryReadFile(inputPath, &inputFileContents))
		{
			PrintLine_E("Couldn't find/open \"%.*s\"!", inputPath.length, inputPath.chars);
			exit(8);
		}
		AddStr(&allFilesContents, inputFileContents);
		if (totalLength > 0) { totalLength += BUILDING_ON_WINDOWS ? 2 : 1; } //+1-2 for the new-line between each file
		totalLength += inputFileContents.length;
		free(inputFileContents.chars);
	}
	
	Str8 combinedContents = ZEROED;
	combinedContents.length = totalLength;
	combinedContents.pntr = malloc(combinedContents.length + 1);
	
	uxx writeIndex = 0;
	for (uxx fIndex = 0; fIndex < allFilesContents.length; fIndex++)
	{
		Str8 inputFileContents = allFilesContents.strings[fIndex];
		if (writeIndex > 0)
		{
			#if BUILDING_ON_WINDOWS
			combinedContents.chars[writeIndex+0] = '\r';
			combinedContents.chars[writeIndex+1] = '\n';
			writeIndex += 2;
			#else
			combinedContents.chars[writeIndex] = '\n';
			writeIndex += 1;
			#endif
		}
		memcpy(&combinedContents.chars[writeIndex], inputFileContents.chars, inputFileContents.length);
		writeIndex += inputFileContents.length;
	}
	assert(writeIndex == combinedContents.length);
	combinedContents.chars[combinedContents.length] = '\0';
	
	CreateAndWriteFile(outputFilePath, combinedContents, false);
	
	FreeStrArray(&allFilesContents);
}

static inline Str8 GetEmscriptenSdkPath()
{
	const char* sdkEnvVariable = getenv("EMSCRIPTEN_SDK_PATH");
	if (sdkEnvVariable == nullptr)
	{
		WriteLine_E("Please set the EMSCRIPTEN_SDK_PATH environment variable before trying to build for the web with USE_EMSCRIPTEN");
		exit(7);
	}
	Str8 result = NewStr8Nt(sdkEnvVariable);
	if (IS_SLASH(result.chars[result.length-1])) { result.length--; } //no trailing slash
	result = CopyStr8(result, true);
	FixPathSlashes(result, PATH_SEP_CHAR);
	return result;
}

#define FILENAME_ORCA_SDK_PATH  "orca_sdk_path.txt"

static inline Str8 GetOrcaSdkPath()
{
	CliArgList cmd = ZEROED;
	AddArg(&cmd, "sdk-path");
	AddArgNt(&cmd, CLI_PIPE_OUTPUT_TO_FILE, FILENAME_ORCA_SDK_PATH);
	int statusCode = RunCliProgram(StrLit("orca"), &cmd);
	if (statusCode != 0)
	{
		PrintLine_E("Failed to run \"orca sdk-path\"! Status code: %d", statusCode);
		WriteLine_E("Make sure Orca SDK is installed and is added to the PATH!");
		exit(statusCode);
	}
	AssertFileExist(StrLit(FILENAME_ORCA_SDK_PATH), false);
	Str8 result = ZEROED;
	bool readSuccess = TryReadFile(StrLit(FILENAME_ORCA_SDK_PATH), &result);
	assert(readSuccess == true);
	assert(result.length > 0);
	FixPathSlashes(result, PATH_SEP_CHAR);
	if (result.chars[result.length-1] == PATH_SEP_CHAR) { result.length--; } //no trailing slash
	return result;
}

static inline Str8 GetPlaydateSdkPath()
{
	const char* sdkEnvVariable = getenv("PLAYDATE_SDK_PATH");
	if (sdkEnvVariable == nullptr)
	{
		WriteLine_E("Please set the PLAYDATE_SDK_PATH environment variable before trying to build for the Playdate");
		exit(7);
	}
	Str8 result = NewStr8Nt(sdkEnvVariable);
	if (IS_SLASH(result.chars[result.length-1])) { result.length--; }
	result = CopyStr8(result, true);
	FixPathSlashes(result, PATH_SEP_CHAR);
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

void ScrapeShaderHeaderFileAndAddExtraInfo(Str8 headerPath, Str8 shaderPath)
{
	Str8 headerFileContents = ReadEntireFile(headerPath);
	
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
		AppendToFile(headerPath, StrLit("} // These should match ShaderAttributeDef plex found in gfx_shader.h\n"), true);
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
		AppendToFile(headerPath, StrLit("} // These should match ShaderImageDef plex found in gfx_shader.h\n"), true);
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
		AppendToFile(headerPath, StrLit("} // These should match ShaderSamplerDef plex found in gfx_shader.h\n"), true);
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
		AppendToFile(headerPath, StrLit("} // These should match ShaderUniformDef plex found in gfx_shader.h\n"), true);
	}
	
	free(headerFileContents.chars);
}

typedef plex FindShadersContext FindShadersContext;
plex FindShadersContext
{
	uxx ignoreListLength;
	Str8* ignoreList;
	StrArray shaderPaths;
	StrArray headerPaths;
	StrArray sourcePaths;
	StrArray objPaths;
	StrArray oPaths;
};

// +==============================+
// |   FindShaderFilesCallback    |
// +==============================+
// bool FindShaderFilesCallback(Str8 path, bool isFolder, void* contextPntr)
RECURSIVE_DIR_WALK_CALLBACK_DEF(FindShaderFilesCallback)
{
	FindShadersContext* context = (FindShadersContext*)contextPntr;
	if (isFolder)
	{
		for (uxx iIndex = 0; iIndex < context->ignoreListLength; iIndex++)
		{
			if (StrExactContains(path, context->ignoreList[iIndex])) { return false; }
		}
	}
	
	if (!isFolder && StrExactEndsWith(path, StrLit(".glsl")))
	{
		Str8 shaderName = GetFileNamePart(path, false);
		Str8 rootPath = StrReplace(path, StrLit(".."), StrLit("[ROOT]"), false);
		FixPathSlashes(rootPath, '/');
		AddStr(&context->shaderPaths, rootPath);
		AddStr(&context->headerPaths, JoinStrings2(rootPath, StrLit(".h"), true));
		AddStr(&context->sourcePaths, JoinStrings2(rootPath, StrLit(".c"), true));
		AddStr(&context->objPaths, JoinStrings2(shaderName, StrLit(".obj"), true));
		AddStr(&context->oPaths, JoinStrings2(shaderName, StrLit(".o"), true));
	}
	return true;
}

#endif //  _TOOLS_BUILD_HELPERS_H
