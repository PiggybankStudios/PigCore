/*
File:   struct_directions.h
Author: Taylor Robbins
Date:   01\05\2025
Description:
	** Contains a few enumerations that map to 2D or 3D directions in space (like up\down\left\right)
	** TODO: Write some more documentation, maybe even draw some pictures, about coordinate systems
	** esp. wrt our own choices for Left-Handed and Y-Up, and the benefits\tradeoffs related to that.
*/

//TODO: Once vectors are added to the repository, remove #if 0's and fixup code

#ifndef _STRUCT_DIRECTIONS_H
#define _STRUCT_DIRECTIONS_H

#include "base/base_typedefs.h"
#include "std/std_basic_math.h"

// +==============================+
// |        2D Directions         |
// +==============================+
// NOTE: Unlike math, most computer graphics systems with with a +y going down
//       the screen and (0, 0) being the top-left of the space. So Down is
//       actually +y in this coordinate system.
//
//       Top/Up
//     +-------+ --> +x
//     |       |
//Left |       | Right
//     |       |
//     +-------+
//     | Bottom/Down
//     v +y

// +==============================+
// |        3D Directions         |
// +==============================+
// NOTE: Our coordinate system in Pig Engine (and everything else I work onin 3D)
//       is implicitly Left-Handed and Y-Up. The ASCII diagram below provides a
//       small visual for what that means, and labels the names we give to each direction/side
//
//                  ^ +z
//         ^ +y    / Front
//         |      / Forward
//         |  +--------+
//         | /  Top   /|
//         |/   Up   / |
//         +--------+  +
//    Left | Back   | / Right
//         |Backward|/
//         +--------+ ---> +x 
//           Bottom
//            Down

typedef enum Dir2 Dir2;
enum Dir2
{
	Dir2_None  = 0x00,
	
	Dir2_Right = 0x01, //+x ( 1,  0)
	Dir2_Down  = 0x02, //+y ( 0,  1)
	Dir2_Left  = 0x04, //-x (-1,  0)
	Dir2_Up    = 0x08, //-y ( 0, -1)
	
	Dir2_All   = 0x0F,
	Dir2_Count = 4,
};

typedef enum Dir2Ex Dir2Ex;
enum Dir2Ex
{
	Dir2Ex_None        = 0x00,
	
	Dir2Ex_Right       = 0x01, //+x ( 1,  0)
	Dir2Ex_Down        = 0x02, //+y ( 0,  1)
	Dir2Ex_Left        = 0x04, //-x (-1,  0)
	Dir2Ex_Up          = 0x08, //-y ( 0, -1)
	Dir2Ex_TopLeft     = 0x10, //-x -y (0, 0)
	Dir2Ex_TopRight    = 0x20, //+x -y (1, 0)
	Dir2Ex_BottomRight = 0x40, //+x +y (1, 1)
	Dir2Ex_BottomLeft  = 0x80, //-x +y (0, 1)
	
	Dir2Ex_Cardinal    = 0x0F,
	Dir2Ex_Diagonal    = 0xF0,
	Dir2Ex_All         = 0xFF,
	Dir2Ex_Count = 8,
};

typedef enum Dir3 Dir3;
enum Dir3
{
	Dir3_None        = 0x00,
	
	Dir3_Right       = 0x01, //+x ( 1,  0,  0)
	Dir3_Left        = 0x02, //-x (-1,  0,  0)
	Dir3_Up          = 0x04, //+y ( 0,  1,  0)
	Dir3_Down        = 0x08, //-y ( 0, -1,  0)
	Dir3_Forward     = 0x10, //+z ( 0,  0,  1)
	Dir3_Backward    = 0x20, //-z ( 0,  0, -1)
	
	Dir3_NotRight    = 0x3E,
	Dir3_NotLeft     = 0x3D,
	Dir3_NotUp       = 0x3B,
	Dir3_NotDown     = 0x37,
	Dir3_NotForward  = 0x2F,
	Dir3_NotBackward = 0x1F,
	
	Dir3_All         = 0x3F,
	Dir3_Count = 6,
};

/*
TODO: If Dir3Ex is actually useful for a particular use-case, then maybe we should implement it fully
      But for now we'll leave it commented out as a reference for how we would do a 3D analog to Dir2Ex
enum Dir3Ex_t
{
	Dir3Ex_None             = 0x00000000,
	
	Dir3Ex_Right            = 0x00000001, //+x ( 1,  0,  0)
	Dir3Ex_Left             = 0x00000002, //-x (-1,  0,  0)
	Dir3Ex_Up               = 0x00000004, //+y ( 0,  1,  0)
	Dir3Ex_Down             = 0x00000008, //-y ( 0, -1,  0)
	Dir3Ex_Forward          = 0x00000010, //+z ( 0,  0,  1)
	Dir3Ex_Backward         = 0x00000020, //-z ( 0,  0, -1)
	
	Dir3Ex_TopRight         = 0x00000040, //+y +x ( 1,  1,  0)
	Dir3Ex_TopBack          = 0x00000080, //+y -z ( 0,  1, -1)
	Dir3Ex_TopLeft          = 0x00000100, //+y -x (-1,  1,  0)
	Dir3Ex_TopFront         = 0x00000200, //+y +z ( 0,  1,  1)
	Dir3Ex_RightBack        = 0x00000400, //+x -z ( 1,  0, -1)
	Dir3Ex_LeftBack         = 0x00000800, //-x -z (-1,  0, -1)
	Dir3Ex_LeftFront        = 0x00001000, //-x +z (-1,  0,  1)
	Dir3Ex_RightFront       = 0x00002000, //+x +z ( 1,  0,  1)
	Dir3Ex_BottomRight      = 0x00004000, //-y +x ( 1, -1,  0)
	Dir3Ex_BottomBack       = 0x00008000, //-y -z ( 0, -1, -1)
	Dir3Ex_BottomLeft       = 0x00010000, //-y -x (-1, -1,  0)
	Dir3Ex_BottomFront      = 0x00020000, //-y +z ( 0, -1,  1)
	
	Dir3Ex_BottomLeftBack   = 0x00040000, //-x -y -z (0, 0, 0)
	Dir3Ex_BottomRightBack  = 0x00080000, //+x -y -z (1, 0, 0)
	Dir3Ex_BottomRightFront = 0x00100000, //+x +y -z (1, 1, 0)
	Dir3Ex_BottomLeftFront  = 0x00200000, //-x +y -z (0, 1, 0)
	Dir3Ex_TopLeftBack      = 0x00400000, //-x -y +z (0, 0, 1)
	Dir3Ex_TopRightBack     = 0x00800000, //+x -y +z (1, 0, 1)
	Dir3Ex_TopRightFront    = 0x01000000, //+x +y +z (1, 1, 1)
	Dir3Ex_TopLeftFront     = 0x02000000, //-x +y +z (0, 1, 1)
	
	Dir3Ex_HorizontalX      = (Dir3Ex_Right   | Dir3Ex_Left    ),
	Dir3Ex_HorizontalZ      = (Dir3Ex_Forward | Dir3Ex_Backward),
	Dir3Ex_Vertical         = (Dir3Ex_Up      | Dir3Ex_Down    ),
	
	Dir3Ex_Cardinal         = 0x0000003F,
	Dir3Ex_Edge             = 0x0003FFC0,
	Dir3Ex_Corner           = 0x03FC0000,
	Dir3Ex_All              = 0x00003FFF,
	Dir3Ex_Count = 26,
};
*/

