/*
File:   cross_vectors_and_raylib.h
Author: Taylor Robbins
Date:   01\24\2025
NOTE: This file may need to be included manually if you include raylib.h AFTER including struct_vectors.h
*/

#ifndef _CROSS_VECTORS_AND_RAYLIB_H
#define _CROSS_VECTORS_AND_RAYLIB_H

//NOTE: Intentionally no includes here

#if BUILD_WITH_RAYLIB

#define rlMakeVector2(xValue, yValue)                 NEW_STRUCT(Vector2){ .x=(xValue), .y=(yValue) }
#define rlMakeVector3(xValue, yValue, zValue)         NEW_STRUCT(Vector3){ .x=(xValue), .y=(yValue), .z=(zValue) }
#define rlMakeVector4(xValue, yValue, zValue, wValue) NEW_STRUCT(Vector4){ .x=(xValue), .y=(yValue), .z=(zValue), .w=(wValue) }

#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Vector2 Vector2FromV2(v2 vector);
	PIG_CORE_INLINE v2 V2FromVector2(Vector2 vector);
	PIG_CORE_INLINE Vector3 Vector3FromV3(v3 vector);
	PIG_CORE_INLINE v3 V3FromVector3(Vector3 vector);
	PIG_CORE_INLINE Vector4 Vector4FromV4(v4 vector);
	PIG_CORE_INLINE v4 V4FromVector4(Vector4 vector);
#endif

#define Vector2_Zero          rlMakeVector2( 0.0f,  0.0f)
#define Vector2_One           rlMakeVector2( 1.0f,  1.0f)
#define Vector2_Half          rlMakeVector2( 0.5f,  0.5f)
#define Vector2_Left          rlMakeVector2(-1.0f,  0.0f)
#define Vector2_Right         rlMakeVector2( 1.0f,  0.0f)
#define Vector2_Up            rlMakeVector2( 0.0f, -1.0f)
#define Vector2_Down          rlMakeVector2( 0.0f,  1.0f)

//NOTE: See struct_directions.h for an explanation of the coordinate system handed-ness and up direction
#define Vector3_Zero          rlMakeVector3( 0.0f,  0.0f,  0.0f)
#define Vector3_One           rlMakeVector3( 1.0f,  1.0f,  1.0f)
#define Vector3_Half          rlMakeVector3( 0.5f,  0.5f,  0.5f)
#define Vector3_Left          rlMakeVector3(-1.0f,  0.0f,  0.0f)
#define Vector3_Right         rlMakeVector3( 1.0f,  0.0f,  0.0f)
#define Vector3_Bottom        rlMakeVector3( 0.0f, -1.0f,  0.0f)
#define Vector3_Top           rlMakeVector3( 0.0f,  1.0f,  0.0f)
#define Vector3_Back          rlMakeVector3( 0.0f,  0.0f, -1.0f)
#define Vector3_Front         rlMakeVector3( 0.0f,  0.0f,  1.0f)
#define Vector3_Down          rlMakeVector3( 0.0f, -1.0f,  0.0f)
#define Vector3_Up            rlMakeVector3( 0.0f,  1.0f,  0.0f)
#define Vector3_Backward      rlMakeVector3( 0.0f,  0.0f, -1.0f)
#define Vector3_Forward       rlMakeVector3( 0.0f,  0.0f,  1.0f)

#define Vector4_Zero          rlMakeVector4( 0.0f,  0.0f,  0.0f, 0.0f)
#define Vector4_One           rlMakeVector4( 1.0f,  1.0f,  1.0f, 1.0f)
#define Vector4_Half          rlMakeVector4( 0.5f,  0.5f,  0.5f, 0.5f)
#define Vector4_Left          rlMakeVector4(-1.0f,  0.0f,  0.0f, 0.0f)
#define Vector4_Right         rlMakeVector4( 1.0f,  0.0f,  0.0f, 0.0f)
#define Vector4_Bottom        rlMakeVector4( 0.0f, -1.0f,  0.0f, 0.0f)
#define Vector4_Top           rlMakeVector4( 0.0f,  1.0f,  0.0f, 0.0f)
#define Vector4_Back          rlMakeVector4( 0.0f,  0.0f, -1.0f, 0.0f)
#define Vector4_Front         rlMakeVector4( 0.0f,  0.0f,  1.0f, 0.0f)
#define Vector4_Down          rlMakeVector4( 0.0f, -1.0f,  0.0f, 0.0f)
#define Vector4_Up            rlMakeVector4( 0.0f,  1.0f,  0.0f, 0.0f)
#define Vector4_Backward      rlMakeVector4( 0.0f,  0.0f, -1.0f, 0.0f)
#define Vector4_Forward       rlMakeVector4( 0.0f,  0.0f,  1.0f, 0.0f)
#define Vector4_Prevward      rlMakeVector4( 0.0f,  0.0f,  0.0f,-1.0f)
#define Vector4_Nextward      rlMakeVector4( 0.0f,  0.0f,  0.0f, 1.0f)

#if PIG_CORE_IMPLEMENTATION

PEXPI Vector2 Vector2FromV2(v2 vector) { return rlMakeVector2(vector.X, vector.Y); }
PEXPI v2 V2FromVector2(Vector2 vector) { return MakeV2(vector.x, vector.y); }

PEXPI Vector3 Vector3FromV3(v3 vector) { return rlMakeVector3(vector.X, vector.Y, vector.Z); }
PEXPI v3 V3FromVector3(Vector3 vector) { return MakeV3(vector.x, vector.y, vector.z); }

PEXPI Vector4 Vector4FromV4(v4 vector) { return rlMakeVector4(vector.X, vector.Y, vector.Z, vector.W); }
PEXPI v4 V4FromVector4(Vector4 vector) { return MakeV4(vector.x, vector.y, vector.z, vector.w); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_RAYLIB

#endif //  _CROSS_VECTORS_AND_RAYLIB_H
