/*
File:   ui_imconfig.h
Author: Taylor Robbins
Date:   02\18\2025
Description:
	** This file ONLY is included by imconfig.h which is in third_party\imgui\
	** It is not included in the list of ui files in ui_all.h and it's not meant
	** to be included outside the ui_imgui_main.cpp compilation unit
*/

#ifndef _UI_IMCONFIG_H
#define _UI_IMCONFIG_H

#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "std/std_includes.h"
START_EXTERN_C
#include "base/base_assert.h"
#include "base/base_math.h"
#include "std/std_basic_math.h"
#include "std/std_trig.h"
#include "std/std_printf.h"
#include "os/os_file.h"
END_EXTERN_C

//---- Define assertion handler. Defaults to calling assert().
// If your macro uses multiple statements, make sure is enclosed in a 'do { .. } while (0)' block so it can be used as a single statement.
#define IM_ASSERT(expression) do { Assert(expression); } while(0)

//---- Define attributes of all API symbols declarations, e.g. for DLL under Windows
// Using Dear ImGui via a shared library is not recommended, because of function call overhead and because we don't guarantee backward nor forward ABI compatibility.
// - Windows DLL users: heaps and globals are not shared across DLL boundaries! You will need to call SetCurrentContext() + SetAllocatorFunctions()
//   for each static/DLL boundary you are calling from. Read "Context and Memory Allocators" section of imgui.cpp for more details.
//#define IMGUI_API __declspec(dllexport)                   // MSVC Windows: DLL export
//#define IMGUI_API __declspec(dllimport)                   // MSVC Windows: DLL import
//#define IMGUI_API __attribute__((visibility("default")))  // GCC/Clang: override visibility when set is hidden

//---- Don't define obsolete functions/enums/behaviors. Consider enabling from time to time after updating to clean your code of obsolete function/names.
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
// #define IMGUI_DISABLE_OBSOLETE_KEYIO TODO: Does this still exist?

//---- Disable all of Dear ImGui or don't implement standard windows/tools.
// It is very strongly recommended to NOT disable the demo windows and debug tool during development. They are extremely useful in day to day work. Please read comments in imgui_demo.cpp.
//#define IMGUI_DISABLE                                     // Disable everything: all headers and source files will be empty.
#if !DEBUG_BUILD
#define IMGUI_DISABLE_DEMO_WINDOWS                          // Disable demo windows: ShowDemoWindow()/ShowStyleEditor() will be empty.
#endif
//#define IMGUI_DISABLE_DEBUG_TOOLS                         // Disable metrics/debugger and other debug tools: ShowMetricsWindow(), ShowDebugLogWindow() and ShowIDStackToolWindow() will be empty.

//---- Don't implement some functions to reduce linkage requirements.
//#define IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS   // [Win32] Don't implement default clipboard handler. Won't use and link with OpenClipboard/GetClipboardData/CloseClipboard etc. (user32.lib/.a, kernel32.lib/.a)
//#define IMGUI_ENABLE_WIN32_DEFAULT_IME_FUNCTIONS          // [Win32] [Default with Visual Studio] Implement default IME handler (require imm32.lib/.a, auto-link for Visual Studio, -limm32 on command-line for MinGW)
//#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS         // [Win32] [Default with non-Visual Studio compilers] Don't implement default IME handler (won't require imm32.lib/.a)
//#define IMGUI_DISABLE_WIN32_FUNCTIONS                     // [Win32] Won't use and link with any Win32 function (clipboard, IME).
//#define IMGUI_ENABLE_OSX_DEFAULT_CLIPBOARD_FUNCTIONS      // [OSX] Implement default OSX clipboard handler (need to link with '-framework ApplicationServices', this is why this is not the default).
//#define IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS             // Don't implement default platform_io.Platform_OpenInShellFn() handler (Win32: ShellExecute(), require shell32.lib/.a, Mac/Linux: use system("")).

#define IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS              // Don't implement ImFormatString/ImFormatStringV so you can implement them yourself (e.g. if you don't want to link with vsnprintf)
int ImFormatString(char* bufferPntr, size_t bufferSize, const char* fmt, ...);
int ImFormatStringV(char* bufferPntr, size_t bufferSize, const char* fmt, va_list args);

