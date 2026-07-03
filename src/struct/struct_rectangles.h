/*
File:   struct_rectangles.h
Author: Taylor Robbins
Date:   01\31\2025
Description:
	** This file contains 2D and 3D structs for both AABB and OBB type rectangular shapes
	** Namely: Rectanglef (2D AABB), Box (3D AABB), Obb2D (2D OBB), Obb3D (3D OBB)
	** and also Rectangli and Boxi (like Rectangle and Boxi but i32 members)
	** These are also aliased as short lowercase names since they are so commonly used:
	**   rec, reci, box, boxi, obb2, obb3
*/

#ifndef _STRUCT_RECTANGLES_H
#define _STRUCT_RECTANGLES_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "std/std_basic_math.h"
#include "struct/struct_vectors.h"
#include "struct/struct_quaternion.h"

//NOTE: "Rectangle" conflicts with a windows.h type so we call it Rectangle_R32.
//      This is mostly fine because we usually use the shortened alias "rec"
typedef car Rectangle_R32 Rectangle_R32;
car Rectangle_R32
{
	r32 elem[4];
	plex { r32 x, y, width, height; };
	plex { r32 left, top, _unused1, _unused2; };
	plex { v2 topLeft, size; };
};
#define MakeRec_Const(xValue, yValue, widthValue, heightValue) { .x=(xValue), .y=(yValue), .width=(widthValue), .height=(heightValue) }
#define MakeRecV_Const(topLeftValue, sizeValue)                { .topLeft=topLeftValue,  .size=sizeValue }
#define MakeRec(x, y, width, height)                           NEW_STRUCT(Rectangle_R32)MakeRec_Const((x), (y), (width), (height))
#define MakeRecV(topLeft, size)                                NEW_STRUCT(Rectangle_R32)MakeRecV_Const((topLeft), (size))

typedef car Rectangle_R64 Rectangle_R64;
car Rectangle_R64
{
	r64 elem[4];
	plex { r64 x, y, width, height; };
	plex { r64 lon, lat, sizeLon, sizeLat; };
	plex { r64 longitude, latitude, sizeLongitude, sizeLatitude; };
	plex { r64 left, top, _unused1, _unused2; };
	plex { v2d topLeft, size; };
};
#define MakeRecd_Const(xValue, yValue, widthValue, heightValue)  { .x=(xValue), .y=(yValue), .width=(widthValue), .height=(heightValue) }
#define MakeRecdV_Const(topLeftValue, sizeValue)                 { .topLeft=topLeftValue, .size=sizeValue }
#define MakeRecd(x, y, width, height)                            NEW_STRUCT(Rectangle_R64)MakeRecd_Const((x), (y), (width), (height))
#define MakeRecdV(topLeft, size)                                 NEW_STRUCT(Rectangle_R64)MakeRecdV_Const((topLeft), (size))

typedef car Rectangle_I32 Rectangle_I32;
car Rectangle_I32
{
	i32 elem[4];
	plex { i32 x, y, width, height; };
	plex { i32 left, top, _unused1, _unused2; };
	plex { v2i topLeft, size; };
};
#define MakeReci_Const(xValue, yValue, widthValue, heightValue)  { .x=(xValue), .y=(yValue), .width=(widthValue), .height=(heightValue) }
#define MakeReciV_Const(topLeftValue, sizeValue)                 { .topLeft=topLeftValue, .size=sizeValue }
#define MakeReci(x, y, width, height)                            NEW_STRUCT(Rectangle_I32)MakeReci_Const((x), (y), (width), (height))
#define MakeReciV(topLeft, size)                                 NEW_STRUCT(Rectangle_I32)MakeReciV_Const((topLeft), (size))

typedef car Box_R32 Box_R32;
car Box_R32
{
	r32 elem[6];
	plex { r32 x, y, z, width, height, depth; };
	plex { r32 left, bottom, back, _unused1, _unused2, _unused3; };
	plex { v3 bottomLeftBack, size; };
};
#define MakeBox_Const(xValue, yValue, zValue, widthValue, heightValue, depthValue) { .x=(xValue), .y=(yValue), .z=(zValue), .width=(widthValue), .height=(heightValue), .depth=(depthValue) }
#define MakeBoxV_Const(bottomLeftBackValue, sizeValue)                             { .bottomLeftBack=bottomLeftBackValue,  .size=sizeValue }
#define MakeBox(x, y, z, width, height, depth)                                     NEW_STRUCT(Box_R32)MakeBox_Const((x), (y), (z), (width), (height), (depth))
#define MakeBoxV(bottomLeftBack, size)                                             NEW_STRUCT(Box_R32)MakeBoxV_Const((bottomLeftBack), (size))

typedef car Box_R64 Box_R64;
car Box_R64
{
	r64 elem[6];
	plex { r64 x, y, z, width, height, depth; };
	plex { r64 left, bottom, back, _unused1, _unused2, _unused3; };
	plex { v3d bottomLeftBack, size; };
};
#define MakeBoxd_Const(xValue, yValue, zValue, widthValue, heightValue, depthValue) { .x=(xValue), .y=(yValue), .z=(zValue), .width=(widthValue), .height=(heightValue), .depth=(depthValue) }
#define MakeBoxdV_Const(bottomLeftBackValue, sizeValue)                             { .bottomLeftBack=bottomLeftBackValue, .size=sizeValue }
#define MakeBoxd(x, y, z, width, height, depth)                                     NEW_STRUCT(Box_R64)MakeBoxd_Const((x), (y), (z), (width), (height), (depth))
#define MakeBoxdV(bottomLeftBack, size)                                             NEW_STRUCT(Box_R64)MakeBoxdV_Const((bottomLeftBack), (size))

typedef car Box_I32 Box_I32;
car Box_I32
{
	i32 elem[6];
	plex { i32 x, y, z, width, height, depth; };
	plex { i32 left, bottom, back, _unused1, _unused2, _unused3; };
	plex { v3i bottomleftback, size; };
};
#define MakeBoxi_Const(xValue, yValue, zValue, widthValue, heightValue, depthValue) { .x=(xValue), .y=(yValue), .z=(zValue), .width=(widthValue), .height=(heightValue), .depth=(depthValue) }
#define MakeBoxiV_Const(bottomLeftBackValue, sizeValue)                             { .bottomLeftBack=bottomLeftBackValue, .size=sizeValue }
#define MakeBoxi(x, y, z, width, height, depth)                                     NEW_STRUCT(Box_I32)MakeBoxi_Const((x), (y), (z), (width), (height), (depth))
#define MakeBoxiV(bottomLeftBack, size)                                             NEW_STRUCT(Box_I32)MakeBoxiV_Const((bottomLeftBack), (size))

typedef car Obb2D_R32 Obb2D_R32;
car Obb2D_R32
{
	r32 elem[5];
	plex { r32 x, y, width, height, rotation; };
	plex { r32 centerX, centerY, _unused1, _unused2, angle; };
	plex { v2 center, size; r32 _unused3; };
};
#define MakeObb2_Const(xValue, yValue, widthValue, heightValue, rotationValue) { .x=(xValue), .y=(yValue), .width=(widthValue), .height=(heightValue), .rotation=(rotationValue) }
#define MakeObb2V_Const(centerValue, sizeValue, rotationValue)                 { .center=centerValue,  .size=sizeValue, ._unused3=(rotationValue) }
#define MakeObb2(x, y, width, height, rotation)                                NEW_STRUCT(Obb2D_R32)MakeObb2_Const((x), (y), (width), (height), (rotation))
#define MakeObb2V(center, size, rotation)                                      NEW_STRUCT(Obb2D_R32)MakeObb2V_Const((center), (size), (rotation))

typedef car Obb2D_R64 Obb2D_R64;
car Obb2D_R64
{
	r64 elem[5];
	plex { r64 x, y, width, height, rotation; };
	plex { r64 centerX, centerY, _unused1, _unused2, angle; };
	plex { v2d center, size; r64 _unused3; };
};
#define MakeObb2d_Const(xValue, yValue, widthValue, heightValue, rotationValue) { .x=(xValue), .y=(yValue), .width=(widthValue), .height=(heightValue), .rotation=(rotationValue) }
#define MakeObb2dV_Const(centerValue, sizeValue, rotationValue)                 { .center=centerValue, .size=sizeValue, ._unused3=(rotationValue) }
#define MakeObb2d(x, y, width, height, rotation)                                NEW_STRUCT(Obb2D_R64)MakeObb2d_Const((x), (y), (width), (height), (rotation))
#define MakeObb2dV(center, size, rotation)                                      NEW_STRUCT(Obb2D_R64)MakeObb2dV_Const((center), (size), (rotation))

//NOTE: quat is HMM_Vec4 which has an SSE intrinsic that forces it to be 16-byte aligned
//      thus we need a bit of padding between center/size and rotation
typedef car Obb3D_R32 Obb3D_R32;
car Obb3D_R32
{
	r32 elem[12]; //indices 8 and 9 are padding
	plex { r32 x, y, z, width, height, depth, _padding1, _padding2; quat rotation; };
	plex { r32 centerX, centerY, centerZ, _unused1, _unused2, _unused3, _padding3, _padding4; quat _unused4; };
	plex { v3 center, size; r32 _padding5, _padding6; quat _unused5; };
};
#if STATIC_ASSERT_AVAILABLE
_Static_assert(sizeof(Obb3D_R32) == sizeof(r32) * 12, "Obb3D_R32 contains padding!");
#endif
#define MakeObb3_Const(xValue, yValue, zValue, widthValue, heightValue, depthValue, rotationValue) { .x=(xValue), .y=(yValue), .z=(zValue), .width=(widthValue), .height=(heightValue), .depth=(depthValue), ._padding1=0, ._padding2=0, .rotation=rotationValue }
#define MakeObb3V_Const(centerValue, sizeValue, rotationValue)                                     { .center=centerValue,  .size=sizeValue, ._padding5=0.0f, ._padding6=0.0f, ._unused5=rotationValue }
#define MakeObb3(x, y, z, width, height, depth, rotation)                                          NEW_STRUCT(Obb3D_R32)MakeObb3_Const((x), (y), (z), (width), (height), (depth), (rotation))
#define MakeObb3V(center, size, rotation)                                                          NEW_STRUCT(Obb3D_R32)MakeObb3V_Const((center), (size), (rotation))

typedef car Obb3D_R64 Obb3D_R64;
car Obb3D_R64
{
	r64 elem[10];
	plex { r64 x, y, z, width, height, depth; quatd rotation; };
	plex { r64 centerX, centerY, centerZ, _unused1, _unused2, _unused3; quatd _unused4; };
	plex { v3d center, size; quatd _unused5; };
};
#if STATIC_ASSERT_AVAILABLE
_Static_assert(sizeof(Obb3D_R64) == sizeof(r64) * 10, "Obb3D_R64 contains padding!");
#endif
#define MakeObb3d_Const(xValue, yValue, zValue, widthValue, heightValue, depthValue, rotationValue) { .x=(xValue), .y=(yValue), .z=(zValue), .width=(widthValue), .height=(heightValue), .depth=(depthValue), .rotation=rotationValue }
#define MakeObb3dV_Const(centerValue, sizeValue, rotationValue)                                     { .center=centerValue, .size=sizeValue, ._unused5=rotationValue }
#define MakeObb3d(x, y, z, width, height, depth, rotation)                                          NEW_STRUCT(Obb3D_R64)MakeObb3d_Const((x), (y), (z), (width), (height), (depth), (rotation))
#define MakeObb3dV(center, size, rotation)                                                          NEW_STRUCT(Obb3D_R64)MakeObb3dV_Const((center), (size), (rotation))