typedef enum Axis Axis;
enum Axis
{
	Axis_None  = 0x00,
	Axis_X     = 0x01,
	Axis_Y     = 0x02,
	Axis_Z     = 0x04,
	Axis_W     = 0x08,
	Axis_All2  = 0x03,
	Axis_All3  = 0x07,
	Axis_All4  = 0x0F,
	Axis_Count = 4,
};

// +--------------------------------------------------------------+
// |               Casting and Conversion Functions               |
// +--------------------------------------------------------------+
#if 0
v2 ToVec2(Dir2 dir2)
{
	v2 result = {};
	if (IsFlagSet(dir2, Dir2_Right)) { result.x += 1.0f; }
	if (IsFlagSet(dir2, Dir2_Left))  { result.x -= 1.0f; }
	if (IsFlagSet(dir2, Dir2_Down))  { result.y += 1.0f; }
	if (IsFlagSet(dir2, Dir2_Up))    { result.y -= 1.0f; }
	return result;
}
v2 ToVec2(Dir2Ex dir2ex)
{
	v2 result = {};
	if (IsFlagSet(dir2ex, Dir2Ex_Right)) { result.x += 1.0f; }
	if (IsFlagSet(dir2ex, Dir2Ex_Left))  { result.x -= 1.0f; }
	if (IsFlagSet(dir2ex, Dir2Ex_Down))  { result.y += 1.0f; }
	if (IsFlagSet(dir2ex, Dir2Ex_Up))    { result.y -= 1.0f; }
	if (IsFlagSet(dir2ex, Dir2Ex_TopLeft))     { result.x -= 1.0f; result.y -= 1.0f; }
	if (IsFlagSet(dir2ex, Dir2Ex_TopRight))    { result.x += 1.0f; result.y -= 1.0f; }
	if (IsFlagSet(dir2ex, Dir2Ex_BottomRight)) { result.x += 1.0f; result.y += 1.0f; }
	if (IsFlagSet(dir2ex, Dir2Ex_BottomLeft))  { result.x -= 1.0f; result.y += 1.0f; }
	return result;
}
v2i ToVec2i(Dir2 dir2)
{
	v2i result = {};
	if (IsFlagSet(dir2, Dir2_Right)) { result.x += 1; }
	if (IsFlagSet(dir2, Dir2_Left))  { result.x -= 1; }
	if (IsFlagSet(dir2, Dir2_Down))  { result.y += 1; }
	if (IsFlagSet(dir2, Dir2_Up))    { result.y -= 1; }
	return result;
}
v2i ToVec2i(Dir2Ex dir2ex)
{
	v2i result = {};
	if (IsFlagSet(dir2ex, Dir2Ex_Right)) { result.x += 1; }
	if (IsFlagSet(dir2ex, Dir2Ex_Left))  { result.x -= 1; }
	if (IsFlagSet(dir2ex, Dir2Ex_Down))  { result.y += 1; }
	if (IsFlagSet(dir2ex, Dir2Ex_Up))    { result.y -= 1; }
	if (IsFlagSet(dir2ex, Dir2Ex_TopLeft))     { result.x -= 1; result.y -= 1; }
	if (IsFlagSet(dir2ex, Dir2Ex_TopRight))    { result.x += 1; result.y -= 1; }
	if (IsFlagSet(dir2ex, Dir2Ex_BottomRight)) { result.x += 1; result.y += 1; }
	if (IsFlagSet(dir2ex, Dir2Ex_BottomLeft))  { result.x -= 1; result.y += 1; }
	return result;
}

