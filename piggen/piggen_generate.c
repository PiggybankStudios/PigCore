/*
File:   piggen_generate.c
Author: Taylor Robbins
Date:   04\01\2025
Description: 
	** Holds the logic that actually generates code from snippets
*/

void FreeSnippet(Snippet* snippet)
{
	NotNull(snippet);
	if (snippet->arena != nullptr)
	{
		if (CanArenaFree(snippet->arena))
		{
			FreeStr8(snippet->arena, &snippet->genFilePath);
			FreeStr8(snippet->arena, &snippet->sourceFilePath);
			FreeStr8(snippet->arena, &snippet->snippetStr);
		}
		FreeVarArray(&snippet->pieces);
		MD_ArenaRelease(snippet->mdArena);
	}
	ClearPointer(snippet);
}

Result ParseSnippet(Arena* arena, FilePath genFilePath, FilePath sourceFilePath, uxx sourceLineNum, Str8 snippetStr, Snippet* snippetOut)
{
	NotNull(arena);
	NotNull(snippetOut);
	ScratchBegin1(scratch, arena);
	Str8 sourceFileName = GetFileNamePart(sourceFilePath, true);
	
	ClearPointer(snippetOut);
	snippetOut->arena = arena;
	snippetOut->genFilePath = AllocStr8(arena, genFilePath);
	snippetOut->sourceFilePath = AllocStr8(arena, sourceFilePath);
	snippetOut->sourceLineNum = sourceLineNum;
	snippetOut->mdArena = MD_ArenaAlloc();
	snippetOut->snippetStr = AllocStr8(arena, snippetStr);
	InitVarArray(SnippetPiece, &snippetOut->pieces, arena);
	
	MD_ParseResult mdResult = MD_ParseWholeString(snippetOut->mdArena, ToMdString8(snippetOut->sourceFilePath), ToMdString8(snippetOut->snippetStr));
	if (mdResult.errors.max_message_kind >= MD_MessageKind_Error)
	{
		uxx numErrors = 0;
		MD_Message* errorIter = mdResult.errors.first;
		while (errorIter != nullptr) { numErrors++; errorIter = errorIter->next; }
		PrintLine_E("Invalid metadesk syntax in snippet in \"%.*s\" line %llu! %llu error%s:",
			StrPrint(sourceFileName),
			(u64)sourceLineNum,
			numErrors, Plural(numErrors, "s")
		);
		errorIter = mdResult.errors.first;
		while (errorIter != nullptr)
		{
			Str8 errorString = ToStr8FromMd(errorIter->string);
			Str8 errorKindStr = ToStr8FromMd(MD_StringFromMessageKind(errorIter->kind));
			PrintLine_E("\t%.*s Error: %.*s", StrPrint(errorKindStr), StrPrint(errorString));
			errorIter = errorIter->next;
		}
		FreeSnippet(snippetOut);
		return Result_InvalidSyntax;
	}
	else
	{
		NotNull(mdResult.node);
		snippetOut->mdRootNode = mdResult.node;
	}
	
	u64 numTopNodes = 0;
	for (MD_EachNode(topNode, snippetOut->mdRootNode->first_child)) { numTopNodes++; }
	VarArrayExpand(&snippetOut->pieces, numTopNodes);
	
	for (MD_EachNode(topNode, snippetOut->mdRootNode->first_child))
	{
		Str8 nodeString = ToStr8FromMd(topNode->string);
		if (MD_NodeIsNil(topNode->first_tag) || topNode->first_tag->string.size == 0)
		{
			PrintLine_W("ERROR: Untagged top-level node \"%.*s\" in \"%.*s\" line %llu. All nodes should have a tag like @struct or @enum preceding their name", StrPrint(nodeString), StrPrint(sourceFileName), (u64)sourceLineNum);
			continue;
		}
		uxx numTags = 0;
		MD_Node* tag = topNode->first_tag;
		while (tag != nullptr && !MD_NodeIsNil(tag)) { numTags++; tag = tag->next; }
		if (numTags > 1)
		{
			PrintLine_W("ERROR: Multiple tags not allowed on top-level node \"%.*s\" in \"%.*s\" line %llu", StrPrint(nodeString), StrPrint(sourceFileName), (u64)sourceLineNum);
			continue;
		}
		
		SnippetPiece* newPiece = VarArrayAdd(SnippetPiece, &snippetOut->pieces);
		NotNull(newPiece);
		ClearPointer(newPiece);
		newPiece->rootNode = topNode;
		newPiece->typeStr = ToStr8FromMd(topNode->first_tag->string);
		newPiece->hasGenerated = false;
		newPiece->code = Str8_Empty;
	}
	
	if (snippetOut->pieces.length == 0)
	{
		PrintLine_W("WARNING: Snippet has no code generating pieces in \"%.*s\" line %llu", StrPrint(sourceFileName), (u64)sourceLineNum);
	}
	
	ScratchEnd(scratch);
	return Result_Success;
}
