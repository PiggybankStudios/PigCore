/*
File:   phys_ode.h
Author: Taylor Robbins
Date:   02\22\2025
Description:
	** Holds the include for the Open Dynamics Engine (ODE) library that resides in the
	** third_party folder, as well as some functions that help us interact with the library
*/

#ifndef _PHYS_ODE_H
#define _PHYS_ODE_H

#include "base/base_defines_check.h"
#include "base/base_assert.h"
#include "base/base_macros.h"
#include "std/std_memset.h"
#include "mem/mem_arena.h"
#include "struct/struct_vectors.h"
#include "struct/struct_rectangles.h"
#include "struct/struct_quaternion.h"
#include "struct/struct_matrices.h"
#include "struct/struct_var_array.h"

#if BUILD_WITH_ODE

#define dSINGLE
#include "ode/ode.h"

#define MAX_NUM_GEOMETRIES_PER_BODY 4

typedef plex PhysicsBody PhysicsBody;
plex PhysicsBody
{
	uxx index;
	dBodyID bodyId;
	uxx numGeometries;
	dGeomID geomIds[MAX_NUM_GEOMETRIES_PER_BODY];
};

typedef plex PhysicsWorld PhysicsWorld;
plex PhysicsWorld
{
	Arena* arena;
	dWorldID world;
	dSpaceID space;
	dJointGroupID contactGroup;
	VarArray bodies; //PhysicsBody
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void ToOdeMatrix3(mat3 matrix, dReal* matrixOut);
	PIG_CORE_INLINE void ToOdeMatrix4(mat3 matrix, dReal* matrixOut);
	PIG_CORE_INLINE mat3 ToMat3FromOde(const dReal* matrixPntr);
	PIG_CORE_INLINE mat3 ToMat3FromOde4x3(const dReal* matrixPntr);
	PIG_CORE_INLINE mat4 ToMat4FromOde(const dReal* matrixPntr);
	PIG_CORE_INLINE mat4 ToMat4FromOde4x3(const dReal* matrixPntr);
	PhysicsWorld* InitPhysicsODE(Arena* arena, v3 gravity);
	PIG_CORE_INLINE PhysicsBody* SpawnPhysicsBox(PhysicsWorld* world, obb3 boundingBox, r32 density);
	PIG_CORE_INLINE void UpdatePhysics(PhysicsWorld* world, r32 simStepSize, r32 elapsedMs);
	PIG_CORE_INLINE void SetBodyPosition(PhysicsWorld* world, uxx bodyIndex, v3 position);
	PIG_CORE_INLINE void SetBodyRotation(PhysicsWorld* world, uxx bodyIndex, quat rotation);
	PIG_CORE_INLINE v3 GetBodyPosition(PhysicsWorld* world, uxx bodyIndex);
	PIG_CORE_INLINE quat GetBodyRotation(PhysicsWorld* world, uxx bodyIndex);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void ToOdeMatrix3(mat3 matrix, dReal* matrixOut)
{
	matrixOut[0] = matrix.Columns[0].X; matrixOut[1] = matrix.Columns[1].X; matrixOut[2] = matrix.Columns[2].X;
	matrixOut[4] = matrix.Columns[0].Y; matrixOut[5] = matrix.Columns[1].Y; matrixOut[6] = matrix.Columns[2].Y;
	matrixOut[8] = matrix.Columns[0].Z; matrixOut[9] = matrix.Columns[1].Z; matrixOut[10] = matrix.Columns[2].Z;
}
PEXPI void ToOdeMatrix4(mat3 matrix, dReal* matrixOut)
{
	matrixOut[0] = matrix.Columns[0].X; matrixOut[1] = matrix.Columns[1].X; matrixOut[2] = matrix.Columns[2].X; matrixOut[3] = matrix.Columns[3].X;
	matrixOut[4] = matrix.Columns[0].Y; matrixOut[5] = matrix.Columns[1].Y; matrixOut[6] = matrix.Columns[2].Y; matrixOut[7] = matrix.Columns[3].Y;
	matrixOut[8] = matrix.Columns[0].Z; matrixOut[9] = matrix.Columns[1].Z; matrixOut[10] = matrix.Columns[2].Z; matrixOut[11] = matrix.Columns[3].Z;
	matrixOut[12] = matrix.Columns[0].W; matrixOut[13] = matrix.Columns[1].W; matrixOut[14] = matrix.Columns[2].W; matrixOut[15] = matrix.Columns[3].W;
}
PEXPI mat3 ToMat3FromOde(const dReal* matrixPntr)
{
	return MakeMat3(
		matrixPntr[0], matrixPntr[1], matrixPntr[2],
		matrixPntr[3], matrixPntr[4], matrixPntr[5],
		matrixPntr[6], matrixPntr[7], matrixPntr[8]
	);
}
PEXPI mat3 ToMat3FromOde4x3(const dReal* matrixPntr)
{
	return MakeMat3(
		matrixPntr[0], matrixPntr[1], matrixPntr[2],
		matrixPntr[4], matrixPntr[5], matrixPntr[6],
		matrixPntr[8], matrixPntr[9], matrixPntr[10]
	);
}
PEXPI mat4 ToMat4FromOde(const dReal* matrixPntr)
{
	return MakeMat4(
		matrixPntr[0], matrixPntr[1], matrixPntr[2], matrixPntr[3],
		matrixPntr[4], matrixPntr[5], matrixPntr[6], matrixPntr[7],
		matrixPntr[8], matrixPntr[9], matrixPntr[10], matrixPntr[11],
		matrixPntr[12], matrixPntr[13], matrixPntr[14], matrixPntr[15]
	);
}
PEXPI mat4 ToMat4FromOde4x3(const dReal* matrixPntr)
{
	return MakeMat4(
		matrixPntr[0], matrixPntr[1], matrixPntr[2], matrixPntr[3],
		matrixPntr[4], matrixPntr[5], matrixPntr[6], matrixPntr[7],
		matrixPntr[8], matrixPntr[9], matrixPntr[10], matrixPntr[11],
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

PEXP PhysicsWorld* InitPhysicsODE(Arena* arena, v3 gravity)
{
	NotNull(arena);
	
	PhysicsWorld* result = AllocType(PhysicsWorld, arena);
	NotNull(result);
	ClearPointer(result);
	result->arena = arena;
	
	InitVarArray(PhysicsBody, &result->bodies, arena);
	
	dInitODE();
	
	result->world = dWorldCreate();
	dWorldSetData(result->world, (void*)result);
	dWorldSetQuickStepNumIterations(result->world, 20); //20 = dWORLDQUICKSTEP_ITERATION_COUNT_DEFAULT
	
	result->space = dHashSpaceCreate(0); //spaceId = 0
	
	result->contactGroup = dJointGroupCreate(0); //max_size = 0
	
	dWorldSetGravity(result->world, gravity.X, gravity.Y, gravity.Z);
	//TODO: dWorldSetERP? (Error Reduction Parameter)
	dWorldSetCFM(result->world, 1e-5f); //CFM = Constraint Force Mixing (this is the default for 32-bit floats)
	// dWorldSetAutoDisableFlag(result->world, true); //bodies will auto-disable after they stop moving for some time
	// dWorldSetAutoDisableAverageSamplesCount(result->world, 1); //instantaneous velocity is used for auto-disabling
	dWorldSetContactMaxCorrectingVel(result->world, 0.1f); //maximum correcting velocity that objects are allowed
	dWorldSetContactSurfaceLayer(result->world, 0.001f); //depth of surface layer around all geometry objects
	
	dGeomID planeId = dCreatePlane(result->space, 0, 1, 0, 0); //x, y, z, dist
	
	return result;
}

PEXPI uxx SpawnPhysicsBox(PhysicsWorld* world, obb3 boundingBox, r32 density)
{
	NotNull(world);
	NotNull(world->arena);
	
	PhysicsBody* newBody = VarArrayAdd(PhysicsBody, &world->bodies);
	NotNull(newBody);
	ClearPointer(newBody);
	newBody->index = world->bodies.length-1;
	newBody->bodyId = dBodyCreate(world->world);
	dBodySetData(newBody->bodyId, newBody); //TODO: This won't work since it lives in a VarArray, the pointer will change if too many items are allocated
	
	dBodySetPosition(newBody->bodyId, boundingBox.Center.X, boundingBox.Center.Y, boundingBox.Center.Z);
	dMatrix3 rotationMatrix;
	mat4 rotationMat4 = ToMat4FromQuat(boundingBox.Rotation);
	mat3 rotationMat3 = ToMat3From4(rotationMat4);
	ToOdeMatrix3(rotationMat3, &rotationMatrix[0]);
	dBodySetRotation(newBody->bodyId, rotationMatrix);
	
	dMass boxMass;
	dMassSetBox(&boxMass, density, boundingBox.Width, boundingBox.Height, boundingBox.Depth);
	dGeomID geomId = dCreateBox(world->space, boundingBox.Width, boundingBox.Height, boundingBox.Depth);
	dGeomSetBody(geomId, newBody->bodyId);
	newBody->geomIds[newBody->numGeometries] = geomId;
	newBody->numGeometries++;
	
	return newBody->index;
}

#define PHYS_ODE_MAX_CONTACTS 64
static void OdeCollisionNearColllback(void* data, dGeomID obj1, dGeomID obj2)
{
	NotNull(data);
	PhysicsWorld* world = (PhysicsWorld*)data;
	
	// exit without doing anything if the two bodies are connected by a joint
	dBodyID body1 = dGeomGetBody(obj1);
	dBodyID body2 = dGeomGetBody(obj2);
	if (body1 && body2 && dAreConnectedExcluding(body1, body2, dJointTypeContact)) { return; }
	
	dContact contact[PHYS_ODE_MAX_CONTACTS];   // up to PHYS_ODE_MAX_CONTACTS contacts per box-box
	ClearArray(contact);
	for (int cIndex = 0; cIndex < PHYS_ODE_MAX_CONTACTS; cIndex++)
	{
		contact[cIndex].surface.mode = dContactSoftERP | dContactSoftCFM | dContactApprox1 | dContactSlip1 | dContactSlip2;
		contact[cIndex].surface.slip1 = 0.7f;
		contact[cIndex].surface.slip2 = 0.7f;
		contact[cIndex].surface.mu = 50; // was: dInfinity
		contact[cIndex].surface.mu2 = 0;
		contact[cIndex].surface.soft_erp = 0.96f;
		contact[cIndex].surface.soft_cfm = 0.04f; //was 0.01f
		// contact[cIndex].surface.bounce = 0.1f;
		// contact[cIndex].surface.bounce_vel = 0.1f;
	}
	
	int numc = dCollide(obj1, obj2, PHYS_ODE_MAX_CONTACTS, &contact[0].geom, sizeof(dContact));
	if (numc)
	{
		for (int cIndex = 0; cIndex < numc; cIndex++)
		{
			dJointID newJoint = dJointCreateContact(world->world, world->contactGroup, &contact[cIndex]);
			dJointAttach(newJoint, body1, body2);
		}
	}
}

PEXPI void UpdatePhysics(PhysicsWorld* world, r32 simStepSize, r32 elapsedMs)
{
	NotNull(world);
	NotNull(world->arena);
	Assert(elapsedMs >= 0);
	uxx numSteps = (uxx)CeilR32i(elapsedMs / simStepSize);
	for (uxx sIndex = 0; sIndex < numSteps; sIndex++)
	{
		dSpaceCollide(world->space, (void*)world, OdeCollisionNearColllback);
		int stepResult = dWorldQuickStep(world->world, simStepSize / 1000.0f);
		Assert(stepResult == 1);
		dJointGroupEmpty(world->contactGroup);
	}
}

PEXPI void SetBodyPosition(PhysicsWorld* world, uxx bodyIndex, v3 position)
{
	Assert(bodyIndex < world->bodies.length);
	PhysicsBody* body = VarArrayGetHard(PhysicsBody, &world->bodies, bodyIndex);
	dBodySetPosition(body->bodyId, position.X, position.Y, position.Z);
}
PEXPI void SetBodyRotation(PhysicsWorld* world, uxx bodyIndex, quat rotation)
{
	Assert(bodyIndex < world->bodies.length);
	PhysicsBody* body = VarArrayGetHard(PhysicsBody, &world->bodies, bodyIndex);
	const dReal* bodyRotationFloats = dBodyGetRotation(body->bodyId);
	mat4 rotationMat4 = ToMat4FromQuat(rotation);
	mat3 rotationMat3 = ToMat3From4(rotationMat4);
	dMatrix3 rotationMatOde = ZEROED;
	ToOdeMatrix3(rotationMat3, &rotationMatOde[0]);
	dBodySetRotation(body->bodyId, &rotationMatOde[0]);
}

PEXPI v3 GetBodyPosition(PhysicsWorld* world, uxx bodyIndex)
{
	Assert(bodyIndex < world->bodies.length);
	PhysicsBody* body = VarArrayGetHard(PhysicsBody, &world->bodies, bodyIndex);
	const dReal* bodyPositionFloats = dBodyGetPosition(body->bodyId);
	return MakeV3(bodyPositionFloats[0], bodyPositionFloats[1], bodyPositionFloats[2]);
}
PEXPI quat GetBodyRotation(PhysicsWorld* world, uxx bodyIndex)
{
	Assert(bodyIndex < world->bodies.length);
	PhysicsBody* body = VarArrayGetHard(PhysicsBody, &world->bodies, bodyIndex);
	const dReal* bodyRotationFloats = dBodyGetRotation(body->bodyId);
	mat3 rotationMat = ToMat3FromOde4x3(bodyRotationFloats);
	return QuatFromMat3(rotationMat);
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_ODE

#endif //  _PHYS_ODE_H
