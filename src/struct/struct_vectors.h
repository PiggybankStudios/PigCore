/*
File:   struct_vectors.h
Author: Taylor Robbins
Date:   01\15\2025
Description:
	** Holds vector types which are 2, 3, or 4 elements of the same type.
	** Most often these vectors represent cartesian coordinates or sizes in a 2D/3D/4D space.
	** The "default" vectors (v2,v3,v4) have r32 elements. There are also variants
	** that holds i32 (v2i,v3i,v4i) and ones that hold r64 (v2d,v3d,v4d).
	**
	** These types have multiple aliases for each member, for example "x" is also
	** called "u" and "red" and "width" and "hori". These are meant to make usage
	** code more readable, with the choice of alias implying the kind of data
	** being held in the vector.
	**
	** Constructing vector types is done through MakeV2, MakeV3, Make4 which construct
	** the type using designated initializer syntax { .x=..., .y=... }. If you want
	** to initialize a vector in a context where compile-time const-ness is required
	** you can use MakeV2_Const, MakeV3_Const, etc. There are also convenience macros
	** like FillV2(1.0f) which fills all members with a single value. Note that some of
	** these macros (like FillV2) may evaluate the inputs multiple times so be careful
	** when passing expressions that have side effects.
	**
	** Many common operators are provided in this file like AddV2, MulV4, DotV2, CrossV3, etc.
	** These can be used directly, in C++ there are operator overloads, in C there are
	** macros that use the C23 _Generic() to switch function called based on type.
	** Because of the way _Generic works, these are only available if you #include:
	**   struct_vectors.h, struct_quaternion.h, struct_matrices.h, and struct_rectangles.h
	** See: cross_vectors_quaternion_matrices_and_rectangles.h
*/

#ifndef _STRUCT_VECTORS_H
#define _STRUCT_VECTORS_H

#include "base/base_defines_check.h"
#include "base/base_assert.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "std/std_includes.h"
#include "std/std_basic_math.h"
#include "std/std_math_ex.h"
#include "std/std_trig.h"

//TODO: Add C++ overloads for casting between various vector types? (with "explicit" keyword)

//NOTE: About curly-bracket initialization in C/C++
//      Both C and C++ have support for curly-bracket syntax when initializing a struct to some value.
//      However their syntax support and generated code differs wildly since C++ calls a constructor for the struct (which is really a class) and for each member
//      We used to use NewV2, NewV3, etc. functions to avoid using curly-bracket syntax and thereby avoid the complexity of what is/isn't allowed in each language/compiler
//      However (in non-optimized builds especially) this can be wasteful since the function call is more expensive than the code to set the members
//      Inlining these functions helps, but only when PigCore implementation is being built directly into the program (not into and external pig_core.dll)
//      So in Nov 2025 we made the switch from "New" functions to "Make" macros which use curly-bracket syntax.
//      In order for this to work properly there are a bunch of things we need to be aware of and do correctly:
//        1. Parenthesis usage around all macro parameters to handle complex parameters that are longer statments and may include syntax that changes the meaning once pasted into the curly-bracket syntax
//        2. All members of the struct must be initialized in the order they appear in the struct
//        3. Desginated initializer syntax (basically naming each member rather than relying on order of members)
//        4. Different syntax for C/C++ for type prefix. In C it's (type){ .member=value } but in C++ it's type{ .member=value } (no parenthesis around type). We use the NEW_STRUCT(type) macro to do the correct thing depending on LANGUAGE_IS_C/LANGUAGE_IS_CPP
//        5. _Const variants that don't have type prefix for use when initializing global variables to an initial value
//        6. Whenever possible, only use the each parameter of a macro once. Using the parameter multiple times can cause unintended multiple execution at runtime if the macro parameter contains a function call
//        7. Macro parameters must have a different name than designated member names to avoid incorrectly generated code. Often times we will suffix the macro parameter with "Value" or the expected type of the parameter like "R32" to make the name unique
//        8. Minimal casting inside the designated initializer to take advantage of type-checking during assignment of members. Sometimes we need to throw away a const-qualifier, this is better done with a union in the structure than a type-cast in the Make macro
//        9. Arrays have weird quirks when initializing with designated members. We had a bug with initial implementation of MakeMat4. TODO: Need to do more research/testing to describe what shoud/shouldn't be done exactly
//       10. Any macro arguments that are structs need to be NOT surrounded by parenthesis when inside a _Const macro. But we should surround them with parenthesis when inside a non _Const macro
//       11. We should not mix names overlapping union members. For example we had a bug in Obb2 where we did { .center=center, .size=size, .rotation=rotation } which caused Center and Size to be zeroed out because Rotation is NOT in the same struct as Center and Size so it zeroes out all other members when set

// +--------------------------------------------------------------+
// |                   Typedefs and Structures                    |
// +--------------------------------------------------------------+
typedef car Vector2_R32 Vector2_R32;
car Vector2_R32
{
	r32 elem[2];
	plex { r32 x, y; };
	plex { r32 u, v; };
	plex { r32 width, height; };
	plex { r32 hori, vert; };
	#if LANGUAGE_IS_CPP
	inline r32& operator[](const int& elemIndex) { return elem[elemIndex]; }
	#endif
};
#define MakeV2_Const(X, Y)   { .x=(X), .y=(Y) }
#define MakeV2(x, y)         NEW_STRUCT(Vector2_R32)MakeV2_Const((x), (y))
#define FillV2_Const(value)  MakeV2_Const((value), (value))
#define FillV2(value)        MakeV2((value), (value))

typedef car Vector3_R32 Vector3_R32;
car Vector3_R32
{
	r32 elem[3];
	plex { r32 x, y, z; };
	plex { r32 u, v, w; };
	plex { r32 r, g, b; };
	plex { r32 red, green, blue; };
	plex { r32 width, height, depth; };
	plex { Vector2_R32 xy; r32 _unused1; };
	plex { r32 _unused2; Vector2_R32 yz; };
	plex { Vector2_R32 uv; r32 _unused3; };
	plex { r32 _unused4; Vector2_R32 vw; };
	plex { Vector2_R32 rg; r32 _unused5; };
	plex { r32 _unused6; Vector2_R32 gb; };
	#if LANGUAGE_IS_CPP
	inline r32& operator[](const int& elemIndex) { return elem[elemIndex]; }
	#endif
};
#define MakeV3_Const(X, Y, Z)  { .x=(X), .y=(Y), .z=(Z) }
#define MakeV3(x, y, z)        NEW_STRUCT(Vector3_R32)MakeV3_Const((x), (y), (z))
#define FillV3_Const(value)    MakeV3_Const((value), (value), (value))
#define FillV3(value)          MakeV3((value), (value), (value))

typedef car Vector4_R32 Vector4_R32;
car Vector4_R32
{
	r32 elem[4];
	plex { r32 x, y, z, w; };
	plex { r32 r, g, b, a; };
	plex { r32 red, green, blue, alpha; };
	plex { r32 width, height, depth, wDepth; };
	plex { r32 left, top, right, bottom; }; //NOTE: These aliases are mostly used for UI side parameters (like borderThickness and margins)
	plex { r32 topLeft, topRight, bottomRight, bottomLeft; }; //NOTE: These aliases are mostly used for UI cornerRadius
	plex { Vector3_R32 xyz; r32 _unused1; };
	plex { r32 _unused2; Vector3_R32 yzw; };
	plex { Vector2_R32 xy, zw; };
	plex { r32 _unused3; Vector2_R32 yz; r32 _unused4; };
	plex { Vector3_R32 rgb; r32 _unused5; };
	plex { r32 _unused6; Vector3_R32 gba; };
	plex { Vector2_R32 rg, ba; };
	plex { r32 _unused7; Vector2_R32 gb; r32 _unused8; };
	#if LANGUAGE_IS_CPP
	inline r32& operator[](const int& elemIndex) { return elem[elemIndex]; }
	#endif
};
#define MakeV4_Const(X, Y, Z, W)  { .x=(X), .y=(Y), .z=(Z), .w=(W) }
#define MakeV4(x, y, z, w)        NEW_STRUCT(Vector4_R32)MakeV4_Const((x), (y), (z), (w))
#define FillV4_Const(value)       MakeV4_Const((value), (value), (value), (value))
#define FillV4(value)             MakeV4((value), (value), (value), (value))

typedef car Vector2_I32 Vector2_I32;
car Vector2_I32
{
	i32 elem[2];
	plex { i32 x, y; };
	plex { i32 u, v; };
	plex { i32 width, height; };
	plex { i32 hori, vert; };
	#if LANGUAGE_IS_CPP
	inline i32& operator[](const int& elemIndex) { return elem[elemIndex]; }
	#endif
};
#define MakeV2i_Const(X, Y)   { .x=(X), .y=(Y) }
#define MakeV2i(x, y)         NEW_STRUCT(Vector2_I32)MakeV2i_Const((x), (y))
#define FillV2i_Const(value)  MakeV2i_Const((value), (value))
#define FillV2i(value)        MakeV2i((value), (value))

typedef car Vector3_I32 Vector3_I32;
car Vector3_I32
{
	i32 elem[3];
	plex { i32 x, y, z; };
	plex { i32 u, v, w; };
	plex { i32 r, g, b; };
	plex { i32 red, green, blue; };
	plex { i32 width, height, depth; };
	plex { Vector2_I32 xy; i32 _unused1; };
	plex { i32 _unused2; Vector2_I32 yz; };
	plex { Vector2_I32 uv; i32 _unused3; };
	plex { i32 _unused4; Vector2_I32 vw; };
	plex { Vector2_I32 rg; i32 _unused5; };
	plex { i32 _unused6; Vector2_I32 gb; };
	#if LANGUAGE_IS_CPP
	inline i32& operator[](const int& elemIndex) { return elem[elemIndex]; }
	#endif
};
#define MakeV3i_Const(X, Y, Z)  { .x=(X), .y=(Y), .z=(Z) }
#define MakeV3i(x, y, z)        NEW_STRUCT(Vector3_I32)MakeV3i_Const((x), (y), (z))
#define FillV3i_Const(value)    MakeV3i_Const((value), (value), (value))
#define FillV3i(value)          MakeV3i((value), (value), (value))

typedef car Vector4_I32 Vector4_I32;
car Vector4_I32
{
	i32 elem[4];
	plex { i32 x, y, z, w; };
	plex { i32 r, g, b, a; };
	plex { i32 red, green, blue, alpha; };
	plex { i32 width, height, depth, wDepth; };
	plex { Vector3_I32 xyz; i32 _unused1; };
	plex { i32 _unused2; Vector3_I32 yzw; };
	plex { Vector2_I32 xy, zw; };
	plex { i32 _unused3; Vector2_I32 yz; i32 _unused4; };
	plex { Vector3_I32 rgb; i32 _unused5; };
	plex { i32 _unused6; Vector3_I32 gba; };
	plex { Vector2_I32 rg, ba; };
	plex { i32 _unused7; Vector2_I32 gb; i32 _unused8; };
	#if LANGUAGE_IS_CPP
	inline i32& operator[](const int& elemIndex) { return elem[elemIndex]; }
	#endif
};
#define MakeV4i_Const(X, Y, Z, W)  { .x=(X), .y=(Y), .z=(Z), .w=(W) }
#define MakeV4i(x, y, z, w)        NEW_STRUCT(Vector4_I32)MakeV4i_Const((x), (y), (z), (w))
#define FillV4i_Const(value)       MakeV4i_Const((value), (value), (value), (value))
#define FillV4i(value)             MakeV4i((value), (value), (value), (value))

typedef car Vector2_R64 Vector2_R64;
car Vector2_R64
{
	r64 elem[2];
	plex { r64 x, y; };
	plex { r64 u, v; };
	plex { r64 width, height; };
	plex { r64 hori, vert; };
	plex { r64 lon, lat; };
	plex { r64 longitude, latitude; };
	#if LANGUAGE_IS_CPP
	inline r64& operator[](const int& elemIndex) { return elem[elemIndex]; }
	#endif
};
#define MakeV2d_Const(X, Y)   { .x=(X), .y=(Y) }
#define MakeV2d(x, y)         NEW_STRUCT(Vector2_R64)MakeV2d_Const((x), (y))
#define FillV2d_Const(value)  MakeV2d_Const((value), (value))
#define FillV2d(value)        MakeV2d((value), (value))

typedef car Vector3_R64 Vector3_R64;
car Vector3_R64
{
	r64 elem[3];
	plex { r64 x, y, z; };
	plex { r64 u, v, w; };
	plex { r64 r, g, b; };
	plex { r64 red, green, blue; };
	plex { r64 width, height, depth; };
	plex { Vector2_R64 xy; r64 _unused1; };
	plex { r64 _unused2; Vector2_R64 yz; };
	plex { Vector2_R64 uv; r64 _unused3; };
	plex { r64 _unused4; Vector2_R64 vw; };
	plex { Vector2_R64 rg; r64 _unused5; };
	plex { r64 _unused6; Vector2_R64 gb; };
	#if LANGUAGE_IS_CPP
	inline r64& operator[](const int& elemIndex) { return elem[elemIndex]; }
	#endif
};
#define MakeV3d_Const(X, Y, Z)  { .x=(X), .y=(Y), .z=(Z) }
#define MakeV3d(x, y, z)        NEW_STRUCT(Vector3_R64)MakeV3d_Const((x), (y), (z))
#define FillV3d_Const(value)    MakeV3d_Const((value), (value), (value))
#define FillV3d(value)          MakeV3d((value), (value), (value))

typedef car Vector4_R64 Vector4_R64;
car Vector4_R64
{
	r64 elem[4];
	plex { r64 x, y, z, w; };
	plex { r64 r, g, b, a; };
	plex { r64 red, green, blue, alpha; };
	plex { r64 width, height, depth, wDepth; };
	plex { Vector3_R64 xyz; r64 _unused1; };
	plex { r64 _unused2; Vector3_R64 yzw; };
	plex { Vector2_R64 xy, zw; };
	plex { r64 _unused3; Vector2_R64 yz; r64 _unused4; };
	plex { Vector3_R64 rgb; r64 _unused5; };
	plex { r64 _unused6; Vector3_R64 gba; };
	plex { Vector2_R64 rg, ba; };
	plex { r64 _unused7; Vector2_R64 gb; r64 _unused8; };
	#if LANGUAGE_IS_CPP
	inline r64& operator[](const int& elemIndex) { return elem[elemIndex]; }
	#endif
};
#define MakeV4d_Const(X, Y, Z, W)  { .x=(X), .y=(Y), .z=(Z), .w=(W) }
#define MakeV4d(x, y, z, w)        NEW_STRUCT(Vector4_R64)MakeV4d_Const((x), (y), (z), (w))
#define FillV4d_Const(value)       MakeV4d_Const((value), (value), (value), (value))
#define FillV4d(value)             MakeV4d((value), (value), (value), (value))

//NOTE: The default assumption for vectors is r32 members
//    'i' suffix refers to "integer" members and implicitly means 32-bit (unsigned and 64-bit integers are not yet supported)
//    'd' suffix refers to "double" members which are 64-bit (we went with a 'd' suffix instead of something like 'r64' because a single character suffix reads a lot nicer and "double" is a well known word to refer to 64-bit floating point numbers)
typedef Vector2_R32 v2;
typedef Vector2_I32 v2i;
typedef Vector2_R64 v2d;

typedef Vector3_R32 v3;
typedef Vector3_I32 v3i;
typedef Vector3_R64 v3d;

typedef Vector4_R32 v4;
typedef Vector4_I32 v4i;
typedef Vector4_R64 v4d;