v3 ToVec3(Dir3 dir3)
{
	v3 result = {};
	if (IsFlagSet(dir3, Dir3_Right))    { result.x += 1.0f; }
	if (IsFlagSet(dir3, Dir3_Left))     { result.x -= 1.0f; }
	if (IsFlagSet(dir3, Dir3_Down))     { result.y -= 1.0f; }
	if (IsFlagSet(dir3, Dir3_Up))       { result.y += 1.0f; }
	if (IsFlagSet(dir3, Dir3_Forward))  { result.z += 1.0f; }
	if (IsFlagSet(dir3, Dir3_Backward)) { result.z -= 1.0f; }
	return result;
}
v3i ToVec3i(Dir3 dir3)
{
	v3i result = {};
	if (IsFlagSet(dir3, Dir3_Right))    { result.x += 1; }
	if (IsFlagSet(dir3, Dir3_Left))     { result.x -= 1; }
	if (IsFlagSet(dir3, Dir3_Down))     { result.y -= 1; }
	if (IsFlagSet(dir3, Dir3_Up))       { result.y += 1; }
	if (IsFlagSet(dir3, Dir3_Forward))  { result.z += 1; }
	if (IsFlagSet(dir3, Dir3_Backward)) { result.z -= 1; }
	return result;
}
v3 ToVec3(Dir3Ex_t dir3ex)
{
	v3 result = {};
	if (IsFlagSet(dir3ex, Dir3Ex_Right))            { result.x += 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_Left))             { result.x -= 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_Down))             { result.y -= 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_Up))               { result.y += 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_Forward))          { result.z += 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_Backward))         { result.z -= 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_TopRight))         { result.y += 1.0f; result.x += 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_TopBack))          { result.y += 1.0f; result.z -= 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_TopLeft))          { result.y += 1.0f; result.x -= 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_TopFront))         { result.y += 1.0f; result.z += 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_RightBack))        { result.x += 1.0f; result.z -= 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_LeftBack))         { result.x -= 1.0f; result.z -= 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_LeftFront))        { result.x -= 1.0f; result.z += 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_RightFront))       { result.x += 1.0f; result.z += 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_BottomRight))      { result.y -= 1.0f; result.x += 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_BottomBack))       { result.y -= 1.0f; result.z -= 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_BottomLeft))       { result.y -= 1.0f; result.x -= 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_BottomFront))      { result.y -= 1.0f; result.z += 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_BottomLeftBack))   { result.x -= 1.0f; result.y -= 1.0f; result.z -= 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_BottomRightBack))  { result.x += 1.0f; result.y -= 1.0f; result.z -= 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_BottomRightFront)) { result.x += 1.0f; result.y -= 1.0f; result.z += 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_BottomLeftFront))  { result.x -= 1.0f; result.y -= 1.0f; result.z += 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_TopLeftBack))      { result.x -= 1.0f; result.y += 1.0f; result.z -= 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_TopRightBack))     { result.x += 1.0f; result.y += 1.0f; result.z -= 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_TopRightFront))    { result.x += 1.0f; result.y += 1.0f; result.z += 1.0f; }
	if (IsFlagSet(dir3ex, Dir3Ex_TopLeftFront))     { result.x -= 1.0f; result.y += 1.0f; result.z += 1.0f; }
	return result;
}
v3i ToVec3i(Dir3Ex_t dir3ex)
{
	v3i result = {};
	if (IsFlagSet(dir3ex, Dir3Ex_Right))            { result.x += 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_Left))             { result.x -= 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_Down))             { result.y -= 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_Up))               { result.y += 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_Forward))          { result.z += 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_Backward))         { result.z -= 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_TopRight))         { result.y += 1; result.x += 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_TopBack))          { result.y += 1; result.z -= 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_TopLeft))          { result.y += 1; result.x -= 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_TopFront))         { result.y += 1; result.z += 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_RightBack))        { result.x += 1; result.z -= 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_LeftBack))         { result.x -= 1; result.z -= 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_LeftFront))        { result.x -= 1; result.z += 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_RightFront))       { result.x += 1; result.z += 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_BottomRight))      { result.y -= 1; result.x += 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_BottomBack))       { result.y -= 1; result.z -= 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_BottomLeft))       { result.y -= 1; result.x -= 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_BottomFront))      { result.y -= 1; result.z += 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_BottomLeftBack))   { result.x -= 1; result.y -= 1; result.z -= 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_BottomRightBack))  { result.x += 1; result.y -= 1; result.z -= 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_BottomRightFront)) { result.x += 1; result.y -= 1; result.z += 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_BottomLeftFront))  { result.x -= 1; result.y -= 1; result.z += 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_TopLeftBack))      { result.x -= 1; result.y += 1; result.z -= 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_TopRightBack))     { result.x += 1; result.y += 1; result.z -= 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_TopRightFront))    { result.x += 1; result.y += 1; result.z += 1; }
	if (IsFlagSet(dir3ex, Dir3Ex_TopLeftFront))     { result.x -= 1; result.y += 1; result.z += 1; }
	return result;
}
v3 ToVec3(Axis axis)
{
	v3 result = {};
	if (IsFlagSet(axis, Axis_X)) { result.x += 1.0f; }
	if (IsFlagSet(axis, Axis_Y)) { result.y += 1.0f; }
	if (IsFlagSet(axis, Axis_Z)) { result.z += 1.0f; }
	return result;
}
v3i ToVec3i(Axis axis)
{
	v3i result = {};
	if (IsFlagSet(axis, Axis_X)) { result.x += 1; }
	if (IsFlagSet(axis, Axis_Y)) { result.y += 1; }
	if (IsFlagSet(axis, Axis_Z)) { result.z += 1; }
	return result;
}

Dir2 ToDir2(v2 vector)
{
	if (vector == Vec2_Zero) { return Dir2_None; }
	if (AbsR32(vector.x) >= AbsR32(vector.y))
	{
		if (vector.x < 0) { return Dir2_Left; }
		else { return Dir2_Right; }
	}
	else
	{
		if (vector.y < 0) { return Dir2_Up; }
		else { return Dir2_Down; }
	}
}
Dir2 ToDir2(v2i vector)
{
	if (vector == Vec2i_Zero) { return Dir2_None; }
	if (AbsI32(vector.x) >= AbsI32(vector.y))
	{
		if (vector.x < 0) { return Dir2_Left; }
		else { return Dir2_Right; }
	}
	else
	{
		if (vector.y < 0) { return Dir2_Up; }
		else { return Dir2_Down; }
	}
}
#endif
Dir2 ToDir2(Axis axis, bool positive)
{
	switch (axis)
	{
		case Axis_X: return positive ? Dir2_Right : Dir2_Left;
		case Axis_Y: return positive ? Dir2_Down  : Dir2_Up;
		default: return Dir2_None;
	}
}

#if 0
Dir3 ToDir3(v3 vector)
{
	if (vector == Vec3_Zero) { return Dir3_None; }
	r32 absX = AbsR32(vector.x);
	r32 absY = AbsR32(vector.y);
	r32 absZ = AbsR32(vector.z);
	if (absX >= absY && absX >= absZ)
	{
		if (vector.x < 0) { return Dir3_Left; }
		else { return Dir3_Right; }
	}
	else if (absY >= absZ)
	{
		if (vector.y < 0) { return Dir3_Down; }
		else { return Dir3_Up; }
	}
	else
	{
		if (vector.z < 0) { return Dir3_Backward; }
		else { return Dir3_Forward; }
	}
}
Dir3 ToDir3(v3i vector)
{
	if (vector == Vec3i_Zero) { return Dir3_None; }
	i32 absX = AbsI32(vector.x);
	i32 absY = AbsI32(vector.y);
	i32 absZ = AbsI32(vector.z);
	if (absX >= absY && absX >= absZ)
	{
		if (vector.x < 0) { return Dir3_Left; }
		else { return Dir3_Right; }
	}
	else if (absY >= absZ)
	{
		if (vector.y < 0) { return Dir3_Down; }
		else { return Dir3_Up; }
	}
	else
	{
		if (vector.z < 0) { return Dir3_Backward; }
		else { return Dir3_Forward; }
	}
}
#endif
Dir3 ToDir3(Axis axis, bool positive)
{
	switch (axis)
	{
		case Axis_X: return (positive ? Dir3_Right   : Dir3_Left);
		case Axis_Y: return (positive ? Dir3_Up      : Dir3_Down);
		case Axis_Z: return (positive ? Dir3_Forward : Dir3_Backward);
		default: return Dir3_None;
	}
}
#if 0
bool IsToDir3Ambiguous(v3i vector)
{
	if (vector == Vec3i_Zero) { return true; }
	i32 absX = AbsI32(vector.x);
	i32 absY = AbsI32(vector.y);
	i32 absZ = AbsI32(vector.z);
	if (absX >= absY && absX >= absZ)
	{
		return (absX == absY || absX == absZ);
	}
	else if (absY >= absZ)
	{
		return (absY == absX || absY == absZ);
	}
	else
	{
		return (absZ == absX || absZ == absY);
	}
}
#endif

