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
#include "struct/struct_vectors.h"
#include "struct/struct_quaternion.h"

//NOTE: "Rectangle" conflicts with a windows.h type so we call it Rectanglef.
//      This is mostly fine because we usually use the shortened alias "rec"
typedef union Rectanglef Rectanglef;
union Rectanglef
{
	r32 Elements[4];
	struct { r32 X, Y, Width, Height; };
	struct { r32 Left, Top, Unused1, Unused2; };
	struct { v2 TopLeft, Size; };
};
#if LANGUAGE_IS_C
_Static_assert(sizeof(Rectanglef) == sizeof(r32) * 4, "Rectanglef contains padding!");
#endif

typedef union Rectanglei Rectanglei;
union Rectanglei
{
	i32 Elements[4];
	struct { i32 X, Y, Width, Height; };
	struct { i32 Left, Top, Unused1, Unused2; };
	struct { v2i TopLeft, Size; };
};
#if LANGUAGE_IS_C
_Static_assert(sizeof(Rectanglei) == sizeof(i32) * 4, "Rectanglei contains padding!");
#endif

typedef union Box Box;
union Box
{
	r32 Elements[6];
	struct { r32 X, Y, Z, Width, Height, Depth; };
	struct { r32 Left, Bottom, Back, Unused1, Unused2, Unused3; };
	struct { v3 BottomLeftBack, Size; };
};
#if LANGUAGE_IS_C
_Static_assert(sizeof(Box) == sizeof(r32) * 6, "Box contains padding!");
#endif

typedef union Boxi Boxi;
union Boxi
{
	i32 Elements[6];
	struct { i32 X, Y, Z, Width, Height, Depth; };
	struct { i32 Left, Bottom, Back, Unused1, Unused2, Unused3; };
	struct { v3i BottomLeftBack, Size; };
};
#if LANGUAGE_IS_C
_Static_assert(sizeof(Boxi) == sizeof(i32) * 6, "Boxi contains padding!");
#endif

typedef union Obb2D Obb2D;
union Obb2D
{
	r32 Elements[5];
	struct { r32 X, Y, Width, Height, Rotation; };
	struct { r32 CenterX, CenterY, Unused1, Unused2, Angle; };
	struct { v2 Center, Size; r32 Unused3; };
};
#if LANGUAGE_IS_C
_Static_assert(sizeof(Obb2D) == sizeof(r32) * 5, "Obb2D contains padding!");
#endif

//NOTE: quat is HMM_Vec4 which has an SSE intrinsic that forces it to be 16-byte aligned
//      thus we need a of padding between center/size and rotation
typedef union Obb3D Obb3D;
union Obb3D
{
	r32 Elements[12]; //indices 8 and 9 are padding
	struct { r32 X, Y, Z, Width, Height, Depth, Padding1, Padding2; quat Rotation; };
	struct { r32 CenterX, CenterY, CenterZ, Unused1, Unused2, Unused3, Padding3, Padding4; quat Unused4; };
	struct { v3 Center, Size; r32 Padding5, Padding6; quat Unused5; };
};
#if LANGUAGE_IS_C
_Static_assert(sizeof(Obb3D) == sizeof(r32) * 12, "Obb3D contains padding!");
#endif

