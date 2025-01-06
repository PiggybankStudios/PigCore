/*
File:   tests_main.c
Author: Taylor Robbins
Date:   12\31\2024
Description: 
	** Holds the main entry point for tests.exe, a program that relies on a variety
	** of things inside this repository, thus by building it we can make sure the
	** code inside this repository is in a good state without needing an external
	** project, and when run it does a variety of tests to make sure things are
	** working as intended.
*/

#define MEM_ARENA_DEBUG_NAMES           1
#define VAR_ARRAY_DEBUG_INFO            1
#define VAR_ARRAY_CLEAR_ITEMS_ON_ADD    1
#define VAR_ARRAY_CLEAR_ITEM_BYTE_VALUE 0xCC

#include "build_config.h"

#include "base/base_all.h"
#include "std/std_all.h"
#include "os/os_all.h"
#include "mem/mem_all.h"
#include "struct/struct_all.h"
#include "misc/misc_all.h"

#if BUILD_WITH_RAYLIB
#include "third_party/raylib/include/raylib.h"
#endif

#if BUILD_WITH_BOX2D
#include "third_party/box2d/box2d.h"
#endif

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
Arena* scratchArenas[2] = ZEROED;

// +--------------------------------------------------------------+
// |                           Helpers                            |
// +--------------------------------------------------------------+
#if 0
void PrintArena(Arena* arena)
{
	if (arena->committed > 0)
	{
		MyPrint("%s %llu/%llu (%llu virtual) - %llu allocations", arena->debugName, arena->used, arena->committed, arena->size, arena->allocCount);
	}
	else
	{
		MyPrint("%s %llu/%llu - %llu allocations", arena->debugName, arena->used, arena->size, arena->allocCount);
	}
}

void PrintVarArray(VarArray* array)
{
	#if VAR_ARRAY_DEBUG_INFO
	const char* creationFileName = array->creationFilePath;
	for (u64 cIndex = 0; array->creationFilePath != nullptr && array->creationFilePath[cIndex] != '\0'; cIndex++) { char c = array->creationFilePath[cIndex]; if (c == '/' || c == '\\') { creationFileName = &array->creationFilePath[cIndex+1]; } }
	MyPrint("VarArray %llu/%llu items (itemSize=%llu, items=%p, from %s:%llu %s)", array->length, array->allocLength, array->itemSize, array->items, creationFileName, array->creationLineNumber, array->creationFuncName);
	#else
	MyPrint("VarArray %llu/%llu items (itemSize=%llu, items=%p)", array->length, array->allocLength, array->itemSize, array->items);
	#endif
}
void PrintNumbers(VarArray* array)
{
	MyPrintNoLine("[%llu]{", array->length);
	VarArrayLoop(array, nIndex)
	{
		VarArrayLoopGetValue(u32, num, array, nIndex);
		MyPrintNoLine(" %u", num);
	}
	MyPrint(" }");
}
#endif

#if BUILD_WITH_BOX2D && BUILD_WITH_RAYLIB
Color RaylibColorFromB2HexColor(b2HexColor b2Color)
{
	Color result = ZEROED;
	result.r = (u8)(((u32)b2Color >> 16) & 0xFF);
	result.g = (u8)(((u32)b2Color >> 8) & 0xFF);
	result.b = (u8)(((u32)b2Color >> 0) & 0xFF);
	result.a = 255;
	return result;
}

const r32 physWorldScaleX = 40.0f;
const r32 physWorldScaleY = -40.0f;
const r32 physWorldOffsetX = 400.0f;
const r32 physWorldOffsetY = 550.0f;
void GetPhysRenderPos(r32 inX, r32 inY, i32* outX, i32* outY)
{
	*outX = (i32)(inX * physWorldScaleX + physWorldOffsetX);
	*outY = (i32)(inY * physWorldScaleY + physWorldOffsetY);
}
void GetPhysPosFromRenderPos(i32 inX, i32 inY, r32* outX, r32* outY)
{
	*outX = ((r32)inX - physWorldOffsetX) / physWorldScaleX;
	*outY = ((r32)inY - physWorldOffsetY) / physWorldScaleY;
}

void DebugBox2d_DrawPolygon(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context)
{
	// static int a = 0; MyPrint("DrawPolygon[%d]", a++);
	for (int vIndex = 0; vIndex < vertexCount; vIndex++)
	{
		i32 v1X, v1Y;
		i32 v2X, v2Y;
		GetPhysRenderPos(vertices[vIndex].x, vertices[vIndex].y, &v1X, &v1Y);
		GetPhysRenderPos(vertices[(vIndex+1)%vertexCount].x, vertices[(vIndex+1)%vertexCount].y, &v2X, &v2Y);
		DrawLine(v1X, v1Y, v2X, v2Y, RaylibColorFromB2HexColor(color));
	}
}

