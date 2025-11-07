/*
File:   cross_scratch_and_poly_simplify.h
Author: Taylor Robbins
Date:   09\12\2025
*/

#ifndef _CROSS_SCRATCH_AND_POLY_SIMPLIFY_H
#define _CROSS_SCRATCH_AND_POLY_SIMPLIFY_H

//NOTE: Intentionally no includes here

#if !PIG_CORE_IMPLEMENTATION
	uxx SimplifyPolygonInPlace(uxx numPolyVerts, v2* polyVerts, r32 epsilon);
	uxx SimplifyPolygonInPlaceR64(uxx numPolyVerts, v2d* polyVerts, r64 epsilon);
	Vec2Slice SimplifyPolygonInArena(Arena* arena, uxx numPolyVerts, const v2* polyVerts, r32 epsilon);
	Vec2R64Slice SimplifyPolygonInArenaR64(Arena* arena, uxx numPolyVerts, const v2d* polyVerts, r64 epsilon);
#endif

#if PIG_CORE_IMPLEMENTATION

PEXP uxx SimplifyPolygonInPlace(uxx numPolyVerts, v2* polyVerts, r32 epsilon)
{
	if (numPolyVerts <= 2) { return numPolyVerts; }
	NotNull(polyVerts);
	ScratchBegin(scratch);
	
	SimpPolygon scratchPoly = ZEROED;
	scratchPoly.numVertices = numPolyVerts;
	scratchPoly.vertices = AllocArray(SimpPolyVert, scratch, numPolyVerts);
	for (uxx vIndex = 0; vIndex < numPolyVerts; vIndex++) { scratchPoly.vertices[vIndex].state = 0; scratchPoly.vertices[vIndex].pos = polyVerts[vIndex]; }
	
	uxx numVerticesLeft = SimplifyPolygon(&scratchPoly, epsilon);
	DebugAssert(numVerticesLeft <= numPolyVerts);
	
	uxx writeIndex = 0;
	for (uxx vIndex = 0; vIndex < numPolyVerts; vIndex++)
	{
		if (scratchPoly.vertices[vIndex].state > 0)
		{
			polyVerts[writeIndex] = polyVerts[vIndex];
			writeIndex++;
		}
	}
	DebugAssert(writeIndex == numVerticesLeft);
	
	ScratchEnd(scratch);
	return numVerticesLeft;
}
PEXP uxx SimplifyPolygonInPlaceR64(uxx numPolyVerts, v2d* polyVerts, r64 epsilon)
{
	if (numPolyVerts <= 2) { return numPolyVerts; }
	NotNull(polyVerts);
	ScratchBegin(scratch);
	
	SimpPolygonR64 scratchPoly = ZEROED;
	scratchPoly.numVertices = numPolyVerts;
	scratchPoly.vertices = AllocArray(SimpPolyVertR64, scratch, numPolyVerts);
	for (uxx vIndex = 0; vIndex < numPolyVerts; vIndex++) { scratchPoly.vertices[vIndex].state = 0; scratchPoly.vertices[vIndex].pos = polyVerts[vIndex]; }
	
	uxx numVerticesLeft = SimplifyPolygonR64(&scratchPoly, epsilon);
	DebugAssert(numVerticesLeft <= numPolyVerts);
	
	uxx writeIndex = 0;
	for (uxx vIndex = 0; vIndex < numPolyVerts; vIndex++)
	{
		if (scratchPoly.vertices[vIndex].state > 0)
		{
			polyVerts[writeIndex] = polyVerts[vIndex];
			writeIndex++;
		}
	}
	DebugAssert(writeIndex == numVerticesLeft);
	
	ScratchEnd(scratch);
	return numVerticesLeft;
}

PEXP Vec2Slice SimplifyPolygonInArena(Arena* arena, uxx numPolyVerts, const v2* polyVerts, r32 epsilon)
{
	NotNull(arena);
	Assert(numPolyVerts == 0 || polyVerts != nullptr);
	Vec2Slice result = ZEROED;
	if (numPolyVerts == 0) { return result; }
	
	ScratchBegin1(scratch, arena);
	
	SimpPolygon scratchPoly = ZEROED;
	scratchPoly.numVertices = numPolyVerts;
	scratchPoly.vertices = AllocArray(SimpPolyVert, scratch, numPolyVerts);
	NotNull(scratchPoly.vertices);
	for (uxx vIndex = 0; vIndex < numPolyVerts; vIndex++) { scratchPoly.vertices[vIndex].state = 0; scratchPoly.vertices[vIndex].pos = polyVerts[vIndex]; }
	
	uxx numVerticesLeft = SimplifyPolygon(&scratchPoly, epsilon);
	DebugAssert(numVerticesLeft <= numPolyVerts);
	result.vectors = AllocArray(v2, arena, numVerticesLeft);
	if (result.vectors == nullptr) { return result; }
	result.length = numVerticesLeft;
	
	uxx writeIndex = 0;
	for (uxx vIndex = 0; vIndex < scratchPoly.numVertices; vIndex++)
	{
		if (scratchPoly.vertices[vIndex].state > 0)
		{
			result.vectors[writeIndex] = scratchPoly.vertices[vIndex].pos;
			writeIndex++;
		}
	}
	DebugAssert(writeIndex == result.length);
	
	ScratchEnd(scratch);
	return result;
}
PEXP Vec2R64Slice SimplifyPolygonInArenaR64(Arena* arena, uxx numPolyVerts, const v2d* polyVerts, r64 epsilon)
{
	NotNull(arena);
	Assert(numPolyVerts == 0 || polyVerts != nullptr);
	Vec2R64Slice result = ZEROED;
	if (numPolyVerts == 0) { return result; }
	
	ScratchBegin1(scratch, arena);
	
	SimpPolygonR64 scratchPoly = ZEROED;
	scratchPoly.numVertices = numPolyVerts;
	scratchPoly.vertices = AllocArray(SimpPolyVertR64, scratch, numPolyVerts);
	NotNull(scratchPoly.vertices);
	for (uxx vIndex = 0; vIndex < numPolyVerts; vIndex++) { scratchPoly.vertices[vIndex].state = 0; scratchPoly.vertices[vIndex].pos = polyVerts[vIndex]; }
	
	uxx numVerticesLeft = SimplifyPolygonR64(&scratchPoly, epsilon);
	DebugAssert(numVerticesLeft <= numPolyVerts);
	result.vectors = AllocArray(v2d, arena, numVerticesLeft);
	if (result.vectors == nullptr) { return result; }
	result.length = numVerticesLeft;
	
	uxx writeIndex = 0;
	for (uxx vIndex = 0; vIndex < scratchPoly.numVertices; vIndex++)
	{
		if (scratchPoly.vertices[vIndex].state > 0)
		{
			result.vectors[writeIndex] = scratchPoly.vertices[vIndex].pos;
			writeIndex++;
		}
	}
	DebugAssert(writeIndex == result.length);
	
	ScratchEnd(scratch);
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_SCRATCH_AND_POLY_SIMPLIFY_H
