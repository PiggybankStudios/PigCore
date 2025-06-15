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
#include "struct/struct_rectangles.h"
#include "struct/struct_circles.h"
#include "struct/struct_color.h"
#include "gfx/gfx_vertices.h"

#define BOX_NUM_FACES                   6
#define BOX_MESH_NUM_VERTICES_PER_FACES 4
#define BOX_MESH_NUM_INDICES_PER_FACES  6 //2 triangles, 3 indices/triangle
#define BOX_MESH_TOP_FACE_INDEX         0
#define BOX_MESH_RIGHT_FACE_INDEX       1
#define BOX_MESH_FRONT_FACE_INDEX       2
#define BOX_MESH_LEFT_FACE_INDEX        3
#define BOX_MESH_BACK_FACE_INDEX        4
#define BOX_MESH_BOTTOM_FACE_INDEX      5
#define BOX_MESH_INDICES_PER_FACE       6

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
	PIG_CORE_INLINE void FreeGeneratedMesh(GeneratedMesh* mesh);
	GeneratedMesh GenerateVertsForBoxEx(Arena* arena, Box boundingBox, Color32* colors);
	PIG_CORE_INLINE GeneratedMesh GenerateVertsForBox(Arena* arena, Box boundingBox, Color32 color);
	GeneratedMesh GenerateVertsForSphere(Arena* arena, Sphere sphere, uxx numRings, uxx numSegments, Color32 color);
#endif //!PIG_CORE_IMPLEMENTATION


// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void FreeGeneratedMesh(GeneratedMesh* mesh)
{
	NotNull(mesh);
	if (mesh->arena != nullptr)
	{
		FreeArray(Vertex3D, mesh->arena, mesh->numVertices, mesh->vertices);
		FreeArray(i32, mesh->arena, mesh->numIndices, mesh->indices);
	}
	ClearPointer(mesh);
}

