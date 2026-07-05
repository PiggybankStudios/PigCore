/*
File:   struct_matrices.h
Author: Taylor Robbins
Date:   01\15\2025
Description:
	** This file holds 2x2, 3x3, and 4x4 matrix types and functions for constructing
	** and manipulating them in common ways. The full name for these types are
	** Matrix2x2_R32, Matrix3x3_R32, and Matrix4x4_R32 but there are aliases
	** mat2, mat3, and mat4 for convenience.
	**
	** This file originally was based on HandmadeMath.h so many of the function
	** implementations are directly copied from there.
	**
	** Matrices are column-major ordering in memory. So that's r0c0, r1c0, etc. and then r0c1, r1c1, etc.
	** When accessing these using elem 2D array, first index is column, second index is row
	**    matrix.elem[Column][Row] = ...
*/

#ifndef _STRUCT_MATRICES_H
#define _STRUCT_MATRICES_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "std/std_includes.h"
#include "struct/struct_vectors.h"

typedef car Matrix2x2_R32 Matrix2x2_R32;
car Matrix2x2_R32
{
	r32 elem[2][2];
	Vector2_R32 columns[2];
	plex { r32 r0c0, r1c0, r0c1, r1c1; };
	#if LANGUAGE_IS_CPP
	inline Vector2_R32& operator[](const int& elemIndex) { return columns[elemIndex]; }
	#endif
};
#define MakeMat2_Const(R0C0, R0C1, R1C0, R1C1) { \
	.r0c0=(R0C0), .r0c1=(R0C1),                  \
	.r1c0=(R1C0), .r1c1=(R1C1)                   \
}
#define MakeMat2(r0c0, r0c1, r1c0, r1c1) NEW_STRUCT(Matrix2x2_R32)MakeMat2_Const( \
	(r0c0), (r0c1),                                                               \
	(r1c0), (r1c1)                                                                \
)
#define FillMat2_Const(value) MakeMat2_Const( \
	(value), (value),                         \
	(value), (value)                          \
)
#define FillMat2(value) NEW_STRUCT(Matrix2x2_R32)FillMat2_Const(value)
#define FillDiagonalMat2_Const(value) MakeMat2_Const( \
	(value), 0.0f,                                    \
	0.0f, (value)                                     \
)
#define FillDiagonalMat2(value) NEW_STRUCT(mat2)FillDiagonalMat2_Const(value)

typedef car Matrix3x3_R32 Matrix3x3_R32;
car Matrix3x3_R32
{
	r32 elem[3][3];
	Vector3_R32 columns[3];
	plex { r32 r0c0, r1c0, r2c0, r0c1, r1c1, r2c1, r0c2, r1c2, r2c2; };
	#if LANGUAGE_IS_CPP
	inline Vector3_R32& operator[](const int& elemIndex) { return columns[elemIndex]; }
	#endif
};
#define MakeMat3_Const(R0C0, R0C1, R0C2, R1C0, R1C1, R1C2, R2C0, R2C1, R2C2) { \
	.r0c0=R0C0, .r1c0=R1C0, .r2c0=R2C0,                                        \
	.r0c1=R0C1, .r1c1=R1C1, .r2c1=R2C1,                                        \
	.r0c2=R0C2, .r1c2=R1C2, .r2c2=R2C2                                         \
}
#define MakeMat3(r0c0, r0c1, r0c2, r1c0, r1c1, r1c2, r2c0, r2c1, r2c2) NEW_STRUCT(Matrix3x3_R32)MakeMat3_Const( \
	(r0c0), (r0c1), (r0c2),                                                                                     \
	(r1c0), (r1c1), (r1c2),                                                                                     \
	(r2c0), (r2c1), (r2c2)                                                                                      \
)
#define FillMat3_Const(value) MakeMat3_Const( \
	(value), (value), (value),                \
	(value), (value), (value),                \
	(value), (value), (value)                 \
)
#define FillMat3(value) NEW_STRUCT(Matrix3x3_R32)FillMat3_Const(value)
#define FillDiagonalMat3_Const(value) MakeMat3_Const( \
	(value), 0.0f, 0.0f,                              \
	0.0f, (value), 0.0f,                              \
	0.0f, 0.0f, (value)                               \
)
#define FillDiagonalMat3(value) NEW_STRUCT(Matrix3x3_R32)FillDiagonalMat3_Const(value)

typedef car Matrix4x4_R32 Matrix4x4_R32;
car Matrix4x4_R32
{
	r32 elem[4][4];
	Vector4_R32 columns[4];
	plex { r32 r0c0, r1c0, r2c0, r3c0, r0c1, r1c1, r2c1, r3c1, r0c2, r1c2, r2c2, r3c2, r0c3, r1c3, r2c3, r3c3; };
	#if LANGUAGE_IS_CPP
	inline Vector4_R32& operator[](const int& elemIndex) { return columns[elemIndex]; }
	#endif
};
#define MakeMat4_Const(R0C0, R0C1, R0C2, R0C3, R1C0, R1C1, R1C2, R1C3, R2C0, R2C1, R2C2, R2C3, R3C0, R3C1, R3C2, R3C3) { \
	.r0c0=R0C0, .r1c0=R1C0, .r2c0=R2C0, .r3c0=R3C0,                                                                      \
	.r0c1=R0C1, .r1c1=R1C1, .r2c1=R2C1, .r3c1=R3C1,                                                                      \
	.r0c2=R0C2, .r1c2=R1C2, .r2c2=R2C2, .r3c2=R3C2,                                                                      \
	.r0c3=R0C3, .r1c3=R1C3, .r2c3=R2C3, .r3c3=R3C3                                                                       \
}
#define MakeMat4(r0c0, r0c1, r0c2, r0c3, r1c0, r1c1, r1c2, r1c3, r2c0, r2c1, r2c2, r2c3, r3c0, r3c1, r3c2, r3c3) NEW_STRUCT(Matrix4x4_R32)MakeMat4_Const( \
	(r0c0), (r0c1), (r0c2), (r0c3),                                                                                                                       \
	(r1c0), (r1c1), (r1c2), (r1c3),                                                                                                                       \
	(r2c0), (r2c1), (r2c2), (r2c3),                                                                                                                       \
	(r3c0), (r3c1), (r3c2), (r3c3)                                                                                                                        \
)
#define FillMat4_Const(value) MakeMat4_Const( \
	(value), (value), (value), (value),       \
	(value), (value), (value), (value),       \
	(value), (value), (value), (value),       \
	(value), (value), (value), (value)        \
)
#define FillMat4(value) NEW_STRUCT(Matrix4x4_R32)FillMat4_Const(value)
#define FillDiagonalMat4_Const(value) MakeMat4_Const( \
	(value), 0.0f, 0.0f, 0.0f,                        \
	0.0f, (value), 0.0f, 0.0f,                        \
	0.0f, 0.0f, (value), 0.0f,                        \
	0.0f, 0.0f, 0.0f, (value)                         \
)
#define FillDiagonalMat4(value) NEW_STRUCT(Matrix4x4_R32)FillDiagonalMat4_Const(value)

