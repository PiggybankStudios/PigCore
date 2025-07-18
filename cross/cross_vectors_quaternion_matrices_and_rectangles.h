/*
File:   cross_vectors_quaternion_matrices_and_rectangles.h
Author: Taylor Robbins
Date:   01\15\2025
*/

#ifndef _CROSS_VECTORS_QUATERNION_MATRICES_AND_RECTANGLES_H
#define _CROSS_VECTORS_QUATERNION_MATRICES_AND_RECTANGLES_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                      C11 Generic Macros                      |
// +--------------------------------------------------------------+
#define Add(left, right) _Generic((left), \
	v2: HMM_AddV2,                        \
	v3: HMM_AddV3,                        \
	v4: HMM_AddV4,                        \
	v2i: AddV2i,                          \
	v3i: AddV3i,                          \
	v4i: AddV4i,                          \
	v2d: AddV2d,                          \
	v3d: AddV3d,                          \
	v4d: AddV4d,                          \
	quat: HMM_AddQ,                       \
	quatd: AddQuatd,                      \
	mat2: HMM_AddM2,                      \
	mat3: HMM_AddM3,                      \
	mat4: HMM_AddM4,                      \
	rec: AddRec,                          \
	reci: AddReci,                        \
	recd: AddRecd,                        \
	box: AddBox,                          \
	boxi: AddBoxi,                        \
	boxd: AddBoxd,                        \
	obb2: AddObb2,                        \
	obb2d: AddObb2d,                      \
	obb3: AddObb3,                        \
	obb3d: AddObb3d                       \
)((left), (right))

#define Sub(left, right) _Generic((left), \
	v2: HMM_SubV2,                        \
	v3: HMM_SubV3,                        \
	v4: HMM_SubV4,                        \
	v2i: SubV2i,                          \
	v3i: SubV3i,                          \
	v4i: SubV4i,                          \
	v2d: SubV2d,                          \
	v3d: SubV3d,                          \
	v4d: SubV4d,                          \
	quat: HMM_SubQ,                       \
	quatd: SubQuatd,                      \
	mat2: HMM_SubM2,                      \
	mat3: HMM_SubM3,                      \
	mat4: HMM_SubM4,                      \
	rec: SubRec,                          \
	reci: SubReci,                        \
	recd: SubRecd,                        \
	box: SubBox,                          \
	boxi: SubBoxi,                        \
	boxd: SubBoxd,                        \
	obb2: SubObb2,                        \
	obb2d: SubObb2d,                      \
	obb3: SubObb3,                        \
	obb3d: SubObb3d                       \
)((left), (right))


//TODO: For some reason the _Generic on (right) is matching r64 case even when the right-hand argument is clearly a v3. So for now Mul does not support 64-bit types
	// r64: _Generic((left),
	// 	v2d: ScaleV2d,
	// 	v3d: ScaleV3d,
	// 	v4d: ScaleV4d,
	// 	quatd: ScaleQuatd,
	// 	recd: ScaleRecd,
	// 	boxd: ScaleBoxd,
	// 	obb2d: ScaleObb2d,
	// 	obb3d: ScaleObb3d
	// ),

//TODO: This should probably handle i32 right-hand type differently, no? I tried doing this once but it seemed to give weird results. Maybe a reordering of the cases would help?
#define Mul(left, right) _Generic((right), \
	r32: _Generic((left),                  \
		v2: HMM_MulV2F,                    \
		v3: HMM_MulV3F,                    \
		v4: HMM_MulV4F,                    \
		quat: HMM_MulQF,                   \
		mat2: HMM_MulM2F,                  \
		mat3: HMM_MulM3F,                  \
		mat4: HMM_MulM4F,                  \
		rec: ScaleRec,                     \
		box: ScaleBox,                     \
		obb2: ScaleObb2,                   \
		obb3: ScaleObb3,                   \
		v2i: ScaleV2i,                     \
		v3i: ScaleV3i,                     \
		v4i: ScaleV4i,                     \
		boxi: ScaleBoxi,                   \
		reci: ScaleReci                    \
	),                                     \
	mat2: HMM_MulM2,                       \
	mat3: HMM_MulM3,                       \
	mat4: HMM_MulM4,                       \
	default: _Generic((left),              \
		v2: HMM_MulV2,                     \
		v3: HMM_MulV3,                     \
		v4: HMM_MulV4,                     \
		v2i: MulV2i,                       \
		v3i: MulV3i,                       \
		v4i: MulV4i,                       \
		v2d: MulV2d,                       \
		v3d: MulV3d,                       \
		v4d: MulV4d,                       \
		mat2: HMM_MulM2V2,                 \
		mat3: HMM_MulM3V3,                 \
		mat4: HMM_MulM4V4,                 \
		quat: HMM_MulQ,                    \
		quatd: MulQuatd,                   \
		rec: MulRec,                       \
		reci: MulReci,                     \
		recd: MulRecd,                     \
		box: MulBox,                       \
		boxi: MulBoxi,                     \
		boxd: MulBoxd,                     \
		obb2: MulObb2,                     \
		obb2d: MulObb2d,                   \
		obb3: MulObb3,                     \
		obb3d: MulObb3d                    \
	)                                      \
)((left), (right))


//TODO: For some reason the _Generic on (right) is matching r64 case even when the right-hand argument is clearly a v3. So for now Mul does not support 64-bit types
	 // r64: _Generic((left),
	 //    v2d: ShrinkV2d,
	 //    v3d: ShrinkV3d,
	 //    v4d: ShrinkV4d,
	 //    quatd: ShrinkQuatd,
	 //    recd: ShrinkRecd,
	 //    boxd: ShrinkBoxd,
	 //    obb2d: ShrinkObb2d,
	 //    obb3d: ShrinkObb3d
	 // ),

