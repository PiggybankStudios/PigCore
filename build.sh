#!/bin/sh

# Build script for OSX specifically, not Linux
# TODO: Should we use this same script for Linux?

mkdir -p _build
pushd _build
root=".."

python3 --version > /dev/null 2> /dev/null
if [ $? -ne 0 ]
then
	echo WARNING: Python isn't installed on this computer. Defines cannot be extracted from build_config.h! And build numbers won't be incremented
	exit
fi

# +==============================+
# |    Scrape build_config.h     |
# +==============================+
extract_defines="python3 $root/_scripts/extract_define.py $root/build_config.h"
DEBUG_BUILD=$($extract_defines DEBUG_BUILD)
BUILD_PIGGEN=$($extract_defines BUILD_PIGGEN)
RUN_PIGGEN=$($extract_defines RUN_PIGGEN)
BUILD_TESTS=$($extract_defines BUILD_TESTS)
RUN_TESTS=$($extract_defines RUN_TESTS)
DUMP_PREPROCESSOR=$($extract_defines DUMP_PREPROCESSOR)

# TODO: Restart computer and see if new path is picked up by Sublime's environment
PATH=/opt/homebrew/opt/llvm/bin:$PATH

# -fdiagnostics-absolute-paths = Print absolute paths in diagnostics TODO: Figure out how to resolve these back to windows paths for Sublime error linking?
# -std=c2x = ?
# NOTE: Clang Warning Options: https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
# -Wall = This enables all the warnings about constructions that some users consider questionable, and that are easy to avoid (or modify to prevent the warning), even in conjunction with macros.
# -Wextra = This enables some extra warning flags that are not enabled by -Wall.
# -Wshadow = Warn whenever a local variable or type declaration shadows another variable, parameter, type, class member (in C++), or instance variable (in Objective-C) or whenever a built-in function is shadowed
# -Wimplicit-fallthrough = Must use [[fallthrough]] on a case label that falls through to the next case
# -arch x86_64 = Build for Intel based processors
# -arch arm64 = Build for M-series processors
common_clang_flags="-fdiagnostics-absolute-paths -std=c2x -Wall -Wextra -Wshadow -Wimplicit-fallthrough -arch x86_64 -arch arm64"
# -Wno-switch = 8 enumeration values not handled in switch: 'ArenaType_None', 'ArenaType_Funcs', 'ArenaType_Generic'...
common_clang_flags="$common_clang_flags -Wno-switch"
# -I = Add directory to the end of the list of include search paths
common_clang_flags="$common_clang_flags -I $root"

common_clang_ld_flags=

# TODO: Dump all macros option using: -E -dD

# +==============================+
# |    Compile piggen Binary     |
# +==============================+
piggen_bin_path=piggen
piggen_source_path=$root/piggen/piggen_main.c
piggen_clang_args="$common_clang_flags -o $piggen_bin_path $piggen_source_path $common_clang_ld_flags"
if [ $BUILD_PIGGEN -eq 1 ]
then
	rm $piggen_bin_path > /dev/null 2> /dev/null
	clang $piggen_clang_args
fi

if [ $RUN_PIGGEN -eq 1 ]
then
	./$piggen_bin_path $root
fi

# +==============================+
# |     Compile tests Binary     |
# +==============================+
tests_bin_path=tests
tests_source_path=$root/tests/tests_main.c
tests_clang_args="$common_clang_flags -o $tests_bin_path $tests_source_path $common_clang_ld_flags"
if [ $BUILD_TESTS -eq 1 ]
then
	rm $tests_bin_path > /dev/null 2> /dev/null
	clang $tests_clang_args
fi

if [ $RUN_TESTS -eq 1 ]
then
	./$tests_bin_path $root
fi

popd
