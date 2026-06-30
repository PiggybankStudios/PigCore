/*
File:   misc_triangulation.h
Author: Taylor Robbins
Date:   08\24\2025
*/

#ifndef _MISC_TRIANGULATION_H
#define _MISC_TRIANGULATION_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "struct/struct_vectors.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"

typedef plex TriangulateResult TriangulateResult;
plex TriangulateResult
{
	Arena* arena;
	uxx* indices;
	uxx numIndices;
	uxx numParts;
	uxx numHoles;
};

typedef plex TriangulatePart TriangulatePart;
plex TriangulatePart
{
	uxx numVertices;
	const v2d* vertices;
	
	//Used by the algorithm. They don't need to be filled beforehand
	bool isClockwise;
	bool isHole;
	uxx holeParentIndex;
	bool hasHoles;
	uxx numHoles;
};

typedef plex TriangulateVert TriangulateVert;
plex TriangulateVert
{
	bool removed;
	uxx index;
	v2d pos;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE bool IsPolygonClockwiseR64(uxx numVertices, v2d* vertices);
	PIG_CORE_INLINE bool IsTriangleClockwiseR64(v2d p0, v2d p1, v2d p2);
	PIG_CORE_INLINE bool IsInsideTriangleR64(v2d p0, v2d p1, v2d p2, v2d test);
	uxx RemoveColinearAndDuplicateVerticesR64(uxx numVertices, v2d* vertices);
	PIG_CORE_INLINE TriangulateVert* GetPrevTriangulateVertR64(TriangulateVert* vertices, uxx totalNumVertices, uxx startIndex);
	PIG_CORE_INLINE TriangulateVert* GetNextTriangulateVertR64(TriangulateVert* vertices, uxx totalNumVertices, uxx startIndex);
	uxx* Triangulate2DEarClipR64(Arena* arena, uxx numVertices, const v2d* vertices, uxx* numIndicesOut);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI bool IsPolygonClockwiseR64(uxx numVertices, v2d* vertices)
{
	if (numVertices < 3) { return true; }
	r64 aggregateValue = 0;
	for (uxx vIndex = 0; vIndex < numVertices; vIndex++)
	{
		v2d currentVert = vertices[vIndex];
		v2d nextVert = vertices[(vIndex+1) % numVertices];
		aggregateValue += (nextVert.X - currentVert.X) * (nextVert.Y + currentVert.Y);
	}
	return (aggregateValue <= 0);
}

PEXPI bool IsTriangleClockwiseR64(v2d p0, v2d p1, v2d p2)
{
	return (DotV2d(SubV2d(p2, p0), PerpRightV2d(SubV2d(p1, p0))) >= 0.0);
}

PEXPI bool IsInsideTriangleR64(v2d p0, v2d p1, v2d p2, v2d test)
{
	v2d perp0 = PerpRightV2d(SubV2d(p1, p0));
	v2d perp1 = PerpRightV2d(SubV2d(p2, p1));
	v2d perp2 = PerpRightV2d(SubV2d(p0, p2));
	if (p0.X == p1.X && test.X == p0.X && test.Y >= MinR64(p0.Y, p1.Y) && test.Y <= MaxR64(p0.Y, p1.Y)) { return true; }
	if (p1.X == p2.X && test.X == p1.X && test.Y >= MinR64(p1.Y, p2.Y) && test.Y <= MaxR64(p1.Y, p2.Y)) { return true; }
	if (p2.X == p0.X && test.X == p2.X && test.Y >= MinR64(p2.Y, p0.Y) && test.Y <= MaxR64(p2.Y, p0.Y)) { return true; }
	if (p0.Y == p1.Y && test.Y == p0.Y && test.X >= MinR64(p0.X, p1.X) && test.X <= MaxR64(p0.X, p1.X)) { return true; }
	if (p1.Y == p2.Y && test.Y == p1.Y && test.X >= MinR64(p1.X, p2.X) && test.X <= MaxR64(p1.X, p2.X)) { return true; }
	if (p2.Y == p0.Y && test.Y == p2.Y && test.X >= MinR64(p2.X, p0.X) && test.X <= MaxR64(p2.X, p0.X)) { return true; }
	return (
		SignOfR64(DotV2d(SubV2d(p2, p0), perp0)) == SignOfR64(DotV2d(SubV2d(test, p0), perp0)) &&
		SignOfR64(DotV2d(SubV2d(p0, p1), perp1)) == SignOfR64(DotV2d(SubV2d(test, p1), perp1)) &&
		SignOfR64(DotV2d(SubV2d(p1, p2), perp2)) == SignOfR64(DotV2d(SubV2d(test, p2), perp2))
	);
}

//Returns new vertex count
PEXP uxx RemoveColinearAndDuplicateVerticesR64(uxx numVertices, v2d* vertices)
{
	uxx numVerticesRemoved = 0;
	uxx writeIndex = 0;
	for (uxx readIndex = 0; readIndex < numVertices; readIndex++)
	{
		v2d baseVert = vertices[readIndex];
		uxx numVerticesRemovedThisRound = 0;
		for (uxx forwardIndex = 0; forwardIndex+1 < numVertices; forwardIndex++)
		{
			uxx nextVertIndex = ((readIndex+forwardIndex+1) % numVertices);
			v2d nextVert = vertices[nextVertIndex];
			v2d nextNextVert = vertices[(readIndex+forwardIndex+2) % numVertices];
			v2d firstVec = NormalizeV2d(SubV2d(baseVert, nextVert));
			v2d secondVec = NormalizeV2d(SubV2d(nextNextVert, nextVert));
			r64 angleBetween =  AngleBetweenV2d(firstVec, secondVec);
			if (AreSimilarR64(AbsR64(angleBetween), Pi64, 0.001) || AreSimilarV2d(nextVert, nextNextVert, 0.001))
			{
				// PrintLine_D("Removing vertex[%llu] (%g,%g)(%g,%g)(%g,%g)",
				// 	nextVertIndex,
				// 	baseVert.x, baseVert.y,
				// 	nextVert.x, nextVert.y,
				// 	nextNextVert.x, nextNextVert.y
				// );
				numVerticesRemovedThisRound++;
			}
			else { break; }
		}
		if (readIndex != writeIndex)
		{
			vertices[writeIndex] = vertices[readIndex];
		}
		numVerticesRemoved += numVerticesRemovedThisRound;
		writeIndex++;
		readIndex += numVerticesRemovedThisRound;
	}
	return numVertices - numVerticesRemoved;
}

PEXPI TriangulateVert* GetPrevTriangulateVertR64(TriangulateVert* vertices, uxx totalNumVertices, uxx startIndex)
{
	Assert(totalNumVertices > 0);
	NotNull(vertices);
	for (uxx vIndex = ((startIndex > 0) ? startIndex-1 : (totalNumVertices-1)); vIndex != startIndex; vIndex = ((vIndex > 0) ? vIndex-1 : (totalNumVertices-1)))
	{
		if (!vertices[vIndex].removed) { return &vertices[vIndex]; }
	}
	return nullptr;
}
PEXPI TriangulateVert* GetNextTriangulateVertR64(TriangulateVert* vertices, uxx totalNumVertices, uxx startIndex)
{
	Assert(totalNumVertices > 0);
	NotNull(vertices);
	for (uxx vIndex = ((startIndex+1) % totalNumVertices); vIndex != startIndex; vIndex = ((vIndex+1) % totalNumVertices))
	{
		if (!vertices[vIndex].removed) { return &vertices[vIndex]; }
	}
	return nullptr;
}

PEXP uxx* Triangulate2DEarClipR64(Arena* arena, uxx numVertices, const v2d* vertices, uxx* numIndicesOut)
{
	Assert(numVertices == 0 || vertices != nullptr);
	SetOptionalOutPntr(numIndicesOut, 0);
	
	uxx numIndices = 0;
	uxx* result = nullptr;
	
	if (numVertices < 3) { return nullptr; }
	else if (numVertices == 3)
	{
		numIndices = 3;
		SetOptionalOutPntr(numIndicesOut, numIndices);
		if (arena == nullptr) { return nullptr; }
		result = AllocArray(uxx, arena, numIndices);
		NotNull(result);
		result[0] = 0; result[1] = 1; result[2] = 2;
		return result;
	}
	
	uxx numTriangles = numVertices - 2;
	numIndices = numTriangles*3;
	SetOptionalOutPntr(numIndicesOut, numIndices);
	if (arena == nullptr) { return nullptr; }
	result = AllocArray(uxx, arena, numIndices);
	NotNull(result);
	
	ScratchBegin1(scratch, arena);
	uxx numUnremovedVerts = numVertices;
	TriangulateVert* workingVerts = AllocArray(TriangulateVert, scratch, numVertices);
	NotNull(workingVerts);
	for (uxx vIndex = 0; vIndex < numVertices; vIndex++)
	{
		workingVerts[vIndex].removed = false;
		workingVerts[vIndex].index = vIndex;
		workingVerts[vIndex].pos = vertices[vIndex];
	}
	
	uxx resultIndex = 0;
	while (numUnremovedVerts > 3)
	{
		TriangulateVert* earVertex = nullptr;
		TriangulateVert* earPrevVertex = nullptr;
		TriangulateVert* earNextVertex = nullptr;
		for (uxx vIndex = 0; vIndex < numVertices; vIndex++)
		{
			TriangulateVert* vert1 = &workingVerts[vIndex];
			if (!vert1->removed)
			{
				TriangulateVert* vert0 = GetPrevTriangulateVertR64(workingVerts, numVertices, vIndex);
				TriangulateVert* vert2 = GetNextTriangulateVertR64(workingVerts, numVertices, vIndex);
				if (vert0 == nullptr || vert2 == nullptr || !IsTriangleClockwiseR64(vert0->pos, vert1->pos, vert2->pos)) { continue; }
				
				earVertex = vert1;
				earPrevVertex = vert0;
				earNextVertex = vert2;
				for (uxx vIndex2 = 0; vIndex2 < numVertices; vIndex2++)
				{
					TriangulateVert* otherVert = &workingVerts[vIndex2];
					if (!otherVert->removed && otherVert->index != vert0->index && otherVert->index != vert1->index && otherVert->index != vert2->index)
					{
						if (IsInsideTriangleR64(vert0->pos, vert1->pos, vert2->pos, otherVert->pos))
						{
							earVertex = nullptr;
							earPrevVertex = nullptr;
							earNextVertex = nullptr;
							break;
						}
					}
				}
				if (earVertex != nullptr) { break; }
			}
		}
		if (earVertex == nullptr)
		{
			// PrintLine_W("The polygon is invalid after %llu triangles!", resultIndex/3);
			ScratchEnd(scratch);
			if (CanArenaFree(arena)) { FreeArray(uxx, arena, numIndices, result); }
			return nullptr;
		}
		
		Assert(resultIndex+3 <= numIndices);
		result[resultIndex + 0] = earPrevVertex->index;
		result[resultIndex + 1] = earVertex->index;
		result[resultIndex + 2] = earNextVertex->index;
		resultIndex += 3;
		
		earVertex->removed = true;
		numUnremovedVerts--;
	}
	Assert(numUnremovedVerts == 3);
	
	TriangulateVert* finalCenterVert = GetNextTriangulateVertR64(workingVerts, numVertices, 0);
	NotNull(finalCenterVert);
	TriangulateVert* finalPrevVert = GetPrevTriangulateVertR64(workingVerts, numVertices, finalCenterVert->index);
	TriangulateVert* finalNextVert = GetNextTriangulateVertR64(workingVerts, numVertices, finalCenterVert->index);
	NotNull(finalPrevVert);
	NotNull(finalNextVert);
	Assert(finalPrevVert->index != finalNextVert->index);
	
	Assert(resultIndex == numIndices-3);
	result[resultIndex + 0] = finalPrevVert->index;
	result[resultIndex + 1] = finalCenterVert->index;
	result[resultIndex + 2] = finalNextVert->index;
	resultIndex += 3;
	
	ScratchEnd(scratch);
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _MISC_TRIANGULATION_H