Axis GetDir2Axis(Dir2 dir2)
{
	switch (dir2)
	{
		case Dir2_Right: return Axis_X;
		case Dir2_Down:  return Axis_Y;
		case Dir2_Left:  return Axis_X;
		case Dir2_Up:    return Axis_Y;
		default: return Axis_None;
	}
}
Axis GetDir3Axis(Dir3 dir3)
{
	switch (dir3)
	{
		case Dir3_Right:    return Axis_X;
		case Dir3_Left:     return Axis_X;
		case Dir3_Up:       return Axis_Y;
		case Dir3_Down:     return Axis_Y;
		case Dir3_Forward:  return Axis_Z;
		case Dir3_Backward: return Axis_Z;
		default: return Axis_None;
	}
}

Dir2Ex Dir2ExFromDir2Flags(u8 dirFlags)
{
	switch (dirFlags)
	{
		case Dir2_Right: return Dir2Ex_Right;
		case Dir2_Left:  return Dir2Ex_Left;
		case Dir2_Up:    return Dir2Ex_Up;
		case Dir2_Down:  return Dir2Ex_Down;
		case Dir2_Left|Dir2_Up:    return Dir2Ex_TopLeft;
		case Dir2_Right|Dir2_Up:   return Dir2Ex_TopRight;
		case Dir2_Right|Dir2_Down: return Dir2Ex_BottomRight;
		case Dir2_Left|Dir2_Down:  return Dir2Ex_BottomLeft;
		default: return Dir2Ex_None;
	}
}
u8 Dir2FlagsFromDir2Ex(Dir2Ex dir2ex)
{
	switch (dir2ex)
	{
		case Dir2Ex_Right:       return Dir2_Right;
		case Dir2Ex_Left:        return Dir2_Left;
		case Dir2Ex_Up:          return Dir2_Up;
		case Dir2Ex_Down:        return Dir2_Down;
		case Dir2Ex_TopLeft:     return Dir2_Left|Dir2_Up;
		case Dir2Ex_TopRight:    return Dir2_Right|Dir2_Up;
		case Dir2Ex_BottomRight: return Dir2_Right|Dir2_Down;
		case Dir2Ex_BottomLeft:  return Dir2_Left|Dir2_Down;
		default: return 0x00;
	}
}

// +--------------------------------------------------------------+
// |                     Validation Functions                     |
// +--------------------------------------------------------------+
bool IsDir2Positive(Dir2 dir2)
{
	return (dir2 == Dir2_Right || dir2 == Dir2_Down);
}

bool IsSingleDir2(Dir2 dir2, bool allowNone)
{
	if (dir2 == Dir2_None && allowNone) { return true; }
	if (dir2 == Dir2_Right) { return true; }
	if (dir2 == Dir2_Down)  { return true; }
	if (dir2 == Dir2_Left)  { return true; }
	if (dir2 == Dir2_Up)    { return true; }
	return false;
}

bool IsSingleDir2Ex(Dir2Ex dir2ex, bool allowNone)
{
	if (dir2ex == Dir2Ex_None && allowNone) { return true; }
	if (dir2ex == Dir2Ex_Right)       { return true; }
	if (dir2ex == Dir2Ex_Down)        { return true; }
	if (dir2ex == Dir2Ex_Left)        { return true; }
	if (dir2ex == Dir2Ex_Up)          { return true; }
	if (dir2ex == Dir2Ex_TopLeft)     { return true; }
	if (dir2ex == Dir2Ex_TopRight)    { return true; }
	if (dir2ex == Dir2Ex_BottomRight) { return true; }
	if (dir2ex == Dir2Ex_BottomLeft)  { return true; }
	return false;
}
bool IsCardinalDir2Ex(Dir2Ex dir2ex, bool allowNone)
{
	if (dir2ex == Dir2Ex_None && allowNone) { return true; }
	if (dir2ex == Dir2Ex_Right) { return true; }
	if (dir2ex == Dir2Ex_Down)  { return true; }
	if (dir2ex == Dir2Ex_Left)  { return true; }
	if (dir2ex == Dir2Ex_Up)    { return true; }
	return false;
}
bool IsDiagonalDir2Ex(Dir2Ex dir2ex, bool allowNone)
{
	if (dir2ex == Dir2Ex_None && allowNone) { return true; }
	if (dir2ex == Dir2Ex_TopLeft)     { return true; }
	if (dir2ex == Dir2Ex_TopRight)    { return true; }
	if (dir2ex == Dir2Ex_BottomRight) { return true; }
	if (dir2ex == Dir2Ex_BottomLeft)  { return true; }
	return false;
}

bool IsDir3Positive(Dir3 dir3)
{
	return (dir3 == Dir3_Right || dir3 == Dir3_Up || dir3 == Dir3_Forward);
}

bool IsSingleDir3(Dir3 dir3, bool allowNone)
{
	if (dir3 == Dir3_None && allowNone) { return true; }
	if (dir3 == Dir3_Right)    { return true; }
	if (dir3 == Dir3_Left)     { return true; }
	if (dir3 == Dir3_Up)       { return true; }
	if (dir3 == Dir3_Down)     { return true; }
	if (dir3 == Dir3_Forward)  { return true; }
	if (dir3 == Dir3_Backward) { return true; }
	return false;
}