// +--------------------------------------------------------------+
// |                      Helper Slice Types                      |
// +--------------------------------------------------------------+
typedef plex Vec2Slice Vec2Slice;
plex Vec2Slice
{
	uxx length;
	car { void* pntr; r32 components; v2* vectors; };
};
typedef plex Vec2iSlice Vec2iSlice;
plex Vec2iSlice
{
	uxx length;
	car { void* pntr; i32 components; v2i* vectors; };
};
typedef plex Vec2R64Slice Vec2R64Slice;
plex Vec2R64Slice
{
	uxx length;
	car { void* pntr; r64 components; v2d* vectors; };
};
typedef plex Vec3Slice Vec3Slice;
plex Vec3Slice
{
	uxx length;
	car { void* pntr; r32 components; v3* vectors; };
};
typedef plex Vec3iSlice Vec3iSlice;
plex Vec3iSlice
{
	uxx length;
	car { void* pntr; i32 components; v3i* vectors; };
};
typedef plex Vec3R64Slice Vec3R64Slice;
plex Vec3R64Slice
{
	uxx length;
	car { void* pntr; r64 components; v3d* vectors; };
};
typedef plex Vec4Slice Vec4Slice;
plex Vec4Slice
{
	uxx length;
	car { void* pntr; r32 components; v4* vectors; };
};
typedef plex Vec4iSlice Vec4iSlice;
plex Vec4iSlice
{
	uxx length;
	car { void* pntr; i32 components; v4i* vectors; };
};
typedef plex Vec4R64Slice Vec4R64Slice;
plex Vec4R64Slice
{
	uxx length;
	car { void* pntr; r64 components; v4d* vectors; };
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE v2 ToV2Fromi(v2i vec2i);
	PIG_CORE_INLINE v2d ToV2dFromi(v2i vec2i);
	PIG_CORE_INLINE v2d ToV2dFromf(v2 vec2);
	PIG_CORE_INLINE v2 ToV2Fromd(v2d vec2d);
	PIG_CORE_INLINE v2 ToV2From3(v3 vec3);
	PIG_CORE_INLINE v2 ToV2From3XZ(v3 vec3);
	PIG_CORE_INLINE v2 ToV2From3YZ(v3 vec3);
	PIG_CORE_INLINE v2i ToV2iFrom3(v3i vec3i);
	PIG_CORE_INLINE v2i ToV2iFrom3XZ(v3i vec3i);
	PIG_CORE_INLINE v2i ToV2iFrom3YZ(v3i vec3i);
	PIG_CORE_INLINE v3 ToV3Fromi(v3i vec3i);
	PIG_CORE_INLINE v3d ToV3dFromi(v3i vec3i);
	PIG_CORE_INLINE v3d ToV3dFromf(v3 vec3);
	PIG_CORE_INLINE v3 ToV3Fromd(v3d vec3d);
	PIG_CORE_INLINE v3 ToV3From2(v2 vec2, r32 z);
	PIG_CORE_INLINE v3 ToV3From2XZ(v2 vec2, r32 y);
	PIG_CORE_INLINE v3 ToV3From4(v4 vec4);
	PIG_CORE_INLINE v3i ToV3iFrom2(v2i vec2i, i32 z);
	PIG_CORE_INLINE v3i ToV3iFrom2XZ(v2i vec2i, i32 y);
	PIG_CORE_INLINE v3i ToV3iFrom4(v4i vec4i);
	PIG_CORE_INLINE v4 ToV4Fromi(v4i vec4i);
	PIG_CORE_INLINE v4d ToV4dFromi(v4i vec4i);
	PIG_CORE_INLINE v4d ToV4dFromf(v4 vec4);
	PIG_CORE_INLINE v4 ToV4Fromd(v4d vec4d);
	PIG_CORE_INLINE v4 ToV4From3(v3 vec3, r32 w);
	PIG_CORE_INLINE v4i ToV4iFrom3(v3i vec3i, i32 w);
	#if TARGET_IS_ORCA
	PIG_CORE_INLINE oc_vec2 ToOcVec2(v2 vector);
	PIG_CORE_INLINE v2 ToV2FromOc(oc_vec2 orcaVector);
	PIG_CORE_INLINE oc_vec3 ToOcVec3(v3 vector);
	PIG_CORE_INLINE v3 ToV3FromOc(oc_vec3 orcaVector);
	PIG_CORE_INLINE oc_vec4 ToOcVec4(v4 vector);
	PIG_CORE_INLINE v4 ToV4FromOc(oc_vec4 orcaVector);
	PIG_CORE_INLINE oc_vec2i ToOcVec2i(v2i vector);
	PIG_CORE_INLINE v2i ToV2iFromOc(oc_vec2i orcaVector);
	#endif //TARGET_IS_ORCA
	PIG_CORE_INLINE v2 AddV2(v2 left, v2 right);
	PIG_CORE_INLINE v3 AddV3(v3 left, v3 right);
	PIG_CORE_INLINE v4 AddV4(v4 left, v4 right);
	PIG_CORE_INLINE v2i AddV2i(v2i left, v2i right);
	PIG_CORE_INLINE v3i AddV3i(v3i left, v3i right);
	PIG_CORE_INLINE v4i AddV4i(v4i left, v4i right);
	PIG_CORE_INLINE v2d AddV2d(v2d left, v2d right);
	PIG_CORE_INLINE v3d AddV3d(v3d left, v3d right);
	PIG_CORE_INLINE v4d AddV4d(v4d left, v4d right);
	PIG_CORE_INLINE v2 SubV2(v2 left, v2 right);
	PIG_CORE_INLINE v3 SubV3(v3 left, v3 right);
	PIG_CORE_INLINE v4 SubV4(v4 left, v4 right);
	PIG_CORE_INLINE v2i SubV2i(v2i left, v2i right);
	PIG_CORE_INLINE v3i SubV3i(v3i left, v3i right);
	PIG_CORE_INLINE v4i SubV4i(v4i left, v4i right);
	PIG_CORE_INLINE v2d SubV2d(v2d left, v2d right);
	PIG_CORE_INLINE v3d SubV3d(v3d left, v3d right);
	PIG_CORE_INLINE v4d SubV4d(v4d left, v4d right);
	PIG_CORE_INLINE v2 MulV2(v2 left, v2 right);
	PIG_CORE_INLINE v3 MulV3(v3 left, v3 right);
	PIG_CORE_INLINE v4 MulV4(v4 left, v4 right);
	PIG_CORE_INLINE v2i MulV2i(v2i left, v2i right);
	PIG_CORE_INLINE v3i MulV3i(v3i left, v3i right);
	PIG_CORE_INLINE v4i MulV4i(v4i left, v4i right);
	PIG_CORE_INLINE v2d MulV2d(v2d left, v2d right);
	PIG_CORE_INLINE v3d MulV3d(v3d left, v3d right);
	PIG_CORE_INLINE v4d MulV4d(v4d left, v4d right);
	PIG_CORE_INLINE v2 DivV2(v2 left, v2 right);
	PIG_CORE_INLINE v3 DivV3(v3 left, v3 right);
	PIG_CORE_INLINE v4 DivV4(v4 left, v4 right);
	PIG_CORE_INLINE v2i DivV2i(v2i left, v2i right);
	PIG_CORE_INLINE v3i DivV3i(v3i left, v3i right);
	PIG_CORE_INLINE v4i DivV4i(v4i left, v4i right);
	PIG_CORE_INLINE v2d DivV2d(v2d left, v2d right);
	PIG_CORE_INLINE v3d DivV3d(v3d left, v3d right);
	PIG_CORE_INLINE v4d DivV4d(v4d left, v4d right);
	PIG_CORE_INLINE v2 ScaleV2(v2 left, r32 scalar);
	PIG_CORE_INLINE v3 ScaleV3(v3 left, r32 scalar);
	PIG_CORE_INLINE v4 ScaleV4(v4 left, r32 scalar);
	PIG_CORE_INLINE v2i ScaleV2i(v2i left, i32 scalar);
	PIG_CORE_INLINE v3i ScaleV3i(v3i left, i32 scalar);
	PIG_CORE_INLINE v4i ScaleV4i(v4i left, i32 scalar);
	PIG_CORE_INLINE v2d ScaleV2d(v2d left, r64 scalar);
	PIG_CORE_INLINE v3d ScaleV3d(v3d left, r64 scalar);
	PIG_CORE_INLINE v4d ScaleV4d(v4d left, r64 scalar);
	PIG_CORE_INLINE v2 ShrinkV2(v2 left, r32 divisor);
	PIG_CORE_INLINE v3 ShrinkV3(v3 left, r32 divisor);
	PIG_CORE_INLINE v4 ShrinkV4(v4 left, r32 divisor);
	PIG_CORE_INLINE v2i ShrinkV2i(v2i left, i32 divisor);
	PIG_CORE_INLINE v3i ShrinkV3i(v3i left, i32 divisor);
	PIG_CORE_INLINE v4i ShrinkV4i(v4i left, i32 divisor);
	PIG_CORE_INLINE v2d ShrinkV2d(v2d left, r64 divisor);
	PIG_CORE_INLINE v3d ShrinkV3d(v3d left, r64 divisor);
	PIG_CORE_INLINE v4d ShrinkV4d(v4d left, r64 divisor);
	PIG_CORE_INLINE bool AreEqualV2(v2 left, v2 right);
	PIG_CORE_INLINE bool AreEqualV3(v3 left, v3 right);
	PIG_CORE_INLINE bool AreEqualV4(v4 left, v4 right);
	PIG_CORE_INLINE bool AreEqualV2i(v2i left, v2i right);
	PIG_CORE_INLINE bool AreEqualV3i(v3i left, v3i right);
	PIG_CORE_INLINE bool AreEqualV4i(v4i left, v4i right);
	PIG_CORE_INLINE bool AreEqualV2d(v2d left, v2d right);
	PIG_CORE_INLINE bool AreEqualV3d(v3d left, v3d right);
	PIG_CORE_INLINE bool AreEqualV4d(v4d left, v4d right);
	PIG_CORE_INLINE r32 DotV2(v2 left, v2 right);
	PIG_CORE_INLINE r32 DotV3(v3 left, v3 right);
	PIG_CORE_INLINE r32 DotV4(v4 left, v4 right);
	PIG_CORE_INLINE i32 DotV2i(v2i left, v2i right);
	PIG_CORE_INLINE i32 DotV3i(v3i left, v3i right);
	PIG_CORE_INLINE i32 DotV4i(v4i left, v4i right);
	PIG_CORE_INLINE r64 DotV2d(v2d left, v2d right);
	PIG_CORE_INLINE r64 DotV3d(v3d left, v3d right);
	PIG_CORE_INLINE r64 DotV4d(v4d left, v4d right);
	PIG_CORE_INLINE v3 CrossV3(v3 left, v3 right);
	PIG_CORE_INLINE v3i CrossV3i(v3i left, v3i right);
	PIG_CORE_INLINE v3d CrossV3d(v3d left, v3d right);
	PIG_CORE_INLINE r32 LengthSquaredV2(v2 vec2);
	PIG_CORE_INLINE r32 LengthSquaredV3(v3 vec3);
	PIG_CORE_INLINE r32 LengthSquaredV4(v4 vec4);
	PIG_CORE_INLINE i32 LengthSquaredV2i(v2i vec2i);
	PIG_CORE_INLINE i32 LengthSquaredV3i(v3i vec3i);
	PIG_CORE_INLINE i32 LengthSquaredV4i(v4i vec4i);
	PIG_CORE_INLINE r64 LengthSquaredV2d(v2d vec2d);
	PIG_CORE_INLINE r64 LengthSquaredV3d(v3d vec3d);
	PIG_CORE_INLINE r64 LengthSquaredV4d(v4d vec4d);
	PIG_CORE_INLINE r32 LengthV2(v2 vec2);
	PIG_CORE_INLINE r32 LengthV3(v3 vec3);
	PIG_CORE_INLINE r32 LengthV4(v4 vec4);
	PIG_CORE_INLINE r32 LengthV2i(v2i vec2i);
	PIG_CORE_INLINE r32 LengthV3i(v3i vec3i);
	PIG_CORE_INLINE r32 LengthV4i(v4i vec4i);
	PIG_CORE_INLINE r64 LengthV2d(v2d vec2d);
	PIG_CORE_INLINE r64 LengthV3d(v3d vec3d);
	PIG_CORE_INLINE r64 LengthV4d(v4d vec4d);
	PIG_CORE_INLINE v2 NormalizeV2(v2 vec2);
	PIG_CORE_INLINE v3 NormalizeV3(v3 vec3);
	PIG_CORE_INLINE v4 NormalizeV4(v4 vec4);
	PIG_CORE_INLINE v2d NormalizeV2d(v2d vec2d);
	PIG_CORE_INLINE v3d NormalizeV3d(v3d vec2d);
	PIG_CORE_INLINE v4d NormalizeV4d(v4d vec2d);
	PIG_CORE_INLINE v2 LerpV2(v2 start, v2 end, r32 amount);
	PIG_CORE_INLINE v3 LerpV3(v3 start, v3 end, r32 amount);
	PIG_CORE_INLINE v4 LerpV4(v4 start, v4 end, r32 amount);
	PIG_CORE_INLINE v2d LerpV2d(v2d start, v2d end, r64 amount);
	PIG_CORE_INLINE v3d LerpV3d(v3d start, v3d end, r64 amount);
	PIG_CORE_INLINE v4d LerpV4d(v4d start, v4d end, r64 amount);
	PIG_CORE_INLINE v2 FloorV2(v2 vec2);
	PIG_CORE_INLINE v3 FloorV3(v3 vec3);
	PIG_CORE_INLINE v4 FloorV4(v4 vec4);
	PIG_CORE_INLINE v2i FloorV2i(v2 vec2);
	PIG_CORE_INLINE v3i FloorV3i(v3 vec3);
	PIG_CORE_INLINE v4i FloorV4i(v4 vec4);
	PIG_CORE_INLINE v2d FloorV2d(v2d vec2d);
	PIG_CORE_INLINE v3d FloorV3d(v3d vec3d);
	PIG_CORE_INLINE v4d FloorV4d(v4d vec4d);
	PIG_CORE_INLINE v2i FloorV2di(v2d vec2d);
	PIG_CORE_INLINE v3i FloorV3di(v3d vec3d);
	PIG_CORE_INLINE v4i FloorV4di(v4d vec4d);
	PIG_CORE_INLINE v2 CeilV2(v2 vec2);
	PIG_CORE_INLINE v3 CeilV3(v3 vec3);
	PIG_CORE_INLINE v4 CeilV4(v4 vec4);
	PIG_CORE_INLINE v2i CeilV2i(v2 vec2);
	PIG_CORE_INLINE v3i CeilV3i(v3 vec3);
	PIG_CORE_INLINE v4i CeilV4i(v4 vec4);
	PIG_CORE_INLINE v2d CeilV2d(v2d vec2d);
	PIG_CORE_INLINE v3d CeilV3d(v3d vec3d);
	PIG_CORE_INLINE v4d CeilV4d(v4d vec4d);
	PIG_CORE_INLINE v2i CeilV2di(v2d vec2d);
	PIG_CORE_INLINE v3i CeilV3di(v3d vec3d);
	PIG_CORE_INLINE v4i CeilV4di(v4d vec4d);
	PIG_CORE_INLINE v2 RoundV2(v2 vec2);
	PIG_CORE_INLINE v3 RoundV3(v3 vec3);
	PIG_CORE_INLINE v4 RoundV4(v4 vec4);
	PIG_CORE_INLINE v2i RoundV2i(v2 vec2);
	PIG_CORE_INLINE v3i RoundV3i(v3 vec3);
	PIG_CORE_INLINE v4i RoundV4i(v4 vec4);
	PIG_CORE_INLINE v2d RoundV2d(v2d vec2d);
	PIG_CORE_INLINE v3d RoundV3d(v3d vec3d);
	PIG_CORE_INLINE v4d RoundV4d(v4d vec4d);
	PIG_CORE_INLINE v2i RoundV2di(v2d vec2d);
	PIG_CORE_INLINE v3i RoundV3di(v3d vec3d);
	PIG_CORE_INLINE v4i RoundV4di(v4d vec4d);
	PIG_CORE_INLINE v2 MinV2(v2 left, v2 right);
	PIG_CORE_INLINE v3 MinV3(v3 left, v3 right);
	PIG_CORE_INLINE v4 MinV4(v4 left, v4 right);
	PIG_CORE_INLINE v2d MinV2d(v2d left, v2d right);
	PIG_CORE_INLINE v3d MinV3d(v3d left, v3d right);
	PIG_CORE_INLINE v4d MinV4d(v4d left, v4d right);
	PIG_CORE_INLINE v2 MaxV2(v2 left, v2 right);
	PIG_CORE_INLINE v3 MaxV3(v3 left, v3 right);
	PIG_CORE_INLINE v4 MaxV4(v4 left, v4 right);
	PIG_CORE_INLINE v2d MaxV2d(v2d left, v2d right);
	PIG_CORE_INLINE v3d MaxV3d(v3d left, v3d right);
	PIG_CORE_INLINE v4d MaxV4d(v4d left, v4d right);
	PIG_CORE_INLINE v2 AbsV2(v2 vec2);
	PIG_CORE_INLINE v3 AbsV3(v3 vec3);
	PIG_CORE_INLINE v4 AbsV4(v4 vec4);
	PIG_CORE_INLINE v2i AbsV2i(v2i vec2i);
	PIG_CORE_INLINE v3i AbsV3i(v3i vec3i);
	PIG_CORE_INLINE v4i AbsV4i(v4i vec4i);
	PIG_CORE_INLINE v2d AbsV2d(v2d vec2d);
	PIG_CORE_INLINE v3d AbsV3d(v3d vec3d);
	PIG_CORE_INLINE v4d AbsV4d(v4d vec4d);
	PIG_CORE_INLINE v2 ClampV2(v2 vec2, v2 minVec2, v2 maxVec2);
	PIG_CORE_INLINE v3 ClampV3(v3 vec3, v3 minVec3, v3 maxVec3);
	PIG_CORE_INLINE v4 ClampV4(v4 vec4, v4 minVec4, v4 maxVec4);
	PIG_CORE_INLINE v2i ClampV2i(v2i vec2i, v2i minVec2i, v2i maxVec2i);
	PIG_CORE_INLINE v3i ClampV3i(v3i vec3i, v3i minVec3i, v3i maxVec3i);
	PIG_CORE_INLINE v4i ClampV4i(v4i vec4i, v4i minVec4i, v4i maxVec4i);
	PIG_CORE_INLINE v2d ClampV2d(v2d vec2d, v2d minVec2d, v2d maxVec2d);
	PIG_CORE_INLINE v3d ClampV3d(v3d vec3d, v3d minVec3d, v3d maxVec3d);
	PIG_CORE_INLINE v4d ClampV4d(v4d vec4d, v4d minVec4d, v4d maxVec4d);
	PIG_CORE_INLINE v2 PerpRightV2(v2 vec2);
	PIG_CORE_INLINE v2 PerpLeftV2(v2 vec2);
	PIG_CORE_INLINE v2d PerpRightV2d(v2d vec2d);
	PIG_CORE_INLINE v2d PerpLeftV2d(v2d vec2d);
	PIG_CORE_INLINE v2 Vec2Rotate(v2 vec2, r32 angle);
	PIG_CORE_INLINE v2d Vec2dRotate(v2d vec2d, r64 angle);
	PIG_CORE_INLINE r32 AngleBetweenV2(v2 left, v2 right);
	PIG_CORE_INLINE r64 AngleBetweenV2d(v2d left, v2d right);
	PIG_CORE_INLINE r32 AngleBetweenV3(v3 left, v3 right);
	PIG_CORE_INLINE r64 AngleBetweenV3d(v3d left, v3d right);
	PIG_CORE_INLINE v3 Vec3From2Angles(r32 facingDirection, r32 rotationUpDown, r32 radius);
	PIG_CORE_INLINE v3d Vec3dFrom2Angles(r64 facingDirection, r64 rotationUpDown, r64 radius);
	bool IsInsideTriangleV2(v2 p0, v2 p1, v2 p2, v2 queryPos);
	r32 DistanceToTriangleEdgeV2(v2 p0, v2 p1, v2 p2, v2 queryPos);
	PIG_CORE_INLINE void AlignV2ToV2(v2* vectorOut, v2 alignmentScale);
	PIG_CORE_INLINE void AlignV2To(v2* vectorOut, r32 alignmentScale);
	PIG_CORE_INLINE void AlignV2(v2* vectorOut);
	PIG_CORE_INLINE void AlignV2dToV2d(v2d* vectorOut, v2d alignmentScale);
	PIG_CORE_INLINE void AlignV2dTo(v2d* vectorOut, r64 alignmentScale);
	PIG_CORE_INLINE void AlignV2d(v2d* vectorOut);
	PIG_CORE_INLINE void AlignV3ToV3(v3* vectorOut, v3 alignmentScale);
	PIG_CORE_INLINE void AlignV3To(v3* vectorOut, r32 alignmentScale);
	PIG_CORE_INLINE void AlignV3(v3* vectorOut);
	PIG_CORE_INLINE void AlignV3dToV3d(v3d* vectorOut, v3d alignmentScale);
	PIG_CORE_INLINE void AlignV3dTo(v3d* vectorOut, r64 alignmentScale);
	PIG_CORE_INLINE void AlignV3d(v3d* vectorOut);
	PIG_CORE_INLINE void AlignV4ToV4(v4* vectorOut, v4 alignmentScale);
	PIG_CORE_INLINE void AlignV4To(v4* vectorOut, r32 alignmentScale);
	PIG_CORE_INLINE void AlignV4(v4* vectorOut);
	PIG_CORE_INLINE void AlignV4dToV4d(v4d* vectorOut, v4d alignmentScale);
	PIG_CORE_INLINE void AlignV4dTo(v4d* vectorOut, r64 alignmentScale);
	PIG_CORE_INLINE void AlignV4d(v4d* vectorOut);
	#if LANGUAGE_IS_CPP
	PIG_CORE_INLINE v2  Add(v2  left, v2  right);
	PIG_CORE_INLINE v3  Add(v3  left, v3  right);
	PIG_CORE_INLINE v4  Add(v4  left, v4  right);
	PIG_CORE_INLINE v2i Add(v2i left, v2i right);
	PIG_CORE_INLINE v3i Add(v3i left, v3i right);
	PIG_CORE_INLINE v4i Add(v4i left, v4i right);
	PIG_CORE_INLINE v2d Add(v2d left, v2d right);
	PIG_CORE_INLINE v3d Add(v3d left, v3d right);
	PIG_CORE_INLINE v4d Add(v4d left, v4d right);
	PIG_CORE_INLINE v2  Sub(v2  left, v2  right);
	PIG_CORE_INLINE v3  Sub(v3  left, v3  right);
	PIG_CORE_INLINE v4  Sub(v4  left, v4  right);
	PIG_CORE_INLINE v2i Sub(v2i left, v2i right);
	PIG_CORE_INLINE v3i Sub(v3i left, v3i right);
	PIG_CORE_INLINE v4i Sub(v4i left, v4i right);
	PIG_CORE_INLINE v2d Sub(v2d left, v2d right);
	PIG_CORE_INLINE v3d Sub(v3d left, v3d right);
	PIG_CORE_INLINE v4d Sub(v4d left, v4d right);
	PIG_CORE_INLINE v2  Mul(v2  left, v2  right);
	PIG_CORE_INLINE v3  Mul(v3  left, v3  right);
	PIG_CORE_INLINE v4  Mul(v4  left, v4  right);
	PIG_CORE_INLINE v2i Mul(v2i left, v2i right);
	PIG_CORE_INLINE v3i Mul(v3i left, v3i right);
	PIG_CORE_INLINE v4i Mul(v4i left, v4i right);
	PIG_CORE_INLINE v2d Mul(v2d left, v2d right);
	PIG_CORE_INLINE v3d Mul(v3d left, v3d right);
	PIG_CORE_INLINE v4d Mul(v4d left, v4d right);
	PIG_CORE_INLINE v2  Div(v2  left, v2  right);
	PIG_CORE_INLINE v3  Div(v3  left, v3  right);
	PIG_CORE_INLINE v4  Div(v4  left, v4  right);
	PIG_CORE_INLINE v2i Div(v2i left, v2i right);
	PIG_CORE_INLINE v3i Div(v3i left, v3i right);
	PIG_CORE_INLINE v4i Div(v4i left, v4i right);
	PIG_CORE_INLINE v2d Div(v2d left, v2d right);
	PIG_CORE_INLINE v3d Div(v3d left, v3d right);
	PIG_CORE_INLINE v4d Div(v4d left, v4d right);
	PIG_CORE_INLINE v2  Mul(v2  vector, r32 scalar);
	PIG_CORE_INLINE v3  Mul(v3  vector, r32 scalar);
	PIG_CORE_INLINE v4  Mul(v4  vector, r32 scalar);
	PIG_CORE_INLINE v2i Mul(v2i vector, i32 scalar);
	PIG_CORE_INLINE v3i Mul(v3i vector, i32 scalar);
	PIG_CORE_INLINE v4i Mul(v4i vector, i32 scalar);
	PIG_CORE_INLINE v2d Mul(v2d vector, r64 scalar);
	PIG_CORE_INLINE v3d Mul(v3d vector, r64 scalar);
	PIG_CORE_INLINE v4d Mul(v4d vector, r64 scalar);
	PIG_CORE_INLINE v2  Div(v2  vector, r32 scalar);
	PIG_CORE_INLINE v3  Div(v3  vector, r32 scalar);
	PIG_CORE_INLINE v4  Div(v4  vector, r32 scalar);
	PIG_CORE_INLINE v2i Div(v2i vector, i32 scalar);
	PIG_CORE_INLINE v3i Div(v3i vector, i32 scalar);
	PIG_CORE_INLINE v4i Div(v4i vector, i32 scalar);
	PIG_CORE_INLINE v2d Div(v2d vector, r64 scalar);
	PIG_CORE_INLINE v3d Div(v3d vector, r64 scalar);
	PIG_CORE_INLINE v4d Div(v4d vector, r64 scalar);
	PIG_CORE_INLINE r32 LengthSquared(v2  vector);
	PIG_CORE_INLINE r32 LengthSquared(v3  vector);
	PIG_CORE_INLINE r32 LengthSquared(v4  vector);
	PIG_CORE_INLINE i32 LengthSquared(v2i vector);
	PIG_CORE_INLINE i32 LengthSquared(v3i vector);
	PIG_CORE_INLINE i32 LengthSquared(v4i vector);
	PIG_CORE_INLINE r64 LengthSquared(v2d vector);
	PIG_CORE_INLINE r64 LengthSquared(v3d vector);
	PIG_CORE_INLINE r64 LengthSquared(v4d vector);
	PIG_CORE_INLINE r32 Length(v2  vector);
	PIG_CORE_INLINE r32 Length(v3  vector);
	PIG_CORE_INLINE r32 Length(v4  vector);
	PIG_CORE_INLINE r32 Length(v2i vector);
	PIG_CORE_INLINE r32 Length(v3i vector);
	PIG_CORE_INLINE r32 Length(v4i vector);
	PIG_CORE_INLINE r64 Length(v2d vector);
	PIG_CORE_INLINE r64 Length(v3d vector);
	PIG_CORE_INLINE r64 Length(v4d vector);
	PIG_CORE_INLINE v2  Normalize(v2  vector);
	PIG_CORE_INLINE v3  Normalize(v3  vector);
	PIG_CORE_INLINE v4  Normalize(v4  vector);
	PIG_CORE_INLINE v2d Normalize(v2d vector);
	PIG_CORE_INLINE v3d Normalize(v3d vector);
	PIG_CORE_INLINE v4d Normalize(v4d vector);
	PIG_CORE_INLINE r32 Dot(v2  left, v2  right);
	PIG_CORE_INLINE r32 Dot(v3  left, v3  right);
	PIG_CORE_INLINE r32 Dot(v4  left, v4  right);
	PIG_CORE_INLINE i32 Dot(v2i left, v2i right);
	PIG_CORE_INLINE i32 Dot(v3i left, v3i right);
	PIG_CORE_INLINE i32 Dot(v4i left, v4i right);
	PIG_CORE_INLINE r64 Dot(v2d left, v2d right);
	PIG_CORE_INLINE r64 Dot(v3d left, v3d right);
	PIG_CORE_INLINE r64 Dot(v4d left, v4d right);
	PIG_CORE_INLINE v2  Lerp(v2  left, v2  right, r32 amount);
	PIG_CORE_INLINE v3  Lerp(v3  left, v3  right, r32 amount);
	PIG_CORE_INLINE v4  Lerp(v4  left, v4  right, r32 amount);
	PIG_CORE_INLINE v2d Lerp(v2d left, v2d right, r32 amount);
	PIG_CORE_INLINE v3d Lerp(v3d left, v3d right, r32 amount);
	PIG_CORE_INLINE v4d Lerp(v4d left, v4d right, r32 amount);
	PIG_CORE_INLINE bool AreEqual(v2  left, v2  right);
	PIG_CORE_INLINE bool AreEqual(v3  left, v3  right);
	PIG_CORE_INLINE bool AreEqual(v4  left, v4  right);
	PIG_CORE_INLINE bool AreEqual(v2i left, v2i right);
	PIG_CORE_INLINE bool AreEqual(v3i left, v3i right);
	PIG_CORE_INLINE bool AreEqual(v4i left, v4i right);
	PIG_CORE_INLINE bool AreEqual(v2d left, v2d right);
	PIG_CORE_INLINE bool AreEqual(v3d left, v3d right);
	PIG_CORE_INLINE bool AreEqual(v4d left, v4d right);
	#endif //LANGUAGE_IS_CPP
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define COORD2D_FROM_INDEX(index, arrayWidth, arrayHeight) MakeV2i( \
	COORD2D_X_FROM_INDEX((index), (arrayWidth), (arrayHeight)),     \
	COORD2D_Y_FROM_INDEX((index), (arrayWidth), (arrayHeight))      \
)
#define COORD3D_FROM_INDEX(voxelIndex, arrayWidth, arrayHeight, arrayDepth) MakeV3i( \
	(i32)COORD3D_X_FROM_INDEX(voxelIndex, arrayWidth),                               \
	(i32)COORD3D_Y_FROM_INDEX(voxelIndex, arrayWidth, arrayDepth),                   \
	(i32)COORD3D_Z_FROM_INDEX(voxelIndex, arrayWidth, arrayHeight, arrayDepth)       \
)

