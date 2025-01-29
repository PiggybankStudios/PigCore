/*
File:   gfx_vertices.h
Author: Taylor Robbins
Date:   01\28\2025
Description:
	** Contains various structures that represent vertices in a 2D or 3D graphical application
*/

#ifndef _GFX_VERTICES_H
#define _GFX_VERTICES_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "struct/struct_vectors.h"

typedef union Vertex2D Vertex2D;
union Vertex2D
{
	r32 values[8];
	struct
	{
		v2 position;
		v2 texCoord;
		v4r color;
	};
};
_Static_assert(sizeof(Vertex2D) == sizeof(r32)*8);

typedef union Vertex3D Vertex3D;
union Vertex3D
{
	r32 values[9];
	struct
	{
		v3 position;
		v2 texCoord;
		v4r color;
	};
};
_Static_assert(sizeof(Vertex3D) == sizeof(r32)*9);

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Vertex2D NewVertex2D(v2 position, v2 texCoord, v4 color);
	PIG_CORE_INLINE Vertex3D NewVertex3D(v3 position, v2 texCoord, v4 color);
#endif

#define Vertex2D_Size       sizeof(Vertex2D)
#define Vertex2D_NumFloats  (sizeof(Vertex2D)/sizeof(r32))
#define Vertex2D_Zero       NewVertex2D(V2_Zero_Const, V2_Zero_Const, V4_Zero_Const)
#define Vertex2D_Zero_Const { .position = V2_Zero_Const, .texCoord = V2_Zero_Const, .color = V4_Zero_Const }

#define Vertex3D_Size       sizeof(Vertex3D)
#define Vertex3D_NumFloats  (sizeof(Vertex3D)/sizeof(r32))
#define Vertex3D_Zero       NewVertex3D(V3_Zero_Const, V2_Zero_Const, V4_Zero_Const)
#define Vertex3D_Zero_Const { .position = V3_Zero_Const, .texCoord = V2_Zero_Const, .color = V4_Zero_Const }

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI Vertex2D NewVertex2D(v2 position, v2 texCoord, v4 color)
{
	Vertex2D result;
	result.position = position;
	result.texCoord = texCoord;
	result.color = ToV4rFrom4(color);
	return result;
}

PEXPI Vertex3D NewVertex3D(v3 position, v2 texCoord, v4 color)
{
	Vertex3D result;
	result.position = position;
	result.texCoord = texCoord;
	result.color = ToV4rFrom4(color);
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _GFX_VERTICES_H
