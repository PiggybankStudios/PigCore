/*
File:   platform_main.c
Author: Taylor Robbins
Date:   01\19\2025
Description: 
	** Holds the main entry point for the application and #includes all source files
	** that are needed for the platform layer to be compiled to an executable.
	** If BUILD_INTO_SINGLE_UNIT then this file #includes app_main.c as well.
*/

#include "build_config.h"
#include "defines.h"
#define PIG_CORE_IMPLEMENTATION BUILD_INTO_SINGLE_UNIT

#include "base/base_all.h"
#include "std/std_all.h"
#include "os/os_all.h"
#include "mem/mem_all.h"
#include "struct/struct_all.h"
#include "misc/misc_all.h"
#include "input/input_all.h"
#include "file_fmt/file_fmt_all.h"
#include "ui/ui_all.h"
#include "gfx/gfx_all.h"
#include "gfx/gfx_system_global.h"
#include "phys/phys_all.h"

#if BUILD_WITH_RAYLIB
#include "third_party/raylib/raylib.h"
#endif

#if BUILD_WITH_SOKOL_APP
#define SOKOL_APP_IMPL
#if TARGET_IS_LINUX
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers" //warning: missing field 'revents' initializer [-Wmissing-field-initializers]
#endif
#include "third_party/sokol/sokol_app.h"
#if TARGET_IS_LINUX
#pragma clang diagnostic pop
#endif
#endif //BUILD_WITH_SOKOL_APP

#if BUILD_WITH_SOKOL_APP
#include "misc/misc_sokol_app_helpers.c"
#endif

#define ENABLE_RAYLIB_LOGS_DEBUG   0
#define ENABLE_RAYLIB_LOGS_INFO    0
#define ENABLE_RAYLIB_LOGS_WARNING 1
#define ENABLE_RAYLIB_LOGS_ERROR   1

// +--------------------------------------------------------------+
// |                         Header Files                         |
// +--------------------------------------------------------------+
#include "platform_interface.h"
#include "platform_main.h"
// TODO: Add header files here

#if BUILD_INTO_SINGLE_UNIT
EXPORT_FUNC(AppGetApi) APP_GET_API_DEF(AppGetApi);
#endif

// +--------------------------------------------------------------+
// |                       Platform Globals                       |
// +--------------------------------------------------------------+
PlatformData* platformData = nullptr;
//These globals are shared between app and platform when BUILD_INTO_SINGLE_UNIT
Arena* stdHeap = nullptr;
PlatformInfo* platformInfo = nullptr;
PlatformApi* platform = nullptr;

// +--------------------------------------------------------------+
// |                    Platform Source Files                     |
// +--------------------------------------------------------------+
#include "platform_api.c"
// TODO: Add source files here

#if BUILD_WITH_RAYLIB
void RaylibLogCallback(int logLevel, const char* text, va_list args)
{
	DbgLevel dbgLevel;
	switch (logLevel)
	{
		case LOG_TRACE:   dbgLevel = DbgLevel_Debug;   break;
		case LOG_DEBUG:   dbgLevel = DbgLevel_Debug;   break;
		case LOG_INFO:    dbgLevel = DbgLevel_Info;    break;
		case LOG_WARNING: dbgLevel = DbgLevel_Warning; break;
		case LOG_ERROR:   dbgLevel = DbgLevel_Error;   break;
		case LOG_FATAL:   dbgLevel = DbgLevel_Error;   break;
		default: dbgLevel = DbgLevel_Regular; break;
	}
	if (dbgLevel == DbgLevel_Debug && !ENABLE_RAYLIB_LOGS_DEBUG) { return; }
	if (dbgLevel == DbgLevel_Info && !ENABLE_RAYLIB_LOGS_INFO) { return; }
	if (dbgLevel == DbgLevel_Warning && !ENABLE_RAYLIB_LOGS_WARNING) { return; }
	if (dbgLevel == DbgLevel_Error && !ENABLE_RAYLIB_LOGS_ERROR) { return; }
	
	ScratchBegin(scratch);
	va_list argsCopy;
	va_copy(argsCopy, args);
	char* formattedText = nullptr;
	int formattedTextLength = MyVaListPrintf(nullptr, 0, text, args);
	if (formattedTextLength >= 0)
	{
		formattedText = AllocArray(char, scratch, formattedTextLength+1);
		if (formattedText != nullptr)
		{
			MyVaListPrintf(formattedText, formattedTextLength+1, text, argsCopy);
			formattedText[formattedTextLength] = '\0';
		}
	}
	va_end(argsCopy);
	if (formattedText != nullptr)
	{
		WriteLineAt(dbgLevel, formattedText);
	}
	else
	{
		WriteLine_E("RaylibLogCallback PRINT FAILURE!");
		WriteLineAt(dbgLevel, text);
	}
	ScratchEnd(scratch);
}
#endif //BUILD_WITH_RAYLIB