#define AreSimilarV2(left, right, tolerance) (AreSimilarR32((left).x, (right).x, (tolerance)) && AreSimilarR32((left).y, (right).y, (tolerance)))
#define AreSimilarV3(left, right, tolerance) (AreSimilarR32((left).x, (right).x, (tolerance)) && AreSimilarR32((left).y, (right).y, (tolerance)) && AreSimilarR32((left).z, (right).z, (tolerance)))
#define AreSimilarV4(left, right, tolerance) (AreSimilarR32((left).x, (right).x, (tolerance)) && AreSimilarR32((left).y, (right).y, (tolerance)) && AreSimilarR32((left).z, (right).z, (tolerance)) && AreSimilarR32((left).w, (right).w, (tolerance)))
#define AreSimilarV2d(left, right, tolerance) (AreSimilarR64((left).x, (right).x, (tolerance)) && AreSimilarR64((left).y, (right).y, (tolerance)))
#define AreSimilarV3d(left, right, tolerance) (AreSimilarR64((left).x, (right).x, (tolerance)) && AreSimilarR64((left).y, (right).y, (tolerance)) && AreSimilarR64((left).z, (right).z, (tolerance)))
#define AreSimilarV4d(left, right, tolerance) (AreSimilarR64((left).x, (right).x, (tolerance)) && AreSimilarR64((left).y, (right).y, (tolerance)) && AreSimilarR64((left).z, (right).z, (tolerance)) && AreSimilarR64((left).w, (right).w, (tolerance)))

// +--------------------------------------------------------------+
// |                   Simple Value Definitions                   |
// +--------------------------------------------------------------+
#define V2_Zero_Const   MakeV2_Const(0.0f, 0.0f)
#define V2i_Zero_Const  MakeV2i_Const(0, 0)
#define V2d_Zero_Const  MakeV2d_Const(0.0, 0.0)
#define V3_Zero_Const   MakeV3_Const(0.0f, 0.0f, 0.0f)
#define V3i_Zero_Const  MakeV3i_Const(0, 0, 0)
#define V3d_Zero_Const  MakeV3d_Const(0.0, 0.0, 0.0)
#define V4_Zero_Const   MakeV4_Const(0.0f, 0.0f, 0.0f, 0.0f)
#define V4i_Zero_Const  MakeV4i_Const(0, 0, 0, 0)
#define V4d_Zero_Const  MakeV4d_Const(0.0, 0.0, 0.0, 0.0)

#define V2_Zero   MakeV2( 0.0f,  0.0f)
#define V2_One    MakeV2( 1.0f,  1.0f)
#define V2_Half   MakeV2( 0.5f,  0.5f)
#define V2_Left   MakeV2(-1.0f,  0.0f)
#define V2_Right  MakeV2( 1.0f,  0.0f)
#define V2_Up     MakeV2( 0.0f, -1.0f)
#define V2_Down   MakeV2( 0.0f,  1.0f)

#define V2i_Zero   MakeV2i( 0,  0)
#define V2i_One    MakeV2i( 1,  1)
#define V2i_Left   MakeV2i(-1,  0)
#define V2i_Right  MakeV2i( 1,  0)
#define V2i_Up     MakeV2i( 0, -1)
#define V2i_Down   MakeV2i( 0,  1)

#define V2d_Zero   MakeV2d( 0.0,  0.0)
#define V2d_One    MakeV2d( 1.0,  1.0)
#define V2d_Half   MakeV2d( 0.5,  0.5)
#define V2d_Left   MakeV2d(-1.0,  0.0)
#define V2d_Right  MakeV2d( 1.0,  0.0)
#define V2d_Up     MakeV2d( 0.0, -1.0)
#define V2d_Down   MakeV2d( 0.0,  1.0)

//NOTE: See struct_directions.h for an explanation of the coordinate system handed-ness and up direction
#define V3_Zero      MakeV3( 0.0f,  0.0f,  0.0f)
#define V3_One       MakeV3( 1.0f,  1.0f,  1.0f)
#define V3_Half      MakeV3( 0.5f,  0.5f,  0.5f)
#define V3_Left      MakeV3(-1.0f,  0.0f,  0.0f)
#define V3_Right     MakeV3( 1.0f,  0.0f,  0.0f)
#define V3_Bottom    MakeV3( 0.0f, -1.0f,  0.0f)
#define V3_Top       MakeV3( 0.0f,  1.0f,  0.0f)
#define V3_Back      MakeV3( 0.0f,  0.0f, -1.0f)
#define V3_Front     MakeV3( 0.0f,  0.0f,  1.0f)
#define V3_Down      MakeV3( 0.0f, -1.0f,  0.0f)
#define V3_Up        MakeV3( 0.0f,  1.0f,  0.0f)
#define V3_Backward  MakeV3( 0.0f,  0.0f, -1.0f)
#define V3_Forward   MakeV3( 0.0f,  0.0f,  1.0f)

