@echo off

REM By default blocks like IF and FOR in batch are treated as a single command and variable values are expended when the block is entered
REM Enabling "delayed expansion" allows us to use !variable! syntax inside blocks to get the value of variables on that line (and at each iteration in the case of loops)
setlocal enabledelayedexpansion

if not exist _build mkdir _build
pushd _build
set root=..
set scripts=%root%\_scripts
set tools=%root%\third_party\_tools\win32
set build_config_path=%root%\build_config.h

python --version > NUL 2> NUL
if errorlevel 1 (
	echo WARNING: Python isn't installed on this computer. Shaders cant be found and compiled, and javascript glue files can't be concatenated together for WebAssembly builds!
)

set initialized_msvc_compiler=0
set tool_compiler_flags=/std:clatest /O2 /FC /nologo /Zi /I"%root%"

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
	cl %tool_compiler_flags% %root%\tools\tools_extract_define_main.c /Fe%extract_define_tool_name%
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
REM set extract_define=python %scripts%\extract_define.py ../build_config.h
set extract_define=%extract_define_tool_name% %build_config_path%
for /f "delims=" %%i in ('%extract_define% DEBUG_BUILD') do set DEBUG_BUILD=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_PIGGEN') do set BUILD_PIGGEN=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_PIG_BUILD') do set BUILD_PIG_BUILD=%%i
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
for /f "delims=" %%i in ('%extract_define% RUN_FUZZER') do set RUN_FUZZER=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_WINDOWS') do set BUILD_WINDOWS=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_LINUX') do set BUILD_LINUX=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_WEB') do set BUILD_WEB=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_ORCA') do set BUILD_ORCA=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_PLAYDATE_DEVICE') do set BUILD_PLAYDATE_DEVICE=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_PLAYDATE_SIMULATOR') do set BUILD_PLAYDATE_SIMULATOR=%%i
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
	cl %tool_compiler_flags% %root%\tools\tools_pig_build_main.c /Fe%pig_build_tool_name% /Fd"%pig_build_pdb_name%" /link Shlwapi.lib
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

exit

:: If we didn't already initialize MSVC compiler for build tools and we're building WINDOWS then let's do that now
if "%initialized_msvc_compiler%"=="0" (
	if "%BUILD_WINDOWS%"=="1" (
		CALL :init_msvc_compiler
		set initialized_msvc_compiler=1
	) else if "%BUILD_PLAYDATE_SIMULATOR%"=="1" (
		CALL :init_msvc_compiler
		set initialized_msvc_compiler=1
	)
)

if "%USE_EMSCRIPTEN%"=="1" (
	call "C:\gamedev\lib\emsdk\emsdk_env.bat" > NUL 2> NUL
)

if "%BUILD_ORCA%"=="1" (
	for /f "delims=" %%i in ('"orca sdk-path"') do set orca="%%i"
)

set playdate_sdk_directory=%PLAYDATE_SDK_PATH%
set pdc_exe_name=%playdate_sdk_directory%\bin\pdc
set playdate_arm_compiler_prefix=arm-none-eabi
set playdate_chip=cortex-m7
set playdate_heap_size=8388208
set playdate_stack_size=61800

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
:: TODO: -nostdlib ?
:: TODO: -Wl,--initial-memory=6553600 ?
set wasm_clang_flags=--target=wasm32 -mbulk-memory -I "../%root%"
:: -Wl, = Pass the following argument(s) to the linker
:: --no-entry = ?
:: --export-dynamic = ?
:: --sysroot = ?
:: -lorca_wasm = ?
:: -D __ORCA__ = #define __ORCA__ so that base_compiler_check.h can set TARGET_IS_ORCA
set orca_clang_flags=-Wl,--no-entry -Wl,--export-dynamic --sysroot %orca%/orca-libc -I "%orca%/src" -I "%orca%/src/ext" -L "%orca%/bin" -lorca_wasm -D __ORCA__
set web_clang_flags=
if "%USE_EMSCRIPTEN%"=="1" (
	set wasm_clang_flags=%wasm_clang_flags% -sUSE_SDL -sALLOW_MEMORY_GROWTH
) else (
	REM --no-entry = ?
	REM --allow-undefined = ?
	REM --no-standard-libraries = ?
	REM --no-standard-includes = ?
	REM --export=__heap_base = ?
	set web_clang_flags=%web_clang_flags% -Wl,--no-entry -Wl,--allow-undefined -I "../%root%/wasm/std/include" --no-standard-libraries --no-standard-includes -Wl,--export=__heap_base
)

