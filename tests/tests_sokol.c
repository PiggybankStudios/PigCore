/*
File:   tests_sokol.c
Author: Taylor Robbins
Date:   01\28\2025
Description: 
	** None
*/

#if BUILD_WITH_SOKOL

#define SOKOL_ASSERT(condition) Assert(condition)
#define SOKOL_UNREACHABLE Assert(false)
#if DEBUG_BUILD
#define SOKOL_DEBUG
#endif

#if TARGET_IS_WINDOWS
#define SOKOL_D3D11
// #define SOKOL_GLCORE
#elif TARGET_IS_LINUX
#define SOKOL_GLCORE
#elif TARGET_IS_WEB
#define SOKOL_WGPU
#endif

#if TARGET_IS_WASM
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough" //warning: unannotated fall-through between switch labels [-Wimplicit-fallthrough]
#endif
#define SOKOL_GFX_IMPL
#include "third_party/sokol/sokol_gfx.h"
#if TARGET_IS_WASM
#pragma clang diagnostic pop
#endif

#if TARGET_IS_LINUX
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers" //warning: missing field 'revents' initializer [-Wmissing-field-initializers]
#endif
#define SOKOL_APP_IMPL
#include "third_party/sokol/sokol_app.h"
#if TARGET_IS_LINUX
#pragma clang diagnostic pop
#endif

#include "tests/simple_shader.glsl.h"

int MyMain(int argc, char* argv[]);

typedef union Vertex Vertex;
union Vertex
{
	r32 values[8];
	struct
	{
		v3 position;
		// r32 padding; //TODO: We are having trouble getting v4 (HMM_Vec4) to not align to 16 bytes!
		v4 color;
	};
};
_Static_assert(STRUCT_VAR_OFFSET(Vertex, color) == sizeof(r32)*4);

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
sg_pass_action sokolPassAction;
sg_shader simpleShader;
sg_bindings bindings;
sg_pipeline pipeline;

// +--------------------------------------------------------------+
// |                    Callbacks and Helpers                     |
// +--------------------------------------------------------------+
void SokolLogCallback(const char* tag, uint32_t logLevel, uint32_t logId, const char* message, uint32_t lineNum, const char* filePath, void* userData)
{
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
	sg_setup(&(sg_desc){
		.environment = CreateSokolEnvironment(),
		.logger.func = SokolLogCallback,
	});
	
	ClearStruct(bindings);
	
	Vertex vertices[] = {
		{  0.0f,  0.5f, 0.5f,  0,  1.0f, 0.0f, 0.0f, 1.0f },
		{  0.5f, -0.5f, 0.5f,  0,  0.0f, 1.0f, 0.0f, 1.0f },
		{ -0.5f, -0.5f, 0.5f,  0,  0.0f, 0.0f, 1.0f, 1.0f },
	};
	sg_buffer_desc bufferDesc = {
		.data = SG_RANGE(vertices),
		.usage = SG_USAGE_IMMUTABLE,
		.label = "triangle-vertices",
	};
	bindings.vertex_buffers[0] = sg_make_buffer(&bufferDesc);
	
	simpleShader = sg_make_shader(simple_shader_desc(sg_query_backend()));
	Assert(simpleShader.id != 0);
	
	sg_pipeline_desc pipelineDesc = {
		.shader = simpleShader,
		.layout = {
			.buffers[0].stride = sizeof(Vertex),
			.attrs = {
				[ATTR_simple_position].format = SG_VERTEXFORMAT_FLOAT3,
				[ATTR_simple_position].offset = STRUCT_VAR_OFFSET(Vertex, position),
				[ATTR_simple_color0].format = SG_VERTEXFORMAT_FLOAT4,
				[ATTR_simple_color0].offset = STRUCT_VAR_OFFSET(Vertex, color),
			},
		},
		.label = "triangle-pipeline",
	};
	pipeline = sg_make_pipeline(&pipelineDesc);
	
	sokolPassAction = (sg_pass_action){
		.colors[0] = {
			.load_action = SG_LOADACTION_CLEAR,
			.clear_value = { 0.75f, 0.8f, 1.0f, 1.0f }
		}
	};
}

void AppCleanup(void)
{
	sg_shutdown();
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void AppFrame(void)
{
	// float newGreen = sokolPassAction.colors[0].clear_value.g + 0.01f;
	// sokolPassAction.colors[0].clear_value.g = (newGreen > 1.0f) ? 0.0f : newGreen;
	sg_pass mainPass = {
		.action = sokolPassAction,
		.swapchain = CreateSokolSwapchain(),
	};
	sg_begin_pass(&mainPass);
	sg_apply_pipeline(pipeline);
	sg_apply_bindings(&bindings);
	sg_draw(0, 3, 1);
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
		case SAPP_EVENTTYPE_MOUSE_ENTER:       WriteLine_D("Event: MOUSE_ENTER");       break;
		case SAPP_EVENTTYPE_MOUSE_LEAVE:       WriteLine_D("Event: MOUSE_LEAVE");       break;
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
