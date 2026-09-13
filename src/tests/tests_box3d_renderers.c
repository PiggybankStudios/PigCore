/*
File:   tests_box3d_renderers.c
Author: Taylor Robbins
Date:   09\13\2026
Description: 
	** None
*/

#if BUILD_WITH_BOX3D

b3DebugDraw physDebugDraw3D = ZEROED;

#if BUILD_WITH_SOKOL_GFX

VertBuffer cube3DBuffer;
typedef plex Box3DShape Box3DShape;
plex Box3DShape
{
	VertBuffer* vertBuffer;
	v3 scale;
};

/// The user needs to be able to create debug draw shapes for multi-pass rendering to work efficiently.
/// These user shapes are created and destroyed via callback so they can be bound to shape lifetime and scaling updates.
/// @ingroup debug_draw
void* DebugBox3d_Sokol_CreateDebugShapeCallback(const b3DebugShape* debugShape, void* userContext)
{
	NotNull(debugShape);
	NotNull(userContext);
	AssertMsg(debugShape->hull != nullptr, "Asked to create a DebugShape for something besides a hull!");
	Box3DShape* result = AllocType(Box3DShape, (Arena*)userContext);
	ClearPointer(result);
	result->vertBuffer = &cube3DBuffer;
	result->scale.x = (debugShape->hull->aabb.upperBound.x - debugShape->hull->aabb.lowerBound.x);
	result->scale.y = (debugShape->hull->aabb.upperBound.y - debugShape->hull->aabb.lowerBound.y);
	result->scale.z = (debugShape->hull->aabb.upperBound.z - debugShape->hull->aabb.lowerBound.z);
	return result;
}
void DebugBox3d_Sokol_DestroyDebugShapeCallback(void* userShape, void* userContext)
{
	NotNull(userShape);
	NotNull(userContext);
	FreeType(Box3DShape, (Arena*)userContext, (Box3DShape*)userShape);
}

void DrawObb3D(obb3 boundingBox, Color32 color)
{
	mat4 worldMat = Mat4_Identity;
	TransformMat4(&worldMat, MakeTranslateXYZMat4(-0.5f, -0.5f, -0.5f));
	TransformMat4(&worldMat, MakeScaleMat4(boundingBox.size));
	TransformMat4(&worldMat, ToMat4FromQuat(boundingBox.rotation));
	TransformMat4(&worldMat, MakeTranslateMat4(boundingBox.center));
	SetWorldMat(worldMat);
	SetTintColor(color);
	BindVertBuffer(&cube3DBuffer);
	DrawVertices();
}

bool DebugBox3d_Sokol_DrawShape(void* userShape, b3WorldTransform transform, b3HexColor color, void* context)
{
	UNUSED(context);
	if (userShape != nullptr)
	{
		Box3DShape* shape = (Box3DShape*)userShape;
		// PrintLine_D("Drawing box(%g,%g,%g) at (%g, %g, %g)",
		// 	shape->scale.x, shape->scale.y, shape->scale.z,
		// 	transform.p.x, transform.p.y, transform.p.z
		// );
		obb3 box = MakeObb3_Const(
			transform.p.x, transform.p.y, transform.p.z,
			shape->scale.x, shape->scale.y, shape->scale.z,
			MakeQuat_Const(transform.q.v.x, transform.q.v.y, transform.q.v.z, transform.q.s)
		);
		DrawObb3D(box, MakeColorU32(color | 0xFF000000));
	}
	else { WriteLine_D("Drawing shape with no userShape!"); }
	return true;
}

void DebugBox3d_Sokol_DrawBox(b3Vec3 extents, b3WorldTransform transform, b3HexColor color, void* context)
{
	UNUSED(context);
	PrintLine_D("Drawing box at (%g, %g, %g)", transform.p.x, transform.p.y, transform.p.z);
	obb3 box = MakeObb3_Const(
		transform.p.x, transform.p.y, transform.p.z,
		extents.x, extents.y, extents.z,
		MakeQuat_Const(transform.q.v.x, transform.q.v.y, transform.q.v.z, transform.q.s)
	);
	DrawObb3D(box, MakeColorU32(color | 0xFF000000));
}

void InitSokolBox3DRender()
{
	ScratchBegin(scratch);
	
	physDebugDraw3D = b3DefaultDebugDraw();
	physDebugDraw3D.drawShapes = true;
	physDebugDraw3D.drawBounds = true;
	physDebugDraw3D.DrawShapeFcn = DebugBox3d_Sokol_DrawShape;
	// void ( *DrawSegmentFcn )( b3Pos p1, b3Pos p2, b3HexColor color, void* context );
	// void ( *DrawTransformFcn )( b3WorldTransform transform, void* context );
	// void ( *DrawPointFcn )( b3Pos p, float size, b3HexColor color, void* context );
	// void ( *DrawSphereFcn )( b3Pos p, float radius, b3HexColor color, float alpha, void* context );
	// void ( *DrawCapsuleFcn )( b3Pos p1, b3Pos p2, float radius, b3HexColor color, float alpha, void* context );
	// void ( *DrawBoundsFcn )( b3AABB aabb, b3HexColor color, void* context );
	physDebugDraw3D.DrawBoxFcn = DebugBox3d_Sokol_DrawBox;
	// void ( *DrawStringFcn )( b3Pos p, const char* s, b3HexColor color, void* context );
	
	GeneratedMesh cubeMesh = GenerateVertsForBox(scratch, MakeBoxV(V3_Zero, V3_One), White);
	Vertex3D* cubeVertices = AllocArray(Vertex3D, scratch, cubeMesh.numIndices);
	for (uxx iIndex = 0; iIndex < cubeMesh.numIndices; iIndex++)
	{
		MyMemCopy(&cubeVertices[iIndex], &cubeMesh.vertices[cubeMesh.indices[iIndex]], sizeof(Vertex3D));
	}
	cube3DBuffer = InitVertBuffer3D(stdHeap, StrLit("cube"), VertBufferUsage_Static, cubeMesh.numIndices, cubeVertices, false);
	Assert(cube3DBuffer.error == Result_Success);
	
	ScratchEnd(scratch);
}

#endif //BUILD_WITH_SOKOL_GFX

#endif //BUILD_WITH_BOX3D
