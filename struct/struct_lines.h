/*
File:   struct_lines.h
Author: Taylor Robbins
Date:   02\02\2025
*/

#ifndef _STRUCT_LINES_H
#define _STRUCT_LINES_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "struct/struct_vectors.h"

typedef car Line2D Line2D;
car Line2D
{
	r32 Elements[4];
	v2 Points[2];
	plex { v2 Start; v2 End; };
};
#if LANGUAGE_IS_C
_Static_assert(sizeof(Line2D) == sizeof(r32)*4, "Line2D contains padding!");
#endif

typedef car Line3D Line3D;
car Line3D
{
	r32 Elements[6];
	v3 Points[2];
	plex { v3 Start; v3 End; };
};
#if LANGUAGE_IS_C
_Static_assert(sizeof(Line3D) == sizeof(r32)*6, "Line3D contains padding!");
#endif

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Line2D NewLine2D(r32 startX, r32 startY, r32 endX, r32 endY);
	PIG_CORE_INLINE Line2D NewLine2DV(v2 start, v2 end);
	PIG_CORE_INLINE Line3D NewLine3D(r32 startX, r32 startY, r32 startZ, r32 endX, r32 endY, r32 endZ);
	PIG_CORE_INLINE Line3D NewLine3DV(v3 start, v3 end);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI Line2D NewLine2D(r32 startX, r32 startY, r32 endX, r32 endY)
{
	Line2D result = ZEROED;
	result.Start.X = startX;
	result.Start.Y = startY;
	result.End.X = endX;
	result.End.Y = endY;
	return result;
}
PEXPI Line2D NewLine2DV(v2 start, v2 end)
{
	Line2D result = ZEROED;
	result.Start = start;
	result.End = end;
	return result;
}

PEXPI Line3D NewLine3D(r32 startX, r32 startY, r32 startZ, r32 endX, r32 endY, r32 endZ)
{
	Line3D result = ZEROED;
	result.Start.X = startX;
	result.Start.Y = startY;
	result.Start.Z = startZ;
	result.End.X = endX;
	result.End.Y = endY;
	result.End.Z = endZ;
	return result;
}
PEXPI Line3D NewLine3DV(v3 start, v3 end)
{
	Line3D result = ZEROED;
	result.Start = start;
	result.End = end;
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_LINES_H