typedef Rectanglef rec;
typedef Rectanglei reci;
typedef Box box;
typedef Boxi boxi;
typedef Obb2D obb2;
typedef Obb3D obb3;

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE rec NewRec(r32 x, r32 y, r32 width, r32 height);
	PIG_CORE_INLINE rec NewRecV(v2 topLeft, v2 size);
	PIG_CORE_INLINE rec NewRecCentered(r32 centerX, r32 centerY, r32 width, r32 height);
	PIG_CORE_INLINE rec NewRecCenteredV(v2 center, v2 size);
	PIG_CORE_INLINE rec NewRecBetween(r32 left, r32 top, r32 right, r32 bottom);
	PIG_CORE_INLINE rec NewRecBetweenV(v2 topLeft, v2 bottomRight);
	PIG_CORE_INLINE reci NewReci(i32 x, i32 y, i32 width, i32 height);
	PIG_CORE_INLINE reci NewReciV(v2i topLeft, v2i size);
	PIG_CORE_INLINE reci NewReciCentered(i32 centerX, i32 centerY, i32 width, i32 height);
	PIG_CORE_INLINE reci NewReciCenteredV(v2i center, v2i size);
	PIG_CORE_INLINE reci NewReciBetween(i32 left, i32 top, i32 right, i32 bottom);
	PIG_CORE_INLINE reci NewReciBetweenV(v2i topLeft, v2i bottomRight);
	PIG_CORE_INLINE box NewBox(r32 x, r32 y, r32 z, r32 width, r32 height, r32 depth);
	PIG_CORE_INLINE box NewBoxV(v3 bottomLeftBack, v3 size);
	PIG_CORE_INLINE box NewBoxCentered(r32 centerX, r32 centerY, r32 centerZ, r32 width, r32 height, r32 depth);
	PIG_CORE_INLINE box NewBoxCenteredV(v3 center, v3 size);
	PIG_CORE_INLINE box NewBoxBetween(r32 left, r32 bottom, r32 back, r32 right, r32 top, r32 front);
	PIG_CORE_INLINE box NewBoxBetweenV(v3 bottomLeftBack, v3 topRightFront);
	PIG_CORE_INLINE boxi NewBoxi(i32 x, i32 y, i32 z, i32 width, i32 height, i32 depth);
	PIG_CORE_INLINE boxi NewBoxiV(v3i bottomLeftBack, v3i size);
	PIG_CORE_INLINE boxi NewBoxiCentered(i32 centerX, i32 centerY, i32 centerZ, i32 width, i32 height, i32 depth);
	PIG_CORE_INLINE boxi NewBoxiCenteredV(v3i center, v3i size);
	PIG_CORE_INLINE boxi NewBoxiBetween(i32 left, i32 bottom, i32 back, i32 right, i32 top, i32 front);
	PIG_CORE_INLINE boxi NewBoxiBetweenV(v3i bottomLeftBack, v3i topRightFront);
	PIG_CORE_INLINE obb2 NewObb2(r32 x, r32 y, r32 width, r32 height, r32 rotation);
	PIG_CORE_INLINE obb2 NewObb2V(v2 center, v2 size, r32 rotation);
	PIG_CORE_INLINE obb3 NewObb3(r32 x, r32 y, r32 z, r32 width, r32 height, r32 depth, quat rotation);
	PIG_CORE_INLINE obb3 NewObb3V(v3 center, v3 size, quat rotation);
	PIG_CORE_INLINE rec ToRecFromi(reci rectangle);
	PIG_CORE_INLINE reci ToReciFromf(rec rectangle);
	PIG_CORE_INLINE rec ToRecFromV4(v4 vector);
	PIG_CORE_INLINE v4 ToV4FromRec(rec rectangle);
	PIG_CORE_INLINE v4r ToV4rFromRec(rec rectangle);
	PIG_CORE_INLINE box ToBoxFromi(boxi boundingBox);
	PIG_CORE_INLINE boxi ToBoxiFromf(box boundingBox);
	PIG_CORE_INLINE obb2 ToObb2FromRec(rec rectangle);
	PIG_CORE_INLINE obb3 ToObb3FromBox(box boundingBox);
	#if TARGET_IS_ORCA
	PIG_CORE_INLINE oc_rect ToOcRect(rec rectangle);
	PIG_CORE_INLINE rec ToRecFromOc(oc_rect orcaRectangle);
	#endif //TARGET_IS_ORCA
	PIG_CORE_INLINE rec AddRec(rec rectangle, v2 offset);
	PIG_CORE_INLINE reci AddReci(reci rectangle, v2i offset);
	PIG_CORE_INLINE box AddBox(box boundingBox, v3 offset);
	PIG_CORE_INLINE boxi AddBoxi(boxi boundingBox, v3i offset);
	PIG_CORE_INLINE obb2 AddObb2(obb2 boundingBox, v2 offset);
	PIG_CORE_INLINE obb3 AddObb3(obb3 boundingBox, v3 offset);
	PIG_CORE_INLINE rec SubRec(rec rectangle, v2 offset);
	PIG_CORE_INLINE reci SubReci(reci rectangle, v2i offset);
	PIG_CORE_INLINE box SubBox(box boundingBox, v3 offset);
	PIG_CORE_INLINE boxi SubBoxi(boxi boundingBox, v3i offset);
	PIG_CORE_INLINE obb2 SubObb2(obb2 boundingBox, v2 offset);
	PIG_CORE_INLINE obb3 SubObb3(obb3 boundingBox, v3 offset);
	PIG_CORE_INLINE rec MulRec(rec rectangle, v2 scale);
	PIG_CORE_INLINE reci MulReci(reci rectangle, v2i scale);
	PIG_CORE_INLINE box MulBox(box boundingBox, v3 scale);
	PIG_CORE_INLINE boxi MulBoxi(boxi boundingBox, v3i scale);
	PIG_CORE_INLINE obb2 MulObb2(obb2 boundingBox, v2 scale);
	PIG_CORE_INLINE obb3 MulObb3(obb3 boundingBox, v3 scale);
	PIG_CORE_INLINE rec DivRec(rec rectangle, v2 divisor);
	PIG_CORE_INLINE reci DivReci(reci rectangle, v2i divisor);
	PIG_CORE_INLINE box DivBox(box boundingBox, v3 divisor);
	PIG_CORE_INLINE boxi DivBoxi(boxi boundingBox, v3i divisor);
	PIG_CORE_INLINE obb2 DivObb2(obb2 boundingBox, v2 divisor);
	PIG_CORE_INLINE obb3 DivObb3(obb3 boundingBox, v3 divisor);
	PIG_CORE_INLINE rec ScaleRec(rec rectangle, r32 scalar);
	PIG_CORE_INLINE reci ScaleReci(reci rectangle, i32 scalar);
	PIG_CORE_INLINE box ScaleBox(box boundingBox, r32 scalar);
	PIG_CORE_INLINE boxi ScaleBoxi(boxi boundingBox, i32 scalar);
	PIG_CORE_INLINE obb2 ScaleObb2(obb2 boundingBox, r32 scalar);
	PIG_CORE_INLINE obb3 ScaleObb3(obb3 boundingBox, r32 scalar);
	PIG_CORE_INLINE rec ShrinkRec(rec rectangle, r32 divisor);
	PIG_CORE_INLINE reci ShrinkReci(reci rectangle, i32 divisor);
	PIG_CORE_INLINE box ShrinkBox(box boundingBox, r32 divisor);
	PIG_CORE_INLINE boxi ShrinkBoxi(boxi boundingBox, i32 divisor);
	PIG_CORE_INLINE obb2 ShrinkObb2(obb2 boundingBox, r32 divisor);
	PIG_CORE_INLINE obb3 ShrinkObb3(obb3 boundingBox, r32 divisor);
	PIG_CORE_INLINE bool AreEqualRec(rec left, rec right);
	PIG_CORE_INLINE bool AreEqualReci(reci left, reci right);
	PIG_CORE_INLINE bool AreEqualBox(box left, box right);
	PIG_CORE_INLINE bool AreEqualBoxi(boxi left, boxi right);
	PIG_CORE_INLINE bool AreEqualObb2(obb2 left, obb2 right);
	PIG_CORE_INLINE bool AreEqualObb3(obb3 left, obb3 right);
	PIG_CORE_INLINE rec InflateRecEx(rec rectangle, r32 leftAmount, r32 rightAmount, r32 topAmount, r32 bottomAmount);
	PIG_CORE_INLINE rec InflateRec(rec rectangle, r32 amount);
	PIG_CORE_INLINE rec InflateRecX(rec rectangle, r32 amount);
	PIG_CORE_INLINE rec InflateRecY(rec rectangle, r32 amount);
	PIG_CORE_INLINE reci InflateReciEx(reci rectangle, i32 leftAmount, i32 rightAmount, i32 topAmount, i32 bottomAmount);
	PIG_CORE_INLINE reci InflateReci(reci rectangle, i32 amount);
	PIG_CORE_INLINE reci InflateReciX(reci rectangle, i32 amount);
	PIG_CORE_INLINE reci InflateReciY(reci rectangle, i32 amount);
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
	PIG_CORE_INLINE rec BothRec(rec left, rec right);
	PIG_CORE_INLINE reci BothReci(reci left, reci right);
	PIG_CORE_INLINE box BothBox(box left, box right);
	PIG_CORE_INLINE boxi BothBoxi(boxi left, boxi right);
	PIG_CORE_INLINE rec OverlapPartRec(rec left, rec right);
	PIG_CORE_INLINE reci OverlapPartReci(reci left, reci right);
	PIG_CORE_INLINE box OverlapPartBox(box left, box right);
	PIG_CORE_INLINE boxi OverlapPartBoxi(boxi left, boxi right);
	PIG_CORE_INLINE bool IsInsideRec(rec rectangle, v2 position);
	PIG_CORE_INLINE bool IsInsideRecInclusive(rec rectangle, v2 position);
	PIG_CORE_INLINE rec RelativeRec(rec reference, rec subReference, rec other);
	PIG_CORE_INLINE box RelativeBox(box reference, box subReference, box other);
	PIG_CORE_INLINE void AlignRecToV2(rec* rectangleOut, v2 alignmentScale);
	PIG_CORE_INLINE void AlignRecTo(rec* rectangleOut, r32 alignmentScale);
	PIG_CORE_INLINE void AlignRec(rec* rectangleOut);
	PIG_CORE_INLINE void AlignBoxToV3(box* boundingBoxOut, v3 alignmentScale);
	PIG_CORE_INLINE void AlignBoxTo(box* boundingBoxOut, r32 alignmentScale);
	PIG_CORE_INLINE void AlignBox(box* boundingBoxOut);
