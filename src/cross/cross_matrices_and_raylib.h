/*
File:   cross_matrices_and_raylib.h
Author: Taylor Robbins
Date:   01\24\2025
NOTE: This file may need to be included manually if you include raylib.h AFTER including struct_matrices.h
*/

#ifndef _CROSS_MATRICES_AND_RAYLIB_H
#define _CROSS_MATRICES_AND_RAYLIB_H

//NOTE: Intentionally no includes here

#if BUILD_WITH_RAYLIB

//TODO: We need to double check column-major vs row-major ordering of Raylib's Matrix struct!

#define rlMakeMatrix(r0c0, r1c0, r2c0, r3c0, r0c1, r1c1, r2c1, r3c1, r0c2, r1c2, r2c2, r3c2, r0c3, r1c3, r2c3, r3c3) NEW_STRUCT(Matrix) { \
	.m0=(r0c0), .m1=(r1c0), .m2=(r2c0), .m3=(r3c0),                                                                                       \
	.m4=(r0c1), .m5=(r1c1), .m6=(r2c1), .m7=(r3c1),                                                                                       \
	.m8=(r0c2), .m9=(r1c2), .m10=(r2c2), .m11=(r3c2),                                                                                     \
	.m12=(r0c3), .m13=(r1c3), .m14=(r2c3), .m15=(r3c3)                                                                                    \
}

#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Matrix MatrixFromMat4(mat4 matrix);
	PIG_CORE_INLINE mat4 Mat4FromMatrix(Matrix matrix);
#endif

#if PIG_CORE_IMPLEMENTATION

PEXPI Matrix MatrixFromMat4(mat4 matrix)
{
	return rlMakeMatrix(
		matrix.columns[0].x, matrix.columns[1].x, matrix.columns[2].x, matrix.columns[3].x,
		matrix.columns[0].y, matrix.columns[1].y, matrix.columns[2].y, matrix.columns[3].y,
		matrix.columns[0].z, matrix.columns[1].z, matrix.columns[2].z, matrix.columns[3].z,
		matrix.columns[0].w, matrix.columns[1].w, matrix.columns[2].w, matrix.columns[3].w
	);
}
PEXPI mat4 Mat4FromMatrix(Matrix matrix)
{
	return MakeMat4(
		matrix.m0, matrix.m1, matrix.m2, matrix.m3,
		matrix.m4, matrix.m5, matrix.m6, matrix.m7,
		matrix.m8, matrix.m9, matrix.m10, matrix.m11,
		matrix.m12, matrix.m13, matrix.m14, matrix.m15
	);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_RAYLIB

#endif //  _CROSS_MATRICES_AND_RAYLIB_H
