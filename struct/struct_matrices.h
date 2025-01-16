/*
File:   struct_matrices.h
Author: Taylor Robbins
Date:   01\15\2025
Description:
	** Since we depend on HandmadeMath.h for the all of our matrix math related
	** functions and types, this file serves more as a aliasing file for all the
	** things in HandmadeMath.h, with some extensions for functions that we had
	** in GyLib that HandmadeMath.h does not have.
*/

#ifndef _STRUCT_MATRICES_H
#define _STRUCT_MATRICES_H

#include "base/base_typedefs.h"
#include "std/std_includes.h"
#include "struct/struct_vectors.h"

#include "struct/struct_handmade_math_include.h"

// +--------------------------------------------------------------+
// |                           Typedefs                           |
// +--------------------------------------------------------------+
typedef HMM_Mat2 mat2;
typedef HMM_Mat3 mat3;
typedef HMM_Mat4 mat4;

// +--------------------------------------------------------------+
// |                        New Functions                         |
// +--------------------------------------------------------------+
static inline mat2 NewMat2(
	r32 r0c0, r32 r0c1,
	r32 r1c0, r32 r1c1)
{
	mat2 result;
	result.Elements[0][0] = r0c0; result.Elements[1][0] = r0c1;
	result.Elements[0][1] = r1c0; result.Elements[1][1] = r1c1;
	return result;
}
static inline mat3 NewMat3(
	r32 r0c0, r32 r0c1, r32 r0c2,
	r32 r1c0, r32 r1c1, r32 r1c2,
	r32 r2c0, r32 r2c1, r32 r2c2)
{
	mat3 result;
	result.Elements[0][0] = r0c0; result.Elements[1][0] = r0c1; result.Elements[2][0] = r0c2;
	result.Elements[0][1] = r1c0; result.Elements[1][1] = r1c1; result.Elements[2][1] = r1c2;
	result.Elements[0][2] = r2c0; result.Elements[1][2] = r2c1; result.Elements[2][2] = r2c2;
	return result;
}
static inline mat4 NewMat4(
	r32 r0c0, r32 r0c1, r32 r0c2, r32 r0c3,
	r32 r1c0, r32 r1c1, r32 r1c2, r32 r1c3,
	r32 r2c0, r32 r2c1, r32 r2c2, r32 r2c3,
	r32 r3c0, r32 r3c1, r32 r3c2, r32 r3c3)
{
	mat4 result;
	result.Elements[0][0] = r0c0; result.Elements[1][0] = r0c1; result.Elements[2][0] = r0c2; result.Elements[3][0] = r0c3;
	result.Elements[0][1] = r1c0; result.Elements[1][1] = r1c1; result.Elements[2][1] = r1c2; result.Elements[3][1] = r1c3;
	result.Elements[0][2] = r2c0; result.Elements[1][2] = r2c1; result.Elements[2][2] = r2c2; result.Elements[3][2] = r2c3;
	result.Elements[0][3] = r3c0; result.Elements[1][3] = r3c1; result.Elements[2][3] = r3c2; result.Elements[3][3] = r3c3;
	return result;
}

#define FillMat2(value) NewMat2((value), (value), (value), (value))
#define FillMat3(value) NewMat3((value), (value), (value), (value), (value), (value), (value), (value), (value))
#define FillMat4(value) NewMat4((value), (value), (value), (value), (value), (value), (value), (value), (value), (value), (value), (value), (value), (value), (value), (value))

#define FillDiagonalMat2(value) HMM_M2D(value)
#define FillDiagonalMat3(value) HMM_M3D(value)
#define FillDiagonalMat4(value) HMM_M4D(value)

// +--------------------------------------------------------------+
// |                   Simple Value Definitions                   |
// +--------------------------------------------------------------+
#define Mat2_Zero FillMat2(0.0f)
#define Mat3_Zero FillMat3(0.0f)
#define Mat4_Zero FillMat4(0.0f)

#define Mat2_Identity FillDiagonalMat2(1.0f)
#define Mat3_Identity FillDiagonalMat3(1.0f)
#define Mat4_Identity FillDiagonalMat4(1.0f)

//TODO: Would we ever use MatN_Zero_Const and MatN_Identity_Const?

// +--------------------------------------------------------------+
// |                Operator Overload Equivalents                 |
// +--------------------------------------------------------------+
#define AddMat2(left, right) HMM_AddM2((left), (right))
#define AddMat3(left, right) HMM_AddM3((left), (right))
#define AddMat4(left, right) HMM_AddM4((left), (right))

