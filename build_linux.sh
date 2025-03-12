#!/bin/bash

mkdir -p _build
pushd _build > /dev/null
root=".."
scripts="$root/_scripts"
tools="$root/third_party/_tools/linux"

python3 --version > /dev/null 2> /dev/null
if [ $? -ne 0 ]
then
	echo "WARNING: Python isn't installed on this computer. Defines cannot be extracted from build_config.h! And build numbers won't be incremented"
	exit
fi

extract_defines="python3 $scripts/extract_define.py $root/build_config.h"
# TODO: We need to somehow figure out if any of these failed
DEBUG_BUILD=$($extract_defines DEBUG_BUILD)
BUILD_LINUX=$($extract_defines BUILD_LINUX)
BUILD_WEB=$($extract_defines BUILD_WEB)
BUILD_SHADERS=$($extract_defines BUILD_SHADERS)
BUILD_PIGGEN=$($extract_defines BUILD_PIGGEN)
RUN_PIGGEN=$($extract_defines RUN_PIGGEN)
BUILD_IMGUI_OBJ=$($extract_defines BUILD_IMGUI_OBJ)
BUILD_PHYSX_OBJ=$($extract_defines BUILD_PHYSX_OBJ)
BUILD_PIG_CORE_DLL=$($extract_defines BUILD_PIG_CORE_DLL)
BUILD_TESTS=$($extract_defines BUILD_TESTS)
RUN_TESTS=$($extract_defines RUN_TESTS)
DUMP_PREPROCESSOR=$($extract_defines DUMP_PREPROCESSOR)
CONVERT_WASM_TO_WAT=$($extract_defines CONVERT_WASM_TO_WAT)
USE_EMSCRIPTEN=$($extract_defines USE_EMSCRIPTEN)
BUILD_WITH_RAYLIB=$($extract_defines BUILD_WITH_RAYLIB)
BUILD_WITH_BOX2D=$($extract_defines BUILD_WITH_BOX2D)
BUILD_WITH_SOKOL_GFX=$($extract_defines BUILD_WITH_SOKOL_GFX)
BUILD_WITH_SOKOL_APP=$($extract_defines BUILD_WITH_SOKOL_APP)
BUILD_WITH_SDL=$($extract_defines BUILD_WITH_SDL)
BUILD_WITH_OPENVR=$($extract_defines BUILD_WITH_OPENVR)
BUILD_WITH_CLAY=$($extract_defines BUILD_WITH_CLAY)
BUILD_WITH_IMGUI=$($extract_defines BUILD_WITH_IMGUI)
BUILD_WITH_PHYSX=$($extract_defines BUILD_WITH_PHYSX)

c_clang_flags="-std=gnu2x"
cpp_clang_flags="TODO"
# -fdiagnostics-absolute-paths = Print absolute paths in diagnostics TODO: Figure out how to resolve these back to windows paths for Sublime error linking?
# -std=gnu2x = Use C20+ language spec (NOTE: We originally had -std=c2x but that didn't define MAP_ANONYMOUS and mmap was failing)
# NOTE: Clang Warning Options: https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
# -Wall = This enables all the warnings about constructions that some users consider questionable, and that are easy to avoid (or modify to prevent the warning), even in conjunction with macros.
# -Wextra = This enables some extra warning flags that are not enabled by -Wall.
# -Wshadow = Warn whenever a local variable or type declaration shadows another variable, parameter, type, class member (in C++), or instance variable (in Objective-C) or whenever a built-in function is shadowed
# -Wimplicit-fallthrough = Must use [[fallthrough]] on a case label that falls through to the next case
common_clang_flags="-fdiagnostics-absolute-paths -Wall -Wextra -Wshadow -Wimplicit-fallthrough"
# -Wno-switch = 8 enumeration values not handled in switch: 'ArenaType_None', 'ArenaType_Funcs', 'ArenaType_Generic'...
# -Wno-unused-function = unused function 'MeowExpandSeed'
common_clang_flags="$common_clang_flags -Wno-switch -Wno-unused-function"
# -I = Add directory to the end of the list of include search paths
# -mssse3 = For MeowHash to work we need sse3 support
# -maes = For MeowHash to work we need aes support
linux_clang_flags="-I$root -mssse3 -maes"
# -lm = Include the math library (required for stuff like sinf, atan, etc.)
# -ldl = Needed for dlopen and similar functions
common_linker_flags="-lm -ldl"
# --target=wasm32 = ?
# -mbulk-memory = ?
# TODO: -Wl,--export-dynamic ?
# TODO: -Wl,--export-dynamic ?
# TODO: -nostdlib ?
# TODO: -Wl,--initial-memory=6553600 ?
wasm_clang_flags="--target=wasm32 -mbulk-memory -I$root"
if [[ $USE_EMSCRIPTEN -eq 1 ]] then
	wasm_clang_flags="$wasm_clang_flags -sUSE_SDL -sALLOW_MEMORY_GROWTH"
else
	# -Wl, = Pass the following argument(s) to the linker
	# --allow-undefined = ?
	# --no-entry = ?
	# --no-standard-libraries = ?
	# --no-standard-includes = ?
	wasm_clang_flags="$wasm_clang_flags -I$root/wasm/std/include --no-standard-libraries --no-standard-includes -Wl,--no-entry -Wl,--export=__heap_base -Wl,--allow-undefined"
