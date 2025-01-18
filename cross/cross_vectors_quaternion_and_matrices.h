/*
File:   cross_vectors_quaternion_and_matrices.h
Author: Taylor Robbins
Date:   01\15\2025
*/

#ifndef _CROSS_VECTORS_QUATERNION_AND_MATRICES_H
#define _CROSS_VECTORS_QUATERNION_AND_MATRICES_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                      C11 Generic Macros                      |
// +--------------------------------------------------------------+
#define Add(left, right) _Generic((left), \
        v2: HMM_AddV2,                    \
        v3: HMM_AddV3,                    \
        v4: HMM_AddV4,                    \
        v2i: AddV2i,                      \
        v3i: AddV3i,                      \
        v4i: AddV4i,                      \
        quat: HMM_AddQ,                   \
        mat2: HMM_AddM2,                  \
        mat3: HMM_AddM3,                  \
        mat4: HMM_AddM4                   \
)((left), (right))

#define Sub(left, right) _Generic((left), \
        v2: HMM_SubV2,                    \
        v3: HMM_SubV3,                    \
        v4: HMM_SubV4,                    \
        v2i: SubV2i,                      \
        v3i: SubV3i,                      \
        v4i: SubV4i,                      \
        quat: HMM_SubQ,                   \
        mat2: HMM_SubM2,                  \
        mat3: HMM_SubM3,                  \
        mat4: HMM_SubM4                   \
)((left), (right))

#define Mul(left, right) _Generic((right), \
     float: _Generic((left),               \
        v2: HMM_MulV2F,                    \
        v3: HMM_MulV3F,                    \
        v4: HMM_MulV4F,                    \
        v2i: ScaleV2i,                     \
        v3i: ScaleV3i,                     \
        v4i: ScaleV4i,                     \
        quat: HMM_MulQF,                   \
        mat2: HMM_MulM2F,                  \
        mat3: HMM_MulM3F,                  \
        mat4: HMM_MulM4F                   \
     ),                                    \
     mat2: HMM_MulM2,                      \
     mat3: HMM_MulM3,                      \
     mat4: HMM_MulM4,                      \
     quat: HMM_MulQ,                       \
     default: _Generic((left),             \
        v2: HMM_MulV2,                     \
        v3: HMM_MulV3,                     \
        v4: HMM_MulV4,                     \
        v2i: MulV2i,                       \
        v3i: MulV3i,                       \
        v4i: MulV4i,                       \
        mat2: HMM_MulM2V2,                 \
        mat3: HMM_MulM3V3,                 \
        mat4: HMM_MulM4V4                  \
    )                                      \
)((left), (right))

#define Div(left, right) _Generic((right), \
     float: _Generic((left),               \
        v2: HMM_DivV2F,                    \
        v3: HMM_DivV3F,                    \
        v4: HMM_DivV4F,                    \
        v2i: ShrinkV2i,                    \
        v3i: ShrinkV3i,                    \
        v4i: ShrinkV4i,                    \
        quat: HMM_DivQF,                   \
        mat2: HMM_DivM2F,                  \
        mat3: HMM_DivM3F,                  \
        mat4: HMM_DivM4F                   \
     ),                                    \
     default: _Generic((left),             \
        v2: HMM_DivV2,                     \
        v3: HMM_DivV3,                     \
        v4: HMM_DivV4,                     \
        v2i: DivV2i,                       \
        v3i: DivV3i,                       \
        v4i: DivV4i                        \
    )                                      \
)((left), (right))

#define Length(vector) _Generic((vector), \
        v2: HMM_LenV2,                    \
        v3: HMM_LenV3,                    \
        v4: HMM_LenV4,                    \
        v2i: LengthV2i,                   \
        v3i: LengthV3i,                   \
        v4i: LengthV4i                    \
)(vector)

#define LengthSquared(vector) _Generic((vector), \
        v2: HMM_LenSqrV2,                        \
        v3: HMM_LenSqrV3,                        \
        v4: HMM_LenSqrV4,                        \
        v2i: LengthSquaredV2i,                   \
        v3i: LengthSquaredV3i,                   \
        v4i: LengthSquaredV4i                    \
)(vector)

#define Normalize(vector) _Generic((vector), \
        v2: HMM_NormV2,                      \
        v3: HMM_NormV3,                      \
        v4: HMM_NormV4                       \
)(vector)

#define Dot(left, right) _Generic((left), \
        v2: HMM_DotV2,                    \
        v3: HMM_DotV3,                    \
        v4: HMM_DotV4,                    \
        v2i: DotV2i,                      \
        v3i: DotV3i,                      \
        v4i: DotV4i                       \
)((left), (right))

#define Lerp(left, right, amount) _Generic((left), \
        float: LerpR32((left), (right), (amount)), \
        v2: HMM_LerpV2((left), (amount), (right)), \
        v3: HMM_LerpV3((left), (amount), (right)), \
        v4: HMM_LerpV4((left), (amount), (right))  \
)

#define AreEqual(left, right) _Generic((left), \
        v2: HMM_EqV2,                          \
        v3: HMM_EqV3,                          \
        v4: HMM_EqV4,                          \
        v2i: AreEqualV2i,                      \
        v3i: AreEqualV3i,                      \
        v4i: AreEqualV4i                       \
)((left), (right))

//TODO: HMM_Transpose(M)
//TODO: HMM_Determinant(M)
//TODO: HMM_InvGeneral(M)

#endif //  _CROSS_VECTORS_QUATERNION_AND_MATRICES_H