#define SubMat2(left, right) HMM_SubM2((left), (right))
#define SubMat3(left, right) HMM_SubM3((left), (right))
#define SubMat4(left, right) HMM_SubM4((left), (right))

#define MulMat2(left, right) HMM_MulM2((left), (right))
#define MulMat3(left, right) HMM_MulM3((left), (right))
#define MulMat4(left, right) HMM_MulM4((left), (right))

#define ScaleMat2(matrix2, scalar) HMM_MulM2F((matrix2), (scalar))
#define ScaleMat3(matrix3, scalar) HMM_MulM3F((matrix3), (scalar))
#define ScaleMat4(matrix4, scalar) HMM_MulM4F((matrix4), (scalar))

#define ShrinkMat2(matrix2, scalar) HMM_DivM2F((matrix2), (scalar))
#define ShrinkMat3(matrix3, scalar) HMM_DivM3F((matrix3), (scalar))
#define ShrinkMat4(matrix4, scalar) HMM_DivM4F((matrix4), (scalar))

#define MulMat2AndV2(matrix2, vec2) HMM_MulM2V2((matrix2), (vec2))
#define MulMat3AndV3(matrix3, vec3) HMM_MulM3V3((matrix3), (vec3))
#define MulMat4AndV4(matrix4, vec4) HMM_MulM4V4((matrix4), (vec4))

//TODO: Do we really need this variant where we pass out the W value?
static inline v3 MulMat4AndV3GetW(mat4 matrix4, v3 vec3, bool includeTranslation, r32* wOut)
{
	v4 vec4 = ToV4From3(vec3, includeTranslation ? 1.0f : 0.0f);
	vec4 = MulMat4AndV4(matrix4, vec4);
	SetOptionalOutPntr(wOut, vec4.W);
	//TODO: This ternary operator makes me think we're doing something wrong with the divide by W thing.
	//      We should really look into how 3D points and vectors are transformed by 4x4 matrix transformations
	return (vec4.W != 0) ? ShrinkV4(vec4, vec4.W).XYZ : vec4.XYZ;
}
static inline v3 MulMat4AndV3(mat4 matrix4, v3 vec3, bool includeTranslation) { return MulMat4AndV3GetW(matrix4, vec3, includeTranslation, nullptr); }

//a.k.a. Apply a new transform matrix from the LEFT side
#define TransformMat4(mat4Pntr, matrix4) *(mat4Pntr) = MulMat4((matrix4), *(mat4Pntr))

// +--------------------------------------------------------------+
// |                      Common Matrix Math                      |
// +--------------------------------------------------------------+
#define TransposeMat2(matrix2) HMM_TransposeM2(matrix2)
#define TransposeMat3(matrix3) HMM_TransposeM3(matrix3)
#define TransposeMat4(matrix4) HMM_TransposeM4(matrix4)

#define DeterminantMat2(matrix2) HMM_DeterminantM2(matrix2)
#define DeterminantMat3(matrix3) HMM_DeterminantM3(matrix3)
#define DeterminantMat4(matrix4) HMM_DeterminantM4(matrix4)

//TODO: CofactorM4?
//TODO: AdjointM4?

#define InverseM2(matrix2) HMM_InvGeneralM2(matrix2)
#define InverseM3(matrix3) HMM_InvGeneralM3(matrix3)
#define InverseM4(matrix4) HMM_InvGeneralM4(matrix4)

//TODO: Not entirely sure what this is useful for?
#define LinearCombineV4Mat4(vec4Left, matrix4Right) HMM_LinearCombineV4M4((vec4Left), (matrix4Right))

// +--------------------------------------------------------------+
// |               Basic Matrix Creation Functions                |
// +--------------------------------------------------------------+
static inline mat3 MakeTranslateMat3(v2 vec2)
{
	return NewMat3(
		1.0f, 0.0f, vec2.X,
		0.0f, 1.0f, vec2.Y,
		0.0f, 0.0f, 1.0f
	);
}
#define MakeTranslateXMat3(x) MakeTranslateMat3(NewV2((x), 0.0f))
#define MakeTranslateYMat3(y) MakeTranslateMat3(NewV2(0.0f, (y)))

#define MakeTranslateMat4(vec3) HMM_Translate(vec3)
#define MakeTranslateXYZMat4(x, y, z) HMM_Translate(NewV3((x), (y), (z)))
#define MakeTranslateXMat4(x) HMM_Translate(NewV3((x), 0.0f, 0.0f))
#define MakeTranslateYMat4(y) HMM_Translate(NewV3(0.0f, (y), 0.0f))
#define MakeTranslateZMat4(z) HMM_Translate(NewV3(0.0f, 0.0f, (z)))

