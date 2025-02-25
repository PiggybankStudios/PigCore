@echo off
setlocal enabledelayedexpansion

if not exist _build mkdir _build
pushd _build
set root=..
set scripts=%root%\_scripts
set tools=%root%\third_party\_tools\win32

:: +--------------------------------------------------------------+
:: |                    Scrape build_config.h                     |
:: +--------------------------------------------------------------+
python --version > NUL 2> NUL
if errorlevel 1 (
	echo WARNING: Python isn't installed on this computer. Defines cannot be extracted from build_config.h, and shaders cant be found and compiled!
	exit
)

set extract_define=python %scripts%\extract_define.py ../build_config.h
for /f "delims=" %%i in ('%extract_define% DEBUG_BUILD') do set DEBUG_BUILD=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_PIGGEN') do set BUILD_PIGGEN=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_SHADERS') do set BUILD_SHADERS=%%i
for /f "delims=" %%i in ('%extract_define% RUN_PIGGEN') do set RUN_PIGGEN=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_IMGUI_OBJ') do set BUILD_IMGUI_OBJ=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_PHYSX_OBJ') do set BUILD_PHYSX_OBJ=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_PIG_CORE_DLL') do set BUILD_PIG_CORE_DLL=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_TESTS') do set BUILD_TESTS=%%i
for /f "delims=" %%i in ('%extract_define% RUN_TESTS') do set RUN_TESTS=%%i
for /f "delims=" %%i in ('%extract_define% DUMP_PREPROCESSOR') do set DUMP_PREPROCESSOR=%%i
for /f "delims=" %%i in ('%extract_define% CONVERT_WASM_TO_WAT') do set CONVERT_WASM_TO_WAT=%%i
for /f "delims=" %%i in ('%extract_define% USE_EMSCRIPTEN') do set USE_EMSCRIPTEN=%%i
for /f "delims=" %%i in ('%extract_define% ENABLE_AUTO_PROFILE') do set ENABLE_AUTO_PROFILE=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_WINDOWS') do set BUILD_WINDOWS=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_LINUX') do set BUILD_LINUX=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_WEB') do set BUILD_WEB=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_WITH_RAYLIB') do set BUILD_WITH_RAYLIB=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_WITH_BOX2D') do set BUILD_WITH_BOX2D=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_WITH_SOKOL_GFX') do set BUILD_WITH_SOKOL_GFX=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_WITH_SOKOL_APP') do set BUILD_WITH_SOKOL_APP=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_WITH_SDL') do set BUILD_WITH_SDL=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_WITH_OPENVR') do set BUILD_WITH_OPENVR=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_WITH_CLAY') do set BUILD_WITH_CLAY=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_WITH_IMGUI') do set BUILD_WITH_IMGUI=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_WITH_PHYSX') do set BUILD_WITH_PHYSX=%%i

