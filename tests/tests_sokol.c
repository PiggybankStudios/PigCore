/*
File:   tests_sokol.c
Author: Taylor Robbins
Date:   01\28\2025
Description: 
	** None
*/

#if BUILD_WITH_SOKOL

#include "tests/simple_shader.glsl.h"
#include "tests/main2d_shader.glsl.h"

int MyMain(int argc, char* argv[]);

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
sg_pass_action sokolPassAction;
GfxSystem gfx;
Shader simpleShader;
Shader main2dShader;
Texture gradientTexture;
VertBuffer squareBuffer;

// +--------------------------------------------------------------+
// |                    Callbacks and Helpers                     |
// +--------------------------------------------------------------+
void SokolLogCallback(const char* tag, uint32_t logLevel, uint32_t logId, const char* message, uint32_t lineNum, const char* filePath, void* userData)
{
	UNUSED(tag); //TODO: Should we output the tag?
	UNUSED(logId); //TODO: Should we output the logId?
	UNUSED(userData);
	DbgLevel dbgLevel;
	switch (logLevel)
	{
		case 0: dbgLevel = DbgLevel_Error; break;
		case 1: dbgLevel = DbgLevel_Error; break;
		case 2: dbgLevel = DbgLevel_Warning; break;
		case 3: dbgLevel = DbgLevel_Other; break;
		default: dbgLevel = DbgLevel_None; break;
	}
	DebugOutputRouter(filePath, lineNum, __func__, dbgLevel, true, message);
	if (logLevel == 0) { Assert(false); }
}

