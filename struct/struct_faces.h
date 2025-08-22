/*
File:   struct_faces.h
Author: Taylor Robbins
Date:   02\02\2025
Description:
	** Contains Triangle2D, Triangle3D, Quad2D, and Quad3D structs which represent
	** fundamental shapes that we use for rendering "faces" in a 3D graphics pipelines
*/

#ifndef _STRUCT_FACES_H
#define _STRUCT_FACES_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "struct/struct_vectors.h"

typedef car Triangle2D Triangle2D;
car Triangle2D
{
	r32 Values[6];
	v2 Verts[3];
	plex { v2 Vert0, Vert1, Vert2; };
};
#if LANGUAGE_IS_C
_Static_assert(sizeof(Triangle2D) == sizeof(r32)*6, "Triangle2D contains padding!");
#endif

typedef car Triangle3D Triangle3D;
car Triangle3D
{
	r32 Values[9];
	v3 Verts[3];
	plex { v3 Vert0, Vert1, Vert2; };
};
#if LANGUAGE_IS_C
_Static_assert(sizeof(Triangle3D) == sizeof(r32)*9, "Triangle3D contains padding!");
#endif

typedef car Quad2D Quad2D;
car Quad2D
{
	r32 Values[8];
	v2 Verts[4];
	plex { v2 Vert0, Vert1, Vert2, Vert3; };
};
#if LANGUAGE_IS_C
_Static_assert(sizeof(Quad2D) == sizeof(r32)*8, "Quad2D contains padding!");
#endif

typedef car Quad3D Quad3D;
car Quad3D
{
	r32 Values[12];
	v3 Verts[4];
	plex { v3 Vert0, Vert1, Vert2, Vert3; };
};
#if LANGUAGE_IS_C
_Static_assert(sizeof(Quad3D) == sizeof(r32)*12, "Quad3D contains padding!");
#endif

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Triangle2D NewTriangle2D(v2 vert0, v2 vert1, v2 vert2);
	PIG_CORE_INLINE Triangle3D NewTriangle3D(v3 vert0, v3 vert1, v3 vert2);
	PIG_CORE_INLINE Quad2D NewQuad2D(v2 vert0, v2 vert1, v2 vert2, v2 vert3);
	PIG_CORE_INLINE Quad3D NewQuad3D(v3 vert0, v3 vert1, v3 vert2, v3 vert3);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI Triangle2D NewTriangle2D(v2 vert0, v2 vert1, v2 vert2)
{
	Triangle2D result = ZEROED;
	result.Vert0 = vert0;
	result.Vert1 = vert1;
	result.Vert2 = vert2;
	return result;
}
PEXPI Triangle3D NewTriangle3D(v3 vert0, v3 vert1, v3 vert2)
{
	Triangle3D result = ZEROED;
	result.Vert0 = vert0;
	result.Vert1 = vert1;
	result.Vert2 = vert2;
	return result;
}

PEXPI Quad2D NewQuad2D(v2 vert0, v2 vert1, v2 vert2, v2 vert3)
{
	Quad2D result = ZEROED;
	result.Vert0 = vert0;
	result.Vert1 = vert1;
	result.Vert2 = vert2;
	result.Vert3 = vert3;
	return result;
}
PEXPI Quad3D NewQuad3D(v3 vert0, v3 vert1, v3 vert2, v3 vert3)
{
	Quad3D result = ZEROED;
	result.Vert0 = vert0;
	result.Vert1 = vert1;
	result.Vert2 = vert2;
	result.Vert3 = vert3;
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_FACES_H