typedef Matrix2x2_R32 mat2;
typedef Matrix3x3_R32 mat3;
typedef Matrix4x4_R32 mat4;

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE mat3 ToMat3From4(mat4 matrix4);
	PIG_CORE_INLINE mat4 ToMat4From3(mat3 matrix3);
	PIG_CORE_INLINE bool AreEqualMat2(mat2 left, mat2 right);
	PIG_CORE_INLINE bool AreEqualMat3(mat3 left, mat3 right);
	PIG_CORE_INLINE bool AreEqualMat4(mat4 left, mat4 right);
	PIG_CORE_INLINE mat2 TransposeMat2(mat2 matrix);
	PIG_CORE_INLINE mat2 AddMat2(mat2 left, mat2 right);
	PIG_CORE_INLINE mat2 SubMat2(mat2 left, mat2 right);
	PIG_CORE_INLINE v2 MulMat2AndV2(mat2 matrix, v2 vector);
	PIG_CORE_INLINE mat2 MulMat2(mat2 left, mat2 right);
	PIG_CORE_INLINE mat2 ScaleMat2(mat2 matrix, r32 scalar);
	PIG_CORE_INLINE mat2 ShrinkMat2(mat2 matrix, r32 scalar);
	PIG_CORE_INLINE r32 DeterminantMat2(mat2 matrix);
	PIG_CORE_INLINE mat2 InverseMat2(mat2 matrix);
	PIG_CORE_INLINE mat3 TransposeMat3(mat3 matrix);
	PIG_CORE_INLINE mat3 AddMat3(mat3 left, mat3 right);
	PIG_CORE_INLINE mat3 SubMat3(mat3 left, mat3 right);
	PIG_CORE_INLINE v3 MulMat3AndV3(mat3 matrix, v3 vector);
	PIG_CORE_INLINE mat3 MulMat3(mat3 left, mat3 right);
	PIG_CORE_INLINE mat3 ScaleMat3(mat3 matrix, r32 scalar);
	PIG_CORE_INLINE mat3 ShrinkMat3(mat3 matrix, r32 scalar);
	PIG_CORE_INLINE r32 DeterminantMat3(mat3 matrix) ;
	PIG_CORE_INLINE mat3 InverseMat3(mat3 matrix) ;
	PIG_CORE_INLINE mat4 TransposeMat4(mat4 matrix);
	PIG_CORE_INLINE mat4 AddMat4(mat4 left, mat4 right);
	PIG_CORE_INLINE mat4 SubMat4(mat4 left, mat4 right);
	PIG_CORE_INLINE v4 MulMat4AndV4(mat4 matrix, v4 vector);
	PIG_CORE_INLINE mat4 MulMat4(mat4 left, mat4 right);
	PIG_CORE_INLINE mat4 ScaleMat4(mat4 matrix, r32 scalar);
	PIG_CORE_INLINE mat4 ShrinkMat4(mat4 matrix, r32 scalar);
	PIG_CORE_INLINE r32 DeterminantMat4(mat4 matrix) ;
	PIG_CORE_INLINE mat4 InverseMat4(mat4 matrix) ;
	PIG_CORE_INLINE v3 MulMat4AndV3GetW(mat4 matrix4, v3 vec3, bool includeTranslation, r32* wOut);
	PIG_CORE_INLINE v3 MulMat4AndV3(mat4 matrix4, v3 vec3, bool includeTranslation);
	PIG_CORE_INLINE mat4 MakeRotateMat4_RH(r32 angle, v3 axis);
	PIG_CORE_INLINE mat4 MakeOrthographicMat4Gl_RH(r32 left, r32 right, r32 bottom, r32 top, r32 zNear, r32 zFar);
	PIG_CORE_INLINE mat4 MakeOrthographicMat4Dx_RH(r32 left, r32 right, r32 bottom, r32 top, r32 zNear, r32 zFar);
	PIG_CORE_INLINE mat4 MakeOrthographicMat4Gl_LH(r32 left, r32 right, r32 bottom, r32 top, r32 zNear, r32 zFar);
	PIG_CORE_INLINE mat4 MakeOrthographicMat4Dx_LH(r32 left, r32 right, r32 bottom, r32 top, r32 zNear, r32 zFar);
	PIG_CORE_INLINE mat4 MakePerspectiveMat4Gl_RH(r32 fov, r32 aspectRatio, r32 zNear, r32 zFar);
	PIG_CORE_INLINE mat4 MakePerspectiveMat4Dx_RH(r32 fov, r32 aspectRatio, r32 zNear, r32 zFar);
	PIG_CORE_INLINE mat4 MakePerspectiveMat4Gl_LH(r32 fov, r32 aspectRatio, r32 zNear, r32 zFar);
	PIG_CORE_INLINE mat4 MakePerspectiveMat4Dx_LH(r32 fov, r32 aspectRatio, r32 zNear, r32 zFar);
	PIG_CORE_INLINE mat4 MakeLookAtMat4_RH(v3 eyePos, v3 targetPos, v3 upVec);
	PIG_CORE_INLINE mat4 MakeLookAtMat4_LH(v3 eyePos, v3 targetPos, v3 upVec);
	#if LANGUAGE_IS_CPP
	PIG_CORE_INLINE mat2 Add(mat2 left, mat2 right);
	PIG_CORE_INLINE mat3 Add(mat3 left, mat3 right);
	PIG_CORE_INLINE mat4 Add(mat4 left, mat4 right);
	PIG_CORE_INLINE mat2 Sub(mat2 left, mat2 right);
	PIG_CORE_INLINE mat3 Sub(mat3 left, mat3 right);
	PIG_CORE_INLINE mat4 Sub(mat4 left, mat4 right);
	PIG_CORE_INLINE mat2 Mul(mat2 left, mat2 right);
	PIG_CORE_INLINE mat3 Mul(mat3 left, mat3 right);
	PIG_CORE_INLINE mat4 Mul(mat4 left, mat4 right);
	PIG_CORE_INLINE mat2 Mul(mat2 matrix, r32 scalar);
	PIG_CORE_INLINE mat3 Mul(mat3 matrix, r32 scalar);
	PIG_CORE_INLINE mat4 Mul(mat4 matrix, r32 scalar);
	PIG_CORE_INLINE mat2 Div(mat2 matrix, r32 scalar);
	PIG_CORE_INLINE mat3 Div(mat3 matrix, r32 scalar);
	PIG_CORE_INLINE mat4 Div(mat4 matrix, r32 scalar);
	PIG_CORE_INLINE bool AreEqual(mat2 left, mat2 right);
	PIG_CORE_INLINE bool AreEqual(mat3 left, mat3 right);
	PIG_CORE_INLINE bool AreEqual(mat4 left, mat4 right);
	#endif //LANGUAGE_IS_CPP
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define Mat2_Zero_Const     FillMat2_Const(0.0f)
#define Mat2_Identity_Const FillDiagonalMat2_Const(1.0f)
#define Mat3_Zero_Const     FillMat3_Const(0.0f)
#define Mat3_Identity_Const FillDiagonalMat3_Const(1.0f)
#define Mat4_Zero_Const     FillMat4_Const(0.0f)
#define Mat4_Identity_Const FillDiagonalMat4_Const(1.0f)

#define Mat2_Zero     FillMat2(0.0f)
#define Mat2_Identity FillDiagonalMat2(1.0f)
#define Mat3_Zero     FillMat3(0.0f)
#define Mat3_Identity FillDiagonalMat3(1.0f)
#define Mat4_Zero     FillMat4(0.0f)
#define Mat4_Identity FillDiagonalMat4(1.0f)

//a.k.a. Apply a new transform matrix from the LEFT side
#define TransformMat4(mat4Pntr, matrix4) *(mat4Pntr) = MulMat4((matrix4), *(mat4Pntr))

//TODO: CofactorM4?
//TODO: AdjointM4?

#define MakeTranslateXYMat3_Const(x, y) MakeMat3_Const( \
	1.0f, 0.0f, (x),                                    \
	0.0f, 1.0f, (y),                                    \
	0.0f, 0.0f, 1.0f                                    \
)
#define MakeTranslateMat3_Const(translationVec) MakeTranslateXYZMat3_Const((translationVec).x, (translationVec).y)
#define MakeTranslateXYMat3(x, y)               NEW_STRUCT(Matrix3x3_R32)MakeTranslateXYMat3_Const((x), (y))
#define MakeTranslateMat3(translationVec)       NEW_STRUCT(Matrix3x3_R32)MakeTranslateMat3_Const(translationVec)
#define MakeTranslateXMat3(x)                   MakeTranslateXYMat3((x), 0.0f)
#define MakeTranslateYMat3(y)                   MakeTranslateXYMat3(0.0f, (y))

