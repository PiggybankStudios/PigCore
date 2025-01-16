/*
File:   struct_vectors.h
Author: Taylor Robbins
Date:   01\15\2025
Description:
	** Since we depend on HandmadeMath.h for the majority of our vector math related
	** functions and types, this file serves more as a aliasing file for all the
	** things in HandmadeMath.h, with some extensions for functions that we had
	** in GyLib that HandmadeMath.h does not have.
*/

#ifndef _STRUCT_VECTORS_H
#define _STRUCT_VECTORS_H

#include "base/base_typedefs.h"
#include "std/std_includes.h"
#include "std/std_basic_math.h" //Needed for SqrtR32
#include "std/std_math_ex.h" //Needed for AreSimilarR32
#include "std/std_trig.h" //Needed for SinR32, CosR32, and AcosR32

#include "struct/struct_handmade_math_include.h"

// +--------------------------------------------------------------+
// |                   Typedefs and Structures                    |
// +--------------------------------------------------------------+
typedef union Vec2i Vec2i;
union Vec2i
{
	struct { i32 X, Y; };
	struct { i32 U, V; };
	struct { i32 Left, Right; };
	struct { i32 Width, Height; };
	i32 Elements[2];
};

typedef union Vec3i Vec3i;
union Vec3i
{
	struct { i32 X, Y, Z; };
	struct { i32 U, V, W; };
	struct { i32 R, G, B; };
	struct { Vec2i XY; i32 _Ignored0; };
	struct { i32 _Ignored1; Vec2i YZ; };
	struct { Vec2i UV; i32 _Ignored2; };
	struct { i32 _Ignored3; Vec2i VW; };
	i32 Elements[3];
};

typedef union Vec4i Vec4i;
union Vec4i
{
	struct
	{
		union
		{
			Vec3i XYZ;
			struct { i32 X, Y, Z; };
		};
		i32 W;
	};
	struct
	{
		union
		{
			Vec3i RGB;
			struct { i32 R, G, B; };
		};
		i32 A;
	};
	struct { Vec2i XY; i32 _Ignored0; i32 _Ignored1; };
	struct { i32 _Ignored2; Vec2i YZ; i32 _Ignored3; };
	struct { i32 _Ignored4; i32 _Ignored5; Vec2i ZW; };
	i32 Elements[4];
};

typedef HMM_Vec2 v2;
typedef Vec2i v2i;
typedef HMM_Vec3 v3;
typedef Vec3i v3i;
typedef HMM_Vec4 v4;
typedef Vec4i v4i;

// +--------------------------------------------------------------+
// |                        New Functions                         |
// +--------------------------------------------------------------+
#define NewV2(x, y) HMM_V2((x), (y))
static inline v2i NewV2i(i32 x, i32 y) { v2i result; result.X = x; result.Y = y; return result; }
#define NewV3(x, y, z) HMM_V3((x), (y), (z))
static inline v3i NewV3i(i32 x, i32 y, i32 z) { v3i result; result.X = x; result.Y = y; result.Z = z; return result; }
#define NewV4(x, y, z, w) HMM_V4((x), (y), (z), (w))
static inline v4i NewV4i(i32 x, i32 y, i32 z, i32 w) { v4i result; result.X = x; result.Y = y; result.Z = z; result.W = w; return result; }

#define FillV2(value)  NewV2((value), (value))
#define FillV2i(value) NewV2i((value), (value))
#define FillV3(value)  NewV3((value), (value), (value))
#define FillV3i(value) NewV3i((value), (value), (value))
#define FillV4(value)  NewV4((value), (value), (value), (value))
#define FillV4i(value) NewV4i((value), (value), (value), (value))

// +--------------------------------------------------------------+
// |                 Simple Conversions and Casts                 |
// +--------------------------------------------------------------+
static inline v2 ToV2Fromi(v2i vec2i) { return NewV2((r32)vec2i.X, (r32)vec2i.Y); }
static inline v2 ToV2From3(v3 vec3) { return NewV2(vec3.X, vec3.Y); }
static inline v2 ToV2From3XZ(v3 vec3) { return NewV2(vec3.X, vec3.Z); }
static inline v2 ToV2From3YZ(v3 vec3) { return NewV2(vec3.Y, vec3.Z); }
static inline v2i ToV2iFrom3(v3i vec3i) { return NewV2i(vec3i.X, vec3i.Y); }
static inline v2i ToV2iFrom3XZ(v3i vec3i) { return NewV2i(vec3i.X, vec3i.Z); }
static inline v2i ToV2iFrom3YZ(v3i vec3i) { return NewV2i(vec3i.Y, vec3i.Z); }

