/*
File:   raylib_autocomplete_dict.h
Author: Taylor Robbins
Date:   01\06\2025
Description:
	** This file is NOT meant to be included! It's solely meant as a reference to help
	** fill out our autocomplete dictionary quickly with raylib functions\types\etc.
	** And as a place where the C syntax in Sublime can properly identify the function names
*/

#ifndef _RAYLIB_AUTOCOMPLETE_DICT_H
#define _RAYLIB_AUTOCOMPLETE_DICT_H

#define RAYLIB_VERSION_MAJOR
#define RAYLIB_VERSION_MINOR
#define RAYLIB_VERSION_PATCH
#define RAYLIB_VERSION
#define PI
#define DEG2RAD
#define RAD2DEG
#define RL_MALLOC(sz)
#define RL_CALLOC(n,sz)
#define RL_REALLOC(ptr,sz)
#define RL_FREE(ptr)
#define CLITERAL(type)
#define RL_COLOR_TYPE
#define RL_RECTANGLE_TYPE
#define RL_VECTOR2_TYPE
#define RL_VECTOR3_TYPE
#define RL_VECTOR4_TYPE
#define RL_QUATERNION_TYPE
#define RL_MATRIX_TYPE
#define LIGHTGRAY
#define GRAY
#define DARKGRAY
#define YELLOW
#define GOLD
#define ORANGE
#define PINK
#define RED
#define MAROON
#define GREEN
#define LIME
#define DARKGREEN
#define SKYBLUE
#define BLUE
#define DARKBLUE
#define PURPLE
#define VIOLET
#define DARKPURPLE
#define BEIGE
#define BROWN
#define DARKBROWN
#define WHITE
#define BLACK
#define BLANK
#define MAGENTA
#define RAYWHITE
#define RL_BOOL_TYPE

struct Vector2
{
    float x;
    float y;
};
struct Vector3
{
    float x;
    float y;
    float z;
};
struct Vector4
{
    float x;
    float y;
    float z;
    float w;
};
typedef Vector4 Quaternion;
struct Matrix
{
    float m0, m4, m8, m12;
    float m1, m5, m9, m13;
    float m2, m6, m10, m14;
    float m3, m7, m11, m15;
};
struct Color
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};
struct Rectangle
{
    float x;
    float y;
    float width;
    float height;
};
struct Image
{
    void *data;
    int width;
    int height;
    int mipmaps;
    int format;
};
struct Texture
{
    unsigned int id;
    int width;
    int height;
    int mipmaps;
    int format;
};
typedef Texture Texture2D;
typedef Texture TextureCubemap;
struct RenderTexture
{
    unsigned int id;
    Texture texture;
    Texture depth;
};
typedef RenderTexture RenderTexture2D;
struct NPatchInfo
{
    Rectangle source;
    int left;
    int top;
    int right;
    int bottom;
    int layout;
};
struct GlyphInfo
{
    int value;
    int offsetX;
    int offsetY;
    int advanceX;
    Image image;
};
struct Font
{
    int baseSize;
    int glyphCount;
    int glyphPadding;
    Texture2D texture;
    Rectangle *recs;
    GlyphInfo *glyphs;
};
struct Camera3D
{
    Vector3 position;
    Vector3 target;
    Vector3 up;
    float fovy;
    int projection;
};
typedef Camera3D Camera;
struct Camera2D
{
    Vector2 offset;
    Vector2 target;
    float rotation;
    float zoom;
};
struct Mesh
{
    int vertexCount;
    int triangleCount;
    // Vertex attributes data
    float *vertices;
    float *texcoords;
    float *texcoords2;
    float *normals;
    float *tangents;
    unsigned char *colors;
    unsigned short *indices;
    // Animation vertex data
    float *animVertices;
    float *animNormals;
    unsigned char *boneIds;
    float *boneWeights;
    Matrix *boneMatrices;
    int boneCount;
    // OpenGL identifiers
    unsigned int vaoId;
    unsigned int *vboId;
};
struct Shader
{
    unsigned int id;
    int *locs;
};
struct MaterialMap
{
    Texture2D texture;
    Color color;
    float value;
};
struct Material
{
    Shader shader;
    MaterialMap *maps;
    float params[4];
};
struct Transform
{
    Vector3 translation;
    Quaternion rotation;
    Vector3 scale;
};
struct BoneInfo
{
    char name[32];
    int parent;
};
struct Model
{
    Matrix transform;

    int meshCount;
    int materialCount;
    Mesh *meshes;
    Material *materials;
    int *meshMaterial;

    // Animation data
    int boneCount;
    BoneInfo *bones;
    Transform *bindPose;
};
struct ModelAnimation
{
    int boneCount;
    int frameCount;
    BoneInfo *bones;
    Transform **framePoses;
    char name[32]
};
struct Ray
{
    Vector3 position;
    Vector3 direction;
};
struct RayCollision
{
    bool hit;
    float distance;
    Vector3 point;
    Vector3 normal;
};
struct BoundingBox
{
    Vector3 min;
    Vector3 max;
};
struct Wave
{
    unsigned int frameCount;
    unsigned int sampleRate;
    unsigned int sampleSize;
    unsigned int channels;
    void *data;
};
typedef struct rAudioBuffer rAudioBuffer;
typedef struct rAudioProcessor rAudioProcessor;
struct AudioStream
{
    rAudioBuffer *buffer;
    rAudioProcessor *processor;

    unsigned int sampleRate;
    unsigned int sampleSize;
    unsigned int channels;
};
struct Sound
{
    AudioStream stream;
    unsigned int frameCount;
};
struct Music
{
    AudioStream stream;
    unsigned int frameCount;
    bool looping;

    int ctxType;
    void *ctxData;
};
struct VrDeviceInfo
{
    int hResolution;
    int vResolution;
    float hScreenSize;
    float vScreenSize;
    float eyeToScreenDistance;
    float lensSeparationDistance;
    float interpupillaryDistance;
    float lensDistortionValues[4];
    float chromaAbCorrection[4];
};
struct VrStereoConfig
{
    Matrix projection[2];
    Matrix viewOffset[2];
    float leftLensCenter[2];
    float rightLensCenter[2];
    float leftScreenCenter[2];
    float rightScreenCenter[2];
    float scale[2];
    float scaleIn[2];
};
struct FilePathList
{
    unsigned int capacity;
    unsigned int count;
    char **paths;
};
struct AutomationEvent
{
    unsigned int frame;
    unsigned int type;
    int params[4];
};
struct AutomationEventList
{
    unsigned int capacity;
    unsigned int count;
    AutomationEvent *events;
};