#define MakeScaleXYZMat3_Const(x, y, z) MakeMat3_Const( \
	 (x), 0.0f, 0.0f,                                   \
	0.0f,  (y), 0.0f,                                   \
	0.0f, 0.0f,  (z)                                    \
)
#define MakeScaleXYMat3_Const(x, y)       MakeScaleXYZMat3_Const((x), (y), 1.0f)
#define Make3DScaleMat3_Const(scaleVec)   MakeScaleXYZMat3_Const((scaleVec).x, (scaleVec).y, (scaleVec).z)
#define Make2DScaleMat3_Const(scaleVec)   MakeScaleXYMat3_Const((scaleVec).x, (scaleVec).y)
#define MakeScaleXYZMat3(x, y, z)         NEW_STRUCT(Matrix3x3_R32)MakeScaleXYZMat3_Const((x), (y), (z))
#define MakeScaleXYMat3(x, y)             NEW_STRUCT(Matrix3x3_R32)MakeScaleXYMat3_Const((x), (y))
#define Make3DScaleMat3(scaleVec)         NEW_STRUCT(Matrix3x3_R32)MakeScaleXYZMat3_Const((scaleVec).x, (scaleVec).y, (scaleVec).z)
#define Make2DScaleMat3(scaleVec)         NEW_STRUCT(Matrix3x3_R32)MakeScaleXYMat3_Const((scaleVec).x, (scaleVec).y)
#define MakeScaleXMat3(x)                 MakeScaleXYZMat3((x), 1.0f, 1.0f)
#define MakeScaleYMat3(y)                 MakeScaleXYZMat3(1.0f, (y), 1.0f)
#define MakeScaleZMat3(z)                 MakeScaleXYZMat3(1.0f, 1.0f, (z))

//TODO: MakeRotateMat3(angle)? (for 2D rotations?)

#define MakeScaleXYZMat4_Const(x, y, z) MakeMat4_Const( \
	 (x), 0.0f, 0.0f, 0.0f,                             \
	0.0f,  (y), 0.0f, 0.0f,                             \
	0.0f, 0.0f,  (z), 0.0f,                             \
	0.0f, 0.0f, 0.0f, 1.0f                              \
)
#define MakeScaleMat4_Const(scaleVec) MakeScaleXYZMat4_Const((scaleVec).x, (scaleVec).y, (scaleVec).z)
#define MakeScaleXYZMat4(x, y, z)     NEW_STRUCT(Matrix4x4_R32)MakeScaleXYZMat4_Const((x), (y), (z))
#define MakeScaleMat4(scaleVec)       NEW_STRUCT(Matrix4x4_R32)MakeScaleXYZMat4_Const((scaleVec).x, (scaleVec).y, (scaleVec).z)
#define MakeScaleXYMat4(x, y)         MakeScaleXYZMat4((x), (y), 0.0f)
#define Make2DScaleMat4(scaleVec)     MakeScaleXYMat4((scaleVec).x, (scaleVec).y)
#define MakeScaleXMat4(x)             MakeScaleXYZMat4((x),  1.0f, 1.0f)
#define MakeScaleYMat4(y)             MakeScaleXYZMat4(1.0f,  (y), 1.0f)
#define MakeScaleZMat4(z)             MakeScaleXYZMat4(1.0f, 1.0f,  (z))

#define MakeTranslateXYZMat4_Const(x, y, z) MakeMat4_Const( \
	1.0f, 0.0f, 0.0f, (x),                                  \
	0.0f, 1.0f, 0.0f, (y),                                  \
	0.0f, 0.0f, 1.0f, (z),                                  \
	0.0f, 0.0f, 0.0f, 1.0f                                  \
)
#define MakeTranslateMat4_Const(translationVec) MakeTranslateXYZMat4_Const((translationVec).x, (translationVec).y, (translationVec).z)
#define MakeTranslateXYZMat4(x, y, z)           NEW_STRUCT(Matrix4x4_R32)MakeTranslateXYZMat4_Const((x), (y), (z))
#define MakeTranslateMat4(translationVec)       NEW_STRUCT(Matrix4x4_R32)MakeTranslateMat4_Const(translationVec)
#define MakeTranslateXMat4(x)                   MakeTranslateXYZMat4((x), 0.0f, 0.0f)
#define MakeTranslateYMat4(y)                   MakeTranslateXYZMat4(0.0f, (y), 0.0f)
#define MakeTranslateZMat4(z)                   MakeTranslateXYZMat4(0.0f, 0.0f, (z))

//NOTE: We assume Left-Handed 3D vector space when no suffix is given.
//      Use _LH and _RH variants to be specific about the handedness of your space.
#define MakeRotateMat4_LH(angle, axis) MakeRotateMat4_RH(-(angle), (axis)) //NOTE: Matrix will be inverse/transpose of RH
#define MakeRotateMat4(angle, axis)    MakeRotateMat4_LH((angle), (axis))
#define MakeRotateXMat4(angle)         MakeRotateMat4_LH((angle), V3_Right)
#define MakeRotateYMat4(angle)         MakeRotateMat4_LH((angle), V3_Up)
#define MakeRotateZMat4(angle)         MakeRotateMat4_LH((angle), V3_Forward)
#define MakeRotate2DMat4(angle)        MakeRotateMat4_LH((angle), V3_Backward) //for 2D rotations the axis is -Z (towards the camera)

//NOTE: When working with OpenGL  use the Gl variant (implying z range [-1, 1])
//      When working with DirectX use the Dx variant (implying z range [0, 1])
#define MakeOrthographicMat4Gl(left, right, top, bottom, zNear, zFar) MakeOrthographicMat4Gl_LH((left), (right), (bottom), (top), (zNear), (zFar))
#define MakeOrthographicMat4Dx(left, right, top, bottom, zNear, zFar) MakeOrthographicMat4Dx_LH((left), (right), (bottom), (top), (zNear), (zFar))

#define MakePerspectiveMat4Gl(fov, aspectRatio, zNear, zFar) MakePerspectiveMat4Gl_LH((fov), (aspectRatio), (zNear), (zFar))
#define MakePerspectiveMat4Dx(fov, aspectRatio, zNear, zFar) MakePerspectiveMat4Dx_LH((fov), (aspectRatio), (zNear), (zFar))

#define MakeLookAtMat4(eyePos, targetPos, upVec) MakeLookAtMat4_LH((eyePos), (targetPos), (upVec))

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI mat3 ToMat3From4(mat4 matrix4)
{
	return MakeMat3(
		matrix4.elem[0][0], matrix4.elem[1][0], matrix4.elem[2][0],
		matrix4.elem[0][1], matrix4.elem[1][1], matrix4.elem[2][1],
		matrix4.elem[0][2], matrix4.elem[1][2], matrix4.elem[2][2]
	);
}
PEXPI mat4 ToMat4From3(mat3 matrix3)
{
	return MakeMat4(
		matrix3.elem[0][0], matrix3.elem[1][0], matrix3.elem[2][0], 0,
		matrix3.elem[0][1], matrix3.elem[1][1], matrix3.elem[2][1], 0,
		matrix3.elem[0][2], matrix3.elem[1][2], matrix3.elem[2][2], 0,
		                 0,                  0,                  0, 1
	);
}

#if TARGET_IS_ORCA
//TODO: oc_mat2x3 conversion functions!
#endif //TARGET_IS_ORCA

PEXPI bool AreEqualMat2(mat2 left, mat2 right) { return (AreEqualV2(left.columns[0], right.columns[0]) && AreEqualV2(left.columns[1], right.columns[1])); }
PEXPI bool AreEqualMat3(mat3 left, mat3 right) { return (AreEqualV3(left.columns[0], right.columns[0]) && AreEqualV3(left.columns[1], right.columns[1]) && AreEqualV3(left.columns[2], right.columns[2])); }
PEXPI bool AreEqualMat4(mat4 left, mat4 right) { return (AreEqualV4(left.columns[0], right.columns[0]) && AreEqualV4(left.columns[1], right.columns[1]) && AreEqualV4(left.columns[2], right.columns[2]) && AreEqualV4(left.columns[3], right.columns[3])); }

// +--------------------------------------------------------------+
// |                     Matrix2x2 Functions                      |
// +--------------------------------------------------------------+
PEXPI mat2 TransposeMat2(mat2 matrix)
{
	mat2 result = matrix;
	result.elem[0][1] = matrix.elem[1][0];
	result.elem[1][0] = matrix.elem[0][1];
	return result;
}

PEXPI mat2 AddMat2(mat2 left, mat2 right)
{
	mat2 result;
	result.elem[0][0] = left.elem[0][0] + right.elem[0][0];
	result.elem[0][1] = left.elem[0][1] + right.elem[0][1];
	result.elem[1][0] = left.elem[1][0] + right.elem[1][0];
	result.elem[1][1] = left.elem[1][1] + right.elem[1][1];
	return result;    
}

