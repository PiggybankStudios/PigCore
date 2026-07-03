/*
File:   cross_quaternion_and_matrices.h
Author: Taylor Robbins
Date:   01\15\2025
*/

#ifndef _CROSS_QUATERNION_AND_MATRICES_H
#define _CROSS_QUATERNION_AND_MATRICES_H

//TODO: Should we add functions for quatd? Maybe once we have 64-bit matrices?

#if !PIG_CORE_IMPLEMENTATION
	quat QuatFromMat3(mat3 matrix3);
#endif

#define ToMat4FromQuat(quaternion) HMM_QToM4(quaternion)
#define ToQuatFromMat4(matrix4) HMM_M4ToQ_LH(matrix4)

#if PIG_CORE_IMPLEMENTATION

PEXP quat QuatFromMat3(mat3 matrix3)
{
	//https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
	// (For matrix notation on the site: https://www.euclideanspace.com/maths/algebra/matrix/index.htm)
	quat result = ZEROED;
	r32 trace = matrix3.Elements[0][0] + matrix3.Elements[1][1] + matrix3.Elements[2][2];
	Assert(trace >= -1.001f);
	if (trace < -1.0f) { trace = -1.0f; }
	if (trace > 0)
	{
		result.w = SqrtR32(1 + trace) / 2.0f;
		result.x = (matrix3.Elements[1][2] - matrix3.Elements[2][1]) / (4 * result.w);
		result.y = (matrix3.Elements[2][0] - matrix3.Elements[0][2]) / (4 * result.w);
		result.z = (matrix3.Elements[0][1] - matrix3.Elements[1][0]) / (4 * result.w);
	}
	//if Trace is <= 0 then find the major diagonal element which has the greatest value
	else if (matrix3.Elements[0][0] > matrix3.Elements[1][1] && matrix3.Elements[0][0] > matrix3.Elements[2][2])
	{
		r32 S = SqrtR32(1.0f + matrix3.Elements[0][0] - matrix3.Elements[1][1] - matrix3.Elements[2][2]) * 2.0f; // S=4*result.x
		result.w = (matrix3.Elements[1][2] - matrix3.Elements[2][1]) / S;
		result.x = S / 4.0f;
		result.y = (matrix3.Elements[1][0] + matrix3.Elements[0][1]) / S;
		result.z = (matrix3.Elements[2][0] + matrix3.Elements[0][2]) / S;
	}
	else if (matrix3.Elements[1][1] > matrix3.Elements[2][2])
	{
		r32 S = SqrtR32(1.0f + matrix3.Elements[1][1] - matrix3.Elements[0][0] - matrix3.Elements[2][2]) * 2.0f; // S=4*result.y
		result.w = (matrix3.Elements[2][0] - matrix3.Elements[0][2]) / S;
		result.x = (matrix3.Elements[1][0] + matrix3.Elements[0][1]) / S;
		result.y = S / 4.0f;
		result.z = (matrix3.Elements[2][1] + matrix3.Elements[1][2]) / S;
	}
	else
	{
		r32 S = SqrtR32(1.0f + matrix3.Elements[2][2] - matrix3.Elements[0][0] - matrix3.Elements[1][1]) * 2.0f; // S=4*result.z
		result.w = (matrix3.Elements[0][1] - matrix3.Elements[1][0]) / S;
		result.x = (matrix3.Elements[2][0] + matrix3.Elements[0][2]) / S;
		result.y = (matrix3.Elements[2][1] + matrix3.Elements[1][2]) / S;
		result.z = S / 4.0f;
	}
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_QUATERNION_AND_MATRICES_H
