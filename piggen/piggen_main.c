/*
File:   piggen_main.c
Author: Taylor Robbins
Date:   12\31\2024
Description: 
	** Holds the main entry point for piggen.exe which is a tool that scrapes C(++)
	** files, searching for special syntax, in order to generate code and modify
	** the source file(s) to #include the generated code
*/

#include "build_config.h"
#if !BUILD_FOR_PIGGEN
#error The wrong build_config.h was found!
#endif

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "std/std_includes.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "misc/misc_result.h"
#include "os/os_file.h"
#include "os/os_virtual_mem.h"
#include "std/std_memset.h"
#include "std/std_malloc.h"
#include "std/std_basic_math.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "base/base_unicode.h"
#include "struct/struct_string.h"
#include "os/os_path.h"
#include "misc/misc_parsing.h"
#include "os/os_program_args.h"
#include "struct/struct_var_array.h"
#include "misc/misc_simple_parsers.h"
#include "parse/parse_metadesk.h"

#include "base/base_debug_output_impl.h"

const char* SourceFileExtensions[] = {
	".c",
	".cpp",
	".h",
	".hpp",
};

typedef struct SnippetPiece SnippetPiece;
struct SnippetPiece
{
	MD_Node* rootNode;
	Str8 typeStr;
	bool hasGenerated;
	Str8 code;
};
typedef struct Snippet Snippet;
struct Snippet
{
	Arena* arena;
	FilePath genFilePath;
	FilePath sourceFilePath;
	u64 sourceLineNum;
	
	Str8 snippetStr;
	MD_Arena* mdArena;
	MD_Node* mdRootNode;
	
	VarArray pieces; //SnippetPiece
};

typedef struct PiggenState PiggenState;
struct PiggenState
{
	Arena* mainArena; //really this is just scratch[0]
	ProgramArgs args;
	FilePath outputFolderPath;
	VarArray searchPaths; //FilePath
	VarArray excludePaths; //FilePath
	VarArray sourceFilePaths; //FilePath
	VarArray snippets; //Snippet
};
PiggenState* piggen = nullptr;

#include "piggen/piggen_generate.c"

bool ShouldExcludePath(FilePath fileOrFolderPath)
{
	VarArrayLoop(&piggen->excludePaths, eIndex)
	{
		VarArrayLoopGetValue(FilePath, excludePath, &piggen->excludePaths, eIndex);
		if (StrAnyCaseStartsWith(fileOrFolderPath, excludePath))
		{
			// PrintLine_D("Skipping \"%.*s\"", StrPrint(filePath));
			return true;
		}
	}
	return false;
}

//Returns number of folders searched
uxx FindSourceFilesInFolderRecursive(FilePath folderPath, uxx maxDepth, uxx depth)
{
	uxx numFoldersSearched = 1;
	ScratchBegin1(scratch, piggen->mainArena);
	OsFileIter fileIter = OsIterateFiles(scratch, folderPath, true, true);
	bool isFolder = false;
	FilePath filePath = Str8_Empty;
	while (OsIterFileStepEx(&fileIter, &isFolder, &filePath, scratch, true))
	{
		if (isFolder)
		{
			if (maxDepth == 0 || depth < maxDepth)
			{
				if (!ShouldExcludePath(filePath))
				{
					numFoldersSearched += FindSourceFilesInFolderRecursive(filePath, maxDepth, depth+1);
				}
			}
		}
		else if (!ShouldExcludePath(filePath))
		{
			bool hasSourceExtension = false;
			for (uxx eIndex = 0; eIndex < ArrayCount(SourceFileExtensions); eIndex++)
			{
				if (StrAnyCaseEndsWith(filePath, StrLit(SourceFileExtensions[eIndex])))
				{
					hasSourceExtension = true;
					break;
				}
			}
			if (hasSourceExtension) { VarArrayAddValue(FilePath, &piggen->sourceFilePaths, AllocStr8(piggen->mainArena, filePath)); }
		}
	}
	ScratchEnd(scratch);
	return numFoldersSearched;
}

