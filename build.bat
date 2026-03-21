@echo off

REM By default blocks like IF and FOR in batch are treated as a single command and variable values are expended when the block is entered
REM Enabling "delayed expansion" allows us to use !variable! syntax inside blocks to get the value of variables on that line (and at each iteration in the case of loops)
setlocal enabledelayedexpansion

if not exist build mkdir build
pushd build

set REBUILD_EXIT_CODE=42
set BUILD_SCRIPT_EXE_NAME=builder.exe
set BUILD_SCRIPT_PDB_NAME=builder.pdb
set BUILD_SCRIPT_OBJ_NAME=build_script.obj
set BUILD_SCRIPT_HASH_PATH=builder_hash.txt
set BUILD_SCRIPT_SOURCE_NAME=build_script.c
set BUILD_SCRIPT_SOURCE_PATH=..\%BUILD_SCRIPT_SOURCE_NAME%
set PIG_BUILD_FOLDER_NAME=pig_build
set PIG_BUILD_FOLDER_PATH=..\%PIG_BUILD_FOLDER_NAME%

REM The flags are only used when compiling the build_script.c (not for compiling your main program, those flags should be defined inside the build_script.c)
set compiler_flags=/std:clatest /INCREMENTAL:NO /Od /FC /nologo /Zi /I".." /I"%PIG_BUILD_FOLDER_PATH%" /link Shlwapi.lib"

REM If the build_script binary doesn't exist then it obviously needs to be built
if not exist %BUILD_SCRIPT_EXE_NAME% (
	echo Compiling %BUILD_SCRIPT_SOURCE_NAME%...
	del %BUILD_SCRIPT_EXE_NAME% > NUL 2> NUL
	del %BUILD_SCRIPT_PDB_NAME% > NUL 2> NUL
	del %BUILD_SCRIPT_HASH_PATH% > NUL 2> NUL
	CALL :init_msvc_compiler
	cl %BUILD_SCRIPT_SOURCE_PATH% /Fe%BUILD_SCRIPT_EXE_NAME% /Fd"%BUILD_SCRIPT_PDB_NAME%" %compiler_flags%
)

REM Run the build binary (first try)
%BUILD_SCRIPT_EXE_NAME% %*
set builder_exit_code=%ERRORLEVEL%

REM If the existing builder gives this exit code, it means it wants to be re-built and then run again (probably there are changes to it's own source code since last time it was compiled)
if "%ERRORLEVEL%"=="%REBUILD_EXIT_CODE%" (
	echo Recompiling %BUILD_SCRIPT_SOURCE_NAME%...
	del %BUILD_SCRIPT_EXE_NAME% > NUL 2> NUL
	del %BUILD_SCRIPT_PDB_NAME% > NUL 2> NUL
	del %BUILD_SCRIPT_HASH_PATH% > NUL 2> NUL
	CALL :init_msvc_compiler
	cl %BUILD_SCRIPT_SOURCE_PATH% /Fe%BUILD_SCRIPT_EXE_NAME% /Fd"%BUILD_SCRIPT_PDB_NAME%" %compiler_flags%
	
	REM Run the build binary (second try)
	%BUILD_SCRIPT_EXE_NAME% %*
)

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