@echo off

REM By default blocks like IF and FOR in batch are treated as a single command and variable values are expended when the block is entered
REM Enabling "delayed expansion" allows us to use !variable! syntax inside blocks to get the value of variables on that line (and at each iteration in the case of loops)
setlocal enabledelayedexpansion

if not exist ..\_build mkdir ..\_build
pushd ..\_build

set ASDK=F:\Programs\android_studio_sdk
set CMAKE=%ASDK%\cmake\4.1.1\bin
set NDK=%ASDK%\ndk\29.0.13599879
set NDK_CMAKE_FILE=%NDK%\build\cmake\android.toolchain.cmake

rem Use cmake on CMakeLists.txt to generate ninja project
%CMAKE%\cmake.exe -H../android -B. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DANDROID_STL=none -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=latest -DANDROID_NDK=%NDK% -DCMAKE_TOOLCHAIN_FILE=%NDK_CMAKE_FILE% -G Ninja

%CMAKE%\cmake.exe --build .
rem ninja

popd
