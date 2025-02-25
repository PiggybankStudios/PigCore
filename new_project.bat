@echo off

pushd ..

if not exist _data mkdir _data
pushd _data
if not exist resources mkdir resources
pushd resources
if not exist image mkdir image
popd
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
COPY core\_template\defines.h app\defines.h
COPY core\_template\app_main.h app\app_main.h
COPY core\_template\app_main.c app\app_main.c
COPY core\_template\app_helpers.c app\app_helpers.c
COPY core\_template\platform_interface.h app\platform_interface.h
COPY core\_template\platform_main.h app\platform_main.h
COPY core\_template\platform_main.c app\platform_main.c
COPY core\_template\platform_api.c app\platform_api.c
COPY core\_template\main2d_shader.glsl app\main2d_shader.glsl
COPY core\_template\win_resources.rc app\win_resources.rc
COPY core\_template\icon.ico _data\resources\image\icon.ico
COPY core\_template\icon_16.png _data\resources\image\icon_16.png
COPY core\_template\icon_24.png _data\resources\image\icon_24.png
COPY core\_template\icon_32.png _data\resources\image\icon_32.png
COPY core\_template\icon_64.png _data\resources\image\icon_64.png
COPY core\_template\icon_120.png _data\resources\image\icon_120.png
COPY core\_template\icon_256.png _data\resources\image\icon_256.png

popd

