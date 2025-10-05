/*
File:   tests_sokol.c
Author: Taylor Robbins
Date:   01\28\2025
Description: 
	** None
*/

#if BUILD_WITH_SOKOL_APP

#if TARGET_IS_ANDROID
#define MAIN_FONT_NAME "DroidSansMono"
#else
#define MAIN_FONT_NAME "Consolas"
#endif

#define SOKOL_APP_IMPL
#if TARGET_IS_LINUX
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers" //warning: missing field 'revents' initializer [-Wmissing-field-initializers]
#endif
#include "third_party/sokol/sokol_app.h"
#if TARGET_IS_LINUX
#pragma clang diagnostic pop
#endif

#include "misc/misc_sokol_app_helpers.c"

#endif //BUILD_WITH_SOKOL_APP

#if BUILD_WITH_SOKOL_GFX && BUILD_WITH_SOKOL_APP

#include "tests/simple_shader.glsl.h"
#include "tests/main2d_shader.glsl.h"
#include "tests/main3d_shader.glsl.h"

int MyMain(int argc, char* argv[]);

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
int argc_copy = 0;
char** argv_copy = nullptr;
sg_pass_action sokolPassAction;
Shader simpleShader;
Shader main2dShader;
Shader main3dShader;
Texture gradientTexture;
PigFont testFont;
VertBuffer cubeBuffer;
VertBuffer sphereBuffer;
u64 programTime = 0;
MouseState mouse = ZEROED;
KeyboardState keyboard = ZEROED;
TouchscreenState touchscreen = ZEROED;
v3 cameraPos = {.X=0.0f, .Y=1.0f, .Z=0.0f};
v3 cameraLookDir = V3_Zero_Const;
v2 wrapPos = V2_Zero_Const;
#if BUILD_WITH_CLAY
ClayUIRenderer clay = ZEROED;
u16 clayFont = 0;
bool isFileMenuOpen = false;
#endif
#if BUILD_WITH_IMGUI
ImguiUI* imgui = nullptr;
bool isImguiDemoWindowOpen = false;
bool isCTokenizerWindowOpen = false;
cTokenizer tokenizer = ZEROED;
#endif
#if BUILD_WITH_PHYSX
PhysicsWorld* physWorld = nullptr;
#endif
v4 screenMargins = V4_Zero_Const;
v4 screenSafeMargins = V4_Zero_Const;
v2i oldWindowSize = V2i_Zero_Const;
#if TARGET_IS_ANDROID
Rot2 screenRotation = Rot2_0;
bool screenRotated = false;
#endif
//TODO: Somehow we need to detect how big our text should be in order to be a particular size on screen with consideration for high DPI displays
#define TEXT_SCALE 3.0f
// #define TEXT_SCALE 1.0f

void UpdateScreenSafeMargins()
{
	#if TARGET_IS_ANDROID
	JavaVMAttachBlock(env)
	{
		if ((*env)->GetVersion(env) > jGetField_Build_VERSION_CODES(env, "P"))
		{
			jobject window = jCall_getWindow(env, AndroidNativeActivity);
			jobject decorView = jCall_getDecorView(env, window);
			
			jobject insets = jCall_getRootWindowInsets(env, decorView);
			screenMargins.X = (r32)jCall_getSystemWindowInsetLeft(env, insets);
			screenMargins.Y = (r32)jCall_getSystemWindowInsetTop(env, insets);
			screenMargins.Z = (r32)jCall_getSystemWindowInsetRight(env, insets);
			screenMargins.W = (r32)jCall_getSystemWindowInsetBottom(env, insets);
			
			jobject displayCutout = jCall_getDisplayCutout(env, insets);
			if (displayCutout != nullptr)
			{
				screenSafeMargins.X = (r32)jCall_getSafeInsetLeft(env, displayCutout);
				screenSafeMargins.Y = (r32)jCall_getSafeInsetTop(env, displayCutout);
				screenSafeMargins.Z = (r32)jCall_getSafeInsetRight(env, displayCutout);
				screenSafeMargins.W = (r32)jCall_getSafeInsetBottom(env, displayCutout);
				
				(*env)->DeleteLocalRef(env, displayCutout);
			}
			(*env)->DeleteLocalRef(env, insets);
			(*env)->DeleteLocalRef(env, decorView);
			(*env)->DeleteLocalRef(env, window);
		}
	}
	#endif
}

#if TARGET_IS_ANDROID
void UpdateScreenRotation()
{
	Rot2 newRotation = screenRotation;
	#if 1
	JavaVMAttachBlock(env)
	{
		jstring windowJStr = NewJStrNt(env, "window");
		jobject windowManager = jCall_getSystemService(env, AndroidNativeActivity, windowJStr);
		FreeJStr(env, windowJStr);
		jobject display = jCall_getDefaultDisplay(env, windowManager);
		i32 rotation = jCall_getRotation(env, display);
		switch (rotation)
		{
			case 0: newRotation = Rot2_0;   break;
			case 1: newRotation = Rot2_90;  break;
			case 2: newRotation = Rot2_180; break;
			case 3: newRotation = Rot2_270; break;
			default: PrintLine_W("Unhandled rotation value: %d", rotation); break;
		}
		(*env)->DeleteLocalRef(env, display);
		(*env)->DeleteLocalRef(env, windowManager);
	}
	#else
	AConfiguration* configuration = AConfiguration_new();
	AConfiguration_fromAssetManager(configuration, AndroidNativeActivity->assetManager);
	switch (AConfiguration_getOrientation(configuration))
	{
		case ACONFIGURATION_ORIENTATION_PORT: newRotation = Rot2_0; break;
		case ACONFIGURATION_ORIENTATION_LAND: newRotation = Rot2_90; break;
		case ACONFIGURATION_ORIENTATION_SQUARE: newRotation = Rot2_0; break;
	}
	AConfiguration_delete(configuration);
	#endif
	if (newRotation != screenRotation)
	{
		PrintLine_W("Rotation is now %s", GetRot2String(newRotation));
		screenRotation = newRotation;
		screenRotated = true;
	}
}
#endif //TARGET_IS_ANDROID

