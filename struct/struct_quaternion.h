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
#include "lib/lib_handmade_math.h"

//TODO: Update HandmadeMath.h to get BYP's HMM_QFromNormPair/HMM_QFromVecPair

// +--------------------------------------------------------------+
// |                   Typedefs and Structures                    |
// +--------------------------------------------------------------+
typedef car QuatR64 QuatR64;
car QuatR64
{
	r64 Elements[4];
	plex
	{
		car
		{
			v3d XYZ;
			plex { r64 X, Y, Z; };
		};
		r64 W;
	};
};

typedef HMM_Quat quat;
typedef QuatR64 quatd;

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE quatd NewQuatd(r64 x, r64 y, r64 z, r64 w);
	PIG_CORE_INLINE quatd ToQuatdFromAxis(v3d axis, r64 angle);
	PIG_CORE_INLINE quatd AddQuatd(quatd left, quatd right);
	PIG_CORE_INLINE quatd SubQuatd(quatd left, quatd right);
	PIG_CORE_INLINE quatd MulQuatd(quatd left, quatd right);
	PIG_CORE_INLINE quatd ScaleQuatd(quatd quaternion, r64 scalar);
	PIG_CORE_INLINE quatd ShrinkQuatd(quatd quaternion, r64 divisor);
	PIG_CORE_INLINE r64 DotQuatd(quatd left, quatd right);
	PIG_CORE_INLINE quatd InvertQuatd(quatd quaternion);
	PIG_CORE_INLINE quatd NormalizeQuatd(quatd quaternion);
	PIG_CORE_INLINE quatd LerpQuatd(quatd start, quatd end, r64 amount);
	PIG_CORE_INLINE quatd SlerpQuatd(quatd start, quatd end, r64 amount);
	PIG_CORE_INLINE bool AreEqualQuat(quat left, quat right);
	PIG_CORE_INLINE bool AreEqualQuatd(quatd left, quatd right);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define NewQuat(x, y, z, w) HMM_Q((x), (y), (z), (w))

#define MakeQuat(x, y, z, w) NEW_STRUCT(quat){ .X=(x), .Y=(y), .Z=(z), .W=(w) }
#define MakeQuatd(x, y, z, w) NEW_STRUCT(quatd){ .X=(x), .Y=(y), .Z=(z), .W=(w) }

#define ToQuatFromV4(vec4) HMM_QV4(vec4)
#define ToQuatdFromV4d(vec4d) MakeQuatd((vec4d).X, (vec4d).Y, (vec4d).Z, (vec4d).W)
#define ToV4FromQuat(quaternion) MakeV4((quaternion).X, (quaternion).Y, (quaternion).Z, (quaternion).W)
#define ToV4dFromQuatd(quaternion) MakeV4d((quaternion).X, (quaternion).Y, (quaternion).Z, (quaternion).W)
#define ToQuatFromAxis(axis, angle) HMM_QFromAxisAngle_LH((axis), (angle))

#define Quat_Identity  MakeQuat(0.0f, 0.0f, 0.0f, 1.0f)
#define Quatd_Identity MakeQuatd(0.0, 0.0, 0.0, 1.0)

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

PEXPI quatd NewQuatd(r64 x, r64 y, r64 z, r64 w)
{
	quatd result;
	result.X = x;
	result.Y = y;
	result.Z = z;
	result.W = w;
	return result;
}

PEXPI quatd ToQuatdFromAxis(v3d axis, r64 angle)
{
	quatd result;
	v3d axisNormalized = NormalizeV3d(axis);
	r64 sineOfRotation = SinR64(angle / 2.0);
	result.XYZ = ScaleV3d(axisNormalized, sineOfRotation);
	result.W = CosR64(angle / 2.0);
	return result;
}

