/*
File:   struct_quaternion.h
Author: Taylor Robbins
Date:   01\15\2025
Description:
	** Holds Quaternion type (basically a vector4) that is used to hold 3D rotations.
	** There are 32-bit and 64-bit variants: Quaternion_R32 (quat) and Quaternion_R64 (quatd)
	**
	** This file originally was based on HandmadeMath.h so many of the function
	** implementations are directly copied from there.
*/

#ifndef _STRUCT_QUATERNION_H
#define _STRUCT_QUATERNION_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "std/std_includes.h"
#include "std/std_trig.h"
#include "struct/struct_vectors.h"

//TODO: Update HandmadeMath.h to get BYP's HMM_QFromNormPair/HMM_QFromVecPair

// +--------------------------------------------------------------+
// |                   Typedefs and Structures                    |
// +--------------------------------------------------------------+
typedef car Quaternion_R32 Quaternion_R32;
car Quaternion_R32
{
	float elem[4];
	plex
	{
		car
		{
			v3 xyz;
			plex { float x, y, z; };
		};
		float w;
	};
	// #ifdef HANDMADE_MATH__USE_SSE
	// __m128 SSE;
	// #endif
};
#define MakeQuat_Const(X, Y, Z, W) { .x=(X), .y=(Y), .z=(Z), .w=(W) }
#define MakeQuat(x, y, z, w)       NEW_STRUCT(Quaternion_R32)MakeQuat_Const((x), (y), (z), (w))

typedef car Quaternion_R64 Quaternion_R64;
car Quaternion_R64
{
	r64 elem[4];
	plex
	{
		car
		{
			v3d xyz;
			plex { r64 x, y, z; };
		};
		r64 w;
	};
};
#define MakeQuatd_Const(X, Y, Z, W) { .x=(X), .y=(Y), .z=(Z), .w=(W) }
#define MakeQuatd(x, y, z, w)       NEW_STRUCT(Quaternion_R64)MakeQuatd_Const((x), (y), (z), (w))

typedef Quaternion_R32 quat;
typedef Quaternion_R64 quatd;

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
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
#define Quat_Identity_Const  MakeQuat_Const( 0.0f, 0.0f, 0.0f, 1.0f)
#define Quatd_Identity_Const MakeQuatd_Const(0.0,  0.0,  0.0,  1.0)

#define Quat_Identity        MakeQuat( 0.0f, 0.0f, 0.0f, 1.0f)
#define Quatd_Identity       MakeQuatd(0.0,  0.0,  0.0,  1.0)

#define ToQuatFromAxis_LH(axisVec, angleOfRotation) ToQuatFromAxis_RH((axisVec), -(angleOfRotation))
#define ToQuatFromAxis(axisVec, angleOfRotation)    ToQuatFromAxis_LH((axisVec), (angleOfRotation))

//TODO: Do we want GetAngleQuat and GetAxisQuat?
//TODO: Do we want EquivalentQuat?
//TODO: Do we want OppositeQuat?

//TODO: HMM_QFromAxisAngle_RH?
//TODO: HMM_QFromAxisAngle_LH?

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI quat ToQuatFromV4(v4 vec4) { return MakeQuat(vec4.x, vec4.y, vec4.z, vec4.w); }
PEXPI quatd ToQuatdFromV4d(v4d vec4d) { return MakeQuatd(vec4d.x, vec4d.y, vec4d.z, vec4d.w); }
PEXPI v4 ToV4FromQuat(quat quaternion) { return MakeV4(quaternion.x, quaternion.y, quaternion.z, quaternion.w); }
PEXPI v4d ToV4dFromQuatd(quatd quaternion) { return MakeV4d(quaternion.x, quaternion.y, quaternion.z, quaternion.w); }

PEXPI quat ToQuatFromAxis_RH(v3 axisVec, r32 angleOfRotation)
{
	quat result;
	v3 axisNormalized = NormalizeV3(axisVec);
	r32 sineOfRotation = SinR32(angleOfRotation / 2.0f);
	result.xyz = ScaleV3(axisNormalized, sineOfRotation);
	result.w = CosR32(angleOfRotation / 2.0f);
	return result;
}

