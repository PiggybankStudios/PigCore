/*
File:   phys_physx.h
Author: Taylor Robbins
Date:   02\23\2025
Description:
	** This file specifies the C-side of the boundary between PhysX (which is C++)
	** and PigCore (which is C). The glue is compiled into an obj in C++ mode using
	** phys_physx_capi_main.cpp and then linked into the application.
	** NOTE: This file should NOT be included inside an extern "C" block when compiling
	** in C++ mode, it will do extern "C" around a portion of it's contents, but it
	** needs to be outside an extern "C" block when it includes the PhysX headers.
*/

#ifndef _PHYS_PHYSX_H
#define _PHYS_PHYSX_H

#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "std/std_includes.h"

#if BUILD_WITH_PHYSX

//This is only defined and set to true in phys_physx_capi_main.cpp
#ifndef PHYSX_CPP_SIDE
#define PHYSX_CPP_SIDE 0
#endif

MAYBE_START_EXTERN_C
#include "mem/mem_arena.h"
#include "struct/struct_var_array.h"
//TODO: We have a problem with HandmadeMath.h thinking the implementation is going to be compiled in C++ mode!
// #include "struct/struct_vectors.h"
// #include "struct/struct_quaternion.h"
MAYBE_END_EXTERN_C


//NOTE: The PxAllocatorCallback does not provide a way for the caller to request alignment
// We are going to assume they want at most 16 bytes of alignment
#define PHYSX_ALLOCATOR_ALIGNMENT 16 //bytes

// +--------------------------------------------------------------+
// |                      Class Declarations                      |
// +--------------------------------------------------------------+
#if PHYSX_CPP_SIDE
class PigCorePhysXErrorCallback : public physx::PxErrorCallback
{
public:
	PigCorePhysXErrorCallback();
	virtual ~PigCorePhysXErrorCallback();
	virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override;
};

class PigCorePhysXAllocator : public physx::PxAllocatorCallback
{
private:
	Arena* arena;
public:
	PigCorePhysXAllocator(Arena* arenaPntr);
	virtual void* allocate(size_t numBytes, const char* filePath, const char* funcName, int lineNumber) override;
	virtual void deallocate(void* allocPntr) override;
};

#endif

// +--------------------------------------------------------------+
// |                     Structures and Types                     |
// +--------------------------------------------------------------+
MAYBE_START_EXTERN_C

typedef plex PhysicsBody PhysicsBody;
plex PhysicsBody
{
	uxx plexSize; //Filled by the C++ side, can be validated by the C side to make sure the plex is the same size of both sides
	uxx index;
	bool isDynamic;
	#if PHYSX_CPP_SIDE
	car
	{
		physx::PxRigidStatic* staticHandle;
		physx::PxRigidDynamic* dynamicHandle;
		physx::PxRigidActor* actorHandle;
	};
	#else
	void* handle;
	#endif
};

typedef plex PhysicsWorld PhysicsWorld;
plex PhysicsWorld
{
	uxx plexSize; //Filled by the C++ side, can be validated by the C side to make sure the bytes array below is the right size
	Arena* arena;
	VarArray bodies; //PhysicsBody
	uxx groundPlaneBodyIndex;
	//NOTE: This car contains items that can only be accessed by the C++ side
	// of the API, so we join it with a byte array that is >= the C++ side information
	// to make sure sizeof(PhysicsWorld) is the same on both sides
	car
	{
		u8 bytes[128];
		#if PHYSX_CPP_SIDE
		plex 
		{
			PigCorePhysXAllocator* allocator;
			PigCorePhysXErrorCallback* errorCallback;
			physx::PxFoundation* foundation;
			physx::PxPhysics* physics;
			physx::PxDefaultCpuDispatcher* dispatcher;
			physx::PxScene* scene;
			physx::PxMaterial* defaultMaterial;
			physx::PxPvd* pvd;
		};
		#endif
	};
};

typedef plex PhysicsBodyTransform PhysicsBodyTransform;
plex PhysicsBodyTransform
{
	plex { r32 X, Y, Z; } position;
	plex { r32 X, Y, Z, W; } rotation;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
PEXP PhysicsWorld* InitPhysicsPhysX(Arena* arena);
PEXP void CreatePhysicsTest(PhysicsWorld* world);
PEXP void UpdatePhysicsWorld(PhysicsWorld* world, r32 elapsedMs);
PEXP PhysicsBodyTransform GetPhysicsBodyTransform(PhysicsBody* body);

//NOTE: Implementations are in phys_physx_capi_main.cpp, they are not in here inside a #if PIG_CORE_IMPLEMENTATION block because they must be compiled in C++ mode

MAYBE_END_EXTERN_C

#endif //BUILD_WITH_PHYSX

#endif //  _PHYS_PHYSX_H
