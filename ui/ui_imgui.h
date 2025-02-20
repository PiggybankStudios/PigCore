/*
File:   ui_imgui.h
Author: Taylor Robbins
Date:   02\17\2025
Description:
	** Holds the include for cimgui.h (if BUILD_WITH_IMGUI) and has some wrapper
	** types and functions around the cimgui interface. If BUILD_WITH_IMGUI is set
	** then you must link with cimgui.lib and the .dll must be shipped with the executable
*/

#ifndef _UI_IMGUI_H
#define _UI_IMGUI_H

#include "base/base_defines_check.h"
#include "base/base_assert.h"
#include "base/base_macros.h"
#include "mem/mem_arena.h"
#include "std/std_memset.h"
#include "os/os_file.h"
#include "gfx/gfx_texture.h"

#if BUILD_WITH_IMGUI //TODO: Change me back!

extern Arena* imguiArena; //defined in ui_imgui_main.cpp

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "third_party/cimgui/cimgui.h"

typedef struct ImguiUI ImguiUI;
struct ImguiUI
{
	Arena* arena;
	ImGuiContext* context;
	ImGuiIO* io;
	ImGuiPlatformIO* platformIo;
	ImGuiViewport* viewport;
	Texture fontTexture;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	ImguiUI* InitImguiUI(Arena* arena, const void* nativeWindowPntr);
#endif

//TODO: Ideally these should not just be named, but also located in the APPDATA folder, not next to the application exe
#define IMGUI_INI_FILE_NAME "debug_window_layout.ini"
#define IMGUI_LOG_FILE_NAME "debug_window_log.txt"

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

static void* ImguiAllocCallback(size_t numBytes, void* userData)
{
	Assert(numBytes <= UINTXX_MAX);
	NotNull(userData);
	Arena* arena = (Arena*)userData;
	return (numBytes > 0) ? AllocMem(arena, (uxx)numBytes) : nullptr;
}
static void ImguiFreeCallback(void* allocPntr, void* userData)
{
	NotNull(userData);
	Arena* arena = (Arena*)userData;
	if (allocPntr != nullptr && CanArenaFree(arena)) { FreeMem(arena, allocPntr, 0); }
}

static const char* ImguiGetClipboardTextCallback(void* userData)
{
	NotNull(userData);
	ImguiUI* imgui = (ImguiUI*)userData;
	UNUSED(imgui);
	//TODO: Implement me!
	return nullptr;
}
static void ImguiSetClipboardTextCallback(void* userData, const char* text)
{
	NotNull(userData);
	NotNull(text);
	ImguiUI* imgui = (ImguiUI*)userData;
	UNUSED(imgui);
	//TODO: Implement me!
}

//NOTE: arena has to have ArenaFlag_AllowFreeWithoutSize set!
PEXP ImguiUI* InitImguiUI(Arena* arena, const void* nativeWindowPntr)
{
	NotNull(arena);
	NotNull(nativeWindowPntr);
	Assert(IsFlagSet(arena->flags, ArenaFlag_AllowFreeWithoutSize));
	
	imguiArena = arena;
	
	ImguiUI* result = AllocType(ImguiUI, arena);
	NotNull(result);
	ClearPointer(result);
	result->arena = arena;
	
	igSetAllocatorFunctions(ImguiAllocCallback, ImguiFreeCallback, arena);
	
	result->context = igCreateContext(nullptr);
	NotNull(result->context);
	result->io = igGetIOEx(result->context);
	NotNull(result->io);
	result->platformIo = igGetPlatformIOEx(result->context);
	NotNull(result->platformIo);
	
	FlagSet(result->io->ConfigFlags, ImGuiConfigFlags_NavEnableKeyboard);
	// FlagSet(result->io.ConfigFlags, ImGuiConfigFlags_NavEnableGamepad); //TODO: Enable me once we have gamepad input support!
	igStyleColorsDark(nullptr);
	
	result->io->BackendPlatformName = AllocAndCopyCharsNt(arena, "PigCore", true);
	result->io->BackendRendererName = AllocAndCopyCharsNt(arena, "PigCore", true);
	result->io->BackendPlatformUserData = (void*)result;
	result->io->BackendRendererUserData = (void*)result;
	result->io->ConfigDebugIsDebuggerPresent = DEBUG_BUILD; //TODO: Can we detect this better?
	result->io->IniFilename = AllocAndCopyCharsNt(arena, IMGUI_INI_FILE_NAME, true);
	result->io->LogFilename = AllocAndCopyCharsNt(arena, IMGUI_LOG_FILE_NAME, true);
	
	result->platformIo->Platform_ClipboardUserData = (void*)result;
	result->platformIo->Platform_SetClipboardTextFn = ImguiSetClipboardTextCallback;
	result->platformIo->Platform_GetClipboardTextFn = ImguiGetClipboardTextCallback;
	
	result->viewport = igGetMainViewport();
	NotNull(result->viewport);
	result->viewport->PlatformHandleRaw = (void*)nativeWindowPntr;
	
	u8* fontAtlasPixels = nullptr;
	int fontAtlasWidth = 0, fontAtlasHeight = 0;
	ImFontAtlas_GetTexDataAsRGBA32(result->io->Fonts, &fontAtlasPixels, &fontAtlasWidth, &fontAtlasHeight, nullptr);
	NotNull(fontAtlasPixels);
	Assert(fontAtlasWidth > 0 && fontAtlasHeight > 0);
	
	result->fontTexture = InitTexture(arena, StrLit("ImGuiFontAtlas"), NewV2i((i32)fontAtlasWidth, (i32)fontAtlasHeight), fontAtlasPixels, TextureFlag_NoMipmaps);
	Assert(result->fontTexture.error == Result_Success);
	ImFontAtlas_SetTexID(result->io->Fonts, (ImTextureID)&result->fontTexture);
	
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_IMGUI

#endif //  _UI_IMGUI_H
