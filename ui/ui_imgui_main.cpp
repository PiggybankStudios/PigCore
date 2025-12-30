/*
File:   ui_imgui_main.cpp
Author: Taylor Robbins
Date:   02\18\2025
Description: 
	** This file is meant to be compiled separately, in C++ mode, and contains
	** the implementation of imgui with cimgui exports on top. We've opted to go
	** this route rather than relying on cimgui's build pipeline because we want
	** to modify many of the things you might change in imconfig to route imgui
	** functionality to our own facilities (increasing our portability by
	** decreasing our dependencies where needed)
*/

#include "build_config.h"
#define PIG_CORE_IMPLEMENTATION 0
#include "base/base_defines_check.h"
#include "ui/ui_imconfig.h"

#undef Min
#undef Max

#if COMPILER_IS_MSVC
#pragma warning(push)
#pragma warning(disable:4100) //unreferenced formal parameter
#pragma warning(disable:5262) //implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases
#endif

// #define CIMGUI_DEFINE_ENUMS_AND_STRUCTS //TODO: Should we be defining this?
// #include "third_party/cimgui/cimgui.h"
#include "third_party/cimgui/cimgui.cpp"

#include "imgui.cpp"
#include "imgui_demo.cpp"
#include "imgui_draw.cpp"
#include "imgui_tables.cpp"
#include "imgui_widgets.cpp"

#if TARGET_IS_WINDOWS
#pragma warning(pop)
#endif

START_EXTERN_C
Arena* imguiArena = nullptr; //Declared in in ui_imgui.h
END_EXTERN_C

int ImFormatString(char* bufferPntr, size_t bufferSize, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int result = MyVaListPrintf(bufferPntr, bufferSize, fmt, args);
	va_end(args);
	if (bufferPntr == nullptr) { return result; }
	if (result == -1 || result >= (int)bufferSize) { result = (int)(bufferSize - 1); }
	bufferPntr[result] = '\0';
	return result;
}
int ImFormatStringV(char* bufferPntr, size_t bufferSize, const char* fmt, va_list args)
{
	int result = MyVaListPrintf(bufferPntr, bufferSize, fmt, args);
	if (bufferPntr == nullptr) { return result; }
	if (result == -1 || result >= (int)bufferSize) { result = (int)(bufferSize - 1); }
	bufferPntr[result] = '\0';
	return result;
}
ImFileHandle ImFileOpen(const char* filename, const char* mode)
{
	NotNull(filename);
	NotNull(mode);
	NotNull(imguiArena);
	ImGuiFile* result = AllocType(ImGuiFile, imguiArena);
	NotNull(result);
	ClearPointer(result);
	result->arena = imguiArena;
	Str8 modeStr = MakeStr8Nt(mode);
	result->convertNewLines = !StrExactContains(modeStr, StrLit("b"));
	OsOpenFileMode openMode = OsOpenFileMode_None;
	if (StrExactContains(modeStr, StrLit("a"))) { openMode = OsOpenFileMode_Append; }
	else if (StrExactContains(modeStr, StrLit("w"))) { openMode = OsOpenFileMode_Create; }
	else if (StrExactContains(modeStr, StrLit("r+"))) { openMode = OsOpenFileMode_Write; }
	else if (StrExactContains(modeStr, StrLit("r"))) { openMode = OsOpenFileMode_Read; }
	bool openResult = OsOpenFile(imguiArena, MakeFilePathNt(filename), openMode, (openMode != OsOpenFileMode_Write), &result->file);
	if (!openResult) { if (CanArenaFree(imguiArena)) { FreeType(ImGuiFile, imguiArena, result); } return nullptr; }
	return result;
}
bool ImFileClose(ImFileHandle file)
{
	NotNull(file);
	NotNull(file->arena);
	NotNull(file->file.arena);
	OsCloseFile(&file->file);
	if (CanArenaFree(file->arena)) { FreeType(ImGuiFile, file->arena, file); }
	return true;
}
u64 ImFileGetSize(ImFileHandle file)
{
	NotNull(file);
	NotNull(file->arena);
	NotNull(file->file.arena);
	return file->file.fileSize;
}
u64 ImFileRead(void* data, u64 size, u64 count, ImFileHandle file)
{
	NotNull(file);
	NotNull(file->arena);
	NotNull(file->file.arena);
	if (size == 0 || count == 0) { return 0; }
	NotNull(data);
	Assert(size * count <= UINTXX_MAX);
	uxx numBytesRead = 0;
	Result readResult = OsReadFromOpenFile(&file->file, (uxx)(size * count), file->convertNewLines, data, &numBytesRead);
	if (readResult != Result_Success && readResult != Result_Partial) { return 0; }
	return numBytesRead;
}
u64 ImFileWrite(const void* data, u64 size, u64 count, ImFileHandle file)
{
	NotNull(file);
	NotNull(file->arena);
	NotNull(file->file.arena);
	bool writeResult = OsWriteToOpenFile(&file->file, MakeSlice((size * count), data), file->convertNewLines);
	return (writeResult ? (size * count) : 0);
}
