@echo off

if not exist _build mkdir _build
pushd _build
set root=..

:: +--------------------------------------------------------------+
:: |                    Scrape build_config.h                     |
:: +--------------------------------------------------------------+
python --version > NUL 2> NUL
if errorlevel 1 (
	echo WARNING: Python isn't installed on this computer. Defines cannot be extracted from build_config.h! And build numbers won't be incremented
	exit
)

set extract_define=python ..\_scripts\extract_define.py ../build_config.h
for /f "delims=" %%i in ('%extract_define% DEBUG_BUILD') do set DEBUG_BUILD=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_PIGGEN') do set BUILD_PIGGEN=%%i
for /f "delims=" %%i in ('%extract_define% RUN_PIGGEN') do set RUN_PIGGEN=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_TESTS') do set BUILD_TESTS=%%i
for /f "delims=" %%i in ('%extract_define% RUN_TESTS') do set RUN_TESTS=%%i
for /f "delims=" %%i in ('%extract_define% DUMP_PREPROCESSOR') do set DUMP_PREPROCESSOR=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_WINDOWS') do set BUILD_WINDOWS=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_LINUX') do set BUILD_LINUX=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_WITH_RAYLIB') do set BUILD_WITH_RAYLIB=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_WITH_BOX2D') do set BUILD_WITH_BOX2D=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_WITH_SOKOL') do set BUILD_WITH_SOKOL=%%i

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

:: +--------------------------------------------------------------+
:: |                       Compiler Options                       |
:: +--------------------------------------------------------------+
:: /FC = Full path for error messages
:: /nologo = Suppress the startup banner
:: /W4 = Warning level 4 [just below /Wall]
:: /WX = Treat warnings as errors
:: /std:clatest = Use latest C language spec features
:: /experimental:c11atomics = Enables _Atomic types
set common_cl_flags=/FC /nologo /W4 /WX /std:clatest /experimental:c11atomics
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
set common_clang_flags=%common_clang_flags% -Wno-switch
:: /I = Adds an include directory to search in when resolving #includes
set common_cl_flags=%common_cl_flags% /I"%root%"
:: -I = Add directory to the end of the list of include search paths
:: -lm = Include the math library (required for stuff like sinf, atan, etc.)
set common_clang_flags=%common_clang_flags% -I "../%root%" -lm
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
	REM -Wno-unused-parameter = warning: unused parameter 'numBytes'
	set common_clang_flags=%common_clang_flags% -Wno-unused-parameter -Wno-unused-variable
) else (
	REM /MD = ?
	REM /Ot = Favors fast code over small code
	REM /Oy = Omit frame pointer [x86 only]
	REM /O2 = Optimization level 2: Creates fast code
	REM /Zi = Generate complete debugging information [optional]
	set common_cl_flags=%common_cl_flags% /MD /Ot /Oy /O2
	set common_clang_flags=%common_clang_flags%
)

:: -incremental:no = Suppresses warning about doing a full link when it can't find the previous .exe result. We don't need this when doing unity builds
:: /LIBPATH = Add a library search path
set common_ld_flags=-incremental:no
set common_clang_ld_flags=

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



for /F "tokens=1-4 delims=:.," %%a in ("%time%") do (
	set /A "build_start_time=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)

:: +--------------------------------------------------------------+
:: |                       Build piggen.exe                       |
:: +--------------------------------------------------------------+
:: /Fe = Set the output exe file name
set piggen_source_path=%root%/piggen/piggen_main.c
set piggen_exe_path=piggen.exe
set piggen_bin_path=piggen
set piggen_cl_args=%common_cl_flags% /Fe%piggen_exe_path% %piggen_source_path% /link %common_ld_flags%
set piggen_clang_args=%common_clang_flags% -o %piggen_bin_path% ../%piggen_source_path% %common_clang_ld_flags%
if "%BUILD_PIGGEN%"=="1" (
	if "%BUILD_WINDOWS%"=="1" (
		echo.
		echo [Building piggen for Windows...]
		del %piggen_exe_path% > NUL 2> NUL
		cl %piggen_cl_args%
		if "%DUMP_PREPROCESSOR%"=="1" (
			COPY main.i piggen_preprocessed.i > NUL
		)
		echo [Built piggen for Windows!]
	)
	if "%BUILD_LINUX%"=="1" (
		echo.
		echo [Building piggen for Linux...]
		if not exist linux mkdir linux
		pushd linux
		wsl clang-18 %piggen_clang_args%
		popd
		echo [Built piggen for Linux!]
	)
)

:: +--------------------------------------------------------------+
:: |                       Build tests.exe                        |
:: +--------------------------------------------------------------+
set tests_source_path=%root%/tests/tests_main.c
set tests_exe_path=tests.exe
set tests_bin_path=tests
set tests_cl_args=%common_cl_flags% /Fe%tests_exe_path% %tests_source_path% /link %common_ld_flags%
if "%BUILD_WITH_RAYLIB%"=="1" (
	REM raylib.lib   = ?
	REM gdi32.lib    = ?
	REM User32.lib   = ?
	REM Shell32.lib  = Shlobj.h ? 
	REM kernel32.lib = ?
	REM winmm.lib    = ?
	REM Winhttp.lib  = ?
	REM Shlwapi.lib  = ?
	REM Ole32.lib    = Combaseapi.h, CoCreateInstance
	REM Advapi32.lib = Processthreadsapi.h, OpenProcessToken, GetTokenInformation
	set tests_cl_args=%tests_cl_args% raylib.lib gdi32.lib User32.lib Shell32.lib kernel32.lib winmm.lib
	REM NOTE: Compiling for Linux with raylib would require following instructions here: https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux
)
if "%BUILD_WITH_BOX2D%"=="1" (
	set tests_cl_args=%tests_cl_args% box2d.lib
)
set tests_clang_args=%common_clang_flags% -o %tests_bin_path% ../%tests_source_path% %common_clang_ld_flags%
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
		wsl clang-18 %tests_clang_args%
		popd
		echo [Built tests for Linux!]
	)
)

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
:: TODO: Move this back up before we build anything since it eventually will be doing code generation that the other builds rely upon
if "%RUN_PIGGEN%"=="1" (
	echo.
	echo [%piggen_exe_path%]
	%piggen_exe_path% %root%
)

if "%RUN_TESTS%"=="1" (
	echo.
	echo [%tests_exe_path%]
	%tests_exe_path%
)

echo.
popd