enum ConfigFlags
{
    FLAG_VSYNC_HINT         = 0x00000040,
    FLAG_FULLSCREEN_MODE    = 0x00000002,
    FLAG_WINDOW_RESIZABLE   = 0x00000004,
    FLAG_WINDOW_UNDECORATED = 0x00000008,
    FLAG_WINDOW_HIDDEN      = 0x00000080,
    FLAG_WINDOW_MINIMIZED   = 0x00000200,
    FLAG_WINDOW_MAXIMIZED   = 0x00000400,
    FLAG_WINDOW_UNFOCUSED   = 0x00000800,
    FLAG_WINDOW_TOPMOST     = 0x00001000,
    FLAG_WINDOW_ALWAYS_RUN  = 0x00000100,
    FLAG_WINDOW_TRANSPARENT = 0x00000010,
    FLAG_WINDOW_HIGHDPI     = 0x00002000,
    FLAG_WINDOW_MOUSE_PASSTHROUGH = 0x00004000,
    FLAG_BORDERLESS_WINDOWED_MODE = 0x00008000,
    FLAG_MSAA_4X_HINT       = 0x00000020,
    FLAG_INTERLACED_HINT    = 0x00010000
};
enum TraceLogLevel
{
    LOG_ALL = 0,
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL,
    LOG_NONE
};
enum KeyboardKey
{
    KEY_NULL            = 0,
    // Alphanumeric keys
    KEY_APOSTROPHE      = 39,
    KEY_COMMA           = 44,
    KEY_MINUS           = 45,
    KEY_PERIOD          = 46,
    KEY_SLASH           = 47,
    KEY_ZERO            = 48,
    KEY_ONE             = 49,
    KEY_TWO             = 50,
    KEY_THREE           = 51,
    KEY_FOUR            = 52,
    KEY_FIVE            = 53,
    KEY_SIX             = 54,
    KEY_SEVEN           = 55,
    KEY_EIGHT           = 56,
    KEY_NINE            = 57,
    KEY_SEMICOLON       = 59,
    KEY_EQUAL           = 61,
    KEY_A               = 65,
    KEY_B               = 66,
    KEY_C               = 67,
    KEY_D               = 68,
    KEY_E               = 69,
    KEY_F               = 70,
    KEY_G               = 71,
    KEY_H               = 72,
    KEY_I               = 73,
    KEY_J               = 74,
    KEY_K               = 75,
    KEY_L               = 76,
    KEY_M               = 77,
    KEY_N               = 78,
    KEY_O               = 79,
    KEY_P               = 80,
    KEY_Q               = 81,
    KEY_R               = 82,
    KEY_S               = 83,
    KEY_T               = 84,
    KEY_U               = 85,
    KEY_V               = 86,
    KEY_W               = 87,
    KEY_X               = 88,
    KEY_Y               = 89,
    KEY_Z               = 90,
    KEY_LEFT_BRACKET    = 91,
    KEY_BACKSLASH       = 92,
    KEY_RIGHT_BRACKET   = 93,
    KEY_GRAVE           = 96,
    // Function keys
    KEY_SPACE           = 32,
    KEY_ESCAPE          = 256,
    KEY_ENTER           = 257,
    KEY_TAB             = 258,
    KEY_BACKSPACE       = 259,
    KEY_INSERT          = 260,
    KEY_DELETE          = 261,
    KEY_RIGHT           = 262,
    KEY_LEFT            = 263,
    KEY_DOWN            = 264,
    KEY_UP              = 265,
    KEY_PAGE_UP         = 266,
    KEY_PAGE_DOWN       = 267,
    KEY_HOME            = 268,
    KEY_END             = 269,
    KEY_CAPS_LOCK       = 280,
    KEY_SCROLL_LOCK     = 281,
    KEY_NUM_LOCK        = 282,
    KEY_PRINT_SCREEN    = 283,
    KEY_PAUSE           = 284,
    KEY_F1              = 290,
    KEY_F2              = 291,
    KEY_F3              = 292,
    KEY_F4              = 293,
    KEY_F5              = 294,
    KEY_F6              = 295,
    KEY_F7              = 296,
    KEY_F8              = 297,
    KEY_F9              = 298,
    KEY_F10             = 299,
    KEY_F11             = 300,
    KEY_F12             = 301,
    KEY_LEFT_SHIFT      = 340,
    KEY_LEFT_CONTROL    = 341,
    KEY_LEFT_ALT        = 342,
    KEY_LEFT_SUPER      = 343,
    KEY_RIGHT_SHIFT     = 344,
    KEY_RIGHT_CONTROL   = 345,
    KEY_RIGHT_ALT       = 346,
    KEY_RIGHT_SUPER     = 347,
    KEY_KB_MENU         = 348,
    // Keypad keys
    KEY_KP_0            = 320,
    KEY_KP_1            = 321,
    KEY_KP_2            = 322,
    KEY_KP_3            = 323,
    KEY_KP_4            = 324,
    KEY_KP_5            = 325,
    KEY_KP_6            = 326,
    KEY_KP_7            = 327,
    KEY_KP_8            = 328,
    KEY_KP_9            = 329,
    KEY_KP_DECIMAL      = 330,
    KEY_KP_DIVIDE       = 331,
    KEY_KP_MULTIPLY     = 332,
    KEY_KP_SUBTRACT     = 333,
    KEY_KP_ADD          = 334,
    KEY_KP_ENTER        = 335,
    KEY_KP_EQUAL        = 336,
    // Android key buttons
    KEY_BACK            = 4,
    KEY_MENU            = 5,
    KEY_VOLUME_UP       = 24,
    KEY_VOLUME_DOWN     = 25
};
#define MOUSE_LEFT_BUTTON   MOUSE_BUTTON_LEFT
#define MOUSE_RIGHT_BUTTON  MOUSE_BUTTON_RIGHT
#define MOUSE_MIDDLE_BUTTON MOUSE_BUTTON_MIDDLE
enum MouseButton
{
    MOUSE_BUTTON_LEFT    = 0,
    MOUSE_BUTTON_RIGHT   = 1,
    MOUSE_BUTTON_MIDDLE  = 2,
    MOUSE_BUTTON_SIDE    = 3,
    MOUSE_BUTTON_EXTRA   = 4,
    MOUSE_BUTTON_FORWARD = 5,
    MOUSE_BUTTON_BACK    = 6,
};
enum MouseCursor
{
    MOUSE_CURSOR_DEFAULT       = 0,
    MOUSE_CURSOR_ARROW         = 1,
    MOUSE_CURSOR_IBEAM         = 2,
    MOUSE_CURSOR_CROSSHAIR     = 3,
    MOUSE_CURSOR_POINTING_HAND = 4,
    MOUSE_CURSOR_RESIZE_EW     = 5,
    MOUSE_CURSOR_RESIZE_NS     = 6,
    MOUSE_CURSOR_RESIZE_NWSE   = 7,
    MOUSE_CURSOR_RESIZE_NESW   = 8,
    MOUSE_CURSOR_RESIZE_ALL    = 9,
    MOUSE_CURSOR_NOT_ALLOWED   = 10
};
enum GamepadButton
{
    GAMEPAD_BUTTON_UNKNOWN = 0,
    GAMEPAD_BUTTON_LEFT_FACE_UP,
    GAMEPAD_BUTTON_LEFT_FACE_RIGHT,
    GAMEPAD_BUTTON_LEFT_FACE_DOWN,
    GAMEPAD_BUTTON_LEFT_FACE_LEFT,
    GAMEPAD_BUTTON_RIGHT_FACE_UP,
    GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,
    GAMEPAD_BUTTON_RIGHT_FACE_DOWN,
    GAMEPAD_BUTTON_RIGHT_FACE_LEFT,
    GAMEPAD_BUTTON_LEFT_TRIGGER_1,
    GAMEPAD_BUTTON_LEFT_TRIGGER_2,
    GAMEPAD_BUTTON_RIGHT_TRIGGER_1,
    GAMEPAD_BUTTON_RIGHT_TRIGGER_2,
    GAMEPAD_BUTTON_MIDDLE_LEFT,
    GAMEPAD_BUTTON_MIDDLE,
    GAMEPAD_BUTTON_MIDDLE_RIGHT,
    GAMEPAD_BUTTON_LEFT_THUMB,
    GAMEPAD_BUTTON_RIGHT_THUMB 
};
enum GamepadAxis
{
    GAMEPAD_AXIS_LEFT_X        = 0,
    GAMEPAD_AXIS_LEFT_Y        = 1,
    GAMEPAD_AXIS_RIGHT_X       = 2,
    GAMEPAD_AXIS_RIGHT_Y       = 3,
    GAMEPAD_AXIS_LEFT_TRIGGER  = 4,
    GAMEPAD_AXIS_RIGHT_TRIGGER = 5
};
enum MaterialMapIndex
{
    MATERIAL_MAP_ALBEDO = 0,
    MATERIAL_MAP_METALNESS,
    MATERIAL_MAP_NORMAL,
    MATERIAL_MAP_ROUGHNESS,
    MATERIAL_MAP_OCCLUSION,
    MATERIAL_MAP_EMISSION,
    MATERIAL_MAP_HEIGHT,
    MATERIAL_MAP_CUBEMAP,
    MATERIAL_MAP_IRRADIANCE,
    MATERIAL_MAP_PREFILTER,
    MATERIAL_MAP_BRDF
};
#define MATERIAL_MAP_DIFFUSE      MATERIAL_MAP_ALBEDO
#define MATERIAL_MAP_SPECULAR     MATERIAL_MAP_METALNESS
enum ShaderLocationIndex
{
    SHADER_LOC_VERTEX_POSITION = 0,
    SHADER_LOC_VERTEX_TEXCOORD01,
    SHADER_LOC_VERTEX_TEXCOORD02,
    SHADER_LOC_VERTEX_NORMAL,
    SHADER_LOC_VERTEX_TANGENT,
    SHADER_LOC_VERTEX_COLOR,
    SHADER_LOC_MATRIX_MVP,
    SHADER_LOC_MATRIX_VIEW,
    SHADER_LOC_MATRIX_PROJECTION,
    SHADER_LOC_MATRIX_MODEL,
    SHADER_LOC_MATRIX_NORMAL,
    SHADER_LOC_VECTOR_VIEW,
    SHADER_LOC_COLOR_DIFFUSE,
    SHADER_LOC_COLOR_SPECULAR,
    SHADER_LOC_COLOR_AMBIENT,
    SHADER_LOC_MAP_ALBEDO,
    SHADER_LOC_MAP_METALNESS,
    SHADER_LOC_MAP_NORMAL,
    SHADER_LOC_MAP_ROUGHNESS,
    SHADER_LOC_MAP_OCCLUSION,
    SHADER_LOC_MAP_EMISSION,
    SHADER_LOC_MAP_HEIGHT,
    SHADER_LOC_MAP_CUBEMAP,
    SHADER_LOC_MAP_IRRADIANCE,
    SHADER_LOC_MAP_PREFILTER,
    SHADER_LOC_MAP_BRDF,
    SHADER_LOC_VERTEX_BONEIDS,
    SHADER_LOC_VERTEX_BONEWEIGHTS,
    SHADER_LOC_BONE_MATRICES
};
#define SHADER_LOC_MAP_DIFFUSE      SHADER_LOC_MAP_ALBEDO
#define SHADER_LOC_MAP_SPECULAR     SHADER_LOC_MAP_METALNESS
enum ShaderUniformDataType
{
    SHADER_UNIFORM_FLOAT = 0,
    SHADER_UNIFORM_VEC2,
    SHADER_UNIFORM_VEC3,
    SHADER_UNIFORM_VEC4,
    SHADER_UNIFORM_INT,
    SHADER_UNIFORM_IVEC2,
    SHADER_UNIFORM_IVEC3,
    SHADER_UNIFORM_IVEC4,
    SHADER_UNIFORM_SAMPLER2D
};
enum ShaderAttributeDataType
{
    SHADER_ATTRIB_FLOAT = 0,
    SHADER_ATTRIB_VEC2,
    SHADER_ATTRIB_VEC3,
    SHADER_ATTRIB_VEC4
};
enum PixelFormat
{
    PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1,
    PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,
    PIXELFORMAT_UNCOMPRESSED_R5G6B5,
    PIXELFORMAT_UNCOMPRESSED_R8G8B8,
    PIXELFORMAT_UNCOMPRESSED_R5G5B5A1,
    PIXELFORMAT_UNCOMPRESSED_R4G4B4A4,
    PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    PIXELFORMAT_UNCOMPRESSED_R32,
    PIXELFORMAT_UNCOMPRESSED_R32G32B32,
    PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,
    PIXELFORMAT_UNCOMPRESSED_R16,
    PIXELFORMAT_UNCOMPRESSED_R16G16B16,
    PIXELFORMAT_UNCOMPRESSED_R16G16B16A16,
    PIXELFORMAT_COMPRESSED_DXT1_RGB,
    PIXELFORMAT_COMPRESSED_DXT1_RGBA,
    PIXELFORMAT_COMPRESSED_DXT3_RGBA,
    PIXELFORMAT_COMPRESSED_DXT5_RGBA,
    PIXELFORMAT_COMPRESSED_ETC1_RGB,
    PIXELFORMAT_COMPRESSED_ETC2_RGB,
    PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA,
    PIXELFORMAT_COMPRESSED_PVRT_RGB,
    PIXELFORMAT_COMPRESSED_PVRT_RGBA,
    PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA,
    PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA
};
enum TextureFilter
{
    TEXTURE_FILTER_POINT = 0,
    TEXTURE_FILTER_BILINEAR,
    TEXTURE_FILTER_TRILINEAR,
    TEXTURE_FILTER_ANISOTROPIC_4X,
    TEXTURE_FILTER_ANISOTROPIC_8X,
    TEXTURE_FILTER_ANISOTROPIC_16X,
};
enum TextureWrap
{
    TEXTURE_WRAP_REPEAT = 0,
    TEXTURE_WRAP_CLAMP,
    TEXTURE_WRAP_MIRROR_REPEAT,
    TEXTURE_WRAP_MIRROR_CLAMP
};
enum CubemapLayout
{
    CUBEMAP_LAYOUT_AUTO_DETECT = 0,
    CUBEMAP_LAYOUT_LINE_VERTICAL,
    CUBEMAP_LAYOUT_LINE_HORIZONTAL,
    CUBEMAP_LAYOUT_CROSS_THREE_BY_FOUR,
    CUBEMAP_LAYOUT_CROSS_FOUR_BY_THREE
};
enum FontType
{
    FONT_DEFAULT = 0,
    FONT_BITMAP,
    FONT_SDF
};
enum BlendMode
{
    BLEND_ALPHA = 0,
    BLEND_ADDITIVE,
    BLEND_MULTIPLIED,
    BLEND_ADD_COLORS,
    BLEND_SUBTRACT_COLORS,
    BLEND_ALPHA_PREMULTIPLY,
    BLEND_CUSTOM,
    BLEND_CUSTOM_SEPARATE
};
enum Gesture
{
    GESTURE_NONE        = 0,
    GESTURE_TAP         = 1,
    GESTURE_DOUBLETAP   = 2,
    GESTURE_HOLD        = 4,
    GESTURE_DRAG        = 8,
    GESTURE_SWIPE_RIGHT = 16,
    GESTURE_SWIPE_LEFT  = 32,
    GESTURE_SWIPE_UP    = 64,
    GESTURE_SWIPE_DOWN  = 128,
    GESTURE_PINCH_IN    = 256,
    GESTURE_PINCH_OUT   = 512
};
enum CameraMode
{
    CAMERA_CUSTOM = 0,
    CAMERA_FREE,
    CAMERA_ORBITAL,
    CAMERA_FIRST_PERSON,
    CAMERA_THIRD_PERSON
};
enum CameraProjection
{
    CAMERA_PERSPECTIVE = 0,
    CAMERA_ORTHOGRAPHIC
};
enum NPatchLayout
{
    NPATCH_NINE_PATCH = 0,
    NPATCH_THREE_PATCH_VERTICAL,
    NPATCH_THREE_PATCH_HORIZONTAL
};
typedef void (*TraceLogCallback)(int logLevel, const char *text, va_list args);
typedef unsigned char *(*LoadFileDataCallback)(const char *fileName, int *dataSize);
typedef bool (*SaveFileDataCallback)(const char *fileName, void *data, int dataSize);
typedef char *(*LoadFileTextCallback)(const char *fileName);
typedef bool (*SaveFileTextCallback)(const char *fileName, char *text);
// Window-related functions
void InitWindow(int width, int height, const char *title);  // Initialize window and OpenGL context
void CloseWindow(void);                                     // Close window and unload OpenGL context
bool WindowShouldClose(void);                               // Check if application should close (KEY_ESCAPE pressed or windows close icon clicked)
bool IsWindowReady(void);                                   // Check if window has been initialized successfully
bool IsWindowFullscreen(void);                              // Check if window is currently fullscreen
bool IsWindowHidden(void);                                  // Check if window is currently hidden
bool IsWindowMinimized(void);                               // Check if window is currently minimized
bool IsWindowMaximized(void);                               // Check if window is currently maximized
bool IsWindowFocused(void);                                 // Check if window is currently focused
bool IsWindowResized(void);                                 // Check if window has been resized last frame
bool IsWindowState(unsigned int flag);                      // Check if one specific window flag is enabled
void SetWindowState(unsigned int flags);                    // Set window configuration state using flags
void ClearWindowState(unsigned int flags);                  // Clear window configuration state flags
void ToggleFullscreen(void);                                // Toggle window state: fullscreen/windowed, resizes monitor to match window resolution
void ToggleBorderlessWindowed(void);                        // Toggle window state: borderless windowed, resizes window to match monitor resolution
void MaximizeWindow(void);                                  // Set window state: maximized, if resizable
void MinimizeWindow(void);                                  // Set window state: minimized, if resizable
void RestoreWindow(void);                                   // Set window state: not minimized/maximized
void SetWindowIcon(Image image);                            // Set icon for window (single image, RGBA 32bit)
void SetWindowIcons(Image *images, int count);              // Set icon for window (multiple images, RGBA 32bit)
void SetWindowTitle(const char *title);                     // Set title for window
void SetWindowPosition(int x, int y);                       // Set window position on screen
void SetWindowMonitor(int monitor);                         // Set monitor for the current window
void SetWindowMinSize(int width, int height);               // Set window minimum dimensions (for FLAG_WINDOW_RESIZABLE)
void SetWindowMaxSize(int width, int height);               // Set window maximum dimensions (for FLAG_WINDOW_RESIZABLE)
void SetWindowSize(int width, int height);                  // Set window dimensions
void SetWindowOpacity(float opacity);                       // Set window opacity [0.0f..1.0f]
void SetWindowFocused(void);                                // Set window focused
void *GetWindowHandle(void);                                // Get native window handle
int GetScreenWidth(void);                                   // Get current screen width
int GetScreenHeight(void);                                  // Get current screen height
int GetRenderWidth(void);                                   // Get current render width (it considers HiDPI)
int GetRenderHeight(void);                                  // Get current render height (it considers HiDPI)
int GetMonitorCount(void);                                  // Get number of connected monitors
int GetCurrentMonitor(void);                                // Get current monitor where window is placed
Vector2 GetMonitorPosition(int monitor);                    // Get specified monitor position
int GetMonitorWidth(int monitor);                           // Get specified monitor width (current video mode used by monitor)
int GetMonitorHeight(int monitor);                          // Get specified monitor height (current video mode used by monitor)
int GetMonitorPhysicalWidth(int monitor);                   // Get specified monitor physical width in millimetres
int GetMonitorPhysicalHeight(int monitor);                  // Get specified monitor physical height in millimetres
int GetMonitorRefreshRate(int monitor);                     // Get specified monitor refresh rate
Vector2 GetWindowPosition(void);                            // Get window position XY on monitor
Vector2 GetWindowScaleDPI(void);                            // Get window scale DPI factor
const char *GetMonitorName(int monitor);                    // Get the human-readable, UTF-8 encoded name of the specified monitor
void SetClipboardText(const char *text);                    // Set clipboard text content
const char *GetClipboardText(void);                         // Get clipboard text content
Image GetClipboardImage(void);                              // Get clipboard image content
void EnableEventWaiting(void);                              // Enable waiting for events on EndDrawing(), no automatic event polling
void DisableEventWaiting(void);                             // Disable waiting for events on EndDrawing(), automatic events polling
// Cursor-related functions
void ShowCursor(void);                                      // Shows cursor
void HideCursor(void);                                      // Hides cursor
bool IsCursorHidden(void);                                  // Check if cursor is not visible
void EnableCursor(void);                                    // Enables cursor (unlock cursor)
void DisableCursor(void);                                   // Disables cursor (lock cursor)
bool IsCursorOnScreen(void);                                // Check if cursor is on the screen
// Drawing-related functions
void ClearBackground(Color color);                          // Set background color (framebuffer clear color)
void BeginDrawing(void);                                    // Setup canvas (framebuffer) to start drawing
void EndDrawing(void);                                      // End canvas drawing and swap buffers (double buffering)
void BeginMode2D(Camera2D camera);                          // Begin 2D mode with custom camera (2D)
void EndMode2D(void);                                       // Ends 2D mode with custom camera
void BeginMode3D(Camera3D camera);                          // Begin 3D mode with custom camera (3D)
void EndMode3D(void);                                       // Ends 3D mode and returns to default 2D orthographic mode
void BeginTextureMode(RenderTexture2D target);              // Begin drawing to render texture
void EndTextureMode(void);                                  // Ends drawing to render texture
void BeginShaderMode(Shader shader);                        // Begin custom shader drawing
void EndShaderMode(void);                                   // End custom shader drawing (use default shader)
void BeginBlendMode(int mode);                              // Begin blending mode (alpha, additive, multiplied, subtract, custom)
void EndBlendMode(void);                                    // End blending mode (reset to default: alpha blending)
void BeginScissorMode(int x, int y, int width, int height); // Begin scissor mode (define screen area for following drawing)
void EndScissorMode(void);                                  // End scissor mode
void BeginVrStereoMode(VrStereoConfig config);              // Begin stereo rendering (requires VR simulator)
void EndVrStereoMode(void);                                 // End stereo rendering (requires VR simulator)
// VR stereo config functions for VR simulator
VrStereoConfig LoadVrStereoConfig(VrDeviceInfo device);     // Load VR stereo config for VR simulator device parameters
void UnloadVrStereoConfig(VrStereoConfig config);           // Unload VR stereo config
// Shader management functions
// NOTE: Shader functionality is not available on OpenGL 1.1
Shader LoadShader(const char *vsFileName, const char *fsFileName);   // Load shader from files and bind default locations
Shader LoadShaderFromMemory(const char *vsCode, const char *fsCode); // Load shader from code strings and bind default locations
bool IsShaderValid(Shader shader);                                   // Check if a shader is valid (loaded on GPU)
int GetShaderLocation(Shader shader, const char *uniformName);       // Get shader uniform location
int GetShaderLocationAttrib(Shader shader, const char *attribName);  // Get shader attribute location
void SetShaderValue(Shader shader, int locIndex, const void *value, int uniformType);               // Set shader uniform value
void SetShaderValueV(Shader shader, int locIndex, const void *value, int uniformType, int count);   // Set shader uniform value vector
void SetShaderValueMatrix(Shader shader, int locIndex, Matrix mat);         // Set shader uniform value (matrix 4x4)
void SetShaderValueTexture(Shader shader, int locIndex, Texture2D texture); // Set shader uniform value for texture (sampler2d)
void UnloadShader(Shader shader);                                    // Unload shader from GPU memory (VRAM)
// Screen-space-related functions
#define GetMouseRay GetScreenToWorldRay     // Compatibility hack for previous raylib versions
Ray GetScreenToWorldRay(Vector2 position, Camera camera);         // Get a ray trace from screen position (i.e mouse)
Ray GetScreenToWorldRayEx(Vector2 position, Camera camera, int width, int height); // Get a ray trace from screen position (i.e mouse) in a viewport
Vector2 GetWorldToScreen(Vector3 position, Camera camera);        // Get the screen space position for a 3d world space position
Vector2 GetWorldToScreenEx(Vector3 position, Camera camera, int width, int height); // Get size position for a 3d world space position
Vector2 GetWorldToScreen2D(Vector2 position, Camera2D camera);    // Get the screen space position for a 2d camera world space position
Vector2 GetScreenToWorld2D(Vector2 position, Camera2D camera);    // Get the world space position for a 2d camera screen space position
Matrix GetCameraMatrix(Camera camera);                            // Get camera transform matrix (view matrix)
Matrix GetCameraMatrix2D(Camera2D camera);                        // Get camera 2d transform matrix
// Timing-related functions
void SetTargetFPS(int fps);                                 // Set target FPS (maximum)
float GetFrameTime(void);                                   // Get time in seconds for last frame drawn (delta time)
double GetTime(void);                                       // Get elapsed time in seconds since InitWindow()
int GetFPS(void);                                           // Get current FPS
// Custom frame control functions
// NOTE: Those functions are intended for advanced users that want full control over the frame processing
// By default EndDrawing() does this job: draws everything + SwapScreenBuffer() + manage frame timing + PollInputEvents()
// To avoid that behaviour and control frame processes manually, enable in config.h: SUPPORT_CUSTOM_FRAME_CONTROL
void SwapScreenBuffer(void);                                // Swap back buffer with front buffer (screen drawing)
void PollInputEvents(void);                                 // Register all input events
void WaitTime(double seconds);                              // Wait for some time (halt program execution)
// Random values generation functions
void SetRandomSeed(unsigned int seed);                      // Set the seed for the random number generator
int GetRandomValue(int min, int max);                       // Get a random value between min and max (both included)
int *LoadRandomSequence(unsigned int count, int min, int max); // Load random values sequence, no values repeated
void UnloadRandomSequence(int *sequence);                   // Unload random values sequence
// Misc. functions
void TakeScreenshot(const char *fileName);                  // Takes a screenshot of current screen (filename extension defines format)
void SetConfigFlags(unsigned int flags);                    // Setup init configuration flags (view FLAGS)
void OpenURL(const char *url);                              // Open URL with default system browser (if available)
// module util functions
void TraceLog(int logLevel, const char *text, ...);         // Show trace log messages (LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR...)
void SetTraceLogLevel(int logLevel);                        // Set the current threshold (minimum) log level
void *MemAlloc(unsigned int size);                          // Internal memory allocator
void *MemRealloc(void *ptr, unsigned int size);             // Internal memory reallocator
void MemFree(void *ptr);                                    // Internal memory free
// Set custom callbacks
// WARNING: Callbacks setup is intended for advanced users
void SetTraceLogCallback(TraceLogCallback callback);         // Set custom trace log
void SetLoadFileDataCallback(LoadFileDataCallback callback); // Set custom file binary data loader
void SetSaveFileDataCallback(SaveFileDataCallback callback); // Set custom file binary data saver
void SetLoadFileTextCallback(LoadFileTextCallback callback); // Set custom file text data loader
void SetSaveFileTextCallback(SaveFileTextCallback callback); // Set custom file text data saver
// Files management functions
unsigned char *LoadFileData(const char *fileName, int *dataSize); // Load file data as byte array (read)
void UnloadFileData(unsigned char *data);                   // Unload file data allocated by LoadFileData()
bool SaveFileData(const char *fileName, void *data, int dataSize); // Save data to file from byte array (write), returns true on success
bool ExportDataAsCode(const unsigned char *data, int dataSize, const char *fileName); // Export data to code (.h), returns true on success
char *LoadFileText(const char *fileName);                   // Load text data from file (read), returns a '\0' terminated string
void UnloadFileText(char *text);                            // Unload file text data allocated by LoadFileText()
bool SaveFileText(const char *fileName, char *text);        // Save text data to file (write), string must be '\0' terminated, returns true on success
// File system functions
bool FileExists(const char *fileName);                      // Check if file exists
bool DirectoryExists(const char *dirPath);                  // Check if a directory path exists
bool IsFileExtension(const char *fileName, const char *ext); // Check file extension (including point: .png, .wav)
int GetFileLength(const char *fileName);                    // Get file length in bytes (NOTE: GetFileSize() conflicts with windows.h)
const char *GetFileExtension(const char *fileName);         // Get pointer to extension for a filename string (includes dot: '.png')
const char *GetFileName(const char *filePath);              // Get pointer to filename for a path string
const char *GetFileNameWithoutExt(const char *filePath);    // Get filename string without extension (uses static string)
const char *GetDirectoryPath(const char *filePath);         // Get full path for a given fileName with path (uses static string)
const char *GetPrevDirectoryPath(const char *dirPath);      // Get previous directory path for a given path (uses static string)
const char *GetWorkingDirectory(void);                      // Get current working directory (uses static string)
const char *GetApplicationDirectory(void);                  // Get the directory of the running application (uses static string)
int MakeDirectory(const char *dirPath);                     // Create directories (including full path requested), returns 0 on success
bool ChangeDirectory(const char *dir);                      // Change working directory, return true on success
bool IsPathFile(const char *path);                          // Check if a given path is a file or a directory
bool IsFileNameValid(const char *fileName);                 // Check if fileName is valid for the platform/OS
FilePathList LoadDirectoryFiles(const char *dirPath);       // Load directory filepaths
FilePathList LoadDirectoryFilesEx(const char *basePath, const char *filter, bool scanSubdirs); // Load directory filepaths with extension filtering and recursive directory scan. Use 'DIR' in the filter string to include directories in the result
void UnloadDirectoryFiles(FilePathList files);              // Unload filepaths
bool IsFileDropped(void);                                   // Check if a file has been dropped into window
FilePathList LoadDroppedFiles(void);                        // Load dropped filepaths
void UnloadDroppedFiles(FilePathList files);                // Unload dropped filepaths
long GetFileModTime(const char *fileName);                  // Get file modification time (last write time)
// Compression/Encoding functionality
unsigned char *CompressData(const unsigned char *data, int dataSize, int *compDataSize);        // Compress data (DEFLATE algorithm), memory must be MemFree()
unsigned char *DecompressData(const unsigned char *compData, int compDataSize, int *dataSize);  // Decompress data (DEFLATE algorithm), memory must be MemFree()
char *EncodeDataBase64(const unsigned char *data, int dataSize, int *outputSize);               // Encode data to Base64 string, memory must be MemFree()
unsigned char *DecodeDataBase64(const unsigned char *data, int *outputSize);                    // Decode Base64 string data, memory must be MemFree()
unsigned int ComputeCRC32(unsigned char *data, int dataSize);     // Compute CRC32 hash code
unsigned int *ComputeMD5(unsigned char *data, int dataSize);      // Compute MD5 hash code, returns static int[4] (16 bytes)
unsigned int *ComputeSHA1(unsigned char *data, int dataSize);      // Compute SHA1 hash code, returns static int[5] (20 bytes)
// Automation events functionality
AutomationEventList LoadAutomationEventList(const char *fileName);                // Load automation events list from file, NULL for empty list, capacity = MAX_AUTOMATION_EVENTS
void UnloadAutomationEventList(AutomationEventList list);                         // Unload automation events list from file
bool ExportAutomationEventList(AutomationEventList list, const char *fileName);   // Export automation events list as text file
void SetAutomationEventList(AutomationEventList *list);                           // Set automation event list to record to
void SetAutomationEventBaseFrame(int frame);                                      // Set automation event internal base frame to start recording
void StartAutomationEventRecording(void);                                         // Start recording automation events (AutomationEventList must be set)
void StopAutomationEventRecording(void);                                          // Stop recording automation events
void PlayAutomationEvent(AutomationEvent event);                                  // Play a recorded automation event
// Input-related functions: keyboard
bool IsKeyPressed(int key);                             // Check if a key has been pressed once
bool IsKeyPressedRepeat(int key);                       // Check if a key has been pressed again
bool IsKeyDown(int key);                                // Check if a key is being pressed
bool IsKeyReleased(int key);                            // Check if a key has been released once
bool IsKeyUp(int key);                                  // Check if a key is NOT being pressed
int GetKeyPressed(void);                                // Get key pressed (keycode), call it multiple times for keys queued, returns 0 when the queue is empty
int GetCharPressed(void);                               // Get char pressed (unicode), call it multiple times for chars queued, returns 0 when the queue is empty
void SetExitKey(int key);                               // Set a custom key to exit program (default is ESC)
// Input-related functions: gamepads
bool IsGamepadAvailable(int gamepad);                                        // Check if a gamepad is available
const char *GetGamepadName(int gamepad);                                     // Get gamepad internal name id
bool IsGamepadButtonPressed(int gamepad, int button);                        // Check if a gamepad button has been pressed once
bool IsGamepadButtonDown(int gamepad, int button);                           // Check if a gamepad button is being pressed
bool IsGamepadButtonReleased(int gamepad, int button);                       // Check if a gamepad button has been released once
bool IsGamepadButtonUp(int gamepad, int button);                             // Check if a gamepad button is NOT being pressed
int GetGamepadButtonPressed(void);                                           // Get the last gamepad button pressed
int GetGamepadAxisCount(int gamepad);                                        // Get gamepad axis count for a gamepad
float GetGamepadAxisMovement(int gamepad, int axis);                         // Get axis movement value for a gamepad axis
int SetGamepadMappings(const char *mappings);                                // Set internal gamepad mappings (SDL_GameControllerDB)
void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor, float duration); // Set gamepad vibration for both motors (duration in seconds)
// Input-related functions: mouse
bool IsMouseButtonPressed(int button);                  // Check if a mouse button has been pressed once
bool IsMouseButtonDown(int button);                     // Check if a mouse button is being pressed
bool IsMouseButtonReleased(int button);                 // Check if a mouse button has been released once
bool IsMouseButtonUp(int button);                       // Check if a mouse button is NOT being pressed
int GetMouseX(void);                                    // Get mouse position X
int GetMouseY(void);                                    // Get mouse position Y
Vector2 GetMousePosition(void);                         // Get mouse position XY
Vector2 GetMouseDelta(void);                            // Get mouse delta between frames
void SetMousePosition(int x, int y);                    // Set mouse position XY
void SetMouseOffset(int offsetX, int offsetY);          // Set mouse offset
void SetMouseScale(float scaleX, float scaleY);         // Set mouse scaling
float GetMouseWheelMove(void);                          // Get mouse wheel movement for X or Y, whichever is larger
Vector2 GetMouseWheelMoveV(void);                       // Get mouse wheel movement for both X and Y
void SetMouseCursor(int cursor);                        // Set mouse cursor
// Input-related functions: touch
int GetTouchX(void);                                    // Get touch position X for touch point 0 (relative to screen size)
int GetTouchY(void);                                    // Get touch position Y for touch point 0 (relative to screen size)
Vector2 GetTouchPosition(int index);                    // Get touch position XY for a touch point index (relative to screen size)
int GetTouchPointId(int index);                         // Get touch point identifier for given index
int GetTouchPointCount(void);                           // Get number of touch points
//Input-related functions: Gestures
void SetGesturesEnabled(unsigned int flags);      // Enable a set of gestures using flags
bool IsGestureDetected(unsigned int gesture);     // Check if a gesture have been detected
int GetGestureDetected(void);                     // Get latest detected gesture
float GetGestureHoldDuration(void);               // Get gesture hold time in seconds
Vector2 GetGestureDragVector(void);               // Get gesture drag vector
float GetGestureDragAngle(void);                  // Get gesture drag angle
Vector2 GetGesturePinchVector(void);              // Get gesture pinch delta
float GetGesturePinchAngle(void);                 // Get gesture pinch angle
// Camera System Functions (Module: rcamera)
void UpdateCamera(Camera *camera, int mode);      // Update camera position for selected mode
void UpdateCameraPro(Camera *camera, Vector3 movement, Vector3 rotation, float zoom); // Update camera movement/rotation
// Basic Shapes Drawing Functions (Module: shapes)
// Set texture and rectangle to be used on shapes drawing
// NOTE: It can be useful when using basic shapes and one single font,
// defining a font char white rectangle would allow drawing everything in a single draw call
void SetShapesTexture(Texture2D texture, Rectangle source);       // Set texture and rectangle to be used on shapes drawing
Texture2D GetShapesTexture(void);                                 // Get texture that is used for shapes drawing
Rectangle GetShapesTextureRectangle(void);                        // Get texture source rectangle that is used for shapes drawing
// Basic shapes drawing functions
void DrawPixel(int posX, int posY, Color color);                                                   // Draw a pixel using geometry [Can be slow, use with care]
void DrawPixelV(Vector2 position, Color color);                                                    // Draw a pixel using geometry (Vector version) [Can be slow, use with care]
void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color);                // Draw a line
void DrawLineV(Vector2 startPos, Vector2 endPos, Color color);                                     // Draw a line (using gl lines)
void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color);                       // Draw a line (using triangles/quads)
void DrawLineStrip(const Vector2 *points, int pointCount, Color color);                            // Draw lines sequence (using gl lines)
void DrawLineBezier(Vector2 startPos, Vector2 endPos, float thick, Color color);                   // Draw line segment cubic-bezier in-out interpolation
void DrawCircle(int centerX, int centerY, float radius, Color color);                              // Draw a color-filled circle
void DrawCircleSector(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color);      // Draw a piece of a circle
void DrawCircleSectorLines(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color); // Draw circle sector outline
void DrawCircleGradient(int centerX, int centerY, float radius, Color inner, Color outer);         // Draw a gradient-filled circle
void DrawCircleV(Vector2 center, float radius, Color color);                                       // Draw a color-filled circle (Vector version)
void DrawCircleLines(int centerX, int centerY, float radius, Color color);                         // Draw circle outline
void DrawCircleLinesV(Vector2 center, float radius, Color color);                                  // Draw circle outline (Vector version)
void DrawEllipse(int centerX, int centerY, float radiusH, float radiusV, Color color);             // Draw ellipse
void DrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV, Color color);        // Draw ellipse outline
void DrawRing(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color); // Draw ring
void DrawRingLines(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color);    // Draw ring outline
void DrawRectangle(int posX, int posY, int width, int height, Color color);                        // Draw a color-filled rectangle
void DrawRectangleV(Vector2 position, Vector2 size, Color color);                                  // Draw a color-filled rectangle (Vector version)
void DrawRectangleRec(Rectangle rec, Color color);                                                 // Draw a color-filled rectangle
void DrawRectanglePro(Rectangle rec, Vector2 origin, float rotation, Color color);                 // Draw a color-filled rectangle with pro parameters
void DrawRectangleGradientV(int posX, int posY, int width, int height, Color top, Color bottom);   // Draw a vertical-gradient-filled rectangle
void DrawRectangleGradientH(int posX, int posY, int width, int height, Color left, Color right);   // Draw a horizontal-gradient-filled rectangle
void DrawRectangleGradientEx(Rectangle rec, Color topLeft, Color bottomLeft, Color topRight, Color bottomRight); // Draw a gradient-filled rectangle with custom vertex colors
void DrawRectangleLines(int posX, int posY, int width, int height, Color color);                   // Draw rectangle outline
void DrawRectangleLinesEx(Rectangle rec, float lineThick, Color color);                            // Draw rectangle outline with extended parameters
void DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color);              // Draw rectangle with rounded edges
void DrawRectangleRoundedLines(Rectangle rec, float roundness, int segments, Color color);         // Draw rectangle lines with rounded edges
void DrawRectangleRoundedLinesEx(Rectangle rec, float roundness, int segments, float lineThick, Color color); // Draw rectangle with rounded edges outline
void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color);                                // Draw a color-filled triangle (vertex in counter-clockwise order!)
void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color);                           // Draw triangle outline (vertex in counter-clockwise order!)
void DrawTriangleFan(const Vector2 *points, int pointCount, Color color);                          // Draw a triangle fan defined by points (first vertex is the center)
void DrawTriangleStrip(const Vector2 *points, int pointCount, Color color);                        // Draw a triangle strip defined by points
void DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color);               // Draw a regular polygon (Vector version)
void DrawPolyLines(Vector2 center, int sides, float radius, float rotation, Color color);          // Draw a polygon outline of n sides
void DrawPolyLinesEx(Vector2 center, int sides, float radius, float rotation, float lineThick, Color color); // Draw a polygon outline of n sides with extended parameters
// Splines drawing functions
void DrawSplineLinear(const Vector2 *points, int pointCount, float thick, Color color);                  // Draw spline: Linear, minimum 2 points
void DrawSplineBasis(const Vector2 *points, int pointCount, float thick, Color color);                   // Draw spline: B-Spline, minimum 4 points
void DrawSplineCatmullRom(const Vector2 *points, int pointCount, float thick, Color color);              // Draw spline: Catmull-Rom, minimum 4 points
void DrawSplineBezierQuadratic(const Vector2 *points, int pointCount, float thick, Color color);         // Draw spline: Quadratic Bezier, minimum 3 points (1 control point): [p1, c2, p3, c4...]
void DrawSplineBezierCubic(const Vector2 *points, int pointCount, float thick, Color color);             // Draw spline: Cubic Bezier, minimum 4 points (2 control points): [p1, c2, c3, p4, c5, c6...]
void DrawSplineSegmentLinear(Vector2 p1, Vector2 p2, float thick, Color color);                    // Draw spline segment: Linear, 2 points
void DrawSplineSegmentBasis(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, float thick, Color color); // Draw spline segment: B-Spline, 4 points
void DrawSplineSegmentCatmullRom(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, float thick, Color color); // Draw spline segment: Catmull-Rom, 4 points
void DrawSplineSegmentBezierQuadratic(Vector2 p1, Vector2 c2, Vector2 p3, float thick, Color color); // Draw spline segment: Quadratic Bezier, 2 points, 1 control point
void DrawSplineSegmentBezierCubic(Vector2 p1, Vector2 c2, Vector2 c3, Vector2 p4, float thick, Color color); // Draw spline segment: Cubic Bezier, 2 points, 2 control points
// Spline segment point evaluation functions, for a given t [0.0f .. 1.0f]
Vector2 GetSplinePointLinear(Vector2 startPos, Vector2 endPos, float t);                           // Get (evaluate) spline point: Linear
Vector2 GetSplinePointBasis(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, float t);              // Get (evaluate) spline point: B-Spline
Vector2 GetSplinePointCatmullRom(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, float t);         // Get (evaluate) spline point: Catmull-Rom
Vector2 GetSplinePointBezierQuad(Vector2 p1, Vector2 c2, Vector2 p3, float t);                     // Get (evaluate) spline point: Quadratic Bezier
Vector2 GetSplinePointBezierCubic(Vector2 p1, Vector2 c2, Vector2 c3, Vector2 p4, float t);        // Get (evaluate) spline point: Cubic Bezier
// Basic shapes collision detection functions
bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2);                                           // Check collision between two rectangles
bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2);        // Check collision between two circles
bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec);                         // Check collision between circle and rectangle
bool CheckCollisionCircleLine(Vector2 center, float radius, Vector2 p1, Vector2 p2);               // Check if circle collides with a line created betweeen two points [p1] and [p2]
bool CheckCollisionPointRec(Vector2 point, Rectangle rec);                                         // Check if point is inside rectangle
bool CheckCollisionPointCircle(Vector2 point, Vector2 center, float radius);                       // Check if point is inside circle
bool CheckCollisionPointTriangle(Vector2 point, Vector2 p1, Vector2 p2, Vector2 p3);               // Check if point is inside a triangle
bool CheckCollisionPointLine(Vector2 point, Vector2 p1, Vector2 p2, int threshold);                // Check if point belongs to line created between two points [p1] and [p2] with defined margin in pixels [threshold]
bool CheckCollisionPointPoly(Vector2 point, const Vector2 *points, int pointCount);                // Check if point is within a polygon described by array of vertices
bool CheckCollisionLines(Vector2 startPos1, Vector2 endPos1, Vector2 startPos2, Vector2 endPos2, Vector2 *collisionPoint); // Check the collision between two lines defined by two points each, returns collision point by reference
Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2);                                         // Get collision rectangle for two rectangles collision
// Texture Loading and Drawing Functions (Module: textures)
// Image loading functions
// NOTE: These functions do not require GPU access
Image LoadImage(const char *fileName);                                                             // Load image from file into CPU memory (RAM)
Image LoadImageRaw(const char *fileName, int width, int height, int format, int headerSize);       // Load image from RAW file data
Image LoadImageAnim(const char *fileName, int *frames);                                            // Load image sequence from file (frames appended to image.data)
Image LoadImageAnimFromMemory(const char *fileType, const unsigned char *fileData, int dataSize, int *frames); // Load image sequence from memory buffer
Image LoadImageFromMemory(const char *fileType, const unsigned char *fileData, int dataSize);      // Load image from memory buffer, fileType refers to extension: i.e. '.png'
Image LoadImageFromTexture(Texture2D texture);                                                     // Load image from GPU texture data
Image LoadImageFromScreen(void);                                                                   // Load image from screen buffer and (screenshot)
bool IsImageValid(Image image);                                                                    // Check if an image is valid (data and parameters)
void UnloadImage(Image image);                                                                     // Unload image from CPU memory (RAM)
bool ExportImage(Image image, const char *fileName);                                               // Export image data to file, returns true on success
unsigned char *ExportImageToMemory(Image image, const char *fileType, int *fileSize);              // Export image to memory buffer
bool ExportImageAsCode(Image image, const char *fileName);                                         // Export image as code file defining an array of bytes, returns true on success
// Image generation functions
Image GenImageColor(int width, int height, Color color);                                           // Generate image: plain color
Image GenImageGradientLinear(int width, int height, int direction, Color start, Color end);        // Generate image: linear gradient, direction in degrees [0..360], 0=Vertical gradient
Image GenImageGradientRadial(int width, int height, float density, Color inner, Color outer);      // Generate image: radial gradient
Image GenImageGradientSquare(int width, int height, float density, Color inner, Color outer);      // Generate image: square gradient
Image GenImageChecked(int width, int height, int checksX, int checksY, Color col1, Color col2);    // Generate image: checked
Image GenImageWhiteNoise(int width, int height, float factor);                                     // Generate image: white noise
Image GenImagePerlinNoise(int width, int height, int offsetX, int offsetY, float scale);           // Generate image: perlin noise
Image GenImageCellular(int width, int height, int tileSize);                                       // Generate image: cellular algorithm, bigger tileSize means bigger cells
Image GenImageText(int width, int height, const char *text);                                       // Generate image: grayscale image from text data
// Image manipulation functions
Image ImageCopy(Image image);                                                                      // Create an image duplicate (useful for transformations)
Image ImageFromImage(Image image, Rectangle rec);                                                  // Create an image from another image piece
Image ImageFromChannel(Image image, int selectedChannel);                                          // Create an image from a selected channel of another image (GRAYSCALE)
Image ImageText(const char *text, int fontSize, Color color);                                      // Create an image from text (default font)
Image ImageTextEx(Font font, const char *text, float fontSize, float spacing, Color tint);         // Create an image from text (custom sprite font)
void ImageFormat(Image *image, int newFormat);                                                     // Convert image data to desired format
void ImageToPOT(Image *image, Color fill);                                                         // Convert image to POT (power-of-two)
void ImageCrop(Image *image, Rectangle crop);                                                      // Crop an image to a defined rectangle
void ImageAlphaCrop(Image *image, float threshold);                                                // Crop image depending on alpha value
void ImageAlphaClear(Image *image, Color color, float threshold);                                  // Clear alpha channel to desired color
void ImageAlphaMask(Image *image, Image alphaMask);                                                // Apply alpha mask to image
void ImageAlphaPremultiply(Image *image);                                                          // Premultiply alpha channel
void ImageBlurGaussian(Image *image, int blurSize);                                                // Apply Gaussian blur using a box blur approximation
void ImageKernelConvolution(Image *image, const float *kernel, int kernelSize);                    // Apply custom square convolution kernel to image
void ImageResize(Image *image, int newWidth, int newHeight);                                       // Resize image (Bicubic scaling algorithm)
void ImageResizeNN(Image *image, int newWidth,int newHeight);                                      // Resize image (Nearest-Neighbor scaling algorithm)
void ImageResizeCanvas(Image *image, int newWidth, int newHeight, int offsetX, int offsetY, Color fill); // Resize canvas and fill with color
void ImageMipmaps(Image *image);                                                                   // Compute all mipmap levels for a provided image
void ImageDither(Image *image, int rBpp, int gBpp, int bBpp, int aBpp);                            // Dither image data to 16bpp or lower (Floyd-Steinberg dithering)
void ImageFlipVertical(Image *image);                                                              // Flip image vertically
void ImageFlipHorizontal(Image *image);                                                            // Flip image horizontally
void ImageRotate(Image *image, int degrees);                                                       // Rotate image by input angle in degrees (-359 to 359)
void ImageRotateCW(Image *image);                                                                  // Rotate image clockwise 90deg
void ImageRotateCCW(Image *image);                                                                 // Rotate image counter-clockwise 90deg
void ImageColorTint(Image *image, Color color);                                                    // Modify image color: tint
void ImageColorInvert(Image *image);                                                               // Modify image color: invert
void ImageColorGrayscale(Image *image);                                                            // Modify image color: grayscale
void ImageColorContrast(Image *image, float contrast);                                             // Modify image color: contrast (-100 to 100)
void ImageColorBrightness(Image *image, int brightness);                                           // Modify image color: brightness (-255 to 255)
void ImageColorReplace(Image *image, Color color, Color replace);                                  // Modify image color: replace color
Color *LoadImageColors(Image image);                                                               // Load color data from image as a Color array (RGBA - 32bit)
Color *LoadImagePalette(Image image, int maxPaletteSize, int *colorCount);                         // Load colors palette from image as a Color array (RGBA - 32bit)
void UnloadImageColors(Color *colors);                                                             // Unload color data loaded with LoadImageColors()
void UnloadImagePalette(Color *colors);                                                            // Unload colors palette loaded with LoadImagePalette()
Rectangle GetImageAlphaBorder(Image image, float threshold);                                       // Get image alpha border rectangle
Color GetImageColor(Image image, int x, int y);                                                    // Get image pixel color at (x, y) position
// Image drawing functions
// NOTE: Image software-rendering functions (CPU)
void ImageClearBackground(Image *dst, Color color);                                                // Clear image background with given color
void ImageDrawPixel(Image *dst, int posX, int posY, Color color);                                  // Draw pixel within an image
void ImageDrawPixelV(Image *dst, Vector2 position, Color color);                                   // Draw pixel within an image (Vector version)
void ImageDrawLine(Image *dst, int startPosX, int startPosY, int endPosX, int endPosY, Color color); // Draw line within an image
void ImageDrawLineV(Image *dst, Vector2 start, Vector2 end, Color color);                          // Draw line within an image (Vector version)
void ImageDrawLineEx(Image *dst, Vector2 start, Vector2 end, int thick, Color color);              // Draw a line defining thickness within an image
void ImageDrawCircle(Image *dst, int centerX, int centerY, int radius, Color color);               // Draw a filled circle within an image
void ImageDrawCircleV(Image *dst, Vector2 center, int radius, Color color);                        // Draw a filled circle within an image (Vector version)
void ImageDrawCircleLines(Image *dst, int centerX, int centerY, int radius, Color color);          // Draw circle outline within an image
void ImageDrawCircleLinesV(Image *dst, Vector2 center, int radius, Color color);                   // Draw circle outline within an image (Vector version)
void ImageDrawRectangle(Image *dst, int posX, int posY, int width, int height, Color color);       // Draw rectangle within an image
void ImageDrawRectangleV(Image *dst, Vector2 position, Vector2 size, Color color);                 // Draw rectangle within an image (Vector version)
void ImageDrawRectangleRec(Image *dst, Rectangle rec, Color color);                                // Draw rectangle within an image
void ImageDrawRectangleLines(Image *dst, Rectangle rec, int thick, Color color);                   // Draw rectangle lines within an image
void ImageDrawTriangle(Image *dst, Vector2 v1, Vector2 v2, Vector2 v3, Color color);               // Draw triangle within an image
void ImageDrawTriangleEx(Image *dst, Vector2 v1, Vector2 v2, Vector2 v3, Color c1, Color c2, Color c3); // Draw triangle with interpolated colors within an image
void ImageDrawTriangleLines(Image *dst, Vector2 v1, Vector2 v2, Vector2 v3, Color color);          // Draw triangle outline within an image
void ImageDrawTriangleFan(Image *dst, Vector2 *points, int pointCount, Color color);               // Draw a triangle fan defined by points within an image (first vertex is the center)
void ImageDrawTriangleStrip(Image *dst, Vector2 *points, int pointCount, Color color);             // Draw a triangle strip defined by points within an image
void ImageDraw(Image *dst, Image src, Rectangle srcRec, Rectangle dstRec, Color tint);             // Draw a source image within a destination image (tint applied to source)
void ImageDrawText(Image *dst, const char *text, int posX, int posY, int fontSize, Color color);   // Draw text (using default font) within an image (destination)
void ImageDrawTextEx(Image *dst, Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint); // Draw text (custom sprite font) within an image (destination)
// Texture loading functions
// NOTE: These functions require GPU access
Texture2D LoadTexture(const char *fileName);                                                       // Load texture from file into GPU memory (VRAM)
Texture2D LoadTextureFromImage(Image image);                                                       // Load texture from image data
TextureCubemap LoadTextureCubemap(Image image, int layout);                                        // Load cubemap from image, multiple image cubemap layouts supported
RenderTexture2D LoadRenderTexture(int width, int height);                                          // Load texture for rendering (framebuffer)
bool IsTextureValid(Texture2D texture);                                                            // Check if a texture is valid (loaded in GPU)
void UnloadTexture(Texture2D texture);                                                             // Unload texture from GPU memory (VRAM)
bool IsRenderTextureValid(RenderTexture2D target);                                                 // Check if a render texture is valid (loaded in GPU)
void UnloadRenderTexture(RenderTexture2D target);                                                  // Unload render texture from GPU memory (VRAM)
void UpdateTexture(Texture2D texture, const void *pixels);                                         // Update GPU texture with new data
void UpdateTextureRec(Texture2D texture, Rectangle rec, const void *pixels);                       // Update GPU texture rectangle with new data
// Texture configuration functions
void GenTextureMipmaps(Texture2D *texture);                                                        // Generate GPU mipmaps for a texture
void SetTextureFilter(Texture2D texture, int filter);                                              // Set texture scaling filter mode
void SetTextureWrap(Texture2D texture, int wrap);                                                  // Set texture wrapping mode
// Texture drawing functions
void DrawTexture(Texture2D texture, int posX, int posY, Color tint);                               // Draw a Texture2D
void DrawTextureV(Texture2D texture, Vector2 position, Color tint);                                // Draw a Texture2D with position defined as Vector2
void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint);  // Draw a Texture2D with extended parameters
void DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position, Color tint);            // Draw a part of a texture defined by a rectangle
void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint); // Draw a part of a texture defined by a rectangle with 'pro' parameters
void DrawTextureNPatch(Texture2D texture, NPatchInfo nPatchInfo, Rectangle dest, Vector2 origin, float rotation, Color tint); // Draws a texture (or part of it) that stretches or shrinks nicely
// Color/pixel related functions
bool ColorIsEqual(Color col1, Color col2);                            // Check if two colors are equal
Color Fade(Color color, float alpha);                                 // Get color with alpha applied, alpha goes from 0.0f to 1.0f
int ColorToInt(Color color);                                          // Get hexadecimal value for a Color (0xRRGGBBAA)
Vector4 ColorNormalize(Color color);                                  // Get Color normalized as float [0..1]
Color ColorFromNormalized(Vector4 normalized);                        // Get Color from normalized values [0..1]
Vector3 ColorToHSV(Color color);                                      // Get HSV values for a Color, hue [0..360], saturation/value [0..1]
Color ColorFromHSV(float hue, float saturation, float value);         // Get a Color from HSV values, hue [0..360], saturation/value [0..1]
Color ColorTint(Color color, Color tint);                             // Get color multiplied with another color
Color ColorBrightness(Color color, float factor);                     // Get color with brightness correction, brightness factor goes from -1.0f to 1.0f
Color ColorContrast(Color color, float contrast);                     // Get color with contrast correction, contrast values between -1.0f and 1.0f
Color ColorAlpha(Color color, float alpha);                           // Get color with alpha applied, alpha goes from 0.0f to 1.0f
Color ColorAlphaBlend(Color dst, Color src, Color tint);              // Get src alpha-blended into dst color with tint
Color ColorLerp(Color color1, Color color2, float factor);            // Get color lerp interpolation between two colors, factor [0.0f..1.0f]
Color GetColor(unsigned int hexValue);                                // Get Color structure from hexadecimal value
Color GetPixelColor(void *srcPtr, int format);                        // Get Color from a source pixel pointer of certain format
void SetPixelColor(void *dstPtr, Color color, int format);            // Set color formatted into destination pixel pointer
int GetPixelDataSize(int width, int height, int format);              // Get pixel data size in bytes for certain format
// Font Loading and Text Drawing Functions (Module: text)
// Font loading/unloading functions
Font GetFontDefault(void);                                                            // Get the default Font
Font LoadFont(const char *fileName);                                                  // Load font from file into GPU memory (VRAM)
Font LoadFontEx(const char *fileName, int fontSize, int *codepoints, int codepointCount); // Load font from file with extended parameters, use NULL for codepoints and 0 for codepointCount to load the default character set, font size is provided in pixels height
Font LoadFontFromImage(Image image, Color key, int firstChar);                        // Load font from Image (XNA style)
Font LoadFontFromMemory(const char *fileType, const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount); // Load font from memory buffer, fileType refers to extension: i.e. '.ttf'
bool IsFontValid(Font font);                                                          // Check if a font is valid (font data loaded, WARNING: GPU texture not checked)
GlyphInfo *LoadFontData(const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount, int type); // Load font data for further use
Image GenImageFontAtlas(const GlyphInfo *glyphs, Rectangle **glyphRecs, int glyphCount, int fontSize, int padding, int packMethod); // Generate image font atlas using chars info
void UnloadFontData(GlyphInfo *glyphs, int glyphCount);                               // Unload font chars info data (RAM)
void UnloadFont(Font font);                                                           // Unload font from GPU memory (VRAM)
bool ExportFontAsCode(Font font, const char *fileName);                               // Export font as code file, returns true on success
// Text drawing functions
void DrawFPS(int posX, int posY);                                                     // Draw current FPS
void DrawText(const char *text, int posX, int posY, int fontSize, Color color);       // Draw text (using default font)
void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint); // Draw text using font and additional parameters
void DrawTextPro(Font font, const char *text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint); // Draw text using Font and pro parameters (rotation)
void DrawTextCodepoint(Font font, int codepoint, Vector2 position, float fontSize, Color tint); // Draw one character (codepoint)
void DrawTextCodepoints(Font font, const int *codepoints, int codepointCount, Vector2 position, float fontSize, float spacing, Color tint); // Draw multiple character (codepoint)
// Text font info functions
void SetTextLineSpacing(int spacing);                                                 // Set vertical line spacing when drawing with line-breaks
int MeasureText(const char *text, int fontSize);                                      // Measure string width for default font
Vector2 MeasureTextEx(Font font, const char *text, float fontSize, float spacing);    // Measure string size for Font
int GetGlyphIndex(Font font, int codepoint);                                          // Get glyph index position in font for a codepoint (unicode character), fallback to '?' if not found
GlyphInfo GetGlyphInfo(Font font, int codepoint);                                     // Get glyph font info data for a codepoint (unicode character), fallback to '?' if not found
Rectangle GetGlyphAtlasRec(Font font, int codepoint);                                 // Get glyph rectangle in font atlas for a codepoint (unicode character), fallback to '?' if not found
// Text codepoints management functions (unicode characters)
char *LoadUTF8(const int *codepoints, int length);                // Load UTF-8 text encoded from codepoints array
void UnloadUTF8(char *text);                                      // Unload UTF-8 text encoded from codepoints array
int *LoadCodepoints(const char *text, int *count);                // Load all codepoints from a UTF-8 text string, codepoints count returned by parameter
void UnloadCodepoints(int *codepoints);                           // Unload codepoints data from memory
int GetCodepointCount(const char *text);                          // Get total number of codepoints in a UTF-8 encoded string
int GetCodepoint(const char *text, int *codepointSize);           // Get next codepoint in a UTF-8 encoded string, 0x3f('?') is returned on failure
int GetCodepointNext(const char *text, int *codepointSize);       // Get next codepoint in a UTF-8 encoded string, 0x3f('?') is returned on failure
int GetCodepointPrevious(const char *text, int *codepointSize);   // Get previous codepoint in a UTF-8 encoded string, 0x3f('?') is returned on failure
const char *CodepointToUTF8(int codepoint, int *utf8Size);        // Encode one codepoint into UTF-8 byte array (array length returned as parameter)
// Text strings management functions (no UTF-8 strings, only byte chars)
// NOTE: Some strings allocate memory internally for returned strings, just be careful!
int TextCopy(char *dst, const char *src);                                             // Copy one string to another, returns bytes copied
bool TextIsEqual(const char *text1, const char *text2);                               // Check if two text string are equal
unsigned int TextLength(const char *text);                                            // Get text length, checks for '\0' ending
const char *TextFormat(const char *text, ...);                                        // Text formatting with variables (sprintf() style)
const char *TextSubtext(const char *text, int position, int length);                  // Get a piece of a text string
char *TextReplace(const char *text, const char *replace, const char *by);             // Replace text string (WARNING: memory must be freed!)
char *TextInsert(const char *text, const char *insert, int position);                 // Insert text in a position (WARNING: memory must be freed!)
const char *TextJoin(const char **textList, int count, const char *delimiter);        // Join text strings with delimiter
const char **TextSplit(const char *text, char delimiter, int *count);                 // Split text into multiple strings
void TextAppend(char *text, const char *append, int *position);                       // Append text at specific position and move cursor!
int TextFindIndex(const char *text, const char *find);                                // Find first text occurrence within a string
const char *TextToUpper(const char *text);                      // Get upper case version of provided string
const char *TextToLower(const char *text);                      // Get lower case version of provided string
const char *TextToPascal(const char *text);                     // Get Pascal case notation version of provided string
const char *TextToSnake(const char *text);                      // Get Snake case notation version of provided string
const char *TextToCamel(const char *text);                      // Get Camel case notation version of provided string
int TextToInteger(const char *text);                            // Get integer value from text (negative values not supported)
float TextToFloat(const char *text);                            // Get float value from text (negative values not supported)
// Basic 3d Shapes Drawing Functions (Module: models)
// Basic geometric 3D shapes drawing functions
void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color);                                    // Draw a line in 3D world space
void DrawPoint3D(Vector3 position, Color color);                                                   // Draw a point in 3D space, actually a small line
void DrawCircle3D(Vector3 center, float radius, Vector3 rotationAxis, float rotationAngle, Color color); // Draw a circle in 3D world space
void DrawTriangle3D(Vector3 v1, Vector3 v2, Vector3 v3, Color color);                              // Draw a color-filled triangle (vertex in counter-clockwise order!)
void DrawTriangleStrip3D(const Vector3 *points, int pointCount, Color color);                      // Draw a triangle strip defined by points
void DrawCube(Vector3 position, float width, float height, float length, Color color);             // Draw cube
void DrawCubeV(Vector3 position, Vector3 size, Color color);                                       // Draw cube (Vector version)
void DrawCubeWires(Vector3 position, float width, float height, float length, Color color);        // Draw cube wires
void DrawCubeWiresV(Vector3 position, Vector3 size, Color color);                                  // Draw cube wires (Vector version)
void DrawSphere(Vector3 centerPos, float radius, Color color);                                     // Draw sphere
void DrawSphereEx(Vector3 centerPos, float radius, int rings, int slices, Color color);            // Draw sphere with extended parameters
void DrawSphereWires(Vector3 centerPos, float radius, int rings, int slices, Color color);         // Draw sphere wires
void DrawCylinder(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color); // Draw a cylinder/cone
void DrawCylinderEx(Vector3 startPos, Vector3 endPos, float startRadius, float endRadius, int sides, Color color); // Draw a cylinder with base at startPos and top at endPos
void DrawCylinderWires(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color); // Draw a cylinder/cone wires
void DrawCylinderWiresEx(Vector3 startPos, Vector3 endPos, float startRadius, float endRadius, int sides, Color color); // Draw a cylinder wires with base at startPos and top at endPos
void DrawCapsule(Vector3 startPos, Vector3 endPos, float radius, int slices, int rings, Color color); // Draw a capsule with the center of its sphere caps at startPos and endPos
void DrawCapsuleWires(Vector3 startPos, Vector3 endPos, float radius, int slices, int rings, Color color); // Draw capsule wireframe with the center of its sphere caps at startPos and endPos
void DrawPlane(Vector3 centerPos, Vector2 size, Color color);                                      // Draw a plane XZ
void DrawRay(Ray ray, Color color);                                                                // Draw a ray line
void DrawGrid(int slices, float spacing);                                                          // Draw a grid (centered at (0, 0, 0))
// Model 3d Loading and Drawing Functions (Module: models)
// Model management functions
Model LoadModel(const char *fileName);                                                // Load model from files (meshes and materials)
Model LoadModelFromMesh(Mesh mesh);                                                   // Load model from generated mesh (default material)
bool IsModelValid(Model model);                                                       // Check if a model is valid (loaded in GPU, VAO/VBOs)
void UnloadModel(Model model);                                                        // Unload model (including meshes) from memory (RAM and/or VRAM)
BoundingBox GetModelBoundingBox(Model model);                                         // Compute model bounding box limits (considers all meshes)
// Model drawing functions
void DrawModel(Model model, Vector3 position, float scale, Color tint);               // Draw a model (with texture if set)
void DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint); // Draw a model with extended parameters
void DrawModelWires(Model model, Vector3 position, float scale, Color tint);          // Draw a model wires (with texture if set)
void DrawModelWiresEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint); // Draw a model wires (with texture if set) with extended parameters
void DrawModelPoints(Model model, Vector3 position, float scale, Color tint); // Draw a model as points
void DrawModelPointsEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint); // Draw a model as points with extended parameters
void DrawBoundingBox(BoundingBox box, Color color);                                   // Draw bounding box (wires)
void DrawBillboard(Camera camera, Texture2D texture, Vector3 position, float scale, Color tint);   // Draw a billboard texture
void DrawBillboardRec(Camera camera, Texture2D texture, Rectangle source, Vector3 position, Vector2 size, Color tint); // Draw a billboard texture defined by source
void DrawBillboardPro(Camera camera, Texture2D texture, Rectangle source, Vector3 position, Vector3 up, Vector2 size, Vector2 origin, float rotation, Color tint); // Draw a billboard texture defined by source and rotation
// Mesh management functions
void UploadMesh(Mesh *mesh, bool dynamic);                                            // Upload mesh vertex data in GPU and provide VAO/VBO ids
void UpdateMeshBuffer(Mesh mesh, int index, const void *data, int dataSize, int offset); // Update mesh vertex data in GPU for a specific buffer index
void UnloadMesh(Mesh mesh);                                                           // Unload mesh data from CPU and GPU
void DrawMesh(Mesh mesh, Material material, Matrix transform);                        // Draw a 3d mesh with material and transform
void DrawMeshInstanced(Mesh mesh, Material material, const Matrix *transforms, int instances); // Draw multiple mesh instances with material and different transforms
BoundingBox GetMeshBoundingBox(Mesh mesh);                                            // Compute mesh bounding box limits
void GenMeshTangents(Mesh *mesh);                                                     // Compute mesh tangents
bool ExportMesh(Mesh mesh, const char *fileName);                                     // Export mesh data to file, returns true on success
bool ExportMeshAsCode(Mesh mesh, const char *fileName);                               // Export mesh as code file (.h) defining multiple arrays of vertex attributes
// Mesh generation functions
Mesh GenMeshPoly(int sides, float radius);                                            // Generate polygonal mesh
Mesh GenMeshPlane(float width, float length, int resX, int resZ);                     // Generate plane mesh (with subdivisions)
Mesh GenMeshCube(float width, float height, float length);                            // Generate cuboid mesh
Mesh GenMeshSphere(float radius, int rings, int slices);                              // Generate sphere mesh (standard sphere)
Mesh GenMeshHemiSphere(float radius, int rings, int slices);                          // Generate half-sphere mesh (no bottom cap)
Mesh GenMeshCylinder(float radius, float height, int slices);                         // Generate cylinder mesh
Mesh GenMeshCone(float radius, float height, int slices);                             // Generate cone/pyramid mesh
Mesh GenMeshTorus(float radius, float size, int radSeg, int sides);                   // Generate torus mesh
Mesh GenMeshKnot(float radius, float size, int radSeg, int sides);                    // Generate trefoil knot mesh
Mesh GenMeshHeightmap(Image heightmap, Vector3 size);                                 // Generate heightmap mesh from image data
Mesh GenMeshCubicmap(Image cubicmap, Vector3 cubeSize);                               // Generate cubes-based map mesh from image data
// Material loading/unloading functions
Material *LoadMaterials(const char *fileName, int *materialCount);                    // Load materials from model file
Material LoadMaterialDefault(void);                                                   // Load default material (Supports: DIFFUSE, SPECULAR, NORMAL maps)
bool IsMaterialValid(Material material);                                              // Check if a material is valid (shader assigned, map textures loaded in GPU)
void UnloadMaterial(Material material);                                               // Unload material from GPU memory (VRAM)
void SetMaterialTexture(Material *material, int mapType, Texture2D texture);          // Set texture for a material map type (MATERIAL_MAP_DIFFUSE, MATERIAL_MAP_SPECULAR...)
void SetModelMeshMaterial(Model *model, int meshId, int materialId);                  // Set material for a mesh
// Model animations loading/unloading functions
ModelAnimation *LoadModelAnimations(const char *fileName, int *animCount);            // Load model animations from file
void UpdateModelAnimation(Model model, ModelAnimation anim, int frame);               // Update model animation pose (CPU)
void UpdateModelAnimationBones(Model model, ModelAnimation anim, int frame);          // Update model animation mesh bone matrices (GPU skinning)
void UnloadModelAnimation(ModelAnimation anim);                                       // Unload animation data
void UnloadModelAnimations(ModelAnimation *animations, int animCount);                // Unload animation array data
bool IsModelAnimationValid(Model model, ModelAnimation anim);                         // Check model animation skeleton match
// Collision detection functions
bool CheckCollisionSpheres(Vector3 center1, float radius1, Vector3 center2, float radius2);   // Check collision between two spheres
bool CheckCollisionBoxes(BoundingBox box1, BoundingBox box2);                                 // Check collision between two bounding boxes
bool CheckCollisionBoxSphere(BoundingBox box, Vector3 center, float radius);                  // Check collision between box and sphere
RayCollision GetRayCollisionSphere(Ray ray, Vector3 center, float radius);                    // Get collision info between ray and sphere
RayCollision GetRayCollisionBox(Ray ray, BoundingBox box);                                    // Get collision info between ray and box
RayCollision GetRayCollisionMesh(Ray ray, Mesh mesh, Matrix transform);                       // Get collision info between ray and mesh
RayCollision GetRayCollisionTriangle(Ray ray, Vector3 p1, Vector3 p2, Vector3 p3);            // Get collision info between ray and triangle
RayCollision GetRayCollisionQuad(Ray ray, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4);    // Get collision info between ray and quad
// Audio Loading and Playing Functions (Module: audio)
typedef void (*AudioCallback)(void *bufferData, unsigned int frames);
// Audio device management functions
void InitAudioDevice(void);                                     // Initialize audio device and context
void CloseAudioDevice(void);                                    // Close the audio device and context
bool IsAudioDeviceReady(void);                                  // Check if audio device has been initialized successfully
void SetMasterVolume(float volume);                             // Set master volume (listener)
float GetMasterVolume(void);                                    // Get master volume (listener)
// Wave/Sound loading/unloading functions
Wave LoadWave(const char *fileName);                            // Load wave data from file
Wave LoadWaveFromMemory(const char *fileType, const unsigned char *fileData, int dataSize); // Load wave from memory buffer, fileType refers to extension: i.e. '.wav'
bool IsWaveValid(Wave wave);                                    // Checks if wave data is valid (data loaded and parameters)
Sound LoadSound(const char *fileName);                          // Load sound from file
Sound LoadSoundFromWave(Wave wave);                             // Load sound from wave data
Sound LoadSoundAlias(Sound source);                             // Create a new sound that shares the same sample data as the source sound, does not own the sound data
bool IsSoundValid(Sound sound);                                 // Checks if a sound is valid (data loaded and buffers initialized)
void UpdateSound(Sound sound, const void *data, int sampleCount); // Update sound buffer with new data
void UnloadWave(Wave wave);                                     // Unload wave data
void UnloadSound(Sound sound);                                  // Unload sound
void UnloadSoundAlias(Sound alias);                             // Unload a sound alias (does not deallocate sample data)
bool ExportWave(Wave wave, const char *fileName);               // Export wave data to file, returns true on success
bool ExportWaveAsCode(Wave wave, const char *fileName);         // Export wave sample data to code (.h), returns true on success
// Wave/Sound management functions
void PlaySound(Sound sound);                                    // Play a sound
void StopSound(Sound sound);                                    // Stop playing a sound
void PauseSound(Sound sound);                                   // Pause a sound
void ResumeSound(Sound sound);                                  // Resume a paused sound
bool IsSoundPlaying(Sound sound);                               // Check if a sound is currently playing
void SetSoundVolume(Sound sound, float volume);                 // Set volume for a sound (1.0 is max level)
void SetSoundPitch(Sound sound, float pitch);                   // Set pitch for a sound (1.0 is base level)
void SetSoundPan(Sound sound, float pan);                       // Set pan for a sound (0.5 is center)
Wave WaveCopy(Wave wave);                                       // Copy a wave to a new wave
void WaveCrop(Wave *wave, int initFrame, int finalFrame);       // Crop a wave to defined frames range
void WaveFormat(Wave *wave, int sampleRate, int sampleSize, int channels); // Convert wave data to desired format
float *LoadWaveSamples(Wave wave);                              // Load samples data from wave as a 32bit float data array
void UnloadWaveSamples(float *samples);                         // Unload samples data loaded with LoadWaveSamples()
// Music management functions
Music LoadMusicStream(const char *fileName);                    // Load music stream from file
Music LoadMusicStreamFromMemory(const char *fileType, const unsigned char *data, int dataSize); // Load music stream from data
bool IsMusicValid(Music music);                                 // Checks if a music stream is valid (context and buffers initialized)
void UnloadMusicStream(Music music);                            // Unload music stream
void PlayMusicStream(Music music);                              // Start music playing
bool IsMusicStreamPlaying(Music music);                         // Check if music is playing
void UpdateMusicStream(Music music);                            // Updates buffers for music streaming
void StopMusicStream(Music music);                              // Stop music playing
void PauseMusicStream(Music music);                             // Pause music playing
void ResumeMusicStream(Music music);                            // Resume playing paused music
void SeekMusicStream(Music music, float position);              // Seek music to a position (in seconds)
void SetMusicVolume(Music music, float volume);                 // Set volume for music (1.0 is max level)
void SetMusicPitch(Music music, float pitch);                   // Set pitch for a music (1.0 is base level)
void SetMusicPan(Music music, float pan);                       // Set pan for a music (0.5 is center)
float GetMusicTimeLength(Music music);                          // Get music time length (in seconds)
float GetMusicTimePlayed(Music music);                          // Get current music time played (in seconds)
// AudioStream management functions
AudioStream LoadAudioStream(unsigned int sampleRate, unsigned int sampleSize, unsigned int channels); // Load audio stream (to stream raw audio pcm data)
bool IsAudioStreamValid(AudioStream stream);                    // Checks if an audio stream is valid (buffers initialized)
void UnloadAudioStream(AudioStream stream);                     // Unload audio stream and free memory
void UpdateAudioStream(AudioStream stream, const void *data, int frameCount); // Update audio stream buffers with data
bool IsAudioStreamProcessed(AudioStream stream);                // Check if any audio stream buffers requires refill
void PlayAudioStream(AudioStream stream);                       // Play audio stream
void PauseAudioStream(AudioStream stream);                      // Pause audio stream
void ResumeAudioStream(AudioStream stream);                     // Resume audio stream
bool IsAudioStreamPlaying(AudioStream stream);                  // Check if audio stream is playing
void StopAudioStream(AudioStream stream);                       // Stop audio stream
void SetAudioStreamVolume(AudioStream stream, float volume);    // Set volume for audio stream (1.0 is max level)
void SetAudioStreamPitch(AudioStream stream, float pitch);      // Set pitch for audio stream (1.0 is base level)
void SetAudioStreamPan(AudioStream stream, float pan);          // Set pan for audio stream (0.5 is centered)
void SetAudioStreamBufferSizeDefault(int size);                 // Default size for new audio streams
void SetAudioStreamCallback(AudioStream stream, AudioCallback callback); // Audio thread callback to request new data
void AttachAudioStreamProcessor(AudioStream stream, AudioCallback processor); // Attach audio stream processor to stream, receives the samples as 'float'
void DetachAudioStreamProcessor(AudioStream stream, AudioCallback processor); // Detach audio stream processor from stream
void AttachAudioMixedProcessor(AudioCallback processor); // Attach audio stream processor to the entire audio pipeline, receives the samples as 'float'
void DetachAudioMixedProcessor(AudioCallback processor); // Detach audio stream processor from the entire audio pipeline