#define V3i_Zero      MakeV3i( 0,  0,  0)
#define V3i_One       MakeV3i( 1,  1,  1)
#define V3i_Left      MakeV3i(-1,  0,  0)
#define V3i_Right     MakeV3i( 1,  0,  0)
#define V3i_Bottom    MakeV3i( 0, -1,  0)
#define V3i_Top       MakeV3i( 0,  1,  0)
#define V3i_Back      MakeV3i( 0,  0, -1)
#define V3i_Front     MakeV3i( 0,  0,  1)
#define V3i_Down      MakeV3i( 0, -1,  0)
#define V3i_Up        MakeV3i( 0,  1,  0)
#define V3i_Backward  MakeV3i( 0,  0, -1)
#define V3i_Forward   MakeV3i( 0,  0,  1)

#define V3d_Zero      MakeV3d( 0.0,  0.0,  0.0)
#define V3d_One       MakeV3d( 1.0,  1.0,  1.0)
#define V3d_Half      MakeV3d( 0.5,  0.5,  0.5)
#define V3d_Left      MakeV3d(-1.0,  0.0,  0.0)
#define V3d_Right     MakeV3d( 1.0,  0.0,  0.0)
#define V3d_Bottom    MakeV3d( 0.0, -1.0,  0.0)
#define V3d_Top       MakeV3d( 0.0,  1.0,  0.0)
#define V3d_Back      MakeV3d( 0.0,  0.0, -1.0)
#define V3d_Front     MakeV3d( 0.0,  0.0,  1.0)
#define V3d_Down      MakeV3d( 0.0, -1.0,  0.0)
#define V3d_Up        MakeV3d( 0.0,  1.0,  0.0)
#define V3d_Backward  MakeV3d( 0.0,  0.0, -1.0)
#define V3d_Forward   MakeV3d( 0.0,  0.0,  1.0)

#define V4_Zero      MakeV4( 0.0f,  0.0f,  0.0f,  0.0f)
#define V4_One       MakeV4( 1.0f,  1.0f,  1.0f,  1.0f)
#define V4_Half      MakeV4( 0.5f,  0.5f,  0.5f,  0.5f)
#define V4_Left      MakeV4(-1.0f,  0.0f,  0.0f,  0.0f)
#define V4_Right     MakeV4( 1.0f,  0.0f,  0.0f,  0.0f)
#define V4_Bottom    MakeV4( 0.0f, -1.0f,  0.0f,  0.0f)
#define V4_Top       MakeV4( 0.0f,  1.0f,  0.0f,  0.0f)
#define V4_Back      MakeV4( 0.0f,  0.0f, -1.0f,  0.0f)
#define V4_Front     MakeV4( 0.0f,  0.0f,  1.0f,  0.0f)
#define V4_Down      MakeV4( 0.0f, -1.0f,  0.0f,  0.0f)
#define V4_Up        MakeV4( 0.0f,  1.0f,  0.0f,  0.0f)
#define V4_Backward  MakeV4( 0.0f,  0.0f, -1.0f,  0.0f)
#define V4_Forward   MakeV4( 0.0f,  0.0f,  1.0f,  0.0f)
#define V4_Prevward  MakeV4( 0.0f,  0.0f,  0.0f, -1.0f)
#define V4_Nextward  MakeV4( 0.0f,  0.0f,  0.0f,  1.0f)

#define V4i_Zero      MakeV4i( 0,  0,  0,  0)
#define V4i_One       MakeV4i( 1,  1,  1,  0)
#define V4i_Left      MakeV4i(-1,  0,  0,  0)
#define V4i_Right     MakeV4i( 1,  0,  0,  0)
#define V4i_Bottom    MakeV4i( 0, -1,  0,  0)
#define V4i_Top       MakeV4i( 0,  1,  0,  0)
#define V4i_Back      MakeV4i( 0,  0, -1,  0)
#define V4i_Front     MakeV4i( 0,  0,  1,  0)
#define V4i_Down      MakeV4i( 0, -1,  0,  0)
#define V4i_Up        MakeV4i( 0,  1,  0,  0)
#define V4i_Backward  MakeV4i( 0,  0, -1,  0)
#define V4i_Forward   MakeV4i( 0,  0,  1,  0)
#define V4i_Prevward  MakeV4i( 0,  0,  0, -1)
#define V4i_Nextward  MakeV4i( 0,  0,  0,  1)

#define V4d_Zero      MakeV4d( 0.0,  0.0,  0.0,  0.0)
#define V4d_One       MakeV4d( 1.0,  1.0,  1.0,  1.0)
#define V4d_Half      MakeV4d( 0.5,  0.5,  0.5,  0.5)
#define V4d_Left      MakeV4d(-1.0,  0.0,  0.0,  0.0)
#define V4d_Right     MakeV4d( 1.0,  0.0,  0.0,  0.0)
#define V4d_Bottom    MakeV4d( 0.0, -1.0,  0.0,  0.0)
#define V4d_Top       MakeV4d( 0.0,  1.0,  0.0,  0.0)
#define V4d_Back      MakeV4d( 0.0,  0.0, -1.0,  0.0)
#define V4d_Front     MakeV4d( 0.0,  0.0,  1.0,  0.0)
#define V4d_Down      MakeV4d( 0.0, -1.0,  0.0,  0.0)
#define V4d_Up        MakeV4d( 0.0,  1.0,  0.0,  0.0)
#define V4d_Backward  MakeV4d( 0.0,  0.0, -1.0,  0.0)
#define V4d_Forward   MakeV4d( 0.0,  0.0,  1.0,  0.0)
#define V4d_Prevward  MakeV4d( 0.0,  0.0,  0.0, -1.0)
#define V4d_Nextward  MakeV4d( 0.0,  0.0,  0.0,  1.0)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                 Simple Conversions and Casts                 |
// +--------------------------------------------------------------+
PEXPI v2 ToV2Fromi(v2i vec2i) { return MakeV2((r32)vec2i.x, (r32)vec2i.y); }
PEXPI v2d ToV2dFromi(v2i vec2i) { return MakeV2d((r64)vec2i.x, (r64)vec2i.y); }
PEXPI v2d ToV2dFromf(v2 vec2) { return MakeV2d((r64)vec2.x, (r64)vec2.y); }
PEXPI v2 ToV2Fromd(v2d vec2d) { return MakeV2((r32)vec2d.x, (r32)vec2d.y); }
PEXPI v2 ToV2From3(v3 vec3) { return MakeV2(vec3.x, vec3.y); }
PEXPI v2 ToV2From3XZ(v3 vec3) { return MakeV2(vec3.x, vec3.z); }
PEXPI v2 ToV2From3YZ(v3 vec3) { return MakeV2(vec3.y, vec3.z); }
PEXPI v2i ToV2iFrom3(v3i vec3i) { return MakeV2i(vec3i.x, vec3i.y); }
PEXPI v2i ToV2iFrom3XZ(v3i vec3i) { return MakeV2i(vec3i.x, vec3i.z); }
PEXPI v2i ToV2iFrom3YZ(v3i vec3i) { return MakeV2i(vec3i.y, vec3i.z); }

PEXPI v3 ToV3Fromi(v3i vec3i) { return MakeV3((r32)vec3i.x, (r32)vec3i.y, (r32)vec3i.z); }
PEXPI v3d ToV3dFromi(v3i vec3i) { return MakeV3d((r64)vec3i.x, (r64)vec3i.y, (r64)vec3i.z); }
PEXPI v3d ToV3dFromf(v3 vec3) { return MakeV3d((r64)vec3.x, (r64)vec3.y, (r64)vec3.z); }
PEXPI v3 ToV3Fromd(v3d vec3d) { return MakeV3((r32)vec3d.x, (r32)vec3d.y, (r32)vec3d.z); }
PEXPI v3 ToV3From2(v2 vec2, r32 z) { return MakeV3(vec2.x, vec2.y, z); }
PEXPI v3 ToV3From2XZ(v2 vec2, r32 y) { return MakeV3(vec2.x, y, vec2.y); }
PEXPI v3 ToV3From4(v4 vec4) { return MakeV3(vec4.x, vec4.y, vec4.z); }
PEXPI v3i ToV3iFrom2(v2i vec2i, i32 z) { return MakeV3i(vec2i.x, vec2i.y, z); }
PEXPI v3i ToV3iFrom2XZ(v2i vec2i, i32 y) { return MakeV3i(vec2i.x, y, vec2i.y); }
PEXPI v3i ToV3iFrom4(v4i vec4i) { return MakeV3i(vec4i.x, vec4i.y, vec4i.z); }

PEXPI v4 ToV4Fromi(v4i vec4i) { return MakeV4((r32)vec4i.x, (r32)vec4i.y, (r32)vec4i.z, (r32)vec4i.w); }
PEXPI v4d ToV4dFromi(v4i vec4i) { return MakeV4d((r64)vec4i.x, (r64)vec4i.y, (r64)vec4i.z, (r64)vec4i.w); }
PEXPI v4d ToV4dFromf(v4 vec4) { return MakeV4d((r64)vec4.x, (r64)vec4.y, (r64)vec4.z, (r64)vec4.w); }
PEXPI v4 ToV4Fromd(v4d vec4d) { return MakeV4((r32)vec4d.x, (r32)vec4d.y, (r32)vec4d.z, (r32)vec4d.w); }
PEXPI v4 ToV4From3(v3 vec3, r32 w) { return MakeV4(vec3.x, vec3.y, vec3.z, w); }
PEXPI v4i ToV4iFrom3(v3i vec3i, i32 w) { return MakeV4i(vec3i.x, vec3i.y, vec3i.z, w); }

#if TARGET_IS_ORCA
PEXPI oc_vec2 ToOcVec2(v2 vector) { return NEW_STRUCT(oc_vec2){ .x = vector.x, .y = vector.y }; }
PEXPI v2 ToV2FromOc(oc_vec2 orcaVector) { return MakeV2(orcaVector.x, orcaVector.y); }

PEXPI oc_vec2i ToOcVec2i(v2i vector) { return NEW_STRUCT(oc_vec2i){ .x = vector.x, .y = vector.y }; }
PEXPI v2i ToV2iFromOc(oc_vec2i orcaVector) { return MakeV2i(orcaVector.x, orcaVector.y); }

PEXPI oc_vec3 ToOcVec3(v3 vector) { return NEW_STRUCT(oc_vec3){ .x = vector.x, .y = vector.y, .z = vector.z }; }
PEXPI v3 ToV3FromOc(oc_vec3 orcaVector) { return MakeV3(orcaVector.x, orcaVector.y, orcaVector.z); }

PEXPI oc_vec4 ToOcVec4(v4 vector) { return NEW_STRUCT(oc_vec4){ .x = vector.x, .y = vector.y, .z = vector.z, .w = vector.w }; }
PEXPI v4 ToV4FromOc(oc_vec4 orcaVector) { return MakeV4(orcaVector.x, orcaVector.y, orcaVector.z, orcaVector.w); }
#endif //TARGET_IS_ORCA

// +--------------------------------------------------------------+
// |                    C++ Operator Overloads                    |
// +--------------------------------------------------------------+
#if LANGUAGE_IS_CPP

PEXPI v2  operator+(v2  left, v2  right) { v2  result; result.x = left.x + right.x; result.y = left.y + right.y; return result; }
PEXPI v3  operator+(v3  left, v3  right) { v3  result; result.x = left.x + right.x; result.y = left.y + right.y; result.z = left.z + right.z; return result; }
PEXPI v4  operator+(v4  left, v4  right) { v4  result; result.x = left.x + right.x; result.y = left.y + right.y; result.z = left.z + right.z; result.w = left.w + right.w; return result; }
PEXPI v2i operator+(v2i left, v2i right) { v2i result; result.x = left.x + right.x; result.y = left.y + right.y; return result; }
PEXPI v3i operator+(v3i left, v3i right) { v3i result; result.x = left.x + right.x; result.y = left.y + right.y; result.z = left.z + right.z; return result; }
PEXPI v4i operator+(v4i left, v4i right) { v4i result; result.x = left.x + right.x; result.y = left.y + right.y; result.z = left.z + right.z; result.w = left.w + right.w; return result; }
PEXPI v2d operator+(v2d left, v2d right) { v2d result; result.x = left.x + right.x; result.y = left.y + right.y; return result; }
PEXPI v3d operator+(v3d left, v3d right) { v3d result; result.x = left.x + right.x; result.y = left.y + right.y; result.z = left.z + right.z; return result; }
PEXPI v4d operator+(v4d left, v4d right) { v4d result; result.x = left.x + right.x; result.y = left.y + right.y; result.z = left.z + right.z; result.w = left.w + right.w; return result; }

PEXPI v2  operator+=(v2&  left, v2  right) { return left = left + right; }
PEXPI v3  operator+=(v3&  left, v3  right) { return left = left + right; }
PEXPI v4  operator+=(v4&  left, v4  right) { return left = left + right; }
PEXPI v2i operator+=(v2i& left, v2i right) { return left = left + right; }
PEXPI v3i operator+=(v3i& left, v3i right) { return left = left + right; }
PEXPI v4i operator+=(v4i& left, v4i right) { return left = left + right; }
PEXPI v2d operator+=(v2d& left, v2d right) { return left = left + right; }
PEXPI v3d operator+=(v3d& left, v3d right) { return left = left + right; }
PEXPI v4d operator+=(v4d& left, v4d right) { return left = left + right; }

PEXPI v2  operator-(v2  left, v2  right) { v2  result; result.x = left.x - right.x; result.y = left.y - right.y; return result; }
PEXPI v3  operator-(v3  left, v3  right) { v3  result; result.x = left.x - right.x; result.y = left.y - right.y; result.z = left.z - right.z; return result; }
PEXPI v4  operator-(v4  left, v4  right) { v4  result; result.x = left.x - right.x; result.y = left.y - right.y; result.z = left.z - right.z; result.w = left.w - right.w; return result; }
PEXPI v2i operator-(v2i left, v2i right) { v2i result; result.x = left.x - right.x; result.y = left.y - right.y; return result; }
PEXPI v3i operator-(v3i left, v3i right) { v3i result; result.x = left.x - right.x; result.y = left.y - right.y; result.z = left.z - right.z; return result; }
PEXPI v4i operator-(v4i left, v4i right) { v4i result; result.x = left.x - right.x; result.y = left.y - right.y; result.z = left.z - right.z; result.w = left.w - right.w; return result; }
PEXPI v2d operator-(v2d left, v2d right) { v2d result; result.x = left.x - right.x; result.y = left.y - right.y; return result; }
PEXPI v3d operator-(v3d left, v3d right) { v3d result; result.x = left.x - right.x; result.y = left.y - right.y; result.z = left.z - right.z; return result; }
PEXPI v4d operator-(v4d left, v4d right) { v4d result; result.x = left.x - right.x; result.y = left.y - right.y; result.z = left.z - right.z; result.w = left.w - right.w; return result; }

PEXPI v2  operator-=(v2&  left, v2  right) { return left = left - right; }
PEXPI v3  operator-=(v3&  left, v3  right) { return left = left - right; }
PEXPI v4  operator-=(v4&  left, v4  right) { return left = left - right; }
PEXPI v2i operator-=(v2i& left, v2i right) { return left = left - right; }
PEXPI v3i operator-=(v3i& left, v3i right) { return left = left - right; }
PEXPI v4i operator-=(v4i& left, v4i right) { return left = left - right; }
PEXPI v2d operator-=(v2d& left, v2d right) { return left = left - right; }
PEXPI v3d operator-=(v3d& left, v3d right) { return left = left - right; }
PEXPI v4d operator-=(v4d& left, v4d right) { return left = left - right; }

PEXPI v2  operator*(v2  left, v2  right) { v2  result; result.x = left.x * right.x; result.y = left.y * right.y; return result; }
PEXPI v3  operator*(v3  left, v3  right) { v3  result; result.x = left.x * right.x; result.y = left.y * right.y; result.z = left.z * right.z; return result; }
PEXPI v4  operator*(v4  left, v4  right) { v4  result; result.x = left.x * right.x; result.y = left.y * right.y; result.z = left.z * right.z; result.w = left.w * right.w; return result; }
PEXPI v2i operator*(v2i left, v2i right) { v2i result; result.x = left.x * right.x; result.y = left.y * right.y; return result; }
PEXPI v3i operator*(v3i left, v3i right) { v3i result; result.x = left.x * right.x; result.y = left.y * right.y; result.z = left.z * right.z; return result; }
PEXPI v4i operator*(v4i left, v4i right) { v4i result; result.x = left.x * right.x; result.y = left.y * right.y; result.z = left.z * right.z; result.w = left.w * right.w; return result; }
PEXPI v2d operator*(v2d left, v2d right) { v2d result; result.x = left.x * right.x; result.y = left.y * right.y; return result; }
PEXPI v3d operator*(v3d left, v3d right) { v3d result; result.x = left.x * right.x; result.y = left.y * right.y; result.z = left.z * right.z; return result; }
PEXPI v4d operator*(v4d left, v4d right) { v4d result; result.x = left.x * right.x; result.y = left.y * right.y; result.z = left.z * right.z; result.w = left.w * right.w; return result; }

