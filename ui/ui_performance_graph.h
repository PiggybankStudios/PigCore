/*
File:   ui_performance_graph.h
Author: Taylor Robbins
Date:   11\14\2025
Description:
	** A simple graph of the elapsedMs for every frame over the last 2 seconds that
	** can be rendered in the corner to monitor general performance of the application
*/

#ifndef _UI_PERFORMANCE_GRAPH_H
#define _UI_PERFORMANCE_GRAPH_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "os/os_time.h"
#include "struct/struct_vectors.h"
#include "struct/struct_rectangles.h"
#include "struct/struct_color.h"
#include "misc/misc_standard_colors.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "misc/misc_printing.h"
#include "gfx/gfx_font.h"
#include "gfx/gfx_font_flow.h"
#include "gfx/gfx_system.h"
#include "gfx/gfx_helpers.h"

#define PERF_GRAPH_NUM_FRAMES 120 //2 seconds when running at 60fps

typedef plex PerfGraphFrame PerfGraphFrame;
plex PerfGraphFrame
{
	OsTime recordTime;
	r32 updateMs;
	r32 renderMs;
};

typedef plex PerfGraph PerfGraph;
plex PerfGraph
{
	r32 targetFrameTime;
	uxx headIndex;
	uxx tailIndex;
	PerfGraphFrame frames[PERF_GRAPH_NUM_FRAMES+1];
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void InitPerfGraph(PerfGraph* graph, r32 targetFrameTime);
	PIG_CORE_INLINE void UpdatePerfGraph(PerfGraph* graph, r32 updateMs, r32 renderMs);
	PIG_CORE_INLINE uxx PerfGraphGetNumFilledFrames(PerfGraph* graph);
	#if BUILD_WITH_SOKOL_GFX
	void RenderPerfGraph(PerfGraph* graph, GfxSystem* gfxSystem, PigFont* font, r32 fontSize, u8 fontStyle, rec graphRec);
	#endif
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void InitPerfGraph(PerfGraph* graph, r32 targetFrameTime)
{
	NotNull(graph);
	ClearPointer(graph);
	graph->targetFrameTime = targetFrameTime;
	graph->headIndex = 0;
	graph->tailIndex = 0;
}

PEXPI void UpdatePerfGraph(PerfGraph* graph, r32 updateMs, r32 renderMs)
{
	NotNull(graph);
	uxx nextIndex = ((graph->headIndex + 1) % ArrayCount(graph->frames));
	if (nextIndex == graph->tailIndex)
	{
		graph->tailIndex = ((graph->tailIndex + 1) % ArrayCount(graph->frames));
	}
	
	PerfGraphFrame* frame = &graph->frames[graph->headIndex];
	frame->recordTime = OsGetTime();
	frame->updateMs = updateMs;
	frame->renderMs = renderMs;
	
	graph->headIndex = nextIndex;
}

PEXPI uxx PerfGraphGetNumFilledFrames(PerfGraph* graph)
{
	return (graph->headIndex >= graph->tailIndex)
		? (graph->headIndex - graph->tailIndex)
		: (graph->headIndex + (ArrayCount(graph->frames) - graph->tailIndex));
}

#if BUILD_WITH_SOKOL_GFX
PEXP void RenderPerfGraph(PerfGraph* graph, GfxSystem* gfxSystem, PigFont* font, r32 fontSize, u8 fontStyle, rec graphRec)
{
	NotNull(graph);
	NotNull(gfxSystem);
	NotNull(font);
	ScratchBegin(scratch);
	
	uxx numFrames = PerfGraphGetNumFilledFrames(graph);
	r32 graphHeightMs = graph->targetFrameTime*2; //TODO: Make this dynamic
	r32 graphFrameWidthPx = graphRec.Width / PERF_GRAPH_NUM_FRAMES;
	r32 graphRecRight = graphRec.X + graphRec.Width;
	r32 graphRecBottom = graphRec.Y + graphRec.Height;
	
	GfxSystem_DrawRectangle(gfxSystem, graphRec, ColorWithAlpha(MonokaiDarkGray, 0.5f));
	
	r32 targetFrameTimeHeight = (graph->targetFrameTime / graphHeightMs);
	GfxSystem_DrawLine(gfxSystem,
		MakeV2(graphRec.X, graphRecBottom - targetFrameTimeHeight),
		MakeV2(graphRecRight, graphRecBottom - targetFrameTimeHeight),
		1.0f, MonokaiBlue
	);
	
	reci oldClipRec = GfxSystem_AddClipRec(gfxSystem, ToReciFromf(graphRec));
	
	PerfGraphFrame averageFrame = ZEROED;
	PerfGraphFrame* prevFrame = nullptr;
	for (uxx frameIndex = 0; frameIndex < numFrames; frameIndex++)
	{
		uxx fifoIndex = ((graph->tailIndex + frameIndex) % ArrayCount(graph->frames));
		if (fifoIndex == graph->headIndex) { break; }
		PerfGraphFrame* frame = &graph->frames[fifoIndex];
		averageFrame.updateMs += frame->updateMs;
		averageFrame.renderMs += frame->renderMs;
		if (prevFrame != nullptr)
		{
			r32 frameUpdateHeight = (frame->updateMs / graphHeightMs) * graphRec.Height;
			r32 prevFrameUpdateHeight = (prevFrame->updateMs / graphHeightMs) * graphRec.Height;
			v2 updateLineStart = MakeV2(
				graphRecRight - ((numFrames - (frameIndex-1)) * graphFrameWidthPx),
				graphRecBottom - prevFrameUpdateHeight
			);
			v2 updateLineEnd = MakeV2(
				graphRecRight - ((numFrames - frameIndex) * graphFrameWidthPx),
				graphRecBottom - frameUpdateHeight
			);
			GfxSystem_DrawLine(gfxSystem, updateLineStart, updateLineEnd, 1.0f, MonokaiYellow);
		}
		prevFrame = frame;
	}
	averageFrame.updateMs /= (r32)numFrames;
	averageFrame.renderMs /= (r32)numFrames;
	PerfGraphFrame* finalFrame = prevFrame;
	
	prevFrame = nullptr;
	for (uxx frameIndex = 0; frameIndex < numFrames; frameIndex++)
	{
		uxx fifoIndex = ((graph->tailIndex + frameIndex) % ArrayCount(graph->frames));
		if (fifoIndex == graph->headIndex) { break; }
		PerfGraphFrame* frame = &graph->frames[fifoIndex];
		if (prevFrame != nullptr)
		{
			r32 frameFlipHeight = ((frame->updateMs + frame->renderMs) / graphHeightMs) * graphRec.Height;
			r32 prevFrameFlipHeight = ((prevFrame->updateMs + prevFrame->renderMs) / graphHeightMs) * graphRec.Height;
			v2 updateLineStart = MakeV2(
				graphRecRight - ((numFrames - (frameIndex-1)) * graphFrameWidthPx),
				graphRecBottom - prevFrameFlipHeight
			);
			v2 updateLineEnd = MakeV2(
				graphRecRight - ((numFrames - frameIndex) * graphFrameWidthPx),
				graphRecBottom - frameFlipHeight
			);
			GfxSystem_DrawLine(gfxSystem, updateLineStart, updateLineEnd, 1.0f, MonokaiMagenta);
		}
		prevFrame = frame;
	}
	
	GfxSystem_SetClipRec(gfxSystem, oldClipRec);
	
	GfxSystem_DrawRectangleOutlineEx(gfxSystem, graphRec, 1.0f, MonokaiWhite, false);
	
	GfxSystem_BindFontEx(gfxSystem, font, fontSize, fontStyle);
	r32 lineHeight = GfxSystem_GetLineHeight(gfxSystem);
	v2 textPos = MakeV2(graphRec.X + graphRec.Width + 5, graphRec.Y + GfxSystem_GetMaxAscend(gfxSystem));
	
	Str8 fpsText = ScratchPrintStr("%.0f FPS", 1000.0f / (averageFrame.updateMs + averageFrame.renderMs));
	TextMeasure fpsTextMeasure = MeasureTextEx(font, fontSize, fontStyle, false, 0.0f, fpsText);
	v2 fpsTextPos = MakeV2(graphRec.X + graphRec.Width - 5 - fpsTextMeasure.Width, textPos.Y + 2);
	// GfxSystem_DrawText(gfxSystem, fpsText, AddV2(fpsTextPos, MakeV2( 0,  1)), Black);
	// GfxSystem_DrawText(gfxSystem, fpsText, AddV2(fpsTextPos, MakeV2( 0, -1)), Black);
	// GfxSystem_DrawText(gfxSystem, fpsText, AddV2(fpsTextPos, MakeV2( 1,  0)), Black);
	// GfxSystem_DrawText(gfxSystem, fpsText, AddV2(fpsTextPos, MakeV2(-1,  0)), Black);
	GfxSystem_DrawText(gfxSystem, fpsText, fpsTextPos, MonokaiWhite);
	
	// GfxSystem_DrawText(gfxSystem, ScratchPrintStr("%llu frames", numFrames), textPos, MonokaiWhite); textPos.Y += lineHeight;
	
	if (finalFrame != nullptr)
	{
		GfxSystem_DrawTextEmbossed(gfxSystem, ScratchPrintStr("GPU %.1fms", finalFrame->renderMs), textPos, MonokaiMagenta); textPos.Y += lineHeight;
		GfxSystem_DrawTextEmbossed(gfxSystem, ScratchPrintStr("CPU %.1fms", finalFrame->updateMs), textPos, MonokaiYellow); textPos.Y += lineHeight;
	}
	
	ScratchEnd(scratch);
}
#endif //BUILD_WITH_SOKOL_GFX

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _UI_PERFORMANCE_GRAPH_H