static inline v3 ToV3Fromi(v3i vec3i) { return NewV3((r32)vec3i.X, (r32)vec3i.Y, (r32)vec3i.Z); }
static inline v3 ToV3From2(v2 vec2, r32 z) { return NewV3(vec2.X, vec2.Y, z); }
static inline v3 ToV3From2XZ(v2 vec2, r32 y) { return NewV3(vec2.X, y, vec2.Y); }
static inline v3 ToV3From4(v4 vec4) { return NewV3(vec4.X, vec4.Y, vec4.Z); }
static inline v3i ToV3iFrom2(v2i vec2i, i32 z) { return NewV3i(vec2i.X, vec2i.Y, z); }
static inline v3i ToV3iFrom2XZ(v2i vec2i, i32 y) { return NewV3i(vec2i.X, y, vec2i.Y); }
static inline v3i ToV3iFrom4(v4i vec4i) { return NewV3i(vec4i.X, vec4i.Y, vec4i.Z); }

static inline v4 ToV4Fromi(v4i vec4i) { return NewV4((r32)vec4i.X, (r32)vec4i.Y, (r32)vec4i.Z, (r32)vec4i.W); }
#define ToV4From3(vec3, w) HMM_V4V((vec3), (w))
static inline v4i ToV4iFrom3(v3i vec3i, i32 w) { return NewV4i(vec3i.X, vec3i.Y, vec3i.Z, w); }

// +--------------------------------------------------------------+
// |                   Simple Value Definitions                   |
// +--------------------------------------------------------------+
#define V2_Zero          NewV2( 0.0f,  0.0f)
#define V2_One           NewV2( 1.0f,  1.0f)
#define V2_Half          NewV2( 0.5f,  0.5f)
#define V2_Left          NewV2(-1.0f,  0.0f)
#define V2_Right         NewV2( 1.0f,  0.0f)
#define V2_Up            NewV2( 0.0f, -1.0f)
#define V2_Down          NewV2( 0.0f,  1.0f)
#define V2_Zero_Const    (v2){{ 0.0f,  0.0f}}

#define V2i_Zero          NewV2i( 0,  0)
#define V2i_One           NewV2i( 1,  1)
#define V2i_Left          NewV2i(-1,  0)
#define V2i_Right         NewV2i( 1,  0)
#define V2i_Up            NewV2i( 0, -1)
#define V2i_Down          NewV2i( 0,  1)
#define V2i_Zero_Const    (v2i){{ 0,  0}}

//NOTE: See struct_directions.h for an explanation of the coordinate system handed-ness and up direction
#define V3_Zero          NewV3( 0.0f,  0.0f,  0.0f)
#define V3_One           NewV3( 1.0f,  1.0f,  1.0f)
#define V3_Half          NewV3( 0.5f,  0.5f,  0.5f)
#define V3_Left          NewV3(-1.0f,  0.0f,  0.0f)
#define V3_Right         NewV3( 1.0f,  0.0f,  0.0f)
#define V3_Bottom        NewV3( 0.0f, -1.0f,  0.0f)
#define V3_Top           NewV3( 0.0f,  1.0f,  0.0f)
#define V3_Back          NewV3( 0.0f,  0.0f, -1.0f)
#define V3_Front         NewV3( 0.0f,  0.0f,  1.0f)
#define V3_Down          NewV3( 0.0f, -1.0f,  0.0f)
#define V3_Up            NewV3( 0.0f,  1.0f,  0.0f)
#define V3_Backward      NewV3( 0.0f,  0.0f, -1.0f)
#define V3_Forward       NewV3( 0.0f,  0.0f,  1.0f)
#define V3_Zero_Const    (v3){{ 0.0f,  0.0f,  0.0f}}