PEXPI v2  operator*=(v2&  left, v2  right) { return left = left * right; }
PEXPI v3  operator*=(v3&  left, v3  right) { return left = left * right; }
PEXPI v4  operator*=(v4&  left, v4  right) { return left = left * right; }
PEXPI v2i operator*=(v2i& left, v2i right) { return left = left * right; }
PEXPI v3i operator*=(v3i& left, v3i right) { return left = left * right; }
PEXPI v4i operator*=(v4i& left, v4i right) { return left = left * right; }
PEXPI v2d operator*=(v2d& left, v2d right) { return left = left * right; }
PEXPI v3d operator*=(v3d& left, v3d right) { return left = left * right; }
PEXPI v4d operator*=(v4d& left, v4d right) { return left = left * right; }

PEXPI v2  operator/(v2  left, v2  right) { v2  result; result.x = left.x / right.x; result.y = left.y / right.y; return result; }
PEXPI v3  operator/(v3  left, v3  right) { v3  result; result.x = left.x / right.x; result.y = left.y / right.y; result.z = left.z / right.z; return result; }
PEXPI v4  operator/(v4  left, v4  right) { v4  result; result.x = left.x / right.x; result.y = left.y / right.y; result.z = left.z / right.z; result.w = left.w / right.w; return result; }
PEXPI v2i operator/(v2i left, v2i right) { v2i result; result.x = left.x / right.x; result.y = left.y / right.y; return result; }
PEXPI v3i operator/(v3i left, v3i right) { v3i result; result.x = left.x / right.x; result.y = left.y / right.y; result.z = left.z / right.z; return result; }
PEXPI v4i operator/(v4i left, v4i right) { v4i result; result.x = left.x / right.x; result.y = left.y / right.y; result.z = left.z / right.z; result.w = left.w / right.w; return result; }
PEXPI v2d operator/(v2d left, v2d right) { v2d result; result.x = left.x / right.x; result.y = left.y / right.y; return result; }
PEXPI v3d operator/(v3d left, v3d right) { v3d result; result.x = left.x / right.x; result.y = left.y / right.y; result.z = left.z / right.z; return result; }
PEXPI v4d operator/(v4d left, v4d right) { v4d result; result.x = left.x / right.x; result.y = left.y / right.y; result.z = left.z / right.z; result.w = left.w / right.w; return result; }

PEXPI v2  operator/=(v2&  left, v2  right) { return left = left / right; }
PEXPI v3  operator/=(v3&  left, v3  right) { return left = left / right; }
PEXPI v4  operator/=(v4&  left, v4  right) { return left = left / right; }
PEXPI v2i operator/=(v2i& left, v2i right) { return left = left / right; }
PEXPI v3i operator/=(v3i& left, v3i right) { return left = left / right; }
PEXPI v4i operator/=(v4i& left, v4i right) { return left = left / right; }
PEXPI v2d operator/=(v2d& left, v2d right) { return left = left / right; }
PEXPI v3d operator/=(v3d& left, v3d right) { return left = left / right; }
PEXPI v4d operator/=(v4d& left, v4d right) { return left = left / right; }

PEXPI v2  operator*(v2  vector, r32 scalar) { v2  result; result.x = vector.x * scalar; result.y = vector.y * scalar; return result; }
PEXPI v3  operator*(v3  vector, r32 scalar) { v3  result; result.x = vector.x * scalar; result.y = vector.y * scalar; result.z = vector.z * scalar; return result; }
PEXPI v4  operator*(v4  vector, r32 scalar) { v4  result; result.x = vector.x * scalar; result.y = vector.y * scalar; result.z = vector.z * scalar; result.w = vector.w * scalar; return result; }
PEXPI v2i operator*(v2i vector, i32 scalar) { v2i result; result.x = vector.x * scalar; result.y = vector.y * scalar; return result; }
PEXPI v3i operator*(v3i vector, i32 scalar) { v3i result; result.x = vector.x * scalar; result.y = vector.y * scalar; result.z = vector.z * scalar; return result; }
PEXPI v4i operator*(v4i vector, i32 scalar) { v4i result; result.x = vector.x * scalar; result.y = vector.y * scalar; result.z = vector.z * scalar; result.w = vector.w * scalar; return result; }
PEXPI v2d operator*(v2d vector, r64 scalar) { v2d result; result.x = vector.x * scalar; result.y = vector.y * scalar; return result; }
PEXPI v3d operator*(v3d vector, r64 scalar) { v3d result; result.x = vector.x * scalar; result.y = vector.y * scalar; result.z = vector.z * scalar; return result; }
PEXPI v4d operator*(v4d vector, r64 scalar) { v4d result; result.x = vector.x * scalar; result.y = vector.y * scalar; result.z = vector.z * scalar; result.w = vector.w * scalar; return result; }

PEXPI v2  operator*=(v2&  vector, r32 scalar) { return vector = vector * scalar; }
PEXPI v3  operator*=(v3&  vector, r32 scalar) { return vector = vector * scalar; }
PEXPI v4  operator*=(v4&  vector, r32 scalar) { return vector = vector * scalar; }
PEXPI v2i operator*=(v2i& vector, i32 scalar) { return vector = vector * scalar; }
PEXPI v3i operator*=(v3i& vector, i32 scalar) { return vector = vector * scalar; }
PEXPI v4i operator*=(v4i& vector, i32 scalar) { return vector = vector * scalar; }
PEXPI v2d operator*=(v2d& vector, r64 scalar) { return vector = vector * scalar; }
PEXPI v3d operator*=(v3d& vector, r64 scalar) { return vector = vector * scalar; }
PEXPI v4d operator*=(v4d& vector, r64 scalar) { return vector = vector * scalar; }

PEXPI v2  operator/(v2  vector, r32 scalar) { v2  result; result.x = vector.x / scalar; result.y = vector.y / scalar; return result; }
PEXPI v3  operator/(v3  vector, r32 scalar) { v3  result; result.x = vector.x / scalar; result.y = vector.y / scalar; result.z = vector.z / scalar; return result; }
PEXPI v4  operator/(v4  vector, r32 scalar) { v4  result; result.x = vector.x / scalar; result.y = vector.y / scalar; result.z = vector.z / scalar; result.w = vector.w / scalar; return result; }
PEXPI v2i operator/(v2i vector, i32 scalar) { v2i result; result.x = vector.x / scalar; result.y = vector.y / scalar; return result; }
PEXPI v3i operator/(v3i vector, i32 scalar) { v3i result; result.x = vector.x / scalar; result.y = vector.y / scalar; result.z = vector.z / scalar; return result; }
PEXPI v4i operator/(v4i vector, i32 scalar) { v4i result; result.x = vector.x / scalar; result.y = vector.y / scalar; result.z = vector.z / scalar; result.w = vector.w / scalar; return result; }
PEXPI v2d operator/(v2d vector, r64 scalar) { v2d result; result.x = vector.x / scalar; result.y = vector.y / scalar; return result; }
PEXPI v3d operator/(v3d vector, r64 scalar) { v3d result; result.x = vector.x / scalar; result.y = vector.y / scalar; result.z = vector.z / scalar; return result; }
PEXPI v4d operator/(v4d vector, r64 scalar) { v4d result; result.x = vector.x / scalar; result.y = vector.y / scalar; result.z = vector.z / scalar; result.w = vector.w / scalar; return result; }

PEXPI v2  operator/=(v2&  vector, r32 scalar) { return vector = vector / scalar; }
PEXPI v3  operator/=(v3&  vector, r32 scalar) { return vector = vector / scalar; }
PEXPI v4  operator/=(v4&  vector, r32 scalar) { return vector = vector / scalar; }
PEXPI v2i operator/=(v2i& vector, i32 scalar) { return vector = vector / scalar; }
PEXPI v3i operator/=(v3i& vector, i32 scalar) { return vector = vector / scalar; }
PEXPI v4i operator/=(v4i& vector, i32 scalar) { return vector = vector / scalar; }
PEXPI v2d operator/=(v2d& vector, r64 scalar) { return vector = vector / scalar; }
PEXPI v3d operator/=(v3d& vector, r64 scalar) { return vector = vector / scalar; }
PEXPI v4d operator/=(v4d& vector, r64 scalar) { return vector = vector / scalar; }

PEXPI v2  operator-(v2  vector) { v2  result; result.x = -vector.x; result.y = -vector.y; return result; }
PEXPI v3  operator-(v3  vector) { v3  result; result.x = -vector.x; result.y = -vector.y; result.z = -vector.z; return result; }
PEXPI v4  operator-(v4  vector) { v4  result; result.x = -vector.x; result.y = -vector.y; result.z = -vector.z; result.w = -vector.w; return result; }
PEXPI v2i operator-(v2i vector) { v2i result; result.x = -vector.x; result.y = -vector.y; return result; }
PEXPI v3i operator-(v3i vector) { v3i result; result.x = -vector.x; result.y = -vector.y; result.z = -vector.z; return result; }
PEXPI v4i operator-(v4i vector) { v4i result; result.x = -vector.x; result.y = -vector.y; result.z = -vector.z; result.w = -vector.w; return result; }
PEXPI v2d operator-(v2d vector) { v2d result; result.x = -vector.x; result.y = -vector.y; return result; }
PEXPI v3d operator-(v3d vector) { v3d result; result.x = -vector.x; result.y = -vector.y; result.z = -vector.z; return result; }
PEXPI v4d operator-(v4d vector) { v4d result; result.x = -vector.x; result.y = -vector.y; result.z = -vector.z; result.w = -vector.w; return result; }

PEXPI bool operator==(v2  left, v2  right) { return (left.x == right.x && left.y == right.y); }
PEXPI bool operator==(v3  left, v3  right) { return (left.x == right.x && left.y == right.y && left.z == right.z); }
PEXPI bool operator==(v4  left, v4  right) { return (left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w); }
PEXPI bool operator==(v2i left, v2i right) { return (left.x == right.x && left.y == right.y); }
PEXPI bool operator==(v3i left, v3i right) { return (left.x == right.x && left.y == right.y && left.z == right.z); }
PEXPI bool operator==(v4i left, v4i right) { return (left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w); }
PEXPI bool operator==(v2d left, v2d right) { return (left.x == right.x && left.y == right.y); }
PEXPI bool operator==(v3d left, v3d right) { return (left.x == right.x && left.y == right.y && left.z == right.z); }
PEXPI bool operator==(v4d left, v4d right) { return (left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w); }

PEXPI bool operator!=(v2  left, v2  right) { return (left.x != right.x || left.y != right.y); }
PEXPI bool operator!=(v3  left, v3  right) { return (left.x != right.x || left.y != right.y || left.z != right.z); }
PEXPI bool operator!=(v4  left, v4  right) { return (left.x != right.x || left.y != right.y || left.z != right.z || left.w != right.w); }
PEXPI bool operator!=(v2i left, v2i right) { return (left.x != right.x || left.y != right.y); }
PEXPI bool operator!=(v3i left, v3i right) { return (left.x != right.x || left.y != right.y || left.z != right.z); }
PEXPI bool operator!=(v4i left, v4i right) { return (left.x != right.x || left.y != right.y || left.z != right.z || left.w != right.w); }
PEXPI bool operator!=(v2d left, v2d right) { return (left.x != right.x || left.y != right.y); }
PEXPI bool operator!=(v3d left, v3d right) { return (left.x != right.x || left.y != right.y || left.z != right.z); }
PEXPI bool operator!=(v4d left, v4d right) { return (left.x != right.x || left.y != right.y || left.z != right.z || left.w != right.w); }

#endif //LANGUAGE_IS_CPP

// +--------------------------------------------------------------+
// |               Operator Overload C Equivalents                |
// +--------------------------------------------------------------+
PEXPI v2   AddV2(v2  left, v2  right) { v2 result;  result.x = left.x + right.x; result.y = left.y + right.y; return result; }
PEXPI v3   AddV3(v3  left, v3  right) { v3 result;  result.x = left.x + right.x; result.y = left.y + right.y; result.z = left.z + right.z; return result; }
PEXPI v4   AddV4(v4  left, v4  right) { v4 result;  result.x = left.x + right.x; result.y = left.y + right.y; result.z = left.z + right.z; result.w = left.w + right.w; return result; }
PEXPI v2i AddV2i(v2i left, v2i right) { v2i result; result.x = left.x + right.x; result.y = left.y + right.y; return result; }
PEXPI v3i AddV3i(v3i left, v3i right) { v3i result; result.x = left.x + right.x; result.y = left.y + right.y; result.z = left.z + right.z; return result; }
PEXPI v4i AddV4i(v4i left, v4i right) { v4i result; result.x = left.x + right.x; result.y = left.y + right.y; result.z = left.z + right.z; result.w = left.w + right.w; return result; }
PEXPI v2d AddV2d(v2d left, v2d right) { v2d result; result.x = left.x + right.x; result.y = left.y + right.y; return result; }
PEXPI v3d AddV3d(v3d left, v3d right) { v3d result; result.x = left.x + right.x; result.y = left.y + right.y; result.z = left.z + right.z; return result; }
PEXPI v4d AddV4d(v4d left, v4d right) { v4d result; result.x = left.x + right.x; result.y = left.y + right.y; result.z = left.z + right.z; result.w = left.w + right.w; return result; }

PEXPI v2   SubV2(v2  left, v2  right) { v2  result; result.x = left.x - right.x; result.y = left.y - right.y; return result; }
PEXPI v3   SubV3(v3  left, v3  right) { v3  result; result.x = left.x - right.x; result.y = left.y - right.y; result.z = left.z - right.z; return result; }
PEXPI v4   SubV4(v4  left, v4  right) { v4  result; result.x = left.x - right.x; result.y = left.y - right.y; result.z = left.z - right.z; result.w = left.w - right.w; return result; }
PEXPI v2i SubV2i(v2i left, v2i right) { v2i result; result.x = left.x - right.x; result.y = left.y - right.y; return result; }
PEXPI v3i SubV3i(v3i left, v3i right) { v3i result; result.x = left.x - right.x; result.y = left.y - right.y; result.z = left.z - right.z; return result; }
PEXPI v4i SubV4i(v4i left, v4i right) { v4i result; result.x = left.x - right.x; result.y = left.y - right.y; result.z = left.z - right.z; result.w = left.w - right.w; return result; }
PEXPI v2d SubV2d(v2d left, v2d right) { v2d result; result.x = left.x - right.x; result.y = left.y - right.y; return result; }
PEXPI v3d SubV3d(v3d left, v3d right) { v3d result; result.x = left.x - right.x; result.y = left.y - right.y; result.z = left.z - right.z; return result; }
PEXPI v4d SubV4d(v4d left, v4d right) { v4d result; result.x = left.x - right.x; result.y = left.y - right.y; result.z = left.z - right.z; result.w = left.w - right.w; return result; }

PEXPI v2   MulV2(v2  left, v2  right) { v2  result; result.x = left.x * right.x; result.y = left.y * right.y; return result; }
PEXPI v3   MulV3(v3  left, v3  right) { v3  result; result.x = left.x * right.x; result.y = left.y * right.y; result.z = left.z * right.z; return result; }
PEXPI v4   MulV4(v4  left, v4  right) { v4  result; result.x = left.x * right.x; result.y = left.y * right.y; result.z = left.z * right.z; result.w = left.w * right.w; return result; }
PEXPI v2i MulV2i(v2i left, v2i right) { v2i result; result.x = left.x * right.x; result.y = left.y * right.y; return result; }
PEXPI v3i MulV3i(v3i left, v3i right) { v3i result; result.x = left.x * right.x; result.y = left.y * right.y; result.z = left.z * right.z; return result; }
PEXPI v4i MulV4i(v4i left, v4i right) { v4i result; result.x = left.x * right.x; result.y = left.y * right.y; result.z = left.z * right.z; result.w = left.w * right.w; return result; }
PEXPI v2d MulV2d(v2d left, v2d right) { v2d result; result.x = left.x * right.x; result.y = left.y * right.y; return result; }
PEXPI v3d MulV3d(v3d left, v3d right) { v3d result; result.x = left.x * right.x; result.y = left.y * right.y; result.z = left.z * right.z; return result; }
PEXPI v4d MulV4d(v4d left, v4d right) { v4d result; result.x = left.x * right.x; result.y = left.y * right.y; result.z = left.z * right.z; result.w = left.w * right.w; return result; }

