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

typedef car Circle Circle;
car Circle
{
	r32 elem[3];
	plex { r32 x, y; r32 r; };
	plex { v2 center; r32 radius; };
};
#define MakeCircle_Const(xValue, yValue, radiusValue) { .x=(xValue), .y=(yValue), .r=(radiusValue) }
#define MakeCircleV_Const(centerValue, radiusValue)   { .center=centerValue, .radius=(radiusValue) }
#define MakeCircle(x, y, radius)                      NEW_STRUCT(Circle)MakeCircle_Const((x), (y), (radius))
#define MakeCircleV(center, radius)                   NEW_STRUCT(Circle)MakeCircleV_Const((center), (radius))

typedef car Sphere Sphere;
car Sphere
{
	r32 elem[4];
	plex { r32 x, y, z; r32 r; };
	plex { v3 center; r32 radius; };
};
#define MakeSphere_Const(xValue, yValue, zValue, radiusValue) { .x=(xValue), .y=(yValue), .z=(zValue), .r=(radiusValue) }
#define MakeSphereV_Const(centerValue, radiusValue)           { .center=centerValue, .radius=(radiusValue) }
#define MakeSphere(x, y, z, radius)                           NEW_STRUCT(Sphere)MakeSphere_Const((x), (y), (z), (radius))
#define MakeSphereV(center, radius)                           NEW_STRUCT(Sphere)MakeSphereV_Const((center), (radius))

#endif //  _STRUCT_CIRCLES_H
