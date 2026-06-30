@echo off

pushd ..

if not exist _data mkdir _data
pushd _data
if not exist resources mkdir resources
pushd resources
if not exist image mkdir image
popd
popd
if not exist media mkdir media
if not exist app mkdir app
if not exist third_party mkdir third_party
pushd third_party
if not exist _lib_debug mkdir _lib_debug
if not exist _lib_release mkdir _lib_release
popd

COPY core\template\build_config.h build_config.h
COPY core\template\build.bat build.bat
COPY core\template\build_script.c build_script.c
COPY core\template\.gitignore .gitignore
COPY core\template\NewPigCoreProject.sublime-project NewPigCoreProject.sublime-project
COPY core\template\defines.h app\defines.h
COPY core\template\app_main.h app\app_main.h
COPY core\template\app_main.c app\app_main.c
COPY core\template\app_helpers.c app\app_helpers.c
COPY core\template\app_resources.c app\app_resources.c
COPY core\template\app_resources.h app\app_resources.h
COPY core\template\platform_interface.h app\platform_interface.h
COPY core\template\platform_main.h app\platform_main.h
COPY core\template\platform_main.c app\platform_main.c
COPY core\template\platform_api.c app\platform_api.c
COPY core\template\main2d_shader.glsl app\main2d_shader.glsl
COPY core\template\win_resources.rc app\win_resources.rc
COPY core\template\icon.ico _data\resources\image\icon.ico
COPY core\template\icon_16.png _data\resources\image\icon_16.png
COPY core\template\icon_24.png _data\resources\image\icon_24.png
COPY core\template\icon_32.png _data\resources\image\icon_32.png
COPY core\template\icon_64.png _data\resources\image\icon_64.png
COPY core\template\icon_120.png _data\resources\image\icon_120.png
COPY core\template\icon_256.png _data\resources\image\icon_256.png

popd

