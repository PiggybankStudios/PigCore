/*
File:   misc_poly_simplify.h
Author: Taylor Robbins
Date:   09\12\2025
Description:
	** This file contains functions that help us simplify 2D polygons or any set
	** of 2D vertices connected in a continuous well-ordered manner. The goal is
	** to reduce the number of vertices in the polygon\line while maintaining a
	** mostly similar shape.
*/

#ifndef _MISC_POLY_SIMPLIFY_H
#define _MISC_POLY_SIMPLIFY_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "std/std_basic_math.h"
#include "struct/struct_vectors.h"

typedef plex SimpPolyVert SimpPolyVert;
plex SimpPolyVert
{
	v2 pos;
	i32 state; //0 = default, -1 = removed, 1 = committed
};
typedef plex SimpPolyVertR64 SimpPolyVertR64;
plex SimpPolyVertR64
{
	v2d pos;
	i32 state; //0 = default, -1 = removed, 1 = committed
};

typedef plex SimpPolygon SimpPolygon;
plex SimpPolygon
{
	uxx numVertices;
	SimpPolyVert* vertices;
};
typedef plex SimpPolygonR64 SimpPolygonR64;
plex SimpPolygonR64
{
	uxx numVertices;
	SimpPolyVertR64* vertices;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	r32 DistanceToLine(v2 lineStart, v2 lineEnd, v2 position);
	r64 DistanceToLineR64(v2d lineStart, v2d lineEnd, v2d position);
	uxx SimplifyPolygon(SimpPolygon* poly, r32 epsilon);
	uxx SimplifyPolygonR64(SimpPolygonR64* poly, r64 epsilon);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP r32 DistanceToLine(v2 lineStart, v2 lineEnd, v2 position)
{
	if (AreEqualV2(lineStart, lineEnd))
	{
		return LengthV2(SubV2(position, lineStart));
	}
	else
	{
		r32 lineLength = LengthV2(SubV2(lineEnd, lineStart));
		v2 forwardVec = ShrinkV2(SubV2(lineEnd, lineStart), lineLength);
		v2 relativePos = SubV2(position, lineStart);
		r32 forwardDot = DotV2(relativePos, forwardVec);
		if (forwardDot <= 0.0f) { return LengthV2(relativePos); }
		else if (forwardDot >= lineLength) { return LengthV2(SubV2(position, lineEnd)); }
		else
		{
			v2 normalVec = PerpRightV2(forwardVec);
			r32 normalDot = DotV2(relativePos, normalVec);
			return AbsR32(normalDot);
		}
	}
}
PEXP r64 DistanceToLineR64(v2d lineStart, v2d lineEnd, v2d position)
{
	if (AreEqualV2d(lineStart, lineEnd))
	{
		return LengthV2d(SubV2d(position, lineStart));
	}
	else
	{
		r64 lineLength = LengthV2d(SubV2d(lineEnd, lineStart));
		v2d forwardVec = ShrinkV2d(SubV2d(lineEnd, lineStart), lineLength);
		v2d relativePos = SubV2d(position, lineStart);
		r64 forwardDot = DotV2d(relativePos, forwardVec);
		if (forwardDot <= 0.0) { return LengthV2d(relativePos); }
		else if (forwardDot >= lineLength) { return LengthV2d(SubV2d(position, lineEnd)); }
		else
		{
			v2d normalVec = PerpRightV2d(forwardVec);
			r64 normalDot = DotV2d(relativePos, normalVec);
			return AbsR64(normalDot);
		}
	}
}

static uxx SimplifyPolygon_(SimpPolygon* poly, r32 epsilon, uxx startIndex, uxx count)
{
	if (count <= 2) { return count; }
	v2 startPos = poly->vertices[startIndex].pos;
	v2 endPos = poly->vertices[startIndex+(count-1)].pos;
	
	r32 greatestDist = 0.0f;
	uxx greatestIndex = count;
	for (uxx vIndex = 1; vIndex < count-1; vIndex++)
	{
		r32 vertDistance = DistanceToLine(startPos, endPos, poly->vertices[startIndex + vIndex].pos);
		if (greatestIndex >= count || vertDistance > greatestDist)
		{
			greatestDist = vertDistance;
			greatestIndex = vIndex;
		}
	}
	
	if (greatestDist > epsilon)
	{
		poly->vertices[startIndex + greatestIndex].state = 1;
		uxx result1 = SimplifyPolygon_(poly, epsilon, startIndex, greatestIndex+1);
		uxx result2 = SimplifyPolygon_(poly, epsilon, startIndex + greatestIndex, count - greatestIndex);
		return result1 + result2 - 1; //-1 to subtract duplicate report of shared vertex
	}
	else
	{
		for (uxx vIndex = 1; vIndex < count-1; vIndex++) { poly->vertices[startIndex+vIndex].state = -1; }
		return 2;
	}
}
static uxx SimplifyPolygonR64_(SimpPolygonR64* poly, r64 epsilon, uxx startIndex, uxx count)
{
	if (count <= 2) { return count; }
	v2d startPos = poly->vertices[startIndex].pos;
	v2d endPos = poly->vertices[startIndex+(count-1)].pos;
	
	r64 greatestDist = 0.0;
	uxx greatestIndex = count;
	for (uxx vIndex = 1; vIndex < count-1; vIndex++)
	{
		r64 vertDistance = DistanceToLineR64(startPos, endPos, poly->vertices[startIndex + vIndex].pos);
		if (greatestIndex >= count || vertDistance > greatestDist)
		{
			greatestDist = vertDistance;
			greatestIndex = vIndex;
		}
	}
	
	if (greatestDist > epsilon)
	{
		poly->vertices[startIndex + greatestIndex].state = 1;
		uxx result1 = SimplifyPolygonR64_(poly, epsilon, startIndex, greatestIndex+1);
		uxx result2 = SimplifyPolygonR64_(poly, epsilon, startIndex + greatestIndex, count - greatestIndex);
		return result1 + result2 - 1; //-1 to subtract duplicate report of shared vertex
	}
	else
	{
		for (uxx vIndex = 1; vIndex < count-1; vIndex++) { poly->vertices[startIndex+vIndex].state = -1; }
		return 2;
	}
}

//NOTE: This is the Ramer-Douglas-Peucker algorithm: https://en.m.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
PEXP uxx SimplifyPolygon(SimpPolygon* poly, r32 epsilon)
{
	NotNull(poly);
	Assert(poly->numVertices == 0 || poly->vertices != nullptr);
	if (poly->numVertices <= 2) { return poly->numVertices; }
	bool isClosedLoop = AreEqualV2(poly->vertices[0].pos, poly->vertices[poly->numVertices-1].pos);
	if (poly->numVertices == 3 && isClosedLoop) { return poly->numVertices; }
	
	//Commit the 2 endpoints and then call the recursive bit with startIndex and count
	poly->vertices[0].state = 1;
	poly->vertices[poly->numVertices-1].state = 1;
	uxx result = SimplifyPolygon_(poly, epsilon, 0, poly->numVertices);
	
	return result;
}

//NOTE: This is the Ramer-Douglas-Peucker algorithm: https://en.m.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
PEXP uxx SimplifyPolygonR64(SimpPolygonR64* poly, r64 epsilon)
{
	NotNull(poly);
	Assert(poly->numVertices == 0 || poly->vertices != nullptr);
	if (poly->numVertices <= 2) { return poly->numVertices; }
	bool isClosedLoop = AreEqualV2d(poly->vertices[0].pos, poly->vertices[poly->numVertices-1].pos);
	if (poly->numVertices == 3 && isClosedLoop) { return poly->numVertices; }
	
	//Commit the 2 endpoints and then call the recursive bit with startIndex and count
	poly->vertices[0].state = 1;
	poly->vertices[poly->numVertices-1].state = 1;
	uxx result = SimplifyPolygonR64_(poly, epsilon, 0, poly->numVertices);
	
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _MISC_POLY_SIMPLIFY_H

#if defined(_MEM_SCRATCH_H) && defined(_MISC_POLY_SIMPLIFY_H)
#include "cross/cross_scratch_and_poly_simplify.h"
#endif
