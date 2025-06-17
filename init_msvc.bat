@echo off

if "%~1"=="" (
    echo Usage: init_msvc.bat output_file.txt
    exit /b 1
)

for /F "tokens=1-4 delims=:.," %%a in ("%time%") do (
	set /A "vsdevcmd_start_time=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)

REM set VSCMD_DEBUG=3
REM NOTE: Uncomment or change one of these lines to match your installation of Visual Studio compiler
REM call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
REM call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64 -no_logo
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 > NUL 2>&1

for /F "tokens=1-4 delims=:.," %%a in ("%time%") do (
	set /A "vsdevcmd_end_time=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)
set /A vsdevcmd_elapsed_hundredths=vsdevcmd_end_time-vsdevcmd_start_time
set /A vsdevcmd_elapsed_seconds_part=vsdevcmd_elapsed_hundredths/100
set /A vsdevcmd_elapsed_hundredths_part=vsdevcmd_elapsed_hundredths%%100
if %vsdevcmd_elapsed_hundredths_part% lss 10 set vsdevcmd_elapsed_hundredths_part=0%vsdevcmd_elapsed_hundredths_part%
echo VsDevCmd.bat took %vsdevcmd_elapsed_seconds_part%.%vsdevcmd_elapsed_hundredths_part% seconds

set > "%~1"