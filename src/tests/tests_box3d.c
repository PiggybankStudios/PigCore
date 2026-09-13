/*
File:   tests_box3d.c
Author: Taylor Robbins
Date:   09\13\2026
Description: 
	** Holds some functions that simulate a simple Box3D scene
*/

#if BUILD_WITH_BOX3D

#define NUM_PHYS3D_SUB_STEPS  4

b3WorldId physWorld3D;

#include "tests/tests_box3d_renderers.c"

void SpawnBox3D(r32 x, r32 y, r32 z, r32 width, r32 height, r32 depth)
{
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.position = MakeB3Vec3(x, y, z);
	b3BodyId bodyId = b3CreateBody(physWorld3D, &bodyDef);

	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = 1.0f;
	shapeDef.baseMaterial.friction = 0.5f;
	shapeDef.baseMaterial.restitution = 0.5f;

	b3BoxHull boxHull = b3MakeBoxHull(width, height, depth);
	b3ShapeId bodyShapeId = b3CreateHullShape(bodyId, &shapeDef, &boxHull.base);
	UNUSED(bodyShapeId);
}

void InitBox3DTest()
{
	b3WorldDef physWorldDef = b3DefaultWorldDef();
	#if BUILD_WITH_SOKOL_GFX
	physWorldDef.createDebugShape = DebugBox3d_Sokol_CreateDebugShapeCallback;
	physWorldDef.destroyDebugShape = DebugBox3d_Sokol_DestroyDebugShapeCallback;
	physWorldDef.userDebugShapeContext = (void*)stdHeap;
	#endif
	physWorld3D = b3CreateWorld(&physWorldDef);
	
	// Ground body
	{
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = b3_staticBody;
		bodyDef.position.y = -2.0f;
		b3BodyId groundId = b3CreateBody(physWorld3D, &bodyDef);

		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3BoxHull boxHull = b3MakeBoxHull(12.0f, 1.0f, 12.0f);
		b3CreateHullShape(groundId, &shapeDef, &boxHull.base);
	}
	
	SpawnBox3D(2,10.0f,1, 0.3f,0.4f,0.3f);
	SpawnBox3D(2,11.5f,1, 0.2f,0.3f,0.2f);
	SpawnBox3D(2,13.0f,1, 0.3f,0.3f,0.5f);
	
	#if BUILD_WITH_SOKOL_GFX
	InitSokolBox3DRender();
	#endif
}

void UpdateBox3DTest(MouseState* mouseState, v3 cameraPos)
{
	if (IsMouseBtnPressed(mouseState, nullptr, MouseBtn_Left))
	{
		SpawnBox3D(cameraPos.x,cameraPos.y,cameraPos.z, GetRandR32Range(mainRandom, 0.2f, 0.8f),GetRandR32Range(mainRandom, 0.2f, 0.8f),GetRandR32Range(mainRandom, 0.2f, 0.8f));
	}
	
	b3World_Step(physWorld3D, 1/60.0f, NUM_PHYS3D_SUB_STEPS);
}

void RenderBox3DTest()
{
	#if BUILD_WITH_SOKOL_GFX
	b3World_Draw(physWorld3D, &physDebugDraw3D, B3_DEFAULT_MASK_BITS);
	#endif
}

#endif //BUILD_WITH_BOX3D
