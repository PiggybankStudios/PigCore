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
	r32 Elements[3];
	plex { r32 X, Y; r32 R; };
	plex { v2 Center; r32 Radius; };
};
#define MakeCircle(x, y, radius) NEW_STRUCT(Circle){ .X=(x), .Y=(y), .R=(radius) }
#define MakeCircleV(center, radius) NEW_STRUCT(Circle){ .Center=(center), .Radius=(radius) }

typedef car Sphere Sphere;
car Sphere
{
	r32 Elements[4];
	plex { r32 X, Y, Z; r32 R; };
	plex { v3 Center; r32 Radius; };
};
#define MakeSphere(x, y, z, radius) NEW_STRUCT(Sphere){.X=(x), .Y=(y), .Z=(z), .R=(radius) }
#define MakeSphereV(center, radius) NEW_STRUCT(Sphere){.Center=(center), .Radius=(radius) }

#endif //  _STRUCT_CIRCLES_H