//NOTE: Naming Convention: The default assumption for rectangles is r32 members
//    'i' suffix refers to "integer" members and implicitly means 32-bit (unsigned and 64-bit integers are not yet supported)
//    'd' suffix refers to "double" members which are 64-bit (we went with a 'd' suffix instead of something like 'r64' because a single character suffix reads a lot nicer and "double" is a well known word to refer to 64-bit floating point numbers)
//TODO: The d suffix to denote "double" members conflicts with obb true names where 2D and 3D denote "dimensions" rather than "double". This doesn't cause any problems since most of the functions omit the D in 2D and 3D but it is confusing
typedef Rectangle_R32 rec;
typedef Rectangle_I32 reci;
typedef Rectangle_R64 recd;
typedef Box_R32 box;
typedef Box_I32 boxi;
typedef Box_R64 boxd;
typedef Obb2D_R32 obb2;
typedef Obb2D_R64 obb2d;
typedef Obb3D_R32 obb3;
typedef Obb3D_R64 obb3d;

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE rec NewRecCentered(r32 centerX, r32 centerY, r32 width, r32 height);
	PIG_CORE_INLINE rec NewRecCenteredV(v2 center, v2 size);
	PIG_CORE_INLINE rec NewRecBetween(r32 left, r32 top, r32 right, r32 bottom);
	PIG_CORE_INLINE rec NewRecBetweenV(v2 topLeft, v2 bottomRight);
	PIG_CORE_INLINE reci NewReciCentered(i32 centerX, i32 centerY, i32 width, i32 height);
	PIG_CORE_INLINE reci NewReciCenteredV(v2i center, v2i size);
	PIG_CORE_INLINE reci NewReciBetween(i32 left, i32 top, i32 right, i32 bottom);
	PIG_CORE_INLINE reci NewReciBetweenV(v2i topLeft, v2i bottomRight);
	PIG_CORE_INLINE recd NewRecdCentered(r64 centerX, r64 centerY, r64 width, r64 height);
	PIG_CORE_INLINE recd NewRecdCenteredV(v2d center, v2d size);
	PIG_CORE_INLINE recd NewRecdBetween(r64 left, r64 top, r64 right, r64 bottom);
	PIG_CORE_INLINE recd NewRecdBetweenV(v2d topLeft, v2d bottomRight);
	PIG_CORE_INLINE recd NewRecdCentered(r64 centerX, r64 centerY, r64 width, r64 height);
	PIG_CORE_INLINE recd NewRecdCenteredV(v2d center, v2d size);
	PIG_CORE_INLINE recd NewRecdBetween(r64 left, r64 top, r64 right, r64 bottom);
	PIG_CORE_INLINE recd NewRecdBetweenV(v2d topLeft, v2d bottomRight);
	PIG_CORE_INLINE box NewBoxCentered(r32 centerX, r32 centerY, r32 centerZ, r32 width, r32 height, r32 depth);
	PIG_CORE_INLINE box NewBoxCenteredV(v3 center, v3 size);
	PIG_CORE_INLINE box NewBoxBetween(r32 left, r32 bottom, r32 back, r32 right, r32 top, r32 front);
	PIG_CORE_INLINE box NewBoxBetweenV(v3 bottomLeftBack, v3 topRightFront);
	PIG_CORE_INLINE boxi NewBoxiCentered(i32 centerX, i32 centerY, i32 centerZ, i32 width, i32 height, i32 depth);
	PIG_CORE_INLINE boxi NewBoxiCenteredV(v3i center, v3i size);
	PIG_CORE_INLINE boxi NewBoxiBetween(i32 left, i32 bottom, i32 back, i32 right, i32 top, i32 front);
	PIG_CORE_INLINE boxi NewBoxiBetweenV(v3i bottomLeftBack, v3i topRightFront);
	PIG_CORE_INLINE boxd NewBoxdCentered(r64 centerX, r64 centerY, r64 centerZ, r64 width, r64 height, r64 depth);
	PIG_CORE_INLINE boxd NewBoxdCenteredV(v3d center, v3d size);
	PIG_CORE_INLINE boxd NewBoxdBetween(r64 left, r64 bottom, r64 back, r64 right, r64 top, r64 front);
	PIG_CORE_INLINE boxd NewBoxdBetweenV(v3d bottomLeftBack, v3d topRightFront);
	PIG_CORE_INLINE rec ToRecFromi(reci rectangle);
	PIG_CORE_INLINE recd ToRecdFromi(reci rectangle);
	PIG_CORE_INLINE recd ToRecdFromf(rec rectangle);
	PIG_CORE_INLINE rec ToRecFromd(recd rectangle);
	PIG_CORE_INLINE reci ToReciFromf(rec rectangle);
	PIG_CORE_INLINE rec ToRecFromV4(v4 vector);
	PIG_CORE_INLINE v4 ToV4FromRec(rec rectangle);
	PIG_CORE_INLINE v4r ToV4rFromRec(rec rectangle);
	PIG_CORE_INLINE v4d ToV4dFromRecd(recd rectangle);
	PIG_CORE_INLINE box ToBoxFromi(boxi boundingBox);
	PIG_CORE_INLINE boxd ToBoxdFromi(boxi boundingBox);
	PIG_CORE_INLINE boxd ToBoxdFromf(box boundingBox);
	PIG_CORE_INLINE box ToBoxFromd(boxd boundingBox);
	PIG_CORE_INLINE boxi ToBoxiFromf(box boundingBox);
	PIG_CORE_INLINE obb2 ToObb2FromRec(rec rectangle);
	PIG_CORE_INLINE obb2d ToObb2dFromRecd(recd rectangle);
	PIG_CORE_INLINE obb3 ToObb3FromBox(box boundingBox);
	PIG_CORE_INLINE obb3d ToObb3dFromBoxd(boxd boundingBox);
	#if TARGET_IS_ORCA
	PIG_CORE_INLINE oc_rect ToOcRect(rec rectangle);
	PIG_CORE_INLINE rec ToRecFromOc(oc_rect orcaRectangle);
	#endif //TARGET_IS_ORCA
	PIG_CORE_INLINE rec AddRec(rec rectangle, v2 offset);
	PIG_CORE_INLINE reci AddReci(reci rectangle, v2i offset);
	PIG_CORE_INLINE recd AddRecd(recd rectangle, v2d offset);
	PIG_CORE_INLINE box AddBox(box boundingBox, v3 offset);
	PIG_CORE_INLINE boxi AddBoxi(boxi boundingBox, v3i offset);
	PIG_CORE_INLINE boxd AddBoxd(boxd boundingBox, v3d offset);
	PIG_CORE_INLINE obb2 AddObb2(obb2 boundingBox, v2 offset);
	PIG_CORE_INLINE obb2d AddObb2d(obb2d boundingBox, v2d offset);
	PIG_CORE_INLINE obb3 AddObb3(obb3 boundingBox, v3 offset);
	PIG_CORE_INLINE obb3d AddObb3d(obb3d boundingBox, v3d offset);
	PIG_CORE_INLINE rec SubRec(rec rectangle, v2 offset);
	PIG_CORE_INLINE reci SubReci(reci rectangle, v2i offset);
	PIG_CORE_INLINE recd SubRecd(recd rectangle, v2d offset);
	PIG_CORE_INLINE box SubBox(box boundingBox, v3 offset);
	PIG_CORE_INLINE boxi SubBoxi(boxi boundingBox, v3i offset);
	PIG_CORE_INLINE boxd SubBoxd(boxd boundingBox, v3d offset);
	PIG_CORE_INLINE obb2 SubObb2(obb2 boundingBox, v2 offset);
	PIG_CORE_INLINE obb2d SubObb2d(obb2d boundingBox, v2d offset);
	PIG_CORE_INLINE obb3 SubObb3(obb3 boundingBox, v3 offset);
	PIG_CORE_INLINE obb3d SubObb3d(obb3d boundingBox, v3d offset);
	PIG_CORE_INLINE rec MulRec(rec rectangle, v2 scale);
	PIG_CORE_INLINE reci MulReci(reci rectangle, v2i scale);
	PIG_CORE_INLINE recd MulRecd(recd rectangle, v2d scale);
	PIG_CORE_INLINE box MulBox(box boundingBox, v3 scale);
	PIG_CORE_INLINE boxi MulBoxi(boxi boundingBox, v3i scale);
	PIG_CORE_INLINE boxd MulBoxd(boxd boundingBox, v3d scale);
	PIG_CORE_INLINE obb2 MulObb2(obb2 boundingBox, v2 scale);
	PIG_CORE_INLINE obb2d MulObb2d(obb2d boundingBox, v2d scale);
	PIG_CORE_INLINE obb3 MulObb3(obb3 boundingBox, v3 scale);
	PIG_CORE_INLINE obb3d MulObb3d(obb3d boundingBox, v3d scale);
	PIG_CORE_INLINE rec DivRec(rec rectangle, v2 divisor);
	PIG_CORE_INLINE reci DivReci(reci rectangle, v2i divisor);
	PIG_CORE_INLINE recd DivRecd(recd rectangle, v2d divisor);
	PIG_CORE_INLINE box DivBox(box boundingBox, v3 divisor);
	PIG_CORE_INLINE boxi DivBoxi(boxi boundingBox, v3i divisor);
	PIG_CORE_INLINE boxd DivBoxd(boxd boundingBox, v3d divisor);
	PIG_CORE_INLINE obb2 DivObb2(obb2 boundingBox, v2 divisor);
	PIG_CORE_INLINE obb2d DivObb2d(obb2d boundingBox, v2d divisor);
	PIG_CORE_INLINE obb3 DivObb3(obb3 boundingBox, v3 divisor);
	PIG_CORE_INLINE obb3d DivObb3d(obb3d boundingBox, v3d divisor);
	PIG_CORE_INLINE rec ScaleRec(rec rectangle, r32 scalar);
	PIG_CORE_INLINE reci ScaleReci(reci rectangle, i32 scalar);
	PIG_CORE_INLINE recd ScaleRecd(recd rectangle, r64 scalar);
	PIG_CORE_INLINE box ScaleBox(box boundingBox, r32 scalar);
	PIG_CORE_INLINE boxi ScaleBoxi(boxi boundingBox, i32 scalar);
	PIG_CORE_INLINE boxd ScaleBoxd(boxd boundingBox, r64 scalar);
	PIG_CORE_INLINE obb2 ScaleObb2(obb2 boundingBox, r32 scalar);
	PIG_CORE_INLINE obb2d ScaleObb2d(obb2d boundingBox, r64 scalar);
	PIG_CORE_INLINE obb3 ScaleObb3(obb3 boundingBox, r32 scalar);
	PIG_CORE_INLINE obb3d ScaleObb3d(obb3d boundingBox, r64 scalar);
	PIG_CORE_INLINE rec ShrinkRec(rec rectangle, r32 divisor);
	PIG_CORE_INLINE reci ShrinkReci(reci rectangle, i32 divisor);
	PIG_CORE_INLINE recd ShrinkRecd(recd rectangle, r64 divisor);
	PIG_CORE_INLINE box ShrinkBox(box boundingBox, r32 divisor);
	PIG_CORE_INLINE boxi ShrinkBoxi(boxi boundingBox, i32 divisor);
	PIG_CORE_INLINE boxd ShrinkBoxd(boxd boundingBox, r64 divisor);
	PIG_CORE_INLINE obb2 ShrinkObb2(obb2 boundingBox, r32 divisor);
	PIG_CORE_INLINE obb2d ShrinkObb2d(obb2d boundingBox, r64 divisor);
	PIG_CORE_INLINE obb3 ShrinkObb3(obb3 boundingBox, r32 divisor);
	PIG_CORE_INLINE obb3d ShrinkObb3d(obb3d boundingBox, r64 divisor);
	PIG_CORE_INLINE bool AreEqualRec(rec left, rec right);
	PIG_CORE_INLINE bool AreEqualReci(reci left, reci right);
	PIG_CORE_INLINE bool AreEqualRecd(recd left, recd right);
	PIG_CORE_INLINE bool AreEqualBox(box left, box right);
	PIG_CORE_INLINE bool AreEqualBoxi(boxi left, boxi right);
	PIG_CORE_INLINE bool AreEqualBoxd(boxd left, boxd right);
	PIG_CORE_INLINE bool AreEqualObb2(obb2 left, obb2 right);
	PIG_CORE_INLINE bool AreEqualObb2d(obb2d left, obb2d right);
	PIG_CORE_INLINE bool AreEqualObb3(obb3 left, obb3 right);
	PIG_CORE_INLINE bool AreEqualObb3d(obb3d left, obb3d right);
	PIG_CORE_INLINE bool DoesOverlapRec(rec rectangle1, rec rectangle2, bool inclusive);
	PIG_CORE_INLINE bool DoesOverlapReci(reci rectangle1, reci rectangle2, bool inclusive);
	PIG_CORE_INLINE bool DoesOverlapRecd(recd rectangle1, recd rectangle2, bool inclusive);
	PIG_CORE_INLINE bool DoesOverlapBox(box rectangle1, box rectangle2, bool inclusive);
	PIG_CORE_INLINE bool DoesOverlapBoxi(boxi rectangle1, boxi rectangle2, bool inclusive);
	PIG_CORE_INLINE bool DoesOverlapBoxd(boxd rectangle1, boxd rectangle2, bool inclusive);
	PIG_CORE_INLINE rec InflateRecEx(rec rectangle, r32 leftAmount, r32 rightAmount, r32 topAmount, r32 bottomAmount);
	PIG_CORE_INLINE rec InflateRec(rec rectangle, r32 amount);
	PIG_CORE_INLINE rec InflateRecX(rec rectangle, r32 amount);
	PIG_CORE_INLINE rec InflateRecY(rec rectangle, r32 amount);
	PIG_CORE_INLINE reci InflateReciEx(reci rectangle, i32 leftAmount, i32 rightAmount, i32 topAmount, i32 bottomAmount);
	PIG_CORE_INLINE reci InflateReci(reci rectangle, i32 amount);
	PIG_CORE_INLINE reci InflateReciX(reci rectangle, i32 amount);
	PIG_CORE_INLINE reci InflateReciY(reci rectangle, i32 amount);
	PIG_CORE_INLINE recd InflateRecdEx(recd rectangle, r64 leftAmount, r64 rightAmount, r64 topAmount, r64 bottomAmount);
	PIG_CORE_INLINE recd InflateRecd(recd rectangle, r64 amount);
	PIG_CORE_INLINE recd InflateRecdX(recd rectangle, r64 amount);
	PIG_CORE_INLINE recd InflateRecdY(recd rectangle, r64 amount);
	PIG_CORE_INLINE box InflateBoxEx(box boundingBox, r32 leftAmount, r32 rightAmount, r32 topAmount, r32 bottomAmount, r32 backAmount, r32 frontAmount);
	PIG_CORE_INLINE box InflateBox(box boundingBox, r32 amount);
	PIG_CORE_INLINE box InflateBoxX(box boundingBox, r32 amount);
	PIG_CORE_INLINE box InflateBoxY(box boundingBox, r32 amount);
	PIG_CORE_INLINE box InflateBoxZ(box boundingBox, r32 amount);
	PIG_CORE_INLINE boxi InflateBoxiEx(boxi boundingBox, i32 leftAmount, i32 rightAmount, i32 topAmount, i32 bottomAmount, i32 backAmount, i32 frontAmount);
	PIG_CORE_INLINE boxi InflateBoxi(boxi boundingBox, i32 amount);
	PIG_CORE_INLINE boxi InflateBoxiX(boxi boundingBox, i32 amount);
	PIG_CORE_INLINE boxi InflateBoxiY(boxi boundingBox, i32 amount);
	PIG_CORE_INLINE boxi InflateBoxiZ(boxi boundingBox, i32 amount);
	PIG_CORE_INLINE boxd InflateBoxdEx(boxd boundingBox, r64 leftAmount, r64 rightAmount, r64 topAmount, r64 bottomAmount, r64 backAmount, r64 frontAmount);
	PIG_CORE_INLINE boxd InflateBoxd(boxd boundingBox, r64 amount);
	PIG_CORE_INLINE boxd InflateBoxdX(boxd boundingBox, r64 amount);
	PIG_CORE_INLINE boxd InflateBoxdY(boxd boundingBox, r64 amount);
	PIG_CORE_INLINE boxd InflateBoxdZ(boxd boundingBox, r64 amount);
	PIG_CORE_INLINE rec BothRec(rec left, rec right);
	PIG_CORE_INLINE recd BothRecd(recd left, recd right);
	PIG_CORE_INLINE reci BothReci(reci left, reci right);
	PIG_CORE_INLINE box BothBox(box left, box right);
	PIG_CORE_INLINE boxi BothBoxi(boxi left, boxi right);
	PIG_CORE_INLINE boxd BothBoxd(boxd left, boxd right);
	PIG_CORE_INLINE rec OverlapPartRec(rec left, rec right);
	PIG_CORE_INLINE reci OverlapPartReci(reci left, reci right);
	PIG_CORE_INLINE recd OverlapPartRecd(recd left, recd right);
	PIG_CORE_INLINE box OverlapPartBox(box left, box right);
	PIG_CORE_INLINE boxi OverlapPartBoxi(boxi left, boxi right);
	PIG_CORE_INLINE boxd OverlapPartBoxd(boxd left, boxd right);
	PIG_CORE_INLINE bool IsInsideRec(rec rectangle, v2 position);
	PIG_CORE_INLINE bool IsInsideRecd(recd rectangle, v2d position);
	PIG_CORE_INLINE bool IsInsideRecInclusive(rec rectangle, v2 position);
	PIG_CORE_INLINE bool IsInsideRecdInclusive(rec rectangle, v2d position);
	PIG_CORE_INLINE void AlignRecToV2(rec* rectangleOut, v2 alignmentScale);
	PIG_CORE_INLINE void AlignRecTo(rec* rectangleOut, r32 alignmentScale);
	PIG_CORE_INLINE void AlignRec(rec* rectangleOut);
	PIG_CORE_INLINE void AlignRecdToV2d(recd* rectangleOut, v2d alignmentScale);
	PIG_CORE_INLINE void AlignRecdTo(recd* rectangleOut, r64 alignmentScale);
	PIG_CORE_INLINE void AlignRecd(recd* rectangleOut);
	PIG_CORE_INLINE void AlignBoxToV3(box* boundingBoxOut, v3 alignmentScale);
	PIG_CORE_INLINE void AlignBoxTo(box* boundingBoxOut, r32 alignmentScale);
	PIG_CORE_INLINE void AlignBox(box* boundingBoxOut);
	PIG_CORE_INLINE void AlignBoxdToV3d(boxd* boundingBoxOut, v3d alignmentScale);
	PIG_CORE_INLINE void AlignBoxdTo(boxd* boundingBoxOut, r64 alignmentScale);
	PIG_CORE_INLINE void AlignBoxd(boxd* boundingBoxOut);
	PIG_CORE_INLINE rec RelativeRec(rec reference, rec subReference, rec other);
	PIG_CORE_INLINE recd RelativeRecd(recd reference, recd subReference, recd other);
	PIG_CORE_INLINE box RelativeBox(box reference, box subReference, box other);
	PIG_CORE_INLINE boxd RelativeBoxd(boxd reference, boxd subReference, boxd other);
