@echo off

if "%~1"=="" (
    echo Usage: init_emsdk.bat output_file.txt
    exit /b 1
)

call "%EMSCRIPTEN_SDK_PATH%\emsdk_env.bat" 2> NUL > NUL

set > "%~1"