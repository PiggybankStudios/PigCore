/*
File:   cross_vectors_quaternion_and_box3d.h
Author: Taylor Robbins
Date:   09\13\2026
*/

#ifndef _CROSS_VECTORS_QUATERNION_AND_BOX_3D_H
#define _CROSS_VECTORS_QUATERNION_AND_BOX_3D_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define MakeB3Transform_Const(posX, posY, posZ, quatX, quatY, quatZ, quatW) { .p={ .x=(posX), .y=(posY), .z=(posZ) }, .q={ .v={ .x=(quatX), .y=(quatY), .z=(quatZ) }, .s=(quatW) } }
#define MakeB3TransformV_Const(position, quaternion)                        { .p=MakeB3Vec3FromV3_Const(position), .q=MakeB3QuatFromQuat_Const(quaternion) }
#define MakeB3Transform(posX, posY, posZ, quatX, quatY, quatZ, quatW)       NEW_STRUCT(b3Transform)MakeB3Transform_Const((posX), (posY), (posZ), (quatX), (quatY), (quatZ), (quatW))
#define MakeB3TransformV(position, quaternion)                              NEW_STRUCT(b3Transform)MakeB3TransformV_Const(position, quaternion)

#endif //  _CROSS_VECTORS_QUATERNION_AND_BOX_3D_H