static inline mat3 MakeScaleMat3(v3 vec3)
{
	return NewMat3(
		vec3.X, 0.0f, 0.0f,
		0.0f, vec3.Y, 0.0f,
		0.0f, 0.0f, vec3.Z
	);
}
#define Make2DScaleMat3(vec2) MakeScaleMat3(ToV3From2((vec2), 1.0f))
#define MakeScaleXYZMat3(x, y, z) MakeScaleMat3(NewV3((x), (y), (z)))
#define MakeScaleXMat3(x) MakeScaleMat3(NewV3((x), 1.0f, 1.0f))
#define MakeScaleYMat3(y) MakeScaleMat3(NewV3(1.0f, (y), 1.0f))
#define MakeScaleZMat3(z) MakeScaleMat3(NewV3(1.0f, 1.0f, (z)))

#define MakeScaleMat4(vec3) HMM_Scale(vec3)
#define Make2DScaleMat4(vec2) HMM_Scale(ToV3From2((vec2), 1.0f))
#define MakeScaleXYZMat4(x, y, z) HMM_Scale(NewV3((x), (y), (z)))
#define MakeScaleXMat4(x) HMM_Scale(NewV3((x), 1.0f, 1.0f))
#define MakeScaleYMat4(y) HMM_Scale(NewV3(1.0f, (y), 1.0f))
#define MakeScaleZMat4(z) HMM_Scale(NewV3(1.0f, 1.0f, (z)))
// TODO: MakeScaleWMat4(w)?

//TODO: MakeRotateMat3(angle)? (for 2D rotations?)

#define MakeRotateMat4(angle, axisVec3) HMM_Rotate_LH((angle), (axisVec3))
#define MakeRotateXMat4(angle) HMM_Rotate_LH((angle), V3_Right)
#define MakeRotateYMat4(angle) HMM_Rotate_LH((angle), V3_Up)
#define MakeRotateZMat4(angle) HMM_Rotate_LH((angle), V3_Forward)

// +--------------------------------------------------------------+
// |              Complex Matrix Creation Functions               |
// +--------------------------------------------------------------+
//NOTE: When working with OpenGL  use the Gl variant (implying z range [-1, 1])
//      When working with DirectX use the Dx variant (implying z range [0, 1])

#define MakeOrthographicMat4Gl(left, right, top, bottom, zNear, zFar) HMM_Orthographic_LH_NO((left), (right), (bottom), (top), (zNear), (zFar))
#define MakeOrthographicMat4Dx(left, right, top, bottom, zNear, zFar) HMM_Orthographic_LH_ZO((left), (right), (bottom), (top), (zNear), (zFar))

#define MakePerspectiveMat4Gl(fov, aspectRatio, zNear, zFar) HMM_Perspective_LH_NO((fov), (aspectRatio), (zNear), (zFar))
#define MakePerspectiveMat4Dx(fov, aspectRatio, zNear, zFar) HMM_Perspective_LH_ZO((fov), (aspectRatio), (zNear), (zFar))

#define MakeLookAtMat4(eye, center, up) HMM_LookAt_LH((eye), (center), (up))

// +--------------------------------------------------------------+
// |                   Invert Matrix Functions                    |
// +--------------------------------------------------------------+
//TODO: InvertTranslateMat3?
//TODO: InvertRotateMat3?
//TODO: InvertScaleMat3?
#define InvertTranslateMat4(matrix4) HMM_InvTranslate(matrix4)
#define InvertRotateMat4(matrix4) HMM_InvRotate(matrix4)
#define InvertScaleMat4(matrix4) HMM_InvScale(matrix4)
#define InvertOrthographicMat4(matrix4) HMM_InvOrthographic(matrix4)
#define InvertPerspectiveMat4(matrix4) HMM_InvPerspective_LH(matrix4)
#define InvertLookAtMat4(matrix4) HMM_InvLookAt(matrix4)

#endif //  _STRUCT_MATRICES_H

#if defined(_STRUCT_QUATERNION_H) && defined(_STRUCT_MATRICES_H)
#include "cross/cross_quaternion_and_matrices.h"
#endif

#if defined(_STRUCT_VECTORS_H) && defined(_STRUCT_QUATERNION_H) && defined(_STRUCT_MATRICES_H)
#include "cross/cross_vectors_quaternion_and_matrices.h"
#endif
