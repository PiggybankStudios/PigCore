/*
File:   cross_vectors_and_box3d.h
Author: Taylor Robbins
Date:   09\13\2026
*/

#ifndef _CROSS_VECTORS_AND_BOX_3D_H
#define _CROSS_VECTORS_AND_BOX_3D_H

//NOTE: Intentionally no includes here

// TODO: b3Pos if BOX3D_DOUBLE_PRECISION ?

// TODO: b3HexColor

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE v2 ToV2FromB3Vec2(b3Vec2 box3dVector);
	PIG_CORE_INLINE b3Vec2 ToB3Vec2FromV2(v2 vector);
	PIG_CORE_INLINE v3 ToV3FromB3Vec3(b3Vec3 box3dVector);
	PIG_CORE_INLINE b3Vec3 ToB3Vec3FromV3(v3 vector);
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define MakeB3Vec2_Const(xValue, yValue) { .x=(xValue), .y=(yValue) }
#define MakeB3Vec2(x, y)                 NEW_STRUCT(b3Vec2)MakeB3Vec2_Const((x), (y))
#define FillB3Vec2_Const(value)          MakeB3Vec2_Const((value), (value))
#define FillB3Vec2(value)                MakeB3Vec2((value), (value))
	
#define MakeV2FromB3Vec2_Const(box3dVector) MakeV2_Const((box3dVector).x, (box3dVector).y)
#define MakeV2FromB3Vec2(box3dVector)       MakeV2((box3dVector).x, (box3dVector).y)
#define MakeB3Vec2FromV3_Const(vector)      MakeB3Vec2_Const((vector).x, (vector).y)
#define MakeB3Vec2FromV3(vector)            MakeB3Vec2((vector).x, (vector).y)

#define MakeB3Vec3_Const(xValue, yValue, zValue) { .x=(xValue), .y=(yValue), .z=(zValue) }
#define MakeB3Vec3(x, y, z)                      NEW_STRUCT(b3Vec3)MakeB3Vec3_Const((x), (y), (z))
#define FillB3Vec3_Const(value)                  MakeB3Vec3_Const((value), (value), (value))
#define FillB3Vec3(value)                        MakeB3Vec3((value), (value), (value))

#define MakeV3FromB3Vec3_Const(box3dVector) MakeV3_Const((box3dVector).x, (box3dVector).y, (box3dVector).z)
#define MakeV3FromB3Vec3(box3dVector)       MakeV3((box3dVector).x, (box3dVector).y, (box3dVector).z)
#define MakeB3Vec3FromV3_Const(vector)      MakeB3Vec3_Const((vector).x, (vector).y, (vector).z)
#define MakeB3Vec3FromV3(vector)            MakeB3Vec3((vector).x, (vector).y, (vector).z)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI v2 ToV2FromB3Vec2(b3Vec2 box3dVector) { return MakeV2(box3dVector.x, box3dVector.y);}
PEXPI b3Vec2 ToB3Vec2FromV2(v2 vector) { return MakeB3Vec2(vector.x, vector.y);}
PEXPI v3 ToV3FromB3Vec3(b3Vec3 box3dVector) { return MakeV3(box3dVector.x, box3dVector.y, box3dVector.z); }
PEXPI b3Vec3 ToB3Vec3FromV3(v3 vector) { return MakeB3Vec3(vector.x, vector.y, vector.z); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_VECTORS_AND_BOX_3D_H