#endif

// +--------------------------------------------------------------+
// |                   Simple Value Definitions                   |
// +--------------------------------------------------------------+
#define Rec_Zero_Const   MakeRec_Const(0.0f, 0.0f, 0.0f, 0.0f)
#define Reci_Zero_Const  MakeReci_Const(0, 0, 0, 0)
#define Recd_Zero_Const  MakeRecd_Const(0.0, 0.0, 0.0, 0.0)
#define Box_Zero_Const   MakeBox_Const(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)
#define Boxi_Zero_Const  MakeBoxi_Const(0, 0, 0, 0, 0, 0)
#define Boxd_Zero_Const  MakeBoxd_Const(0.0, 0.0, 0.0, 0.0, 0.0, 0.0)
#define Obb2_Zero_Const  MakeObb2_Const(0.0f, 0.0f, 0.0f, 0.0f, 0.0f)
#define Obb2d_Zero_Const MakeObb2d_Const(0.0, 0.0, 0.0, 0.0, 0.0)
// #define Obb3_Zero_Const  MakeObb3_Const(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, Quat_Identity)
// #define Obb3d_Zero_Const MakeObb3d_Const(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, Quatd_Identity)

#define Rec_Zero     MakeRec( 0.0f,  0.0f, 0.0f, 0.0f)
#define Rec_Default  MakeRec( 0.0f,  0.0f, 1.0f, 1.0f)
#define Rec_UnitRec  MakeRec(-1.0f, -1.0f, 2.0f, 2.0f)

#define Reci_Zero     MakeReci( 0,  0, 0, 0)
#define Reci_Default  MakeReci( 0,  0, 1, 1)
#define Reci_UnitRec  MakeReci(-1, -1, 2, 2)

#define Recd_Zero     MakeRecd( 0.0,  0.0, 0.0, 0.0)
#define Recd_Default  MakeRecd( 0.0,  0.0, 1.0, 1.0)
#define Recd_UnitRec  MakeRecd(-1.0, -1.0, 2.0, 2.0)

#define Box_Zero     MakeBox( 0.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f)
#define Box_Default  MakeBox( 0.0f,  0.0f,  0.0f, 1.0f, 1.0f, 1.0f)
#define Box_UnitBox  MakeBox(-1.0f, -1.0f, -1.0f, 2.0f, 2.0f, 2.0f)

#define Boxi_Zero     MakeBoxi( 0,  0,  0, 0, 0, 0)
#define Boxi_Default  MakeBoxi( 0,  0,  0, 1, 1, 1)
#define Boxi_UnitBox  MakeBoxi(-1, -1, -1, 2, 2, 2)

#define Boxd_Zero     MakeBoxd( 0.0,  0.0,  0.0, 0.0, 0.0, 0.0)
#define Boxd_Default  MakeBoxd( 0.0,  0.0,  0.0, 1.0, 1.0, 1.0)
#define Boxd_UnitBox  MakeBoxd(-1.0, -1.0, -1.0, 2.0, 2.0, 2.0)

#define Obb2_Zero     MakeObb2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f)
#define Obb2_Default  MakeObb2(0.5f, 0.5f, 1.0f, 1.0f, 0.0f)
#define Obb2_UnitBox  MakeObb2(0.0f, 0.0f, 2.0f, 2.0f, 0.0f)

#define Obb2d_Zero     MakeObb2d(0.0, 0.0, 0.0, 0.0, 0.0)
#define Obb2d_Default  MakeObb2d(0.5, 0.5, 1.0, 1.0, 0.0)
#define Obb2d_UnitBox  MakeObb2d(0.0, 0.0, 2.0, 2.0, 0.0)