void DebugBox2d_DrawSolidPolygon(b2Transform transform, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* context)
{
	// static int a = 0; MyPrint("DrawSolidPolygon[%d]", a++);
	Arena* scratch = scratchArenas[0];
	uxx scratchMark = ArenaGetMark(scratch);
	if (vertexCount > 0)
	{
		Vector2* convertedVertices = (Vector2*)AllocMem(scratch, sizeof(Vector2) * vertexCount);
		NotNull(convertedVertices);
		for (i32 vIndex = 0; vIndex < vertexCount; vIndex++)
		{
			b2Vec2 transformedVertex = b2TransformPoint(transform, vertices[vIndex]);
			int vX, vY;
			GetPhysRenderPos(transformedVertex.x, transformedVertex.y, &vX, &vY);
			convertedVertices[vIndex].x = (r32)vX;
			convertedVertices[vIndex].y = (r32)vY;
		}
		DrawTriangleFan(convertedVertices, vertexCount, RaylibColorFromB2HexColor(color));
	}
	ArenaResetToMark(scratch, scratchMark);
}

void DebugBox2d_DrawCircle(b2Vec2 center, float radius, b2HexColor color, void* context)
{
	// static int a = 0; MyPrint("DrawCircle[%d]", a++);
	int centerX, centerY;
	GetPhysRenderPos(center.x, center.y, &centerX, &centerY);
	DrawCircleLines(centerX, centerY, radius, RaylibColorFromB2HexColor(color));
}

void DebugBox2d_DrawSolidCircle(b2Transform transform, float radius, b2HexColor color, void* context)
{
	// static int a = 0; MyPrint("DrawSolidCircle[%d]", a++);
	//TODO: Take into account the transform.r!
	int centerX, centerY;
	GetPhysRenderPos(transform.p.x, transform.p.y, &centerX, &centerY);
	DrawCircle(centerX, centerY, radius, RaylibColorFromB2HexColor(color));
}

void DebugBox2d_DrawCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context)
{
	static int a = 0; MyPrint("DrawCapsule[%d]", a++);
	//TODO: Implement me!
}

void DebugBox2d_DrawSolidCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context)
{
	static int a = 0; MyPrint("DrawSolidCapsule[%d]", a++);
	//TODO: Implement me!
}

void DebugBox2d_DrawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context)
{
	// static int a = 0; MyPrint("DrawSegment[%d]", a++);
	int p1X, p1Y;
	GetPhysRenderPos(p1.x, p1.y, &p1X, &p1Y);
	int p2X, p2Y;
	GetPhysRenderPos(p2.x, p2.y, &p2X, &p2Y);
	DrawLine(p1X, p1Y, p2X, p2Y, RaylibColorFromB2HexColor(color));
}

void DebugBox2d_DrawTransform(b2Transform transform, void* context)
{
	static int a = 0; MyPrint("DrawTransform[%d]", a++);
	//TODO: Implement me!
}

void DebugBox2d_DrawPoint(b2Vec2 p, float size, b2HexColor color, void* context)
{
	static int a = 0; MyPrint("DrawPoint[%d]", a++);
	//TODO: Implement me!
}

void DebugBox2d_DrawString(b2Vec2 p, const char* s, void* context)
{
	// static int a = 0; MyPrint("DrawString[%d]", a++);
	int textX, textY;
	GetPhysRenderPos(p.x, p.y, &textX, &textY);
	DrawText(s, textX, textY, 10, DARKGRAY);
}

void SpawnBox(b2WorldId physWorld, r32 x, r32 y, r32 width, r32 height)
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
#endif

