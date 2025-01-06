/*
File:   tests_box2d.c
Author: Taylor Robbins
Date:   01\06\2025
Description: 
	** Holds some functions that simulate a little Box2D test scene with a static
	** floor body, one box that spawns above the floor, and support for spawning
	** new boxes by left clicking anywhere on screen.
*/

#if BUILD_WITH_BOX2D

#define PHYS_WORLD_SCALE_X  40.0f
#define PHYS_WORLD_SCALE_Y  -40.0f
#define PHYS_WORLD_OFFSET_X 400.0f
#define PHYS_WORLD_OFFSET_Y 550.0f
#define NUM_PHYS_SUB_STEPS  4

b2WorldId physWorld;

void GetPhysRenderPos(r32 inX, r32 inY, i32* outX, i32* outY)
{
	*outX = (i32)(inX * PHYS_WORLD_SCALE_X + PHYS_WORLD_OFFSET_X);
	*outY = (i32)(inY * PHYS_WORLD_SCALE_Y + PHYS_WORLD_OFFSET_Y);
}
void GetPhysPosFromRenderPos(i32 inX, i32 inY, r32* outX, r32* outY)
{
	*outX = ((r32)inX - PHYS_WORLD_OFFSET_X) / PHYS_WORLD_SCALE_X;
	*outY = ((r32)inY - PHYS_WORLD_OFFSET_Y) / PHYS_WORLD_SCALE_Y;
}

#include "tests/tests_raylib_box2d_render.c"

void SpawnBox(r32 x, r32 y, r32 width, r32 height)
{
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	// bodyDef.position = { -55.0f, 13.5f };
	bodyDef.position.x = x;
	bodyDef.position.y = y;
	b2BodyId bodyId = b2CreateBody(physWorld, &bodyDef);

	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.density = 1.0f;
	shapeDef.friction = 0.5f;
	shapeDef.restitution = 0.9f;

	b2Polygon box = b2MakeBox(width, height);
	b2ShapeId bodyShapeId = b2CreatePolygonShape(bodyId, &shapeDef, &box);
}

void InitBox2DTest()
{
	b2WorldDef physWorldDef = b2DefaultWorldDef();
	physWorld = b2CreateWorld(&physWorldDef);
	
	// Ground body
	{
		b2BodyDef bodyDef = b2DefaultBodyDef();
		b2BodyId groundId = b2CreateBody(physWorld, &bodyDef);

		b2ShapeDef shapeDef = b2DefaultShapeDef();
		b2Segment segment = { { -20.0f, 0.0f }, { 20.0f, 0.0f } };
		b2CreateSegmentShape(groundId, &shapeDef, &segment);
	}
	
	SpawnBox(0.0f, 13.5f, 0.5f, 0.5f);
	
	#if BUILD_WITH_RAYLIB
	InitRaylibBox2DRender();
	#endif
}

void UpdateBox2DTest()
{
	#if BUILD_WITH_RAYLIB
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		r32 physMouseX, physMouseY;
		GetPhysPosFromRenderPos(GetMouseX(), GetMouseY(), &physMouseX, &physMouseY);
		SpawnBox(physMouseX, physMouseY, GetRandR32Range(mainRandom, 0.3f, 1.0f), GetRandR32Range(mainRandom, 0.3f, 1.0f));
	}
	#endif
	
	#if BUILD_WITH_RAYLIB
	b2World_Step(physWorld, GetFrameTime(), NUM_PHYS_SUB_STEPS);
	#else
	b2World_Step(physWorld, 1/60.0f, NUM_PHYS_SUB_STEPS);
	#endif
}

void RenderBox2DTest()
{
	#if BUILD_WITH_RAYLIB
	b2World_Draw(physWorld, &physDebugDraw);
	#endif //BUILD_WITH_RAYLIB
}

#endif //BUILD_WITH_BOX2D
