/*
File:   gfx_mesh_generation.h
Author: Taylor Robbins
Date:   02\02\2025
Description:
	** Contains algorithms for generating 3D mesh vertices and indices for common geometric shapes (like cubes, spheres, etc.)
*/

#ifndef _GFX_MESH_GENERATION_H
#define _GFX_MESH_GENERATION_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "mem/mem_arena.h"
#include "struct/struct_vectors.h"
#include "struct/struct_circles.h"
#include "gfx/gfx_vertices.h"

typedef struct GeneratedMesh GeneratedMesh;
struct GeneratedMesh
{
	Arena* arena;
	uxx numVertices;
	uxx numIndices;
	Vertex3D* vertices;
	i32* indices;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	GeneratedMesh GenerateVertsForSphere(Arena* arena, Sphere sphere, uxx numRings, uxx numSegments, Color32 color);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP GeneratedMesh GenerateVertsForSphere(Arena* arena, Sphere sphere, uxx numRings, uxx numSegments, Color32 color)
{
	Assert(numRings >= 1);
	Assert(numSegments >= 3);
	
	GeneratedMesh result = ZEROED;
	result.arena = arena;
	result.numVertices = 2; //top and bottom verts
	result.numVertices += numSegments * numRings;
	result.numIndices = numSegments * 2 * 3; //top and bottom triangles
	result.numIndices += numSegments*2 * (numRings-1) * 3;
	if (arena == nullptr) { return result; }
	
	result.vertices = AllocArray(Vertex3D, arena, result.numVertices);
	NotNull(result.vertices);
	result.indices = AllocArray(i32, arena, result.numIndices);
	NotNull(result.indices);
	MyMemSet(result.indices, 0x00, sizeof(i32) * result.numIndices); //TODO: Remove me!
	
	const i32 bottomCenterIndex = 0;
	const i32 topCenterIndex = (i32)result.numVertices-1;
	
	result.vertices[topCenterIndex].position    = NewV3(sphere.X, sphere.Y + sphere.Radius, sphere.Z);
	result.vertices[bottomCenterIndex].position = NewV3(sphere.X, sphere.Y - sphere.Radius, sphere.Z);
	
	v4r colorV4r = ToV4rFromColor32(color);
	r32 ringStep = Pi32 / (r32)(numRings+1);
	r32 segmentStep = TwoPi32 / (r32)(numSegments);
	r32 texCoordStepX = 1.0f / (r32)numSegments;
	r32 texCoordStepY = 1.0f / (r32)(numRings-1);
	for (uxx rIndex = 0; rIndex < numRings; rIndex++)
	{
		r32 ringAngle = -HalfPi32 + (ringStep * (rIndex+1));
		r32 ringY = sphere.Y + (SinR32(ringAngle) * sphere.Radius);
		r32 ringRadius = CosR32(ringAngle) * sphere.Radius;
		for (uxx sIndex = 0; sIndex < numSegments; sIndex++)
		{
			r32 segmentAngle = (sIndex * segmentStep);
			i32 ringVertIndex = (i32)(1 + (rIndex * numSegments) + sIndex);
			Assert(ringVertIndex < result.numVertices);
			result.vertices[ringVertIndex].position = NewV3(sphere.X + CosR32(segmentAngle) * ringRadius, ringY, sphere.Z + SinR32(segmentAngle) * ringRadius);
			result.vertices[ringVertIndex].normal = Normalize(Sub(result.vertices[ringVertIndex].position, sphere.Center));
			result.vertices[ringVertIndex].texCoord = NewV2(1.0f - ((r32)sIndex * texCoordStepX), 1.0f - ((r32)rIndex * texCoordStepY));
			result.vertices[ringVertIndex].color = colorV4r;
		}
	}
	
	for (uxx rIndex = 0; rIndex <= numRings; rIndex++)
	{
		for (uxx sIndex = 0; sIndex < numSegments; sIndex++)
		{
			i32 upperVertIndex = 0;
			if (rIndex < numRings) { upperVertIndex = (i32)(1 + (rIndex * numSegments) + sIndex); }
			else { upperVertIndex = topCenterIndex; }
			Assert((uxx)upperVertIndex < result.numVertices);
			
			i32 lowerVertIndex = 0;
			if (rIndex > 0) { lowerVertIndex = (i32)(1 + ((rIndex-1) * numSegments) + sIndex); }
			else { lowerVertIndex = bottomCenterIndex; }
			Assert((uxx)lowerVertIndex < result.numVertices);
			
			i32 upperNextVertIndex = 0;
			if (rIndex < numRings)
			{
				if (sIndex+1 < numSegments) { upperNextVertIndex = (i32)(1 + (rIndex * numSegments) + (sIndex+1)); }
				else { upperNextVertIndex = (i32)(1 + (rIndex * numSegments) + (0)); }
			}
			else { upperNextVertIndex = topCenterIndex; }
			Assert((uxx)upperNextVertIndex < result.numVertices);
			
			i32 lowerNextVertIndex = 0;
			if (rIndex > 0)
			{
				if (sIndex+1 < numSegments) { lowerNextVertIndex = (i32)(1 + ((rIndex-1) * numSegments) + (sIndex+1)); }
				else { lowerNextVertIndex = (i32)(1 + ((rIndex-1) * numSegments) + (0)); }
			}
			else { lowerNextVertIndex = bottomCenterIndex; }
			Assert((uxx)lowerNextVertIndex < result.numVertices);
			
			i32 ringBaseIndex = 0;
			if (rIndex > 0) { ringBaseIndex = (i32)((numSegments * 3) + (numSegments * (rIndex-1) * 3 * 2)); }
			if (rIndex > 0 && rIndex < numRings)
			{
				i32 triIndex1 = ringBaseIndex + (i32)(sIndex * 3 * 2) + 0;
				i32 triIndex2 = ringBaseIndex + (i32)(sIndex * 3 * 2) + 3;
				Assert((uxx)triIndex1+3 <= result.numIndices);
				Assert((uxx)triIndex2+3 <= result.numIndices);
				
				result.indices[triIndex1 + 0] = upperVertIndex;
				result.indices[triIndex1 + 1] = upperNextVertIndex;
				result.indices[triIndex1 + 2] = lowerVertIndex;
				
				result.indices[triIndex2 + 0] = lowerNextVertIndex;
				result.indices[triIndex2 + 1] = lowerVertIndex;
				result.indices[triIndex2 + 2] = upperNextVertIndex;
			}
			else
			{
				i32 triIndex = ringBaseIndex + (i32)(sIndex * 3);
				Assert((uxx)triIndex < result.numIndices);
				if (rIndex == 0)
				{
					result.indices[triIndex + 0] = lowerVertIndex;
					result.indices[triIndex + 1] = upperVertIndex;
					result.indices[triIndex + 2] = upperNextVertIndex;
				}
				else //rIndex == numRings
				{
					result.indices[triIndex + 0] = upperVertIndex;
					result.indices[triIndex + 1] = lowerNextVertIndex;
					result.indices[triIndex + 2] = lowerVertIndex;
				}
			}
		}
	}
	
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _GFX_MESH_GENERATION_H