#define V3i_Zero          NewV3i( 0,  0,  0)
#define V3i_One           NewV3i( 1,  1,  1)
#define V3i_Left          NewV3i(-1,  0,  0)
#define V3i_Right         NewV3i( 1,  0,  0)
#define V3i_Bottom        NewV3i( 0, -1,  0)
#define V3i_Top           NewV3i( 0,  1,  0)
#define V3i_Back          NewV3i( 0,  0, -1)
#define V3i_Front         NewV3i( 0,  0,  1)
#define V3i_Down          NewV3i( 0, -1,  0)
#define V3i_Up            NewV3i( 0,  1,  0)
#define V3i_Backward      NewV3i( 0,  0, -1)
#define V3i_Forward       NewV3i( 0,  0,  1)
#define V3i_Zero_Const    (v3i){{ 0,  0,  0}}

#define V4_Zero          NewV4( 0.0f,  0.0f,  0.0f, 0.0f)
#define V4_One           NewV4( 1.0f,  1.0f,  1.0f, 1.0f)
#define V4_Half          NewV4( 0.5f,  0.5f,  0.5f, 0.5f)
#define V4_Left          NewV4(-1.0f,  0.0f,  0.0f, 0.0f)
#define V4_Right         NewV4( 1.0f,  0.0f,  0.0f, 0.0f)
#define V4_Bottom        NewV4( 0.0f, -1.0f,  0.0f, 0.0f)
#define V4_Top           NewV4( 0.0f,  1.0f,  0.0f, 0.0f)
#define V4_Back          NewV4( 0.0f,  0.0f, -1.0f, 0.0f)
#define V4_Front         NewV4( 0.0f,  0.0f,  1.0f, 0.0f)
#define V4_Down          NewV4( 0.0f, -1.0f,  0.0f, 0.0f)
#define V4_Up            NewV4( 0.0f,  1.0f,  0.0f, 0.0f)
#define V4_Backward      NewV4( 0.0f,  0.0f, -1.0f, 0.0f)
#define V4_Forward       NewV4( 0.0f,  0.0f,  1.0f, 0.0f)
#define V4_Prevward      NewV4( 0.0f,  0.0f,  0.0f,-1.0f)
#define V4_Nextward      NewV4( 0.0f,  0.0f,  0.0f, 1.0f)
#define V4_Zero_Const (v4){{{{{ 0.0f,  0.0f,  0.0f}}},0.0f}}

#define V4i_Zero          NewV4i( 0,  0,  0, 0)
#define V4i_One           NewV4i( 1,  1,  1, 0)
#define V4i_Left          NewV4i(-1,  0,  0, 0)
#define V4i_Right         NewV4i( 1,  0,  0, 0)
#define V4i_Bottom        NewV4i( 0, -1,  0, 0)
#define V4i_Top           NewV4i( 0,  1,  0, 0)
#define V4i_Back          NewV4i( 0,  0, -1, 0)
#define V4i_Front         NewV4i( 0,  0,  1, 0)
#define V4i_Down          NewV4i( 0, -1,  0, 0)
#define V4i_Up            NewV4i( 0,  1,  0, 0)
#define V4i_Backward      NewV4i( 0,  0, -1, 0)
#define V4i_Forward       NewV4i( 0,  0,  1, 0)
#define V4i_Prevward      NewV4i( 0,  0,  0,-1)
#define V4i_Nextward      NewV4i( 0,  0,  0, 1)
#define V4i_Zero_Const (v4i){{{{{ 0,  0,  0}}},0}}

// +--------------------------------------------------------------+
// |                Operator Overload Equivalents                 |
// +--------------------------------------------------------------+
#define AddV2(left, right) HMM_AddV2((left), (right))
#define AddV3(left, right) HMM_AddV3((left), (right))
#define AddV4(left, right) HMM_AddV4((left), (right))
static inline v2i AddV2i(v2i left, v2i right) { v2i result; result.X = left.X + right.X; result.Y = left.Y + right.Y; return result; }
static inline v3i AddV3i(v3i left, v3i right) { v3i result; result.X = left.X + right.X; result.Y = left.Y + right.Y; result.Z = left.Z + right.Z; return result; }
static inline v4i AddV4i(v4i left, v4i right) { v4i result; result.X = left.X + right.X; result.Y = left.Y + right.Y; result.Z = left.Z + right.Z; result.W = left.W + right.W; return result; }