sg_environment CreateSokolEnvironment()
{
	sg_environment result = ZEROED;
	result.defaults.color_format = (sg_pixel_format)sapp_color_format();
	result.defaults.depth_format = (sg_pixel_format)sapp_depth_format();
	result.defaults.sample_count = sapp_sample_count();
	result.metal.device = sapp_metal_get_device();
	result.d3d11.device = sapp_d3d11_get_device();
	result.d3d11.device_context = sapp_d3d11_get_device_context();
	result.wgpu.device = sapp_wgpu_get_device();
	return result;
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void AppInit(void)
{
	ScratchBegin(scratch);
	
	sg_setup(&(sg_desc){
		.environment = CreateSokolEnvironment(),
		.logger.func = SokolLogCallback,
	});
	
	InitGfxSystem(stdHeap, &gfx);
	
	Vertex2D squareVertices[] = {
		{ .X=0.0f, .Y=0.0f,   .tX=0.0f, .tY=0.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		{ .X=1.0f, .Y=0.0f,   .tX=1.0f, .tY=0.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		{ .X=0.0f, .Y=1.0f,   .tX=0.0f, .tY=1.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		
		{ .X=1.0f, .Y=1.0f,   .tX=1.0f, .tY=1.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		{ .X=0.0f, .Y=1.0f,   .tX=0.0f, .tY=1.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		{ .X=1.0f, .Y=0.0f,   .tX=1.0f, .tY=0.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
	};
	squareBuffer = InitVertBuffer2D(stdHeap, StrLit("square"), VertBufferUsage_Static, ArrayCount(squareVertices), &squareVertices[0], false);
	Assert(squareBuffer.error == Result_Success);
	
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
	
	InitCompiledShader(&simpleShader, stdHeap, simple); Assert(simpleShader.error == Result_Success);
	InitCompiledShader(&main2dShader, stdHeap, main2d); Assert(main2dShader.error == Result_Success);
	
	ScratchEnd(scratch);
}

void AppCleanup(void)
{
	sg_shutdown();
}

void DrawRectangle(Shader* shader, v2 topLeft, v2 size, Color32 color)
{
	NotNull(shader);
	
	mat4 worldMat = Mat4_Identity;
	TransformMat4(&worldMat, MakeScaleXYZMat4(size.Width, size.Height, 1.0f));
	TransformMat4(&worldMat, MakeTranslateXYZMat4(topLeft.X, topLeft.Y, 0.0f));
	SetSystemWorldMat(&gfx, worldMat);
	SetSystemTintColor(&gfx, color);
	
	BindSystemVertBuffer(&gfx, &squareBuffer);
	DrawSystemVertices(&gfx);
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void AppFrame(void)
{
	v2 windowSize = NewV2(sapp_widthf(), sapp_heightf());
	// float newGreen = sokolPassAction.colors[0].clear_value.g + 0.01f;
	// sokolPassAction.colors[0].clear_value.g = (newGreen > 1.0f) ? 0.0f : newGreen;
	BeginSystemFrame(&gfx, MonokaiBack, 1.0f);
	BindSystemShader(&gfx, &main2dShader);
	BindSystemTexture(&gfx, &gradientTexture);
	
	mat4 projMat = Mat4_Identity;
	TransformMat4(&projMat, MakeScaleXYZMat4(1.0f/(windowSize.Width/2.0f), 1.0f/(windowSize.Height/2.0f), 1.0f));
	TransformMat4(&projMat, MakeTranslateXYZMat4(-1.0f, -1.0f, 0.0f));
	TransformMat4(&projMat, MakeScaleYMat4(-1.0f));
	SetSystemProjectionMat(&gfx, projMat);
	SetSystemViewMat(&gfx, Mat4_Identity);
	SetSystemWorldMat(&gfx, Mat4_Identity);
	SetSystemSourceRec(&gfx, NewV4(0, 0, (r32)gradientTexture.Width, (r32)gradientTexture.Height));
	// SetSystemUniformByNameV2(&gfx, StrLit("main2d_texture0_size"), ToV2Fromi(gradientTexture.size));
	
	v2 tileSize = ToV2Fromi(gradientTexture.size); //NewV2(48, 27);
	i32 numColumns = CeilR32i(windowSize.Width / tileSize.Width);
	i32 numRows = CeilR32i(windowSize.Height / tileSize.Height);
	u64 colorIndex = 0;
	for (i32 yIndex = 0; yIndex < numRows; yIndex++)
	{
		for (i32 xIndex = 0; xIndex < numColumns; xIndex++)
		{
			DrawRectangle(&main2dShader, NewV2(tileSize.Width * xIndex, tileSize.Height * yIndex), tileSize, White);
			colorIndex++;
		}
	}
	
	EndSystemFrame(&gfx);
	sg_commit();
	
	// PrintLine_D("numPipelineChanges: %llu", gfx.numPipelineChanges);
	// PrintLine_D("numBindingChanges: %llu", gfx.numBindingChanges);
	// PrintLine_D("numDrawCalls: %llu", gfx.numDrawCalls);
	gfx.numPipelineChanges = 0;
	gfx.numBindingChanges = 0;
	gfx.numDrawCalls = 0;
}

// +--------------------------------------------------------------+
// |                            Event                             |
// +--------------------------------------------------------------+
void AppEvent(const sapp_event* event)
{
	switch (event->type)
	{
		case SAPP_EVENTTYPE_KEY_DOWN:          WriteLine_D("Event: KEY_DOWN");          break;
		case SAPP_EVENTTYPE_KEY_UP:            WriteLine_D("Event: KEY_UP");            break;
		case SAPP_EVENTTYPE_CHAR:              WriteLine_D("Event: CHAR");              break;
		case SAPP_EVENTTYPE_MOUSE_DOWN:        WriteLine_D("Event: MOUSE_DOWN");        break;
		case SAPP_EVENTTYPE_MOUSE_UP:          WriteLine_D("Event: MOUSE_UP");          break;
		case SAPP_EVENTTYPE_MOUSE_SCROLL:      WriteLine_D("Event: MOUSE_SCROLL");      break;
		case SAPP_EVENTTYPE_MOUSE_MOVE:        /*WriteLine_D("Event: MOUSE_MOVE");*/    break;
		case SAPP_EVENTTYPE_MOUSE_ENTER:       /*WriteLine_D("Event: MOUSE_ENTER");*/   break;
		case SAPP_EVENTTYPE_MOUSE_LEAVE:       /*WriteLine_D("Event: MOUSE_LEAVE");*/   break;
		case SAPP_EVENTTYPE_TOUCHES_BEGAN:     WriteLine_D("Event: TOUCHES_BEGAN");     break;
		case SAPP_EVENTTYPE_TOUCHES_MOVED:     WriteLine_D("Event: TOUCHES_MOVED");     break;
		case SAPP_EVENTTYPE_TOUCHES_ENDED:     WriteLine_D("Event: TOUCHES_ENDED");     break;
		case SAPP_EVENTTYPE_TOUCHES_CANCELLED: WriteLine_D("Event: TOUCHES_CANCELLED"); break;
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

// +--------------------------------------------------------------+
// |                         Entry Point                          |
// +--------------------------------------------------------------+
sapp_desc sokol_main(int argc, char* argv[])
{
	//NOTE: The App callbacks may happen on a different thread than this one!
	UNUSED(argc);
	UNUSED(argv);
	
	MyMain(argc, argv); //call MyMain to initialize arenas and whatnot
	
	return (sapp_desc){
		.init_cb = AppInit,
		.frame_cb = AppFrame,
		.cleanup_cb = AppCleanup,
		.event_cb = AppEvent,
		.width = 1900,
		.height = 1000,
		.window_title = "Simple Sokol App!",
		.icon.sokol_default = true,
		.logger.func = SokolLogCallback,
	};
}

#endif //BUILD_WITH_SOKOL
