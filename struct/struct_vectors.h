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

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "std/std_includes.h"
#include "std/std_basic_math.h"
#include "std/std_math_ex.h"
#include "std/std_trig.h"
#include "lib/lib_handmade_math.h"

// +--------------------------------------------------------------+
// |                   Typedefs and Structures                    |
// +--------------------------------------------------------------+
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

#define MakeV2_Const(x, y)       { .X=(x), .Y=(y) }
#define MakeV3_Const(x, y, z)    { .X=(x), .Y=(y), .Z=(z) }
#define MakeV4_Const(x, y, z, w) { .X=(x), .Y=(y), .Z=(z), .W=(w) }
#define MakeV2(x, y)             NEW_STRUCT(HMM_Vec2)MakeV2_Const((x), (y))
#define MakeV3(x, y, z)          NEW_STRUCT(HMM_Vec3)MakeV3_Const((x), (y), (z))
#define MakeV4(x, y, z, w)       NEW_STRUCT(HMM_Vec4)MakeV4_Const((x), (y), (z), (w))

#define FillV2_Const(value)  MakeV2_Const((value), (value))
#define FillV3_Const(value)  MakeV3_Const((value), (value), (value))
#define FillV4_Const(value)  MakeV4_Const((value), (value), (value), (value))
#define FillV2(value)        MakeV2((value), (value))
#define FillV3(value)        MakeV3((value), (value), (value))
#define FillV4(value)        MakeV4((value), (value), (value), (value))

//NOTE: Vec4Raw is exactly the same as HMM_Vec4 but it doesn't include the SSE type __m128
//      which means it's alignment is 4 instead of 16! HMM_Vec4 is the only HMM vector
//      type that is 16-byte aligned, so this is the only "Raw" variant we have
typedef car Vec4Raw Vec4Raw;
car Vec4Raw
{
	r32 Elements[4];
	
	plex
	{
		car
		{
			HMM_Vec3 XYZ;
			plex { r32 X, Y, Z; };
		};
		r32 W;
	};
	plex
	{
		car
		{
			HMM_Vec3 RGB;
			plex { r32 R, G, B; };
		};
		r32 A;
	};
	
	plex { HMM_Vec2 XY; r32 _Ignored0; r32 _Ignored1; };
	plex { r32 _Ignored2; HMM_Vec2 YZ; r32 _Ignored3; };
	plex { r32 _Ignored4; r32 _Ignored5; HMM_Vec2 ZW; };

};
#define MakeV4r_Const(x, y, z, w) { .X=(x), .Y=(y), .Z=(z), .W=(w) }
#define MakeV4r(x, y, z, w)       NEW_STRUCT(Vec4Raw)MakeV4r_Const((x), (y), (z), (w))
#define FillV4r_Const(value)      MakeV4r_Const((value), (value), (value), (value))
#define FillV4r(value)            MakeV4r((value), (value), (value), (value))

typedef car Vec2i Vec2i;
car Vec2i
{
	plex { i32 X, Y; };
	plex { i32 U, V; };
	plex { i32 Left, Right; };
	plex { i32 Width, Height; };
	i32 Elements[2];
};
#define MakeV2i_Const(x, y)  { .X=(x), .Y=(y) }
#define MakeV2i(x, y)        NEW_STRUCT(Vec2i)MakeV2i_Const((x), (y))
#define FillV2i_Const(value) MakeV2i_Const((value), (value))
#define FillV2i(value)       MakeV2i((value), (value))

typedef car Vec3i Vec3i;
car Vec3i
{
	plex { i32 X, Y, Z; };
	plex { i32 U, V, W; };
	plex { i32 R, G, B; };
	plex { Vec2i XY; i32 _Ignored0; };
	plex { i32 _Ignored1; Vec2i YZ; };
	plex { Vec2i UV; i32 _Ignored2; };
	plex { i32 _Ignored3; Vec2i VW; };
	i32 Elements[3];
};
#define MakeV3i_Const(x, y, z) { .X=(x), .Y=(y), .Z=(z) }
#define MakeV3i(x, y, z)       NEW_STRUCT(Vec3i)MakeV3i_Const((x), (y), (z))
#define FillV3i_Const(value)   MakeV3i_Const((value), (value), (value))
#define FillV3i(value)         MakeV3i((value), (value), (value))

typedef car Vec4i Vec4i;
car Vec4i
{
	plex
	{
		car
		{
			Vec3i XYZ;
			plex { i32 X, Y, Z; };
		};
		i32 W;
	};
	plex
	{
		car
		{
			Vec3i RGB;
			plex { i32 R, G, B; };
		};
		i32 A;
	};
	plex { Vec2i XY; i32 _Ignored0; i32 _Ignored1; };
	plex { i32 _Ignored2; Vec2i YZ; i32 _Ignored3; };
	plex { i32 _Ignored4; i32 _Ignored5; Vec2i ZW; };
	i32 Elements[4];
};
#define MakeV4i_Const(x, y, z, w) { .X=(x), .Y=(y), .Z=(z), .W=(w) }
#define MakeV4i(x, y, z, w)       NEW_STRUCT(Vec4i)MakeV4i_Const((x), (y), (z), (w))
#define FillV4i_Const(value)      MakeV4i_Const((value), (value), (value), (value))
#define FillV4i(value)            MakeV4i((value), (value), (value), (value))

typedef car Vec2R64 Vec2R64;
car Vec2R64
{
	r64 Elements[2];
	plex { r64 X, Y; };
	plex { r64 U, V; };
	plex { r64 Width, Height; };
	plex { r64 Lon, Lat; };
	plex { r64 Longitude, Latitude; };
};
#define MakeV2d_Const(x, y)  { .X=(x), .Y=(y) }
#define MakeV2d(x, y)        NEW_STRUCT(Vec2R64)MakeV2d_Const((x), (y))
#define FillV2d_Const(value) MakeV2d_Const((value), (value))
#define FillV2d(value)       MakeV2d((value), (value))

typedef car Vec3R64 Vec3R64;
car Vec3R64
{
	r64 Elements[3];
	plex { r64 X, Y, Z; };
	plex { Vec2R64 XY; r64 _Ignored0; };
	plex { r64 _Ignored1; Vec2R64 YZ; };
	plex { r64 R, G, B; };
	plex { r64 U, V, _Ignored2; };
	plex { Vec2R64 UV; r64 _Ignored3; };
	plex { r64 _Ignored4; Vec2R64 VW; };
	plex { r64 Width, Height, Depth; };
};
#define MakeV3d_Const(x, y, z) { .X=(x), .Y=(y), .Z=(z) }
#define MakeV3d(x, y, z)       NEW_STRUCT(Vec3R64)MakeV3d_Const((x), (y), (z))
#define FillV3d_Const(value)   MakeV3d_Const((value), (value), (value))
#define FillV3d(value)         MakeV3d((value), (value), (value))

typedef car Vec4R64 Vec4R64;
car Vec4R64
{
	r64 Elements[4];
	plex
	{
		car
		{
			Vec3R64 XYZ;
			plex { r64 X, Y, Z; };
		};
		r64 W;
	};
	plex
	{
		car
		{
			Vec3R64 RGB;
			plex { r64 R, G, B; };
		};
		r64 A;
	};
	plex { Vec2R64 XY; r64 _Ignored0; r64 _Ignored1; };
	plex { r64 _Ignored2; Vec2R64 YZ; r64 _Ignored3; };
	plex { r64 _Ignored4; r64 _Ignored5; Vec2R64 ZW; };
};
#define MakeV4d_Const(x, y, z, w) { .X=(x), .Y=(y), .Z=(z), .W=(w) }
#define MakeV4d(x, y, z, w)       NEW_STRUCT(Vec4R64)MakeV4d_Const((x), (y), (z), (w))
#define FillV4d_Const(value)      MakeV4d_Const((value), (value), (value), (value))
#define FillV4d(value)            MakeV4d((value), (value), (value), (value))

