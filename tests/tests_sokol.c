/*
File:   tests_sokol.c
Author: Taylor Robbins
Date:   01\28\2025
Description: 
	** Holds various tests that run inside a Sokol window (sokol_app.h + sokol_gfx.h)
*/

#if BUILD_WITH_SOKOL_APP

#if TARGET_IS_ANDROID
#define MAIN_FONT_NAME "DroidSansMono"
#else
#define MAIN_FONT_NAME "Consolas"
#endif

#include "lib/lib_sokol_app_impl.c"

#endif //BUILD_WITH_SOKOL_APP

#if BUILD_WITH_SOKOL_GFX && BUILD_WITH_SOKOL_APP

#include "tests/simple_shader.glsl.h"
#include "tests/main2d_shader.glsl.h"
#include "tests/main3d_shader.glsl.h"

static void EarlyInit();
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
PigFont debugFont;
VertBuffer cubeBuffer;
VertBuffer sphereBuffer;
OsTime prevFrameTime = OsTime_Zero_Const;
uxx frameIndex = 0;
u64 programTime = 0;
r32 elapsedMs = 0.0f;
r32 timeScale = 1.0f;
r32 prevUpdateMs = 0.0f;
bool showPerfGraph = false;
MouseState mouse = ZEROED;
KeyboardState keyboard = ZEROED;
TouchscreenState touchscreen = ZEROED;
v3 cameraPos = MakeV3_Const(0.0f, 1.0f, 0.0f);
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
Texture mipmapTexture = ZEROED;
Texture noMipmapTexture = ZEROED;
Texture testTexture = ZEROED;
PerfGraph perfGraph = ZEROED;
#if BUILD_WITH_PIG_UI
UiContext uiContext = ZEROED;
r32 uiScale = 1.0f;
#endif

//TODO: Somehow we need to detect how big our text should be in order to be a particular size on screen with consideration for high DPI displays
#if TARGET_IS_ANDROID
#define TEXT_SCALE 3.0f
#else
#define TEXT_SCALE 1.0f
#endif

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
		MakeStr8Nt(btnText),
		CLAY_TEXT_CONFIG({
			.fontId = clayFont,
			.fontSize = (u16)(18*textScale),
			.textColor = textColor,
		})
	);
	bool isHovered = (Clay_PointerOver(btnId) || Clay_PointerOver(menuId));
	if (Clay_PointerOver(btnId) && (IsMouseBtnPressed(&mouse, nullptr, MouseBtn_Left) || touchscreen.mainTouch->started))
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
	bool isPressed = (isHovered && (IsMouseBtnDown(&mouse, nullptr, MouseBtn_Left) || (touchscreen.mainTouch->id != TOUCH_ID_INVALID && !touchscreen.mainTouch->stopped)));
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
		MakeStr8Nt(btnText),
		CLAY_TEXT_CONFIG({
			.fontId = clayFont,
			.fontSize = (u16)(18*textScale),
			.textColor = textColor,
			.userData = { .richText = true },
		})
	);
	ScratchEnd(scratch);
	return (isHovered && (IsMouseBtnPressed(&mouse, nullptr, MouseBtn_Left) || touchscreen.mainTouch->started));
}
#endif //BUILD_WITH_CLAY

#if BUILD_WITH_PIG_UI
// +==============================+
// | TestsGlobalUiThemerCallback  |
// +==============================+
// bool TestsGlobalUiThemerCallback(plex UiContext* context, UiElement* element, void* userPntr)
UI_THEMER_CALLBACK_DEF(TestsGlobalUiThemerCallback)
{
	// if (AreEqualV4r(element->config.borderThickness, V4r_Zero) && element->config.borderColor.valueU32 == PigUiDefaultColor_Value)
	// {
	// 	element->config.borderThickness = FillV4r(2.0f);
	// 	element->config.borderColor = ColorWithAlpha(Black, 0.5f);
	// }
	// if (element->depth >= (IsKeyboardKeyDown(&keyboard, nullptr, Key_Shift) ? 4 : 3)) { return false; }
	u8 borderAlpha = element->config.borderColor.a;
	element->config.borderColor = ColorLerpSimple(GetPredefPalColorByIndex(element->id.id), White, 0.5f);
	element->config.borderColor.a = borderAlpha;
	return true;
}
// +==============================+
// |    TestsUiThemerCallback     |
// +==============================+
// bool TestsUiThemerCallback(plex UiContext* context, UiElement* element, void* userPntr)
UI_THEMER_CALLBACK_DEF(TestsUiThemerCallback)
{
	if (!element->config.themer.isButton && element->config.texture == nullptr)
	{
		element->config.texture = &testTexture;
		element->config.dontSizeToTexture = true;
	}
	return true;
}
#endif //BUILD_WITH_PIG_UI

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

Texture LoadTexture(Arena* arena, Str8 path, TextureFlag flags)
{
	ScratchBegin1(scratch, arena);
	Texture result = ZEROED;
	Slice fileContents = Slice_Empty;
	bool readFileResult = OsReadBinFile(path, scratch, &fileContents);
	if (!readFileResult)
	{
		DebugAssertMsg(readFileResult == true, "Failed to find texture file!");
		result.error = Result_FailedToReadFile;
		ScratchEnd(scratch);
		return result;
	}
	ImageData imageData = ZEROED;
	Result parseResult = TryParseImageFile(fileContents, arena, &imageData);
	if (parseResult != Result_Success)
	{
		DebugAssertMsg(parseResult == Result_Success, "Failed to parse texture file!");
		result.error = parseResult;
		ScratchEnd(scratch);
		return result;
	}
	result = InitTexture(arena, path, imageData.size, imageData.pixels, flags);
	ScratchEnd(scratch);
	return result;
}

Result TryAttachLocalFontFile(PigFont* font, Str8 fileName, u8 styleFlags)
{
	ScratchBegin1(scratch, font->arena);
	Str8 filePath = JoinStringsInArena(scratch, StrLit("../_fonts/"), fileName, false);
	Slice fileContents = Slice_Empty;
	if (!OsReadBinFile(filePath, scratch, &fileContents)) { ScratchEnd(scratch); return Result_FailedToReadFile; }
	Result result = TryAttachFontFile(font, fileName, fileContents, styleFlags, true);
	ScratchEnd(scratch);
	return result;
}