#define Obb3_Zero     MakeObb3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, Quat_Identity)
#define Obb3_Default  MakeObb3(0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, Quat_Identity)
#define Obb3_UnitBox  MakeObb3(0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 2.0f, Quat_Identity)

#define Obb3d_Zero     MakeObb3d(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, Quatd_Identity)
#define Obb3d_Default  MakeObb3d(0.5, 0.5, 0.5, 1.0, 1.0, 1.0, Quatd_Identity)
#define Obb3d_UnitBox  MakeObb3d(0.0, 0.0, 0.0, 2.0, 2.0, 2.0, Quatd_Identity)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI rec NewRecCentered(r32 centerX, r32 centerY, r32 width, r32 height)
{
	rec result;
	result.x = centerX - width/2.0f;
	result.y = centerY - height/2.0f;
	result.width = width;
	result.height = height;
	return result;
}
PEXPI rec NewRecCenteredV(v2 center, v2 size)
{
	rec result;
	result.topLeft = SubV2(center, ShrinkV2(size, 2.0f));
	result.size = size;
	return result;
}
PEXPI rec NewRecBetween(r32 left, r32 top, r32 right, r32 bottom)
{
	rec result;
	result.x = MinR32(left, right);
	result.y = MinR32(top, bottom);
	result.width = MaxR32(left, right) - result.x;
	result.height = MaxR32(top, bottom) - result.y;
	return result;
}
PEXPI rec NewRecBetweenV(v2 topLeft, v2 bottomRight)
{
	rec result;
	result.x = MinR32(topLeft.x, bottomRight.x);
	result.y = MinR32(topLeft.y, bottomRight.y);
	result.width = MaxR32(topLeft.x, bottomRight.x) - result.x;
	result.height = MaxR32(topLeft.y, bottomRight.y) - result.y;
	return result;
}

//NOTE: This only works as expected if width/height are even numbers, since we can't center a uneven size rectangle around a integer coordinate
PEXPI reci NewReciCentered(i32 centerX, i32 centerY, i32 width, i32 height)
{
	reci result;
	result.x = centerX - width/2;
	result.y = centerY - height/2;
	result.width = width;
	result.height = height;
	return result;
}
PEXPI reci NewReciCenteredV(v2i center, v2i size)
{
	reci result;
	result.topLeft = SubV2i(center, ShrinkV2i(size, 2));
	result.size = size;
	return result;
}
PEXPI reci NewReciBetween(i32 left, i32 top, i32 right, i32 bottom)
{
	reci result;
	result.x = MinI32(left, right);
	result.y = MinI32(top, bottom);
	result.width = MaxI32(left, right) - result.x;
	result.height = MaxI32(top, bottom) - result.y;
	return result;
}
PEXPI reci NewReciBetweenV(v2i topLeft, v2i bottomRight)
{
	reci result;
	result.x = MinI32(topLeft.x, bottomRight.x);
	result.y = MinI32(topLeft.y, bottomRight.y);
	result.width = MaxI32(topLeft.x, bottomRight.x) - result.x;
	result.height = MaxI32(topLeft.y, bottomRight.y) - result.y;
	return result;
}

PEXPI recd NewRecdCentered(r64 centerX, r64 centerY, r64 width, r64 height)
{
	recd result;
	result.x = centerX - width/2.0;
	result.y = centerY - height/2.0;
	result.width = width;
	result.height = height;
	return result;
}
PEXPI recd NewRecdCenteredV(v2d center, v2d size)
{
	recd result;
	result.topLeft = SubV2d(center, ShrinkV2d(size, 2.0));
	result.size = size;
	return result;
}
PEXPI recd NewRecdBetween(r64 left, r64 top, r64 right, r64 bottom)
{
	recd result;
	result.x = MinR64(left, right);
	result.y = MinR64(top, bottom);
	result.width = MaxR64(left, right) - result.x;
	result.height = MaxR64(top, bottom) - result.y;
	return result;
}
PEXPI recd NewRecdBetweenV(v2d topLeft, v2d bottomRight)
{
	recd result;
	result.x = MinR64(topLeft.x, bottomRight.x);
	result.y = MinR64(topLeft.y, bottomRight.y);
	result.width = MaxR64(topLeft.x, bottomRight.x) - result.x;
	result.height = MaxR64(topLeft.y, bottomRight.y) - result.y;
	return result;
}

PEXPI box NewBoxCentered(r32 centerX, r32 centerY, r32 centerZ, r32 width, r32 height, r32 depth)
{
	box result;
	result.x = centerX - width/2.0f;
	result.y = centerY - height/2.0f;
	result.z = centerZ - depth/2.0f;
	result.width = width;
	result.height = height;
	result.depth = depth;
	return result;
}
PEXPI box NewBoxCenteredV(v3 center, v3 size)
{
	box result;
	result.bottomLeftBack = SubV3(center, ShrinkV3(size, 2.0f));
	result.size = size;
	return result;
}
PEXPI box NewBoxBetween(r32 left, r32 bottom, r32 back, r32 right, r32 top, r32 front)
{
	box result;
	result.x = MinR32(left, right);
	result.y = MinR32(bottom, top);
	result.z = MinR32(back, front);
	result.width = MaxR32(left, right) - result.x;
	result.height = MaxR32(bottom, top) - result.y;
	result.depth = MaxR32(back, front) - result.z;
	return result;
}
PEXPI box NewBoxBetweenV(v3 bottomLeftBack, v3 topRightFront)
{
	box result;
	result.x = MinR32(bottomLeftBack.x, topRightFront.x);
	result.y = MinR32(bottomLeftBack.y, topRightFront.y);
	result.z = MinR32(bottomLeftBack.z, topRightFront.z);
	result.width = MaxR32(bottomLeftBack.x, topRightFront.x) - result.x;
	result.height = MaxR32(bottomLeftBack.y, topRightFront.y) - result.y;
	result.depth = MaxR32(bottomLeftBack.z, topRightFront.z) - result.z;
	return result;
}

//NOTE: This only works as expected if width/height/depth are even numbers, since we can't center a uneven size box around a integer coordinate
PEXPI boxi NewBoxiCentered(i32 centerX, i32 centerY, i32 centerZ, i32 width, i32 height, i32 depth)
{
	boxi result;
	result.x = centerX - width/2;
	result.y = centerY - height/2;
	result.z = centerZ - depth/2;
	result.width = width;
	result.height = height;
	result.depth = depth;
	return result;
}
PEXPI boxi NewBoxiCenteredV(v3i center, v3i size)
{
	boxi result;
	result.bottomLeftBack = SubV3i(center, ShrinkV3i(size, 2));
	result.size = size;
	return result;
}
PEXPI boxi NewBoxiBetween(i32 left, i32 bottom, i32 back, i32 right, i32 top, i32 front)
{
	boxi result;
	result.x = MinI32(left, right);
	result.y = MinI32(bottom, top);
	result.z = MinI32(back, front);
	result.width = MaxI32(left, right) - result.x;
	result.height = MaxI32(bottom, top) - result.y;
	result.depth = MaxI32(back, front) - result.z;
	return result;
}
PEXPI boxi NewBoxiBetweenV(v3i bottomLeftBack, v3i topRightFront)
{
	boxi result;
	result.x = MinI32(bottomLeftBack.x, topRightFront.x);
	result.y = MinI32(bottomLeftBack.y, topRightFront.y);
	result.z = MinI32(bottomLeftBack.z, topRightFront.z);
	result.width = MaxI32(bottomLeftBack.x, topRightFront.x) - result.x;
	result.height = MaxI32(bottomLeftBack.y, topRightFront.y) - result.y;
	result.depth = MaxI32(bottomLeftBack.z, topRightFront.z) - result.z;
	return result;
}

PEXPI boxd NewBoxdCentered(r64 centerX, r64 centerY, r64 centerZ, r64 width, r64 height, r64 depth)
{
	boxd result;
	result.x = centerX - width/2.0;
	result.y = centerY - height/2.0;
	result.z = centerZ - depth/2.0;
	result.width = width;
	result.height = height;
	result.depth = depth;
	return result;
}
PEXPI boxd NewBoxdCenteredV(v3d center, v3d size)
{
	boxd result;
	result.bottomLeftBack = SubV3d(center, ShrinkV3d(size, 2.0));
	result.size = size;
	return result;
}
PEXPI boxd NewBoxdBetween(r64 left, r64 bottom, r64 back, r64 right, r64 top, r64 front)
{
	boxd result;
	result.x = MinR64(left, right);
	result.y = MinR64(bottom, top);
	result.z = MinR64(back, front);
	result.width = MaxR64(left, right) - result.x;
	result.height = MaxR64(bottom, top) - result.y;
	result.depth = MaxR64(back, front) - result.z;
	return result;
}
PEXPI boxd NewBoxdBetweenV(v3d bottomLeftBack, v3d topRightFront)
{
	boxd result;
	result.x = MinR64(bottomLeftBack.x, topRightFront.x);
	result.y = MinR64(bottomLeftBack.y, topRightFront.y);
	result.z = MinR64(bottomLeftBack.z, topRightFront.z);
	result.width = MaxR64(bottomLeftBack.x, topRightFront.x) - result.x;
	result.height = MaxR64(bottomLeftBack.y, topRightFront.y) - result.y;
	result.depth = MaxR64(bottomLeftBack.z, topRightFront.z) - result.z;
	return result;
}

// +--------------------------------------------------------------+
// |                 Simple Conversions and Casts                 |
// +--------------------------------------------------------------+
PEXPI rec   ToRecFromi(reci rectangle)        { return MakeRec((r32)rectangle.x, (r32)rectangle.y, (r32)rectangle.width, (r32)rectangle.height); }
PEXPI recd  ToRecdFromi(reci rectangle)       { return MakeRecd((r64)rectangle.x, (r64)rectangle.y, (r64)rectangle.width, (r64)rectangle.height); }
PEXPI recd  ToRecdFromf(rec rectangle)        { return MakeRecd((r64)rectangle.x, (r64)rectangle.y, (r64)rectangle.width, (r64)rectangle.height); }
PEXPI rec   ToRecFromd(recd rectangle)        { return MakeRec((r32)rectangle.x, (r32)rectangle.y, (r32)rectangle.width, (r32)rectangle.height); }
PEXPI reci  ToReciFromf(rec rectangle)        { return MakeReci((i32)rectangle.x, (i32)rectangle.y, (i32)rectangle.width, (i32)rectangle.height); }
PEXPI rec   ToRecFromV4(v4 vector)            { return MakeRec(vector.x, vector.y, vector.z, vector.w); }
PEXPI v4    ToV4FromRec(rec rectangle)        { return MakeV4(rectangle.x, rectangle.y, rectangle.width, rectangle.height); }
PEXPI v4r   ToV4rFromRec(rec rectangle)       { return MakeV4r(rectangle.x, rectangle.y, rectangle.width, rectangle.height); }
PEXPI v4d   ToV4dFromRecd(recd rectangle)     { return MakeV4d(rectangle.x, rectangle.y, rectangle.width, rectangle.height); }
PEXPI box   ToBoxFromi(boxi boundingBox)      { return MakeBox((r32)boundingBox.x, (r32)boundingBox.y, (r32)boundingBox.z, (r32)boundingBox.width, (r32)boundingBox.height, (r32)boundingBox.depth); }
PEXPI boxd  ToBoxdFromi(boxi boundingBox)     { return MakeBoxd((r64)boundingBox.x, (r64)boundingBox.y, (r64)boundingBox.z, (r64)boundingBox.width, (r64)boundingBox.height, (r64)boundingBox.depth); }
PEXPI boxd  ToBoxdFromf(box boundingBox)      { return MakeBoxd((r64)boundingBox.x, (r64)boundingBox.y, (r64)boundingBox.z, (r64)boundingBox.width, (r64)boundingBox.height, (r64)boundingBox.depth); }
PEXPI box   ToBoxFromd(boxd boundingBox)      { return MakeBox((r32)boundingBox.x, (r32)boundingBox.y, (r32)boundingBox.z, (r32)boundingBox.width, (r32)boundingBox.height, (r32)boundingBox.depth); }
PEXPI boxi  ToBoxiFromf(box boundingBox)      { return MakeBoxi((i32)boundingBox.x, (i32)boundingBox.y, (i32)boundingBox.z, (i32)boundingBox.width, (i32)boundingBox.height, (i32)boundingBox.depth); }
PEXPI obb2  ToObb2FromRec(rec rectangle)      { return MakeObb2(rectangle.x + rectangle.width/2.0f, rectangle.y + rectangle.height/2.0f, rectangle.width, rectangle.height, 0.0f); }
PEXPI obb2d ToObb2dFromRecd(recd rectangle)   { return MakeObb2d(rectangle.x + rectangle.width/2.0, rectangle.y + rectangle.height/2.0, rectangle.width, rectangle.height, 0.0); }
PEXPI obb3  ToObb3FromBox(box boundingBox)    { return MakeObb3(boundingBox.x + boundingBox.width/2.0f, boundingBox.y + boundingBox.height/2.0f, boundingBox.z + boundingBox.depth/2.0f, boundingBox.width, boundingBox.height, boundingBox.depth, Quat_Identity); }
PEXPI obb3d ToObb3dFromBoxd(boxd boundingBox) { return MakeObb3d(boundingBox.x + boundingBox.width/2.0, boundingBox.y + boundingBox.height/2.0, boundingBox.z + boundingBox.depth/2.0, boundingBox.width, boundingBox.height, boundingBox.depth, Quatd_Identity); }

