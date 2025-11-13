/*
File:   struct_faces.h
Author: Taylor Robbins
Date:   02\02\2025
Description:
	** Contains Triangle2D, Triangle3D, Quad2D, and Quad3D structs which represent
	** fundamental shapes that we use for rendering "faces" in graphics pipelines
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
#define MakeTriangle2D_Const(v0x, v0y, v1x, v1y, v2x, v2y) { .Vert0=MakeV2((v0x), (v0y)), .Vert1=MakeV2((v1x), (v1y)), .Vert2=MakeV2((v2x), (v2y)) }
#define MakeTriangle2DV_Const(vert0, vert1, vert2)         { .Vert0=vert0, .Vert1=vert1, .Vert2=vert2 }
#define MakeTriangle2D(v0x, v0y, v1x, v1y, v2x, v2y)       NEW_STRUCT(Triangle2D)MakeTriangle2D_Const((v0x), (v0y), (v1x), (v1y), (v2x), (v2y))
#define MakeTriangle2DV(vert0, vert1, vert2)               NEW_STRUCT(Triangle2D)MakeTriangle2DV_Const((vert0), (vert1), (vert2))

typedef car Triangle3D Triangle3D;
car Triangle3D
{
	r32 Values[9];
	v3 Verts[3];
	plex { v3 Vert0, Vert1, Vert2; };
};
#define MakeTriangle3D_Const(v0x, v0y, v0z, v1x, v1y, v1z, v2x, v2y, v2z) { .Vert0=MakeV3((v0x), (v0y), (v0z)), .Vert1=MakeV3((v1x), (v1y), (v1z)), .Vert2=MakeV3((v2x), (v2y), (v2z)) }
#define MakeTriangle3DV_Const(vert0, vert1, vert2)                        { .Vert0=vert0, .Vert1=vert1, .Vert2=vert2 }
#define MakeTriangle3D(v0x, v0y, v0z, v1x, v1y, v1z, v2x, v2y, v2z)       NEW_STRUCT(Triangle3D)MakeTriangle3D_Const((v0x), (v0y), (v0z), (v1x), (v1y), (v1z), (v2x), (v2y), (v2z))
#define MakeTriangle3DV(vert0, vert1, vert2)                              NEW_STRUCT(Triangle3D)MakeTriangle3DV_Const((vert0), (vert1), (vert2))

typedef car Quad2D Quad2D;
car Quad2D
{
	r32 Values[8];
	v2 Verts[4];
	plex { v2 Vert0, Vert1, Vert2, Vert3; };
};
#define MakeQuad2D_Const(v0x, v0y, v1x, v1y, v2x, v2y, v3x, v3y) { .Vert0=MakeV2((v0x), (v0y)), .Vert1=MakeV2((v1x), (v1y)), .Vert2=MakeV2((v2x), (v2y)), .Vert3=MakeV2((v3x), (v3y)) }
#define MakeQuad2DV_Const(vert0, vert1, vert2, vert3)            { .Vert0=vert0, .Vert1=vert1, .Vert2=vert2, .Vert3=vert3 }
#define MakeQuad2D(v0x, v0y, v1x, v1y, v2x, v2y, v3x, v3y)       NEW_STRUCT(Quad2D)MakeQuad2D_Const((v0x), (v0y), (v1x), (v1y), (v2x), (v2y), (v3x), (v3y))
#define MakeQuad2DV(vert0, vert1, vert2, vert3)                  NEW_STRUCT(Quad2D)MakeQuad2DV_Const((vert0), (vert1), (vert2), (vert3))

typedef car Quad3D Quad3D;
car Quad3D
{
	r32 Values[12];
	v3 Verts[4];
	plex { v3 Vert0, Vert1, Vert2, Vert3; };
};
#define MakeQuad3D_Const(v0x, v0y, v0z, v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z) { .Vert0=MakeV2((v0x), (v0y), (v0z)), .Vert1=MakeV2((v1x), (v1y), (v1z)), .Vert2=MakeV2((v2x), (v2y), (v2z)), .Vert3=MakeV2((v3x), (v3y), (v3z)) }
#define MakeQuad3DV_Const(vert0, vert1, vert2, vert3)                                { .Vert0=vert0, .Vert1=vert1, .Vert2=vert2, .Vert3=vert3 }
#define MakeQuad3D(v0x, v0y, v0z, v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z)       NEW_STRUCT(Quad3D)MakeQuad3D_Const((v0x), (v0y), (v0z), (v1x), (v1y), (v1z), (v2x), (v2y), (v2z), (v3x), (v3y), (v3z))
#define MakeQuad3DV(vert0, vert1, vert2, vert3)                                      NEW_STRUCT(Quad3D)MakeQuad3DV_Const((vert0), (vert1), (vert2), (vert3))

#endif //  _STRUCT_FACES_H