u8 Dir2BitwiseCount(Dir2 dir2)
{
	u8 result = 0;
	if (IsFlagSet(dir2, Dir2_Right)) { result++; }
	if (IsFlagSet(dir2, Dir2_Down))  { result++; }
	if (IsFlagSet(dir2, Dir2_Left))  { result++; }
	if (IsFlagSet(dir2, Dir2_Up))    { result++; }
	return result;
}
u8 Dir2ExBitwiseCount(Dir2Ex dir2ex)
{
	u8 result = 0;
	if (IsFlagSet(dir2ex, Dir2Ex_Right))       { result++; }
	if (IsFlagSet(dir2ex, Dir2Ex_Down))        { result++; }
	if (IsFlagSet(dir2ex, Dir2Ex_Left))        { result++; }
	if (IsFlagSet(dir2ex, Dir2Ex_Up))          { result++; }
	if (IsFlagSet(dir2ex, Dir2Ex_TopLeft))     { result++; }
	if (IsFlagSet(dir2ex, Dir2Ex_TopRight))    { result++; }
	if (IsFlagSet(dir2ex, Dir2Ex_BottomRight)) { result++; }
	if (IsFlagSet(dir2ex, Dir2Ex_BottomLeft))  { result++; }
	return result;
}
u8 Dir3BitwiseCount(Dir3 dir3)
{
	u8 result = 0;
	if (IsFlagSet(dir3, Dir3_Right))    { result++; }
	if (IsFlagSet(dir3, Dir3_Left))     { result++; }
	if (IsFlagSet(dir3, Dir3_Up))       { result++; }
	if (IsFlagSet(dir3, Dir3_Down))     { result++; }
	if (IsFlagSet(dir3, Dir3_Forward))  { result++; }
	if (IsFlagSet(dir3, Dir3_Backward)) { result++; }
	return result;
}

// +--------------------------------------------------------------+
// |                     GetString Functions                      |
// +--------------------------------------------------------------+
const char* GetDir2String(Dir2 dir2)
{
	switch ((u8)dir2) //cast to satiate compiler warning
	{
		case Dir2_None:  return "None";
		case Dir2_Right: return "Right";
		case Dir2_Left:  return "Left";
		case Dir2_Up:    return "Up";
		case Dir2_Down:  return "Down";
		case (Dir2_Left  | Dir2_Right): return "Horizontal";
		case (Dir2_Up    | Dir2_Down):  return "Vertical";
		case (Dir2_Left  | Dir2_Up):    return "TopLeft";
		case (Dir2_Right | Dir2_Up):    return "TopRight";
		case (Dir2_Left  | Dir2_Down):  return "BottomLeft";
		case (Dir2_Right | Dir2_Down):  return "BottomRight";
		case (Dir2_Right | Dir2_Up    | Dir2_Down): return "NotLeft";
		case (Dir2_Left  | Dir2_Up    | Dir2_Down): return "NotRight";
		case (Dir2_Left  | Dir2_Right | Dir2_Down): return "NotUp";
		case (Dir2_Left  | Dir2_Right | Dir2_Up):   return "NotDown";
		case Dir2_All: return "All";
		default: return "Unknown";
	};
}

const char* GetDir2ExString(Dir2Ex dir2ex)
{
	switch ((u8)dir2ex) //cast to satiate compiler warning
	{
		case Dir2Ex_None:        return "None";
		case Dir2Ex_Right:       return "Right";
		case Dir2Ex_Left:        return "Left";
		case Dir2Ex_Up:          return "Up";
		case Dir2Ex_Down:        return "Down";
		case Dir2Ex_TopLeft:     return "TopLeft";
		case Dir2Ex_TopRight:    return "TopRight";
		case Dir2Ex_BottomLeft:  return "BottomLeft";
		case Dir2Ex_BottomRight: return "BottomRight";
		case Dir2Ex_Cardinal:    return "Cardinal";
		case Dir2Ex_Diagonal:    return "Diagonal";
		case (Dir2Ex_Left  | Dir2Ex_Right): return "Horizontal";
		case (Dir2Ex_Up    | Dir2Ex_Down):  return "Vertical";
		case (Dir2Ex_Left  | Dir2Ex_Up):    return "TopLeft";
		case (Dir2Ex_Right | Dir2Ex_Up):    return "TopRight";
		case (Dir2Ex_Left  | Dir2Ex_Down):  return "BottomLeft";
		case (Dir2Ex_Right | Dir2Ex_Down):  return "BottomRight";
		case (Dir2Ex_Right | Dir2Ex_Up    | Dir2Ex_Down): return "NotLeft";
		case (Dir2Ex_Left  | Dir2Ex_Up    | Dir2Ex_Down): return "NotRight";
		case (Dir2Ex_Left  | Dir2Ex_Right | Dir2Ex_Down): return "NotUp";
		case (Dir2Ex_Left  | Dir2Ex_Right | Dir2Ex_Up):   return "NotDown";
		//TODO: Do we want to give names to any other combinations?
		case Dir2Ex_All:         return "All";
		default: return "Unknown";
	}
}

const char* GetDir3String(Dir3 dir3)
{
	switch ((u8)dir3) //cast to satiate compiler warning
	{
		case Dir3_None:     return "None";
		case Dir3_Right:    return "Right";
		case Dir3_Left:     return "Left";
		case Dir3_Up:       return "Up";
		case Dir3_Down:     return "Down";
		case Dir3_Forward:  return "Forward";
		case Dir3_Backward: return "Backward";
		case (Dir3_Left     | Dir3_Right):    return "HorizontalX";
		case (Dir3_Up       | Dir3_Down):     return "Vertical";
		case (Dir3_Forward  | Dir3_Backward): return "HorizontalZ";
		case (Dir3_Up       | Dir3_Left ):    return "UpLeft";
		case (Dir3_Up       | Dir3_Right):    return "UpRight";
		case (Dir3_Up       | Dir3_Forward):  return "UpForward";
		case (Dir3_Up       | Dir3_Backward): return "UpBackward";
		case (Dir3_Down     | Dir3_Left ):    return "DownLeft";
		case (Dir3_Down     | Dir3_Right):    return "DownRight";
		case (Dir3_Down     | Dir3_Forward):  return "DownForward";
		case (Dir3_Down     | Dir3_Backward): return "DownBackward";
		case (Dir3_Forward  | Dir3_Left ):    return "ForwardLeft";
		case (Dir3_Forward  | Dir3_Right):    return "ForwardRight";
		case (Dir3_Backward | Dir3_Left):     return "BackwardLeft";
		case (Dir3_Backward | Dir3_Right):    return "BackwardRight";
		case (Dir3_Down | Dir3_Left  | Dir3_Backward): return "BottomLeftBack";
		case (Dir3_Down | Dir3_Right | Dir3_Backward): return "BottomRightBack";
		case (Dir3_Down | Dir3_Right | Dir3_Forward):  return "BottomRightFront";
		case (Dir3_Down | Dir3_Left  | Dir3_Forward):  return "BottomLeftFront";
		case (Dir3_Up   | Dir3_Left  | Dir3_Backward): return "TopLeftBack";
		case (Dir3_Up   | Dir3_Right | Dir3_Backward): return "TopRightBack";
		case (Dir3_Up   | Dir3_Right | Dir3_Forward):  return "TopRightFront";
		case (Dir3_Up   | Dir3_Left  | Dir3_Forward):  return "TopLeftFront";
		//NOTE: This is onle 31/64 possible values but all other 3 and more bit combinations
		//      don't really have a good name and they have at least 1 pair of opposite directions
		//TODO: We could add Not[Dir] variants
		//TODO: We could add Not[Hori/Vert] variants
		case Dir3_All: return "All";
		default: return "Unknown";
	};
}