#define SubV2(left, right) HMM_SubV2((left), (right))
#define SubV3(left, right) HMM_SubV3((left), (right))
#define SubV4(left, right) HMM_SubV4((left), (right))
static inline v2i SubV2i(v2i left, v2i right) { v2i result; result.X = left.X - right.X; result.Y = left.Y - right.Y; return result; }
static inline v3i SubV3i(v3i left, v3i right) { v3i result; result.X = left.X - right.X; result.Y = left.Y - right.Y; result.Z = left.Z - right.Z; return result; }
static inline v4i SubV4i(v4i left, v4i right) { v4i result; result.X = left.X - right.X; result.Y = left.Y - right.Y; result.Z = left.Z - right.Z; result.W = left.W - right.W; return result; }

#define MulV2(left, right) HMM_MulV2((left), (right))
#define MulV3(left, right) HMM_MulV3((left), (right))
#define MulV4(left, right) HMM_MulV4((left), (right))
static inline v2i MulV2i(v2i left, v2i right) { v2i result; result.X = left.X * right.X; result.Y = left.Y * right.Y; return result; }
static inline v3i MulV3i(v3i left, v3i right) { v3i result; result.X = left.X * right.X; result.Y = left.Y * right.Y; result.Z = left.Z * right.Z; return result; }
static inline v4i MulV4i(v4i left, v4i right) { v4i result; result.X = left.X * right.X; result.Y = left.Y * right.Y; result.Z = left.Z * right.Z; result.W = left.W * right.W; return result; }

#define DivV2(left, right) HMM_DivV2((left), (right))
#define DivV3(left, right) HMM_DivV3((left), (right))
#define DivV4(left, right) HMM_DivV4((left), (right))
static inline v2i DivV2i(v2i left, v2i right) { v2i result; result.X = left.X / right.X; result.Y = left.Y / right.Y; return result; }
static inline v3i DivV3i(v3i left, v3i right) { v3i result; result.X = left.X / right.X; result.Y = left.Y / right.Y; result.Z = left.Z / right.Z; return result; }
static inline v4i DivV4i(v4i left, v4i right) { v4i result; result.X = left.X / right.X; result.Y = left.Y / right.Y; result.Z = left.Z / right.Z; result.W = left.W / right.W; return result; }

#define ScaleV2(vec2, scalar) HMM_MulV2F((vec2), (scalar))
#define ScaleV3(vec3, scalar) HMM_MulV3F((vec3), (scalar))
#define ScaleV4(vec4, scalar) HMM_MulV4F((vec4), (scalar))
static inline v2i ScaleV2i(v2i left, i32 scalar) { v2i result; result.X = left.X * scalar; result.Y = left.Y * scalar; return result; }
static inline v3i ScaleV3i(v3i left, i32 scalar) { v3i result; result.X = left.X * scalar; result.Y = left.Y * scalar; result.Z = left.Z * scalar; return result; }
static inline v4i ScaleV4i(v4i left, i32 scalar) { v4i result; result.X = left.X * scalar; result.Y = left.Y * scalar; result.Z = left.Z * scalar; result.W = left.W * scalar; return result; }

#define ShrinkV2(vec2, divisor) HMM_DivV2F((vec2), (divisor))
#define ShrinkV3(vec3, divisor) HMM_DivV3F((vec3), (divisor))
#define ShrinkV4(vec4, divisor) HMM_DivV4F((vec4), (divisor))
static inline v2i ShrinkV2i(v2i left, i32 divisor) { v2i result; result.X = left.X / divisor; result.Y = left.Y / divisor; return result; }
static inline v3i ShrinkV3i(v3i left, i32 divisor) { v3i result; result.X = left.X / divisor; result.Y = left.Y / divisor; result.Z = left.Z / divisor; return result; }
static inline v4i ShrinkV4i(v4i left, i32 divisor) { v4i result; result.X = left.X / divisor; result.Y = left.Y / divisor; result.Z = left.Z / divisor; result.W = left.W / divisor; return result; }

#define AreEqualV2(left, right) HMM_EqV2((left), (right))
#define AreEqualV3(left, right) HMM_EqV3((left), (right))
#define AreEqualV4(left, right) HMM_EqV4((left), (right))
#define AreEqualV2i(left, right) ((left).X == (right).X && (left).Y == (right).Y)
#define AreEqualV3i(left, right) ((left).X == (right).X && (left).Y == (right).Y && (left).Z == (right).Z)
#define AreEqualV4i(left, right) ((left).X == (right).X && (left).Y == (right).Y && (left).Z == (right).Z && (left).W == (right).W)

