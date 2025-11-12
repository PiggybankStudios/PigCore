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
#define MakeLine2D(x0, y0, x1, y1) NEW_STRUCT(Line2D){ .Start=MakeV2((x0), (y0)), .End=MakeV2((x1), (y1)) }
#define MakeLine2DV(startV2, endV2) NEW_STRUCT(Line2D){ .Start=(startV2), .End=(endV2) }

typedef car Line2DR64 Line2DR64;
car Line2DR64
{
	r64 Elements[4];
	v2d Points[2];
	plex { v2d Start; v2d End; };
};
#define MakeLine2DR64(x0, y0, x1, y1) NEW_STRUCT(Line2DR64){ .Start=MakeV2d((x0), (y0)), .End=MakeV2d((x1), (y1)) }
#define MakeLine2DR64V(startV2d, endV2d) NEW_STRUCT(Line2DR64){ .Start=(startV2d), .End=(endV2d) }

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
#define MakeLine3D(x0, y0, z0, x1, y1, z1) NEW_STRUCT(Line3D){ .Start=MakeV3((x0), (y0), (z0)), .End=MakeV3((x1), (y1), (z1)) }
#define MakeLine3DV(startV3, endV3) NEW_STRUCT(Line3D){ .Start=(startV3), .End=(endV3) }

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE r32 DistanceToLine2D(Line2D line, v2 point, v2* closestPointOut);
	PIG_CORE_INLINE r64 DistanceToLine2DR64(Line2DR64 line, v2d point, v2d* closestPointOut);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

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
