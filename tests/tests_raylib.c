/*
File:   tests_raylib.c
Author: Taylor Robbins
Date:   02\24\2025
Description: 
	** None
*/

#define RAYLIB_3D 0

#if BUILD_WITH_RAYLIB

void InitRaylibTests()
{
	InitWindow(800, 600, "Tests (PigCore)");
	SetWindowMinSize(400, 200);
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(60);
	
	#if BUILD_WITH_BOX2D && !RAYLIB_3D
	InitBox2DTest();
	#endif
}

void RunRaylibTests()
{
	// +==============================+
	// |      RayLib/Box2D Tests      |
	// +==============================+
	#if !BUILD_WITH_OPENVR
	{
		#if RAYLIB_3D
		Camera3D camera = ZEROED;
		camera.position = NewVector3(1, 2, -10);
		camera.target = NewVector3(0, 0, 0);
		camera.up = Vector3FromV3(V3_Up);
		camera.fovy = 60; //ToDegrees32(QuarterPi32);
		camera.projection = CAMERA_PERSPECTIVE;
	    DisableCursor();
		#else
		Camera2D camera = ZEROED;
		camera.target = NewVector2((r32)GetRenderWidth()/2, (r32)GetRenderHeight()/2);
		camera.offset = NewVector2((r32)GetRenderWidth()/2, (r32)GetRenderHeight()/2);
		camera.rotation = 0;
		camera.zoom = 1.0f;
		#endif
		
		while (!WindowShouldClose())
		{
			v2i windowSize = NewV2i(GetRenderWidth(), GetRenderHeight());
			
			#if RAYLIB_3D
			if (IsKeyPressed(KEY_ESCAPE)) { CloseWindow(); }
			if (IsKeyDown(KEY_Z)) { camera.target = NewVector3(0.0f, 0.0f, 0.0f); }
			UpdateCamera(&camera, CAMERA_FIRST_PERSON);
			#endif
			
			BeginDrawing();
			ClearBackground(RAYWHITE);
			
			#if RAYLIB_3D
			{
				BeginMode3D(camera);
				
				DrawCube(camera.target, 0.5f, 0.5f, 0.5f, PURPLE);
				
				// Draw grid on the ground
				for (i32 gridIndex = -100; gridIndex <= 100; gridIndex++)
				{
					const r32 gridThickness = 0.01f;
					const r32 gridSize = 400;
					const r32 gridStep = 1.0f;
					DrawCubeV(NewVector3(0 + gridStep*gridIndex, 0, 0), NewVector3(gridThickness, gridThickness, gridSize), ColorFromColor32(MonokaiGray1));
					DrawCubeV(NewVector3(0, 0, 0 + gridStep*gridIndex), NewVector3(gridSize, gridThickness, gridThickness), ColorFromColor32(MonokaiGray1));
				}
				
				DrawCubeV(NewVector3(10, 0, 0), NewVector3(20, 0.1f, 0.1f), ColorFromColor32(MonokaiRed));
				DrawCubeV(NewVector3(0, 10, 0), NewVector3(0.1f, 20, 0.1f), ColorFromColor32(MonokaiGreen));
				DrawCubeV(NewVector3(0, 0, 10), NewVector3(0.1f, 0.1f, 20), ColorFromColor32(MonokaiBlue));
				
				EndMode3D();
			}
			#else //!RAYLIB_3D
			{
				BeginMode2D(camera);
				
				const char* textStr = "Congrats! You created your first window!";
				const int textSize = 20;
				int textWidth = MeasureText(textStr, textSize);
				DrawText(textStr, windowSize.Width/2 - textWidth/2, windowSize.Height/2 - textSize/2, textSize, LIGHTGRAY);
				
				#if BUILD_WITH_BOX2D
				UpdateBox2DTest();
				RenderBox2DTest();
				#endif
				
				#if 0
				r32 textureScale = MinR32((r32)windowSize.Width / (r32)zipTextureSize.Width, (r32)windowSize.Height / (r32)zipTextureSize.Height);
				v2 textureSize = Mul(ToV2Fromi(zipTextureSize), textureScale);
				Vector2 topLeft = (Vector2){
					.x = (r32)windowSize.Width/2 - textureSize.Width/2,
					.y = (r32)windowSize.Height/2 - textureSize.Height/2
				};
				DrawTextureEx(zipTexture, topLeft, 0.0f, textureScale, WHITE);
				#endif
				
				EndMode2D();
			}
			#endif //RAYLIB_3D
			
			EndDrawing();
		}
		CloseWindow();
	}
	#endif //!BUILD_WITH_OPENVR
}

#endif //BUILD_WITH_RAYLIB