#define AreSimilarV2(left, right, tolerance) return (AreSimilarR32((left).X, (right).X, (tolerance)) && AreSimilarR32((left).Y, (right).Y, (tolerance)))
#define AreSimilarV3(left, right, tolerance) return (AreSimilarR32((left).X, (right).X, (tolerance)) && AreSimilarR32((left).Y, (right).Y, (tolerance)) && AreSimilarR32((left).Z, (right).Z, (tolerance)))
#define AreSimilarV4(left, right, tolerance) return (AreSimilarR32((left).X, (right).X, (tolerance)) && AreSimilarR32((left).Y, (right).Y, (tolerance)) && AreSimilarR32((left).Z, (right).Z, (tolerance)) && AreSimilarR32((left).W, (right).W, (tolerance)))

// +--------------------------------------------------------------+
// |                      Common Vector Math                      |
// +--------------------------------------------------------------+
#define DotV2(left, right) HMM_DotV2((left), (right))
#define DotV3(left, right) HMM_DotV3((left), (right))
#define DotV4(left, right) HMM_DotV4((left), (right))
static inline i32 DotV2i(v2i left, v2i right) { return (left.X * right.X) + (left.Y * right.Y); }
static inline i32 DotV3i(v3i left, v3i right) { return (left.X * right.X) + (left.Y * right.Y) + (left.Z * right.Z); }
static inline i32 DotV4i(v4i left, v4i right) { return (left.X * right.X) + (left.Y * right.Y) + (left.Z * right.Z) + (left.W * right.W); }

#define CrossV3(left, right) HMM_Cross((left), (right))
static inline v3i CrossV3i(v3i left, v3i right) { return NewV3i(left.Y*right.Z - left.Z*right.Y, left.Z*right.X - left.X*right.Z, left.X*right.Y - left.Y*right.X); }

#define LengthSquaredV2(vec2) HMM_LenSqrV2(vec2)
#define LengthSquaredV3(vec3) HMM_LenSqrV3(vec3)
#define LengthSquaredV4(vec4) HMM_LenSqrV4(vec4)
static inline i32 LengthSquaredV2i(v2i vec2i) { return DotV2i(vec2i, vec2i); }
static inline i32 LengthSquaredV3i(v3i vec3i) { return DotV3i(vec3i, vec3i); }
static inline i32 LengthSquaredV4i(v4i vec4i) { return DotV4i(vec4i, vec4i); }

#define LengthV2(vec2) HMM_LenV2(vec2)
#define LengthV3(vec3) HMM_LenV3(vec3)
#define LengthV4(vec4) HMM_LenV4(vec4)
static inline r32 LengthV2i(v2i vec2i) { return SqrtR32((r32)LengthSquaredV2i(vec2i)); }
static inline r32 LengthV3i(v3i vec3i) { return SqrtR32((r32)LengthSquaredV3i(vec3i)); }
static inline r32 LengthV4i(v4i vec4i) { return SqrtR32((r32)LengthSquaredV4i(vec4i)); }

#define NormalizeV2(vec2) HMM_NormV2(vec2)
#define NormalizeV3(vec3) HMM_NormV3(vec3)
#define NormalizeV4(vec4) HMM_NormV4(vec4)

#define LerpV2(start, end, amount) HMM_LerpV2((start), (amount), (end))
#define LerpV3(start, end, amount) HMM_LerpV3((start), (amount), (end))
#define LerpV4(start, end, amount) HMM_LerpV4((start), (amount), (end))

// +--------------------------------------------------------------+
// |              Componentwise Intrinsic Functions               |
// +--------------------------------------------------------------+
//NOTE: These int overloads for Floor/Ceil/Round are a bit misleading with their name.
// They actually take floating point vector types, even though the name reads like
// functions that take integer vector types. They RETURN integer vector types, and
// that's what the i suffix is meant to convey.
static inline v2 FloorV2(v2 vec2) { return NewV2(FloorR32(vec2.X), FloorR32(vec2.Y)); }
static inline v3 FloorV3(v3 vec3) { return NewV3(FloorR32(vec3.X), FloorR32(vec3.Y), FloorR32(vec3.Z)); }
static inline v4 FloorV4(v4 vec4) { return NewV4(FloorR32(vec4.X), FloorR32(vec4.Y), FloorR32(vec4.Z), FloorR32(vec4.W)); }
static inline v2i FloorV2i(v2 vec2) { return NewV2i(FloorR32i(vec2.X), FloorR32i(vec2.Y)); }
static inline v3i FloorV3i(v3 vec3) { return NewV3i(FloorR32i(vec3.X), FloorR32i(vec3.Y), FloorR32i(vec3.Z)); }
static inline v4i FloorV4i(v4 vec4) { return NewV4i(FloorR32i(vec4.X), FloorR32i(vec4.Y), FloorR32i(vec4.Z), FloorR32i(vec4.W)); }