:: +--------------------------------------------------------------+
:: |                      Init MSVC Compiler                      |
:: +--------------------------------------------------------------+
for /F "tokens=1-4 delims=:.," %%a in ("%time%") do (
	set /A "vsdevcmd_start_time=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)
if "%BUILD_WINDOWS%"=="1" (
	REM set VSCMD_DEBUG=3
	REM NOTE: Uncomment or change one of these lines to match your installation of Visual Studio compiler
	REM call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
	REM call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64 -no_logo
	call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 > NUL
)
for /F "tokens=1-4 delims=:.," %%a in ("%time%") do (
	set /A "vsdevcmd_end_time=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)
set /A vsdevcmd_elapsed_hundredths=vsdevcmd_end_time-vsdevcmd_start_time
set /A vsdevcmd_elapsed_seconds_part=vsdevcmd_elapsed_hundredths/100
set /A vsdevcmd_elapsed_hundredths_part=vsdevcmd_elapsed_hundredths%%100
if %vsdevcmd_elapsed_hundredths_part% lss 10 set vsdevcmd_elapsed_hundredths_part=0%vsdevcmd_elapsed_hundredths_part%
if "%BUILD_WINDOWS%"=="1" ( echo VsDevCmd.bat took %vsdevcmd_elapsed_seconds_part%.%vsdevcmd_elapsed_hundredths_part% seconds )

if "%USE_EMSCRIPTEN%"=="1" (
	call "C:\gamedev\lib\emsdk\emsdk_env.bat" > NUL 2> NUL
)

:: +--------------------------------------------------------------+
:: |                       Compiler Options                       |
:: +--------------------------------------------------------------+
:: /std:clatest = Use latest C language spec features
:: /experimental:c11atomics = Enables _Atomic types
set c_cl_flags=/std:clatest /experimental:c11atomics
:: /wd4471 = a forward declaration of an unscoped enumeration must have an underlying type
:: /wd5054 = operator '|': deprecated between enumerations of different types
set cpp_cl_flags=/std:c++20 /wd4471 /wd5054
:: /FC = Full path for error messages
:: /nologo = Suppress the startup banner
:: /W4 = Warning level 4 [just below /Wall]
:: /WX = Treat warnings as errors
set common_cl_flags=/FC /nologo /W4 /WX
:: -fdiagnostics-absolute-paths = Print absolute paths in diagnostics TODO: Figure out how to resolve these back to windows paths for Sublime error linking?
:: -std=gnu2x = Use C20+ language spec (NOTE: We originally had -std=c2x but that didn't define MAP_ANONYMOUS and mmap was failing)
:: NOTE: Clang Warning Options: https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
:: -Wall = This enables all the warnings about constructions that some users consider questionable, and that are easy to avoid (or modify to prevent the warning), even in conjunction with macros.
:: -Wextra = This enables some extra warning flags that are not enabled by -Wall.
:: -Wshadow = Warn whenever a local variable or type declaration shadows another variable, parameter, type, class member (in C++), or instance variable (in Objective-C) or whenever a built-in function is shadowed
:: -Wimplicit-fallthrough = Must use [[fallthrough]] on a case label that falls through to the next case
set common_clang_flags=-fdiagnostics-absolute-paths -std=gnu2x -Wall -Wextra -Wshadow -Wimplicit-fallthrough
:: /wd4130 = Logical operation on address of string constant [W4] TODO: Should we re-enable this one? Don't know any scenarios where I want to do this
:: /wd4201 = Nonstandard extension used: nameless struct/union [W4] TODO: Should we re-enable this restriction for ANSI compatibility?
:: /wd4324 = Structure was padded due to __declspec[align[]] [W4]
:: /wd4458 = Declaration of 'identifier' hides class member [W4]
:: /wd4505 = Unreferenced local function has been removed [W4]
:: /wd4996 = Usage of deprecated function, class member, variable, or typedef [W3]
:: /wd4706 = assignment within conditional expression [W?]
:: /we5262 = Enable the [[fallthrough]] missing warning
set common_cl_flags=%common_cl_flags% /wd4130 /wd4201 /wd4324 /wd4458 /wd4505 /wd4996 /wd4706 /we5262
:: -Wno-switch = 8 enumeration values not handled in switch: 'ArenaType_None', 'ArenaType_Funcs', 'ArenaType_Generic'...
:: -Wno-unused-function = unused function 'MeowExpandSeed'
set common_clang_flags=%common_clang_flags% -Wno-switch -Wno-unused-function
:: /I = Adds an include directory to search in when resolving #includes
set common_cl_flags=%common_cl_flags% /I"%root%"
:: -I = Add directory to the end of the list of include search paths
:: -mssse3 = For MeowHash to work we need sse3 support
:: -maes = For MeowHash to work we need aes support
set linux_clang_flags=-I "../%root%" -mssse3 -maes
:: -lm = Include the math library (required for stuff like sinf, atan, etc.)
:: -ldl = Needed for dlopen and similar functions
set linux_linker_flags=-lm -ldl
:: --target=wasm32 = ?
:: -mbulk-memory = ?
:: TODO: -Wl,--export-dynamic ?
:: TODO: -Wl,--export-dynamic ?
:: TODO: -nostdlib ?
:: TODO: -Wl,--initial-memory=6553600 ?
set wasm_clang_flags=--target=wasm32 -mbulk-memory -I "../%root%"
if "%USE_EMSCRIPTEN%"=="1" (
	set wasm_clang_flags=%wasm_clang_flags% -sUSE_SDL -sALLOW_MEMORY_GROWTH
) else (
	REM -Wl, = Pass the following argument(s) to the linker
	REM --allow-undefined = ?
	REM --no-entry = ?
	REM --no-standard-libraries = ?
	REM --no-standard-includes = ?
	set wasm_clang_flags=%wasm_clang_flags% -I "../%root%/wasm/std/include" --no-standard-libraries --no-standard-includes -Wl,--no-entry -Wl,--export=__heap_base -Wl,--allow-undefined
)
if "%DEBUG_BUILD%"=="1" (
	REM /MDd = ?
	REM /Od = Optimization level: Debug
	REM /Zi = Generate complete debugging information
	REM /wd4065 = Switch statement contains 'default' but no 'case' labels
	REM /wd4100 = Unreferenced formal parameter [W4]
	REM /wd4101 = Unreferenced local variable [W3]
	REM /wd4127 = Conditional expression is constant [W4]
	REM /wd4189 = Local variable is initialized but not referenced [W4]
	REM /wd4702 = Unreachable code [W4]
	set common_cl_flags=%common_cl_flags% /MDd /Od /Zi /wd4065 /wd4100 /wd4101 /wd4127 /wd4189 /wd4702
	set shader_cl_flags=%shader_cl_flags% /MDd /Od /Zi
	REM -Wno-unused-parameter = warning: unused parameter 'numBytes'
	set common_clang_flags=%common_clang_flags% -Wno-unused-parameter -Wno-unused-variable
) else (
	REM /MD = ?
	REM /Ot = Favors fast code over small code
	REM /Oy = Omit frame pointer [x86 only]
	REM /O2 = Optimization level 2: Creates fast code
	REM /Zi = Generate complete debugging information [optional]
	set common_cl_flags=%common_cl_flags% /MD /Ot /Oy /O2
	set shader_cl_flags=%shader_cl_flags% /MD /Ot /Oy /O2
	set common_clang_flags=%common_clang_flags%
)

:: Gdi32.lib = Needed for CreateFontA and other Windows graphics functions
:: User32.lib = Needed for GetForegroundWindow, GetDC, etc.
set tests_libraries=Gdi32.lib User32.lib
set tests_clang_libraries=
set pig_core_dll_libraries=Gdi32.lib User32.lib
if "%BUILD_WITH_RAYLIB%"=="1" (
	REM raylib.lib   = Raylib (NOTE: It has to come BEFORE User32.lib otherwise CloseWindow will conflict)
	REM Shell32.lib  = Shlobj.h ? 
	REM kernel32.lib = ?
	REM winmm.lib    = ?
	REM Winhttp.lib  = ?
	REM Shlwapi.lib  = ?
	REM Ole32.lib    = Combaseapi.h, CoCreateInstance
	REM Advapi32.lib = Processthreadsapi.h, OpenProcessToken, GetTokenInformation
	set tests_libraries=raylib.lib %tests_libraries% Shell32.lib kernel32.lib winmm.lib
	REM NOTE: Compiling for Linux with raylib would require following instructions here: https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux
	set pig_core_dll_libraries=raylib.lib %pig_core_dll_libraries% gdi32.lib User32.lib Shell32.lib kernel32.lib winmm.lib
)
if "%BUILD_WITH_BOX2D%"=="1" (
	set tests_libraries=%tests_libraries% box2d.lib
	set pig_core_dll_libraries=%pig_core_dll_libraries% box2d.lib
)
if "%BUILD_WITH_SDL%"=="1" (
	set tests_libraries=%tests_libraries% SDL2.lib
	set pig_core_dll_libraries=%pig_core_dll_libraries% SDL2.lib
)
if "%BUILD_WITH_SOKOL_GFX%"=="1" (
	set tests_clang_libraries=%tests_clang_libraries% -lGL
)
if "%BUILD_WITH_SOKOL_APP%"=="1" (
	set tests_clang_libraries=%tests_clang_libraries% -lX11 -lXi -ldl -lXcursor
)
if "%BUILD_WITH_OPENVR%"=="1" (
	set tests_libraries=%tests_libraries% openvr_api.lib
	set pig_core_dll_libraries=%pig_core_dll_libraries% openvr_api.lib
)

:: -incremental:no = Suppresses warning about doing a full link when it can't find the previous .exe result. We don't need this when doing unity builds
:: /LIBPATH = Add a library search path
set common_ld_flags=-incremental:no

if "%DEBUG_BUILD%"=="1" (
	set common_ld_flags=%common_ld_flags% /LIBPATH:"%root%\third_party\_lib_debug"
) else (
	set common_ld_flags=%common_ld_flags% /LIBPATH:"%root%\third_party\_lib_release"
)

if "%DUMP_PREPROCESSOR%"=="1" (
	REM /P = Output the result of the preprocessor to {file_name}.i (disables the actual compilation)
	REM /C = Preserve comments through the preprocessor
	set common_cl_flags=/P /C %common_cl_flags%
	REM -E = Only run the preprocessor
	set common_clang_flags=%common_clang_flags% -E
)

set wasm_js_files=..\%root%\wasm\wasm_globals.js
set wasm_js_files=%wasm_js_files% ..\%root%\wasm\std\include\internal\wasm_std_js_api.js
set wasm_js_files=%wasm_js_files% ..\%root%\wasm\wasm_app_js_api.js
set wasm_js_files=%wasm_js_files% ..\%root%\wasm\wasm_main.js

for /F "tokens=1-4 delims=:.," %%a in ("%time%") do (
	set /A "build_start_time=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)

:: +--------------------------------------------------------------+
:: |                        Build Shaders                         |
:: +--------------------------------------------------------------+
set shader_list_file=shader_list_source.txt
if "%BUILD_SHADERS%"=="1" (
	if not exist sokol-shdc.exe (
		echo "[Copying sokol-shdc.exe from %tools%...]"
		COPY %tools%\sokol-shdc.exe sokol-shdc.exe
	)
	echo.
	echo [Compiling Shaders...]
	python %scripts%\find_and_compile_shaders.py "%root%" --exclude="third_party" --exclude="_template" --exclude=".git" --exclude="_build" --list_file=%shader_list_file%
)

:: Read the list file into %shader_list% variable
for /f "delims=" %%x in (%shader_list_file%) do set shader_list=%%x

set shader_object_files=
set shader_linux_object_files=
REM Separate list by commas, for each item compile to .obj file, add output path to %shader_object_files%
for %%y in ("%shader_list:,=" "%") do (
	set object_name=%%~ny%.obj
	set linux_object_name=%%~ny%.o
	if "%BUILD_SHADERS%"=="1" (
		set shader_file_path=%%~y%
		set shader_file_path_fw_slash=!shader_file_path:\=/!
		set shader_file_dir=%%y:~0,-1%
		if "%BUILD_WINDOWS%"=="1" (
			cl /c %common_cl_flags% /I"!shader_file_dir!" /Fo"!object_name!" !shader_file_path!
		)
		if "%BUILD_LINUX%"=="1" (
			if not exist linux mkdir linux
			pushd linux
			wsl clang-18 -c %common_clang_flags% %linux_clang_flags% -I "../!shader_file_dir!" -o "!linux_object_name!" ../!shader_file_path_fw_slash!
			popd
		)
	)
	set shader_object_files=!shader_object_files! !object_name!
	set shader_linux_object_files=!shader_linux_object_files! !linux_object_name!
)
if "%BUILD_SHADERS%"=="1" ( echo [Shaders Compiled!] )
rem echo shader_object_files %shader_object_files%
rem echo shader_linux_object_files %shader_linux_object_files%

:: +--------------------------------------------------------------+
:: |                       Build piggen.exe                       |
:: +--------------------------------------------------------------+
:: /Fe = Set the output exe file name
set piggen_source_path=%root%/piggen/piggen_main.c
set piggen_exe_path=piggen.exe
set piggen_bin_path=piggen
set piggen_cl_args=%common_cl_flags% %c_cl_flags% /Fe%piggen_exe_path% %piggen_source_path% /link %common_ld_flags%
set piggen_clang_args=%common_clang_flags% %linux_clang_flags% %linux_linker_flags% -o %piggen_bin_path% ../%piggen_source_path%
if "%BUILD_PIGGEN%"=="1" (
	echo.
	echo [Building piggen...]
	del %piggen_exe_path% > NUL 2> NUL
	cl %piggen_cl_args%
	if "%DUMP_PREPROCESSOR%"=="1" (
		COPY main.i piggen_preprocessed.i > NUL
	)
	echo [Built piggen!]
)

if "%RUN_PIGGEN%"=="1" (
	echo.
	echo [%piggen_exe_path%]
	%piggen_exe_path% %root%
)

:: +--------------------------------------------------------------+
:: |                       Build imgui.obj                        |
:: +--------------------------------------------------------------+
set imgui_source_path=%root%/ui/ui_imgui_main.cpp
set imgui_obj_path=imgui.obj
set imgui_cl_args=/c %common_cl_flags% %cpp_cl_flags% /I"%root%\third_party\imgui" /Fo%imgui_obj_path% %imgui_source_path%
if "%BUILD_IMGUI_OBJ%"=="1" (
	if "%BUILD_WINDOWS%"=="1" (
		del "%imgui_obj_path%" > NUL 2> NUL
		
		echo.
		echo [Building %imgui_obj_path% for Windows...]
		cl %imgui_cl_args%
		echo [Built %imgui_obj_path% for Windows!]
	)
)

:: +--------------------------------------------------------------+
:: |                      Build pig_core.dll                      |
:: +--------------------------------------------------------------+
set pig_core_dll_source_path=%root%/dll/dll_main.c
set pig_core_dll_path=pig_core.dll
set pig_core_lib_path=pig_core.lib
set pig_core_so_path=libpig_core.so
set pig_core_dll_args=%common_cl_flags% %c_cl_flags% /Fe%pig_core_dll_path% %pig_core_dll_source_path% /link /DLL %common_ld_flags% %pig_core_dll_libraries%
set pig_core_so_clang_args=%common_clang_flags% %linux_clang_flags% %linux_linker_flags% -fPIC -shared -o %pig_core_so_path% ../%pig_core_dll_source_path%
if "%BUILD_WITH_IMGUI%"=="1" (
	set pig_core_dll_args=%pig_core_dll_args% %imgui_obj_path%
)
if "%BUILD_PIG_CORE_DLL%"=="1" (
	if "%BUILD_WINDOWS%"=="1" (
		echo.
		echo [Building %pig_core_dll_path% for Windows...]
		
		del %pig_core_dll_path% > NUL 2> NUL
		cl %pig_core_dll_args%
		
		echo [Built %pig_core_dll_path% for Windows!]
	)
	if "%BUILD_LINUX%"=="1" (
		echo.
		echo [Building %pig_core_so_path% for Linux...]
		if not exist linux mkdir linux
		pushd linux
		
		wsl clang-18 %pig_core_so_clang_args%
		
		popd
		echo [Built %pig_core_so_path% for Linux!]
	)
)

:: +--------------------------------------------------------------+
:: |                       Build tests.exe                        |
:: +--------------------------------------------------------------+
set tests_source_path=%root%/tests/tests_main.c
set tests_exe_path=tests.exe
set tests_bin_path=tests
set tests_wasm_path=app.wasm
set tests_wat_path=app.wat
set tests_html_path=index.html
set tests_cl_args=%common_cl_flags% %c_cl_flags% /Fe%tests_exe_path% %tests_source_path% %shader_object_files% /link %common_ld_flags% %tests_libraries%
set tests_clang_args=%common_clang_flags% %linux_clang_flags% %linux_linker_flags% %tests_clang_libraries% -o %tests_bin_path% ../%tests_source_path% %shader_linux_object_files%
if "%ENABLE_AUTO_PROFILE%"=="1" (
	set tests_clang_args=-finstrument-functions %tests_clang_args%
)
if "%BUILD_WITH_IMGUI%"=="1" (
	set tests_cl_args=%tests_cl_args% %imgui_obj_path%
)
set tests_web_args=%common_clang_flags% %wasm_clang_flags% ../%tests_source_path%"
if "%USE_EMSCRIPTEN%"=="1" (
	set tests_web_args=-o %tests_html_path% %tests_web_args%
) else (
	set tests_web_args=-o %tests_wasm_path% %tests_web_args%
)
if "%BUILD_TESTS%"=="1" (
	if "%BUILD_WINDOWS%"=="1" (
		echo.
		echo [Building tests for Windows...]
		
		del %tests_exe_path% > NUL 2> NUL
		cl %tests_cl_args%
		
		if "%DUMP_PREPROCESSOR%"=="1" (
			COPY main.i tests_preprocessed.i > NUL
		)
		
		echo [Built tests for Windows!]
	)
	if "%BUILD_LINUX%"=="1" (
		echo.
		echo [Building tests for Linux...]
		if not exist linux mkdir linux
		pushd linux
		
		del %tests_bin_path% > NUL 2> NUL
		wsl clang-18 %tests_clang_args%
		
		popd
		echo [Built tests for Linux!]
	)
	if "%BUILD_WEB%"=="1" (
		echo.
		echo [Building tests for Web...]
		if not exist web mkdir web
		pushd web
		
		del *.wasm > NUL 2> NUL
		del *.wat > NUL 2> NUL
		del *.css > NUL 2> NUL
		del *.html > NUL 2> NUL
		del *.js > NUL 2> NUL
		
		if "%USE_EMSCRIPTEN%"=="1" (
			emcc %tests_web_args%
			if "%CONVERT_WASM_TO_WAT%"=="1" (
				wasm2wat index.wasm > index.wat
			)
		) else (
			clang %tests_web_args%
			COPY ..\%root%\wasm\wasm_app_style.css main.css > NUL 2> NUL
			COPY ..\%root%\wasm\wasm_app_index.html index.html > NUL 2> NUL
			python ..\%root%\_scripts\combine_files.py combined.js %wasm_js_files%
			if "%CONVERT_WASM_TO_WAT%"=="1" (
				wasm2wat %tests_wasm_path% > %tests_wat_path%
			)
		)
		
		
		popd web
		echo [Built tests for Web!]
	)
)
REM TODO: For some reason when building for WEB with EMSCRIPTEN we are never running anything below this end parens!

:: +--------------------------------------------------------------+
:: |                  Measure Build Elapsed Time                  |
:: +--------------------------------------------------------------+
for /F "tokens=1-4 delims=:.," %%a in ("%time%") do (
	set /A "build_end_time=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)
set /A build_elapsed_hundredths=build_end_time-build_start_time
set /A build_elapsed_seconds_part=build_elapsed_hundredths/100
set /A build_elapsed_hundredths_part=build_elapsed_hundredths%%100
if %build_elapsed_hundredths_part% lss 10 set build_elapsed_hundredths_part=0%build_elapsed_hundredths_part%
echo.
echo Build took %build_elapsed_seconds_part%.%build_elapsed_hundredths_part% seconds

:: +--------------------------------------------------------------+
:: |                          Run Things                          |
:: +--------------------------------------------------------------+
if "%RUN_TESTS%"=="1" (
	echo.
	echo [%tests_exe_path%]
	%tests_exe_path%
)

echo.
popd