#endif

// +--------------------------------------------------------------+
// |                   Simple Value Definitions                   |
// +--------------------------------------------------------------+
#define Rec_Zero NewRec(0.0f, 0.0f, 0.0f, 0.0f)
#define Rec_Default NewRec(0.0f, 0.0f, 1.0f, 1.0f)
#define Rec_UnitRec NewRec(-1.0f, -1.0f, 2.0f, 2.0f)
#define Rec_Zero_Const (rec){{ 0.0f, 0.0f, 0.0f, 0.0f }}
#define Rec_Default_Const (rec){{ 0.0f, 0.0f, 1.0f, 1.0f }}
#define Rec_UnitRec_Const (rec){{ -1.0f, -1.0f, 2.0f, 2.0f }}

#define Reci_Zero NewReci(0, 0, 0, 0)
#define Reci_Default NewReci(0, 0, 1, 1)
#define Reci_UnitRec NewReci(-1, -1, 2, 2)
#define Reci_Zero_Const (reci){{ 0, 0, 0, 0 }}
#define Reci_Default_Const (reci){{ 0, 0, 1, 1 }}
#define Reci_UnitRec_Const (reci){{ -1, -1, 2, 2 }}

#define Box_Zero NewBox(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)
#define Box_Default NewBox(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f)
#define Box_UnitBox NewBox(-1.0f, -1.0f, -1.0f, 2.0f, 2.0f, 2.0f)
#define Box_Zero_Const (box){ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }
#define Box_Default_Const (box){{ 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f }}
#define Box_UnitBox_Const (box){{ -1.0f, -1.0f, -1.0f, 2.0f, 2.0f, 2.0f }}

#define Boxi_Zero NewBoxi(0, 0, 0, 0, 0, 0)
#define Boxi_Default NewBoxi(0, 0, 0, 1, 1, 1)
#define Boxi_UnitBox NewBoxi(-1, -1, -1, 2, 2, 2)
#define Boxi_Zero_Const (boxi){{ 0, 0, 0, 0, 0, 0 }}
#define Boxi_Default_Const (boxi){{ 0, 0, 0, 1, 1, 1 }}
#define Boxi_UnitBox_Const (boxi){{ -1, -1, -1, 2, 2, 2 }}

#define Obb2_Zero NewObb2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f)
#define Obb2_Default NewObb2(0.5f, 0.5f, 1.0f, 1.0f, 0.0f)
#define Obb2_UnitBox NewObb2(0.0f, 0.0f, 2.0f, 2.0f, 0.0f)
#define Obb2_Zero_Const (obb2){{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}
#define Obb2_Default_Const (obb2){{ 0.5f, 0.5f, 1.0f, 1.0f, 0.0f }}
#define Obb2_UnitBox_Const (obb2){{ 0.0f, 0.0f, 2.0f, 2.0f, 0.0f }}

#define Obb3_Zero NewObb3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, Quat_Identity)
#define Obb3_Default NewObb3(0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, Quat_Identity)
#define Obb3_UnitBox NewObb3(0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 2.0f, Quat_Identity)
//TODO: For some reason these curly brack initializations only work if we have name designations?
#define Obb3_Zero_Const (obb3){ .X=0.0f, .Y=0.0f, .Z=0.0f, .Width=0.0f, .Height=0.0f, .Depth=0.0f, .Padding1=0.0f, .Padding2=0.0f, .Rotation=Quat_Identity_Const }
#define Obb3_Default_Const (obb3){ .X=0.5f, .Y=0.5f, .Z=0.5f, .Width=1.0f, .Height=1.0f, .Depth=1.0f, .Padding1=0.0f, .Padding2=0.0f, .Rotation=Quat_Identity_Const }
#define Obb3_UnitBox_Const (obb3){ .X=0.0f, .Y=0.0f, .Z=0.0f, .Width=2.0f, .Height=2.0f, .Depth=2.0f, .Padding1=0.0f, .Padding2=0.0f, .Rotation=Quat_Identity_Const }

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI rec NewRec(r32 x, r32 y, r32 width, r32 height)
{
	rec result;
	result.X = x;
	result.Y = y;
	result.Width = width;
	result.Height = height;
	return result;
}
PEXPI rec NewRecV(v2 topLeft, v2 size)
{
	rec result;
	result.TopLeft = topLeft;
	result.Size = size;
	return result;
}
PEXPI rec NewRecCentered(r32 centerX, r32 centerY, r32 width, r32 height)
{
	rec result;
	result.X = centerX - width/2.0f;
	result.Y = centerY - height/2.0f;
	result.Width = width;
	result.Height = height;
	return result;
}
PEXPI rec NewRecCenteredV(v2 center, v2 size)
{
	rec result;
	result.TopLeft = SubV2(center, ShrinkV2(size, 2.0f));
	result.Size = size;
	return result;
}
PEXPI rec NewRecBetween(r32 left, r32 top, r32 right, r32 bottom)
{
	rec result;
	result.X = left;
	result.Y = top;
	result.Width = right - left;
	result.Height = bottom - top;
	return result;
}
PEXPI rec NewRecBetweenV(v2 topLeft, v2 bottomRight)
{
	rec result;
	result.TopLeft = topLeft;
	result.Size = SubV2(bottomRight, topLeft);
	return result;
}

