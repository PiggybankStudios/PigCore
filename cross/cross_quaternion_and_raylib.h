/*
File:   cross_quaternion_and_raylib.h
Author: Taylor Robbins
Date:   01\24\2025
NOTE: This file may need to be included manually if you include raylib.h AFTER including struct_quaternion.h
*/

#ifndef _CROSS_QUATERNION_AND_RAYLIB_H
#define _CROSS_QUATERNION_AND_RAYLIB_H

//NOTE: Intentionally no includes here

#if BUILD_WITH_RAYLIB

#define rlMakeQuaternion(xValue, yValue, zValue, wValue) NEW_STRUCT(Quaternion){ .x=(xValue), .y=(yValue), .z=(zValue), .w=(wValue) }

#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Quaternion QuaternionFromQuat(quat quaternion);
	PIG_CORE_INLINE quat QuatFromQuaternion(Quaternion quaternion);
#endif

#if PIG_CORE_IMPLEMENTATION

PEXPI Quaternion QuaternionFromQuat(quat quaternion) { return rlMakeQuaternion(quaternion.X, quaternion.Y, quaternion.Z, quaternion.W); }
PEXPI quat QuatFromQuaternion(Quaternion quaternion) { return MakeQuat(quaternion.x, quaternion.y, quaternion.z, quaternion.w); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_RAYLIB

#endif //  _CROSS_QUATERNION_AND_RAYLIB_H
