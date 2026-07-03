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

#define ToQuatFromMat4(matrix4) HMM_M4ToQ_LH(matrix4)

#if PIG_CORE_IMPLEMENTATION

PEXPI mat4 ToMat4FromQuat(quat quaternion)
{
	mat4 result;
	quat normalizedQuat = NormalizeQuat(quaternion);
	
	r32 xx = normalizedQuat.x * normalizedQuat.x;
	r32 yy = normalizedQuat.y * normalizedQuat.y;
	r32 zz = normalizedQuat.z * normalizedQuat.z;
	r32 xy = normalizedQuat.x * normalizedQuat.y;
	r32 xz = normalizedQuat.x * normalizedQuat.z;
	r32 yz = normalizedQuat.y * normalizedQuat.z;
	r32 wx = normalizedQuat.w * normalizedQuat.x;
	r32 wy = normalizedQuat.w * normalizedQuat.y;
	r32 wz = normalizedQuat.w * normalizedQuat.z;
	
	result.elem[0][0] = 1.0f - 2.0f * (yy + zz);
	result.elem[0][1] = 2.0f * (xy + wz);
	result.elem[0][2] = 2.0f * (xz - wy);
	result.elem[0][3] = 0.0f;
	
	result.elem[1][0] = 2.0f * (xy - wz);
	result.elem[1][1] = 1.0f - 2.0f * (xx + zz);
	result.elem[1][2] = 2.0f * (yz + wx);
	result.elem[1][3] = 0.0f;
	
	result.elem[2][0] = 2.0f * (xz + wy);
	result.elem[2][1] = 2.0f * (yz - wx);
	result.elem[2][2] = 1.0f - 2.0f * (xx + yy);
	result.elem[2][3] = 0.0f;
	
	result.elem[3][0] = 0.0f;
	result.elem[3][1] = 0.0f;
	result.elem[3][2] = 0.0f;
	result.elem[3][3] = 1.0f;
	
	return result;
}


PEXP quat QuatFromMat3(mat3 matrix3)
{
	//https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
	// (For matrix notation on the site: https://www.euclideanspace.com/maths/algebra/matrix/index.htm)
	quat result = ZEROED;
	r32 trace = matrix3.elem[0][0] + matrix3.elem[1][1] + matrix3.elem[2][2];
	Assert(trace >= -1.001f);
	if (trace < -1.0f) { trace = -1.0f; }
	if (trace > 0)
	{
		result.w = SqrtR32(1 + trace) / 2.0f;
		result.x = (matrix3.elem[1][2] - matrix3.elem[2][1]) / (4 * result.w);
		result.y = (matrix3.elem[2][0] - matrix3.elem[0][2]) / (4 * result.w);
		result.z = (matrix3.elem[0][1] - matrix3.elem[1][0]) / (4 * result.w);
	}
	//if Trace is <= 0 then find the major diagonal element which has the greatest value
	else if (matrix3.elem[0][0] > matrix3.elem[1][1] && matrix3.elem[0][0] > matrix3.elem[2][2])
	{
		r32 S = SqrtR32(1.0f + matrix3.elem[0][0] - matrix3.elem[1][1] - matrix3.elem[2][2]) * 2.0f; // S=4*result.x
		result.w = (matrix3.elem[1][2] - matrix3.elem[2][1]) / S;
		result.x = S / 4.0f;
		result.y = (matrix3.elem[1][0] + matrix3.elem[0][1]) / S;
		result.z = (matrix3.elem[2][0] + matrix3.elem[0][2]) / S;
	}
	else if (matrix3.elem[1][1] > matrix3.elem[2][2])
	{
		r32 S = SqrtR32(1.0f + matrix3.elem[1][1] - matrix3.elem[0][0] - matrix3.elem[2][2]) * 2.0f; // S=4*result.y
		result.w = (matrix3.elem[2][0] - matrix3.elem[0][2]) / S;
		result.x = (matrix3.elem[1][0] + matrix3.elem[0][1]) / S;
		result.y = S / 4.0f;
		result.z = (matrix3.elem[2][1] + matrix3.elem[1][2]) / S;
	}
	else
	{
		r32 S = SqrtR32(1.0f + matrix3.elem[2][2] - matrix3.elem[0][0] - matrix3.elem[1][1]) * 2.0f; // S=4*result.z
		result.w = (matrix3.elem[0][1] - matrix3.elem[1][0]) / S;
		result.x = (matrix3.elem[2][0] + matrix3.elem[0][2]) / S;
		result.y = (matrix3.elem[2][1] + matrix3.elem[1][2]) / S;
		result.z = S / 4.0f;
	}
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_QUATERNION_AND_MATRICES_H