#if BUILD_WITH_CLAY
//Call Clay__CloseElement once if false, three times if true (i.e. twicfe inside the if statement and once after)
bool ClayTopBtn(const char* btnText, bool* isOpenPntr, Color32 backColor, Color32 textColor, r32 dropDownWidth)
{
	ScratchBegin(scratch);
	r32 textScale = TEXT_SCALE/sapp_dpi_scale();
	Color32 highlightColor = ColorLerpSimple(backColor, White, 0.3f);
	Str8 btnIdStr = PrintInArenaStr(scratch, "%s_TopBtn", btnText);
	Str8 menuIdStr = PrintInArenaStr(scratch, "%s_TopBtnMenu", btnText);
	Clay_ElementId btnId = ToClayId(btnIdStr);
	Clay_ElementId menuId = ToClayId(menuIdStr);
	bool isBtnHoveredOrMenuOpen = (Clay_PointerOver(btnId) || *isOpenPntr);
	Clay__OpenElement();
	Clay__ConfigureOpenElement((Clay_ElementDeclaration){
		.id = btnId,
		.layout = { .padding = { 12, 12, 8, 8 } },
		.backgroundColor = (isBtnHoveredOrMenuOpen ? highlightColor : backColor),
		.cornerRadius = CLAY_CORNER_RADIUS(5),
	});
	CLAY_TEXT(
		StrLit(btnText),
		CLAY_TEXT_CONFIG({
			.fontId = clayFont,
			.fontSize = 18*textScale,
			.textColor = textColor,
		})
	);
	bool isHovered = (Clay_PointerOver(btnId) || Clay_PointerOver(menuId));
	if (Clay_PointerOver(btnId) && (IsMouseBtnPressed(&mouse, MouseBtn_Left) || touchscreen.mainTouch->started))
	{
		*isOpenPntr = !*isOpenPntr;
	}
	if (*isOpenPntr == true && !isHovered) { *isOpenPntr = false; }
	if (*isOpenPntr)
	{
		Clay__OpenElement();
		Clay__ConfigureOpenElement((Clay_ElementDeclaration){
			.id = menuId,
			.floating = {
				.attachTo = CLAY_ATTACH_TO_PARENT,
				.attachPoints = {
					.parent = CLAY_ATTACH_POINT_LEFT_BOTTOM,
				},
			},
			.layout = {
				.padding = { 0, 0, 0, 0 },
			}
		});
		
		Clay__OpenElement();
		Clay__ConfigureOpenElement((Clay_ElementDeclaration){
			.layout = {
				.layoutDirection = CLAY_TOP_TO_BOTTOM,
				.sizing = {
					.width = CLAY_SIZING_FIXED(dropDownWidth),
				},
				.childGap = 2,
			},
			.backgroundColor = MonokaiBack,
			.cornerRadius = CLAY_CORNER_RADIUS(8),
		});
	}
	ScratchEnd(scratch);
	return *isOpenPntr;
}

//Call Clay__CloseElement once after if statement
bool ClayBtn(const char* btnText, Color32 backColor, Color32 textColor)
{
	ScratchBegin(scratch);
	r32 textScale = TEXT_SCALE/sapp_dpi_scale();
	Color32 hoverColor = ColorLerpSimple(backColor, White, 0.3f);
	Color32 pressColor = ColorLerpSimple(backColor, White, 0.1f);
	Str8 btnIdStr = PrintInArenaStr(scratch, "%s_Btn", btnText);
	Clay_ElementId btnId = ToClayId(btnIdStr);
	bool isHovered = Clay_PointerOver(btnId);
	bool isPressed = (isHovered && (IsMouseBtnDown(&mouse, MouseBtn_Left) || (touchscreen.mainTouch->id != TOUCH_ID_INVALID && !touchscreen.mainTouch->stopped)));
	Clay__OpenElement();
	Clay__ConfigureOpenElement((Clay_ElementDeclaration){
		.id = btnId,
		.layout = {
			.padding = CLAY_PADDING_ALL(8),
			.sizing = { .width = CLAY_SIZING_GROW(0), },
		},
		.backgroundColor = (isPressed ? pressColor : (isHovered ? hoverColor : backColor)),
		.cornerRadius = CLAY_CORNER_RADIUS(8),
	});
	CLAY_TEXT(
		StrLit(btnText),
		CLAY_TEXT_CONFIG({
			.fontId = clayFont,
			.fontSize = 18*textScale,
			.textColor = textColor,
			.userData = { .richText = true },
		})
	);
	ScratchEnd(scratch);
	return (isHovered && (IsMouseBtnPressed(&mouse, MouseBtn_Left) || touchscreen.mainTouch->started));
}
#endif //BUILD_WITH_CLAY