// +--------------------------------------------------------------+
// |                             Main                             |
// +--------------------------------------------------------------+
int main()
{
	MyPrint("Running tests...\n");
	
	#if 1
	{
		#if COMPILER_IS_MSVC
		MyPrint("Compiled by MSVC");
		#endif
		#if COMPILER_IS_CLANG
		MyPrint("Compiled by Clang");
		#endif
		#if COMPILER_IS_GCC
		MyPrint("Compiled by GCC");
		#endif
		#if TARGET_IS_WINDOWS
		MyPrint("Running on Windows");
		#endif
		#if TARGET_IS_LINUX
		MyPrint("Running on Linux");
		#endif
		#if TARGET_IS_OSX
		MyPrint("Running on OSX");
		#endif
	}
	#endif
	
	#if 1
	Arena stdHeap = ZEROED;
	InitArenaStdHeap(&stdHeap);
	Arena stdAlias = ZEROED;
	InitArenaAlias(&stdAlias, &stdHeap);
	u8 arenaBuffer1[256] = ZEROED;
	Arena bufferArena = ZEROED;
	InitArenaBuffer(&bufferArena, arenaBuffer1, ArrayCount(arenaBuffer1));
	Arena scratch1 = ZEROED;
	InitArenaStackVirtual(&scratch1, Gigabytes(4));
	Arena scratch2 = ZEROED;
	InitArenaStackVirtual(&scratch2, Gigabytes(4));
	scratchArenas[0] = &scratch1;
	scratchArenas[1] = &scratch2;
	#endif
	
	#if 1
	RandomSeries random;
	InitRandomSeriesDefault(&random);
	SeedRandomSeriesU64(&random, 42); //TODO: Actually seed the random number generator!
	#endif
	
	#if 0
	{
		u32* allocatedInt1 = AllocMem(&stdHeap, sizeof(u32));
		MyPrint("allocatedInt1: %p", allocatedInt1);
		PrintArena(&stdHeap);
		u32* allocatedInt2 = AllocMem(&stdAlias, sizeof(u32));
		MyPrint("allocatedInt2: %p", allocatedInt2);
		PrintArena(&stdHeap);
		FreeMem(&stdAlias, allocatedInt1, sizeof(u32));
		PrintArena(&stdHeap);
		u32* allocatedInt3 = AllocMem(&stdAlias, sizeof(u32));
		MyPrint("allocatedInt3: %p", allocatedInt3);
		PrintArena(&stdHeap);
		
		u32* allocatedInt4 = AllocMem(&bufferArena, sizeof(u32));
		MyPrint("allocatedInt4: %p", allocatedInt4);
		PrintArena(&bufferArena);
		u32* allocatedInt5 = AllocMem(&bufferArena, sizeof(u32));
		MyPrint("allocatedInt5: %p", allocatedInt5);
		PrintArena(&bufferArena);
		FreeMem(&bufferArena, allocatedInt5, sizeof(u32));
		PrintArena(&bufferArena);
		u32* allocatedInt6 = AllocMem(&bufferArena, sizeof(u32));
		MyPrint("allocatedInt6: %p", allocatedInt6);
		PrintArena(&bufferArena);
		
		u64 mark1 = ArenaGetMark(&scratch1);
		PrintArena(&scratch1);
		u32* num1 = (u32*)AllocMem(&scratch1, sizeof(u32));
		MyPrint("num1 %p", num1);
		PrintArena(&scratch1);
		u32* num2 = (u32*)AllocMem(&scratch1, sizeof(u32));
		MyPrint("num2 %p", num2);
		PrintArena(&scratch1);
		u32* num3 = (u32*)AllocMem(&scratch1, sizeof(u32));
		MyPrint("num3 %p", num3);
		PrintArena(&scratch1);
		FreeMem(&scratch1, num3, sizeof(u32));
		PrintArena(&scratch1);
		ArenaResetToMark(&scratch1, mark1);
		PrintArena(&scratch1);
	}
	#endif
	
	#if TARGET_IS_OSX
	MTLCreateSystemDefaultDevice();
	#endif
	
	#if 0
	{
		VarArray array1;
		InitVarArrayWithInitial(u32, &array1, &stdHeap, 89);
		PrintVarArray(&array1);
		PrintNumbers(&array1);
		
		VarArray array2;
		InitVarArray(u32, &array2, &stdHeap);
		PrintVarArray(&array2);
		PrintNumbers(&array2);
		array2.maxLength = 3;
		
		u32* numPntr1 = VarArrayAdd(u32, &array2);
		*numPntr1 = 7;
		PrintVarArray(&array2);
		PrintNumbers(&array2);
		
		VarArrayAddValue(u32, &array2, 9);
		PrintVarArray(&array2);
		PrintNumbers(&array2);
		
		VarArrayPush(u32, &array2, VarArrayGetValue(u32, &array2, 1));
		PrintVarArray(&array2);
		PrintNumbers(&array2);
		
		u32* numPntr2 = VarArrayAdd(u32, &array2);
		if (numPntr2 != nullptr) { *numPntr2 = 42; }
		PrintVarArray(&array2);
		PrintNumbers(&array2);
		
		VarArray array3;
		VarArrayCopy(&array3, &array2, &stdHeap);
		
		u32 removedNum = VarArrayGetAndRemoveValueAt(u32, &array2, 2);
		MyPrint("Removed array[2] = %u", removedNum);
		PrintVarArray(&array2);
		PrintNumbers(&array2);
		
		VarArrayInsertValue(u32, &array2, 1, VarArrayGetValue(u32, &array2, 0) * VarArrayGetValue(u32, &array2, 1));
		PrintVarArray(&array2);
		PrintNumbers(&array2);
		
		VarArrayRemoveAt(u32, &array2, 1);
		PrintVarArray(&array2);
		PrintNumbers(&array2);
		
		numPntr1 = VarArrayGet(u32, &array2, 0); //refresh our pointer to the value
		VarArrayRemove(u32, &array2, numPntr1);
		PrintVarArray(&array2);
		PrintNumbers(&array2);
		
		FreeVarArray(&array2);
		PrintVarArray(&array2);
		
		PrintVarArray(&array3);
		PrintNumbers(&array3);
	}
	#endif
	
	#if BUILD_WITH_BOX2D
	b2WorldId physWorld;
	b2DebugDraw physDebugDraw = ZEROED;
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
		
		SpawnBox(physWorld, 0.0f, 13.5f, 0.5f, 0.5f);
		
		physDebugDraw.DrawPolygon = DebugBox2d_DrawPolygon;
		physDebugDraw.DrawSolidPolygon = DebugBox2d_DrawSolidPolygon;
		physDebugDraw.DrawCircle = DebugBox2d_DrawCircle;
		physDebugDraw.DrawSolidCircle = DebugBox2d_DrawSolidCircle;
		physDebugDraw.DrawCapsule = DebugBox2d_DrawCapsule;
		physDebugDraw.DrawSolidCapsule = DebugBox2d_DrawSolidCapsule;
		physDebugDraw.DrawSegment = DebugBox2d_DrawSegment;
		physDebugDraw.DrawTransform = DebugBox2d_DrawTransform;
		physDebugDraw.DrawPoint = DebugBox2d_DrawPoint;
		physDebugDraw.DrawString = DebugBox2d_DrawString;
		physDebugDraw.drawingBounds.lowerBound.x = 0.0f;
		physDebugDraw.drawingBounds.lowerBound.y = 0.0f;
		physDebugDraw.drawingBounds.upperBound.x = 0.0f;
		physDebugDraw.drawingBounds.upperBound.y = 0.0f;
		physDebugDraw.useDrawingBounds = false;
		physDebugDraw.drawShapes = true;
		physDebugDraw.drawJoints = true;
		physDebugDraw.drawJointExtras = false;
		physDebugDraw.drawAABBs = false;
		physDebugDraw.drawMass = false;
		physDebugDraw.drawContacts = false;
		physDebugDraw.drawGraphColors = false;
		physDebugDraw.drawContactNormals = false;
		physDebugDraw.drawContactImpulses = false;
		physDebugDraw.drawFrictionImpulses = false;
		physDebugDraw.context = nullptr;
	}
	#endif
	
	#if BUILD_WITH_RAYLIB
	{
		InitWindow(800, 600, "Tests (Pigglen)");
		SetWindowMinSize(400, 200);
		SetWindowState(FLAG_WINDOW_RESIZABLE);
		SetTargetFPS(60);
		while (!WindowShouldClose())
		{
			int windowWidth = GetRenderWidth();
			int windowHeight = GetRenderHeight();
			BeginDrawing();
			ClearBackground(RAYWHITE);
			const char* textStr = "Congrats! You created your first window!";
			const int textSize = 20;
			int textWidth = MeasureText(textStr, textSize);
			DrawText(textStr, windowWidth/2 - textWidth/2, windowHeight/2 - textSize/2, textSize, LIGHTGRAY);
			
			#if BUILD_WITH_BOX2D
			{
				if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
				{
					r32 physMouseX, physMouseY;
					GetPhysPosFromRenderPos(GetMouseX(), GetMouseY(), &physMouseX, &physMouseY);
					SpawnBox(physWorld, physMouseX, physMouseY, GetRandR32Range(&random, 0.3f, 1.0f), GetRandR32Range(&random, 0.3f, 1.0f));
				}
				
				const int numPhysSubSteps = 4;
				b2World_Step(physWorld, GetFrameTime(), numPhysSubSteps);
				
				b2World_Draw(physWorld, &physDebugDraw);
			}
			#endif
			
			EndDrawing();
		}
		CloseWindow();
	}
	#endif
	
	MyPrint("All tests completed successfully!");
	return 0;
}
