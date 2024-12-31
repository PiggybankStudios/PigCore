@echo off

if not exist _build mkdir _build
pushd _build
set root=..

:: +==============================+
:: |    Scrape build_config.h     |
:: +==============================+
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

:: +==============================+
:: |        Find Compiler         |
:: +==============================+
:: NOTE: Uncomment or change one of these lines to match your installation of Visual Studio compiler
:: call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
:: call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64 -no_logo
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 > NUL

:: +==============================+
:: |       Compiler Options       |
:: +==============================+
:: /FC = Full path for error messages
:: /nologo = Suppress the startup banner
:: /W4 = Warning level 4 [just below /Wall]
:: /WX = Treat warnings as errors
set common_cl_flags=/FC /nologo /W4 /WX
:: /wd4130 = Logical operation on address of string constant [W4] TODO: Should we re-enable this one? Don't know any scenarios where I want to do this
:: /wd4201 = Nonstandard extension used: nameless struct/union [W4] TODO: Should we re-enable this restriction for ANSI compatibility?
:: /wd4324 = Structure was padded due to __declspec[align[]] [W4]
:: /wd4458 = Declaration of 'identifier' hides class member [W4]
:: /wd4505 = Unreferenced local function has been removed [W4]
:: /wd4996 = Usage of deprecated function, class member, variable, or typedef [W3]
:: /wd4127 = Conditional expression is constant [W4]
:: /wd4706 = assignment within conditional expression [W?]
set common_cl_flags=%common_cl_flags% /wd4130 /wd4201 /wd4324 /wd4458 /wd4505 /wd4996 /wd4127 /wd4706
if "%DEBUG_BUILD%"=="1" (
	REM /Od = Optimization level: Debug
	REM /Zi = Generate complete debugging information
	REM /wd4065 = Switch statement contains 'default' but no 'case' labels
	REM /wd4100 = Unreferenced formal parameter [W4]
	REM /wd4101 = Unreferenced local variable [W3]
	REM /wd4127 = Conditional expression is constant [W4]
	REM /wd4189 = Local variable is initialized but not referenced [W4]
	REM /wd4702 = Unreachable code [W4]
	set common_cl_flags=%common_cl_flags% /Od /Zi /wd4065 /wd4100 /wd4101 /wd4127 /wd4189 /wd4702
) else (
	REM /Ot = Favors fast code over small code
	REM /Oy = Omit frame pointer [x86 only]
	REM /O2 = Optimization level 2: Creates fast code
	REM /Zi = Generate complete debugging information [optional]
	set common_cl_flags=%common_cl_flags% /Ot /Oy /O2
)

set common_ld_flags=

:: +--------------------------------------------------------------+
:: |                       Build piggen.exe                       |
:: +--------------------------------------------------------------+
:: /Fe = Set the output file name
set piggen_cl_flags=%common_cl_flags% /Fepiggen.exe
set piggen_ld_flags=%common_ld_flags%
if "%BUILD_PIGGEN%"=="1" (
	cl %piggen_cl_flags% %root%\piggen\main.c /link %piggen_ld_flags%
)

:: +--------------------------------------------------------------+
:: |                       Build tests.exe                        |
:: +--------------------------------------------------------------+
set tests_cl_flags=%common_cl_flags% /Fetests.exe
set tests_ld_flags=%common_ld_flags%
if "%BUILD_TESTS%"=="1" (
	cl %tests_cl_flags% %root%\tests\main.c /link %tests_ld_flags%
)
if "%RUN_TESTS%"=="1" (
	tests.exe
)

popd