const char* GetDir3SideString(Dir3 dir3)
{
	switch ((u8)dir3) //cast to satiate compiler warning
	{
		case Dir3_None:     return "None";
		case Dir3_Right:    return "Right";
		case Dir3_Left:     return "Left";
		case Dir3_Up:       return "Top";
		case Dir3_Down:     return "Bottom";
		case Dir3_Forward:  return "Front";
		case Dir3_Backward: return "Back";
		case (Dir3_Left     | Dir3_Right):    return "HorizontalX";
		case (Dir3_Up       | Dir3_Down):     return "Vertical";
		case (Dir3_Forward  | Dir3_Backward): return "HorizontalZ";
		case (Dir3_Up       | Dir3_Left ):    return "TopLeft";
		case (Dir3_Up       | Dir3_Right):    return "TopRight";
		case (Dir3_Up       | Dir3_Forward):  return "TopFront";
		case (Dir3_Up       | Dir3_Backward): return "TopBack";
		case (Dir3_Down     | Dir3_Left ):    return "BottomLeft";
		case (Dir3_Down     | Dir3_Right):    return "BottomRight";
		case (Dir3_Down     | Dir3_Forward):  return "BottomFront";
		case (Dir3_Down     | Dir3_Backward): return "BottomBack";
		case (Dir3_Forward  | Dir3_Left ):    return "FrontLeft";
		case (Dir3_Forward  | Dir3_Right):    return "FrontRight";
		case (Dir3_Backward | Dir3_Left):     return "BackLeft";
		case (Dir3_Backward | Dir3_Right):    return "BackRight";
		case (Dir3_Down | Dir3_Left  | Dir3_Backward): return "BottomLeftBack";
		case (Dir3_Down | Dir3_Right | Dir3_Backward): return "BottomRightBack";
		case (Dir3_Down | Dir3_Right | Dir3_Forward):  return "BottomRightFront";
		case (Dir3_Down | Dir3_Left  | Dir3_Forward):  return "BottomLeftFront";
		case (Dir3_Up   | Dir3_Left  | Dir3_Backward): return "TopLeftBack";
		case (Dir3_Up   | Dir3_Right | Dir3_Backward): return "TopRightBack";
		case (Dir3_Up   | Dir3_Right | Dir3_Forward):  return "TopRightFront";
		case (Dir3_Up   | Dir3_Left  | Dir3_Forward):  return "TopLeftFront";
		//NOTE: This is onle 31/64 possible values but all other 3 and more bit combinations
		//      don't really have a good name and they have at least 1 pair of opposite directions
		//TODO: We could add Not[Dir] variants
		//TODO: We could add Not[Hori/Vert] variants
		case Dir3_All: return "All";
		default: return "Unknown";
	};
}

// +--------------------------------------------------------------+
// |                   Index Related Functions                    |
// +--------------------------------------------------------------+
// +==============================+
// |         GetDirIndex          |
// +==============================+
u8 GetDir2Index(Dir2 dir2)
{
	switch (dir2)
	{
		case Dir2_Right: return 0;
		case Dir2_Down:  return 1;
		case Dir2_Left:  return 2;
		case Dir2_Up:    return 3;
		default: return Dir2_Count;
	}
}
u8 GetDir2ExIndex(Dir2Ex dir2ex)
{
	switch (dir2ex)
	{
		case Dir2Ex_Right:       return 0;
		case Dir2Ex_Down:        return 1;
		case Dir2Ex_Left:        return 2;
		case Dir2Ex_Up:          return 3;
		case Dir2Ex_TopLeft:     return 4;
		case Dir2Ex_TopRight:    return 5;
		case Dir2Ex_BottomRight: return 6;
		case Dir2Ex_BottomLeft:  return 7;
		default: return Dir2Ex_Count;
	}
}
u8 GetDir3Index(Dir3 dir3)
{
	switch (dir3)
	{
		case Dir3_Right:    return 0;
		case Dir3_Left:     return 1;
		case Dir3_Up:       return 2;
		case Dir3_Down:     return 3;
		case Dir3_Forward:  return 4;
		case Dir3_Backward: return 5;
		default: return Dir3_Count;
	}
}
u8 GetAxisIndex(Axis axis)
{
	switch (axis)
	{
		case Axis_X: return 0;
		case Axis_Y: return 1;
		case Axis_Z: return 2;
		case Axis_W: return 3;
		default: return Axis_Count;
	}
}

// +==============================+
// |         DirFromIndex         |
// +==============================+
Dir2 Dir2FromIndex(u64 index) //clockwise starting from right
{
	switch (index % Dir2_Count)
	{
		case 0: return Dir2_Right;
		case 1: return Dir2_Down;
		case 2: return Dir2_Left;
		case 3: return Dir2_Up;
		default: Assert(false); return Dir2_None; //shouldn't be possible because % above
	}
}
Dir2 Dir2FromCwIndex(u64 index) //same as Dir2FromIndex
{
	return Dir2FromIndex(index);
}
Dir2 Dir2FromCwIndexStartingWith(Dir2 startingDir, u64 index)
{
	u64 offset = GetDir2Index(startingDir);
	return Dir2FromCwIndex(index + offset);
}
Dir2 Dir2FromCcwIndex(u64 index)
{
	switch (index % Dir2_Count)
	{
		case 0: return Dir2_Right;
		case 1: return Dir2_Up;
		case 2: return Dir2_Left;
		case 3: return Dir2_Down;
		default: Assert(false); return Dir2_None; //shouldn't be possible because % above
	}
}
Dir2 Dir2FromCcwIndexStartingWith(Dir2 startingDir, u64 index)
{
	u64 offset = (startingDir != Dir2_Right) ? (Dir2_Count - GetDir2Index(startingDir)) : 0;
	return Dir2FromCcwIndex(index + offset);
}
Dir2Ex Dir2ExFromIndex(u64 index)
{
	switch (index % Dir2Ex_Count)
	{
		case 0: return Dir2Ex_Right;
		case 1: return Dir2Ex_Down;
		case 2: return Dir2Ex_Left;
		case 3: return Dir2Ex_Up;
		case 4: return Dir2Ex_TopLeft;
		case 5: return Dir2Ex_TopRight;
		case 6: return Dir2Ex_BottomRight;
		case 7: return Dir2Ex_BottomLeft;
		default: Assert(false); return Dir2Ex_None; //shouldn't be possible because % above
	}
}
Dir3 Dir3FromIndex(u64 index)
{
	switch (index % Dir3_Count)
	{
		case 0: return Dir3_Right;
		case 1: return Dir3_Left;
		case 2: return Dir3_Up;
		case 3: return Dir3_Down;
		case 4: return Dir3_Forward;
		case 5: return Dir3_Backward;
		default: Assert(false); return Dir3_None; //shouldn't be possible because % above
	}
}
Axis AxisFromIndex(u64 index)
{
	switch (index % Axis_Count)
	{
		case 0: return Axis_X;
		case 1: return Axis_Y;
		case 2: return Axis_Z;
		case 3: return Axis_W;
		default: Assert(false); return Axis_None; //shouldn't be possible because % above
	}
}

