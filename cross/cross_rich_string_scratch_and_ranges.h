/*
File:   cross_rich_string_scratch_and_ranges.h
Author: Taylor Robbins
Date:   08\10\2025
*/

#ifndef _CROSS_RICH_STRING_SCRATCH_AND_RANGES_H
#define _CROSS_RICH_STRING_SCRATCH_AND_RANGES_H

//NOTE: Intentionally no includes here!

typedef plex RichStrStyleChangeRange RichStrStyleChangeRange;
plex RichStrStyleChangeRange
{
	RichStrStyleChange style;
	RangeUXX range;
};

//This is only really used internally in NewRichStrFromRanges to allow sorting a list of edges before generating pieces
typedef plex RichStrStyleChangeEdge RichStrStyleChangeEdge;
plex RichStrStyleChangeEdge
{
	uxx index;
	const RichStrStyleChangeRange* range;
	bool begin;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	RichStr NewRichStrFromRanges(Arena* arena, Str8 str, uxx numRanges, const RichStrStyleChangeRange* ranges);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

//TODO: This doesn't account for multiple ranges that change the same style option and overlap (like multiple ranges that affect fontSize)
//
//"Some longer string with overlapping style changes!"
//      [____________________________]
//          Highlight [____________________]           Ranges
//                              Bold
//                                                       |
//                                                       | NewRichStrFromRanges()
//                                                       V
//"Some longer string with overlapping style changes!"
// [___][____________][______________][____][_______]  Pieces
// None   Highlight         Bold      !High   !Bold 
//
PEXP RichStr NewRichStrFromRanges(Arena* arena, Str8 str, uxx numRanges, const RichStrStyleChangeRange* ranges)
{
	NotNullStr(str);
	Assert(numRanges == 0 || ranges != nullptr);
	if (numRanges == 0)
	{
		RichStrPiece singlePiece = ZEROED;
		singlePiece.styleChange = RichStrStyleChange_None;
		singlePiece.str = str;
		return NewRichStr(arena, 1, &singlePiece);
	}
	ScratchBegin1(scratch, arena);
	
	uxx numEdges = numRanges*2;
	RichStrStyleChangeEdge* edges = AllocArray(RichStrStyleChangeEdge, scratch, numEdges);
	NotNull(edges);
	for (uxx rIndex = 0; rIndex < numRanges; rIndex++)
	{
		Assert(ranges[rIndex].range.min <= str.length);
		Assert(ranges[rIndex].range.max <= str.length);
		RichStrStyleChangeEdge* edge1 = &edges[rIndex*2 + 0];
		RichStrStyleChangeEdge* edge2 = &edges[rIndex*2 + 1];
		edge1->index = ranges[rIndex].range.min; edge1->range = &ranges[rIndex]; edge1->begin = true;
		edge2->index = ranges[rIndex].range.max; edge2->range = &ranges[rIndex]; edge2->begin = false;
	}
	
	//TODO: Use an actual good sorting algorithm (this is "Selection Sort")
	for (uxx eIndex = 0; eIndex < numEdges; eIndex++)
	{
		RichStrStyleChangeEdge* minEdge = nullptr;
		for (uxx eIndex2 = eIndex; eIndex2 < numEdges; eIndex2++)
		{
			if (minEdge == nullptr || edges[eIndex2].index < minEdge->index)
			{
				minEdge = &edges[eIndex2];
			}
		}
		if (minEdge != nullptr && minEdge != &edges[eIndex])
		{
			SwapPntrs(RichStrStyleChangeEdge, &edges[eIndex], minEdge);
		}
	}
	
	uxx numPieces = numEdges+1;
	RichStrPiece* pieces = AllocArray(RichStrPiece, scratch, numPieces);
	NotNull(pieces);
	pieces[0].styleChange = RichStrStyleChange_None;
	pieces[0].str = StrSlice(str, 0, edges[0].index);
	for (uxx eIndex = 0; eIndex < numEdges; eIndex++)
	{
		RichStrStyleChangeEdge* pieceBreak = &edges[eIndex];
		RichStrPiece* piece = &pieces[1 + eIndex];
		uxx endIndex = (eIndex+1 < numEdges) ? edges[eIndex+1].index : str.length;
		DebugAssert(endIndex >= pieceBreak->index); //Make sure the sorting algorithm worked
		piece->styleChange = (pieceBreak->begin ? pieceBreak->range->style : OppositeRichStrStyleChange(pieceBreak->range->style));
		piece->str = StrSlice(str, pieceBreak->index, endIndex);
	}
	
	RichStr result = NewRichStr(arena, numPieces, pieces);
	
	ScratchEnd(scratch);
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_RICH_STRING_SCRATCH_AND_RANGES_H