PEXPI mat2 SubMat2(mat2 left, mat2 right)
{
	mat2 result;
	result.elem[0][0] = left.elem[0][0] - right.elem[0][0];
	result.elem[0][1] = left.elem[0][1] - right.elem[0][1];
	result.elem[1][0] = left.elem[1][0] - right.elem[1][0];
	result.elem[1][1] = left.elem[1][1] - right.elem[1][1];
	return result;
}

PEXPI v2 MulMat2AndV2(mat2 matrix, v2 vector)
{
	v2 result;
	result.x = vector.elem[0] * matrix.columns[0].x;
	result.y = vector.elem[0] * matrix.columns[0].y;
	result.x += vector.elem[1] * matrix.columns[1].x;
	result.y += vector.elem[1] * matrix.columns[1].y;
	return result;    
}

PEXPI mat2 MulMat2(mat2 left, mat2 right)
{
	mat2 result;
	result.columns[0] = MulMat2AndV2(left, right.columns[0]);
	result.columns[1] = MulMat2AndV2(left, right.columns[1]);
	return result;    
}

PEXPI mat2 ScaleMat2(mat2 matrix, r32 scalar)
{
	mat2 result;
	result.elem[0][0] = matrix.elem[0][0] * scalar;
	result.elem[0][1] = matrix.elem[0][1] * scalar;
	result.elem[1][0] = matrix.elem[1][0] * scalar;
	result.elem[1][1] = matrix.elem[1][1] * scalar;
	return result;
}

PEXPI mat2 ShrinkMat2(mat2 matrix, r32 scalar)
{
	mat2 result;
	result.elem[0][0] = matrix.elem[0][0] / scalar;
	result.elem[0][1] = matrix.elem[0][1] / scalar;
	result.elem[1][0] = matrix.elem[1][0] / scalar;
	result.elem[1][1] = matrix.elem[1][1] / scalar;
	return result;
}

PEXPI r32 DeterminantMat2(mat2 matrix)
{
	return (matrix.elem[0][0] * matrix.elem[1][1]) - (matrix.elem[0][1] * matrix.elem[1][0]);
}


PEXPI mat2 InverseMat2(mat2 matrix)
{
	mat2 result;
	r32 invDeterminant = 1.0f / DeterminantMat2(matrix);
	result.elem[0][0] = invDeterminant * +matrix.elem[1][1];
	result.elem[1][1] = invDeterminant * +matrix.elem[0][0];
	result.elem[0][1] = invDeterminant * -matrix.elem[0][1];
	result.elem[1][0] = invDeterminant * -matrix.elem[1][0];
	return result;
}

// +==================================+
// | Matrix2x2 C++ Operator Overloads |
// +==================================+
#if LANGUAGE_IS_CPP
PEXPI mat2 operator+(mat2 left, mat2 right) { return AddMat2(left, right); }
PEXPI mat2 operator-(mat2 left, mat2 right) { return SubMat2(left, right); }
PEXPI mat2 operator*(mat2 left, mat2 right) { return MulMat2(left, right); }
//NOTE: No operator/(mat2, mat2)
PEXPI mat2 operator*(mat2 matrix, r32 scalar) { return ScaleMat2(matrix, scalar); }
PEXPI mat2 operator/(mat2 matrix, r32 scalar) { return ShrinkMat2(matrix, scalar); }
//TODO: Add mat2 * v2 operator?
PEXPI mat2 operator+=(mat2& left, mat2 right) { return left = AddMat2(left, right); }
PEXPI mat2 operator-=(mat2& left, mat2 right) { return left = SubMat2(left, right); }
PEXPI mat2 operator*=(mat2& left, mat2 right) { return left = MulMat2(left, right); }
//NOTE: No operator/=(mat2, mat2)
PEXPI mat2 operator*=(mat2& matrix, r32 scalar) { return matrix = ScaleMat2(matrix, scalar); }
PEXPI mat2 operator/=(mat2& matrix, r32 scalar) { return matrix = ShrinkMat2(matrix, scalar); }
//TODO: Add v2 *= mat2 operator?
PEXPI bool operator==(mat2 left, mat2 right) { return AreEqualMat2(left, right); }
PEXPI bool operator!=(mat2 left, mat2 right) { return !AreEqualMat2(left, right); }
#endif //LANGUAGE_IS_CPP

// +--------------------------------------------------------------+
// |                     Matrix3x3 Functions                      |
// +--------------------------------------------------------------+
PEXPI mat3 TransposeMat3(mat3 matrix)
{
	mat3 result = matrix;
	result.elem[0][1] = matrix.elem[1][0];
	result.elem[0][2] = matrix.elem[2][0];
	result.elem[1][0] = matrix.elem[0][1];
	result.elem[1][2] = matrix.elem[2][1];
	result.elem[2][1] = matrix.elem[1][2];
	result.elem[2][0] = matrix.elem[0][2];
	return result;
}

PEXPI mat3 AddMat3(mat3 left, mat3 right)
{
	mat3 result;
	result.elem[0][0] = left.elem[0][0] + right.elem[0][0];
	result.elem[0][1] = left.elem[0][1] + right.elem[0][1];
	result.elem[0][2] = left.elem[0][2] + right.elem[0][2];
	result.elem[1][0] = left.elem[1][0] + right.elem[1][0];
	result.elem[1][1] = left.elem[1][1] + right.elem[1][1];
	result.elem[1][2] = left.elem[1][2] + right.elem[1][2];
	result.elem[2][0] = left.elem[2][0] + right.elem[2][0];
	result.elem[2][1] = left.elem[2][1] + right.elem[2][1];
	result.elem[2][2] = left.elem[2][2] + right.elem[2][2];
	return result;    
}

PEXPI mat3 SubMat3(mat3 left, mat3 right)
{
	mat3 result;
	result.elem[0][0] = left.elem[0][0] - right.elem[0][0];
	result.elem[0][1] = left.elem[0][1] - right.elem[0][1];
	result.elem[0][2] = left.elem[0][2] - right.elem[0][2];
	result.elem[1][0] = left.elem[1][0] - right.elem[1][0];
	result.elem[1][1] = left.elem[1][1] - right.elem[1][1];
	result.elem[1][2] = left.elem[1][2] - right.elem[1][2];
	result.elem[2][0] = left.elem[2][0] - right.elem[2][0];
	result.elem[2][1] = left.elem[2][1] - right.elem[2][1];
	result.elem[2][2] = left.elem[2][2] - right.elem[2][2];
	return result;
}

PEXPI v3 MulMat3AndV3(mat3 matrix, v3 vector)
{
	v3 result;
	result.x = vector.elem[0] * matrix.columns[0].x;
	result.y = vector.elem[0] * matrix.columns[0].y;
	result.z = vector.elem[0] * matrix.columns[0].z;
	result.x += vector.elem[1] * matrix.columns[1].x;
	result.y += vector.elem[1] * matrix.columns[1].y;
	result.z += vector.elem[1] * matrix.columns[1].z;
	result.x += vector.elem[2] * matrix.columns[2].x;
	result.y += vector.elem[2] * matrix.columns[2].y;
	result.z += vector.elem[2] * matrix.columns[2].z;
	return result;    
}

PEXPI mat3 MulMat3(mat3 left, mat3 right)
{
	mat3 result;
	result.columns[0] = MulMat3AndV3(left, right.columns[0]);
	result.columns[1] = MulMat3AndV3(left, right.columns[1]);
	result.columns[2] = MulMat3AndV3(left, right.columns[2]);
	return result;    
}

PEXPI mat3 ScaleMat3(mat3 matrix, r32 scalar)
{
	mat3 result;
	result.elem[0][0] = matrix.elem[0][0] * scalar;
	result.elem[0][1] = matrix.elem[0][1] * scalar;
	result.elem[0][2] = matrix.elem[0][2] * scalar;
	result.elem[1][0] = matrix.elem[1][0] * scalar;
	result.elem[1][1] = matrix.elem[1][1] * scalar;
	result.elem[1][2] = matrix.elem[1][2] * scalar;
	result.elem[2][0] = matrix.elem[2][0] * scalar;
	result.elem[2][1] = matrix.elem[2][1] * scalar;
	result.elem[2][2] = matrix.elem[2][2] * scalar;
	return result;            
}