PEXPI reci NewReci(i32 x, i32 y, i32 width, i32 height)
{
	reci result;
	result.X = x;
	result.Y = y;
	result.Width = width;
	result.Height = height;
	return result;
}
PEXPI reci NewReciV(v2i topLeft, v2i size)
{
	reci result;
	result.TopLeft = topLeft;
	result.Size = size;
	return result;
}
//NOTE: This only works as expected if width/height are even numbers, since we can't center a uneven size rectangle around a integer coordinate
PEXPI reci NewReciCentered(i32 centerX, i32 centerY, i32 width, i32 height)
{
	reci result;
	result.X = centerX - width/2;
	result.Y = centerY - height/2;
	result.Width = width;
	result.Height = height;
	return result;
}
PEXPI reci NewReciCenteredV(v2i center, v2i size)
{
	reci result;
	result.TopLeft = SubV2i(center, ShrinkV2i(size, 2));
	result.Size = size;
	return result;
}
PEXPI reci NewReciBetween(i32 left, i32 top, i32 right, i32 bottom)
{
	reci result;
	result.X = left;
	result.Y = top;
	result.Width = right - left;
	result.Height = bottom - top;
	return result;
}
PEXPI reci NewReciBetweenV(v2i topLeft, v2i bottomRight)
{
	reci result;
	result.TopLeft = topLeft;
	result.Size = SubV2i(bottomRight, topLeft);
	return result;
}

PEXPI box NewBox(r32 x, r32 y, r32 z, r32 width, r32 height, r32 depth)
{
	box result;
	result.X = x;
	result.Y = y;
	result.Z = z;
	result.Width = width;
	result.Height = height;
	result.Depth = depth;
	return result;
}
PEXPI box NewBoxV(v3 bottomLeftBack, v3 size)
{
	box result;
	result.BottomLeftBack = bottomLeftBack;
	result.Size = size;
	return result;
}
PEXPI box NewBoxCentered(r32 centerX, r32 centerY, r32 centerZ, r32 width, r32 height, r32 depth)
{
	box result;
	result.X = centerX - width/2.0f;
	result.Y = centerY - height/2.0f;
	result.Z = centerZ - depth/2.0f;
	result.Width = width;
	result.Height = height;
	result.Depth = depth;
	return result;
}
PEXPI box NewBoxCenteredV(v3 center, v3 size)
{
	box result;
	result.BottomLeftBack = SubV3(center, ShrinkV3(size, 2.0f));
	result.Size = size;
	return result;
}
PEXPI box NewBoxBetween(r32 left, r32 bottom, r32 back, r32 right, r32 top, r32 front)
{
	box result;
	result.X = left;
	result.Y = bottom;
	result.Z = back;
	result.Width = right - left;
	result.Height = top - bottom;
	result.Depth = front - back;
	return result;
}
PEXPI box NewBoxBetweenV(v3 bottomLeftBack, v3 topRightFront)
{
	box result;
	result.BottomLeftBack = bottomLeftBack;
	result.Size = SubV3(topRightFront, bottomLeftBack);
	return result;
}

PEXPI boxi NewBoxi(i32 x, i32 y, i32 z, i32 width, i32 height, i32 depth)
{
	boxi result;
	result.X = x;
	result.Y = y;
	result.Z = z;
	result.Width = width;
	result.Height = height;
	result.Depth = depth;
	return result;
}
PEXPI boxi NewBoxiV(v3i bottomLeftBack, v3i size)
{
	boxi result;
	result.BottomLeftBack = bottomLeftBack;
	result.Size = size;
	return result;
}
//NOTE: This only works as expected if width/height/depth are even numbers, since we can't center a uneven size box around a integer coordinate
PEXPI boxi NewBoxiCentered(i32 centerX, i32 centerY, i32 centerZ, i32 width, i32 height, i32 depth)
{
	boxi result;
	result.X = centerX - width/2;
	result.Y = centerY - height/2;
	result.Z = centerZ - depth/2;
	result.Width = width;
	result.Height = height;
	result.Depth = depth;
	return result;
}
PEXPI boxi NewBoxiCenteredV(v3i center, v3i size)
{
	boxi result;
	result.BottomLeftBack = SubV3i(center, ShrinkV3i(size, 2));
	result.Size = size;
	return result;
}
PEXPI boxi NewBoxiBetween(i32 left, i32 bottom, i32 back, i32 right, i32 top, i32 front)
{
	boxi result;
	result.X = left;
	result.Y = bottom;
	result.Z = back;
	result.Width = right - left;
	result.Height = top - bottom;
	result.Depth = front - back;
	return result;
}
PEXPI boxi NewBoxiBetweenV(v3i bottomLeftBack, v3i topRightFront)
{
	boxi result;
	result.BottomLeftBack = bottomLeftBack;
	result.Size = SubV3i(topRightFront, bottomLeftBack);
	return result;
}

PEXPI obb2 NewObb2(r32 x, r32 y, r32 width, r32 height, r32 rotation)
{
	obb2 result;
	result.X = x;
	result.Y = y;
	result.Width = width;
	result.Height = height;
	result.Rotation = rotation;
	return result;
}
PEXPI obb2 NewObb2V(v2 center, v2 size, r32 rotation)
{
	obb2 result;
	result.Center = center;
	result.Size = size;
	result.Rotation = rotation;
	return result;
}

PEXPI obb3 NewObb3(r32 x, r32 y, r32 z, r32 width, r32 height, r32 depth, quat rotation)
{
	obb3 result;
	result.X = x;
	result.Y = y;
	result.Z = z;
	result.Width = width;
	result.Height = height;
	result.Depth = depth;
	result.Rotation = rotation;
	return result;
}
PEXPI obb3 NewObb3V(v3 center, v3 size, quat rotation)
{
	obb3 result;
	result.Center = center;
	result.Size = size;
	result.Rotation = rotation;
	return result;
}