PEXPI quatd AddQuatd(quatd left, quatd right) { quatd result; result.X = left.X + right.X; result.Y = left.Y + right.Y; result.Z = left.Z + right.Z; result.W = left.W + right.W; return result; }
PEXPI quatd SubQuatd(quatd left, quatd right) { quatd result; result.X = left.X - right.X; result.Y = left.Y - right.Y; result.Z = left.Z - right.Z; result.W = left.W - right.W; return result; }

PEXPI quatd MulQuatd(quatd left, quatd right)
{
	quatd result;
	
	result.X =  right.W * +left.X;
	result.Y =  right.Z * -left.X;
	result.Z =  right.Y * +left.X;
	result.W =  right.X * -left.X;
	
	result.X += right.Z * +left.Y;
	result.Y += right.W * +left.Y;
	result.Z += right.X * -left.Y;
	result.W += right.Y * -left.Y;
	
	result.X += right.Y * -left.Z;
	result.Y += right.X * +left.Z;
	result.Z += right.W * +left.Z;
	result.W += right.Z * -left.Z;
	
	result.X += right.X * +left.W;
	result.Y += right.Y * +left.W;
	result.Z += right.Z * +left.W;
	result.W += right.W * +left.W;
	
	return result;
}

PEXPI quatd ScaleQuatd(quatd quaternion, r64 scalar)
{
	quatd result;
	result.X = quaternion.X * scalar;
	result.Y = quaternion.Y * scalar;
	result.Z = quaternion.Z * scalar;
	result.W = quaternion.W * scalar;
	return result;
}

PEXPI quatd ShrinkQuatd(quatd quaternion, r64 divisor)
{
	quatd result;
	result.X = quaternion.X / divisor;
	result.Y = quaternion.Y / divisor;
	result.Z = quaternion.Z / divisor;
	result.W = quaternion.W / divisor;
	return result;
}

PEXPI r64 DotQuatd(quatd left, quatd right)
{
	//NOTE: We removed "extra" parenthesis around these adds, but technically this might cause slightly more floating point error accumulation
	return (left.X * right.X) + (left.Y * right.Y) + (left.Z * right.Z) + (left.W * right.W);
}

PEXPI quatd InvertQuatd(quatd quaternion)
{
	quatd result;
	result.X = -quaternion.X;
	result.Y = -quaternion.Y;
	result.Z = -quaternion.Z;
	result.W = quaternion.W;
	return result;
}

PEXPI quatd NormalizeQuatd(quatd quaternion)
{
	v4d vector = ToV4dFromQuatd(quaternion);
	vector = NormalizeV4d(vector);
	return ToQuatdFromV4d(vector);
}

PEXPI quatd LerpQuatd(quatd start, quatd end, r64 amount)
{
	quatd result = AddQuatd(ScaleQuatd(start, (1.0-amount)), ScaleQuatd(end, amount));
	result = NormalizeQuatd(result);
	return result;
}
PEXPI quatd SlerpQuatd(quatd start, quatd end, r64 amount)
{
	quatd result;
	r64 cosTheta = DotQuatd(start, end);
	
	if (cosTheta < 0.0)
	{
		cosTheta = -cosTheta;
		end = NewQuatd(-end.X, -end.Y, -end.Z, -end.W);
	}
	
	if (cosTheta > 0.9995) //TODO: Should we choose a number closer to 1.0 when working in 64-bit floats?
	{
		result = LerpQuatd(start, end, amount);
	}
	else
	{
		r64 angle = AcosR64(cosTheta);
		r64 mixStart = SinR64((1.0 - amount) * angle);
		r64 mixEnd = SinR64(amount * angle);
		result = AddQuatd(ScaleQuatd(start, mixStart), ScaleQuatd(end, mixEnd));
		result = NormalizeQuatd(result);
	}
	
	return result;
}

PEXPI bool AreEqualQuat(quat left, quat right) { return (left.X == right.X && left.Y == right.Y && left.Z == right.Z && left.W == right.W); }
PEXPI bool AreEqualQuatd(quatd left, quatd right) { return (left.X == right.X && left.Y == right.Y && left.Z == right.Z && left.W == right.W); }

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