//TODO: This should probably handle i32 right-hand type differently, no? I tried doing this once but it seemed to give weird results. Maybe a reordering of the cases would help?
#define Div(left, right) _Generic((right), \
	 r32: _Generic((left),                 \
		v2: HMM_DivV2F,                    \
		v3: HMM_DivV3F,                    \
		v4: HMM_DivV4F,                    \
		quat: HMM_DivQF,                   \
		mat2: HMM_DivM2F,                  \
		mat3: HMM_DivM3F,                  \
		mat4: HMM_DivM4F,                  \
		rec: ShrinkRec,                    \
		box: ShrinkBox,                    \
		obb2: ShrinkObb2,                  \
		obb3: ShrinkObb3,                  \
		v2i: ShrinkV2i,                    \
		v3i: ShrinkV3i,                    \
		v4i: ShrinkV4i,                    \
		reci: ShrinkReci,                  \
		boxi: ShrinkBoxi                   \
	 ),                                    \
	 default: _Generic((left),             \
		v2: HMM_DivV2,                     \
		v3: HMM_DivV3,                     \
		v4: HMM_DivV4,                     \
		v2i: DivV2i,                       \
		v3i: DivV3i,                       \
		v4i: DivV4i,                       \
		v2d: DivV2d,                       \
		v3d: DivV3d,                       \
		v4d: DivV4d,                       \
		rec: DivRec,                       \
		reci: DivReci,                     \
		recd: DivRecd,                     \
		box: DivBox,                       \
		boxi: DivBoxi,                     \
		boxd: DivBoxd,                     \
		obb2: DivObb2,                     \
		obb2d: DivObb2d,                   \
		obb3: DivObb3,                     \
		obb3d: DivObb3d                    \
	)                                      \
)((left), (right))

//TODO: These 4 should go in struct_vectors.h directly?
#define Length(vector) _Generic((vector), \
		v2: HMM_LenV2,                    \
		v3: HMM_LenV3,                    \
		v4: HMM_LenV4,                    \
		v2i: LengthV2i,                   \
		v3i: LengthV3i,                   \
		v4i: LengthV4i,                   \
		v2d: LengthV2d,                   \
		v3d: LengthV3d,                   \
		v4d: LengthV4d                    \
)(vector)

#define LengthSquared(vector) _Generic((vector), \
		v2: HMM_LenSqrV2,                        \
		v3: HMM_LenSqrV3,                        \
		v4: HMM_LenSqrV4,                        \
		v2i: LengthSquaredV2i,                   \
		v3i: LengthSquaredV3i,                   \
		v4i: LengthSquaredV4i,                   \
		v2d: LengthSquaredV2d,                   \
		v3d: LengthSquaredV3d,                   \
		v4d: LengthSquaredV4d                    \
)(vector)

#define Normalize(vector) _Generic((vector), \
		v2: HMM_NormV2,                      \
		v3: HMM_NormV3,                      \
		v4: HMM_NormV4,                      \
		v2d: HMM_NormV2d,                    \
		v3d: HMM_NormV3d,                    \
		v4d: HMM_NormV4d,                    \
		quat: HMM_NormQ,                     \
		quatd: NormalizeQuatd                \
)(vector)

#define Dot(left, right) _Generic((left), \
		v2: HMM_DotV2,                    \
		v3: HMM_DotV3,                    \
		v4: HMM_DotV4,                    \
		v2i: DotV2i,                      \
		v3i: DotV3i,                      \
		v4i: DotV4i,                      \
		v2d: DotV2d,                      \
		v3d: DotV3d,                      \
		v4d: DotV4d,                      \
		quat: HMM_DotQ,                   \
		quatd: DotQuatd                   \
)((left), (right))

#define Lerp(left, right, amount) _Generic((left),  \
		r32: LerpR32((left), (right), (amount)),    \
		r64: LerpR64((left), (right), (amount)),    \
		v2: HMM_LerpV2((left), (amount), (right)),  \
		v3: HMM_LerpV3((left), (amount), (right)),  \
		v4: HMM_LerpV4((left), (amount), (right)),  \
		v2d: LerpV2d((left), (right), (amount)),    \
		v3d: LerpV3d((left), (right), (amount)),    \
		v4d: LerpV4d((left), (right), (amount)),    \
		quat: HMM_NLerp((left), (amount), (right)), \
		quatd: LerpQuatd((left), (right), (amount)) \
)

#define AreEqual(left, right) _Generic((left), \
		v2: HMM_EqV2,                          \
		v3: HMM_EqV3,                          \
		v4: HMM_EqV4,                          \
		v2i: AreEqualV2i,                      \
		v3i: AreEqualV3i,                      \
		v4i: AreEqualV4i,                      \
		v2d: AreEqualV2d,                      \
		v3d: AreEqualV3d,                      \
		v4d: AreEqualV4d,                      \
		quat: AreEqualQuat,                    \
		quatd: AreEqualQuatd,                  \
		mat2: AreEqualMat2,                    \
		mat3: AreEqualMat3,                    \
		mat4: AreEqualMat4,                    \
		rec: AreEqualRec,                      \
		reci: AreEqualReci,                    \
		recd: AreEqualRecd,                    \
		box: AreEqualBox,                      \
		boxi: AreEqualBoxi,                    \
		boxd: AreEqualBoxd,                    \
		obb2: AreEqualObb2,                    \
		obb2d: AreEqualObb2d,                  \
		obb3: AreEqualObb3,                    \
		obb3d: AreEqualObb3d                   \
)((left), (right))

//TODO: HMM_Transpose(M)
//TODO: HMM_Determinant(M)
//TODO: HMM_InvGeneral(M)

#endif //  _CROSS_VECTORS_QUATERNION_MATRICES_AND_RECTANGLES_H
