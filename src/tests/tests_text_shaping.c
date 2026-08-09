/*
File:   tests_text_shaping.c
Author: Taylor Robbins
Date:   08\09\2026
Description: 
	** Holds some test code that exercises the kb_text_shape.h APIs
*/

#if (BUILD_WITH_SOKOL_GFX && BUILD_WITH_SOKOL_APP)

typedef plex TextShapingState TextShapingState;
plex TextShapingState
{
	kbts_shape_context* context;
	VarArray fontFiles; //Slice
	VarArray fonts; //kbts_font*
};
static TextShapingState* textShaping = nullptr;

//Pre-declared in tests_main.c
void InitTextShapingTests()
{
	ScratchBegin(scratch);
	textShaping = AllocType(TextShapingState, stdHeap);
	NotNull(textShaping);
	ClearPointer(textShaping);
	textShaping->context = kbts_CreateShapeContext(/*AllocatorFunc*/ nullptr, /*AllocatorData*/ (void*)untrackedStdHeap);
	// kbts_DestroyShapeContext(textShaping->context); //Test destroying to make sure free is working, since we don't really clean up in this test normally
	
	const char* fontFileNames[] = {
		"NotoSans-Regular.ttf",
		"NotoSansJP-Regular.ttf",
		"NotoSansSymbols-Regular.ttf",
		"NotoEmoji-Regular.ttf",
		"NotoColorEmoji-Regular.ttf",
		"seguiemj.ttf",
	};
	InitVarArrayWithInitial(Slice, &textShaping->fontFiles, stdHeap, ArrayCount(fontFileNames));
	InitVarArrayWithInitial(kbts_font*, &textShaping->fonts, stdHeap, ArrayCount(fontFileNames));
	for (uxx fIndex = 0; fIndex < ArrayCount(fontFileNames); fIndex++)
	{
		FilePath fontPath = JoinPathsInArena(scratch, FilePathLit("../fonts"), MakeStr8Nt(fontFileNames[fIndex]), false);
		Slice newFontContents = Slice_Empty;
		bool readFontFile = OsReadBinFile(fontPath, stdHeap, &newFontContents);
		Assert(readFontFile == true);
		Assert(newFontContents.length <= INT_MAX);
		int numFontsInFile = kbts_FontCount(newFontContents.bytes, (int)newFontContents.length);
		PrintLine_D("Font file \"%s\" is %d bytes has %d font%s", fontFileNames[fIndex], (int)newFontContents.length, numFontsInFile, Plural(numFontsInFile, "s"));
		//TODO: Does kb now own the memory, can we drop our copy of the font now?
		kbts_font* newKbFontPntr = kbts_ShapePushFontFromMemory(textShaping->context, newFontContents.bytes, (int)newFontContents.length, /*FontIndex=*/0);
		NotNull(newKbFontPntr);
		Assert(kbts_FontIsValid(newKbFontPntr));
		VarArrayAddValue(Slice, &textShaping->fontFiles, newFontContents);
		VarArrayAddValue(kbts_font*, &textShaping->fonts, newKbFontPntr);
	}
	
	ScratchEnd(scratch);
}

//Pre-declared in tests_main.c
void UpdateTextShapingTests()
{
	NotNull(textShaping);
	
}

//Pre-declared in tests_main.c
void RenderTextShapingTests()
{
	NotNull(textShaping);
	
}

#endif //(BUILD_WITH_SOKOL_GFX && BUILD_WITH_SOKOL_APP)