PEXPI v2   DivV2(v2  left, v2  right) { v2  result; result.x = left.x / right.x; result.y = left.y / right.y; return result; }
PEXPI v3   DivV3(v3  left, v3  right) { v3  result; result.x = left.x / right.x; result.y = left.y / right.y; result.z = left.z / right.z; return result; }
PEXPI v4   DivV4(v4  left, v4  right) { v4  result; result.x = left.x / right.x; result.y = left.y / right.y; result.z = left.z / right.z; result.w = left.w / right.w; return result; }
PEXPI v2i DivV2i(v2i left, v2i right) { v2i result; result.x = left.x / right.x; result.y = left.y / right.y; return result; }
PEXPI v3i DivV3i(v3i left, v3i right) { v3i result; result.x = left.x / right.x; result.y = left.y / right.y; result.z = left.z / right.z; return result; }
PEXPI v4i DivV4i(v4i left, v4i right) { v4i result; result.x = left.x / right.x; result.y = left.y / right.y; result.z = left.z / right.z; result.w = left.w / right.w; return result; }
PEXPI v2d DivV2d(v2d left, v2d right) { v2d result; result.x = left.x / right.x; result.y = left.y / right.y; return result; }
PEXPI v3d DivV3d(v3d left, v3d right) { v3d result; result.x = left.x / right.x; result.y = left.y / right.y; result.z = left.z / right.z; return result; }
PEXPI v4d DivV4d(v4d left, v4d right) { v4d result; result.x = left.x / right.x; result.y = left.y / right.y; result.z = left.z / right.z; result.w = left.w / right.w; return result; }

PEXPI v2   ScaleV2(v2  left, r32 scalar) { v2  result; result.x = left.x * scalar; result.y = left.y * scalar; return result; }
PEXPI v3   ScaleV3(v3  left, r32 scalar) { v3  result; result.x = left.x * scalar; result.y = left.y * scalar; result.z = left.z * scalar; return result; }
PEXPI v4   ScaleV4(v4  left, r32 scalar) { v4  result; result.x = left.x * scalar; result.y = left.y * scalar; result.z = left.z * scalar; result.w = left.w * scalar; return result; }
PEXPI v2i ScaleV2i(v2i left, i32 scalar) { v2i result; result.x = left.x * scalar; result.y = left.y * scalar; return result; }
PEXPI v3i ScaleV3i(v3i left, i32 scalar) { v3i result; result.x = left.x * scalar; result.y = left.y * scalar; result.z = left.z * scalar; return result; }
PEXPI v4i ScaleV4i(v4i left, i32 scalar) { v4i result; result.x = left.x * scalar; result.y = left.y * scalar; result.z = left.z * scalar; result.w = left.w * scalar; return result; }
PEXPI v2d ScaleV2d(v2d left, r64 scalar) { v2d result; result.x = left.x * scalar; result.y = left.y * scalar; return result; }
PEXPI v3d ScaleV3d(v3d left, r64 scalar) { v3d result; result.x = left.x * scalar; result.y = left.y * scalar; result.z = left.z * scalar; return result; }
PEXPI v4d ScaleV4d(v4d left, r64 scalar) { v4d result; result.x = left.x * scalar; result.y = left.y * scalar; result.z = left.z * scalar; result.w = left.w * scalar; return result; }

PEXPI v2   ShrinkV2(v2  left, r32 divisor) { v2  result; result.x = left.x / divisor; result.y = left.y / divisor; return result; }
PEXPI v3   ShrinkV3(v3  left, r32 divisor) { v3  result; result.x = left.x / divisor; result.y = left.y / divisor; result.z = left.z / divisor; return result; }
PEXPI v4   ShrinkV4(v4  left, r32 divisor) { v4  result; result.x = left.x / divisor; result.y = left.y / divisor; result.z = left.z / divisor; result.w = left.w / divisor; return result; }
PEXPI v2i ShrinkV2i(v2i left, i32 divisor) { v2i result; result.x = left.x / divisor; result.y = left.y / divisor; return result; }
PEXPI v3i ShrinkV3i(v3i left, i32 divisor) { v3i result; result.x = left.x / divisor; result.y = left.y / divisor; result.z = left.z / divisor; return result; }
PEXPI v4i ShrinkV4i(v4i left, i32 divisor) { v4i result; result.x = left.x / divisor; result.y = left.y / divisor; result.z = left.z / divisor; result.w = left.w / divisor; return result; }
PEXPI v2d ShrinkV2d(v2d left, r64 divisor) { v2d result; result.x = left.x / divisor; result.y = left.y / divisor; return result; }
PEXPI v3d ShrinkV3d(v3d left, r64 divisor) { v3d result; result.x = left.x / divisor; result.y = left.y / divisor; result.z = left.z / divisor; return result; }
PEXPI v4d ShrinkV4d(v4d left, r64 divisor) { v4d result; result.x = left.x / divisor; result.y = left.y / divisor; result.z = left.z / divisor; result.w = left.w / divisor; return result; }

PEXPI bool  AreEqualV2(v2  left, v2  right) { return (left.x == right.x && left.y == right.y); }
PEXPI bool  AreEqualV3(v3  left, v3  right) { return (left.x == right.x && left.y == right.y && left.z == right.z); }
PEXPI bool  AreEqualV4(v4  left, v4  right) { return (left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w); }
PEXPI bool AreEqualV2i(v2i left, v2i right) { return (left.x == right.x && left.y == right.y); }
PEXPI bool AreEqualV3i(v3i left, v3i right) { return (left.x == right.x && left.y == right.y && left.z == right.z); }
PEXPI bool AreEqualV4i(v4i left, v4i right) { return (left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w); }
PEXPI bool AreEqualV2d(v2d left, v2d right) { return (left.x == right.x && left.y == right.y); }
PEXPI bool AreEqualV3d(v3d left, v3d right) { return (left.x == right.x && left.y == right.y && left.z == right.z); }
PEXPI bool AreEqualV4d(v4d left, v4d right) { return (left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w); }

// +--------------------------------------------------------------+
// |                      Common Vector Math                      |
// +--------------------------------------------------------------+
PEXPI r32  DotV2(v2  left, v2  right) { return (left.x * right.x) + (left.y * right.y); }
PEXPI r32  DotV3(v3  left, v3  right) { return (left.x * right.x) + (left.y * right.y) + (left.z * right.z); }
PEXPI r32  DotV4(v4  left, v4  right) { return (left.x * right.x) + (left.y * right.y) + (left.z * right.z) + (left.w * right.w); }
PEXPI i32 DotV2i(v2i left, v2i right) { return (left.x * right.x) + (left.y * right.y); }
PEXPI i32 DotV3i(v3i left, v3i right) { return (left.x * right.x) + (left.y * right.y) + (left.z * right.z); }
PEXPI i32 DotV4i(v4i left, v4i right) { return (left.x * right.x) + (left.y * right.y) + (left.z * right.z) + (left.w * right.w); }
PEXPI r64 DotV2d(v2d left, v2d right) { return (left.x * right.x) + (left.y * right.y); }
PEXPI r64 DotV3d(v3d left, v3d right) { return (left.x * right.x) + (left.y * right.y) + (left.z * right.z); }
PEXPI r64 DotV4d(v4d left, v4d right) { return (left.x * right.x) + (left.y * right.y) + (left.z * right.z) + (left.w * right.w); }

PEXPI v3   CrossV3(v3  left, v3  right) { return  MakeV3((left.y*right.z - left.z*right.y), (left.z*right.x - left.x*right.z), (left.x*right.y - left.y*right.x)); }
PEXPI v3i CrossV3i(v3i left, v3i right) { return MakeV3i((left.y*right.z - left.z*right.y), (left.z*right.x - left.x*right.z), (left.x*right.y - left.y*right.x)); }
PEXPI v3d CrossV3d(v3d left, v3d right) { return MakeV3d((left.y*right.z - left.z*right.y), (left.z*right.x - left.x*right.z), (left.x*right.y - left.y*right.x)); }

PEXPI r32  LengthSquaredV2(v2  vec2)  { return  DotV2(vec2,  vec2);  }
PEXPI r32  LengthSquaredV3(v3  vec3)  { return  DotV3(vec3,  vec3);  }
PEXPI r32  LengthSquaredV4(v4  vec4)  { return  DotV4(vec4,  vec4);  }
PEXPI i32 LengthSquaredV2i(v2i vec2i) { return DotV2i(vec2i, vec2i); }
PEXPI i32 LengthSquaredV3i(v3i vec3i) { return DotV3i(vec3i, vec3i); }
PEXPI i32 LengthSquaredV4i(v4i vec4i) { return DotV4i(vec4i, vec4i); }
PEXPI r64 LengthSquaredV2d(v2d vec2d) { return DotV2d(vec2d, vec2d); }
PEXPI r64 LengthSquaredV3d(v3d vec3d) { return DotV3d(vec3d, vec3d); }
PEXPI r64 LengthSquaredV4d(v4d vec4d) { return DotV4d(vec4d, vec4d); }

PEXPI r32  LengthV2(v2  vec2)  { return SqrtR32(LengthSquaredV2(vec2)); }
PEXPI r32  LengthV3(v3  vec3)  { return SqrtR32(LengthSquaredV3(vec3)); }
PEXPI r32  LengthV4(v4  vec4)  { return SqrtR32(LengthSquaredV4(vec4)); }
PEXPI r32 LengthV2i(v2i vec2i) { return SqrtR32((r32)LengthSquaredV2i(vec2i)); }
PEXPI r32 LengthV3i(v3i vec3i) { return SqrtR32((r32)LengthSquaredV3i(vec3i)); }
PEXPI r32 LengthV4i(v4i vec4i) { return SqrtR32((r32)LengthSquaredV4i(vec4i)); }
PEXPI r64 LengthV2d(v2d vec2d) { return SqrtR64(LengthSquaredV2d(vec2d)); }
PEXPI r64 LengthV3d(v3d vec3d) { return SqrtR64(LengthSquaredV3d(vec3d)); }
PEXPI r64 LengthV4d(v4d vec4d) { return SqrtR64(LengthSquaredV4d(vec4d)); }

PEXPI v2   NormalizeV2(v2  vec2)  { return  ShrinkV2(vec2,   LengthV2(vec2));  }
PEXPI v3   NormalizeV3(v3  vec3)  { return  ShrinkV3(vec3,   LengthV3(vec3));  }
PEXPI v4   NormalizeV4(v4  vec4)  { return  ShrinkV4(vec4,   LengthV4(vec4));  }
PEXPI v2d NormalizeV2d(v2d vec2d) { return ShrinkV2d(vec2d, LengthV2d(vec2d)); }
PEXPI v3d NormalizeV3d(v3d vec3d) { return ShrinkV3d(vec3d, LengthV3d(vec3d)); }
PEXPI v4d NormalizeV4d(v4d vec4d) { return ShrinkV4d(vec4d, LengthV4d(vec4d)); }

PEXPI v2   LerpV2(v2  start, v2  end, r32 amount) { return  AddV2(ScaleV2(start, (1.0f - amount)),  ScaleV2(end, amount)); }
PEXPI v3   LerpV3(v3  start, v3  end, r32 amount) { return  AddV3(ScaleV3(start, (1.0f - amount)),  ScaleV3(end, amount)); }
PEXPI v4   LerpV4(v4  start, v4  end, r32 amount) { return  AddV4(ScaleV4(start, (1.0f - amount)),  ScaleV4(end, amount)); }
PEXPI v2d LerpV2d(v2d start, v2d end, r64 amount) { return AddV2d(ScaleV2d(start, (1.0 - amount)), ScaleV2d(end, amount)); }
PEXPI v3d LerpV3d(v3d start, v3d end, r64 amount) { return AddV3d(ScaleV3d(start, (1.0 - amount)), ScaleV3d(end, amount)); }
PEXPI v4d LerpV4d(v4d start, v4d end, r64 amount) { return AddV4d(ScaleV4d(start, (1.0 - amount)), ScaleV4d(end, amount)); }

// +--------------------------------------------------------------+
// |              Componentwise Intrinsic Functions               |
// +--------------------------------------------------------------+
//NOTE: These int overloads for Floor/Ceil/Round are a bit misleading with their name.
// They actually take floating point vector types, even though the name reads like
// functions that take integer vector types. They RETURN integer vector types, and
// that's what the i suffix is meant to convey.
PEXPI v2  FloorV2(v2    vec2)  { return  MakeV2(FloorR32(vec2.x), FloorR32(vec2.y)); }
PEXPI v3  FloorV3(v3    vec3)  { return  MakeV3(FloorR32(vec3.x), FloorR32(vec3.y), FloorR32(vec3.z)); }
PEXPI v4  FloorV4(v4    vec4)  { return  MakeV4(FloorR32(vec4.x), FloorR32(vec4.y), FloorR32(vec4.z), FloorR32(vec4.w)); }
PEXPI v2i FloorV2i(v2   vec2)  { return MakeV2i(FloorR32i(vec2.x), FloorR32i(vec2.y)); }
PEXPI v3i FloorV3i(v3   vec3)  { return MakeV3i(FloorR32i(vec3.x), FloorR32i(vec3.y), FloorR32i(vec3.z)); }
PEXPI v4i FloorV4i(v4   vec4)  { return MakeV4i(FloorR32i(vec4.x), FloorR32i(vec4.y), FloorR32i(vec4.z), FloorR32i(vec4.w)); }
PEXPI v2d FloorV2d(v2d  vec2d) { return MakeV2d(FloorR64(vec2d.x), FloorR64(vec2d.y)); }
PEXPI v3d FloorV3d(v3d  vec3d) { return MakeV3d(FloorR64(vec3d.x), FloorR64(vec3d.y), FloorR64(vec3d.z)); }
PEXPI v4d FloorV4d(v4d  vec4d) { return MakeV4d(FloorR64(vec4d.x), FloorR64(vec4d.y), FloorR64(vec4d.z), FloorR64(vec4d.w)); }
PEXPI v2i FloorV2di(v2d vec2d) { return MakeV2i((i32)FloorR64i(vec2d.x), (i32)FloorR64i(vec2d.y)); }
PEXPI v3i FloorV3di(v3d vec3d) { return MakeV3i((i32)FloorR64i(vec3d.x), (i32)FloorR64i(vec3d.y), (i32)FloorR64i(vec3d.z)); }
PEXPI v4i FloorV4di(v4d vec4d) { return MakeV4i((i32)FloorR64i(vec4d.x), (i32)FloorR64i(vec4d.y), (i32)FloorR64i(vec4d.z), (i32)FloorR64i(vec4d.w)); }

PEXPI v2  CeilV2(v2    vec2)  { return  MakeV2(CeilR32(vec2.x), CeilR32(vec2.y)); }
PEXPI v3  CeilV3(v3    vec3)  { return  MakeV3(CeilR32(vec3.x), CeilR32(vec3.y), CeilR32(vec3.z)); }
PEXPI v4  CeilV4(v4    vec4)  { return  MakeV4(CeilR32(vec4.x), CeilR32(vec4.y), CeilR32(vec4.z), CeilR32(vec4.w)); }
PEXPI v2i CeilV2i(v2   vec2)  { return MakeV2i(CeilR32i(vec2.x), CeilR32i(vec2.y)); }
PEXPI v3i CeilV3i(v3   vec3)  { return MakeV3i(CeilR32i(vec3.x), CeilR32i(vec3.y), CeilR32i(vec3.z)); }
PEXPI v4i CeilV4i(v4   vec4)  { return MakeV4i(CeilR32i(vec4.x), CeilR32i(vec4.y), CeilR32i(vec4.z), CeilR32i(vec4.w)); }
PEXPI v2d CeilV2d(v2d  vec2d) { return MakeV2d(CeilR64(vec2d.x), CeilR64(vec2d.y)); }
PEXPI v3d CeilV3d(v3d  vec3d) { return MakeV3d(CeilR64(vec3d.x), CeilR64(vec3d.y), CeilR64(vec3d.z)); }
PEXPI v4d CeilV4d(v4d  vec4d) { return MakeV4d(CeilR64(vec4d.x), CeilR64(vec4d.y), CeilR64(vec4d.z), CeilR64(vec4d.w)); }
PEXPI v2i CeilV2di(v2d vec2d) { return MakeV2i((i32)CeilR64i(vec2d.x), (i32)CeilR64i(vec2d.y)); }
PEXPI v3i CeilV3di(v3d vec3d) { return MakeV3i((i32)CeilR64i(vec3d.x), (i32)CeilR64i(vec3d.y), (i32)CeilR64i(vec3d.z)); }
PEXPI v4i CeilV4di(v4d vec4d) { return MakeV4i((i32)CeilR64i(vec4d.x), (i32)CeilR64i(vec4d.y), (i32)CeilR64i(vec4d.z), (i32)CeilR64i(vec4d.w)); }