// +--------------------------------------------------------------+
// |                 Relative Direction Functions                 |
// +--------------------------------------------------------------+
//TODO: Make the Opposite functions handle multiple bits being set in the direction
// +==============================+
// |           Opposite           |
// +==============================+
Dir2 Dir2Opposite(Dir2 dir2)
{
	Assert(IsSingleDir2(dir2, false));
	switch (dir2)
	{
		case Dir2_Right: return Dir2_Left;
		case Dir2_Down:  return Dir2_Up;
		case Dir2_Left:  return Dir2_Right;
		case Dir2_Up:    return Dir2_Down;
		default: return Dir2_None;
	}
}
Dir2Ex Dir2ExOpposite(Dir2Ex dir2ex)
{
	Assert(IsSingleDir2Ex(dir2ex, false));
	switch (dir2ex)
	{
		case Dir2Ex_Right: return Dir2Ex_Left;
		case Dir2Ex_Down:  return Dir2Ex_Up;
		case Dir2Ex_Left:  return Dir2Ex_Right;
		case Dir2Ex_Up:    return Dir2Ex_Down;
		case Dir2Ex_TopLeft:     return Dir2Ex_BottomRight;
		case Dir2Ex_TopRight:    return Dir2Ex_BottomLeft;
		case Dir2Ex_BottomRight: return Dir2Ex_TopLeft;
		case Dir2Ex_BottomLeft:  return Dir2Ex_TopRight;
		default: return Dir2Ex_None;
	}
}
Dir3 Dir3Opposite(Dir3 dir3)
{
	Assert(IsSingleDir3(dir3, false));
	switch (dir3)
	{
		case Dir3_Right:    return Dir3_Left;
		case Dir3_Left:     return Dir3_Right;
		case Dir3_Up:       return Dir3_Down;
		case Dir3_Down:     return Dir3_Up;
		case Dir3_Forward:  return Dir3_Backward;
		case Dir3_Backward: return Dir3_Forward;
		default: return Dir3_None;
	}
}

// +==============================+
// |          Clockwise           |
// +==============================+
Dir2 Dir2ClockwiseBy(Dir2 dir2, u64 numQuarterTurns)
{
	Assert(IsSingleDir2(dir2, false));
	return Dir2FromCwIndexStartingWith(dir2, numQuarterTurns);
}
Dir2 Dir2Clockwise(Dir2 dir2) { return Dir2ClockwiseBy(dir2, 1); }
Dir2 Dir2CounterClockwiseBy(Dir2 dir2, u64 numQuarterTurns)
{
	Assert(IsSingleDir2(dir2, false));
	return Dir2FromCcwIndexStartingWith(dir2, numQuarterTurns);
}
Dir2 Dir2CounterClockwise(Dir2 dir2) { return Dir2CounterClockwiseBy(dir2, 1); }

//TODO: Add Dir2ExClockwise and Dir2ExCounterClockwise

// +==============================+
// |      GetTurnsBetweenDir      |
// +==============================+
u8 GetCwTurnsBetweenDir2(Dir2 start, Dir2 end)
{
	Assert(IsSingleDir2(start, false));
	Assert(IsSingleDir2(end, false));
	u8 startIndex = GetDir2Index(start);
	u8 endIndex = GetDir2Index(end);
	if (endIndex < startIndex) { endIndex += Dir2_Count; }
	return endIndex - startIndex;
}
u8 GetCcwTurnsBetweenDir2(Dir2 start, Dir2 end)
{
	Assert(IsSingleDir2(start, false));
	Assert(IsSingleDir2(end, false));
	u8 startIndex = Dir2_Count - GetDir2Index(start);
	u8 endIndex = Dir2_Count - GetDir2Index(end);
	if (endIndex < startIndex) { endIndex += Dir2_Count; }
	return endIndex - startIndex;
}
u8 GetTurnsBetweenDir3(Dir3 start, Dir3 end)
{
	Assert(IsSingleDir3(start, false));
	Assert(IsSingleDir3(end, false));
	switch (start)
	{
		case Dir3_Right: switch (end)
		{
			case Dir3_Right:    return 0;
			case Dir3_Left:     return 2;
			case Dir3_Up:       return 1;
			case Dir3_Down:     return 1;
			case Dir3_Forward:  return 1;
			case Dir3_Backward: return 1;
			default: return 0;
		} break;
		case Dir3_Left: switch (end)
		{
			case Dir3_Right:    return 2;
			case Dir3_Left:     return 0;
			case Dir3_Up:       return 1;
			case Dir3_Down:     return 1;
			case Dir3_Forward:  return 1;
			case Dir3_Backward: return 1;
			default: return 0;
		} break;
		case Dir3_Up: switch (end)
		{
			case Dir3_Right:    return 1;
			case Dir3_Left:     return 1;
			case Dir3_Up:       return 0;
			case Dir3_Down:     return 2;
			case Dir3_Forward:  return 1;
			case Dir3_Backward: return 1;
			default: return 0;
		} break;
		case Dir3_Down: switch (end)
		{
			case Dir3_Right:    return 1;
			case Dir3_Left:     return 1;
			case Dir3_Up:       return 2;
			case Dir3_Down:     return 0;
			case Dir3_Forward:  return 1;
			case Dir3_Backward: return 1;
			default: return 0;
		} break;
		case Dir3_Forward: switch (end)
		{
			case Dir3_Right:    return 1;
			case Dir3_Left:     return 1;
			case Dir3_Up:       return 1;
			case Dir3_Down:     return 1;
			case Dir3_Forward:  return 0;
			case Dir3_Backward: return 2;
			default: return 0;
		} break;
		case Dir3_Backward: switch (end)
		{
			case Dir3_Right:    return 1;
			case Dir3_Left:     return 1;
			case Dir3_Up:       return 1;
			case Dir3_Down:     return 1;
			case Dir3_Forward:  return 2;
			case Dir3_Backward: return 0;
			default: return 0;
		} break;
		default: return 0;
	}
}