static inline v2 CeilV2(v2 vec2) { return NewV2(CeilR32(vec2.X), CeilR32(vec2.Y)); }
static inline v3 CeilV3(v3 vec3) { return NewV3(CeilR32(vec3.X), CeilR32(vec3.Y), CeilR32(vec3.Z)); }
static inline v4 CeilV4(v4 vec4) { return NewV4(CeilR32(vec4.X), CeilR32(vec4.Y), CeilR32(vec4.Z), CeilR32(vec4.W)); }
static inline v2i CeilV2i(v2 vec2) { return NewV2i(CeilR32i(vec2.X), CeilR32i(vec2.Y)); }
static inline v3i CeilV3i(v3 vec3) { return NewV3i(CeilR32i(vec3.X), CeilR32i(vec3.Y), CeilR32i(vec3.Z)); }
static inline v4i CeilV4i(v4 vec4) { return NewV4i(CeilR32i(vec4.X), CeilR32i(vec4.Y), CeilR32i(vec4.Z), CeilR32i(vec4.W)); }

static inline v2 RoundV2(v2 vec2) { return NewV2(RoundR32(vec2.X), RoundR32(vec2.Y)); }
static inline v3 RoundV3(v3 vec3) { return NewV3(RoundR32(vec3.X), RoundR32(vec3.Y), RoundR32(vec3.Z)); }
static inline v4 RoundV4(v4 vec4) { return NewV4(RoundR32(vec4.X), RoundR32(vec4.Y), RoundR32(vec4.Z), RoundR32(vec4.W)); }
static inline v2i RoundV2i(v2 vec2) { return NewV2i(RoundR32i(vec2.X), RoundR32i(vec2.Y)); }
static inline v3i RoundV3i(v3 vec3) { return NewV3i(RoundR32i(vec3.X), RoundR32i(vec3.Y), RoundR32i(vec3.Z)); }
static inline v4i RoundV4i(v4 vec4) { return NewV4i(RoundR32i(vec4.X), RoundR32i(vec4.Y), RoundR32i(vec4.Z), RoundR32i(vec4.W)); }

static inline v2 MinV2(v2 left, v2 right) { return NewV2(MinR32(left.X, right.X), MinR32(left.Y, right.Y)); }
static inline v3 MinV3(v3 left, v3 right) { return NewV3(MinR32(left.X, right.X), MinR32(left.Y, right.Y), MinR32(left.Z, right.Z)); }
static inline v4 MinV4(v4 left, v4 right) { return NewV4(MinR32(left.X, right.X), MinR32(left.Y, right.Y), MinR32(left.Z, right.Z), MinR32(left.W, right.W)); }

static inline v2 MaxV2(v2 left, v2 right) { return NewV2(MaxR32(left.X, right.X), MaxR32(left.Y, right.Y)); }
static inline v3 MaxV3(v3 left, v3 right) { return NewV3(MaxR32(left.X, right.X), MaxR32(left.Y, right.Y), MaxR32(left.Z, right.Z)); }
static inline v4 MaxV4(v4 left, v4 right) { return NewV4(MaxR32(left.X, right.X), MaxR32(left.Y, right.Y), MaxR32(left.Z, right.Z), MaxR32(left.W, right.W)); }

static inline v2 AbsV2(v2 vec2) { return NewV2(AbsR32(vec2.X), AbsR32(vec2.Y)); }
static inline v3 AbsV3(v3 vec3) { return NewV3(AbsR32(vec3.X), AbsR32(vec3.Y), AbsR32(vec3.Z)); }
static inline v4 AbsV4(v4 vec4) { return NewV4(AbsR32(vec4.X), AbsR32(vec4.Y), AbsR32(vec4.Z), AbsR32(vec4.W)); }
static inline v2i AbsV2i(v2i vec2i) { return NewV2i(AbsI32(vec2i.X), AbsI32(vec2i.Y)); }
static inline v3i AbsV3i(v3i vec3i) { return NewV3i(AbsI32(vec3i.X), AbsI32(vec3i.Y), AbsI32(vec3i.Z)); }
static inline v4i AbsV4i(v4i vec4i) { return NewV4i(AbsI32(vec4i.X), AbsI32(vec4i.Y), AbsI32(vec4i.Z), AbsI32(vec4i.W)); }

