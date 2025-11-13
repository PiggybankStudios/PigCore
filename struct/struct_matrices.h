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

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "std/std_includes.h"
#include "struct/struct_vectors.h"
#include "lib/lib_handmade_math.h"

#define MakeMat2_Const(r0c0, r0c1, r1c0, r1c1) { \
	.Columns[0].X=(r0c0), .Columns[1].X=(r0c1),  \
	.Columns[0].Y=(r1c0), .Columns[1].Y=(r1c1)   \
}
#define MakeMat3_Const(r0c0, r0c1, r0c2, r1c0, r1c1, r1c2, r2c0, r2c1, r2c2) { \
	.Columns[0].X=(r0c0), .Columns[1].X=(r0c1), .Columns[2].X=(r0c2),          \
	.Columns[0].Y=(r1c0), .Columns[1].Y=(r1c1), .Columns[2].Y=(r1c2),          \
	.Columns[0].Z=(r2c0), .Columns[1].Z=(r2c1), .Columns[2].Z=(r2c2)           \
}
#define MakeMat4_Const(r0c0, r0c1, r0c2, r0c3, r1c0, r1c1, r1c2, r1c3, r2c0, r2c1, r2c2, r2c3, r3c0, r3c1, r3c2, r3c3) { \
	.Columns[0].X=(r0c0), .Columns[1].X=(r0c1), .Columns[2].X=(r0c2), .Columns[3].X=(r0c3),                              \
	.Columns[0].Y=(r1c0), .Columns[1].Y=(r1c1), .Columns[2].Y=(r1c2), .Columns[3].Y=(r1c3),                              \
	.Columns[0].Z=(r2c0), .Columns[1].Z=(r2c1), .Columns[2].Z=(r2c2), .Columns[3].Z=(r2c3),                              \
	.Columns[0].W=(r3c0), .Columns[1].W=(r3c1), .Columns[2].W=(r3c2), .Columns[3].W=(r3c3)                               \
}

#define MakeMat2(r0c0, r0c1, r1c0, r1c1)                                                                                                                                                 NEW_STRUCT(HMM_Mat2)MakeMat2_Const((r0c0), (r0c1), (r1c0), (r1c1))
#define MakeMat3(r0c0, r0c1, r0c2, r1c0, r1c1, r1c2, r2c0, r2c1, r2c2)                                                                                     NEW_STRUCT(HMM_Mat3)MakeMat3_Const((r0c0), (r0c1), (r0c2), (r1c0), (r1c1), (r1c2), (r2c0), (r2c1), (r2c2))
#define MakeMat4(r0c0, r0c1, r0c2, r0c3, r1c0, r1c1, r1c2, r1c3, r2c0, r2c1, r2c2, r2c3, r3c0, r3c1, r3c2, r3c3) NEW_STRUCT(HMM_Mat4)MakeMat4_Const((r0c0), (r0c1), (r0c2), (r0c3), (r1c0), (r1c1), (r1c2), (r1c3), (r2c0), (r2c1), (r2c2), (r2c3), (r3c0), (r3c1), (r3c2), (r3c3))

#define FillMat2_Const(value) MakeMat2_Const((value), (value), (value), (value))
#define FillMat3_Const(value) MakeMat3_Const((value), (value), (value), (value), (value), (value), (value), (value), (value))
#define FillMat4_Const(value) MakeMat4_Const((value), (value), (value), (value), (value), (value), (value), (value), (value), (value), (value), (value), (value), (value), (value), (value))
#define FillMat2(value) MakeMat2((value), (value), (value), (value))
#define FillMat3(value) MakeMat3((value), (value), (value), (value), (value), (value), (value), (value), (value))
#define FillMat4(value) MakeMat4((value), (value), (value), (value), (value), (value), (value), (value), (value), (value), (value), (value), (value), (value), (value), (value))

#define FillDiagonalMat2_Const(value) MakeMat2_Const((value), 0.0f, 0.0f, (value))
#define FillDiagonalMat3_Const(value) MakeMat3_Const((value), 0.0f, 0.0f, 0.0f, (value), 0.0f, 0.0f, 0.0f, (value))
#define FillDiagonalMat4_Const(value) MakeMat4_Const((value), 0.0f, 0.0f, 0.0f, 0.0f, (value), 0.0f, 0.0f, 0.0f, 0.0f, (value), 0.0f, 0.0f, 0.0f, 0.0f, (value))
#define FillDiagonalMat2(value) MakeMat2((value), 0.0f, 0.0f, (value))
#define FillDiagonalMat3(value) MakeMat3((value), 0.0f, 0.0f, 0.0f, (value), 0.0f, 0.0f, 0.0f, (value))
#define FillDiagonalMat4(value) MakeMat4((value), 0.0f, 0.0f, 0.0f, 0.0f, (value), 0.0f, 0.0f, 0.0f, 0.0f, (value), 0.0f, 0.0f, 0.0f, 0.0f, (value))

