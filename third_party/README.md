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

**Fixup Required:** Add contents of include folder to SDL folder. Compile SDL in both Debug and Release x64 modes and copy both `SDL2.lib` versions to `_lib_debug/` and `_lib_release/` folders (also copy `SDL2.pdb` for debug). Copy `SDL2.dll` to `_build` folder

**Required For:**

* Building tests.exe into a little graphical window application (only if `BUILD_WITH_SDL` is enabled in `build_config.h`)



## HandmadeMath.h
**URL(s):** [https://github.com/HandmadeMath/HandmadeMath/releases/tag/v2.0.0](https://github.com/HandmadeMath/HandmadeMath/releases/tag/v2.0.0)

**Folder Name:** `handmade_math`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** v2.0.0 from Feb 20th 2023

**Fixup Required:** Simply copy HandmadeMath.h into the folder

**Required For:**

* `struct_vectors.h`, `struct_matrices.h`, and `struct_quaternion.h` depend on `HandmadeMath.h`



## meow_hash 0.5
**URL(s):** [https://github.com/cmuratori/meow_hash](https://github.com/cmuratori/meow_hash) [https://www.computerenhance.com/p/meowhash](https://www.computerenhance.com/p/meowhash)

**Folder Name:** `meow_hash`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Commit `b080caa7e51576fe3151c8976110df7966fa6a38` from Jul 2nd 2021

**Fixup Required:** Delete everything besides `meow_hash_x64_aesni.h`, `LICENSE`, and `README.md`

**Required For:**

* `misc_hash.h` depends on meow_hash

---

## miniz 3.0.2
**URL(s):** [https://github.com/richgel999/miniz](https://github.com/richgel999/miniz) [Release 3.0.2](https://github.com/richgel999/miniz/releases/tag/3.0.2)

**Folder Name:** `miniz`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Release 3.0.2 from Jan 15th 2023

**Fixup Required:** Delete everything besides `miniz.h`, `miniz.c` and `LICENSE`

**Required For:**

* `misc_zip.h` depends on miniz to parse `.zip` files

---

## OpenVR SDK 2.5.1
**URL(s):** [https://github.com/ValveSoftware/openvr](https://github.com/ValveSoftware/openvr) [Release 2.5.1](https://github.com/ValveSoftware/openvr/releases/tag/v2.5.1)

**Folder Name:** `openvr`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Release 2.5.1 from Mar 26th 2024

**Fixup Required:** Copy the contents of the `header/` folder and delete everything else (besides the LICENSE)

**Required For:**

* Building `tests.exe` into a small VR application (only if `BUILD_WITH_VR` is enabled in `build_config.h`)

---

## Super Light Regular Expression library (SLRE)
**URL(s):** [https://github.com/cesanta/slre](https://github.com/cesanta/slre)

**Folder Name:** `slre`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Commit `9075c67cad47d62ba4a4f8f452ae46bb21124f7b` from Oct 26th 2020

**Fixup Required:** Delete everything besides `slre.h`, `slre.c` and `LICENSE`

**Required For:**

* `misc_regex.h` depends upon this library

---

## CglTF v1.15
**URL(s):** [https://github.com/jkuhlmann/cgltf](https://github.com/jkuhlmann/cgltf) [v1.15](https://github.com/jkuhlmann/cgltf/releases/tag/v1.15)

**Folder Name:** `cgltf`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Version 1.15 from Feb 9th 2025

**Fixup Required:** Delete everything besides `cgltf.h` and `LICENSE`

**Required For:**

* `file_fmt_gltf.h` depends upon this library

---

## Clay UI v0.13 (with many modifications)
**URL(s):** [https://github.com/nicbarker/clay](https://github.com/nicbarker/clay) [v0.13](https://github.com/nicbarker/clay/releases/tag/v0.13)

**Folder Name:** `clay`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Version 0.13 from Feb 11th 2025

**Fixup Required:** Delete everything besides clay.h and LICENSE.md. Add CLAY_DECOR optional macro and use it before all functions that need to be called from other binaries. Fix CLAY__DEFAULT_STRUCT to always be {0}, not {}

**Required For:**

* `ui_clay.h` depends upon this library (only if `BUILD_WITH_CLAY` is enabled)

---

## Dear ImGui v1.91.8
**URL(s):** [https://github.com/ocornut/imgui/](https://github.com/ocornut/imgui/) [v1.91.8](https://github.com/ocornut/imgui/tree/11b3a7c8ca23201294464c7f368614a9106af2a1)

**Folder Name:** `imgui`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Version 1.91.8 from Jan 31st 2025

**Fixup Required:** Make custom `imconfig.h` with just an include for `ui/ui_imconfig.h`. Delete everything besides: `imgui.cpp`, `imgui.h`, `imgui_demo.cpp`, `imgui_draw.cpp`, `imgui_internal.h`, `imgui_tables.cpp`, `imgui_widgets.cpp`, `imstb_rectpack.h`, `imstb_textedit.h`, `imstb_truetype.h`, and `LICENSE.txt`.

**Required For:**

* `ui_imgui.h` depends upon this library (only if `BUILD_WITH_IMGUI` is enabled)

---

## cImGui for Dear ImGui v1.91.8
**URL(s):** [https://github.com/cimgui/cimgui](https://github.com/cimgui/cimgui) [Dear ImGui v1.91.8](https://github.com/ocornut/imgui/tree/11b3a7c8ca23201294464c7f368614a9106af2a1)

**Folder Name:** `cimgui`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** cImGui commit `2e5db87e996af08b8b4162ca324ab0d7964abbf8` from from Feb 5th 2025 which is generated from Dear ImGui Version 1.91.8 from Jan 31st 2025

**Fixup Required:** Delete everything besides `cimgui.h`, `cimgui.cpp` and `LICENSE`.

**Required For:**

* `ui_imgui.h` depends upon this library (only if `BUILD_WITH_IMGUI` is enabled)

---

## Tracy v0.12.2
**URL(s):** [https://github.com/wolfpld/tracy](https://github.com/wolfpld/tracy) [v0.12.2](https://github.com/wolfpld/tracy/releases/tag/v0.12.1)

**Folder Name:** `tracy`

**Download Required:** All necassary files for compilation are included in the repository. For analyzing generated profiling information either downloaded the Windows binary from the release page, or compile the Tracy Profiler using the instructions in the `tracy.pdf` - **First Steps** > **Building the server**

**Current Version:** Version 0.12.2 from Jun 25th 2025

**Fixup Required:** TODO: Fill out this section

**Required For:**

* `lib_tracy.h` always includes the `TracyC.h` header, but only actually does anything if `PROFILING_ENABLED` is enabled
* A `tracy.dll/lib/so` can be built from `TracyClient.cpp` and is required for `PROFILING_ENABLED` builds to work

---

## PhysX v5.5.0.e41b928f
**URL(s):** [https://github.com/NVIDIA-Omniverse/PhysX](https://github.com/NVIDIA-Omniverse/PhysX) [v5.5.0](https://github.com/NVIDIA-Omniverse/PhysX/releases/tag/106.4-physx-5.5.0)

**Folder Name:** `physx`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** PhysX SDK Version 5.5.0.e41b928f from Dec 16th 2024

**Fixup Required:** Copy the contents of `physx/include` into `third_party/physx`. Copy build.bat and build_config.h over to root folder of downloaded PhysX SDK source and run build.bat once with DEBUG_BUILD 1 (in build_config.h), copy the `_build/PhysX_static_64.lib/pdb` to `third_party/_lib_debug`, and then once with DEBUG_BUILD 0 (and copy to `third_party/_lib_release`). Our build.bat will take longer than the regular build process with VS, but it helps us ensure the PhysX gets compiled exactly how want it.

**Required For:**

* `phys_physx.h` depends upon this library (only if `BUILD_WITH_PHYSX` is enabled)

---

## Protobuf-C 
**URL(s):** [Github](https://github.com/protobuf-c/protobuf-c) [v1.5.2](https://github.com/protobuf-c/protobuf-c/releases/tag/v1.5.2)

**Folder Name:** `protobuf_c`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** v1.5.2 released on Apr 6th 2025

**Fixup Required:** Copy `protobuf-c/protobuf-c.c` and `protobuf-c/protobuf-c.h` into `third_party/protobuf_c/protobuf-c/` folder. Compile `protobuf-gen-c` for *Linux* (use WSL on Windows) and place into `thid_party/_tools/linux/protoc-gen-c` (run this under WSL whenever we need to regenerate protobuf code)

**Required For:**

* `parse_protobuf.h` if `BUILD_WITH_PROTOBUF` is enabled, allows us to serialize/deserialize protobuf files/streams in C

---

## FreeType 2.14.1
**URL(s):** [FreeType.org](https://freetype.org/) [Download Page](https://download.savannah.gnu.org/releases/freetype/)

**Folder Name:** `freetype`

**Download Required:** Yes, download `freetype-2.14.1.tar.gz` and copy `src/` and `include/` folders into `third_party/freetype`. In the `src/` folder, delete everything but `base/`, `gzip/`, `psnames/`, `sfnt/`, `smooth/` and `truetype/` folders.

**Current Version:** v2.14.1 from Sep. 11th 2025

**Fixup Required:**

* Change `Normalize` function to something like `FT_Normalize` in `src/truetype/ttinterp.c` (We use `Normalize` as a C11 `_Generic` macro).
* Change `interface` local variable to something like `interface_` in `src/base/ftobjs.c` in `ft_property_do` (`interface` macro is defined by D3D11 headers?)
* Change `Assert` macro in `src/gzip/zutil.h` to something like `z_Assert` (We use `Assert` macro and our arguments are different)

**Required For:**

* `misc_freetype_include.h` if `BUILD_WITH_FREETYPE` is enabled. This acts as a better option for font rasterization instead of `stb_truetype.h`

---

## kb_text_shape.h
**URL(s):** [Github](https://github.com/JimmyLefevre/kb)

**Folder Name:** `kb_text_shape`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Commit `880ebea2d4d9ee9b2478eecd1ba060751adc5d45` from July 19th 2025

**Fixup Required:** Delete everything besides `kb_text_shape.h` and `LICENSE`.

**Required For:** Glyph positioning in `gfx_font_flow.h`

---

## Pluto SVG v0.0.7 and Pluto VG v1.3.1
**URL(s):** [plutosvg Github](https://github.com/sammycage/plutosvg) [plutosvg v0.0.7](https://github.com/sammycage/plutosvg/releases/tag/v0.0.7) [plutovg Github](https://github.com/sammycage/plutovg) [plutovg v1.3.1](https://github.com/sammycage/plutovg/releases/tag/v1.3.1)

**Folder Name:** `plutosvg`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** plutosvg v0.0.7 released on May 15th 2025, and plutovg v1.3.1 released on September 13th 2025

**Fixup Required:** Populate submodules or download `plutovg` v1.3.1 manually from [here](https://github.com/sammycage/plutovg/releases/tag/v1.3.1). Extract `source` folder from PlutoSVG and `source` and `include` folders from PlutoVG into base folder. Delete `plutovg-stb-image.h`, `plutovg-stb-image-write.h`, `plutovg-stb-truetype.h`, `plutovg-ft-types.h`, `plutovg-ft-stroker.h`, `plutovg-ft-stroker.c`, `plutovg-ft-math.h`, and `plutovg-ft-math.c`. Many fixups are required to get PlutoVG compiling as part of our unity build with FreeType. See all locations marked with `NOTE(Taylor):`

**Required For:** When `BUILD_WITH_FREETYPE` and trying to render an SVG-based glyph from an OTF font file we use plutosvg