void PlatDoUpdate(void)
{
	//TODO: Check for dll changes, reload it!
	
	//Swap which appInput is being written to and pass the static version to the application
	AppInput* oldAppInput = platformData->currentAppInput;
	AppInput* newAppInput = (platformData->currentAppInput == &platformData->appInputs[0]) ? &platformData->appInputs[1] : &platformData->appInputs[0];
	
	#if BUILD_WITH_RAYLIB
	v2i newScreenSize = NewV2i((i32)GetRenderWidth(), (i32)GetRenderHeight());
	bool newIsFullScreen = IsWindowFullscreen();
	bool isMouseLocked = IsCursorHidden();
	#elif BUILD_WITH_SOKOL_APP
	v2i newScreenSize = NewV2i(sapp_width(), sapp_height());
	bool newIsFullScreen = sapp_is_fullscreen();
	bool isMouseLocked = sapp_mouse_locked();
	#else
	v2i newScreenSize = NewV2i(800, 600);
	bool newIsFullScreen = false;
	bool isMouseLocked = false;
	#endif
	
	if (!AreEqual(newScreenSize, oldAppInput->screenSize)) { oldAppInput->screenSizeChanged = true; }
	oldAppInput->screenSize = newScreenSize;
	if (oldAppInput->isFullscreen != newIsFullScreen) { oldAppInput->isFullscreenChanged = true; }
	oldAppInput->isFullscreen = newIsFullScreen;
	
	MyMemCopy(newAppInput, oldAppInput, sizeof(AppInput));
	newAppInput->screenSizeChanged = false;
	newAppInput->isFullscreenChanged = false;
	newAppInput->isMinimizedChanged = false;
	newAppInput->isFocusedChanged = false;
	RefreshKeyboardState(&newAppInput->keyboard);
	RefreshMouseState(&newAppInput->mouse, isMouseLocked, NewV2((r32)newScreenSize.Width/2.0f, (r32)newScreenSize.Height/2.0f));
	IncrementU64(newAppInput->frameIndex);
	IncrementU64By(newAppInput->programTime, 16); //TODO: Replace this hardcoded increment!
	platformData->oldAppInput = oldAppInput;
	platformData->currentAppInput = newAppInput;
	
	bool shouldContinueRunning = platformData->appApi.AppUpdate(platformInfo, platform, platformData->appMemoryPntr, oldAppInput);
	
	#if BUILD_WITH_RAYLIB
	if (!shouldContinueRunning) { CloseWindow(); }
	#elif BUILD_WITH_SOKOL_APP
	if (!shouldContinueRunning) { sapp_quit(); }
	#else
	UNUSED(shouldContinueRunning);
	#endif
}

