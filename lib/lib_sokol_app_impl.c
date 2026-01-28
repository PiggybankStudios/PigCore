/*
File:   lib_sokol_app_impl.c
Author: Taylor Robbins
Date:   10\31\2025
Description: 
	** Include this file in the base executable layer where sokol_app.h implementations should be defined. These can't exist in pig_core.dll, they have to exist in the .exe itself
*/

#define SOKOL_APP_IMPL

#if COMPILER_IS_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers" //warning: missing field 'revents' initializer [-Wmissing-field-initializers]
#pragma clang diagnostic ignored "-Wdeprecated-declarations" //warning: 'NSOpenGLView' is deprecated: first deprecated in macOS 10.14 - Please use MTKView instead.
#pragma clang diagnostic ignored "-Wunused-variable" //warning: unused variable 'frameResult'
#endif

#include "third_party/sokol/sokol_app.h"

#if COMPILER_IS_CLANG
#pragma clang diagnostic pop
#endif

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
void SokolLogCallback(const char* tag, u32 logLevel, u32 logId, const char* message, u32 lineNum, const char* filePath, void* userData)
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
	DebugOutputRouter(filePath, lineNum, __func__, dbgLevel, false, true, message);
	if (logLevel == 0) { AssertMsg(false, "Sokol Panic!"); }
}

sg_environment GetSokolGfxEnvironment()
{
	sg_environment result = ZEROED;
	sapp_environment appEnvironment = sapp_get_environment();
	result.defaults.color_format     = (sg_pixel_format)appEnvironment.defaults.color_format;
	result.defaults.depth_format     = (sg_pixel_format)appEnvironment.defaults.depth_format;
	result.defaults.sample_count     = appEnvironment.defaults.sample_count;
	result.metal.device              = appEnvironment.metal.device;
	result.d3d11.device              = appEnvironment.d3d11.device;
	result.d3d11.device_context      = appEnvironment.d3d11.device_context;
	result.wgpu.device               = appEnvironment.wgpu.device;
	result.vulkan.instance           = appEnvironment.vulkan.instance;
	result.vulkan.physical_device    = appEnvironment.vulkan.physical_device;
	result.vulkan.device             = appEnvironment.vulkan.device;
	result.vulkan.queue              = appEnvironment.vulkan.queue;
	result.vulkan.queue_family_index = appEnvironment.vulkan.queue_family_index;
	return result;
}

sg_swapchain GetSokolGfxSwapchain()
{
	sg_swapchain result = ZEROED;
	sapp_swapchain appSwapchain = sapp_get_swapchain();
	result.width = appSwapchain.width;
	result.height = appSwapchain.height;
	result.sample_count = appSwapchain.sample_count;
	result.color_format = (sg_pixel_format)appSwapchain.color_format;
	result.depth_format = (sg_pixel_format)appSwapchain.depth_format;
	result.metal.current_drawable = appSwapchain.metal.current_drawable;
	result.metal.depth_stencil_texture = appSwapchain.metal.depth_stencil_texture;
	result.metal.msaa_color_texture = appSwapchain.metal.msaa_color_texture;
	result.d3d11.render_view = appSwapchain.d3d11.render_view;
	result.d3d11.resolve_view = appSwapchain.d3d11.resolve_view;
	result.d3d11.depth_stencil_view = appSwapchain.d3d11.depth_stencil_view;
	result.wgpu.render_view = appSwapchain.wgpu.render_view;
	result.wgpu.resolve_view = appSwapchain.wgpu.resolve_view;
	result.wgpu.depth_stencil_view = appSwapchain.wgpu.depth_stencil_view;
	result.vulkan.render_image = appSwapchain.vulkan.render_image;
	result.vulkan.render_view = appSwapchain.vulkan.render_view;
	result.vulkan.resolve_image = appSwapchain.vulkan.resolve_image;
	result.vulkan.resolve_view = appSwapchain.vulkan.resolve_view;
	result.vulkan.depth_stencil_image = appSwapchain.vulkan.depth_stencil_image;
	result.vulkan.depth_stencil_view = appSwapchain.vulkan.depth_stencil_view;
	result.vulkan.render_finished_semaphore = appSwapchain.vulkan.render_finished_semaphore;
	result.vulkan.present_complete_semaphore = appSwapchain.vulkan.present_complete_semaphore;
	result.gl.framebuffer = appSwapchain.gl.framebuffer;
	return result;
}