PEXPI quat AddQuat(quat left, quat right)
{
    quat result;
	// #ifdef HANDMADE_MATH__USE_SSE
    // result.SSE = _mm_add_ps(left.SSE, right.SSE);
	// #else
    result.x = left.x + right.x;
    result.y = left.y + right.y;
    result.z = left.z + right.z;
    result.w = left.w + right.w;
	// #endif
    return result;
}

PEXPI quat SubQuat(quat left, quat right)
{
    quat result;
	// #ifdef HANDMADE_MATH__USE_SSE
    // result.SSE = _mm_sub_ps(left.SSE, right.SSE);
	// #else
    result.x = left.x - right.x;
    result.y = left.y - right.y;
    result.z = left.z - right.z;
    result.w = left.w - right.w;
	// #endif
    return result;
}

PEXPI quat MulQuat(quat left, quat right)
{
	quat result;
	// #ifdef HANDMADE_MATH__USE_SSE
	// __m128 SSEResultOne = _mm_xor_ps(_mm_shuffle_ps(left.SSE, left.SSE, _MM_SHUFFLE(0, 0, 0, 0)), _mm_setr_ps(0.f, -0.f, 0.f, -0.f));
	// __m128 SSEResultTwo = _mm_shuffle_ps(right.SSE, right.SSE, _MM_SHUFFLE(0, 1, 2, 3));
	// __m128 SSEResultThree = _mm_mul_ps(SSEResultTwo, SSEResultOne);
	
	// SSEResultOne = _mm_xor_ps(_mm_shuffle_ps(left.SSE, left.SSE, _MM_SHUFFLE(1, 1, 1, 1)) , _mm_setr_ps(0.f, 0.f, -0.f, -0.f));
	// SSEResultTwo = _mm_shuffle_ps(right.SSE, right.SSE, _MM_SHUFFLE(1, 0, 3, 2));
	// SSEResultThree = _mm_add_ps(SSEResultThree, _mm_mul_ps(SSEResultTwo, SSEResultOne));
	
	// SSEResultOne = _mm_xor_ps(_mm_shuffle_ps(left.SSE, left.SSE, _MM_SHUFFLE(2, 2, 2, 2)), _mm_setr_ps(-0.f, 0.f, 0.f, -0.f));
	// SSEResultTwo = _mm_shuffle_ps(right.SSE, right.SSE, _MM_SHUFFLE(2, 3, 0, 1));
	// SSEResultThree = _mm_add_ps(SSEResultThree, _mm_mul_ps(SSEResultTwo, SSEResultOne));
	
	// SSEResultOne = _mm_shuffle_ps(left.SSE, left.SSE, _MM_SHUFFLE(3, 3, 3, 3));
	// SSEResultTwo = _mm_shuffle_ps(right.SSE, right.SSE, _MM_SHUFFLE(3, 2, 1, 0));
	// result.SSE = _mm_add_ps(SSEResultThree, _mm_mul_ps(SSEResultTwo, SSEResultOne));
	// #else
	result.x =  right.Elements[3] * +left.Elements[0];
	result.y =  right.Elements[2] * -left.Elements[0];
	result.z =  right.Elements[1] * +left.Elements[0];
	result.w =  right.Elements[0] * -left.Elements[0];
	
	result.x += right.Elements[2] * +left.Elements[1];
	result.y += right.Elements[3] * +left.Elements[1];
	result.z += right.Elements[0] * -left.Elements[1];
	result.w += right.Elements[1] * -left.Elements[1];
	
	result.x += right.Elements[1] * -left.Elements[2];
	result.y += right.Elements[0] * +left.Elements[2];
	result.z += right.Elements[3] * +left.Elements[2];
	result.w += right.Elements[2] * -left.Elements[2];
	
	result.x += right.Elements[0] * +left.Elements[3];
	result.y += right.Elements[1] * +left.Elements[3];
	result.z += right.Elements[2] * +left.Elements[3];
	result.w += right.Elements[3] * +left.Elements[3];
	// #endif
	return result;
}

PEXPI quat HMM_MulQF(quat left, float Multiplicative)
{
	quat result;
	// #ifdef HANDMADE_MATH__USE_SSE
	// __m128 Scalar = _mm_set1_ps(Multiplicative);
	// result.SSE = _mm_mul_ps(left.SSE, Scalar);
	// #else
	result.x = left.x * Multiplicative;
	result.y = left.y * Multiplicative;
	result.z = left.z * Multiplicative;
	result.w = left.w * Multiplicative;
	// #endif
	return result;
}

