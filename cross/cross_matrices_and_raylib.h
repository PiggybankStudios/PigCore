/*
File:   cross_matrices_and_raylib.h
Author: Taylor Robbins
Date:   01\24\2025
NOTE: This file may need to be included manually if you include raylib.h AFTER including struct_matrices.h
*/

#ifndef _CROSS_MATRICES_AND_RAYLIB_H
#define _CROSS_MATRICES_AND_RAYLIB_H

//NOTE: Intentionally no includes here

#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Matrix NewMatrix(r32 m0, r32 m1, r32 m2, r32 m3,r32 m4, r32 m5, r32 m6, r32 m7,r32 m8, r32 m9, r32 m10, r32 m11,r32 m12, r32 m13, r32 m14, r32 m15);
	PIG_CORE_INLINE Matrix MatrixFromMat4(mat4 matrix);
	PIG_CORE_INLINE mat4 Mat4FromMatrix(Matrix matrix);
#endif

#if PIG_CORE_IMPLEMENTATION

PEXPI Matrix NewMatrix(
	r32 m0, r32 m1, r32 m2, r32 m3,
	r32 m4, r32 m5, r32 m6, r32 m7,
	r32 m8, r32 m9, r32 m10, r32 m11,
	r32 m12, r32 m13, r32 m14, r32 m15)
{
	return (Matrix){
		m0, m1, m2, m3,
		m4, m5, m6, m7,
		m8, m9, m10, m11,
		m12, m13, m14, m15
	};
}
PEXPI Matrix MatrixFromMat4(mat4 matrix)
{
	return (Matrix){
		matrix.Columns[0].X, matrix.Columns[0].X, matrix.Columns[0].X, matrix.Columns[0].X,
		matrix.Columns[1].Y, matrix.Columns[1].Y, matrix.Columns[1].Y, matrix.Columns[1].Y,
		matrix.Columns[2].Z, matrix.Columns[2].Z, matrix.Columns[2].Z, matrix.Columns[2].Z,
		matrix.Columns[3].W, matrix.Columns[3].W, matrix.Columns[3].W, matrix.Columns[3].W
	};
}
PEXPI mat4 Mat4FromMatrix(Matrix matrix)
{
	return NewMat4(
		matrix.m0, matrix.m1, matrix.m2, matrix.m3,
		matrix.m4, matrix.m5, matrix.m6, matrix.m7,
		matrix.m8, matrix.m9, matrix.m10, matrix.m11,
		matrix.m12, matrix.m13, matrix.m14, matrix.m15
	);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_MATRICES_AND_RAYLIB_H