//NOTE: The default assumption for vectors is r32 members
//    'i' suffix refers to "integer" members and implicitly means 32-bit (unsigned and 64-bit integers are not yet supported)
//    'd' suffix refers to "double" members which are 64-bit (we went with a 'd' suffix instead of something like 'r64' because a single character suffix reads a lot nicer and "double" is a well known word to refer to 64-bit floating point numbers)
//    'r' suffix refers to "raw", which is the same as normal (r32 members) but without SIMD member so alignment is 4 instead of 16
typedef HMM_Vec2 v2;
typedef Vec2i    v2i;
typedef Vec2R64  v2d;

typedef HMM_Vec3 v3;
typedef Vec3i    v3i;
typedef Vec3R64  v3d;

typedef HMM_Vec4 v4;
typedef Vec4Raw  v4r;
typedef Vec4i    v4i;
typedef Vec4R64  v4d;

// +--------------------------------------------------------------+
// |                      Helper Slice Types                      |
// +--------------------------------------------------------------+
typedef plex Vec2Slice Vec2Slice;
plex Vec2Slice
{
	uxx length;
	car { void* pntr; r32 components; HMM_Vec2* vectors; };
};
typedef plex Vec2iSlice Vec2iSlice;
plex Vec2iSlice
{
	uxx length;
	car { void* pntr; i32 components; Vec2i* vectors; };
};
typedef plex Vec2R64Slice Vec2R64Slice;
plex Vec2R64Slice
{
	uxx length;
	car { void* pntr; r64 components; Vec2R64* vectors; };
};
typedef plex Vec3Slice Vec3Slice;
plex Vec3Slice
{
	uxx length;
	car { void* pntr; r32 components; HMM_Vec3* vectors; };
};
typedef plex Vec3iSlice Vec3iSlice;
plex Vec3iSlice
{
	uxx length;
	car { void* pntr; i32 components; Vec3i* vectors; };
};
typedef plex Vec3R64Slice Vec3R64Slice;
plex Vec3R64Slice
{
	uxx length;
	car { void* pntr; r64 components; Vec3R64* vectors; };
};
typedef plex Vec4Slice Vec4Slice;
plex Vec4Slice
{
	uxx length;
	car { void* pntr; r32 components; HMM_Vec4* vectors; };
};
typedef plex Vec4RawSlice Vec4RawSlice;
plex Vec4RawSlice
{
	uxx length;
	car { void* pntr; r32 components; Vec4Raw* vectors; };
};
typedef plex Vec4iSlice Vec4iSlice;
plex Vec4iSlice
{
	uxx length;
	car { void* pntr; i32 components; Vec4i* vectors; };
};
typedef plex Vec4R64Slice Vec4R64Slice;
plex Vec4R64Slice
{
	uxx length;
	car { void* pntr; r64 components; Vec4R64* vectors; };
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
	PIG_CORE_INLINE v4i ToV4iFrom3(v3i vec3i, i32 w);
	PIG_CORE_INLINE v4r ToV4rFrom4(v4 vec4);
	PIG_CORE_INLINE v4 ToV4Fromr(v4r vec4r);
	#if TARGET_IS_ORCA
	PIG_CORE_INLINE oc_vec2 ToOcVec2(v2 vector);
	PIG_CORE_INLINE v2 ToV2FromOc(oc_vec2 orcaVector);
	PIG_CORE_INLINE oc_vec3 ToOcVec3(v3 vector);
	PIG_CORE_INLINE v3 ToV3FromOc(oc_vec3 orcaVector);
	PIG_CORE_INLINE oc_vec4 ToOcVec4(v4 vector);
	PIG_CORE_INLINE v4 ToV4FromOc(oc_vec4 orcaVector);
	PIG_CORE_INLINE oc_vec2i ToOcVec2i(v2i vector);
	PIG_CORE_INLINE v2i ToV2iFromOc(oc_vec2i orcaVector);
	PIG_CORE_INLINE oc_color ToOcColorFromV4r(v4r colorVec);
	PIG_CORE_INLINE v4r ToV4rFromOcColor(oc_color orcaColor);
	#endif //TARGET_IS_ORCA
	PIG_CORE_INLINE v2i AddV2i(v2i left, v2i right);
	PIG_CORE_INLINE v3i AddV3i(v3i left, v3i right);
	PIG_CORE_INLINE v4i AddV4i(v4i left, v4i right);
	PIG_CORE_INLINE v2d AddV2d(v2d left, v2d right);
	PIG_CORE_INLINE v3d AddV3d(v3d left, v3d right);
	PIG_CORE_INLINE v4d AddV4d(v4d left, v4d right);
	PIG_CORE_INLINE v2i SubV2i(v2i left, v2i right);
	PIG_CORE_INLINE v3i SubV3i(v3i left, v3i right);
	PIG_CORE_INLINE v4i SubV4i(v4i left, v4i right);
	PIG_CORE_INLINE v2d SubV2d(v2d left, v2d right);
	PIG_CORE_INLINE v3d SubV3d(v3d left, v3d right);
	PIG_CORE_INLINE v4d SubV4d(v4d left, v4d right);
	PIG_CORE_INLINE v2i MulV2i(v2i left, v2i right);
	PIG_CORE_INLINE v3i MulV3i(v3i left, v3i right);
	PIG_CORE_INLINE v4i MulV4i(v4i left, v4i right);
	PIG_CORE_INLINE v2d MulV2d(v2d left, v2d right);
	PIG_CORE_INLINE v3d MulV3d(v3d left, v3d right);
	PIG_CORE_INLINE v4d MulV4d(v4d left, v4d right);
	PIG_CORE_INLINE v2i DivV2i(v2i left, v2i right);
	PIG_CORE_INLINE v3i DivV3i(v3i left, v3i right);
	PIG_CORE_INLINE v4i DivV4i(v4i left, v4i right);
	PIG_CORE_INLINE v2d DivV2d(v2d left, v2d right);
	PIG_CORE_INLINE v3d DivV3d(v3d left, v3d right);
	PIG_CORE_INLINE v4d DivV4d(v4d left, v4d right);
	PIG_CORE_INLINE v2i ScaleV2i(v2i left, i32 scalar);
	PIG_CORE_INLINE v3i ScaleV3i(v3i left, i32 scalar);
	PIG_CORE_INLINE v4i ScaleV4i(v4i left, i32 scalar);
	PIG_CORE_INLINE v2d ScaleV2d(v2d left, r64 scalar);
	PIG_CORE_INLINE v3d ScaleV3d(v3d left, r64 scalar);
	PIG_CORE_INLINE v4d ScaleV4d(v4d left, r64 scalar);
	PIG_CORE_INLINE v2i ShrinkV2i(v2i left, i32 divisor);
	PIG_CORE_INLINE v3i ShrinkV3i(v3i left, i32 divisor);
	PIG_CORE_INLINE v4i ShrinkV4i(v4i left, i32 divisor);
	PIG_CORE_INLINE v2d ShrinkV2d(v2d left, r64 divisor);
	PIG_CORE_INLINE v3d ShrinkV3d(v3d left, r64 divisor);
	PIG_CORE_INLINE v4d ShrinkV4d(v4d left, r64 divisor);
	PIG_CORE_INLINE bool AreEqualV4r(v4r left, v4r right);
	PIG_CORE_INLINE bool AreEqualV2i(v2i left, v2i right);
	PIG_CORE_INLINE bool AreEqualV3i(v3i left, v3i right);
	PIG_CORE_INLINE bool AreEqualV4i(v4i left, v4i right);
	PIG_CORE_INLINE bool AreEqualV2d(v2d left, v2d right);
	PIG_CORE_INLINE bool AreEqualV3d(v3d left, v3d right);
	PIG_CORE_INLINE bool AreEqualV4d(v4d left, v4d right);
	PIG_CORE_INLINE i32 DotV2i(v2i left, v2i right);
	PIG_CORE_INLINE i32 DotV3i(v3i left, v3i right);
	PIG_CORE_INLINE i32 DotV4i(v4i left, v4i right);
	PIG_CORE_INLINE r64 DotV2d(v2d left, v2d right);
	PIG_CORE_INLINE r64 DotV3d(v3d left, v3d right);
	PIG_CORE_INLINE r64 DotV4d(v4d left, v4d right);
	PIG_CORE_INLINE v3i CrossV3i(v3i left, v3i right);
	PIG_CORE_INLINE v3d CrossV3d(v3d left, v3d right);
	PIG_CORE_INLINE i32 LengthSquaredV2i(v2i vec2i);
	PIG_CORE_INLINE i32 LengthSquaredV3i(v3i vec3i);
	PIG_CORE_INLINE i32 LengthSquaredV4i(v4i vec4i);
	PIG_CORE_INLINE r64 LengthSquaredV2d(v2d vec2d);
	PIG_CORE_INLINE r64 LengthSquaredV3d(v3d vec3d);
	PIG_CORE_INLINE r64 LengthSquaredV4d(v4d vec4d);
	PIG_CORE_INLINE r32 LengthV2i(v2i vec2i);
	PIG_CORE_INLINE r32 LengthV3i(v3i vec3i);
	PIG_CORE_INLINE r32 LengthV4i(v4i vec4i);
	PIG_CORE_INLINE r64 LengthV2d(v2d vec2d);
	PIG_CORE_INLINE r64 LengthV3d(v3d vec3d);
	PIG_CORE_INLINE r64 LengthV4d(v4d vec4d);
	PIG_CORE_INLINE v2d NormalizeV2d(v2d vec2d);
	PIG_CORE_INLINE v3d NormalizeV3d(v3d vec2d);
	PIG_CORE_INLINE v4d NormalizeV4d(v4d vec2d);
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

#define ToV4From3(vec3, w) HMM_V4V((vec3), (w))

#define AddV2(left, right) HMM_AddV2((left), (right))
#define AddV3(left, right) HMM_AddV3((left), (right))
#define AddV4(left, right) HMM_AddV4((left), (right))

#define SubV2(left, right) HMM_SubV2((left), (right))
#define SubV3(left, right) HMM_SubV3((left), (right))
#define SubV4(left, right) HMM_SubV4((left), (right))

#define MulV2(left, right) HMM_MulV2((left), (right))
#define MulV3(left, right) HMM_MulV3((left), (right))
#define MulV4(left, right) HMM_MulV4((left), (right))

#define DivV2(left, right) HMM_DivV2((left), (right))
#define DivV3(left, right) HMM_DivV3((left), (right))
#define DivV4(left, right) HMM_DivV4((left), (right))

#define ScaleV2(vec2, scalar) HMM_MulV2F((vec2), (scalar))
#define ScaleV3(vec3, scalar) HMM_MulV3F((vec3), (scalar))
#define ScaleV4(vec4, scalar) HMM_MulV4F((vec4), (scalar))

#define ShrinkV2(vec2, divisor) HMM_DivV2F((vec2), (divisor))
#define ShrinkV3(vec3, divisor) HMM_DivV3F((vec3), (divisor))
#define ShrinkV4(vec4, divisor) HMM_DivV4F((vec4), (divisor))

#define AreEqualV2(left, right) HMM_EqV2((left), (right))
#define AreEqualV3(left, right) HMM_EqV3((left), (right))
#define AreEqualV4(left, right) HMM_EqV4((left), (right))

#define AreSimilarV2(left, right, tolerance) (AreSimilarR32((left).X, (right).X, (tolerance)) && AreSimilarR32((left).Y, (right).Y, (tolerance)))
#define AreSimilarV3(left, right, tolerance) (AreSimilarR32((left).X, (right).X, (tolerance)) && AreSimilarR32((left).Y, (right).Y, (tolerance)) && AreSimilarR32((left).Z, (right).Z, (tolerance)))
#define AreSimilarV4(left, right, tolerance) (AreSimilarR32((left).X, (right).X, (tolerance)) && AreSimilarR32((left).Y, (right).Y, (tolerance)) && AreSimilarR32((left).Z, (right).Z, (tolerance)) && AreSimilarR32((left).W, (right).W, (tolerance)))
#define AreSimilarV2d(left, right, tolerance) (AreSimilarR64((left).X, (right).X, (tolerance)) && AreSimilarR64((left).Y, (right).Y, (tolerance)))
#define AreSimilarV3d(left, right, tolerance) (AreSimilarR64((left).X, (right).X, (tolerance)) && AreSimilarR64((left).Y, (right).Y, (tolerance)) && AreSimilarR64((left).Z, (right).Z, (tolerance)))
#define AreSimilarV4d(left, right, tolerance) (AreSimilarR64((left).X, (right).X, (tolerance)) && AreSimilarR64((left).Y, (right).Y, (tolerance)) && AreSimilarR64((left).Z, (right).Z, (tolerance)) && AreSimilarR64((left).W, (right).W, (tolerance)))

#define DotV2(left, right) HMM_DotV2((left), (right))
#define DotV3(left, right) HMM_DotV3((left), (right))
#define DotV4(left, right) HMM_DotV4((left), (right))

#define CrossV3(left, right) HMM_Cross((left), (right))

#define LengthSquaredV2(vec2) HMM_LenSqrV2(vec2)
#define LengthSquaredV3(vec3) HMM_LenSqrV3(vec3)
#define LengthSquaredV4(vec4) HMM_LenSqrV4(vec4)

#define LengthV2(vec2) HMM_LenV2(vec2)
#define LengthV3(vec3) HMM_LenV3(vec3)
#define LengthV4(vec4) HMM_LenV4(vec4)

#define NormalizeV2(vec2) HMM_NormV2(vec2)
#define NormalizeV3(vec3) HMM_NormV3(vec3)
#define NormalizeV4(vec4) HMM_NormV4(vec4)

#define LerpV2(start, end, amount) HMM_LerpV2((start), (amount), (end))
#define LerpV3(start, end, amount) HMM_LerpV3((start), (amount), (end))
#define LerpV4(start, end, amount) HMM_LerpV4((start), (amount), (end))

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
#define V4r_Zero_Const  MakeV4r_Const(0.0f, 0.0f, 0.0f, 0.0f)
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

#define V4r_Zero      MakeV4r( 0.0f,  0.0f,  0.0f,  0.0f)
#define V4r_One       MakeV4r( 1.0f,  1.0f,  1.0f,  1.0f)
#define V4r_Half      MakeV4r( 0.5f,  0.5f,  0.5f,  0.5f)
#define V4r_Left      MakeV4r(-1.0f,  0.0f,  0.0f,  0.0f)
#define V4r_Right     MakeV4r( 1.0f,  0.0f,  0.0f,  0.0f)
#define V4r_Bottom    MakeV4r( 0.0f, -1.0f,  0.0f,  0.0f)
#define V4r_Top       MakeV4r( 0.0f,  1.0f,  0.0f,  0.0f)
#define V4r_Back      MakeV4r( 0.0f,  0.0f, -1.0f,  0.0f)
#define V4r_Front     MakeV4r( 0.0f,  0.0f,  1.0f,  0.0f)
#define V4r_Down      MakeV4r( 0.0f, -1.0f,  0.0f,  0.0f)
#define V4r_Up        MakeV4r( 0.0f,  1.0f,  0.0f,  0.0f)
#define V4r_Backward  MakeV4r( 0.0f,  0.0f, -1.0f,  0.0f)
#define V4r_Forward   MakeV4r( 0.0f,  0.0f,  1.0f,  0.0f)
#define V4r_Prevward  MakeV4r( 0.0f,  0.0f,  0.0f, -1.0f)
#define V4r_Nextward  MakeV4r( 0.0f,  0.0f,  0.0f,  1.0f)

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
PEXPI v2 ToV2Fromi(v2i vec2i) { return MakeV2((r32)vec2i.X, (r32)vec2i.Y); }
PEXPI v2d ToV2dFromi(v2i vec2i) { return MakeV2d((r64)vec2i.X, (r64)vec2i.Y); }
PEXPI v2d ToV2dFromf(v2 vec2) { return MakeV2d((r64)vec2.X, (r64)vec2.Y); }
PEXPI v2 ToV2Fromd(v2d vec2d) { return MakeV2((r32)vec2d.X, (r32)vec2d.Y); }
PEXPI v2 ToV2From3(v3 vec3) { return MakeV2(vec3.X, vec3.Y); }
PEXPI v2 ToV2From3XZ(v3 vec3) { return MakeV2(vec3.X, vec3.Z); }
PEXPI v2 ToV2From3YZ(v3 vec3) { return MakeV2(vec3.Y, vec3.Z); }
PEXPI v2i ToV2iFrom3(v3i vec3i) { return MakeV2i(vec3i.X, vec3i.Y); }
PEXPI v2i ToV2iFrom3XZ(v3i vec3i) { return MakeV2i(vec3i.X, vec3i.Z); }
PEXPI v2i ToV2iFrom3YZ(v3i vec3i) { return MakeV2i(vec3i.Y, vec3i.Z); }

PEXPI v3 ToV3Fromi(v3i vec3i) { return MakeV3((r32)vec3i.X, (r32)vec3i.Y, (r32)vec3i.Z); }
PEXPI v3d ToV3dFromi(v3i vec3i) { return MakeV3d((r64)vec3i.X, (r64)vec3i.Y, (r64)vec3i.Z); }
PEXPI v3d ToV3dFromf(v3 vec3) { return MakeV3d((r64)vec3.X, (r64)vec3.Y, (r64)vec3.Z); }
PEXPI v3 ToV3Fromd(v3d vec3d) { return MakeV3((r32)vec3d.X, (r32)vec3d.Y, (r32)vec3d.Z); }
PEXPI v3 ToV3From2(v2 vec2, r32 z) { return MakeV3(vec2.X, vec2.Y, z); }
PEXPI v3 ToV3From2XZ(v2 vec2, r32 y) { return MakeV3(vec2.X, y, vec2.Y); }
PEXPI v3 ToV3From4(v4 vec4) { return MakeV3(vec4.X, vec4.Y, vec4.Z); }
PEXPI v3i ToV3iFrom2(v2i vec2i, i32 z) { return MakeV3i(vec2i.X, vec2i.Y, z); }
PEXPI v3i ToV3iFrom2XZ(v2i vec2i, i32 y) { return MakeV3i(vec2i.X, y, vec2i.Y); }
PEXPI v3i ToV3iFrom4(v4i vec4i) { return MakeV3i(vec4i.X, vec4i.Y, vec4i.Z); }

PEXPI v4 ToV4Fromi(v4i vec4i) { return MakeV4((r32)vec4i.X, (r32)vec4i.Y, (r32)vec4i.Z, (r32)vec4i.W); }
PEXPI v4d ToV4dFromi(v4i vec4i) { return MakeV4d((r64)vec4i.X, (r64)vec4i.Y, (r64)vec4i.Z, (r64)vec4i.W); }
PEXPI v4d ToV4dFromf(v4 vec4) { return MakeV4d((r64)vec4.X, (r64)vec4.Y, (r64)vec4.Z, (r64)vec4.W); }
PEXPI v4 ToV4Fromd(v4d vec4d) { return MakeV4((r32)vec4d.X, (r32)vec4d.Y, (r32)vec4d.Z, (r32)vec4d.W); }
PEXPI v4i ToV4iFrom3(v3i vec3i, i32 w) { return MakeV4i(vec3i.X, vec3i.Y, vec3i.Z, w); }
PEXPI v4r ToV4rFrom4(v4 vec4) { return MakeV4r(vec4.X, vec4.Y, vec4.Z, vec4.W); }
PEXPI v4 ToV4Fromr(v4r vec4r) { return MakeV4(vec4r.X, vec4r.Y, vec4r.Z, vec4r.W); }

#if TARGET_IS_ORCA
PEXPI oc_vec2 ToOcVec2(v2 vector) { return NEW_STRUCT(oc_vec2){ .x = vector.X, .y = vector.Y }; }
PEXPI v2 ToV2FromOc(oc_vec2 orcaVector) { return MakeV2(orcaVector.x, orcaVector.y); }

PEXPI oc_vec2i ToOcVec2i(v2i vector) { return NEW_STRUCT(oc_vec2i){ .x = vector.X, .y = vector.Y }; }
PEXPI v2i ToV2iFromOc(oc_vec2i orcaVector) { return MakeV2i(orcaVector.x, orcaVector.y); }

PEXPI oc_vec3 ToOcVec3(v3 vector) { return NEW_STRUCT(oc_vec3){ .x = vector.X, .y = vector.Y, .z = vector.Z }; }
PEXPI v3 ToV3FromOc(oc_vec3 orcaVector) { return MakeV3(orcaVector.x, orcaVector.y, orcaVector.z); }

PEXPI oc_vec4 ToOcVec4(v4 vector) { return NEW_STRUCT(oc_vec4){ .x = vector.X, .y = vector.Y, .z = vector.Z, .w = vector.W }; }
PEXPI v4 ToV4FromOc(oc_vec4 orcaVector) { return MakeV4(orcaVector.x, orcaVector.y, orcaVector.z, orcaVector.w); }

PEXPI oc_color ToOcColorFromV4r(v4r colorVec) { return NEW_STRUCT(oc_color){ .r = colorVec.X, .g = colorVec.Y, .b = colorVec.Z, .a = colorVec.W }; }
PEXPI v4r ToV4rFromOcColor(oc_color orcaColor) { return MakeV4r(orcaColor.r, orcaColor.g, orcaColor.b, orcaColor.a); }
#endif //TARGET_IS_ORCA

// +--------------------------------------------------------------+
// |                Operator Overload Equivalents                 |
// +--------------------------------------------------------------+
PEXPI v2i AddV2i(v2i left, v2i right) { v2i result; result.X = left.X + right.X; result.Y = left.Y + right.Y; return result; }
PEXPI v3i AddV3i(v3i left, v3i right) { v3i result; result.X = left.X + right.X; result.Y = left.Y + right.Y; result.Z = left.Z + right.Z; return result; }
PEXPI v4i AddV4i(v4i left, v4i right) { v4i result; result.X = left.X + right.X; result.Y = left.Y + right.Y; result.Z = left.Z + right.Z; result.W = left.W + right.W; return result; }
PEXPI v2d AddV2d(v2d left, v2d right) { v2d result; result.X = left.X + right.X; result.Y = left.Y + right.Y; return result; }
PEXPI v3d AddV3d(v3d left, v3d right) { v3d result; result.X = left.X + right.X; result.Y = left.Y + right.Y; result.Z = left.Z + right.Z; return result; }
PEXPI v4d AddV4d(v4d left, v4d right) { v4d result; result.X = left.X + right.X; result.Y = left.Y + right.Y; result.Z = left.Z + right.Z; result.W = left.W + right.W; return result; }

PEXPI v2i SubV2i(v2i left, v2i right) { v2i result; result.X = left.X - right.X; result.Y = left.Y - right.Y; return result; }
PEXPI v3i SubV3i(v3i left, v3i right) { v3i result; result.X = left.X - right.X; result.Y = left.Y - right.Y; result.Z = left.Z - right.Z; return result; }
PEXPI v4i SubV4i(v4i left, v4i right) { v4i result; result.X = left.X - right.X; result.Y = left.Y - right.Y; result.Z = left.Z - right.Z; result.W = left.W - right.W; return result; }
PEXPI v2d SubV2d(v2d left, v2d right) { v2d result; result.X = left.X - right.X; result.Y = left.Y - right.Y; return result; }
PEXPI v3d SubV3d(v3d left, v3d right) { v3d result; result.X = left.X - right.X; result.Y = left.Y - right.Y; result.Z = left.Z - right.Z; return result; }
PEXPI v4d SubV4d(v4d left, v4d right) { v4d result; result.X = left.X - right.X; result.Y = left.Y - right.Y; result.Z = left.Z - right.Z; result.W = left.W - right.W; return result; }

PEXPI v2i MulV2i(v2i left, v2i right) { v2i result; result.X = left.X * right.X; result.Y = left.Y * right.Y; return result; }
PEXPI v3i MulV3i(v3i left, v3i right) { v3i result; result.X = left.X * right.X; result.Y = left.Y * right.Y; result.Z = left.Z * right.Z; return result; }
PEXPI v4i MulV4i(v4i left, v4i right) { v4i result; result.X = left.X * right.X; result.Y = left.Y * right.Y; result.Z = left.Z * right.Z; result.W = left.W * right.W; return result; }
PEXPI v2d MulV2d(v2d left, v2d right) { v2d result; result.X = left.X * right.X; result.Y = left.Y * right.Y; return result; }
PEXPI v3d MulV3d(v3d left, v3d right) { v3d result; result.X = left.X * right.X; result.Y = left.Y * right.Y; result.Z = left.Z * right.Z; return result; }
PEXPI v4d MulV4d(v4d left, v4d right) { v4d result; result.X = left.X * right.X; result.Y = left.Y * right.Y; result.Z = left.Z * right.Z; result.W = left.W * right.W; return result; }

PEXPI v2i DivV2i(v2i left, v2i right) { v2i result; result.X = left.X / right.X; result.Y = left.Y / right.Y; return result; }
PEXPI v3i DivV3i(v3i left, v3i right) { v3i result; result.X = left.X / right.X; result.Y = left.Y / right.Y; result.Z = left.Z / right.Z; return result; }
PEXPI v4i DivV4i(v4i left, v4i right) { v4i result; result.X = left.X / right.X; result.Y = left.Y / right.Y; result.Z = left.Z / right.Z; result.W = left.W / right.W; return result; }
PEXPI v2d DivV2d(v2d left, v2d right) { v2d result; result.X = left.X / right.X; result.Y = left.Y / right.Y; return result; }
PEXPI v3d DivV3d(v3d left, v3d right) { v3d result; result.X = left.X / right.X; result.Y = left.Y / right.Y; result.Z = left.Z / right.Z; return result; }
PEXPI v4d DivV4d(v4d left, v4d right) { v4d result; result.X = left.X / right.X; result.Y = left.Y / right.Y; result.Z = left.Z / right.Z; result.W = left.W / right.W; return result; }

PEXPI v2i ScaleV2i(v2i left, i32 scalar) { v2i result; result.X = left.X * scalar; result.Y = left.Y * scalar; return result; }
PEXPI v3i ScaleV3i(v3i left, i32 scalar) { v3i result; result.X = left.X * scalar; result.Y = left.Y * scalar; result.Z = left.Z * scalar; return result; }
PEXPI v4i ScaleV4i(v4i left, i32 scalar) { v4i result; result.X = left.X * scalar; result.Y = left.Y * scalar; result.Z = left.Z * scalar; result.W = left.W * scalar; return result; }
PEXPI v2d ScaleV2d(v2d left, r64 scalar) { v2d result; result.X = left.X * scalar; result.Y = left.Y * scalar; return result; }
PEXPI v3d ScaleV3d(v3d left, r64 scalar) { v3d result; result.X = left.X * scalar; result.Y = left.Y * scalar; result.Z = left.Z * scalar; return result; }
PEXPI v4d ScaleV4d(v4d left, r64 scalar) { v4d result; result.X = left.X * scalar; result.Y = left.Y * scalar; result.Z = left.Z * scalar; result.W = left.W * scalar; return result; }

PEXPI v2i ShrinkV2i(v2i left, i32 divisor) { v2i result; result.X = left.X / divisor; result.Y = left.Y / divisor; return result; }
PEXPI v3i ShrinkV3i(v3i left, i32 divisor) { v3i result; result.X = left.X / divisor; result.Y = left.Y / divisor; result.Z = left.Z / divisor; return result; }
PEXPI v4i ShrinkV4i(v4i left, i32 divisor) { v4i result; result.X = left.X / divisor; result.Y = left.Y / divisor; result.Z = left.Z / divisor; result.W = left.W / divisor; return result; }
PEXPI v2d ShrinkV2d(v2d left, r64 divisor) { v2d result; result.X = left.X / divisor; result.Y = left.Y / divisor; return result; }
PEXPI v3d ShrinkV3d(v3d left, r64 divisor) { v3d result; result.X = left.X / divisor; result.Y = left.Y / divisor; result.Z = left.Z / divisor; return result; }
PEXPI v4d ShrinkV4d(v4d left, r64 divisor) { v4d result; result.X = left.X / divisor; result.Y = left.Y / divisor; result.Z = left.Z / divisor; result.W = left.W / divisor; return result; }

PEXPI bool AreEqualV4r(v4r left, v4r right) { return (left.X == right.X && left.Y == right.Y && left.Z == right.Z && left.W == right.W); }
PEXPI bool AreEqualV2i(v2i left, v2i right) { return (left.X == right.X && left.Y == right.Y); }
PEXPI bool AreEqualV3i(v3i left, v3i right) { return (left.X == right.X && left.Y == right.Y && left.Z == right.Z); }
PEXPI bool AreEqualV4i(v4i left, v4i right) { return (left.X == right.X && left.Y == right.Y && left.Z == right.Z && left.W == right.W); }
PEXPI bool AreEqualV2d(v2d left, v2d right) { return (left.X == right.X && left.Y == right.Y); }
PEXPI bool AreEqualV3d(v3d left, v3d right) { return (left.X == right.X && left.Y == right.Y && left.Z == right.Z); }
PEXPI bool AreEqualV4d(v4d left, v4d right) { return (left.X == right.X && left.Y == right.Y && left.Z == right.Z && left.W == right.W); }

// +--------------------------------------------------------------+
// |                      Common Vector Math                      |
// +--------------------------------------------------------------+
PEXPI i32 DotV2i(v2i left, v2i right) { return (left.X * right.X) + (left.Y * right.Y); }
PEXPI i32 DotV3i(v3i left, v3i right) { return (left.X * right.X) + (left.Y * right.Y) + (left.Z * right.Z); }
PEXPI i32 DotV4i(v4i left, v4i right) { return (left.X * right.X) + (left.Y * right.Y) + (left.Z * right.Z) + (left.W * right.W); }
PEXPI r64 DotV2d(v2d left, v2d right) { return (left.X * right.X) + (left.Y * right.Y); }
PEXPI r64 DotV3d(v3d left, v3d right) { return (left.X * right.X) + (left.Y * right.Y) + (left.Z * right.Z); }
PEXPI r64 DotV4d(v4d left, v4d right) { return (left.X * right.X) + (left.Y * right.Y) + (left.Z * right.Z) + (left.W * right.W); }

PEXPI v3i CrossV3i(v3i left, v3i right) { return MakeV3i(left.Y*right.Z - left.Z*right.Y, left.Z*right.X - left.X*right.Z, left.X*right.Y - left.Y*right.X); }
PEXPI v3d CrossV3d(v3d left, v3d right) { return MakeV3d(left.Y*right.Z - left.Z*right.Y, left.Z*right.X - left.X*right.Z, left.X*right.Y - left.Y*right.X); }

PEXPI i32 LengthSquaredV2i(v2i vec2i) { return DotV2i(vec2i, vec2i); }
PEXPI i32 LengthSquaredV3i(v3i vec3i) { return DotV3i(vec3i, vec3i); }
PEXPI i32 LengthSquaredV4i(v4i vec4i) { return DotV4i(vec4i, vec4i); }
PEXPI r64 LengthSquaredV2d(v2d vec2d) { return DotV2d(vec2d, vec2d); }
PEXPI r64 LengthSquaredV3d(v3d vec3d) { return DotV3d(vec3d, vec3d); }
PEXPI r64 LengthSquaredV4d(v4d vec4d) { return DotV4d(vec4d, vec4d); }

PEXPI r32 LengthV2i(v2i vec2i) { return SqrtR32((r32)LengthSquaredV2i(vec2i)); }
PEXPI r32 LengthV3i(v3i vec3i) { return SqrtR32((r32)LengthSquaredV3i(vec3i)); }
PEXPI r32 LengthV4i(v4i vec4i) { return SqrtR32((r32)LengthSquaredV4i(vec4i)); }
PEXPI r64 LengthV2d(v2d vec2d) { return SqrtR64(LengthSquaredV2d(vec2d)); }
PEXPI r64 LengthV3d(v3d vec3d) { return SqrtR64(LengthSquaredV3d(vec3d)); }
PEXPI r64 LengthV4d(v4d vec4d) { return SqrtR64(LengthSquaredV4d(vec4d)); }

PEXPI v2d NormalizeV2d(v2d vec2d) { return ShrinkV2d(vec2d, LengthV2d(vec2d)); }
PEXPI v3d NormalizeV3d(v3d vec3d) { return ShrinkV3d(vec3d, LengthV3d(vec3d)); }
PEXPI v4d NormalizeV4d(v4d vec4d) { return ShrinkV4d(vec4d, LengthV4d(vec4d)); }

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
PEXPI v2 FloorV2(v2 vec2) { return MakeV2(FloorR32(vec2.X), FloorR32(vec2.Y)); }
PEXPI v3 FloorV3(v3 vec3) { return MakeV3(FloorR32(vec3.X), FloorR32(vec3.Y), FloorR32(vec3.Z)); }
PEXPI v4 FloorV4(v4 vec4) { return MakeV4(FloorR32(vec4.X), FloorR32(vec4.Y), FloorR32(vec4.Z), FloorR32(vec4.W)); }
PEXPI v2i FloorV2i(v2 vec2) { return MakeV2i(FloorR32i(vec2.X), FloorR32i(vec2.Y)); }
PEXPI v3i FloorV3i(v3 vec3) { return MakeV3i(FloorR32i(vec3.X), FloorR32i(vec3.Y), FloorR32i(vec3.Z)); }
PEXPI v4i FloorV4i(v4 vec4) { return MakeV4i(FloorR32i(vec4.X), FloorR32i(vec4.Y), FloorR32i(vec4.Z), FloorR32i(vec4.W)); }
PEXPI v2d FloorV2d(v2d vec2d) { return MakeV2d(FloorR64(vec2d.X), FloorR64(vec2d.Y)); }
PEXPI v3d FloorV3d(v3d vec3d) { return MakeV3d(FloorR64(vec3d.X), FloorR64(vec3d.Y), FloorR64(vec3d.Z)); }
PEXPI v4d FloorV4d(v4d vec4d) { return MakeV4d(FloorR64(vec4d.X), FloorR64(vec4d.Y), FloorR64(vec4d.Z), FloorR64(vec4d.W)); }
PEXPI v2i FloorV2di(v2d vec2d) { return MakeV2i((i32)FloorR64i(vec2d.X), (i32)FloorR64i(vec2d.Y)); }
PEXPI v3i FloorV3di(v3d vec3d) { return MakeV3i((i32)FloorR64i(vec3d.X), (i32)FloorR64i(vec3d.Y), (i32)FloorR64i(vec3d.Z)); }
PEXPI v4i FloorV4di(v4d vec4d) { return MakeV4i((i32)FloorR64i(vec4d.X), (i32)FloorR64i(vec4d.Y), (i32)FloorR64i(vec4d.Z), (i32)FloorR64i(vec4d.W)); }

PEXPI v2 CeilV2(v2 vec2) { return MakeV2(CeilR32(vec2.X), CeilR32(vec2.Y)); }
PEXPI v3 CeilV3(v3 vec3) { return MakeV3(CeilR32(vec3.X), CeilR32(vec3.Y), CeilR32(vec3.Z)); }
PEXPI v4 CeilV4(v4 vec4) { return MakeV4(CeilR32(vec4.X), CeilR32(vec4.Y), CeilR32(vec4.Z), CeilR32(vec4.W)); }
PEXPI v2i CeilV2i(v2 vec2) { return MakeV2i(CeilR32i(vec2.X), CeilR32i(vec2.Y)); }
PEXPI v3i CeilV3i(v3 vec3) { return MakeV3i(CeilR32i(vec3.X), CeilR32i(vec3.Y), CeilR32i(vec3.Z)); }
PEXPI v4i CeilV4i(v4 vec4) { return MakeV4i(CeilR32i(vec4.X), CeilR32i(vec4.Y), CeilR32i(vec4.Z), CeilR32i(vec4.W)); }
PEXPI v2d CeilV2d(v2d vec2d) { return MakeV2d(CeilR64(vec2d.X), CeilR64(vec2d.Y)); }
PEXPI v3d CeilV3d(v3d vec3d) { return MakeV3d(CeilR64(vec3d.X), CeilR64(vec3d.Y), CeilR64(vec3d.Z)); }
PEXPI v4d CeilV4d(v4d vec4d) { return MakeV4d(CeilR64(vec4d.X), CeilR64(vec4d.Y), CeilR64(vec4d.Z), CeilR64(vec4d.W)); }
PEXPI v2i CeilV2di(v2d vec2d) { return MakeV2i((i32)CeilR64i(vec2d.X), (i32)CeilR64i(vec2d.Y)); }
PEXPI v3i CeilV3di(v3d vec3d) { return MakeV3i((i32)CeilR64i(vec3d.X), (i32)CeilR64i(vec3d.Y), (i32)CeilR64i(vec3d.Z)); }
PEXPI v4i CeilV4di(v4d vec4d) { return MakeV4i((i32)CeilR64i(vec4d.X), (i32)CeilR64i(vec4d.Y), (i32)CeilR64i(vec4d.Z), (i32)CeilR64i(vec4d.W)); }

PEXPI v2 RoundV2(v2 vec2) { return MakeV2(RoundR32(vec2.X), RoundR32(vec2.Y)); }
PEXPI v3 RoundV3(v3 vec3) { return MakeV3(RoundR32(vec3.X), RoundR32(vec3.Y), RoundR32(vec3.Z)); }
PEXPI v4 RoundV4(v4 vec4) { return MakeV4(RoundR32(vec4.X), RoundR32(vec4.Y), RoundR32(vec4.Z), RoundR32(vec4.W)); }
PEXPI v2i RoundV2i(v2 vec2) { return MakeV2i(RoundR32i(vec2.X), RoundR32i(vec2.Y)); }
PEXPI v3i RoundV3i(v3 vec3) { return MakeV3i(RoundR32i(vec3.X), RoundR32i(vec3.Y), RoundR32i(vec3.Z)); }
PEXPI v4i RoundV4i(v4 vec4) { return MakeV4i(RoundR32i(vec4.X), RoundR32i(vec4.Y), RoundR32i(vec4.Z), RoundR32i(vec4.W)); }
PEXPI v2d RoundV2d(v2d vec2d) { return MakeV2d(RoundR64(vec2d.X), RoundR64(vec2d.Y)); }
PEXPI v3d RoundV3d(v3d vec3d) { return MakeV3d(RoundR64(vec3d.X), RoundR64(vec3d.Y), RoundR64(vec3d.Z)); }
PEXPI v4d RoundV4d(v4d vec4d) { return MakeV4d(RoundR64(vec4d.X), RoundR64(vec4d.Y), RoundR64(vec4d.Z), RoundR64(vec4d.W)); }
PEXPI v2i RoundV2di(v2d vec2d) { return MakeV2i((i32)RoundR64i(vec2d.X), (i32)RoundR64i(vec2d.Y)); }
PEXPI v3i RoundV3di(v3d vec3d) { return MakeV3i((i32)RoundR64i(vec3d.X), (i32)RoundR64i(vec3d.Y), (i32)RoundR64i(vec3d.Z)); }
PEXPI v4i RoundV4di(v4d vec4d) { return MakeV4i((i32)RoundR64i(vec4d.X), (i32)RoundR64i(vec4d.Y), (i32)RoundR64i(vec4d.Z), (i32)RoundR64i(vec4d.W)); }

PEXPI v2 MinV2(v2 left, v2 right) { return MakeV2(MinR32(left.X, right.X), MinR32(left.Y, right.Y)); }
PEXPI v3 MinV3(v3 left, v3 right) { return MakeV3(MinR32(left.X, right.X), MinR32(left.Y, right.Y), MinR32(left.Z, right.Z)); }
PEXPI v4 MinV4(v4 left, v4 right) { return MakeV4(MinR32(left.X, right.X), MinR32(left.Y, right.Y), MinR32(left.Z, right.Z), MinR32(left.W, right.W)); }
PEXPI v2d MinV2d(v2d left, v2d right) { return MakeV2d(MinR64(left.X, right.X), MinR64(left.Y, right.Y)); }
PEXPI v3d MinV3d(v3d left, v3d right) { return MakeV3d(MinR64(left.X, right.X), MinR64(left.Y, right.Y), MinR64(left.Z, right.Z)); }
PEXPI v4d MinV4d(v4d left, v4d right) { return MakeV4d(MinR64(left.X, right.X), MinR64(left.Y, right.Y), MinR64(left.Z, right.Z), MinR64(left.W, right.W)); }

PEXPI v2 MaxV2(v2 left, v2 right) { return MakeV2(MaxR32(left.X, right.X), MaxR32(left.Y, right.Y)); }
PEXPI v3 MaxV3(v3 left, v3 right) { return MakeV3(MaxR32(left.X, right.X), MaxR32(left.Y, right.Y), MaxR32(left.Z, right.Z)); }
PEXPI v4 MaxV4(v4 left, v4 right) { return MakeV4(MaxR32(left.X, right.X), MaxR32(left.Y, right.Y), MaxR32(left.Z, right.Z), MaxR32(left.W, right.W)); }
PEXPI v2d MaxV2d(v2d left, v2d right) { return MakeV2d(MaxR64(left.X, right.X), MaxR64(left.Y, right.Y)); }
PEXPI v3d MaxV3d(v3d left, v3d right) { return MakeV3d(MaxR64(left.X, right.X), MaxR64(left.Y, right.Y), MaxR64(left.Z, right.Z)); }
PEXPI v4d MaxV4d(v4d left, v4d right) { return MakeV4d(MaxR64(left.X, right.X), MaxR64(left.Y, right.Y), MaxR64(left.Z, right.Z), MaxR64(left.W, right.W)); }

PEXPI v2 AbsV2(v2 vec2) { return MakeV2(AbsR32(vec2.X), AbsR32(vec2.Y)); }
PEXPI v3 AbsV3(v3 vec3) { return MakeV3(AbsR32(vec3.X), AbsR32(vec3.Y), AbsR32(vec3.Z)); }
PEXPI v4 AbsV4(v4 vec4) { return MakeV4(AbsR32(vec4.X), AbsR32(vec4.Y), AbsR32(vec4.Z), AbsR32(vec4.W)); }
PEXPI v2i AbsV2i(v2i vec2i) { return MakeV2i(AbsI32(vec2i.X), AbsI32(vec2i.Y)); }
PEXPI v3i AbsV3i(v3i vec3i) { return MakeV3i(AbsI32(vec3i.X), AbsI32(vec3i.Y), AbsI32(vec3i.Z)); }
PEXPI v4i AbsV4i(v4i vec4i) { return MakeV4i(AbsI32(vec4i.X), AbsI32(vec4i.Y), AbsI32(vec4i.Z), AbsI32(vec4i.W)); }
PEXPI v2d AbsV2d(v2d vec2d) { return MakeV2d(AbsR64(vec2d.X), AbsR64(vec2d.Y)); }
PEXPI v3d AbsV3d(v3d vec3d) { return MakeV3d(AbsR64(vec3d.X), AbsR64(vec3d.Y), AbsR64(vec3d.Z)); }
PEXPI v4d AbsV4d(v4d vec4d) { return MakeV4d(AbsR64(vec4d.X), AbsR64(vec4d.Y), AbsR64(vec4d.Z), AbsR64(vec4d.W)); }

PEXPI v2 ClampV2(v2 vec2, v2 minVec2, v2 maxVec2) { return MakeV2(ClampR32(vec2.X, minVec2.X, maxVec2.X), ClampR32(vec2.Y, minVec2.Y, maxVec2.Y)); }
PEXPI v3 ClampV3(v3 vec3, v3 minVec3, v3 maxVec3) { return MakeV3(ClampR32(vec3.X, minVec3.X, maxVec3.X), ClampR32(vec3.Y, minVec3.Y, maxVec3.Y), ClampR32(vec3.Z, minVec3.Z, maxVec3.Z)); }
PEXPI v4 ClampV4(v4 vec4, v4 minVec4, v4 maxVec4) { return MakeV4(ClampR32(vec4.X, minVec4.X, maxVec4.X), ClampR32(vec4.Y, minVec4.Y, maxVec4.Y), ClampR32(vec4.Z, minVec4.Z, maxVec4.Z), ClampR32(vec4.W, minVec4.W, maxVec4.W)); }
PEXPI v2i ClampV2i(v2i vec2i, v2i minVec2i, v2i maxVec2i) { return MakeV2i(ClampI32(vec2i.X, minVec2i.X, maxVec2i.X), ClampI32(vec2i.Y, minVec2i.Y, maxVec2i.Y)); }
PEXPI v3i ClampV3i(v3i vec3i, v3i minVec3i, v3i maxVec3i) { return MakeV3i(ClampI32(vec3i.X, minVec3i.X, maxVec3i.X), ClampI32(vec3i.Y, minVec3i.Y, maxVec3i.Y), ClampI32(vec3i.Z, minVec3i.Z, maxVec3i.Z)); }
PEXPI v4i ClampV4i(v4i vec4i, v4i minVec4i, v4i maxVec4i) { return MakeV4i(ClampI32(vec4i.X, minVec4i.X, maxVec4i.X), ClampI32(vec4i.Y, minVec4i.Y, maxVec4i.Y), ClampI32(vec4i.Z, minVec4i.Z, maxVec4i.Z), ClampI32(vec4i.W, minVec4i.W, maxVec4i.W)); }
PEXPI v2d ClampV2d(v2d vec2d, v2d minVec2d, v2d maxVec2d) { return MakeV2d(ClampR64(vec2d.X, minVec2d.X, maxVec2d.X), ClampR64(vec2d.Y, minVec2d.Y, maxVec2d.Y)); }
PEXPI v3d ClampV3d(v3d vec3d, v3d minVec3d, v3d maxVec3d) { return MakeV3d(ClampR64(vec3d.X, minVec3d.X, maxVec3d.X), ClampR64(vec3d.Y, minVec3d.Y, maxVec3d.Y), ClampR64(vec3d.Z, minVec3d.Z, maxVec3d.Z)); }
PEXPI v4d ClampV4d(v4d vec4d, v4d minVec4d, v4d maxVec4d) { return MakeV4d(ClampR64(vec4d.X, minVec4d.X, maxVec4d.X), ClampR64(vec4d.Y, minVec4d.Y, maxVec4d.Y), ClampR64(vec4d.Z, minVec4d.Z, maxVec4d.Z), ClampR64(vec4d.W, minVec4d.W, maxVec4d.W)); }

//TODO: Should we add "Inner" and "Determinant" functions like we had in GyLib?

// +--------------------------------------------------------------+
// |                   Other Helpful Functions                    |
// +--------------------------------------------------------------+
PEXPI v2 PerpRightV2(v2 vec2) { return MakeV2(-vec2.Y, vec2.X); }
PEXPI v2 PerpLeftV2(v2 vec2) { return MakeV2(vec2.Y, -vec2.X); }
PEXPI v2d PerpRightV2d(v2d vec2d) { return MakeV2d(-vec2d.Y, vec2d.X); }
PEXPI v2d PerpLeftV2d(v2d vec2d) { return MakeV2d(vec2d.Y, -vec2d.X); }

PEXPI v2 Vec2Rotate(v2 vec2, r32 angle)
{
	return MakeV2(
		CosR32(angle) * vec2.X - SinR32(angle) * vec2.Y,
		SinR32(angle) * vec2.X + CosR32(angle) * vec2.Y
	);
}
PEXPI v2d Vec2dRotate(v2d vec2d, r64 angle)
{
	return MakeV2d(
		CosR64(angle) * vec2d.X - SinR64(angle) * vec2d.Y,
		SinR64(angle) * vec2d.X + CosR64(angle) * vec2d.Y
	);
}

PEXPI r32 AngleBetweenV2(v2 left, v2 right)
{
	r32 dotProduct = DotV2(left, right);
	if (dotProduct == -1) { return Pi32; }
	return SignOfR32(left.X * right.Y - left.Y * right.X) * AcosR32(dotProduct / (LengthV2(left) * LengthV2(right)));
}
PEXPI r64 AngleBetweenV2d(v2d left, v2d right)
{
	r64 dotProduct = DotV2d(left, right);
	if (dotProduct == -1) { return Pi64; }
	return SignOfR64(left.X * right.Y - left.Y * right.X) * AcosR64(dotProduct / (LengthV2d(left) * LengthV2d(right)));
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

// +--------------------------------------------------------------+
// |             Quickhand Functions for Layout Code              |
// +--------------------------------------------------------------+
PEXPI void AlignV2ToV2(v2* vectorOut, v2 alignmentScale)
{
	DebugNotNull(vectorOut);
	if (alignmentScale.X > 0) { vectorOut->X = RoundR32(vectorOut->X * alignmentScale.X) / alignmentScale.X; }
	if (alignmentScale.Y > 0) { vectorOut->Y = RoundR32(vectorOut->Y * alignmentScale.Y) / alignmentScale.Y; }
}
PEXPI void AlignV2To(v2* vectorOut, r32 alignmentScale) { AlignV2ToV2(vectorOut, FillV2(alignmentScale)); }
PEXPI void AlignV2(v2* vectorOut) { AlignV2ToV2(vectorOut, V2_One); }
PEXPI void AlignV2dToV2d(v2d* vectorOut, v2d alignmentScale)
{
	DebugNotNull(vectorOut);
	if (alignmentScale.X > 0) { vectorOut->X = RoundR64(vectorOut->X * alignmentScale.X) / alignmentScale.X; }
	if (alignmentScale.Y > 0) { vectorOut->Y = RoundR64(vectorOut->Y * alignmentScale.Y) / alignmentScale.Y; }
}
PEXPI void AlignV2dTo(v2d* vectorOut, r64 alignmentScale) { AlignV2dToV2d(vectorOut, FillV2d(alignmentScale)); }
PEXPI void AlignV2d(v2d* vectorOut) { AlignV2dToV2d(vectorOut, V2d_One); }

PEXPI void AlignV3ToV3(v3* vectorOut, v3 alignmentScale)
{
	DebugNotNull(vectorOut);
	if (alignmentScale.X > 0) { vectorOut->X = RoundR32(vectorOut->X * alignmentScale.X) / alignmentScale.X; }
	if (alignmentScale.Y > 0) { vectorOut->Y = RoundR32(vectorOut->Y * alignmentScale.Y) / alignmentScale.Y; }
	if (alignmentScale.Z > 0) { vectorOut->Z = RoundR32(vectorOut->Z * alignmentScale.Z) / alignmentScale.Z; }
}
PEXPI void AlignV3To(v3* vectorOut, r32 alignmentScale) { AlignV3ToV3(vectorOut, FillV3(alignmentScale)); }
PEXPI void AlignV3(v3* vectorOut) { AlignV3ToV3(vectorOut, V3_One); }
PEXPI void AlignV3dToV3d(v3d* vectorOut, v3d alignmentScale)
{
	DebugNotNull(vectorOut);
	if (alignmentScale.X > 0) { vectorOut->X = RoundR64(vectorOut->X * alignmentScale.X) / alignmentScale.X; }
	if (alignmentScale.Y > 0) { vectorOut->Y = RoundR64(vectorOut->Y * alignmentScale.Y) / alignmentScale.Y; }
	if (alignmentScale.Z > 0) { vectorOut->Z = RoundR64(vectorOut->Z * alignmentScale.Z) / alignmentScale.Z; }
}
PEXPI void AlignV3dTo(v3d* vectorOut, r64 alignmentScale) { AlignV3dToV3d(vectorOut, FillV3d(alignmentScale)); }
PEXPI void AlignV3d(v3d* vectorOut) { AlignV3dToV3d(vectorOut, V3d_One); }

PEXPI void AlignV4ToV4(v4* vectorOut, v4 alignmentScale)
{
	DebugNotNull(vectorOut);
	if (alignmentScale.X > 0) { vectorOut->X = RoundR32(vectorOut->X * alignmentScale.X) / alignmentScale.X; }
	if (alignmentScale.Y > 0) { vectorOut->Y = RoundR32(vectorOut->Y * alignmentScale.Y) / alignmentScale.Y; }
	if (alignmentScale.Z > 0) { vectorOut->Z = RoundR32(vectorOut->Z * alignmentScale.Z) / alignmentScale.Z; }
	if (alignmentScale.W > 0) { vectorOut->W = RoundR32(vectorOut->W * alignmentScale.W) / alignmentScale.W; }
}
PEXPI void AlignV4To(v4* vectorOut, r32 alignmentScale) { AlignV4ToV4(vectorOut, FillV4(alignmentScale)); }
PEXPI void AlignV4(v4* vectorOut) { AlignV4ToV4(vectorOut, V4_One); }
PEXPI void AlignV4dToV4d(v4d* vectorOut, v4d alignmentScale)
{
	DebugNotNull(vectorOut);
	if (alignmentScale.X > 0) { vectorOut->X = RoundR64(vectorOut->X * alignmentScale.X) / alignmentScale.X; }
	if (alignmentScale.Y > 0) { vectorOut->Y = RoundR64(vectorOut->Y * alignmentScale.Y) / alignmentScale.Y; }
	if (alignmentScale.Z > 0) { vectorOut->Z = RoundR64(vectorOut->Z * alignmentScale.Z) / alignmentScale.Z; }
	if (alignmentScale.W > 0) { vectorOut->W = RoundR64(vectorOut->W * alignmentScale.W) / alignmentScale.W; }
}
PEXPI void AlignV4dTo(v4d* vectorOut, r64 alignmentScale) { AlignV4dToV4d(vectorOut, FillV4d(alignmentScale)); }
PEXPI void AlignV4d(v4d* vectorOut) { AlignV4dToV4d(vectorOut, V4d_One); }

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

#if defined(_INPUT_BTN_STATE_H) && defined(_INPUT_MOUSE_BTNS_H) && defined(_STRUCT_VECTORS_H)
#include "cross/cross_mouse_btns_btn_state_and_vectors.h"
#endif

#if defined(_STRUCT_VECTORS_H) && defined(RAYLIB_H)
#include "cross/cross_vectors_and_raylib.h"
#endif