// +--------------------------------------------------------------+
// |                 Simple Conversions and Casts                 |
// +--------------------------------------------------------------+
PEXPI rec ToRecFromi(reci rectangle) { return NewRec((r32)rectangle.X, (r32)rectangle.Y, (r32)rectangle.Width, (r32)rectangle.Height); }
PEXPI reci ToReciFromf(rec rectangle) { return NewReci((i32)rectangle.X, (i32)rectangle.Y, (i32)rectangle.Width, (i32)rectangle.Height); }
PEXPI rec ToRecFromV4(v4 vector) { return NewRec(vector.X, vector.Y, vector.Z, vector.W); }
PEXPI v4 ToV4FromRec(rec rectangle) { return NewV4(rectangle.X, rectangle.Y, rectangle.Width, rectangle.Height); }
PEXPI v4r ToV4rFromRec(rec rectangle) { return NewV4r(rectangle.X, rectangle.Y, rectangle.Width, rectangle.Height); }
PEXPI box ToBoxFromi(boxi boundingBox) { return NewBox((r32)boundingBox.X, (r32)boundingBox.Y, (r32)boundingBox.Z, (r32)boundingBox.Width, (r32)boundingBox.Height, (r32)boundingBox.Depth); }
PEXPI boxi ToBoxiFromf(box boundingBox) { return NewBoxi((i32)boundingBox.X, (i32)boundingBox.Y, (i32)boundingBox.Z, (i32)boundingBox.Width, (i32)boundingBox.Height, (i32)boundingBox.Depth); }
PEXPI obb2 ToObb2FromRec(rec rectangle) { return NewObb2(rectangle.X + rectangle.Width/2.0f, rectangle.Y + rectangle.Height/2.0f, rectangle.Width, rectangle.Height, 0.0f); }
PEXPI obb3 ToObb3FromBox(box boundingBox) { return NewObb3(boundingBox.X + boundingBox.Width/2.0f, boundingBox.Y + boundingBox.Height/2.0f, boundingBox.Z + boundingBox.Depth/2.0f, boundingBox.Width, boundingBox.Height, boundingBox.Depth, Quat_Identity); }

#if TARGET_IS_ORCA
PEXPI oc_rect ToOcRect(rec rectangle) { return NEW_STRUCT(oc_rect){ .x = rectangle.X, .y = rectangle.Y, .w = rectangle.Width, .h = rectangle.Height }; }
PEXPI rec ToRecFromOc(oc_rect orcaRectangle) { return NewRec(orcaRectangle.x, orcaRectangle.y, orcaRectangle.w, orcaRectangle.h); }
#endif //TARGET_IS_ORCA

// +--------------------------------------------------------------+
// |                Operator Overload Equivalents                 |
// +--------------------------------------------------------------+
PEXPI rec AddRec(rec rectangle, v2 offset) { return NewRec(rectangle.X + offset.X, rectangle.Y + offset.Y, rectangle.Width, rectangle.Height); }
PEXPI reci AddReci(reci rectangle, v2i offset) { return NewReci(rectangle.X + offset.X, rectangle.Y + offset.Y, rectangle.Width, rectangle.Height); }
PEXPI box AddBox(box boundingBox, v3 offset) { return NewBox(boundingBox.X + offset.X, boundingBox.Y + offset.Y, boundingBox.Z + offset.Z, boundingBox.Width, boundingBox.Height, boundingBox.Depth); }
PEXPI boxi AddBoxi(boxi boundingBox, v3i offset) { return NewBoxi(boundingBox.X + offset.X, boundingBox.Y + offset.Y, boundingBox.Z + offset.Z, boundingBox.Width, boundingBox.Height, boundingBox.Depth); }
PEXPI obb2 AddObb2(obb2 boundingBox, v2 offset) { return NewObb2(boundingBox.X + offset.X, boundingBox.Y + offset.Y, boundingBox.Width, boundingBox.Height, boundingBox.Rotation); }
PEXPI obb3 AddObb3(obb3 boundingBox, v3 offset) { return NewObb3(boundingBox.X + offset.X, boundingBox.Y + offset.Y, boundingBox.Z + offset.Z, boundingBox.Width, boundingBox.Height, boundingBox.Depth, boundingBox.Rotation); }

PEXPI rec SubRec(rec rectangle, v2 offset) { return NewRec(rectangle.X - offset.X, rectangle.Y - offset.Y, rectangle.Width, rectangle.Height); }
PEXPI reci SubReci(reci rectangle, v2i offset) { return NewReci(rectangle.X - offset.X, rectangle.Y - offset.Y, rectangle.Width, rectangle.Height); }
PEXPI box SubBox(box boundingBox, v3 offset) { return NewBox(boundingBox.X - offset.X, boundingBox.Y - offset.Y, boundingBox.Z - offset.Z, boundingBox.Width, boundingBox.Height, boundingBox.Depth); }
PEXPI boxi SubBoxi(boxi boundingBox, v3i offset) { return NewBoxi(boundingBox.X - offset.X, boundingBox.Y - offset.Y, boundingBox.Z - offset.Z, boundingBox.Width, boundingBox.Height, boundingBox.Depth); }
PEXPI obb2 SubObb2(obb2 boundingBox, v2 offset) { return NewObb2(boundingBox.X - offset.X, boundingBox.Y - offset.Y, boundingBox.Width, boundingBox.Height, boundingBox.Rotation); }
PEXPI obb3 SubObb3(obb3 boundingBox, v3 offset) { return NewObb3(boundingBox.X - offset.X, boundingBox.Y - offset.Y, boundingBox.Z - offset.Z, boundingBox.Width, boundingBox.Height, boundingBox.Depth, boundingBox.Rotation); }

PEXPI rec MulRec(rec rectangle, v2 scale) { return NewRec(rectangle.X * scale.X, rectangle.Y * scale.Y, rectangle.Width * scale.X, rectangle.Height * scale.Y); }
PEXPI reci MulReci(reci rectangle, v2i scale) { return NewReci(rectangle.X * scale.X, rectangle.Y * scale.Y, rectangle.Width * scale.X, rectangle.Height * scale.Y); }
PEXPI box MulBox(box boundingBox, v3 scale) { return NewBox(boundingBox.X * scale.X, boundingBox.Y * scale.Y, boundingBox.Z * scale.Z, boundingBox.Width * scale.X, boundingBox.Height * scale.Y, boundingBox.Depth * scale.Z); }
PEXPI boxi MulBoxi(boxi boundingBox, v3i scale) { return NewBoxi(boundingBox.X * scale.X, boundingBox.Y * scale.Y, boundingBox.Z * scale.Z, boundingBox.Width * scale.X, boundingBox.Height * scale.Y, boundingBox.Depth * scale.Z); }
PEXPI obb2 MulObb2(obb2 boundingBox, v2 scale) { return NewObb2(boundingBox.X * scale.X, boundingBox.Y * scale.Y, boundingBox.Width * scale.X, boundingBox.Height * scale.Y, boundingBox.Rotation); }
PEXPI obb3 MulObb3(obb3 boundingBox, v3 scale) { return NewObb3(boundingBox.X * scale.X, boundingBox.Y * scale.Y, boundingBox.Z * scale.Z, boundingBox.Width * scale.X, boundingBox.Height * scale.Y, boundingBox.Depth * scale.Z, boundingBox.Rotation); }