#if TARGET_IS_ORCA
PEXPI oc_rect ToOcRect(rec rectangle) { return NEW_STRUCT(oc_rect){ .x = rectangle.x, .y = rectangle.y, .w = rectangle.width, .h = rectangle.height }; }
PEXPI rec ToRecFromOc(oc_rect orcaRectangle) { return MakeRec(orcaRectangle.x, orcaRectangle.y, orcaRectangle.w, orcaRectangle.h); }
#endif //TARGET_IS_ORCA

// +--------------------------------------------------------------+
// |                Operator Overload Equivalents                 |
// +--------------------------------------------------------------+
PEXPI rec   AddRec(rec rectangle,       v2  offset) { return MakeRec(rectangle.x + offset.x, rectangle.y + offset.y, rectangle.width, rectangle.height); }
PEXPI reci  AddReci(reci rectangle,     v2i offset) { return MakeReci(rectangle.x + offset.x, rectangle.y + offset.y, rectangle.width, rectangle.height); }
PEXPI recd  AddRecd(recd rectangle,     v2d offset) { return MakeRecd(rectangle.x + offset.x, rectangle.y + offset.y, rectangle.width, rectangle.height); }
PEXPI box   AddBox(box boundingBox,     v3  offset) { return MakeBox(boundingBox.x + offset.x, boundingBox.y + offset.y, boundingBox.z + offset.z, boundingBox.width, boundingBox.height, boundingBox.depth); }
PEXPI boxi  AddBoxi(boxi boundingBox,   v3i offset) { return MakeBoxi(boundingBox.x + offset.x, boundingBox.y + offset.y, boundingBox.z + offset.z, boundingBox.width, boundingBox.height, boundingBox.depth); }
PEXPI boxd  AddBoxd(boxd boundingBox,   v3d offset) { return MakeBoxd(boundingBox.x + offset.x, boundingBox.y + offset.y, boundingBox.z + offset.z, boundingBox.width, boundingBox.height, boundingBox.depth); }
PEXPI obb2  AddObb2(obb2 boundingBox,   v2  offset) { return MakeObb2(boundingBox.x + offset.x, boundingBox.y + offset.y, boundingBox.width, boundingBox.height, boundingBox.rotation); }
PEXPI obb2d AddObb2d(obb2d boundingBox, v2d offset) { return MakeObb2d(boundingBox.x + offset.x, boundingBox.y + offset.y, boundingBox.width, boundingBox.height, boundingBox.rotation); }
PEXPI obb3  AddObb3(obb3 boundingBox,   v3  offset) { return MakeObb3(boundingBox.x + offset.x, boundingBox.y + offset.y, boundingBox.z + offset.z, boundingBox.width, boundingBox.height, boundingBox.depth, boundingBox.rotation); }
PEXPI obb3d AddObb3d(obb3d boundingBox, v3d offset) { return MakeObb3d(boundingBox.x + offset.x, boundingBox.y + offset.y, boundingBox.z + offset.z, boundingBox.width, boundingBox.height, boundingBox.depth, boundingBox.rotation); }

PEXPI rec   SubRec(rec rectangle,       v2  offset) { return MakeRec(rectangle.x - offset.x, rectangle.y - offset.y, rectangle.width, rectangle.height); }
PEXPI reci  SubReci(reci rectangle,     v2i offset) { return MakeReci(rectangle.x - offset.x, rectangle.y - offset.y, rectangle.width, rectangle.height); }
PEXPI recd  SubRecd(recd rectangle,     v2d offset) { return MakeRecd(rectangle.x - offset.x, rectangle.y - offset.y, rectangle.width, rectangle.height); }
PEXPI box   SubBox(box boundingBox,     v3  offset) { return MakeBox(boundingBox.x - offset.x, boundingBox.y - offset.y, boundingBox.z - offset.z, boundingBox.width, boundingBox.height, boundingBox.depth); }
PEXPI boxi  SubBoxi(boxi boundingBox,   v3i offset) { return MakeBoxi(boundingBox.x - offset.x, boundingBox.y - offset.y, boundingBox.z - offset.z, boundingBox.width, boundingBox.height, boundingBox.depth); }
PEXPI boxd  SubBoxd(boxd boundingBox,   v3d offset) { return MakeBoxd(boundingBox.x - offset.x, boundingBox.y - offset.y, boundingBox.z - offset.z, boundingBox.width, boundingBox.height, boundingBox.depth); }
PEXPI obb2  SubObb2(obb2 boundingBox,   v2  offset) { return MakeObb2(boundingBox.x - offset.x, boundingBox.y - offset.y, boundingBox.width, boundingBox.height, boundingBox.rotation); }
PEXPI obb2d SubObb2d(obb2d boundingBox, v2d offset) { return MakeObb2d(boundingBox.x - offset.x, boundingBox.y - offset.y, boundingBox.width, boundingBox.height, boundingBox.rotation); }
PEXPI obb3  SubObb3(obb3 boundingBox,   v3  offset) { return MakeObb3(boundingBox.x - offset.x, boundingBox.y - offset.y, boundingBox.z - offset.z, boundingBox.width, boundingBox.height, boundingBox.depth, boundingBox.rotation); }
PEXPI obb3d SubObb3d(obb3d boundingBox, v3d offset) { return MakeObb3d(boundingBox.x - offset.x, boundingBox.y - offset.y, boundingBox.z - offset.z, boundingBox.width, boundingBox.height, boundingBox.depth, boundingBox.rotation); }

PEXPI rec   MulRec(rec rectangle,       v2  scale) { return MakeRec(rectangle.x * scale.x, rectangle.y * scale.y, rectangle.width * scale.x, rectangle.height * scale.y); }
PEXPI reci  MulReci(reci rectangle,     v2i scale) { return MakeReci(rectangle.x * scale.x, rectangle.y * scale.y, rectangle.width * scale.x, rectangle.height * scale.y); }
PEXPI recd  MulRecd(recd rectangle,     v2d scale) { return MakeRecd(rectangle.x * scale.x, rectangle.y * scale.y, rectangle.width * scale.x, rectangle.height * scale.y); }
PEXPI box   MulBox(box boundingBox,     v3  scale) { return MakeBox(boundingBox.x * scale.x, boundingBox.y * scale.y, boundingBox.z * scale.z, boundingBox.width * scale.x, boundingBox.height * scale.y, boundingBox.depth * scale.z); }
PEXPI boxi  MulBoxi(boxi boundingBox,   v3i scale) { return MakeBoxi(boundingBox.x * scale.x, boundingBox.y * scale.y, boundingBox.z * scale.z, boundingBox.width * scale.x, boundingBox.height * scale.y, boundingBox.depth * scale.z); }
PEXPI boxd  MulBoxd(boxd boundingBox,   v3d scale) { return MakeBoxd(boundingBox.x * scale.x, boundingBox.y * scale.y, boundingBox.z * scale.z, boundingBox.width * scale.x, boundingBox.height * scale.y, boundingBox.depth * scale.z); }
PEXPI obb2  MulObb2(obb2 boundingBox,   v2  scale) { return MakeObb2(boundingBox.x * scale.x, boundingBox.y * scale.y, boundingBox.width * scale.x, boundingBox.height * scale.y, boundingBox.rotation); }
PEXPI obb2d MulObb2d(obb2d boundingBox, v2d scale) { return MakeObb2d(boundingBox.x * scale.x, boundingBox.y * scale.y, boundingBox.width * scale.x, boundingBox.height * scale.y, boundingBox.rotation); }
PEXPI obb3  MulObb3(obb3 boundingBox,   v3  scale) { return MakeObb3(boundingBox.x * scale.x, boundingBox.y * scale.y, boundingBox.z * scale.z, boundingBox.width * scale.x, boundingBox.height * scale.y, boundingBox.depth * scale.z, boundingBox.rotation); }
PEXPI obb3d MulObb3d(obb3d boundingBox, v3d scale) { return MakeObb3d(boundingBox.x * scale.x, boundingBox.y * scale.y, boundingBox.z * scale.z, boundingBox.width * scale.x, boundingBox.height * scale.y, boundingBox.depth * scale.z, boundingBox.rotation); }

PEXPI rec   DivRec(rec rectangle,       v2  divisor) { return MakeRec(rectangle.x / divisor.x, rectangle.y / divisor.y, rectangle.width / divisor.x, rectangle.height / divisor.y); }
PEXPI reci  DivReci(reci rectangle,     v2i divisor) { return MakeReci(rectangle.x / divisor.x, rectangle.y / divisor.y, rectangle.width / divisor.x, rectangle.height / divisor.y); }
PEXPI recd  DivRecd(recd rectangle,     v2d divisor) { return MakeRecd(rectangle.x / divisor.x, rectangle.y / divisor.y, rectangle.width / divisor.x, rectangle.height / divisor.y); }
PEXPI box   DivBox(box boundingBox,     v3  divisor) { return MakeBox(boundingBox.x / divisor.x, boundingBox.y / divisor.y, boundingBox.z / divisor.z, boundingBox.width / divisor.x, boundingBox.height / divisor.y, boundingBox.depth / divisor.z); }
PEXPI boxi  DivBoxi(boxi boundingBox,   v3i divisor) { return MakeBoxi(boundingBox.x / divisor.x, boundingBox.y / divisor.y, boundingBox.z / divisor.z, boundingBox.width / divisor.x, boundingBox.height / divisor.y, boundingBox.depth / divisor.z); }
PEXPI boxd  DivBoxd(boxd boundingBox,   v3d divisor) { return MakeBoxd(boundingBox.x / divisor.x, boundingBox.y / divisor.y, boundingBox.z / divisor.z, boundingBox.width / divisor.x, boundingBox.height / divisor.y, boundingBox.depth / divisor.z); }
PEXPI obb2  DivObb2(obb2 boundingBox,   v2  divisor) { return MakeObb2(boundingBox.x / divisor.x, boundingBox.y / divisor.y, boundingBox.width / divisor.x, boundingBox.height / divisor.y, boundingBox.rotation); }
PEXPI obb2d DivObb2d(obb2d boundingBox, v2d divisor) { return MakeObb2d(boundingBox.x / divisor.x, boundingBox.y / divisor.y, boundingBox.width / divisor.x, boundingBox.height / divisor.y, boundingBox.rotation); }
PEXPI obb3  DivObb3(obb3 boundingBox,   v3  divisor) { return MakeObb3(boundingBox.x / divisor.x, boundingBox.y / divisor.y, boundingBox.z / divisor.z, boundingBox.width / divisor.x, boundingBox.height / divisor.y, boundingBox.depth / divisor.z, boundingBox.rotation); }
PEXPI obb3d DivObb3d(obb3d boundingBox, v3d divisor) { return MakeObb3d(boundingBox.x / divisor.x, boundingBox.y / divisor.y, boundingBox.z / divisor.z, boundingBox.width / divisor.x, boundingBox.height / divisor.y, boundingBox.depth / divisor.z, boundingBox.rotation); }

