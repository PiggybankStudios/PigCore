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
#if STATIC_ASSERT_AVAILABLE
_Static_assert(sizeof(Line2D) == sizeof(r32)*4, "Line2D contains padding!");
#endif

typedef car Line2DR64 Line2DR64;
car Line2DR64
{
	r64 Elements[4];
	v2d Points[2];
	plex { v2d Start; v2d End; };
};

typedef car Line3D Line3D;
car Line3D
{
	r32 Elements[6];
	v3 Points[2];
	plex { v3 Start; v3 End; };
};
#if STATIC_ASSERT_AVAILABLE
_Static_assert(sizeof(Line3D) == sizeof(r32)*6, "Line3D contains padding!");
#endif

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Line2D NewLine2D(r32 startX, r32 startY, r32 endX, r32 endY);
	PIG_CORE_INLINE Line2D NewLine2DV(v2 start, v2 end);
	PIG_CORE_INLINE Line2DR64 NewLine2DR64(r64 startX, r64 startY, r64 endX, r64 endY);
	PIG_CORE_INLINE Line2DR64 NewLine2DR64V(v2d start, v2d end);
	PIG_CORE_INLINE Line3D NewLine3D(r32 startX, r32 startY, r32 startZ, r32 endX, r32 endY, r32 endZ);
	PIG_CORE_INLINE Line3D NewLine3DV(v3 start, v3 end);
	PIG_CORE_INLINE r32 DistanceToLine2D(Line2D line, v2 point, v2* closestPointOut);
	PIG_CORE_INLINE r64 DistanceToLine2DR64(Line2DR64 line, v2d point, v2d* closestPointOut);
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

PEXPI Line2DR64 NewLine2DR64(r64 startX, r64 startY, r64 endX, r64 endY)
{
	Line2DR64 result = ZEROED;
	result.Start.X = startX;
	result.Start.Y = startY;
	result.End.X = endX;
	result.End.Y = endY;
	return result;
}
PEXPI Line2DR64 NewLine2DR64V(v2d start, v2d end)
{
	Line2DR64 result = ZEROED;
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

PEXPI r32 DistanceToLine2D(Line2D line, v2 point, v2* closestPointOut)
{
	v2 forwardVec = SubV2(line.End, line.Start);
	r32 forwardDot = DotV2(point, forwardVec);
	if (forwardDot >= 1.0f)
	{
		SetOptionalOutPntr(closestPointOut, line.End);
		return LengthV2(SubV2(point, line.End));
	}
	else if (forwardDot < 0.0f)
	{
		SetOptionalOutPntr(closestPointOut, line.Start);
		return LengthV2(SubV2(point, line.Start));
	}
	else
	{
		v2 closestPoint = AddV2(line.Start, ScaleV2(forwardVec, forwardDot));
		SetOptionalOutPntr(closestPointOut, closestPoint);
		return LengthV2(SubV2(point, closestPoint));
	}
}
PEXPI r64 DistanceToLine2DR64(Line2DR64 line, v2d point, v2d* closestPointOut)
{
	v2d forwardVec = SubV2d(line.End, line.Start);
	r64 forwardDot = DotV2d(point, forwardVec);
	if (forwardDot >= 1.0)
	{
		SetOptionalOutPntr(closestPointOut, line.End);
		return LengthV2d(SubV2d(point, line.End));
	}
	else if (forwardDot < 0.0)
	{
		SetOptionalOutPntr(closestPointOut, line.Start);
		return LengthV2d(SubV2d(point, line.Start));
	}
	else
	{
		v2d closestPoint = AddV2d(line.Start, ScaleV2d(forwardVec, forwardDot));
		SetOptionalOutPntr(closestPointOut, closestPoint);
		return LengthV2d(SubV2d(point, closestPoint));
	}
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_LINES_H