#define IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS              // Don't implement ImFabs/ImSqrt/ImPow/ImFmod/ImCos/ImSin/ImAcos/ImAtan2 so you can implement them yourself.
#define ImFabs(X)           AbsR32(X)
#define ImSqrt(x)           SqrtR32(x)
#define ImFmod(X, Y)        ModR32((X), (Y))
#define ImCos(X)            CosR32(X)
#define ImSin(X)            SinR32(X)
#define ImAcos(X)           AcosR32(X)
#define ImAtan2(Y, X)       AtanR32((Y), (X))
#define ImAtof(STR)         MyStrToFloat(STR)
#define ImCeil(X)           CeilR32(X)
static inline float  ImPow(float x, float y)    { return PowR32(x, y); }
static inline double ImPow(double x, double y)  { return PowR64(x, y); }
static inline float  ImLog(float x)             { return Log2R32(x); }
static inline double ImLog(double x)            { return Log2R64(x); }
static inline int    ImAbs(int x)               { return AbsI32(x); }
static inline float  ImAbs(float x)             { return AbsR32(x); }
static inline double ImAbs(double x)            { return AbsR64(x); }
static inline float  ImSign(float x)            { return SignOfR32(x); }
static inline double ImSign(double x)           { return SignOfR64(x); }
#ifdef IMGUI_ENABLE_SSE
static inline float  ImRsqrt(float x)           { return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x))); }
#else
static inline float  ImRsqrt(float x)           { return 1.0f / SqrtR32(x); }
#endif
static inline double ImRsqrt(double x)          { return 1.0 / SqrtR64(x); }

//#define IMGUI_DISABLE_FILE_FUNCTIONS                      // Don't implement ImFileOpen/ImFileClose/ImFileRead/ImFileWrite and ImFileHandle at all (replace them with dummies)

#define IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS                // Don't implement ImFileOpen/ImFileClose/ImFileRead/ImFileWrite and ImFileHandle so you can implement them yourself if you don't want to link with fopen/fclose/fread/fwrite. This will also disable the LogToTTY() function.
typedef struct ImGuiFile ImGuiFile;
struct ImGuiFile
{
	Arena* arena;
	bool convertNewLines;
	OsFile file;
};
typedef ImGuiFile* ImFileHandle;
ImFileHandle ImFileOpen(const char* filename, const char* mode);
bool ImFileClose(ImFileHandle file);
u64 ImFileGetSize(ImFileHandle file);
u64 ImFileRead(void* data, u64 size, u64 count, ImFileHandle file);
u64 ImFileWrite(const void* data, u64 size, u64 count, ImFileHandle file);

#define IMGUI_DISABLE_DEFAULT_ALLOCATORS                    // Don't implement default allocators calling malloc()/free() to avoid linking with them. You will need to call ImGui::SetAllocatorFunctions().

//#define IMGUI_DISABLE_DEFAULT_FONT                        // Disable default embedded font (ProggyClean.ttf), remove ~9.5 KB from output binary. AddFontDefault() will assert.
//#define IMGUI_DISABLE_SSE                                 // Disable use of SSE intrinsics even if available

//---- Enable Test Engine / Automation features.
//#define IMGUI_ENABLE_TEST_ENGINE                          // Enable imgui_test_engine hooks. Generally set automatically by include "imgui_te_config.h", see Test Engine for details.

//---- Include imgui_user.h at the end of imgui.h as a convenience
// May be convenient for some users to only explicitly include vanilla imgui.h and have extra stuff included.
//#define IMGUI_INCLUDE_IMGUI_USER_H
//#define IMGUI_USER_H_FILENAME         "my_folder/my_imgui_user.h"

//---- Pack vertex colors as BGRA8 instead of RGBA8 (to avoid converting from one to another). Need dedicated backend support.
//#define IMGUI_USE_BGRA_PACKED_COLOR

//---- Use legacy CRC32-adler tables (used before 1.91.6), in order to preserve old .ini data that you cannot afford to invalidate.
//#define IMGUI_USE_LEGACY_CRC32_ADLER

//---- Use 32-bit for ImWchar (default is 16-bit) to support Unicode planes 1-16. (e.g. point beyond 0xFFFF like emoticons, dingbats, symbols, shapes, ancient languages, etc...)
//#define IMGUI_USE_WCHAR32