Str8 SpliceFile(Arena* arena, FilePath filePath, Str8 fileContents, uxx startIndex, uxx endIndex, Str8 spliceStr)
{
	NotNull(arena);
	Assert(startIndex < fileContents.length);
	Assert(endIndex < fileContents.length);
	Assert(startIndex >= endIndex);
	Str8 firstPart = StrSlice(fileContents, 0, startIndex);
	Str8 lastPart = StrSliceFrom(fileContents, endIndex);
	Str8 newFileContents = JoinStringsInArena3(arena, firstPart, spliceStr, lastPart, false);
	bool writeSuccess = OsWriteTextFile(filePath, newFileContents);
	if (!writeSuccess)
	{
		PrintLine_E("ERROR: Failed to change file \"%.*s\"", StrPrint(filePath));
		return fileContents;
	}
	return newFileContents;
}

int main(int argc, char* argv[])
{
	InitScratchArenasVirtual(Gigabytes(4));
	ScratchBegin(scratch);
	ScratchBegin1(scratch2, scratch);
	ScratchBegin2(scratch3, scratch, scratch2);
	
	PrintLine_N("Running piggen...");
	piggen = AllocType(PiggenState, scratch);
	NotNull(piggen);
	ClearPointer(piggen);
	piggen->mainArena = scratch;
	ParseProgramArgs(scratch, (uxx)argc-1, (const char**)&argv[1], &piggen->args);
	InitVarArrayWithInitial(FilePath, &piggen->searchPaths, piggen->mainArena, (uxx)piggen->args.args.length);
	InitVarArrayWithInitial(FilePath, &piggen->excludePaths, piggen->mainArena, (uxx)piggen->args.args.length);
	InitVarArrayWithInitial(FilePath, &piggen->sourceFilePaths, piggen->mainArena, 128);
	InitVarArrayWithInitial(Snippet, &piggen->snippets, piggen->mainArena, 128);
	
	piggen->outputFolderPath = FindNamedProgramArgStr(&piggen->args, StrLit("output"), StrLit("o"), StrLit("."));
	piggen->outputFolderPath = OsGetFullPath(scratch2, piggen->outputFolderPath);
	piggen->outputFolderPath = AllocFolderPath(piggen->mainArena, piggen->outputFolderPath, false);
	Assert(DoesPathHaveTrailingSlash(piggen->outputFolderPath));
	PrintLine_D("Outputting to \"%.*s\"", StrPrint(piggen->outputFolderPath));
	
	// +==============================+
	// |   Injest Search Path Args    |
	// +==============================+
	{
		uxx namelessArgIndex = 0;
		while (true)
		{
			Str8 argStr = GetNamelessProgramArg(&piggen->args, namelessArgIndex);
			if (IsEmptyStr(argStr)) { break; }
			FilePath fullRootPath = OsGetFullPath(scratch, argStr);
			VarArrayAddValue(FilePath, &piggen->searchPaths, fullRootPath);
			namelessArgIndex++;
		}
	}
	
	// +==============================+
	// |   Injest Exclude Path Args   |
	// +==============================+
	{
		uxx excludeArgIndex = 0;
		while (true)
		{
			Str8 argStr = FindNamedProgramArgStrEx(&piggen->args, StrLit("exclude"), StrLit("e"), Str8_Empty, excludeArgIndex);
			if (IsEmptyStr(argStr)) { break; }
			FilePath fullExcludePath = OsGetFullPath(scratch, argStr);
			if (DoesPathHaveTrailingSlash(fullExcludePath)) { fullExcludePath.length--; }
			// PrintLine_D("Excluding \"%.*s\"", StrPrint(fullExcludePath));
			VarArrayAddValue(FilePath, &piggen->excludePaths, fullExcludePath);
			excludeArgIndex++;
		}
	}
	
	// +==============================+
	// |    Find All Source Files     |
	// +==============================+
	{
		uxx numFoldersSearched = 0;
		VarArrayLoop(&piggen->searchPaths, pIndex)
		{
			VarArrayLoopGetValue(FilePath, rootPath, &piggen->searchPaths, pIndex);
			Print_D("Searching \"%.*s\"...", StrPrint(rootPath));
			uxx numFilesBefore = piggen->sourceFilePaths.length;
			uxx numFoldersBefore = numFoldersSearched;
			numFoldersSearched += FindSourceFilesInFolderRecursive(rootPath, 0, 0);
			PrintLine_D("Found %llu file%s (in %llu subfolders)",
				piggen->sourceFilePaths.length - numFilesBefore, Plural(piggen->sourceFilePaths.length - numFilesBefore, "s"),
				numFoldersSearched - numFoldersBefore, Plural(numFoldersSearched - numFoldersBefore, "s")
			);
		}
		PrintLine_D("Searched %llu folder%s, found %llu source file%s",
			numFoldersSearched, Plural(numFoldersSearched, "s"),
			piggen->sourceFilePaths.length, Plural(piggen->sourceFilePaths.length, "s")
		);
		// VarArrayLoop(&sourceFilePaths, fIndex)
		// {
		// 	VarArrayLoopGetValue(Str8, sourceFilePath, &sourceFilePaths, fIndex);
		// 	PrintLine_D("\t[%llu] \"%.*s\"", (u64)fIndex, StrPrint(sourceFilePath));
		// }
	}
	
	// +==============================+
	// |   Find and Parse Snippets    |
	// +==============================+
	{
		VarArrayLoop(&piggen->sourceFilePaths, fIndex)
		{
			uxx scratchMark = ArenaGetMark(scratch2);
			VarArrayLoopGetValue(FilePath, sourceFilePath, &piggen->sourceFilePaths, fIndex);
			Str8 sourceFileName = GetFileNamePart(sourceFilePath, true);
			
			Str8 fileContents = Str8_Empty;
			bool readSuccess = OsReadTextFile(sourceFilePath, scratch2, &fileContents);
			if (!readSuccess) { PrintLine_E("ERROR: Failed to read file at \"%.*s\"", StrPrint(sourceFilePath)); continue; }
			
			uxx snippetIndex = 0;
			LineParser lineParser = NewLineParser(fileContents);
			Str8 fileLine = Str8_Empty;
			while (LineParserGetLine(&lineParser, &fileLine))
			{
				Str8 trimmedLine = TrimWhitespace(fileLine);
				Str8 indentationStr = StrSlice(fileLine, 0, (uxx)(trimmedLine.chars - fileLine.chars));
				Str8 inlineMacroStartStr = StrLit("PIGGEN_INLINE(");
				// +==============================+
				// |      Handle #if PIGGEN       |
				// +==============================+
				if (StrExactStartsWith(trimmedLine, StrLit("#if PIGGEN")))
				{
					uxx snippetStartIndex = lineParser.byteIndex;
					bool foundElse = false;
					uxx elseByteIndex = 0;
					uxx elseContentsByteIndex = 0;
					bool foundEndif = false;
					uxx endifByteIndex = 0;
					LineParser scanLineParser = lineParser;
					Str8 scanLine = Str8_Empty;
					while (LineParserGetLine(&scanLineParser, &scanLine))
					{
						Str8 trimmedScanLine = TrimWhitespace(scanLine);
						if (StrExactStartsWith(trimmedScanLine, StrLit("#else")))
						{
							foundElse = true;
							elseByteIndex = scanLineParser.lineBeginByteIndex;
							elseContentsByteIndex = scanLineParser.byteIndex;
						}
						else if (StrExactStartsWith(trimmedScanLine, StrLit("#endif")))
						{
							foundEndif = true;
							endifByteIndex = scanLineParser.lineBeginByteIndex;
							break;
						}
					}
					
					if (foundEndif)
					{
						uxx snippetEndIndex = endifByteIndex;
						if (foundElse) { snippetEndIndex = elseByteIndex; }
						Str8 snippetStr = StrSlice(fileContents, snippetStartIndex, snippetEndIndex);
						// PrintLine_D("Found %llu byte snippet in \"%.*s\" line %llu", (u64)snippetStr.length, StrPrint(sourceFileName), lineParser.lineIndex);
						
						Str8 sanitizedFileName = StrReplace(scratch2, sourceFileName, StrLit("."), StrLit("_"), false);
						Str8 genFileName = PrintInArenaStr(scratch2, "%.*s_%llu.h", StrPrint(sanitizedFileName), (u64)snippetIndex);
						FilePath genFilePath = JoinStringsInArena(scratch2, piggen->outputFolderPath, genFileName, false);
						
						Snippet snippet = ZEROED;
						Result parseResult = ParseSnippet(piggen->mainArena, genFilePath, sourceFilePath, lineParser.lineIndex, snippetStr, &snippet);
						if (parseResult != Result_Success)
						{
							PrintLine_E("Error parsing PIGGEN region in \"%.*s\" line %llu: %s", StrPrint(sourceFileName), (u64)lineParser.lineIndex, GetResultStr(parseResult));
						}
						else
						{
							PrintLine_D("Snippet has %llu piece%s in \"%.*s\" line %llu:", snippet.pieces.length, Plural(snippet.pieces.length, "s"), StrPrint(sourceFileName), lineParser.lineIndex);
							VarArrayLoop(&snippet.pieces, pIndex)
							{
								VarArrayLoopGet(SnippetPiece, piece, &snippet.pieces, pIndex);
								Str8 snippetNodeName = ToStr8FromMd(piece->rootNode->string);
								PrintLine_D("\tPiece[%llu]: %.*s \"%.*s\"", (u64)pIndex, StrPrint(piece->typeStr), StrPrint(snippetNodeName));
							}
							
							// +================================+
							// | Insert Generated File #include |
							// +================================+
							{
								Str8 genFileInclude = PrintInArenaStr(scratch2, "#include \"%.*s\"", StrPrint(genFileName));
								bool needToInsertElse = !foundElse;
								if (foundElse)
								{
									Str8 elseContents = TrimWhitespaceAndNewLines(StrSlice(fileContents, elseContentsByteIndex, endifByteIndex));
									if (!StrExactEquals(elseContents, genFileInclude)) { needToInsertElse = true; }
								}
								if (needToInsertElse)
								{
									uxx replaceStartIndex = (foundElse ? elseContentsByteIndex : endifByteIndex);
									uxx replaceEndIndex = endifByteIndex;
									
									Str8 spliceStr = PrintInArenaStr(scratch2, "%.*s%s%.*s%.*s\n",
										StrPrint(foundElse ? Str8_Empty : indentationStr),
										foundElse ? "" : "#else\n",
										StrPrint(indentationStr),
										StrPrint(genFileInclude)
									);
									fileContents = SpliceFile(scratch2, sourceFilePath, fileContents, replaceStartIndex, replaceEndIndex, spliceStr);
									
									if ((replaceEndIndex - replaceStartIndex) > spliceStr.length)
									{
										uxx numBytesRemoved = (replaceEndIndex - replaceStartIndex) - spliceStr.length;
										scanLineParser.byteIndex -= numBytesRemoved;
										scanLineParser.lineBeginByteIndex -= numBytesRemoved;
									}
									else if ((replaceEndIndex - replaceStartIndex) < spliceStr.length)
									{
										uxx numBytesAdded = spliceStr.length - (replaceEndIndex - replaceStartIndex);
										scanLineParser.byteIndex += numBytesAdded;
										scanLineParser.lineBeginByteIndex += numBytesAdded;
									}
									scanLineParser.inputStr = fileContents;
								}
							}
							
							Snippet* newSnippet = VarArrayAdd(Snippet, &piggen->snippets);
							NotNull(newSnippet);
							MyMemCopy(newSnippet, &snippet, sizeof(Snippet));
						}
						
						snippetIndex++;
						lineParser = scanLineParser;
					}
					else
					{
						PrintLine_W("WARNING: #if PIGGEN region doesn't have a closing #endif in \"%.*s\" line %llu", StrPrint(sourceFilePath), lineParser.lineIndex);
					}
				}
				// +==============================+
				// |  Handle PIGGEN_INLINE (...)  |
				// +==============================+
				else if (StrExactContains(trimmedLine, inlineMacroStartStr))
				{
					uxx macroIndex = StrExactFind(trimmedLine, inlineMacroStartStr);
					uxx closeParensIndex = FindNextCharInStrEx(trimmedLine, macroIndex + inlineMacroStartStr.length, StrLit(")"), true);
					if (closeParensIndex < trimmedLine.length)
					{
						Str8 snippetStr = TrimWhitespace(StrSlice(trimmedLine, macroIndex + inlineMacroStartStr.length, closeParensIndex));
						if (StrExactStartsWith(snippetStr, StrLit("\"")) && StrExactEndsWith(snippetStr, StrLit("\"")))
						{
							snippetStr = StrSlice(snippetStr, 1, snippetStr.length-1);
							//TODO: Convert escape sequences to there equivalent characters!
						}
						
						Str8 sanitizedFileName = StrReplace(scratch2, sourceFileName, StrLit("."), StrLit("_"), false);
						Str8 genFileName = PrintInArenaStr(scratch2, "%.*s_%llu.h", StrPrint(sanitizedFileName), (u64)snippetIndex);
						FilePath genFilePath = JoinStringsInArena(scratch2, piggen->outputFolderPath, genFileName, false);
						
						Snippet snippet = ZEROED;
						Result parseResult = ParseSnippet(piggen->mainArena, genFilePath, sourceFilePath, lineParser.lineIndex, snippetStr, &snippet);
						if (parseResult != Result_Success)
						{
							PrintLine_E("Error parsing PIGGEN_INLINE macro in \"%.*s\" line %llu: %s", StrPrint(sourceFileName), (u64)lineParser.lineIndex, GetResultStr(parseResult));
						}
						else
						{
							PrintLine_D("Snippet has %llu piece%s in \"%.*s\" line %llu:", snippet.pieces.length, Plural(snippet.pieces.length, "s"), StrPrint(sourceFileName), lineParser.lineIndex);
							VarArrayLoop(&snippet.pieces, pIndex)
							{
								VarArrayLoopGet(SnippetPiece, piece, &snippet.pieces, pIndex);
								Str8 snippetNodeName = ToStr8FromMd(piece->rootNode->string);
								PrintLine_D("\tPiece[%llu]: %.*s \"%.*s\"", (u64)pIndex, StrPrint(piece->typeStr), StrPrint(snippetNodeName));
							}
							
							// +================================+
							// | Insert Generated File #include |
							// +================================+
							{
								Str8 genFileInclude = PrintInArenaStr(scratch2, "#include \"%.*s\"", StrPrint(genFileName));
								bool needToInsertInclude = true;
								uxx replaceStartIndex = lineParser.byteIndex;
								uxx replaceEndIndex = lineParser.byteIndex;
								LineParser scanLineParser = lineParser;
								Str8 nextLine = Str8_Empty;
								if (LineParserGetLine(&scanLineParser, &nextLine))
								{
									nextLine = TrimWhitespace(nextLine);
									if (StrExactStartsWith(nextLine, genFileInclude))
									{
										needToInsertInclude = false;
									}
									else if (StrExactStartsWith(nextLine, StrLit("#include")))
									{
										replaceEndIndex = scanLineParser.byteIndex;
									}
								}
								
								if (needToInsertInclude)
								{
									Str8 spliceStr = PrintInArenaStr(scratch2, "%.*s%.*s\n",
										StrPrint(indentationStr),
										StrPrint(genFileInclude)
									);
									fileContents = SpliceFile(scratch2, sourceFilePath, fileContents, replaceStartIndex, replaceEndIndex, spliceStr);
									
									if ((replaceEndIndex - replaceStartIndex) > spliceStr.length)
									{
										uxx numBytesRemoved = (replaceEndIndex - replaceStartIndex) - spliceStr.length;
										scanLineParser.byteIndex -= numBytesRemoved;
										scanLineParser.lineBeginByteIndex -= numBytesRemoved;
									}
									else if ((replaceEndIndex - replaceStartIndex) < spliceStr.length)
									{
										uxx numBytesAdded = spliceStr.length - (replaceEndIndex - replaceStartIndex);
										scanLineParser.byteIndex += numBytesAdded;
										scanLineParser.lineBeginByteIndex += numBytesAdded;
									}
									scanLineParser.inputStr = fileContents;
								}
							}
							
							Snippet* newSnippet = VarArrayAdd(Snippet, &piggen->snippets);
							NotNull(newSnippet);
							MyMemCopy(newSnippet, &snippet, sizeof(Snippet));
						}
					}
					else
					{
						PrintLine_W("WARNING: PIGGEN_INLINE macro does not have a closing parenthesis in \"%.*s\" line %llu", StrPrint(sourceFileName), lineParser.lineIndex);
					}
				}
			}
			ArenaResetToMark(scratch2, scratchMark);
		}
		
		uxx numSnippetsTotal = 0;
		uxx numFilesWithSnippets = 0;
		VarArrayLoop(&piggen->snippets, sIndex)
		{
			VarArrayLoopGet(Snippet, snippet, &piggen->snippets, sIndex);
			if (sIndex == 0 || !StrExactEquals(snippet->sourceFilePath, VarArrayGet(Snippet, &piggen->snippets, sIndex-1)->sourceFilePath)) { numFilesWithSnippets++; }
			numSnippetsTotal++;
		}
		PrintLine_D("Found %llu snippet%s across %llu file%s",
			numSnippetsTotal, Plural(numSnippetsTotal, "s"),
			numFilesWithSnippets, Plural(numFilesWithSnippets, "s")
		);
	}
	
	// getchar(); //wait for user to press ENTER
	PrintLine_N("DONE!");
	
	ScratchEnd(scratch);
	ScratchEnd(scratch2);
	ScratchEnd(scratch3);
	return 0;
}
