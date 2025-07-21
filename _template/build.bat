@echo off
REM By default blocks like IF and FOR in batch are treated as a single command and variable values are expended when the block is entered
REM Enabling "delayed expansion" allows us to use !variable! syntax inside blocks to get the value of variables on that line (and at each iteration in the case of loops)
setlocal enabledelayedexpansion

if not exist _build mkdir _build
pushd _build
set root=..
set build_config_path=%root%\build_config.h
set initialized_msvc_compiler=0
set tool_compiler_flags=/std:clatest /O2 /FC /nologo /Zi /I"%root%" /I"%root%\core"

:: +--------------------------------------------------------------+
:: |                  Compile extract_define.exe                  |
:: +--------------------------------------------------------------+
set extract_define_tool_name=extract_define.exe

:: If we need to build any tools, then we'll need to init MSVC compiler early, otherwise we initialize it later ONLY if BUILD_WINDOWS is true in build_config.h
:: VsDevCmd.bat often takes 1-2 seconds to run (often longer than the actual compile), so skipping it when it's not needed saves us a lot of time
set need_to_build_extract_define=0
if not exist %extract_define_tool_name% (
	set need_to_build_extract_define=1
)

if "%need_to_build_extract_define%"=="1" (
	if "%initialized_msvc_compiler%"=="0" (
		CALL :init_msvc_compiler
		set initialized_msvc_compiler=1
	)
)

if "%need_to_build_extract_define%"=="1" (
	echo [Building %extract_define_tool_name%...]
	del %extract_define_tool_name% > NUL 2> NUL
	cl %tool_compiler_flags% %root%\core\tools\tools_extract_define_main.c /Fe%extract_define_tool_name%
	if !ERRORLEVEL! NEQ 0 (
		echo [FAILED to build %extract_define_tool_name%!]
		exit
	)
	if not exist %extract_define_tool_name% (
		echo [cl did NOT produce %extract_define_tool_name%!]
		exit
	)
	echo [Built %extract_define_tool_name%!]
)

:: +--------------------------------------------------------------+
:: |                    Scrape build_config.h                     |
:: +--------------------------------------------------------------+
set extract_define=%extract_define_tool_name% %build_config_path%
for /f "delims=" %%i in ('%extract_define% BUILD_PIG_BUILD') do set BUILD_PIG_BUILD=%%i

:: +--------------------------------------------------------------+
:: |                     Build pig_build.exe                      |
:: +--------------------------------------------------------------+
set pig_build_tool_name=pig_build.exe
set pig_build_pdb_name=pig_build.pdb
set need_to_build_pig_build=0
if not exist %pig_build_tool_name% (
	set need_to_build_pig_build=1
)
if "%BUILD_PIG_BUILD%"=="1" (
	set need_to_build_pig_build=1
)

if "%need_to_build_pig_build%"=="1" (
	if "%initialized_msvc_compiler%"=="0" (
		CALL :init_msvc_compiler
		set initialized_msvc_compiler=1
	)
)

if "%need_to_build_pig_build%"=="1" (
	echo [Building %pig_build_tool_name%...]
	cl %tool_compiler_flags% %root%\build_script.c /Fe%pig_build_tool_name% /Fd"%pig_build_pdb_name%" /link Shlwapi.lib
	if !ERRORLEVEL! NEQ 0 (
		echo [FAILED to build %pig_build_tool_name%!]
		exit
	)
	if not exist %pig_build_tool_name% (
		echo [cl did NOT produce %pig_build_tool_name%!]
		exit
	)
	echo [Built %pig_build_tool_name%!]
)

%pig_build_tool_name%

popd

GOTO :eof

:: +--------------------------------------------------------------+
:: |                      Init MSVC Compiler                      |
:: +--------------------------------------------------------------+
:init_msvc_compiler

for /F "tokens=1-4 delims=:.," %%a in ("%time%") do (
	set /A "vsdevcmd_start_time=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)

REM set VSCMD_DEBUG=3
REM NOTE: Uncomment or change one of these lines to match your installation of Visual Studio compiler
REM call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
REM call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64 -no_logo
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 > NUL

for /F "tokens=1-4 delims=:.," %%a in ("%time%") do (
	set /A "vsdevcmd_end_time=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)
set /A vsdevcmd_elapsed_hundredths=vsdevcmd_end_time-vsdevcmd_start_time
set /A vsdevcmd_elapsed_seconds_part=vsdevcmd_elapsed_hundredths/100
set /A vsdevcmd_elapsed_hundredths_part=vsdevcmd_elapsed_hundredths%%100
if %vsdevcmd_elapsed_hundredths_part% lss 10 set vsdevcmd_elapsed_hundredths_part=0%vsdevcmd_elapsed_hundredths_part%
echo VsDevCmd.bat took %vsdevcmd_elapsed_seconds_part%.%vsdevcmd_elapsed_hundredths_part% seconds

EXIT /B
