@echo off

REM By default blocks like IF and FOR in batch are treated as a single command and variable values are expended when the block is entered
REM Enabling "delayed expansion" allows us to use !variable! syntax inside blocks to get the value of variables on that line (and at each iteration in the case of loops)
setlocal enabledelayedexpansion

if "%ANDROID_SDK%"=="" (
	echo ANDROID_SDK must be defined!
	goto :end
)

rem if not exist _log mkdir _log
rem pushd _log
%ANDROID_SDK%\emulator\emulator.exe -avd Pixel_3a_API_34 -verbose
rem popd

:end