PEXPI mat3 ShrinkMat3(mat3 matrix, r32 scalar)
{
	mat3 result;
	result.elem[0][0] = matrix.elem[0][0] / scalar;
	result.elem[0][1] = matrix.elem[0][1] / scalar;
	result.elem[0][2] = matrix.elem[0][2] / scalar;
	result.elem[1][0] = matrix.elem[1][0] / scalar;
	result.elem[1][1] = matrix.elem[1][1] / scalar;
	result.elem[1][2] = matrix.elem[1][2] / scalar;
	result.elem[2][0] = matrix.elem[2][0] / scalar;
	result.elem[2][1] = matrix.elem[2][1] / scalar;
	result.elem[2][2] = matrix.elem[2][2] / scalar;
	return result;                    
}

PEXPI r32 DeterminantMat3(mat3 matrix)
{
	mat3 Cross;
	Cross.columns[0] = CrossV3(matrix.columns[1], matrix.columns[2]);
	Cross.columns[1] = CrossV3(matrix.columns[2], matrix.columns[0]);
	Cross.columns[2] = CrossV3(matrix.columns[0], matrix.columns[1]);
	return DotV3(Cross.columns[2], matrix.columns[2]);
}

PEXPI mat3 InverseMat3(mat3 matrix)
{
	mat3 cross;
	cross.columns[0] = CrossV3(matrix.columns[1], matrix.columns[2]);
	cross.columns[1] = CrossV3(matrix.columns[2], matrix.columns[0]);
	cross.columns[2] = CrossV3(matrix.columns[0], matrix.columns[1]);
	
	r32 invDeterminant = 1.0f / DotV3(cross.columns[2], matrix.columns[2]);
	
	mat3 result;
	result.columns[0] = ScaleV3(cross.columns[0], invDeterminant);
	result.columns[1] = ScaleV3(cross.columns[1], invDeterminant);
	result.columns[2] = ScaleV3(cross.columns[2], invDeterminant);
	
	return TransposeMat3(result);
}

// +==================================+
// | Matrix3x3 C++ Operator Overloads |
// +==================================+
#if LANGUAGE_IS_CPP
PEXPI mat3 operator+(mat3 left, mat3 right) { return AddMat3(left, right); }
PEXPI mat3 operator-(mat3 left, mat3 right) { return SubMat3(left, right); }
PEXPI mat3 operator*(mat3 left, mat3 right) { return MulMat3(left, right); }
//NOTE: No operator/(mat3, mat3)
PEXPI mat3 operator*(mat3 matrix, r32 scalar) { return ScaleMat3(matrix, scalar); }
PEXPI mat3 operator/(mat3 matrix, r32 scalar) { return ShrinkMat3(matrix, scalar); }
//TODO: Add mat3 * v3 operator?
PEXPI mat3 operator+=(mat3& left, mat3 right) { return left = AddMat3(left, right); }
PEXPI mat3 operator-=(mat3& left, mat3 right) { return left = SubMat3(left, right); }
PEXPI mat3 operator*=(mat3& left, mat3 right) { return left = MulMat3(left, right); }
//NOTE: No operator/=(mat3, mat3)
PEXPI mat3 operator*=(mat3& matrix, r32 scalar) { return matrix = ScaleMat3(matrix, scalar); }
PEXPI mat3 operator/=(mat3& matrix, r32 scalar) { return matrix = ShrinkMat3(matrix, scalar); }
//TODO: Add v3 *= mat3 operator?
PEXPI bool operator==(mat3 left, mat3 right) { return AreEqualMat3(left, right); }
PEXPI bool operator!=(mat3 left, mat3 right) { return !AreEqualMat3(left, right); }
#endif //LANGUAGE_IS_CPP

// +--------------------------------------------------------------+
// |                     Matrix4x4 Functions                      |
// +--------------------------------------------------------------+
PEXPI mat4 TransposeMat4(mat4 matrix)
{
	mat4 result = matrix;
	// #ifdef HANDMADE_MATH__USE_SSE
	// _MM_TRANSPOSE4_PS(result.columns[0].SSE, result.columns[1].SSE, result.columns[2].SSE, result.columns[3].SSE);
	// #else
	result.elem[0][1] = matrix.elem[1][0];
	result.elem[0][2] = matrix.elem[2][0];
	result.elem[0][3] = matrix.elem[3][0];
	result.elem[1][0] = matrix.elem[0][1];
	result.elem[1][2] = matrix.elem[2][1];
	result.elem[1][3] = matrix.elem[3][1];
	result.elem[2][1] = matrix.elem[1][2];
	result.elem[2][0] = matrix.elem[0][2];
	result.elem[2][3] = matrix.elem[3][2];
	result.elem[3][1] = matrix.elem[1][3];
	result.elem[3][2] = matrix.elem[2][3];
	result.elem[3][0] = matrix.elem[0][3];
	// #endif
	return result;
}

PEXPI mat4 AddMat4(mat4 left, mat4 right)
{
	mat4 result;
	// #ifdef HANDMADE_MATH__USE_SSE
	// result.columns[0].SSE = _mm_add_ps(left.columns[0].SSE, right.columns[0].SSE);
	// result.columns[1].SSE = _mm_add_ps(left.columns[1].SSE, right.columns[1].SSE);
	// result.columns[2].SSE = _mm_add_ps(left.columns[2].SSE, right.columns[2].SSE);
	// result.columns[3].SSE = _mm_add_ps(left.columns[3].SSE, right.columns[3].SSE);
	// #else
	result.elem[0][0] = left.elem[0][0] + right.elem[0][0];
	result.elem[0][1] = left.elem[0][1] + right.elem[0][1];
	result.elem[0][2] = left.elem[0][2] + right.elem[0][2];
	result.elem[0][3] = left.elem[0][3] + right.elem[0][3];
	result.elem[1][0] = left.elem[1][0] + right.elem[1][0];
	result.elem[1][1] = left.elem[1][1] + right.elem[1][1];
	result.elem[1][2] = left.elem[1][2] + right.elem[1][2];
	result.elem[1][3] = left.elem[1][3] + right.elem[1][3];
	result.elem[2][0] = left.elem[2][0] + right.elem[2][0];
	result.elem[2][1] = left.elem[2][1] + right.elem[2][1];
	result.elem[2][2] = left.elem[2][2] + right.elem[2][2];
	result.elem[2][3] = left.elem[2][3] + right.elem[2][3];
	result.elem[3][0] = left.elem[3][0] + right.elem[3][0];
	result.elem[3][1] = left.elem[3][1] + right.elem[3][1];
	result.elem[3][2] = left.elem[3][2] + right.elem[3][2];
	result.elem[3][3] = left.elem[3][3] + right.elem[3][3];
	// #endif
	return result;
}

PEXPI mat4 SubMat4(mat4 left, mat4 right)
{
	mat4 result;
	// #ifdef HANDMADE_MATH__USE_SSE
	// result.columns[0].SSE = _mm_sub_ps(left.columns[0].SSE, right.columns[0].SSE);
	// result.columns[1].SSE = _mm_sub_ps(left.columns[1].SSE, right.columns[1].SSE);
	// result.columns[2].SSE = _mm_sub_ps(left.columns[2].SSE, right.columns[2].SSE);
	// result.columns[3].SSE = _mm_sub_ps(left.columns[3].SSE, right.columns[3].SSE);
	// #else
	result.elem[0][0] = left.elem[0][0] - right.elem[0][0];
	result.elem[0][1] = left.elem[0][1] - right.elem[0][1];
	result.elem[0][2] = left.elem[0][2] - right.elem[0][2];
	result.elem[0][3] = left.elem[0][3] - right.elem[0][3];
	result.elem[1][0] = left.elem[1][0] - right.elem[1][0];
	result.elem[1][1] = left.elem[1][1] - right.elem[1][1];
	result.elem[1][2] = left.elem[1][2] - right.elem[1][2];
	result.elem[1][3] = left.elem[1][3] - right.elem[1][3];
	result.elem[2][0] = left.elem[2][0] - right.elem[2][0];
	result.elem[2][1] = left.elem[2][1] - right.elem[2][1];
	result.elem[2][2] = left.elem[2][2] - right.elem[2][2];
	result.elem[2][3] = left.elem[2][3] - right.elem[2][3];
	result.elem[3][0] = left.elem[3][0] - right.elem[3][0];
	result.elem[3][1] = left.elem[3][1] - right.elem[3][1];
	result.elem[3][2] = left.elem[3][2] - right.elem[3][2];
	result.elem[3][3] = left.elem[3][3] - right.elem[3][3];
	// #endif
	return result;
}