void DrawBox(box boundingBox, Color32 color)
{
	mat4 worldMat = Mat4_Identity;
	TransformMat4(&worldMat, MakeScaleMat4(boundingBox.Size));
	TransformMat4(&worldMat, MakeTranslateMat4(boundingBox.BottomLeftBack));
	SetWorldMat(worldMat);
	SetTintColor(color);
	BindVertBuffer(&cubeBuffer);
	DrawVertices();
}
void DrawObb3(obb3 boundingBox, Color32 color)
{
	mat4 worldMat = Mat4_Identity;
	TransformMat4(&worldMat, MakeTranslateMat4(FillV3(-0.5f)));
	TransformMat4(&worldMat, MakeScaleMat4(boundingBox.Size));
	TransformMat4(&worldMat, ToMat4FromQuat(boundingBox.Rotation));
	TransformMat4(&worldMat, MakeTranslateMat4(boundingBox.Center));
	SetWorldMat(worldMat);
	SetTintColor(color);
	BindVertBuffer(&cubeBuffer);
	DrawVertices();
}
void DrawSphere(Sphere sphere, Color32 color)
{
	mat4 worldMat = Mat4_Identity;
	TransformMat4(&worldMat, MakeScaleMat4(FillV3(sphere.Radius)));
	TransformMat4(&worldMat, MakeTranslateMat4(sphere.Center));
	SetWorldMat(worldMat);
	SetTintColor(color);
	BindVertBuffer(&sphereBuffer);
	DrawVertices();
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void AppInit(void)
{
	MyMain(argc_copy, argv_copy); //call MyMain to initialize arenas and whatnot
	
	ScratchBegin(scratch);
	InitSokolGraphics((sg_desc){
		.environment = CreateSokolAppEnvironment(),
		.logger.func = SokolLogCallback,
	});
	
	InitGfxSystem(stdHeap, &gfx);
	
	v2i gradientSize = NewV2i(64, 64);
	Color32* gradientPixels = AllocArray(Color32, scratch, (uxx)(gradientSize.Width * gradientSize.Height));
	for (i32 pixelY = 0; pixelY < gradientSize.Height; pixelY++)
	{
		for (i32 pixelX = 0; pixelX < gradientSize.Width; pixelX++)
		{
			Color32* pixel = &gradientPixels[INDEX_FROM_COORD2D(pixelX, pixelY, gradientSize.Width, gradientSize.Height)];
			pixel->r = ClampCastI32ToU8(RoundR32i(LerpR32(0, 255.0f, (r32)pixelX / (r32)gradientSize.Width)));
			pixel->g = ClampCastI32ToU8(RoundR32i(LerpR32(0, 255.0f, (r32)pixelY / (r32)gradientSize.Height)));
			pixel->b = pixel->r/2 + pixel->g/2;
			pixel->a = 255;
		}
	}
	
	gradientTexture = InitTexture(stdHeap, StrLit("gradient"), gradientSize, gradientPixels, TextureFlag_IsRepeating);
	Assert(gradientTexture.error == Result_Success);
	
	testFont = InitFont(stdHeap, StrLit("testFont"));
	{
		// OsWriteBinFile(FilePathLit("Default.ttf"), testFont.ttfFile);
		FontCharRange charRanges[] = {
			FontCharRange_ASCII,
			FontCharRange_LatinSupplementAccent,
		};
		
		r32 textScale = TEXT_SCALE/sapp_dpi_scale();
		Result attachResult1 = AttachOsTtfFileToFont(&testFont, StrLit(MAIN_FONT_NAME), 18*textScale, FontStyleFlag_None);
		if (attachResult1 == Result_Success)
		{
			Result bakeResult1 = BakeFontAtlas(&testFont, 18*textScale, FontStyleFlag_None, NewV2i(512, 512), ArrayCount(charRanges), &charRanges[0]);
			Assert(bakeResult1 == Result_Success);
			FillFontKerningTable(&testFont);
			RemoveAttachedTtfFile(&testFont);
		}
		else { PrintLine_E("Failed to find/attach platform font \"" MAIN_FONT_NAME "\" 18 Regular: %s", GetResultStr(attachResult1)); }
		
		Result attachResult2 = AttachOsTtfFileToFont(&testFont, StrLit(MAIN_FONT_NAME), 18*textScale, FontStyleFlag_Bold);
		if (attachResult2 == Result_Success)
		{
			Result bakeResult2 = BakeFontAtlas(&testFont, 18*textScale, FontStyleFlag_Bold, NewV2i(512, 512), ArrayCount(charRanges), &charRanges[0]);
			Assert(bakeResult2 == Result_Success);
			RemoveAttachedTtfFile(&testFont);
		}
		else { PrintLine_E("Failed to find/attach platform font \"" MAIN_FONT_NAME "\" 18 Bold: %s", GetResultStr(attachResult2)); }
		
		Result attachResult3 = AttachOsTtfFileToFont(&testFont, StrLit(MAIN_FONT_NAME), 18*textScale, FontStyleFlag_Italic);
		if (attachResult3 == Result_Success)
		{
			Result bakeResult3 = BakeFontAtlas(&testFont, 18*textScale, FontStyleFlag_Italic, NewV2i(512, 512), ArrayCount(charRanges), &charRanges[0]);
			Assert(bakeResult3 == Result_Success);
			RemoveAttachedTtfFile(&testFont);
		}
		else { PrintLine_E("Failed to find/attach platform font \"" MAIN_FONT_NAME "\" 18 Italic: %s", GetResultStr(attachResult3)); }
		
		Result attachResult4 = AttachOsTtfFileToFont(&testFont, StrLit(MAIN_FONT_NAME), 18*textScale, FontStyleFlag_Bold|FontStyleFlag_Italic);
		if (attachResult4 == Result_Success)
		{
			Result bakeResult4 = BakeFontAtlas(&testFont, 18*textScale, FontStyleFlag_Bold|FontStyleFlag_Italic, NewV2i(512, 512), ArrayCount(charRanges), &charRanges[0]);
			Assert(bakeResult4 == Result_Success);
			RemoveAttachedTtfFile(&testFont);
		}
		else { PrintLine_E("Failed to find/attach platform font \"" MAIN_FONT_NAME "\" 18 Bold+Italic: %s", GetResultStr(attachResult4)); }
		
		Result attachResult5 = AttachOsTtfFileToFont(&testFont, StrLit(MAIN_FONT_NAME), 10*textScale, FontStyleFlag_Bold);
		if (attachResult5 == Result_Success)
		{
			Result bakeResult5 = BakeFontAtlas(&testFont, 10*textScale, FontStyleFlag_Bold, NewV2i(512, 512), ArrayCount(charRanges), &charRanges[0]);
			Assert(bakeResult5 == Result_Success);
			RemoveAttachedTtfFile(&testFont);
		}
		else { PrintLine_E("Failed to find/attach platform font \"" MAIN_FONT_NAME "\" 10 Bold: %s", GetResultStr(attachResult5)); }
		
		Result attachResult6 = AttachOsTtfFileToFont(&testFont, StrLit(MAIN_FONT_NAME), 26*textScale, FontStyleFlag_Bold);
		if (attachResult6 == Result_Success)
		{
			Result bakeResult6 = BakeFontAtlas(&testFont, 26*textScale, FontStyleFlag_Bold, NewV2i(1024, 1024), ArrayCount(charRanges), &charRanges[0]);
			Assert(bakeResult6 == Result_Success);
			RemoveAttachedTtfFile(&testFont);
		}
		else { PrintLine_E("Failed to find/attach platform font \"" MAIN_FONT_NAME "\" 26 Bold: %s", GetResultStr(attachResult6)); }
	}
	
	GeneratedMesh cubeMesh = GenerateVertsForBox(scratch, NewBoxV(V3_Zero, V3_One), White);
	Vertex3D* cubeVertices = AllocArray(Vertex3D, scratch, cubeMesh.numIndices);
	for (uxx iIndex = 0; iIndex < cubeMesh.numIndices; iIndex++)
	{
		MyMemCopy(&cubeVertices[iIndex], &cubeMesh.vertices[cubeMesh.indices[iIndex]], sizeof(Vertex3D));
	}
	cubeBuffer = InitVertBuffer3D(stdHeap, StrLit("cube"), VertBufferUsage_Static, cubeMesh.numIndices, cubeVertices, false);
	Assert(cubeBuffer.error == Result_Success);
	
	GeneratedMesh sphereMesh = GenerateVertsForSphere(scratch, NewSphereV(V3_Zero, 1.0f), 12, 20, White);
	Vertex3D* sphereVertices = AllocArray(Vertex3D, scratch, sphereMesh.numIndices);
	for (uxx iIndex = 0; iIndex < sphereMesh.numIndices; iIndex++)
	{
		MyMemCopy(&sphereVertices[iIndex], &sphereMesh.vertices[sphereMesh.indices[iIndex]], sizeof(Vertex3D));
	}
	sphereBuffer = InitVertBuffer3D(stdHeap, StrLit("sphere"), VertBufferUsage_Static, sphereMesh.numIndices, sphereVertices, false);
	Assert(sphereBuffer.error == Result_Success);
	
	InitCompiledShader(&simpleShader, stdHeap, simple); Assert(simpleShader.error == Result_Success);
	InitCompiledShader(&main2dShader, stdHeap, main2d); Assert(main2dShader.error == Result_Success);
	InitCompiledShader(&main3dShader, stdHeap, main3d); Assert(main3dShader.error == Result_Success);
	
	#if BUILD_WITH_CLAY
	AssertMsg(testFont.atlases.length > 0, "Clay rendering requires that at least one atlas in the testFont was baked correctly!");
	InitClayUIRenderer(stdHeap, V2_Zero, &clay);
	clayFont = AddClayUIRendererFont(&clay, &testFont, GetDefaultFontStyleFlags(&testFont));
	#endif
	
	#if BUILD_WITH_IMGUI
	FlagSet(stdHeap->flags, ArenaFlag_AllowFreeWithoutSize);
	#if TARGET_IS_WINDOWS
	const void* nativeWindowHandle = sapp_win32_get_hwnd();
	#else
	#error We need to figure out how to get the name window handle from sokol_app on this TARGET!
	#endif
	imgui = InitImguiUI(stdHeap, nativeWindowHandle);
	#endif
	
	InitMouseState(&mouse);
	InitKeyboardState(&keyboard);
	InitTouchscreenState(&touchscreen);
	cameraLookDir = V3_Right;
	
	#if BUILD_WITH_BOX2D
	InitBox2DTest();
	#endif
	
	#if BUILD_WITH_PHYSX
	FlagSet(stdHeap->flags, ArenaFlag_AllowFreeWithoutSize);
	physWorld = InitPhysicsPhysX(stdHeap);
	CreatePhysicsTest(physWorld);
	#endif
	
	UpdateScreenSafeMargins();
	oldWindowSize = NewV2i(sapp_width(), sapp_height());
	
	ScratchEnd(scratch);
}

void AppCleanup(void)
{
	sg_shutdown();
}

#if 0
void DrawRectangle(Shader* shader, v2 topLeft, v2 size, Color32 color)
{
	NotNull(shader);
	
	mat4 worldMat = Mat4_Identity;
	TransformMat4(&worldMat, MakeScaleXYZMat4(size.Width, size.Height, 1.0f));
	TransformMat4(&worldMat, MakeTranslateXYZMat4(topLeft.X, topLeft.Y, 0.0f));
	SetWorldMat(worldMat);
	SetTintColor(color);
	
	BindVertBuffer(&squareBuffer);
	DrawVertices();
}
#endif

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
bool AppFrame(void)
{
	TracyCFrameMark;
	TracyCZoneN(Zone_Update, "Update", true);
	
	ScratchBegin(scratch);
	bool frameRendered = true;
	programTime += 16; //TODO: Calculate this!
	v2i windowSizei = NewV2i(sapp_width(), sapp_height());
	v2 windowSize = NewV2(sapp_widthf(), sapp_heightf());
	// v2 touchPos = touchscreen.mainTouch->pos;
	#if TARGET_IS_ANDROID
	UpdateScreenRotation();
	#endif
	if (AreEqualV2i(oldWindowSize, windowSizei)) { UpdateScreenSafeMargins(); }
	
	if (IsMouseBtnDown(&mouse, MouseBtn_Left)) { wrapPos = mouse.position; }
	if (touchscreen.mainTouch->id != TOUCH_ID_INVALID) { wrapPos = touchscreen.mainTouch->pos; }
	
	if (IsKeyboardKeyPressed(&keyboard, Key_F, false)) { sapp_lock_mouse(!sapp_mouse_locked()); }
	if (IsKeyboardKeyPressed(&keyboard, Key_Escape, false) && sapp_mouse_locked()) { sapp_lock_mouse(false); }
	if (sapp_mouse_locked())
	{
		r32 cameraHoriRot = AtanR32(cameraLookDir.Z, cameraLookDir.X);
		r32 cameraVertRot = AtanR32(cameraLookDir.Y, Length(NewV2(cameraLookDir.X, cameraLookDir.Z)));
		cameraHoriRot = AngleFixR32(cameraHoriRot - mouse.lockedPosDelta.X / 500.0f);
		cameraVertRot = ClampR32(cameraVertRot - mouse.lockedPosDelta.Y / 500.0f, -HalfPi32+0.05f, HalfPi32-0.05f);
		r32 horizontalRadius = CosR32(cameraVertRot);
		cameraLookDir = NewV3(CosR32(cameraHoriRot) * horizontalRadius, SinR32(cameraVertRot), SinR32(cameraHoriRot) * horizontalRadius);
		
		v3 horizontalForwardVec = Normalize(NewV3(cameraLookDir.X, 0.0f, cameraLookDir.Z));
		v3 horizontalRightVec = Normalize(NewV3(cameraLookDir.Z, 0.0f, -cameraLookDir.X));
		const r32 moveSpeed = IsKeyboardKeyDown(&keyboard, Key_Shift) ? 0.08f : 0.02f;
		if (IsKeyboardKeyDown(&keyboard, Key_W)) { cameraPos = Add(cameraPos, Mul(horizontalForwardVec, moveSpeed)); }
		if (IsKeyboardKeyDown(&keyboard, Key_A)) { cameraPos = Add(cameraPos, Mul(horizontalRightVec, -moveSpeed)); }
		if (IsKeyboardKeyDown(&keyboard, Key_S)) { cameraPos = Add(cameraPos, Mul(horizontalForwardVec, -moveSpeed)); }
		if (IsKeyboardKeyDown(&keyboard, Key_D)) { cameraPos = Add(cameraPos, Mul(horizontalRightVec, moveSpeed)); }
		if (IsKeyboardKeyDown(&keyboard, Key_E)) { cameraPos = Add(cameraPos, Mul(V3_Up, moveSpeed)); }
		if (IsKeyboardKeyDown(&keyboard, Key_Q)) { cameraPos = Add(cameraPos, Mul(V3_Down, moveSpeed)); }
	}
	
	for (uxx tIndex = 0; tIndex < MAX_TOUCH_INPUTS; tIndex++)
	{
		TouchState* touch = &touchscreen.touches[tIndex];
		if (touch->id != TOUCH_ID_INVALID)
		{
			if (touch->moved)
			{
				v2 delta = SubV2(touch->pos, touch->prevPos);
				r32 cameraHoriRot = AtanR32(cameraLookDir.Z, cameraLookDir.X);
				r32 cameraVertRot = AtanR32(cameraLookDir.Y, Length(NewV2(cameraLookDir.X, cameraLookDir.Z)));
				cameraHoriRot = AngleFixR32(cameraHoriRot - delta.X / 500.0f);
				cameraVertRot = ClampR32(cameraVertRot - delta.Y / 500.0f, -HalfPi32+0.05f, HalfPi32-0.05f);
				r32 horizontalRadius = CosR32(cameraVertRot);
				cameraLookDir = NewV3(CosR32(cameraHoriRot) * horizontalRadius, SinR32(cameraVertRot), SinR32(cameraHoriRot) * horizontalRadius);
			}
		}
	}
	
	
	#if BUILD_WITH_BOX2D
	if (IsMouseBtnPressed(&mouse, MouseBtn_Left))
	{
		r32 physMouseX, physMouseY;
		GetPhysPosFromRenderPos((i32)mouse.position.X, (i32)mouse.position.Y, &physMouseX, &physMouseY);
		SpawnBox(physMouseX, physMouseY, GetRandR32Range(mainRandom, 0.3f, 1.0f), GetRandR32Range(mainRandom, 0.3f, 1.0f));
	}
	UpdateBox2DTest();
	#endif
	
	#if BUILD_WITH_PHYSX
	UpdatePhysicsWorld(physWorld, 16.6f);
	if (IsKeyboardKeyDown(&keyboard, Key_R)) { CreatePhysicsTest(physWorld); }
	#endif
	
	#if BUILD_WITH_IMGUI
	{
		ImguiInput imguiInput = ZEROED;
		imguiInput.elapsedMs = NUM_MS_PER_SECOND/60.0f; //TODO: Actually get deltaTime from appInput
		imguiInput.keyboard = &keyboard;
		imguiInput.mouse = &mouse;
		imguiInput.isMouseOverOther = false;
		imguiInput.isWindowFocused = true;
		imguiInput.windowFocusedChanged = false;
		imguiInput.isTyping = false;
		ImguiOutput imguiOutput = ZEROED;
		UpdateImguiInput(imgui, &imguiInput, &imguiOutput);
		// platform->SetMouseCursorType(imguiOutput.cursorType);
		// if (!isTyping && imguiOutput.isImguiTypingFocused) { isTyping = true; }
		// if (!isMouseOverUi && imguiOutput.isMouseOverImgui) { isMouseOverUi = true; }
	}
	#endif
	
	TracyCZoneEnd(Zone_Update);
	
	r32 textScale = TEXT_SCALE/sapp_dpi_scale();
	TracyCZoneN(Zone_Draw, "Draw", true);
	BeginFrame(GetSokolAppSwapchain(), windowSizei, MonokaiDarkGray, 1.0f);
	{
		// +==============================+
		// |         3D Rendering         |
		// +==============================+
		{
			BindShader(&main3dShader);
			#if defined(SOKOL_GLCORE)
			mat4 projMat = MakePerspectiveMat4Gl(ToRadians32(45), windowSize.Width/windowSize.Height, 0.05f, 400);
			#else
			mat4 projMat = MakePerspectiveMat4Dx(ToRadians32(45), windowSize.Width/windowSize.Height, 0.05f, 400);
			#endif
			SetProjectionMat(projMat);
			mat4 viewMat = MakeLookAtMat4(cameraPos, Add(cameraPos, cameraLookDir), V3_Up);
			SetViewMat(viewMat);
			
			BindTexture(&gfx.pixelTexture);
			DrawBox(NewBox(3, 0.5f, 0, 1, 1, 1), MonokaiPurple);
			DrawSphere(NewSphere(2.5f, 0, 0.8f, 1.0f), MonokaiGreen);
			
			#if BUILD_WITH_PHYSX
			VarArrayLoop(&physWorld->bodies, bIndex)
			{
				VarArrayLoopGet(PhysicsBody, body, &physWorld->bodies, bIndex);
				PhysicsBodyTransform transform = GetPhysicsBodyTransform(body);
				v3 position = NewV3(transform.position.X, transform.position.Y, transform.position.Z);
				quat rotation = NewQuat(transform.rotation.X, transform.rotation.Y, transform.rotation.Z, transform.rotation.W);
				if (body->index == physWorld->groundPlaneBodyIndex)
				{
					//TODO: Figure out how PhysX want's us to intepret rotation/position on a Plane when drawing it
					DrawObb3(NewObb3V(position, NewV3(100.0f, 0.0001f, 100.0f), Quat_Identity), PalGreenDarker);
				}
				else
				{
					DrawObb3(NewObb3V(position, NewV3(1.0f, 1.0f, 1.0f), rotation), GetPredefPalColorByIndex(bIndex));
				}
			}
			#endif
		}
		
		// +==============================+
		// |         2D Rendering         |
		// +==============================+
		{
			SetDepth(1.0f);
			BindShader(&main2dShader);
			ClearDepthBuffer(1.0f);
			BindTexture(&gradientTexture);
			
			mat4 projMat = Mat4_Identity;
			TransformMat4(&projMat, MakeScaleXYZMat4(1.0f/(windowSize.Width/2.0f), 1.0f/(windowSize.Height/2.0f), 1.0f));
			TransformMat4(&projMat, MakeTranslateXYZMat4(-1.0f, -1.0f, 0.0f));
			TransformMat4(&projMat, MakeScaleYMat4(-1.0f));
			SetProjectionMat(projMat);
			SetViewMat(Mat4_Identity);
			SetTextBackgroundColor(MonokaiBack);
			
			#if 0
			{
				DrawRectangleOutline(NewRec(0, 0, screenSafeMargins.X, windowSize.Height), 10.0f, MonokaiMagenta);
				DrawRectangleOutline(NewRec(0, 0, windowSize.Width, screenSafeMargins.Y), 10.0f, MonokaiBlue);
				DrawRectangleOutline(NewRec(windowSize.Width - screenSafeMargins.Z, 0, screenSafeMargins.Z, windowSize.Height), 10.0f, MonokaiPurple);
				DrawRectangleOutline(NewRec(0, windowSize.Height - screenSafeMargins.W, windowSize.Width, screenSafeMargins.W), 10.0f, MonokaiYellow);
				
				DrawRectangleOutline(NewRec(0, 0, screenMargins.X, windowSize.Height), 5.0f, MonokaiLightRed);
				DrawRectangleOutline(NewRec(0, 0, windowSize.Width, screenMargins.Y), 5.0f, MonokaiLightBlue);
				DrawRectangleOutline(NewRec(windowSize.Width - screenMargins.Z, 0, screenMargins.Z, windowSize.Height), 5.0f, MonokaiLightPurple);
				DrawRectangleOutline(NewRec(0, windowSize.Height - screenMargins.W, windowSize.Width, screenMargins.W), 5.0f, MonokaiOrange);
			}
			#endif
			
			#if 1
			if (testFont.atlases.length > 0)
			{
				BindFont(&testFont);
				FontAtlas* fontAtlas = GetFontAtlas(&testFont, 18*textScale, FontStyleFlag_None);
				NotNull(fontAtlas);
				
				v2 textPos = NewV2(screenSafeMargins.X + 10, screenSafeMargins.Y + 110 + fontAtlas->maxAscend);
				Str8 infoStr = PrintInArenaStr(scratch, "HighDpi: %s Scale: x%g WindowSize: %gx%g", sapp_high_dpi() ? "true" : "false", sapp_dpi_scale(), windowSize.Width, windowSize.Height);
				DrawText(infoStr, textPos, MonokaiWhite);
				
				textPos.Y += fontAtlas->lineHeight;
				r32 wrapWidth = MaxR32(wrapPos.X - textPos.X, 0.0f);
				if (wrapWidth == 0.0f) { wrapWidth = windowSize.Width - textPos.X; }
				RichStr loremIpsumRich = DecodeStrToRichStr(scratch, StrLit("Lorem ipsum dolor sit amet, [size=10]consectetur adipiscing elit, [size]sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. [highlight]Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.[highlight] Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum"));
				DrawWrappedRichTextWithFont(
					&testFont, 18*textScale, FontStyleFlag_None,
					loremIpsumRich,
					textPos,
					wrapWidth,
					MonokaiWhite
				);
				rec logicalRec = gfx.prevFontFlow.logicalRec;
				rec visualRec = gfx.prevFontFlow.visualRec;
				DrawRectangleOutlineEx(logicalRec, 1, MonokaiYellow, false);
				DrawRectangleOutlineEx(visualRec, 1, MonokaiBlue, false);
				DrawRectangle(NewRec(textPos.X + wrapWidth, 0, 1, windowSize.Height), MonokaiRed);
			}
			#endif
			
			#if 0
			rec buttonRec = NewRec(screenSafeMargins.X + 10, screenSafeMargins.Y + 10, 100, 100);
			DrawRectangle(buttonRec, ColorWithAlpha(MonokaiRed, 0.40f));
			for (uxx tIndex = 0; tIndex < MAX_TOUCH_INPUTS; tIndex++)
			{
				TouchState* touch = &touchscreen.touches[tIndex];
				if (touch->id != TOUCH_ID_INVALID && touch->stopped && touch->visitRadius < 10 && IsInsideRec(buttonRec, touch->pos))
				{
					Str8 clipboardStr = Str8_Empty;
					Result pasteResult = OsGetClipboardString(OsWindowHandleEmpty, scratch, &clipboardStr);
					if (pasteResult == Result_Success)
					{
						PrintLine_I("Clipboard has string: \"%.*s\"", StrPrint(clipboardStr));
					}
					else { PrintLine_E("Couldn't get clipboard string: %s", GetResultStr(pasteResult)); }
					
					Result copyResult = OsSetClipboardString(OsWindowHandleEmpty, StrLit("Hello Android clipboard!"));
					PrintLine_I("copyResult: %s", GetResultStr(copyResult));
				}
			}
			#endif
			
			#if 1
			for (uxx tIndex = 0; tIndex < MAX_TOUCH_INPUTS; tIndex++)
			{
				TouchState* touch = &touchscreen.touches[tIndex];
				if (touch->id != TOUCH_ID_INVALID)
				{
					for (uxx pIndex = 1; pIndex < TOUCH_PATH_LENGTH; pIndex++)
					{
						if (AreEqualV2(touch->path[pIndex], TOUCH_PATH_INVALID)) { break; }
						DrawLine(touch->path[pIndex-1], touch->path[pIndex], 1, MonokaiBrown);
					}
					bool isMainTouch = (touchscreen.mainTouchIndex == tIndex);
					DrawCircle(NewCircleV(touch->startPos, touch->visitRadius), ColorWithAlpha(isMainTouch ? MonokaiYellow : MonokaiOrange, 0.25f));
					DrawRectangle(touch->visitBounds, ColorWithAlpha(MonokaiGreen, 0.25f));
					DrawRectangle(NewRecCentered(touch->startPos.X, touch->startPos.Y, 15, 15), MonokaiBlue);
					DrawRectangle(NewRecCentered(touch->pos.X, touch->pos.Y, 15, 15), MonokaiMagenta);
				}
			}
			#endif
			
			#if 0
			v2 tileSize = ToV2Fromi(gradientTexture.size); //NewV2(48, 27);
			i32 numColumns = FloorR32i(windowSize.Width / tileSize.Width);
			i32 numRows = FloorR32i(windowSize.Height / tileSize.Height);
			for (i32 yIndex = 0; yIndex < numRows; yIndex++)
			{
				for (i32 xIndex = 0; xIndex < numColumns; xIndex++)
				{
					DrawTexturedRectangle(NewRec(tileSize.Width * xIndex, tileSize.Height * yIndex, tileSize.Width, tileSize.Height), White, &gradientTexture);
				}
			}
			#endif
			
			#if 0
			r32 atlasRenderPosX = 10.0f;
			VarArrayLoop(&testFont.atlases, aIndex)
			{
				VarArrayLoopGet(FontAtlas, fontAtlas, &testFont.atlases, aIndex);
				rec atlasRenderRec = NewRec(atlasRenderPosX, 10, (r32)fontAtlas->texture.Width, (r32)fontAtlas->texture.Height);
				DrawTexturedRectangle(atlasRenderRec, White, &fontAtlas->texture);
				DrawRectangleOutline(atlasRenderRec, 1, White);
				atlasRenderPosX += atlasRenderRec.Width + 10;
			}
			#endif
			
			#if BUILD_WITH_BOX2D
			RenderBox2DTest();
			#endif
			
			#if BUILD_WITH_CLAY
			UpdateClayScrolling(&clay.clay, 16.6f, false, mouse.scrollDelta, TARGET_IS_ANDROID);
			v2 uiMousePosition = (TARGET_IS_ANDROID ? touchscreen.mainTouch->pos : mouse.position);
			bool uiMouseDown = (TARGET_IS_ANDROID ? (touchscreen.mainTouch->id != TOUCH_ID_INVALID && !touchscreen.mainTouch->stopped) : IsMouseBtnDown(&mouse, MouseBtn_Left));
			BeginClayUIRender(&clay.clay, windowSize, false, uiMousePosition, uiMouseDown);
			{
				CLAY({ .id = CLAY_ID("FullscreenContainer"),
					.layout = {
						.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
						.padding = { .left = screenMargins.X, .top = screenMargins.Y, .right = screenMargins.Z, .bottom = screenMargins.W },
					}
				})
				{
					CLAY({.id = CLAY_ID("SafeInsetLeft"),
						.layout = {
							.sizing = { .width=CLAY_SIZING_FIXED(screenMargins.X), .height=CLAY_SIZING_FIXED(windowSize.Height) }
						},
						.floating = {
							.attachTo = CLAY_ATTACH_TO_PARENT,
							.attachPoints = { .parent = CLAY_ATTACH_POINT_LEFT_TOP, .element = CLAY_ATTACH_POINT_LEFT_TOP },
						},
						.backgroundColor = MonokaiBack,
					}) { }
					CLAY({.id = CLAY_ID("SafeInsetTop"),
						.layout = {
							.sizing = { .width=CLAY_SIZING_FIXED(windowSize.Width), .height=CLAY_SIZING_FIXED(screenMargins.Y) }
						},
						.floating = {
							.attachTo = CLAY_ATTACH_TO_PARENT,
							.attachPoints = { .parent = CLAY_ATTACH_POINT_LEFT_TOP, .element = CLAY_ATTACH_POINT_LEFT_TOP },
						},
						.backgroundColor = MonokaiBack,
					}) { }
					CLAY({.id = CLAY_ID("SafeInsetRight"),
						.layout = {
							.sizing = { .width=CLAY_SIZING_FIXED(screenMargins.Z), .height=CLAY_SIZING_FIXED(windowSize.Height) }
						},
						.floating = {
							.attachTo = CLAY_ATTACH_TO_PARENT,
							.attachPoints = { .parent = CLAY_ATTACH_POINT_RIGHT_TOP, .element = CLAY_ATTACH_POINT_RIGHT_TOP },
						},
						.backgroundColor = MonokaiBack,
					}) { }
					CLAY({.id = CLAY_ID("SafeInsetBottom"),
						.layout = {
							.sizing = { .width=CLAY_SIZING_FIXED(windowSize.Width), .height=CLAY_SIZING_FIXED(screenMargins.W) }
						},
						.floating = {
							.attachTo = CLAY_ATTACH_TO_PARENT,
							.attachPoints = { .parent = CLAY_ATTACH_POINT_LEFT_BOTTOM, .element = CLAY_ATTACH_POINT_LEFT_BOTTOM },
						},
						.backgroundColor = MonokaiBack,
					}) { }
					
					CLAY({ .id = CLAY_ID("SafeContainer"), .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM, .sizing={ .width=CLAY_SIZING_GROW(0), .height=CLAY_SIZING_GROW(0) } } })
					{
						
						FontAtlas* fontAtlas = GetFontAtlas(&testFont, 18*textScale, FontStyleFlag_None);
						NotNull(fontAtlas);
						CLAY({ .id = CLAY_ID("Topbar"),
							.layout = {
								.sizing = {
									.height = CLAY_SIZING_FIXED(fontAtlas->lineHeight + 30),
									.width = CLAY_SIZING_GROW(0),
								},
								.padding = { 0, 0, 0, 0 },
								.childGap = 2,
								.childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
							},
							.backgroundColor = MonokaiBack,
						})
						{
							if (ClayTopBtn("File", &isFileMenuOpen, MonokaiBack, MonokaiWhite, 340 * textScale))
							{
								if (ClayBtn("Op[color=FF00FF]e[highlight]n [size=10]\bCo[color]lor\b[size][highlight]!", Transparent, MonokaiWhite))
								{
									//TODO: Implement me!
								} Clay__CloseElement();
								
								if (ClayBtn("Close Program", Transparent, MonokaiWhite))
								{
									sapp_request_quit();
								} Clay__CloseElement();
								
								Clay__CloseElement();
								Clay__CloseElement();
							}
							Clay__CloseElement();
							
							CLAY({ .layout={ .sizing={ .width=CLAY_SIZING_FIXED(16) } } }) {}
							
							u64 utcTimestamp = OsGetCurrentTimestamp(false);
							i64 timezoneOffset = 0;
							u64 localTimestamp = OsGetCurrentTimestampEx(true, &timezoneOffset, nullptr);
							Str8 displayStr = ScratchPrintStr("UTC: %llu Local: %llu (%s%lld)", utcTimestamp, localTimestamp, timezoneOffset >= 0 ? "+" : "-", AbsI64(timezoneOffset));
							CLAY_TEXT(
								displayStr,
								CLAY_TEXT_CONFIG({
									.fontId = clayFont,
									.fontSize = 18*textScale,
									.textColor = MonokaiWhite,
								})
							);
						}
					}
				}
			}
			Clay_RenderCommandArray clayRenderCommands = EndClayUIRender(&clay.clay);
			RenderClayCommandArray(&clay, &gfx, &clayRenderCommands);
			#endif //BUILD_WITH_CLAY
			
			#if BUILD_WITH_IMGUI
			GfxSystem_ImguiBeginFrame(&gfx, imgui);
			if (igBeginMainMenuBar())
			{
				if (igBeginMenu("Menu", true))
				{
					igMenuItem_BoolPtr("Demo Window", nullptr, &isImguiDemoWindowOpen, true);
					igMenuItem_BoolPtr("C Tokenizer", nullptr, &isCTokenizerWindowOpen, true);
					if (igMenuItem_Bool("Close", "Alt+F4", false, true)) { sapp_request_quit(); }
					igEndMenu();
				}
				// igSameLine(igGetWindowWidth() - 120, 0);
				// igTextColored(ToImVec4FromColor(MonokaiGray1), "(%s to Toggle)", GetKeyStr(IMGUI_TOPBAR_TOGGLE_HOTKEY));
				igEndMainMenuBar();
			}
			if (isImguiDemoWindowOpen)
			{
				igShowDemoWindow(&isImguiDemoWindowOpen);
			}
			
			// +==============================+
			// |   C Tokenizer Imgui Window   |
			// +==============================+
			if (isCTokenizerWindowOpen)
			{
				if (IsKeyboardKeyPressed(&keyboard, Key_R, false) && tokenizer.arena != nullptr)
				{
					FreeStr8(stdHeap, &tokenizer.inputStr);
					FreeCTokenizer(&tokenizer);
				}
				
				if (tokenizer.arena == nullptr)
				{
					Str8 fileContents = Str8_Empty;
					if (OsReadTextFile(FilePathLit("tokenizer_test.c"), stdHeap, &fileContents))
					{
						tokenizer = NewCTokenizer(stdHeap, fileContents);
					}
					else
					{
						tokenizer = NewCTokenizer(stdHeap, Str8_Empty);
					}
				}
				
				if (igBegin("C Tokenizer", &isCTokenizerWindowOpen, ImGuiWindowFlags_None))
				{
					igText("%llu Token%s: (%s)", (u64)tokenizer.tokens.length, Plural(tokenizer.tokens.length, "s"), GetResultStr(tokenizer.error));
					igIndent(1.0f);
					
					tokenizer.outputTokenIndex = 0;
					tokenizer.finished = false;
					cToken* token = NextCToken(&tokenizer);
					while (token != nullptr)
					{
						igText("[%llu] %s \"%.*s\" (Raw \"%.*s\")",
							(u64)token->index,
							GetcTokenTypeStr(token->type),
							StrPrint(token->str),
							StrPrint(token->rawStr)
						);
						token = NextCToken(&tokenizer);
					}
					
					igUnindent(1.0f);
				}
				igEnd();
			}
			else if (tokenizer.arena != nullptr)
			{
				FreeStr8(stdHeap, &tokenizer.inputStr);
				FreeCTokenizer(&tokenizer);
			}
			
			GfxSystem_ImguiEndFrame(&gfx, imgui);
			#endif
		}
	}
	EndFrame();
	TracyCZoneEnd(Zone_Draw);
	
	TracyCZoneN(Zone_Commit, "Commit", true);
	sg_commit();
	TracyCZoneEnd(Zone_Commit);
	
	// PrintLine_D("numPipelineChanges: %llu", gfx.numPipelineChanges);
	// PrintLine_D("numBindingChanges: %llu", gfx.numBindingChanges);
	// PrintLine_D("numDrawCalls: %llu", gfx.numDrawCalls);
	gfx.numPipelineChanges = 0;
	gfx.numBindingChanges = 0;
	gfx.numDrawCalls = 0;
	RefreshMouseState(&mouse, sapp_mouse_locked(), NewV2(sapp_widthf()/2.0f, sapp_heightf()/2.0f));
	RefreshKeyboardState(&keyboard);
	RefreshTouchscreenState(&touchscreen);
	#if TARGET_IS_ANDROID
	screenRotated = false;
	#endif
	ScratchEnd(scratch);
	return frameRendered;
}