PEXPI quat HMM_DivQF(quat left, float Divnd)
{
    quat result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 Scalar = _mm_set1_ps(Divnd);
    result.SSE = _mm_div_ps(left.SSE, Scalar);
#else
    result.x = left.x / Divnd;
    result.y = left.y / Divnd;
    result.z = left.z / Divnd;
    result.w = left.w / Divnd;
#endif

    return result;
}

//TODO: #define AddQuat(left, right) HMM_AddQ((left), (right))
//TODO: #define SubQuat(left, right) HMM_SubQ((left), (right))
//TODO: #define MulQuat(left, right) HMM_MulQ((left), (right))
//TODO: #define ScaleQuat(quaternion, scalar) HMM_MulQF((quaternion), (scalar))
//TODO: #define ShrinkQuat(quaternion, divisor) HMM_DivQF((quaternion), (divisor))

//TODO: #define DotQuat(left, right) HMM_DotQ((left), (right))

//TODO: #define InvertQuat(quaternion) HMM_InvQ(quaternion)

//TODO: #define NormalizeQuat(quaternion) HMM_NormQ(quaternion)

//TODO: #define LerpQuat(start, end, amount) HMM_NLerp((start), (amount), (end))
//TODO: #define SlerpQuat(start, end, amount) HMM_SLerp((start), (amount), (end))

PEXPI quatd ToQuatdFromAxis(v3d axis, r64 angle)
{
	quatd result;
	v3d axisNormalized = NormalizeV3d(axis);
	r64 sineOfRotation = SinR64(angle / 2.0);
	result.xYZ = ScaleV3d(axisNormalized, sineOfRotation);
	result.w = CosR64(angle / 2.0);
	return result;
}

PEXPI quatd AddQuatd(quatd left, quatd right) { quatd result; result.x = left.x + right.x; result.y = left.y + right.y; result.z = left.z + right.z; result.w = left.w + right.w; return result; }
PEXPI quatd SubQuatd(quatd left, quatd right) { quatd result; result.x = left.x - right.x; result.y = left.y - right.y; result.z = left.z - right.z; result.w = left.w - right.w; return result; }

PEXPI quatd MulQuatd(quatd left, quatd right)
{
	quatd result;
	
	result.x =  right.w * +left.x;
	result.y =  right.z * -left.x;
	result.z =  right.y * +left.x;
	result.w =  right.x * -left.x;
	
	result.x += right.z * +left.y;
	result.y += right.w * +left.y;
	result.z += right.x * -left.y;
	result.w += right.y * -left.y;
	
	result.x += right.y * -left.z;
	result.y += right.x * +left.z;
	result.z += right.w * +left.z;
	result.w += right.z * -left.z;
	
	result.x += right.x * +left.w;
	result.y += right.y * +left.w;
	result.z += right.z * +left.w;
	result.w += right.w * +left.w;
	
	return result;
}

PEXPI quatd ScaleQuatd(quatd quaternion, r64 scalar)
{
	quatd result;
	result.x = quaternion.x * scalar;
	result.y = quaternion.y * scalar;
	result.z = quaternion.z * scalar;
	result.w = quaternion.w * scalar;
	return result;
}

PEXPI quatd ShrinkQuatd(quatd quaternion, r64 divisor)
{
	quatd result;
	result.x = quaternion.x / divisor;
	result.y = quaternion.y / divisor;
	result.z = quaternion.z / divisor;
	result.w = quaternion.w / divisor;
	return result;
}

PEXPI r64 DotQuatd(quatd left, quatd right)
{
	//NOTE: We removed "extra" parenthesis around these adds, but technically this might cause slightly more floating point error accumulation
	return (left.x * right.x) + (left.y * right.y) + (left.z * right.z) + (left.w * right.w);
}

PEXPI quatd InvertQuatd(quatd quaternion)
{
	quatd result;
	result.x = -quaternion.x;
	result.y = -quaternion.y;
	result.z = -quaternion.z;
	result.w = quaternion.w;
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
		end = MakeQuatd(-end.x, -end.y, -end.z, -end.w);
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

PEXPI bool AreEqualQuat(quat left, quat right) { return (left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w); }
PEXPI bool AreEqualQuatd(quatd left, quatd right) { return (left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w); }

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
