/*
File:   tests_box2d_renderers.c
Author: Taylor Robbins
Date:   01\06\2025
Description:
	** Contains a b2DebugDraw definition for Box2D that uses Raylib or Sokol Gfx to render
	** NOTE: #included from tests_box2d.c
*/

#if BUILD_WITH_BOX2D
b2DebugDraw physDebugDraw = ZEROED;
#endif

// +--------------------------------------------------------------+
// |                       Raylib Renderer                        |
// +--------------------------------------------------------------+
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

void DebugBox2d_Raylib_DrawPolygon(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context)
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

void DebugBox2d_Raylib_DrawSolidPolygon(b2Transform transform, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* context)
{
	// static int a = 0; MyPrint("DrawSolidPolygon[%d]", a++);
	ScratchBegin(scratch);
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
	ScratchEnd(scratch);
}

void DebugBox2d_Raylib_DrawCircle(b2Vec2 center, float radius, b2HexColor color, void* context)
{
	// static int a = 0; MyPrint("DrawCircle[%d]", a++);
	int centerX, centerY;
	GetPhysRenderPos(center.x, center.y, &centerX, &centerY);
	DrawCircleLines(centerX, centerY, radius, RaylibColorFromB2HexColor(color));
}

void DebugBox2d_Raylib_DrawSolidCircle(b2Transform transform, float radius, b2HexColor color, void* context)
{
	// static int a = 0; MyPrint("DrawSolidCircle[%d]", a++);
	//TODO: Take into account the transform.r!
	int centerX, centerY;
	GetPhysRenderPos(transform.p.x, transform.p.y, &centerX, &centerY);
	DrawCircle(centerX, centerY, radius, RaylibColorFromB2HexColor(color));
}

void DebugBox2d_Raylib_DrawCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context)
{
	static int a = 0; MyPrint("DrawCapsule[%d]", a++);
	//TODO: Implement me!
}

void DebugBox2d_Raylib_DrawSolidCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context)
{
	static int a = 0; MyPrint("DrawSolidCapsule[%d]", a++);
	//TODO: Implement me!
}

void DebugBox2d_Raylib_DrawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context)
{
	// static int a = 0; MyPrint("DrawSegment[%d]", a++);
	int p1X, p1Y;
	GetPhysRenderPos(p1.x, p1.y, &p1X, &p1Y);
	int p2X, p2Y;
	GetPhysRenderPos(p2.x, p2.y, &p2X, &p2Y);
	DrawLine(p1X, p1Y, p2X, p2Y, RaylibColorFromB2HexColor(color));
}

void DebugBox2d_Raylib_DrawTransform(b2Transform transform, void* context)
{
	static int a = 0; MyPrint("DrawTransform[%d]", a++);
	//TODO: Implement me!
}

void DebugBox2d_Raylib_DrawPoint(b2Vec2 p, float size, b2HexColor color, void* context)
{
	static int a = 0; MyPrint("DrawPoint[%d]", a++);
	//TODO: Implement me!
}

void DebugBox2d_Raylib_DrawString(b2Vec2 p, const char* s, void* context)
{
	// static int a = 0; MyPrint("DrawString[%d]", a++);
	int textX, textY;
	GetPhysRenderPos(p.x, p.y, &textX, &textY);
	DrawText(s, textX, textY, 10, DARKGRAY);
}