#endif //  _RAYLIB_AUTOCOMPLETE_DICT_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Defines
RAYLIB_VERSION_MAJOR
RAYLIB_VERSION_MINOR
RAYLIB_VERSION_PATCH
RAYLIB_VERSION
PI
DEG2RAD
RAD2DEG
RL_COLOR_TYPE
RL_RECTANGLE_TYPE
RL_VECTOR2_TYPE
RL_VECTOR3_TYPE
RL_VECTOR4_TYPE
RL_QUATERNION_TYPE
RL_MATRIX_TYPE
LIGHTGRAY
GRAY
DARKGRAY
YELLOW
GOLD
ORANGE
PINK
RED
MAROON
GREEN
LIME
DARKGREEN
SKYBLUE
BLUE
DARKBLUE
PURPLE
VIOLET
DARKPURPLE
BEIGE
BROWN
DARKBROWN
WHITE
BLACK
BLANK
MAGENTA
RAYWHITE
RL_BOOL_TYPE
FLAG_VSYNC_HINT
FLAG_FULLSCREEN_MODE
FLAG_WINDOW_RESIZABLE
FLAG_WINDOW_UNDECORATED
FLAG_WINDOW_HIDDEN
FLAG_WINDOW_MINIMIZED
FLAG_WINDOW_MAXIMIZED
FLAG_WINDOW_UNFOCUSED
FLAG_WINDOW_TOPMOST
FLAG_WINDOW_ALWAYS_RUN
FLAG_WINDOW_TRANSPARENT
FLAG_WINDOW_HIGHDPI
FLAG_WINDOW_MOUSE_PASSTHROUGH
FLAG_BORDERLESS_WINDOWED_MODE
FLAG_MSAA_4X_HINT
FLAG_INTERLACED_HINT
LOG_ALL
LOG_TRACE
LOG_DEBUG
LOG_INFO
LOG_WARNING
LOG_ERROR
LOG_FATAL
LOG_NONE
KEY_NULL
KEY_APOSTROPHE
KEY_COMMA
KEY_MINUS
KEY_PERIOD
KEY_SLASH
KEY_ZERO
KEY_ONE
KEY_TWO
KEY_THREE
KEY_FOUR
KEY_FIVE
KEY_SIX
KEY_SEVEN
KEY_EIGHT
KEY_NINE
KEY_SEMICOLON
KEY_EQUAL
KEY_A
KEY_B
KEY_C
KEY_D
KEY_E
KEY_F
KEY_G
KEY_H
KEY_I
KEY_J
KEY_K
KEY_L
KEY_M
KEY_N
KEY_O
KEY_P
KEY_Q
KEY_R
KEY_S
KEY_T
KEY_U
KEY_V
KEY_W
KEY_X
KEY_Y
KEY_Z
KEY_LEFT_BRACKET
KEY_BACKSLASH
KEY_RIGHT_BRACKET
KEY_GRAVE
KEY_SPACE
KEY_ESCAPE
KEY_ENTER
KEY_TAB
KEY_BACKSPACE
KEY_INSERT
KEY_DELETE
KEY_RIGHT
KEY_LEFT
KEY_DOWN
KEY_UP
KEY_PAGE_UP
KEY_PAGE_DOWN
KEY_HOME
KEY_END
KEY_CAPS_LOCK
KEY_SCROLL_LOCK
KEY_NUM_LOCK
KEY_PRINT_SCREEN
KEY_PAUSE
KEY_F1
KEY_F2
KEY_F3
KEY_F4
KEY_F5
KEY_F6
KEY_F7
KEY_F8
KEY_F9
KEY_F10
KEY_F11
KEY_F12
KEY_LEFT_SHIFT
KEY_LEFT_CONTROL
KEY_LEFT_ALT
KEY_LEFT_SUPER
KEY_RIGHT_SHIFT
KEY_RIGHT_CONTROL
KEY_RIGHT_ALT
KEY_RIGHT_SUPER
KEY_KB_MENU
KEY_KP_0
KEY_KP_1
KEY_KP_2
KEY_KP_3
KEY_KP_4
KEY_KP_5
KEY_KP_6
KEY_KP_7
KEY_KP_8
KEY_KP_9
KEY_KP_DECIMAL
KEY_KP_DIVIDE
KEY_KP_MULTIPLY
KEY_KP_SUBTRACT
KEY_KP_ADD
KEY_KP_ENTER
KEY_KP_EQUAL
KEY_BACK
KEY_MENU
KEY_VOLUME_UP
KEY_VOLUME_DOWN
MOUSE_LEFT_BUTTON
MOUSE_RIGHT_BUTTON
MOUSE_MIDDLE_BUTTON
MOUSE_BUTTON_LEFT
MOUSE_BUTTON_RIGHT
MOUSE_BUTTON_MIDDLE
MOUSE_BUTTON_SIDE
MOUSE_BUTTON_EXTRA
MOUSE_BUTTON_FORWARD
MOUSE_BUTTON_BACK
MOUSE_CURSOR_DEFAULT
MOUSE_CURSOR_ARROW
MOUSE_CURSOR_IBEAM
MOUSE_CURSOR_CROSSHAIR
MOUSE_CURSOR_POINTING_HAND
MOUSE_CURSOR_RESIZE_EW
MOUSE_CURSOR_RESIZE_NS
MOUSE_CURSOR_RESIZE_NWSE
MOUSE_CURSOR_RESIZE_NESW
MOUSE_CURSOR_RESIZE_ALL
MOUSE_CURSOR_NOT_ALLOWED
GAMEPAD_BUTTON_UNKNOWN
GAMEPAD_BUTTON_LEFT_FACE_UP
GAMEPAD_BUTTON_LEFT_FACE_RIGHT
GAMEPAD_BUTTON_LEFT_FACE_DOWN
GAMEPAD_BUTTON_LEFT_FACE_LEFT
GAMEPAD_BUTTON_RIGHT_FACE_UP
GAMEPAD_BUTTON_RIGHT_FACE_RIGHT
GAMEPAD_BUTTON_RIGHT_FACE_DOWN
GAMEPAD_BUTTON_RIGHT_FACE_LEFT
GAMEPAD_BUTTON_LEFT_TRIGGER_1
GAMEPAD_BUTTON_LEFT_TRIGGER_2
GAMEPAD_BUTTON_RIGHT_TRIGGER_1
GAMEPAD_BUTTON_RIGHT_TRIGGER_2
GAMEPAD_BUTTON_MIDDLE_LEFT
GAMEPAD_BUTTON_MIDDLE
GAMEPAD_BUTTON_MIDDLE_RIGHT
GAMEPAD_BUTTON_LEFT_THUMB
GAMEPAD_BUTTON_RIGHT_THUMB
GAMEPAD_AXIS_LEFT_X
GAMEPAD_AXIS_LEFT_Y
GAMEPAD_AXIS_RIGHT_X
GAMEPAD_AXIS_RIGHT_Y
GAMEPAD_AXIS_LEFT_TRIGGER
GAMEPAD_AXIS_RIGHT_TRIGGER
MATERIAL_MAP_ALBEDO
MATERIAL_MAP_METALNESS
MATERIAL_MAP_NORMAL
MATERIAL_MAP_ROUGHNESS
MATERIAL_MAP_OCCLUSION
MATERIAL_MAP_EMISSION
MATERIAL_MAP_HEIGHT
MATERIAL_MAP_CUBEMAP
MATERIAL_MAP_IRRADIANCE
MATERIAL_MAP_PREFILTER
MATERIAL_MAP_BRDF
MATERIAL_MAP_DIFFUSE
MATERIAL_MAP_SPECULAR
SHADER_LOC_VERTEX_POSITION
SHADER_LOC_VERTEX_TEXCOORD01
SHADER_LOC_VERTEX_TEXCOORD02
SHADER_LOC_VERTEX_NORMAL
SHADER_LOC_VERTEX_TANGENT
SHADER_LOC_VERTEX_COLOR
SHADER_LOC_MATRIX_MVP
SHADER_LOC_MATRIX_VIEW
SHADER_LOC_MATRIX_PROJECTION
SHADER_LOC_MATRIX_MODEL
SHADER_LOC_MATRIX_NORMAL
SHADER_LOC_VECTOR_VIEW
SHADER_LOC_COLOR_DIFFUSE
SHADER_LOC_COLOR_SPECULAR
SHADER_LOC_COLOR_AMBIENT
SHADER_LOC_MAP_ALBEDO
SHADER_LOC_MAP_METALNESS
SHADER_LOC_MAP_NORMAL
SHADER_LOC_MAP_ROUGHNESS
SHADER_LOC_MAP_OCCLUSION
SHADER_LOC_MAP_EMISSION
SHADER_LOC_MAP_HEIGHT
SHADER_LOC_MAP_CUBEMAP
SHADER_LOC_MAP_IRRADIANCE
SHADER_LOC_MAP_PREFILTER
SHADER_LOC_MAP_BRDF
SHADER_LOC_VERTEX_BONEIDS
SHADER_LOC_VERTEX_BONEWEIGHTS
SHADER_LOC_BONE_MATRICES
SHADER_LOC_MAP_DIFFUSE
SHADER_LOC_MAP_SPECULAR
SHADER_UNIFORM_FLOAT
SHADER_UNIFORM_VEC2
SHADER_UNIFORM_VEC3
SHADER_UNIFORM_VEC4
SHADER_UNIFORM_INT
SHADER_UNIFORM_IVEC2
SHADER_UNIFORM_IVEC3
SHADER_UNIFORM_IVEC4
SHADER_UNIFORM_SAMPLER2D
SHADER_ATTRIB_FLOAT
SHADER_ATTRIB_VEC2
SHADER_ATTRIB_VEC3
SHADER_ATTRIB_VEC4
PIXELFORMAT_UNCOMPRESSED_GRAYSCALE
PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA
PIXELFORMAT_UNCOMPRESSED_R5G6B5
PIXELFORMAT_UNCOMPRESSED_R8G8B8
PIXELFORMAT_UNCOMPRESSED_R5G5B5A1
PIXELFORMAT_UNCOMPRESSED_R4G4B4A4
PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
PIXELFORMAT_UNCOMPRESSED_R32
PIXELFORMAT_UNCOMPRESSED_R32G32B32
PIXELFORMAT_UNCOMPRESSED_R32G32B32A32
PIXELFORMAT_UNCOMPRESSED_R16
PIXELFORMAT_UNCOMPRESSED_R16G16B16
PIXELFORMAT_UNCOMPRESSED_R16G16B16A16
PIXELFORMAT_COMPRESSED_DXT1_RGB
PIXELFORMAT_COMPRESSED_DXT1_RGBA
PIXELFORMAT_COMPRESSED_DXT3_RGBA
PIXELFORMAT_COMPRESSED_DXT5_RGBA
PIXELFORMAT_COMPRESSED_ETC1_RGB
PIXELFORMAT_COMPRESSED_ETC2_RGB
PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA
PIXELFORMAT_COMPRESSED_PVRT_RGB
PIXELFORMAT_COMPRESSED_PVRT_RGBA
PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA
PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA
TEXTURE_FILTER_POINT
TEXTURE_FILTER_BILINEAR
TEXTURE_FILTER_TRILINEAR
TEXTURE_FILTER_ANISOTROPIC_4X
TEXTURE_FILTER_ANISOTROPIC_8X
TEXTURE_FILTER_ANISOTROPIC_16X
TEXTURE_WRAP_REPEAT
TEXTURE_WRAP_CLAMP
TEXTURE_WRAP_MIRROR_REPEAT
TEXTURE_WRAP_MIRROR_CLAMP
CUBEMAP_LAYOUT_AUTO_DETECT
CUBEMAP_LAYOUT_LINE_VERTICAL
CUBEMAP_LAYOUT_LINE_HORIZONTAL
CUBEMAP_LAYOUT_CROSS_THREE_BY_FOUR
CUBEMAP_LAYOUT_CROSS_FOUR_BY_THREE
FONT_DEFAULT
FONT_BITMAP
FONT_SDF
BLEND_ALPHA
BLEND_ADDITIVE
BLEND_MULTIPLIED
BLEND_ADD_COLORS
BLEND_SUBTRACT_COLORS
BLEND_ALPHA_PREMULTIPLY
BLEND_CUSTOM
BLEND_CUSTOM_SEPARATE
GESTURE_NONE
GESTURE_TAP
GESTURE_DOUBLETAP
GESTURE_HOLD
GESTURE_DRAG
GESTURE_SWIPE_RIGHT
GESTURE_SWIPE_LEFT
GESTURE_SWIPE_UP
GESTURE_SWIPE_DOWN
GESTURE_PINCH_IN
GESTURE_PINCH_OUT
CAMERA_CUSTOM
CAMERA_FREE
CAMERA_ORBITAL
CAMERA_FIRST_PERSON
CAMERA_THIRD_PERSON
CAMERA_PERSPECTIVE
CAMERA_ORTHOGRAPHIC
NPATCH_NINE_PATCH
NPATCH_THREE_PATCH_VERTICAL
NPATCH_THREE_PATCH_HORIZONTAL
@Types
Vector2
Vector3
Vector4
Quaternion
Matrix
Color
Rectangle
Image
Texture
Texture2D
TextureCubemap
RenderTexture
RenderTexture2D
NPatchInfo
GlyphInfo
Font
Camera3D
Camera
Camera2D
Mesh
Shader
MaterialMap
Material
Transform
BoneInfo
Model
ModelAnimation
Ray
RayCollision
BoundingBox
Wave
rAudioBuffer
rAudioProcessor
AudioStream
Sound
Music
VrDeviceInfo
VrStereoConfig
FilePathList
AutomationEvent
AutomationEventList
ConfigFlags
TraceLogLevel
KeyboardKey
MouseButton
MouseCursor
GamepadButton
GamepadAxis
MaterialMapIndex
ShaderLocationIndex
ShaderUniformDataType
ShaderAttributeDataType
PixelFormat
TextureFilter
TextureWrap
CubemapLayout
FontType
BlendMode
Gesture
CameraMode
CameraProjection
NPatchLayout
TraceLogCallback
LoadFileDataCallback
SaveFileDataCallback
LoadFileTextCallback
SaveFileTextCallback
AudioCallback
@Functions
RL_MALLOC(sz)
RL_CALLOC(n,sz)
RL_REALLOC(ptr,sz)
RL_FREE(ptr)
CLITERAL(type)
void InitWindow(int width, int height, const char *title)
void CloseWindow(void)
bool WindowShouldClose(void)
bool IsWindowReady(void)
bool IsWindowFullscreen(void)
bool IsWindowHidden(void)
bool IsWindowMinimized(void)
bool IsWindowMaximized(void)
bool IsWindowFocused(void)
bool IsWindowResized(void)
bool IsWindowState(unsigned int flag)
void SetWindowState(unsigned int flags)
void ClearWindowState(unsigned int flags)
void ToggleFullscreen(void)
void ToggleBorderlessWindowed(void)
void MaximizeWindow(void)
void MinimizeWindow(void)
void RestoreWindow(void)
void SetWindowIcon(Image image)
void SetWindowIcons(Image *images, int count)
void SetWindowTitle(const char *title)
void SetWindowPosition(int x, int y)
void SetWindowMonitor(int monitor)
void SetWindowMinSize(int width, int height)
void SetWindowMaxSize(int width, int height)
void SetWindowSize(int width, int height)
void SetWindowOpacity(float opacity)
void SetWindowFocused(void)
void *GetWindowHandle(void)
int GetScreenWidth(void)
int GetScreenHeight(void)
int GetRenderWidth(void)
int GetRenderHeight(void)
int GetMonitorCount(void)
int GetCurrentMonitor(void)
Vector2 GetMonitorPosition(int monitor)
int GetMonitorWidth(int monitor)
int GetMonitorHeight(int monitor)
int GetMonitorPhysicalWidth(int monitor)
int GetMonitorPhysicalHeight(int monitor)
int GetMonitorRefreshRate(int monitor)
Vector2 GetWindowPosition(void)
Vector2 GetWindowScaleDPI(void)
const char *GetMonitorName(int monitor)
void SetClipboardText(const char *text)
const char *GetClipboardText(void)
Image GetClipboardImage(void)
void EnableEventWaiting(void)
void DisableEventWaiting(void)
void ShowCursor(void)
void HideCursor(void)
bool IsCursorHidden(void)
void EnableCursor(void)
void DisableCursor(void)
bool IsCursorOnScreen(void)
void ClearBackground(Color color)
void BeginDrawing(void)
void EndDrawing(void)
void BeginMode2D(Camera2D camera)
void EndMode2D(void)
void BeginMode3D(Camera3D camera)
void EndMode3D(void)
void BeginTextureMode(RenderTexture2D target)
void EndTextureMode(void)
void BeginShaderMode(Shader shader)
void EndShaderMode(void)
void BeginBlendMode(int mode)
void EndBlendMode(void)
void BeginScissorMode(int x, int y, int width, int height)
void EndScissorMode(void)
void BeginVrStereoMode(VrStereoConfig config)
void EndVrStereoMode(void)
VrStereoConfig LoadVrStereoConfig(VrDeviceInfo device)
void UnloadVrStereoConfig(VrStereoConfig config)
Shader LoadShader(const char *vsFileName, const char *fsFileName)
Shader LoadShaderFromMemory(const char *vsCode, const char *fsCode)
bool IsShaderValid(Shader shader)
int GetShaderLocation(Shader shader, const char *uniformName)
int GetShaderLocationAttrib(Shader shader, const char *attribName)
void SetShaderValue(Shader shader, int locIndex, const void *value, int uniformType)
void SetShaderValueV(Shader shader, int locIndex, const void *value, int uniformType, int count)
void SetShaderValueMatrix(Shader shader, int locIndex, Matrix mat)
void SetShaderValueTexture(Shader shader, int locIndex, Texture2D texture)
void UnloadShader(Shader shader)
Ray GetScreenToWorldRay(Vector2 position, Camera camera)
Ray GetScreenToWorldRayEx(Vector2 position, Camera camera, int width, int height)
Vector2 GetWorldToScreen(Vector3 position, Camera camera)
Vector2 GetWorldToScreenEx(Vector3 position, Camera camera, int width, int height)
Vector2 GetWorldToScreen2D(Vector2 position, Camera2D camera)
Vector2 GetScreenToWorld2D(Vector2 position, Camera2D camera)
Matrix GetCameraMatrix(Camera camera)
Matrix GetCameraMatrix2D(Camera2D camera)
void SetTargetFPS(int fps)
float GetFrameTime(void)
double GetTime(void)
int GetFPS(void)
void SwapScreenBuffer(void)
void PollInputEvents(void)
void WaitTime(double seconds)
void SetRandomSeed(unsigned int seed)
int GetRandomValue(int min, int max)
int *LoadRandomSequence(unsigned int count, int min, int max)
void UnloadRandomSequence(int *sequence)
void TakeScreenshot(const char *fileName)
void SetConfigFlags(unsigned int flags)
void OpenURL(const char *url)
void TraceLog(int logLevel, const char *text, ...)
void SetTraceLogLevel(int logLevel)
void *MemAlloc(unsigned int size)
void *MemRealloc(void *ptr, unsigned int size)
void MemFree(void *ptr)
void SetTraceLogCallback(TraceLogCallback callback)
void SetLoadFileDataCallback(LoadFileDataCallback callback)
void SetSaveFileDataCallback(SaveFileDataCallback callback)
void SetLoadFileTextCallback(LoadFileTextCallback callback)
void SetSaveFileTextCallback(SaveFileTextCallback callback)
unsigned char *LoadFileData(const char *fileName, int *dataSize)
void UnloadFileData(unsigned char *data)
bool SaveFileData(const char *fileName, void *data, int dataSize)
bool ExportDataAsCode(const unsigned char *data, int dataSize, const char *fileName)
char *LoadFileText(const char *fileName)
void UnloadFileText(char *text)
bool SaveFileText(const char *fileName, char *text)
bool FileExists(const char *fileName)
bool DirectoryExists(const char *dirPath)
bool IsFileExtension(const char *fileName, const char *ext)
int GetFileLength(const char *fileName)
const char *GetFileExtension(const char *fileName)
const char *GetFileName(const char *filePath)
const char *GetFileNameWithoutExt(const char *filePath)
const char *GetDirectoryPath(const char *filePath)
const char *GetPrevDirectoryPath(const char *dirPath)
const char *GetWorkingDirectory(void)
const char *GetApplicationDirectory(void)
int MakeDirectory(const char *dirPath)
bool ChangeDirectory(const char *dir)
bool IsPathFile(const char *path)
bool IsFileNameValid(const char *fileName)
FilePathList LoadDirectoryFiles(const char *dirPath)
FilePathList LoadDirectoryFilesEx(const char *basePath, const char *filter, bool scanSubdirs)
void UnloadDirectoryFiles(FilePathList files)
bool IsFileDropped(void)
FilePathList LoadDroppedFiles(void)
void UnloadDroppedFiles(FilePathList files)
long GetFileModTime(const char *fileName)
unsigned char *CompressData(const unsigned char *data, int dataSize, int *compDataSize)
unsigned char *DecompressData(const unsigned char *compData, int compDataSize, int *dataSize)
char *EncodeDataBase64(const unsigned char *data, int dataSize, int *outputSize)
unsigned char *DecodeDataBase64(const unsigned char *data, int *outputSize)
unsigned int ComputeCRC32(unsigned char *data, int dataSize)
unsigned int *ComputeMD5(unsigned char *data, int dataSize)
unsigned int *ComputeSHA1(unsigned char *data, int dataSize)
AutomationEventList LoadAutomationEventList(const char *fileName)
void UnloadAutomationEventList(AutomationEventList list)
bool ExportAutomationEventList(AutomationEventList list, const char *fileName)
void SetAutomationEventList(AutomationEventList *list)
void SetAutomationEventBaseFrame(int frame)
void StartAutomationEventRecording(void)
void StopAutomationEventRecording(void)
void PlayAutomationEvent(AutomationEvent event)
bool IsKeyPressed(int key)
bool IsKeyPressedRepeat(int key)
bool IsKeyDown(int key)
bool IsKeyReleased(int key)
bool IsKeyUp(int key)
int GetKeyPressed(void)
int GetCharPressed(void)
void SetExitKey(int key)
bool IsGamepadAvailable(int gamepad)
const char *GetGamepadName(int gamepad)
bool IsGamepadButtonPressed(int gamepad, int button)
bool IsGamepadButtonDown(int gamepad, int button)
bool IsGamepadButtonReleased(int gamepad, int button)
bool IsGamepadButtonUp(int gamepad, int button)
int GetGamepadButtonPressed(void)
int GetGamepadAxisCount(int gamepad)
float GetGamepadAxisMovement(int gamepad, int axis)
int SetGamepadMappings(const char *mappings)
void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor, float duration)
bool IsMouseButtonPressed(int button)
bool IsMouseButtonDown(int button)
bool IsMouseButtonReleased(int button)
bool IsMouseButtonUp(int button)
int GetMouseX(void)
int GetMouseY(void)
Vector2 GetMousePosition(void)
Vector2 GetMouseDelta(void)
void SetMousePosition(int x, int y)
void SetMouseOffset(int offsetX, int offsetY)
void SetMouseScale(float scaleX, float scaleY)
float GetMouseWheelMove(void)
Vector2 GetMouseWheelMoveV(void)
void SetMouseCursor(int cursor)
int GetTouchX(void)
int GetTouchY(void)
Vector2 GetTouchPosition(int index)
int GetTouchPointId(int index)
int GetTouchPointCount(void)
void SetGesturesEnabled(unsigned int flags)
bool IsGestureDetected(unsigned int gesture)
int GetGestureDetected(void)
float GetGestureHoldDuration(void)
Vector2 GetGestureDragVector(void)
float GetGestureDragAngle(void)
Vector2 GetGesturePinchVector(void)
float GetGesturePinchAngle(void)
void UpdateCamera(Camera *camera, int mode)
void UpdateCameraPro(Camera *camera, Vector3 movement, Vector3 rotation, float zoom)
void SetShapesTexture(Texture2D texture, Rectangle source)
Texture2D GetShapesTexture(void)
Rectangle GetShapesTextureRectangle(void)
void DrawPixel(int posX, int posY, Color color)
void DrawPixelV(Vector2 position, Color color)
void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color)
void DrawLineV(Vector2 startPos, Vector2 endPos, Color color)
void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color)
void DrawLineStrip(const Vector2 *points, int pointCount, Color color)
void DrawLineBezier(Vector2 startPos, Vector2 endPos, float thick, Color color)
void DrawCircle(int centerX, int centerY, float radius, Color color)
void DrawCircleSector(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color)
void DrawCircleSectorLines(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color)
void DrawCircleGradient(int centerX, int centerY, float radius, Color inner, Color outer)
void DrawCircleV(Vector2 center, float radius, Color color)
void DrawCircleLines(int centerX, int centerY, float radius, Color color)
void DrawCircleLinesV(Vector2 center, float radius, Color color)
void DrawEllipse(int centerX, int centerY, float radiusH, float radiusV, Color color)
void DrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV, Color color)
void DrawRing(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color)
void DrawRingLines(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color)
void DrawRectangle(int posX, int posY, int width, int height, Color color)
void DrawRectangleV(Vector2 position, Vector2 size, Color color)
void DrawRectangleRec(Rectangle rec, Color color)
void DrawRectanglePro(Rectangle rec, Vector2 origin, float rotation, Color color)
void DrawRectangleGradientV(int posX, int posY, int width, int height, Color top, Color bottom)
void DrawRectangleGradientH(int posX, int posY, int width, int height, Color left, Color right)
void DrawRectangleGradientEx(Rectangle rec, Color topLeft, Color bottomLeft, Color topRight, Color bottomRight)
void DrawRectangleLines(int posX, int posY, int width, int height, Color color)
void DrawRectangleLinesEx(Rectangle rec, float lineThick, Color color)
void DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color)
void DrawRectangleRoundedLines(Rectangle rec, float roundness, int segments, Color color)
void DrawRectangleRoundedLinesEx(Rectangle rec, float roundness, int segments, float lineThick, Color color)
void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color)
void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color)
void DrawTriangleFan(const Vector2 *points, int pointCount, Color color)
void DrawTriangleStrip(const Vector2 *points, int pointCount, Color color)
void DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color)
void DrawPolyLines(Vector2 center, int sides, float radius, float rotation, Color color)
void DrawPolyLinesEx(Vector2 center, int sides, float radius, float rotation, float lineThick, Color color)
void DrawSplineLinear(const Vector2 *points, int pointCount, float thick, Color color)
void DrawSplineBasis(const Vector2 *points, int pointCount, float thick, Color color)
void DrawSplineCatmullRom(const Vector2 *points, int pointCount, float thick, Color color)
void DrawSplineBezierQuadratic(const Vector2 *points, int pointCount, float thick, Color color)
void DrawSplineBezierCubic(const Vector2 *points, int pointCount, float thick, Color color)
void DrawSplineSegmentLinear(Vector2 p1, Vector2 p2, float thick, Color color)
void DrawSplineSegmentBasis(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, float thick, Color color)
void DrawSplineSegmentCatmullRom(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, float thick, Color color)
void DrawSplineSegmentBezierQuadratic(Vector2 p1, Vector2 c2, Vector2 p3, float thick, Color color)
void DrawSplineSegmentBezierCubic(Vector2 p1, Vector2 c2, Vector2 c3, Vector2 p4, float thick, Color color)
Vector2 GetSplinePointLinear(Vector2 startPos, Vector2 endPos, float t)
Vector2 GetSplinePointBasis(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, float t)
Vector2 GetSplinePointCatmullRom(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, float t)
Vector2 GetSplinePointBezierQuad(Vector2 p1, Vector2 c2, Vector2 p3, float t)
Vector2 GetSplinePointBezierCubic(Vector2 p1, Vector2 c2, Vector2 c3, Vector2 p4, float t)
bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2)
bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2)
bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec)
bool CheckCollisionCircleLine(Vector2 center, float radius, Vector2 p1, Vector2 p2)
bool CheckCollisionPointRec(Vector2 point, Rectangle rec)
bool CheckCollisionPointCircle(Vector2 point, Vector2 center, float radius)
bool CheckCollisionPointTriangle(Vector2 point, Vector2 p1, Vector2 p2, Vector2 p3)
bool CheckCollisionPointLine(Vector2 point, Vector2 p1, Vector2 p2, int threshold)
bool CheckCollisionPointPoly(Vector2 point, const Vector2 *points, int pointCount)
bool CheckCollisionLines(Vector2 startPos1, Vector2 endPos1, Vector2 startPos2, Vector2 endPos2, Vector2 *collisionPoint)
Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2)
Image LoadImage(const char *fileName)
Image LoadImageRaw(const char *fileName, int width, int height, int format, int headerSize)
Image LoadImageAnim(const char *fileName, int *frames)
Image LoadImageAnimFromMemory(const char *fileType, const unsigned char *fileData, int dataSize, int *frames)
Image LoadImageFromMemory(const char *fileType, const unsigned char *fileData, int dataSize)
Image LoadImageFromTexture(Texture2D texture)
Image LoadImageFromScreen(void)
bool IsImageValid(Image image)
void UnloadImage(Image image)
bool ExportImage(Image image, const char *fileName)
unsigned char *ExportImageToMemory(Image image, const char *fileType, int *fileSize)
bool ExportImageAsCode(Image image, const char *fileName)
Image GenImageColor(int width, int height, Color color)
Image GenImageGradientLinear(int width, int height, int direction, Color start, Color end)
Image GenImageGradientRadial(int width, int height, float density, Color inner, Color outer)
Image GenImageGradientSquare(int width, int height, float density, Color inner, Color outer)
Image GenImageChecked(int width, int height, int checksX, int checksY, Color col1, Color col2)
Image GenImageWhiteNoise(int width, int height, float factor)
Image GenImagePerlinNoise(int width, int height, int offsetX, int offsetY, float scale)
Image GenImageCellular(int width, int height, int tileSize)
Image GenImageText(int width, int height, const char *text)
Image ImageCopy(Image image)
Image ImageFromImage(Image image, Rectangle rec)
Image ImageFromChannel(Image image, int selectedChannel)
Image ImageText(const char *text, int fontSize, Color color)
Image ImageTextEx(Font font, const char *text, float fontSize, float spacing, Color tint)
void ImageFormat(Image *image, int newFormat)
void ImageToPOT(Image *image, Color fill)
void ImageCrop(Image *image, Rectangle crop)
void ImageAlphaCrop(Image *image, float threshold)
void ImageAlphaClear(Image *image, Color color, float threshold)
void ImageAlphaMask(Image *image, Image alphaMask)
void ImageAlphaPremultiply(Image *image)
void ImageBlurGaussian(Image *image, int blurSize)
void ImageKernelConvolution(Image *image, const float *kernel, int kernelSize)
void ImageResize(Image *image, int newWidth, int newHeight)
void ImageResizeNN(Image *image, int newWidth,int newHeight)
void ImageResizeCanvas(Image *image, int newWidth, int newHeight, int offsetX, int offsetY, Color fill)
void ImageMipmaps(Image *image)
void ImageDither(Image *image, int rBpp, int gBpp, int bBpp, int aBpp)
void ImageFlipVertical(Image *image)
void ImageFlipHorizontal(Image *image)
void ImageRotate(Image *image, int degrees)
void ImageRotateCW(Image *image)
void ImageRotateCCW(Image *image)
void ImageColorTint(Image *image, Color color)
void ImageColorInvert(Image *image)
void ImageColorGrayscale(Image *image)
void ImageColorContrast(Image *image, float contrast)
void ImageColorBrightness(Image *image, int brightness)
void ImageColorReplace(Image *image, Color color, Color replace)
Color *LoadImageColors(Image image)
Color *LoadImagePalette(Image image, int maxPaletteSize, int *colorCount)
void UnloadImageColors(Color *colors)
void UnloadImagePalette(Color *colors)
Rectangle GetImageAlphaBorder(Image image, float threshold)
Color GetImageColor(Image image, int x, int y)
void ImageClearBackground(Image *dst, Color color)
void ImageDrawPixel(Image *dst, int posX, int posY, Color color)
void ImageDrawPixelV(Image *dst, Vector2 position, Color color)
void ImageDrawLine(Image *dst, int startPosX, int startPosY, int endPosX, int endPosY, Color color)
void ImageDrawLineV(Image *dst, Vector2 start, Vector2 end, Color color)
void ImageDrawLineEx(Image *dst, Vector2 start, Vector2 end, int thick, Color color)
void ImageDrawCircle(Image *dst, int centerX, int centerY, int radius, Color color)
void ImageDrawCircleV(Image *dst, Vector2 center, int radius, Color color)
void ImageDrawCircleLines(Image *dst, int centerX, int centerY, int radius, Color color)
void ImageDrawCircleLinesV(Image *dst, Vector2 center, int radius, Color color)
void ImageDrawRectangle(Image *dst, int posX, int posY, int width, int height, Color color)
void ImageDrawRectangleV(Image *dst, Vector2 position, Vector2 size, Color color)
void ImageDrawRectangleRec(Image *dst, Rectangle rec, Color color)
void ImageDrawRectangleLines(Image *dst, Rectangle rec, int thick, Color color)
void ImageDrawTriangle(Image *dst, Vector2 v1, Vector2 v2, Vector2 v3, Color color)
void ImageDrawTriangleEx(Image *dst, Vector2 v1, Vector2 v2, Vector2 v3, Color c1, Color c2, Color c3)
void ImageDrawTriangleLines(Image *dst, Vector2 v1, Vector2 v2, Vector2 v3, Color color)
void ImageDrawTriangleFan(Image *dst, Vector2 *points, int pointCount, Color color)
void ImageDrawTriangleStrip(Image *dst, Vector2 *points, int pointCount, Color color)
void ImageDraw(Image *dst, Image src, Rectangle srcRec, Rectangle dstRec, Color tint)
void ImageDrawText(Image *dst, const char *text, int posX, int posY, int fontSize, Color color)
void ImageDrawTextEx(Image *dst, Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint)
Texture2D LoadTexture(const char *fileName)
Texture2D LoadTextureFromImage(Image image)
TextureCubemap LoadTextureCubemap(Image image, int layout)
RenderTexture2D LoadRenderTexture(int width, int height)
bool IsTextureValid(Texture2D texture)
void UnloadTexture(Texture2D texture)
bool IsRenderTextureValid(RenderTexture2D target)
void UnloadRenderTexture(RenderTexture2D target)
void UpdateTexture(Texture2D texture, const void *pixels)
void UpdateTextureRec(Texture2D texture, Rectangle rec, const void *pixels)
void GenTextureMipmaps(Texture2D *texture)
void SetTextureFilter(Texture2D texture, int filter)
void SetTextureWrap(Texture2D texture, int wrap)
void DrawTexture(Texture2D texture, int posX, int posY, Color tint)
void DrawTextureV(Texture2D texture, Vector2 position, Color tint)
void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint)
void DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position, Color tint)
void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint)
void DrawTextureNPatch(Texture2D texture, NPatchInfo nPatchInfo, Rectangle dest, Vector2 origin, float rotation, Color tint)
bool ColorIsEqual(Color col1, Color col2)
Color Fade(Color color, float alpha)
int ColorToInt(Color color)
Vector4 ColorNormalize(Color color)
Color ColorFromNormalized(Vector4 normalized)
Vector3 ColorToHSV(Color color)
Color ColorFromHSV(float hue, float saturation, float value)
Color ColorTint(Color color, Color tint)
Color ColorBrightness(Color color, float factor)
Color ColorContrast(Color color, float contrast)
Color ColorAlpha(Color color, float alpha)
Color ColorAlphaBlend(Color dst, Color src, Color tint)
Color ColorLerp(Color color1, Color color2, float factor)
Color GetColor(unsigned int hexValue)
Color GetPixelColor(void *srcPtr, int format)
void SetPixelColor(void *dstPtr, Color color, int format)
int GetPixelDataSize(int width, int height, int format)
Font GetFontDefault(void)
Font LoadFont(const char *fileName)
Font LoadFontEx(const char *fileName, int fontSize, int *codepoints, int codepointCount)
Font LoadFontFromImage(Image image, Color key, int firstChar)
Font LoadFontFromMemory(const char *fileType, const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount)
bool IsFontValid(Font font)
GlyphInfo *LoadFontData(const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount, int type)
Image GenImageFontAtlas(const GlyphInfo *glyphs, Rectangle **glyphRecs, int glyphCount, int fontSize, int padding, int packMethod)
void UnloadFontData(GlyphInfo *glyphs, int glyphCount)
void UnloadFont(Font font)
bool ExportFontAsCode(Font font, const char *fileName)
void DrawFPS(int posX, int posY)
void DrawText(const char *text, int posX, int posY, int fontSize, Color color)
void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint)
void DrawTextPro(Font font, const char *text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint)
void DrawTextCodepoint(Font font, int codepoint, Vector2 position, float fontSize, Color tint)
void DrawTextCodepoints(Font font, const int *codepoints, int codepointCount, Vector2 position, float fontSize, float spacing, Color tint)
void SetTextLineSpacing(int spacing)
int MeasureText(const char *text, int fontSize)
Vector2 MeasureTextEx(Font font, const char *text, float fontSize, float spacing)
int GetGlyphIndex(Font font, int codepoint)
GlyphInfo GetGlyphInfo(Font font, int codepoint)
Rectangle GetGlyphAtlasRec(Font font, int codepoint)
char *LoadUTF8(const int *codepoints, int length)
void UnloadUTF8(char *text)
int *LoadCodepoints(const char *text, int *count)
void UnloadCodepoints(int *codepoints)
int GetCodepointCount(const char *text)
int GetCodepoint(const char *text, int *codepointSize)
int GetCodepointNext(const char *text, int *codepointSize)
int GetCodepointPrevious(const char *text, int *codepointSize)
const char *CodepointToUTF8(int codepoint, int *utf8Size)
int TextCopy(char *dst, const char *src)
bool TextIsEqual(const char *text1, const char *text2)
unsigned int TextLength(const char *text)
const char *TextFormat(const char *text, ...)
const char *TextSubtext(const char *text, int position, int length)
char *TextReplace(const char *text, const char *replace, const char *by)
char *TextInsert(const char *text, const char *insert, int position)
const char *TextJoin(const char **textList, int count, const char *delimiter)
const char **TextSplit(const char *text, char delimiter, int *count)
void TextAppend(char *text, const char *append, int *position)
int TextFindIndex(const char *text, const char *find)
const char *TextToUpper(const char *text)
const char *TextToLower(const char *text)
const char *TextToPascal(const char *text)
const char *TextToSnake(const char *text)
const char *TextToCamel(const char *text)
int TextToInteger(const char *text)
float TextToFloat(const char *text)
void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color)
void DrawPoint3D(Vector3 position, Color color)
void DrawCircle3D(Vector3 center, float radius, Vector3 rotationAxis, float rotationAngle, Color color)
void DrawTriangle3D(Vector3 v1, Vector3 v2, Vector3 v3, Color color)
void DrawTriangleStrip3D(const Vector3 *points, int pointCount, Color color)
void DrawCube(Vector3 position, float width, float height, float length, Color color)
void DrawCubeV(Vector3 position, Vector3 size, Color color)
void DrawCubeWires(Vector3 position, float width, float height, float length, Color color)
void DrawCubeWiresV(Vector3 position, Vector3 size, Color color)
void DrawSphere(Vector3 centerPos, float radius, Color color)
void DrawSphereEx(Vector3 centerPos, float radius, int rings, int slices, Color color)
void DrawSphereWires(Vector3 centerPos, float radius, int rings, int slices, Color color)
void DrawCylinder(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color)
void DrawCylinderEx(Vector3 startPos, Vector3 endPos, float startRadius, float endRadius, int sides, Color color)
void DrawCylinderWires(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color)
void DrawCylinderWiresEx(Vector3 startPos, Vector3 endPos, float startRadius, float endRadius, int sides, Color color)
void DrawCapsule(Vector3 startPos, Vector3 endPos, float radius, int slices, int rings, Color color)
void DrawCapsuleWires(Vector3 startPos, Vector3 endPos, float radius, int slices, int rings, Color color)
void DrawPlane(Vector3 centerPos, Vector2 size, Color color)
void DrawRay(Ray ray, Color color)
void DrawGrid(int slices, float spacing)
Model LoadModel(const char *fileName)
Model LoadModelFromMesh(Mesh mesh)
bool IsModelValid(Model model)
void UnloadModel(Model model)
BoundingBox GetModelBoundingBox(Model model)
void DrawModel(Model model, Vector3 position, float scale, Color tint)
void DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint)
void DrawModelWires(Model model, Vector3 position, float scale, Color tint)
void DrawModelWiresEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint)
void DrawModelPoints(Model model, Vector3 position, float scale, Color tint)
void DrawModelPointsEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint)
void DrawBoundingBox(BoundingBox box, Color color)
void DrawBillboard(Camera camera, Texture2D texture, Vector3 position, float scale, Color tint)
void DrawBillboardRec(Camera camera, Texture2D texture, Rectangle source, Vector3 position, Vector2 size, Color tint)
void DrawBillboardPro(Camera camera, Texture2D texture, Rectangle source, Vector3 position, Vector3 up, Vector2 size, Vector2 origin, float rotation, Color tint)
void UploadMesh(Mesh *mesh, bool dynamic)
void UpdateMeshBuffer(Mesh mesh, int index, const void *data, int dataSize, int offset)
void UnloadMesh(Mesh mesh)
void DrawMesh(Mesh mesh, Material material, Matrix transform)
void DrawMeshInstanced(Mesh mesh, Material material, const Matrix *transforms, int instances)
BoundingBox GetMeshBoundingBox(Mesh mesh)
void GenMeshTangents(Mesh *mesh)
bool ExportMesh(Mesh mesh, const char *fileName)
bool ExportMeshAsCode(Mesh mesh, const char *fileName)
Mesh GenMeshPoly(int sides, float radius)
Mesh GenMeshPlane(float width, float length, int resX, int resZ)
Mesh GenMeshCube(float width, float height, float length)
Mesh GenMeshSphere(float radius, int rings, int slices)
Mesh GenMeshHemiSphere(float radius, int rings, int slices)
Mesh GenMeshCylinder(float radius, float height, int slices)
Mesh GenMeshCone(float radius, float height, int slices)
Mesh GenMeshTorus(float radius, float size, int radSeg, int sides)
Mesh GenMeshKnot(float radius, float size, int radSeg, int sides)
Mesh GenMeshHeightmap(Image heightmap, Vector3 size)
Mesh GenMeshCubicmap(Image cubicmap, Vector3 cubeSize)
Material *LoadMaterials(const char *fileName, int *materialCount)
Material LoadMaterialDefault(void)
bool IsMaterialValid(Material material)
void UnloadMaterial(Material material)
void SetMaterialTexture(Material *material, int mapType, Texture2D texture)
void SetModelMeshMaterial(Model *model, int meshId, int materialId)
ModelAnimation *LoadModelAnimations(const char *fileName, int *animCount)
void UpdateModelAnimation(Model model, ModelAnimation anim, int frame)
void UpdateModelAnimationBones(Model model, ModelAnimation anim, int frame)
void UnloadModelAnimation(ModelAnimation anim)
void UnloadModelAnimations(ModelAnimation *animations, int animCount)
bool IsModelAnimationValid(Model model, ModelAnimation anim)
bool CheckCollisionSpheres(Vector3 center1, float radius1, Vector3 center2, float radius2)
bool CheckCollisionBoxes(BoundingBox box1, BoundingBox box2)
bool CheckCollisionBoxSphere(BoundingBox box, Vector3 center, float radius)
RayCollision GetRayCollisionSphere(Ray ray, Vector3 center, float radius)
RayCollision GetRayCollisionBox(Ray ray, BoundingBox box)
RayCollision GetRayCollisionMesh(Ray ray, Mesh mesh, Matrix transform)
RayCollision GetRayCollisionTriangle(Ray ray, Vector3 p1, Vector3 p2, Vector3 p3)
RayCollision GetRayCollisionQuad(Ray ray, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4)
void InitAudioDevice(void)
void CloseAudioDevice(void)
bool IsAudioDeviceReady(void)
void SetMasterVolume(float volume)
float GetMasterVolume(void)
Wave LoadWave(const char *fileName)
Wave LoadWaveFromMemory(const char *fileType, const unsigned char *fileData, int dataSize)
bool IsWaveValid(Wave wave)
Sound LoadSound(const char *fileName)
Sound LoadSoundFromWave(Wave wave)
Sound LoadSoundAlias(Sound source)
bool IsSoundValid(Sound sound)
void UpdateSound(Sound sound, const void *data, int sampleCount)
void UnloadWave(Wave wave)
void UnloadSound(Sound sound)
void UnloadSoundAlias(Sound alias)
bool ExportWave(Wave wave, const char *fileName)
bool ExportWaveAsCode(Wave wave, const char *fileName)
void PlaySound(Sound sound)
void StopSound(Sound sound)
void PauseSound(Sound sound)
void ResumeSound(Sound sound)
bool IsSoundPlaying(Sound sound)
void SetSoundVolume(Sound sound, float volume)
void SetSoundPitch(Sound sound, float pitch)
void SetSoundPan(Sound sound, float pan)
Wave WaveCopy(Wave wave)
void WaveCrop(Wave *wave, int initFrame, int finalFrame)
void WaveFormat(Wave *wave, int sampleRate, int sampleSize, int channels)
float *LoadWaveSamples(Wave wave)
void UnloadWaveSamples(float *samples)
Music LoadMusicStream(const char *fileName)
Music LoadMusicStreamFromMemory(const char *fileType, const unsigned char *data, int dataSize)
bool IsMusicValid(Music music)
void UnloadMusicStream(Music music)
void PlayMusicStream(Music music)
bool IsMusicStreamPlaying(Music music)
void UpdateMusicStream(Music music)
void StopMusicStream(Music music)
void PauseMusicStream(Music music)
void ResumeMusicStream(Music music)
void SeekMusicStream(Music music, float position)
void SetMusicVolume(Music music, float volume)
void SetMusicPitch(Music music, float pitch)
void SetMusicPan(Music music, float pan)
float GetMusicTimeLength(Music music)
float GetMusicTimePlayed(Music music)
AudioStream LoadAudioStream(unsigned int sampleRate, unsigned int sampleSize, unsigned int channels)
bool IsAudioStreamValid(AudioStream stream)
void UnloadAudioStream(AudioStream stream)
void UpdateAudioStream(AudioStream stream, const void *data, int frameCount)
bool IsAudioStreamProcessed(AudioStream stream)
void PlayAudioStream(AudioStream stream)
void PauseAudioStream(AudioStream stream)
void ResumeAudioStream(AudioStream stream)
bool IsAudioStreamPlaying(AudioStream stream)
void StopAudioStream(AudioStream stream)
void SetAudioStreamVolume(AudioStream stream, float volume)
void SetAudioStreamPitch(AudioStream stream, float pitch)
void SetAudioStreamPan(AudioStream stream, float pan)
void SetAudioStreamBufferSizeDefault(int size)
void SetAudioStreamCallback(AudioStream stream, AudioCallback callback)
void AttachAudioStreamProcessor(AudioStream stream, AudioCallback processor)
void DetachAudioStreamProcessor(AudioStream stream, AudioCallback processor)
void AttachAudioMixedProcessor(AudioCallback processor)
void DetachAudioMixedProcessor(AudioCallback processor)
*/