fi
if [[ $DEBUG_BUILD -eq 1 ]] then
	# -Wno-unused-parameter = warning: unused parameter 'numBytes'
	# -Wno-unused-variable = ?
	common_clang_flags="$common_clang_flags -Wno-unused-parameter -Wno-unused-variable"
	common_linker_flags="$common_linker_flags -L$root/third_party/_lib_debug"
else
	common_clang_flags="$common_clang_flags"
	common_linker_flags="$common_linker_flags -L$root/third_party/_lib_release"
fi
if [[ $DUMP_PREPROCESSOR -eq 1 ]] then
	# -E = Only run the preprocessor
	common_clang_flags="$common_clang_flags -E"
fi

pig_core_so_libraries=""
tests_libraries=""
if [[ $BUILD_WITH_RAYLIB -eq 1 ]] then
	tests_libraries="$tests_libraries -lraylib -lGL -lpthread -lrt -lX11"
fi
if [[ $BUILD_WITH_BOX2D -eq 1 ]] then
	tests_libraries="$tests_libraries -lbox2d"
fi
if [[ $BUILD_WITH_SOKOL_GFX -eq 1 ]] then
	tests_libraries="$tests_libraries -lGL"
fi
if [[ $BUILD_WITH_SOKOL_APP -eq 1 ]] then
	tests_libraries="$tests_libraries -lX11 -lXi -lXcursor"
fi

wasm_js_files="$root/wasm/wasm_globals.js"
wasm_js_files="$wasm_js_files $root/wasm/std/include/internal/wasm_std_js_api.js"
wasm_js_files="$wasm_js_files $root/wasm/wasm_app_js_api.js"
wasm_js_files="$wasm_js_files $root/wasm/wasm_main.js"

# +--------------------------------------------------------------+
# |                        Build Shaders                         |
# +--------------------------------------------------------------+
shader_list_file="shader_list_source.txt"
shader_clang_args="$common_clang_flags $c_clang_flags $linux_clang_flags"
if [[ $BUILD_SHADERS -eq 1 ]] then
	if [[ ! -f sokol-shdc ]] then
		cp $tools/sokol-shdc sokol-shdc
	fi
	python3 $scripts/find_and_compile_shaders.py "$root" --exclude="third_party" --exclude="_template" --exclude=".git" --exclude="_build" --list_file="$shader_list_file"
fi
shader_source_files=`cat $shader_list_file`

shader_o_paths=""
for shader_path in ${shader_source_files//,/ }
do
	shader_pathname="$(basename "$shader_path")"
	shader_o_path="${shader_pathname%.*}.o"
	if [[ $BUILD_SHADERS -eq 1 ]] then
		echo "[Compiling $shader_path...]"
		clang-18 -c $shader_clang_args $shader_path -o "$shader_o_path"
	fi
	if [[ -z "${shader_o_paths}" ]] then
		shader_o_paths="$shader_o_path"
	else
		shader_o_paths="$shader_o_paths $shader_o_path"
	fi
done

# +--------------------------------------------------------------+
# |                         Build piggen                         |
# +--------------------------------------------------------------+
if [[ $BUILD_PIGGEN -eq 1 ]] then
	echo "TODO: Build piggen!"
fi

# +--------------------------------------------------------------+
# |                        Build imgui.o                         |
# +--------------------------------------------------------------+
if [[ $BUILD_IMGUI_OBJ -eq 1 ]] then
	echo "TODO: Build imgui.o!"
fi

# +--------------------------------------------------------------+
# |                      Build physx_capi.o                      |
# +--------------------------------------------------------------+
if [[ $BUILD_PHYSX_OBJ -eq 1 ]] then
	echo "TODO: Build physx_capi.o!"
fi

# +--------------------------------------------------------------+
# |                      Build pig_core.so                       |
# +--------------------------------------------------------------+
pig_core_so_path=pig_core.so
pig_core_dll_source_path=$root/dll/dll_main.c
pig_core_so_clang_args="$common_clang_flags $c_clang_flags $linux_clang_flags -fPIC -shared -o $pig_core_so_path $pig_core_dll_source_path $common_linker_flags $pig_core_so_libraries"
if [[ $BUILD_PIG_CORE_DLL -eq 1 ]] then
	if [[ $BUILD_LINUX -eq 1 ]] then
		echo "[Building pig_core.so for Linux...]"
		clang-18 $pig_core_so_clang_args
		echo "[Built pig_core.so for Linux!]"
	fi
	# TODO: Add support for building to web
fi

# +--------------------------------------------------------------+
# |                         Build tests                          |
# +--------------------------------------------------------------+
tests_bin_path=tests
tests_source_path=$root/tests/tests_main.c
tests_clang_args="$common_clang_flags $c_clang_flags $linux_clang_flags -o $tests_bin_path $tests_source_path $common_linker_flags $tests_libraries $shader_o_paths"
if [[ $BUILD_TESTS -eq 1 ]] then
	if [[ $BUILD_LINUX -eq 1 ]] then
		echo "[Building tests for Linux...]"
		clang-18 $tests_clang_args
		echo "[Built tests for Linux!]"
	fi
	# TODO: Add support for building to web
fi

popd > /dev/null