PEXPI rec DivRec(rec rectangle, v2 divisor) { return NewRec(rectangle.X / divisor.X, rectangle.Y / divisor.Y, rectangle.Width / divisor.X, rectangle.Height / divisor.Y); }
PEXPI reci DivReci(reci rectangle, v2i divisor) { return NewReci(rectangle.X / divisor.X, rectangle.Y / divisor.Y, rectangle.Width / divisor.X, rectangle.Height / divisor.Y); }
PEXPI box DivBox(box boundingBox, v3 divisor) { return NewBox(boundingBox.X / divisor.X, boundingBox.Y / divisor.Y, boundingBox.Z / divisor.Z, boundingBox.Width / divisor.X, boundingBox.Height / divisor.Y, boundingBox.Depth / divisor.Z); }
PEXPI boxi DivBoxi(boxi boundingBox, v3i divisor) { return NewBoxi(boundingBox.X / divisor.X, boundingBox.Y / divisor.Y, boundingBox.Z / divisor.Z, boundingBox.Width / divisor.X, boundingBox.Height / divisor.Y, boundingBox.Depth / divisor.Z); }
PEXPI obb2 DivObb2(obb2 boundingBox, v2 divisor) { return NewObb2(boundingBox.X / divisor.X, boundingBox.Y / divisor.Y, boundingBox.Width / divisor.X, boundingBox.Height / divisor.Y, boundingBox.Rotation); }
PEXPI obb3 DivObb3(obb3 boundingBox, v3 divisor) { return NewObb3(boundingBox.X / divisor.X, boundingBox.Y / divisor.Y, boundingBox.Z / divisor.Z, boundingBox.Width / divisor.X, boundingBox.Height / divisor.Y, boundingBox.Depth / divisor.Z, boundingBox.Rotation); }

PEXPI rec ScaleRec(rec rectangle, r32 scalar) { return NewRec(rectangle.X * scalar, rectangle.Y * scalar, rectangle.Width * scalar, rectangle.Height * scalar); }
PEXPI reci ScaleReci(reci rectangle, i32 scalar) { return NewReci(rectangle.X * scalar, rectangle.Y * scalar, rectangle.Width * scalar, rectangle.Height * scalar); }
PEXPI box ScaleBox(box boundingBox, r32 scalar) { return NewBox(boundingBox.X * scalar, boundingBox.Y * scalar, boundingBox.Z * scalar, boundingBox.Width * scalar, boundingBox.Height * scalar, boundingBox.Depth * scalar); }
PEXPI boxi ScaleBoxi(boxi boundingBox, i32 scalar) { return NewBoxi(boundingBox.X * scalar, boundingBox.Y * scalar, boundingBox.Z * scalar, boundingBox.Width * scalar, boundingBox.Height * scalar, boundingBox.Depth * scalar); }
PEXPI obb2 ScaleObb2(obb2 boundingBox, r32 scalar) { return NewObb2(boundingBox.X * scalar, boundingBox.Y * scalar, boundingBox.Width * scalar, boundingBox.Height * scalar, boundingBox.Rotation); }
PEXPI obb3 ScaleObb3(obb3 boundingBox, r32 scalar) { return NewObb3(boundingBox.X * scalar, boundingBox.Y * scalar, boundingBox.Z * scalar, boundingBox.Width * scalar, boundingBox.Height * scalar, boundingBox.Depth * scalar, boundingBox.Rotation); }

PEXPI rec ShrinkRec(rec rectangle, r32 divisor) { return NewRec(rectangle.X / divisor, rectangle.Y / divisor, rectangle.Width / divisor, rectangle.Height / divisor); }
PEXPI reci ShrinkReci(reci rectangle, i32 divisor) { return NewReci(rectangle.X / divisor, rectangle.Y / divisor, rectangle.Width / divisor, rectangle.Height / divisor); }
PEXPI box ShrinkBox(box boundingBox, r32 divisor) { return NewBox(boundingBox.X / divisor, boundingBox.Y / divisor, boundingBox.Z / divisor, boundingBox.Width / divisor, boundingBox.Height / divisor, boundingBox.Depth / divisor); }
PEXPI boxi ShrinkBoxi(boxi boundingBox, i32 divisor) { return NewBoxi(boundingBox.X / divisor, boundingBox.Y / divisor, boundingBox.Z / divisor, boundingBox.Width / divisor, boundingBox.Height / divisor, boundingBox.Depth / divisor); }
PEXPI obb2 ShrinkObb2(obb2 boundingBox, r32 divisor) { return NewObb2(boundingBox.X / divisor, boundingBox.Y / divisor, boundingBox.Width / divisor, boundingBox.Height / divisor, boundingBox.Rotation); }
PEXPI obb3 ShrinkObb3(obb3 boundingBox, r32 divisor) { return NewObb3(boundingBox.X / divisor, boundingBox.Y / divisor, boundingBox.Z / divisor, boundingBox.Width / divisor, boundingBox.Height / divisor, boundingBox.Depth / divisor, boundingBox.Rotation); }