set playdate_cl_flags=/D"__HEAP_SIZE=%playdate_heap_size%" /D"__STACK_SIZE=%playdate_stack_size%" /D "TARGET_EXTENSION=1" /I"%root%" /I"%playdate_sdk_directory%\C_API"
set playdate_gcc_flags=-D__HEAP_SIZE=%playdate_heap_size% -D__STACK_SIZE=%playdate_stack_size% -D "TARGET_EXTENSION=1" -I "%root%" -I "%playdate_sdk_directory%\C_API"

set playdate_sim_cl_flags=/D "TARGET_SIMULATOR=1" /D "_WINDLL" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "_WINDLL=1"
:: /GS = Buffer overrun protection is turned on
:: /Zi = Generates complete debugging information
:: /Gm- = Deprecated. Enables minimal rebuild
:: /Od = Disables optimization TODO: Debug only?
:: /RTC1 = Enable fast runtime checks (equivalent to /RTCsu)
:: /std:clatest = Use latest C language spec features
:: /experimental:c11atomics = Enables _Atomic types
:: /Gd = Uses the __cdecl calling convention (x86 only)
:: /MDd = Compiles to create a debug multithreaded DLL, by using MSVCRTD.lib
:: /Ob0 = Controls inline expansion (0 level = no expansion?)
:: /W3 = Set output warning level
:: /WX- = (Don't?) Treat warnings as errors TODO: Do we need this?
:: /nologo = Suppress the startup banner
set playdate_sim_cl_flags=%playdate_sim_cl_flags% /GS /Zi /Gm- /Od /RTC1 /std:clatest /experimental:c11atomics /Gd /MDd /Ob0 /W3 /WX- /nologo
:: /errorReport:prompt = Deprecated. Windows Error Reporting (WER) settings control error reporting TODO: Do we need this?
:: /diagnostics:column =  Diagnostics format: prints column information. TODO: Do we need this? (Optional)
:: /Zc:forScope = Enforce Standard C++ for scoping rules (on by default)
:: /Zc:inline = Remove unreferenced functions or data if they're COMDAT or have internal linkage only (off by default)
:: /Zc:wchar_t = wchar_t is a native type, not a typedef (on by default)
:: /fp:precise = "precise" floating-point model; results are predictable
set playdate_sim_cl_flags=%playdate_sim_cl_flags% /errorReport:prompt /Zc:forScope /Zc:inline /Zc:wchar_t /fp:precise
set playdate_sim_linker_flags=/MANIFEST /NXCOMPAT /DYNAMICBASE /DEBUG /DLL /MACHINE:X64 /INCREMENTAL /SUBSYSTEM:CONSOLE /ERRORREPORT:PROMPT /NOLOGO /TLBID:1
set playdate_sim_linker_flags=%playdate_sim_linker_flags% /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /ManifestFile:"tests.intermediate.manifest" /LTCGOUT:"tests.iobj" /ILK:"tests.ilk"
set playdate_sim_libraries="kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "comdlg32.lib" "advapi32.lib"

set playdate_dev_gcc_common_flags=-D "TARGET_PLAYDATE=1"
:: -mfloat-abi=hard = Use hardware instructions for floating-point operations (from FPU)
:: -mfpu=fpv5-sp-d16 = Specifies the target FPU architecture, that is the floating-point hardware available on the target. (Armv7 FPv5-SP-D16 floating-point extension) (from FPU)
:: __FPU_USED = ? (from FPU)
:: -mthumb = Requests that the compiler targets the T32 (Thumb) instruction set instead of A32 (Arm) (from MCFLAGS)
:: -mcpu=cortex-m7 = Enables code generation for a specific Arm processor. (from MCFLAGS)
:: -specs=nano.specs = Required for things like _read, _write, _exit, etc. to not be pulled in as requirements from standard library (https://stackoverflow.com/questions/5764414/undefined-reference-to-sbrk) and (https://devforum.play.date/t/c-api-converting-string-to-float/10097)
:: -specs=nosys.specs = ?
set playdate_dev_gcc_common_flags=%playdate_dev_gcc_common_flags% -mthumb -mcpu=%playdate_chip% -mfloat-abi=hard -mfpu=fpv5-sp-d16 -D__FPU_USED=1 -specs=nano.specs -specs=nosys.specs
:: -g3 = Produce debugging information in the operating system's native format (3 = ?)
:: -MD = (MSVC Option) Use the multithread-specific and DLL-specific version of the run-time library
:: -MT = (MSVC Option) Use the multithread, static version of the run-time library
:: -MP = This option instructs CPP to add a phony target for each dependency other than the main file, causing each to depend on nothing. These dummy rules work around errors make gives if you remove header files without updating the Makefile to match
::       ==OR== (MSVC Option) Build multiple source files concurrently (removed)
:: -MF tests.d = (Optional) When used with the driver options -MD or -MMD, -MF overrides the default dependency output file
:: -gdwarf-2 = Produce debugging information in DWARF version 2 format (if that is supported). This is the format used by DBX on IRIX 6. With this option, GCC uses features of DWARF version 3 when they are useful; version 3 is upward compatible with version 2, but may still cause problems for older debuggers.
set playdate_dev_gcc_compile_flags= -g3 -MD -MF tests.d -gdwarf-2
:: -fverbose-asm = ?
:: -fno-common = ?
:: -falign-functions = ? (from OPT)
:: -fomit-frame-pointer = ? (from OPT)
:: -ffunction-sections = ?
:: -fdata-sections = ?
:: -fno-exceptions = ? (Removed)
:: -mword-relocations = ?
set playdate_dev_gcc_compile_flags=%playdate_dev_gcc_compile_flags% -fverbose-asm -fno-common -falign-functions=16 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-exceptions -mword-relocations
:: -Wall = ?
:: -Wno-unknown-pragmas = ?
:: -Wdouble-promotion = ?
:: -Wno-comment = Don't warn about multi-line comments using // syntax
:: -Wno-switch = Don't warn about switch statements on enums with unhandled values
:: -Wno-nonnull = Don't warn about potentially null arguments passed to functions like memset
:: -Wno-unused = ?
:: -Wno-missing-braces = Suppress warning: missing braces around initializer
:: -Wno-char-subscripts = Suppress warning: array subscript has type 'char'
:: -Wno-double-promotion = Suppress warning: implicit conversion from 'float' to 'double' to match other operand of binary expression
:: -Wstrict-prototypes = (Removed because it isn't compatible with C++ builds?)
:: -Wa,-ahlms=%ProjectNameSafe%.lst = ? (Removed)
set playdate_dev_gcc_compile_flags=%playdate_dev_gcc_compile_flags% -Wall -Wno-unknown-pragmas -Wdouble-promotion -Wno-comment -Wno-switch -Wno-nonnull -Wno-unused -Wno-missing-braces -Wno-char-subscripts -Wno-double-promotion
:: -nostartfiles = ?
:: --entry eventHandlerShim
:: --no-warn-rwx-segments = Suppress warning about Exectuable+Writable LOAD segmenet (which is defined by the Playdate linker script presumably)
:: --cref = ?
:: --gc-sections = ?
:: --no-warn-mismatch = ?
:: --emit-relocs = ?
set playdate_dev_linker_flags=-nostartfiles --entry eventHandler -Wl,--no-warn-rwx-segments,--cref,--gc-sections,--no-warn-mismatch,--emit-relocs
set playdate_dev_linker_flags=%playdate_dev_linker_flags% -T"%playdate_sdk_directory%\C_API\buildsupport\link_map.ld"
set playdate_pdc_flags=-q -sdkpath "%playdate_sdk_directory%"

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
	REM -g = Generate debug information
	set wasm_clang_flags=%wasm_clang_flags% -g
	set shader_cl_flags=%shader_cl_flags% /MDd /Od /Zi
	REM -Wno-unused-parameter = warning: unused parameter 'numBytes'
	set common_clang_flags=%common_clang_flags% -Wno-unused-parameter -Wno-unused-variable
	REM -gdwarf-4 = Generate debug information in the DWARF format version 4 (gdb on WSL was not liking other versions of DWARF)
	set linux_clang_flags=%linux_clang_flags% -gdwarf-4
	rem -Og = No optimizations?
	set playdate_dev_gcc_compile_flags=%playdate_dev_gcc_compile_flags% -Og
) else (
	REM /MD = ?
	REM /Ot = Favors fast code over small code
	REM /Oy = Omit frame pointer [x86 only]
	REM /O2 = Optimization level 2: Creates fast code
	REM /Zi = Generate complete debugging information [optional]
	set common_cl_flags=%common_cl_flags% /MD /Ot /Oy /O2
	REM -O2 = Optimization level 2
	set wasm_clang_flags=%wasm_clang_flags% -O2
	set shader_cl_flags=%shader_cl_flags% /MD /Ot /Oy /O2
	set common_clang_flags=%common_clang_flags%
	rem TODO: Change this back to -O2 once we figure out why it's causing problems!
	rem -O2 = Optimize even more. GCC performs nearly all supported optimizations that do not involve a space-speed tradeoff. (from OPT)
	set playdate_dev_gcc_compile_flags=%playdate_dev_gcc_compile_flags% -O1
)

:: Gdi32.lib = Needed for CreateFontA and other Windows graphics functions
:: User32.lib = Needed for GetForegroundWindow, GetDC, etc.
:: Ole32.lib = Needed for Combaseapi.h, CoInitializeEx, CoCreateInstance, etc.
:: Shell32.lib = Needed for SHGetSpecialFolderPathA
:: Shlwapi.lib = Needed for PathFileExistsA
set tests_libraries=Gdi32.lib User32.lib Ole32.lib Shell32.lib Shlwapi.lib
set tests_clang_libraries=-lfontconfig
set piggen_libraries=Shlwapi.lib
set piggen_clang_libraries=
set pig_core_dll_libraries=Gdi32.lib User32.lib Ole32.lib Shell32.lib Shlwapi.lib
set pig_core_clang_libraries=-lfontconfig
if "%BUILD_WITH_RAYLIB%"=="1" (
	REM raylib.lib   = Raylib (NOTE: It has to come BEFORE User32.lib otherwise CloseWindow will conflict)
	REM kernel32.lib = ?
	REM winmm.lib    = ?
	REM Winhttp.lib  = ?
	REM Advapi32.lib = Processthreadsapi.h, OpenProcessToken, GetTokenInformation
	set tests_libraries=raylib.lib %tests_libraries% kernel32.lib winmm.lib
	REM NOTE: Compiling for Linux with raylib would require following instructions here: https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux
	set pig_core_dll_libraries=raylib.lib %pig_core_dll_libraries% kernel32.lib winmm.lib
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
	set pig_core_clang_libraries=%pig_core_clang_libraries% -lGL
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

if "%RUN_FUZZER%"=="1" (
	REM /fsanitize=fuzzer = Enable the fuzzing entry-point so we can run tests using the "Fuzzing" approach to generate random inputs
	set common_cl_flags=%common_cl_flags% /fsanitize=fuzzer
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
			cl /c %common_cl_flags% %c_cl_flags% /I"!shader_file_dir!" /Fo"!object_name!" !shader_file_path!
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
set piggen_cl_args=%common_cl_flags% %c_cl_flags% /Fe%piggen_exe_path% %piggen_source_path% /link %common_ld_flags% %piggen_libraries%
set piggen_clang_args=%common_clang_flags% %linux_clang_flags% %linux_linker_flags% -o %piggen_bin_path% ../%piggen_source_path% %piggen_clang_libraries%
if "%BUILD_PIGGEN%"=="1" (
	if "%BUILD_WINDOWS%"=="1" (
		echo.
		echo [Building %piggen_exe_path% for Windows...]
		del %piggen_exe_path% > NUL 2> NUL
		cl %piggen_cl_args%
		if "%DUMP_PREPROCESSOR%"=="1" (
			COPY main.i piggen_preprocessed.i > NUL
		)
		echo [Built %piggen_exe_path% for Windows!]
	)
	if "%BUILD_LINUX%"=="1" (
		echo.
		echo [Building %piggen_bin_path% for Linux...]
		if not exist linux mkdir linux
		pushd linux
		
		wsl clang-18 %piggen_clang_args%
		
		popd
		echo [Built %piggen_bin_path% for Linux!]
	)
)

if "%RUN_PIGGEN%"=="1" (
	echo.
	echo [%piggen_exe_path%]
	%piggen_exe_path% %root% -o="gen/" -e="%root%/third_party" -e="%root%/piggen" -e="%root%/wasm/std" -e="%root%/.git" -e="%root%/_scripts" -e="%root%/_media" -e="%root%/_template" -e="%root%/base/base_defines_check.h"
)

:: +--------------------------------------------------------------+
:: |                       Build imgui.obj                        |
:: +--------------------------------------------------------------+
set imgui_source_path=%root%/ui/ui_imgui_main.cpp
set imgui_obj_path=imgui.obj
set imgui_cl_args=/c %common_cl_flags% %cpp_cl_flags% /I"%root%\third_party\imgui" /Fo%imgui_obj_path% %imgui_source_path%
if "%BUILD_WITH_IMGUI%"=="1" (
	set pig_core_dll_libraries=%pig_core_dll_libraries% %imgui_obj_path%
	set pig_core_clang_libraries=%pig_core_clang_libraries% %imgui_obj_path%
	set tests_libraries=%tests_libraries% %imgui_obj_path%
)
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
:: |                     Build physx_capi.obj                     |
:: +--------------------------------------------------------------+
set physx_source_path=%root%/phys/phys_physx_capi_main.cpp
set physx_obj_path=physx_capi.obj
set physx_cl_args=/c %common_cl_flags% %cpp_cl_flags% /I"%root%\third_party\physx" /Fo%physx_obj_path% %physx_source_path%
if "%BUILD_WITH_PHYSX%"=="1" (
	set pig_core_dll_libraries=%pig_core_dll_libraries% %physx_obj_path% PhysX_static_64.lib
	set pig_core_clang_libraries=%pig_core_clang_libraries% %physx_obj_path% PhysX_static_64.lib
	set tests_libraries=%tests_libraries% %physx_obj_path% PhysX_static_64.lib
)

if "%BUILD_PHYSX_OBJ%"=="1" (
	if "%BUILD_WINDOWS%"=="1" (
		del "%physx_obj_path%" > NUL 2> NUL
		
		echo.
		echo [Building %physx_obj_path% for Windows...]
		cl %physx_cl_args%
		echo [Built %physx_obj_path% for Windows!]
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
set pig_core_so_clang_args=%common_clang_flags% %linux_clang_flags% %linux_linker_flags% %pig_core_clang_libraries% -fPIC -shared -o %pig_core_so_path% ../%pig_core_dll_source_path%
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
set tests_obj_path=tests.obj
set tests_exe_path=tests.exe
set tests_elf_path=pdex.elf
set tests_dll_path=pdex.dll
set tests_lib_path=pdex.lib
set tests_pdb_path=pdex.pdb
set tests_map_name=tests.map
set tests_pdx_name=tests.pdx
set tests_bin_path=tests
set tests_wasm_path=app.wasm
set tests_orca_wasm_path=module.wasm
set tests_wat_path=app.wat
set tests_html_path=index.html
set tests_win_input_files=%tests_source_path%
set tests_linux_input_files=../%tests_source_path%
if "%BUILD_WITH_SOKOL_GFX%"=="1" (
	set tests_win_input_files=%tests_win_input_files% %shader_object_files%
	set tests_linux_input_files=%tests_linux_input_files% %shader_linux_object_files%
)
set tests_cl_args=%common_cl_flags% %c_cl_flags% /Fe%tests_exe_path% %tests_win_input_files% /link %common_ld_flags% %tests_libraries%
set tests_clang_args=%common_clang_flags% %linux_clang_flags% %linux_linker_flags% %tests_clang_libraries% -o %tests_bin_path% %tests_linux_input_files%
if "%ENABLE_AUTO_PROFILE%"=="1" (
	set tests_clang_args=-finstrument-functions %tests_clang_args%
)
set tests_web_args=%common_clang_flags% %wasm_clang_flags% %web_clang_flags% ../%tests_source_path%"
if "%USE_EMSCRIPTEN%"=="1" (
	set tests_web_args=-o %tests_html_path% %tests_web_args%
) else (
	set tests_web_args=-o %tests_wasm_path% %tests_web_args%
)
set tests_orca_clang_args=%common_clang_flags% %wasm_clang_flags% %orca_clang_flags% -o %tests_orca_wasm_path% ../%tests_source_path%
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
	if "%BUILD_ORCA%"=="1" (
		echo.
		echo [Building tests for Orca...]
		if not exist orca mkdir orca
		pushd orca
		
		del %tests_orca_wasm_path% > NUL 2> NUL
		clang %tests_orca_clang_args%
		
		echo [Bundling...]
		
		rmdir /s /q tests
		orca bundle --name tests %tests_orca_wasm_path%
		
		popd
		echo [Built tests for Orca!]
	)
	if "%BUILD_PLAYDATE_DEVICE%"=="1" (
		REM compile to .obj
		%playdate_arm_compiler_prefix%-gcc -c %tests_source_path% -o %tests_obj_path% %playdate_gcc_flags% %playdate_dev_gcc_common_flags% %playdate_dev_gcc_compile_flags%
		REM then link into .elf
		%playdate_arm_compiler_prefix%-gcc %tests_obj_path% %playdate_gcc_flags% %playdate_dev_gcc_common_flags% %playdate_dev_linker_flags% -o %tests_elf_path% -Wl,-Map=%tests_map_name%
		
		if not exist playdate_data mkdir playdate_data
		COPY "%tests_elf_path%" "playdate_data\%tests_elf_path%"
		COPY "%root%\pdxinfo" "playdate_data\pdxinfo"
		"%pdc_exe_name%" %playdate_pdc_flags% "playdate_data" "%tests_pdx_name%"
	)
	if "%BUILD_PLAYDATE_SIMULATOR%"=="1" (
		del playdate_data\%tests_dll_path% > NUL 2> NUL
		del playdate_data\pdxinfo > NUL 2> NUL
		cl /Fo"%tests_obj_path%" %playdate_cl_flags% %playdate_sim_cl_flags% /c "%tests_source_path%"
		LINK %playdate_sim_linker_flags% %playdate_sim_libraries% "%tests_obj_path%" /OUT:"%tests_dll_path%" /IMPLIB:"%tests_lib_path%" /PDB:"%tests_pdb_path%"
		if not exist playdate_data mkdir playdate_data
		COPY "%tests_dll_path%" "playdate_data\%tests_dll_path%"
		COPY "%root%\pdxinfo" "playdate_data\pdxinfo"
		"%pdc_exe_name%" %playdate_pdc_flags% "playdate_data" "%tests_pdx_name%"
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