void InitRaylibBox2DRender()
{
	physDebugDraw.DrawPolygon = DebugBox2d_Raylib_DrawPolygon;
	physDebugDraw.DrawSolidPolygon = DebugBox2d_Raylib_DrawSolidPolygon;
	physDebugDraw.DrawCircle = DebugBox2d_Raylib_DrawCircle;
	physDebugDraw.DrawSolidCircle = DebugBox2d_Raylib_DrawSolidCircle;
	physDebugDraw.DrawCapsule = DebugBox2d_Raylib_DrawCapsule;
	physDebugDraw.DrawSolidCapsule = DebugBox2d_Raylib_DrawSolidCapsule;
	physDebugDraw.DrawSegment = DebugBox2d_Raylib_DrawSegment;
	physDebugDraw.DrawTransform = DebugBox2d_Raylib_DrawTransform;
	physDebugDraw.DrawPoint = DebugBox2d_Raylib_DrawPoint;
	physDebugDraw.DrawString = DebugBox2d_Raylib_DrawString;
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

#endif //BUILD_WITH_BOX2D && BUILD_WITH_RAYLIB

// +--------------------------------------------------------------+
// |                        Sokol Renderer                        |
// +--------------------------------------------------------------+
#if BUILD_WITH_BOX2D && BUILD_WITH_SOKOL_GFX

Color32 Color32FromB2HexColor(b2HexColor b2Color)
{
	Color32 result = ZEROED;
	result.r = (u8)(((u32)b2Color >> 16) & 0xFF);
	result.g = (u8)(((u32)b2Color >> 8) & 0xFF);
	result.b = (u8)(((u32)b2Color >> 0) & 0xFF);
	result.a = 255;
	return result;
}

void DebugBox2d_Sokol_DrawPolygon(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context)
{
	for (int vIndex = 0; vIndex < vertexCount; vIndex++)
	{
		int vX, vY;
		GetPhysRenderPos(vertices[vIndex].x, vertices[vIndex].y, &vX, &vY);
		DrawRectangle(NewRecCentered((r32)vX, (r32)vY, 2, 2), Color32FromB2HexColor(color));
	}
}
void DebugBox2d_Sokol_DrawSolidPolygon(b2Transform transform, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* context)
{
	ScratchBegin(scratch);
	bool isRectangle = (vertexCount == 4 && AbsR32(DotV2(
		MakeV2(vertices[1].x - vertices[0].x, vertices[1].y - vertices[0].y),
		MakeV2(vertices[3].x - vertices[0].x, vertices[3].y - vertices[0].y))) <= 0.01f);
	
	v2* verticesV2 = AllocArray(v2, scratch, (uxx)vertexCount);
	for (int vIndex = 0; vIndex < vertexCount; vIndex++)
	{
		b2Vec2 transformedVertex = b2TransformPoint(transform, vertices[vIndex]);
		int vX, vY; GetPhysRenderPos(transformedVertex.x, transformedVertex.y, &vX, &vY);
		verticesV2[vIndex] = MakeV2((r32)vX, (r32)vY);
	}
	
	if (isRectangle)
	{
		v2 center = Mul(Add(verticesV2[0], verticesV2[2]), 0.5f);
		r32 width = LengthV2(Sub(verticesV2[1], verticesV2[0]));
		r32 height = LengthV2(Sub(verticesV2[3], verticesV2[0]));
		r32 rotation = AtanR32(verticesV2[1].Y - verticesV2[0].Y, verticesV2[1].X - verticesV2[0].X);
		DrawObb2(NewObb2(center.X, center.Y, width, height, rotation), Color32FromB2HexColor(color));
	}
	else
	{
		for (int vIndex = 0; vIndex < vertexCount; vIndex++)
		{
			DrawRectangle(NewRecCentered(verticesV2[vIndex].X, verticesV2[vIndex].Y, 2, 2), Color32FromB2HexColor(color));
		}
	}
	ScratchEnd(scratch);
}
void DebugBox2d_Sokol_DrawCircle(b2Vec2 center, float radius, b2HexColor color, void* context)
{
	//TODO: Implement me!
}
void DebugBox2d_Sokol_DrawSolidCircle(b2Transform transform, float radius, b2HexColor color, void* context)
{
	//TODO: Implement me!
}
void DebugBox2d_Sokol_DrawCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context)
{
	//TODO: Implement me!
}
void DebugBox2d_Sokol_DrawSolidCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context)
{
	//TODO: Implement me!
}
void DebugBox2d_Sokol_DrawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context)
{
	//TODO: Implement me!
}
void DebugBox2d_Sokol_DrawTransform(b2Transform transform, void* context)
{
	//TODO: Implement me!
}
void DebugBox2d_Sokol_DrawPoint(b2Vec2 p, float size, b2HexColor color, void* context)
{
	//TODO: Implement me!
}
void DebugBox2d_Sokol_DrawString(b2Vec2 p, const char* s, void* context)
{
	//TODO: Implement me!
}

void InitSokolBox2DRender()
{
	physDebugDraw.DrawPolygon = DebugBox2d_Sokol_DrawPolygon;
	physDebugDraw.DrawSolidPolygon = DebugBox2d_Sokol_DrawSolidPolygon;
	physDebugDraw.DrawCircle = DebugBox2d_Sokol_DrawCircle;
	physDebugDraw.DrawSolidCircle = DebugBox2d_Sokol_DrawSolidCircle;
	physDebugDraw.DrawCapsule = DebugBox2d_Sokol_DrawCapsule;
	physDebugDraw.DrawSolidCapsule = DebugBox2d_Sokol_DrawSolidCapsule;
	physDebugDraw.DrawSegment = DebugBox2d_Sokol_DrawSegment;
	physDebugDraw.DrawTransform = DebugBox2d_Sokol_DrawTransform;
	physDebugDraw.DrawPoint = DebugBox2d_Sokol_DrawPoint;
	physDebugDraw.DrawString = DebugBox2d_Sokol_DrawString;
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

#endif //BUILD_WITH_BOX2D && BUILD_WITH_SOKOL_GFX
