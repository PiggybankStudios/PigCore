/*
File:   phys_physx_capi_main.cpp
Author: Taylor Robbins
Date:   02\23\2025
Description: 
	** This file is meant to be compiled to an .obj separately, in C++ mode, and
	** contains the glue between the real C++ PhysX API and an exported set of
	** C functions that can be called by the application.
*/

#include "build_config.h"

#ifdef BUILD_WITH_PHYSX
#undef BUILD_WITH_PHYSX
#endif
#define BUILD_WITH_PHYSX 1
#define PIG_CORE_IMPLEMENTATION 0
#define PHYSX_CPP_SIDE 1
#define PIG_CORE_BUILDING_AS_DLL !BUILD_INTO_SINGLE_UNIT

#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "std/std_includes.h"
START_EXTERN_C
#include "base/base_assert.h"
#include "base/base_debug_output.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
END_EXTERN_C

#define PX_PHYSX_STATIC_LIB
#if DEBUG_BUILD
#ifndef _DEBUG
#define _DEBUG
#endif
#else
#define NDEBUG
#endif
#include "third_party/physx/PxPhysicsAPI.h"
#include "third_party/physx/extensions/PxExtensionsAPI.h"

#include "phys/phys_physx.h"

#include "phys/phys_physx_capi_classes.cpp"

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
START_EXTERN_C

PEXP PhysicsWorld* InitPhysicsPhysX(Arena* arena)
{
	PhysicsWorld* result = AllocType(PhysicsWorld, arena);
	NotNull(result);
	ClearPointer(result);
	result->structSize = sizeof(PhysicsWorld);
	result->arena = arena;
	result->allocator = new PigCorePhysXAllocator(arena);
	result->errorCallback = new PigCorePhysXErrorCallback();
	
	//TODO: Implement me!
	PrintLine_D("alignment_of(PxFoundation) = %llu", (u64)std::alignment_of<physx::PxFoundation>());
	result->foundation = PxCreateFoundation(PX_PHYSICS_VERSION, *result->allocator, *result->errorCallback);
	NotNull(result->foundation);
	
	result->pvd = PxCreatePvd(*result->foundation);
	NotNull(result->pvd);
	physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate(
		"127.0.0.1", //host
		5425, //port
		10 //timeoutInMilliseconds
	);
	NotNull(transport);
	result->pvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);
	
	result->physics = PxCreatePhysics(
		PX_PHYSICS_VERSION,
		*result->foundation,
		physx::PxTolerancesScale(),
		true, //trackOutstandingAllocations
		result->pvd
	);
	
	result->dispatcher = physx::PxDefaultCpuDispatcherCreate(2); //numThreads = 2
	
	physx::PxSceneDesc sceneDesc(result->physics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = result->dispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	result->scene = result->physics->createScene(sceneDesc);
	
	physx::PxPvdSceneClient* pvdClient = result->scene->getScenePvdClient();
	if (pvdClient != nullptr)
	{
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	
	result->defaultMaterial = result->physics->createMaterial(
		0.5f, //staticFriction
		0.5f, //dynamicFriction
		0.6f //restitution
	);
	
	InitVarArray(PhysicsBody, &result->bodies, arena);
	
	return result;
}

static PhysicsBody* AddRigidStaticToWorld(PhysicsWorld* world, physx::PxRigidStatic* rigidStatic)
{
	NotNull(world);
	NotNull(rigidStatic);
	world->scene->addActor(*rigidStatic);
	PhysicsBody* newBody = VarArrayAdd(PhysicsBody, &world->bodies);
	NotNull(newBody);
	ClearPointer(newBody);
	newBody->structSize = sizeof(PhysicsBody);
	newBody->index = world->bodies.length-1;
	newBody->isDynamic = false;
	newBody->staticHandle = rigidStatic;
	return newBody;
}
static PhysicsBody* AddRigidDynamicToWorld(PhysicsWorld* world, physx::PxRigidDynamic* rigidDynamic)
{
	NotNull(world);
	NotNull(rigidDynamic);
	world->scene->addActor(*rigidDynamic);
	PhysicsBody* newBody = VarArrayAdd(PhysicsBody, &world->bodies);
	NotNull(newBody);
	ClearPointer(newBody);
	newBody->structSize = sizeof(PhysicsBody);
	newBody->index = world->bodies.length-1;
	newBody->isDynamic = true;
	newBody->dynamicHandle = rigidDynamic;
	return newBody;
}

PEXP void CreatePhysicsTest(PhysicsWorld* world)
{
	NotNull(world);
	NotNull(world->arena);
	
	VarArrayLoop(&world->bodies, bIndex)
	{
		VarArrayLoopGet(PhysicsBody, body, &world->bodies, bIndex);
		if (body->isDynamic)
		{
			world->scene->removeActor(*body->dynamicHandle);
			body->dynamicHandle->release();
		}
		else
		{
			world->scene->removeActor(*body->staticHandle);
			body->staticHandle->release();
		}
	}
	VarArrayClear(&world->bodies);
	
	physx::PxRigidStatic* groundStaticBody = PxCreatePlane(*world->physics, physx::PxPlane(0, 1, 0, 0.5f), *world->defaultMaterial);
	world->groundPlaneBodyIndex = AddRigidStaticToWorld(world, groundStaticBody)->index;
	
	physx::PxShape* boxShape = world->physics->createShape(physx::PxBoxGeometry(1.0f/2.0f, 1.0f/2.0f, 1.0f/2.0f), *world->defaultMaterial);
	for (uxx bIndex = 0; bIndex < 10; bIndex++)
	{
		physx::PxTransform boxTransform(physx::PxVec3(physx::PxReal(bIndex*0.0f), physx::PxReal(bIndex*2.0f), physx::PxReal(0)));
		physx::PxRigidDynamic* dynamicBody = world->physics->createRigidDynamic(boxTransform);
		dynamicBody->attachShape(*boxShape);
		physx::PxRigidBodyExt::updateMassAndInertia(*dynamicBody, 10.0f);
		AddRigidDynamicToWorld(world, dynamicBody);
	}
	boxShape->release();
}

PEXP void UpdatePhysicsWorld(PhysicsWorld* world, r32 elapsedMs)
{
	NotNull(world);
	NotNull(world->scene);
	world->scene->simulate(elapsedMs / 1000.0f);
	world->scene->fetchResults(true);
}

PEXP PhysicsBodyTransform GetPhysicsBodyTransform(PhysicsBody* body)
{
	physx::PxTransform transform = body->actorHandle->getGlobalPose();
	PhysicsBodyTransform result = ZEROED;
	
	result.position.X = transform.p.x;
	result.position.Y = transform.p.y;
	result.position.Z = transform.p.z;
	
	result.rotation.X = transform.q.x;
	result.rotation.Y = transform.q.y;
	result.rotation.Z = transform.q.z;
	result.rotation.W = transform.q.w;
	
	return result;
}

END_EXTERN_C