// +--------------------------------------------------------------+
// |                       Other Functions                        |
// +--------------------------------------------------------------+
r32 GetDir2AngleR32(Dir2 dir2)
{
	Assert(IsSingleDir2(dir2, false));
	switch (dir2)
	{
		case Dir2_Right: return 0.0f;
		case Dir2_Down:  return HalfPi32;
		case Dir2_Left:  return Pi32;
		case Dir2_Up:    return ThreeHalfsPi32;
		default: return 0.0f;
	}
}
r64 GetDir2AngleR64(Dir2 dir2)
{
	Assert(IsSingleDir2(dir2, false));
	switch (dir2)
	{
		case Dir2_Right: return 0.0;
		case Dir2_Down:  return HalfPi64;
		case Dir2_Left:  return Pi64;
		case Dir2_Up:    return ThreeHalfsPi64;
		default: return 0.0;
	}
}

r32 GetDir2ExAngleR32(Dir2Ex dir2ex)
{
	Assert(IsSingleDir2Ex(dir2ex, false));
	switch (dir2ex)
	{
		case Dir2Ex_Right: return 0.0f;
		case Dir2Ex_Down:  return HalfPi32;
		case Dir2Ex_Left:  return Pi32;
		case Dir2Ex_Up:    return ThreeHalfsPi32;
		case Dir2Ex_TopLeft:     return (Pi32 + QuarterPi32);
		case Dir2Ex_TopRight:    return (Pi32 + 3*QuarterPi32);
		case Dir2Ex_BottomRight: return QuarterPi32;
		case Dir2Ex_BottomLeft:  return 3*QuarterPi32;
		default: return 0.0f;
	}
}
r64 GetDir2ExAngleR64(Dir2Ex dir2ex)
{
	Assert(IsSingleDir2Ex(dir2ex, false));
	switch (dir2ex)
	{
		case Dir2Ex_Right: return 0.0;
		case Dir2Ex_Down:  return HalfPi64;
		case Dir2Ex_Left:  return Pi64;
		case Dir2Ex_Up:    return ThreeHalfsPi64;
		case Dir2Ex_TopLeft:     return (Pi64 + QuarterPi64);
		case Dir2Ex_TopRight:    return (Pi64 + 3*QuarterPi64);
		case Dir2Ex_BottomRight: return QuarterPi64;
		case Dir2Ex_BottomLeft:  return 3*QuarterPi64;
		default: return 0.0;
	}
}

char GetDir2Char(Dir2 dir2)
{
	Assert(IsSingleDir2(dir2, true) || dir2 == Dir2_All);
	switch (dir2)
	{
		case Dir2_None:  return 'o';
		case Dir2_Right: return '>';
		case Dir2_Down:  return 'v';
		case Dir2_Left:  return '<';
		case Dir2_Up:    return '^';
		case Dir2_All:   return '+';
		default: return '?';
	}
}

Dir2 GetCardinalDir2sFromDir2Ex(Dir2Ex diagonalDir)
{
	u8 result = 0x00;
	if (IsFlagSet(diagonalDir, Dir2Ex_Right))       { result |= Dir2_Right;           }
	if (IsFlagSet(diagonalDir, Dir2Ex_Down))        { result |= Dir2_Down;            }
	if (IsFlagSet(diagonalDir, Dir2Ex_Left))        { result |= Dir2_Left;            }
	if (IsFlagSet(diagonalDir, Dir2Ex_Up))          { result |= Dir2_Up;              }
	if (IsFlagSet(diagonalDir, Dir2Ex_TopRight))    { result |= Dir2_Up|Dir2_Right;   }
	if (IsFlagSet(diagonalDir, Dir2Ex_TopLeft))     { result |= Dir2_Up|Dir2_Left;    }
	if (IsFlagSet(diagonalDir, Dir2Ex_BottomRight)) { result |= Dir2_Down|Dir2_Right; }
	if (IsFlagSet(diagonalDir, Dir2Ex_BottomLeft))  { result |= Dir2_Down|Dir2_Left;  }
	return (Dir2)result;
}

#if 0
//TODO: Rename these Vec2Clockwise90 and add VecCounterClockwise90
v2 RotateVec2NumTurnsClockwise(v2 vector, u64 numQuarterTurns)
{
	switch (numQuarterTurns % Dir2_Count)
	{
		case 0: return vector;                 //multiple of 4 turns is basically no turns
		case 1: return Vec2PerpRight(vector);  //1 turn clockwise is perp right
		case 2: return -vector;                //2 turns clockwise is opposite
		case 3: return Vec2PerpLeft(vector);   //3 turns clockwise is perp left
		default: Assert(false); return vector; //shouldn't be possible because % above
	}
}
v2i RotateVec2iNumTurnsClockwise(v2i vector, u64 numQuarterTurns)
{
	switch (numQuarterTurns % Dir2_Count)
	{
		case 0: return vector;                 //multiple of 4 turns is basically no turns
		case 1: return Vec2iPerpRight(vector); //1 turn clockwise is perp right
		case 2: return -vector;                //2 turns clockwise is opposite
		case 3: return Vec2iPerpLeft(vector);  //3 turns clockwise is perp left
		default: Assert(false); return vector; //shouldn't be possible because % above
	}
}

//TODO: Rename to Vec2UnitRecCornerFromIndex and add Vec2UnitCubeCornerFromIndex
v2 Get2DCornerVecByIndex(u64 cornerIndex)
{
	switch (cornerIndex % 4)
	{
		case 0: return NewVec2(0, 0);
		case 1: return NewVec2(1, 0);
		case 2: return NewVec2(1, 1);
		case 3: return NewVec2(0, 1);
		default: Assert(false); return Vec2_Zero;
	}
}

//TODO: Rename this Vec3iDotDir3 and add Vec3DotDir3, Vec2DotDir2, and Vec2iDotDir2
i32 Vec3iAmountInDir(v3i vector, Dir3 direction)
{
	switch (direction)
	{
		case Dir3_Right:    return vector.x;
		case Dir3_Up:       return vector.y;
		case Dir3_Forward:  return vector.z;
		case Dir3_Left:     return -vector.x;
		case Dir3_Down:     return -vector.y;
		case Dir3_Backward: return -vector.z;
		default: DebugAssert(false); return 0;
	}
}
#endif

#endif //  _STRUCT_DIRECTIONS_H