PEXPI bool AreEqualRec(rec left, rec right) { return (left.X == right.X && left.Y == right.Y && left.Width == right.Width && left.Height == right.Height); }
PEXPI bool AreEqualReci(reci left, reci right) { return (left.X == right.X && left.Y == right.Y && left.Width == right.Width && left.Height == right.Height); }
PEXPI bool AreEqualBox(box left, box right) { return (left.X == right.X && left.Y == right.Y && left.Z == right.Z && left.Width == right.Width && left.Height == right.Height && left.Depth == right.Depth); }
PEXPI bool AreEqualBoxi(boxi left, boxi right) { return (left.X == right.X && left.Y == right.Y && left.Z == right.Z && left.Width == right.Width && left.Height == right.Height && left.Depth == right.Depth); }
PEXPI bool AreEqualObb2(obb2 left, obb2 right) { return (left.X == right.X && left.Y == right.Y && left.Width == right.Width && left.Height == right.Height && left.Rotation == right.Rotation); }
PEXPI bool AreEqualObb3(obb3 left, obb3 right) { return (left.X == right.X && left.Y == right.Y && left.Z == right.Z && left.Width == right.Width && left.Height == right.Height && left.Depth == right.Depth && AreEqualQuat(left.Rotation, right.Rotation)); }

//TODO: AreSimilarRec?

// +--------------------------------------------------------------+
// |            Basic Manipulation and Math Functions             |
// +--------------------------------------------------------------+
//TODO: InvertRec/InvertXRec/InvertYRec?
//TODO: LerpRec?
//TODO: RotateObb2Clockwise(uxx numQuarterTurns)/RotateObb2CounterClockwise(uxx numQuarterTurns)

PEXPI rec InflateRecEx(rec rectangle, r32 leftAmount, r32 rightAmount, r32 topAmount, r32 bottomAmount) { return NewRec(rectangle.X - leftAmount, rectangle.Y - topAmount, rectangle.Width + leftAmount + rightAmount, rectangle.Height + topAmount + bottomAmount); }
PEXPI rec InflateRec(rec rectangle, r32 amount) { return InflateRecEx(rectangle, amount, amount, amount, amount); }
PEXPI rec InflateRecX(rec rectangle, r32 amount) { return InflateRecEx(rectangle, amount, amount, 0.0f, 0.0f); }
PEXPI rec InflateRecY(rec rectangle, r32 amount) { return InflateRecEx(rectangle, 0.0f, 0.0f, amount, amount); }
PEXPI reci InflateReciEx(reci rectangle, i32 leftAmount, i32 rightAmount, i32 topAmount, i32 bottomAmount) { return NewReci(rectangle.X - leftAmount, rectangle.Y - topAmount, rectangle.Width + leftAmount + rightAmount, rectangle.Height + topAmount + bottomAmount); }
PEXPI reci InflateReci(reci rectangle, i32 amount) { return InflateReciEx(rectangle, amount, amount, amount, amount); }
PEXPI reci InflateReciX(reci rectangle, i32 amount) { return InflateReciEx(rectangle, amount, amount, 0, 0); }
PEXPI reci InflateReciY(reci rectangle, i32 amount) { return InflateReciEx(rectangle, 0, 0, amount, amount); }
PEXPI box InflateBoxEx(box boundingBox, r32 leftAmount, r32 rightAmount, r32 topAmount, r32 bottomAmount, r32 backAmount, r32 frontAmount) { return NewBox(boundingBox.X - leftAmount, boundingBox.Y - bottomAmount, boundingBox.Z - backAmount, boundingBox.Width + leftAmount + rightAmount, boundingBox.Height + bottomAmount + topAmount, boundingBox.Depth + backAmount + frontAmount); }
PEXPI box InflateBox(box boundingBox, r32 amount) { return InflateBoxEx(boundingBox, amount, amount, amount, amount, amount, amount); }
PEXPI box InflateBoxX(box boundingBox, r32 amount) { return InflateBoxEx(boundingBox, amount, amount, 0.0f, 0.0f, 0.0f, 0.0f); }
PEXPI box InflateBoxY(box boundingBox, r32 amount) { return InflateBoxEx(boundingBox, 0.0f, 0.0f, amount, amount, 0.0f, 0.0f); }
PEXPI box InflateBoxZ(box boundingBox, r32 amount) { return InflateBoxEx(boundingBox, 0.0f, 0.0f, 0.0f, 0.0f, amount, amount); }
PEXPI boxi InflateBoxiEx(boxi boundingBox, i32 leftAmount, i32 rightAmount, i32 topAmount, i32 bottomAmount, i32 backAmount, i32 frontAmount) { return NewBoxi(boundingBox.X - leftAmount, boundingBox.Y - bottomAmount, boundingBox.Z - backAmount, boundingBox.Width + leftAmount + rightAmount, boundingBox.Height + bottomAmount + topAmount, boundingBox.Depth + backAmount + frontAmount); }
PEXPI boxi InflateBoxi(boxi boundingBox, i32 amount) { return InflateBoxiEx(boundingBox, amount, amount, amount, amount, amount, amount); }
PEXPI boxi InflateBoxiX(boxi boundingBox, i32 amount) { return InflateBoxiEx(boundingBox, amount, amount, 0, 0, 0, 0); }
PEXPI boxi InflateBoxiY(boxi boundingBox, i32 amount) { return InflateBoxiEx(boundingBox, 0, 0, amount, amount, 0, 0); }
PEXPI boxi InflateBoxiZ(boxi boundingBox, i32 amount) { return InflateBoxiEx(boundingBox, 0, 0, 0, 0, amount, amount); }

//TODO: DeflateRec/DeflateXRec/DeflateYRec?
//TODO: ExpandRightRec/ExpandLeftRec/ExpandUpRec/ExpandDownRec?
//TODO: RetractRightRec/RetractLeftRec/RetractUpRec/RetractDownRec?
//TODO: SquarifyRec/CubifyBox?

PEXPI rec BothRec(rec left, rec right) { r32 minX = MinR32(left.X, right.X); r32 minY = MinR32(left.Y, right.Y); return NewRec(minX, minY, MaxR32(left.X + left.Width, right.X + right.Width) - minX, MaxR32(left.Y + left.Height, right.Y + right.Height) - minY); }
PEXPI reci BothReci(reci left, reci right) { i32 minX = MinI32(left.X, right.X); i32 minY = MinI32(left.Y, right.Y); return NewReci(minX, minY, MaxI32(left.X + left.Width, right.X + right.Width) - minX, MaxI32(left.Y + left.Height, right.Y + right.Height) - minY); }
PEXPI box BothBox(box left, box right) { r32 minX = MinR32(left.X, right.X); r32 minY = MinR32(left.Y, right.Y); r32 minZ = MinR32(left.Z, right.Z); return NewBox(minX, minY, minZ, MaxR32(left.X + left.Width, right.X + right.Width) - minX, MaxR32(left.Y + left.Height, right.Y + right.Height) - minY, MaxR32(left.Z + left.Depth, right.Z + right.Depth) - minZ); }
PEXPI boxi BothBoxi(boxi left, boxi right) { i32 minX = MinI32(left.X, right.X); i32 minY = MinI32(left.Y, right.Y); i32 minZ = MinI32(left.Z, right.Z); return NewBoxi(minX, minY, minZ, MaxI32(left.X + left.Width, right.X + right.Width) - minX, MaxI32(left.Y + left.Height, right.Y + right.Height) - minY, MaxI32(left.Z + left.Depth, right.Z + right.Depth) - minZ); }