void UpdateTimingInfo()
{
	OsTime currentTime = OsGetTime();
	OsTime prevTime = prevFrameTime;
	if (frameIndex == 0) { prevTime = currentTime; } //ignore difference between 0 and first frame time
	prevFrameTime = currentTime;
	programTime = currentTime.msSinceStart;
	elapsedMs = ClampR32(OsTimeDiffMsR32(prevTime, currentTime), 5.0f, 67.0f);
	timeScale = elapsedMs / (1000.0f / 60.0f); //TODO: How do we know the target framerate?
	if (AreSimilarR32(timeScale, 1.0f, 0.1f)) { timeScale = 1.0; }
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void AppInit(void)
{
	MyMain(argc_copy, argv_copy); //call MyMain to initialize arenas and whatnot
	
	ScratchBegin(scratch);
	InitSokolGraphics((sg_desc){
		.environment = GetSokolGfxEnvironment(),
		.logger.func = SokolLogCallback,
		.metal.use_command_buffer_with_retained_references = true,
	});
	
	InitGfxSystem(stdHeap, &gfx);
	
	v2i gradientSize = FillV2i(64);
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
	
	gradientTexture = InitTexture(stdHeap, StrLit("gradient"), gradientSize, gradientPixels, TextureFlag_IsRepeating|TextureFlag_NoMipmaps);
	Assert(gradientTexture.error == Result_Success);
	
	#if !TARGET_IS_OSX //TODO: Remove me once we get fonts working on OSX
	const u32 Filled = 0xFFFFFFFF;
	const u32 _Empty = 0x00FFFFFF;
	u32 checkerGlyph18Pixels[12*18] = {
		_Empty, _Empty, _Empty, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, _Empty, _Empty,
		_Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, _Empty,
		_Empty, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty,
		_Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled,
		Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty,
		_Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled,
		Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty,
		_Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled,
		Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty,
		_Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled,
		Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty,
		_Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled,
		Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty,
		_Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled,
		Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty,
		_Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, _Empty,
		_Empty, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, _Empty, _Empty,
		_Empty, _Empty, _Empty, Filled, _Empty, Filled, _Empty, Filled, _Empty, _Empty, _Empty, _Empty,
	};
	ImageData checkerGlyph18ImageData = MakeImageData(MakeV2i(12, 18), &checkerGlyph18Pixels[0]);
	CustomFontGlyph checkerGlyph18 = { .codepoint=UNICODE_UNKNOWN_CHAR_CODEPOINT, .imageData=checkerGlyph18ImageData, .sourceRec = MakeReci(0, 0, 12, 18) };
	CustomFontCharRange customCharRanges[] = {
		{ .startCodepoint=UNICODE_UNKNOWN_CHAR_CODEPOINT, .endCodepoint=UNICODE_UNKNOWN_CHAR_CODEPOINT, .glyphs=&checkerGlyph18 },
	};
	r32 textScale = TEXT_SCALE/sapp_dpi_scale();
	bool useActiveFont = true;
	testFont = InitFont(stdHeap, StrLit("testFont"));
	if (useActiveFont)
	{
		Result attachResult = Result_None;
		Result bakeResult = Result_None;
		FontCharRange basicCharRanges[] = {
			FontCharRange_ASCII,
			// FontCharRange_LowercaseLetters,
			// FontCharRange_UppercaseLetters,
			// FontCharRange_LatinSupplementAccent,
			// MakeFontCharRangeSingle(UNICODE_SPACE_CODEPOINT),
			// MakeFontCharRangeSingle(UNICODE_ZERO_WIDTH_SPACE_CODEPOINT),
			// MakeFontCharRangeSingle(UNICODE_NON_BREAKING_SPACE_CODEPOINT),
			// MakeFontCharRangeSingle(UNICODE_NON_BREAKING_HYPHEN_CODEPOINT),
			// MakeFontCharRangeSingle(UNICODE_UNKNOWN_CHAR_CODEPOINT),
		};
		// FontCharRange japaneseCharRanges[] = {
		// 	FontCharRange_Hiragana,
		// 	FontCharRange_Katakana,
		// };
		
		// attachResult = TryAttachOsTtfFileToFont(&testFont, StrLit(MAIN_FONT_NAME), 18*textScale, FontStyleFlag_None); Assert(attachResult == Result_Success);
		// bakeResult = TryBakeFontAtlasWithCustomGlyphs(&testFont, 18*textScale, FontStyleFlag_None, 256, 1024, ArrayCount(basicCharRanges), &basicCharRanges[0], ArrayCount(customCharRanges), &customCharRanges[0]); Assert(bakeResult == Result_Success);
		// FillFontKerningTable(&testFont);
		// RemoveAttachedFontFiles(&testFont);
		
		// attachResult = TryAttachLocalFontFile(&testFont, StrLit("NotoSansJP-Regular.ttf"), FontStyleFlag_None); Assert(attachResult == Result_Success);
		// bakeResult = TryBakeFontAtlas(&testFont, 18*textScale, FontStyleFlag_None, 256, 1024, ArrayCount(japaneseCharRanges), &japaneseCharRanges[0]); Assert(bakeResult == Result_Success);
		// RemoveAttachedFontFiles(&testFont);
		
		#if 0
		FontCharRange emojiCharRanges[] = {
			MakeFontCharRange(0x1F90C, 0x1F91F),
			MakeFontCharRangeSingle(0x1F60A), //😊
			MakeFontCharRangeSingle(0x1F602), //😂
			MakeFontCharRangeSingle(0x1F923), //🤣
			MakeFontCharRangeSingle(0x1F612), //😒
			MakeFontCharRangeSingle(0x1F601), //😁
			MakeFontCharRangeSingle(0x1F60D), //😍
			MakeFontCharRangeSingle(0x1F64C), //🙌
		};
		attachResult = TryAttachLocalFontFile(&testFont, StrLit("seguiemj.ttf"), FontStyleFlag_ColoredGlyphs); Assert(attachResult == Result_Success);
		bakeResult = TryBakeFontAtlas(&testFont, 18*textScale, FontStyleFlag_None, 256, 1024, ArrayCount(emojiCharRanges), &emojiCharRanges[0]); Assert(bakeResult == Result_Success);
		RemoveAttachedFontFiles(&testFont);
		attachResult = TryAttachLocalFontFile(&testFont, StrLit("NotoColorEmoji-Regular.ttf"), FontStyleFlag_ColoredGlyphs); Assert(attachResult == Result_Success);
		bakeResult = TryBakeFontAtlas(&testFont, 18*textScale, FontStyleFlag_None, 256, 1024, ArrayCount(emojiCharRanges), &emojiCharRanges[0]); Assert(bakeResult == Result_Success);
		RemoveAttachedFontFiles(&testFont);
		#endif
		
		MakeFontActive(&testFont, 64, 256, 5, 0, 0);
		attachResult = TryAttachOsTtfFileToFont(&testFont, StrLit(MAIN_FONT_NAME), 18*textScale, FontStyleFlag_None); Assert(attachResult == Result_Success);
		attachResult = TryAttachOsTtfFileToFont(&testFont, StrLit(MAIN_FONT_NAME), 18*textScale, FontStyleFlag_Bold); Assert(attachResult == Result_Success);
		attachResult = TryAttachOsTtfFileToFont(&testFont, StrLit(MAIN_FONT_NAME), 18*textScale, FontStyleFlag_Italic); Assert(attachResult == Result_Success);
		attachResult = TryAttachOsTtfFileToFont(&testFont, StrLit(MAIN_FONT_NAME), 18*textScale, FontStyleFlag_Bold|FontStyleFlag_Italic); Assert(attachResult == Result_Success);
		// attachResult = TryAttachOsTtfFileToFont(&testFont, StrLit("Meiryo UI Regular"), 18*textScale, FontStyleFlag_None); Assert(attachResult == Result_Success);
		attachResult = TryAttachLocalFontFile(&testFont, StrLit("NotoSansJP-Regular.ttf"), FontStyleFlag_None); Assert(attachResult == Result_Success);
		// attachResult = TryAttachOsTtfFileToFont(&testFont, StrLit("Segoe UI Symbol"), 18*textScale, FontStyleFlag_None); Assert(attachResult == Result_Success);
		attachResult = TryAttachLocalFontFile(&testFont, StrLit("NotoSansSymbols-Regular.ttf"), FontStyleFlag_None); Assert(attachResult == Result_Success);
		// attachResult = TryAttachOsTtfFileToFont(&testFont, StrLit("Segoe UI Symbol"), 18*textScale, FontStyleFlag_Bold); Assert(attachResult == Result_Success);
		// attachResult = TryAttachLocalFontFile(&testFont, StrLit("NotoEmoji-Regular.ttf"), FontStyleFlag_None); Assert(attachResult == Result_Success);
		#if TARGET_IS_WINDOWS //TODO: Remove this once we have NotoColorEmoji-Regular.ttf committed to the repository
		attachResult = TryAttachLocalFontFile(&testFont, StrLit("NotoColorEmoji-Regular.ttf"), FontStyleFlag_ColoredGlyphs); Assert(attachResult == Result_Success);
		#endif
		// attachResult = TryAttachLocalFontFile(&testFont, StrLit("seguiemj.ttf"), FontStyleFlag_ColoredGlyphs); Assert(attachResult == Result_Success);
		
		bakeResult = TryBakeFontAtlasWithCustomGlyphs(&testFont, 18*textScale, FontStyleFlag_None, 256, 1024, ArrayCount(basicCharRanges), &basicCharRanges[0], ArrayCount(customCharRanges), &customCharRanges[0]); Assert(bakeResult == Result_Success || bakeResult == Result_Partial);
		// bakeResult = TryBakeFontAtlas(&testFont, 18*textScale, FontStyleFlag_None, 256, 1024, ArrayCount(japaneseCharRanges), &japaneseCharRanges[0]); Assert(bakeResult == Result_Success || bakeResult == Result_Partial);
	}
	else
	{
		FontCharRange charRanges[] = {
			FontCharRange_ASCII,
			FontCharRange_LatinSupplementAccent,
		};
		// OsWriteBinFile(FilePathLit("Default.ttf"), testFont.ttfFile);
		FontBakeSettings bakeSettings[] = {
			{ .name=StrLit(MAIN_FONT_NAME), .size=18*textScale, .style=FontStyleFlag_None, .fillKerningTable=true },
			{ .name=StrLit(MAIN_FONT_NAME), .size=10*textScale, .style=FontStyleFlag_None },
			{ .name=StrLit(MAIN_FONT_NAME), .size=26*textScale, .style=FontStyleFlag_None },
			{ .name=StrLit(MAIN_FONT_NAME), .size=18*textScale, .style=FontStyleFlag_Bold },
			{ .name=StrLit(MAIN_FONT_NAME), .size=10*textScale, .style=FontStyleFlag_Bold },
			{ .name=StrLit(MAIN_FONT_NAME), .size=26*textScale, .style=FontStyleFlag_Bold },
			{ .name=StrLit(MAIN_FONT_NAME), .size=18*textScale, .style=FontStyleFlag_Italic },
			{ .name=StrLit(MAIN_FONT_NAME), .size=10*textScale, .style=FontStyleFlag_Italic },
			{ .name=StrLit(MAIN_FONT_NAME), .size=26*textScale, .style=FontStyleFlag_Italic },
			{ .name=StrLit(MAIN_FONT_NAME), .size=18*textScale, .style=FontStyleFlag_Bold|FontStyleFlag_Italic },
			{ .name=StrLit(MAIN_FONT_NAME), .size=10*textScale, .style=FontStyleFlag_Bold|FontStyleFlag_Italic },
			{ .name=StrLit(MAIN_FONT_NAME), .size=26*textScale, .style=FontStyleFlag_Bold|FontStyleFlag_Italic },
		};
		
		Result bakeResult = TryAttachAndMultiBakeFontAtlases(&testFont, ArrayCount(bakeSettings), &bakeSettings[0], 256, 1024, ArrayCount(charRanges), &charRanges[0]);
		Assert(bakeResult == Result_Success);
	}
	
	debugFont = InitFont(stdHeap, StrLit("debugFont"));
	{
		FontCharRange charRanges[] = {
			FontCharRange_ASCII,
			FontCharRange_LatinSupplementAccent,
		};
		FontBakeSettings bakeSettings[] = {
			{ .name=StrLit("Consolas"), .size=12*textScale, .style=FontStyleFlag_None },
			{ .name=StrLit("Consolas"), .size=12*textScale, .style=FontStyleFlag_Bold },
			{ .name=StrLit("Consolas"), .size=12*textScale, .style=FontStyleFlag_Italic },
			{ .name=StrLit("Consolas"), .size=12*textScale, .style=FontStyleFlag_Bold|FontStyleFlag_Italic },
		};
		Result bakeResult = TryAttachAndMultiBakeFontAtlases(&debugFont, ArrayCount(bakeSettings), &bakeSettings[0], 128, 512, ArrayCount(charRanges), &charRanges[0]);
		Assert(bakeResult == Result_Success);
	}
	#endif //!TARGET_IS_OSX
	
	GeneratedMesh cubeMesh = GenerateVertsForBox(scratch, MakeBoxV(V3_Zero, V3_One), White);
	Vertex3D* cubeVertices = AllocArray(Vertex3D, scratch, cubeMesh.numIndices);
	for (uxx iIndex = 0; iIndex < cubeMesh.numIndices; iIndex++)
	{
		MyMemCopy(&cubeVertices[iIndex], &cubeMesh.vertices[cubeMesh.indices[iIndex]], sizeof(Vertex3D));
	}
	cubeBuffer = InitVertBuffer3D(stdHeap, StrLit("cube"), VertBufferUsage_Static, cubeMesh.numIndices, cubeVertices, false);
	Assert(cubeBuffer.error == Result_Success);
	
	GeneratedMesh sphereMesh = GenerateVertsForSphere(scratch, MakeSphereV(V3_Zero, 1.0f), 12, 20, White);
	Vertex3D* sphereVertices = AllocArray(Vertex3D, scratch, sphereMesh.numIndices);
	for (uxx iIndex = 0; iIndex < sphereMesh.numIndices; iIndex++)
	{
		MyMemCopy(&sphereVertices[iIndex], &sphereMesh.vertices[sphereMesh.indices[iIndex]], sizeof(Vertex3D));
	}
	sphereBuffer = InitVertBuffer3D(stdHeap, StrLit("sphere"), VertBufferUsage_Static, sphereMesh.numIndices, sphereVertices, false);
	Assert(sphereBuffer.error == Result_Success);
	
	InitPerfGraph(&perfGraph, 1000.0f/60.0f); //TODO: How do we know the target framerate?
	
	InitCompiledShader(&simpleShader, stdHeap, simple); Assert(simpleShader.error == Result_Success);
	InitCompiledShader(&main2dShader, stdHeap, main2d); Assert(main2dShader.error == Result_Success);
	InitCompiledShader(&main3dShader, stdHeap, main3d); Assert(main3dShader.error == Result_Success);
	
	#if BUILD_WITH_CLAY
	AssertMsg(testFont.atlases.length > 0, "Clay rendering requires that at least one atlas in the testFont was baked correctly!");
	InitClayUIRenderer(stdHeap, V2_Zero, &clay);
	clayFont = AddClayUIRendererFont(&clay, &testFont, GetDefaultFontStyleFlags(&testFont));
	#endif
	
	#if BUILD_WITH_PIG_UI
	InitUiContext(stdHeap, &uiContext);
	#endif
	
	#if BUILD_WITH_IMGUI
	FlagSet(stdHeap->flags, ArenaFlag_AllowFreeWithoutSize);
	#if TARGET_IS_WINDOWS
	const void* nativeWindowHandle = sapp_win32_get_hwnd();
	imgui = InitImguiUI(stdHeap, nativeWindowHandle);
	#else
	// const void* nativeWindowHandle = sapp_x11_get_window();
	imgui = InitImguiUI(stdHeap);
	#endif
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
	oldWindowSize = MakeV2i(sapp_width(), sapp_height());
	
	#if !TARGET_IS_OSX //TODO: Remove me once we get files working on OSX
	#if TARGET_IS_WINDOWS
	FilePath testImagePath = FilePathLit("Q:/test.png");
	#elif TARGET_IS_LINUX
	FilePath testImagePath = FilePathLit("/home/robbitay/test.png");
	#else
	FilePath testImagePath = FilePathLit("test.png");
	#endif
	mipmapTexture = LoadTexture(stdHeap, testImagePath, TextureFlag_None);
	noMipmapTexture = LoadTexture(stdHeap, testImagePath, TextureFlag_NoMipmaps);
	#endif //!TARGET_IS_OSX
	
	ImageData testTextureData = ZEROED;
	testTextureData.size = MakeV2i(512, 512);
	testTextureData.numPixels = (uxx)(testTextureData.size.Width * testTextureData.size.Height);
	testTextureData.pixels = AllocArray(u32, scratch, testTextureData.numPixels);
	NotNull(testTextureData.pixels);
	for (i32 yOffset = 0; yOffset < testTextureData.size.Height; yOffset++)
	{
		for (i32 xOffset = 0; xOffset < testTextureData.size.Width; xOffset++)
		{
			Color32* pixel = (Color32*)&testTextureData.pixels[INDEX_FROM_COORD2D(xOffset, yOffset, testTextureData.size.Width, testTextureData.size.Height)];
			//TODO: GetRandU8 causes a very noticable pattern!
			// pixel->r = GetRandU8(mainRandom);
			// pixel->g = GetRandU8(mainRandom);
			// pixel->b = GetRandU8(mainRandom);
			pixel->valueU32 = GetRandU32(mainRandom);
			pixel->a = 255;
		}
	}
	testTexture = InitTexture(stdHeap, StrLit("testTexture"), testTextureData.size, testTextureData.pixels, TextureFlag_HasCopy);
	Assert(testTexture.error == Result_Success);
	
	OsMarkStartTime();
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
	OsTime beforeUpdateTime = OsGetTime();
	
	UpdateTimingInfo();
	ScratchBegin(scratch);
	bool frameRendered = true;
	v2i windowSizei = MakeV2i(sapp_width(), sapp_height());
	v2 windowSize = MakeV2(sapp_widthf(), sapp_heightf());
	// v2 touchPos = touchscreen.mainTouch->pos;
	#if TARGET_IS_ANDROID
	UpdateScreenRotation();
	#endif
	if (AreEqualV2i(oldWindowSize, windowSizei)) { UpdateScreenSafeMargins(); }
	if (frameIndex > 0) { UpdatePerfGraph(&perfGraph, prevUpdateMs, (elapsedMs - prevUpdateMs)); }
	#if !TARGET_IS_OSX //TODO: Remove me once we get fonts working on OSX
	FontNewFrame(&testFont, programTime);
	#endif
	
	if (IsMouseBtnDown(&mouse, nullptr, MouseBtn_Left)) { wrapPos = mouse.position; }
	if (touchscreen.mainTouch->id != TOUCH_ID_INVALID) { wrapPos = touchscreen.mainTouch->pos; }
	
	if (IsKeyboardKeyPressed(&keyboard, nullptr, Key_F, false)) { sapp_lock_mouse(!sapp_mouse_locked()); }
	if (IsKeyboardKeyPressed(&keyboard, nullptr, Key_Escape, false) && sapp_mouse_locked()) { sapp_lock_mouse(false); }
	if (sapp_mouse_locked())
	{
		r32 cameraHoriRot = AtanR32(cameraLookDir.Z, cameraLookDir.X);
		r32 cameraVertRot = AtanR32(cameraLookDir.Y, Length(MakeV2(cameraLookDir.X, cameraLookDir.Z)));
		cameraHoriRot = AngleFixR32(cameraHoriRot - mouse.lockedPosDelta.X / 500.0f);
		cameraVertRot = ClampR32(cameraVertRot - mouse.lockedPosDelta.Y / 500.0f, -HalfPi32+0.05f, HalfPi32-0.05f);
		r32 horizontalRadius = CosR32(cameraVertRot);
		cameraLookDir = MakeV3(CosR32(cameraHoriRot) * horizontalRadius, SinR32(cameraVertRot), SinR32(cameraHoriRot) * horizontalRadius);
		
		v3 horizontalForwardVec = Normalize(MakeV3(cameraLookDir.X, 0.0f, cameraLookDir.Z));
		v3 horizontalRightVec = Normalize(MakeV3(cameraLookDir.Z, 0.0f, -cameraLookDir.X));
		const r32 moveSpeed = IsKeyboardKeyDown(&keyboard, nullptr, Key_Shift) ? 0.08f : 0.02f;
		if (IsKeyboardKeyDown(&keyboard, nullptr, Key_W)) { cameraPos = Add(cameraPos, Mul(horizontalForwardVec, moveSpeed)); }
		if (IsKeyboardKeyDown(&keyboard, nullptr, Key_A)) { cameraPos = Add(cameraPos, Mul(horizontalRightVec, -moveSpeed)); }
		if (IsKeyboardKeyDown(&keyboard, nullptr, Key_S)) { cameraPos = Add(cameraPos, Mul(horizontalForwardVec, -moveSpeed)); }
		if (IsKeyboardKeyDown(&keyboard, nullptr, Key_D)) { cameraPos = Add(cameraPos, Mul(horizontalRightVec, moveSpeed)); }
		if (IsKeyboardKeyDown(&keyboard, nullptr, Key_E)) { cameraPos = Add(cameraPos, Mul(V3_Up, moveSpeed)); }
		if (IsKeyboardKeyDown(&keyboard, nullptr, Key_Q)) { cameraPos = Add(cameraPos, Mul(V3_Down, moveSpeed)); }
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
				r32 cameraVertRot = AtanR32(cameraLookDir.Y, Length(MakeV2(cameraLookDir.X, cameraLookDir.Z)));
				cameraHoriRot = AngleFixR32(cameraHoriRot - delta.X / 500.0f);
				cameraVertRot = ClampR32(cameraVertRot - delta.Y / 500.0f, -HalfPi32+0.05f, HalfPi32-0.05f);
				r32 horizontalRadius = CosR32(cameraVertRot);
				cameraLookDir = MakeV3(CosR32(cameraHoriRot) * horizontalRadius, SinR32(cameraVertRot), SinR32(cameraHoriRot) * horizontalRadius);
			}
		}
	}
	
	if (IsKeyboardKeyPressed(&keyboard, nullptr, Key_F6, false))
	{
		showPerfGraph = !showPerfGraph;
	}
	
	if (IsKeyboardKeyPressed(&keyboard, nullptr, Key_P, true))
	{
		reci sourceRec = MakeReci(
			GetRandI32Range(mainRandom, 0, testTexture.Width-1),
			GetRandI32Range(mainRandom, 0, testTexture.Height-1),
			0, 0
		);
		sourceRec.Width = GetRandI32Range(mainRandom, 1, (testTexture.Width - sourceRec.X)+1);
		sourceRec.Height = GetRandI32Range(mainRandom, 1, (testTexture.Height - sourceRec.Y)+1);
		ImageData newImageData = ZEROED;
		newImageData.size = sourceRec.Size;
		newImageData.numPixels = (uxx)(newImageData.size.Width * newImageData.size.Height);
		newImageData.pixels = AllocArray(u32, scratch, newImageData.numPixels);
		NotNull(newImageData.pixels);
		Color32 color = GetPredefPalColorByIndex(GetRandU32(mainRandom));
		for (uxx pIndex = 0; pIndex < newImageData.numPixels; pIndex++)
		{
			newImageData.pixels[pIndex] = color.valueU32;
		}
		UpdateTexturePart(&testTexture, sourceRec, newImageData.pixels);
	}
	
	#if !TARGET_IS_OSX //TODO: Remove me once we get fonts working on OSX
	if (IsKeyboardKeyPressed(&keyboard, nullptr, Key_G, false))
	{
		PrintLine_D("testFont has %llu atlas%s:", testFont.atlases.length, PluralEx(testFont.atlases.length, "", "es"));
		VarArrayLoop(&testFont.atlases, aIndex)
		{
			VarArrayLoopGet(FontAtlas, fontAtlas, &testFont.atlases, aIndex);
			bool isBold = IsFlagSet(fontAtlas->styleFlags, FontStyleFlag_Bold);
			bool isItalic = IsFlagSet(fontAtlas->styleFlags, FontStyleFlag_Italic);
			PrintLine_D("Atlas[%llu]:%s %g %s%s%s %dx%d 0x%08X-0x%08X",
				aIndex,
				fontAtlas->isActive ? " Active" : "",
				fontAtlas->fontSize,
				isBold ? "Bold" : "", (isBold && isItalic) ? "|" : "", isItalic ? "Italic" : "",
				fontAtlas->texture.Width, fontAtlas->texture.Height,
				fontAtlas->glyphRange.startCodepoint, fontAtlas->glyphRange.endCodepoint
			);
			PrintLine_D("\t%llu Char Range%s:", fontAtlas->charRanges.length, Plural(fontAtlas->charRanges.length, "s"));
			VarArrayLoop(&fontAtlas->charRanges, rIndex)
			{
				VarArrayLoopGet(FontCharRange, charRange, &fontAtlas->charRanges, rIndex);
				PrintLine_D("\t\tRange[%llu]: 0x%08X-0x%08X glyphIndex=%llu", rIndex, charRange->startCodepoint, charRange->endCodepoint, charRange->glyphArrayStartIndex);
			}
			PrintLine_D("\t%llu Glyph%s:", fontAtlas->glyphs.length, Plural(fontAtlas->glyphs.length, "s"));
			VarArrayLoop(&fontAtlas->glyphs, gIndex)
			{
				VarArrayLoopGet(FontGlyph, glyph, &fontAtlas->glyphs, gIndex);
				#if DEBUG_BUILD
				const char* codepointName = DebugGetCodepointName(glyph->codepoint);
				#else
				const char* codepointName = "-";
				#endif
				PrintLine_D("\t\tGlyph[%llu]: \'%s\' 0x%08X sourceRec=(%d, %d, %d, %d) offset=(%g, %g) advanceX=%g logical=(%g, %g, %g, %g)",
					gIndex,
					codepointName, glyph->codepoint,
					glyph->atlasSourcePos.X, glyph->atlasSourcePos.Y,
					glyph->metrics.glyphSize.Width, glyph->metrics.glyphSize.Height,
					glyph->metrics.renderOffset.X, glyph->metrics.renderOffset.Y,
					glyph->metrics.advanceX,
					glyph->metrics.logicalRec.X, glyph->metrics.logicalRec.Y, glyph->metrics.logicalRec.Width, glyph->metrics.logicalRec.Height
				);
				//TODO: ttfGlyphIndex
			}
			if (fontAtlas->texture.error != Result_Success) { PrintLine_E("\tTexture Error: %s", GetResultStr(fontAtlas->texture.error)); }
			PrintLine_D("\tlineHeight: %f", fontAtlas->metrics.lineHeight);
			PrintLine_D("\tmaxAscend: %f", fontAtlas->metrics.maxAscend);
			PrintLine_D("\tmaxDescend: %f", fontAtlas->metrics.maxDescend);
			PrintLine_D("\tcenterOffset: %f", fontAtlas->metrics.centerOffset);
			if (fontAtlas->isActive)
			{
				PrintLine_D("\tLast Used: %llu (%llums ago)", fontAtlas->lastUsedTime, TimeSinceBy(programTime, fontAtlas->lastUsedTime));
				PrintLine_D("\tCell Size: %dx%d", fontAtlas->activeCellSize.Width, fontAtlas->activeCellSize.Height);
				PrintLine_D("\tGrid Size: %dx%d", fontAtlas->activeCellGridSize.Width, fontAtlas->activeCellGridSize.Height);
				for (i32 yOffset = 0; yOffset < fontAtlas->activeCellGridSize.Height; yOffset++)
				{
					for (i32 xOffset = 0; xOffset < fontAtlas->activeCellGridSize.Width; xOffset++)
					{
						FontActiveCell* cell = &fontAtlas->cells[INDEX_FROM_COORD2D(xOffset, yOffset, fontAtlas->activeCellGridSize.Width, fontAtlas->activeCellGridSize.Height)];
						if (cell->codepoint != FONT_CODEPOINT_EMPTY)
						{
							#if DEBUG_BUILD
							const char* codepointName = DebugGetCodepointName(cell->codepoint);
							#else
							const char* codepointName = "-";
							#endif
							PrintLine_D("\t\tCell[%d,%d]: \'%s\' 0x%08X glyph[%llu]", xOffset, yOffset, codepointName, cell->codepoint, cell->glyphIndex);
						}
					}
				}
				PrintLine_D("\tpushedTextureUpdates: %s", fontAtlas->pushedTextureUpdates ? "True" : "False");
				PrintLine_D("\t%llu pendingTextureUpdate%s:", fontAtlas->pendingTextureUpdates.length, Plural(fontAtlas->pendingTextureUpdates.length, "s"));
				VarArrayLoop(&fontAtlas->pendingTextureUpdates, uIndex)
				{
					VarArrayLoopGet(FontActiveAtlasTextureUpdate, update, &fontAtlas->pendingTextureUpdates, uIndex);
					PrintLine_D("\t\tUpdate[%llu]: (%d, %d, %d, %d)", uIndex, update->sourcePos.X, update->sourcePos.Y, update->imageData.size.Width, update->imageData.size.Height);
				}
			}
		}
	}
	#endif //!TARGET_IS_OSX
	
	#if BUILD_WITH_BOX2D
	if (IsMouseBtnPressed(&mouse, nullptr, MouseBtn_Left))
	{
		r32 physMouseX, physMouseY;
		GetPhysPosFromRenderPos((i32)mouse.position.X, (i32)mouse.position.Y, &physMouseX, &physMouseY);
		SpawnBox(physMouseX, physMouseY, GetRandR32Range(mainRandom, 0.3f, 1.0f), GetRandR32Range(mainRandom, 0.3f, 1.0f));
	}
	UpdateBox2DTest();
	#endif
	
	#if BUILD_WITH_PHYSX
	UpdatePhysicsWorld(physWorld, 16.6f);
	if (IsKeyboardKeyDown(&keyboard, nullptr, Key_R)) { CreatePhysicsTest(physWorld); }
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
	
	r32 textScale = TEXT_SCALE/sapp_dpi_scale(); UNUSED(textScale);
	OsTime afterUpdateTime = OsGetTime();
	BeginFrame(GetSokolGfxSwapchain(), windowSizei, MonokaiDarkGray, 1.0f);
	OsTime beforeRenderTime = OsGetTime();
	{
		TracyCZoneN(Zone_Draw, "Draw", true);
		
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
			DrawBox(MakeBox(3, 0.5f, 0, 1, 1, 1), MonokaiPurple);
			DrawSphere(MakeSphere(2.5f, 0, 0.8f, 1.0f), MonokaiGreen);
			
			#if BUILD_WITH_PHYSX
			VarArrayLoop(&physWorld->bodies, bIndex)
			{
				VarArrayLoopGet(PhysicsBody, body, &physWorld->bodies, bIndex);
				PhysicsBodyTransform transform = GetPhysicsBodyTransform(body);
				v3 position = MakeV3(transform.position.X, transform.position.Y, transform.position.Z);
				quat rotation = NewQuat(transform.rotation.X, transform.rotation.Y, transform.rotation.Z, transform.rotation.W);
				if (body->index == physWorld->groundPlaneBodyIndex)
				{
					//TODO: Figure out how PhysX want's us to intepret rotation/position on a Plane when drawing it
					DrawObb3(NewObb3V(position, MakeV3(100.0f, 0.0001f, 100.0f), Quat_Identity), PalGreenDarker);
				}
				else
				{
					DrawObb3(NewObb3V(position, MakeV3(1.0f, 1.0f, 1.0f), rotation), GetPredefPalColorByIndex(bIndex));
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
			
			#if 0
			Texture* mipTextureToUse = (IsKeyboardKeyDown(&keyboard, nullptr, Key_Shift) ? &noMipmapTexture : &mipmapTexture);
			rec mipmapTextureRec = MakeRec(windowSize.Width/2, windowSize.Height/2, 0, 0);
			mipmapTextureRec.Width = mouse.position.X - mipmapTextureRec.X;
			mipmapTextureRec.Height = mouse.position.Y - mipmapTextureRec.Y;
			DrawTexturedRectangle(mipmapTextureRec, White, mipTextureToUse);
			#endif
			
			#if 1
			{
				r32 fontLineHeight = GetFontLineHeight(&testFont, 18*textScale, FontStyleFlag_None);
				r32 fontMaxAscend = GetFontMaxAscend(&testFont, 18*textScale, FontStyleFlag_None);
				v2 textPos = MakeV2(screenSafeMargins.X + 10, screenSafeMargins.Y + 410 + fontMaxAscend);
				Str8 infoStr = PrintInArenaStr(scratch, "HighDpi: %s Scale: x%g WindowSize: %gx%g", sapp_high_dpi() ? "true" : "false", sapp_dpi_scale(), windowSize.Width, windowSize.Height);
				BindFont(&debugFont);
				DrawText(infoStr, textPos, MonokaiWhite);
				textPos.Y += fontLineHeight;
				
				r32 wrapWidth = MaxR32(wrapPos.X - textPos.X, 0.0f);
				if (wrapWidth == 0.0f) { wrapWidth = windowSize.Width - textPos.X; }
				char kanjiUtf8Buffer[16];
				uxx kanjiBufferIndex = 0;
				u32 kanjiCodepoints[] = { 0x4E09, 0x5CF6, 0x5E83, 0x5C0F, 0x8DEF };
				for (uxx codepointIndex = 0; codepointIndex < ArrayCount(kanjiCodepoints); codepointIndex++) { kanjiBufferIndex += GetUtf8BytesForCode(kanjiCodepoints[codepointIndex], (u8*)&kanjiUtf8Buffer[kanjiBufferIndex], false); }
				kanjiUtf8Buffer[kanjiBufferIndex] = '\0';
				
				static int displayStrIndex = 0;
				static uxx typeAnimCodepointIndex = 0;
				Str8 displayStrs[] = {
					StrLit("Lor[highlight]em\b \bipsum dolor sit " UNICODE_CHECK_MARK_STR " amet, [color=FF0000][size=8]consectetur [size=10]\badipiscing\b [size=12]elit, [size=14]sed [size=16]do [size][color]eiusmod tempor incididunt ut labore et dolore magna aliqua.[highlight] Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum"),
					StrLit("This is a test of font sizes! [size=8]This is a test of font sizes! [size=10]This is a test of font sizes! [size=12]This is a test of font sizes! [size=14]This is a test of font sizes! [size=16]This is a test of font sizes! [size=18]This is a test of font sizes!"),
					StrLit("This is a test of \bbold text\b, and \aitalic text\a and \b\aBOLD ITALIC TEXT\b\a!"),
					StrLit("This is 😂 \bBräcke 😂 € (\xE2\x97\x8F'\xE2\x97\xA1'\xE2\x97\x8F)\b!"), //\xE2\x97\xA1
					StrLit("ABC[size=10]DEF[size]GHI ABCDEFGHI"),
					StrLit("\xE3\x81\x82\xE3\x82\x8A\xE3\x81\x8C\xE3\x81\xA8\xE3\x81\x86\xE3\x81\x94\xE3\x81\x96\xE3\x81\x84\xE3\x81\xBE\xE3\x81\x97\xE3\x81\x9F"),
					MakeStr8Nt(kanjiUtf8Buffer),
					StrLit("\xE4\xB8\x89\xE5\xB3\xB6\xE5\xBA\x83\xE5\xB0\x8F\xE8\xB7\xAF\x20\x2D\x20\xE4\xBC\x8A\xE8\xB1\x86\xE4\xBB\x81\xE7\x94\xB0\x20\x2D\x20\xE7\x94\xB0\xE4\xBA\xAC\x20\x2D\x20\xE5\xA4\xA7\xE5\xA0\xB4\x20\x2D\x20\xE5\x8E\x9F\xE6\x9C\xA8\x20\x2D\x20\xE4\xB8\x89\xE5\xB3\xB6\xE4\xBA\x8C\xE6\x97\xA5\xE7\x94\xBA\x20\x2D\x20\xE9\x9F\xAE\xE5\xB1\xB1\x20\x2D\x20\xE4\xB8\x89\xE5\xB3\xB6\x20\x2D\x20\xE4\xBC\x8A\xE8\xB1\x86\xE5\xA4\x9A\xE8\xB3\x80\x20\x2D\x20\xE5\xAE\x87\xE4\xBD\x90\xE7\xBE\x8E\x20\x2D\x20\xE7\xB6\xB2\xE4\xBB\xA3\x20\x2D\x20\xE5\xBD\xAB\xE5\x88\xBB\xE3\x81\xAE\xE6\xA3\xAE\x20\x2D\x20\xE5\xA1\x94\xE3\x83\x8E\xE6\xB2\xA2\x20\x2D\x20\xE5\x85\xA5\xE7\x94\x9F\xE7\x94\xB0\x20\x2D\x20\xE9\xA2\xA8\xE7\xA5\xAD\x20\x2D\x20\xE5\xB0\x8F\xE6\xB6\x8C\xE8\xB0\xB7\x20\x2D\x20\xE4\xBB\x99\xE4\xBA\xBA\xE5\x8F\xB0\xE4\xBF\xA1\xE5\x8F\xB7\xE5\xA0\xB4\x20\x2D\x20\xE5\xA4\xA7\xE5\xB2\xA1\x20\x2D\x20\xE8\xA3\xBE\xE9\x87\x8E\x20\x2D\x20\xE9\x95\xB7\xE6\xB3\x89\xE3\x81\xAA\xE3\x82\x81\xE3\x82\x8A\x20\x2D\x20\xE4\xB8\x8B\xE5\x9C\x9F\xE7\x8B\xA9\x20\x2D\x20\xE7\x89\x87\xE6\xB5\x9C\x20\x2D\x20\xE5\x8E\x9F\x20\x2D\x20\xE6\x9D\xB1\xE7\x94\xB0\xE5\xAD\x90\xE3\x81\xAE\xE6\xB5\xA6\x20\x2D\x20\xE6\xA0\xB9\xE5\xBA\x9C\xE5\xB7\x9D\x20\x2D\x20\xE6\xB9\xAF\xE6\xB2\xB3\xE5\x8E\x9F\x20\x2D\x20\xE5\x87\xBA\xE5\xB1\xB1\xE4\xBF\xA1\xE5\x8F\xB7\xE5\xA0\xB4\x20\x2D\x20\xE7\x86\xB1\xE6\xB5\xB7\x20\x2D\x20\xE7\x9C\x9F\xE9\xB6\xB4\x20\x2D\x20\xE4\xBC\x8A\xE8\xB1\x86\xE9\x95\xB7\xE5\xB2\xA1\x20\x2D\x20\xE5\xA4\xA7\xE5\xB9\xB3\xE5\x8F\xB0"),
					// StrLit("\xF0\x9F\x98\x80\xF0\x9F\x98\x81\xF0\x9F\x98\x82\xF0\x9F\xA4\xA3\xF0\x9F\x98\x83\xF0\x9F\x98\xAB\xF0\x9F\x90\xB1\xE2\x8C\xA8"),
					StrLit("😊[color=FF0000]😂[color]🤣😒😁[size=64]😍🙌"),
					StrLit("Non" UNICODE_NON_BREAKING_HYPHEN_STR "breaking" UNICODE_NON_BREAKING_SPACE_STR "string Another" UNICODE_NON_BREAKING_SPACE_STR "non" UNICODE_NON_BREAKING_HYPHEN_STR "breaking" UNICODE_NON_BREAKING_SPACE_STR "string String" UNICODE_ZERO_WIDTH_SPACE_STR "With" UNICODE_ZERO_WIDTH_SPACE_STR "Zero" UNICODE_ZERO_WIDTH_SPACE_STR "Width" UNICODE_ZERO_WIDTH_SPACE_STR "Spaces"),
					StrLit("This is [highlight]a string\nwith new-line    \ncharacters[highlight] in it!\r\nHello!"),
				};
				if (IsKeyboardKeyPressed(&keyboard, nullptr, Key_Plus, true)) { displayStrIndex = ((displayStrIndex+1) % ArrayCount(displayStrs)); typeAnimCodepointIndex = 0; }
				IncrementUXX(typeAnimCodepointIndex);
				Str8 displayStr = displayStrs[displayStrIndex];
				RichStr displayStrRich = DecodeStrToRichStr(scratch, displayStr);
				uxx typedByteIndex = displayStrRich.fullPiece.str.length;
				{
					uxx codepointIndex = 0;
					for (uxx byteIndex = 0; byteIndex < displayStrRich.fullPiece.str.length; byteIndex++)
					{
						u8 codepointSize = GetCodepointForUtf8Str(displayStrRich.fullPiece.str, byteIndex, nullptr);
						if (codepointSize == 0) { codepointSize = 1; }
						if (codepointIndex >= typeAnimCodepointIndex) { typedByteIndex = byteIndex; break; }
						byteIndex += (codepointSize-1);
						codepointIndex++;
					}
					if (typedByteIndex < displayStrRich.fullPiece.str.length)
					{
						displayStrRich = RichStrSlice(scratch, displayStrRich, 0, typedByteIndex);
					}
				}
				BindFont(&testFont);
				if (IsKeyboardKeyDown(&keyboard, nullptr, Key_Shift))
				{
					DrawRichTextWithFont(
						&testFont, 18*textScale, FontStyleFlag_ColoredGlyphs,
						displayStrRich,
						textPos,
						ColorWithAlpha(MonokaiWhite, 0.75f)
					);
				}
				else
				{
					DrawWrappedRichTextWithFont(
						&testFont, 18*textScale, FontStyleFlag_ColoredGlyphs,
						displayStrRich,
						textPos,
						wrapWidth,
						ColorWithAlpha(MonokaiWhite, 0.75f)
					);
				}
				// rec logicalRec = gfx.prevFontFlow.logicalRec;
				// rec visualRec = gfx.prevFontFlow.visualRec;
				// DrawRectangleOutlineEx(logicalRec, 1, MonokaiYellow, false);
				// DrawRectangleOutlineEx(visualRec, 1, MonokaiBlue, false);
				DrawRectangle(MakeRec(textPos.X + wrapWidth, 0, 1, windowSize.Height), MonokaiRed);
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
					DrawCircle(MakeCircleV(touch->startPos, touch->visitRadius), ColorWithAlpha(isMainTouch ? MonokaiYellow : MonokaiOrange, 0.25f));
					DrawRectangle(touch->visitBounds, ColorWithAlpha(MonokaiGreen, 0.25f));
					DrawRectangle(NewRecCentered(touch->startPos.X, touch->startPos.Y, 15, 15), MonokaiBlue);
					DrawRectangle(NewRecCentered(touch->pos.X, touch->pos.Y, 15, 15), MonokaiMagenta);
				}
			}
			#endif
			
			#if 0
			v2 tileSize = ToV2Fromi(gradientTexture.size); //MakeV2(48, 27);
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
			
			#if 1
			r32 atlasRenderPosX = 10.0f;
			#if BUILD_WITH_CLAY
			rec topbarRec = GetClayElementDrawRec(CLAY_ID("Topbar"));
			r32 atlasRenderPosY = topbarRec.Y + topbarRec.Height + 10;
			#else
			r32 atlasRenderPosY = 10.0f;
			#endif
			VarArrayLoop(&testFont.atlases, aIndex)
			{
				VarArrayLoopGet(FontAtlas, fontAtlas, &testFont.atlases, aIndex);
				rec atlasRenderRec = MakeRec(atlasRenderPosX, atlasRenderPosY, (r32)fontAtlas->texture.Width, (r32)fontAtlas->texture.Height);
				if (fontAtlas->isActive)
				{
					for (i32 cellY = 0; cellY < fontAtlas->activeCellGridSize.Height; cellY++)
					{
						for (i32 cellX = 0; cellX < fontAtlas->activeCellGridSize.Width; cellX++)
						{
							rec cellRec = MakeRec(
								atlasRenderRec.X + (r32)(cellX * fontAtlas->activeCellSize.Width),
								atlasRenderRec.Y + (r32)(cellY * fontAtlas->activeCellSize.Height),
								(r32)fontAtlas->activeCellSize.Width,
								(r32)fontAtlas->activeCellSize.Height
							);
							DrawRectangle(cellRec, ColorWithAlpha(MonokaiWhite, ((cellX + cellY)%2 == 0) ? 0.1f : 0.0f));
						}
					}
				}
				DrawTexturedRectangle(atlasRenderRec, White, &fontAtlas->texture);
				DrawRectangleOutline(atlasRenderRec, 1, White);
				BindFont(&debugFont);
				v2 infoTextPos = MakeV2(atlasRenderRec.X, atlasRenderRec.Y + atlasRenderRec.Height + 5 + GetMaxAscend());
				Str8 infoStr = PrintInArenaStr(scratch, "%g %dx%d%s", fontAtlas->fontSize, fontAtlas->texture.Width, fontAtlas->texture.Height, fontAtlas->isActive ? "" : " (Static)");
				DrawText(infoStr, infoTextPos, MonokaiWhite); infoTextPos.Y += GetLineHeight();
				bool isBold = IsFlagSet(fontAtlas->styleFlags, FontStyleFlag_Bold);
				bool isItalic = IsFlagSet(fontAtlas->styleFlags, FontStyleFlag_Italic);
				infoStr = PrintInArenaStr(scratch, "%s%s%s%s", (!isBold && !isItalic) ? "Default" : "", isBold ? "Bold" : "", (isBold && isItalic) ? "|" : "", isItalic ? "Italic" : "");
				DrawText(infoStr, infoTextPos, MonokaiWhite); infoTextPos.Y += GetLineHeight();
				infoStr = PrintInArenaStr(scratch, "%llu glyph%s", fontAtlas->glyphs.length, Plural(fontAtlas->glyphs.length, "s"));
				DrawText(infoStr, infoTextPos, MonokaiWhite); infoTextPos.Y += GetLineHeight();
				infoStr = PrintInArenaStr(scratch, "%llu range%s", fontAtlas->charRanges.length, Plural(fontAtlas->charRanges.length, "s"));
				DrawText(infoStr, infoTextPos, MonokaiWhite); infoTextPos.Y += GetLineHeight();
				atlasRenderPosX += atlasRenderRec.Width + 10;
				VarArrayLoop(&fontAtlas->glyphs, gIndex)
				{
					VarArrayLoopGet(FontGlyph, glyph, &fontAtlas->glyphs, gIndex);
					rec glyphRec = MakeRec(
						atlasRenderRec.X + atlasRenderRec.Width * ((r32)glyph->atlasSourcePos.X / fontAtlas->texture.Width),
						atlasRenderRec.Y + atlasRenderRec.Height * ((r32)glyph->atlasSourcePos.Y / fontAtlas->texture.Height),
						atlasRenderRec.Width * ((r32)glyph->metrics.glyphSize.Width / fontAtlas->texture.Width),
						atlasRenderRec.Height * ((r32)glyph->metrics.glyphSize.Height / fontAtlas->texture.Height)
					);
					bool isMouseHovered = IsInsideRec(glyphRec, mouse.position);
					DrawRectangleOutline(glyphRec, 1, isMouseHovered ? MonokaiLightPurple : MonokaiPurple);
					if (isMouseHovered)
					{
						#if DEBUG_BUILD
						const char* codepointName = DebugGetCodepointName(glyph->codepoint);
						#else
						const char* codepointName = "-";
						#endif
						infoStr = PrintInArenaStr(scratch, "Glyph[%llu] \'%s\' 0x%08X %dx%d", gIndex, codepointName, glyph->codepoint, glyph->metrics.glyphSize.Width, glyph->metrics.glyphSize.Height);
						DrawText(infoStr, infoTextPos, MonokaiWhite); infoTextPos.Y += GetLineHeight();
					}
				}
			}
			#endif
			
			#if 0
			BindFont(&debugFont);
			Str8 loremIpsum = StrLit("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum");
			uxx numLines = (uxx)CeilR32i(mouse.position.Y / GetLineHeight());
			Str8 lines;
			lines.length = (loremIpsum.length+1) * numLines;
			lines.chars = AllocArray(char, scratch, lines.length);
			NotNull(lines.chars);
			for (uxx lIndex = 0; lIndex < numLines; lIndex++)
			{
				char* linePntr = &lines.chars[lIndex * (loremIpsum.length+1)];
				MyMemCopy(linePntr, loremIpsum.chars, loremIpsum.length);
				linePntr[loremIpsum.length] = (lIndex < numLines-1) ? '\n' : '!';
			}
			v2 textPos = MakeV2(5, 0 + GetMaxAscend());
			DrawText(lines, textPos, MonokaiWhite);
			#endif
			
			#if BUILD_WITH_BOX2D
			RenderBox2DTest();
			#endif
			
			// +==============================+
			// |         Clay UI Test         |
			// +==============================+
			#if BUILD_WITH_CLAY
			UpdateClayScrolling(&clay.clay, 16.6f, false, mouse.scrollDelta, TARGET_IS_ANDROID);
			v2 uiMousePosition = (TARGET_IS_ANDROID ? touchscreen.mainTouch->pos : mouse.position);
			bool uiMouseDown = (TARGET_IS_ANDROID ? (touchscreen.mainTouch->id != TOUCH_ID_INVALID && !touchscreen.mainTouch->stopped) : IsMouseBtnDown(&mouse, nullptr, MouseBtn_Left));
			BeginClayUIRender(&clay.clay, windowSize, false, uiMousePosition, uiMouseDown);
			{
				CLAY({ .id = CLAY_ID("FullscreenContainer"),
					.layout = {
						.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
						.padding = { .left = (u16)screenMargins.X, .top = (u16)screenMargins.Y, .right = (u16)screenMargins.Z, .bottom = (u16)screenMargins.W },
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
						r32 lineHeight = GetFontLineHeight(&testFont, 18*textScale, FontStyleFlag_None);
						CLAY({ .id = CLAY_ID("Topbar"),
							.layout = {
								.sizing = {
									.height = CLAY_SIZING_FIXED(lineHeight + 30),
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
								
								if (ClayBtn("Open...", Transparent, MonokaiWhite))
								{
									FilePath chosenPath = FilePath_Empty;
									Result dialogResult = OsDoOpenFileDialogBlocking(scratch, &chosenPath);
									if (dialogResult == Result_Success)
									{
										PrintLine_I("Chose file: \"%.*s\"", StrPrint(chosenPath));
									}
									else
									{
										PrintLine_E("Dialog error: %s", GetResultStr(dialogResult));
									}
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
									.fontSize = (u16)(18*textScale),
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
			
			// +==============================+
			// |      Pig UI System Test      |
			// +==============================+
			#if BUILD_WITH_PIG_UI
			if (mouse.scrollDelta.Y != 0.0f)
			{
				uiScale *= 1.0f + (0.1f * (mouse.scrollDelta.Y > 0 ? 1.0f : -1.0f));
				uiScale = RoundR32(uiScale * 100.0f) / 100.0f;
				uiScale = ClampR32(uiScale, 0.1f, 10.0f);
			}
			
			StartUiFrame(&uiContext, windowSize, MonokaiLightGray, uiScale, programTime, &keyboard, &mouse, &touchscreen);
			
			// PushUiFields({ .color = MonokaiDarkGray });
			PushUiFieldsText({ .textColor = MonokaiDarkGray });
			// PushUiFields({ .padding = {
			// 	.outer = FillV4r(15.0f), //FillV4r(OscillateBy(programTime, 0.0f, 15.0f, 4000, 0)),
			// 	.child = 15.0f, //OscillateBy(programTime, 0.0f, 15.0f, 4000, 0),
			// }});
			PushUiFields({ .borderThickness = FillV4r(2.0f), .padding = {.inner = FillV4r(2.0f) }, .borderColor = ColorWithAlpha(White, 0.75f) });
			PushUiThemer(&uiContext.themers, TestsGlobalUiThemerCallback, nullptr);
			#define SIMPLETEXTELEM(strLit, isMousePassthrough) UIELEM_LEAF({ .sizing = UI_TEXT_FULL(), .padding = { .outer = FillV4r(4) }, .text = StrLit(strLit), .font = &testFont, .textColor = MonokaiWhite, .mousePassthrough=(isMousePassthrough)});
			
			UiElemConfig rootElem = { .id = UiIdLit("Root") };
			rootElem.direction = IsKeyboardKeyDown(&keyboard, nullptr, Key_Shift) ? UiLayoutDir_BottomUp : UiLayoutDir_TopDown;
			UIELEM(rootElem)
			{
				UiElemConfig orangeElem = { .id = UiIdLit("Orange"), .color=MonokaiOrange };
				orangeElem.direction   = UiLayoutDir_LeftToRight;
				UIELEM(orangeElem) { SIMPLETEXTELEM("Orange", false); }
				
				UiElemConfig percentageRowElem = { .id = UiIdLit("PercentageRow") };
				percentageRowElem.direction = UiLayoutDir_RightToLeft;
				percentageRowElem.borderThickness = FillV4r(IsUiElementBeingClicked(percentageRowElem.id, MouseBtn_Left) ? 30.0f : 0.0f);
				percentageRowElem.borderColor = MonokaiPurple;
				percentageRowElem.borderDepth = UI_DEPTH_ZERO;
				percentageRowElem.condition = UiConditionType_MouseHover;
				UIELEM(percentageRowElem)
				{
					uxx testThemerId = PushUiThemer(&uiContext.themers, TestsUiThemerCallback, nullptr);
					
					UiElemConfig greenElem = { .id = UiIdLit("Green"), .color=MonokaiGreen };
					greenElem.sizing.x = NEW_STRUCT(UiSizingAxis)UI_PERCENT(0.20f);
					greenElem.alignment.x = UiAlignmentType_Right;
					greenElem.alignment.y = UiAlignmentType_Bottom;
					UIELEM(greenElem) { SIMPLETEXTELEM("Green", false); }
					
					UiElemConfig blueElem = { .id = UiIdLit("Blue"), .color=MonokaiBlue };
					blueElem.sizing.x = NEW_STRUCT(UiSizingAxis)UI_PERCENT(0.10f);
					blueElem.condition = UiConditionType_MouseLeftClicked;
					if (!IsKeyboardKeyDown(&keyboard, nullptr, Key_Control)) { blueElem.depth = -1; }
					UIELEM(blueElem)
					{
						WriteLine_D("You click on the blue element!");
					}
					
					UiElemConfig purpleElem = { .id = UiIdLit("Purple"), .color=MonokaiPurple };
					purpleElem.sizing.x = NEW_STRUCT(UiSizingAxis)UI_PERCENT(0.60f);
					UIELEM(purpleElem) { SIMPLETEXTELEM("Purple", false); }
					
					PopUiThemer(&uiContext.themers, testThemerId);
				}
				
				UIELEM({ .id = UiIdLit("Yellow"),
					.direction = UiLayoutDir_LeftToRight,
					.color=MonokaiYellow,
					.clipChildren = true,
				})
				{
					
				}
				
				UIELEM({ .id = UiIdLit("Red"),
					.sizing = { .x=UI_FIT(), .y=UI_EXPAND() },
					.padding = {
						// .inner = { .Right = 15 },
						.child = 15,
					},
					.direction = UiLayoutDir_LeftToRight,
					.color=MonokaiRed,
				})
				{
					UIELEM({ .id = UiIdLit("DarkGreen"),
						.color=MonokaiDarkGreen,
						.sizing=UI_FIXED2(100, 200),
						.alignment = { .y = UiAlignmentType_Top },
					})
					{
						SIMPLETEXTELEM("Green", false);
					}
					
					UIELEM_LEAF({ .id = UiIdLit("LoremIpsum"),
						.sizing = UI_TEXT_WRAP(30.0f),
						.padding = { .outer = FillV4r(4) },
						.richText = DecodeStrToRichStr(UiCtx->frameArena, StrLit("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed [alpha=0.5]do eiusmod tempor incididunt[alpha] ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum")),
						.font = &testFont,
						// .textColor = MonokaiWhite,
					});
					
					for (uxx tIndex = 0; tIndex < 4; tIndex++)
					{
						Texture* texture = ((tIndex%2) == 0) ? &mipmapTexture : &noMipmapTexture;
						UIELEM_LEAF({ .id = UiIdLitIndex("Texture", tIndex),
							.sizing = UI_FIXED2(texture->Width*0.3f, texture->Height*0.3f),
							.color=ColorLerpSimple(GetPredefPalColorByIndex(tIndex), White, 0.5f),
							.texture = texture,
						});
					}
				}
				
				if (UiCtx->mouseHoveredId.id != 0 && GetUiElementById(UiCtx->mouseHoveredId, false) != nullptr)
				{
					UIELEM({ .id = UiIdLit("FloatingMenu"),
						.sizing = UI_FIT2(),
						// .sizing = UI_PERCENT2(0.8f, 0.5f),
						// .sizing = UI_EXPAND2(),
						.direction = UiLayoutDir_TopDown,
						.padding = { .child = 5, .inner = FillV4r(10) },
						.color=MonokaiDarkGray,
						.depth = -1.0f,
						.mousePassthrough = true,
						.floating = {
							// .type = UiFloatingType_Parent,
							.type = UiFloatingType_Id,
							.attachId = UiCtx->mouseHoveredId,
							.offset = MakeV2(0, -2 * uiScale), //SubV2(mouse.position, ScaleV2(windowSize, 0.25f)), //MakeV2(15, 45),
							.parentSide = UiSide_TopCenter,
							.elemSide = UiSide_BottomCenter,
						},
						.clipChildren = true,
					})
					{
						UIELEM({.sizing={.x=UI_FIXED(100),.y=UI_FIT()}, .padding={.inner=FillV4r(8)}, .color = MonokaiRed,    .mousePassthrough = true }) { SIMPLETEXTELEM("Red", true); }
						UIELEM({.sizing={.x=UI_FIXED(100),.y=UI_FIT()}, .padding={.inner=FillV4r(8)}, .color = MonokaiPurple, .mousePassthrough = true }) { SIMPLETEXTELEM("Purple", true); }
						UIELEM({.sizing={.x=UI_FIXED(100),.y=UI_FIT()}, .padding={.inner=FillV4r(8)}, .color = MonokaiOrange, .mousePassthrough = true }) { SIMPLETEXTELEM("OrangeOrangeOrangeOrange", true); }
					}
				}
			}
			
			UiRenderList* uiRenderList = GetUiRenderList();
			//TODO: Send this off to some Ui Renderer implementation
			VarArrayLoop(&uiRenderList->commands, cIndex)
			{
				VarArrayLoopGet(UiRenderCmd, cmd, &uiRenderList->commands, cIndex);
				SetClipRec(ToReciFromf(cmd->clipRec));
				switch (cmd->type)
				{
					case UiRenderCmdType_Rectangle:
					{
						if (cmd->rectangle.texture != nullptr)
						{
							DrawTexturedRectangle(cmd->rectangle.rectangle, cmd->color, cmd->rectangle.texture);
						}
						else
						{
							DrawRectangle(cmd->rectangle.rectangle, cmd->color);
						}
						if (cmd->rectangle.borderThickness.X > 0.0f)
						{
							DrawRectangleOutlineSidesEx(
								cmd->rectangle.rectangle,
								cmd->rectangle.borderThickness.Left, cmd->rectangle.borderThickness.Right, cmd->rectangle.borderThickness.Top, cmd->rectangle.borderThickness.Bottom,
								cmd->rectangle.borderColor,
								false
							);
						}
					} break;
					
					case UiRenderCmdType_Text:
					{
						RichStr richStr = ToRichStr(cmd->text.text);
						DrawWrappedRichTextWithFont(cmd->text.font, cmd->text.fontSize, cmd->text.fontStyle, richStr, cmd->text.position, cmd->text.wrapWidth, cmd->color);
					} break;
					
					case UiRenderCmdType_RichText:
					{
						DrawWrappedRichTextWithFont(cmd->richText.font, cmd->richText.fontSize, cmd->richText.fontStyle, cmd->richText.text, cmd->richText.position, cmd->richText.wrapWidth, cmd->color);
					} break;
				}
			}
			DisableClipRec();
			
			EndUiFrame();
			
			BindFont(&debugFont);
			DrawText(PrintInArenaStr(scratch, "%.2fx", uiScale), MakeV2(10, 30), Black);
			#endif //BUILD_WITH_PIG_UI
			
			// +==============================+
			// |      Dear Imgui UI Test      |
			// +==============================+
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
				if (IsKeyboardKeyPressed(&keyboard, nullptr, Key_R, false) && tokenizer.arena != nullptr)
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
			
			// rec testTextureRec = MakeRec(windowSize.Width - (r32)testTexture.Width, windowSize.Height - (r32)testTexture.Height, (r32)testTexture.Width, (r32)testTexture.Height);
			// DrawTexturedRectangle(testTextureRec, White, &testTexture);
			
			// +==============================+
			// |       Render Overlays        |
			// +==============================+
			if (showPerfGraph)
			{
				RenderPerfGraph(&perfGraph, &gfx, &debugFont, GetDefaultFontSize(&debugFont), GetDefaultFontStyleFlags(&debugFont), MakeRec(10, 10, 400, 100));
			}
		}
		
		TracyCZoneEnd(Zone_Draw);
	}
	#if !TARGET_IS_OSX //TODO: Remove me once we get fonts working on OSX
	CommitAllFontTextureUpdates(&testFont);
	#endif
	OsTime afterRenderTime = OsGetTime();
	TracyCZoneN(Zone_EndFrame, "EndFrame", true);
	EndFrame();
	TracyCZoneEnd(Zone_EndFrame);
	
	// PrintLine_D("numPipelineChanges: %llu", gfx.numPipelineChanges);
	// PrintLine_D("numBindingChanges: %llu", gfx.numBindingChanges);
	// PrintLine_D("numDrawCalls: %llu", gfx.numDrawCalls);
	gfx.numPipelineChanges = 0;
	gfx.numBindingChanges = 0;
	gfx.numDrawCalls = 0;
	RefreshMouseState(&mouse, sapp_mouse_locked(), MakeV2(sapp_widthf()/2.0f, sapp_heightf()/2.0f));
	RefreshKeyboardState(&keyboard);
	RefreshTouchscreenState(&touchscreen);
	#if TARGET_IS_ANDROID
	screenRotated = false;
	#endif
	frameIndex++;
	prevUpdateMs =
		OsTimeDiffMsR32(beforeUpdateTime, afterUpdateTime) +
		OsTimeDiffMsR32(beforeRenderTime, afterRenderTime);
	ScratchEnd(scratch);
	return frameRendered;
}

// +--------------------------------------------------------------+
// |                            Event                             |
// +--------------------------------------------------------------+
const char* Get_SAPP_EVENTTYPE_Str(sapp_event_type eventType)
{
	switch (eventType)
	{
		case SAPP_EVENTTYPE_INVALID:           return "INVALID";
		case SAPP_EVENTTYPE_KEY_DOWN:          return "KEY_DOWN";
		case SAPP_EVENTTYPE_KEY_UP:            return "KEY_UP";
		case SAPP_EVENTTYPE_CHAR:              return "CHAR";
		case SAPP_EVENTTYPE_MOUSE_DOWN:        return "MOUSE_DOWN";
		case SAPP_EVENTTYPE_MOUSE_UP:          return "MOUSE_UP";
		case SAPP_EVENTTYPE_MOUSE_SCROLL:      return "MOUSE_SCROLL";
		case SAPP_EVENTTYPE_MOUSE_MOVE:        return "MOUSE_MOVE";
		case SAPP_EVENTTYPE_MOUSE_ENTER:       return "MOUSE_ENTER";
		case SAPP_EVENTTYPE_MOUSE_LEAVE:       return "MOUSE_LEAVE";
		case SAPP_EVENTTYPE_TOUCHES_BEGAN:     return "TOUCHES_BEGAN";
		case SAPP_EVENTTYPE_TOUCHES_MOVED:     return "TOUCHES_MOVED";
		case SAPP_EVENTTYPE_TOUCHES_ENDED:     return "TOUCHES_ENDED";
		case SAPP_EVENTTYPE_TOUCHES_CANCELLED: return "TOUCHES_CANCELLED";
		case SAPP_EVENTTYPE_RESIZED:           return "RESIZED";
		case SAPP_EVENTTYPE_ICONIFIED:         return "ICONIFIED";
		case SAPP_EVENTTYPE_RESTORED:          return "RESTORED";
		case SAPP_EVENTTYPE_FOCUSED:           return "FOCUSED";
		case SAPP_EVENTTYPE_UNFOCUSED:         return "UNFOCUSED";
		case SAPP_EVENTTYPE_SUSPENDED:         return "SUSPENDED";
		case SAPP_EVENTTYPE_RESUMED:           return "RESUMED";
		case SAPP_EVENTTYPE_QUIT_REQUESTED:    return "QUIT_REQUESTED";
		case SAPP_EVENTTYPE_CLIPBOARD_PASTED:  return "CLIPBOARD_PASTED";
		case SAPP_EVENTTYPE_FILES_DROPPED:     return "FILES_DROPPED";
		case SAPP_EVENTTYPE_RESIZE_RENDER:     return "RESIZE_RENDER";
		default: return UNKNOWN_STR;
	}
}
void AppEvent(const sapp_event* event)
{
	TracyCZoneN(Zone_Func, "AppEvent", true);
	bool handledEvent = HandleSokolKeyboardMouseAndTouchEvents(event, programTime, MakeV2i(sapp_width(), sapp_height()), &keyboard, &mouse, &touchscreen, sapp_mouse_locked());
	// if (event->type != SAPP_EVENTTYPE_MOUSE_MOVE)
	// {
	// 	PrintLine_D("SokolEvent: SAPP_EVENTTYPE_%s%s", Get_SAPP_EVENTTYPE_Str(event->type), handledEvent ? " (Handled)" : "");
	// }
	
	if (!handledEvent)
	{
		switch (event->type)
		{
			case SAPP_EVENTTYPE_RESIZED:           PrintLine_D("Size: Window=%dx%d Framebuffer=%dx%d", event->window_width, event->window_height, event->framebuffer_width, event->framebuffer_height); break;
			// case SAPP_EVENTTYPE_ICONIFIED:         break;
			// case SAPP_EVENTTYPE_RESTORED:          break;
			// case SAPP_EVENTTYPE_FOCUSED:           break;
			// case SAPP_EVENTTYPE_UNFOCUSED:         break;
			// case SAPP_EVENTTYPE_SUSPENDED:         break;
			// case SAPP_EVENTTYPE_RESUMED:           break;
			// case SAPP_EVENTTYPE_QUIT_REQUESTED:    break;
			// case SAPP_EVENTTYPE_CLIPBOARD_PASTED:  break;
			// case SAPP_EVENTTYPE_FILES_DROPPED:     break;
			
			//NOTE: We currently only get this event when using OpenGL as the rendering backend since D3D11 has weird problems when we try to resize/render inside the WM_PAINT event
			#if TARGET_IS_WINDOWS
			//NOTE: I added this event type in order to update/render while the app is resized on Windows
			case SAPP_EVENTTYPE_RESIZE_RENDER:
			{
				AppFrame();
				sapp_consume_event(); //This tells Sokol backend that we actually rendered and want a frame flip
			} break;
			#endif //TARGET_IS_WINDOWS
			
			// default: PrintLine_D("Event: UNKNOWN(%d)", event->type); break;
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
	
	EarlyInit();
	
	//NOTE: On some platforms (like Android) this call happens on a separate thread to AppInit, AppFrame, etc. So we shouldn't do any initialization here that is thread specific
	argc_copy = argc;
	argv_copy = argv;
	
	sapp_desc result = {
		.init_cb = AppInit,
		.frame_cb = AppFrame,
		.cleanup_cb = AppCleanup,
		.event_cb = AppEvent,
		.width = 1100,
		.height = 630,
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