// +--------------------------------------------------------------+
// |                            Event                             |
// +--------------------------------------------------------------+
void AppEvent(const sapp_event* event)
{
	TracyCZoneN(Zone_Func, "AppEvent", true);
	bool handledEvent = HandleSokolKeyboardMouseAndTouchEvents(event, programTime, NewV2i(sapp_width(), sapp_height()), &keyboard, &mouse, &touchscreen, sapp_mouse_locked());
	
	if (!handledEvent)
	{
		switch (event->type)
		{
			case SAPP_EVENTTYPE_RESIZED:           PrintLine_D("Event: RESIZED %dx%d / %dx%d", event->window_width, event->window_height, event->framebuffer_width, event->framebuffer_height); break;
			case SAPP_EVENTTYPE_ICONIFIED:         WriteLine_D("Event: ICONIFIED");         break;
			case SAPP_EVENTTYPE_RESTORED:          WriteLine_D("Event: RESTORED");          break;
			case SAPP_EVENTTYPE_FOCUSED:           WriteLine_D("Event: FOCUSED");           break;
			case SAPP_EVENTTYPE_UNFOCUSED:         WriteLine_D("Event: UNFOCUSED");         break;
			case SAPP_EVENTTYPE_SUSPENDED:         WriteLine_D("Event: SUSPENDED");         break;
			case SAPP_EVENTTYPE_RESUMED:           WriteLine_D("Event: RESUMED");           break;
			case SAPP_EVENTTYPE_QUIT_REQUESTED:    WriteLine_D("Event: QUIT_REQUESTED");    break;
			case SAPP_EVENTTYPE_CLIPBOARD_PASTED:  WriteLine_D("Event: CLIPBOARD_PASTED");  break;
			case SAPP_EVENTTYPE_FILES_DROPPED:     WriteLine_D("Event: FILES_DROPPED");     break;
			default: PrintLine_D("Event: UNKNOWN(%d)", event->type); break;
		}
	}
	
	TracyCZoneEnd(Zone_Func);
}

// +--------------------------------------------------------------+
// |                         Entry Point                          |
// +--------------------------------------------------------------+
sapp_desc sokol_main(int argc, char* argv[])
{
	TracyCZoneN(Zone_Func, "sokol_main", true);
	//NOTE: The App callbacks may happen on a different thread than this one!
	UNUSED(argc);
	UNUSED(argv);
	
	//NOTE: On some platforms (like Android) this call happens on a separate thread to AppInit, AppFrame, etc. So we shouldn't do any initialization here that is thread specific
	argc_copy = argc;
	argv_copy = argv;
	
	sapp_desc result = {
		.init_cb = AppInit,
		.frame_cb = AppFrame,
		.cleanup_cb = AppCleanup,
		.event_cb = AppEvent,
		.width = 1900,
		.height = 1000,
		.high_dpi = true,
		.window_title = "Simple Sokol App!",
		.icon.sokol_default = true,
		.logger.func = SokolLogCallback,
		.enable_touch_input = true,
	};
	
	TracyCZoneEnd(Zone_Func);
	return result;
}

#endif //BUILD_WITH_SOKOL_GFX && BUILD_WITH_SOKOL_APP
