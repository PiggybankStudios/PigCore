#!/bin/bash

# We want all the build artifacts to be contained in a "build" folder so make that folder if needed and pushd into it before we do anything else
mkdir -p build
pushd build > /dev/null

# If you want to change any of these values, make sure you change them in build.bat and build_system/build_system_recompile.h
REBUILD_EXIT_CODE="42"
BUILD_SCRIPT_EXE_NAME="builder"
BUILD_SCRIPT_HASH_PATH="builder_hash.txt"
BUILD_SCRIPT_SOURCE_NAME="build_script.c"
BUILD_SCRIPT_SOURCE_PATH="../$BUILD_SCRIPT_SOURCE_NAME"

# The flags are only used when compiling the build_script.c (not for compiling your main program, those flags should be defined inside the build_script.c)
compiler_flags="-std=gnu2x -fdiagnostics-absolute-paths -O0 -g -I.."

# If the build_script binary doesn't exist then it obviously needs to be built
if [ ! -e "$BUILD_SCRIPT_EXE_NAME" ]; then
	echo "Compiling $BUILD_SCRIPT_SOURCE_NAME..."
	rm -f "$BUILD_SCRIPT_EXE_NAME"
	rm -f "$BUILD_SCRIPT_HASH_PATH"
	clang $compiler_flags -o "$BUILD_SCRIPT_EXE_NAME" $BUILD_SCRIPT_SOURCE_PATH
fi

# Keep shell from exiting if builder returns a non-zero exit code
set +e
# Run the build binary (first try)
./$BUILD_SCRIPT_EXE_NAME "$@"
builder_exit_code="$?"
# Re-instate the default exit behavior
set -e

# If the existing builder gives this exit code, it means it wants to be re-built and then run again (probably there are changes to it's own source code since last time it was compiled)
if [ $builder_exit_code -eq $REBUILD_EXIT_CODE ]; then
	echo "Recompiling $BUILD_SCRIPT_SOURCE_NAME..."
	
	rm -f "$BUILD_SCRIPT_EXE_NAME"
	rm -f "$BUILD_SCRIPT_HASH_PATH"
	clang $compiler_flags -o "$BUILD_SCRIPT_EXE_NAME" $BUILD_SCRIPT_SOURCE_PATH
	
	# Run the build binary (second try). This time, let shell exit if error code is non-zero
	./$BUILD_SCRIPT_EXE_NAME $@
fi

popd
