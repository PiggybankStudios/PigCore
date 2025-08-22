/*
File:   cross_vectors_and_raylib.h
Author: Taylor Robbins
Date:   01\24\2025
NOTE: This file may need to be included manually if you include raylib.h AFTER including struct_vectors.h
*/

#ifndef _CROSS_VECTORS_AND_RAYLIB_H
#define _CROSS_VECTORS_AND_RAYLIB_H

//NOTE: Intentionally no includes here

#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE Vector2 NewVector2(r32 x, r32 y);
	PIG_CORE_INLINE Vector2 Vector2FromV2(v2 vector);
	PIG_CORE_INLINE v2 V2FromVector2(Vector2 vector);
	PIG_CORE_INLINE Vector3 NewVector3(r32 x, r32 y, r32 z);
	PIG_CORE_INLINE Vector3 Vector3FromV3(v3 vector);
	PIG_CORE_INLINE v3 V3FromVector3(Vector3 vector);
	PIG_CORE_INLINE Vector4 NewVector4(r32 x, r32 y, r32 z, r32 w);
	PIG_CORE_INLINE Vector4 Vector4FromV4(v4 vector);
	PIG_CORE_INLINE v4 V4FromVector4(Vector4 vector);
#endif

#define Vector2_Zero          NewVector2( 0.0f,  0.0f)
#define Vector2_One           NewVector2( 1.0f,  1.0f)
#define Vector2_Half          NewVector2( 0.5f,  0.5f)
#define Vector2_Left          NewVector2(-1.0f,  0.0f)
#define Vector2_Right         NewVector2( 1.0f,  0.0f)
#define Vector2_Up            NewVector2( 0.0f, -1.0f)
#define Vector2_Down          NewVector2( 0.0f,  1.0f)
#define Vector2_Zero_Const           {.x=0.0f,.y=0.0f}

//NOTE: See struct_directions.h for an explanation of the coordinate system handed-ness and up direction
#define Vector3_Zero          NewVector3( 0.0f,  0.0f,  0.0f)
#define Vector3_One           NewVector3( 1.0f,  1.0f,  1.0f)
#define Vector3_Half          NewVector3( 0.5f,  0.5f,  0.5f)
#define Vector3_Left          NewVector3(-1.0f,  0.0f,  0.0f)
#define Vector3_Right         NewVector3( 1.0f,  0.0f,  0.0f)
#define Vector3_Bottom        NewVector3( 0.0f, -1.0f,  0.0f)
#define Vector3_Top           NewVector3( 0.0f,  1.0f,  0.0f)
#define Vector3_Back          NewVector3( 0.0f,  0.0f, -1.0f)
#define Vector3_Front         NewVector3( 0.0f,  0.0f,  1.0f)
#define Vector3_Down          NewVector3( 0.0f, -1.0f,  0.0f)
#define Vector3_Up            NewVector3( 0.0f,  1.0f,  0.0f)
#define Vector3_Backward      NewVector3( 0.0f,  0.0f, -1.0f)
#define Vector3_Forward       NewVector3( 0.0f,  0.0f,  1.0f)
#define Vector3_Zero_Const          {.x=0.0f,.y=0.0f,.z=0.0f}

#define Vector4_Zero          NewVector4( 0.0f,  0.0f,  0.0f, 0.0f)
#define Vector4_One           NewVector4( 1.0f,  1.0f,  1.0f, 1.0f)
#define Vector4_Half          NewVector4( 0.5f,  0.5f,  0.5f, 0.5f)
#define Vector4_Left          NewVector4(-1.0f,  0.0f,  0.0f, 0.0f)
#define Vector4_Right         NewVector4( 1.0f,  0.0f,  0.0f, 0.0f)
#define Vector4_Bottom        NewVector4( 0.0f, -1.0f,  0.0f, 0.0f)
#define Vector4_Top           NewVector4( 0.0f,  1.0f,  0.0f, 0.0f)
#define Vector4_Back          NewVector4( 0.0f,  0.0f, -1.0f, 0.0f)
#define Vector4_Front         NewVector4( 0.0f,  0.0f,  1.0f, 0.0f)
#define Vector4_Down          NewVector4( 0.0f, -1.0f,  0.0f, 0.0f)
#define Vector4_Up            NewVector4( 0.0f,  1.0f,  0.0f, 0.0f)
#define Vector4_Backward      NewVector4( 0.0f,  0.0f, -1.0f, 0.0f)
#define Vector4_Forward       NewVector4( 0.0f,  0.0f,  1.0f, 0.0f)
#define Vector4_Prevward      NewVector4( 0.0f,  0.0f,  0.0f,-1.0f)
#define Vector4_Nextward      NewVector4( 0.0f,  0.0f,  0.0f, 1.0f)
#define Vector4_Zero_Const        {.x=0.0f,.y=0.0f,.z=0.0f,.w=0.0f}

#if PIG_CORE_IMPLEMENTATION

PEXPI Vector2 NewVector2(r32 x, r32 y) { return (Vector2){ x, y }; }
PEXPI Vector2 Vector2FromV2(v2 vector) { return (Vector2){ vector.X, vector.Y }; }
PEXPI v2 V2FromVector2(Vector2 vector) { return NewV2(vector.x, vector.y); }

PEXPI Vector3 NewVector3(r32 x, r32 y, r32 z) { return (Vector3){ x, y, z }; }
PEXPI Vector3 Vector3FromV3(v3 vector) { return (Vector3){ vector.X, vector.Y, vector.Z }; }
PEXPI v3 V3FromVector3(Vector3 vector) { return NewV3(vector.x, vector.y, vector.z); }

PEXPI Vector4 NewVector4(r32 x, r32 y, r32 z, r32 w) { return (Vector4){ x, y, z, w }; }
PEXPI Vector4 Vector4FromV4(v4 vector) { return (Vector4){ vector.X, vector.Y, vector.Z, vector.W }; }
PEXPI v4 V4FromVector4(Vector4 vector) { return NewV4(vector.x, vector.y, vector.z, vector.w); }

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_VECTORS_AND_RAYLIB_H