PEXPI v2  RoundV2(v2    vec2)  { return  MakeV2(RoundR32(vec2.x), RoundR32(vec2.y)); }
PEXPI v3  RoundV3(v3    vec3)  { return  MakeV3(RoundR32(vec3.x), RoundR32(vec3.y), RoundR32(vec3.z)); }
PEXPI v4  RoundV4(v4    vec4)  { return  MakeV4(RoundR32(vec4.x), RoundR32(vec4.y), RoundR32(vec4.z), RoundR32(vec4.w)); }
PEXPI v2i RoundV2i(v2   vec2)  { return MakeV2i(RoundR32i(vec2.x), RoundR32i(vec2.y)); }
PEXPI v3i RoundV3i(v3   vec3)  { return MakeV3i(RoundR32i(vec3.x), RoundR32i(vec3.y), RoundR32i(vec3.z)); }
PEXPI v4i RoundV4i(v4   vec4)  { return MakeV4i(RoundR32i(vec4.x), RoundR32i(vec4.y), RoundR32i(vec4.z), RoundR32i(vec4.w)); }
PEXPI v2d RoundV2d(v2d  vec2d) { return MakeV2d(RoundR64(vec2d.x), RoundR64(vec2d.y)); }
PEXPI v3d RoundV3d(v3d  vec3d) { return MakeV3d(RoundR64(vec3d.x), RoundR64(vec3d.y), RoundR64(vec3d.z)); }
PEXPI v4d RoundV4d(v4d  vec4d) { return MakeV4d(RoundR64(vec4d.x), RoundR64(vec4d.y), RoundR64(vec4d.z), RoundR64(vec4d.w)); }
PEXPI v2i RoundV2di(v2d vec2d) { return MakeV2i((i32)RoundR64i(vec2d.x), (i32)RoundR64i(vec2d.y)); }
PEXPI v3i RoundV3di(v3d vec3d) { return MakeV3i((i32)RoundR64i(vec3d.x), (i32)RoundR64i(vec3d.y), (i32)RoundR64i(vec3d.z)); }
PEXPI v4i RoundV4di(v4d vec4d) { return MakeV4i((i32)RoundR64i(vec4d.x), (i32)RoundR64i(vec4d.y), (i32)RoundR64i(vec4d.z), (i32)RoundR64i(vec4d.w)); }

PEXPI v2  MinV2(v2   left, v2  right) { return  MakeV2(MinR32(left.x, right.x), MinR32(left.y, right.y)); }
PEXPI v3  MinV3(v3   left, v3  right) { return  MakeV3(MinR32(left.x, right.x), MinR32(left.y, right.y), MinR32(left.z, right.z)); }
PEXPI v4  MinV4(v4   left, v4  right) { return  MakeV4(MinR32(left.x, right.x), MinR32(left.y, right.y), MinR32(left.z, right.z), MinR32(left.w, right.w)); }
PEXPI v2d MinV2d(v2d left, v2d right) { return MakeV2d(MinR64(left.x, right.x), MinR64(left.y, right.y)); }
PEXPI v3d MinV3d(v3d left, v3d right) { return MakeV3d(MinR64(left.x, right.x), MinR64(left.y, right.y), MinR64(left.z, right.z)); }
PEXPI v4d MinV4d(v4d left, v4d right) { return MakeV4d(MinR64(left.x, right.x), MinR64(left.y, right.y), MinR64(left.z, right.z), MinR64(left.w, right.w)); }

PEXPI v2  MaxV2(v2   left, v2  right) { return  MakeV2(MaxR32(left.x, right.x), MaxR32(left.y, right.y)); }
PEXPI v3  MaxV3(v3   left, v3  right) { return  MakeV3(MaxR32(left.x, right.x), MaxR32(left.y, right.y), MaxR32(left.z, right.z)); }
PEXPI v4  MaxV4(v4   left, v4  right) { return  MakeV4(MaxR32(left.x, right.x), MaxR32(left.y, right.y), MaxR32(left.z, right.z), MaxR32(left.w, right.w)); }
PEXPI v2d MaxV2d(v2d left, v2d right) { return MakeV2d(MaxR64(left.x, right.x), MaxR64(left.y, right.y)); }
PEXPI v3d MaxV3d(v3d left, v3d right) { return MakeV3d(MaxR64(left.x, right.x), MaxR64(left.y, right.y), MaxR64(left.z, right.z)); }
PEXPI v4d MaxV4d(v4d left, v4d right) { return MakeV4d(MaxR64(left.x, right.x), MaxR64(left.y, right.y), MaxR64(left.z, right.z), MaxR64(left.w, right.w)); }

PEXPI v2  AbsV2(v2   vec2)  { return  MakeV2(AbsR32(vec2.x), AbsR32(vec2.y)); }
PEXPI v3  AbsV3(v3   vec3)  { return  MakeV3(AbsR32(vec3.x), AbsR32(vec3.y), AbsR32(vec3.z)); }
PEXPI v4  AbsV4(v4   vec4)  { return  MakeV4(AbsR32(vec4.x), AbsR32(vec4.y), AbsR32(vec4.z), AbsR32(vec4.w)); }
PEXPI v2i AbsV2i(v2i vec2i) { return MakeV2i(AbsI32(vec2i.x), AbsI32(vec2i.y)); }
PEXPI v3i AbsV3i(v3i vec3i) { return MakeV3i(AbsI32(vec3i.x), AbsI32(vec3i.y), AbsI32(vec3i.z)); }
PEXPI v4i AbsV4i(v4i vec4i) { return MakeV4i(AbsI32(vec4i.x), AbsI32(vec4i.y), AbsI32(vec4i.z), AbsI32(vec4i.w)); }
PEXPI v2d AbsV2d(v2d vec2d) { return MakeV2d(AbsR64(vec2d.x), AbsR64(vec2d.y)); }
PEXPI v3d AbsV3d(v3d vec3d) { return MakeV3d(AbsR64(vec3d.x), AbsR64(vec3d.y), AbsR64(vec3d.z)); }
PEXPI v4d AbsV4d(v4d vec4d) { return MakeV4d(AbsR64(vec4d.x), AbsR64(vec4d.y), AbsR64(vec4d.z), AbsR64(vec4d.w)); }

PEXPI v2  ClampV2(v2   vec2,  v2  minVec2,  v2  maxVec2)  { return  MakeV2(ClampR32(vec2.x, minVec2.x, maxVec2.x), ClampR32(vec2.y, minVec2.y, maxVec2.y)); }
PEXPI v3  ClampV3(v3   vec3,  v3  minVec3,  v3  maxVec3)  { return  MakeV3(ClampR32(vec3.x, minVec3.x, maxVec3.x), ClampR32(vec3.y, minVec3.y, maxVec3.y), ClampR32(vec3.z, minVec3.z, maxVec3.z)); }
PEXPI v4  ClampV4(v4   vec4,  v4  minVec4,  v4  maxVec4)  { return  MakeV4(ClampR32(vec4.x, minVec4.x, maxVec4.x), ClampR32(vec4.y, minVec4.y, maxVec4.y), ClampR32(vec4.z, minVec4.z, maxVec4.z), ClampR32(vec4.w, minVec4.w, maxVec4.w)); }
PEXPI v2i ClampV2i(v2i vec2i, v2i minVec2i, v2i maxVec2i) { return MakeV2i(ClampI32(vec2i.x, minVec2i.x, maxVec2i.x), ClampI32(vec2i.y, minVec2i.y, maxVec2i.y)); }
PEXPI v3i ClampV3i(v3i vec3i, v3i minVec3i, v3i maxVec3i) { return MakeV3i(ClampI32(vec3i.x, minVec3i.x, maxVec3i.x), ClampI32(vec3i.y, minVec3i.y, maxVec3i.y), ClampI32(vec3i.z, minVec3i.z, maxVec3i.z)); }
PEXPI v4i ClampV4i(v4i vec4i, v4i minVec4i, v4i maxVec4i) { return MakeV4i(ClampI32(vec4i.x, minVec4i.x, maxVec4i.x), ClampI32(vec4i.y, minVec4i.y, maxVec4i.y), ClampI32(vec4i.z, minVec4i.z, maxVec4i.z), ClampI32(vec4i.w, minVec4i.w, maxVec4i.w)); }
PEXPI v2d ClampV2d(v2d vec2d, v2d minVec2d, v2d maxVec2d) { return MakeV2d(ClampR64(vec2d.x, minVec2d.x, maxVec2d.x), ClampR64(vec2d.y, minVec2d.y, maxVec2d.y)); }
PEXPI v3d ClampV3d(v3d vec3d, v3d minVec3d, v3d maxVec3d) { return MakeV3d(ClampR64(vec3d.x, minVec3d.x, maxVec3d.x), ClampR64(vec3d.y, minVec3d.y, maxVec3d.y), ClampR64(vec3d.z, minVec3d.z, maxVec3d.z)); }
PEXPI v4d ClampV4d(v4d vec4d, v4d minVec4d, v4d maxVec4d) { return MakeV4d(ClampR64(vec4d.x, minVec4d.x, maxVec4d.x), ClampR64(vec4d.y, minVec4d.y, maxVec4d.y), ClampR64(vec4d.z, minVec4d.z, maxVec4d.z), ClampR64(vec4d.w, minVec4d.w, maxVec4d.w)); }

//TODO: Should we add "Inner" and "Determinant" functions like we had in GyLib?

// +--------------------------------------------------------------+
// |                   Other Helpful Functions                    |
// +--------------------------------------------------------------+
PEXPI v2 PerpRightV2(v2 vec2) { return MakeV2(-vec2.y, vec2.x); }
PEXPI v2 PerpLeftV2(v2 vec2) { return MakeV2(vec2.y, -vec2.x); }
PEXPI v2d PerpRightV2d(v2d vec2d) { return MakeV2d(-vec2d.y, vec2d.x); }
PEXPI v2d PerpLeftV2d(v2d vec2d) { return MakeV2d(vec2d.y, -vec2d.x); }

PEXPI v2 Vec2Rotate(v2 vec2, r32 angle)
{
	return MakeV2(
		CosR32(angle) * vec2.x - SinR32(angle) * vec2.y,
		SinR32(angle) * vec2.x + CosR32(angle) * vec2.y
	);
}
PEXPI v2d Vec2dRotate(v2d vec2d, r64 angle)
{
	return MakeV2d(
		CosR64(angle) * vec2d.x - SinR64(angle) * vec2d.y,
		SinR64(angle) * vec2d.x + CosR64(angle) * vec2d.y
	);
}

PEXPI r32 AngleBetweenV2(v2 left, v2 right)
{
	r32 dotProduct = DotV2(left, right);
	if (dotProduct == -1) { return Pi32; }
	return SignOfR32(left.x * right.y - left.y * right.x) * AcosR32(dotProduct / (LengthV2(left) * LengthV2(right)));
}
PEXPI r64 AngleBetweenV2d(v2d left, v2d right)
{
	r64 dotProduct = DotV2d(left, right);
	if (dotProduct == -1) { return Pi64; }
	return SignOfR64(left.x * right.y - left.y * right.x) * AcosR64(dotProduct / (LengthV2d(left) * LengthV2d(right)));
}

PEXPI r32 AngleBetweenV3(v3 left, v3 right)
{
	return AcosR32(DotV3(left, right) / (LengthV3(left) * LengthV3(right)));
}
PEXPI r64 AngleBetweenV3d(v3d left, v3d right)
{
	return AcosR64(DotV3d(left, right) / (LengthV3d(left) * LengthV3d(right)));
}

// This Assumes +Y is up and is phrased in terms of a first person camera (rotationUpDown = rotation around Z and then facingDirection = rotation around Y)
PEXPI v3 Vec3From2Angles(r32 facingDirection, r32 rotationUpDown, r32 radius)
{
	r32 circleRadius = CosR32(rotationUpDown) * radius;
	return MakeV3(
		CosR32(facingDirection) * circleRadius,
		SinR32(facingDirection) * circleRadius,
		SinR32(rotationUpDown) //TODO: Should this be multiplied by radius?
	);
}
PEXPI v3d Vec3dFrom2Angles(r64 facingDirection, r64 rotationUpDown, r64 radius)
{
	r64 circleRadius = CosR64(rotationUpDown) * radius;
	return MakeV3d(
		CosR64(facingDirection) * circleRadius,
		SinR64(facingDirection) * circleRadius,
		SinR64(rotationUpDown)
	);
}

//TODO: This should probably find a better home. I didn't want to put it in misc_triangulation.h because it doesn't need memory management
PEXP bool IsInsideTriangleV2(v2 p0, v2 p1, v2 p2, v2 queryPos)
{
	if (p0.x == p1.x && queryPos.x == p0.x && queryPos.y >= MinR32(p0.y, p1.y) && queryPos.y <= MaxR32(p0.y, p1.y)) { return true; }
	if (p1.x == p2.x && queryPos.x == p1.x && queryPos.y >= MinR32(p1.y, p2.y) && queryPos.y <= MaxR32(p1.y, p2.y)) { return true; }
	if (p2.x == p0.x && queryPos.x == p2.x && queryPos.y >= MinR32(p2.y, p0.y) && queryPos.y <= MaxR32(p2.y, p0.y)) { return true; }
	if (p0.y == p1.y && queryPos.y == p0.y && queryPos.x >= MinR32(p0.x, p1.x) && queryPos.x <= MaxR32(p0.x, p1.x)) { return true; }
	if (p1.y == p2.y && queryPos.y == p1.y && queryPos.x >= MinR32(p1.x, p2.x) && queryPos.x <= MaxR32(p1.x, p2.x)) { return true; }
	if (p2.y == p0.y && queryPos.y == p2.y && queryPos.x >= MinR32(p2.x, p0.x) && queryPos.x <= MaxR32(p2.x, p0.x)) { return true; }
	v2 perp0 = PerpRightV2(SubV2(p1, p0));
	v2 perp1 = PerpRightV2(SubV2(p2, p1));
	v2 perp2 = PerpRightV2(SubV2(p0, p2));
	return (
		SignOfR32(DotV2(SubV2(p2, p0), perp0)) == SignOfR32(DotV2(SubV2(queryPos, p0), perp0)) &&
		SignOfR32(DotV2(SubV2(p0, p1), perp1)) == SignOfR32(DotV2(SubV2(queryPos, p1), perp1)) &&
		SignOfR32(DotV2(SubV2(p1, p2), perp2)) == SignOfR32(DotV2(SubV2(queryPos, p2), perp2))
	);
}
PEXP r32 DistanceToTriangleEdgeV2(v2 p0, v2 p1, v2 p2, v2 queryPos)
{
	if (p0.x == p1.x && queryPos.x == p0.x && queryPos.y >= MinR32(p0.y, p1.y) && queryPos.y <= MaxR32(p0.y, p1.y)) { return 0.0f; }
	if (p1.x == p2.x && queryPos.x == p1.x && queryPos.y >= MinR32(p1.y, p2.y) && queryPos.y <= MaxR32(p1.y, p2.y)) { return 0.0f; }
	if (p2.x == p0.x && queryPos.x == p2.x && queryPos.y >= MinR32(p2.y, p0.y) && queryPos.y <= MaxR32(p2.y, p0.y)) { return 0.0f; }
	if (p0.y == p1.y && queryPos.y == p0.y && queryPos.x >= MinR32(p0.x, p1.x) && queryPos.x <= MaxR32(p0.x, p1.x)) { return 0.0f; }
	if (p1.y == p2.y && queryPos.y == p1.y && queryPos.x >= MinR32(p1.x, p2.x) && queryPos.x <= MaxR32(p1.x, p2.x)) { return 0.0f; }
	if (p2.y == p0.y && queryPos.y == p2.y && queryPos.x >= MinR32(p2.x, p0.x) && queryPos.x <= MaxR32(p2.x, p0.x)) { return 0.0f; }
	v2 edgeNorm01 = NormalizeV2(SubV2(p1, p0));
	v2 edgeNorm12 = NormalizeV2(SubV2(p2, p1));
	v2 edgeNorm20 = NormalizeV2(SubV2(p0, p2));
	v2 perp01 = PerpRightV2(edgeNorm01);
	v2 perp12 = PerpRightV2(edgeNorm12);
	v2 perp20 = PerpRightV2(edgeNorm20);
	r32 dot0 = -DotV2(edgeNorm20, perp01);
	r32 dot1 = -DotV2(edgeNorm01, perp12);
	r32 dot2 = -DotV2(edgeNorm12, perp20);
	r32 queryDot0 = DotV2(SubV2(queryPos, p0), perp01);
	r32 queryDot1 = DotV2(SubV2(queryPos, p1), perp12);
	r32 queryDot2 = DotV2(SubV2(queryPos, p2), perp20);
	bool insideEdge0 = (SignOfR32(dot0) == SignOfR32(queryDot0));
	bool insideEdge1 = (SignOfR32(dot1) == SignOfR32(queryDot1));
	bool insideEdge2 = (SignOfR32(dot2) == SignOfR32(queryDot2));
	if (insideEdge0 && insideEdge1 && insideEdge2)
	{
		return 0.0f;
	}
	else
	{
		if (!insideEdge0 && !insideEdge1) { return LengthV2(SubV2(queryPos, p1)); }
		else if (!insideEdge1 && !insideEdge2) { return LengthV2(SubV2(queryPos, p2)); }
		else if (!insideEdge2 && !insideEdge0) { return LengthV2(SubV2(queryPos, p0)); }
		else if (!insideEdge0) { return AbsR32(queryDot0); }
		else if (!insideEdge1) { return AbsR32(queryDot1); }
		else if (!insideEdge2) { return AbsR32(queryDot2); }
		else { return 0.0f; } //Really we shouldn't reach this
	}
}

