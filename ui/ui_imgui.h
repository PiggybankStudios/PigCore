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

#if BUILD_WITH_IMGUI

//TODO: We are not routing imgui allocations through our memory arenas!
// In order to do that we'd have to make modifications to imgui.h to replace the implementations of IM_MALLOC, IM_NEW, etc. and then regenerate/recompile cimgui!
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "third_party/cimgui/cimgui.h"
// #if PIG_CORE_IMPLEMENTATION
// #include "third_party/cimgui/cimgui.cpp"
// #endif

typedef struct ImguiUI ImguiUI;
struct ImguiUI
{
	Arena* arena;
	ImGuiContext* context;
	ImGuiIO* io;
	ImGuiPlatformIO* platformIo;
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	ImguiUI InitImguiUI(Arena* arena);
#endif

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
	if (CanArenaFree(arena)) { FreeMem(arena, allocPntr, 0); }
}

//NOTE: arena has to have ArenaFlag_AllowFreeWithoutSize set!
PEXP ImguiUI InitImguiUI(Arena* arena)
{
	NotNull(arena);
	ImguiUI result = ZEROED;
	result.arena = arena;
	
	igSetAllocatorFunctions(ImguiAllocCallback, ImguiFreeCallback, arena);
	
	result.context = igCreateContext(nullptr);
	NotNull(result.context);
	result.io = igGetIOEx(result.context);
	NotNull(result.io);
	result.platformIo = igGetPlatformIOEx(result.context);
	NotNull(result.platformIo);
	
	FlagSet(result.io->ConfigFlags, ImGuiConfigFlags_NavEnableKeyboard);
	// FlagSet(result.io.ConfigFlags, ImGuiConfigFlags_NavEnableGamepad); //TODO: Enable me once we have gamepad input support!
	igStyleColorsDark(nullptr);
	
	return result;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_IMGUI

#endif //  _UI_IMGUI_H
