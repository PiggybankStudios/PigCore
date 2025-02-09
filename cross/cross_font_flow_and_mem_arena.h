/*
File:   cross_font_flow_and_mem_arena.h
Author: Taylor Robbins
Date:   02\09\2025
*/

#ifndef _CROSS_FONT_FLOW_AND_MEM_ARENA_H
#define _CROSS_FONT_FLOW_AND_MEM_ARENA_H

typedef struct TextLayout TextLayout;
struct TextLayout
{
	Arena* arena;
	FontFlow flow;
	uxx numGlyphs;
	uxx numGlyphsAlloc;
	FontFlowGlyph* glyphs;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeTextLayout(TextLayout* layout);
	Result DoTextLayoutInArena(Arena* arena, FontFlowState* state, TextLayout* layoutOut);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void FreeTextLayout(TextLayout* layout)
{
	if (layout->arena != nullptr)
	{
		if (layout->glyphs != nullptr) { FreeMem(layout->arena, layout->glyphs, sizeof(FontFlowGlyph) * layout->numGlyphsAlloc); }
	}
	ClearPointer(layout);
}

PEXP Result DoTextLayoutInArena(Arena* arena, FontFlowState* state, TextLayout* layoutOut)
{
	NotNull(arena);
	NotNull(state);
	NotNull(layoutOut);
	
	ClearPointer(layoutOut);
	layoutOut->arena = arena;
	
	uxx numCodepoints = 0;
	while (state->byteIndex < state->text.length)
	{
		u32 codepoint = 0;
		u8 utf8ByteSize = GetCodepointForUtf8Str(state->text, state->byteIndex, &codepoint);
		if (utf8ByteSize == 0)
		{
			//TODO: Should we handle invalid UTF-8 differently?
			codepoint = CharToU32(state->text.chars[state->byteIndex]);
			utf8ByteSize = 1;
		}
		numCodepoints++;
		state->byteIndex += utf8ByteSize;
	}
	state->byteIndex = 0;
	
	layoutOut->numGlyphsAlloc = numCodepoints;
	if (numCodepoints > 0)
	{
		layoutOut->glyphs = AllocArray(FontFlowGlyph, arena, layoutOut->numGlyphsAlloc);
		if (layoutOut->glyphs == nullptr) { return Result_FailedToAllocateMemory; }
		#if DEBUG_BUILD
		MyMemSet(layoutOut->glyphs, 0xCC, sizeof(FontFlowGlyph) * layoutOut->numGlyphsAlloc);
		#endif
	}
	layoutOut->flow.numGlyphsAlloc = layoutOut->numGlyphsAlloc;
	layoutOut->flow.glyphs = layoutOut->glyphs;
	
	Result flowResult = DoFontFlow(state, nullptr, &layoutOut->flow);
	Assert(layoutOut->flow.numGlyphs <= layoutOut->flow.numGlyphsAlloc);
	layoutOut->numGlyphs = layoutOut->flow.numGlyphs;
	
	return flowResult;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_FONT_FLOW_AND_MEM_ARENA_H
