#!/bin/bash

mkdir -p _build
pushd _build > /dev/null
root=".."
build_config_path="$root/build_config.h"

python3 --version > /dev/null 2> /dev/null
if [ $? -ne 0 ]
then
	echo "WARNING: Python isn't installed on this computer. Defines cannot be extracted from build_config.h! And build numbers won't be incremented"
	exit
fi

# NOTE: For some reason I can't have quotes around the -I "$root" argument!
tool_compiler_flags="-std=gnu2x -O2 -fdiagnostics-absolute-paths -g -I$root"

# +--------------------------------------------------------------+
# |                Compile extract_define Program                |
# +--------------------------------------------------------------+
extract_define_tool_name="extract_define"

need_to_build_extract_define=0
if [ ! -e "$extract_define_tool_name" ]; then
	need_to_build_extract_define=1
fi

if [ $need_to_build_extract_define -eq 1 ]; then
	echo "[Building $extract_define_tool_name...]"
	clang $tool_compiler_flags "$root/tools/tools_extract_define_main.c" -o $extract_define_tool_name
	status_code=$?
	if [ $status_code -ne 0 ]; then
		echo "Clang failed to build $extract_define_tool_name. Exit code: $status_code"
		exit
	fi
	if [ ! -e "$extract_define_tool_name" ]; then
		echo "$extract_define_tool_name was not created by compiler! Exiting!"
		exit
	fi
fi

BUILD_PIG_BUILD=`./$extract_define_tool_name $build_config_path $extract_define BUILD_PIG_BUILD`

# +--------------------------------------------------------------+
# |                  Compile pig_build Program                   |
# +--------------------------------------------------------------+
pig_build_tool_name="pig_build"
need_to_build_pig_build=$BUILD_PIG_BUILD
if [ ! -e "$pig_build_tool_name" ]; then
	need_to_build_pig_build=1
fi

if [ $need_to_build_pig_build -eq 1 ]; then
	echo "[Building $pig_build_tool_name...]"
	clang $tool_compiler_flags "$root/build_script.c" -o $pig_build_tool_name
	status_code=$?
	if [ $status_code -ne 0 ]; then
		echo "Clang failed to build $pig_build_tool_name. Exit code: $status_code"
		exit
	fi
	if [ ! -e "$pig_build_tool_name" ]; then
		echo "$pig_build_tool_name was not created by compiler! Exiting!"
		exit
	fi
fi

./$pig_build_tool_name

popd > /dev/null