typedef HMM_Mat2 mat2;
typedef HMM_Mat3 mat3;
typedef HMM_Mat4 mat4;

//NOTE: When accessing these using Elements 2D array, first index is column, second index is row
//      matrix.Elements[Column][Row] = ...

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE mat3 ToMat3From4(mat4 matrix4);
	PIG_CORE_INLINE mat4 ToMat4From3(mat3 matrix3);
	PIG_CORE_INLINE bool AreEqualMat2(mat2 left, mat2 right);
	PIG_CORE_INLINE bool AreEqualMat3(mat3 left, mat3 right);
	PIG_CORE_INLINE bool AreEqualMat4(mat4 left, mat4 right);
	PIG_CORE_INLINE v3 MulMat4AndV3GetW(mat4 matrix4, v3 vec3, bool includeTranslation, r32* wOut);
	PIG_CORE_INLINE v3 MulMat4AndV3(mat4 matrix4, v3 vec3, bool includeTranslation);
	PIG_CORE_INLINE mat3 MakeTranslateMat3(v2 vec2);
	PIG_CORE_INLINE mat3 MakeScaleMat3(v3 vec3);
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

//a.k.a. Apply a new transform matrix from the LEFT side
#define TransformMat4(mat4Pntr, matrix4) *(mat4Pntr) = MulMat4((matrix4), *(mat4Pntr))

#define TransposeMat2(matrix2) HMM_TransposeM2(matrix2)
#define TransposeMat3(matrix3) HMM_TransposeM3(matrix3)
#define TransposeMat4(matrix4) HMM_TransposeM4(matrix4)

#define DeterminantMat2(matrix2) HMM_DeterminantM2(matrix2)
#define DeterminantMat3(matrix3) HMM_DeterminantM3(matrix3)
#define DeterminantMat4(matrix4) HMM_DeterminantM4(matrix4)

//TODO: CofactorM4?
//TODO: AdjointM4?

#define InverseMat2(matrix2) HMM_InvGeneralM2(matrix2)
#define InverseMat3(matrix3) HMM_InvGeneralM3(matrix3)
#define InverseMat4(matrix4) HMM_InvGeneralM4(matrix4)

//TODO: Not entirely sure what this is useful for?
#define LinearCombineV4Mat4(vec4Left, matrix4Right) HMM_LinearCombineV4M4((vec4Left), (matrix4Right))

#define MakeTranslateXMat3(x) MakeTranslateMat3(MakeV2((x), 0.0f))
#define MakeTranslateYMat3(y) MakeTranslateMat3(MakeV2(0.0f, (y)))

#define MakeTranslateMat4(vec3) HMM_Translate(vec3)
#define MakeTranslateXYZMat4(x, y, z) HMM_Translate(MakeV3((x), (y), (z)))
#define MakeTranslateXMat4(x) HMM_Translate(MakeV3((x), 0.0f, 0.0f))
#define MakeTranslateYMat4(y) HMM_Translate(MakeV3(0.0f, (y), 0.0f))
#define MakeTranslateZMat4(z) HMM_Translate(MakeV3(0.0f, 0.0f, (z)))

#define Make2DScaleMat3(vec2) MakeScaleMat3(ToV3From2((vec2), 1.0f))
#define MakeScaleXYZMat3(x, y, z) MakeScaleMat3(MakeV3((x), (y), (z)))
#define MakeScaleXMat3(x) MakeScaleMat3(MakeV3((x), 1.0f, 1.0f))
#define MakeScaleYMat3(y) MakeScaleMat3(MakeV3(1.0f, (y), 1.0f))
#define MakeScaleZMat3(z) MakeScaleMat3(MakeV3(1.0f, 1.0f, (z)))

#define MakeScaleMat4(vec3) HMM_Scale(vec3)
#define Make2DScaleMat4(vec2) HMM_Scale(ToV3From2((vec2), 1.0f))
#define MakeScaleXYZMat4(x, y, z) HMM_Scale(MakeV3((x), (y), (z)))
#define MakeScaleXMat4(x) HMM_Scale(MakeV3((x), 1.0f, 1.0f))
#define MakeScaleYMat4(y) HMM_Scale(MakeV3(1.0f, (y), 1.0f))
#define MakeScaleZMat4(z) HMM_Scale(MakeV3(1.0f, 1.0f, (z)))
// TODO: MakeScaleWMat4(w)?

//TODO: MakeRotateMat3(angle)? (for 2D rotations?)

#define MakeRotateMat4(angle, axisVec3) HMM_Rotate_LH((angle), (axisVec3))
#define MakeRotateXMat4(angle) HMM_Rotate_LH((angle), V3_Right)
#define MakeRotateYMat4(angle) HMM_Rotate_LH((angle), V3_Up)
#define MakeRotateZMat4(angle) HMM_Rotate_LH((angle), V3_Forward)
#define MakeRotate2DMat4(angle) HMM_Rotate_LH((angle), V3_Backward) //for 2D rotations the axis is -Z (towards the camera)

