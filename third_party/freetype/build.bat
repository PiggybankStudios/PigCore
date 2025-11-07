@echo off
setlocal enabledelayedexpansion

if not exist _build mkdir _build
pushd _build

call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 > NUL

set root=..

set source_files=
set source_files=%source_files% %root%\src\base\ftsystem.c
set source_files=%source_files% %root%\src\base\ftinit.c
set source_files=%source_files% %root%\src\base\ftdebug.c
set source_files=%source_files% %root%\src\base\ftbase.c
set source_files=%source_files% %root%\src\base\ftbbox.c
set source_files=%source_files% %root%\src\base\ftglyph.c
set source_files=%source_files% %root%\src\base\ftbitmap.c
set source_files=%source_files% %root%\src\sfnt\sfnt.c
set source_files=%source_files% %root%\src\truetype\truetype.c
set source_files=%source_files% %root%\src\smooth\smooth.c
set source_files=%source_files% %root%\src\psnames\psnames.c

set compiler_flags=/Od /FC /nologo /Zi /I"%root%\include" /DFT2_BUILD_LIBRARY
set linker_flags=/nologo /OUT:"freetype.lib"

cl /c %compiler_flags% %source_files%
lib %linker_flags% *.obj

popd