static inline v2 ClampV2(v2 vec2, v2 minVec2, v2 maxVec2) { return NewV2(ClampR32(vec2.X, minVec2.X, maxVec2.X), ClampR32(vec2.Y, minVec2.Y, maxVec2.Y)); }
static inline v3 ClampV3(v3 vec3, v3 minVec3, v3 maxVec3) { return NewV3(ClampR32(vec3.X, minVec3.X, maxVec3.X), ClampR32(vec3.Y, minVec3.Y, maxVec3.Y), ClampR32(vec3.Z, minVec3.Z, maxVec3.Z)); }
static inline v4 ClampV4(v4 vec4, v4 minVec4, v4 maxVec4) { return NewV4(ClampR32(vec4.X, minVec4.X, maxVec4.X), ClampR32(vec4.Y, minVec4.Y, maxVec4.Y), ClampR32(vec4.Z, minVec4.Z, maxVec4.Z), ClampR32(vec4.W, minVec4.W, maxVec4.W)); }
static inline v2i ClampV2i(v2i vec2i, v2i minVec2i, v2i maxVec2i) { return NewV2i(ClampI32(vec2i.X, minVec2i.X, maxVec2i.X), ClampI32(vec2i.Y, minVec2i.Y, maxVec2i.Y)); }
static inline v3i ClampV3i(v3i vec3i, v3i minVec3i, v3i maxVec3i) { return NewV3i(ClampI32(vec3i.X, minVec3i.X, maxVec3i.X), ClampI32(vec3i.Y, minVec3i.Y, maxVec3i.Y), ClampI32(vec3i.Z, minVec3i.Z, maxVec3i.Z)); }
static inline v4i ClampV4i(v4i vec4i, v4i minVec4i, v4i maxVec4i) { return NewV4i(ClampI32(vec4i.X, minVec4i.X, maxVec4i.X), ClampI32(vec4i.Y, minVec4i.Y, maxVec4i.Y), ClampI32(vec4i.Z, minVec4i.Z, maxVec4i.Z), ClampI32(vec4i.W, minVec4i.W, maxVec4i.W)); }

//TODO: Should we add "Inner" and "Determinant" functions like we had in GyLib?

// +--------------------------------------------------------------+
// |                   Other Helpful Functions                    |
// +--------------------------------------------------------------+
static inline v2 PerpRightV2(v2 vec2) { return NewV2(-vec2.Y, vec2.X); }
static inline v2 PerpLeftV2(v2 vec2) { return NewV2(vec2.Y, -vec2.X); }

static inline v2 Vec2Rotate(v2 vec2, r32 angle)
{
	return NewV2(
		CosR32(angle) * vec2.X - SinR32(angle) * vec2.Y,
		SinR32(angle) * vec2.X + CosR32(angle) * vec2.Y
	);
}

static inline r32 Vec3AngleBetween(v3 left, v3 right)
{
	return AcosR32(DotV3(left, right) / (LengthV3(left) * LengthV3(right)));
}

// This Assumes +Y is up and is phrased in terms of a first person camera (rotationUpDown = rotation around Z and then facingDirection = rotation around Y)
static inline v3 Vec3From2Angles(r32 facingDirection, r32 rotationUpDown, r32 radius)
{
	r32 circleRadius = CosR32(rotationUpDown) * radius;
	return NewV3(
		CosR32(facingDirection) * circleRadius,
		SinR32(facingDirection) * circleRadius,
		SinR32(rotationUpDown)
	);
}

// +--------------------------------------------------------------+
// |             Quickhand Functions for Layout Code              |
// +--------------------------------------------------------------+

#endif //  _STRUCT_VECTORS_H

#if defined(_STRUCT_VECTORS_H) && defined(_STRUCT_QUATERNION_H) && defined(_STRUCT_MATRICES_H)
#include "cross/cross_vectors_quaternion_and_matrices.h"
#endif
