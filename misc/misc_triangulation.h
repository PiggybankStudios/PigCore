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

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _MISC_TRIANGULATION_H
