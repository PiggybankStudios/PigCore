# third_party Folder
This folder contains (or needs to be made to contain) copies of all third party library headers and source files. Some of these libraries are included directly in the git repository, others need to be downloaded into this folder manually before compilation. Refer to the list below to figure out which libraries are needed for which pieces of the repository, and which ones you'll need to download manually after cloning.

# List of Libraries

## stb
**URL(s):** [www.github.com/nothings/stb](https://www.github.com/nothings/stb)
**Folder Name:** `stb`
**Download Required:** No, all necassary files are included in the repository
**Current Version:** Commit `5c205738c191bcb0abc65c4febfa9bd25ff35234` from Nov 8th 2024
**Fixup Required:** Delete all non-essential files after cloning (everything that's not `stb_*.h`)
**Required For:**
* Compiling for Playdate and WebAssembly we use `stb_sprintf.h` as a printf implementation

## raylib 5.5
**URL(s):** [www.raylib.com](https://www.raylib.com/) [github.com/raysan5/raylib](https://github.com/raysan5/raylib)
**Folder Name:** `raylib`
**Download Required:** No, all necassary files are included in the repository
**Current Version:** [v5.5](https://github.com/raysan5/raylib/releases/tag/5.5) from Nov 18 2024
**Fixup Required:** Copy files from `lib/` to both `_lib_debug/` and `_lib_release/` folders in `third_party/` then copy `include/` folder contents into `third_party/raylib`
**Required For:**
* Building `test.exe` into a raylib based graphical application (only if `BUILD_WITH_RAYLIB` is enabled in `build_config.h`)

## Box2D 3.0
**URL(s):** [box2d.org](https://box2d.org/) [github.com/erincatto/box2d](https://github.com/erincatto/box2d)
**Folder Name:** `box2d`
**Download Required:** No, all necassary files are included in the repository
**Current Version:** [v3.0.0](https://github.com/erincatto/box2d/releases/tag/v3.0.0) from Aug 11 2024
**Fixup Required:** Compile `box2d.lib` in debug and release and add to `_lib_debug/` and `_lib_release/`. Copy `include/box2d/` folder contents into `third_party/box2d`
**Required For:**
* Building tests.exe with a little physics test (only if `BUILD_WITH_BOX2D` is enabled in `build_config.h`, also only renders if raylib is also enabled)