PEXPI rec OverlapPartRec(rec left, rec right)
{
	r32 minX = MaxR32(left.X, right.X);
	r32 minY = MaxR32(left.Y, right.Y);
	r32 maxX = MaxR32(MinR32(left.X + left.Width, right.X + right.Width), minX);
	r32 maxY = MaxR32(MinR32(left.Y + left.Height, right.Y + right.Height), minY);
	return NewRec(minX, minY, maxX - minX, maxY - minY);
}
PEXPI reci OverlapPartReci(reci left, reci right)
{
	i32 minX = MaxI32(left.X, right.X);
	i32 minY = MaxI32(left.Y, right.Y);
	i32 maxX = MaxI32(MinI32(left.X + left.Width, right.X + right.Width), minX);
	i32 maxY = MaxI32(MinI32(left.Y + left.Height, right.Y + right.Height), minY);
	return NewReci(minX, minY, maxX - minX, maxY - minY);
}
PEXPI box OverlapPartBox(box left, box right)
{
	r32 minX = MaxR32(left.X, right.X);
	r32 minY = MaxR32(left.Y, right.Y);
	r32 minZ = MaxR32(left.Z, right.Z);
	r32 maxX = MaxR32(MinR32(left.X + left.Width, right.X + right.Width), minX);
	r32 maxY = MaxR32(MinR32(left.Y + left.Height, right.Y + right.Height), minY);
	r32 maxZ = MaxR32(MinR32(left.Z + left.Depth, right.Z + right.Depth), minZ);
	return NewBox(minX, minY, minZ, maxX - minX, maxY - minY, maxZ - minZ);
}
PEXPI boxi OverlapPartBoxi(boxi left, boxi right)
{
	i32 minX = MaxI32(left.X, right.X);
	i32 minY = MaxI32(left.Y, right.Y);
	i32 minZ = MaxI32(left.Z, right.Z);
	i32 maxX = MaxI32(MinI32(left.X + left.Width, right.X + right.Width), minX);
	i32 maxY = MaxI32(MinI32(left.Y + left.Height, right.Y + right.Height), minY);
	i32 maxZ = MaxI32(MinI32(left.Z + left.Depth, right.Z + right.Depth), minZ);
	return NewBoxi(minX, minY, minZ, maxX - minX, maxY - minY, maxZ - minZ);
}

PEXPI bool IsInsideRec(rec rectangle, v2 position) { return (position.X >= rectangle.X && position.Y >= rectangle.Y && position.X < rectangle.X + rectangle.Width && position.Y < rectangle.Y + rectangle.Height); }
PEXPI bool IsInsideRecInclusive(rec rectangle, v2 position) { return (position.X >= rectangle.X && position.Y >= rectangle.Y && position.X <= rectangle.X + rectangle.Width && position.Y <= rectangle.Y + rectangle.Height); }

PEXPI void AlignRecToV2(rec* rectangleOut, v2 alignmentScale)
{
	v2 bottomRight = AddV2(rectangleOut->TopLeft, rectangleOut->Size);
	AlignV2ToV2(&bottomRight, alignmentScale);
	AlignV2ToV2(&rectangleOut->TopLeft, alignmentScale);
	rectangleOut->Size = SubV2(bottomRight, rectangleOut->TopLeft);
}
PEXPI void AlignRecTo(rec* rectangleOut, r32 alignmentScale) { AlignRecToV2(rectangleOut, FillV2(alignmentScale)); }
PEXPI void AlignRec(rec* rectangleOut) { AlignRecToV2(rectangleOut, V2_One); }

PEXPI void AlignBoxToV3(box* boundingBoxOut, v3 alignmentScale)
{
	v3 topRightFront = AddV3(boundingBoxOut->BottomLeftBack, boundingBoxOut->Size);
	AlignV3ToV3(&topRightFront, alignmentScale);
	AlignV3ToV3(&boundingBoxOut->BottomLeftBack, alignmentScale);
	boundingBoxOut->Size = SubV3(topRightFront, boundingBoxOut->BottomLeftBack);
}
PEXPI void AlignBoxTo(box* boundingBoxOut, r32 alignmentScale) { AlignBoxToV3(boundingBoxOut, FillV3(alignmentScale)); }
PEXPI void AlignBox(box* boundingBoxOut) { AlignBoxToV3(boundingBoxOut, V3_One); }


//TODO: ExpandRecToV2/ExpandReciToV2i?
//TODO: IsInsideBox/IsInsideObb2/IsRecInsideRec?
//TODO: DoesOverlapRec?
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
	return NewRec(
		other.X + other.Width * InverseLerpR32(reference.X, reference.X + reference.Width, subReference.X),
		other.Y + other.Height * InverseLerpR32(reference.Y, reference.Y + reference.Height, subReference.Y),
		other.Width * InverseLerpR32(0, reference.Width, subReference.Width),
		other.Height * InverseLerpR32(0, reference.Height, subReference.Height)
	);
}
PEXPI box RelativeBox(box reference, box subReference, box other)
{
	return NewBox(
		other.X + other.Width * InverseLerpR32(reference.X, reference.X + reference.Width, subReference.X),
		other.Y + other.Height * InverseLerpR32(reference.Y, reference.Y + reference.Height, subReference.Y),
		other.Z + other.Depth * InverseLerpR32(reference.Z, reference.Z + reference.Depth, subReference.Z),
		other.Width * InverseLerpR32(0, reference.Width, subReference.Width),
		other.Height * InverseLerpR32(0, reference.Height, subReference.Height),
		other.Depth * InverseLerpR32(0, reference.Depth, subReference.Depth)
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