// +--------------------------------------------------------------+
// |                       Main Entry Point                       |
// +--------------------------------------------------------------+
#if BUILD_WITH_SOKOL_APP
void PlatSappInit(void)
#else
int main()
#endif
{
	Arena stdHeapLocal = ZEROED;
	InitArenaStdHeap(&stdHeapLocal);
	platformData = AllocType(PlatformData, &stdHeapLocal);
	NotNull(platformData);
	ClearPointer(platformData);
	MyMemCopy(&platformData->stdHeap, &stdHeapLocal, sizeof(Arena));
	stdHeap = &platformData->stdHeap;
	InitArenaStdHeap(&platformData->stdHeapAllowFreeWithoutSize);
	FlagSet(platformData->stdHeapAllowFreeWithoutSize.flags, ArenaFlag_AllowFreeWithoutSize);
	InitScratchArenasVirtual(Gigabytes(4));
	
	ScratchBegin(loadScratch);
	
	#if BUILD_WITH_RAYLIB
	SetTraceLogCallback(RaylibLogCallback);
	InitWindow(800, 600, PROJECT_READABLE_NAME_STR);
	SetWindowMinSize(400, 200);
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(60);
	#endif //BUILD_WITH_RAYLIB
	
	InitKeyboardState(&platformData->appInputs[0].keyboard);
	InitKeyboardState(&platformData->appInputs[1].keyboard);
	InitMouseState(&platformData->appInputs[0].mouse);
	InitMouseState(&platformData->appInputs[1].mouse);
	platformData->currentAppInput = &platformData->appInputs[0];
	platformData->oldAppInput = &platformData->appInputs[1];
	
	platformInfo = AllocType(PlatformInfo, stdHeap);
	NotNull(platformInfo);
	ClearPointer(platformInfo);
	platformInfo->platformStdHeap = stdHeap;
	platformInfo->platformStdHeapAllowFreeWithoutSize = &platformData->stdHeapAllowFreeWithoutSize;
	
	platform = AllocType(PlatformApi, stdHeap);
	NotNull(platform);
	ClearPointer(platform);
	platform->GetNativeWindowHandle = Plat_GetNativeWindowHandle;
	#if BUILD_WITH_SOKOL_APP
	platform->GetSokolSwapchain = Plat_GetSokolSwapchain;
	platform->SetMouseLocked = Plat_SetMouseLocked;
	platform->SetMouseCursorType = Plat_SetMouseCursorType;
	platform->SetWindowTitle = Plat_SetWindowTitle;
	platform->SetWindowIcon = Plat_SetWindowIcon;
	#endif
	
	#if BUILD_INTO_SINGLE_UNIT
	{
		WriteLine_N("Compiled as single unit!");
		platformData->appApi = AppGetApi();
	}
	#else
	{
		#if TARGET_IS_WINDOWS
		FilePath dllPath = StrLit(PROJECT_DLL_NAME_STR ".dll");
		#elif TARGET_IS_LINUX
		FilePath dllPath = StrLit("./" PROJECT_DLL_NAME_STR ".so");
		#else
		#error Current TARGET doesn't have an implementation for shared library suffix!
		#endif
		Result loadDllResult = OsLoadDll(dllPath, &platformData->appDll);
		if (loadDllResult != Result_Success) { PrintLine_E("Failed to load \"%.*s\": %s", StrPrint(dllPath), GetResultStr(loadDllResult)); }
		Assert(loadDllResult == Result_Success);
		
		AppGetApi_f* appGetApi = (AppGetApi_f*)OsFindDllFunc(&platformData->appDll, StrLit("AppGetApi"));
		NotNull(appGetApi);
		platformData->appApi = appGetApi();
		NotNull(platformData->appApi.AppInit);
		NotNull(platformData->appApi.AppUpdate);
	}
	#endif
	
	//TODO: Should we do an early call into app dll to get options?
	
	#if BUILD_WITH_SOKOL_GFX
	InitSokolGraphics((sg_desc){
		// .buffer_pool_size = ?; //int
		// .image_pool_size = ?; //int
		// .sampler_pool_size = ?; //int
		// .shader_pool_size = ?; //int
		// .pipeline_pool_size = ?; //int
		// .attachments_pool_size = ?; //int
		// .uniform_buffer_size = ?; //int
		// .max_commit_listeners = ?; //int
		// .disable_validation = ?; //bool    // disable validation layer even in debug mode, useful for tests
		// .d3d11_shader_debugging = ?; //bool    // if true, HLSL shaders are compiled with D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION
		// .mtl_force_managed_storage_mode = ?; //bool // for debugging: use Metal managed storage mode for resources even with UMA
		// .mtl_use_command_buffer_with_retained_references = ?; //bool    // Metal: use a managed MTLCommandBuffer which ref-counts used resources
		// .wgpu_disable_bindgroups_cache = ?; //bool  // set to true to disable the WebGPU backend BindGroup cache
		// .wgpu_bindgroups_cache_size = ?; //int      // number of slots in the WebGPU bindgroup cache (must be 2^N)
		// .allocator = ?; //sg_allocator TODO: Fill this out!
		.environment = CreateSokolAppEnvironment(),
		.logger.func = SokolLogCallback,
		
	});
	InitGfxSystem(stdHeap, &gfx);
	#if DEBUG_BUILD
	gfx.prevFontFlow.numGlyphsAlloc = 256;
	gfx.prevFontFlow.glyphs = AllocArray(FontFlowGlyph, stdHeap, gfx.prevFontFlow.numGlyphsAlloc);
	NotNull(gfx.prevFontFlow.glyphs);
	#endif
	#endif
	
	platformData->appMemoryPntr = platformData->appApi.AppInit(platformInfo, platform);
	NotNull(platformData->appMemoryPntr);
	
	ScratchEnd(loadScratch);
	
	// +--------------------------------------------------------------+
	// |                        Main Game Loop                        |
	// +--------------------------------------------------------------+
	#if BUILD_WITH_RAYLIB
	while (!WindowShouldClose())
	{
		//Grab all scratch arenas so we can ensure they get reset at the end of each frame
		ScratchBegin(scratch1);
		ScratchBegin1(scratch2, scratch1);
		ScratchBegin2(scratch3, scratch1, scratch2);
		
		PlatDoUpdate();
		
		ScratchEnd(scratch1);
		ScratchEnd(scratch2);
		ScratchEnd(scratch3);
	}
	#if !BUILD_INTO_SINGLE_UNIT
	CloseWindow();
	#endif
	#endif //BUILD_WITH_RAYLIB
	
	#if !BUILD_WITH_SOKOL_APP
	return 0;
	#endif
}

