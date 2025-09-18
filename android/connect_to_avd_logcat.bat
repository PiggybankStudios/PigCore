@echo off

REM By default blocks like IF and FOR in batch are treated as a single command and variable values are expended when the block is entered
REM Enabling "delayed expansion" allows us to use !variable! syntax inside blocks to get the value of variables on that line (and at each iteration in the case of loops)
setlocal enabledelayedexpansion

if "%ANDROID_SDK%"=="" (
	echo ANDROID_SDK must be defined!
	goto :end
)

set ADB_EXE=%ANDROID_SDK%\platform-tools\adb.exe

%ADB_EXE% logcat -c

echo ======== Beginning of Log ========
%ADB_EXE% logcat --format=color,tag pigcore:V *:F
rem %ADB_EXE% logcat --format=color,tag

:end