PEXPI rec   ScaleRec(rec     rectangle,   r32 scalar) { return MakeRec(rectangle.x * scalar, rectangle.y * scalar, rectangle.width * scalar, rectangle.height * scalar); }
PEXPI reci  ScaleReci(reci   rectangle,   i32 scalar) { return MakeReci(rectangle.x * scalar, rectangle.y * scalar, rectangle.width * scalar, rectangle.height * scalar); }
PEXPI recd  ScaleRecd(recd   rectangle,   r64 scalar) { return MakeRecd(rectangle.x * scalar, rectangle.y * scalar, rectangle.width * scalar, rectangle.height * scalar); }
PEXPI box   ScaleBox(box     boundingBox, r32 scalar) { return MakeBox(boundingBox.x * scalar, boundingBox.y * scalar, boundingBox.z * scalar, boundingBox.width * scalar, boundingBox.height * scalar, boundingBox.depth * scalar); }
PEXPI boxi  ScaleBoxi(boxi   boundingBox, i32 scalar) { return MakeBoxi(boundingBox.x * scalar, boundingBox.y * scalar, boundingBox.z * scalar, boundingBox.width * scalar, boundingBox.height * scalar, boundingBox.depth * scalar); }
PEXPI boxd  ScaleBoxd(boxd   boundingBox, r64 scalar) { return MakeBoxd(boundingBox.x * scalar, boundingBox.y * scalar, boundingBox.z * scalar, boundingBox.width * scalar, boundingBox.height * scalar, boundingBox.depth * scalar); }
PEXPI obb2  ScaleObb2(obb2   boundingBox, r32 scalar) { return MakeObb2(boundingBox.x * scalar, boundingBox.y * scalar, boundingBox.width * scalar, boundingBox.height * scalar, boundingBox.rotation); }
PEXPI obb2d ScaleObb2d(obb2d boundingBox, r64 scalar) { return MakeObb2d(boundingBox.x * scalar, boundingBox.y * scalar, boundingBox.width * scalar, boundingBox.height * scalar, boundingBox.rotation); }
PEXPI obb3  ScaleObb3(obb3   boundingBox, r32 scalar) { return MakeObb3(boundingBox.x * scalar, boundingBox.y * scalar, boundingBox.z * scalar, boundingBox.width * scalar, boundingBox.height * scalar, boundingBox.depth * scalar, boundingBox.rotation); }
PEXPI obb3d ScaleObb3d(obb3d boundingBox, r64 scalar) { return MakeObb3d(boundingBox.x * scalar, boundingBox.y * scalar, boundingBox.z * scalar, boundingBox.width * scalar, boundingBox.height * scalar, boundingBox.depth * scalar, boundingBox.rotation); }

PEXPI rec   ShrinkRec(rec rectangle,       r32 divisor) { return MakeRec(rectangle.x / divisor, rectangle.y / divisor, rectangle.width / divisor, rectangle.height / divisor); }
PEXPI reci  ShrinkReci(reci rectangle,     i32 divisor) { return MakeReci(rectangle.x / divisor, rectangle.y / divisor, rectangle.width / divisor, rectangle.height / divisor); }
PEXPI recd  ShrinkRecd(recd rectangle,     r64 divisor) { return MakeRecd(rectangle.x / divisor, rectangle.y / divisor, rectangle.width / divisor, rectangle.height / divisor); }
PEXPI box   ShrinkBox(box boundingBox,     r32 divisor) { return MakeBox(boundingBox.x / divisor, boundingBox.y / divisor, boundingBox.z / divisor, boundingBox.width / divisor, boundingBox.height / divisor, boundingBox.depth / divisor); }
PEXPI boxi  ShrinkBoxi(boxi boundingBox,   i32 divisor) { return MakeBoxi(boundingBox.x / divisor, boundingBox.y / divisor, boundingBox.z / divisor, boundingBox.width / divisor, boundingBox.height / divisor, boundingBox.depth / divisor); }
PEXPI boxd  ShrinkBoxd(boxd boundingBox,   r64 divisor) { return MakeBoxd(boundingBox.x / divisor, boundingBox.y / divisor, boundingBox.z / divisor, boundingBox.width / divisor, boundingBox.height / divisor, boundingBox.depth / divisor); }
PEXPI obb2  ShrinkObb2(obb2 boundingBox,   r32 divisor) { return MakeObb2(boundingBox.x / divisor, boundingBox.y / divisor, boundingBox.width / divisor, boundingBox.height / divisor, boundingBox.rotation); }
PEXPI obb2d ShrinkObb2d(obb2d boundingBox, r64 divisor) { return MakeObb2d(boundingBox.x / divisor, boundingBox.y / divisor, boundingBox.width / divisor, boundingBox.height / divisor, boundingBox.rotation); }
PEXPI obb3  ShrinkObb3(obb3 boundingBox,   r32 divisor) { return MakeObb3(boundingBox.x / divisor, boundingBox.y / divisor, boundingBox.z / divisor, boundingBox.width / divisor, boundingBox.height / divisor, boundingBox.depth / divisor, boundingBox.rotation); }
PEXPI obb3d ShrinkObb3d(obb3d boundingBox, r64 divisor) { return MakeObb3d(boundingBox.x / divisor, boundingBox.y / divisor, boundingBox.z / divisor, boundingBox.width / divisor, boundingBox.height / divisor, boundingBox.depth / divisor, boundingBox.rotation); }

PEXPI bool AreEqualRec(rec     left, rec   right) { return (left.x == right.x && left.y == right.y && left.width == right.width && left.height == right.height); }
PEXPI bool AreEqualReci(reci   left, reci  right) { return (left.x == right.x && left.y == right.y && left.width == right.width && left.height == right.height); }
PEXPI bool AreEqualRecd(recd   left, recd  right) { return (left.x == right.x && left.y == right.y && left.width == right.width && left.height == right.height); }
PEXPI bool AreEqualBox(box     left, box   right) { return (left.x == right.x && left.y == right.y && left.z == right.z && left.width == right.width && left.height == right.height && left.depth == right.depth); }
PEXPI bool AreEqualBoxi(boxi   left, boxi  right) { return (left.x == right.x && left.y == right.y && left.z == right.z && left.width == right.width && left.height == right.height && left.depth == right.depth); }
PEXPI bool AreEqualBoxd(boxd   left, boxd  right) { return (left.x == right.x && left.y == right.y && left.z == right.z && left.width == right.width && left.height == right.height && left.depth == right.depth); }
PEXPI bool AreEqualObb2(obb2   left, obb2  right) { return (left.x == right.x && left.y == right.y && left.width == right.width && left.height == right.height && left.rotation == right.rotation); }
PEXPI bool AreEqualObb2d(obb2d left, obb2d right) { return (left.x == right.x && left.y == right.y && left.width == right.width && left.height == right.height && left.rotation == right.rotation); }
PEXPI bool AreEqualObb3(obb3   left, obb3  right) { return (left.x == right.x && left.y == right.y && left.z == right.z && left.width == right.width && left.height == right.height && left.depth == right.depth && AreEqualQuat(left.rotation, right.rotation)); }
PEXPI bool AreEqualObb3d(obb3d left, obb3d right) { return (left.x == right.x && left.y == right.y && left.z == right.z && left.width == right.width && left.height == right.height && left.depth == right.depth && AreEqualQuatd(left.rotation, right.rotation)); }

//TODO: AreSimilarRec?

// +--------------------------------------------------------------+
// |            Basic Manipulation and Math Functions             |
// +--------------------------------------------------------------+
//TODO: InvertRec/InvertXRec/InvertYRec?
//TODO: LerpRec?
//TODO: RotateObb2Clockwise(uxx numQuarterTurns)/RotateObb2CounterClockwise(uxx numQuarterTurns)

