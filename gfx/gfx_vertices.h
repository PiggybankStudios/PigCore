/*
File:   gfx_vertices.h
Author: Taylor Robbins
Date:   01\28\2025
Description:
	** Contains various structures that represent vertices in a 2D or 3D graphical application
*/

#ifndef _GFX_VERTICES_H
#define _GFX_VERTICES_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "struct/struct_vectors.h"

#define MAX_NUM_VERT_ATTRIBUTES    8

typedef enum VertAttributeType VertAttributeType;
enum VertAttributeType
{
	VertAttributeType_None = 0,
	VertAttributeType_Position,
	VertAttributeType_Normal,
	VertAttributeType_Tangent,
	VertAttributeType_Color,
	VertAttributeType_TexCoord,
	VertAttributeType_Count,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetVertAttributeTypeStr(VertAttributeType enumValue);
const char* GetVertAttributeMatchStr(VertAttributeType enumValue);
#else
PEXP const char* GetVertAttributeTypeStr(VertAttributeType enumValue)
{
	switch (enumValue)
	{
		case VertAttributeType_None:     return "None";
		case VertAttributeType_Position: return "Position";
		case VertAttributeType_Normal:   return "Normal";
		case VertAttributeType_Tangent:  return "Tangent";
		case VertAttributeType_Color:    return "Color";
		case VertAttributeType_TexCoord: return "TexCoord";
		default: return UNKNOWN_STR;
	}
}
//NOTE: This is the string we match against when evaluating the name of an attribute in the shader file to determine it's Type
PEXP const char* GetVertAttributeMatchStr(VertAttributeType enumValue)
{
	switch (enumValue)
	{
		case VertAttributeType_Position: return "position";
		case VertAttributeType_Normal:   return "normal";
		case VertAttributeType_Tangent:  return "tangent";
		case VertAttributeType_Color:    return "color";
		case VertAttributeType_TexCoord: return "tex";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef plex VertAttribute VertAttribute;
plex VertAttribute
{
	VertAttributeType type;
	u8 size; //in bytes
	u8 offset; //in bytes
};

typedef car Vertex2D Vertex2D;
car Vertex2D
{
	r32 values[8];
	plex
	{
		v2 position;
		v2 texCoord;
		v4r color;
	};
	plex
	{
		r32 X, Y;
		r32 tX, tY;
		r32 R, G, B, A;
	};
};
#if STATIC_ASSERT_AVAILABLE
_Static_assert(sizeof(Vertex2D) == sizeof(r32)*8, "Vertex2D contains padding!");
#endif

typedef car Vertex3D Vertex3D;
car Vertex3D
{
	r32 values[12];
	plex
	{
		v3 position;
		v3 normal;
		v2 texCoord;
		v4r color;
	};
	plex
	{
		r32 X, Y, Z;
		r32 nX, nY, nZ;
		r32 tX, tY;
		r32 R, G, B, A;
	};
};
#if STATIC_ASSERT_AVAILABLE
_Static_assert(sizeof(Vertex3D) == sizeof(r32)*12, "Vertex3D contains padding!");
#endif

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Vertex2D NewVertex2D(v2 position, v2 texCoord, v4 color);
	PIG_CORE_INLINE Vertex3D NewVertex3D(v3 position, v3 normal, v2 texCoord, v4 color);
#endif

#define Vertex2D_Size       sizeof(Vertex2D)
#define Vertex2D_NumFloats  (sizeof(Vertex2D)/sizeof(r32))
#define Vertex2D_Zero       NewVertex2D(V2_Zero_Const, V2_Zero_Const, V4_Zero_Const)
#define Vertex2D_Zero_Const { .position = V2_Zero_Const, .texCoord = V2_Zero_Const, .color = V4_Zero_Const }

#define Vertex3D_Size       sizeof(Vertex3D)
#define Vertex3D_NumFloats  (sizeof(Vertex3D)/sizeof(r32))
#define Vertex3D_Zero       NewVertex3D(V3_Zero_Const, V3_Zero_Const, V2_Zero_Const, V4_Zero_Const)
#define Vertex3D_Zero_Const { .position = V3_Zero_Const, .normal = V3_Zero_Const, .texCoord = V2_Zero_Const, .color = V4_Zero_Const }

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI Vertex2D NewVertex2D(v2 position, v2 texCoord, v4 color)
{
	Vertex2D result;
	result.position = position;
	result.texCoord = texCoord;
	result.color = ToV4rFrom4(color);
	return result;
}

PEXPI Vertex3D NewVertex3D(v3 position, v3 normal, v2 texCoord, v4 color)
{
	Vertex3D result;
	result.position = position;
	result.normal = normal;
	result.texCoord = texCoord;
	result.color = ToV4rFrom4(color);
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _GFX_VERTICES_H
