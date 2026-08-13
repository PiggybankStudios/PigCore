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
		"NotoSans-Regular.ttf",        //   629,024 bytes, 1 font
		"NotoSansJP-Regular.ttf",      // 5,450,308 bytes, 1 font
		"NotoSansSymbols-Regular.ttf", //   185,152 bytes, 1 font
		"NotoEmoji-Regular.ttf",       //   880,860 bytes, 1 font
		"NotoColorEmoji-Regular.ttf",  //25,096,376 bytes, 1 font
		// "seguiemj.ttf",                //12,450,664 bytes, 1 font
	};
	InitVarArrayWithInitial(Slice, &textShaping->fontFiles, stdHeap, ArrayCount(fontFileNames));
	InitVarArrayWithInitial(kbts_font*, &textShaping->fonts, stdHeap, ArrayCount(fontFileNames));
	for (uxx fIndex = ArrayCount(fontFileNames); fIndex > 0; fIndex--)
	{
		FilePath fontPath = JoinPathsInArena(scratch, FilePathLit("../fonts"), MakeStr8Nt(fontFileNames[fIndex-1]), false);
		Slice newFontContents = Slice_Empty;
		bool readFontFile = OsReadBinFile(fontPath, stdHeap, &newFontContents);
		Assert(readFontFile == true);
		Assert(newFontContents.length <= INT_MAX);
		int numFontsInFile = kbts_FontCount(newFontContents.bytes, (int)newFontContents.length);
		PrintLine_D("Font file \"%s\" is %d bytes has %d font%s", fontFileNames[fIndex-1], (int)newFontContents.length, numFontsInFile, Plural(numFontsInFile, "s"));
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
	
    // Layout runs naively left to right.
    {
		kbts_ShapeBegin(textShaping->context, KBTS_DIRECTION_DONT_KNOW, KBTS_LANGUAGE_DONT_KNOW);
		Str8 text = StrLit("Let's shape something!");
		kbts_ShapeUtf8(textShaping->context, text.chars, (int)text.length, KBTS_USER_ID_GENERATION_MODE_CODEPOINT_INDEX);
		kbts_ShapeEnd(textShaping->context);
		
		r32 textScale = TEXT_SCALE/sapp_dpi_scale();
    	// PrintLine_D("Shaping \"%.*s\" (%llu bytes, %llu codepoints)", StrPrint(text), text.length, CountCodepointsUtf8Str(text));
		kbts_run kbRun;
		v2i cursorPos = MakeV2i(50, 250);
		while(kbts_ShapeRun(textShaping->context, &kbRun))
		{
			kbts_glyph* glyph;
			while(kbts_GlyphIteratorNext(&kbRun.Glyphs, &glyph))
			{
				v2i glyphPos = AddV2i(cursorPos, MakeV2i(glyph->OffsetX, glyph->OffsetY));
				
				// PrintLine_D("Display 0x%X \'%c\' at (%d, %d)", glyph->Codepoint, (char)glyph->Codepoint, glyphPos.x, glyphPos.y);
				// DisplayGlyph(glyph->Id, glyphPos.x, glyphPos.y);
				
				FontAtlas* fontAtlas = nullptr;
				FontGlyph* fontGlyph = TryGetFontGlyphForCodepoint(&testFont, glyph->Codepoint, 18*textScale, FontStyleFlag_None, /*allowActiveAtlasCreation*/true, &fontAtlas);
				rec glyphDrawRec = MakeRec((r32)glyphPos.x, (r32)glyphPos.y-10, 10, 10);
				if (fontGlyph != nullptr)
				{
					glyphDrawRec = MakeRec(
						glyphPos.x + fontGlyph->metrics.renderOffset.x,
						glyphPos.y + fontGlyph->metrics.renderOffset.x,
						(r32)fontGlyph->metrics.glyphSize.width,
						(r32)fontGlyph->metrics.glyphSize.height
					);
					CommitFontAtlasTextureUpdates(&testFont, fontAtlas);
					rec atlasSourceRec = ToRecFromi(MakeReciV(fontGlyph->atlasSourcePos, fontGlyph->metrics.glyphSize));
					DrawTexturedRectangleEx(glyphDrawRec, White, &fontAtlas->texture, atlasSourceRec);
				}
				else
				{
					DrawRectangleOutline(glyphDrawRec, 1, MonokaiRed);
					DrawRectangleOutline(InflateRec(glyphDrawRec, -2), 1, MonokaiBlue);
				}
				
				cursorPos.x += glyph->AdvanceX;
				cursorPos.y += glyph->AdvanceY;
			}
		}
	}
}

#endif //(BUILD_WITH_SOKOL_GFX && BUILD_WITH_SOKOL_APP)
