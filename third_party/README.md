# third_party Folder
This folder contains (or needs to be made to contain) copies of all third party library headers and source files. Some of these libraries are included directly in the git repository, others need to be downloaded into this folder manually before compilation. Refer to the list below to figure out which libraries are needed for which pieces of the repository, and which ones you'll need to download manually after cloning.

---

# List of Libraries

## stb
**URL(s):** [www.github.com/nothings/stb](https://www.github.com/nothings/stb)

**Folder Name:** `stb`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Commit `5c205738c191bcb0abc65c4febfa9bd25ff35234` from Nov 8th 2024

**Fixup Required:** Delete all non-essential files after cloning (everything that's not `stb_*.h`)

**Required For:**

* Compiling for Playdate and WebAssembly we use `stb_sprintf.h` as a printf implementation

---

## raylib 5.5
**URL(s):** [www.raylib.com](https://www.raylib.com/) [github.com/raysan5/raylib](https://github.com/raysan5/raylib)

**Folder Name:** `raylib`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** [v5.5](https://github.com/raysan5/raylib/releases/tag/5.5) from Nov 18 2024

**Fixup Required:** Copy files from `lib/` to both `_lib_debug/` and `_lib_release/` folders in `third_party/` then copy `include/` folder contents into `third_party/raylib`

**Required For:**

* Building `test.exe` into a raylib based graphical application (only if `BUILD_WITH_RAYLIB` is enabled in `build_config.h`)

---

## Box2D 3.0
**URL(s):** [box2d.org](https://box2d.org/) [github.com/erincatto/box2d](https://github.com/erincatto/box2d)

**Folder Name:** `box2d`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** [v3.0.0](https://github.com/erincatto/box2d/releases/tag/v3.0.0) from Aug 11 2024

**Fixup Required:** Compile `box2d.lib` in debug and release and add to `_lib_debug/` and `_lib_release/`. Copy `include/box2d/` folder contents into `third_party/box2d`

**Required For:**

* Building tests.exe with a little physics test (only if `BUILD_WITH_BOX2D` is enabled in `build_config.h`, also only renders if raylib is also enabled)

---

## Sokol
**URL(s):** [github.com/floooh/sokol](https://github.com/floooh/sokol)

**Folder Name:** `sokol`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Commit `c1cc713a48669fb78c8fadc1a3cb9dd6c3bb97d3` from Dec 29 2024

**Fixup Required:** Delete all non-essential files after cloning (everything  that's not soko_*.h)

**Required For:**

* Building tests.exe into a little graphical window application (only if `BUILD_WITH_SOKOL` is enabled in `build_config.h`)

---

## GLAD (OpenGL ES)
**URL(s):** [https://glad.dav1d.de/](https://glad.dav1d.de/)

**Folder Name:** `glad_es`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Generated using the following settings
	* Language: C/C++
	* Specification: OpenGL
	* Profile: Compatibility
	* gl: None
	* gles1: None
	* gles2: Version 3.0
	* glsc2: None
	* All Extensions

**Fixup Required:** Flatten 3 files to glad folder. Change `#include <KHR/khrplatform.h>` to `#include <glad/khrplatform.h>` in `glad.h` or put `khrplatform.h` in a separate `KHR` folder

**Required For:**

* Building tests.exe into a graphical web application (only if `BUILD_WEB` and `BUILD_WITH_SOKOL` are enabled in `build_config.h`)

---

## SDL 2.30.11
**URL(s):** [https://github.com/libsdl-org/SDL/releases/tag/release-2.30.11](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.11)

**Folder Name:** `SDL`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Release 2.30.11 from Jan 1st 2025

**Fixup Required:** Add contents of include folder to SDL folder. Compile SDL in both Debug and Release x64 modes and copy both `SDL2.lib` versions to `_lib_debug/` and `_lib_release/` folders (also copy `SDL2.pdb` for debug). Copy SDL2.dll to `_build` folder

**Required For:**

* Building tests.exe into a little graphical window application (only if `BUILD_WITH_SDL` is enabled in `build_config.h`)



## HandmadeMath.h
**URL(s):** [https://github.com/HandmadeMath/HandmadeMath/releases/tag/v2.0.0](https://github.com/HandmadeMath/HandmadeMath/releases/tag/v2.0.0)

**Folder Name:** `handmade_math`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** v2.0.0 from Feb 20th 2023

**Fixup Required:** Simply copy HandmadeMath.h into the folder

**Required For:**

* struct_vectors.h, struct_matrices.h, and struct_quaternion.h depend on HandmadeMath.h



## meow_hash 0.5
**URL(s):** [https://github.com/cmuratori/meow_hash](https://github.com/cmuratori/meow_hash) [https://www.computerenhance.com/p/meowhash](https://www.computerenhance.com/p/meowhash)

**Folder Name:** `meow_hash`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Commit `b080caa7e51576fe3151c8976110df7966fa6a38` from Jul 2nd 2021

**Fixup Required:** Delete everything besides meow_hash_x64_aesni.h, LICENSE, and README.md

**Required For:**

* misc_hash.h depends on meow_hash

---

## miniz 3.0.2
**URL(s):** [https://github.com/richgel999/miniz](https://github.com/richgel999/miniz) [Release 3.0.2](https://github.com/richgel999/miniz/releases/tag/3.0.2)

**Folder Name:** `miniz`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Release 3.0.2 from Jan 15th 2023

**Fixup Required:** Delete everything besides miniz.h, miniz.c and LICENSE

**Required For:**

* misc_zip.h depends on miniz to parse .zip files

---

## OpenVR SDK 2.5.1
**URL(s):** [https://github.com/ValveSoftware/openvr](https://github.com/ValveSoftware/openvr) [Release 2.5.1](https://github.com/ValveSoftware/openvr/releases/tag/v2.5.1)

**Folder Name:** `openvr`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Release 2.5.1 from Mar 26th 2024

**Fixup Required:** Copy the contents of the `header/` folder and delete everything else (besides the LICENSE)

**Required For:**

* Building tests.exe into a small VR application (only if `BUILD_WITH_VR` is enabled in `build_config.h`)

---

## Super Light Regular Expression libraru (SLRE)
**URL(s):** [https://github.com/cesanta/slre](https://github.com/cesanta/slre)

**Folder Name:** `slre`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Commit `9075c67cad47d62ba4a4f8f452ae46bb21124f7b` from Oct 26th 2020

**Fixup Required:** Delete everything besides slre.h, slre.c and LICENSE

**Required For:**

* misc_regex.h depends upon this library

---

## CglTF v1.15
**URL(s):** [https://github.com/jkuhlmann/cgltf](https://github.com/jkuhlmann/cgltf) [v1.15](https://github.com/jkuhlmann/cgltf/releases/tag/v1.15)

**Folder Name:** `cgltf`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Version 1.15 from Feb 9th 2025

**Fixup Required:** Delete everything besides cgltf.h and LICENSE

**Required For:**

* file_fmt_gltf.h depends upon this library

---

## Clay UI v0.13
**URL(s):** [https://github.com/nicbarker/clay](https://github.com/nicbarker/clay) [v0.13](https://github.com/nicbarker/clay/releases/tag/v0.13)

**Folder Name:** `clay`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Version 0.13 from Feb 11th 2025

**Fixup Required:** Delete everything besides clay.h and LICENSE.md. Add CLAY_DECOR optional macro and use it before all functions that need to be called from other binaries. Fix CLAY__DEFAULT_STRUCT to always be {0}, not {}

**Required For:**

* ui_clay.h depends upon this library

---

## cImGui for Dear ImGui v1.91.8
**URL(s):** [https://github.com/cimgui/cimgui](https://github.com/cimgui/cimgui) [Dear ImGui v1.91.8](https://github.com/ocornut/imgui/tree/11b3a7c8ca23201294464c7f368614a9106af2a1)

**Folder Name:** `cimgui`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** cImGui commit `2e5db87e996af08b8b4162ca324ab0d7964abbf8` from from Feb 5th 2025 which is generated from Dear ImGui Version 1.91.8 from Jan 31st 2025

**Fixup Required:** Compile using CMake or Make and put static libraries in `_lib_debug/` and `_lib_release/`. Delete everything besides cimgui.h and LICENSE.

**Required For:**

* ui_imgui.h depends upon this library