//---- Avoid multiple STB libraries implementations, or redefine path/filenames to prioritize another version
// By default the embedded implementations are declared static and not available outside of Dear ImGui sources files.
//#define IMGUI_STB_TRUETYPE_FILENAME   "my_folder/stb_truetype.h"
//#define IMGUI_STB_RECT_PACK_FILENAME  "my_folder/stb_rect_pack.h"
//#define IMGUI_STB_SPRINTF_FILENAME    "my_folder/stb_sprintf.h"    // only used if IMGUI_USE_STB_SPRINTF is defined.
//#define IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
//#define IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION
//#define IMGUI_DISABLE_STB_SPRINTF_IMPLEMENTATION                   // only disabled if IMGUI_USE_STB_SPRINTF is defined.

//---- Use stb_sprintf.h for a faster implementation of vsnprintf instead of the one from libc (unless IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS is defined)
// Compatibility checks of arguments and formats done by clang and GCC will be disabled in order to support the extra formats provided by stb_sprintf.h.
//#define IMGUI_USE_STB_SPRINTF

//---- Use FreeType to build and rasterize the font atlas (instead of stb_truetype which is embedded by default in Dear ImGui)
// Requires FreeType headers to be available in the include path. Requires program to be compiled with 'misc/freetype/imgui_freetype.cpp' (in this repository) + the FreeType library (not provided).
// On Windows you may use vcpkg with 'vcpkg install freetype --triplet=x64-windows' + 'vcpkg integrate install'.
//#define IMGUI_ENABLE_FREETYPE

//---- Use FreeType + plutosvg or lunasvg to render OpenType SVG fonts (SVGinOT)
// Only works in combination with IMGUI_ENABLE_FREETYPE.
// - lunasvg is currently easier to acquire/install, as e.g. it is part of vcpkg.
// - plutosvg will support more fonts and may load them faster. It currently requires to be built manually but it is fairly easy. See misc/freetype/README for instructions.
// - Both require headers to be available in the include path + program to be linked with the library code (not provided).
// - (note: lunasvg implementation is based on Freetype's rsvg-port.c which is licensed under CeCILL-C Free Software License Agreement)
//#define IMGUI_ENABLE_FREETYPE_PLUTOSVG
//#define IMGUI_ENABLE_FREETYPE_LUNASVG

//---- Use stb_truetype to build and rasterize the font atlas (default)
// The only purpose of this define is if you want force compilation of the stb_truetype backend ALONG with the FreeType backend.
//#define IMGUI_ENABLE_STB_TRUETYPE

//---- Define constructor and implicit cast operators to convert back<>forth between your math types and ImVec2/ImVec4.
// This will be inlined as part of ImVec2 and ImVec4 class declarations.
/*
#define IM_VEC2_CLASS_EXTRA                                                     \
        constexpr ImVec2(const MyVec2& f) : x(f.x), y(f.y) {}                   \
        operator MyVec2() const { return MyVec2(x,y); }

#define IM_VEC4_CLASS_EXTRA                                                     \
        constexpr ImVec4(const MyVec4& f) : x(f.x), y(f.y), z(f.z), w(f.w) {}   \
        operator MyVec4() const { return MyVec4(x,y,z,w); }
*/
//---- ...Or use Dear ImGui's own very basic math operators.
#define IMGUI_DEFINE_MATH_OPERATORS

//---- Use 32-bit vertex indices (default is 16-bit) is one way to allow large meshes with more than 64K vertices.
// Your renderer backend will need to support it (most example renderer backends support both 16/32-bit indices).
// Another way to allow large meshes while keeping 16-bit indices is to handle ImDrawCmd::VtxOffset in your renderer.
// Read about ImGuiBackendFlags_RendererHasVtxOffset for details.
//#define ImDrawIdx unsigned int

//---- Override ImDrawCallback signature (will need to modify renderer backends accordingly)
//struct ImDrawList;
//struct ImDrawCmd;
//typedef void (*MyImDrawCallback)(const ImDrawList* draw_list, const ImDrawCmd* cmd, void* my_renderer_user_data);
//#define ImDrawCallback MyImDrawCallback

//---- Debug Tools: Macro to break in Debugger (we provide a default implementation of this in the codebase)
// (use 'Metrics->Tools->Item Picker' to pick widgets with the mouse and break into them for easy debugging.)
#define IM_DEBUG_BREAK() MyBreak()

//---- Debug Tools: Enable slower asserts
//#define IMGUI_DEBUG_PARANOID

//---- Tip: You can add extra functions within the ImGui:: namespace from anywhere (e.g. your own sources/header files)
/*
namespace ImGui
{
    void MyFunction(const char* name, MyMatrix44* mtx);
}
*/

#endif //  _UI_IMCONFIG_H
