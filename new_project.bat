@echo off

pushd ..

if not exist _data mkdir _data
pushd _data
if not exist resources mkdir resources
popd
if not exist _media mkdir _media
if not exist app mkdir app
if not exist third_party mkdir third_party
pushd third_party
if not exist _lib_debug mkdir _lib_debug
if not exist _lib_release mkdir _lib_release
popd

COPY core\_template\build_config.h build_config.h
COPY core\_template\build.bat build.bat
COPY core\_template\.gitignore .gitignore
COPY core\_template\NewPigCoreProject.sublime-project NewPigCoreProject.sublime-project
COPY core\_template\app_main.c app\app_main.c
COPY core\_template\platform_main.c app\platform_main.c
COPY core\_template\main2d_shader.glsl app\main2d_shader.glsl

popd

