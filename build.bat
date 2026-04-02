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
set MSVC_ENVIRONMENT_TXT_PATH=msvc_environment.txt

REM The flags are only used when compiling the build_script.c (not for compiling your main program, those flags should be defined inside the build_script.c)
set compiler_flags=/std:clatest /INCREMENTAL:NO /Od /FC /nologo /Zi /I".." /I"%PIG_BUILD_FOLDER_PATH%" /link Shlwapi.lib"

REM If the build_script binary doesn't exist then it obviously needs to be built
if not exist %BUILD_SCRIPT_EXE_NAME% (
	echo Compiling %BUILD_SCRIPT_SOURCE_NAME%...
	del %BUILD_SCRIPT_EXE_NAME% > NUL 2> NUL
	del %BUILD_SCRIPT_PDB_NAME% > NUL 2> NUL
	del %BUILD_SCRIPT_HASH_PATH% > NUL 2> NUL
	CALL %PIG_BUILD_FOLDER_PATH%\shell\init_msvc.bat %MSVC_ENVIRONMENT_TXT_PATH%
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
	CALL %PIG_BUILD_FOLDER_PATH%\shell\init_msvc.bat %MSVC_ENVIRONMENT_TXT_PATH%
	cl %BUILD_SCRIPT_SOURCE_PATH% /Fe%BUILD_SCRIPT_EXE_NAME% /Fd"%BUILD_SCRIPT_PDB_NAME%" %compiler_flags%
	
	REM Run the build binary (second try)
	%BUILD_SCRIPT_EXE_NAME% %*
)

popd