PEXPI v4 MulMat4AndV4(mat4 matrix, v4 vector)
{
	v4 result;
	// #ifdef HANDMADE_MATH__USE_SSE
	// result.SSE = _mm_mul_ps(_mm_shuffle_ps(vector.SSE, vector.SSE, 0x00), matrix.columns[0].SSE);
	// result.SSE = _mm_add_ps(result.SSE, _mm_mul_ps(_mm_shuffle_ps(vector.SSE, vector.SSE, 0x55), matrix.columns[1].SSE));
	// result.SSE = _mm_add_ps(result.SSE, _mm_mul_ps(_mm_shuffle_ps(vector.SSE, vector.SSE, 0xaa), matrix.columns[2].SSE));
	// result.SSE = _mm_add_ps(result.SSE, _mm_mul_ps(_mm_shuffle_ps(vector.SSE, vector.SSE, 0xff), matrix.columns[3].SSE));
	// #else
	result.x = vector.elem[0] * matrix.columns[0].x;
	result.y = vector.elem[0] * matrix.columns[0].y;
	result.z = vector.elem[0] * matrix.columns[0].z;
	result.w = vector.elem[0] * matrix.columns[0].w;
	
	result.x += vector.elem[1] * matrix.columns[1].x;
	result.y += vector.elem[1] * matrix.columns[1].y;
	result.z += vector.elem[1] * matrix.columns[1].z;
	result.w += vector.elem[1] * matrix.columns[1].w;
	
	result.x += vector.elem[2] * matrix.columns[2].x;
	result.y += vector.elem[2] * matrix.columns[2].y;
	result.z += vector.elem[2] * matrix.columns[2].z;
	result.w += vector.elem[2] * matrix.columns[2].w;
	
	result.x += vector.elem[3] * matrix.columns[3].x;
	result.y += vector.elem[3] * matrix.columns[3].y;
	result.z += vector.elem[3] * matrix.columns[3].z;
	result.w += vector.elem[3] * matrix.columns[3].w;
	// #endif
	return result;
}

PEXPI mat4 MulMat4(mat4 left, mat4 right)
{
	mat4 result;
	result.columns[0] = MulMat4AndV4(left, right.columns[0]);
	result.columns[1] = MulMat4AndV4(left, right.columns[1]);
	result.columns[2] = MulMat4AndV4(left, right.columns[2]);
	result.columns[3] = MulMat4AndV4(left, right.columns[3]);
	return result;
}

PEXPI mat4 ScaleMat4(mat4 matrix, r32 scalar)
{
	mat4 result;
	// #ifdef HANDMADE_MATH__USE_SSE
	// __m128 SSEscalar = _mm_set1_ps(scalar);
	// result.columns[0].SSE = _mm_mul_ps(matrix.columns[0].SSE, SSEscalar);
	// result.columns[1].SSE = _mm_mul_ps(matrix.columns[1].SSE, SSEscalar);
	// result.columns[2].SSE = _mm_mul_ps(matrix.columns[2].SSE, SSEscalar);
	// result.columns[3].SSE = _mm_mul_ps(matrix.columns[3].SSE, SSEscalar);
	// #else
	result.elem[0][0] = matrix.elem[0][0] * scalar;
	result.elem[0][1] = matrix.elem[0][1] * scalar;
	result.elem[0][2] = matrix.elem[0][2] * scalar;
	result.elem[0][3] = matrix.elem[0][3] * scalar;
	result.elem[1][0] = matrix.elem[1][0] * scalar;
	result.elem[1][1] = matrix.elem[1][1] * scalar;
	result.elem[1][2] = matrix.elem[1][2] * scalar;
	result.elem[1][3] = matrix.elem[1][3] * scalar;
	result.elem[2][0] = matrix.elem[2][0] * scalar;
	result.elem[2][1] = matrix.elem[2][1] * scalar;
	result.elem[2][2] = matrix.elem[2][2] * scalar;
	result.elem[2][3] = matrix.elem[2][3] * scalar;
	result.elem[3][0] = matrix.elem[3][0] * scalar;
	result.elem[3][1] = matrix.elem[3][1] * scalar;
	result.elem[3][2] = matrix.elem[3][2] * scalar;
	result.elem[3][3] = matrix.elem[3][3] * scalar;
	// #endif
	return result;
}

PEXPI mat4 ShrinkMat4(mat4 matrix, r32 scalar)
{
	mat4 result;
	// #ifdef HANDMADE_MATH__USE_SSE
	// __m128 SSEscalar = _mm_set1_ps(scalar);
	// result.columns[0].SSE = _mm_div_ps(matrix.columns[0].SSE, SSEscalar);
	// result.columns[1].SSE = _mm_div_ps(matrix.columns[1].SSE, SSEscalar);
	// result.columns[2].SSE = _mm_div_ps(matrix.columns[2].SSE, SSEscalar);
	// result.columns[3].SSE = _mm_div_ps(matrix.columns[3].SSE, SSEscalar);
	// #else
	result.elem[0][0] = matrix.elem[0][0] / scalar;
	result.elem[0][1] = matrix.elem[0][1] / scalar;
	result.elem[0][2] = matrix.elem[0][2] / scalar;
	result.elem[0][3] = matrix.elem[0][3] / scalar;
	result.elem[1][0] = matrix.elem[1][0] / scalar;
	result.elem[1][1] = matrix.elem[1][1] / scalar;
	result.elem[1][2] = matrix.elem[1][2] / scalar;
	result.elem[1][3] = matrix.elem[1][3] / scalar;
	result.elem[2][0] = matrix.elem[2][0] / scalar;
	result.elem[2][1] = matrix.elem[2][1] / scalar;
	result.elem[2][2] = matrix.elem[2][2] / scalar;
	result.elem[2][3] = matrix.elem[2][3] / scalar;
	result.elem[3][0] = matrix.elem[3][0] / scalar;
	result.elem[3][1] = matrix.elem[3][1] / scalar;
	result.elem[3][2] = matrix.elem[3][2] / scalar;
	result.elem[3][3] = matrix.elem[3][3] / scalar;
	// #endif
	return result;
}

PEXPI r32 DeterminantMat4(mat4 matrix)
{
	v3 cross01 = CrossV3(matrix.columns[0].xyz, matrix.columns[1].xyz);
	v3 cross23 = CrossV3(matrix.columns[2].xyz, matrix.columns[3].xyz);
	v3 sub10 = SubV3(ScaleV3(matrix.columns[0].xyz, matrix.columns[1].w), ScaleV3(matrix.columns[1].xyz, matrix.columns[0].w));
	v3 sub32 = SubV3(ScaleV3(matrix.columns[2].xyz, matrix.columns[3].w), ScaleV3(matrix.columns[3].xyz, matrix.columns[2].w));
	return DotV3(cross01, sub32) + DotV3(cross23, sub10);
}