// +--------------------------------------------------------------+
// |             Quickhand Functions for Layout Code              |
// +--------------------------------------------------------------+
PEXPI void AlignV2ToV2(v2* vectorOut, v2 alignmentScale)
{
	DebugNotNull(vectorOut);
	if (alignmentScale.x > 0) { vectorOut->x = RoundR32(vectorOut->x * alignmentScale.x) / alignmentScale.x; }
	if (alignmentScale.y > 0) { vectorOut->y = RoundR32(vectorOut->y * alignmentScale.y) / alignmentScale.y; }
}
PEXPI void AlignV2To(v2* vectorOut, r32 alignmentScale) { AlignV2ToV2(vectorOut, FillV2(alignmentScale)); }
PEXPI void AlignV2(v2* vectorOut) { AlignV2ToV2(vectorOut, V2_One); }
PEXPI void AlignV2dToV2d(v2d* vectorOut, v2d alignmentScale)
{
	DebugNotNull(vectorOut);
	if (alignmentScale.x > 0) { vectorOut->x = RoundR64(vectorOut->x * alignmentScale.x) / alignmentScale.x; }
	if (alignmentScale.y > 0) { vectorOut->y = RoundR64(vectorOut->y * alignmentScale.y) / alignmentScale.y; }
}
PEXPI void AlignV2dTo(v2d* vectorOut, r64 alignmentScale) { AlignV2dToV2d(vectorOut, FillV2d(alignmentScale)); }
PEXPI void AlignV2d(v2d* vectorOut) { AlignV2dToV2d(vectorOut, V2d_One); }

PEXPI void AlignV3ToV3(v3* vectorOut, v3 alignmentScale)
{
	DebugNotNull(vectorOut);
	if (alignmentScale.x > 0) { vectorOut->x = RoundR32(vectorOut->x * alignmentScale.x) / alignmentScale.x; }
	if (alignmentScale.y > 0) { vectorOut->y = RoundR32(vectorOut->y * alignmentScale.y) / alignmentScale.y; }
	if (alignmentScale.z > 0) { vectorOut->z = RoundR32(vectorOut->z * alignmentScale.z) / alignmentScale.z; }
}
PEXPI void AlignV3To(v3* vectorOut, r32 alignmentScale) { AlignV3ToV3(vectorOut, FillV3(alignmentScale)); }
PEXPI void AlignV3(v3* vectorOut) { AlignV3ToV3(vectorOut, V3_One); }
PEXPI void AlignV3dToV3d(v3d* vectorOut, v3d alignmentScale)
{
	DebugNotNull(vectorOut);
	if (alignmentScale.x > 0) { vectorOut->x = RoundR64(vectorOut->x * alignmentScale.x) / alignmentScale.x; }
	if (alignmentScale.y > 0) { vectorOut->y = RoundR64(vectorOut->y * alignmentScale.y) / alignmentScale.y; }
	if (alignmentScale.z > 0) { vectorOut->z = RoundR64(vectorOut->z * alignmentScale.z) / alignmentScale.z; }
}
PEXPI void AlignV3dTo(v3d* vectorOut, r64 alignmentScale) { AlignV3dToV3d(vectorOut, FillV3d(alignmentScale)); }
PEXPI void AlignV3d(v3d* vectorOut) { AlignV3dToV3d(vectorOut, V3d_One); }

PEXPI void AlignV4ToV4(v4* vectorOut, v4 alignmentScale)
{
	DebugNotNull(vectorOut);
	if (alignmentScale.x > 0) { vectorOut->x = RoundR32(vectorOut->x * alignmentScale.x) / alignmentScale.x; }
	if (alignmentScale.y > 0) { vectorOut->y = RoundR32(vectorOut->y * alignmentScale.y) / alignmentScale.y; }
	if (alignmentScale.z > 0) { vectorOut->z = RoundR32(vectorOut->z * alignmentScale.z) / alignmentScale.z; }
	if (alignmentScale.w > 0) { vectorOut->w = RoundR32(vectorOut->w * alignmentScale.w) / alignmentScale.w; }
}
PEXPI void AlignV4To(v4* vectorOut, r32 alignmentScale) { AlignV4ToV4(vectorOut, FillV4(alignmentScale)); }
PEXPI void AlignV4(v4* vectorOut) { AlignV4ToV4(vectorOut, V4_One); }
PEXPI void AlignV4dToV4d(v4d* vectorOut, v4d alignmentScale)
{
	DebugNotNull(vectorOut);
	if (alignmentScale.x > 0) { vectorOut->x = RoundR64(vectorOut->x * alignmentScale.x) / alignmentScale.x; }
	if (alignmentScale.y > 0) { vectorOut->y = RoundR64(vectorOut->y * alignmentScale.y) / alignmentScale.y; }
	if (alignmentScale.z > 0) { vectorOut->z = RoundR64(vectorOut->z * alignmentScale.z) / alignmentScale.z; }
	if (alignmentScale.w > 0) { vectorOut->w = RoundR64(vectorOut->w * alignmentScale.w) / alignmentScale.w; }
}
PEXPI void AlignV4dTo(v4d* vectorOut, r64 alignmentScale) { AlignV4dToV4d(vectorOut, FillV4d(alignmentScale)); }
PEXPI void AlignV4d(v4d* vectorOut) { AlignV4dToV4d(vectorOut, V4d_One); }

// +--------------------------------------------------------------+
// |                    C++ Function Overloads                    |
// +--------------------------------------------------------------+
//NOTE: See cross_vectors_quaternion_matrices_and_rectangles.h for C macros that use C23 _Generic to generate the type-specific function name
#if LANGUAGE_IS_CPP

PEXPI v2  Add(v2  left, v2  right) { return AddV2(left,  right); }
PEXPI v3  Add(v3  left, v3  right) { return AddV3(left,  right); }
PEXPI v4  Add(v4  left, v4  right) { return AddV4(left,  right); }
PEXPI v2i Add(v2i left, v2i right) { return AddV2i(left, right); }
PEXPI v3i Add(v3i left, v3i right) { return AddV3i(left, right); }
PEXPI v4i Add(v4i left, v4i right) { return AddV4i(left, right); }
PEXPI v2d Add(v2d left, v2d right) { return AddV2d(left, right); }
PEXPI v3d Add(v3d left, v3d right) { return AddV3d(left, right); }
PEXPI v4d Add(v4d left, v4d right) { return AddV4d(left, right); }

PEXPI v2  Sub(v2  left, v2  right) { return SubV2(left,  right); }
PEXPI v3  Sub(v3  left, v3  right) { return SubV3(left,  right); }
PEXPI v4  Sub(v4  left, v4  right) { return SubV4(left,  right); }
PEXPI v2i Sub(v2i left, v2i right) { return SubV2i(left, right); }
PEXPI v3i Sub(v3i left, v3i right) { return SubV3i(left, right); }
PEXPI v4i Sub(v4i left, v4i right) { return SubV4i(left, right); }
PEXPI v2d Sub(v2d left, v2d right) { return SubV2d(left, right); }
PEXPI v3d Sub(v3d left, v3d right) { return SubV3d(left, right); }
PEXPI v4d Sub(v4d left, v4d right) { return SubV4d(left, right); }

PEXPI v2  Mul(v2  left, v2  right) { return MulV2(left,  right); }
PEXPI v3  Mul(v3  left, v3  right) { return MulV3(left,  right); }
PEXPI v4  Mul(v4  left, v4  right) { return MulV4(left,  right); }
PEXPI v2i Mul(v2i left, v2i right) { return MulV2i(left, right); }
PEXPI v3i Mul(v3i left, v3i right) { return MulV3i(left, right); }
PEXPI v4i Mul(v4i left, v4i right) { return MulV4i(left, right); }
PEXPI v2d Mul(v2d left, v2d right) { return MulV2d(left, right); }
PEXPI v3d Mul(v3d left, v3d right) { return MulV3d(left, right); }
PEXPI v4d Mul(v4d left, v4d right) { return MulV4d(left, right); }

PEXPI v2  Div(v2  left, v2  right) { return DivV2(left,  right); }
PEXPI v3  Div(v3  left, v3  right) { return DivV3(left,  right); }
PEXPI v4  Div(v4  left, v4  right) { return DivV4(left,  right); }
PEXPI v2i Div(v2i left, v2i right) { return DivV2i(left, right); }
PEXPI v3i Div(v3i left, v3i right) { return DivV3i(left, right); }
PEXPI v4i Div(v4i left, v4i right) { return DivV4i(left, right); }
PEXPI v2d Div(v2d left, v2d right) { return DivV2d(left, right); }
PEXPI v3d Div(v3d left, v3d right) { return DivV3d(left, right); }
PEXPI v4d Div(v4d left, v4d right) { return DivV4d(left, right); }

PEXPI v2  Mul(v2  vector, r32 scalar) { return ScaleV2(vector,  scalar); }
PEXPI v3  Mul(v3  vector, r32 scalar) { return ScaleV3(vector,  scalar); }
PEXPI v4  Mul(v4  vector, r32 scalar) { return ScaleV4(vector,  scalar); }
PEXPI v2i Mul(v2i vector, i32 scalar) { return ScaleV2i(vector, scalar); }
PEXPI v3i Mul(v3i vector, i32 scalar) { return ScaleV3i(vector, scalar); }
PEXPI v4i Mul(v4i vector, i32 scalar) { return ScaleV4i(vector, scalar); }
PEXPI v2d Mul(v2d vector, r64 scalar) { return ScaleV2d(vector, scalar); }
PEXPI v3d Mul(v3d vector, r64 scalar) { return ScaleV3d(vector, scalar); }
PEXPI v4d Mul(v4d vector, r64 scalar) { return ScaleV4d(vector, scalar); }

PEXPI v2  Div(v2  vector, r32 scalar) { return ShrinkV2(vector,  scalar); }
PEXPI v3  Div(v3  vector, r32 scalar) { return ShrinkV3(vector,  scalar); }
PEXPI v4  Div(v4  vector, r32 scalar) { return ShrinkV4(vector,  scalar); }
PEXPI v2i Div(v2i vector, i32 scalar) { return ShrinkV2i(vector, scalar); }
PEXPI v3i Div(v3i vector, i32 scalar) { return ShrinkV3i(vector, scalar); }
PEXPI v4i Div(v4i vector, i32 scalar) { return ShrinkV4i(vector, scalar); }
PEXPI v2d Div(v2d vector, r64 scalar) { return ShrinkV2d(vector, scalar); }
PEXPI v3d Div(v3d vector, r64 scalar) { return ShrinkV3d(vector, scalar); }
PEXPI v4d Div(v4d vector, r64 scalar) { return ShrinkV4d(vector, scalar); }

PEXPI r32 LengthSquared(v2  vector) { return LengthSquaredV2(vector);  }
PEXPI r32 LengthSquared(v3  vector) { return LengthSquaredV3(vector);  }
PEXPI r32 LengthSquared(v4  vector) { return LengthSquaredV4(vector);  }
PEXPI i32 LengthSquared(v2i vector) { return LengthSquaredV2i(vector); }
PEXPI i32 LengthSquared(v3i vector) { return LengthSquaredV3i(vector); }
PEXPI i32 LengthSquared(v4i vector) { return LengthSquaredV4i(vector); }
PEXPI r64 LengthSquared(v2d vector) { return LengthSquaredV2d(vector); }
PEXPI r64 LengthSquared(v3d vector) { return LengthSquaredV3d(vector); }
PEXPI r64 LengthSquared(v4d vector) { return LengthSquaredV4d(vector); }

PEXPI r32 Length(v2  vector) { return LengthV2(vector);  }
PEXPI r32 Length(v3  vector) { return LengthV3(vector);  }
PEXPI r32 Length(v4  vector) { return LengthV4(vector);  }
PEXPI r32 Length(v2i vector) { return LengthV2i(vector); }
PEXPI r32 Length(v3i vector) { return LengthV3i(vector); }
PEXPI r32 Length(v4i vector) { return LengthV4i(vector); }
PEXPI r64 Length(v2d vector) { return LengthV2d(vector); }
PEXPI r64 Length(v3d vector) { return LengthV3d(vector); }
PEXPI r64 Length(v4d vector) { return LengthV4d(vector); }

PEXPI v2  Normalize(v2  vector) { return NormalizeV2(vector);  }
PEXPI v3  Normalize(v3  vector) { return NormalizeV3(vector);  }
PEXPI v4  Normalize(v4  vector) { return NormalizeV4(vector);  }
//NOTE: No Normalize functions for integer vector types
PEXPI v2d Normalize(v2d vector) { return NormalizeV2d(vector); }
PEXPI v3d Normalize(v3d vector) { return NormalizeV3d(vector); }
PEXPI v4d Normalize(v4d vector) { return NormalizeV4d(vector); }

PEXPI r32 Dot(v2  left, v2  right) { return DotV2(left,  right); }
PEXPI r32 Dot(v3  left, v3  right) { return DotV3(left,  right); }
PEXPI r32 Dot(v4  left, v4  right) { return DotV4(left,  right); }
PEXPI i32 Dot(v2i left, v2i right) { return DotV2i(left, right); }
PEXPI i32 Dot(v3i left, v3i right) { return DotV3i(left, right); }
PEXPI i32 Dot(v4i left, v4i right) { return DotV4i(left, right); }
PEXPI r64 Dot(v2d left, v2d right) { return DotV2d(left, right); }
PEXPI r64 Dot(v3d left, v3d right) { return DotV3d(left, right); }
PEXPI r64 Dot(v4d left, v4d right) { return DotV4d(left, right); }

PEXPI v2  Lerp(v2  left, v2  right, r32 amount) { return LerpV2(left,  right, amount); }
PEXPI v3  Lerp(v3  left, v3  right, r32 amount) { return LerpV3(left,  right, amount); }
PEXPI v4  Lerp(v4  left, v4  right, r32 amount) { return LerpV4(left,  right, amount); }
//NOTE: No Lerp functions for integer vector types
PEXPI v2d Lerp(v2d left, v2d right, r32 amount) { return LerpV2d(left, right, amount); }
PEXPI v3d Lerp(v3d left, v3d right, r32 amount) { return LerpV3d(left, right, amount); }
PEXPI v4d Lerp(v4d left, v4d right, r32 amount) { return LerpV4d(left, right, amount); }

PEXPI bool AreEqual(v2  left, v2  right) { return AreEqualV2(left,  right); }
PEXPI bool AreEqual(v3  left, v3  right) { return AreEqualV3(left,  right); }
PEXPI bool AreEqual(v4  left, v4  right) { return AreEqualV4(left,  right); }
PEXPI bool AreEqual(v2i left, v2i right) { return AreEqualV2i(left, right); }
PEXPI bool AreEqual(v3i left, v3i right) { return AreEqualV3i(left, right); }
PEXPI bool AreEqual(v4i left, v4i right) { return AreEqualV4i(left, right); }
PEXPI bool AreEqual(v2d left, v2d right) { return AreEqualV2d(left, right); }
PEXPI bool AreEqual(v3d left, v3d right) { return AreEqualV3d(left, right); }
PEXPI bool AreEqual(v4d left, v4d right) { return AreEqualV4d(left, right); }

#endif

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_VECTORS_H

#if defined(_MISC_PARSING_H) && defined(_STRUCT_VECTORS_H)
#include "cross/cross_parsing_and_vectors.h"
#endif

#if defined(_STRUCT_VECTORS_H) && defined(_STRUCT_QUATERNION_H) && defined(_STRUCT_MATRICES_H) && defined(_STRUCT_RECTANGLES_H)
#include "cross/cross_vectors_quaternion_matrices_and_rectangles.h"
#endif

#if defined(_STRUCT_COLOR_H) && defined(_STRUCT_VECTORS_H)
#include "cross/cross_color_and_vectors.h"
#endif

#if defined(_STRUCT_VECTORS_H) && defined(RAYLIB_H)
#include "cross/cross_vectors_and_raylib.h"
#endif

#if defined(_STRUCT_VECTORS_H) && BUILD_WITH_BOX3D
#include "cross/cross_vectors_and_box3d.h"
#endif

#if defined(_STRUCT_VECTORS_H) && defined(_STRUCT_QUATERNION_H) && BUILD_WITH_BOX3D
#include "cross/cross_vectors_quaternion_and_box3d.h"
#endif
