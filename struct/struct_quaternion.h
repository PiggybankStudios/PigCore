/*
File:   struct_quaternion.h
Author: Taylor Robbins
Date:   01\15\2025
Description:
	** Since we depend on HandmadeMath.h for the all of our quaternion math related
	** functions and types, this file serves more as a aliasing file for all the
	** things in HandmadeMath.h, with some extensions for functions that we had
	** in GyLib that HandmadeMath.h does not have.
*/

#ifndef _STRUCT_QUATERNION_H
#define _STRUCT_QUATERNION_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "std/std_includes.h"
#include "struct/struct_vectors.h"

#include "struct/struct_handmade_math_include.h"

// +--------------------------------------------------------------+
// |                           Typedefs                           |
// +--------------------------------------------------------------+
typedef HMM_Quat quat;

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE bool AreEqualQuat(quat left, quat right);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define NewQuat(x, y, z, w) HMM_Q((x), (y), (z), (w))

#define ToQuatFromV4(vec4) HMM_QV4(vec4)
#define ToV4FromQuat(quaternion) NewV4((quaternion).X, (quaternion).Y, (quaternion).Z, (quaternion).W)
#define ToQuatFromAxis(axis, angle) HMM_QFromAxisAngle_LH((axis), (angle))

#define Quat_Identity NewQuat(0.0f, 0.0f, 0.0f, 1.0f)
#define Quat_Identity_Const (quat){{{{{ 0.0f, 0.0f, 0.0f }}}, 1.0f }}

#define AddQuat(left, right) HMM_AddQ((left), (right))

#define SubQuat(left, right) HMM_SubQ((left), (right))

#define MulQuat(left, right) HMM_MulQ((left), (right))

#define ScaleQuat(quaternion, scalar) HMM_MulQF((quaternion), (scalar))

#define ShrinkQuat(quaternion, divisor) HMM_DivQF((quaternion), (divisor))

#define DotQuat(left, right) HMM_DotQ((left), (right))

#define InvertQuat(quaternion) HMM_InvQ(quaternion)

#define NormalizeQuat(quaternion) HMM_NormQ(quaternion)

#define LerpQuat(start, end, amount) HMM_NLerp((start), (amount), (end))
#define SlerpQuat(start, end, amount) HMM_SLerp((start), (amount), (end))

//TODO: Do we want GetAngleQuat and GetAxisQuat?
//TODO: Do we want EquivalentQuat?
//TODO: Do we want OppositeQuat?

// HMM_QFromAxisAngle_RH
// HMM_QFromAxisAngle_LH

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI bool AreEqualQuat(quat left, quat right) { return (left.X == right.X && left.Y == right.Y && left.Z == right.Z && left.W == right.W); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_QUATERNION_H

#if defined(_STRUCT_QUATERNION_H) && defined(_STRUCT_MATRICES_H)
#include "cross/cross_quaternion_and_matrices.h"
#endif

#if defined(_STRUCT_VECTORS_H) && defined(_STRUCT_QUATERNION_H) && defined(_STRUCT_MATRICES_H) && defined(_STRUCT_RECTANGLES_H)
#include "cross/cross_vectors_quaternion_matrices_and_rectangles.h"
#endif

#if defined(_STRUCT_QUATERNION_H) && defined(RAYLIB_H)
#include "cross/cross_quaternion_and_raylib.h"
#endif
