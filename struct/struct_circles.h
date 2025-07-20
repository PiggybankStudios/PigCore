/*
File:   struct_circles.h
Author: Taylor Robbins
Date:   02\02\2025
Description:
	** Contains Circle and Sphere structs which represent a shapes that have a
	** center (in 2D or 3D) and a radius
*/

#ifndef _STRUCT_CIRCLES_H
#define _STRUCT_CIRCLES_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "struct/struct_vectors.h"

typedef union Circle Circle;
union Circle
{
	r32 Elements[3];
	plex { r32 X, Y; r32 R; };
	plex { v2 Center; r32 Radius; };
};
#if LANGUAGE_IS_C
_Static_assert(sizeof(Circle) == sizeof(r32)*3, "Circle contains padding!");
#endif

typedef union Sphere Sphere;
union Sphere
{
	r32 Elements[4];
	plex { r32 X, Y, Z; r32 R; };
	plex { v3 Center; r32 Radius; };
};
#if LANGUAGE_IS_C
_Static_assert(sizeof(Sphere) == sizeof(r32)*4, "Sphere contains padding!");
#endif

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Circle NewCircle(r32 x, r32 y, r32 radius);
	PIG_CORE_INLINE Circle NewCircleV(v2 center, r32 radius);
	PIG_CORE_INLINE Sphere NewSphere(r32 x, r32 y, r32 z, r32 radius);
	PIG_CORE_INLINE Sphere NewSphereV(v3 center, r32 radius);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI Circle NewCircle(r32 x, r32 y, r32 radius)
{
	Circle result = ZEROED;
	result.X = x;
	result.Y = y;
	result.Radius = radius;
	return result;
}
PEXPI Circle NewCircleV(v2 center, r32 radius)
{
	Circle result = ZEROED;
	result.Center = center;
	result.Radius = radius;
	return result;
}

PEXPI Sphere NewSphere(r32 x, r32 y, r32 z, r32 radius)
{
	Sphere result = ZEROED;
	result.X = x;
	result.Y = y;
	result.Z = z;
	result.Radius = radius;
	return result;
}
PEXPI Sphere NewSphereV(v3 center, r32 radius)
{
	Sphere result = ZEROED;
	result.Center = center;
	result.Radius = radius;
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_CIRCLES_H