// Returns a general-purpose inverse of an mat4. Note that special-purpose inverses of many transformations
// are available and will be more efficient.
PEXPI mat4 InverseMat4(mat4 matrix)
{
	v3 cross01 = CrossV3(matrix.columns[0].xyz, matrix.columns[1].xyz);
	v3 cross23 = CrossV3(matrix.columns[2].xyz, matrix.columns[3].xyz);
	v3 sub10 = SubV3(ScaleV3(matrix.columns[0].xyz, matrix.columns[1].w), ScaleV3(matrix.columns[1].xyz, matrix.columns[0].w));
	v3 sub32 = SubV3(ScaleV3(matrix.columns[2].xyz, matrix.columns[3].w), ScaleV3(matrix.columns[3].xyz, matrix.columns[2].w));
	
	r32 invDeterminant = 1.0f / (DotV3(cross01, sub32) + DotV3(cross23, sub10));
	cross01 = ScaleV3(cross01, invDeterminant);
	cross23 = ScaleV3(cross23, invDeterminant);
	sub10 = ScaleV3(sub10, invDeterminant);
	sub32 = ScaleV3(sub32, invDeterminant);
	
	mat4 result;
	result.columns[0] = ToV4From3(AddV3(CrossV3(matrix.columns[1].xyz, sub32), ScaleV3(cross23, matrix.columns[1].w)), -DotV3(matrix.columns[1].xyz, cross23));
	result.columns[1] = ToV4From3(SubV3(CrossV3(sub32, matrix.columns[0].xyz), ScaleV3(cross23, matrix.columns[0].w)), +DotV3(matrix.columns[0].xyz, cross23));
	result.columns[2] = ToV4From3(AddV3(CrossV3(matrix.columns[3].xyz, sub10), ScaleV3(cross01, matrix.columns[3].w)), -DotV3(matrix.columns[3].xyz, cross01));
	result.columns[3] = ToV4From3(SubV3(CrossV3(sub10, matrix.columns[2].xyz), ScaleV3(cross01, matrix.columns[2].w)), +DotV3(matrix.columns[2].xyz, cross01));
	
	return TransposeMat4(result);
}

//TODO: Do we really need this variant where we pass out the W value?
PEXPI v3 MulMat4AndV3GetW(mat4 matrix4, v3 vec3, bool includeTranslation, r32* wOut)
{
	v4 vec4 = ToV4From3(vec3, includeTranslation ? 1.0f : 0.0f);
	vec4 = MulMat4AndV4(matrix4, vec4);
	SetOptionalOutPntr(wOut, vec4.w);
	//TODO: This ternary operator makes me think we're doing something wrong with the divide by W thing.
	//      We should really look into how 3D points and vectors are transformed by 4x4 matrix transformations
	return (vec4.w != 0) ? ShrinkV4(vec4, vec4.w).xyz : vec4.xyz;
}
PEXPI v3 MulMat4AndV3(mat4 matrix4, v3 vec3, bool includeTranslation) { return MulMat4AndV3GetW(matrix4, vec3, includeTranslation, nullptr); }

// +==================================+
// | Matrix4x4 C++ Operator Overloads |
// +==================================+
#if LANGUAGE_IS_CPP
PEXPI mat4 operator+(mat4 left, mat4 right) { return AddMat4(left, right); }
PEXPI mat4 operator-(mat4 left, mat4 right) { return SubMat4(left, right); }
PEXPI mat4 operator*(mat4 left, mat4 right) { return MulMat4(left, right); }
//NOTE: No operator/(mat4, mat4)
PEXPI mat4 operator*(mat4 matrix, r32 scalar) { return ScaleMat4(matrix, scalar); }
PEXPI mat4 operator/(mat4 matrix, r32 scalar) { return ShrinkMat4(matrix, scalar); }
//TODO: Add mat4 * v4 operator?
PEXPI mat4 operator+=(mat4& left, mat4 right) { return left = AddMat4(left, right); }
PEXPI mat4 operator-=(mat4& left, mat4 right) { return left = SubMat4(left, right); }
PEXPI mat4 operator*=(mat4& left, mat4 right) { return left = MulMat4(left, right); }
//NOTE: No operator/=(mat4, mat4)
PEXPI mat4 operator*=(mat4& matrix, r32 scalar) { return matrix = ScaleMat4(matrix, scalar); }
PEXPI mat4 operator/=(mat4& matrix, r32 scalar) { return matrix = ShrinkMat4(matrix, scalar); }
//TODO: Add v4 *= mat4 operator?
PEXPI bool operator==(mat4 left, mat4 right) { return AreEqualMat4(left, right); }
PEXPI bool operator!=(mat4 left, mat4 right) { return !AreEqualMat4(left, right); }
#endif //LANGUAGE_IS_CPP

// +--------------------------------------------------------------+
// |                 Common Matrix4x4 Transforms                  |
// +--------------------------------------------------------------+
PEXPI mat4 MakeRotateMat4_RH(r32 angle, v3 axis)
{
	mat4 result = Mat4_Identity;
	axis = NormalizeV3(axis);
	r32 sinTheta = SinR32(angle);
	r32 cosTheta = CosR32(angle);
	r32 cosValue = 1.0f - cosTheta;
	
	result.elem[0][0] = (axis.x * axis.x * cosValue) + cosTheta;
	result.elem[0][1] = (axis.x * axis.y * cosValue) + (axis.z * sinTheta);
	result.elem[0][2] = (axis.x * axis.z * cosValue) - (axis.y * sinTheta);
	
	result.elem[1][0] = (axis.y * axis.x * cosValue) - (axis.z * sinTheta);
	result.elem[1][1] = (axis.y * axis.y * cosValue) + cosTheta;
	result.elem[1][2] = (axis.y * axis.z * cosValue) + (axis.x * sinTheta);
	
	result.elem[2][0] = (axis.z * axis.x * cosValue) + (axis.y * sinTheta);
	result.elem[2][1] = (axis.z * axis.y * cosValue) - (axis.x * sinTheta);
	result.elem[2][2] = (axis.z * axis.z * cosValue) + cosTheta;
	
	return result;
}

// Produces a right-handed orthographic projection matrix with Z ranging from -1 to 1 (the GL convention).
// left, right, bottom, and top specify the coordinates of their respective clipping planes.
// zNear and zFar specify the distances to the near and far clipping planes.
PEXPI mat4 MakeOrthographicMat4Gl_RH(r32 left, r32 right, r32 bottom, r32 top, r32 zNear, r32 zFar)
{
	mat4 result = ZEROED;
	result.elem[0][0] = 2.0f / (right - left);
	result.elem[1][1] = 2.0f / (top - bottom);
	result.elem[2][2] = 2.0f / (zNear - zFar);
	result.elem[3][3] = 1.0f;
	result.elem[3][0] = (left + right) / (left - right);
	result.elem[3][1] = (bottom + top) / (bottom - top);
	result.elem[3][2] = (zNear + zFar) / (zNear - zFar);
	return result;
}
// Produces a right-handed orthographic projection matrix with Z ranging from 0 to 1 (the DirectX convention).
// left, right, bottom, and top specify the coordinates of their respective clipping planes.
// zNear and zFar specify the distances to the near and far clipping planes.
PEXPI mat4 MakeOrthographicMat4Dx_RH(r32 left, r32 right, r32 bottom, r32 top, r32 zNear, r32 zFar)
{
	mat4 result = ZEROED;
	result.elem[0][0] = 2.0f / (right - left);
	result.elem[1][1] = 2.0f / (top - bottom);
	result.elem[2][2] = 1.0f / (zNear - zFar);
	result.elem[3][3] = 1.0f;
	result.elem[3][0] = (left + right) / (left - right);
	result.elem[3][1] = (bottom + top) / (bottom - top);
	result.elem[3][2] = (zNear) / (zNear - zFar);
	return result;
}
// Produces a left-handed orthographic projection matrix with Z ranging from -1 to 1 (the GL convention).
// left, right, bottom, and top specify the coordinates of their respective clipping planes.
// zNear and zFar specify the distances to the near and far clipping planes.
PEXPI mat4 MakeOrthographicMat4Gl_LH(r32 left, r32 right, r32 bottom, r32 top, r32 zNear, r32 zFar)
{
	mat4 result = MakeOrthographicMat4Gl_RH(left, right, bottom, top, zNear, zFar);
	result.elem[2][2] = -result.elem[2][2];
	return result;
}
// Produces a left-handed orthographic projection matrix with Z ranging from 0 to 1 (the DirectX convention).
// left, right, bottom, and top specify the coordinates of their respective clipping planes.
// zNear and zFar specify the distances to the near and far clipping planes.
PEXPI mat4 MakeOrthographicMat4Dx_LH(r32 left, r32 right, r32 bottom, r32 top, r32 zNear, r32 zFar)
{
	mat4 result = MakeOrthographicMat4Dx_RH(left, right, bottom, top, zNear, zFar);
	result.elem[2][2] = -result.elem[2][2];
	return result;
}

