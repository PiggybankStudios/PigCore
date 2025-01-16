/*
File:   cross_quaternion_and_matrices.h
Author: Taylor Robbins
Date:   01\15\2025
*/

#ifndef _CROSS_QUATERNION_AND_MATRICES_H
#define _CROSS_QUATERNION_AND_MATRICES_H

#define ToMat4FromQuat(quaternion) HMM_QToM4(quaternion)
#define ToQuatFromMat4(matrix4) HMM_M4ToQ_LH(matrix4)

#endif //  _CROSS_QUATERNION_AND_MATRICES_H