#if BUILD_WITH_SOKOL_APP

void PlatSappCleanup(void)
{
	platformData->appApi.AppClosing(platformInfo, platform, platformData->appMemoryPntr);
	ShutdownSokolGraphics();
}

void PlatSappEvent(const sapp_event* event)
{
	bool handledEvent = false;
	
	if (platformData->currentAppInput != nullptr)
	{
		handledEvent = HandleSokolKeyboardAndMouseEvents(
			event,
			platformData->currentAppInput->programTime, //TODO: Calculate a more accurate programTime to pass here!
			&platformData->currentAppInput->keyboard,
			&platformData->currentAppInput->mouse,
			sapp_mouse_locked()
		);
	}
	
	if (!handledEvent)
	{
		switch (event->type)
		{
			case SAPP_EVENTTYPE_TOUCHES_BEGAN:     WriteLine_D("Event: TOUCHES_BEGAN");     break;
			case SAPP_EVENTTYPE_TOUCHES_MOVED:     WriteLine_D("Event: TOUCHES_MOVED");     break;
			case SAPP_EVENTTYPE_TOUCHES_ENDED:     WriteLine_D("Event: TOUCHES_ENDED");     break;
			case SAPP_EVENTTYPE_TOUCHES_CANCELLED: WriteLine_D("Event: TOUCHES_CANCELLED"); break;
			case SAPP_EVENTTYPE_RESIZED:           /*PrintLine_D("Event: RESIZED %dx%d / %dx%d", event->window_width, event->window_height, event->framebuffer_width, event->framebuffer_height);*/ break;
			case SAPP_EVENTTYPE_ICONIFIED:
			{
				if (platformData->currentAppInput != nullptr && platformData->currentAppInput->isMinimized == false)
				{
					platformData->currentAppInput->isMinimized = true;
					platformData->currentAppInput->isMinimizedChanged = true;
				}
			} break;
			case SAPP_EVENTTYPE_RESTORED:
			{
				if (platformData->currentAppInput != nullptr && platformData->currentAppInput->isMinimized == true)
				{
					platformData->currentAppInput->isMinimized = false;
					platformData->currentAppInput->isMinimizedChanged = true;
				}
			} break;
			case SAPP_EVENTTYPE_FOCUSED:
			{
				if (platformData->currentAppInput != nullptr && platformData->currentAppInput->isFocused == false)
				{
					platformData->currentAppInput->isFocused = true;
					platformData->currentAppInput->isFocusedChanged = true;
				}
			} break;
			case SAPP_EVENTTYPE_UNFOCUSED:
			{
				if (platformData->currentAppInput != nullptr && platformData->currentAppInput->isFocused == true)
				{
					platformData->currentAppInput->isFocused = false;
					platformData->currentAppInput->isFocusedChanged = true;
				}
			} break;
			case SAPP_EVENTTYPE_SUSPENDED:         WriteLine_D("Event: SUSPENDED");         break;
			case SAPP_EVENTTYPE_RESUMED:           WriteLine_D("Event: RESUMED");           break;
			case SAPP_EVENTTYPE_QUIT_REQUESTED:    WriteLine_D("Event: QUIT_REQUESTED");    break;
			case SAPP_EVENTTYPE_CLIPBOARD_PASTED:  WriteLine_D("Event: CLIPBOARD_PASTED");  break;
			case SAPP_EVENTTYPE_FILES_DROPPED:     WriteLine_D("Event: FILES_DROPPED");     break;
			default: PrintLine_D("Event: UNKNOWN(%d)", event->type); break;
		}
	}
}

sapp_desc sokol_main(int argc, char* argv[])
{
	UNUSED(argc);
	UNUSED(argv);
	return (sapp_desc){
		.init_cb = PlatSappInit,
		.frame_cb = PlatDoUpdate,
		.cleanup_cb = PlatSappCleanup,
		.event_cb = PlatSappEvent,
		.width = 800,
		.height = 600,
		.window_title = "Loading...",
		.icon.sokol_default = false,
		.logger.func = SokolLogCallback,
	};
}

#endif

#if BUILD_INTO_SINGLE_UNIT
#include "app/app_main.c"
#endif