PEXPI bool DoesOverlapRec(rec rectangle1, rec rectangle2, bool inclusive)
{
	return inclusive ? (
		rectangle1.x <= (rectangle2.x + rectangle2.width)  &&
		rectangle1.y <= (rectangle2.y + rectangle2.height) &&
		rectangle2.x <= (rectangle1.x + rectangle1.width)  &&
		rectangle2.y <= (rectangle1.y + rectangle1.height)
	) : (
		rectangle1.x < (rectangle2.x + rectangle2.width)  &&
		rectangle1.y < (rectangle2.y + rectangle2.height) &&
		rectangle2.x < (rectangle1.x + rectangle1.width)  &&
		rectangle2.y < (rectangle1.y + rectangle1.height)
	);
}
PEXPI bool DoesOverlapReci(reci rectangle1, reci rectangle2, bool inclusive)
{
	return inclusive ? (
		rectangle1.x <= (rectangle2.x + rectangle2.width)  &&
		rectangle1.y <= (rectangle2.y + rectangle2.height) &&
		rectangle2.x <= (rectangle1.x + rectangle1.width)  &&
		rectangle2.y <= (rectangle1.y + rectangle1.height)
	) : (
		rectangle1.x < (rectangle2.x + rectangle2.width)  &&
		rectangle1.y < (rectangle2.y + rectangle2.height) &&
		rectangle2.x < (rectangle1.x + rectangle1.width)  &&
		rectangle2.y < (rectangle1.y + rectangle1.height)
	);
}
PEXPI bool DoesOverlapRecd(recd rectangle1, recd rectangle2, bool inclusive)
{
	return inclusive ? (
		rectangle1.x <= (rectangle2.x + rectangle2.width)  &&
		rectangle1.y <= (rectangle2.y + rectangle2.height) &&
		rectangle2.x <= (rectangle1.x + rectangle1.width)  &&
		rectangle2.y <= (rectangle1.y + rectangle1.height)
	) : (
		rectangle1.x < (rectangle2.x + rectangle2.width)  &&
		rectangle1.y < (rectangle2.y + rectangle2.height) &&
		rectangle2.x < (rectangle1.x + rectangle1.width)  &&
		rectangle2.y < (rectangle1.y + rectangle1.height)
	);
}
PEXPI bool DoesOverlapBox(box rectangle1, box rectangle2, bool inclusive)
{
	return inclusive ? (
		rectangle1.x <= (rectangle2.x + rectangle2.width)  &&
		rectangle1.y <= (rectangle2.y + rectangle2.height) &&
		rectangle1.z <= (rectangle2.z + rectangle2.depth)  &&
		rectangle2.x <= (rectangle1.x + rectangle1.width)  &&
		rectangle2.y <= (rectangle1.y + rectangle1.height) &&
		rectangle2.z <= (rectangle1.z + rectangle1.depth)
	) : (
		rectangle1.x < (rectangle2.x + rectangle2.width)  &&
		rectangle1.y < (rectangle2.y + rectangle2.height) &&
		rectangle1.z < (rectangle2.z + rectangle2.depth)  &&
		rectangle2.x < (rectangle1.x + rectangle1.width)  &&
		rectangle2.y < (rectangle1.y + rectangle1.height) &&
		rectangle2.z < (rectangle1.z + rectangle1.depth)
	);
}
PEXPI bool DoesOverlapBoxi(boxi rectangle1, boxi rectangle2, bool inclusive)
{
	return inclusive ? (
		rectangle1.x <= (rectangle2.x + rectangle2.width)  &&
		rectangle1.y <= (rectangle2.y + rectangle2.height) &&
		rectangle1.z <= (rectangle2.z + rectangle2.depth)  &&
		rectangle2.x <= (rectangle1.x + rectangle1.width)  &&
		rectangle2.y <= (rectangle1.y + rectangle1.height) &&
		rectangle2.z <= (rectangle1.z + rectangle1.depth)
	) : (
		rectangle1.x < (rectangle2.x + rectangle2.width)  &&
		rectangle1.y < (rectangle2.y + rectangle2.height) &&
		rectangle1.z < (rectangle2.z + rectangle2.depth)  &&
		rectangle2.x < (rectangle1.x + rectangle1.width)  &&
		rectangle2.y < (rectangle1.y + rectangle1.height) &&
		rectangle2.z < (rectangle1.z + rectangle1.depth)
	);
}
PEXPI bool DoesOverlapBoxd(boxd rectangle1, boxd rectangle2, bool inclusive)
{
	return inclusive ? (
		rectangle1.x <= (rectangle2.x + rectangle2.width)  &&
		rectangle1.y <= (rectangle2.y + rectangle2.height) &&
		rectangle1.z <= (rectangle2.z + rectangle2.depth)  &&
		rectangle2.x <= (rectangle1.x + rectangle1.width)  &&
		rectangle2.y <= (rectangle1.y + rectangle1.height) &&
		rectangle2.z <= (rectangle1.z + rectangle1.depth)
	) : (
		rectangle1.x < (rectangle2.x + rectangle2.width)  &&
		rectangle1.y < (rectangle2.y + rectangle2.height) &&
		rectangle1.z < (rectangle2.z + rectangle2.depth)  &&
		rectangle2.x < (rectangle1.x + rectangle1.width)  &&
		rectangle2.y < (rectangle1.y + rectangle1.height) &&
		rectangle2.z < (rectangle1.z + rectangle1.depth)
	);
}

PEXPI rec  InflateRecEx(rec   rectangle,   r32 leftAmount, r32 rightAmount, r32 topAmount, r32 bottomAmount) { return MakeRec(rectangle.x - leftAmount, rectangle.y - topAmount, rectangle.width + leftAmount + rightAmount, rectangle.height + topAmount + bottomAmount); }
PEXPI rec  InflateRec(rec     rectangle,   r32 amount) { return InflateRecEx(rectangle, amount, amount, amount, amount); }
PEXPI rec  InflateRecX(rec    rectangle,   r32 amount) { return InflateRecEx(rectangle, amount, amount, 0.0f, 0.0f); }
PEXPI rec  InflateRecY(rec    rectangle,   r32 amount) { return InflateRecEx(rectangle, 0.0f, 0.0f, amount, amount); }
PEXPI reci InflateReciEx(reci rectangle,   i32 leftAmount, i32 rightAmount, i32 topAmount, i32 bottomAmount) { return MakeReci(rectangle.x - leftAmount, rectangle.y - topAmount, rectangle.width + leftAmount + rightAmount, rectangle.height + topAmount + bottomAmount); }
PEXPI reci InflateReci(reci   rectangle,   i32 amount) { return InflateReciEx(rectangle, amount, amount, amount, amount); }
PEXPI reci InflateReciX(reci  rectangle,   i32 amount) { return InflateReciEx(rectangle, amount, amount, 0, 0); }
PEXPI reci InflateReciY(reci  rectangle,   i32 amount) { return InflateReciEx(rectangle, 0, 0, amount, amount); }
PEXPI recd InflateRecdEx(recd rectangle,   r64 leftAmount, r64 rightAmount, r64 topAmount, r64 bottomAmount) { return MakeRecd(rectangle.x - leftAmount, rectangle.y - topAmount, rectangle.width + leftAmount + rightAmount, rectangle.height + topAmount + bottomAmount); }
PEXPI recd InflateRecd(recd   rectangle,   r64 amount) { return InflateRecdEx(rectangle, amount, amount, amount, amount); }
PEXPI recd InflateRecdX(recd  rectangle,   r64 amount) { return InflateRecdEx(rectangle, amount, amount, 0.0, 0.0); }
PEXPI recd InflateRecdY(recd  rectangle,   r64 amount) { return InflateRecdEx(rectangle, 0.0, 0.0, amount, amount); }
PEXPI box  InflateBoxEx(box   boundingBox, r32 leftAmount, r32 rightAmount, r32 topAmount, r32 bottomAmount, r32 backAmount, r32 frontAmount) { return MakeBox(boundingBox.x - leftAmount, boundingBox.y - bottomAmount, boundingBox.z - backAmount, boundingBox.width + leftAmount + rightAmount, boundingBox.height + bottomAmount + topAmount, boundingBox.depth + backAmount + frontAmount); }
PEXPI box  InflateBox(box     boundingBox, r32 amount) { return InflateBoxEx(boundingBox, amount, amount, amount, amount, amount, amount); }
PEXPI box  InflateBoxX(box    boundingBox, r32 amount) { return InflateBoxEx(boundingBox, amount, amount, 0.0f, 0.0f, 0.0f, 0.0f); }
PEXPI box  InflateBoxY(box    boundingBox, r32 amount) { return InflateBoxEx(boundingBox, 0.0f, 0.0f, amount, amount, 0.0f, 0.0f); }
PEXPI box  InflateBoxZ(box    boundingBox, r32 amount) { return InflateBoxEx(boundingBox, 0.0f, 0.0f, 0.0f, 0.0f, amount, amount); }
PEXPI boxi InflateBoxiEx(boxi boundingBox, i32 leftAmount, i32 rightAmount, i32 topAmount, i32 bottomAmount, i32 backAmount, i32 frontAmount) { return MakeBoxi(boundingBox.x - leftAmount, boundingBox.y - bottomAmount, boundingBox.z - backAmount, boundingBox.width + leftAmount + rightAmount, boundingBox.height + bottomAmount + topAmount, boundingBox.depth + backAmount + frontAmount); }
PEXPI boxi InflateBoxi(boxi   boundingBox, i32 amount) { return InflateBoxiEx(boundingBox, amount, amount, amount, amount, amount, amount); }
PEXPI boxi InflateBoxiX(boxi  boundingBox, i32 amount) { return InflateBoxiEx(boundingBox, amount, amount, 0, 0, 0, 0); }
PEXPI boxi InflateBoxiY(boxi  boundingBox, i32 amount) { return InflateBoxiEx(boundingBox, 0, 0, amount, amount, 0, 0); }
PEXPI boxi InflateBoxiZ(boxi  boundingBox, i32 amount) { return InflateBoxiEx(boundingBox, 0, 0, 0, 0, amount, amount); }
PEXPI boxd InflateBoxdEx(boxd boundingBox, r64 leftAmount, r64 rightAmount, r64 topAmount, r64 bottomAmount, r64 backAmount, r64 frontAmount) { return MakeBoxd(boundingBox.x - leftAmount, boundingBox.y - bottomAmount, boundingBox.z - backAmount, boundingBox.width + leftAmount + rightAmount, boundingBox.height + bottomAmount + topAmount, boundingBox.depth + backAmount + frontAmount); }
PEXPI boxd InflateBoxd(boxd   boundingBox, r64 amount) { return InflateBoxdEx(boundingBox, amount, amount, amount, amount, amount, amount); }
PEXPI boxd InflateBoxdX(boxd  boundingBox, r64 amount) { return InflateBoxdEx(boundingBox, amount, amount, 0.0, 0.0, 0.0, 0.0); }
PEXPI boxd InflateBoxdY(boxd  boundingBox, r64 amount) { return InflateBoxdEx(boundingBox, 0.0, 0.0, amount, amount, 0.0, 0.0); }
PEXPI boxd InflateBoxdZ(boxd  boundingBox, r64 amount) { return InflateBoxdEx(boundingBox, 0.0, 0.0, 0.0, 0.0, amount, amount); }

//TODO: DeflateRec/DeflateXRec/DeflateYRec?
//TODO: ExpandRightRec/ExpandLeftRec/ExpandUpRec/ExpandDownRec?
//TODO: RetractRightRec/RetractLeftRec/RetractUpRec/RetractDownRec?
//TODO: SquarifyRec/CubifyBox?

PEXPI rec  BothRec(rec   left, rec  right) { r32 minX = MinR32(left.x, right.x); r32 minY = MinR32(left.y, right.y); return MakeRec(minX, minY, MaxR32(left.x + left.width, right.x + right.width) - minX, MaxR32(left.y + left.height, right.y + right.height) - minY); }
PEXPI reci BothReci(reci left, reci right) { i32 minX = MinI32(left.x, right.x); i32 minY = MinI32(left.y, right.y); return MakeReci(minX, minY, MaxI32(left.x + left.width, right.x + right.width) - minX, MaxI32(left.y + left.height, right.y + right.height) - minY); }
PEXPI recd BothRecd(recd left, recd right) { r64 minX = MinR64(left.x, right.x); r64 minY = MinR64(left.y, right.y); return MakeRecd(minX, minY, MaxR64(left.x + left.width, right.x + right.width) - minX, MaxR64(left.y + left.height, right.y + right.height) - minY); }
PEXPI box  BothBox(box   left, box  right) { r32 minX = MinR32(left.x, right.x); r32 minY = MinR32(left.y, right.y); r32 minZ = MinR32(left.z, right.z); return MakeBox(minX, minY, minZ, MaxR32(left.x + left.width, right.x + right.width) - minX, MaxR32(left.y + left.height, right.y + right.height) - minY, MaxR32(left.z + left.depth, right.z + right.depth) - minZ); }
PEXPI boxi BothBoxi(boxi left, boxi right) { i32 minX = MinI32(left.x, right.x); i32 minY = MinI32(left.y, right.y); i32 minZ = MinI32(left.z, right.z); return MakeBoxi(minX, minY, minZ, MaxI32(left.x + left.width, right.x + right.width) - minX, MaxI32(left.y + left.height, right.y + right.height) - minY, MaxI32(left.z + left.depth, right.z + right.depth) - minZ); }
PEXPI boxd BothBoxd(boxd left, boxd right) { r64 minX = MinR64(left.x, right.x); r64 minY = MinR64(left.y, right.y); r64 minZ = MinR64(left.z, right.z); return MakeBoxd(minX, minY, minZ, MaxR64(left.x + left.width, right.x + right.width) - minX, MaxR64(left.y + left.height, right.y + right.height) - minY, MaxR64(left.z + left.depth, right.z + right.depth) - minZ); }