//NOTE: When working with OpenGL  use the Gl variant (implying z range [-1, 1])
//      When working with DirectX use the Dx variant (implying z range [0, 1])

#define MakeOrthographicMat4Gl(left, right, top, bottom, zNear, zFar) HMM_Orthographic_LH_NO((left), (right), (bottom), (top), (zNear), (zFar))
#define MakeOrthographicMat4Dx(left, right, top, bottom, zNear, zFar) HMM_Orthographic_LH_ZO((left), (right), (bottom), (top), (zNear), (zFar))

#define MakePerspectiveMat4Gl(fov, aspectRatio, zNear, zFar) HMM_Perspective_LH_NO((fov), (aspectRatio), (zNear), (zFar))
#define MakePerspectiveMat4Dx(fov, aspectRatio, zNear, zFar) HMM_Perspective_LH_ZO((fov), (aspectRatio), (zNear), (zFar))

#define MakeLookAtMat4(eye, center, up) HMM_LookAt_LH((eye), (center), (up))

//TODO: InvertTranslateMat3?
//TODO: InvertRotateMat3?
//TODO: InvertScaleMat3?
#define InvertTranslateMat4(matrix4) HMM_InvTranslate(matrix4)
#define InvertRotateMat4(matrix4) HMM_InvRotate(matrix4)
#define InvertScaleMat4(matrix4) HMM_InvScale(matrix4)
#define InvertOrthographicMat4(matrix4) HMM_InvOrthographic(matrix4)
#define InvertPerspectiveMat4(matrix4) HMM_InvPerspective_LH(matrix4)
#define InvertLookAtMat4(matrix4) HMM_InvLookAt(matrix4)

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI mat3 ToMat3From4(mat4 matrix4)
{
	return MakeMat3(
		matrix4.Elements[0][0], matrix4.Elements[1][0], matrix4.Elements[2][0],
		matrix4.Elements[0][1], matrix4.Elements[1][1], matrix4.Elements[2][1],
		matrix4.Elements[0][2], matrix4.Elements[1][2], matrix4.Elements[2][2]
	);
}
PEXPI mat4 ToMat4From3(mat3 matrix3)
{
	return MakeMat4(
		matrix3.Elements[0][0], matrix3.Elements[1][0], matrix3.Elements[2][0], 0,
		matrix3.Elements[0][1], matrix3.Elements[1][1], matrix3.Elements[2][1], 0,
		matrix3.Elements[0][2], matrix3.Elements[1][2], matrix3.Elements[2][2], 0,
		                     0,                      0,                      0, 1
	);
}

#if TARGET_IS_ORCA
//TODO: oc_mat2x3 conversion functions!
#endif //TARGET_IS_ORCA

PEXPI bool AreEqualMat2(mat2 left, mat2 right) { return (HMM_EqV2(left.Columns[0], right.Columns[0]) && HMM_EqV2(left.Columns[1], right.Columns[1])); }
PEXPI bool AreEqualMat3(mat3 left, mat3 right) { return (HMM_EqV3(left.Columns[0], right.Columns[0]) && HMM_EqV3(left.Columns[1], right.Columns[1]) && HMM_EqV3(left.Columns[2], right.Columns[2])); }
PEXPI bool AreEqualMat4(mat4 left, mat4 right) { return (HMM_EqV4(left.Columns[0], right.Columns[0]) && HMM_EqV4(left.Columns[1], right.Columns[1]) && HMM_EqV4(left.Columns[2], right.Columns[2]) && HMM_EqV4(left.Columns[3], right.Columns[3])); }

//TODO: Do we really need this variant where we pass out the W value?
PEXPI v3 MulMat4AndV3GetW(mat4 matrix4, v3 vec3, bool includeTranslation, r32* wOut)
{
	v4 vec4 = ToV4From3(vec3, includeTranslation ? 1.0f : 0.0f);
	vec4 = MulMat4AndV4(matrix4, vec4);
	SetOptionalOutPntr(wOut, vec4.W);
	//TODO: This ternary operator makes me think we're doing something wrong with the divide by W thing.
	//      We should really look into how 3D points and vectors are transformed by 4x4 matrix transformations
	return (vec4.W != 0) ? ShrinkV4(vec4, vec4.W).XYZ : vec4.XYZ;
}
PEXPI v3 MulMat4AndV3(mat4 matrix4, v3 vec3, bool includeTranslation) { return MulMat4AndV3GetW(matrix4, vec3, includeTranslation, nullptr); }

PEXPI mat3 MakeTranslateMat3(v2 vec2)
{
	return MakeMat3(
		1.0f, 0.0f, vec2.X,
		0.0f, 1.0f, vec2.Y,
		0.0f, 0.0f, 1.0f
	);
}

PEXPI mat3 MakeScaleMat3(v3 vec3)
{
	return MakeMat3(
		vec3.X, 0.0f, 0.0f,
		0.0f, vec3.Y, 0.0f,
		0.0f, 0.0f, vec3.Z
	);
}

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