// See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
PEXPI mat4 MakePerspectiveMat4Gl_RH(r32 fov, r32 aspectRatio, r32 zNear, r32 zFar)
{
	mat4 result = ZEROED;
	r32 cotangent = 1.0f / TanR32(fov / 2.0f);
	
	result.elem[0][0] = cotangent / aspectRatio;
	result.elem[1][1] = cotangent;
	result.elem[2][3] = -1.0f;
	
	result.elem[2][2] = (zNear + zFar) / (zNear - zFar);
	result.elem[3][2] = (2.0f * zNear * zFar) / (zNear - zFar);
	
	return result;
}

// See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
PEXPI mat4 MakePerspectiveMat4Dx_RH(r32 fov, r32 aspectRatio, r32 zNear, r32 zFar)
{
	mat4 result = ZEROED;
	r32 cotangent = 1.0f / TanR32(fov / 2.0f);
	
	result.elem[0][0] = cotangent / aspectRatio;
	result.elem[1][1] = cotangent;
	result.elem[2][3] = -1.0f;
	
	result.elem[2][2] = (zFar) / (zNear - zFar);
	result.elem[3][2] = (zNear * zFar) / (zNear - zFar);
	
	return result;
}

PEXPI mat4 MakePerspectiveMat4Gl_LH(r32 fov, r32 aspectRatio, r32 zNear, r32 zFar)
{
	mat4 result = MakePerspectiveMat4Gl_RH(fov, aspectRatio, zNear, zFar);
	result.elem[2][2] = -result.elem[2][2];
	result.elem[2][3] = -result.elem[2][3];
	return result;
}

PEXPI mat4 MakePerspectiveMat4Dx_LH(r32 fov, r32 aspectRatio, r32 zNear, r32 zFar)
{
	mat4 result = MakePerspectiveMat4Dx_RH(fov, aspectRatio, zNear, zFar);
	result.elem[2][2] = -result.elem[2][2];
	result.elem[2][3] = -result.elem[2][3];
	return result;
}

static inline mat4 MakeLookAtMat4_Helper(v3 basisForward, v3 basisRight, v3 basisUp, v3 eyePos)
{
	mat4 result;
	
	result.elem[0][0] = basisRight.x;
	result.elem[0][1] = basisUp.x;
	result.elem[0][2] = -basisForward.x;
	result.elem[0][3] = 0.0f;
	
	result.elem[1][0] = basisRight.y;
	result.elem[1][1] = basisUp.y;
	result.elem[1][2] = -basisForward.y;
	result.elem[1][3] = 0.0f;
	
	result.elem[2][0] = basisRight.z;
	result.elem[2][1] = basisUp.z;
	result.elem[2][2] = -basisForward.z;
	result.elem[2][3] = 0.0f;
	
	result.elem[3][0] = -DotV3(basisRight, eyePos);
	result.elem[3][1] = -DotV3(basisUp, eyePos);
	result.elem[3][2] = DotV3(basisForward, eyePos);
	result.elem[3][3] = 1.0f;
	
	return result;
}
PEXPI mat4 MakeLookAtMat4_RH(v3 eyePos, v3 targetPos, v3 upVec)
{
	v3 forward = NormalizeV3(SubV3(targetPos, eyePos));
	v3 right = NormalizeV3(CrossV3(forward, upVec));
	v3 trueUp = CrossV3(right, forward);
	return MakeLookAtMat4_Helper(forward, right, trueUp, eyePos);
}
PEXPI mat4 MakeLookAtMat4_LH(v3 eyePos, v3 targetPos, v3 upVec)
{
	v3 forward = NormalizeV3(SubV3(eyePos, targetPos));
	v3 right = NormalizeV3(CrossV3(forward, upVec));
	v3 trueUp = CrossV3(right, forward);
	return MakeLookAtMat4_Helper(forward, right, trueUp, eyePos);
}

//TODO: InvertTranslateMat3?
//TODO: InvertRotateMat3?
//TODO: InvertScaleMat3?
//TODO: #define InvertTranslateMat4(matrix4) HMM_InvTranslate(matrix4)
//TODO: #define InvertRotateMat4(matrix4) HMM_InvRotate(matrix4)
//TODO: #define InvertScaleMat4(matrix4) HMM_InvScale(matrix4)
//TODO: #define InvertOrthographicMat4(matrix4) HMM_InvOrthographic(matrix4)
//TODO: #define InvertPerspectiveMat4(matrix4) HMM_InvPerspective_LH(matrix4)
//TODO: #define InvertLookAtMat4(matrix4) HMM_InvLookAt(matrix4)

// +--------------------------------------------------------------+
// |                    C++ Function Overloads                    |
// +--------------------------------------------------------------+
//NOTE: See cross_vectors_quaternion_matrices_and_rectangles.h for C macros that use C23 _Generic to generate the type-specific function name
#if LANGUAGE_IS_CPP

PEXPI mat2 Add(mat2 left, mat2 right) { return AddMat2(left, right); }
PEXPI mat3 Add(mat3 left, mat3 right) { return AddMat3(left, right); }
PEXPI mat4 Add(mat4 left, mat4 right) { return AddMat4(left, right); }

PEXPI mat2 Sub(mat2 left, mat2 right) { return SubMat2(left, right); }
PEXPI mat3 Sub(mat3 left, mat3 right) { return SubMat3(left, right); }
PEXPI mat4 Sub(mat4 left, mat4 right) { return SubMat4(left, right); }

PEXPI mat2 Mul(mat2 left, mat2 right) { return MulMat2(left, right); }
PEXPI mat3 Mul(mat3 left, mat3 right) { return MulMat3(left, right); }
PEXPI mat4 Mul(mat4 left, mat4 right) { return MulMat4(left, right); }

//NOTE: No Div(mat2, mat2), Div(mat3, mat3), or Div(mat4, mat4)

PEXPI mat2 Mul(mat2 matrix, r32 scalar) { return ScaleMat2(matrix, scalar); }
PEXPI mat3 Mul(mat3 matrix, r32 scalar) { return ScaleMat3(matrix, scalar); }
PEXPI mat4 Mul(mat4 matrix, r32 scalar) { return ScaleMat4(matrix, scalar); }

PEXPI mat2 Div(mat2 matrix, r32 scalar) { return ShrinkMat2(matrix, scalar); }
PEXPI mat3 Div(mat3 matrix, r32 scalar) { return ShrinkMat3(matrix, scalar); }
PEXPI mat4 Div(mat4 matrix, r32 scalar) { return ShrinkMat4(matrix, scalar); }

//TODO: Add Mul(mat2, v2) overload?
//TODO: Add Mul(mat3, v3) overload?
//TODO: Add Mul(mat4, v4) overload?

PEXPI bool AreEqual(mat2 left, mat2 right) { return AreEqualMat2(left, right); }
PEXPI bool AreEqual(mat3 left, mat3 right) { return AreEqualMat3(left, right); }
PEXPI bool AreEqual(mat4 left, mat4 right) { return AreEqualMat4(left, right); }

#endif //LANGUAGE_IS_CPP

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_MATRICES_H

#if defined(_LIB_SOKOL_GFX_H) && defined(_STRUCT_MATRICES_H)
#include "cross/cross_sokol_gfx_and_matrices.h"
#endif

#if defined(_MISC_PARSING_H) && defined(_STRUCT_MATRICES_H)
#include "cross/cross_parsing_and_matrices.h"
#endif

#if defined(_STRUCT_QUATERNION_H) && defined(_STRUCT_MATRICES_H)
#include "cross/cross_quaternion_and_matrices.h"
#endif

#if defined(_STRUCT_VECTORS_H) && defined(_STRUCT_QUATERNION_H) && defined(_STRUCT_MATRICES_H) && defined(_STRUCT_RECTANGLES_H)
#include "cross/cross_vectors_quaternion_matrices_and_rectangles.h"
#endif

#if defined(_STRUCT_MATRICES_H) && defined(RAYLIB_H)
#include "cross/cross_matrices_and_raylib.h"
#endif