PEXPI rec OverlapPartRec(rec left, rec right)
{
	r32 minX = MaxR32(left.x, right.x);
	r32 minY = MaxR32(left.y, right.y);
	r32 maxX = MaxR32(MinR32(left.x + left.width, right.x + right.width), minX);
	r32 maxY = MaxR32(MinR32(left.y + left.height, right.y + right.height), minY);
	return MakeRec(minX, minY, maxX - minX, maxY - minY);
}
PEXPI reci OverlapPartReci(reci left, reci right)
{
	i32 minX = MaxI32(left.x, right.x);
	i32 minY = MaxI32(left.y, right.y);
	i32 maxX = MaxI32(MinI32(left.x + left.width, right.x + right.width), minX);
	i32 maxY = MaxI32(MinI32(left.y + left.height, right.y + right.height), minY);
	return MakeReci(minX, minY, maxX - minX, maxY - minY);
}
PEXPI recd OverlapPartRecd(recd left, recd right)
{
	r64 minX = MaxR64(left.x, right.x);
	r64 minY = MaxR64(left.y, right.y);
	r64 maxX = MaxR64(MinR64(left.x + left.width, right.x + right.width), minX);
	r64 maxY = MaxR64(MinR64(left.y + left.height, right.y + right.height), minY);
	return MakeRecd(minX, minY, maxX - minX, maxY - minY);
}
PEXPI box OverlapPartBox(box left, box right)
{
	r32 minX = MaxR32(left.x, right.x);
	r32 minY = MaxR32(left.y, right.y);
	r32 minZ = MaxR32(left.z, right.z);
	r32 maxX = MaxR32(MinR32(left.x + left.width, right.x + right.width), minX);
	r32 maxY = MaxR32(MinR32(left.y + left.height, right.y + right.height), minY);
	r32 maxZ = MaxR32(MinR32(left.z + left.depth, right.z + right.depth), minZ);
	return MakeBox(minX, minY, minZ, maxX - minX, maxY - minY, maxZ - minZ);
}
PEXPI boxi OverlapPartBoxi(boxi left, boxi right)
{
	i32 minX = MaxI32(left.x, right.x);
	i32 minY = MaxI32(left.y, right.y);
	i32 minZ = MaxI32(left.z, right.z);
	i32 maxX = MaxI32(MinI32(left.x + left.width, right.x + right.width), minX);
	i32 maxY = MaxI32(MinI32(left.y + left.height, right.y + right.height), minY);
	i32 maxZ = MaxI32(MinI32(left.z + left.depth, right.z + right.depth), minZ);
	return MakeBoxi(minX, minY, minZ, maxX - minX, maxY - minY, maxZ - minZ);
}
PEXPI boxd OverlapPartBoxd(boxd left, boxd right)
{
	r64 minX = MaxR64(left.x, right.x);
	r64 minY = MaxR64(left.y, right.y);
	r64 minZ = MaxR64(left.z, right.z);
	r64 maxX = MaxR64(MinR64(left.x + left.width, right.x + right.width), minX);
	r64 maxY = MaxR64(MinR64(left.y + left.height, right.y + right.height), minY);
	r64 maxZ = MaxR64(MinR64(left.z + left.depth, right.z + right.depth), minZ);
	return MakeBoxd(minX, minY, minZ, maxX - minX, maxY - minY, maxZ - minZ);
}

PEXPI bool IsInsideRec(rec rectangle, v2 position) { return (position.x >= rectangle.x && position.y >= rectangle.y && position.x < rectangle.x + rectangle.width && position.y < rectangle.y + rectangle.height); }
PEXPI bool IsInsideRecd(recd rectangle, v2d position) { return (position.x >= rectangle.x && position.y >= rectangle.y && position.x < rectangle.x + rectangle.width && position.y < rectangle.y + rectangle.height); }
PEXPI bool IsInsideRecInclusive(rec rectangle, v2 position) { return (position.x >= rectangle.x && position.y >= rectangle.y && position.x <= rectangle.x + rectangle.width && position.y <= rectangle.y + rectangle.height); }
PEXPI bool IsInsideRecdInclusive(rec rectangle, v2d position) { return (position.x >= rectangle.x && position.y >= rectangle.y && position.x <= rectangle.x + rectangle.width && position.y <= rectangle.y + rectangle.height); }

PEXPI void AlignRecToV2(rec* rectangleOut, v2 alignmentScale)
{
	v2 bottomRight = AddV2(rectangleOut->topLeft, rectangleOut->size);
	AlignV2ToV2(&bottomRight, alignmentScale);
	AlignV2ToV2(&rectangleOut->topLeft, alignmentScale);
	rectangleOut->size = SubV2(bottomRight, rectangleOut->topLeft);
}
PEXPI void AlignRecTo(rec* rectangleOut, r32 alignmentScale) { AlignRecToV2(rectangleOut, FillV2(alignmentScale)); }
PEXPI void AlignRec(rec* rectangleOut) { AlignRecToV2(rectangleOut, V2_One); }

PEXPI void AlignRecdToV2d(recd* rectangleOut, v2d alignmentScale)
{
	v2d bottomRight = AddV2d(rectangleOut->topLeft, rectangleOut->size);
	AlignV2dToV2d(&bottomRight, alignmentScale);
	AlignV2dToV2d(&rectangleOut->topLeft, alignmentScale);
	rectangleOut->size = SubV2d(bottomRight, rectangleOut->topLeft);
}
PEXPI void AlignRecdTo(recd* rectangleOut, r64 alignmentScale) { AlignRecdToV2d(rectangleOut, FillV2d(alignmentScale)); }
PEXPI void AlignRecd(recd* rectangleOut) { AlignRecdToV2d(rectangleOut, V2d_One); }

PEXPI void AlignBoxToV3(box* boundingBoxOut, v3 alignmentScale)
{
	v3 topRightFront = AddV3(boundingBoxOut->bottomLeftBack, boundingBoxOut->size);
	AlignV3ToV3(&topRightFront, alignmentScale);
	AlignV3ToV3(&boundingBoxOut->bottomLeftBack, alignmentScale);
	boundingBoxOut->size = SubV3(topRightFront, boundingBoxOut->bottomLeftBack);
}
PEXPI void AlignBoxTo(box* boundingBoxOut, r32 alignmentScale) { AlignBoxToV3(boundingBoxOut, FillV3(alignmentScale)); }
PEXPI void AlignBox(box* boundingBoxOut) { AlignBoxToV3(boundingBoxOut, V3_One); }

PEXPI void AlignBoxdToV3d(boxd* boundingBoxOut, v3d alignmentScale)
{
	v3d topRightFront = AddV3d(boundingBoxOut->bottomLeftBack, boundingBoxOut->size);
	AlignV3dToV3d(&topRightFront, alignmentScale);
	AlignV3dToV3d(&boundingBoxOut->bottomLeftBack, alignmentScale);
	boundingBoxOut->size = SubV3d(topRightFront, boundingBoxOut->bottomLeftBack);
}
PEXPI void AlignBoxdTo(boxd* boundingBoxOut, r64 alignmentScale) { AlignBoxdToV3d(boundingBoxOut, FillV3d(alignmentScale)); }
PEXPI void AlignBoxd(boxd* boundingBoxOut) { AlignBoxdToV3d(boundingBoxOut, V3d_One); }

//TODO: ExpandRecToV2/ExpandReciToV2i?
//TODO: IsInsideBox/IsInsideObb2/IsRecInsideRec?
//TODO: ?
//TODO: GetObb2AlignedBounds?
//TODO: AlignRec/AlignRecTopLeft/AlignRecSize?
//TODO: MakeObb2Line(v2 start, v2 end, r32 thickness)

//TODO: v2 GetObb2DRelativePos(obb2 boundingBox, v2 point);
//TODO: v2 GetObb2DWorldPoint(obb2 boundingBox, v2 relativeOffset);
//TODO: v2 GetObb2DRightVec(obb2 boundingBox);
//TODO: v2 GetObb2DDownVec(obb2 boundingBox);
//TODO: v2 GetObb2DLeftVec(obb2 boundingBox);
//TODO: v2 GetObb2DUpVec(obb2 boundingBox);
//TODO: v2 GetObb2DTopLeft(obb2 boundingBox);
//TODO: v2 GetObb2DTopRight(obb2 boundingBox);
//TODO: v2 GetObb2DBottomLeft(obb2 boundingBox);
//TODO: v2 GetObb2DBottomRight(obb2 boundingBox);

// +--------------------------------------------------------------+
// |           Advanced Manipulation and Math Functions           |
// +--------------------------------------------------------------+
//TODO: Can we come up with better names for the parameters? Or maybe a better name for the function?
// Produces a rectangle that is a subset of other that is proportionally to the relationship between reference and subReference
// This is most commonly used to find a texture sourceRec when drawing a larger shape in pieces, where other is sourceRec, and reference/subReference are the total rectangle and piece rectangle on screen
// This is similar to solving for X when you have two equivalent fractions and know 3/4 values: A/B = X/C => X = (A * C)/B
PEXPI rec RelativeRec(rec reference, rec subReference, rec other)
{
	return MakeRec(
		other.x + other.width * InverseLerpR32(reference.x, reference.x + reference.width, subReference.x),
		other.y + other.height * InverseLerpR32(reference.y, reference.y + reference.height, subReference.y),
		other.width * InverseLerpR32(0, reference.width, subReference.width),
		other.height * InverseLerpR32(0, reference.height, subReference.height)
	);
}
PEXPI recd RelativeRecd(recd reference, recd subReference, recd other)
{
	return MakeRecd(
		other.x + other.width * InverseLerpR64(reference.x, reference.x + reference.width, subReference.x),
		other.y + other.height * InverseLerpR64(reference.y, reference.y + reference.height, subReference.y),
		other.width * InverseLerpR64(0, reference.width, subReference.width),
		other.height * InverseLerpR64(0, reference.height, subReference.height)
	);
}
PEXPI box RelativeBox(box reference, box subReference, box other)
{
	return MakeBox(
		other.x + other.width * InverseLerpR32(reference.x, reference.x + reference.width, subReference.x),
		other.y + other.height * InverseLerpR32(reference.y, reference.y + reference.height, subReference.y),
		other.z + other.depth * InverseLerpR32(reference.z, reference.z + reference.depth, subReference.z),
		other.width * InverseLerpR32(0, reference.width, subReference.width),
		other.height * InverseLerpR32(0, reference.height, subReference.height),
		other.depth * InverseLerpR32(0, reference.depth, subReference.depth)
	);
}
PEXPI boxd RelativeBoxd(boxd reference, boxd subReference, boxd other)
{
	return MakeBoxd(
		other.x + other.width * InverseLerpR64(reference.x, reference.x + reference.width, subReference.x),
		other.y + other.height * InverseLerpR64(reference.y, reference.y + reference.height, subReference.y),
		other.z + other.depth * InverseLerpR64(reference.z, reference.z + reference.depth, subReference.z),
		other.width * InverseLerpR64(0, reference.width, subReference.width),
		other.height * InverseLerpR64(0, reference.height, subReference.height),
		other.depth * InverseLerpR64(0, reference.depth, subReference.depth)
	);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_RECTANGLES_H

#if defined(_MISC_PARSING_H) && defined(_STRUCT_RECTANGLES_H)
#include "cross/cross_parsing_and_rectangles.h"
#endif

#if defined(_STRUCT_VECTORS_H) && defined(_STRUCT_QUATERNION_H) && defined(_STRUCT_MATRICES_H) && defined(_STRUCT_RECTANGLES_H)
#include "cross/cross_vectors_quaternion_matrices_and_rectangles.h"
#endif
