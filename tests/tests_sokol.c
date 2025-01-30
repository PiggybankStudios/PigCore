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
Shader simpleShader;
Shader main2dShader;
VertBuffer squareBuffer;
sg_bindings bindings;
sg_pipeline pipeline;

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

sg_swapchain CreateSokolSwapchain()
{
	sg_swapchain result = ZEROED;
	result.width = sapp_width();
	result.height = sapp_height();
	result.sample_count = sapp_sample_count();
	result.color_format = (sg_pixel_format)sapp_color_format();
	result.depth_format = (sg_pixel_format)sapp_depth_format();
	result.metal.current_drawable = sapp_metal_get_current_drawable();
	result.metal.depth_stencil_texture = sapp_metal_get_depth_stencil_texture();
	result.metal.msaa_color_texture = sapp_metal_get_msaa_color_texture();
	result.d3d11.render_view = sapp_d3d11_get_render_view();
	result.d3d11.resolve_view = sapp_d3d11_get_resolve_view();
	result.d3d11.depth_stencil_view = sapp_d3d11_get_depth_stencil_view();
	result.wgpu.render_view = sapp_wgpu_get_render_view();
	result.wgpu.resolve_view = sapp_wgpu_get_resolve_view();
	result.wgpu.depth_stencil_view = sapp_wgpu_get_depth_stencil_view();
	result.gl.framebuffer = sapp_gl_get_framebuffer();
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
	
	ClearStruct(bindings);
	
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
	BindVertBuffer(&bindings, &squareBuffer, 0);
	
	v2i imageSize = NewV2i(32, 32);
	Color32* pixels = AllocArray(Color32, scratch, (uxx)(imageSize.Width * imageSize.Height));
	for (i32 pixelY = 0; pixelY < imageSize.Height; pixelY++)
	{
		for (i32 pixelX = 0; pixelX < imageSize.Width; pixelX++)
		{
			Color32* pixel = &pixels[INDEX_FROM_COORD2D(pixelX, pixelY, imageSize.Width, imageSize.Height)];
			pixel->r = ClampCastI32ToU8(RoundR32i(LerpR32(0, 255.0f, (r32)pixelX / (r32)imageSize.Width)));
			pixel->g = ClampCastI32ToU8(RoundR32i(LerpR32(0, 255.0f, (r32)pixelY / (r32)imageSize.Height)));
			pixel->b = pixel->r/2 + pixel->g/2;
			pixel->a = 255;
		}
	}
	bindings.images[0] = sg_make_image(&(sg_image_desc){
		.width = imageSize.Width,
		.height = imageSize.Height,
		.pixel_format = SG_PIXELFORMAT_BGRA8,
		.data.subimage[0][0] = (sg_range){pixels, sizeof(u32) * imageSize.Width * imageSize.Height},
		.label = "test_texture"
	});
	bindings.samplers[0] = sg_make_sampler(&(sg_sampler_desc){ .label = "test_sampler" });
	
	InitCompiledShader(&simpleShader, stdHeap, simple); Assert(simpleShader.error == Result_Success);
	InitCompiledShader(&main2dShader, stdHeap, main2d); Assert(main2dShader.error == Result_Success);
	
	sg_pipeline_desc pipelineDesc = ZEROED;
	pipelineDesc.label = "triangle-pipeline";
	FillPipelineDescLayout(&pipelineDesc, &main2dShader, &squareBuffer);
	pipelineDesc.depth.pixel_format = _SG_PIXELFORMAT_DEFAULT; //TODO: What format is DEFAULT?
	pipelineDesc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
	pipelineDesc.depth.write_enabled = true;
	pipelineDesc.stencil.enabled = false;
	pipelineDesc.color_count = 1;
	pipelineDesc.colors[0].pixel_format = _SG_PIXELFORMAT_DEFAULT; //TODO: What format is DEFAULT?
	pipelineDesc.colors[0].write_mask = SG_COLORMASK_RGBA;
	pipelineDesc.colors[0].blend.enabled = true;
	pipelineDesc.colors[0].blend.src_factor_rgb = SG_BLENDFACTOR_ONE;
	pipelineDesc.colors[0].blend.dst_factor_rgb = SG_BLENDFACTOR_ZERO;
	pipelineDesc.colors[0].blend.op_rgb = SG_BLENDOP_ADD;
	pipelineDesc.colors[0].blend.src_factor_alpha = SG_BLENDFACTOR_ONE;
	pipelineDesc.colors[0].blend.dst_factor_alpha = SG_BLENDFACTOR_ZERO;
	pipelineDesc.colors[0].blend.op_alpha = SG_BLENDOP_ADD;
	pipelineDesc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
	pipelineDesc.index_type = SG_INDEXTYPE_NONE;
	pipelineDesc.cull_mode = SG_CULLMODE_BACK;
	pipelineDesc.face_winding = SG_FACEWINDING_CW;
	pipeline = sg_make_pipeline(&pipelineDesc);
	
	sokolPassAction = (sg_pass_action){
		.colors[0] = {
			.load_action = SG_LOADACTION_CLEAR,
			.clear_value = { 0.75f, 0.8f, 1.0f, 1.0f }
		},
		.depth = {
			.load_action = SG_LOADACTION_CLEAR,
			.clear_value = 1.0f,
		},
	};
	
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
	SetShaderWorldMat(shader, worldMat);
	SetShaderTintColor(shader, color);
	ApplyShaderUniforms(shader);
	sg_draw(0, (int)squareBuffer.numVertices, 1);
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void AppFrame(void)
{
	v2 windowSize = NewV2(sapp_widthf(), sapp_heightf());
	// float newGreen = sokolPassAction.colors[0].clear_value.g + 0.01f;
	// sokolPassAction.colors[0].clear_value.g = (newGreen > 1.0f) ? 0.0f : newGreen;
	sg_pass mainPass = {
		.action = sokolPassAction,
		.swapchain = CreateSokolSwapchain(),
	};
	sg_begin_pass(&mainPass);
	sg_apply_pipeline(pipeline);
	sg_apply_bindings(&bindings);
	
	mat4 projMat = Mat4_Identity;
	TransformMat4(&projMat, MakeScaleXYZMat4(1.0f/(windowSize.Width/2.0f), 1.0f/(windowSize.Height/2.0f), 1.0f));
	TransformMat4(&projMat, MakeTranslateXYZMat4(-1.0f, -1.0f, 0.0f));
	TransformMat4(&projMat, MakeScaleYMat4(-1.0f));
	SetShaderProjectionMat(&main2dShader, projMat);
	SetShaderViewMat(&main2dShader, Mat4_Identity);
	SetShaderWorldMat(&main2dShader, Mat4_Identity);
	SetShaderSourceRec(&main2dShader, NewV4(0, 0, 4, 4));
	SetShaderUniformByNameV2(&main2dShader, StrLit("main2d_texture0_size"), NewV2(4, 4));
	
	v2 tileSize = NewV2(48, 27);
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
	
	sg_end_pass();
	sg_commit();
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
