/*
File:   cross_quaternion_and_box3d.h
Author: Taylor Robbins
Date:   09\13\2026
*/

#ifndef _CROSS_QUATERNION_AND_BOX_3D_H
#define _CROSS_QUATERNION_AND_BOX_3D_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE quat ToQuatFromB3Quat(b3Quat box3dQuaternion);
	PIG_CORE_INLINE b3Quat ToB3QuatFromQuat(quat quaternion);
#endif

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define MakeB3Quat_Const(xValue, yValue, zValue, wValue) { .v={ .x=(xValue), .y=(yValue), .z=(zValue) }, .s=(wValue) }
#define MakeB3Quat(x, y, z, w)                           NEW_STRUCT(b3Quat)MakeB3Quat_Const((x), (y), (z), (w))
#define MakeB3QuatFromQuat_Const(quaternion)             MakeB3Quat_Const((quaternion).x, (quaternion).y, (quaternion).z, (quaternion).w)
#define MakeB3QuatFromQuat(quaternion)                   MakeB3Quat((quaternion).x, (quaternion).y, (quaternion).z, (quaternion).w)
#define MakeQuatFromB3Quat_Const(box3dQuaternion)        MakeQuat_Const((box3dQuaternion).v.x, (box3dQuaternion).v.y, (box3dQuaternion).v.z, (box3dQuaternion).s)
#define MakeQuatFromB3Quat(box3dQuaternion)              MakeQuat((box3dQuaternion).v.x, (box3dQuaternion).v.y, (box3dQuaternion).v.z, (box3dQuaternion).s)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI quat ToQuatFromB3Quat(b3Quat box3dQuaternion) { return MakeQuatFromB3Quat(box3dQuaternion); }
PEXPI b3Quat ToB3QuatFromQuat(quat quaternion) { return MakeB3QuatFromQuat(quaternion); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_QUATERNION_AND_BOX_3D_H