PEXP GeneratedMesh GenerateVertsForBoxEx(Arena* arena, Box boundingBox, Color32* colors)
{
	GeneratedMesh result = ZEROED;
	result.arena = arena;
	result.numVertices = BOX_NUM_FACES * BOX_MESH_NUM_VERTICES_PER_FACES; //24 vertices
	result.numIndices = BOX_NUM_FACES * BOX_MESH_NUM_INDICES_PER_FACES; //36 indices
	if (arena == nullptr) { return result; }
	NotNull(colors);
	
	result.vertices = AllocArray(Vertex3D, arena, result.numVertices);
	NotNull(result.vertices);
	result.indices = AllocArray(i32, arena, result.numIndices);
	NotNull(result.indices);
	
	v4 colorsV4[BOX_NUM_FACES];
	colorsV4[0] = ToV4FromColor32(colors[0]);
	colorsV4[1] = ToV4FromColor32(colors[1]);
	colorsV4[2] = ToV4FromColor32(colors[2]);
	colorsV4[3] = ToV4FromColor32(colors[3]);
	colorsV4[4] = ToV4FromColor32(colors[4]);
	colorsV4[5] = ToV4FromColor32(colors[5]);
	
	v3 vertPositions[8];
	const uxx blbIndex = 0; //bottomLeftBack
	const uxx brbIndex = 1; //bottomRightBack
	const uxx blfIndex = 2; //bottomLeftFront
	const uxx brfIndex = 3; //bottomRightFront
	const uxx tlbIndex = 4; //topLeftBack
	const uxx trbIndex = 5; //topRightBack
	const uxx tlfIndex = 6; //topLeftFront
	const uxx trfIndex = 7; //topRightFront
	vertPositions[blbIndex] = NewV3(boundingBox.X,                     boundingBox.Y,                      boundingBox.Z);
	vertPositions[brbIndex] = NewV3(boundingBox.X + boundingBox.Width, boundingBox.Y,                      boundingBox.Z);
	vertPositions[blfIndex] = NewV3(boundingBox.X,                     boundingBox.Y,                      boundingBox.Z + boundingBox.Depth);
	vertPositions[brfIndex] = NewV3(boundingBox.X + boundingBox.Width, boundingBox.Y,                      boundingBox.Z + boundingBox.Depth);
	vertPositions[tlbIndex] = NewV3(boundingBox.X,                     boundingBox.Y + boundingBox.Height, boundingBox.Z);
	vertPositions[trbIndex] = NewV3(boundingBox.X + boundingBox.Width, boundingBox.Y + boundingBox.Height, boundingBox.Z);
	vertPositions[tlfIndex] = NewV3(boundingBox.X,                     boundingBox.Y + boundingBox.Height, boundingBox.Z + boundingBox.Depth);
	vertPositions[trfIndex] = NewV3(boundingBox.X + boundingBox.Width, boundingBox.Y + boundingBox.Height, boundingBox.Z + boundingBox.Depth);
	
	i32 vIndex = 0;
	i32 iIndex = 0;
	
	//top face (+y) forward is up
	i32 topStartIndex = vIndex; DebugAssert(topStartIndex == BOX_MESH_TOP_FACE_INDEX * BOX_MESH_NUM_VERTICES_PER_FACES);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[tlfIndex], V3_Up, NewV2(0, 0), colorsV4[BOX_MESH_TOP_FACE_INDEX]);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[trfIndex], V3_Up, NewV2(1, 0), colorsV4[BOX_MESH_TOP_FACE_INDEX]);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[trbIndex], V3_Up, NewV2(1, 1), colorsV4[BOX_MESH_TOP_FACE_INDEX]);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[tlbIndex], V3_Up, NewV2(0, 1), colorsV4[BOX_MESH_TOP_FACE_INDEX]);
	
	//right face (+x) up is up
	i32 rightStartIndex = vIndex; DebugAssert(rightStartIndex == BOX_MESH_RIGHT_FACE_INDEX * BOX_MESH_NUM_VERTICES_PER_FACES);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[trbIndex], V3_Right, NewV2(0, 0), colorsV4[BOX_MESH_RIGHT_FACE_INDEX]);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[trfIndex], V3_Right, NewV2(1, 0), colorsV4[BOX_MESH_RIGHT_FACE_INDEX]);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[brfIndex], V3_Right, NewV2(1, 1), colorsV4[BOX_MESH_RIGHT_FACE_INDEX]);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[brbIndex], V3_Right, NewV2(0, 1), colorsV4[BOX_MESH_RIGHT_FACE_INDEX]);
	
	//front face (+z) up is up
	i32 frontStartIndex = vIndex; DebugAssert(frontStartIndex == BOX_MESH_FRONT_FACE_INDEX * BOX_MESH_NUM_VERTICES_PER_FACES);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[trfIndex], V3_Forward, NewV2(0, 0), colorsV4[BOX_MESH_FRONT_FACE_INDEX]);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[tlfIndex], V3_Forward, NewV2(1, 0), colorsV4[BOX_MESH_FRONT_FACE_INDEX]);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[blfIndex], V3_Forward, NewV2(1, 1), colorsV4[BOX_MESH_FRONT_FACE_INDEX]);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[brfIndex], V3_Forward, NewV2(0, 1), colorsV4[BOX_MESH_FRONT_FACE_INDEX]);
	
	//left face (-x) up is up
	i32 leftStartIndex = vIndex; DebugAssert(leftStartIndex == BOX_MESH_LEFT_FACE_INDEX * BOX_MESH_NUM_VERTICES_PER_FACES);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[tlfIndex], V3_Left, NewV2(0, 0), colorsV4[BOX_MESH_LEFT_FACE_INDEX]);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[tlbIndex], V3_Left, NewV2(1, 0), colorsV4[BOX_MESH_LEFT_FACE_INDEX]);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[blbIndex], V3_Left, NewV2(1, 1), colorsV4[BOX_MESH_LEFT_FACE_INDEX]);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[blfIndex], V3_Left, NewV2(0, 1), colorsV4[BOX_MESH_LEFT_FACE_INDEX]);
	
	//back face (-z) up is up
	i32 backStartIndex = vIndex; DebugAssert(backStartIndex == BOX_MESH_BACK_FACE_INDEX * BOX_MESH_NUM_VERTICES_PER_FACES);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[tlbIndex], V3_Backward, NewV2(0, 0), colorsV4[BOX_MESH_BACK_FACE_INDEX]);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[trbIndex], V3_Backward, NewV2(1, 0), colorsV4[BOX_MESH_BACK_FACE_INDEX]);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[brbIndex], V3_Backward, NewV2(1, 1), colorsV4[BOX_MESH_BACK_FACE_INDEX]);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[blbIndex], V3_Backward, NewV2(0, 1), colorsV4[BOX_MESH_BACK_FACE_INDEX]);
	
	//bottom face (-y) backward is up
	i32 bottomStartIndex = vIndex; DebugAssert(bottomStartIndex == BOX_MESH_BOTTOM_FACE_INDEX * BOX_MESH_NUM_VERTICES_PER_FACES);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[blbIndex], V3_Down, NewV2(0, 0), colorsV4[BOX_MESH_BOTTOM_FACE_INDEX]);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[brbIndex], V3_Down, NewV2(1, 0), colorsV4[BOX_MESH_BOTTOM_FACE_INDEX]);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[brfIndex], V3_Down, NewV2(1, 1), colorsV4[BOX_MESH_BOTTOM_FACE_INDEX]);
	result.vertices[vIndex++] = NewVertex3D(vertPositions[blfIndex], V3_Down, NewV2(0, 1), colorsV4[BOX_MESH_BOTTOM_FACE_INDEX]);
	
	DebugAssert((uxx)vIndex == result.numVertices);
	
	//top face
	result.indices[iIndex++] = topStartIndex+0;
	result.indices[iIndex++] = topStartIndex+1;
	result.indices[iIndex++] = topStartIndex+3;
	result.indices[iIndex++] = topStartIndex+2;
	result.indices[iIndex++] = topStartIndex+3;
	result.indices[iIndex++] = topStartIndex+1;
	
	//right face
	result.indices[iIndex++] = rightStartIndex+0;
	result.indices[iIndex++] = rightStartIndex+1;
	result.indices[iIndex++] = rightStartIndex+3;
	result.indices[iIndex++] = rightStartIndex+2;
	result.indices[iIndex++] = rightStartIndex+3;
	result.indices[iIndex++] = rightStartIndex+1;
	
	//front face
	result.indices[iIndex++] = frontStartIndex+0;
	result.indices[iIndex++] = frontStartIndex+1;
	result.indices[iIndex++] = frontStartIndex+3;
	result.indices[iIndex++] = frontStartIndex+2;
	result.indices[iIndex++] = frontStartIndex+3;
	result.indices[iIndex++] = frontStartIndex+1;
	
	//left face
	result.indices[iIndex++] = leftStartIndex+0;
	result.indices[iIndex++] = leftStartIndex+1;
	result.indices[iIndex++] = leftStartIndex+3;
	result.indices[iIndex++] = leftStartIndex+2;
	result.indices[iIndex++] = leftStartIndex+3;
	result.indices[iIndex++] = leftStartIndex+1;
	
	//back face
	result.indices[iIndex++] = backStartIndex+0;
	result.indices[iIndex++] = backStartIndex+1;
	result.indices[iIndex++] = backStartIndex+3;
	result.indices[iIndex++] = backStartIndex+2;
	result.indices[iIndex++] = backStartIndex+3;
	result.indices[iIndex++] = backStartIndex+1;
	
	//bottom face
	result.indices[iIndex++] = bottomStartIndex+0;
	result.indices[iIndex++] = bottomStartIndex+1;
	result.indices[iIndex++] = bottomStartIndex+3;
	result.indices[iIndex++] = bottomStartIndex+2;
	result.indices[iIndex++] = bottomStartIndex+3;
	result.indices[iIndex++] = bottomStartIndex+1;
	
	DebugAssert((uxx)iIndex == result.numIndices);
	return result;
}
PEXPI GeneratedMesh GenerateVertsForBox(Arena* arena, Box boundingBox, Color32 color)
{
	Color32 colors[BOX_NUM_FACES] = { color, color, color, color, color, color };
	return GenerateVertsForBoxEx(arena, boundingBox, &colors[0]);
}

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
			Assert((uxx)ringVertIndex < result.numVertices);
			result.vertices[ringVertIndex].position = NewV3(sphere.X + CosR32(segmentAngle) * ringRadius, ringY, sphere.Z + SinR32(segmentAngle) * ringRadius);
			result.vertices[ringVertIndex].normal = NormalizeV3(SubV3(result.vertices[ringVertIndex].position, sphere.Center));
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
